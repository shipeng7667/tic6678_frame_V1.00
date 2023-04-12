#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "mpart.h"
#include "intc.h"

#include "mpart_aux.h"


#define MPART_BLOCK_SIZE	64		//mpart模块block大小


far uint32_t mpart_block_size = MPART_BLOCK_SIZE;


int32_t mpart_create_kernel(char *addr, size_t size, mpart_id mid);
void memlock(mpart_id mid, int32_t *int_status);
void memunlock(mpart_id mid, int32_t *int_status);
void mpart_error(void);


int32_t mpart_create(char *addr, size_t size, mpart_id *mid)
{
	mpart_id_ *mi;
	int32_t result;

	if((mid == NULL) || (size == 0) || (addr == NULL))
	{
		return -1;
	}

	mi = (mpart_id_ *)memalign(64, sizeof(mpart_id_));
	if(mi == NULL)
	{
		return -2;
	}

	memset(mi, 0, sizeof(mpart_id_));

	result = mpart_create_kernel(addr, size, mi);
	if(result != 0)
	{
		return result;
	}

	*mid = mi;

	return 0;
}

int32_t mpart_delete(mpart_id mid)
{
	mpart_id_ *_mid = (mpart_id_ *)mid;

	if((_mid == NULL) || (_mid->is_initialized != 1))
	{
		return -1;
	}

	free(_mid);

	return 0;
}

int32_t mpart_create_kernel(char *addr, size_t size, mpart_id mid)
{
	mpart_id_ *_mid = (mpart_id_ *)mid;

	size_t divider;
	uintptr_t start_align_addr, end_align_addr;

	if(_mid == NULL)
	{
		return -1;
	}

	start_align_addr = (((uintptr_t)addr + mpart_block_size - 1) & (~(mpart_block_size - 1)));
	end_align_addr = ((uintptr_t)addr + size) & (~(mpart_block_size - 1));
	divider = (end_align_addr - start_align_addr)/ (mpart_block_size + sizeof(mpack_t));

	_mid->addr = addr;
	_mid->size = size;
	_mid->alloc_addr = (char *)start_align_addr;
	_mid->alloc_size = divider * mpart_block_size;

	if(_mid->alloc_size == 0)
	{
		return -2;
	}

	_mid->dp_size = divider * sizeof(mpack_t);
	_mid->dp_addr = (mpack_t *)((uintptr_t)_mid->alloc_addr + _mid->alloc_size);

	_mid->usable = _mid->alloc_size - mpart_block_size;
	_mid->usable_min = _mid->alloc_size - mpart_block_size;

	_mid->alloc_cnt=0;
	_mid->free_cnt=0;

	memset(_mid->dp_addr, 0, _mid->dp_size);

	_mid->dp_addr->size = mpart_block_size;
	_mid->dp_addr->prev = NULL;
	_mid->dp_addr->next = NULL;
	_mid->dp_addr->crt_block_end = _mid->dp_addr;

	_mid->is_initialized = 1;

	return 0;
}

void *mpart_alloc(mpart_id mid, size_t size)
{
	mpart_id_ *_mid = (mpart_id_ *)mid;

	uintptr_t start_addr, crt_block_addr_end, boundary;
	mpack_t *start_alloc, *crt_block_start_alloc, *crt_block_end_alloc, *next_block_start_alloc, *new_alloc;
	int32_t status;
	size_t newsize;

	memlock(mid, &status);			//这里以后要考虑互斥量

	if((_mid == NULL) || (_mid->is_initialized != 1))
	{
		memunlock(mid, &status);
		return NULL;
	}

	if(size == 0)
	{
		memunlock(mid, &status);
		return NULL;
	}

	start_addr = (uintptr_t)_mid->alloc_addr;
	start_alloc = _mid->dp_addr;
	crt_block_start_alloc = _mid->dp_addr;

	newsize = (size + mpart_block_size - 1) & (~(mpart_block_size - 1));

	while(1)
	{
		crt_block_end_alloc = crt_block_start_alloc->crt_block_end;
		next_block_start_alloc = crt_block_end_alloc->next;
		crt_block_addr_end = (start_addr + ((uintptr_t)(crt_block_end_alloc - start_alloc) * mpart_block_size)) + crt_block_end_alloc->size;

		if(next_block_start_alloc == NULL)
		{
			boundary = (uintptr_t)_mid->alloc_addr + _mid->alloc_size;
		}
		else
		{
			boundary = start_addr + ((uintptr_t)(next_block_start_alloc - start_alloc) * mpart_block_size);
		}

		if((boundary - crt_block_addr_end) >= newsize)
		{
			new_alloc = _mid->dp_addr + ((crt_block_addr_end - start_addr) / mpart_block_size);

			new_alloc->prev = crt_block_end_alloc;
			new_alloc->size = newsize;
			new_alloc->next = next_block_start_alloc;
			new_alloc->crt_block_end = NULL;

			crt_block_start_alloc->crt_block_end = new_alloc;
			crt_block_end_alloc->next = new_alloc;

			if(next_block_start_alloc != NULL)
			{
				next_block_start_alloc->prev = new_alloc;

				if((boundary - crt_block_addr_end) == newsize)		//和右BLOCK连上了
				{
					crt_block_start_alloc->crt_block_end = next_block_start_alloc->crt_block_end;
					next_block_start_alloc->crt_block_end = NULL;
				}
			}

			_mid->alloc_cnt++;
			_mid->usable-=newsize;
			if(_mid->usable < _mid->usable_min)
			{
				_mid->usable_min = _mid->usable;
			}

			while((crt_block_addr_end & (mpart_block_size - 1)) != 0);
			if((uintptr_t)crt_block_addr_end < (uintptr_t)_mid->alloc_addr)
			{
				mpart_error();
			}
			if((uintptr_t)crt_block_addr_end > ((uintptr_t)_mid->alloc_addr + _mid->alloc_size))
			{
				mpart_error();
			}
			memunlock(mid, &status);
			return (void *)crt_block_addr_end;
		}
		else
		{
			if(next_block_start_alloc != NULL)
			{
				crt_block_start_alloc = next_block_start_alloc;
				continue;
			}
			else
			{
				memunlock(mid, &status);
				return NULL;
			}
		}
	}
}

void *mpart_calloc(mpart_id mid, size_t size)
{
	void *result;

	result = mpart_alloc(mid, size);

	if(result != NULL)
	{
		memset(result, 0, size);
	}

	return result;
}

int32_t mpart_free(mpart_id mid, char *addr)
{
	mpart_id_ *_mid = (mpart_id_ *)mid;

	uintptr_t start_addr;
	mpack_t *prev_alloc, *crt_alloc, *next_alloc, *crt_block_start_alloc, *tgt_alloc;
	int32_t status;

	memlock(mid, &status);

	if((_mid == NULL) || (_mid->is_initialized != 1))
	{
		memunlock(mid, &status);
		return -1;
	}

	start_addr = (uintptr_t)_mid->alloc_addr;

	prev_alloc = NULL;
	crt_alloc = _mid->dp_addr;
	next_alloc = crt_alloc->next;
	crt_block_start_alloc = _mid->dp_addr;
	tgt_alloc = _mid->dp_addr + (((uintptr_t)addr - start_addr) / mpart_block_size);

	if((addr == NULL) || (addr == _mid->alloc_addr) || (tgt_alloc->size == 0))
	{
		memunlock(mid, &status);
		return -1;
	}

	while(1)
	{
		if(tgt_alloc <= crt_block_start_alloc->crt_block_end)
		{
			prev_alloc = tgt_alloc->prev;
			next_alloc = tgt_alloc->next;
			prev_alloc->next = next_alloc;
			next_alloc->prev = prev_alloc;

			if(crt_block_start_alloc != crt_block_start_alloc->crt_block_end)
			{
				if(tgt_alloc == crt_block_start_alloc)
				{
					next_alloc->crt_block_end = crt_block_start_alloc->crt_block_end;
				}
				else if(tgt_alloc == crt_block_start_alloc->crt_block_end)
				{
					crt_block_start_alloc->crt_block_end = prev_alloc;
				}
				else
				{
					next_alloc->crt_block_end = crt_block_start_alloc->crt_block_end;
					crt_block_start_alloc->crt_block_end = prev_alloc;
				}
			}

			_mid->free_cnt++;
			_mid->usable += tgt_alloc->size;

			tgt_alloc->size = 0;
			tgt_alloc->crt_block_end = NULL;
			tgt_alloc->next = NULL;
			tgt_alloc->prev = NULL;

			memunlock(mid, &status);

			return 0;
		}
		else
		{
			crt_block_start_alloc = crt_block_start_alloc->crt_block_end->next;
		}
	}
}

void memlock(mpart_id mid, int32_t *int_status)
{
	*int_status = int_lock();
}

void memunlock(mpart_id mid, int32_t *int_status)
{
	int_unlock(*int_status);
}

void mpart_error(void)
{
	while(1);
}
