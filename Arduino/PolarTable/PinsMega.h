/*************************************************************
  All the pins.
*************************************************************/
#pragma once

/*
typedef struct PinID {
  const uint8_t pin;
  PinID(const uint8_t _pin): pin(_pin) {};

  template <class T>
  void pinMode(T mode) const{
    return ::pinMode(pin, mode);
  }
  
  int digitalRead() const {
    return ::digitalRead(pin);
  }
  
  void digitalWrite(const int value) const{
    return ::digitalWrite(pin, value);
  }

  // TODO: Look at const expression version using templates.
  // https://stackoverflow.com/questions/31493886/using-constant-struct-members-in-array-declaration
  constexpr operator uint8_t() const {
    return pin;
  }
} PinID;
*/

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
typedef PinID PinID;


// Arduino
static const PinID ARDUINO_D0 = PinID<D0>(); const PinID& ARDUINO_PE0 = ARDUINO_D0, ARDUINO_PCINT8 = ARDUINO_D0;
static const PinID ARDUINO_D1 = PinID(1); const PinID& ARDUINO_PE1 = ARDUINO_D1;
static const PinID ARDUINO_D2 = PinID(2); const PinID& ARDUINO_PE4 = ARDUINO_D2, ARDUINO_INT4 = ARDUINO_D2;
static const PinID ARDUINO_D3 = PinID(3); const PinID& ARDUINO_PE5 = ARDUINO_D3, ARDUINO_INT5 = ARDUINO_D3;
static const PinID ARDUINO_D4 = PinID(4); const PinID& ARDUINO_PG5 = ARDUINO_D4;
static const PinID ARDUINO_D5 = PinID(5); const PinID& ARDUINO_PE3 = ARDUINO_D5;
static const PinID ARDUINO_D6 = PinID(6); const PinID& ARDUINO_PH3 = ARDUINO_D6;
static const PinID ARDUINO_D7 = PinID(7); const PinID& ARDUINO_PH4 = ARDUINO_D7;
static const PinID ARDUINO_D8 = PinID(8); const PinID& ARDUINO_PH5 = ARDUINO_D8;
static const PinID ARDUINO_D9 = PinID(9); const PinID& ARDUINO_PH6 = ARDUINO_D9;

static const PinID ARDUINO_D10 = PinID(10); const PinID& ARDUINO_PB4 = ARDUINO_D10, ARDUINO_PCINT4 = ARDUINO_D10;
static const PinID ARDUINO_D11 = PinID(11); const PinID& ARDUINO_PB5 = ARDUINO_D11, ARDUINO_PCINT5 = ARDUINO_D11;
static const PinID ARDUINO_D12 = PinID(12); const PinID& ARDUINO_PB6 = ARDUINO_D12, ARDUINO_PCINT6 = ARDUINO_D12;
static const PinID ARDUINO_D13 = PinID(13); const PinID& ARDUINO_PB7 = ARDUINO_D13, ARDUINO_PCINT7 = ARDUINO_D13;

static const PinID ARDUINO_D14 = PinID(14); const PinID& ARDUINO_PJ1 = ARDUINO_D14, ARDUINO_PCINT10 = ARDUINO_D14;
static const PinID ARDUINO_D15 = PinID(15); const PinID& ARDUINO_PJ0 = ARDUINO_D15, ARDUINO_PCINT9 = ARDUINO_D15;
static const PinID ARDUINO_D16 = PinID(16); const PinID& ARDUINO_PH1 = ARDUINO_D16;
static const PinID ARDUINO_D17 = PinID(17); const PinID& ARDUINO_PH0 = ARDUINO_D17;
static const PinID ARDUINO_D18 = PinID(18); const PinID& ARDUINO_PD3 = ARDUINO_D18, ARDUINO_INT3 = ARDUINO_D18;
static const PinID ARDUINO_D19 = PinID(19); const PinID& ARDUINO_PD2 = ARDUINO_D19, ARDUINO_INT2 = ARDUINO_D19;
static const PinID ARDUINO_D20 = PinID(20); const PinID& ARDUINO_PD1 = ARDUINO_D20, ARDUINO_INT1 = ARDUINO_D20;
static const PinID ARDUINO_D21 = PinID(21); const PinID& ARDUINO_PD0 = ARDUINO_D21, ARDUINO_INT0 = ARDUINO_D21;

static const PinID ARDUINO_D22 = PinID(22); const PinID& ARDUINO_PA0 = ARDUINO_D22;
static const PinID ARDUINO_D23 = PinID(23); const PinID& ARDUINO_PA1 = ARDUINO_D23;
static const PinID ARDUINO_D24 = PinID(24); const PinID& ARDUINO_PA2 = ARDUINO_D24;
static const PinID ARDUINO_D25 = PinID(25); const PinID& ARDUINO_PA3 = ARDUINO_D25;
static const PinID ARDUINO_D26 = PinID(26); const PinID& ARDUINO_PA4 = ARDUINO_D26;
static const PinID ARDUINO_D27 = PinID(27); const PinID& ARDUINO_PA5 = ARDUINO_D27;
static const PinID ARDUINO_D28 = PinID(28); const PinID& ARDUINO_PA6 = ARDUINO_D28;
static const PinID ARDUINO_D29 = PinID(29); const PinID& ARDUINO_PA7 = ARDUINO_D29;
static const PinID ARDUINO_D30 = PinID(30); const PinID& ARDUINO_PC7 = ARDUINO_D30;
static const PinID ARDUINO_D31 = PinID(31); const PinID& ARDUINO_PC6 = ARDUINO_D31;
static const PinID ARDUINO_D32 = PinID(32); const PinID& ARDUINO_PC5 = ARDUINO_D32;
static const PinID ARDUINO_D33 = PinID(33); const PinID& ARDUINO_PC4 = ARDUINO_D33;
static const PinID ARDUINO_D34 = PinID(34); const PinID& ARDUINO_PC3 = ARDUINO_D34;
static const PinID ARDUINO_D35 = PinID(35); const PinID& ARDUINO_PC2 = ARDUINO_D35;
static const PinID ARDUINO_D36 = PinID(36); const PinID& ARDUINO_PC1 = ARDUINO_D36;
static const PinID ARDUINO_D37 = PinID(37); const PinID& ARDUINO_PC0 = ARDUINO_D37;
static const PinID ARDUINO_D38 = PinID(38); const PinID& ARDUINO_PD7 = ARDUINO_D38;
static const PinID ARDUINO_D39 = PinID(39); const PinID& ARDUINO_PG2 = ARDUINO_D39;
static const PinID ARDUINO_D40 = PinID(40); const PinID& ARDUINO_PG1 = ARDUINO_D40;
static const PinID ARDUINO_D41 = PinID(41); const PinID& ARDUINO_PG0 = ARDUINO_D41;
static const PinID ARDUINO_D42 = PinID(42); const PinID& ARDUINO_PL7 = ARDUINO_D42;
static const PinID ARDUINO_D43 = PinID(43); const PinID& ARDUINO_PL6 = ARDUINO_D43;
static const PinID ARDUINO_D44 = PinID(44); const PinID& ARDUINO_PL5 = ARDUINO_D44;
static const PinID ARDUINO_D45 = PinID(45); const PinID& ARDUINO_PL4 = ARDUINO_D45;
static const PinID ARDUINO_D46 = PinID(46); const PinID& ARDUINO_PL3 = ARDUINO_D46;
static const PinID ARDUINO_D47 = PinID(47); const PinID& ARDUINO_PL2 = ARDUINO_D47;
static const PinID ARDUINO_D48 = PinID(48); const PinID& ARDUINO_PL1 = ARDUINO_D48;
static const PinID ARDUINO_D49 = PinID(49); const PinID& ARDUINO_PL0 = ARDUINO_D49;
static const PinID ARDUINO_D50 = PinID(50); const PinID& ARDUINO_PB3 = ARDUINO_D50, ARDUINO_PCINT3 = ARDUINO_D50;
static const PinID ARDUINO_D51 = PinID(51); const PinID& ARDUINO_PB2 = ARDUINO_D51, ARDUINO_PCINT2 = ARDUINO_D51;
static const PinID ARDUINO_D52 = PinID(52); const PinID& ARDUINO_PB1 = ARDUINO_D52, ARDUINO_PCINT1 = ARDUINO_D52;
static const PinID ARDUINO_D53 = PinID(53); const PinID& ARDUINO_PB0 = ARDUINO_D53, ARDUINO_PCINT0 = ARDUINO_D53;

static const PinID ARDUINO_D54 = PinID(54); const PinID& ARDUINO_PF0 = ARDUINO_D54, ARDUINO_A0 = ARDUINO_D54;
static const PinID ARDUINO_D55 = PinID(55); const PinID& ARDUINO_PF1 = ARDUINO_D55, ARDUINO_A1 = ARDUINO_D55;
static const PinID ARDUINO_D56 = PinID(56); const PinID& ARDUINO_PF2 = ARDUINO_D56, ARDUINO_A2 = ARDUINO_D56;
static const PinID ARDUINO_D57 = PinID(57); const PinID& ARDUINO_PF3 = ARDUINO_D57, ARDUINO_A3 = ARDUINO_D57;
static const PinID ARDUINO_D58 = PinID(58); const PinID& ARDUINO_PF4 = ARDUINO_D58, ARDUINO_A4 = ARDUINO_D58;
static const PinID ARDUINO_D59 = PinID(59); const PinID& ARDUINO_PF5 = ARDUINO_D59, ARDUINO_A5 = ARDUINO_D59;
static const PinID ARDUINO_D60 = PinID(60); const PinID& ARDUINO_PF6 = ARDUINO_D60, ARDUINO_A6 = ARDUINO_D60;
static const PinID ARDUINO_D61 = PinID(61); const PinID& ARDUINO_PF7 = ARDUINO_D61, ARDUINO_A7 = ARDUINO_D61;

static const PinID ARDUINO_D62 = PinID(62); const PinID& ARDUINO_PK0 = ARDUINO_D62, ARDUINO_A8 = ARDUINO_D62, ARDUINO_PCINT16 = ARDUINO_D62;
static const PinID ARDUINO_D63 = PinID(63); const PinID& ARDUINO_PK1 = ARDUINO_D63, ARDUINO_A9 = ARDUINO_D63, ARDUINO_PCINT17 = ARDUINO_D63;
static const PinID ARDUINO_D64 = PinID(64); const PinID& ARDUINO_PK2 = ARDUINO_D64, ARDUINO_A10 = ARDUINO_D64, ARDUINO_PCINT18 = ARDUINO_D64;
static const PinID ARDUINO_D65 = PinID(65); const PinID& ARDUINO_PK3 = ARDUINO_D65, ARDUINO_A11 = ARDUINO_D65, ARDUINO_PCINT19 = ARDUINO_D65;
static const PinID ARDUINO_D66 = PinID(66); const PinID& ARDUINO_PK4 = ARDUINO_D66, ARDUINO_A12 = ARDUINO_D66, ARDUINO_PCINT20 = ARDUINO_D66;
static const PinID ARDUINO_D67 = PinID(67); const PinID& ARDUINO_PK5 = ARDUINO_D67, ARDUINO_A13 = ARDUINO_D67, ARDUINO_PCINT21 = ARDUINO_D67;
static const PinID ARDUINO_D68 = PinID(68); const PinID& ARDUINO_PK6 = ARDUINO_D68, ARDUINO_A14 = ARDUINO_D68, ARDUINO_PCINT22 = ARDUINO_D68;
static const PinID ARDUINO_D69 = PinID(69); const PinID& ARDUINO_PK7 = ARDUINO_D69, ARDUINO_A15 = ARDUINO_D69, ARDUINO_PCINT23 = ARDUINO_D69;

const PinID& ARDUINO_ICSP_MISO = ARDUINO_D50;
const PinID& ARDUINO_ICSP_MOSI = ARDUINO_D51;
const PinID& ARDUINO_ICSP_SCK = ARDUINO_D52;
const PinID& ARDUINO_ICSP_NOT_SS = ARDUINO_D53;

const PinID& ARDUINO_TX3 = ARDUINO_D14;
const PinID& ARDUINO_RX3 = ARDUINO_D15;

const PinID& ARDUINO_TX2 = ARDUINO_D16;
const PinID& ARDUINO_RX2 = ARDUINO_D17;

const PinID& ARDUINO_TX1 = ARDUINO_D18;
const PinID& ARDUINO_RX1 = ARDUINO_D19;

const PinID& ARDUINO_I2C_SDA = ARDUINO_D20;
const PinID& ARDUINO_I2C_SCL = ARDUINO_D21;


/*************************************************************
  All the assignments.
*************************************************************/

// Pull high to cut the power to the Raspberry Pi
static const PinID PIN_PI_POWER = ARDUINO_A3;

// I2C bus
static const PinID PIN_I2C_SDA = ARDUINO_I2C_SDA;
static const PinID PIN_I2C_SCL = ARDUINO_I2C_SCL;

// Wake/Sleep switch
static const PinID PIN_WAKE_SWITCH = ARDUINO_INT5;  // ARDUINO_D3
static const PinID PIN_INTERUPT = PIN_WAKE_SWITCH;
static const PinID PIN_WAKE_LED = ARDUINO_D4;

// Motor control
static const PinID PIN_MOTOR_YSTEP = ARDUINO_D5;
static const PinID PIN_MOTOR_YDIR = ARDUINO_D6;
static const PinID PIN_MOTOR_XSTEP = ARDUINO_D7;
static const PinID PIN_MOTOR_XDIR = ARDUINO_D8;

// Radio Module
static const PinID PIN_RADIO_RXDATA = ARDUINO_D9;

// Switches for homing
static const PinID PIN_A_SWITCH = ARDUINO_D68;
static const PinID PIN_B_SWITCH = ARDUINO_D69;
//static const PinID PIN_C_SWITCH = ARDUINO_D12;
//static const PinID PIN_D_SWITCH = ARDUINO_D13;

// LED controller
static const PinID PIN_LED_SCIN = ARDUINO_ICSP_SCK;
static const PinID PIN_LED_SDIN = ARDUINO_ICSP_MOSI;

// Switches for relative encoding
static const PinID PIN_E_SWITCH = ARDUINO_D66;
static const PinID PIN_F_SWITCH = ARDUINO_D67;

// Motor Settings
static const PinID PIN_MOTOR_SLEEPX = ARDUINO_D22;
static const PinID PIN_MOTOR_ENABLEX = ARDUINO_D23;
static const PinID PIN_MOTOR_SLEEPY = ARDUINO_D24;
static const PinID PIN_MOTOR_ENABLEY = ARDUINO_D25;
static const PinID PIN_MOTOR_XMS1 = ARDUINO_D26;
static const PinID PIN_MOTOR_XMS2 = ARDUINO_D27;
static const PinID PIN_MOTOR_YMS1 = ARDUINO_D28;
static const PinID PIN_MOTOR_YMS2 = ARDUINO_D29;

// Error Light
static const PinID PIN_ERROR_LED = ARDUINO_D30;

// IR sensors for absolute encoding
static const PinID PIN_G_IR = ARDUINO_A8;
static const PinID PIN_H_IR = ARDUINO_A9;
static const PinID PIN_I_IR = ARDUINO_A10;
static const PinID PIN_J_IR = ARDUINO_A11;

void pin_setup(){}
