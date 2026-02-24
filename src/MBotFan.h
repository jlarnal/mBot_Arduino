/**
 * @file MBotFan.h
 * @brief Auxiliary DC motor driver via L9110S H-bridge on PCA9685.
 *
 * Controls a small DC motor (typically used as a fan for fire-extinguishing)
 * through an L9110S H-bridge connected to two PCA9685 PWM channels.
 *
 * Forward and reverse are defined by which channel receives the PWM signal
 * while the other is held low.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

class PCA9685;

/**
 * @class MBotFan
 * @brief Drives the auxiliary fan / DC motor via the L9110S H-bridge.
 */
class MBotFan {
public:
    /**
     * @brief Construct the fan driver.
     * @param pca Pointer to the shared PCA9685 instance.
     */
    explicit MBotFan(PCA9685* pca);

    /**
     * @brief Run the fan forward at a given speed.
     * @param speed PWM duty (0–4095, default full).
     */
    void forward(uint16_t speed = FAN_SPEED_MAX);

    /**
     * @brief Run the fan in reverse at a given speed.
     * @param speed PWM duty (0–4095, default full).
     */
    void reverse(uint16_t speed = FAN_SPEED_MAX);

    /**
     * @brief Set fan speed with direction.
     *
     * Positive values drive forward, negative values drive reverse,
     * zero stops the fan.
     *
     * @param speed Signed speed (−4095 to +4095).
     */
    void set(int16_t speed);

    /** @brief Stop the fan (both channels off). */
    void stop();

private:
    PCA9685* _pca;  ///< Shared PWM controller.
};
