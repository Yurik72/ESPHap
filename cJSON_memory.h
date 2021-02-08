#ifndef cJSON_MEMORY__h
#define cJSON_MEMORY__h

#include "port_x.h"
#ifdef __cplusplus
extern "C"
{
#endif

void set_allocator_buffer(void* b, size_t size);
void *internal_buffered_malloc(size_t size);
void internal_buffered_free(void *pointer);
void *internal_buffered_realloc(void *pointer, size_t size);
void reset_allocator();
#ifdef __cplusplus
}
#endif
#endif