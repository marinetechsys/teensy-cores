
#include <Arduino.h>
#include "HardwareSerial.h"
void serialEvent1() __attribute__((weak));
void serialEvent1() {}		// No use calling this so disable if called...
extern const uint8_t _serialEvent1_default PROGMEM = 1;
