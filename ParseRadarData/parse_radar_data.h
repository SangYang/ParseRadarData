#ifndef RADAR_PARSE_DATA_H
#define RADAR_PARSE_DATA_H


#ifndef __cplusplus
#define bool  int
#define false 0
#define true  1
#endif // __cplusplus

typedef struct {
	double m_x;    // x 方向
	double m_y;    // y 方向
} t_Direction;

typedef struct {
	t_Direction  m_position; // 位置
	t_Direction  m_speed;    // 速度
	double       m_length;   // 长度
	int          m_id;       // ID
} t_Car;

typedef struct {
	t_Car  m_car[64];   // 一次雷达消息循环最大保存车辆数
	int    m_car_count; // 一次雷达消息循环实际保存车辆数
} t_CarArray;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	bool  ParseCycleData(const unsigned char *cycle_data, const int cycle_size, t_CarArray *car_array);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif // RADAR_PARSE_DATA_H