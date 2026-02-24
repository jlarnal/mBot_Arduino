# MBotIR — IR Remote Receiver Design

**Date:** 2026-02-24
**Status:** Approved

## Summary

Add `MBotIR` driver for a TL1838 IR receiver on pin P8, decoding NEC protocol frames from the common 21-key mini remote. Interrupt-driven, non-blocking, zero external dependencies.

## Hardware

- **Receiver:** TL1838 (or compatible VS1838B) — 38 kHz demodulating IR receiver
- **Pin:** P8 on BBC micro:bit edge connector
- **Remote:** 21-key mini NEC remote (commonly bundled with Arduino kits)

## Approach

GPIO interrupt (`CHANGE`) on P8 timestamps every edge with `micros()`. An ISR fills a fixed buffer of 66 edge timestamps (one complete NEC frame). `read()` decodes the timing data into address + command bytes, validates via inverse bytes, and maps the command to a named `IrKey` enum value.

No hardware timers are used — avoids conflict with MBotDisplay's TIMER2.

### NEC Protocol

1. **Leader:** 9 ms mark + 4.5 ms space
2. **32 data bits:** address (8) + ~address (8) + command (8) + ~command (8)
3. **Bit encoding:** 562 us mark, then 562 us space (0) or 1687 us space (1)
4. **Repeat code:** 9 ms mark + 2.25 ms space + 562 us mark (held button)

## Public API

```cpp
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

class MBotIR {
public:
    IrKey   read();          // Decoded key or IrKey::NONE
    uint8_t rawCommand();    // Last raw NEC command byte
    uint8_t rawAddress();    // Last raw NEC address byte
};
```

## BSP Constants (`bsp.h`)

```cpp
static constexpr uint8_t  IR_PIN           = 8;
static constexpr uint16_t IR_NEC_HDR_MARK  = 9000;   // us
static constexpr uint16_t IR_NEC_HDR_SPACE = 4500;
static constexpr uint16_t IR_NEC_BIT_MARK  = 562;
static constexpr uint16_t IR_NEC_ONE_SPACE = 1687;
static constexpr uint16_t IR_NEC_ZERO_SPACE = 562;
static constexpr uint16_t IR_NEC_RPT_SPACE = 2250;
static constexpr uint8_t  IR_NEC_TOLERANCE = 25;     // percent
static constexpr uint8_t  IR_NEC_EDGES     = 66;
```

## Internal Design

- **Global pointer:** `static MBotIR* _g_irInstance` (same pattern as `MBotDisplay`)
- **ISR:** Records `micros()` timestamp on each edge into `volatile uint32_t _edges[66]`; sets `volatile bool _frameReady` when buffer is full
- **Lazy init:** `ensureInit()` calls `pinMode(INPUT)` and `attachInterrupt(CHANGE)` on first use
- **Repeat handling:** Repeat codes (short space after leader mark) return the last successfully decoded key
- **Validation:** Address and command bytes are checked against their inverse bytes; mismatches return `IrKey::NONE`

## Key Code Mapping

The 21-key mini remote NEC command bytes mapped to `IrKey` values. Exact hex codes to be determined empirically during implementation (common mapping: 0x45=CH-, 0x46=CH, 0x47=CH+, etc.).

## CLI Integration

- `ir` command prints last received key name + raw hex code
- Monitor mode includes IR readings in periodic output

## Files

| File | Change |
|------|--------|
| `src/bsp.h` | Add IR constants |
| `src/MBotIR.h` | New file |
| `src/MBotIR.cpp` | New file |
| `src/mBot.h` | Add `MBotIR ir` member |
| `src/MBotCLI.cpp` | Add `ir` command |
