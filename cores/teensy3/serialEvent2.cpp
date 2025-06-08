
#include <Arduino.h>
#include "HardwareSerial.h"
void serialEvent2() __attribute__((weak));
void serialEvent2() {}		// No use calling this so disable if called...
extern const uint8_t _serialEvent2_default PROGMEM = 1;
