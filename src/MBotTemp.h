/**
 * @file MBotTemp.h
 * @brief Temperature reading via the nRF52833 on-die TEMP peripheral.
 *
 * Uses the SoC's built-in temperature sensor — no external hardware needed.
 */
#pragma once

#include <Arduino.h>

/**
 * @class MBotTemp
 * @brief Reads the nRF52833 die temperature.
 */
class MBotTemp {
public:
    MBotTemp();

    /**
     * @brief Get the temperature in degrees Celsius.
     * @return Temperature (°C).
     */
    float celsius();


private:
    bool _initialized;  ///< @c true after peripheral setup.

    /** @brief Initialise the TEMP peripheral (runs once). */
    void ensureInit();
};
