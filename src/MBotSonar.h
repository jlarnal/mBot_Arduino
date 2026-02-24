/**
 * @file MBotSonar.h
 * @brief Ultrasonic distance sensor driver (HC-SR04 style).
 *
 * Measures distance via a trigger/echo GPIO pair.  Returns the result
 * in metres as a @c float (the nRF52833 has a hardware FPU).
 *
 * When multiple samples are requested the readings are **median-filtered**,
 * which is particularly effective at rejecting the outlier spikes that
 * ultrasonic sensors produce from multi-path reflections and missed echoes.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @class MBotSonar
 * @brief Non-blocking-capable ultrasonic range finder.
 */
class MBotSonar {
public:
    MBotSonar();

    /**
     * @brief Take a distance measurement.
     *
     * When @p samples is 1 a single raw reading is returned.
     * When @p samples > 1 the readings are median-filtered.
     *
     * @param samples Number of echo measurements to take (1–@ref SONAR_MAX_SAMPLES,
     *                default @ref SONAR_DEFAULT_SAMPLES).
     * @return Distance in metres, or a negative value on timeout / error.
     */
    float read(uint8_t samples = SONAR_DEFAULT_SAMPLES);

    /**
     * @brief Take a single unfiltered distance measurement.
     * @return Distance in metres, or a negative value on timeout.
     */
    float readRaw();

private:
    bool _initialized;  ///< @c true after GPIO setup.

    /** @brief Configure trigger and echo GPIO pins (runs once). */
    void ensureInit();

    /**
     * @brief Fire the trigger pulse and measure the echo duration.
     * @return Echo pulse width in microseconds, or 0 on timeout.
     */
    uint32_t ping();

    /**
     * @brief Convert an echo duration to a distance in metres.
     * @param echoUs Echo pulse width in microseconds.
     * @return Distance in metres.
     */
    static float usToMetres(uint32_t echoUs);

    /**
     * @brief In-place insertion sort for a small array of floats.
     * @param arr  Array to sort.
     * @param len  Number of elements.
     */
    static void sortFloats(float* arr, uint8_t len);
};
