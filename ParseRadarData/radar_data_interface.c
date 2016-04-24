#include "radar_data_interface.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "debug_assert.h"
#include "debug_log.h"
#include "com_port.h"
#include "device_config.h"
#include "grid_coord.h"
#include "radar_data_list.h"
#include "parse_radar_data.h"


#define  ARRAY_LENGTH(a)  (sizeof(a) / sizeof(a[0]))

static void CompareRadarAndCameraInPoint(const t_Point radar_point, const t_Point camera_point, int *p_similary) {
	t_Polygon radar_area;
	t_Polygon camera_area;
	t_GridCoord radar_coord;
	t_GridCoord camera_coord;
	char config_path[256];
	bool ok_radar_coord;
	bool ok_camera_coord;
	int similary;

	GetConfigPath(config_path, sizeof(config_path));
	GetRadarArea(config_path, &radar_area);
	GetCameraArea(config_path, &camera_area);
	ok_radar_coord = GetGridCoord(radar_area, radar_point, &radar_coord);
	ok_camera_coord = GetGridCoord(camera_area, camera_point, &camera_coord);
	if (false == ok_radar_coord || false == ok_camera_coord) 
		similary = 0;
	else
		similary = GetGridCoordSimilary(radar_coord, camera_coord);
	*p_similary = similary;
}

static void CompareRadarAndCameraInLine(const t_Point radar_point, const t_Point camera_point, const double line_width, int *p_max_similary) {
	t_Point per_radar_point;
	int per_similary;
	int compare_count;
	int compare_index;
	int max_similary;
	char config_path[255];
	int basic_width;

	ASSERT(NULL != p_max_similary);
	max_similary = 0;
	GetConfigPath(config_path, sizeof(config_path));
	GetBasicWidthOfRadarCompareArea(config_path, &basic_width);
	compare_count = (int)(line_width / basic_width * 1000);
	for (compare_index = 0; compare_index < compare_count; compare_index++) {
		per_radar_point.m_x = radar_point.m_x;
		per_radar_point.m_y = radar_point.m_y + (double)(basic_width) * compare_index / 1000;
		CompareRadarAndCameraInPoint(per_radar_point, camera_point, &per_similary);
		if (per_similary > max_similary)
			max_similary = per_similary;
		else
			continue;
	}
	*p_max_similary = max_similary;
}

static void CompareRadarAndCameraInArea(const t_Point radar_point, const t_Point camera_point, const double area_width, const double area_length, int *p_max_similary) {
	t_Point per_radar_point;
	int per_similary;
	int compare_count;
	int compare_index;
	int max_similary;
	char config_path[255];
	int basic_length;

	ASSERT(NULL != p_max_similary);
	max_similary = 0;
	GetConfigPath(config_path, sizeof(config_path));
	GetBasicLengthOfRadarCompareArea(config_path, &basic_length);
	compare_count = (int)(area_length / basic_length * 1000);
	for (compare_index = 0; compare_index < compare_count; compare_index++) {
		per_radar_point.m_x = radar_point.m_x - area_length / 2 + (double)(basic_length) * compare_index / 1000;
		per_radar_point.m_y = radar_point.m_y;
		CompareRadarAndCameraInLine(per_radar_point, camera_point, area_width, &per_similary);
		if (per_similary > max_similary)
			max_similary = per_similary;
		else
			continue;
	}
	*p_max_similary = max_similary;
}


static bool FindCar(const t_PlateArea plate_area, const t_CarArray car_array, t_Car *p_car, int *p_car_similary) {
	const int c_SimilaryMax = 120;
	t_Car car;
	t_PixelsCoord pixels_coord;
	t_Point radar_point;
	t_Point camera_point;
	int car_count;
	int car_index;
	int plate_count;
	int plate_index;
	int max_similary;
	int per_similary;
	int target_index;
	char config_path[255];
	int total_width;
	int total_length;

	ASSERT(NULL != p_car && NULL != p_car_similary);
	max_similary = 0;
	GetConfigPath(config_path, sizeof(config_path));
	GetTotalWidthOfRadarCompareArea(config_path, &total_width);
	GetTotalLengthOfRadarCompareArea(config_path, &total_length);
	plate_count = ARRAY_LENGTH(plate_area.m_coord);
	car_count = car_array.m_car_count;
	for (plate_index = 0; plate_index < plate_count; plate_index++) {
		memcpy(&pixels_coord, &plate_area.m_coord[plate_index], sizeof(t_PixelsCoord));
		for (car_index = 0; car_index < car_count; car_index++) {
			memcpy(&car, &car_array.m_car[car_index], sizeof(t_Car));
			if (1 == car_count) {
				memcpy(p_car, &car, sizeof(t_Car));
				*p_car_similary = c_SimilaryMax + 3;
				return true;
			}
			else if (c_SimilaryMax == max_similary) {
				memcpy(p_car, &car, sizeof(t_Car));
				*p_car_similary = c_SimilaryMax;
				return true;
			}
			else {
				radar_point.m_x = car.m_position.m_x;
				radar_point.m_y = car.m_position.m_y;
				camera_point.m_x = pixels_coord.m_x;
				camera_point.m_y = pixels_coord.m_y;
				CompareRadarAndCameraInArea(radar_point, camera_point, (double)(total_width) / 1000, (double)(total_length) / 1000, &per_similary);
				if (max_similary < per_similary) {
					max_similary = per_similary;
					target_index = car_index;
				}
				else
					continue;
			}
		}
	}
	if (0 == max_similary) {
		*p_car_similary = 0;
		return false;
	}
	else {
		memcpy(p_car, &car_array.m_car[target_index], sizeof(t_Car));
		*p_car_similary = max_similary;
		return true;
	}
}

static void GetSpeedXY(const double speed_x, const double speed_y, double *p_speed_xy) {
	double speed_xy;
	double speed_xy_square;

	ASSERT(NULL != p_speed_xy);
	speed_xy_square = pow(speed_x, 2) + pow(speed_y, 2);
	speed_xy = sqrt(speed_xy_square);
	*p_speed_xy = speed_xy;
}

bool GetSpeed(const t_PlateArea plate_area, double *p_speed, int *p_speed_confidence) {
	t_CarArray car_array;
	t_Car car;
	double speed;
	bool ok_get;
	bool ok_find;
	int car_similary;
	char config_path[255];
	int past_num;

	ASSERT(NULL != p_speed && NULL != p_speed_confidence);
	GetConfigPath(config_path, sizeof(config_path));
	GetPastNumOfRadarDataList(config_path, &past_num);
	ok_get = GetCarArrayFromList(past_num, &car_array);
	if (false == ok_get) {
		return false;
	}
	else {
		ok_find = FindCar(plate_area, car_array, &car, &car_similary);
		if (false == ok_find) {
			return false;
		}
		else {
			speed = car.m_speed.m_x;
			*p_speed = speed;
			*p_speed_confidence = car_similary;
			return true;
		}
	}
}

bool RunRadarDataList() {
	const int c_PerComDataSize = 1024;
	unsigned char com_data[1024];
	int act_com_size;
	bool ok_com;
	char config_path[255];
	int port_num;
	int baud_rate;

	GetConfigPath(config_path, sizeof(config_path));
	GetPortNumOfSerialInterface(config_path, &port_num);
	GetBaudRateOfSerialInterface(config_path, &baud_rate);
	ok_com = OpenComport(port_num, baud_rate);
	if (false == ok_com) {
		LOG("OpenComport() error!\n");
		return false;
	}
	else {
		while (true) {
			act_com_size = ReadComport(com_data, c_PerComDataSize);
			if (0 == act_com_size)
				continue;
			else
				AnalyseComData(com_data, act_com_size);
		}
		CloseComport();
		return true;
	}
}
