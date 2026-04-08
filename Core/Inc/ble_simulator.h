/**
 * @file    ble_simulator.h
 * @brief   BLE Packet Simulator — Statistics Engine and UART Transmitter
 *
 * @details
 *   This module simulates a BLE (Bluetooth Low Energy) advertisement or
 *   notification payload by:
 *     1. Reading all samples currently stored in a circular buffer.
 *     2. Computing Min, Max, Median, and Standard Deviation over those samples.
 *     3. Packing the results into a ble_packet_t structure.
 *     4. Formatting and transmitting the packet over UART2 (115200 8N1) so
 *        that the output is visible on a serial terminal — emulating what a
 *        real BLE peripheral would broadcast.
 *
 *   Output format on UART (one line per packet):
 *     [BLE][TEMP   ] Min=20.06 Max=34.94 Med=27.50 StdDev=4.23 (N=30)
 *     [BLE][HUMIDITY] Min=30.05 Max=89.97 Med=60.11 StdDev=8.71 (N=30)
 *     [BLE][CO2    ] Min=402.0 Max=1498.0 Med=950.0 StdDev=159.2 (N=30)
 *
 *   To port to real BLE hardware: replace send_ble_packet_uart() with a
 *   function that serialises ble_packet_t into a GATT characteristic value
 *   and triggers a BLE notification.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef BLE_SIMULATOR_H
#define BLE_SIMULATOR_H

#include <stdint.h>
#include "sensor_manager.h"   /* sensor_t enum                       */
#include "circular_buffer.h"  /* buf_handle_t                        */

/* ─────────────── BLE Packet Payload ────────────────────────────────────── */

/**
 * @brief  Statistics packet transmitted (simulated) over BLE / UART.
 *
 * @note   All floating-point fields are in the native physical unit of the
 *         originating sensor (°C, %RH, or ppm).
 */
typedef struct
{
    sensor_t sensor_type;   /**< Which sensor produced this packet           */
    float    min_value;     /**< Minimum value over the sample window        */
    float    max_value;     /**< Maximum value over the sample window        */
    float    median_value;  /**< Median value over the sample window         */
    float    std_dev;       /**< Population standard deviation               */
    uint16_t sample_count;  /**< Number of samples used in computation       */
} ble_packet_t;

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Computes statistics from a circular buffer and fills a BLE packet.
 *
 * @details
 *   Reads up to buf_handle_t::max_len samples from the buffer using
 *   buffer_peek_all() (non-destructive), then computes:
 *     - Min  : smallest element
 *     - Max  : largest element
 *     - Median: middle element of a sorted copy (insertion sort)
 *     - StdDev: population standard deviation σ = sqrt(Σ(xi−μ)² / N)
 *
 *   If the buffer is empty all statistics are set to 0.0f.
 *
 * @param  p_buf      Pointer to an initialised, populated circular buffer.
 * @param  sensor     Sensor type tag stored in the resulting packet.
 * @param  p_packet   Output packet (caller-allocated); filled by this function.
 *
 * @retval  0   Success.
 * @retval -1   NULL pointer argument.
 */
int ble_compute_statistics(struct buf_handle_t *p_buf,
                           sensor_t             sensor,
                           ble_packet_t        *p_packet);

/**
 * @brief  Formats a BLE packet as a human-readable string and sends it via UART.
 *
 * @details
 *   Uses printf() (redirected to UART2 via the _write syscall) to emit one
 *   formatted line per call.  Example output:
 *     [BLE][TEMP   ] Min=20.06 Max=34.94 Med=27.50 StdDev= 4.23 N=30
 *
 * @param  p_packet    BLE packet produced by ble_compute_statistics().
 * @param  sensor_type Sensor type; used to choose the label string.
 *                     (Redundant with p_packet->sensor_type; kept for API
 *                     symmetry with the assignment specification.)
 */
void send_ble_packet_uart(ble_packet_t *p_packet, sensor_t sensor_type);

#endif /* BLE_SIMULATOR_H */
