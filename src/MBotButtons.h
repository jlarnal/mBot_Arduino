/**
 * @file MBotButtons.h
 * @brief Driver for the two user buttons (A and B) on the BBC micro:bit v2.
 *
 * Buttons are active-low with internal pull-ups.  GPIO setup is deferred
 * until the first read.
 */
#pragma once

#include <Arduino.h>

/**
 * @class MBotButtons
 * @brief Reads the state of buttons A and B.
 */
class MBotButtons {
public:
    MBotButtons();

    /**
     * @brief Read button A.
     * @return @c true if button A is currently pressed.
     */
    bool a();

    /**
     * @brief Read button B.
     * @return @c true if button B is currently pressed.
     */
    bool b();

private:
    bool _initialized;  ///< @c true after GPIO setup.

    /** @brief Configure button GPIO pins (runs once). */
    void ensureInit();
};
