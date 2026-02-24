/**
 * @file MBotMic.h
 * @brief Driver for the on-board MEMS microphone on the BBC micro:bit v2.
 *
 * The microphone is enabled via a dedicated GPIO before reading the
 * analogue level.  Initialisation is deferred until the first call.
 */
#pragma once

#include <Arduino.h>

/**
 * @class MBotMic
 * @brief Reads the ambient sound level from the on-board microphone.
 */
class MBotMic {
public:
    MBotMic();

    /**
     * @brief Read the current microphone level.
     * @return 10-bit ADC value (0–1023).
     */
    uint16_t level();

    /**
     * @brief Check whether the sound level exceeds a threshold.
     * @param threshold ADC threshold (default 512).
     * @return @c true if the current level is above @p threshold.
     */
    bool isLoud(uint16_t threshold = 512);

private:
    bool _initialized;  ///< @c true after GPIO setup.

    /** @brief Configure microphone enable and analogue pins (runs once). */
    void ensureInit();
};
