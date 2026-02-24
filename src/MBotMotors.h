/**
 * @file MBotMotors.h
 * @brief Dual-motor driver using the MX1508 H-bridge via PCA9685 PWM channels.
 *
 * Each motor is controlled by two PCA9685 channels (forward / reverse).
 * Positive speed drives the forward channel; negative speed drives the
 * reverse channel.  Channel assignments are defined in bsp.h.
 */
#pragma once

#include <Arduino.h>

class PCA9685;

/**
 * @class MBotMotors
 * @brief High-level interface for the left and right drive motors.
 */
class MBotMotors {
public:
    /**
     * @brief Construct the motor driver.
     * @param pca Pointer to the shared PCA9685 instance.
     */
    explicit MBotMotors(PCA9685* pca);

    /**
     * @brief Drive both motors forward.
     * @param speed Speed value (0–255).
     */
    void forward(uint8_t speed);

    /**
     * @brief Drive both motors backward.
     * @param speed Speed value (0–255).
     */
    void backward(uint8_t speed);

    /**
     * @brief Pivot left (right motor forward, left motor backward).
     * @param speed Speed value (0–255).
     */
    void left(uint8_t speed);

    /**
     * @brief Pivot right (left motor forward, right motor backward).
     * @param speed Speed value (0–255).
     */
    void right(uint8_t speed);

    /** @brief Stop both motors immediately. */
    void stop();

    /**
     * @brief Set the left motor speed with direction.
     * @param speed Signed speed (−255 to +255; positive = forward).
     */
    void setLeft(int16_t speed);

    /**
     * @brief Set the right motor speed with direction.
     * @param speed Signed speed (−255 to +255; positive = forward).
     */
    void setRight(int16_t speed);

    /**
     * @brief Set both motors independently.
     * @param left  Left motor speed (−255 to +255).
     * @param right Right motor speed (−255 to +255).
     */
    void set(int16_t left, int16_t right);

private:
    PCA9685* _pca;  ///< Shared PWM controller.

    /**
     * @brief Apply a signed speed to a motor's H-bridge channels.
     * @param fwdCh PCA9685 channel for the forward direction.
     * @param revCh PCA9685 channel for the reverse direction.
     * @param speed Signed speed (−255 to +255).
     */
    void setMotor(uint8_t fwdCh, uint8_t revCh, int16_t speed);

    /**
     * @brief Convert an unsigned 0–255 speed to a 12-bit PCA9685 PWM value.
     * @param speed Unsigned speed (0–255).
     * @return 12-bit PWM value (0–4095).
     */
    uint16_t speedToPWM(uint8_t speed);
};
