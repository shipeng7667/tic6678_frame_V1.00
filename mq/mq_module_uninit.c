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
#include "frame_aux.h"


int32_t mq_module_uninit(void)
{
	int32_t result;

	result = resPool_delete(pResPool_mq);
	if(result)
	{
		return -1;
	}

	return 0;
}
