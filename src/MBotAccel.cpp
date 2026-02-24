#include "MBotAccel.h"
#include "bsp.h"

// Registers
static constexpr uint8_t CTRL_REG1_A = 0x20;
static constexpr uint8_t CTRL_REG4_A = 0x23;
static constexpr uint8_t OUT_X_L_A   = 0x28;

// Config: 100 Hz, all axes enabled, normal mode, +/-2g
static constexpr uint8_t CTRL_REG1_VAL = 0x57;  // 100Hz, normal, XYZ
static constexpr uint8_t CTRL_REG4_VAL = 0x00;  // +/-2g, HR off


MBotAccel::MBotAccel()
    : _initialized(false), _x(0), _y(0), _z(0), _lastRead(0) {}

void MBotAccel::ensureInit() {
    if (_initialized) return;
    _initialized = true;

    Wire1.begin();
    writeReg(CTRL_REG1_A, CTRL_REG1_VAL);
    writeReg(CTRL_REG4_A, CTRL_REG4_VAL);
    delay(ACCEL_SETTLE_MS);  // let sensor settle
}

void MBotAccel::readAll() {
    ensureInit();
    unsigned long now = millis();
    if (now - _lastRead < ACCEL_READ_INTERVAL_MS && _lastRead != 0) return;
    _lastRead = now;

    uint8_t buf[6];
    readRegs(OUT_X_L_A | 0x80, buf, 6);  // 0x80 for auto-increment

    // LSM303AGR: 10-bit left-justified in 16-bit, so shift right by 6
    // Then convert to milli-g: at +/-2g, 1 LSB = ~4 mg (after >>6)
    int16_t rawX = (int16_t)(buf[1] << 8 | buf[0]) >> 6;
    int16_t rawY = (int16_t)(buf[3] << 8 | buf[2]) >> 6;
    int16_t rawZ = (int16_t)(buf[5] << 8 | buf[4]) >> 6;

    _x = rawX * ACCEL_MG_PER_LSB;  // approximate milli-g
    _y = rawY * ACCEL_MG_PER_LSB;
    _z = rawZ * ACCEL_MG_PER_LSB;
}

int16_t MBotAccel::x() { readAll(); return _x; }
int16_t MBotAccel::y() { readAll(); return _y; }
int16_t MBotAccel::z() { readAll(); return _z; }

bool MBotAccel::isShaking(uint16_t threshold) {
    readAll();
    int32_t mag = (int32_t)_x * _x + (int32_t)_y * _y + (int32_t)_z * _z;
    return mag > (int32_t)threshold * threshold;
}

void MBotAccel::writeReg(uint8_t reg, uint8_t value) {
    Wire1.beginTransmission(LSM303_ACCEL_ADDR);
    Wire1.write(reg);
    Wire1.write(value);
    Wire1.endTransmission();
}

uint8_t MBotAccel::readReg(uint8_t reg) {
    Wire1.beginTransmission(LSM303_ACCEL_ADDR);
    Wire1.write(reg);
    Wire1.endTransmission();
    Wire1.requestFrom(LSM303_ACCEL_ADDR, (uint8_t)1);
    return Wire1.read();
}

void MBotAccel::readRegs(uint8_t reg, uint8_t* buf, uint8_t len) {
    Wire1.beginTransmission(LSM303_ACCEL_ADDR);
    Wire1.write(reg);
    Wire1.endTransmission();
    Wire1.requestFrom(LSM303_ACCEL_ADDR, len);
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = Wire1.read();
    }
}
