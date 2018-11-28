
typedef struct Shift {

  byte latch_pin, data_pin, clock_pin;
  byte port;
  Shift(byte _latch, byte _data, byte _clock): latch_pin(_latch), data_pin(_data), clock_pin(_clock), port(0){
    pinMode(latch_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
  };

  void send_state() {
    // turn off the output so the pins don't light up
    // while you're shifting bits:
    digitalWrite(latch_pin, LOW);
  
    // shift the bits out:
    shiftOut(data_pin, clock_pin, MSBFIRST, port);
  
    // turn on the output so the LEDs can light up:
    digitalWrite(latch_pin, HIGH);
  }

  void set_port(byte value){
    port = value;
  }
  byte get_port() {
    return port;
  }
  
} Shift;

typedef struct ShiftPinID : public PinID {
  Shift &shifter;
  ShiftPinID(byte _pin, Shift &_shifter): PinID(_pin), shifter(_shifter) {};

  template <class T>
  void pinMode(T mode){
    return ::pinMode(pin, mode);
  }
  
  int digitalRead() {
    return (shifter.get_port() >> pin) & 0x0001;
  }
  
  template <class T>
  void digitalWrite(T value) {
    // Clamp the value to a single bit and shift into position
    value = ((value & 0x0001) << pin);
    byte mask = (1 << pin);
    byte port = (shifter.get_port() & ~mask) & value;
    shifter.set_port(port);
    shifter.send_state();
  }
  
  operator uint8_t() {
    return (uint8_t) pin;
  }
} ShiftPinID;
