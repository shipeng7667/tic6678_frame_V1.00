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
#include "mpart_aux.h"
#include "frame_aux.h"


mqd_t mq_create(int32_t mq_blockflag, int32_t max_msgs, int32_t msg_size, int32_t waitq_type)
{
	int32_t status = 0;
	mqd_t mqd = (mqd_t)0;
	uint32_t msg_size_real;
	uint32_t i;
	msg_list_t *pMsg_list;
	mq_des_t *pMq_des;

	if((max_msgs < 0) || (msg_size < 0) || ((waitq_type != PTHREAD_WAITQ_FIFO) && (waitq_type != PTHREAD_WAITQ_PRIO)) || ((mq_blockflag != 0) && (mq_blockflag != O_NONBLOCK)))
	{
		status = -1;
		goto exit;
	}

	//申请消息队列描述符
	pMq_des = resPool_alloc(pResPool_mq, &mqd);
	if(pMq_des == 0)
	{
		status = -2;
		goto exit;
	}

	if(max_msgs == 0)
	{
		max_msgs = 10;
	}

	if(msg_size == 0)
	{
		msg_size = 1024;
	}

	msg_size_real = ((uint32_t)msg_size + mpart_block_size - 1) & (~(mpart_block_size - 1));

	pMq_des->pMsg = frameHeap_alloc(msg_size_real * max_msgs);
	if(pMq_des->pMsg == NULL)
	{
		status = -3;
		goto exit;
	}

	pMq_des->pResPool_msgs = resPool_create(sizeof(msg_list_t), max_msgs, &mid_frameHeap, mpart_block_size);
	if(pMq_des->pResPool_msgs == NULL)
	{
		status = -4;
		goto exit;
	}

	for(i = 0; i < max_msgs; i++)
	{
		pMsg_list = resPool_alloc(pMq_des->pResPool_msgs, NULL);
		pMsg_list->pMsg = &pMq_des->pMsg[msg_size_real * i];
	}

	for(i = 0; i < max_msgs; i++)
	{
		resPool_freeByHdl(pMq_des->pResPool_msgs, i);
	}

	pMq_des->mq_blockflag = mq_blockflag;
	pMq_des->mq_maxmsg = max_msgs;
	pMq_des->mq_msgsize = msg_size;
	pMq_des->mq_msgsize_real = msg_size_real;
	pMq_des->mq_waittype = waitq_type;
	pMq_des->mq_curmsgs = 0;
	pMq_des->pMsg_list_start = NULL;
	pMq_des->pMsg_list_end = NULL;

	pMq_des->is_initialized = 1;

exit:
	if(status < 0)
	{
		if(pMq_des->pMsg)
		{
			frameHeap_free((char *)pMq_des->pMsg);
		}

		if(pMq_des->pResPool_msgs)
		{
			resPool_delete(pMq_des->pResPool_msgs);
		}

		if(pMq_des)
		{
			resPool_freeByAddr(pResPool_mq, pMq_des);
		}

		return ((mqd_t)-1);
	}

	return mqd;
}
