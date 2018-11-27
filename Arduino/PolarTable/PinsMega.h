/*************************************************************
  All the pins.
*************************************************************/
#pragma once

typedef byte PinID;

// Arduino
static const PinID ARDUINO_D0 = 0, ARDUINO_PE0 = 0, ARDUINO_PCINT8 = 0;
static const PinID ARDUINO_D1 = 1, ARDUINO_PE1 = 1;
static const PinID ARDUINO_D2 = 2, ARDUINO_PE4 = 2, ARDUINO_INT4 = 2;
static const PinID ARDUINO_D3 = 3, ARDUINO_PE5 = 3, ARDUINO_INT5 = 3;
static const PinID ARDUINO_D4 = 4, ARDUINO_PG5 = 4;
static const PinID ARDUINO_D5 = 5, ARDUINO_PE3 = 5;
static const PinID ARDUINO_D6 = 6, ARDUINO_PH3 = 6;
static const PinID ARDUINO_D7 = 7, ARDUINO_PH4 = 7;

static const PinID ARDUINO_D8 = 8, ARDUINO_PH5 = 8;
static const PinID ARDUINO_D9 = 9, ARDUINO_PH6 = 9;

static const PinID ARDUINO_D10 = 10, ARDUINO_PB4 = 10, ARDUINO_PCINT4 = 10;
static const PinID ARDUINO_D11 = 11, ARDUINO_PB5 = 11, ARDUINO_PCINT5 = 11;
static const PinID ARDUINO_D12 = 12, ARDUINO_PB6 = 12, ARDUINO_PCINT6 = 12;
static const PinID ARDUINO_D13 = 13, ARDUINO_PB7 = 13, ARDUINO_PCINT7 = 13;

static const PinID ARDUINO_D14 = 14, ARDUINO_PJ1 = 14, ARDUINO_PCINT10 = 14;
static const PinID ARDUINO_D15 = 15, ARDUINO_PJ0 = 15, ARDUINO_PCINT9 = 15;
static const PinID ARDUINO_D16 = 16, ARDUINO_PH1 = 16;
static const PinID ARDUINO_D17 = 17, ARDUINO_PH0 = 17;
static const PinID ARDUINO_D18 = 18, ARDUINO_PD3 = 18, ARDUINO_INT3 = 18;
static const PinID ARDUINO_D19 = 19, ARDUINO_PD2 = 19, ARDUINO_INT2 = 19;
static const PinID ARDUINO_D20 = 20, ARDUINO_PD1 = 20, ARDUINO_INT1 = 20;
static const PinID ARDUINO_D21 = 21, ARDUINO_PD0 = 21, ARDUINO_INT0 = 21;

static const PinID ARDUINO_D22 = 22, ARDUINO_PA0 = 22;
static const PinID ARDUINO_D23 = 23, ARDUINO_PA1 = 23;
static const PinID ARDUINO_D24 = 24, ARDUINO_PA2 = 24;
static const PinID ARDUINO_D25 = 25, ARDUINO_PA3 = 25;
static const PinID ARDUINO_D26 = 26, ARDUINO_PA4 = 26;
static const PinID ARDUINO_D27 = 27, ARDUINO_PA5 = 27;
static const PinID ARDUINO_D28 = 28, ARDUINO_PA6 = 28;
static const PinID ARDUINO_D29 = 29, ARDUINO_PA7 = 29;
static const PinID ARDUINO_D30 = 30, ARDUINO_PC7 = 30;
static const PinID ARDUINO_D31 = 31, ARDUINO_PC6 = 31;
static const PinID ARDUINO_D32 = 32, ARDUINO_PC5 = 32;
static const PinID ARDUINO_D33 = 33, ARDUINO_PC4 = 33;
static const PinID ARDUINO_D34 = 34, ARDUINO_PC3 = 34;
static const PinID ARDUINO_D35 = 35, ARDUINO_PC2 = 35;
static const PinID ARDUINO_D36 = 36, ARDUINO_PC1 = 36;
static const PinID ARDUINO_D37 = 37, ARDUINO_PC0 = 37;
static const PinID ARDUINO_D38 = 38, ARDUINO_PD7 = 38;
static const PinID ARDUINO_D39 = 39, ARDUINO_PG2 = 39;
static const PinID ARDUINO_D40 = 40, ARDUINO_PG1 = 40;
static const PinID ARDUINO_D41 = 41, ARDUINO_PG0 = 41;
static const PinID ARDUINO_D42 = 42, ARDUINO_PL7 = 42;
static const PinID ARDUINO_D43 = 43, ARDUINO_PL6 = 43;
static const PinID ARDUINO_D44 = 44, ARDUINO_PL5 = 44;
static const PinID ARDUINO_D45 = 45, ARDUINO_PL4 = 45;
static const PinID ARDUINO_D46 = 46, ARDUINO_PL3 = 46;
static const PinID ARDUINO_D47 = 47, ARDUINO_PL2 = 47;
static const PinID ARDUINO_D48 = 48, ARDUINO_PL1 = 48;
static const PinID ARDUINO_D49 = 49, ARDUINO_PL0 = 49;
static const PinID ARDUINO_D50 = 50, ARDUINO_PB3 = 50, ARDUINO_PCINT3 = 50;
static const PinID ARDUINO_D51 = 51, ARDUINO_PB2 = 51, ARDUINO_PCINT2 = 51;
static const PinID ARDUINO_D52 = 52, ARDUINO_PB1 = 52, ARDUINO_PCINT1 = 52;
static const PinID ARDUINO_D53 = 53, ARDUINO_PB0 = 53, ARDUINO_PCINT0 = 53;

static const PinID ARDUINO_D54 = 54, ARDUINO_PF0 = 54, ARDUINO_A0 = 54;
static const PinID ARDUINO_D55 = 55, ARDUINO_PF1 = 55, ARDUINO_A1 = 55;
static const PinID ARDUINO_D56 = 56, ARDUINO_PF2 = 56, ARDUINO_A2 = 56;
static const PinID ARDUINO_D57 = 57, ARDUINO_PF3 = 57, ARDUINO_A3 = 57;
static const PinID ARDUINO_D58 = 58, ARDUINO_PF4 = 58, ARDUINO_A4 = 58;
static const PinID ARDUINO_D59 = 59, ARDUINO_PF5 = 59, ARDUINO_A5 = 59;
static const PinID ARDUINO_D60 = 60, ARDUINO_PF6 = 60, ARDUINO_A6 = 60;
static const PinID ARDUINO_D61 = 61, ARDUINO_PF7 = 61, ARDUINO_A7 = 61;

static const PinID ARDUINO_D62 = 62, ARDUINO_PK0 = 62, ARDUINO_A8 = 62, ARDUINO_PCINT16 = 62;
static const PinID ARDUINO_D63 = 63, ARDUINO_PK1 = 63, ARDUINO_A9 = 63, ARDUINO_PCINT17 = 63;
static const PinID ARDUINO_D64 = 64, ARDUINO_PK2 = 64, ARDUINO_A10 = 64, ARDUINO_PCINT18 = 64;
static const PinID ARDUINO_D65 = 65, ARDUINO_PK3 = 65, ARDUINO_A11 = 65, ARDUINO_PCINT19 = 65;
static const PinID ARDUINO_D66 = 66, ARDUINO_PK4 = 66, ARDUINO_A12 = 66, ARDUINO_PCINT20 = 66;
static const PinID ARDUINO_D67 = 67, ARDUINO_PK5 = 67, ARDUINO_A13 = 67, ARDUINO_PCINT21 = 67;
static const PinID ARDUINO_D68 = 68, ARDUINO_PK6 = 68, ARDUINO_A14 = 68, ARDUINO_PCINT22 = 68;
static const PinID ARDUINO_D69 = 69, ARDUINO_PK7 = 69, ARDUINO_A15 = 69, ARDUINO_PCINT23 = 69;

static const PinID ARDUINO_ICSP_MISO = ARDUINO_D50;
static const PinID ARDUINO_ICSP_MOSI = ARDUINO_D51;
static const PinID ARDUINO_ICSP_SCK = ARDUINO_D52;
static const PinID ARDUINO_ICSP_NOT_SS = ARDUINO_D53;

static const PinID ARDUINO_TX3 = ARDUINO_D14;
static const PinID ARDUINO_RX3 = ARDUINO_D15;

static const PinID ARDUINO_TX2 = ARDUINO_D16;
static const PinID ARDUINO_RX2 = ARDUINO_D17;

static const PinID ARDUINO_TX1 = ARDUINO_D18;
static const PinID ARDUINO_RX1 = ARDUINO_D19;

static const PinID ARDUINO_I2C_SDA = ARDUINO_D20;
static const PinID ARDUINO_I2C_SCL = ARDUINO_D21;


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
static const PinID PIN_WAKE_LED = ARDUINO_D4;

// Motor control
static const PinID PIN_MOTOR_YSTEP = ARDUINO_D5;
static const PinID PIN_MOTOR_YDIR = ARDUINO_D6;
static const PinID PIN_MOTOR_XSTEP = ARDUINO_D7;
static const PinID PIN_MOTOR_XDIR = ARDUINO_D8;

// Radio Module
static const PinID PIN_RADIO_RXDATA = ARDUINO_9;

// Switches for homing
static const PinID PIN_A_SWITCH = ARDUINO_D10;
static const PinID PIN_B_SWITCH = ARDUINO_D11;
static const PinID PIN_C_SWITCH = ARDUINO_D12;
static const PinID PIN_D_SWITCH = ARDUINO_D13;

// LED controller
static const PinID PIN_LED_SCIN = ARDUINO_ICSP_SCK;
static const PinID PIN_LED_SDIN = ARDUINO_ICSP_MOSI;

// Switches for relative encoding
static const PinID PIN_E_SWITCH = ARDUINO_INT3; // ARDUINO_D18
static const PinID PIN_F_SWITCH = ARDUINO_INT2; // ARDUINO_D19

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
