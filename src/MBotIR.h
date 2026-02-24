/**
 * @file MBotIR.h
 * @brief NEC infrared remote receiver driver (TL1838 on P8).
 *
 * Interrupt-driven decoder for NEC-protocol IR remotes.
 * Call available() from loop() to check for new key presses.
 * The ISR captures edge timestamps; decoding happens in available().
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @class MBotIR
 * @brief Reads NEC IR frames from the TL1838 receiver on @ref IR_PIN.
 *
 * Uses a GPIO interrupt (CHANGE) to capture pulse-edge timestamps.
 * Fully non-blocking — available() returns immediately.
 */
class MBotIR {
public:
    MBotIR();

    /**
     * @brief Check whether a new IR key press has been received.
     * @return @c true if a new frame (or repeat) was decoded.
     *         Use command() / address() to retrieve the values.
     */
    bool available();

    /**
     * @brief NEC command byte from the last decoded frame.
     * @return Command byte (0x00–0xFF).
     */
    uint8_t command() const;

    /**
     * @brief NEC address byte from the last decoded frame.
     * @return Address byte (0x00–0xFF).
     */
    uint8_t address() const;

private:
    bool _initialized;                        ///< true after GPIO + interrupt setup.
    volatile bool _frameReady;                ///< Set by ISR when edge buffer is full.
    volatile bool _repeatReady;               ///< Set by ISR on repeat code detection.
    volatile uint8_t _edgeCount;              ///< Number of edges captured so far.
    volatile uint32_t _edges[IR_NEC_EDGES];   ///< Timestamps (µs) of captured edges.

    uint8_t _lastCmd;   ///< Last successfully decoded command byte.
    uint8_t _lastAddr;  ///< Last successfully decoded address byte.

    void ensureInit();

    /**
     * @brief Decode the edge buffer into address + command bytes.
     * @param[out] addr Decoded address byte.
     * @param[out] cmd  Decoded command byte.
     * @return true if the frame is valid (inverses match).
     */
    bool decodeFrame(uint8_t& addr, uint8_t& cmd);

    /**
     * @brief Check if a measured duration is within tolerance of an expected value.
     * @param measured Actual duration (µs).
     * @param expected Nominal duration (µs).
     * @return true if within IR_NEC_TOLERANCE percent.
     */
    static bool matchTiming(uint32_t measured, uint16_t expected);

    /** @brief ISR entry point — called on every edge of IR_PIN. */
    static void isrHandler();
};
