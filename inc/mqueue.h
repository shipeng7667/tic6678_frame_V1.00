#ifndef MQUEUE_H
#define MQUEUE_H


#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


#define PTHREAD_WAITQ_FIFO			0	//�����ȴ���Դ�����񽫰����Ƚ��ȳ����ŶӲ��Խ������
#define PTHREAD_WAITQ_PRIO			1	//�����ȴ���Դ�����񽫰������ȼ���ռ���ŶӲ��Խ������

#define O_NONBLOCK					1


typedef int32_t mqd_t;


/***************************************************************************************
 * ��������
 * 			mq_create
 * �������ܣ�
 * 			mq_create()���ڴ���һ����ָ�����Ƶ���Ϣ���С�
 * 			����Ϣ���п�����max_msgs����Ϣ��ÿ����Ϣ����󳤶�Ϊmsg_size��
 * ����˵����
 *			mq_blockflag��
 *				��Ϣ����������־��ȡֵ������0��O_NONBLOCK��
 *			max_msgs��
 *				��Ϣ�����пɴ�ŵ���Ϣ����ȡֵΪ0��ʹ��Ĭ��ֵ10��
 *			msg_size��
 *				��Ϣ�����е���Ϣ���ȣ�ȡֵΪ0 ��ʹ��Ĭ��ֵ1024��
 *			waitq_type��
 *				��������Ϣ�����ϵ�����ĵȴ����ԣ�ȡֵ����PTHREAD_WAITQ_FIFO��PTHREAD_WAITQ_PRIO��
 * ���úˣ�
 * 			���к˾������е���
 * ����ֵ��
 *			return != (mqd_t)-1		���ɹ�
 *			return == (mqd_t)-1		��ʧ�ܣ��趨 errnoָ������
 * ��ע��
 * 			�����룺
 * 				EINVAL			max_msgs��msg_sizeС��0����waitq_typeȡֵ���Ϸ���
 * 				EMFILE			ϵͳ�д��˹������Ϣ������������
 * 				ENOMEM			�ڴ治�㣻
 * 				ECALLEDINISR	�ýӿڲ������ж��������е��ã�
 * 				EMNOTINITED		��Ϣ����ģ����δ��ʼ����
 ***************************************************************************************/
extern mqd_t mq_create(int32_t mq_blockflag, int32_t max_msgs, int32_t msg_size, int32_t waitq_type);

/***************************************************************************************
 * ��������
 * 			mq_delete
 * �������ܣ�
 * 			mq_delete()����ɾ��mq_create()��������δ������Ϣ���С�
 * 			���������mq_send()�ȴ�������Ϣ�͵���mq_receive()�ȴ�������Ϣ������ᱻ��������������ش���
 * 			�ú������ú�mqdes�Ͳ���ָ��һ����Ч����Ϣ���С�
 * ����˵����
 *			mqdes��
 *				��Ϣ������������
 * ���úˣ�
 * 			���к˾������е���
 * ����ֵ��
 *			return == 0		���ɹ�
 *			return == -1	��ʧ�ܣ�����ı���Ϣ���У��趨 errnoָ������
 * ��ע��
 * 			�����룺
 * 				ENOENT			ָ������Ϣ���в����ڣ�
 * 				EINVAL			��ɾ������Ϣ���в�����mq_create()������δ������Ϣ���У�
 * 				ECALLEDINISR	�ýӿڲ������ж��������е��ã�
 * 				EMNOTINITED		��Ϣ����ģ����δ��ʼ����
 ***************************************************************************************/
extern int32_t mq_delete(mqd_t mqdes);

/***************************************************************************************
 * ��������
 * 			mq_send
 * �������ܣ�
 * 			mq_send()������msg_ptrָ�����Ϣ���ӵ�mqdesָ������Ϣ�����С�
 * 			���ָ����Ϣ���в������ģ�mq_send()�����Ϣ���뵽msg_prioָ����λ�á�
 * 			msg_prioֵ�ϴ����Ϣ����뵽msg_prioֵ��С��ǰ�档
 * 			���msg_prio��ȣ���Ϣ���뵽����msg_prio��ȵ���Ϣ�ĺ��档
 * 			�����Ϣ��������������Ϣ����������mqdes��û������O_NONBLOCK��־����ô���������������ֱ����Ϣ���пռ���û��ߵ��ñ��ź��жϣ�
 * 			���������O_NONBLOCK��־����������Ͳ���������ֱ�ӷ��ش���
 * 			����ж�������ڵȴ����ͣ���ô����Ϣ���пռ����ʱ���ȴ�������ᰴ����Ϣ���е�����mq_waitqtype����������������������
 * 			���mq_waitqtype����ΪPTHREAD_WAITQ_PRIO���������ȼ����Խ�������������
 * 			���mq_waitqtype����ΪPTHREAD_WAITQ_FIFO������FIFO�Ĳ��Խ�������������
 * ����˵����
 *			mqdes��
 *				��Ϣ������������
 *			msg_ptr��
 *				�����Ϣ�Ļ����ַ��
 *			msg_len��
 *				��Ϣ���ȣ���ȡֵӦС�ڵ�����Ϣ���е�msg_msgsize���ԣ�������û�ʧ�ܣ�
 *			msg_prio��
 *				��Ϣ�����ȼ�����ȡֵӦС��MQ_PRIO_MAX��
 * ���úˣ�
 * 			���к˾������е���
 * ����ֵ��
 *			return == 0		���ɹ�
 *			return == -1	��ʧ�ܣ���Ϣ������ӣ��趨 errnoָ������
 * ��ע��
 * 			�����룺
 * 				EINVAL		msg_ptr����Чָ�룬����msg_prio��ȡֵ���Ϸ���
 * 				EBADF		mqdesָ������Ϣ�������������Ϸ�������mqdesָ������Ϣ������ֻ���ģ�
 * 				EMSGSIZE	msg_len ָ������Ϣ���ȳ�������Ϣ���е������Ϣ�������ԣ�
 * 				EAGAIN		��Ϣ����������O_NONBLOCK ��־�����ҵ��øýӿ�ʱ��Ϣ����������
 * 				EINTR		���������ź��жϡ�
 ***************************************************************************************/
extern int32_t mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, uint32_t msg_prio);

/***************************************************************************************
 * ��������
 * 			mq_receive
 * �������ܣ�
 * 			mq_receive()�������mqdesָ������Ϣ�����н���������ȼ���Ϣ���������Ϣ��
 * 			����ɲ���msg_lenָ���Ļ�������СС����Ϣ�������� mq_msgsize�����û�ʧ�ܲ����ش���;
 * 			����ѡ������Ϣ��Ӷ�����ɾ���������Ƶ� msg_ptrָ��Ļ������С�
 * 			���ָ������Ϣ����Ϊ�գ�����Ϣ����������mqdesû������O_NONBLOCK��־λ��
 * 			mq_receive()��������ֱ��һ����Ϣ���뵽�����л�ӿڵ��ñ��ź��жϡ�
 * 			����ж�������ڵȴ�������Ϣ������Ϣ����ʱ���ȴ�������ᰴ����Ϣ���е�����mq_waitqtype����������������������
 * 			mq_waitqtype��˵���μ�mq_send()�ӿڵ�˵����
 * 			���ָ������Ϣ����Ϊ�գ�����Ϣ����������mqdes ������O_NONBLOCK��־λ���򷵻ش���
 * ����˵����
 *			mqdes��
 *				��Ϣ������������
 *			msg_ptr��
 *				�����Ϣ�Ļ����ַ��
 *			msg_len��
 *				�����Ϣ�Ļ������ĳ��ȣ�
 *			msg_prio��
 *				�����Ϣ���ȼ���ָ�룬���msg_prio��Ϊ�գ�ѡ����Ϣ�����ȼ��ᱣ����msg_prio���õ�λ�á�
 * ���úˣ�
 * 			���к˾������е���
 * ����ֵ��
 *			return > 0		���ɹ����Ӷ�����ɾ��ѡ������Ϣ������ѡ����Ϣ���ֽڳ��ȣ�
 *			return == -1	��ʧ�ܣ���ɾ����Ϣ���趨errnoָ������
 * ��ע��
 * 			�����룺
 * 				EINVAL		����msg_ptr����Чָ�룻
 * 				EBADF		mqdesָ������Ϣ�������������Ϸ�������mqdesָ������Ϣ������ֻд�ģ�
 * 				EMSGSIZE	msg_len��ֵС����Ϣ���е���Ϣ��С���ԣ�
 * 				EAGAIN		��Ϣ����������mqdes��������O_NONBLOCK��־λ���ҽӿڵ���ʱ��Ϣ�����ǿյģ�
 * 				EINTR		���������ź��жϡ�
 ***************************************************************************************/
extern int32_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint32_t *msg_prio);


#ifdef __cplusplus
}
#endif


#endif /* MQUEUE_H */