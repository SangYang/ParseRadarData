#include "unsign_string.h"
#include <stdio.h>
#include "debug_assert.h"
#include "debug_log.h"

const void* UstrUstr(const unsigned char *data, const int data_size, const unsigned char *ustr, const int ustr_size) {
	int dtindex;
	int usindex;
	int tpindex;
	ASSERT(NULL != data && NULL != ustr && 0 != ustr_size);
	usindex = 0;
	for (dtindex = 0; dtindex + ustr_size <= data_size; dtindex++) {
		tpindex = dtindex;
		for (usindex = 0; usindex < ustr_size; usindex++, tpindex++) {
			if (ustr[usindex] != data[tpindex])
				break;
			else
				continue;
		}
		if (ustr_size == usindex)
			break;
		else
			continue;
	}
	if (ustr_size != usindex)
		return NULL;
	else 
		return data + dtindex;
}