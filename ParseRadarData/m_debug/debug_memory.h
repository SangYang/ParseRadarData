#ifndef DEBUG_BLOCK_H
#define DEBUG_BLOCK_H


#ifndef __cplusplus
#define bool   int
#define false  0
#define true   1
#endif

typedef unsigned char byte;

#ifdef __linux
typedef unsigned int  size_t;
#endif // __linux

#ifdef __cplusplus
extern "C" {
#endif

	bool NewMemory(void *block, size_t size);
	void FreeMemory(void *block);
	bool ResizeMemory(void *block, size_t new_size);
	bool FillUpMemory(void *block, byte uch, size_t size);

#ifdef __cplusplus
};
#endif


#endif // DEBUG_BLOCK_H

