# MBotIR Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add an interrupt-driven NEC IR remote receiver driver (`MBotIR`) for a TL1838 on pin P8, with named key enum, raw code access, and CLI integration.

**Architecture:** GPIO interrupt on CHANGE captures edge timestamps with `micros()` into a 66-element ring buffer. `read()` decodes the NEC frame (9ms leader + 32 data bits), validates address/command inverses, and maps the command byte to an `IrKey` enum. Repeat codes return the last valid key.

**Tech Stack:** Arduino framework on nRF52833 (BBC micro:bit v2), PlatformIO build system, zero external dependencies.

---

### Task 1: Add IR constants to BSP

**Files:**
- Modify: `src/bsp.h:127` (append before closing `/// @}` of the serial section, or after it)

**Step 1: Add the IR receiver constant group**

Append this block after the `bsp_serial` group (after line 127) in `src/bsp.h`:

```cpp

/// @defgroup bsp_ir IR receiver (TL1838, NEC protocol)
/// @{
static constexpr uint8_t  IR_PIN             = 8;     ///< GPIO pin for IR receiver data.
static constexpr uint16_t IR_NEC_HDR_MARK    = 9000;  ///< Leader mark duration (µs).
static constexpr uint16_t IR_NEC_HDR_SPACE   = 4500;  ///< Leader space duration (µs).
static constexpr uint16_t IR_NEC_BIT_MARK    = 562;   ///< Bit mark duration (µs).
static constexpr uint16_t IR_NEC_ONE_SPACE   = 1687;  ///< Logic-1 space duration (µs).
static constexpr uint16_t IR_NEC_ZERO_SPACE  = 562;   ///< Logic-0 space duration (µs).
static constexpr uint16_t IR_NEC_RPT_SPACE   = 2250;  ///< Repeat code space duration (µs).
static constexpr uint8_t  IR_NEC_TOLERANCE   = 25;    ///< Timing tolerance (percent).
static constexpr uint8_t  IR_NEC_EDGES       = 66;    ///< Edges per complete NEC frame.
/// @}
```

**Step 2: Build to verify no syntax errors**

Run: `pio run`
Expected: SUCCESS (no warnings from bsp.h)

**Step 3: Commit**

```bash
git add src/bsp.h
git commit -m "bsp: add IR receiver (TL1838) pin and NEC timing constants"
```

---

### Task 2: Create MBotIR header

**Files:**
- Create: `src/MBotIR.h`

**Step 1: Write the header file**

```cpp
/**
 * @file MBotIR.h
 * @brief NEC infrared remote receiver driver (TL1838 on P8).
 *
 * Interrupt-driven decoder for the common 21-key mini NEC remote.
 * Call read() from loop() to poll for decoded key presses.
 * The ISR captures edge timestamps; decoding happens in read().
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

/**
 * @enum IrKey
 * @brief Named keys for the 21-key mini NEC remote.
 */
enum class IrKey : uint8_t {
    NONE = 0,
    CH_MINUS, CH, CH_PLUS,
    PREV, NEXT, PLAY,
    VOL_MINUS, VOL_PLUS, EQ,
    NUM_0, NUM_100, NUM_200,
    NUM_1, NUM_2, NUM_3,
    NUM_4, NUM_5, NUM_6,
    NUM_7, NUM_8, NUM_9
};

/**
 * @class MBotIR
 * @brief Reads NEC IR frames from the TL1838 receiver on @ref IR_PIN.
 *
 * Uses a GPIO interrupt (CHANGE) to capture pulse-edge timestamps.
 * Fully non-blocking — read() returns immediately with the decoded
 * key or IrKey::NONE.
 */
class MBotIR {
public:
    MBotIR();

    /**
     * @brief Poll for a decoded IR key press.
     * @return The decoded key, or @c IrKey::NONE if no new frame is ready.
     */
    IrKey read();

    /**
     * @brief Get the raw NEC command byte from the last decoded frame.
     * @return Command byte (0x00–0xFF).
     */
    uint8_t rawCommand() const;

    /**
     * @brief Get the raw NEC address byte from the last decoded frame.
     * @return Address byte (0x00–0xFF).
     */
    uint8_t rawAddress() const;

    /**
     * @brief Return a human-readable name for an IrKey value.
     * @param key The key to name.
     * @return Pointer to a PROGMEM string (e.g. "CH-", "PLAY", "5").
     */
    static const char* keyName(IrKey key);

private:
    bool _initialized;                        ///< true after GPIO + interrupt setup.
    volatile bool _frameReady;                ///< Set by ISR when edge buffer is full.
    volatile bool _repeatReady;               ///< Set by ISR on repeat code detection.
    volatile uint8_t _edgeCount;              ///< Number of edges captured so far.
    volatile uint32_t _edges[IR_NEC_EDGES];   ///< Timestamps (µs) of captured edges.

    uint8_t _lastCmd;   ///< Last successfully decoded command byte.
    uint8_t _lastAddr;  ///< Last successfully decoded address byte.
    IrKey   _lastKey;   ///< Last successfully decoded key.

    void ensureInit();

    /**
     * @brief Decode the edge buffer into address + command bytes.
     * @param[out] addr Decoded address byte.
     * @param[out] cmd  Decoded command byte.
     * @return true if the frame is valid (inverses match).
     */
    bool decodeFrame(uint8_t& addr, uint8_t& cmd);

    /**
     * @brief Map a raw NEC command byte to an IrKey.
     * @param cmd The command byte.
     * @return The corresponding IrKey, or IrKey::NONE if unmapped.
     */
    static IrKey mapCommand(uint8_t cmd);

    /**
     * @brief Check if a measured duration is within tolerance of an expected value.
     * @param measured Actual duration (µs).
     * @param expected Nominal duration (µs).
     * @return true if within IR_NEC_TOLERANCE percent.
     */
    static bool matchTiming(uint32_t measured, uint16_t expected);

    /** @brief ISR entry point — called on every edge of IR_PIN. */
    static void isrHandler();

    friend void irISR();
};
```

**Step 2: Build to verify header compiles**

Run: `pio run`
Expected: SUCCESS (header not yet included anywhere, so vacuously passes)

**Step 3: Commit**

```bash
git add src/MBotIR.h
git commit -m "Add MBotIR header with IrKey enum and class declaration"
```

---

### Task 3: Implement MBotIR driver

**Files:**
- Create: `src/MBotIR.cpp`

**Step 1: Write the implementation**

```cpp
#include "MBotIR.h"
#include "bsp.h"

/// Global instance pointer for ISR access (same pattern as MBotDisplay).
static MBotIR* _g_irInstance = nullptr;

// ── Key name table (PROGMEM) ────────────────────────────────────────

static const char _kn_none[]      PROGMEM = "NONE";
static const char _kn_ch_minus[]  PROGMEM = "CH-";
static const char _kn_ch[]        PROGMEM = "CH";
static const char _kn_ch_plus[]   PROGMEM = "CH+";
static const char _kn_prev[]      PROGMEM = "PREV";
static const char _kn_next[]      PROGMEM = "NEXT";
static const char _kn_play[]      PROGMEM = "PLAY";
static const char _kn_vol_minus[] PROGMEM = "VOL-";
static const char _kn_vol_plus[]  PROGMEM = "VOL+";
static const char _kn_eq[]        PROGMEM = "EQ";
static const char _kn_0[]         PROGMEM = "0";
static const char _kn_100[]       PROGMEM = "100+";
static const char _kn_200[]       PROGMEM = "200+";
static const char _kn_1[]         PROGMEM = "1";
static const char _kn_2[]         PROGMEM = "2";
static const char _kn_3[]         PROGMEM = "3";
static const char _kn_4[]         PROGMEM = "4";
static const char _kn_5[]         PROGMEM = "5";
static const char _kn_6[]         PROGMEM = "6";
static const char _kn_7[]         PROGMEM = "7";
static const char _kn_8[]         PROGMEM = "8";
static const char _kn_9[]         PROGMEM = "9";

static const char* const _keyNames[] PROGMEM = {
    _kn_none,
    _kn_ch_minus, _kn_ch, _kn_ch_plus,
    _kn_prev, _kn_next, _kn_play,
    _kn_vol_minus, _kn_vol_plus, _kn_eq,
    _kn_0, _kn_100, _kn_200,
    _kn_1, _kn_2, _kn_3,
    _kn_4, _kn_5, _kn_6,
    _kn_7, _kn_8, _kn_9
};

// ── NEC command byte → IrKey mapping ────────────────────────────────
// Common 21-key mini remote codes.  If your remote differs, update these.

static IrKey mapCommand(uint8_t cmd) {
    switch (cmd) {
        case 0x45: return IrKey::CH_MINUS;
        case 0x46: return IrKey::CH;
        case 0x47: return IrKey::CH_PLUS;
        case 0x44: return IrKey::PREV;
        case 0x40: return IrKey::NEXT;
        case 0x43: return IrKey::PLAY;
        case 0x07: return IrKey::VOL_MINUS;
        case 0x15: return IrKey::VOL_PLUS;
        case 0x09: return IrKey::EQ;
        case 0x16: return IrKey::NUM_0;
        case 0x19: return IrKey::NUM_100;
        case 0x0D: return IrKey::NUM_200;
        case 0x0C: return IrKey::NUM_1;
        case 0x18: return IrKey::NUM_2;
        case 0x5E: return IrKey::NUM_3;
        case 0x08: return IrKey::NUM_4;
        case 0x1C: return IrKey::NUM_5;
        case 0x5A: return IrKey::NUM_6;
        case 0x42: return IrKey::NUM_7;
        case 0x52: return IrKey::NUM_8;
        case 0x4A: return IrKey::NUM_9;
        default:   return IrKey::NONE;
    }
}

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
        // First edge (falling = start of leader mark)
        ir._edges[0] = now;
        ir._edgeCount = 1;
        return;
    }

    // Check for timeout — if gap since last edge > 15ms, restart
    if (now - ir._edges[ir._edgeCount - 1] > 15000) {
        ir._edges[0] = now;
        ir._edgeCount = 1;
        return;
    }

    ir._edges[ir._edgeCount] = now;
    ir._edgeCount++;

    // After 3 edges we can detect a repeat code:
    // edge[0]=leader mark start, edge[1]=leader mark end, edge[2]=space end
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
      _edgeCount(0), _lastCmd(0), _lastAddr(0), _lastKey(IrKey::NONE) {}

void MBotIR::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    _g_irInstance = this;
    pinMode(IR_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(IR_PIN), isrHandler, CHANGE);
}

// ── Frame decode ────────────────────────────────────────────────────

bool MBotIR::decodeFrame(uint8_t& addr, uint8_t& cmd) {
    // Validate leader: edges[0..1] = mark, edges[1..2] = space
    uint32_t leaderMark  = _edges[1] - _edges[0];
    uint32_t leaderSpace = _edges[2] - _edges[1];
    if (!matchTiming(leaderMark, IR_NEC_HDR_MARK) ||
        !matchTiming(leaderSpace, IR_NEC_HDR_SPACE)) {
        return false;
    }

    // Decode 32 data bits from edges[2..65]
    // Each bit occupies 2 edges: mark start → mark end (bit mark),
    // mark end → next mark start (space — determines 0 or 1).
    uint32_t data = 0;
    for (uint8_t i = 0; i < 32; i++) {
        uint8_t base = 2 + i * 2;      // index of this bit's mark-start edge
        uint32_t spaceLen = _edges[base + 2] - _edges[base + 1];
        data <<= 1;
        if (matchTiming(spaceLen, IR_NEC_ONE_SPACE)) {
            data |= 1;
        } else if (!matchTiming(spaceLen, IR_NEC_ZERO_SPACE)) {
            return false;  // bad timing
        }
    }

    // NEC sends LSB first within each byte, but our shift-left decoding
    // produces MSB-first.  The 32-bit word is:
    //   [addr MSBfirst] [~addr MSBfirst] [cmd MSBfirst] [~cmd MSBfirst]
    uint8_t a  = (data >> 24) & 0xFF;
    uint8_t na = (data >> 16) & 0xFF;
    uint8_t c  = (data >>  8) & 0xFF;
    uint8_t nc =  data        & 0xFF;

    if ((uint8_t)(a ^ na) != 0xFF || (uint8_t)(c ^ nc) != 0xFF) {
        return false;  // inverse validation failed
    }

    addr = a;
    cmd  = c;
    return true;
}

// ── Public API ──────────────────────────────────────────────────────

IrKey MBotIR::read() {
    ensureInit();

    if (_repeatReady) {
        _repeatReady = false;
        return _lastKey;  // repeat of last valid key
    }

    if (!_frameReady) return IrKey::NONE;

    // Copy volatile buffer while interrupts are briefly disabled
    uint32_t edgesCopy[IR_NEC_EDGES];
    noInterrupts();
    memcpy(edgesCopy, (const void*)_edges, sizeof(edgesCopy));
    _frameReady = false;
    interrupts();

    // Temporarily swap in the copy for decoding
    uint32_t saved[IR_NEC_EDGES];
    memcpy(saved, (const void*)_edges, sizeof(saved));
    memcpy((void*)_edges, edgesCopy, sizeof(_edges));

    uint8_t addr, cmd;
    bool ok = decodeFrame(addr, cmd);

    // Restore
    memcpy((void*)_edges, saved, sizeof(_edges));

    if (!ok) return IrKey::NONE;

    _lastAddr = addr;
    _lastCmd  = cmd;
    _lastKey  = mapCommand(cmd);
    return _lastKey;
}

uint8_t MBotIR::rawCommand() const { return _lastCmd; }
uint8_t MBotIR::rawAddress() const { return _lastAddr; }

const char* MBotIR::keyName(IrKey key) {
    uint8_t idx = static_cast<uint8_t>(key);
    if (idx > 21) idx = 0;
    return (const char*)pgm_read_ptr(&_keyNames[idx]);
}
```

**Step 2: Build to verify (won't link yet — not referenced)**

Run: `pio run`
Expected: SUCCESS (PlatformIO auto-compiles all .cpp in lib)

**Step 3: Commit**

```bash
git add src/MBotIR.cpp
git commit -m "Implement MBotIR: interrupt-driven NEC decoder with key mapping"
```

---

### Task 4: Wire MBotIR into the MBot facade

**Files:**
- Modify: `src/mBot.h:24` (add include) and `src/mBot.h:62` (add member)

**Step 1: Add include to mBot.h**

After the `#include "MBotFan.h"` line (line 24), add:

```cpp
#include "MBotIR.h"
```

**Step 2: Add public member to MBot class**

After `MBotFan fan;` (line 62), add:

```cpp
    MBotIR          ir;       ///< NEC IR remote receiver.
```

**Step 3: Build**

Run: `pio run`
Expected: SUCCESS — `ir` is default-constructed (GPIO-only, no PCA9685 pointer needed).

**Step 4: Commit**

```bash
git add src/mBot.h
git commit -m "Wire MBotIR into MBot facade as public 'ir' member"
```

---

### Task 5: Add CLI `ir` command and monitor support

**Files:**
- Modify: `src/MBotCLI.h:58` (add `MON_IR` to enum) and add `handleIR` declaration
- Modify: `src/MBotCLI.cpp` (dispatch, help, monitor, handleIR)

**Step 1: Add MON_IR to MonitorTarget enum in MBotCLI.h**

In the `MonitorTarget` enum (line 58), after `MON_SIDE`, add:

```cpp
        MON_IR        ///< IR remote (last key).
```

**Step 2: Add handleIR declaration in MBotCLI.h**

After the `handleFan` declaration (around line 140), add:

```cpp
    /**
     * @brief Handle the "ir" command.
     * @param args Remaining tokens after "ir".
     */
    void handleIR(char* args);
```

**Step 3: Add dispatch entry in MBotCLI.cpp**

In `dispatch()`, before the `} else if (strcmp(cmd, "freq") == 0)` block (around line 170), add:

```cpp
    } else if (strcmp(cmd, "ir") == 0) {
        handleIR(rest);
```

**Step 4: Add help text entry in MBotCLI.cpp**

In `printHelp()`, before the `"fan forward|reverse..."` line (around line 215), add:

```
        "ir  (last received IR key + raw code)\n"
```

**Step 5: Add monitor target in handleMonitor()**

In `handleMonitor()`, after the `else if (strcmp(sub, "side") == 0)` line (around line 352), add:

```cpp
    else if (strcmp(sub, "ir") == 0)     _monTarget = MON_IR;
```

**Step 6: Add monitor output in tickMonitor()**

In `tickMonitor()`, before the `default:` case (around line 403), add:

```cpp
        case MON_IR: {
            IrKey key = _bot->ir.read();
            if (key != IrKey::NONE) {
                Serial.print(MBotIR::keyName(key));
                Serial.print(F("\t0x"));
                if (_bot->ir.rawCommand() < 0x10) Serial.print('0');
                Serial.println(_bot->ir.rawCommand(), HEX);
            }
            break;
        }
```

**Step 7: Write handleIR implementation**

At the end of MBotCLI.cpp (before `pushHistory`), add:

```cpp
void MBotCLI::handleIR(char* args) {
    (void)args;
    IrKey key = _bot->ir.read();
    if (key == IrKey::NONE) {
        Serial.println(F("no signal"));
    } else {
        Serial.print(MBotIR::keyName(key));
        Serial.print(F("  cmd=0x"));
        if (_bot->ir.rawCommand() < 0x10) Serial.print('0');
        Serial.print(_bot->ir.rawCommand(), HEX);
        Serial.print(F("  addr=0x"));
        if (_bot->ir.rawAddress() < 0x10) Serial.print('0');
        Serial.println(_bot->ir.rawAddress(), HEX);
    }
}
```

**Step 8: Update help text monitor line**

In `printHelp()`, update the monitor usage line to include `ir`:

Change:
```
"monitor accel|mic|temp|buttons|sonar|line|side\n"
```
To:
```
"monitor accel|mic|temp|buttons|sonar|line|side|ir\n"
```

**Step 9: Build**

Run: `pio run`
Expected: SUCCESS — full compile with IR driver linked in.

**Step 10: Commit**

```bash
git add src/MBotCLI.h src/MBotCLI.cpp
git commit -m "CLI: add 'ir' command and monitor target for IR remote"
```

---

### Task 6: Manual verification on hardware

**Files:** None (testing only)

**Step 1: Flash the board**

Run: `pio run -t upload`

**Step 2: Open serial monitor**

Run: `pio device monitor`

**Step 3: Test IR command**

Type `ir` — should print `no signal`.

**Step 4: Test with remote**

Point the mini NEC remote at the TL1838 receiver and press a button. Type `ir` — should print the key name + hex code.

**Step 5: Test monitor mode**

Type `monitor ir`, then press remote buttons. Key names and codes should stream.

**Step 6: Verify key mapping**

Press each of the 21 buttons and note the key names. If any show as `NONE` with a non-zero raw code, the command byte mapping in `mapCommand()` needs updating for your specific remote variant.

**Step 7: Final commit (if mapping changes needed)**

```bash
git add src/MBotIR.cpp
git commit -m "Fix IR key mapping for specific remote variant"
```
