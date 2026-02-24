# mBot Arduino Library

Arduino library for the **BBC micro:bit v2** + **MakeBit carrier board** robot kit.

Provides high-level drivers for every on-board peripheral, a centralised board-support header (`bsp.h`), and an interactive serial CLI for testing and control — all fully documented with Doxygen.

## Hardware

| Component | IC / Peripheral | Interface |
|---|---|---|
| PWM controller | PCA9685 (16-ch, 12-bit) | I2C @ 0x41 |
| Drive motors (x2) | MX1508 dual H-bridge | PCA9685 CH 12-15 |
| Fan / aux motor | L9110S H-bridge | PCA9685 CH 4-5 |
| RGB LED | -- | PCA9685 CH 0-2 |
| Servo (S1) | Standard RC (sonar pan) | PCA9685 CH 3 |
| Ultrasonic sonar | HC-SR04 style | GPIO pin 14 (trig) / pin 15 (echo) |
| Line tracker | IR reflectance (L/R) | GPIO pin 1 (left) / pin 2 (right) |
| Side sensors | Swappable IR / LDR / flame (L/R) | GPIO pin 13 (left) / pin 12 (right) |
| LED matrix | 5x5 (micro:bit built-in) | GPIO row/col multiplex |
| Speaker | On-board piezo | GPIO pin 0 |
| Microphone | MEMS (micro:bit built-in) | Analogue pin 29 + enable pin 20 |
| Accelerometer | LSM303AGR | I2C (Wire1) @ 0x19 |
| Temperature | nRF52833 TEMP peripheral | SoC register |
| Buttons | A & B (micro:bit built-in) | GPIO (active-low) |

## Modules

| Class | File | Description |
|---|---|---|
| `MBot` | `mBot.h` | Top-level facade -- aggregates all subsystems |
| `PCA9685` | `PCA9685.h` | 16-channel PWM driver with runtime frequency control |
| `MBotMotors` | `MBotMotors.h` | Dual DC motor control (forward/backward/left/right/stop, or per-motor signed speed) |
| `MBotRGB` | `MBotRGB.h` | RGB LED colour control |
| `MBotServo` | `MBotServo.h` | RC servo positioning (angle or raw pulse width) for S1 (sonar pan) |
| `MBotFan` | `MBotFan.h` | Auxiliary DC motor / fan via L9110S H-bridge |
| `MBotDisplay` | `MBotDisplay.h` | 5x5 LED matrix with character display, pixel access, and **non-blocking marquee scrolling** (hardware timer ISR) |
| `MBotSonar` | `MBotSonar.h` | Ultrasonic distance sensor with median filtering (metres) |
| `MBotLineTracker` | `MBotLineTracker.h` | Under-chassis line-following sensor (left/right digital) |
| `MBotSideSensors` | `MBotSideSensors.h` | Side-mounted multi-purpose sensors (IR / LDR / flame, swappable) |
| `MBotSpeaker` | `MBotSpeaker.h` | Tone generation and beep |
| `MBotMic` | `MBotMic.h` | Microphone level reading |
| `MBotAccel` | `MBotAccel.h` | 3-axis accelerometer (milli-g), shake detection |
| `MBotTemp` | `MBotTemp.h` | Die temperature in Celsius |
| `MBotButtons` | `MBotButtons.h` | Button A/B state reading |
| `MBotCLI` | `MBotCLI.h` | Serial command-line interface with echo, history, and monitor modes |
| -- | `bsp.h` | Board Support Package -- all pin assignments, I2C addresses, and tuning constants in one place |

## Installation

### As a PlatformIO library dependency

Add to your `platformio.ini`:

```ini
[env:bbcmicrobit_v2]
platform = nordicnrf52
board = bbcmicrobit_v2
framework = arduino
lib_deps =
    https://github.com/jlarnal/mBot_Arduino.git
```

### As a local library

Clone into your project's `lib/` directory:

```bash
cd your_project/lib
git clone https://github.com/jlarnal/mBot_Arduino.git mBot
```

## Quick Start

```cpp
#include <Arduino.h>
#include "mBot.h"

MBot bot;

void setup() { }

void loop() {
    bot.cli.update();       // process serial commands
    bot.display.refresh();  // maintain LED matrix multiplexing
}
```

That's it -- the CLI gives you immediate interactive access to everything. Open a serial monitor at **115200 baud** and type `help`.

## Programmatic API Examples

```cpp
// Drive motors
bot.motors.forward(100);          // both forward at speed 100
bot.motors.set(-80, 80);          // spin left
bot.motors.stop();

// Fan / auxiliary motor
bot.fan.forward(2048);            // half speed forward
bot.fan.reverse();                // full speed reverse
bot.fan.set(-1000);               // signed speed (negative = reverse)
bot.fan.stop();

// RGB LED
bot.rgb.setColor(255, 0, 128);    // purple-ish
bot.rgb.off();

// Servo (S1, sonar pan)
bot.servo.setAngle(90);           // centre
bot.servo.setPulse(1500);         // raw pulse (us)
bot.servo.off();

// Display
bot.display.showChar('A');
bot.display.showString("HELLO");   // non-blocking marquee scroll
bot.display.showString("INV", 150, true);  // inverted, 150 ms/column
bot.display.showString(nullptr);   // stop marquee
bot.display.setPixel(2, 2, true);  // centre pixel on
bot.display.clear();

// Speaker
bot.speaker.beep();
bot.speaker.tone(440, 500);       // A4 for 500 ms
bot.speaker.noTone();

// Sensors
float dist = bot.sonar.read();         // metres, median-filtered (5 samples)
float raw  = bot.sonar.readRaw();      // single unfiltered reading
bool lineL = bot.line.left();          // line tracker
bool lineR = bot.line.right();
bool flame = bot.side.leftFlame();     // flame detection (active-low inverted)
bool sideR = bot.side.rightRaw();      // raw digital read
float temp = bot.temp.celsius();
uint16_t mic = bot.mic.level();        // 0-1023
int16_t ax = bot.accel.x();            // milli-g
bool shaking = bot.accel.isShaking();
bool btnA = bot.buttons.a();

// PCA9685 frequency
bot.pca().setFreqHz(50);           // 50 Hz (default, good for servos)
bot.pca().setFreqHz(1000);         // 1 kHz (better for motors/LEDs)
uint16_t hz = bot.pca().freqHz();  // read back
```

## Serial CLI

Connect at **115200 baud**. Commands:

```
motors forward|backward|left|right <speed>  (0-255)
motors stop
motors set <left> <right>  (-255..+255)
fan forward|reverse [speed]  (0-4095)
fan stop
rgb <r> <g> <b>  (0-255)
rgb off
servo <angle>  (0-180)
servo pulse <us>
servo off
display char <c>
display scroll <text>  (marquee)
display scroll  (stop marquee)
display clear
display pixel <x> <y> <0|1>
beep
tone <freq> <ms>
notone
buttons
temp
mic
sonar [samples]  (default 5, median-filtered)
line  (line tracker L/R)
side  (raw digital L/R)
side flame  (flame detection L/R)
accel
freq [hz]  (get/set PCA9685 PWM frequency)
monitor accel|mic|temp|buttons|sonar|line|side
monitor off
test pca9685  (cycle outputs one by one)
help
```

Features:
- **Local echo** with backspace editing
- **Tab** cycles through the last 3 successful commands
- **Monitor mode** prints sensor readings continuously (any command stops it)
- **PCA9685 test** walks through all 16 channels one by one on Enter

## Board Support Package (`bsp.h`)

All hardware constants are centralised in a single header. To adapt the library to a different carrier board or wiring, edit `bsp.h` only:

| Group | Constants |
|---|---|
| PCA9685 | I2C address, prescaler, PWM resolution, oscillator settle time |
| Drive motors | PCA9685 channel assignments (L/R, FWD/REV), speed limits |
| Fan motor | PCA9685 channel assignments (FWD/REV) |
| RGB | PCA9685 channel assignments (R/G/B) |
| Servo | PCA9685 channel (S1), pulse range, angle limits |
| Sonar | Trigger/echo GPIO pins, timing constants, sample limits |
| Line tracker | Left/right GPIO pins |
| Side sensors | Left/right GPIO pins |
| Display | Row/column GPIO pins, dwell time, glyph width, marquee defaults |
| Speaker | GPIO pin, beep frequency/duration |
| Microphone | Analogue + enable pins |
| Accelerometer | I2C address, read interval, sensitivity |
| Temperature | Conversion factor |
| Serial/CLI | Baud rate, buffer sizes, history depth, monitor interval |

## Architecture Notes

- **Lazy initialisation** -- every driver defers its hardware setup until first use (no `begin()` calls needed).
- **Non-blocking** -- the marquee uses nRF52 TIMER2 with an ISR; the CLI is polled. No blocking delays in the main loop.
- **Single PCA9685 instance** -- owned by `MBot`, shared by motors, RGB, servo, and fan via pointer.
- **Doxygen documented** -- every class, method, and constant has documentation comments.

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
pio run                    # compile
pio run -t upload          # flash via USB
pio device monitor         # serial monitor (115200 baud)
```

## License

MIT
