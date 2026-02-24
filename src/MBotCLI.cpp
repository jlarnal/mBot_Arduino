#include "MBotCLI.h"
#include "mBot.h"
#include <string.h>

MBotCLI::MBotCLI()
    : _bot(nullptr), _initialized(false), _pos(0),
      _monTarget(MON_NONE), _lastMonitor(0),
      _histCount(0), _histIndex(0), _testCh(-1) {
    memset(_history, 0, sizeof(_history));
}

void MBotCLI::attach(MBot* bot) {
    _bot = bot;
}

void MBotCLI::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    Serial.begin(SERIAL_BAUD);
    while (!Serial) {}
    Serial.println(F("mBot CLI ready. Type 'help' for commands."));
    Serial.print(F("> "));
}

void MBotCLI::update() {
    ensureInit();

    // Read incoming serial characters
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\r') continue;  // ignore CR

        // Tab: cycle through command history
        if (c == '\t') {
            if (_histCount > 0) {
                // Copy history entry into _buf
                uint8_t idx = _histIndex % _histCount;
                strncpy(_buf, _history[idx], sizeof(_buf) - 1);
                _buf[sizeof(_buf) - 1] = '\0';
                clearLine();
                _histIndex++;
            }
            continue;
        }

        // Any non-Tab key resets history cycling
        _histIndex = 0;

        // Backspace / DEL
        if (c == 0x08 || c == 0x7F) {
            if (_pos > 0) {
                _pos--;
                Serial.print(F("\b \b"));
            }
            continue;
        }

        if (c == '\n') {
            Serial.print(F("\r\n"));

            // Interactive test mode: Enter advances to next channel
            if (_testCh >= 0) {
                testPcaStep();
                continue;
            }

            _buf[_pos] = '\0';
            if (_pos > 0) {
                // Save copy before dispatch (which modifies _buf via tokenizer)
                char saved[CLI_BUF_SIZE];
                strncpy(saved, _buf, sizeof(saved));
                saved[sizeof(saved) - 1] = '\0';
                _monTarget = MON_NONE;  // any command cancels monitor
                if (dispatch(_buf)) {
                    pushHistory(saved);
                }
            }
            _pos = 0;
            if (_monTarget == MON_NONE) Serial.print(F("> "));
            continue;
        }

        // Printable character: echo and store
        if (_pos < sizeof(_buf) - 1) {
            _buf[_pos++] = c;
            Serial.print(c);
        }
    }

    tickMonitor();
}

bool MBotCLI::dispatch(char* line) {
    char* rest = line;
    char* cmd = nextToken(&rest);
    if (!cmd) return false;

    if (strcmp(cmd, "help") == 0) {
        printHelp();
    } else if (strcmp(cmd, "motors") == 0) {
        handleMotors(rest);
    } else if (strcmp(cmd, "rgb") == 0) {
        handleRGB(rest);
    } else if (strcmp(cmd, "servo") == 0) {
        handleServo(rest);
    } else if (strcmp(cmd, "display") == 0) {
        handleDisplay(rest);
    } else if (strcmp(cmd, "beep") == 0) {
        _bot->speaker.beep();
        Serial.println(F("beep!"));
    } else if (strcmp(cmd, "tone") == 0) {
        char* sFreq = nextToken(&rest);
        char* sMs = nextToken(&rest);
        if (sFreq && sMs) {
            _bot->speaker.tone(parseInt(sFreq), parseInt(sMs));
            Serial.println(F("ok"));
        } else {
            Serial.println(F("usage: tone <freq> <ms>"));
        }
    } else if (strcmp(cmd, "notone") == 0) {
        _bot->speaker.noTone();
        Serial.println(F("ok"));
    } else if (strcmp(cmd, "buttons") == 0) {
        Serial.print(F("A="));
        Serial.print(_bot->buttons.a() ? "pressed" : "released");
        Serial.print(F("  B="));
        Serial.println(_bot->buttons.b() ? "pressed" : "released");
    } else if (strcmp(cmd, "temp") == 0) {
        Serial.print(_bot->temp.celsius());
        Serial.println(F(" C"));
    } else if (strcmp(cmd, "mic") == 0) {
        Serial.println(_bot->mic.level());
    } else if (strcmp(cmd, "sonar") == 0) {
        char* sN = nextToken(&rest);
        float d = sN ? _bot->sonar.read(parseInt(sN)) : _bot->sonar.read();
        if (d < 0.0f) {
            Serial.println(F("timeout"));
        } else {
            Serial.print(d, 3);
            Serial.println(F(" m"));
        }
    } else if (strcmp(cmd, "accel") == 0) {
        Serial.print(F("x="));
        Serial.print(_bot->accel.x());
        Serial.print(F("  y="));
        Serial.print(_bot->accel.y());
        Serial.print(F("  z="));
        Serial.print(_bot->accel.z());
        Serial.println(F(" mg"));
    } else if (strcmp(cmd, "line") == 0) {
        Serial.print(F("L="));
        Serial.print(_bot->line.left() ? "line" : "---");
        Serial.print(F("  R="));
        Serial.println(_bot->line.right() ? "line" : "---");
    } else if (strcmp(cmd, "side") == 0) {
        char* sub = nextToken(&rest);
        if (sub && strcmp(sub, "flame") == 0) {
            Serial.print(F("L="));
            Serial.print(_bot->side.leftFlame() ? "FIRE" : "ok");
            Serial.print(F("  R="));
            Serial.println(_bot->side.rightFlame() ? "FIRE" : "ok");
        } else {
            Serial.print(F("L="));
            Serial.print(_bot->side.leftRaw());
            Serial.print(F("  R="));
            Serial.println(_bot->side.rightRaw());
        }
    } else if (strcmp(cmd, "fan") == 0) {
        handleFan(rest);
    } else if (strcmp(cmd, "freq") == 0) {
        char* sHz = nextToken(&rest);
        if (sHz) {
            _bot->pca().setFreqHz(parseInt(sHz));
        }
        Serial.print(_bot->pca().freqHz());
        Serial.println(F(" Hz"));
    } else if (strcmp(cmd, "monitor") == 0) {
        handleMonitor(rest);
    } else if (strcmp(cmd, "test") == 0) {
        handleTest(rest);
    } else {
        Serial.print(F("unknown command: "));
        Serial.println(cmd);
        return false;
    }
    return true;
}

void MBotCLI::printHelp() {
    Serial.println(F(
        "--- mBot CLI ---\n"
        "motors forward|backward|left|right <speed>  (0-255)\n"
        "motors stop\n"
        "motors set <left> <right>  (-255..+255)\n"
        "rgb <r> <g> <b>  (0-255)\n"
        "rgb off\n"
        "servo <angle>  (0-180)\n"
        "servo pulse <us>\n"
        "servo off\n"
        "display char <c>\n"
        "display scroll <text>  (marquee)\n"
        "display scroll  (stop marquee)\n"
        "display clear\n"
        "display pixel <x> <y> <0|1>\n"
        "beep\n"
        "tone <freq> <ms>\n"
        "notone\n"
        "buttons\n"
        "temp\n"
        "mic\n"
        "sonar [samples]  (default 5, median-filtered)\n"
        "line  (line tracker L/R)\n"
        "side  (raw digital L/R)\n"
        "side flame  (flame detection L/R)\n"
        "fan forward|reverse [speed]  (0-4095)\n"
        "fan stop\n"
        "accel\n"
        "freq [hz]  (get/set PCA9685 PWM frequency)\n"
        "monitor accel|mic|temp|buttons|sonar|line|side\n"
        "monitor off\n"
        "test pca9685  (cycle outputs one by one)\n"
        "help"
    ));
}

void MBotCLI::handleMotors(char* args) {
    char* sub = nextToken(&args);
    if (!sub) { Serial.println(F("usage: motors <cmd> [args]")); return; }

    if (strcmp(sub, "forward") == 0) {
        char* s = nextToken(&args);
        _bot->motors.forward(s ? parseInt(s) : MOTOR_DEFAULT_SPEED);
    } else if (strcmp(sub, "backward") == 0) {
        char* s = nextToken(&args);
        _bot->motors.backward(s ? parseInt(s) : MOTOR_DEFAULT_SPEED);
    } else if (strcmp(sub, "left") == 0) {
        char* s = nextToken(&args);
        _bot->motors.left(s ? parseInt(s) : MOTOR_DEFAULT_SPEED);
    } else if (strcmp(sub, "right") == 0) {
        char* s = nextToken(&args);
        _bot->motors.right(s ? parseInt(s) : MOTOR_DEFAULT_SPEED);
    } else if (strcmp(sub, "stop") == 0) {
        _bot->motors.stop();
    } else if (strcmp(sub, "set") == 0) {
        char* sL = nextToken(&args);
        char* sR = nextToken(&args);
        if (sL && sR) {
            _bot->motors.set(parseInt(sL), parseInt(sR));
        } else {
            Serial.println(F("usage: motors set <left> <right>"));
            return;
        }
    } else {
        Serial.println(F("unknown motors command"));
        return;
    }
    Serial.println(F("ok"));
}

void MBotCLI::handleRGB(char* args) {
    char* first = nextToken(&args);
    if (!first) { Serial.println(F("usage: rgb <r> <g> <b> | rgb off")); return; }

    if (strcmp(first, "off") == 0) {
        _bot->rgb.off();
    } else {
        char* sG = nextToken(&args);
        char* sB = nextToken(&args);
        if (sG && sB) {
            _bot->rgb.setColor(parseInt(first), parseInt(sG), parseInt(sB));
        } else {
            Serial.println(F("usage: rgb <r> <g> <b>"));
            return;
        }
    }
    Serial.println(F("ok"));
}

void MBotCLI::handleServo(char* args) {
    char* first = nextToken(&args);
    if (!first) {
        Serial.println(F("usage: servo <angle> | servo pulse <us> | servo off"));
        return;
    }

    if (strcmp(first, "off") == 0) {
        _bot->servo.off();
    } else if (strcmp(first, "pulse") == 0) {
        char* sUs = nextToken(&args);
        if (sUs) {
            _bot->servo.setPulse(parseInt(sUs));
        } else {
            Serial.println(F("usage: servo pulse <us>"));
            return;
        }
    } else {
        // Treat as angle
        _bot->servo.setAngle(parseInt(first));
    }
    Serial.println(F("ok"));
}

void MBotCLI::handleDisplay(char* args) {
    char* sub = nextToken(&args);
    if (!sub) { Serial.println(F("usage: display char|clear|pixel|scroll")); return; }

    if (strcmp(sub, "char") == 0) {
        char* s = nextToken(&args);
        if (s) {
            _bot->display.showChar(s[0]);
            Serial.println(F("ok"));
        } else {
            Serial.println(F("usage: display char <c>"));
        }
    } else if (strcmp(sub, "scroll") == 0) {
        // Remaining args is the text to scroll (rest of line after "scroll ")
        if (args && args[0]) {
            _bot->display.showString(args);
            Serial.println(F("ok"));
        } else {
            _bot->display.showString(nullptr);
            Serial.println(F("marquee stopped"));
        }
    } else if (strcmp(sub, "clear") == 0) {
        _bot->display.clear();
        Serial.println(F("ok"));
    } else if (strcmp(sub, "pixel") == 0) {
        char* sX = nextToken(&args);
        char* sY = nextToken(&args);
        char* sOn = nextToken(&args);
        if (sX && sY && sOn) {
            _bot->display.setPixel(parseInt(sX), parseInt(sY), parseInt(sOn));
            Serial.println(F("ok"));
        } else {
            Serial.println(F("usage: display pixel <x> <y> <0|1>"));
        }
    } else {
        Serial.println(F("unknown display command"));
    }
}

void MBotCLI::handleMonitor(char* args) {
    char* sub = nextToken(&args);
    if (!sub) { Serial.println(F("usage: monitor accel|mic|temp|buttons|sonar|off")); return; }

    if (strcmp(sub, "accel") == 0)        _monTarget = MON_ACCEL;
    else if (strcmp(sub, "mic") == 0)     _monTarget = MON_MIC;
    else if (strcmp(sub, "temp") == 0)    _monTarget = MON_TEMP;
    else if (strcmp(sub, "buttons") == 0) _monTarget = MON_BUTTONS;
    else if (strcmp(sub, "sonar") == 0)   _monTarget = MON_SONAR;
    else if (strcmp(sub, "line") == 0)    _monTarget = MON_LINE;
    else if (strcmp(sub, "side") == 0)    _monTarget = MON_SIDE;
    else if (strcmp(sub, "off") == 0)   { _monTarget = MON_NONE; Serial.println(F("monitor off")); }
    else { Serial.println(F("unknown monitor target")); return; }

    if (_monTarget != MON_NONE) {
        Serial.print(F("monitoring "));
        Serial.print(sub);
        Serial.println(F(" (send any command to stop)"));
        _lastMonitor = 0;  // trigger immediate first reading
    }
}

void MBotCLI::tickMonitor() {
    if (_monTarget == MON_NONE) return;
    unsigned long now = millis();
    if (now - _lastMonitor < MONITOR_INTERVAL_MS) return;
    _lastMonitor = now;

    switch (_monTarget) {
        case MON_ACCEL:
            Serial.print(_bot->accel.x());
            Serial.print(F("\t"));
            Serial.print(_bot->accel.y());
            Serial.print(F("\t"));
            Serial.println(_bot->accel.z());
            break;
        case MON_MIC:
            Serial.println(_bot->mic.level());
            break;
        case MON_TEMP:
            Serial.print(_bot->temp.celsius());
            Serial.println(F(" C"));
            break;
        case MON_BUTTONS:
            Serial.print(_bot->buttons.a() ? "A " : "_ ");
            Serial.println(_bot->buttons.b() ? "B" : "_");
            break;
        case MON_SONAR: {
            float d = _bot->sonar.read(1);  // single sample for speed
            if (d < 0.0f) Serial.println(F("timeout"));
            else { Serial.print(d, 3); Serial.println(F(" m")); }
            break;
        }
        case MON_LINE:
            Serial.print(_bot->line.left() ? "L " : "_ ");
            Serial.println(_bot->line.right() ? "R" : "_");
            break;
        case MON_SIDE:
            Serial.print(_bot->side.leftRaw() ? "L " : "_ ");
            Serial.println(_bot->side.rightRaw() ? "R" : "_");
            break;
        default:
            break;
    }
}

void MBotCLI::handleTest(char* args) {
    char* sub = nextToken(&args);
    if (!sub || strcmp(sub, "pca9685") != 0) {
        Serial.println(F("usage: test pca9685"));
        return;
    }
    // Turn all channels off
    for (uint8_t ch = 0; ch < PCA9685_NUM_CHANNELS; ch++) {
        _bot->pca().setChannelOff(ch);
    }
    _testCh = 0;
    _bot->pca().setChannel(_testCh, PCA9685_PWM_MAX);
    Serial.print(F("PCA9685 test — CH 0 HIGH.  Press [Enter] to advance, "));
    Serial.println(F("any other key to abort."));
}

void MBotCLI::testPcaStep() {
    // Turn off current channel
    _bot->pca().setChannelOff(_testCh);
    _testCh++;

    if (_testCh >= PCA9685_NUM_CHANNELS) {
        _testCh = -1;
        Serial.println(F("PCA9685 test complete — all channels off."));
        Serial.print(F("> "));
        return;
    }

    _bot->pca().setChannel(_testCh, PCA9685_PWM_MAX);
    Serial.print(F("CH "));
    Serial.print(_testCh);
    Serial.println(F(" HIGH.  [Enter] = next"));
}

void MBotCLI::handleFan(char* args) {
    char* sub = nextToken(&args);
    if (!sub) { Serial.println(F("usage: fan forward|reverse [speed] | fan stop")); return; }

    if (strcmp(sub, "stop") == 0) {
        _bot->fan.stop();
    } else if (strcmp(sub, "forward") == 0) {
        char* s = nextToken(&args);
        _bot->fan.forward(s ? parseInt(s) : FAN_SPEED_MAX);
    } else if (strcmp(sub, "reverse") == 0) {
        char* s = nextToken(&args);
        _bot->fan.reverse(s ? parseInt(s) : FAN_SPEED_MAX);
    } else {
        Serial.println(F("unknown fan command"));
        return;
    }
    Serial.println(F("ok"));
}

void MBotCLI::pushHistory(const char* cmd) {
    // Shift entries down, store newest at [0]
    uint8_t slots = _histCount < HISTORY_COUNT ? _histCount : HISTORY_COUNT - 1;
    for (uint8_t i = slots; i > 0; i--) {
        memcpy(_history[i], _history[i - 1], sizeof(_history[0]));
    }
    strncpy(_history[0], cmd, sizeof(_history[0]) - 1);
    _history[0][sizeof(_history[0]) - 1] = '\0';
    if (_histCount < HISTORY_COUNT) _histCount++;
}

void MBotCLI::clearLine() {
    // Erase current line on terminal: overwrite with spaces, then reprint buffer
    Serial.print('\r');
    Serial.print(F("> "));
    for (uint8_t i = 0; i < sizeof(_buf) - 1; i++) Serial.print(' ');
    Serial.print('\r');
    Serial.print(F("> "));
    _pos = strlen(_buf);
    Serial.print(_buf);
}

char* MBotCLI::nextToken(char** rest) {
    char* p = *rest;
    while (*p == ' ') p++;      // skip leading spaces
    if (*p == '\0') return nullptr;
    char* start = p;
    while (*p && *p != ' ') p++;  // find end of token
    if (*p) *p++ = '\0';         // null-terminate
    *rest = p;
    return start;
}

int MBotCLI::parseInt(const char* s) {
    return atoi(s);
}
