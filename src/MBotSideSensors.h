/**
 * @file MBotSideSensors.h
 * @brief Multi-purpose side sensor driver (IR / LDR / flame — swappable).
 *
 * The mBot has two side-mounted sensor sockets (left and right) that accept
 * interchangeable digital sensor modules: infrared reflectance, LDR (light),
 * or flame detection.  All variants present a simple digital interface.
 *
 * For flame sensors the logic is active-low: LOW = flame detected,
 * HIGH = no flame.  The @ref leftFlame() / @ref rightFlame() convenience
 * methods account for this inversion.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @class MBotSideSensors
 * @brief Reads the left and right side-mounted digital sensor modules.
 */
class MBotSideSensors {
public:
    MBotSideSensors();

    /**
     * @brief Raw digital read of the left sensor.
     * @return @c true if the pin is HIGH.
     */
    bool leftRaw();

    /**
     * @brief Raw digital read of the right sensor.
     * @return @c true if the pin is HIGH.
     */
    bool rightRaw();

    /**
     * @brief Left flame detection (active-low logic inverted).
     * @return @c true if a flame is detected (pin LOW).
     */
    bool leftFlame();

    /**
     * @brief Right flame detection (active-low logic inverted).
     * @return @c true if a flame is detected (pin LOW).
     */
    bool rightFlame();

private:
    bool _initialized;  ///< @c true after GPIO setup.

    /** @brief Configure GPIO pins (runs once). */
    void ensureInit();
};
