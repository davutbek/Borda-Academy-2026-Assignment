/**
 * @file    sensor_manager.h
 * @brief   Sensor Manager — Unified Interface for Environmental I2C Sensors
 *
 * @details
 *   This module provides the function prototype required by the Borda Academy
 *   assignment specification and acts as a dispatcher layer between the
 *   application (main.c) and the individual sensor drivers.
 *
 *   Dispatch table:
 *     TEMP     → tmp102_read_temp()     (addr 0x48, TMP102)
 *     HUMIDITY → shtc3_read_humidity()  (addr 0x70, SHTC3)
 *     CO2      → sgp30_read_co2()       (addr 0x58, SGP30)
 *
 *   The device_address parameter allows the caller to explicitly specify the
 *   target sensor address, enabling future multi-instance support (e.g., two
 *   TMP102 sensors at 0x48 and 0x49).  In the current implementation the
 *   address is validated against the known sensor table, but the low-level
 *   drivers still use their compile-time addresses.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 */

#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <stdint.h>

/* ─────────────── Sensor Type Enumeration ───────────────────────────────── */

/**
 * @brief  Identifiers for the three environmental sensor types in this project.
 */
typedef enum
{
    TEMP     = 0U,  /**< Temperature sensor     — TMP102 @ 0x48 — unit: °C   */
    HUMIDITY = 1U,  /**< Relative humidity sensor — SHTC3 @ 0x70 — unit: %RH */
    CO2      = 2U   /**< eCO2 concentration sensor — SGP30 @ 0x58 — unit: ppm */
} sensor_t;

/** Total number of sensor types managed by this module */
#define SENSOR_MANAGER_SENSOR_COUNT  (3U)

/* ─────────────── Known Sensor Addresses ────────────────────────────────── */
#define SENSOR_ADDR_TEMP     (0x48U)  /**< TMP102 I2C address (ADD0 = GND) */
#define SENSOR_ADDR_HUMIDITY (0x70U)  /**< SHTC3  I2C address (fixed)      */
#define SENSOR_ADDR_CO2      (0x58U)  /**< SGP30  I2C address (fixed)      */

/* ─────────────── Public API ─────────────────────────────────────────────── */

/**
 * @brief  Reads a sensor value using the unified Borda Academy interface.
 *
 * @details
 *   This is the primary entry point specified in the assignment brief:
 *
 *     float i2c_sensor_read(uint8_t device_address, enum sensor_t sensor_type);
 *
 *   The function dispatches to the correct low-level driver based on
 *   sensor_type, using device_address for optional address validation.
 *
 *   Return units:
 *     TEMP     → degrees Celsius   (e.g. 24.75)
 *     HUMIDITY → percent RH        (e.g. 55.3)
 *     CO2      → ppm               (e.g. 650.0)
 *
 * @param  device_address  7-bit I2C address of the target sensor
 * @param  sensor_type     Which physical quantity to read (TEMP, HUMIDITY, CO2)
 *
 * @retval Sensor measurement in the appropriate physical unit.
 * @retval -1.0f if sensor_type is unknown or the underlying read fails.
 */
float i2c_sensor_read(uint8_t device_address, sensor_t sensor_type);

#endif /* SENSOR_MANAGER_H */
