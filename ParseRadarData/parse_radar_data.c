#include "parse_radar_data.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "debug_assert.h"
#include "debug_log.h"
#include "unsign_string.h"

/*
t_CarArray g_car_array;

static void g_SetCarArray(const t_CarArray *car_array) {
	ASSERT(NULL != car_array);
	strcpy_s(&g_car_array, car_array, sizeof(*car_array));
}

static void g_GetCarArray(t_CarArray *car_array) {
	ASSERT(NULL != car_array);
	strcpy_s(car_array, &g_car_array, sizeof(g_car_array));
}
*/

static double GetPosX(const void *msg, const int msg_size) {
	const int c_Offset = 8192;
	const int c_Resolution = 64;  // 解析度 64mm
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	double result;
	
	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = p_msg[7];
	valid_msg[1] = (p_msg[6] & 0x3F);
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static double GetPosY(const void *msg, const int msg_size) {
	const int c_Offset = 8192;
	const int c_Resolution = 64;  // 解析度 64mm
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	double result;

	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = (p_msg[6] & 0xC0) >> 6;
	valid_msg[0] = (valid_msg[0] | (p_msg[5] << 2));
	valid_msg[1] = ((p_msg[5] >> 6) | ((p_msg[4] & 0x0F) << 2));
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static double GetSpeedX(const void *msg, const int msg_size) {
	const int c_Offset = 1024;
	const int c_Resolution = 100;  // 解析度 100mm/s
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	double result;

	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = (p_msg[4] >> 4);
	valid_msg[0] = (valid_msg[0] | (p_msg[3] << 4));
	valid_msg[1] = ((p_msg[3] >> 4) & 0x07);
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static double GetSpeedY(const void *msg, const int msg_size) {
	const int c_Offset = 1024;
	const int c_Resolution = 100;  // 解析度 100mm/s
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	double result;

	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = (p_msg[3] >> 7);
	valid_msg[0] = (valid_msg[0] | (p_msg[2] << 1));
	valid_msg[1] = (p_msg[2] >> 7);
	valid_msg[1] = (valid_msg[1] | ((p_msg[1] & 0x03) << 1));
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static double GetLength(const void *msg, const int msg_size) {
	const int c_Offset = 0;
	const int c_Resolution = 200;  // 解析度 200mm
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	double result;

	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = (p_msg[1] >> 2);
	valid_msg[0] = (valid_msg[0] | (p_msg[0] << 6));
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static int GetId(const void *msg, const int msg_size) {
	const int c_Offset = 0;
	const int c_Resolution = 1000;  // 解析度 1000
	unsigned char *p_msg;
	unsigned char valid_msg[8];
	int valid_pos;
	int result;

	ASSERT(NULL != msg && 8 <= msg_size);
	p_msg = (unsigned char *)msg;
	memset(valid_msg, 0, 8);
	valid_msg[0] = (p_msg[0] >> 2);
	valid_pos = *(int *)valid_msg;
	result = (double)(valid_pos - c_Offset) * c_Resolution / 1000;
	return result;
}

static void GetFirstLineFromCycle(const unsigned char *p_cycle_data, unsigned char *p_line, int *p_line_size) {
	const int c_LineHead = 2; // 02 FF 
	int size;
	int index;
	ASSERT(NULL != p_cycle_data && NULL != p_line && NULL != p_line_size);
	p_cycle_data += c_LineHead;
	size = (int)*p_cycle_data;
	p_cycle_data++;
	for (index = 0; index < size; index ++) {
		p_line[index] = p_cycle_data[index];
	}
	*p_line_size = size;
}

static bool SaveCarArray(const char *path, const t_CarArray *p_car_array) {
	FILE *pf;
	t_Car *p_car;
	int car_count;
	int car_index;
	time_t i_time;

	ASSERT(NULL != path && NULL != p_car_array);
	pf = fopen(path, "ab");
	if (NULL == pf)
		return false;
	else {
		car_count = p_car_array->m_car_count;
		fprintf(pf, "car_count:%d\n", car_count);
		for (car_index = 0; car_index < car_count; car_index++) {
			i_time = time(NULL);
			fprintf(pf, "time:%d ", (int)i_time);
			p_car = (t_Car *)&(p_car_array->m_car[car_index]);
			fprintf(pf, "pos_x:%0.4f pos_y:%0.4f speed_x:%0.4f speed_y:%0.4f length:%0.4f id:%d\n",
				p_car->m_position.m_x, p_car->m_position.m_y, 
				p_car->m_speed.m_x, p_car->m_speed.m_y, 
				p_car->m_length, p_car->m_id);
		}
		fprintf(pf, "\n");
		fclose(pf);
		return true;
	}
}

bool ParseCycleData(const unsigned char *p_cycle_data, const int cycle_size, t_CarArray *p_car_array) {
	const unsigned char c_CycleHead[] = {0xCA, 0xCB, 0xCC, 0xCD};
	const unsigned char c_CycleTail[] = {0xEA, 0xEB, 0xEC, 0xED};
	const int c_LineHeadSize = 3;      // 02 FF 08 类似标记所占字节
	unsigned char *p_cycle_head;
	unsigned char *p_cycle_tail;
	unsigned char line[32];
	int line_size;
	t_Car *p_car;
	int car_count;
	int car_index;
	double pos_x;
	double pos_y;
	double speed_x;
	double speed_y;
	double length;
	int id;

	ASSERT(NULL != p_cycle_data && 0 != cycle_size && NULL != p_car_array);
	p_cycle_head = (unsigned char *)UstrUstr(p_cycle_data, cycle_size, c_CycleHead, sizeof(c_CycleHead));
	p_cycle_tail = (unsigned char *)UstrUstr(p_cycle_data, cycle_size, c_CycleTail, sizeof(c_CycleTail));
	if (NULL == p_cycle_head || NULL == p_cycle_tail || p_cycle_tail < p_cycle_head + sizeof(c_CycleHead))
		return false;
	else {
		p_cycle_head += sizeof(c_CycleHead);
		if (0x02 == p_cycle_head[0] && 0xFF == p_cycle_head[1]) {
			GetFirstLineFromCycle(p_cycle_head, line, &line_size); // 02 FF
			p_cycle_head += c_LineHeadSize + line_size;	
			if (0x05 == p_cycle_head[0] && 0x00 == p_cycle_head[1]) {
				GetFirstLineFromCycle(p_cycle_head, line, &line_size); // 05 00
				p_cycle_head += c_LineHeadSize + line_size;
				if (0x05 == p_cycle_head[0] && 0x01 == p_cycle_head[1]) {
					GetFirstLineFromCycle(p_cycle_head, line, &line_size); // 05 01
					car_count = (int)(line[7]);
				}
				else
					return false;
			}
			else 
				return false;
		}
		else
			return false;

		p_car_array->m_car_count = car_count;
		for (car_index = 0; car_index < car_count; car_index++) {
			p_cycle_head += c_LineHeadSize + line_size;			
			GetFirstLineFromCycle(p_cycle_head, line, &line_size);

			pos_x = GetPosX(line, line_size);
			pos_y = GetPosY(line, line_size);
			speed_x = GetSpeedX(line, line_size);
			speed_y = GetSpeedY(line, line_size);
			length = GetLength(line, line_size);
			id = GetId(line, line_size);

			p_car = &(p_car_array->m_car[car_index]);
			p_car->m_position.m_x = pos_x;
			p_car->m_position.m_y = pos_y;
			p_car->m_speed.m_x = speed_x;
			p_car->m_speed.m_y = speed_y;
			p_car->m_length = length;
			p_car->m_id = id;
		}

 		if (p_car->m_position.m_x > 50 && p_car->m_position.m_x < 70 
 			&& p_car->m_position.m_y > -5 && p_car->m_position.m_y < 15)
			SaveCarArray("123.txt", p_car_array);
		return true;
	}
}


#if 0
int main() {
	double pos_x;
	double pos_y;
	double speed_x;
	double speed_y;
	double length;
	int id;
	//unsigned char msg[8] = {0x70, 0x46, 0x02, 0xB7, 0xF7, 0xDB, 0xE2, 0x95};
	//unsigned char msg[8] = {0x6C, 0x46, 0x02, 0x37, 0xE7, 0xA2, 0xA3, 0x0D};
	//unsigned char msg[8] = {0x78, 0x46, 0x00, 0x37, 0x57, 0x9E, 0xA4, 0xC2};
	unsigned char msg[8] = {0x10, 0x59, 0xFA, 0xB6, 0x98, 0x23, 0x21, 0x0E};
	//unsigned char msg[8] = {0x1C, 0x5A, 0x00, 0x37, 0x48, 0x13, 0x62, 0x77};

	unsigned char data[] = { 
		0xFF, 0xFF, 0xFF, 0xFF, 0xCA, 0xCB, 0xCC, 0xCD,
		0x02, 0xFF, 0x08, 0x00, 0x00, 0x1B, 0xCD, 0xDA, 0xFC, 0x07, 0xA0,
		0x05, 0x00, 0x08, 0x00, 0x38, 0xF1, 0x8E, 0x00, 0x00, 0x00, 0x00,
		0x05, 0x01, 0x08, 0x00, 0x01, 0x26, 0x66, 0x00, 0x30, 0x01, 0x13,
		0x05, 0x10, 0x08, 0x50, 0x59, 0xFD, 0xB7, 0x08, 0x25, 0xA2, 0x19,
		0x05, 0x11, 0x08, 0x08, 0x16, 0x00, 0x40, 0x07, 0xB1, 0xE3, 0xA9,
		0x05, 0x12, 0x08, 0x54, 0x59, 0xFF, 0xB7, 0x38, 0x26, 0xA4, 0x95,
		0x05, 0x13, 0x08, 0x70, 0x2A, 0x00, 0x44, 0x17, 0xD6, 0x25, 0x01,
		0x05, 0x14, 0x08, 0x14, 0x16, 0x00, 0x40, 0x07, 0xB1, 0xE5, 0x7E,
		0x05, 0x15, 0x08, 0x18, 0x16, 0x00, 0x40, 0x07, 0xB1, 0xE7, 0x52,
		0x05, 0x16, 0x08, 0x5C, 0x59, 0xFF, 0xB7, 0x48, 0x26, 0xA8, 0x34,
		0x05, 0x17, 0x08, 0x64, 0x29, 0xFF, 0xC4, 0x17, 0xD7, 0xE9, 0x24,
		0x05, 0x18, 0x08, 0x60, 0x59, 0xFF, 0xB7, 0x48, 0x26, 0xAB, 0xA7,
		0x05, 0x19, 0x08, 0x58, 0x2A, 0x00, 0x44, 0x17, 0xD8, 0x2C, 0x41,
		0x05, 0x1A, 0x08, 0x4C, 0x2A, 0x00, 0x44, 0x17, 0xD8, 0x2F, 0x1D,
		0x05, 0x1B, 0x08, 0x68, 0x59, 0xFF, 0xB7, 0x58, 0x26, 0xAF, 0xAE,
		0x05, 0x1C, 0x08, 0x40, 0x29, 0xFF, 0xC4, 0x07, 0xD8, 0x32, 0x21,
		0x05, 0x1D, 0x08, 0x6C, 0x59, 0xFF, 0xB7, 0x58, 0x26, 0xB2, 0xB8,
		0x05, 0x1E, 0x08, 0x30, 0x2A, 0x00, 0x44, 0x07, 0xD8, 0x75, 0x92,
		0x05, 0x1F, 0x08, 0x74, 0x41, 0xFF, 0xB7, 0x58, 0x26, 0xB6, 0x02,
		0x05, 0x20, 0x08, 0x78, 0x41, 0xFF, 0xB7, 0x58, 0x27, 0x38, 0x7B,
		0x05, 0x21, 0x08, 0x28, 0x29, 0xFF, 0xC4, 0x07, 0xD8, 0x38, 0xBD,
		0x05, 0x22, 0x08, 0x1C, 0x29, 0xFF, 0xC4, 0x07, 0xD8, 0x7B, 0x87,
		0x07, 0x82, 0x03, 0x04, 0x00, 0xD0,
		0x07, 0x83, 0x05, 0x00, 0x03, 0x00, 0x01, 0x0E,
		0x07, 0x83, 0x06, 0x00, 0x01, 0x08, 0x00, 0x00, 0x00,
		0x07, 0x83, 0x04, 0x00, 0x00, 0x00, 0x01,
		0x64,
		0xEA, 0xEB, 0xEC, 0xED
	};
	t_CarArray car_array;


	pos_x = GetPosX(msg, sizeof(msg));
	pos_y = GetPosY(msg, sizeof(msg));
	speed_x = GetSpeedX(msg, sizeof(msg));
	speed_y = GetSpeedY(msg, sizeof(msg));
	length = GetLength(msg, sizeof(msg));
	id = GetId(msg, sizeof(msg));
	ParseCycleData(data, sizeof(data), &car_array);
	return 0;
}
#endif


/*
05 10 08 10 59 FA B6 98 23 21 0E(目标信息)
转换为二进制后截取相应的位数如下:
000100(6位) 00010110(8位) 01111110101(11位) 01101101001(11位) 10000010001100(14位) 10000100001110(14位)

下面的括号内为二进制 转换为 十进制值 
000100(4) 00010110(22) 
01111110101(1013) 计算(Y方向的车速不用管)Y_Speed:-1.1=(1013-1024)*0.1 
01101101001(873)  计算车速X_Speed:-15.1=(873-1024)*0.1  
10000010001100(8332)  计算车道位置Y_Pos:4.48=(8332-8192)*0.064  
10000100001110(8462)  计算距离位置X_Pos:8.64=(8462-8192)*0.064
*/
