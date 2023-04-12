#ifndef RESPOOL_H
#define RESPOOL_H


#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif


#define RESPOOL_RESCHECK_ABORT					(0)
#define RESPOOL_RESCHECK_CONTINUE				(-1)
#define RESPOOL_RESCHECK_DELETE					(-2)
#define RESPOOL_RESCHECK_DELETEANDABORT			(-3)


typedef struct _resPoolList_t
{
	struct _resPoolList_t		*next;
	uint32_t					inUse;

	int32_t						lockCnt;
	int32_t						gie;

	void 						*pRes;
}resPoolList_t;

typedef struct
{
	uint32_t				totalNum;
	uint32_t				useNum;
	uint32_t				freeNum;

	uint32_t				originalSize;
	uint32_t				alignmentSize;

	uint32_t				*pStack;
	char					*pBufferRes;
	resPoolList_t			*pBufferList;

	resPoolList_t			*pListStart;
	resPoolList_t			*pListEnd;

	int32_t					lockCnt;
	int32_t					gie;

	mpart_id				mid;

	uint32_t				is_initialized;
}resPool_t;


extern resPool_t *resPool_create(uint32_t resSize, uint32_t resNum, mpart_id mid, uint32_t alignment);
extern int32_t resPool_delete(resPool_t *pResPool);

extern void *resPool_alloc(resPool_t *pResPool, int32_t *pResHdl);
extern int32_t resPool_freeByHdl(resPool_t *pResPool, int32_t resHdl);
extern int32_t resPool_freeByAddr(resPool_t *pResPool, void *resAddr);

extern void *resPool_resAddrGet(resPool_t *pResPool, int32_t resHdl);
extern int32_t resPool_resHdlGet(resPool_t *pResPool, void *resAddr);

extern int32_t resPool_resCheck(resPool_t *pResPool, int32_t (*func)(void *resAddr, int32_t resHdl, void *arg), void *arg);

extern int32_t resPool_reset(resPool_t *pResPool);

extern int32_t resPool_poolLock(resPool_t *pResPool);
extern int32_t resPool_poolUnlock(resPool_t *pResPool);

extern int32_t resPool_resLock(resPool_t *pResPool, int32_t resHdl);
extern int32_t resPool_resUnlock(resPool_t *pResPool, int32_t resHdl);

extern int32_t resPool_listAdd(resPool_t *pResPool, int32_t resHdl);
extern int32_t resPool_listRemove(resPool_t *pResPool, int32_t resHdl);


#ifdef __cplusplus
}
#endif


#endif /* RESPOOL_H */
