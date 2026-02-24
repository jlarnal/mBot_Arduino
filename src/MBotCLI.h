/**
 * @file MBotCLI.h
 * @brief Interactive serial command-line interface for the mBot.
 *
 * Provides a human-friendly text interface over the UART serial port.
 * Features include local echo, backspace editing, Tab-based command
 * history cycling, a continuous sensor monitor mode, and an interactive
 * PCA9685 channel test.
 *
 * Call update() from loop() to process incoming characters and service
 * the monitor output.
 */
#pragma once

#include <Arduino.h>
#include "bsp.h"

class MBot;

/**
 * @class MBotCLI
 * @brief Serial command-line interface for controlling all mBot subsystems.
 */
class MBotCLI {
public:
    MBotCLI();

    /**
     * @brief Bind the CLI to an MBot instance.
     * @param bot Pointer to the owning MBot object.
     */
    void attach(MBot* bot);

    /**
     * @brief Process serial input and service monitor output.
     *
     * Must be called frequently from loop().  Handles character echo,
     * line editing, command dispatch, and periodic monitor updates.
     */
    void update();

private:
    MBot* _bot;           ///< Back-pointer to the owning MBot.
    bool _initialized;    ///< @c true after Serial.begin() has been called.

    char _buf[CLI_BUF_SIZE]; ///< Current command input buffer.
    uint8_t _pos;            ///< Write position within @ref _buf.

    /** @brief Sensor targets for the continuous monitor mode. */
    enum MonitorTarget : uint8_t {
        MON_NONE,     ///< Monitor off.
        MON_ACCEL,    ///< Accelerometer (x / y / z).
        MON_MIC,      ///< Microphone level.
        MON_TEMP,     ///< Temperature.
        MON_BUTTONS,  ///< Button A / B state.
        MON_SONAR,    ///< Ultrasonic distance (m).
        MON_LINE,     ///< Line tracker (left / right).
        MON_SIDE      ///< Side sensors (left / right).
    };
    MonitorTarget _monTarget;    ///< Currently active monitor target.
    unsigned long _lastMonitor;  ///< Timestamp of the last monitor print (ms).

    /// @name Command history
    /// @{
    char _history[HISTORY_COUNT][HISTORY_SIZE]; ///< Ring buffer of recent commands.
    uint8_t _histCount;  ///< Number of filled history slots (0–@ref HISTORY_COUNT).
    uint8_t _histIndex;  ///< Current Tab cycling position.
    /// @}

    /// @name Interactive PCA9685 channel test
    /// @{
    int8_t _testCh;      ///< Current test channel (−1 = not testing, 0–15 = active).

    /**
     * @brief Start the PCA9685 interactive channel walk-through.
     * @param args Remaining tokens after "test" (expects "pca9685").
     */
    void handleTest(char* args);

    /** @brief Advance the PCA9685 test to the next channel. */
    void testPcaStep();
    /// @}

    /**
     * @brief Push a command string into the history ring buffer.
     * @param cmd Null-terminated command to store.
     */
    void pushHistory(const char* cmd);

    /** @brief Erase the current terminal line and reprint @ref _buf. */
    void clearLine();

    /** @brief Lazily initialise the serial port and print the welcome banner. */
    void ensureInit();

    /**
     * @brief Parse and execute a command line.
     * @param line Mutable buffer containing the command (will be tokenised).
     * @return @c true if the command was recognised, @c false otherwise.
     */
    bool dispatch(char* line);

    /** @brief Print the help text listing all available commands. */
    void printHelp();

    /**
     * @brief Handle the "motors" family of sub-commands.
     * @param args Remaining tokens after "motors".
     */
    void handleMotors(char* args);

    /**
     * @brief Handle the "rgb" command.
     * @param args Remaining tokens after "rgb".
     */
    void handleRGB(char* args);

    /**
     * @brief Handle the "servo" command.
     * @param args Remaining tokens after "servo".
     */
    void handleServo(char* args);

    /**
     * @brief Handle the "display" family of sub-commands.
     * @param args Remaining tokens after "display".
     */
    void handleDisplay(char* args);

    /**
     * @brief Handle the "monitor" command.
     * @param args Remaining tokens after "monitor".
     */
    void handleMonitor(char* args);

    /**
     * @brief Handle the "fan" command.
     * @param args Remaining tokens after "fan".
     */
    void handleFan(char* args);

    /** @brief Print one monitor sample if the interval has elapsed. */
    void tickMonitor();

    /**
     * @brief Extract the next whitespace-delimited token from a string.
     * @param[in,out] rest Pointer to the remaining unparsed input;
     *                     advanced past the returned token on exit.
     * @return Pointer to the token, or @c nullptr if none remain.
     */
    static char* nextToken(char** rest);

    /**
     * @brief Parse a decimal integer from a string.
     * @param s Null-terminated string.
     * @return The parsed integer value.
     */
    static int parseInt(const char* s);
};
