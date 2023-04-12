#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mpart.h"
#include "intc.h"
#include "mqueue.h"
#include "frame.h"
#include "resPool.h"


resPool_t *resPool_create(uint32_t resSize, uint32_t resNum, mpart_id mid, uint32_t alignment)
{
	int32_t status = 0;

	resPool_t *pResPool = NULL;
	uint32_t *pStack = NULL;
	char *pBufferRes = NULL;
	resPoolList_t *pBufferList = NULL;

	uint32_t i;
	uint32_t size_alignment;

	if((resSize == 0) || (resNum == 0) || (mid == NULL))
	{
		status = -1;

		goto exit;
	}

	size_alignment = ((resSize - 1) / alignment + 1) * alignment;

reAllocResPool:
	pResPool = (resPool_t *)mpart_alloc(mid, sizeof(*pResPool));
	if(pResPool == NULL)
	{
		goto reAllocResPool;
	}

reAllocStack:
	pStack = (uint32_t *)mpart_alloc(mid, resNum * sizeof(uint32_t));
	if(pStack == NULL)
	{
		goto reAllocStack;
	}

reAllocBufferRes:
	pBufferRes = (char *)mpart_alloc(mid, resNum * size_alignment);
	if(pBufferRes == NULL)
	{
		goto reAllocBufferRes;
	}

reAllocBufferList:
	pBufferList = (resPoolList_t *)mpart_alloc(mid, resNum * sizeof(*pBufferList));
	if(pBufferList == NULL)
	{
		goto reAllocBufferList;
	}

	memset(pResPool, 0, sizeof(*pResPool));
	memset(pStack, 0, resNum * sizeof(uint32_t));
	memset(pBufferRes, 0, resNum * size_alignment);
	memset(pBufferList, 0, resNum * sizeof(*pBufferList));

exit:
	if(status)
	{
		if(pBufferList)
		{
			mpart_free(mid, (char *)pBufferList);
		}

		if(pBufferRes)
		{
			mpart_free(mid, (char *)pBufferRes);
		}

		if(pStack)
		{
			mpart_free(mid, (char *)pStack);
		}

		if(pResPool)
		{
			mpart_free(mid, (char *)pResPool);
		}

		return NULL;
	}

	for(i = 0; i < resNum; i++)
	{
		pStack[resNum - i - 1] = i;
		pBufferList[i].pRes = &(pBufferRes[i * size_alignment]);
	}

	pResPool->totalNum = resNum;
	pResPool->useNum = 0;
	pResPool->freeNum = resNum;

	pResPool->originalSize = resSize;
	pResPool->alignmentSize = size_alignment;

	pResPool->pStack = pStack;
	pResPool->pBufferRes = pBufferRes;
	pResPool->pBufferList = pBufferList;

	pResPool->pListStart = NULL;
	pResPool->pListEnd = NULL;

	pResPool->lockCnt = 0;

	pResPool->mid = mid;

	pResPool->is_initialized = 1;

	return pResPool;
}

int32_t resPool_delete(resPool_t *pResPool)
{
	if((pResPool == NULL) || (!pResPool->is_initialized) || (pResPool->pStack == NULL) || (pResPool->pBufferRes == NULL) || (pResPool->pBufferList == NULL) || (pResPool->mid == NULL))
	{
		return -1;
	}

	mpart_free(pResPool->mid, (char *)pResPool->pStack);
	mpart_free(pResPool->mid, (char *)pResPool->pBufferRes);
	mpart_free(pResPool->mid, (char *)pResPool->pBufferList);
	mpart_free(pResPool->mid, (char *)pResPool);

	return 0;
}

int32_t resPool_reset(resPool_t *pResPool)
{
	uint32_t i;

	if(pResPool == NULL)
	{
		return -1;
	}

	if((!pResPool->is_initialized) || (pResPool->pStack == NULL) || (pResPool->pBufferRes == NULL) || (pResPool->pBufferList == NULL))
	{
		return -2;
	}

	resPool_poolLock(pResPool);

	pResPool->useNum = 0;
	pResPool->freeNum = pResPool->totalNum;

	pResPool->pListStart = NULL;
	pResPool->pListEnd = NULL;

	memset(pResPool->pBufferRes, 0, pResPool->totalNum * pResPool->alignmentSize);
	memset(pResPool->pBufferList, 0, pResPool->totalNum * sizeof(*pResPool->pBufferList));

	for(i = 0; i < pResPool->totalNum; i++)
	{
		pResPool->pStack[pResPool->totalNum - i - 1] = i;
		pResPool->pBufferList[i].pRes = &(pResPool->pBufferRes[i * pResPool->alignmentSize]);
	}

	resPool_poolUnlock(pResPool);

	return 0;
}

void *resPool_alloc(resPool_t *pResPool, int32_t *pResHdl)
{
	uint32_t resHdl;

	if(pResPool == NULL)
	{
		return NULL;
	}

	if(!pResPool->is_initialized)
	{
		return NULL;
	}

	if(pResPool->freeNum == 0)
	{
		return NULL;
	}

	resPool_poolLock(pResPool);

	pResPool->useNum++;
	pResPool->freeNum--;

	resHdl = pResPool->pStack[pResPool->freeNum];

	resPool_listAdd(pResPool, resHdl);

	if(pResHdl)
	{
		*pResHdl = resHdl;
	}

	resPool_poolUnlock(pResPool);

	return &(pResPool->pBufferRes[pResPool->alignmentSize * resHdl]);
}

int32_t resPool_freeByHdl(resPool_t *pResPool, int32_t resHdl)
{
	if((pResPool == NULL) || (resHdl < 0))
	{
		return -1;
	}

	if(!pResPool->is_initialized)
	{
		return -2;
	}

	if(pResPool->useNum == 0)
	{
		return -3;
	}

	resPool_poolLock(pResPool);

	if(resPool_listRemove(pResPool, resHdl) < 0)
	{
		resPool_poolUnlock(pResPool);

		return -4;
	}

	pResPool->pStack[pResPool->freeNum] = resHdl;

	pResPool->freeNum++;
	pResPool->useNum--;

	resPool_poolUnlock(pResPool);

	return 0;
}

int32_t resPool_freeByAddr(resPool_t *pResPool, void *resAddr)
{
	int32_t resHdl;

	if((pResPool == NULL) || (resAddr == NULL))
	{
		return -1;
	}

	if(!pResPool->is_initialized)
	{
		return -2;
	}

	if(pResPool->useNum == 0)
	{
		return -3;
	}

	resPool_poolLock(pResPool);

	resHdl = ((uintptr_t)resAddr - (uintptr_t)pResPool->pBufferRes) / pResPool->alignmentSize;

	if(resPool_listRemove(pResPool, resHdl) < 0)
	{
		resPool_poolUnlock(pResPool);

		return -4;
	}

	pResPool->pStack[pResPool->freeNum] = resHdl;

	pResPool->freeNum++;
	pResPool->useNum--;

	resPool_poolUnlock(pResPool);

	return 0;
}

void *resPool_resAddrGet(resPool_t *pResPool, int32_t resHdl)
{
	void *resAddr;

	if((pResPool == NULL) || (resHdl < 0))
	{
		return NULL;
	}

	if(!pResPool->is_initialized)
	{
		return NULL;
	}

	resPool_poolLock(pResPool);

	resAddr = &(pResPool->pBufferRes[pResPool->alignmentSize * resHdl]);

	resPool_poolUnlock(pResPool);

	return resAddr;
}

int32_t resPool_resHdlGet(resPool_t *pResPool, void *resAddr)
{
	int32_t resHdl;

	if((pResPool == NULL) || (resAddr == NULL))
	{
		return -1;
	}

	if(!pResPool->is_initialized)
	{
		return -2;
	}

	resPool_poolLock(pResPool);

	resHdl = ((uintptr_t)resAddr - (uintptr_t)pResPool->pBufferRes) / pResPool->alignmentSize;

	resPool_poolUnlock(pResPool);

	return resHdl;
}

int32_t resPool_resCheck(resPool_t *pResPool, int32_t (*func)(void *resAddr, int32_t resHdl, void *arg), void *arg)
{
	int32_t result, status = 0;

	int32_t resHdl;

	uint32_t exist;

	resPoolList_t *pList_crt, *pList_pre, *pList_temp;

	if((pResPool == NULL) || (func == NULL))
	{
		return -1;
	}

	if(!pResPool->is_initialized)
	{
		return -2;
	}

	if(pResPool->useNum == 0)
	{
		return -3;
	}

	resPool_poolLock(pResPool);

	pList_crt = pResPool->pListStart;
	pList_pre = NULL;

	exist = 1;

	while(1)
	{
		if(pList_crt == NULL)
		{
			exist = 0;

			break;
		}

		resHdl = ((uintptr_t)pList_crt->pRes - (uintptr_t)pResPool->pBufferRes) / pResPool->alignmentSize;

		result = func(pList_crt->pRes, resHdl, arg);

		if(result == RESPOOL_RESCHECK_ABORT)			//停止搜索链表
		{
			break;
		}
		else if((result == RESPOOL_RESCHECK_DELETE) || (result == RESPOOL_RESCHECK_DELETEANDABORT))	//删除此成员
		{
			pResPool->pStack[pResPool->freeNum] = resHdl;

			pResPool->freeNum++;
			pResPool->useNum--;

			if(pList_crt == pResPool->pListEnd)
			{
				pResPool->pListEnd = pList_pre;
			}

			if(pList_pre == NULL)
			{
				pResPool->pListStart = pList_crt->next;
			}
			else
			{
				pList_pre->next = pList_crt->next;
			}

			pList_temp = pList_crt->next;

			pList_crt->next = NULL;
			pList_crt->inUse = 0;

			pList_crt = pList_temp;

			if(result == RESPOOL_RESCHECK_DELETEANDABORT)
			{
				break;
			}
		}
		else //if(result == RESPOOL_RESCHECK_CONTINUE)
		{
			pList_pre = pList_crt;

			pList_crt = pList_crt->next;
		}
	}

	if(!exist)
	{
		status = -4;

		goto exit;
	}

	if(pResPool->pListStart == NULL)
	{
		status = -5;

		goto exit;
	}

	resHdl = ((uintptr_t)(pList_crt->pRes) - (uintptr_t)(pResPool->pBufferRes)) / pResPool->alignmentSize;

exit:
	resPool_poolUnlock(pResPool);

	if(status)
	{
		return status;
	}

	return resHdl;
}

int32_t resPool_poolLock(resPool_t *pResPool)
{
	int32_t gie;

	if(pResPool == NULL)
	{
		return -1;
	}

	gie = int_lock();

	if(pResPool->lockCnt == 0)
	{
		pResPool->gie = gie;

		pResPool->lockCnt++;
	}
	else
	{
		pResPool->lockCnt++;

		int_unlock(gie);
	}

	return 0;
}

int32_t resPool_poolUnlock(resPool_t *pResPool)
{
	if(pResPool == NULL)
	{
		return -1;
	}

	if(pResPool->lockCnt == 0)
	{
		while(1);
	}
	else
	{
		pResPool->lockCnt--;

		if(pResPool->lockCnt == 0)
		{
			int_unlock(pResPool->gie);
		}
	}

	return 0;
}

int32_t resPool_resLock(resPool_t *pResPool, int32_t resHdl)
{
	int32_t gie;

	resPoolList_t *pList;

	if((pResPool == NULL) || (resHdl < 0))
	{
		return -1;
	}

	pList = &(pResPool->pBufferList[resHdl]);

	gie = int_lock();

	if(pList->lockCnt == 0)
	{
		pList->gie = gie;

		pList->lockCnt++;
	}
	else
	{
		pList->lockCnt++;

		int_unlock(gie);
	}

	return 0;
}

int32_t resPool_resUnlock(resPool_t *pResPool, int32_t resHdl)
{
	resPoolList_t *pList;

	if((pResPool == NULL) || (resHdl < 0))
	{
		return -1;
	}

	pList = &(pResPool->pBufferList[resHdl]);

	if(pList->lockCnt == 0)
	{
		while(1);
	}
	else
	{
		pList->lockCnt--;

		if(pList->lockCnt == 0)
		{
			int_unlock(pList->gie);
		}
	}

	return 0;
}

int32_t resPool_listAdd(resPool_t *pResPool, int32_t resHdl)
{
	int32_t status = 0;

	resPoolList_t *pList;

	if((pResPool == NULL) || (resHdl < 0))
	{
		return -1;
	}

	//resPool_poolLock(pResPool);

	if(!pResPool->is_initialized)
	{
		status = -1;

		goto exit;
	}

	pList = &(pResPool->pBufferList[resHdl]);

	pList->next = NULL;
	pList->inUse = 1;

	if(pResPool->pListStart == NULL)
	{
		pResPool->pListStart = pList;
	}
	else
	{
		pResPool->pListEnd->next = pList;
	}

	pResPool->pListEnd = pList;

exit:
	//resPool_poolUnlock(pResPool);

	return status;
}

int32_t resPool_listRemove(resPool_t *pResPool, int32_t resHdl)
{
	int32_t status = 0;

	resPoolList_t *pList, *pList_crt, *pList_pre;

	uint32_t exist;

	if((pResPool == NULL) || (resHdl < 0))
	{
		return -1;
	}

	//resPool_poolLock(pResPool);

	if(!pResPool->is_initialized)
	{
		status = -2;

		goto exit;
	}

	pList = &(pResPool->pBufferList[resHdl]);

	pList_crt = pResPool->pListStart;
	pList_pre = NULL;

	exist = 1;

	while(1)
	{
		if(pList_crt == NULL)
		{
			exist = 0;

			break;
		}

		if(pList_crt == pList)
		{
			break;
		}

		pList_pre = pList_crt;

		pList_crt = pList_crt->next;
	}

	if(!exist)
	{
		status = -3;

		goto exit;
	}

	//删除节点
	if(pList_crt == pResPool->pListEnd)
	{
		pResPool->pListEnd = pList_pre;
	}

	if(pList_pre == NULL)
	{
		pResPool->pListStart = pList_crt->next;
	}
	else
	{
		pList_pre->next = pList_crt->next;
	}

	pList->next = NULL;
	pList->inUse = 0;

exit:
	//resPool_poolUnlock(pResPool);

	return status;
}
