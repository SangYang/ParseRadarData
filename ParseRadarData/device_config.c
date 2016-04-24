#include "device_config.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

#include "debug_assert.h"
#include "debug_log.h"
#include "grid_coord.h"

#define ARRAY_LENGTH(a)          (sizeof(a) / sizeof(a[0]))

#ifdef __linux
typedef unsigned long   DWORD;

DWORD GetPrivateProfileString(const char *p_app_name, const char *p_key_name, const char *p_default, char *p_returned_string, DWORD returned_string_size, const char *p_file_name) {
	FILE *pf_ini;
	char line[256];
	char *p_find_app;
	char *p_find_key;
	int returned_length;
	DWORD dw_result;

	ASSERT(NULL != p_app_name && NULL != p_key_name);
	ASSERT(NULL != p_returned_string && NULL != p_file_name);
	pf_ini = fopen(p_file_name, "rb");
	if (NULL == pf_ini)
		dw_result = 0;
	else {
		p_find_app = NULL;
		p_find_key = NULL;
		while (true) {
			fgets(line, sizeof(line), pf_ini);
			if (feof(pf_ini))
				break;
			else {
				if (NULL == p_find_app)
					p_find_app = strstr(line, p_app_name);
				else if (NULL == p_find_key) {
					p_find_key = strstr(line, p_key_name);
					if (NULL != p_find_key) {
						sscanf(line, "%*[^=]=%[^ \r\n]", p_returned_string);
						returned_length = strlen(p_returned_string);
						if (0 >= returned_length || returned_length >= returned_string_size)
							dw_result = 0;
						else
							dw_result = returned_length + 1;
						break;
					}
					else
						continue;
				}
			}
		}
		fclose(pf_ini);
	}
	return dw_result;	
}
#endif // __linux

static void GetWorkPath(char *p_work_path, const int work_path_size) {
#ifdef _WIN32
	HMODULE hmodule;
	DWORD act_size;
	char *p_find;

	ASSERT(NULL != p_work_path);
	hmodule = GetModuleHandle(NULL);
	act_size = GetModuleFileName(hmodule, p_work_path, work_path_size);
	ASSERT(0 != act_size);
	p_find = strrchr(p_work_path, '\\');
	ASSERT(NULL != p_find);
	*p_find = 0;
#else
	ssize_t act_size;
	char *p_find;

	act_size = readlink("/proc/self/exe", p_work_path, work_path_size);
	ASSERT(-1 != act_size);
	p_find = strrchr(p_work_path, '/');
	ASSERT(NULL != p_find);
	*p_find = 0;
#endif // _WIN32
}

void GetConfigPath(char *p_config_path, const int config_path_size) {
	char work_path[256];
	char config_path[256];
	int config_path_len;

	ASSERT(NULL != config_path && 0 != config_path_size);
	GetWorkPath(work_path, sizeof(work_path));
	strcpy(config_path, work_path);
#ifdef _WIN32
	strcat(config_path, "\\config.ini");
#else
	strcat(config_path, "/config.ini");
#endif // _WIN32
	config_path_len = strlen(config_path);
	ASSERT(config_path_len < config_path_size);
	strcpy(p_config_path, config_path);
}

static void GetPointFromAreaPoint(const char *p_area_point, t_Point *p_point) {
	char buffer[32];
	t_Point point;

	ASSERT(NULL != p_area_point && NULL != p_point);
	sscanf(p_area_point, "(%[^,]", buffer);
	point.m_x = atof(buffer);
	sscanf(p_area_point, "%*[^,],%[^)]", buffer);
	point.m_y = atof(buffer);
	memcpy(p_point, &point, sizeof(point));
}

void GetRadarArea(const char *p_config_path, void *pv_radar_area) {
	t_Polygon *p_radar_area = (t_Polygon *)pv_radar_area;
	char area_item[][32] = {"area_left_top", "area_right_top", "area_right_down", "area_left_down"};
	char area_point[32];
	int area_item_num;
	int area_item_index;
	DWORD area_copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_radar_area);
	area_item_num = ARRAY_LENGTH(area_item);
	for (area_item_index = 0; area_item_index < area_item_num; area_item_index++) {
		area_copy_byte = GetPrivateProfileString("radar", area_item[area_item_index], NULL, area_point, sizeof(area_point), p_config_path);	
		ASSERT(0 != area_copy_byte);
		GetPointFromAreaPoint(area_point, &p_radar_area->m_point[area_item_index]);
	}
}

void GetCameraArea(const char *p_config_path, void *pv_camera_area) {
	t_Polygon *p_camera_area = (t_Polygon *)pv_camera_area;
	char area_item[][32] = {"area_left_top", "area_right_top", "area_right_down", "area_left_down"};
	char area_point[32];
	int area_item_num;
	int area_item_index;
	DWORD area_copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_camera_area);
	area_item_num = ARRAY_LENGTH(area_item);
	for (area_item_index = 0; area_item_index < area_item_num; area_item_index++) {
		area_copy_byte = GetPrivateProfileString("camera", area_item[area_item_index], NULL, area_point, sizeof(area_point), p_config_path);	
		ASSERT(0 != area_copy_byte);
		GetPointFromAreaPoint(area_point, &p_camera_area->m_point[area_item_index]);
	}
}


void GetPastNumOfRadarDataList(const char *p_config_path, int *p_past_num) {
	char ach_past_num[16];
	int past_num;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_past_num);
	copy_byte = GetPrivateProfileString("radar_data_list", "past_num", NULL, ach_past_num, sizeof(ach_past_num), p_config_path);	
	ASSERT(0 != copy_byte);
	past_num = atoi(ach_past_num);
	*p_past_num = past_num;
}

void GetMemberNumOfRadarDataList(const char *p_config_path, int *p_member_num) {
	char ach_member_num[16];
	int member_num;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_member_num);
	copy_byte = GetPrivateProfileString("radar_data_list", "member_num", NULL, ach_member_num, sizeof(ach_member_num), p_config_path);	
	ASSERT(0 != copy_byte);
	member_num = atoi(ach_member_num);
	*p_member_num = member_num;
}

void GetBasicWidthOfRadarCompareArea(const char *p_config_path, int *p_basic_width) {
	char ach_basic_width[16];
	int basic_width;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_basic_width);
	copy_byte = GetPrivateProfileString("radar_compare_area", "basic_width", NULL, ach_basic_width, sizeof(ach_basic_width), p_config_path);	
	ASSERT(0 != copy_byte);
	basic_width = atoi(ach_basic_width);
	*p_basic_width = basic_width;
}

void GetBasicLengthOfRadarCompareArea(const char *p_config_path, int *p_basic_length) {
	char ach_basic_length[16];
	int basic_length;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_basic_length);
	copy_byte = GetPrivateProfileString("radar_compare_area", "basic_length", NULL, ach_basic_length, sizeof(ach_basic_length), p_config_path);	
	ASSERT(0 != copy_byte);
	basic_length = atoi(ach_basic_length);
	*p_basic_length = basic_length;
}

void GetTotalWidthOfRadarCompareArea(const char *p_config_path, int *p_total_width) {
	char ach_total_width[16];
	int total_width;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_total_width);
	copy_byte = GetPrivateProfileString("radar_compare_area", "total_width", NULL, ach_total_width, sizeof(ach_total_width), p_config_path);	
	ASSERT(0 != copy_byte);
	total_width = atoi(ach_total_width);
	*p_total_width = total_width;
}

void GetTotalLengthOfRadarCompareArea(const char *p_config_path, int *p_total_length) {
	char ach_total_length[16];
	int total_length;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_total_length);
	copy_byte = GetPrivateProfileString("radar_compare_area", "total_length", NULL, ach_total_length, sizeof(ach_total_length), p_config_path);	
	ASSERT(0 != copy_byte);
	total_length = atoi(ach_total_length);
	*p_total_length = total_length;
}

void GetPortNumOfSerialInterface(const char *p_config_path, int *p_port_num) {
	char ach_port_num[16];
	int port_num;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_port_num);
	copy_byte = GetPrivateProfileString("serial_interface", "port_num", NULL, ach_port_num, sizeof(ach_port_num), p_config_path);	
	ASSERT(0 != copy_byte);
	port_num = atoi(ach_port_num);
	*p_port_num = port_num;
}

void GetBaudRateOfSerialInterface(const char *p_config_path, int *p_baud_rate) {
	char ach_baud_rate[16];
	int baud_rate;
	DWORD copy_byte;

	ASSERT(NULL != p_config_path && NULL != p_baud_rate);
	copy_byte = GetPrivateProfileString("serial_interface", "baud_rate", NULL, ach_baud_rate, sizeof(ach_baud_rate), p_config_path);	
	ASSERT(0 != copy_byte);
	baud_rate = atoi(ach_baud_rate);
	*p_baud_rate = baud_rate;
}


