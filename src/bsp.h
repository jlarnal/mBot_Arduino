/**
 * @file bsp.h
 * @brief Board Support Package — MakeBit carrier + BBC micro:bit v2.
 *
 * Centralised hardware constants for the mBot library.  Every pin assignment,
 * I2C address, PWM parameter and peripheral default lives here so that
 * board-level changes only need a single-file edit.
 */
#pragma once

#include <Arduino.h>

/// @defgroup bsp_pca9685 PCA9685 PWM Controller
/// @{
static constexpr uint8_t  PCA9685_I2C_ADDR   = 0x41;  ///< I2C slave address of the PCA9685.
static constexpr uint8_t  PCA9685_PRESCALE    = 121;   ///< Default prescaler value (~50 Hz PWM).
static constexpr uint16_t PCA9685_PWM_MAX     = 4095;  ///< Maximum 12-bit PWM count.
static constexpr uint16_t PCA9685_OSC_STARTUP_US = 500; ///< Oscillator settle time (µs).
static constexpr uint8_t  PCA9685_NUM_CHANNELS   = 16; ///< Number of PWM output channels.
/// @}

/// @defgroup bsp_motors Motors (PCA9685 channels, MX1508 H-bridge)
/// @{
static constexpr uint8_t  MOTOR_LEFT_FWD_CH   = 13;   ///< PCA9685 channel for left motor forward (L_INA).
static constexpr uint8_t  MOTOR_LEFT_REV_CH   = 12;   ///< PCA9685 channel for left motor reverse (L_INB).
static constexpr uint8_t  MOTOR_RIGHT_FWD_CH  = 14;   ///< PCA9685 channel for right motor forward (R_INA).
static constexpr uint8_t  MOTOR_RIGHT_REV_CH  = 15;   ///< PCA9685 channel for right motor reverse (R_INB).
static constexpr int16_t  MOTOR_SPEED_MAX     = 255;   ///< Maximum motor speed value (maps to full PWM).
static constexpr uint8_t  MOTOR_DEFAULT_SPEED = 80;    ///< Default speed used by CLI convenience commands.
/// @}

/// @defgroup bsp_rgb RGB LEDs (PCA9685 channels)
/// @{
static constexpr uint8_t  RGB_CH_R = 0;               ///< PCA9685 channel for the red LED.
static constexpr uint8_t  RGB_CH_G = 1;               ///< PCA9685 channel for the green LED.
static constexpr uint8_t  RGB_CH_B = 2;               ///< PCA9685 channel for the blue LED.
static constexpr uint8_t  RGB_VALUE_MAX = 255;         ///< Maximum per-channel colour value.
/// @}

/// @defgroup bsp_servo RC servos (PCA9685 channels)
/// @{
static constexpr uint8_t  SERVO_CH_S1      = 3;       ///< PCA9685 channel for servo S1.
static constexpr int8_t   SERVO_TRIM_DEG   = -5;      ///< Mechanical trim: servo spline offset from chassis axis (degrees).
static constexpr uint16_t SERVO_PULSE_MIN_US  = 500;  ///< Minimum pulse width (µs) — maps to 0°.
static constexpr uint16_t SERVO_PULSE_MAX_US  = 2500; ///< Maximum pulse width (µs) — maps to 180°.
static constexpr uint16_t SERVO_ANGLE_MAX     = 180;  ///< Maximum angle in degrees.
static constexpr uint8_t  SERVO_DEFAULT_ANGLE = 90;   ///< Default centre position (degrees).
/// @}

/// @defgroup bsp_fan Auxiliary fan motor (PCA9685 channels, L9110S H-bridge)
/// @{
static constexpr uint8_t  FAN_FWD_CH  = 4;            ///< PCA9685 channel for fan forward.
static constexpr uint8_t  FAN_REV_CH  = 5;            ///< PCA9685 channel for fan reverse.
static constexpr int16_t  FAN_SPEED_MAX = 4095;       ///< Maximum fan speed (12-bit PWM).
/// @}

/// @defgroup bsp_line Line tracker (digital GPIO)
/// @{
static constexpr uint8_t  LINE_LEFT_PIN  = 1;         ///< GPIO pin for left line sensor.
static constexpr uint8_t  LINE_RIGHT_PIN = 2;         ///< GPIO pin for right line sensor.
/// @}

/// @defgroup bsp_side Side sensors (digital GPIO, swappable IR / LDR / flame)
/// @{
static constexpr uint8_t  SIDE_LEFT_PIN  = 13;        ///< GPIO pin for left side sensor.
static constexpr uint8_t  SIDE_RIGHT_PIN = 12;        ///< GPIO pin for right side sensor.
/// @}


/// @defgroup bsp_display 5×5 LED matrix (micro:bit v2 display)
/// @{
static constexpr uint8_t  DISPLAY_ROWS = 5;           ///< Number of display rows.
static constexpr uint8_t  DISPLAY_COLS = 5;           ///< Number of display columns.
static constexpr uint8_t  DISPLAY_ROW_PINS[DISPLAY_ROWS] = { 21, 22, 23, 24, 25 }; ///< GPIO pins driving each row.
static constexpr uint8_t  DISPLAY_COL_PINS[DISPLAY_COLS] = { 4, 7, 3, 6, 10 };     ///< GPIO pins driving each column.
static constexpr uint16_t DISPLAY_ROW_DWELL_US = 500;  ///< Per-row persistence time (µs) during multiplexing.
static constexpr uint8_t  DISPLAY_GLYPH_WIDTH = 5;    ///< Width of each font glyph in columns.
static constexpr uint8_t  DISPLAY_CHAR_GAP    = 1;    ///< Blank columns between characters in marquee.
static constexpr uint16_t DISPLAY_SCROLL_MS   = 100;  ///< Default marquee speed (ms per column shift).
static constexpr uint16_t SHOWSTRING_MARQUEE_DEFAULT_MS = 100; ///< Default marquee speed (ms per column shift).
/// @}

/// @defgroup bsp_sonar Ultrasonic sonar (HC-SR04 style)
/// @{
static constexpr uint8_t  SONAR_TRIG_PIN       = 14;    ///< GPIO pin for trigger pulse.
static constexpr uint8_t  SONAR_ECHO_PIN       = 15;    ///< GPIO pin for echo input.
static constexpr uint16_t SONAR_US_PER_CM      = 58;    ///< Round-trip µs per centimetre (≈343 m/s).
static constexpr uint32_t SONAR_TIMEOUT_US     = 25000; ///< Echo timeout (µs) — caps at ~4.3 m.
static constexpr float    SONAR_MAX_RANGE_M    = 4.0f;  ///< Maximum measurable distance (m).
static constexpr uint8_t  SONAR_DEFAULT_SAMPLES = 5;    ///< Default number of samples for filtered reads.
static constexpr uint8_t  SONAR_MAX_SAMPLES    = 15;    ///< Maximum samples per filtered read.
/// @}

/// @defgroup bsp_speaker Speaker
/// @{
static constexpr uint8_t  SPEAKER_PIN       = 0;      ///< GPIO pin connected to the speaker.
static constexpr uint16_t BEEP_FREQ_HZ      = 1000;   ///< Default beep frequency (Hz).
static constexpr uint32_t BEEP_DURATION_MS   = 100;   ///< Default beep duration (ms).
static constexpr uint32_t TONE_BURST_MS      = 100;   ///< Continuous-tone fallback burst length (ms).
/// @}

/// @defgroup bsp_mic Microphone
/// @{
static constexpr uint8_t  MIC_PIN    = 29;             ///< Analogue input pin (P0.05).
static constexpr uint8_t  MIC_EN_PIN = 20;             ///< Microphone enable control pin.
/// @}

/// @defgroup bsp_accel Accelerometer (LSM303AGR on Wire1)
/// @{
static constexpr uint8_t  LSM303_ACCEL_ADDR  = 0x19;  ///< I2C address of the LSM303AGR accelerometer.
static constexpr unsigned long ACCEL_READ_INTERVAL_MS = 10; ///< Minimum interval between readings (ms).
static constexpr uint8_t  ACCEL_SETTLE_MS    = 10;    ///< Post-init settle time (ms).
static constexpr uint8_t  ACCEL_MG_PER_LSB   = 4;     ///< Milligravity per LSB at ±2 g after >>6.
/// @}

/// @defgroup bsp_temp Temperature sensor (nRF52833 TEMP peripheral)
/// @{
static constexpr float    TEMP_UNIT_CELSIUS  = 0.25f;  ///< Degrees Celsius per raw TEMP unit.
/// @}

/// @defgroup bsp_serial Serial / CLI
/// @{
static constexpr uint32_t SERIAL_BAUD        = 115200; ///< Serial port baud rate.
static constexpr uint8_t  CLI_BUF_SIZE       = 64;     ///< Command input buffer size (bytes).
static constexpr uint8_t  HISTORY_COUNT      = 3;      ///< Number of remembered commands.
static constexpr uint8_t  HISTORY_SIZE       = CLI_BUF_SIZE; ///< Bytes per history entry.
static constexpr uint16_t MONITOR_INTERVAL_MS = 200;   ///< Polling interval for `monitor` output (ms).
/// @}

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
static constexpr uint8_t  IR_NEC_EDGES       = 68;    ///< Edges per complete NEC frame.
/// @}
