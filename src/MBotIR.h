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

enum IR_Key : uint8_t
{
    IR_CHN = 0XA2,
    IR_CH = 0X62,
    IR_CHP = 0XE2,
    IR_PREV = 0X22,
    IR_NEXT = 0X2,
    IR_PLAY = 0XC2,
    IR_VOLN = 0XE0,
    IR_VOLP = 0XA8,
    IR_EQ = 0X90,
    IR_NUM_0 = 0X68,
    IR_NUM_100 = 0X98,
    IR_NUM_200 = 0XB0,
    IR_NUM_1 = 0X30,
    IR_NUM_2 = 0X18,
    IR_NUM_3 = 0X7A,
    IR_NUM_4 = 0X10,
    IR_NUM_5 = 0X38,
    IR_NUM_6 = 0X5A,
    IR_NUM_7 = 0X42,
    IR_NUM_8 = 0X4A,
    IR_NUM_9 = 0X52,
};

/**
 * @class MBotIR
 * @brief Reads NEC IR frames from the TL1838 receiver on @ref IR_PIN.
 *
 * Uses a GPIO interrupt (CHANGE) to capture pulse-edge timestamps.
 * Fully non-blocking — available() returns immediately.
 */
class MBotIR
{
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
    bool _initialized;                      ///< true after GPIO + interrupt setup.
    volatile bool _frameReady;              ///< Set by ISR when edge buffer is full.
    volatile bool _repeatReady;             ///< Set by ISR on repeat code detection.
    volatile uint8_t _edgeCount;            ///< Number of edges captured so far.
    volatile uint32_t _edges[IR_NEC_EDGES]; ///< Timestamps (µs) of captured edges.

    uint8_t _lastCmd;  ///< Last successfully decoded command byte.
    uint8_t _lastAddr; ///< Last successfully decoded address byte.

    void ensureInit();

    /**
     * @brief Decode the edge buffer into address + command bytes.
     * @param[out] addr Decoded address byte.
     * @param[out] cmd  Decoded command byte.
     * @return true if the frame is valid (inverses match).
     */
    bool decodeFrame(uint8_t &addr, uint8_t &cmd);

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
