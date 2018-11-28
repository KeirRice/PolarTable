#include "SparkFunSX1509.h"
SX1509 io;

typedef struct SX1509PinID : public PinID {
  SX1509 &io;
  SX1509PinID(byte _pin, SX1509 &_io): PinID(_pin), io(_io) {};

  template <class T>
  void pinMode(T mode){
    return io.pinMode(pin, mode);
  }

  void enableInterrupt(byte riseFall){
    io.enableInterrupt(pin, riseFall);
  }
  
  int digitalRead() {
    return io.digitalRead(pin);
  }
  
  template <class T>
  void digitalWrite(T value) {
    return io.digitalWrite(pin, value);
  }
  
  template<typename... Args>
  void ledDriverInit(Args... args){
    io.ledDriverInit(pin, args...);
  }
  
  template<typename... Args>
  void setupBlink(Args... args){
    io.setupBlink(pin, args...);
  }
  
  template<typename... Args>
  void blink(Args... args){
    io.blink(pin, args...);
  }
  
  template<typename... Args>
  void breathe(Args... args) {
    return io.breathe(pin, args...);
  }
 
  operator uint8_t() {
    return (uint8_t) pin;
  }
} SX1509PinID;


void sx1509_setup(){
  if (!io.begin(SX1509_I2C_ADDRESS))
  {
    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
    while (1);
  }  
}


void sx1509_shutdown(){
  // SX1509 shutdown? We don't have a hardware reset line so use software.
  io.reset(false);
}
