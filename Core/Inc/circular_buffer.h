/**
 * @file    circular_buffer.h
 * @brief   Generic Circular (Ring) Buffer for Float Sensor Data
 *
 * @details
 *   A fixed-capacity FIFO circular buffer.  When the buffer is full,
 *   new samples overwrite the oldest entry (overrun / wrap-around policy).
 *   This guarantees that the buffer always holds the N most-recent readings
 *   — appropriate for a sliding-window statistics engine.
 *
 *   Usage pattern:
 *     1. Allocate a backing array: float my_array[CAPACITY];
 *     2. Declare a handle:         struct buf_handle_t my_buf;
 *     3. Initialise the handle:    buffer_init(&my_buf, my_array, CAPACITY);
 *     4. In the 1-Hz loop:         buffer_put_value(&my_buf, new_sample);
 *     5. In the 30-s stats loop:   snapshot all values for computation.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

/* ─────────────── Return Codes ──────────────────────────────────────────── */
#define CIRCULAR_BUF_OK      ( 0)  /**< Operation succeeded                  */
#define CIRCULAR_BUF_ERROR   (-1)  /**< NULL handle / NULL backing array     */
#define CIRCULAR_BUF_EMPTY   (-2)  /**< get() called on an empty buffer      */

/* ─────────────── Handle Type ───────────────────────────────────────────── */

/**
 * @brief  Circular buffer handle (as specified in the assignment brief).
 *
 * @note   Do NOT modify the fields directly — use the API functions below.
 *         The backing array (`buffer`) must be allocated by the caller and
 *         must remain valid for the lifetime of the handle.
 */
struct buf_handle_t
{
    float    *buffer;    /**< Pointer to caller-allocated backing array       */
    uint16_t  head;      /**< Read index  — oldest element                   */
    uint16_t  tail;      /**< Write index — next free slot                   */
    uint16_t  max_len;   /**< Capacity (number of float elements)            */
    bool      full;      /**< True when tail has caught up with head         */
};

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Initialises a circular buffer handle.
 *
 * @param  p_handle      Pointer to a caller-allocated buf_handle_t struct.
 * @param  backing_array Caller-allocated float array of length `capacity`.
 * @param  capacity      Number of float elements in `backing_array`.
 *
 * @retval CIRCULAR_BUF_OK     Success.
 * @retval CIRCULAR_BUF_ERROR  NULL pointer or zero capacity.
 */
int buffer_init(struct buf_handle_t *p_handle,
                float               *backing_array,
                uint16_t             capacity);

/**
 * @brief  Inserts a sample into the buffer.
 *
 * @details
 *   If the buffer is full the oldest element is silently overwritten
 *   (overrun / wrap-around policy).  head advances so that the buffer
 *   always contains the most-recent `max_len` readings.
 *
 * @param  p_handle     Pointer to an initialised buf_handle_t.
 * @param  sensor_data  Value to store.
 *
 * @retval CIRCULAR_BUF_OK     Success (or silent overwrite on full buffer).
 * @retval CIRCULAR_BUF_ERROR  NULL pointer.
 */
int buffer_put_value(struct buf_handle_t *p_handle, float sensor_data);

/**
 * @brief  Retrieves the oldest sample from the buffer (FIFO order).
 *
 * @param  p_handle      Pointer to an initialised buf_handle_t.
 * @param  p_sensor_data Output pointer; receives the oldest stored value.
 *
 * @retval CIRCULAR_BUF_OK     Success — *p_sensor_data is valid.
 * @retval CIRCULAR_BUF_EMPTY  Buffer is empty; *p_sensor_data is unchanged.
 * @retval CIRCULAR_BUF_ERROR  NULL pointer.
 */
int buffer_get_value(struct buf_handle_t *p_handle, float *p_sensor_data);

/**
 * @brief  Returns the number of elements currently stored in the buffer.
 *
 * @param  p_handle  Pointer to an initialised buf_handle_t.
 * @retval Number of elements (0 … max_len).  Returns 0 on NULL input.
 */
uint16_t buffer_get_count(const struct buf_handle_t *p_handle);

/**
 * @brief  Copies all stored samples into a flat array (oldest first).
 *
 * @details
 *   Useful for computing statistics over the entire buffer without
 *   destructively consuming entries.  The output array must be at
 *   least buffer_get_count() elements long.
 *
 * @param  p_handle   Pointer to an initialised buf_handle_t.
 * @param  out_array  Destination array (caller-allocated).
 * @param  out_len    Number of elements written.
 *
 * @retval CIRCULAR_BUF_OK    Success.
 * @retval CIRCULAR_BUF_ERROR NULL pointer.
 */
int buffer_peek_all(const struct buf_handle_t *p_handle,
                    float                     *out_array,
                    uint16_t                  *out_len);

#endif /* CIRCULAR_BUFFER_H */
