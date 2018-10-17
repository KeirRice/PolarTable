#pragma once

#ifndef ENABLE_MOCK_WIRE_LIBRARY

#include <Wire.h>

#else

# pragma message "Using mocked version of Wire.h"

class SerialWire {
  public:

    SerialWire();
    void begin(char address);
    void onReceive(void (*func)(int));
    void onRequest(void (*func)());

    bool available();
    template<typename T>
    T read();
    template<typename T>
    int write(T value);

    static void (*user_onRequest)(void);
    static void (*user_onReceive)(int);
    static void onRequestService();
    static void onReceiveService();
};

SerialWire::SerialWire() {}
void SerialWire::begin(char address) {}
void SerialWire::onReceive(void (*func)(int)) {
  int bytesInBuffer = 0;
  (*func)(bytesInBuffer);
}
void SerialWire::onRequest(void (*func)()) {
  (*func)();
}
bool SerialWire::available() {
  return Serial.available();
}

template<typename T>
T SerialWire::read() {
  return Serial.read();
}

template<typename T>
int SerialWire::write(T value) {
  return Serial.write(value);
}

void SerialWire::onReceiveService()
{
  int numBytes = 1;
  user_onReceive(numBytes);
}
SerialWire Wire;

void serialEvent() {
  Wire.onReceiveService();
}

#endif // ENABLE_MOCK_WIRE_LIBRARY
