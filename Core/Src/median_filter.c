/**
 * @file    median_filter.c
 * @brief   Moving Median Filter — Implementation
 *
 * @details
 *   Maintains MEDIAN_FILTER_MAX_INSTANCES (3) independent filter instances
 *   in a statically-allocated structure.  Each call to filter_sensor_value()
 *   automatically advances to the next instance in round-robin order
 *   (0 → 1 → 2 → 0 → …), so sensors must always be read in the same fixed
 *   order within each 1-Hz cycle.
 *
 *   Sorting algorithm: insertion sort — O(N²), optimal for N ≤ 15.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#include "median_filter.h"
#include <string.h>  /* memcpy() */

/* ─────────────── Private types ─────────────────────────────────────────── */

/**
 * @brief  Internal state for a single median filter stream.
 */
typedef struct
{
    float    window[MEDIAN_FILTER_MAX_WINDOW]; /**< Circular sample buffer      */
    uint8_t  head;                             /**< Index of oldest sample       */
    uint8_t  count;                            /**< Samples stored (0..window_sz)*/
    uint8_t  window_size;                      /**< Active window length (odd)   */
} FilterInstance_t;

/* ─────────────── Private state ─────────────────────────────────────────── */

/** All filter instances, zero-initialised at startup */
static FilterInstance_t s_filters[MEDIAN_FILTER_MAX_INSTANCES];

/** Round-robin index: which instance handles the next call */
static uint8_t s_current_instance = 0U;

/* ─────────────── Private helpers ───────────────────────────────────────── */

/**
 * @brief  Insertion-sort a copy of the window buffer in ascending order.
 *
 * @param  arr   Array to sort (modified in place).
 * @param  len   Number of elements.
 */
static void insertion_sort(float *arr, uint8_t len)
{
    uint8_t i;
    uint8_t j;
    float   key;

    for (i = 1U; i < len; i++)
    {
        key = arr[i];
        j   = i;
        while ((j > 0U) && (arr[j - 1U] > key))
        {
            arr[j] = arr[j - 1U];
            j--;
        }
        arr[j] = key;
    }
}

/* ─────────────── Public API ─────────────────────────────────────────────── */

void median_filter_reset_all(void)
{
    memset(s_filters, 0, sizeof(s_filters));
    s_current_instance = 0U;
}

float filter_sensor_value(float raw_sensor_value, uint8_t window_size)
{
    FilterInstance_t *inst;
    float             sorted_buf[MEDIAN_FILTER_MAX_WINDOW];
    uint8_t           effective_window;
    uint8_t           insert_idx;
    uint8_t           i;
    float             median;

    /* ── Select and advance the round-robin instance ─────────────────────── */
    inst               = &s_filters[s_current_instance];
    s_current_instance = (uint8_t)((s_current_instance + 1U) %
                                   MEDIAN_FILTER_MAX_INSTANCES);

    /* ── Clamp window_size to allowed range ──────────────────────────────── */
    if (window_size == 0U)
    {
        window_size = 1U;
    }
    if (window_size > MEDIAN_FILTER_MAX_WINDOW)
    {
        window_size = MEDIAN_FILTER_MAX_WINDOW;
    }

    /* ── Promote even window_size to the next odd value ─────────────────── */
    if ((window_size % 2U) == 0U)
    {
        window_size = (uint8_t)(window_size + 1U);
        /* Re-clamp after promotion */
        if (window_size > MEDIAN_FILTER_MAX_WINDOW)
        {
            window_size = MEDIAN_FILTER_MAX_WINDOW; /* already odd (15) */
        }
    }

    /* ── Store active window size (may change on first call only) ─────────── */
    inst->window_size = window_size;

    /* ── Insert new sample into circular buffer ──────────────────────────── */
    if (inst->count < window_size)
    {
        /* Window not yet full: append at tail position */
        insert_idx = inst->count;
        inst->count++;
    }
    else
    {
        /* Window full: overwrite oldest sample (head) and advance head */
        insert_idx = inst->head;
        inst->head = (uint8_t)((inst->head + 1U) % window_size);
    }
    inst->window[insert_idx] = raw_sensor_value;

    /* ── Copy window to temporary buffer for sorting (preserves original) ── */
    effective_window = inst->count;          /* could be < window_size initially */
    for (i = 0U; i < effective_window; i++)
    {
        sorted_buf[i] = inst->window[i];
    }

    /* ── Sort and pick the middle element ───────────────────────────────── */
    insertion_sort(sorted_buf, effective_window);
    median = sorted_buf[effective_window / 2U];

    return median;
}
