/**
 * @file    median_filter.h
 * @brief   Moving Median Filter for Sensor Data Smoothing
 *
 * @details
 *   A moving (sliding-window) median filter is more robust than a moving
 *   average filter because it completely rejects impulsive noise (spikes)
 *   rather than attenuating them.  This property makes it ideal for
 *   environmental sensor data, which can occasionally produce erroneous
 *   readings caused by I2C glitches or sensor self-heating transients.
 *
 *   Algorithm:
 *     1. Maintain a circular FIFO window of the N most-recent samples.
 *     2. On each call, insert the new sample and, if the window is full,
 *        overwrite the oldest sample.
 *     3. Copy the window to a temporary buffer, sort it (insertion sort —
 *        O(N²), optimal for small N), and return the middle element.
 *
 *   Multiple independent filter instances:
 *     Because this project samples 3 sensors sequentially (TEMP, then
 *     HUMIDITY, then CO2), the module maintains 3 internal filter instances
 *     indexed 0/1/2.  Each call to filter_sensor_value() automatically
 *     selects the next instance in round-robin order (0→1→2→0→…), so
 *     the call sequence in main.c must remain fixed:
 *       filter_sensor_value(temp_raw,  FILTER_WINDOW_SIZE)  → instance 0
 *       filter_sensor_value(hum_raw,   FILTER_WINDOW_SIZE)  → instance 1
 *       filter_sensor_value(co2_raw,   FILTER_WINDOW_SIZE)  → instance 2
 *       (next 1-Hz cycle: instance 0 again, and so on)
 *
 *   NOTE: For a future API that is robust to arbitrary call ordering, replace
 *   filter_sensor_value() with a version that accepts a (FilterInstance_t *)
 *   context pointer, allocating one instance per sensor in the caller.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <stdint.h>

/* ─────────────── Configuration ─────────────────────────────────────────── */

/** Maximum number of simultaneous filter streams (one per sensor type) */
#define MEDIAN_FILTER_MAX_INSTANCES  (3U)

/** Maximum allowed window size (must be an odd number for an unambiguous median) */
#define MEDIAN_FILTER_MAX_WINDOW     (15U)

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Filters a raw sensor sample using a moving median algorithm.
 *
 * @details
 *   On the first MEDIAN_FILTER_MAX_INSTANCES calls the instance index cycles
 *   from 0 to (MEDIAN_FILTER_MAX_INSTANCES − 1).  Subsequent calls repeat
 *   the cycle.  Consequently, in a 3-sensor system where sensors are always
 *   read in the same fixed order, each sensor gets its own independent window.
 *
 *   If window_size exceeds MEDIAN_FILTER_MAX_WINDOW, it is silently clamped.
 *   If window_size is even, it is promoted to (window_size + 1) to ensure
 *   an unambiguous median (middle element at index window_size / 2).
 *
 *   The window fills gradually from empty: before the window is full, the
 *   median is computed from however many samples are available so far.
 *   This avoids returning 0.0 for the first (window_size − 1) readings.
 *
 * @param  raw_sensor_value  Latest raw measurement from i2c_sensor_read()
 * @param  window_size       Number of samples in the sliding window (1–15)
 *
 * @retval Median of the window_size most-recent samples.
 */
float filter_sensor_value(float raw_sensor_value, uint8_t window_size);

/**
 * @brief  Resets all filter instances to their initial (empty) state.
 * @note   Call once during system initialisation, before the main loop.
 */
void median_filter_reset_all(void);

#endif /* MEDIAN_FILTER_H */
