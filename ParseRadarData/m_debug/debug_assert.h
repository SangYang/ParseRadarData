#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H


#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

	void AssertFunc(const char *file_path, unsigned int line, const char *func_name);

#ifdef _WIN32
	void BeginCrtCheck();
	void EndCrtCheck();
#endif // _WIN32

#ifdef __cplusplus
};
#endif // __cplusplus


#if defined(_DEBUG) || defined(NDEBUG) || defined(__linux)
#define ASSERT(f)  do { \
	if (f) \
		NULL; \
	else \
		AssertFunc(__FILE__, __LINE__, __FUNCTION__); \
} while(0)
#else
#define ASSERT(f)  NULL
#endif // _DEBUG || NDEBUG || __linux


#if defined(_DEBUG) && defined(_WIN32)
#define CRTMEM_BEGIN       BeginCrtCheck
#define CRTMEM_END         EndCrtCheck
#else
#define CRTMEM_BEGIN       NULL
#define CRTMEM_END         NULL
#endif // _DEBUG && _WIN32


#endif // DEBUG_ASSERT_H