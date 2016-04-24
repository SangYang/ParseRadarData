#include "debug_memory.h"
#include <stdlib.h>
#include <memory.h>
#include "debug_assert.h"


#if defined(_DEBUG) || defined(NDEBUG) || defined(__linux)
#define MEMINFO_CREATE            CreateBlockInfo
#define MEMINFO_FREE              FreeBlockInfo
#define MEMINFO_UPDATE            UpdateBlockInfo
#define MEMINFO_GETSIZE           GetSizeOfBlock
#define MEMINFO_ISIN              IsInBlockInfo
#define MEMINFO_REFCLEAR          ClearMemoryRefs
#define MEMINFO_REFNOTE           NoteMemoryRefs
#define MEMINFO_REFCHECK          CheckMemoryRefs
#define MEMSET                    memset
#else
#define MEMINFO_CREATE            NULL
#define MEMINFO_FREE              NULL
#define MEMINFO_UPDATE            NULL 
#define MEMINFO_GETSIZE           NULL
#define MEMINFO_ISIN              NULL
#define MEMINFO_REFCLEAR          NULL
#define MEMINFO_REFNOTE           NULL
#define MEMINFO_REFCHECK          NULL
#define MEMSET                    NULL
#endif // _DEBUG || NDEBUG || __linux

#define BLOCK_BEGIN(begin)        (begin)
#define BLOCK_END(begin, size)    (begin + size - 1)

#define GARBAGE_FLAG         0xA3   // 垃圾字符

typedef struct _blockinfo {
	struct _blockinfo *m_next;
	byte              *m_begin;	    // 存储块的开始位置
	size_t            m_size;	    // 存储块的长度
	bool              m_refered;	// 曾经引用过吗?
} t_BlockInfo;		


static t_BlockInfo* g_head = NULL;  // 单向链表表头


static t_BlockInfo* GetBlockInfo(byte *block) {
	t_BlockInfo *p_blockinfo;
	byte *p_begin;
	byte *p_end;

	ASSERT(NULL != block);
	for (p_blockinfo = g_head; NULL != p_blockinfo; p_blockinfo = p_blockinfo->m_next) {
		p_begin = BLOCK_BEGIN(p_blockinfo->m_begin);
		p_end = BLOCK_END(p_blockinfo->m_begin, p_blockinfo->m_size);
		if (p_begin > block || p_end < block)
			continue;
		else
			break;
	}

	ASSERT(NULL != p_blockinfo);
	return p_blockinfo;
}

static bool CreateBlockInfo(byte *block, size_t size) {
	t_BlockInfo* p_blockinfo;

	ASSERT(NULL != block && 0 != size);
	p_blockinfo = (t_BlockInfo *)malloc(sizeof(t_BlockInfo));
	if (NULL == p_blockinfo) 
		return false;
	else {
		p_blockinfo->m_begin = block;
		p_blockinfo->m_size = size;
		p_blockinfo->m_next = g_head;
		g_head = p_blockinfo ;
		return true;
	}
}

static void FreeBlockInfo(byte *block) {
	t_BlockInfo *p_blockinfo;
	t_BlockInfo *p_prevblockinfo;

	ASSERT(NULL != block);
	p_prevblockinfo = NULL;
	for (p_blockinfo = g_head; NULL != p_blockinfo; p_blockinfo = p_blockinfo->m_next) {
		if (p_blockinfo->m_begin != block) 
			p_prevblockinfo = p_blockinfo;
		else {
			if (NULL == p_prevblockinfo)
				g_head = p_blockinfo->m_next;
			else
				p_prevblockinfo->m_next = p_blockinfo->m_next;
			break;
		}
	}
	ASSERT(NULL != p_blockinfo);
	memset(p_blockinfo, GARBAGE_FLAG, sizeof(t_BlockInfo));
	free(p_blockinfo);
}


static void UpdateBlockInfo(byte *old_block, byte *new_block, size_t new_size) {
	t_BlockInfo *p_blockinfo;

	ASSERT(NULL != old_block && NULL != new_block && 0 != new_size);
	p_blockinfo = GetBlockInfo(old_block);
	ASSERT(p_blockinfo->m_begin == old_block);
	p_blockinfo->m_begin = new_block;
	p_blockinfo->m_size = new_size;
}

static size_t GetSizeOfBlock(byte *block) {
	t_BlockInfo *p_blockinfo;

	ASSERT(NULL != block);
	p_blockinfo = GetBlockInfo(block);
	ASSERT(p_blockinfo->m_begin == block);	
	return p_blockinfo->m_size;
}

static bool IsInBlockInfo(void *block, size_t size) {
	t_BlockInfo *p_blockinfo;
	byte *p_block = (byte *)block;
	byte *p_virtual_end;
	byte *p_actual_end;

	ASSERT(NULL != p_block && 0 != size);
	p_blockinfo = GetBlockInfo(p_block);
	p_virtual_end = BLOCK_END(p_block, size);
	p_actual_end = BLOCK_END(p_blockinfo->m_begin, p_blockinfo->m_size);
	if (p_virtual_end > p_actual_end)
		return false;
	else
		return true;
}

static void ClearMemoryRefs(void) {
	t_BlockInfo *p_blockinfo;
	for (p_blockinfo = g_head; p_blockinfo != NULL; p_blockinfo = p_blockinfo->m_next)
		p_blockinfo->m_refered = false;
}

static void NoteMemoryRefs(void *block) {
	t_BlockInfo *p_blockinfo;
	byte *p_block = (byte *)block;

	ASSERT(NULL != p_block);
	p_blockinfo = GetBlockInfo(p_block);
	p_blockinfo->m_refered = true;
}

static void CheckMemoryRefs(void) {
	t_BlockInfo *p_blockinfo ;
	for (p_blockinfo = g_head; NULL != p_blockinfo; p_blockinfo = p_blockinfo->m_next) {
		ASSERT(NULL != p_blockinfo->m_begin && 0 != p_blockinfo->m_size);
		ASSERT(p_blockinfo->m_refered);
	}
}


bool NewMemory(void *block, size_t size) {
	byte **pp_block = (byte **)block;
	byte *p_block;

	ASSERT(NULL != pp_block && 0 != size);
	p_block = (byte *)malloc(size);
	if (NULL == p_block)
		return false;
	else {
		MEMSET(p_block, GARBAGE_FLAG, size);
		*pp_block = p_block;
		MEMINFO_CREATE(p_block, size);
		return true;
	}
}


void FreeMemory(void *block) {
	byte *p_block = (byte *)block;

	ASSERT(NULL != p_block);
	MEMSET(p_block, GARBAGE_FLAG, MEMINFO_GETSIZE(p_block));
	MEMINFO_FREE(p_block);
	free(p_block);
}


bool ResizeMemory(void *block, size_t new_size) {
	byte **pp_block = (byte **)block;
	byte *p_block;
	size_t old_size;
	byte *p_newblock;
	bool ok_new;

	ASSERT(NULL != pp_block && 0 != new_size);
	p_block = *pp_block;
	old_size = MEMINFO_GETSIZE(p_block);

	if (old_size > new_size)
		MEMSET(p_block + new_size, GARBAGE_FLAG, old_size - new_size);
	else if (old_size < new_size) {
		ok_new = NewMemory(&p_newblock, new_size);
		if (!ok_new)
			return false;
		else {
			memcpy(p_newblock, p_block, old_size);
			FreeMemory(p_block);
			*pp_block = p_newblock;
			//MEMINFO_UPDATE(p_block, p_newblock, new_size);
			return true;
		}
	}
	else
		return true;
}


bool FillUpMemory(void *block, byte uch, size_t size) {
	byte *p_block = (byte *)block;
	if (!MEMINFO_ISIN(p_block, size))
		return false;
	else {
		memset(p_block, uch, size);
		return true;
	}
}

