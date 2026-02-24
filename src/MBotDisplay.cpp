#include "MBotDisplay.h"
#include "bsp.h"
#include <string.h>
#include <stdlib.h>

// ── Font data ────────────────────────────────────────────────────────
// Each glyph: 5 bytes (rows), bits 4..0 = columns left-to-right.

static const uint8_t FONT_DIGITS[][5] PROGMEM = {
    { 0b01110, 0b10001, 0b10001, 0b10001, 0b01110 }, // 0
    { 0b00100, 0b01100, 0b00100, 0b00100, 0b01110 }, // 1
    { 0b01110, 0b10001, 0b00110, 0b01000, 0b11111 }, // 2
    { 0b11110, 0b00001, 0b01110, 0b00001, 0b11110 }, // 3
    { 0b10010, 0b10010, 0b11111, 0b00010, 0b00010 }, // 4
    { 0b11111, 0b10000, 0b11110, 0b00001, 0b11110 }, // 5
    { 0b01110, 0b10000, 0b11110, 0b10001, 0b01110 }, // 6
    { 0b11111, 0b00001, 0b00010, 0b00100, 0b00100 }, // 7
    { 0b01110, 0b10001, 0b01110, 0b10001, 0b01110 }, // 8
    { 0b01110, 0b10001, 0b01111, 0b00001, 0b01110 }, // 9
};

static const uint8_t FONT_ALPHA[][5] PROGMEM = {
    { 0b01110, 0b10001, 0b11111, 0b10001, 0b10001 }, // A
    { 0b11110, 0b10001, 0b11110, 0b10001, 0b11110 }, // B
    { 0b01110, 0b10001, 0b10000, 0b10001, 0b01110 }, // C
    { 0b11110, 0b10001, 0b10001, 0b10001, 0b11110 }, // D
    { 0b11111, 0b10000, 0b11110, 0b10000, 0b11111 }, // E
    { 0b11111, 0b10000, 0b11110, 0b10000, 0b10000 }, // F
    { 0b01110, 0b10000, 0b10011, 0b10001, 0b01110 }, // G
    { 0b10001, 0b10001, 0b11111, 0b10001, 0b10001 }, // H
    { 0b01110, 0b00100, 0b00100, 0b00100, 0b01110 }, // I
    { 0b00111, 0b00001, 0b00001, 0b10001, 0b01110 }, // J
    { 0b10001, 0b10010, 0b11100, 0b10010, 0b10001 }, // K
    { 0b10000, 0b10000, 0b10000, 0b10000, 0b11111 }, // L
    { 0b10001, 0b11011, 0b10101, 0b10001, 0b10001 }, // M
    { 0b10001, 0b11001, 0b10101, 0b10011, 0b10001 }, // N
    { 0b01110, 0b10001, 0b10001, 0b10001, 0b01110 }, // O
    { 0b11110, 0b10001, 0b11110, 0b10000, 0b10000 }, // P
    { 0b01110, 0b10001, 0b10101, 0b10010, 0b01101 }, // Q
    { 0b11110, 0b10001, 0b11110, 0b10010, 0b10001 }, // R
    { 0b01111, 0b10000, 0b01110, 0b00001, 0b11110 }, // S
    { 0b11111, 0b00100, 0b00100, 0b00100, 0b00100 }, // T
    { 0b10001, 0b10001, 0b10001, 0b10001, 0b01110 }, // U
    { 0b10001, 0b10001, 0b10001, 0b01010, 0b00100 }, // V
    { 0b10001, 0b10001, 0b10101, 0b11011, 0b10001 }, // W
    { 0b10001, 0b01010, 0b00100, 0b01010, 0b10001 }, // X
    { 0b10001, 0b01010, 0b00100, 0b00100, 0b00100 }, // Y
    { 0b11111, 0b00010, 0b00100, 0b01000, 0b11111 }, // Z
};

/// Retrieve the 5-row glyph for a character (NULL if unsupported).
static const uint8_t* glyphFor(char c) {
    if (c >= '0' && c <= '9') return FONT_DIGITS[c - '0'];
    if (c >= 'A' && c <= 'Z') return FONT_ALPHA[c - 'A'];
    if (c >= 'a' && c <= 'z') return FONT_ALPHA[c - 'a'];
    return nullptr;  // space or unsupported → blank
}

// ── Global ISR trampoline state ──────────────────────────────────────

MBotDisplay* _g_marqueeDisplay = nullptr;

extern "C" void TIMER2_IRQHandler(void) {
    if (NRF_TIMER2->EVENTS_COMPARE[0]) {
        NRF_TIMER2->EVENTS_COMPARE[0] = 0;
        if (_g_marqueeDisplay) {
            _g_marqueeDisplay->advanceMarquee();
        }
    }
}

// ── MBotDisplay implementation ───────────────────────────────────────

MBotDisplay::MBotDisplay()
    : _initialized(false), _scrolling(false),
      _strip(nullptr), _stripLen(0), _scrollCol(0), _invert(false) {
    memset(_buffer, 0, sizeof(_buffer));
}

MBotDisplay::~MBotDisplay() {
    stopMarquee();
}

void MBotDisplay::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    for (uint8_t i = 0; i < DISPLAY_ROWS; i++) {
        pinMode(DISPLAY_ROW_PINS[i], OUTPUT);
        digitalWrite(DISPLAY_ROW_PINS[i], LOW);
    }
    for (uint8_t i = 0; i < DISPLAY_COLS; i++) {
        pinMode(DISPLAY_COL_PINS[i], OUTPUT);
        digitalWrite(DISPLAY_COL_PINS[i], HIGH);  // active-low: HIGH = off
    }
}

void MBotDisplay::show(const uint8_t image[DISPLAY_ROWS]) {
    stopMarquee();
    ensureInit();
    memcpy(_buffer, image, DISPLAY_ROWS);
}

void MBotDisplay::showChar(char c) {
    stopMarquee();
    ensureInit();
    const uint8_t* glyph = glyphFor(c);
    if (glyph) {
        memcpy_P(_buffer, glyph, DISPLAY_ROWS);
    } else {
        memset(_buffer, 0, DISPLAY_ROWS);
    }
}

// ── Marquee ──────────────────────────────────────────────────────────

void MBotDisplay::showString(const char* text, uint16_t scrollMs, bool invert) {
    stopMarquee();
    ensureInit();

    // NULL or empty string: just cancel, don't clear display
    if (!text || text[0] == '\0') return;

    _invert = invert;
    _stripLen = buildStrip(text);
    if (_stripLen == 0) return;

    _scrollCol = 0;
    _g_marqueeDisplay = this;
    _scrolling = true;

    // Render first frame immediately
    advanceMarquee();

    startTimer(scrollMs);
}

uint16_t MBotDisplay::buildStrip(const char* text) {
    // First pass: compute glyph width + trailing blank padding
    uint16_t glyphCols = 0;
    for (const char* p = text; *p; p++) {
        if (p != text) glyphCols += DISPLAY_CHAR_GAP;
        glyphCols += DISPLAY_GLYPH_WIDTH;
    }
    // Add DISPLAY_COLS blank columns so text scrolls fully off before wrapping
    uint16_t totalCols = glyphCols + DISPLAY_COLS;

    // Allocate: DISPLAY_ROWS rows × totalCols columns (calloc zeroes the padding)
    // Layout: row-major — _strip[row * totalCols + col]
    free(_strip);
    _strip = (uint8_t*)calloc((uint32_t)totalCols * DISPLAY_ROWS, 1);
    if (!_strip) return 0;

    // Second pass: blit each glyph column-by-column at stride totalCols
    uint16_t col = 0;
    for (const char* p = text; *p; p++) {
        if (p != text) col += DISPLAY_CHAR_GAP;  // gap is already zeroed by calloc

        const uint8_t* glyph = glyphFor(*p);
        for (uint8_t gc = 0; gc < DISPLAY_GLYPH_WIDTH; gc++) {
            for (uint8_t row = 0; row < DISPLAY_ROWS; row++) {
                uint8_t rowBits = glyph ? pgm_read_byte(&glyph[row]) : 0;
                // Bit 4 = leftmost column (gc=0), bit 0 = rightmost (gc=4)
                uint8_t bit = (rowBits >> (DISPLAY_GLYPH_WIDTH - 1 - gc)) & 1;
                _strip[(uint32_t)row * totalCols + col + gc] = bit;
            }
        }
        col += DISPLAY_GLYPH_WIDTH;
    }

    return totalCols;
}

void MBotDisplay::advanceMarquee() {
    if (_scrollCol >= _stripLen) {
        // Wrap around for continuous loop
        _scrollCol = 0;
    }

    // Extract DISPLAY_COLS columns starting at _scrollCol into _buffer
    for (uint8_t row = 0; row < DISPLAY_ROWS; row++) {
        uint8_t rowByte = 0;
        for (uint8_t c = 0; c < DISPLAY_COLS; c++) {
            uint16_t srcCol = _scrollCol + c;
            uint8_t bit = 0;
            if (srcCol < _stripLen) {
                bit = _strip[(uint32_t)row * _stripLen + srcCol];
            }
            if (_invert) bit ^= 1;
            rowByte |= (bit << (DISPLAY_COLS - 1 - c));
        }
        _buffer[row] = rowByte;
    }

    _scrollCol++;
}

void MBotDisplay::stopMarquee() {
    if (_scrolling) {
        stopTimer();
        _scrolling = false;
    }
    if (_strip) {
        free(_strip);
        _strip = nullptr;
        _stripLen = 0;
    }
    _g_marqueeDisplay = nullptr;
}

// ── Hardware timer (nRF52 TIMER2) ────────────────────────────────────

void MBotDisplay::startTimer(uint16_t scrollMs) {
    // TIMER2: 32-bit, 1 MHz tick (prescaler 4 → 16 MHz / 2^4 = 1 MHz)
    NRF_TIMER2->TASKS_STOP  = 1;
    NRF_TIMER2->MODE        = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE     = TIMER_BITMODE_BITMODE_32Bit;
    NRF_TIMER2->PRESCALER   = 4;  // 16 MHz / 16 = 1 MHz → 1 µs per tick
    NRF_TIMER2->CC[0]       = (uint32_t)scrollMs * 1000UL;  // ms → µs ticks
    NRF_TIMER2->SHORTS      = TIMER_SHORTS_COMPARE0_CLEAR_Msk;
    NRF_TIMER2->INTENSET    = TIMER_INTENSET_COMPARE0_Msk;

    NVIC_SetPriority(TIMER2_IRQn, 3);
    NVIC_ClearPendingIRQ(TIMER2_IRQn);
    NVIC_EnableIRQ(TIMER2_IRQn);

    NRF_TIMER2->TASKS_CLEAR = 1;
    NRF_TIMER2->TASKS_START = 1;
}

void MBotDisplay::stopTimer() {
    NRF_TIMER2->TASKS_STOP = 1;
    NRF_TIMER2->INTENCLR   = TIMER_INTENCLR_COMPARE0_Msk;
    NVIC_DisableIRQ(TIMER2_IRQn);
}

// ── Static display operations ────────────────────────────────────────

void MBotDisplay::setPixel(uint8_t x, uint8_t y, bool on) {
    stopMarquee();
    ensureInit();
    if (x >= DISPLAY_COLS || y >= DISPLAY_ROWS) return;
    if (on) {
        _buffer[y] |= (1 << (DISPLAY_COLS - 1 - x));
    } else {
        _buffer[y] &= ~(1 << (DISPLAY_COLS - 1 - x));
    }
}

void MBotDisplay::clear() {
    stopMarquee();
    ensureInit();
    memset(_buffer, 0, sizeof(_buffer));
}

void MBotDisplay::refresh() {
    ensureInit();
    for (uint8_t row = 0; row < DISPLAY_ROWS; row++) {
        scanRow(row);
    }
}

void MBotDisplay::scanRow(uint8_t row) {
    for (uint8_t r = 0; r < DISPLAY_ROWS; r++) {
        digitalWrite(DISPLAY_ROW_PINS[r], LOW);
    }
    for (uint8_t col = 0; col < DISPLAY_COLS; col++) {
        bool on = _buffer[row] & (1 << (DISPLAY_COLS - 1 - col));
        digitalWrite(DISPLAY_COL_PINS[col], on ? LOW : HIGH);
    }
    digitalWrite(DISPLAY_ROW_PINS[row], HIGH);
    delayMicroseconds(DISPLAY_ROW_DWELL_US);
    digitalWrite(DISPLAY_ROW_PINS[row], LOW);
}
