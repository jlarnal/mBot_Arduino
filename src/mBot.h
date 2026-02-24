/**
 * @file mBot.h
 * @brief Top-level façade that aggregates every mBot subsystem.
 *
 * A single MBot instance owns the PCA9685 PWM controller and exposes all
 * peripheral drivers (motors, RGB, display, buttons, accelerometer,
 * speaker, microphone, temperature) plus the serial CLI as public members.
 */
#pragma once

#include "PCA9685.h"
#include "MBotMotors.h"
#include "MBotRGB.h"
#include "MBotServo.h"
#include "MBotDisplay.h"
#include "MBotButtons.h"
#include "MBotAccel.h"
#include "MBotSpeaker.h"
#include "MBotMic.h"
#include "MBotSonar.h"
#include "MBotTemp.h"
#include "MBotCLI.h"

/**
 * @class MBot
 * @brief Root object for the mBot — owns the PCA9685 and all subsystem drivers.
 *
 * Usage:
 * @code
 * MBot bot;
 * void setup() { }
 * void loop()  { bot.cli.update(); bot.display.refresh(); }
 * @endcode
 */
class MBot {
    PCA9685 _pca;  ///< Shared PWM controller (constructed first; motors & rgb depend on it).

public:
    MBot();

    /**
     * @brief Access the underlying PCA9685 driver.
     * @return Reference to the PCA9685 instance.
     */
    PCA9685& pca() { return _pca; }

    MBotMotors  motors;   ///< Dual DC motor driver.
    MBotRGB     rgb;      ///< RGB LED driver.
    MBotServo   servo;    ///< RC servo driver (S1–S3).
    MBotDisplay display;  ///< 5×5 LED matrix.
    MBotButtons buttons;  ///< User buttons A & B.
    MBotAccel   accel;    ///< 3-axis accelerometer.
    MBotSpeaker speaker;  ///< On-board speaker.
    MBotMic     mic;      ///< MEMS microphone.
    MBotSonar   sonar;    ///< Ultrasonic distance sensor.
    MBotTemp    temp;     ///< Die temperature sensor.
    MBotCLI     cli;      ///< Serial command-line interface.
};
