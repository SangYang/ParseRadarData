#ifndef SSY_SECTION_LOCK_H
#define SSY_SECTION_LOCK_H


void InitLock(void *pv_lock);
void EnterLock(void *pv_lock); 
void LeaveLock(void *pv_lock);
void UninitLock(void *pv_lock);


#endif // SSY_SECTION_LOCK_H

