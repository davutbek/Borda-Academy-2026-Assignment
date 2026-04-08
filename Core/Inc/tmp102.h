/**
 * @file    tmp102.h
 * @brief   Driver for Texas Instruments TMP102 Digital Temperature Sensor
 *
 * @datasheet
 *   https://www.ti.com/lit/ds/symlink/tmp102.pdf?ts=1775630739272&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMP102%252Fpart-details%252FTMP102AIDRLR
 *   (SBOS397I – AUGUST 2007 – REVISED JUNE 2024)
 *
 * @details
 *   The TMP102 is a 12-bit (or 13-bit in extended mode) I2C temperature sensor
 *   with ±0.5 °C accuracy over 0 °C to 65 °C and ±1 °C over –40 °C to 125 °C.
 *
 *   I2C Address (7-bit, ADD0 tied to GND):  0x48
 *   Address Options:
 *     ADD0 → GND  : 0x48  (selected in this project)
 *     ADD0 → V+   : 0x49
 *     ADD0 → SDA  : 0x4A
 *     ADD0 → SCL  : 0x4B
 *
 *   Temperature Register (0x00):
 *     Byte 0 (MSB): D[11:4]
 *     Byte 1 (LSB): D[3:0] in bits [7:4]; bits [3:0] are flag/alert bits
 *     Resolution : 0.0625 °C / LSB   (12-bit normal mode)
 *     Negative values are in 12-bit two's complement format.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef TMP102_H
#define TMP102_H

#include <stdint.h>

/* ─────────────── I2C Address (ADD0 = GND) ──────────────────────────────── */
#define TMP102_I2C_ADDR            (0x48U)

/* ─────────────── Register Addresses (datasheet Table 2) ────────────────── */
#define TMP102_REG_TEMPERATURE     (0x00U)   /**< Temperature result (read-only) */
#define TMP102_REG_CONFIG          (0x01U)   /**< Configuration register         */
#define TMP102_REG_T_LOW           (0x02U)   /**< T_LOW alert threshold          */
#define TMP102_REG_T_HIGH          (0x03U)   /**< T_HIGH alert threshold         */

/* ─────────────── Conversion Constants ──────────────────────────────────── */
#define TMP102_RESOLUTION_DEG_C    (0.0625f) /**< °C per LSB in 12-bit mode      */
#define TMP102_REG_BYTE_COUNT      (2U)      /**< Bytes in the temperature register */
#define TMP102_ERROR_VALUE_DEG_C   (-273.15f)/**< Sentinel: absolute zero = error */

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Reads the current temperature from the TMP102 via the I2C HAL.
 *
 * @details
 *   Reads 2 bytes from register 0x00, reconstructs the 16-bit word, arithmetic-
 *   right-shifts by 4 to obtain the signed 12-bit value, then multiplies by
 *   0.0625 to convert to degrees Celsius.
 *
 *   Conversion steps (normal 12-bit mode):
 *     1. raw_16bit = (byte[0] << 8) | byte[1]
 *     2. raw_12bit = raw_16bit >> 4   (arithmetic shift — preserves sign)
 *     3. temperature_C = raw_12bit * 0.0625
 *
 * @retval Temperature in degrees Celsius (positive or negative).
 * @retval TMP102_ERROR_VALUE_DEG_C (-273.15) if the I2C read fails.
 */
float tmp102_read_temp(void);

#endif /* TMP102_H */
