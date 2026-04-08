/**
 * @file    i2c_hal.c
 * @brief   I2C HAL — Mock Implementation with Varying Random Values
 *
 * @details Simulates I2C sensor reads by generating byte patterns that,
 *          when decoded by the sensor drivers, produce physically realistic
 *          values that vary randomly on each call.
 *
 *          Mock value ranges (updated for Borda Academy requirements):
 *            TMP102  (0x48): Temperature 20.0 – 30.0 °C
 *            SHTC3   (0x70): Relative Humidity 40.0 – 60.0 %RH
 *            SGP30   (0x58): eCO2  400 – 1000 ppm
 */

#include "i2c_hal.h"
#include <stdlib.h>   /* rand() */
#include <string.h>   /* memset() */

/* ─────────────────────── Device Address Constants ──────────────────────── */
#define MOCK_ADDR_TMP102   (0x48U)
#define MOCK_ADDR_SHTC3    (0x70U)
#define MOCK_ADDR_SGP30    (0x58U)

/* ──────────────── TMP102 raw register bounds (0.0625 °C/LSB) ───────────── */
/* 20.0 °C → 20.0 / 0.0625 = 320   |   30.0 °C → 30.0 / 0.0625 = 480      */
#define MOCK_TEMP_RAW_MIN  (320U)
#define MOCK_TEMP_RAW_MAX  (480U)

/* ──────────────── SHTC3 raw output bounds (RH = raw/65535*100) ─────────── */
/* 40 %  → 40 * 65535 / 100 = 26214   |   60 % → 60 * 65535 / 100 = 39321  */
#define MOCK_HUM_RAW_MIN   (26214U)
#define MOCK_HUM_RAW_MAX   (39321U)

/* ──────────────── SGP30 eCO2 bounds (unit: ppm) ────────────────────────── */
#define MOCK_CO2_RAW_MIN   (400U)
#define MOCK_CO2_RAW_MAX   (1000U)

/* ─────────────────────── Private helper ────────────────────────────────── */

/**
 * @brief  Returns a pseudo-random integer in the closed interval [min, max].
 * @note   Caller must have seeded rand() before invoking this function.
 *         Seed is set once in main() using srand(HAL_GetTick()).
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
     * IMPORTANT: srand() is NOT called here anymore.
     * Seed is set ONCE at system startup in main.c.
     * This ensures truly varying random values on each call.
     */

    /* Start with a clean buffer */
    memset(data, 0, (size_t)len);

    switch (dev_addr)
    {
        /* ── TMP102: 12-bit two's-complement temperature, MSB first ───── */
        case MOCK_ADDR_TMP102:
        {
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
            uint16_t raw_rh = (uint16_t)mock_rand_range(MOCK_HUM_RAW_MIN,
                                                         MOCK_HUM_RAW_MAX);
            if (len >= 1U) { data[0] = (uint8_t)((raw_rh >> 8U) & 0xFFU); }
            if (len >= 2U) { data[1] = (uint8_t)( raw_rh        & 0xFFU); }
            break;
        }

        /* ── SGP30: 16-bit eCO2 concentration, MSB first ────────────── */
        case MOCK_ADDR_SGP30:
        {
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
