#include <Arduino.h>
#include <assert.h>

typedef volatile uint8_t RoReg; /**< Read only 8-bit register (volatile const unsigned int) */
typedef volatile uint8_t RwReg; /**< Read-Write 8-bit register (volatile unsigned int) */
typedef volatile RwReg * port_ptr_t;
typedef RwReg port_t;

/* Return the number of sifts needed to put a 1 in the LSBit*/
unsigned int maskToShiftCount(byte mask){
  for(unsigned int i = 0; i < 8; ++i){
    if(mask & 1){
      return i;
    }
    mask = mask >> 1;
  }
  return 0;
}

template<uint8_t... Pins> class PinBank {
public:
  RwReg sPinMask;
  volatile RwReg *sPort;
  volatile RoReg *sInPort;
  volatile RwReg *sDDRPort;
  uint8_t sPortShiftOnRead;

  /* Compile time assert that pins are contiguous. */
  template<class T, class... Args>
  constexpr static bool check(T arg1, T arg2){
    return (bool)(arg1 + 1 == arg2);
  };
  template<class T, class... Args>
  constexpr static bool check(T arg1, T arg2, Args... args){
    return (bool) ((arg1 + 1 == arg2) & check(arg2, args...));
  };
  static_assert(check(Pins...), "Invalid pins specified. We need pins in a contigous range on the same port.");

  PinBank(){
    _init(Pins...);
    sPortShiftOnRead = maskToShiftCount(sPinMask);
  }

  /* Build up port data. */
  void _init(){};
  template<class T, class... Args>
  void _init(T arg, Args... args, int counter=0){
    if(counter == 0){
      // Set these values once, then we can just check the reset of the pins match.
      sPort = portOutputRegister(digitalPinToPort(arg));
      sInPort = portInputRegister(digitalPinToPort(arg));
      sDDRPort = portModeRegister(digitalPinToPort(arg));
    }
    else {
      // Fail if any of the pins are on a diffrent port.
      assert(sPort == portOutputRegister(digitalPinToPort(arg)));
    }
    // Accumulate the bit mask
    sPinMask |= digitalPinToBitMask(arg);
    _init(args..., ++counter);
  };

  inline byte read(){
    return ((*sInPort& sPinMask) >> sPortShiftOnRead);
  }
};

template<uint8_t... Pins> class PinGroup {
public:
  PinGroup(){};

  uint8_t pin_count = sizeof...(Pins);
  uint8_t pins[sizeof...(Pins)] = {uint8_t(Pins)...};

  inline byte read(){
    byte value = 0;
    for(uint8_t i=0; i < pin_count; ++i){
      value |= digitalRead(pins[i]) << i;
    }
    return value;
  }
};

void setup() {
  // put your setup code here, to run once:
  PinBank<A11, A12> bank;
  bank.read();
  PinGroup<A11, A12> group;
  group.read();
}

void loop() {
  // put your main code here, to run repeatedly:

}
