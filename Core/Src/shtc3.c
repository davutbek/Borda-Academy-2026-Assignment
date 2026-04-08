/**
 * @file    shtc3.c
 * @brief   SHTC3 Relative Humidity Sensor Driver Implementation
 *
 * @datasheet
 *   https://sensirion.com/media/documents/643F9C8E/63A5A436/Datasheet_SHTC3.pdf
 *   (Version 4 – December 2022)
 *
 * @details
 *   Communication sequence on real hardware (Normal Power, RH-first):
 *
 *   1. [Optional] Send WAKE-UP command (0x3517) — required only if sensor was
 *      put into sleep mode.
 *   2. Send measurement command: I2C_START + (0x70 << 1 | WRITE) + 0x78 + 0x66
 *   3. Wait for measurement to complete: typ. 12 ms (see datasheet Table 5).
 *   4. Read 6 bytes: I2C_START + (0x70 << 1 | READ)
 *      → [RH_MSB][RH_LSB][RH_CRC][T_MSB][T_LSB][T_CRC]
 *
 *   RH raw-to-percent conversion (datasheet §4.1, Eq. 1):
 *     RH (%) = (S_RH / 65535) × 100
 *
 *   Example — 50 %RH:
 *     S_RH = 50 × 65535 / 100 = 32767 = 0x7FFF
 *     byte[0] = 0x7F, byte[1] = 0xFF
 *
 *   In mock mode: the HAL fills bytes [0:1] with a random value in the range
 *   that corresponds to 30 %RH – 90 %RH (raw 19660 – 58981).
 */

#include "shtc3.h"
#include "i2c_hal.h"

float shtc3_read_humidity(void)
{
    uint8_t  raw_bytes[SHTC3_RESPONSE_BYTE_COUNT] = {0};
    int8_t   status;
    uint16_t raw_rh;
    float    humidity;

    /*
     * Request a humidity measurement from the SHTC3.
     *
     * On real hardware this triggers:
     *   START + ADDR_WRITE + CMD_MSB (0x78) + CMD_LSB (0x66) + STOP
     *   ... delay 12 ms ...
     *   START + ADDR_READ  + [6 bytes] + STOP
     *
     * In mock mode: i2c_hal_read_regs() ignores the command bytes and
     * returns simulated data based on the device address (0x70).
     * The reg_addr parameter carries CMD_MSB so the HAL can identify
     * the sensor family even in simulation.
     */
    status = i2c_hal_read_regs(SHTC3_I2C_ADDR,
                                SHTC3_CMD_MEASURE_MSB,
                                raw_bytes,
                                SHTC3_RESPONSE_BYTE_COUNT);

    if (status != I2C_HAL_OK)
    {
        return SHTC3_ERROR_VALUE; /* Sentinel value: invalid humidity */
    }

    /* ── Reconstruct 16-bit big-endian RH raw value ─────────────────────── */
    raw_rh = (uint16_t)(((uint16_t)raw_bytes[SHTC3_RH_MSB_IDX] << 8U) |
                         (uint16_t)raw_bytes[SHTC3_RH_LSB_IDX]);

    /* ── Apply datasheet conversion formula (Eq. 1) ─────────────────────── */
    humidity = ((float)raw_rh / SHTC3_RH_FULL_SCALE) * SHTC3_RH_PERCENT_SCALE;

    /* ── Clamp to physical bounds (guards against floating-point rounding) ─ */
    if (humidity < SHTC3_RH_MIN_PERCENT) { humidity = SHTC3_RH_MIN_PERCENT; }
    if (humidity > SHTC3_RH_MAX_PERCENT) { humidity = SHTC3_RH_MAX_PERCENT; }

    return humidity;
}
