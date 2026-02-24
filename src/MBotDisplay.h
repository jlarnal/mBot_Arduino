/**
 * @file MBotDisplay.h
 * @brief Driver for the BBC micro:bit v2 5×5 LED matrix.
 *
 * The display is multiplexed row-by-row.  Call refresh() frequently (e.g.
 * every loop() iteration) to maintain a flicker-free image.  Each row is
 * stored as a byte whose lower bits represent column states.
 *
 * showString() launches a non-blocking marquee that scrolls text one
 * column at a time using a hardware timer ISR (nRF52 TIMER2).
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @class MBotDisplay
 * @brief 5×5 LED matrix display with character rendering, pixel access
 *        and non-blocking marquee scrolling.
 */
class MBotDisplay {
public:
    MBotDisplay();
    ~MBotDisplay();

    /**
     * @brief Show a raw 5-row bitmap on the display.
     * @param image Array of @ref DISPLAY_ROWS bytes; each bit = one column.
     */
    void show(const uint8_t image[DISPLAY_ROWS]);

    /**
     * @brief Show a single ASCII character using the built-in 5×5 font.
     * @param c Character to display (printable ASCII).
     */
    void showChar(char c);

    /**
     * @brief Scroll a string across the display as a marquee.
     *
     * Non-blocking.  Passing @c nullptr or an empty string cancels any
     * running marquee without clearing the display.
     *
     * @param text     Null-terminated string to scroll (copied internally).
     * @param scrollMs Time per column shift in milliseconds (defaults to SHOWSTRING_MARQUEE_DEFAULT_MS).
     * @param invert   If @c true, lit/unlit pixels are swapped.
     */
    void showString(const char* text, uint16_t scrollMs = SHOWSTRING_MARQUEE_DEFAULT_MS, bool invert = false);

    /**
     * @brief Set or clear an individual pixel.
     * @param x  Column index (0 = left, 4 = right).
     * @param y  Row index (0 = top, 4 = bottom).
     * @param on @c true to light the pixel, @c false to clear it.
     */
    void setPixel(uint8_t x, uint8_t y, bool on);

    /**
     * @brief Clear the display and cancel any running marquee.
     */
    void clear();

    /**
     * @brief Multiplex one display row.
     *
     * Must be called frequently from loop() to keep the display visible.
     * Each call lights one row for @ref DISPLAY_ROW_DWELL_US microseconds.
     */
    void refresh();

    /**
     * @brief Check whether a marquee is currently scrolling.
     * @return @c true while a marquee is active.
     */
    bool isScrolling() const { return _scrolling; }

    /// @cond INTERNAL
    /** @brief Advance the marquee by one column (called from TIMER2 ISR). */
    void advanceMarquee();
    /// @endcond

private:
    bool _initialized;                    ///< @c true after GPIO setup.
    uint8_t _buffer[DISPLAY_ROWS];        ///< Frame buffer (one byte per row, bit = column).

    /// @name Marquee state
    /// @{
    volatile bool _scrolling;             ///< @c true while the marquee timer is running.
    uint8_t* _strip;                      ///< Column strip: _stripLen bytes per row, 5 rows.
    uint16_t _stripLen;                   ///< Total number of columns in the strip.
    volatile uint16_t _scrollCol;         ///< Current scroll offset (column index).
    bool _invert;                         ///< Invert pixel state when rendering.
    /// @}

    /** @brief Configure row and column GPIO pins (runs once). */
    void ensureInit();

    /**
     * @brief Light a single row of the display.
     * @param row Row index to scan (0–4).
     */
    void scanRow(uint8_t row);

    /** @brief Stop the marquee timer and release the column strip. */
    void stopMarquee();

    /**
     * @brief Build a column strip from a string and its font glyphs.
     * @param text Null-terminated string.
     * @return Total number of columns in the strip.
     */
    uint16_t buildStrip(const char* text);

    /** @brief Start the hardware timer at the requested interval. */
    void startTimer(uint16_t scrollMs);

    /** @brief Stop and disable the hardware timer. */
    void stopTimer();

};

/// @cond INTERNAL
/** @brief Global pointer used by the timer ISR to reach the display instance. */
extern MBotDisplay* _g_marqueeDisplay;
/// @endcond
