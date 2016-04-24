#ifndef RADAR_PARSE_DATA_H
#define RADAR_PARSE_DATA_H


#ifndef __cplusplus
#define bool  int
#define false 0
#define true  1
#endif // __cplusplus

typedef struct {
	double m_x;    // x ����
	double m_y;    // y ����
} t_Direction;

typedef struct {
	t_Direction  m_position; // λ��
	t_Direction  m_speed;    // �ٶ�
	double       m_length;   // ����
	int          m_id;       // ID
} t_Car;

typedef struct {
	t_Car  m_car[64];   // һ���״���Ϣѭ����󱣴泵����
	int    m_car_count; // һ���״���Ϣѭ��ʵ�ʱ��泵����
} t_CarArray;


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	bool  ParseCycleData(const unsigned char *cycle_data, const int cycle_size, t_CarArray *car_array);

#ifdef __cplusplus
};
#endif // __cplusplus


#endif // RADAR_PARSE_DATA_H