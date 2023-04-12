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


int32_t mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, uint32_t msg_prio)
{
	int32_t status = 0;

	mq_des_t *pMq_des = NULL;
	msg_list_t *pMsg_list = NULL;
	msg_list_t *pMsg_list_cur = NULL, *pMsg_list_pre = NULL;

	if((mqdes == (mqd_t)-1) || (msg_ptr == NULL) || (msg_len == 0))
	{
		return -1;
	}

	pMq_des = resPool_resAddrGet(pResPool_mq, mqdes);
	if(pMq_des == NULL)
	{
		return -1;
	}

reSend:
	resPool_resLock(pResPool_mq, mqdes);

	if(!pMq_des->is_initialized || (msg_len > pMq_des->mq_msgsize_real))
	{
		status = -1;
		goto exit;
	}

	if(pMq_des->mq_curmsgs >= pMq_des->mq_maxmsg)
	{
		if(pMq_des->mq_blockflag == O_NONBLOCK)
		{
			status = -2;
			goto exit;
		}
		else
		{
			resPool_resUnlock(pResPool_mq, mqdes);
			goto reSend;
		}
	}

	pMsg_list = resPool_alloc(pMq_des->pResPool_msgs, NULL);
	if(pMsg_list == NULL)
	{
		status = -2;
		goto exit;
	}

	memcpy(pMsg_list->pMsg, msg_ptr, msg_len);
	pMsg_list->msg_size = msg_len;
	pMsg_list->msg_prio = msg_prio;

	if(pMq_des->pMsg_list_start == NULL)
	{
		pMq_des->pMsg_list_start = pMsg_list;
		pMq_des->pMsg_list_end = pMq_des->pMsg_list_start;
	}
	else
	{
		pMsg_list_cur = pMq_des->pMsg_list_start;
		pMsg_list_pre = NULL;

		while(1)
		{
			if(msg_prio >= pMsg_list_cur->msg_prio)
			{
				if(pMsg_list_cur == pMq_des->pMsg_list_start)
				{
					pMq_des->pMsg_list_start = pMsg_list;
				}
				else
				{
					pMsg_list_pre->next = pMsg_list;
				}

				pMsg_list->next = pMsg_list_cur;

				break;
			}

			if(pMsg_list_cur->next == NULL)
			{
				pMq_des->pMsg_list_end->next = pMsg_list;
				pMq_des->pMsg_list_end = pMsg_list;

				break;
			}

			pMsg_list_pre = pMsg_list_cur;
			pMsg_list_cur = pMsg_list_cur->next;
		}
	}

	pMq_des->mq_curmsgs++;

exit:
	resPool_resUnlock(pResPool_mq, mqdes);

	if(status < 0)
	{
		return -1;
	}

	return 0;
}
