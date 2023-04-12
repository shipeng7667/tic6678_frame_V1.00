#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mpart.h"
#include "intc.h"
#include "mqueue.h"
#include "frame.h"
#include "resPool.h"

#include "mqueue_aux.h"
#include "frame_aux.h"


#pragma DATA_SECTION(pResPool_mq, ".frame:private")
resPool_t *pResPool_mq;


int32_t mq_module_init(uint32_t max_mqueues)
{
	if(max_mqueues == 0)
	{
		return -1;
	}

	if(pResPool_mq)
	{
		return -2;
	}

	pResPool_mq = resPool_create(sizeof(mq_des_t), max_mqueues, &mid_frameHeap, mpart_block_size);
	if(pResPool_mq == NULL)
	{
		return -3;
	}

	return 0;
}
