#ifndef RADAR_DATA_INTERFACE_H
#define RADAR_DATA_INTERFACE_H


#ifndef __cplusplus
#define bool   int
#define false  0
#define true   1
#endif 

typedef struct {
	double m_x;
	double m_y;
} t_PixelsCoord;

typedef struct {
	t_PixelsCoord m_coord[4];
} t_PlateArea;

#ifdef __cplusplus
extern "C" {
#endif

	bool GetSpeed(const t_PlateArea plate_area, double *p_speed, int *p_speed_confidence);
	bool RunRadarDataList();

#ifdef __cplusplus
};
#endif


#endif // RADAR_DATA_INTERFACE_H