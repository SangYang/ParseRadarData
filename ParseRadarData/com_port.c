#include "com_port.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#endif // _WIN32

#include "debug_assert.h"
#include "debug_log.h"
#include "parse_radar_data.h"
#include "radar_data_list.h"
#include "unsign_string.h"

#define ARRAY_LENGTH(a)  (sizeof(a) / sizeof(a[0]))

#ifdef _WIN32
static HANDLE g_port_handle = NULL;
#else
static int    g_port_handle = 0;
#endif // _WIN32

static void g_SetHandle(const void *pv_handle) {
#ifdef _WIN32
	HANDLE *p_handle;

	ASSERT(NULL != pv_handle);
	p_handle = (HANDLE *)pv_handle;
	g_port_handle = *p_handle;
#else
	int *p_handle;

	ASSERT(NULL != pv_handle);
	p_handle = (int *)pv_handle;
	g_port_handle = *p_handle;
#endif // _WIN32
}

static void g_GetHandle(void *pv_handle) {
#ifdef _WIN32
	HANDLE *p_handle;

	ASSERT(NULL != g_port_handle && NULL != pv_handle);
	p_handle = (HANDLE *)pv_handle;
	*p_handle = g_port_handle;
#else
	int *p_handle;

	ASSERT(0 != g_port_handle && NULL != pv_handle);
	p_handle = (int *)pv_handle;
	*p_handle = g_port_handle;
#endif // _WIN32
}

#ifdef __linux
static bool SetBaudRate(const int handle, const int baud_rate) {
	int speed_array[] = {
		B0, B50, B75, B110, B134, B150, B200, B300, B600, B1200, 
		B1800, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400
	};
	int duplicate_array[] = {
		0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 
		1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400
	};
	int array_length;
	int array_index;
	struct termios termios_status;
	int res_getaddr;
	int res_setaddr; 

	res_getaddr = tcgetattr(handle, &termios_status);
	if (0 != res_getaddr) {
		LOG("tcgetattr() error=%d", res_getaddr);
		return false;
	}
	else {
		array_length = ARRAY_LENGTH(speed_array);
		for (array_index = 0; array_index < array_length; array_index++) {
			if (baud_rate == duplicate_array[array_index]) {
				tcflush(handle, TCIOFLUSH);     
				cfsetispeed(&termios_status, speed_array[array_index]);  
				cfsetospeed(&termios_status, speed_array[array_index]);   
				tcflush(handle, TCIOFLUSH);  
				res_setaddr = tcsetattr(handle, TCSANOW, &termios_status);  
				if  (0 != res_setaddr) {        
					LOG("tcsetattr() error!");
					return false;
				}  
				else 
					return true;
			}
			else
				continue;
		}
	}
}
#endif // __linux

bool OpenComport(const int port, const int baud_rate) {
#ifdef _WIN32
	const int c_QueueSize = 4096;
	char ach_name[32];
	HANDLE handle;
	DCB dcb;
	COMMTIMEOUTS timeouts;
	bool ok_comm;

	sprintf_s(ach_name, sizeof(ach_name), "\\\\.\\COM%d", port);
	handle = CreateFile(ach_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, 
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == handle) {
		LOG("CreateFile() error! handle = %d", handle);
		return false;
	}
	else {
		ok_comm = GetCommState(handle, &dcb);
		ASSERT(true == ok_comm);
		dcb.BaudRate = baud_rate; // 设置波特率
		ok_comm = SetCommState(handle, &dcb);
		ASSERT(true == ok_comm);
		ok_comm = SetupComm(handle, c_QueueSize, c_QueueSize);
		ASSERT(true == ok_comm); // 设置I/O缓冲区的大小
		ok_comm = GetCommTimeouts(handle, &timeouts);
		ASSERT(true == ok_comm);
		timeouts.ReadIntervalTimeout = 0xFFFFFFFF; // 读间隔超时
		timeouts.ReadTotalTimeoutMultiplier = 0; // 读时间系数 
		timeouts.ReadTotalTimeoutConstant = 1000; // 读时间常量 
		timeouts.WriteTotalTimeoutMultiplier = 0; // 写时间系数
		timeouts.WriteTotalTimeoutConstant = 1000; // 写时间常量
		ok_comm = SetCommTimeouts(handle, &timeouts);
		ASSERT(true == ok_comm);

		g_SetHandle(&handle);
		return true;
	}
#else
	char ach_name[32];
	int handle;
	bool ok_getbaudrate;

	sprintf(ach_name, "/dev/ttyS%d", port);
	handle = open(ach_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (-1 == handle) {
		LOG("open() error! handle = %d", handle);
		return false;
	}
	else {
		ok_getbaudrate = SetBaudRate(handle, baud_rate);
		if (false == ok_getbaudrate) {
			close(handle);
			LOG("SetBaudRate() error!");
			return false;
		}
		else {
			g_SetHandle(&handle);
			return true;
		}
	}
#endif // _WIN32
}

void CloseComport() {
#ifdef _WIN32
	HANDLE handle;

	g_GetHandle(&handle);
	CloseHandle(handle);
#else
	int handle;
	g_GetHandle(&handle);
	close(handle);
#endif // _WIN32
}

static int ReadData(const void *pv_handle, void *data, const int size) {
	const int c_PerSize = 256;
#ifdef _WIN32
	HANDLE *p_handle = (HANDLE *)pv_handle;
#else
	int *p_handle = (int *)pv_handle;
#endif // _WIN32
	unsigned char *p_data;
	int read_persize;
	int read_allsize;
	int remain_size;
	bool ok_read;

	ASSERT(NULL != pv_handle && NULL != data && 0 != size);
	p_data = (unsigned char *)data;
	read_allsize = 0;
	while (read_allsize < size) {
		remain_size = size - read_allsize;
#ifdef _WIN32
		if (c_PerSize > remain_size)
			ok_read = ReadFile(*p_handle, p_data + read_allsize, remain_size, &read_persize, NULL);	
		else 
			ok_read = ReadFile(*p_handle, p_data + read_allsize, c_PerSize, &read_persize, NULL);
#else
		if (c_PerSize > remain_size)
			read_persize = read(*p_handle, p_data + read_allsize, remain_size);	
		else 
			read_persize = read(*p_handle, p_data + read_allsize, c_PerSize);
		if (-1 == read_persize)
			ok_read = false;
		else
			ok_read = true;
#endif // _WIN32
		read_allsize += read_persize;
		if (!ok_read || (remain_size != read_persize && c_PerSize != read_persize))
			break;
		else
			continue;
	}
	return read_allsize;
}

static int WriteData(const void *pv_handle, const void *data, const int size) {
	const int c_PerSize = 256;
#ifdef _WIN32
	HANDLE *p_handle = (HANDLE *)pv_handle;
#else
	int *p_handle = (int *)pv_handle;
#endif // _WIN32
	unsigned char *p_data;
	int write_persize;
	int write_allsize;
	int remain_size;
	bool ok_write;

	ASSERT(NULL != pv_handle && NULL != data && 0 != size);
	p_data = (unsigned char *)data;
	write_allsize = 0;
	while (write_allsize < size) {
		remain_size = size - write_allsize;
#ifdef _WIN32
		if (c_PerSize > remain_size)
			ok_write = WriteFile(*p_handle, p_data + write_allsize, remain_size, &write_persize, NULL);	
		else 
			ok_write = WriteFile(*p_handle, p_data + write_allsize, c_PerSize, &write_persize, NULL);
#else
		if (c_PerSize > remain_size)
			write_persize = write(*p_handle, p_data + write_allsize, remain_size);	
		else 
			write_persize = write(*p_handle, p_data + write_allsize, c_PerSize);
		if (-1 == write_persize)
			ok_write = false;
		else
			ok_write = true;
#endif // _WIN32
		write_allsize += write_persize;
		if (!ok_write || (remain_size != write_persize && c_PerSize != write_persize))
			break;
		else
			continue;
	}
	return write_allsize;
}

int ReadComport(void *data, const int size) {
	unsigned char *p_data;
#ifdef _WIN32
	HANDLE handle;
#else
	int handle;
#endif // _WIN32
	int act_size;
	bool ok_comm;

	ASSERT(NULL != data && 0 != size);
	p_data = (unsigned char *)data;
	g_GetHandle(&handle);
#ifdef _WIN32
	ok_comm = ClearCommError(handle, NULL, NULL);
	ASSERT(true == ok_comm);
#endif // _WIN32
	act_size = ReadData(&handle, p_data, size);
	return act_size;
}

int WriteComport(const void *data, const int size) {
	unsigned char *p_data;
#ifdef _WIN32
	HANDLE handle;
#else
	int handle;
#endif // _WIN32
	int act_size;
	bool ok_comm;

	ASSERT(NULL != data && 0 != size);
	p_data = (unsigned char *)data;
	g_GetHandle(&handle);
#ifdef _WIN32
	ok_comm = ClearCommError(handle, NULL, NULL);
	ASSERT(true == ok_comm);
#endif // _WIN32
	act_size = WriteData(&handle, p_data, size);
	return act_size;
}

static bool GetFirstCycleData(const unsigned char *com_data, const int com_size, int *cycle_offset, unsigned char *cycle_data, int *cycle_size) {
	const unsigned char c_CycleHead[] = {0xCA, 0xCB, 0xCC, 0xCD};
	const unsigned char c_CycleTail[] = {0xEA, 0xEB, 0xEC, 0xED};
	unsigned char *cycle_head;
	unsigned char *cycle_tail;
	int i_cycle_offset;
	int remain_size;
	int i_cycle_size;
	int cycle_index;

	ASSERT(NULL != com_data && 0 != com_size && NULL != cycle_data && NULL != cycle_size);
	cycle_head = (unsigned char *)UstrUstr(com_data, com_size, c_CycleHead, sizeof(c_CycleHead));
	if (NULL == cycle_head) {
		LOG("UstrUstr() error! not found {0xCA, 0xCB, 0xCC, 0xCD}");
		return false;
	}
	else {
		i_cycle_offset = cycle_head - com_data;
		remain_size = com_size - i_cycle_offset;
		cycle_tail = (unsigned char *)UstrUstr(cycle_head, remain_size, c_CycleTail, sizeof(c_CycleTail));
		if (NULL == cycle_tail) {
			LOG("UstrUstr() error! not found {0xEA, 0xEB, 0xEC, 0xED}");
			return false;
		}
		else {
			i_cycle_size = cycle_tail - cycle_head + sizeof(c_CycleHead);
			for (cycle_index = 0; cycle_index < i_cycle_size; cycle_index++) {
				cycle_data[cycle_index] = cycle_head[cycle_index];
			}
			*cycle_size = i_cycle_size;
			*cycle_offset = i_cycle_offset;
			return true;
		}
	}
}


void AnalyseComData(const unsigned char *com_data, const int com_size) {
	unsigned char cycle_data[512];
	unsigned char *p_com_data;
	int com_offset;	
	int cycle_offset;
	int i_com_size;
	int i_cycle_size;
	bool ok_cycle;
	t_CarArray car_array;

	ASSERT(NULL != com_data && 0 != com_size);
	com_offset = 0;
	while (com_size > com_offset) {
		p_com_data = (unsigned char *)com_data + com_offset;
		i_com_size = com_size - com_offset;
		ok_cycle = GetFirstCycleData(p_com_data, i_com_size, &cycle_offset, cycle_data, &i_cycle_size);
		if (false == ok_cycle)
			break;
		else {
			com_offset += cycle_offset;
			com_offset += i_cycle_size;
			ParseCycleData(cycle_data, i_cycle_size, &car_array);
			InsertCarArrayToList(&car_array);
		}
	}
}


#if 0
static void SaveComData(const unsigned char *p_data, const int data_size) {
	FILE *f;

	f = fopen("123456.txt", "ab");
	fwrite(p_data, 1, data_size, f);
	fclose(f);
}

int main(void) {
	unsigned char buffer[1024];
	int read_res;
	bool ok_com;
	int count;

	ok_com = OpenComport(2, 115200);
	if (false == ok_com)
		printf("OpenComport() failure!, port=%d, saut=%d\n", 2, 115200);
	else {
		count = 0;
		while (true) {
			printf("count = %d\n", count);
			read_res = ReadComport(buffer, 512);
			SaveComData(buffer, read_res);
			count++;
		}
		CloseComport();
	}

	return 0;
}

#endif