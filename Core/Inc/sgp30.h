/**
 * @file    sgp30.h
 * @brief   Driver for Sensirion SGP30 eCO2 and TVOC Gas Sensor
 *
 * @datasheet
 *   https://sensirion.com/media/documents/984E0DD5/61644B8B/Sensirion_Gas_Sensors_Datasheet_SGP30.pdf
 *   (Version 1.0 – May 2020 – D1)
 *
 * @details
 *   The SGP30 is a digital multi-pixel gas sensor platform supporting eCO2
 *   (equivalent CO2) and TVOC (Total Volatile Organic Compounds) measurements.
 *   It communicates using a command-based I2C protocol (write command → read data).
 *
 *   I2C Address (7-bit, fixed):  0x58
 *   (The SGP30 has a single fixed address; only one per I2C bus is supported.)
 *
 *   Measure Air Quality Command:
 *     0x20, 0x08  →  triggers eCO2 + TVOC measurement
 *     Measurement duration: typ. 12 ms
 *
 *   Read response (6 bytes, big-endian):
 *     Bytes [0:1] — eCO2 (ppm,  uint16, range 400–60 000)
 *     Byte  [2]   — eCO2 CRC-8 (poly: 0x31, init: 0xFF) ← not checked in mock
 *     Bytes [3:4] — TVOC (ppb,  uint16)
 *     Byte  [5]   — TVOC CRC-8                           ← not checked in mock
 *
 *   Initialization note (datasheet §3.3):
 *     The SGP30 requires an Init_air_quality command (0x2003) before the first
 *     Measure_air_quality call.  The sensor returns a fixed eCO2 = 400 ppm /
 *     TVOC = 0 ppb for the first 15 seconds after init while the baseline is
 *     being established.  This behaviour is emulated by the mock HAL.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef SGP30_H
#define SGP30_H

#include <stdint.h>

/* ─────────────── I2C Address (fixed) ───────────────────────────────────── */
#define SGP30_I2C_ADDR              (0x58U)

/* ─────────────── Command Bytes (datasheet Table 8) ─────────────────────── */
/** Measure Air Quality: triggers eCO2 + TVOC measurement */
#define SGP30_CMD_MEASURE_MSB       (0x20U)   /**< Sent as reg_addr to HAL */
#define SGP30_CMD_MEASURE_LSB       (0x08U)

/** Init Air Quality: must be sent once before first measurement */
#define SGP30_CMD_INIT_MSB          (0x20U)
#define SGP30_CMD_INIT_LSB          (0x03U)

/* ─────────────── Response Layout ───────────────────────────────────────── */
#define SGP30_RESPONSE_BYTE_COUNT   (6U)    /**< Total bytes in one measurement result */
#define SGP30_CO2_MSB_IDX           (0U)    /**< Index of eCO2 MSB in response buffer  */
#define SGP30_CO2_LSB_IDX           (1U)    /**< Index of eCO2 LSB in response buffer  */

/* ─────────────── Physical Limits ───────────────────────────────────────── */
#define SGP30_CO2_PPM_MIN           (400.0f)    /**< Minimum valid eCO2 output (ppm) */
#define SGP30_CO2_PPM_MAX           (60000.0f)  /**< Maximum valid eCO2 output (ppm) */
#define SGP30_ERROR_VALUE           (-1.0f)     /**< Sentinel: I2C read failure       */

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Reads the eCO2 concentration from the SGP30 via the I2C HAL.
 *
 * @details
 *   Sends the Measure_air_quality command (0x2008), reads 6 bytes, and
 *   reconstructs the 16-bit eCO2 value from bytes [0] and [1] (big-endian).
 *   The raw uint16 value already represents ppm — no scaling formula is needed.
 *
 *   Byte reconstruction:
 *     raw_co2 = (byte[0] << 8) | byte[1]
 *     eCO2 (ppm) = (float)raw_co2        ← direct, no factor
 *
 *   The result is clamped to [400, 60000] ppm per the datasheet valid range.
 *
 *   CRC verification is omitted in this implementation (mock mode).  In a
 *   production build, validate each data+CRC triplet with CRC-8 polynomial 0x31.
 *
 * @retval eCO2 concentration in parts-per-million (ppm), clamped to [400, 60000].
 * @retval SGP30_ERROR_VALUE (-1.0) if the I2C read fails.
 */
float sgp30_read_co2(void);

#endif /* SGP30_H */
