/**
 * @file MBotServo.h
 * @brief RC servo driver for the sonar pan servo (S1) via PCA9685.
 *
 * Controls the RC servo on PCA9685 channel @ref SERVO_CH_S1.
 * Provides angle-based and raw-microsecond positioning.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

class PCA9685;

/**
 * @class MBotServo
 * @brief Positions the RC servo connected to PCA9685 channel S1.
 */
class MBotServo {
public:
    /**
     * @brief Construct the servo driver.
     * @param pca Pointer to the shared PCA9685 instance.
     */
    explicit MBotServo(PCA9685* pca);

    /**
     * @brief Set the servo's pulse width directly.
     * @param pulseUs Pulse width in microseconds.
     */
    void setPulse_us(uint16_t pulseUs);

    
    /** @brief Return the current servo pulse width in microseconds. */
    uint16_t getPulse_us();


    void setAngle(int8_t angle=0);

    /** @brief Read back the current servo angle from the hardware.
     *  @return Angle in degrees (−45 to +45), derived from the active pulse width.
     */
    int8_t getAngle();

    /** @brief Turn the servo channel off (no PWM output). */
    void off();

private:
    PCA9685* _pca;      ///< Shared PWM controller.
    bool     _freqSet;  ///< true after 60Hz switch has been performed.

    /** @brief Switch PCA9685 to 60Hz if not already done. Turns off RGB channels first. */
    void ensureServoFreq();

   
    /** @brief Return the current servo pulse width in PCA9685 ticks. */
    uint16_t getPulseTicks();

    /** @brief Convert a pulse width in microseconds to a PCA9685 tick count.
     * @param pulseUs Pulse width in microseconds.
     * @return 12-bit tick count (0–4095).
     */
    uint16_t usToTicks(uint16_t pulseUs);
};
