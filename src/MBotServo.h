/**
 * @file MBotServo.h
 * @brief RC servo driver using PCA9685 PWM channels.
 *
 * Controls up to @ref SERVO_COUNT standard hobby servos (S1–S3) through
 * the PCA9685.  Provides angle-based and raw-microsecond positioning.
 *
 * @note The PCA9685 runs at ~1 kHz (prescale 5), giving a ~983 µs period.
 *       Pulse widths above that are clamped to full-on.  Many digital
 *       servos tolerate this frequency; analogue servos typically require
 *       50 Hz — adjust PCA9685_PRESCALE in bsp.h if needed.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

class PCA9685;

/**
 * @class MBotServo
 * @brief Positions RC servos connected to PCA9685 channels S1–S3.
 */
class MBotServo {
public:
    /**
     * @brief Construct the servo driver.
     * @param pca Pointer to the shared PCA9685 instance.
     */
    explicit MBotServo(PCA9685* pca);

    /**
     * @brief Move a servo to a given angle.
     * @param index Servo index (1–3 for S1–S3).
     * @param angle Target angle in degrees (0–@ref SERVO_ANGLE_MAX).
     */
    void setAngle(uint8_t index, uint16_t angle);

    /**
     * @brief Set a servo's pulse width directly.
     * @param index   Servo index (1–3 for S1–S3).
     * @param pulseUs Pulse width in microseconds.
     */
    void setPulse(uint8_t index, uint16_t pulseUs);

    /**
     * @brief Turn a servo channel off (no PWM output).
     * @param index Servo index (1–3 for S1–S3).
     */
    void off(uint8_t index);

    /** @brief Turn all servo channels off. */
    void allOff();

private:
    PCA9685* _pca;  ///< Shared PWM controller.

    /**
     * @brief Resolve a 1-based servo index to a PCA9685 channel.
     * @param index Servo index (1–3).
     * @return PCA9685 channel number, or 0xFF if index is out of range.
     */
    uint8_t channelFor(uint8_t index);

    /**
     * @brief Convert a pulse width in microseconds to a PCA9685 tick count.
     * @param pulseUs Pulse width in microseconds.
     * @return 12-bit tick count (0–4095).
     */
    uint16_t pulseToTicks(uint16_t pulseUs);
};
