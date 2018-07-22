#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

enum Wrap {
	SAME_WRAP,
	DIFF_WRAP
};

typedef struct RingBuffer {
	size_t read_pos;
	size_t write_pos;
	size_t element_count;
	size_t element_size;
	enum Wrap rw_wrap;
	char *data;
} RingBuffer;

RingBuffer *Acowa_CreateBuffer(size_t element_count, size_t element_size);
void Acowa_InitBuffer(RingBuffer *handle);
void Acowa_FreeBuffer(void *handle);

size_t Acowa_ReadBuffer(RingBuffer *handle, void **data_ptr, void *data, size_t element_count);

size_t Acowa_WriteBuffer(RingBuffer *handle, const void *data, size_t element_count);

int Acowa_MoveReadPtr(RingBuffer *handle, int element_count);

size_t Acowa_available_read(const RingBuffer *handle);

size_t Acowa_available_write(const RingBuffer *handle);

#ifdef __cplusplus	
}
#endif

#endif /* __RING_BUFFER_H__ */
