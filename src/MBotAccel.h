/**
 * @file MBotAccel.h
 * @brief Driver for the LSM303AGR accelerometer on the BBC micro:bit v2.
 *
 * Communicates over I2C (Wire1).  Readings are cached and refreshed at
 * most every @ref ACCEL_READ_INTERVAL_MS milliseconds to avoid excessive
 * bus traffic.
 */
#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * @class MBotAccel
 * @brief Provides 3-axis acceleration in milli-g.
 */
class MBotAccel {
public:
    MBotAccel();

    /**
     * @brief Get the X-axis acceleration.
     * @return Acceleration in milli-g.
     */
    int16_t x();

    /**
     * @brief Get the Y-axis acceleration.
     * @return Acceleration in milli-g.
     */
    int16_t y();

    /**
     * @brief Get the Z-axis acceleration.
     * @return Acceleration in milli-g.
     */
    int16_t z();

    /**
     * @brief Detect a shaking motion.
     * @param threshold Sum-of-absolute-axes threshold in milli-g (default 1500).
     * @return @c true if the current acceleration exceeds @p threshold.
     */
    bool isShaking(uint16_t threshold = 1500);

private:
    bool _initialized;       ///< @c true after I2C setup.
    int16_t _x, _y, _z;     ///< Cached axis readings (milli-g).
    unsigned long _lastRead; ///< Timestamp of the last I2C read (ms).

    /** @brief Initialise the LSM303AGR accelerometer (runs once). */
    void ensureInit();

    /** @brief Read all three axes from the sensor if the cache has expired. */
    void readAll();

    /**
     * @brief Write a single byte to an LSM303AGR register.
     * @param reg   Register address.
     * @param value Byte to write.
     */
    void writeReg(uint8_t reg, uint8_t value);

    /**
     * @brief Read a single byte from an LSM303AGR register.
     * @param reg Register address.
     * @return The byte read.
     */
    uint8_t readReg(uint8_t reg);

    /**
     * @brief Read multiple consecutive registers.
     * @param reg Starting register address.
     * @param buf Destination buffer.
     * @param len Number of bytes to read.
     */
    void readRegs(uint8_t reg, uint8_t* buf, uint8_t len);
};
