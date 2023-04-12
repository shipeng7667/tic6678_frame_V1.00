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
	volatile int32_t mq_blockflag;					/* ��Ϣ����������־ */
	int32_t mq_maxmsg; 								/* ��Ϣ�����е������Ϣ��Ŀ */
	int32_t mq_msgsize;								/* ��Ϣ�����е���Ϣ���� */
	int32_t mq_msgsize_real;						/* ��Ϣ�����е���Ϣ���� */
	int32_t mq_waittype;							/* ��Ϣ���еĵȴ����� */
	volatile int32_t mq_curmsgs;					/* �����еĵ�ǰ��Ϣ��Ŀ */
	void *pResPool_msgs;
	uint8_t *pMsg;
	msg_list_t *pMsg_list_start;
	msg_list_t *pMsg_list_end;
	int32_t is_initialized;							/* �Ƿ��ѱ���ʼ�� */
}mq_des_t;


extern resPool_t *pResPool_mq;


extern int32_t mq_module_init(uint32_t max_mqueues);
extern int32_t mq_module_uninit(void);


#ifdef __cplusplus
}
#endif


#endif /* MQUEUE_AUX_H */
