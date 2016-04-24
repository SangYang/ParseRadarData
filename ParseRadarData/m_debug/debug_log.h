#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H


#ifndef __cplusplus
#define bool    int
#define false   0
#define true    1
#endif //__cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	bool HaveIni(void);
	int  WriteLog(const char *p_format, ...);

#ifdef __cplusplus
};
#endif // __cplusplus


#if defined(_DEBUG) || defined(NDEBUG) || defined(__linux)
#define USABLE_LOG
#define DEFAULT_LOG 
#endif

#ifdef USABLE_LOG
	#define LOG_HAVE(b)          do { b = HaveIni(); } while(0)    
	#define LOG_VALUE(e)         do { e } while(0)

#if defined(DEFAULT_LOG) && defined(_WIN32) 
	#define LOG(fmt, ...)        WriteLog("%s(%d)-<%s>\r\n\t\t"##fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#elif defined(DEFAULT_LOG) && defined(__linux)
	#define LOG(fmt, ...)        WriteLog("%s(%d)-<%s>\r\n\t\t"fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
	#define LOG                  WriteLog
#endif // DEFAULT_LOG

#else
	#define LOG_HAVE             NULL
	#define LOG_VALUE(e)         NULL
	#define LOG(fmt,...)         NULL
#endif // USABLE_LOG


#endif // DEBUG_LOG_H