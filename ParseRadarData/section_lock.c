#include "section_lock.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif // WIN32

void InitLock(void *pv_lock) {
#ifdef _WIN32
	CRITICAL_SECTION *p_lock = (CRITICAL_SECTION *)pv_lock;
	InitializeCriticalSection(p_lock);
#else
	pthread_mutex_t *p_lock = (pthread_mutex_t *)pv_lock;
	pthread_mutex_init(p_lock, NULL);
#endif // _WIN32
}

void EnterLock(void *pv_lock) {
#ifdef _WIN32
	CRITICAL_SECTION *p_lock = (CRITICAL_SECTION *)pv_lock;
	EnterCriticalSection(p_lock);
#else
	pthread_mutex_t *p_lock = (pthread_mutex_t *)pv_lock;
	pthread_mutex_lock(p_lock);
#endif // _WIN32
}

void LeaveLock(void *pv_lock) {
#ifdef _WIN32
	CRITICAL_SECTION *p_lock = (CRITICAL_SECTION *)pv_lock;
	LeaveCriticalSection(p_lock);
#else
	pthread_mutex_t *p_lock = (pthread_mutex_t *)pv_lock;
	pthread_mutex_unlock(p_lock);
#endif // _WIN32
}

void UninitLock(void *pv_lock) {
#ifdef _WIN32
	CRITICAL_SECTION *p_lock = (CRITICAL_SECTION *)pv_lock;
	DeleteCriticalSection(p_lock);
#else
	pthread_mutex_t *p_lock = (pthread_mutex_t *)pv_lock;
	pthread_mutex_destroy(p_lock);
#endif // _WIN32
}
