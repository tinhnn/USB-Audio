
#ifndef NAMI_RINGBUFFER_H_
#define NAMI_RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>

struct ringbuffer {
    uint8_t *buffer_ptr;
    uint16_t read_mirror :1;
    uint16_t read_index :15;
    uint16_t write_mirror :1;
    uint16_t write_index :15;
    uint16_t buffer_size;
};

enum ringbuffer_state {
    RT_RINGBUFFER_EMPTY,
    RT_RINGBUFFER_FULL,
    RT_RINGBUFFER_HALFFULL
};


void ringbuffer_init(struct ringbuffer *rb, uint8_t *pool, uint16_t size);
uint32_t ringbuffer_put(struct ringbuffer *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_put_force(struct  ringbuffer *rb, const uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch);
uint32_t ringbuffer_putchar_force(struct ringbuffer *rb, const uint8_t ch);
uint32_t ringbuffer_get(struct ringbuffer *rb, uint8_t *ptr, uint16_t length);
uint32_t ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch);
void  ringbuffer_flush(struct ringbuffer *rb);
uint16_t ringbuffer_get_size(struct ringbuffer *rb);
enum ringbuffer_state ringbuffer_status(struct  ringbuffer *rb);
uint16_t ringbuffer_data_len(struct  ringbuffer *rb);

#define ringbuffer_empty_space(rb) ((rb)->buffer_size -  ringbuffer_data_len(rb))

#endif /* NAMI_RINGBUFFER_H_ */
