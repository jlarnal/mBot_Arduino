/**
 * @file MBotRGB.h
 * @brief RGB LED driver using PCA9685 PWM channels.
 *
 * Controls the on-board RGB LED set through three dedicated PCA9685
 * channels (one per colour).  Channel assignments are defined in bsp.h.
 */
#pragma once

#include <Arduino.h>

class PCA9685;

/**
 * @class MBotRGB
 * @brief Controls the RGB LED(s) on the MakeBit carrier board.
 */
class MBotRGB {
public:
    /**
     * @brief Construct the RGB driver.
     * @param pca Pointer to the shared PCA9685 instance.
     */
    explicit MBotRGB(PCA9685* pca);

    /**
     * @brief Set the LED colour.
     * @param r Red intensity (0–255).
     * @param g Green intensity (0–255).
     * @param b Blue intensity (0–255).
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief Set the LED colour (indexed variant — index is ignored).
     * @param index LED index (unused; only one set of RGB LEDs exists).
     * @param r Red intensity (0–255).
     * @param g Green intensity (0–255).
     * @param b Blue intensity (0–255).
     */
    void setColor(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

    /** @brief Turn the RGB LED off. */
    void off();

private:
    PCA9685* _pca;  ///< Shared PWM controller.

    /**
     * @brief Convert an 8-bit colour value to a 12-bit PCA9685 PWM value.
     * @param value Colour intensity (0–255).
     * @return 12-bit PWM value (0–4095).
     */
    uint16_t colorToPWM(uint8_t value);
};
