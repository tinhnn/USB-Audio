
#include "nami_RingBuffer.h"
#include <string.h>

/******************************************************************************
 * Function:    ringbuffer_status
 * Description: return status of Ring Buffer: empty, full or half-full
 *
 *****************************************************************************/
__inline enum ringbuffer_state  ringbuffer_status(struct ringbuffer *rb)
{
    if (rb->read_index == rb->write_index) {
        if (rb->read_mirror == rb->write_mirror)
            return RT_RINGBUFFER_EMPTY;
        else
            return RT_RINGBUFFER_FULL;
    }
    return RT_RINGBUFFER_HALFFULL;
}

/******************************************************************************
 * Function:    ringbuffer_data_len
 * Description: return the size of data in Ring Buffer
 *
 *****************************************************************************/
uint16_t  ringbuffer_data_len(struct  ringbuffer *rb)
{
    switch ( ringbuffer_status(rb)) {
    case RT_RINGBUFFER_EMPTY:
        return 0;
    case RT_RINGBUFFER_FULL:
        return rb->buffer_size;
    case RT_RINGBUFFER_HALFFULL:
    default:
        if (rb->write_index > rb->read_index)
            return rb->write_index - rb->read_index;
        else
            return (uint16_t)(rb->buffer_size - (rb->read_index - rb->write_index));
    };
}

/******************************************************************************
 * Function:    ringbuffer_free_len
 * Description: return the free space size of Ring Buffer
 *
 *****************************************************************************/
uint16_t ringbuffer_free_len(struct ringbuffer *rb)
{
    return rb->buffer_size - ringbuffer_data_len(rb);
}

/******************************************************************************
 * Function:    ringbuffer_put
 * Description: put a block of data into Ring Buffer
 *
 *****************************************************************************/
uint32_t  ringbuffer_put(struct ringbuffer *rb,
                         const uint8_t     *ptr,
                         uint16_t           length)
{
    uint16_t size;

    /* whether has enough space */
    size =  ringbuffer_empty_space(rb);

    /* no space */
    if (size == 0)
        return 0;

    /* drop some data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->write_index > length)
    {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;
        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index],
           &ptr[0],
           rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           (size_t)(length - (rb->buffer_size - rb->write_index)));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    return length;
}

/******************************************************************************
 * Function:    ringbuffer_put_force
 * Description: put a block of data into Ring Buffer
 *              When the buffer is full, it will discard the old data.
 *
 *****************************************************************************/
uint32_t ringbuffer_put_force(struct ringbuffer *rb, const uint8_t *ptr, uint16_t length)
{
    enum ringbuffer_state old_state;

    old_state = ringbuffer_status(rb);

    if (length > rb->buffer_size)
        length = rb->buffer_size;

    if (rb->buffer_size - rb->write_index > length) {
        /* read_index - write_index = empty space */
        memcpy(&rb->buffer_ptr[rb->write_index], ptr, length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->write_index += length;

        if (old_state == RT_RINGBUFFER_FULL)
            rb->read_index = rb->write_index;

        return length;
    }

    memcpy(&rb->buffer_ptr[rb->write_index], &ptr[0], rb->buffer_size - rb->write_index);
    memcpy(&rb->buffer_ptr[0],
           &ptr[rb->buffer_size - rb->write_index],
           (size_t)(length - (rb->buffer_size - rb->write_index)));

    /* we are going into the other side of the mirror */
    rb->write_mirror = ~rb->write_mirror;
    rb->write_index = length - (rb->buffer_size - rb->write_index);

    if (old_state == RT_RINGBUFFER_FULL) {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = rb->write_index;
    }

    return length;
}

/******************************************************************************
 * Function:    ringbuffer_get
 * Description: get data from Ring Buffer
 *
 *****************************************************************************/
uint32_t  ringbuffer_get(struct ringbuffer *rb,
                         uint8_t           *ptr,
                         uint16_t           length)
{
    uint16_t size;

    /* whether has enough data  */
    size =  ringbuffer_data_len(rb);

    /* no data */
    if (size == 0)
        return 0;

    /* less data */
    if (size < length)
        length = size;

    if (rb->buffer_size - rb->read_index > length)
    {
        /* copy all of data */
        memcpy(ptr, &rb->buffer_ptr[rb->read_index], length);
        /* this should not cause overflow because there is enough space for
         * length of data in current mirror */
        rb->read_index += length;
        return length;
    }

    memcpy(&ptr[0],
           &rb->buffer_ptr[rb->read_index],
           rb->buffer_size - rb->read_index);
    memcpy(&ptr[rb->buffer_size - rb->read_index],
           &rb->buffer_ptr[0],
           (size_t)(length - (rb->buffer_size - rb->read_index)));

    /* we are going into the other side of the mirror */
    rb->read_mirror = ~rb->read_mirror;
    rb->read_index = length - (rb->buffer_size - rb->read_index);

    return length;
}

/******************************************************************************
 * Function:    ringbuffer_putchar
 * Description: put a character into Ring Buffer
 *
 *****************************************************************************/
uint32_t ringbuffer_putchar(struct ringbuffer *rb, const uint8_t ch)
{
    /* whether has enough space */
    if (! ringbuffer_empty_space(rb))
        return 0;

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size - 1) {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
    } else {
        rb->write_index++;
    }

    return 1;
}

/******************************************************************************
 * Function:    ringbuffer_putchar_force
 * Description: When the buffer is full, it will discard one old data.
 *
 *****************************************************************************/
uint32_t  ringbuffer_putchar_force(struct ringbuffer *rb, const uint8_t ch)
{
    enum  ringbuffer_state old_state;

    old_state =  ringbuffer_status(rb);

    rb->buffer_ptr[rb->write_index] = ch;

    /* flip mirror */
    if (rb->write_index == rb->buffer_size-1)
    {
        rb->write_mirror = ~rb->write_mirror;
        rb->write_index = 0;
        if (old_state == RT_RINGBUFFER_FULL)
        {
            rb->read_mirror = ~rb->read_mirror;
            rb->read_index = rb->write_index;
        }
    }
    else
    {
        rb->write_index++;
        if (old_state == RT_RINGBUFFER_FULL)
            rb->read_index = rb->write_index;
    }

    return 1;
}

/******************************************************************************
 * Function:    ringbuffer_getchar
 * Description: get a character from a Ring Buffer
 *
 *****************************************************************************/
uint32_t  ringbuffer_getchar(struct ringbuffer *rb, uint8_t *ch)
{
    /* ringbuffer is empty */
    if (! ringbuffer_data_len(rb))
        return 0;

    /* put character */
    *ch = rb->buffer_ptr[rb->read_index];

    if (rb->read_index == rb->buffer_size-1)
    {
        rb->read_mirror = ~rb->read_mirror;
        rb->read_index = 0;
    }
    else
    {
        rb->read_index++;
    }

    return 1;
}

/******************************************************************************
 * Function:    ringbuffer_flush
 * Description: Clear Index data in Ring Buffer
 *
 *****************************************************************************/
void  ringbuffer_flush(struct ringbuffer *rb)
{
    /* initialize read and write index */
    rb->read_mirror = 0;
    rb->read_index  = 0;
    rb->write_mirror = 0;
    rb->write_index  = 0;
}

/******************************************************************************
 * Function:    ringbuffer_init
 * Description: Init new Ring Buffer
 *
 *****************************************************************************/
void  ringbuffer_init(struct ringbuffer *rb,
                      uint8_t           *pool,
                      uint16_t           size)
{

    /* initialize read and write index */
    rb->read_mirror = 0;
    rb->read_index  = 0;
    rb->write_mirror = 0;
    rb->write_index  = 0;

    /* set buffer pool and size */
    rb->buffer_ptr = pool;
    rb->buffer_size = size; //ALIGN_DOWN(size, ALIGN_SIZE);
}
