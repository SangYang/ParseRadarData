#include "debug_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#pragma warning(disable : 4996)

#define INI_FILE_PATH    "test.ini"
#define LOG_FILE_PATH    "test.log"

bool HaveIni() {
	FILE *pf_ini;

	pf_ini = fopen(INI_FILE_PATH, "rb");
	if (NULL == pf_ini)
		return false;
	else {
		fclose(pf_ini);
		return true;
	}
}

static bool GetCurrentTimes(char *p_current_time, const int time_size) {
	const int c_TimeSizeMin = sizeof("20151122101033");
	struct tm *p_tm_time;
	time_t i_time;

	if (NULL == p_current_time || c_TimeSizeMin > time_size)
		return false;
	else {
		time(&i_time);
		p_tm_time = localtime(&i_time);
		if (NULL == p_tm_time)
			return false;
		else {
			sprintf(p_current_time, "%04d%02d%02d%02d%02d%02d", 
				p_tm_time->tm_year + 1900, p_tm_time->tm_mon + 1, p_tm_time->tm_mday,
				p_tm_time->tm_hour, p_tm_time->tm_min, p_tm_time->tm_sec);
			return true;
		}
	}
}

int WriteLog(const char *p_format, ...) {
	FILE *pf_log;
	bool have_ini;
	char current_time[32];
	va_list arg_ptr;
	int writed_byte;
	int error_code;

	have_ini = HaveIni();
	if (false == have_ini)
		writed_byte = 0;
	else {
		pf_log = fopen(LOG_FILE_PATH, "ab");
		if (NULL == pf_log)
			writed_byte = 0;
		else {
			va_start(arg_ptr, p_format);
			GetCurrentTimes(current_time, sizeof(current_time));
			fprintf(pf_log, "\r\n%s  ", current_time);
			writed_byte = vfprintf(pf_log, p_format, arg_ptr);
			va_end(arg_ptr);
			fclose(pf_log);	
		}	
	}
	return writed_byte;
}


