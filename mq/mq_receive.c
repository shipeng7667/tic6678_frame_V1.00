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


int32_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint32_t *msg_prio)
{
	int32_t status = 0;

	mq_des_t *pMq_des = NULL;
	msg_list_t *pMsg_list = NULL;
	msg_list_t *pMsg_list_cur = NULL, *pMsg_list_pre = NULL;
	int32_t receive_len;
	pResPool_mq->lockCnt = 0;
	if((mqdes == (mqd_t)-1) || (msg_ptr == NULL) || (msg_len == 0))
	{
		return -1;
	}

	pMq_des = resPool_resAddrGet(pResPool_mq, mqdes);
	if(pMq_des == NULL)
	{
		return -1;
	}

reReceive:
	resPool_resLock(pResPool_mq, mqdes);

	if(!pMq_des->is_initialized || (msg_len > pMq_des->mq_msgsize_real))
	{
		status = -1;
		goto exit;
	}

	if(pMq_des->mq_curmsgs <= 0)
	{
		if(pMq_des->mq_blockflag == O_NONBLOCK)
		{
			status = -2;
			goto exit;
		}
		else
		{
			resPool_resUnlock(pResPool_mq, mqdes);
			goto reReceive;
		}
	}

	pMsg_list_cur = pMq_des->pMsg_list_start;
	pMsg_list_pre = NULL;

	while(1)
	{
		if(pMsg_list_cur->next == NULL)
		{
			pMsg_list = pMsg_list_cur;

			if(pMq_des->pMsg_list_start == pMq_des->pMsg_list_end)
			{
				pMq_des->pMsg_list_start = NULL;
			}
			else
			{
				pMsg_list_pre->next = NULL;
			}

			pMq_des->pMsg_list_end = pMsg_list_pre;

			break;
		}

		pMsg_list_pre = pMsg_list_cur;
		pMsg_list_cur = pMsg_list_cur->next;
	}

	if(pMsg_list->msg_size < msg_len)
	{
		receive_len = pMsg_list->msg_size;
	}
	else
	{
		receive_len = msg_len;
	}

	memcpy(msg_ptr, pMsg_list->pMsg, receive_len);

	if(msg_prio)
	{
		*msg_prio = pMsg_list->msg_prio;
	}

	resPool_freeByAddr(pMq_des->pResPool_msgs, pMsg_list);

	pMq_des->mq_curmsgs--;

exit:
	resPool_resUnlock(pResPool_mq, mqdes);

	if(status < 0)
	{
		return -1;
	}

	return receive_len;
}
