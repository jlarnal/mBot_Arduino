#include "MBotTemp.h"
#include "bsp.h"

MBotTemp::MBotTemp()
    : _initialized(false) {}

void MBotTemp::ensureInit() {
    if (_initialized) return;
    _initialized = true;
    // nRF52 TEMP peripheral needs no special init
}

float MBotTemp::celsius() {
    ensureInit();
    NRF_TEMP->TASKS_START = 1;
    while (NRF_TEMP->EVENTS_DATARDY == 0) {}
    NRF_TEMP->EVENTS_DATARDY = 0;
    float temp = NRF_TEMP->TEMP * TEMP_UNIT_CELSIUS;
    NRF_TEMP->TASKS_STOP = 1;
    return temp;
}

