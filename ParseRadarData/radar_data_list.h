#ifndef RADAR_DATA_LIST_H
#define RADAR_DATA_LIST_H


#ifndef __cplusplus
#define bool   int
#define false  0
#define true   1
#endif 


#ifdef __cplusplus
extern "C" {
#endif

	void InsertCarArrayToList(const void *pv_car_array);
	bool GetCarArrayFromList(const int car_array_index, void *pv_car_array);

#ifdef __cplusplus
};
#endif


#endif // RADAR_DATA_LIST_H