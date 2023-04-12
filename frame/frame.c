#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mpart.h"

#include "mqueue_aux.h"
#include "mpart_aux.h"


#pragma DATA_SECTION(mid_frameHeap, ".frame:private")
far mpart_id_ mid_frameHeap;


int32_t frame_init(void *addr, size_t size)
{
	int32_t result;

	result = mpart_create_kernel(addr, size, (mpart_id)&mid_frameHeap);
	if(result != 0)
	{
		return -1;
	}

	result = mq_module_init(64);
	if(result != 0)
	{
		return -2;
	}

	return 0;
}

int32_t frame_uninit(void)
{
	int32_t result;

	result = mq_module_uninit();
	if(result != 0)
	{
		return -1;
	}

	return 0;
}

size_t frame_heap_useMax_get(void)
{
	int32_t result;

	if(!mid_frameHeap.is_initialized)
	{
		return (size_t)-1;
	}

	return (mid_frameHeap.size - mid_frameHeap.usable_min);
}

void *frameHeap_alloc(size_t size)
{
	return mpart_alloc(&mid_frameHeap, size);
}

void *frameHeap_calloc(size_t size)
{
	return mpart_calloc(&mid_frameHeap, size);
}

int32_t frameHeap_free(char *addr)
{
	return mpart_free(&mid_frameHeap, addr);
}
