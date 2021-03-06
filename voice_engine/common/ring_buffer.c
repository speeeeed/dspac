#include "ring_buffer.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static size_t GetBufferReadRegions(RingBuffer *buf, size_t element_count, void **data_ptr_1, size_t *data_ptr_bytes_1, void **data_ptr_2, size_t *data_ptr_bytes_2)
{
	const size_t readable_elements = Acowa_available_read(buf);
	const size_t read_elements = (readable_elements < element_count ? readable_elements : element_count);
	const size_t margin = buf->element_count - buf->read_pos;

	if (read_elements > margin) {
		*data_ptr_1 = buf->data + buf->read_pos * buf->element_size;
		*data_ptr_bytes_1 = margin * buf->element_size;
		*data_ptr_2 = buf->data;
		*data_ptr_bytes_2 = (read_elements - margin) * buf->element_size;
	} else {
		*data_ptr_1 = buf->data + buf->read_pos * buf->element_size;
		*data_ptr_bytes_1 = read_elements * buf->element_size;
		*data_ptr_2 = NULL;
		*data_ptr_bytes_2 = 0;
	}

	return read_elements;
}

RingBuffer *Acowa_CreateBuffer(size_t element_count, size_t element_size)
{
	RingBuffer *self = NULL;

	if (element_count == 0 || element_size == 0) {
		return NULL;
	}

	self = (RingBuffer *)malloc(sizeof(RingBuffer));
	if (NULL == self) {
		return NULL;
	}

	self->data = (char *)malloc(element_count * element_size);
	if (NULL == self->data) {
		free(self);
		self = NULL;
		return NULL;
	}

	self->element_count = element_count;
	self->element_size = element_size;
	Acowa_InitBuffer(self);

	return self;
}

void Acowa_InitBuffer(RingBuffer *self)
{
	self->read_pos = 0;
	self->write_pos = 0;
	self->rw_wrap = SAME_WRAP;

	memset(self->data, 0, self->element_count * self->element_size);
}

void Acowa_FreeBuffer(void *handle)
{
	RingBuffer *self = (RingBuffer *)handle;

	if (NULL == self) {
		return;
	}

	free(self->data);
	free(self);
}

size_t Acowa_ReadBuffer(RingBuffer *self, void **data_ptr, void *data, size_t element_count)
{
	if (NULL == self) {
		return 0;
	}
	if (NULL == data) {
		return 0;
	}

	void *buf_ptr_1 = NULL;
	void *buf_ptr_2 = NULL;
	size_t buf_ptr_bytes_1 = 0;
	size_t buf_ptr_bytes_2 = 0;
	const size_t read_count = GetBufferReadRegions(self, element_count, &buf_ptr_1, &buf_ptr_bytes_1, &buf_ptr_2, &buf_ptr_bytes_2);

	if (buf_ptr_bytes_2 > 0) {
		memcpy(data, buf_ptr_1, buf_ptr_bytes_1);
		memcpy((char *)data + buf_ptr_bytes_1, buf_ptr_2, buf_ptr_bytes_2);
		buf_ptr_1 = data;
	} else if (!data_ptr) {
		memcpy(data, buf_ptr_1, buf_ptr_bytes_1);
	}

	if (data_ptr) {
		*data_ptr = read_count == 0 ? NULL : buf_ptr_1;
	}

	Acowa_MoveReadPtr(self, (int)read_count);

	return read_count;
}

size_t Acowa_WriteBuffer(RingBuffer *self, const void *data, size_t element_count)
{
	if (NULL == self) {
		return 0;
	}
	if (NULL == data) {
		return 0;
	}

	const size_t free_elements = Acowa_available_write(self);
	const size_t write_elements = (free_elements < element_count ? free_elements : element_count);
	size_t n = write_elements;
	const size_t margin = self->element_count - self->write_pos;

	if (write_elements > margin) {
		memcpy(self->data + self->write_pos * self->element_size, data, margin * self->element_size);
		self->write_pos = 0;
		n -= margin;
		self->rw_wrap = DIFF_WRAP;
	}

	memcpy(self->data + self->write_pos * self->element_size, ((const char *)data) + ((write_elements - n) * self->element_size), n * self->element_size);
	self->write_pos += n;

	return write_elements;
}

int Acowa_MoveReadPtr(RingBuffer *self, int element_count)
{
	if (NULL == self) {
		return 0;
	}

	const int free_elements = (int)Acowa_available_write(self);
	const int readable_elements = (int )Acowa_available_read(self);
	int read_pos = (int)self->read_pos;

	if (element_count > readable_elements) {
		element_count = readable_elements;
	}
	if (element_count < -free_elements) {
		element_count = -free_elements;
	}

	read_pos += element_count;
	if (read_pos > (int)self->element_count) {
		read_pos -= (int)self->element_count;
		self->rw_wrap = SAME_WRAP;
	}
	if (read_pos < 0) {
		read_pos += (int)self->element_count;
		self->rw_wrap = DIFF_WRAP;
	}

	self->read_pos = (size_t)read_pos;

	return element_count;
}

size_t Acowa_available_read(const RingBuffer *self)
{
	if (NULL == self) {
		return 0;
	}

	if (self->rw_wrap == SAME_WRAP) {
		return self->write_pos - self->read_pos;
	} else {
		return self->element_count - self->read_pos + self->write_pos;
	}
}

size_t Acowa_available_write(const RingBuffer *self)
{
	if (NULL == self) {
		return 0;
	}

	return self->element_count - Acowa_available_read(self);
}
