/**
 * @file    i2c_hal.h
 * @brief   I2C Hardware Abstraction Layer — Mock Implementation for STM32F4DISCO
 *
 * @details This module decouples sensor drivers from the physical I2C peripheral.
 *          Because the STM32F4DISCO board has no environmental sensors soldered
 *          on-board, this HAL simulates I2C bus transactions in software.
 *          The mock generates deterministic pseudo-random bytes whose values,
 *          when decoded by each sensor driver, produce physically plausible
 *          readings (temperature 20-35 °C, humidity 30-90 %RH, CO2 400-1500 ppm).
 *
 *          To port to real hardware: replace i2c_hal_read_regs() body with
 *          HAL_I2C_Mem_Read() (STM32 HAL) or a bare-metal I2C state-machine.
 *          The rest of the codebase remains unchanged.
 *
 * @board   STM32F4DISCO (STM32F407VGT6)
 * @author  Borda Academy 2026 — Embedded Systems Developer Assignment
 * @date    2026-04-08
 */

#ifndef I2C_HAL_H
#define I2C_HAL_H

#include <stdint.h>

/* ──────────────────────────── Return Codes ─────────────────────────────── */
#define I2C_HAL_OK     ( 0)   /**< Operation completed successfully */
#define I2C_HAL_ERROR  (-1)   /**< Operation failed (NULL ptr, bad args, bus error) */

/* ─────────────────────────── Public API ────────────────────────────────── */

/**
 * @brief  Reads registers from an I2C device (Mock / Simulated implementation).
 *
 * @details In a real implementation this function would:
 *            1. Send a START condition on the I2C bus.
 *            2. Transmit (dev_addr << 1 | WRITE) + reg_addr.
 *            3. Send a REPEATED START.
 *            4. Transmit (dev_addr << 1 | READ).
 *            5. Clock in `len` bytes into `data[]`, sending NACK on the last byte.
 *            6. Send a STOP condition.
 *
 *          In this mock implementation:
 *            - The function uses rand() seeded with (dev_addr * 31 + reg_addr)
 *              to fill `data[]` with bytes that decode to plausible sensor values.
 *            - Each device address produces a distinct data pattern so that
 *              multiple sensors can coexist without interfering.
 *
 * @param  dev_addr   7-bit I2C device address (right-aligned, R/W bit NOT included)
 * @param  reg_addr   Register (or command MSB) address to read from
 * @param  data       Pointer to caller-allocated buffer; must hold at least `len` bytes
 * @param  len        Number of bytes to read (must be > 0)
 *
 * @retval I2C_HAL_OK    ( 0)  Success — `data[]` contains valid bytes
 * @retval I2C_HAL_ERROR (-1)  Failure — `data` is NULL or `len` is 0
 */
int8_t i2c_hal_read_regs(uint8_t dev_addr,
                          uint8_t reg_addr,
                          uint8_t *data,
                          uint16_t len);

#endif /* I2C_HAL_H */
