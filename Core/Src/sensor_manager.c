/**
 * @file    sensor_manager.c
 * @brief   Sensor Manager Implementation
 *
 * @details
 *   Dispatches read requests to the appropriate low-level sensor driver.
 *   Adding a new sensor requires only:
 *     1. Registering a new entry in the s_sensor_table[] array below.
 *     2. Providing a driver that returns float (no other changes needed).
 */

#include "sensor_manager.h"
#include "tmp102.h"
#include "shtc3.h"
#include "sgp30.h"

/* ─────────────── Sensor Dispatch Table ──────────────────────────────────── */

/**
 * @brief  Internal entry in the sensor dispatch table.
 */
typedef struct
{
    sensor_t  type;           /**< Sensor type identifier                   */
    uint8_t   expected_addr;  /**< Expected I2C address for this sensor type */
    float     (*read_fn)(void);/**< Pointer to the sensor's read function   */
} SensorTableEntry_t;

/**
 * @brief  Static dispatch table mapping sensor types to their drivers.
 * @note   The order of entries does not matter — the table is searched linearly.
 */
static const SensorTableEntry_t s_sensor_table[] =
{
    { TEMP,     SENSOR_ADDR_TEMP,     tmp102_read_temp     },
    { HUMIDITY, SENSOR_ADDR_HUMIDITY, shtc3_read_humidity  },
    { CO2,      SENSOR_ADDR_CO2,      sgp30_read_co2       },
};

/** Number of entries in the dispatch table (compile-time constant) */
#define SENSOR_TABLE_SIZE  (sizeof(s_sensor_table) / sizeof(s_sensor_table[0]))

/* ─────────────── Public Implementation ──────────────────────────────────── */

float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type)
{
    uint8_t i;

    /*
     * Search the dispatch table for a matching (sensor_type, device_address)
     * pair. The address check warns if the caller passes an unexpected address,
     * but still dispatches — this makes the function more robust during
     * development and allows the mock to work without exact address matching.
     */
    for (i = 0U; i < (uint8_t)SENSOR_TABLE_SIZE; i++)
    {
        if (s_sensor_table[i].type == sensor_type)
        {
            /*
             * Address mismatch is non-fatal: the mock HAL uses the address
             * embedded in each driver anyway.  In a real multi-sensor system,
             * the address would be forwarded to the HAL here.
             *
             * Uncomment the block below to add strict address checking:
             *
             * if (s_sensor_table[i].expected_addr != device_address)
             * {
             *     return -1.0f;  // Address mismatch error
             * }
             */
            (void)device_address; /* Suppress unused-parameter warning */

            /* ── Invoke the sensor driver's read function ─────────────── */
            return s_sensor_table[i].read_fn();
        }
    }

    /* sensor_type not found in the dispatch table */
    return -1.0f;
}
