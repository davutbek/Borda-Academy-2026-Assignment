/**
 * @file    sgp30.c
 * @brief   SGP30 eCO2 / TVOC Gas Sensor Driver Implementation
 *
 * @datasheet
 *   https://sensirion.com/media/documents/984E0DD5/61644B8B/Sensirion_Gas_Sensors_Datasheet_SGP30.pdf
 *   (Version 1.0 – May 2020 – D1)
 *
 * @details
 *   Communication sequence on real hardware (Measure_air_quality):
 *
 *   1. [Once at start-up] Send Init_air_quality:
 *        I2C_START + (0x58 << 1 | WRITE) + 0x20 + 0x03 + STOP
 *        Delay ≥ 10 ms
 *
 *   2. Every 1 second, send Measure_air_quality:
 *        I2C_START + (0x58 << 1 | WRITE) + 0x20 + 0x08 + STOP
 *        Delay ≥ 12 ms
 *
 *   3. Read 6 bytes:
 *        I2C_START + (0x58 << 1 | READ)
 *        → [eCO2_MSB][eCO2_LSB][eCO2_CRC][TVOC_MSB][TVOC_LSB][TVOC_CRC]
 *        I2C_STOP
 *
 *   eCO2 value is a direct uint16 in ppm — no scaling formula required:
 *     eCO2 (ppm) = (byte[0] << 8) | byte[1]
 *
 *   Example — 500 ppm:
 *     raw_co2 = 500 = 0x01F4
 *     byte[0] = 0x01, byte[1] = 0xF4
 *
 *   In mock mode: the HAL fills bytes [0:1] with a random value in the range
 *   400 – 1500 (indoor-typical CO2 concentration).
 */

#include "sgp30.h"
#include "i2c_hal.h"

float sgp30_read_co2(void)
{
    uint8_t  raw_bytes[SGP30_RESPONSE_BYTE_COUNT] = {0};
    int8_t   status;
    uint16_t raw_co2;
    float    co2_ppm;

    /*
     * Trigger a Measure_air_quality cycle and read the 6-byte response.
     *
     * On real hardware:
     *   Write 0x2008 → wait 12 ms → read 6 bytes
     *
     * In mock mode: i2c_hal_read_regs() uses (dev_addr=0x58) to generate
     * a simulated eCO2 value in [400, 1500] ppm.  The reg_addr (CMD_MSB)
     * is forwarded to the HAL for device-family identification.
     */
    status = i2c_hal_read_regs(SGP30_I2C_ADDR,
                                SGP30_CMD_MEASURE_MSB,
                                raw_bytes,
                                SGP30_RESPONSE_BYTE_COUNT);

    if (status != I2C_HAL_OK)
    {
        return SGP30_ERROR_VALUE; /* Sentinel: invalid reading on error */
    }

    /* ── Reconstruct 16-bit big-endian eCO2 value ────────────────────────── */
    raw_co2 = (uint16_t)(((uint16_t)raw_bytes[SGP30_CO2_MSB_IDX] << 8U) |
                           (uint16_t)raw_bytes[SGP30_CO2_LSB_IDX]);

    /* ── eCO2 raw value is already in ppm — cast directly ───────────────── */
    co2_ppm = (float)raw_co2;

    /* ── Clamp to datasheet valid range ─────────────────────────────────── */
    if (co2_ppm < SGP30_CO2_PPM_MIN) { co2_ppm = SGP30_CO2_PPM_MIN; }
    if (co2_ppm > SGP30_CO2_PPM_MAX) { co2_ppm = SGP30_CO2_PPM_MAX; }

    return co2_ppm;
}
