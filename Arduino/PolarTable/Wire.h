#ifdef WIRE_ENABLED
  #include <Wire.h>
#else
class SerialWire {
  public:
  
  SerialWire();
  void begin(char address)
  void onReceive(void (*func)(int))
  void onRequest(void (*func)())
  
  bool available()
  int read()
  int write()
  
  static void (*user_onRequest)(void);
  static void (*user_onReceive)(int);
  static void onRequestService();
  static void onReceiveService();
}
void SerialWire::SerialWire() {}
void SerialWire::begin(char address){}
void SerialWire::onReceive(void (*func)(int)){
  int bytesInBuffer = 0;
  (*func)(bytesInBuffer);
}
void SerialWire::onRequest(void (*func)()){
  (*func)();
}
void SerialWire::available(){
  return Serial.available();
}
int SerialWire::read(){
  return Serial.read()
}
<template T>
int SerialWire::write(T value){
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
#endif // WIRE_ENABLED
