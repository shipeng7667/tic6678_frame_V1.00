#ifndef MQUEUE_AUX_H
#define MQUEUE_AUX_H


#include <stdint.h>
#include <stddef.h>

#include "resPool.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _msg_list_t
{
	void				*pMsg;
	uint32_t			msg_size;
	uint32_t			msg_prio;
	struct _msg_list_t	*next;
}msg_list_t;

typedef struct
{
	volatile int32_t mq_blockflag;					/* 消息队列阻塞标志 */
	int32_t mq_maxmsg; 								/* 消息队列中的最大消息数目 */
	int32_t mq_msgsize;								/* 消息队列中的消息长度 */
	int32_t mq_msgsize_real;						/* 消息队列中的消息长度 */
	int32_t mq_waittype;							/* 消息队列的等待策略 */
	volatile int32_t mq_curmsgs;					/* 队列中的当前消息数目 */
	void *pResPool_msgs;
	uint8_t *pMsg;
	msg_list_t *pMsg_list_start;
	msg_list_t *pMsg_list_end;
	int32_t is_initialized;							/* 是否已被初始化 */
}mq_des_t;


extern resPool_t *pResPool_mq;


extern int32_t mq_module_init(uint32_t max_mqueues);
extern int32_t mq_module_uninit(void);


#ifdef __cplusplus
}
#endif


#endif /* MQUEUE_AUX_H */
