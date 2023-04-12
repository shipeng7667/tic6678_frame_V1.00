#ifndef MPART_AUX_H
#define MPART_AUX_H


#include <stdint.h>
#include <stddef.h>

#include "resPool.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef struct _mpack_t
{
	size_t size;
	struct _mpack_t *prev;
    struct _mpack_t *next;
    struct _mpack_t *crt_block_end;
}mpack_t;

typedef struct _mpart_id_
{
	char *addr;					//�ڴ�����׵�ַ
	size_t size;				//�ڴ��������
	char *alloc_addr;			//�������׵�ַ
	size_t alloc_size;			//����������
	mpack_t *dp_addr;			//�������׵�ַ
	size_t dp_size;				//����������

	size_t usable;				//���������ô�С
	size_t usable_min;			//��С������
	uint32_t alloc_cnt;
	uint32_t free_cnt;
	int32_t is_initialized;
}mpart_id_;


extern far uint32_t mpart_block_size;


extern int32_t mpart_create_kernel(char *addr, size_t size, mpart_id mid);


#ifdef __cplusplus
}
#endif


#endif /* MPART_AUX_H */
