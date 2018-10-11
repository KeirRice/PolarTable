#pragma once

#ifdef ENABLE_SX1509
#include <SparkFunSX1509.h> // Include SX1509 library

#else
# pragma message "Using mocked version of SparkFunSX1509.h"
// If we are not using the sx1509 mock it out so we don't crash trying to use the library.
// Header at https://github.com/sparkfun/SparkFun_SX1509_Arduino_Library/blob/master/src/SparkFunSX1509.h

#define INTERNAL_CLOCK_2MHZ	2
#define ANALOG_OUTPUT 0x3 // To set a pin mode for PWM output

class SX1509
{
  public:
    SX1509();

    void ledDriverInit(byte pin, byte freq = 1, bool log = false);
    void enableInterrupt(byte pin, byte riseFall);
    unsigned int interruptSource(bool clear = true);
    void debounceTime(byte time);
    byte begin(byte address = 0x3E, byte resetPin = 0xFF);
    void clock(byte oscSource = 2, byte oscDivider = 1, byte oscPinFunction = 0, byte oscFreqOut = 0);
    void pinMode(byte pin, byte inOut);
    void digitalWrite(byte pin, byte highLow);
    byte digitalRead(byte pin);
    void breathe(byte pin, unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, byte onInt = 255, byte offInt = 0, bool log = 0);
};
SX1509::SX1509() {}
void SX1509::ledDriverInit(byte pin, byte freq = 1, bool log = 0) {}
void SX1509::enableInterrupt(byte pin, byte riseFall) {}
unsigned int SX1509::interruptSource(bool clear = true) {}
void SX1509::debounceTime(byte time) {}
byte SX1509::begin(byte address = 0x3E, byte resetPin = 0xFF) {
  return 1;
}
void SX1509::clock(byte oscSource = 2, byte oscDivider = 1, byte oscPinFunction = 0, byte oscFreqOut = 0) {}
void SX1509::pinMode(byte pin, byte inOut) {}
void SX1509::digitalWrite(byte pin, byte highLow) {}
byte SX1509::digitalRead(byte pin) {
  return 0;
}
void SX1509::breathe(byte pin, unsigned long tOn, unsigned long tOff, unsigned long rise, unsigned long fall, byte onInt = 255, byte offInt = 0, bool log = 0) {}
#endif // ENABLE_SX1509
