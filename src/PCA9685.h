/**
 * @file PCA9685.h
 * @brief Driver for the NXP PCA9685 16-channel, 12-bit PWM controller.
 *
 * Communicates over I2C (Wire) and provides a simple API to set individual
 * channel duty cycles and change the PWM frequency at runtime.
 * Initialisation is lazy — the first call to any public method triggers
 * the I2C/register setup sequence.
 */
#pragma once

#include <Arduino.h>
#include <Wire.h>

/**
 * @class PCA9685
 * @brief Manages a single PCA9685 PWM controller on the I2C bus.
 */
class PCA9685 {
public:
    /**
     * @brief Construct a PCA9685 driver.
     * @param addr 7-bit I2C slave address (default 0x40).
     */
    explicit PCA9685(uint8_t addr = 0x40);

    /**
     * @brief Ensure the device has been initialised.
     *
     * Called automatically by the channel-access methods.  Safe to call
     * multiple times; the init sequence runs only once.
     */
    void ensureInit();

    /**
     * @brief Set the raw ON/OFF tick counts for a channel.
     * @param channel Channel index (0–15).
     * @param on      12-bit tick at which the output turns ON.
     * @param off     12-bit tick at which the output turns OFF.
     */
    void setPWM(uint8_t channel, uint16_t on, uint16_t off);

    /**
     * @brief Set a channel's duty cycle by value.
     * @param channel Channel index (0–15).
     * @param value   Duty-cycle value (0 = fully off, 4095 = fully on).
     */
    void setChannel(uint8_t channel, uint16_t value);

    /**
     * @brief Turn a channel fully off.
     * @param channel Channel index (0–15).
     */
    void setChannelOff(uint8_t channel);

    /**
     * @brief Set the PWM frequency.
     *
     * Puts the chip to sleep, writes the prescaler, and restarts.
     * All channel outputs retain their duty-cycle settings.
     *
     * @param hz Desired frequency in Hz (24–1526 per datasheet).
     */
    void setFreqHz(uint16_t hz);

    /**
     * @brief Get the current PWM frequency.
     * @return Approximate frequency in Hz, derived from the prescaler.
     */
    uint16_t freqHz() const;

    /**
     * @brief Get the current prescaler value.
     * @return Raw prescaler register value.
     */
    uint8_t prescale() const { return _prescale; }

private:
    uint8_t _addr;        ///< 7-bit I2C address.
    uint8_t _prescale;    ///< Current prescaler value.
    bool _initialized;    ///< @c true after the init sequence has run.

    /**
     * @brief Write a single byte to a PCA9685 register.
     * @param reg   Register address.
     * @param value Byte to write.
     */
    void writeReg(uint8_t reg, uint8_t value);

    /**
     * @brief Read a single byte from a PCA9685 register.
     * @param reg Register address.
     * @return The byte read.
     */
    uint8_t readReg(uint8_t reg);
};
