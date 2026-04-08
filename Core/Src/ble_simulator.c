/**
 * @file    ble_simulator.c
 * @brief   BLE Packet Simulator — Implementation
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#include "ble_simulator.h"
#include "circular_buffer.h"
#include <stdio.h>    /* printf()   */
#include <stddef.h>   /* NULL       */
#include <math.h>     /* sqrtf()    */
#include <string.h>   /* memset()   */

/* ─────────────── Private helpers ───────────────────────────────────────── */

/**
 * @brief  Insertion-sort a float array in ascending order (in place).
 */
static void sort_ascending(float *arr, uint16_t len)
{
    uint16_t i;
    uint16_t j;
    float    key;

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

int ble_compute_statistics(struct buf_handle_t *p_buf,
                           sensor_t             sensor,
                           ble_packet_t        *p_packet)
{
    /*
     * Statically sized scratch buffer.
     * Sized for a 30-second window at 1 Hz = 30 samples.
     * Increase BLE_SCRATCH_BUF_SIZE if max_len grows beyond 64.
     */
#define BLE_SCRATCH_BUF_SIZE  (64U)
    float    scratch[BLE_SCRATCH_BUF_SIZE];
    uint16_t n = 0U;
    uint16_t i;
    float    sum;
    float    mean;
    float    variance;
    float    diff;

    if ((p_buf == NULL) || (p_packet == NULL))
    {
        return -1;
    }

    memset(p_packet, 0, sizeof(ble_packet_t));
    p_packet->sensor_type = sensor;

    /* ── Snapshot buffer contents non-destructively ───────────────────── */
    if (buffer_peek_all(p_buf, scratch, &n) != CIRCULAR_BUF_OK)
    {
        return -1;
    }

    /*
     * Safety clamp: guard against scratch[] overflow if the caller ever
     * passes a buffer whose max_len exceeds BLE_SCRATCH_BUF_SIZE.
     * buffer_peek_all() writes exactly n elements; clamping n here ensures
     * subsequent loops and the sort stay within the allocated scratch size.
     */
    if (n > BLE_SCRATCH_BUF_SIZE)
    {
        n = BLE_SCRATCH_BUF_SIZE;
    }

    p_packet->sample_count = n;

    if (n == 0U)
    {
        /* Nothing to compute — leave all stats at 0.0f */
        return 0;
    }

    /* ── Min and Max ──────────────────────────────────────────────────── */
    float v_min = scratch[0];
    float v_max = scratch[0];
    sum = 0.0f;

    for (i = 0U; i < n; i++)
    {
        if (scratch[i] < v_min) { v_min = scratch[i]; }
        if (scratch[i] > v_max) { v_max = scratch[i]; }
        sum += scratch[i];
    }

    p_packet->min_value = v_min;
    p_packet->max_value = v_max;

    /* ── Mean ─────────────────────────────────────────────────────────── */
    mean = sum / (float)n;

    /* ── Population Standard Deviation ───────────────────────────────── */
    variance = 0.0f;
    for (i = 0U; i < n; i++)
    {
        diff      = scratch[i] - mean;
        variance += diff * diff;
    }
    variance            /= (float)n;
    p_packet->std_dev    = sqrtf(variance);

    /* ── Median (sort a copy — scratch is already a copy of the buffer) ─ */
    sort_ascending(scratch, n);
    p_packet->median_value = scratch[n / 2U];

    return 0;
}

void send_ble_packet_uart(ble_packet_t *p_packet, sensor_t sensor_type)
{
    const char *label;

    if (p_packet == NULL)
    {
        return;
    }

    /* ── Map sensor type to a human-readable label ───────────────────── */
    switch (sensor_type)
    {
        case TEMP:     label = "TEMP    "; break;
        case HUMIDITY: label = "HUMIDITY"; break;
        case CO2:      label = "CO2     "; break;
        default:       label = "UNKNOWN "; break;
    }

    /*
     * Single-line output — easy to parse with a Python script or grep.
     * Example:
     *   [BLE][TEMP    ] Min=20.06 Max=34.94 Med=27.50 StdDev= 4.23 N=30
     */
    printf("[BLE][%s] Min=%7.2f Max=%7.2f Med=%7.2f StdDev=%6.2f N=%u\r\n",
           label,
           (double)p_packet->min_value,
           (double)p_packet->max_value,
           (double)p_packet->median_value,
           (double)p_packet->std_dev,
           (unsigned int)p_packet->sample_count);
}
