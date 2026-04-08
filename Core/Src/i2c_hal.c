/**
 * @file    i2c_hal.c
 * @brief   I2C HAL — Mock Implementation
 *
 * @details Simulates I2C sensor reads by generating byte patterns that,
 *          when decoded by the sensor drivers, produce physically realistic
 *          values. The seed for rand() is mixed from dev_addr and reg_addr
 *          so each sensor produces a distinct, repeatable stream.
 *
 *          Mock value ranges:
 *            TMP102  (0x48): Temperature 20.0 – 35.0 °C
 *            SHTC3   (0x70): Relative Humidity 30.0 – 90.0 %RH
 *            SGP30   (0x58): eCO2  400 – 1500 ppm
 */

#include "i2c_hal.h"
#include <stdlib.h>   /* rand(), srand() */
#include <string.h>   /* memset()        */

/* ─────────────────────── Device Address Constants ──────────────────────── */
#define MOCK_ADDR_TMP102   (0x48U)
#define MOCK_ADDR_SHTC3    (0x70U)
#define MOCK_ADDR_SGP30    (0x58U)

/* ──────────────── TMP102 raw register bounds (0.0625 °C/LSB) ───────────── */
/* 20.0 °C → 20.0 / 0.0625 = 320   |   35.0 °C → 35.0 / 0.0625 = 560      */
#define MOCK_TEMP_RAW_MIN  (320U)
#define MOCK_TEMP_RAW_MAX  (560U)

/* ──────────────── SHTC3 raw output bounds (RH = raw/65535*100) ─────────── */
/* 30 %  → 30 * 65535 / 100 = 19660   |   90 % → 90 * 65535 / 100 = 58981  */
#define MOCK_HUM_RAW_MIN   (19660U)
#define MOCK_HUM_RAW_MAX   (58981U)

/* ──────────────── SGP30 eCO2 bounds (unit: ppm) ────────────────────────── */
#define MOCK_CO2_RAW_MIN   (400U)
#define MOCK_CO2_RAW_MAX   (1500U)

/* ─────────────────────── Private helper ────────────────────────────────── */

/**
 * @brief  Returns a pseudo-random integer in the closed interval [min, max].
 * @note   Caller must have seeded rand() before invoking this function.
 */
static uint32_t mock_rand_range(uint32_t min_val, uint32_t max_val)
{
    return min_val + (uint32_t)((uint32_t)rand() % (max_val - min_val + 1U));
}

/* ─────────────────────── Public implementation ──────────────────────────── */

int8_t i2c_hal_read_regs(uint8_t  dev_addr,
                          uint8_t  reg_addr,
                          uint8_t *data,
                          uint16_t len)
{
    if ((data == NULL) || (len == 0U))
    {
        return I2C_HAL_ERROR;
    }

    /*
     * Seed the PRNG with a value that is unique to this (device, register) pair.
     * Multiplying dev_addr by a small prime (31) before adding reg_addr reduces
     * the chance that two different pairs produce the same seed.
     *
     * NOTE: On real hardware, remove this entire mock body and replace with:
     *   return (HAL_I2C_Mem_Read(&hi2c1,
     *                            (uint16_t)(dev_addr << 1),
     *                            reg_addr,
     *                            I2C_MEMADD_SIZE_8BIT,
     *                            data, len, HAL_MAX_DELAY) == HAL_OK)
     *          ? I2C_HAL_OK : I2C_HAL_ERROR;
     */
    srand((unsigned int)((uint32_t)dev_addr * 31U + (uint32_t)reg_addr));

    /* Start with a clean buffer */
    memset(data, 0, (size_t)len);

    switch (dev_addr)
    {
        /* ── TMP102: 12-bit two's-complement temperature, MSB first ───── */
        case MOCK_ADDR_TMP102:
        {
            /*
             * TMP102 Temperature Register layout (datasheet §7.5.1):
             *
             *   Byte 0 (MSB): [D11 D10 D9 D8 D7 D6 D5 D4]
             *   Byte 1 (LSB): [D3  D2  D1 D0  0  0  0  0]  (EM=0, normal 12-bit mode)
             *
             * The 12-bit value sits in bits [15:4] of the 16-bit word.
             * Temperature (°C) = raw_12bit × 0.0625
             *
             * Example: 25.0 °C → raw_12bit = 400 = 0x190
             *   reg_val = 0x190 << 4 = 0x1900
             *   byte[0] = 0x19, byte[1] = 0x00
             */
            uint16_t raw_12bit = (uint16_t)mock_rand_range(MOCK_TEMP_RAW_MIN,
                                                            MOCK_TEMP_RAW_MAX);
            uint16_t reg_val   = (uint16_t)(raw_12bit << 4U);

            if (len >= 1U) { data[0] = (uint8_t)((reg_val >> 8U) & 0xFFU); }
            if (len >= 2U) { data[1] = (uint8_t)( reg_val        & 0xFFU); }
            break;
        }

        /* ── SHTC3: 16-bit relative humidity, MSB first ─────────────── */
        case MOCK_ADDR_SHTC3:
        {
            /*
             * SHTC3 measurement output (datasheet §4.1, RH-first mode):
             *
             *   Bytes [0:1] → RH (16-bit unsigned big-endian)
             *   Byte  [2]   → RH CRC-8  (skipped in mock)
             *   Bytes [3:4] → Temperature (16-bit big-endian)
             *   Byte  [5]   → Temp CRC-8 (skipped in mock)
             *
             * RH (%) = (raw_16bit / 65535.0) × 100.0
             */
            uint16_t raw_rh = (uint16_t)mock_rand_range(MOCK_HUM_RAW_MIN,
                                                         MOCK_HUM_RAW_MAX);
            if (len >= 1U) { data[0] = (uint8_t)((raw_rh >> 8U) & 0xFFU); }
            if (len >= 2U) { data[1] = (uint8_t)( raw_rh        & 0xFFU); }
            /* Remaining bytes (CRC, temperature) left as 0 — not used by driver */
            break;
        }

        /* ── SGP30: 16-bit eCO2 concentration, MSB first ────────────── */
        case MOCK_ADDR_SGP30:
        {
            /*
             * SGP30 Measure Air Quality response (datasheet §6.3):
             *
             *   Bytes [0:1] → eCO2 (ppm, big-endian uint16)
             *   Byte  [2]   → eCO2 CRC-8  (skipped in mock)
             *   Bytes [3:4] → TVOC (ppb, big-endian uint16)
             *   Byte  [5]   → TVOC CRC-8  (skipped in mock)
             *
             * Valid eCO2 range: 400 – 60 000 ppm.  Indoor typical: 400 – 1500 ppm.
             */
            uint16_t raw_co2 = (uint16_t)mock_rand_range(MOCK_CO2_RAW_MIN,
                                                          MOCK_CO2_RAW_MAX);
            if (len >= 1U) { data[0] = (uint8_t)((raw_co2 >> 8U) & 0xFFU); }
            if (len >= 2U) { data[1] = (uint8_t)( raw_co2        & 0xFFU); }
            break;
        }

        /* ── Unknown device: fill with generic random bytes ──────────── */
        default:
        {
            for (uint16_t i = 0U; i < len; i++)
            {
                data[i] = (uint8_t)((uint32_t)rand() & 0xFFU);
            }
            break;
        }
    }

    return I2C_HAL_OK;
}
