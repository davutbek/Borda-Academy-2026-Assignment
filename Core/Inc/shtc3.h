/**
 * @file    shtc3.h
 * @brief   Driver for Sensirion SHTC3 Relative Humidity and Temperature Sensor
 *
 * @datasheet
 *   https://sensirion.com/media/documents/643F9C8E/63A5A436/Datasheet_SHTC3.pdf
 *   (Version 4 – December 2022)
 *
 * @details
 *   The SHTC3 is a digital humidity and temperature sensor with a single, fixed
 *   I2C address. It communicates using a command-based protocol: the host writes
 *   a 2-byte command to trigger a measurement, waits for the measurement to
 *   complete, then reads the result.
 *
 *   I2C Address (7-bit, fixed):  0x70
 *   (The SHTC3 has no address-select pin — only one SHTC3 per bus is supported.)
 *
 *   Measurement Command (Normal Power, RH-first, no clock-stretching):
 *     0x78, 0x66  →  defined as SHTC3_CMD_MEASURE_MSB / LSB
 *     Measurement time: typ. 12 ms, max. 13 ms
 *
 *   Read response (6 bytes, big-endian):
 *     Bytes [0:1] — RH  raw value (uint16)
 *     Byte  [2]   — RH  CRC-8 (poly: 0x31, init: 0xFF)  ← not checked in mock mode
 *     Bytes [3:4] — Temp raw value (uint16)
 *     Byte  [5]   — Temp CRC-8                           ← not checked in mock mode
 *
 *   RH Conversion:
 *     RH (%) = (raw_rh / 65535.0) × 100.0
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef SHTC3_H
#define SHTC3_H

#include <stdint.h>

/* ─────────────── I2C Address (fixed — no ADD pin) ──────────────────────── */
#define SHTC3_I2C_ADDR              (0x70U)

/* ─────────────── Measurement Commands (datasheet Table 4) ──────────────── */
/** Normal power, RH first, no clock stretching — 2-byte command split for HAL */
#define SHTC3_CMD_MEASURE_MSB       (0x78U)   /**< Command byte 0 (sent as reg_addr) */
#define SHTC3_CMD_MEASURE_LSB       (0x66U)   /**< Command byte 1 */

/** Wake-up command (required before measurement after sleep) */
#define SHTC3_CMD_WAKEUP_MSB        (0x35U)
#define SHTC3_CMD_WAKEUP_LSB        (0x17U)

/** Sleep command (optional power saving) */
#define SHTC3_CMD_SLEEP_MSB         (0xB0U)
#define SHTC3_CMD_SLEEP_LSB         (0x98U)

/* ─────────────── Response Layout ───────────────────────────────────────── */
#define SHTC3_RESPONSE_BYTE_COUNT   (6U)      /**< Total bytes in one measurement result */
#define SHTC3_RH_MSB_IDX            (0U)      /**< Index of RH MSB in response buffer    */
#define SHTC3_RH_LSB_IDX            (1U)      /**< Index of RH LSB in response buffer    */

/* ─────────────── Conversion Constants ──────────────────────────────────── */
#define SHTC3_RH_FULL_SCALE         (65535.0f) /**< Maximum raw output value (2^16 − 1) */
#define SHTC3_RH_PERCENT_SCALE      (100.0f)   /**< Percent scaling factor              */
#define SHTC3_RH_MIN_PERCENT        (0.0f)     /**< Physical minimum (clamped)          */
#define SHTC3_RH_MAX_PERCENT        (100.0f)   /**< Physical maximum (clamped)          */
#define SHTC3_ERROR_VALUE           (-1.0f)    /**< Sentinel: invalid RH on I2C error   */

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Reads the relative humidity from the SHTC3 via the I2C HAL.
 *
 * @details
 *   Reads SHTC3_RESPONSE_BYTE_COUNT (6) bytes using the I2C HAL, then
 *   reconstructs the 16-bit RH value from bytes [0] and [1] (big-endian),
 *   and converts to percent using the datasheet formula:
 *
 *     RH (%) = (raw_rh / 65535.0) × 100.0
 *
 *   The result is clamped to [0.0, 100.0] to guard against any rounding
 *   artefacts near the extremes of the sensor's range.
 *
 *   CRC verification is omitted in this implementation because the mock HAL
 *   does not produce valid CRC bytes.  In a production build, insert a CRC-8
 *   check (polynomial 0x31, init value 0xFF) for each data+CRC triplet.
 *
 * @retval Relative humidity in percent (%RH), clamped to [0.0, 100.0].
 * @retval SHTC3_ERROR_VALUE (-1.0) if the I2C read fails.
 */
float shtc3_read_humidity(void);

#endif /* SHTC3_H */
