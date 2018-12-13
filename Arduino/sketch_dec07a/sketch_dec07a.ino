#include <arduino.h>

struct PinID {
  const uint8_t mPin;

  constexpr PinID(const uint8_t _pin) : mPin(_pin){};

  constexpr uint8_t pin() const{
    return mPin;
  }

  template <class T>
  void pinMode(T mode) const{
    return ::pinMode(mPin, mode);
  }
  
  int digitalRead() const {
    return ::digitalRead(mPin);
  }
  
  void digitalWrite(const int value) const{
    return ::digitalWrite(mPin, value);
  }
  
  // TODO: Look at const expression version using templates.
  // https://stackoverflow.com/questions/31493886/using-constant-struct-members-in-array-declaration
  constexpr operator uint8_t() const {
    return mPin;
  }
};

static const constexpr PinID ARDUINO_D0 = PinID(0);
static const constexpr PinID ARDUINO_D1 = PinID(1);


#include <EnableInterrupt.h>
#include <kEncoder.h>
kEncoder::AbsoluteEncoder abs_encoder((kEncoder::PinCollectionInterface) kEncoder::PinBank<ARDUINO_D0, ARDUINO_D1>());


void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
