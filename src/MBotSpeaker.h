/**
 * @file MBotSpeaker.h
 * @brief Speaker driver for the BBC micro:bit v2 on-board speaker.
 *
 * Uses the Arduino @c tone() / @c noTone() API.  A zero-duration tone
 * plays indefinitely until stopped with noTone().
 */
#pragma once

#include <Arduino.h>

/**
 * @class MBotSpeaker
 * @brief Generates tones and beeps on the micro:bit speaker.
 */
class MBotSpeaker {
public:
    MBotSpeaker();

    /**
     * @brief Play a tone at a given frequency.
     * @param freq       Frequency in Hz.
     * @param durationMs Duration in milliseconds (0 = play until noTone()).
     */
    void tone(uint16_t freq, uint32_t durationMs = 0);

    /** @brief Stop any currently playing tone. */
    void noTone();

    /**
     * @brief Play a short default beep.
     *
     * Uses @ref BEEP_FREQ_HZ and @ref BEEP_DURATION_MS from bsp.h.
     */
    void beep();

private:
    bool _initialized;  ///< @c true after GPIO setup.
    bool _playing;       ///< @c true while a tone is active.

    /** @brief Configure the speaker pin (runs once). */
    void ensureInit();
};
