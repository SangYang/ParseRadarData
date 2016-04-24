#include "radar_data_list.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif // _WIN32

#include "debug_assert.h"
#include "debug_log.h"
#include "device_config.h"
#include "parse_radar_data.h"
#include "section_lock.h"

#define CAR_ARRAY_COUNT   100

typedef struct {
	t_CarArray m_car_array[CAR_ARRAY_COUNT];
	int        m_over_index;
	int        m_total_index;
} t_RadarDataList;

static t_RadarDataList g_radar_data_list = {0};
static bool g_is_first = true;

#ifdef _WIN32
static CRITICAL_SECTION g_list_lock;
#else
static pthread_mutex_t  g_list_lock;
#endif // _WIN32


static void g_SetRadarDataList(const t_RadarDataList radar_data_list) {
	EnterLock(&g_list_lock);
	memcpy(&g_radar_data_list, &radar_data_list, sizeof(t_RadarDataList));
	LeaveLock(&g_list_lock);
}

static void g_GetRadarDataList(t_RadarDataList *p_radar_data_list) {
	ASSERT(NULL != p_radar_data_list);
	EnterLock(&g_list_lock);
	memcpy(p_radar_data_list, &g_radar_data_list, sizeof(t_RadarDataList));
	LeaveLock(&g_list_lock);
}

void InsertCarArrayToList(const void *pv_car_array) {
	const t_CarArray *p_car_array = (const t_CarArray *)pv_car_array;
	t_RadarDataList radar_data_list;
	int current_index;
	char config_path[255];
	int member_num;

	ASSERT(NULL != pv_car_array);	
	if (true == g_is_first) {
		InitLock(&g_list_lock);
		g_is_first = false;
	}
	else
		NULL;
	GetConfigPath(config_path, sizeof(config_path));
	GetMemberNumOfRadarDataList(config_path, &member_num);
	g_GetRadarDataList(&radar_data_list);
	current_index = radar_data_list.m_over_index;
	if (0 == radar_data_list.m_total_index) {
		memcpy(&radar_data_list.m_car_array[0], p_car_array, sizeof(t_CarArray));
		radar_data_list.m_over_index = 0;
		radar_data_list.m_total_index++;
	}
	else if (member_num == radar_data_list.m_total_index && (member_num - 1) == radar_data_list.m_over_index) {
		memcpy(&radar_data_list.m_car_array[0], p_car_array, sizeof(t_CarArray));
		radar_data_list.m_over_index = 0;
	}
	else {
		memcpy(&radar_data_list.m_car_array[current_index + 1], p_car_array, sizeof(t_CarArray));
		radar_data_list.m_over_index++;
		if (member_num == radar_data_list.m_total_index)
			NULL;
		else
			radar_data_list.m_total_index++;
	}
	g_SetRadarDataList(radar_data_list);
}

bool GetCarArrayFromList(const int car_array_index, void *pv_car_array) {
	t_CarArray *p_car_array = (t_CarArray *)pv_car_array;
	t_RadarDataList radar_data_list;
	int index_differ;
	int list_index;

	if (true == g_is_first) {
		InitLock(&g_list_lock);
		g_is_first = false;
	}
	else
		NULL;
	ASSERT(NULL != pv_car_array);
	g_GetRadarDataList(&radar_data_list);
	if (car_array_index > radar_data_list.m_total_index) 
		return false;
	else {
		index_differ = radar_data_list.m_over_index - car_array_index;
		if (0 > index_differ)
			list_index = index_differ + radar_data_list.m_total_index;
		else
			list_index = index_differ;
		memcpy(p_car_array, &radar_data_list.m_car_array[list_index], sizeof(t_CarArray));
		return true;
	}
}

