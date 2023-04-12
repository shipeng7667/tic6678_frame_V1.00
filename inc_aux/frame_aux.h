#ifndef FRAME_AUX_H
#define FRAME_AUX_H


#include <stdint.h>
#include <stddef.h>

#include "mpart_aux.h"


#ifdef __cplusplus
extern "C" {
#endif


extern far mpart_id_ mid_frameHeap;


extern void *frameHeap_alloc(size_t size);
extern void *frameHeap_calloc(size_t size);
extern int32_t frameHeap_free(char *addr);


#ifdef __cplusplus
}
#endif


#endif /* FRAME_AUX_H */
