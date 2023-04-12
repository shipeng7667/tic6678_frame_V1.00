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


int32_t mq_delete(mqd_t mqdes)
{
	int32_t status = 0;

	mq_des_t *pMq_des = NULL;
	int32_t gie;

	gie = int_lock();

	if((mqdes == (mqd_t)-1))
	{
		status = -1;
		goto exit;
	}

	pMq_des = resPool_resAddrGet(pResPool_mq, mqdes);
	if(pMq_des == NULL)
	{
		status = -2;
		goto exit;
	}

	if(!pMq_des->is_initialized)
	{
		status = -3;
		goto exit;
	}

	if(pMq_des->pMsg)
	{
		frameHeap_free((char *)pMq_des->pMsg);
	}

	if(pMq_des->pResPool_msgs)
	{
		resPool_delete(pMq_des->pResPool_msgs);
	}

	pMq_des->is_initialized = 0;

	if(pMq_des)
	{
		resPool_freeByAddr(pResPool_mq, pMq_des);
	}

exit:
	int_unlock(gie);

	return status;
}
