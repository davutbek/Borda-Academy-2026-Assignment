/**
 * @file    tmp102.c
 * @brief   TMP102 Temperature Sensor Driver Implementation
 *
 * @datasheet
 *   https://www.ti.com/lit/ds/symlink/tmp102.pdf?ts=1775630739272&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FTMP102%252Fpart-details%252FTMP102AIDRLR
 *   (SBOS397I – AUGUST 2007 – REVISED JUNE 2024)
 *
 * @details
 *   The TMP102 temperature register (Pointer = 0x00) contains a 16-bit
 *   two's-complement value in MSB-first order.  In the default 12-bit mode
 *   the significant data occupies bits [15:4]; bits [3:0] of the LSB byte
 *   contain alert-mode flags and must be discarded.
 *
 *   Register layout (normal 12-bit mode):
 *
 *     Byte 0 (MSB): [D11][D10][D9][D8][D7][D6][D5][D4]
 *     Byte 1 (LSB): [D3 ][D2 ][D1][D0][ 0][ 0][ 0][ 0]
 *                                         ↑ flag / alert bits (ignored)
 *
 *   Decoding:
 *     raw_16bit = (MSB << 8) | LSB
 *     raw_12bit = (int16_t)raw_16bit >> 4   ← arithmetic shift (sign-extends)
 *     temperature_C = raw_12bit * 0.0625
 *
 *   Example — 25.0 °C:
 *     raw_12bit = 25.0 / 0.0625 = 400 = 0x190
 *     raw_16bit = 0x190 << 4   = 0x1900
 *     byte[0] = 0x19, byte[1] = 0x00
 *
 *   Example — −10.0 °C (negative, two's complement):
 *     raw_12bit = −10.0 / 0.0625 = −160 = 0xF60 (12-bit) → 0xFF60 (16-bit after shift)
 *     byte[0] = 0xFF, byte[1] = 0x60
 */

#include "tmp102.h"
#include "i2c_hal.h"

float tmp102_read_temp(void)
{
    uint8_t raw_bytes[TMP102_REG_BYTE_COUNT] = {0U, 0U};
    int8_t  status;
    int16_t raw_16bit;
    int16_t raw_12bit;
    float   temperature;

    /* ── Step 1: Read 2 bytes from the Temperature Register (0x00) ─────── */
    status = i2c_hal_read_regs(TMP102_I2C_ADDR,
                                TMP102_REG_TEMPERATURE,
                                raw_bytes,
                                TMP102_REG_BYTE_COUNT);

    if (status != I2C_HAL_OK)
    {
        return TMP102_ERROR_VALUE_DEG_C; /* Sentinel: absolute zero on error */
    }

    /* ── Step 2: Reconstruct 16-bit big-endian word ─────────────────────── */
    raw_16bit = (int16_t)(((uint16_t)raw_bytes[0] << 8U) |
                           (uint16_t)raw_bytes[1]);

    /* ── Step 3: Arithmetic right-shift by 4 to isolate the 12-bit value ── */
    /*
     * C99 specifies that right-shifting a negative signed integer is
     * implementation-defined, but all ARM Cortex-M compilers (GCC, ARMCC,
     * IAR) implement arithmetic right-shift (sign-extending), which is the
     * correct behaviour for two's-complement decoding.
     */
    raw_12bit = raw_16bit >> 4;

    /* ── Step 4: Scale to degrees Celsius ──────────────────────────────── */
    temperature = (float)raw_12bit * TMP102_RESOLUTION_DEG_C;

    return temperature;
}
