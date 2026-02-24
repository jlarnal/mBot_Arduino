#include "MBotIR.h"
#include "bsp.h"

/// Global instance pointer for ISR access (same pattern as MBotDisplay).
static MBotIR* _g_irInstance = nullptr;

// ── Timing helper ───────────────────────────────────────────────────

bool MBotIR::matchTiming(uint32_t measured, uint16_t expected) {
    uint32_t margin = (uint32_t)expected * IR_NEC_TOLERANCE / 100;
    return measured >= (expected - margin) && measured <= (expected + margin);
}

// ── ISR ─────────────────────────────────────────────────────────────

void MBotIR::isrHandler() {
    if (!_g_irInstance) return;
    MBotIR& ir = *_g_irInstance;

    uint32_t now = micros();

    // If we already have a complete frame, ignore edges until read() clears it
    if (ir._frameReady || ir._repeatReady) return;

    if (ir._edgeCount == 0) {
        ir._edges[0] = now;
        ir._edgeCount = 1;
        return;
    }

    // Timeout — if gap since last edge > 15ms, restart
    if (now - ir._edges[ir._edgeCount - 1] > 15000) {
        ir._edges[0] = now;
        ir._edgeCount = 1;
        return;
    }

    ir._edges[ir._edgeCount] = now;
    ir._edgeCount++;

    // Detect repeat code after 4 edges: leader mark + short space
    if (ir._edgeCount == 4) {
        uint32_t markLen  = ir._edges[1] - ir._edges[0];
        uint32_t spaceLen = ir._edges[2] - ir._edges[1];
        if (matchTiming(markLen, IR_NEC_HDR_MARK) &&
            matchTiming(spaceLen, IR_NEC_RPT_SPACE)) {
            ir._repeatReady = true;
            ir._edgeCount = 0;
            return;
        }
    }

    if (ir._edgeCount >= IR_NEC_EDGES) {
        ir._frameReady = true;
        ir._edgeCount = 0;
    }
}

// ── Constructor / init ──────────────────────────────────────────────

MBotIR::MBotIR()
    : _initialized(false), _frameReady(false), _repeatReady(false),
      _edgeCount(0), _lastCmd(0), _lastAddr(0) {}

void MBotIR::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    _g_irInstance = this;
    pinMode(IR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), isrHandler, CHANGE);
}

// ── Frame decode ────────────────────────────────────────────────────

bool MBotIR::decodeFrame(uint8_t& addr, uint8_t& cmd) {
    uint32_t leaderMark  = _edges[1] - _edges[0];
    uint32_t leaderSpace = _edges[2] - _edges[1];
    if (!matchTiming(leaderMark, IR_NEC_HDR_MARK) ||
        !matchTiming(leaderSpace, IR_NEC_HDR_SPACE)) {
        return false;
    }

    uint32_t data = 0;
    for (uint8_t i = 0; i < 32; i++) {
        uint8_t base = 2 + i * 2;
        uint32_t spaceLen = _edges[base + 2] - _edges[base + 1];
        data <<= 1;
        if (matchTiming(spaceLen, IR_NEC_ONE_SPACE)) {
            data |= 1;
        } else if (!matchTiming(spaceLen, IR_NEC_ZERO_SPACE)) {
            return false;
        }
    }

    uint8_t a  = (data >> 24) & 0xFF;
    uint8_t na = (data >> 16) & 0xFF;
    uint8_t c  = (data >>  8) & 0xFF;
    uint8_t nc =  data        & 0xFF;

    if ((uint8_t)(a ^ na) != 0xFF || (uint8_t)(c ^ nc) != 0xFF) {
        return false;
    }

    addr = a;
    cmd  = c;
    return true;
}

// ── Public API ──────────────────────────────────────────────────────

bool MBotIR::available() {
    ensureInit();

    if (_repeatReady) {
        _repeatReady = false;
        return (_lastCmd != 0);  // repeat only valid if we decoded before
    }

    if (!_frameReady) return false;

    uint8_t addr, cmd;
    bool ok = decodeFrame(addr, cmd);
    _frameReady = false;

    if (!ok) return false;

    _lastAddr = addr;
    _lastCmd  = cmd;
    return true;
}

uint8_t MBotIR::command() const { return _lastCmd; }
uint8_t MBotIR::address() const { return _lastAddr; }
