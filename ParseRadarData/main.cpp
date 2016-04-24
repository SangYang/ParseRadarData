#if 1
#include <stdio.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <pthread.h>
#endif // _WIN32

#include "radar_data_interface.h"

#ifdef _WIN32
static void RadarDataListFunc(void *pv) {
#else
static void* RadarDataListFunc(void *pv) {
#endif // _WIN32
	bool ok_run;

	while (true) {
		ok_run = RunRadarDataList();
		if (false == ok_run)
			continue;
	}
}

int main() {
	t_PlateArea plate_area = {
		{{200, 300}, {260, 300}, {260, 330}, {200, 330}}
	};
/*
	t_PlateArea plate_area = {
		{{200, 300}, {300, 300}, {300, 400}, {200, 400}}
	};*/
/*
	t_PlateArea plate_area = {
		{{400, 300}, {500, 300}, {500, 400}, {400, 400}}
	};*/

	double speed;
	int speed_confidence;
	bool ok_get;
#ifdef __linux
	pthread_t list_func_id;
#endif // __linux

	printf("Main() start...\n");
#ifdef _WIN32
	_beginthread(&RadarDataListFunc, 10240, NULL);
	//Sleep(5000);
#else
	pthread_create(&list_func_id, NULL, &RadarDataListFunc, NULL);
#endif // _WIN32
	
	printf("Enter loop...\n");
	while (true) {
		ok_get = GetSpeed(plate_area, &speed, &speed_confidence);
		if (false == ok_get) {
			printf("get_speed failure!\n");
		}
		else {
			printf("speed: %f, confidence: %d\n", speed, speed_confidence);
		}
	}

	return 0;
}
#endif





#if 0
#include <stdio.h>
#include <string.h>
#include "debug_assert.h"
#include "debug_log.h"
#include "com_port.h"
#include "device_config.h"
#include "grid_coord.h"
#include "parse_radar_data.h"


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

int main() {
	const int c_PerComDataSize = 1024;
	unsigned char com_data[1024];
	int act_com_size;
	bool ok_com;

	t_Point radar_point;
	t_Point camera_point;
	int similay;
	int max_similary;
	int area_max_similary;
	int compare_area_width;
	int compare_area_length;
	radar_point.m_x = 68;
	radar_point.m_y = -4;
	camera_point.m_x = 208;
	camera_point.m_y = 700;
	compare_area_width = 2;
	compare_area_length = 1;
	CompareRadarAndCameraInArea(radar_point, camera_point, compare_area_width, compare_area_length, &area_max_similary);

	LOG("start");
	ok_com = OpenComport(2, 115200);
	if (!ok_com)
		printf("OpenComport() error!\n");
	else {
		while (true) {
			printf("next>>\n");
			act_com_size = ReadComport(com_data, c_PerComDataSize);
			if (0 == act_com_size)
				continue;
			else
				AnalyseComData(com_data, act_com_size);
		}

		CloseComport();
	}

	return 0;
}
#endif


