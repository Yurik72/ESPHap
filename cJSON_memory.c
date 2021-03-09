#include <string.h>
#include <stdio.h>

#include <stdlib.h>

#include <ctype.h>

#include "esphap_cJSON.h"
#include "cJSON_memory.h"

//#define JSON_MEMORY_DEBUG

#ifdef JSON_MEMORY_DEBUG
#define TRC(message, ...) printf(">json mem " message "\n",  ##__VA_ARGS__)
#else
#define TRC(message, ...)
#endif
typedef struct {
	void*  root;
	void*  buffer;
	int buffer_size;
	int offset;
} json_allocator_t __attribute__((aligned(4)));
#ifdef CJSON_USE_PREALLOCATED_BUFFER
static json_allocator_t  allocator = { NULL,NULL,0,0 };
static cJSON_Hooks allocator_hooks = { internal_buffered_malloc, internal_buffered_free };
#endif

size_t align_json_memory_size(size_t size) {
	if (size % sizeof(void*)) {
		size += sizeof(void*) - size % sizeof(void*);
	}
	return size;
}

void set_allocator_buffer(void* b, size_t size) {
#ifdef CJSON_USE_PREALLOCATED_BUFFER
	TRC("json  set_allocator_buffer");
	allocator.buffer_size = size;
	allocator.buffer = b;
	allocator.root = NULL;
	esphap_cJSON_InitHooks(&allocator_hooks);

#endif
}
void *internal_buffered_malloc(size_t size)
{
#ifdef CJSON_USE_PREALLOCATED_BUFFER
	TRC("json malloc %d", size);
	size = align_json_memory_size(size);
	if (allocator.buffer && (size + allocator.offset) < allocator.buffer_size) {


		void *b = (void*)(allocator.buffer + allocator.offset);

		if (allocator.offset == 0) {
			allocator.root = b;

			TRC("json malloc set root %d", allocator.root);
		}
		allocator.offset += size;
		TRC("json offset %d, pointer %d", allocator.offset, b);
		
		return b;
	}
#endif
	return malloc(size);
}
void internal_buffered_free(void *pointer)
{
#ifdef CJSON_USE_PREALLOCATED_BUFFER
	TRC("json free %d", pointer);

	if (pointer >= allocator.buffer && pointer <= (allocator.buffer + allocator.buffer_size)) {
		if (pointer == allocator.buffer)
			reset_allocator();
			return;
	}

#endif		
	free(pointer);
}
void *internal_buffered_realloc(void *pointer, size_t size)
{
#ifdef CJSON_USE_PREALLOCATED_BUFFER
	TRC("json realloc %d", size);
	if (pointer >= allocator.buffer && pointer <= (allocator.buffer + allocator.buffer_size)) {
		void* newpointer = internal_buffered_malloc(size);
		TRC("json realloc %d->%d", pointer, newpointer);
		if (newpointer && newpointer != pointer) {
			memcpy(newpointer, pointer, size);
			TRC("json copy realloc");
		}
		if (newpointer >= allocator.buffer && newpointer <= (allocator.buffer + allocator.buffer_size)) {

		}
		TRC("json  realloc return");
		return newpointer;
	}
	TRC("json  realloc stndard return");
#endif
	return  realloc(pointer, size);
}
/*
void* create_from_allocator(const internal_hooks * const hooks) {
	void* json = hooks->allocate(sizeof(cJSON));
	allocator.json = json;
	return json;
}
*/

void reset_allocator() {
#ifdef CJSON_USE_PREALLOCATED_BUFFER
	TRC("reset_allocator");
	esphap_cJSON_InitHooks(NULL);

	allocator.root = NULL;
	allocator.offset = 0;
	allocator.buffer_size = 0;

#endif
}