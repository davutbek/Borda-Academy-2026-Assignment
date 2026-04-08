/**
 * @file    circular_buffer.c
 * @brief   Generic Circular (Ring) Buffer — Implementation
 *
 * @details
 *   All operations are O(1) except buffer_peek_all() which is O(N).
 *   Thread-safety is NOT guaranteed; disable interrupts around put/get
 *   pairs if the buffer is shared between an ISR and the main loop.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#include "circular_buffer.h"
#include <stddef.h>  /* NULL */

/* ─────────────── Private helpers ───────────────────────────────────────── */

/**
 * @brief  Returns true if the buffer contains no elements.
 */
static bool buf_is_empty(const struct buf_handle_t *p_handle)
{
    return (!p_handle->full) && (p_handle->head == p_handle->tail);
}

/* ─────────────── Public API ─────────────────────────────────────────────── */

int buffer_init(struct buf_handle_t *p_handle,
                float               *backing_array,
                uint16_t             capacity)
{
    if ((p_handle == NULL) || (backing_array == NULL) || (capacity == 0U))
    {
        return CIRCULAR_BUF_ERROR;
    }

    p_handle->buffer  = backing_array;
    p_handle->head    = 0U;
    p_handle->tail    = 0U;
    p_handle->max_len = capacity;
    p_handle->full    = false;

    return CIRCULAR_BUF_OK;
}

int buffer_put_value(struct buf_handle_t *p_handle, float sensor_data)
{
    if (p_handle == NULL)
    {
        return CIRCULAR_BUF_ERROR;
    }

    /* Write new sample at tail position */
    p_handle->buffer[p_handle->tail] = sensor_data;

    if (p_handle->full)
    {
        /*
         * Buffer full: advance head to discard the oldest element.
         * This implements the overwrite / wrap-around policy required
         * by the assignment brief.
         */
        p_handle->head = (uint16_t)((p_handle->head + 1U) % p_handle->max_len);
    }

    /* Advance tail to the next write position */
    p_handle->tail = (uint16_t)((p_handle->tail + 1U) % p_handle->max_len);

    /* Update full flag: full when tail has wrapped around to head */
    p_handle->full = (p_handle->tail == p_handle->head);

    return CIRCULAR_BUF_OK;
}

int buffer_get_value(struct buf_handle_t *p_handle, float *p_sensor_data)
{
    if ((p_handle == NULL) || (p_sensor_data == NULL))
    {
        return CIRCULAR_BUF_ERROR;
    }

    if (buf_is_empty(p_handle))
    {
        return CIRCULAR_BUF_EMPTY;
    }

    /* Read oldest element from head */
    *p_sensor_data = p_handle->buffer[p_handle->head];

    /* Advance head and clear full flag */
    p_handle->full = false;
    p_handle->head = (uint16_t)((p_handle->head + 1U) % p_handle->max_len);

    return CIRCULAR_BUF_OK;
}

uint16_t buffer_get_count(const struct buf_handle_t *p_handle)
{
    if (p_handle == NULL)
    {
        return 0U;
    }

    if (p_handle->full)
    {
        return p_handle->max_len;
    }

    if (p_handle->tail >= p_handle->head)
    {
        return (uint16_t)(p_handle->tail - p_handle->head);
    }

    /* Tail has wrapped behind head */
    return (uint16_t)(p_handle->max_len - p_handle->head + p_handle->tail);
}

int buffer_peek_all(const struct buf_handle_t *p_handle,
                    float                     *out_array,
                    uint16_t                  *out_len)
{
    uint16_t count;
    uint16_t i;
    uint16_t idx;

    if ((p_handle == NULL) || (out_array == NULL) || (out_len == NULL))
    {
        return CIRCULAR_BUF_ERROR;
    }

    count = buffer_get_count(p_handle);
    *out_len = count;

    /* Walk from head forward, wrapping around the ring */
    for (i = 0U; i < count; i++)
    {
        idx          = (uint16_t)((p_handle->head + i) % p_handle->max_len);
        out_array[i] = p_handle->buffer[idx];
    }

    return CIRCULAR_BUF_OK;
}
