/**
 * @file MBotLineTracker.h
 * @brief Digital line-following sensor driver.
 *
 * The mBot has two infrared line-tracking sensors mounted under the chassis.
 * Each sensor returns a digital value: HIGH when over a dark line, LOW when
 * over a light surface (or vice-versa depending on the module revision).
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @class MBotLineTracker
 * @brief Reads the left and right digital line sensors.
 */
class MBotLineTracker {
public:
    MBotLineTracker();

    /**
     * @brief Read the left line sensor.
     * @return @c true if the sensor detects a line (HIGH).
     */
    bool left();

    /**
     * @brief Read the right line sensor.
     * @return @c true if the sensor detects a line (HIGH).
     */
    bool right();

private:
    bool _initialized;  ///< @c true after GPIO setup.

    /** @brief Configure GPIO pins (runs once). */
    void ensureInit();
};
