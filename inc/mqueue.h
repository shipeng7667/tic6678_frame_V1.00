#ifndef MQUEUE_H
#define MQUEUE_H


#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


#define PTHREAD_WAITQ_FIFO			0	//阻塞等待资源的任务将按照先进先出的排队策略解除阻塞
#define PTHREAD_WAITQ_PRIO			1	//阻塞等待资源的任务将按照优先级抢占的排队策略解除阻塞

#define O_NONBLOCK					1


typedef int32_t mqd_t;


/***************************************************************************************
 * 函数名：
 * 			mq_create
 * 函数功能：
 * 			mq_create()用于创建一个不指定名称的消息队列。
 * 			该消息队列可容纳max_msgs条消息，每条消息的最大长度为msg_size。
 * 参数说明：
 *			mq_blockflag：
 *				消息队列阻塞标志，取值可以是0或O_NONBLOCK；
 *			max_msgs：
 *				消息队列中可存放的消息数，取值为0则使用默认值10；
 *			msg_size：
 *				消息队列中的消息长度，取值为0 则使用默认值1024；
 *			waitq_type：
 *				阻塞在消息队列上的任务的等待策略，取值包括PTHREAD_WAITQ_FIFO或PTHREAD_WAITQ_PRIO。
 * 调用核：
 * 			所有核均可自行调用
 * 返回值：
 *			return != (mqd_t)-1		：成功
 *			return == (mqd_t)-1		：失败，设定 errno指出错误。
 * 备注：
 * 			错误码：
 * 				EINVAL			max_msgs或msg_size小于0，或waitq_type取值不合法；
 * 				EMFILE			系统中打开了过多的消息队列描述符；
 * 				ENOMEM			内存不足；
 * 				ECALLEDINISR	该接口不能在中断上下文中调用；
 * 				EMNOTINITED		消息队列模块尚未初始化。
 ***************************************************************************************/
extern mqd_t mq_create(int32_t mq_blockflag, int32_t max_msgs, int32_t msg_size, int32_t waitq_type);

/***************************************************************************************
 * 函数名：
 * 			mq_delete
 * 函数功能：
 * 			mq_delete()用于删除mq_create()所创建的未命名消息队列。
 * 			所有因调用mq_send()等待发送消息和调用mq_receive()等待接收消息的任务会被解除阻塞，并返回错误。
 * 			该函数调用后，mqdes就不再指向一个有效的消息队列。
 * 参数说明：
 *			mqdes：
 *				消息队列描述符。
 * 调用核：
 * 			所有核均可自行调用
 * 返回值：
 *			return == 0		：成功
 *			return == -1	：失败，不会改变消息队列，设定 errno指出错误。
 * 备注：
 * 			错误码：
 * 				ENOENT			指定的消息队列不存在；
 * 				EINVAL			所删除的消息队列不是由mq_create()创建的未命名消息队列；
 * 				ECALLEDINISR	该接口不能在中断上下文中调用；
 * 				EMNOTINITED		消息队列模块尚未初始化。
 ***************************************************************************************/
extern int32_t mq_delete(mqd_t mqdes);

/***************************************************************************************
 * 函数名：
 * 			mq_send
 * 函数功能：
 * 			mq_send()用来将msg_ptr指向的消息添加到mqdes指定的消息队列中。
 * 			如果指定消息队列不是满的，mq_send()会把消息插入到msg_prio指定的位置。
 * 			msg_prio值较大的消息会插入到msg_prio值较小的前面。
 * 			如果msg_prio相等，消息插入到其他msg_prio相等的消息的后面。
 * 			如果消息队列已满，且消息队列描述符mqdes中没有设置O_NONBLOCK标志，那么调用任务会阻塞，直到消息队列空间可用或者调用被信号中断；
 * 			如果设置了O_NONBLOCK标志，调用任务就不会阻塞，直接返回错误。
 * 			如果有多个任务在等待发送，那么在消息队列空间可用时，等待的任务会按照消息队列的属性mq_waitqtype的设置来解除任务的阻塞。
 * 			如果mq_waitqtype设置为PTHREAD_WAITQ_PRIO，则按照优先级策略解除任务的阻塞；
 * 			如果mq_waitqtype设置为PTHREAD_WAITQ_FIFO，则按照FIFO的策略解除任务的阻塞。
 * 参数说明：
 *			mqdes：
 *				消息队列描述符；
 *			msg_ptr：
 *				存放消息的缓存地址；
 *			msg_len：
 *				消息长度，其取值应小于等于消息队列的msg_msgsize属性，否则调用会失败；
 *			msg_prio：
 *				消息的优先级，其取值应小于MQ_PRIO_MAX。
 * 调用核：
 * 			所有核均可自行调用
 * 返回值：
 *			return == 0		：成功
 *			return == -1	：失败，消息不会入队，设定 errno指出错误。
 * 备注：
 * 			错误码：
 * 				EINVAL		msg_ptr是无效指针，或者msg_prio的取值不合法；
 * 				EBADF		mqdes指定的消息队列描述符不合法，或者mqdes指定的消息队列是只读的；
 * 				EMSGSIZE	msg_len 指定的消息长度超过了消息队列的最大消息长度属性；
 * 				EAGAIN		消息队列设置了O_NONBLOCK 标志，并且调用该接口时消息队列已满；
 * 				EINTR		调用任务被信号中断。
 ***************************************************************************************/
extern int32_t mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, uint32_t msg_prio);

/***************************************************************************************
 * 函数名：
 * 			mq_receive
 * 函数功能：
 * 			mq_receive()用来会从mqdes指定的消息队列中接收最高优先级消息中最早的消息。
 * 			如果由参数msg_len指定的缓存区大小小于消息队列属性 mq_msgsize，调用会失败并返回错误;
 * 			否则，选定的消息会从队列中删除，并复制到 msg_ptr指向的缓存区中。
 * 			如果指定的消息队列为空，且消息队列描述符mqdes没有设置O_NONBLOCK标志位，
 * 			mq_receive()会阻塞，直到一个消息加入到队列中或接口调用被信号中断。
 * 			如果有多个任务在等待接收消息，当消息到达时，等待的任务会按照消息队列的属性mq_waitqtype的设置来解除任务的阻塞。
 * 			mq_waitqtype的说明参见mq_send()接口的说明。
 * 			如果指定的消息队列为空，且消息队列描述符mqdes 设置了O_NONBLOCK标志位，则返回错误。
 * 参数说明：
 *			mqdes：
 *				消息队列描述符；
 *			msg_ptr：
 *				存放消息的缓存地址；
 *			msg_len：
 *				存放消息的缓存区的长度；
 *			msg_prio：
 *				存放消息优先级的指针，如果msg_prio不为空，选定消息的优先级会保存在msg_prio引用的位置。
 * 调用核：
 * 			所有核均可自行调用
 * 返回值：
 *			return > 0		：成功，从队列中删除选定的消息，返回选定消息的字节长度；
 *			return == -1	：失败，不删除消息，设定errno指出错误。
 * 备注：
 * 			错误码：
 * 				EINVAL		参数msg_ptr是无效指针；
 * 				EBADF		mqdes指定的消息队列描述符不合法，或者mqdes指定的消息队列是只写的；
 * 				EMSGSIZE	msg_len的值小于消息队列的消息大小属性；
 * 				EAGAIN		消息队列描述符mqdes中设置了O_NONBLOCK标志位，且接口调用时消息队列是空的；
 * 				EINTR		调用任务被信号中断。
 ***************************************************************************************/
extern int32_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, uint32_t *msg_prio);


#ifdef __cplusplus
}
#endif


#endif /* MQUEUE_H */
