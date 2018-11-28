/*************************************************************
  All the pins.
*************************************************************/
#pragma once

typedef struct PinID {
  byte pin;
  PinID(byte _pin): pin(_pin) {};

  template <class T>
  void pinMode(T mode){
    return ::pinMode(pin, mode);
  }
  
  int digitalRead() {
    return ::digitalRead(pin);
  }
  
  template <class T>
  void digitalWrite(T value) {
    ::digitalWrite(pin, value);
  }
  
  operator uint8_t() {
    return (uint8_t) pin;
  }
} PinID;


// Arduino
// PORTD
static const PinID ARDUINO_D0 = PinID(0);
static const PinID ARDUINO_D1 = PinID(1);
static const PinID ARDUINO_D2 = PinID(2);
static const PinID ARDUINO_D3 = PinID(3);
static const PinID ARDUINO_D4 = PinID(4);
static const PinID ARDUINO_D5 = PinID(5);
static const PinID ARDUINO_D6 = PinID(6);
static const PinID ARDUINO_D7 = PinID(7);
// PORTB
static const PinID ARDUINO_D8 = PinID(8);
static const PinID ARDUINO_D9 = PinID(9);
static const PinID ARDUINO_D10 = PinID(10); static const PinID ARDUINO_SPI_SELECT = ARDUINO_D10;
static const PinID ARDUINO_D11 = PinID(11); static const PinID ARDUINO_SPI_MOSI = ARDUINO_D11;
static const PinID ARDUINO_D12 = PinID(12); static const PinID ARDUINO_SPI_MISO = ARDUINO_D12;
static const PinID ARDUINO_D13 = PinID(13); static const PinID ARDUINO_SPI_CLOCK = ARDUINO_D13;
// PORTC // Analog pins
static const PinID ARDUINO_D14 = PinID(14); static const PinID ARDUINO_A0 = A0;
static const PinID ARDUINO_D15 = PinID(15); static const PinID ARDUINO_A1 = A1;
static const PinID ARDUINO_D16 = PinID(16); static const PinID ARDUINO_A2 = A2;
static const PinID ARDUINO_D17 = PinID(17); static const PinID ARDUINO_A3 = A3;
static const PinID ARDUINO_D18 = PinID(18); static const PinID ARDUINO_A4 = A4; static const PinID ARDUINO_I2C_SDA = ARDUINO_A4;
static const PinID ARDUINO_D19 = PinID(19); static const PinID ARDUINO_A5 = A5; static const PinID ARDUINO_I2C_SCL = ARDUINO_A5;


// SX1509
#include "SparkFunSX1509.h"
SX1509 io;
typedef struct SX1509PinID : public PinID {
  SX1509 &io;
  SX1509PinID(byte _pin, SX1509 &_io): PinID(_pin), io(_io) {};

  template <class T>
  void pinMode(T mode){
    return io.pinMode(pin, mode);
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
    io.blink(pin, args...)
  }
  
  template<typename... Args>
  void breathe(Args... args) {
    return io.breathe(pin, args...);
  }
 
  operator uint8_t() {
    return (uint8_t) pin;
  }
} SX1509PinID;


// BANK A
static const PinID SX1509_B0 = SX1509PinID(0, io);
static const PinID SX1509_B1 = SX1509PinID(1, io);
static const PinID SX1509_B2 = SX1509PinID(2, io);
static const PinID SX1509_B3 = SX1509PinID(3, io);
static const PinID SX1509_B4 = SX1509PinID(4, io);
static const PinID SX1509_B5 = SX1509PinID(5, io);
static const PinID SX1509_B6 = SX1509PinID(6, io);
static const PinID SX1509_B7 = SX1509PinID(7, io);
// BANK B
static const PinID SX1509_B8 = SX1509PinID(8, io);
static const PinID SX1509_B9 = SX1509PinID(9, io);
static const PinID SX1509_B10 = SX1509PinID(10, io);
static const PinID SX1509_B11 = SX1509PinID(11, io);
static const PinID SX1509_B12 = SX1509PinID(12, io);
static const PinID SX1509_B13 = SX1509PinID(13, io);
static const PinID SX1509_B14 = SX1509PinID(14, io);
static const PinID SX1509_B15 = SX1509PinID(15, io);


#include "Shifter.h"
// Shift registor for motor config
static const PinID PIN_SHIFT_DATA = ARDUINO_D14;   // (A0) Pin connected to Data in (DS) of 74HC595
static const PinID PIN_SHIFT_CLOCK = ARDUINO_D15;  // (A1) Pin connected to clock pin (SH_CP) of 74HC595
static const PinID PIN_SHIFT_LATCH = ARDUINO_D16;  // (A2) Pin connected to latch pin (ST_CP) of 74HC595

Shift shifter = Shift(PIN_SHIFT_LATCH, PIN_SHIFT_DATA, PIN_SHIFT_CLOCK);
static const ShiftPinID SHIFT_S0 = ShiftPinID(0, shifter);
static const ShiftPinID SHIFT_S1 = ShiftPinID(1, shifter);
static const ShiftPinID SHIFT_S2 = ShiftPinID(2, shifter);
static const ShiftPinID SHIFT_S3 = ShiftPinID(3, shifter);
static const ShiftPinID SHIFT_S4 = ShiftPinID(4, shifter);
static const ShiftPinID SHIFT_S5 = ShiftPinID(5, shifter);
static const ShiftPinID SHIFT_S6 = ShiftPinID(6, shifter);
static const ShiftPinID SHIFT_S7 = ShiftPinID(7, shifter);


/*************************************************************
  All the assignments.
*************************************************************/

// Interupts from the sx1509
static const PinID PIN_SX1509_INT = ARDUINO_D2; // Active low
// Interupts from Encoder/Switch
static const PinID PIN_INTERUPT = ARDUINO_D3;



// Pull high to cut the power to the Raspberry Pi
static const PinID PIN_PI_POWER = ARDUINO_A3;

// I2C bus
static const PinID PIN_I2C_SDA = ARDUINO_I2C_SDA;
static const PinID PIN_I2C_SCL = ARDUINO_I2C_SCL;

// Wake/Sleep switch
static const PinID PIN_WAKE_SWITCH = ARDUINO_D10;
static const PinID PIN_WAKE_LED = SX1509_B7;

// LED controller
static const PinID PIN_LED_SCIN = ARDUINO_SPI_CLOCK;
static const PinID PIN_LED_SDIN = ARDUINO_SPI_MOSI;

// Radio Module
static const PinID PIN_RADIO_RXDATA = ARDUINO_D12;

// Switches for homing
static const PinID PIN_A_SWITCH = SX1509_B10;
static const PinID PIN_B_SWITCH = SX1509_B11;
static const PinID PIN_C_SWITCH = SX1509_B8;
static const PinID PIN_D_SWITCH = SX1509_B9;

// Switches for relative encoding
static const PinID PIN_E_SWITCH = ARDUINO_D8;
static const PinID PIN_F_SWITCH = ARDUINO_D9;
static const PinID PIN_EF_SWITCH_INT = ARDUINO_D3;

// IR sensors for absolute encoding
static const PinID PIN_G_IR = SX1509_B15;
static const PinID PIN_H_IR = SX1509_B14;
static const PinID PIN_I_IR = SX1509_B13;
static const PinID PIN_J_IR = SX1509_B12;

// Motor control
static const PinID PIN_MOTOR_YSTEP = ARDUINO_D4;
static const PinID PIN_MOTOR_YDIR = ARDUINO_D5;
static const PinID PIN_MOTOR_XSTEP = ARDUINO_D6;
static const PinID PIN_MOTOR_XDIR = ARDUINO_D7;

// Motor Settings
static const PinID PIN_MOTOR_SLEEPX = SHIFT_S0;
static const PinID PIN_MOTOR_ENABLEX = SHIFT_S1;
static const PinID PIN_MOTOR_SLEEPY = SHIFT_S2;
static const PinID PIN_MOTOR_ENABLEY = SHIFT_S3;
static const PinID PIN_MOTOR_XMS1 = SHIFT_S4;
static const PinID PIN_MOTOR_XMS2 = SHIFT_S5;
static const PinID PIN_MOTOR_YMS1 = SHIFT_S6;
static const PinID PIN_MOTOR_YMS2 = SHIFT_S7;

// Error Light
static const PinID PIN_ERROR_LED = ARDUINO_D13;
