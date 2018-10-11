#pragma once

/*************************************************************
All the pins.
*************************************************************/

typedef byte PinID;

static const PinID ARDUINO_D0 = 0;
static const PinID ARDUINO_D1 = 1;
static const PinID ARDUINO_D2 = 2;
static const PinID ARDUINO_D3 = 3;
static const PinID ARDUINO_D4 = 4;
static const PinID ARDUINO_D5 = 5;
static const PinID ARDUINO_D6 = 6;
static const PinID ARDUINO_D7 = 7;
static const PinID ARDUINO_D8 = 8;
static const PinID ARDUINO_D9 = 9;
static const PinID ARDUINO_D10 = 10;
static const PinID ARDUINO_D11 = 11;
static const PinID ARDUINO_D12 = 12;
static const PinID ARDUINO_D13 = 13;
static const PinID ARDUINO_D14 = 14;
static const PinID ARDUINO_D15 = 15;

static const PinID ARDUINO_A0 = A0;
static const PinID ARDUINO_A1 = A1;
static const PinID ARDUINO_A2 = A2;
static const PinID ARDUINO_A3 = A3;
static const PinID ARDUINO_A4 = A4;
static const PinID ARDUINO_A5 = A5;

static const PinID SX1509_B0 = 0;
static const PinID SX1509_B1 = 1;
static const PinID SX1509_B2 = 2;
static const PinID SX1509_B3 = 3;
static const PinID SX1509_B4 = 4;
static const PinID SX1509_B5 = 5;
static const PinID SX1509_B6 = 6;
static const PinID SX1509_B7 = 7;
static const PinID SX1509_B8 = 8;
static const PinID SX1509_B9 = 9;
static const PinID SX1509_B10 = 10;
static const PinID SX1509_B11 = 11;
static const PinID SX1509_B12 = 12;
static const PinID SX1509_B13 = 13;
static const PinID SX1509_B14 = 14;
static const PinID SX1509_B15 = 15;

static const PinID SHIFT_S0 = 0;
static const PinID SHIFT_S1 = 1;
static const PinID SHIFT_S2 = 2;
static const PinID SHIFT_S3 = 3;
static const PinID SHIFT_S4 = 4;
static const PinID SHIFT_S5 = 5;
static const PinID SHIFT_S6 = 6;
static const PinID SHIFT_S7 = 7;


/*************************************************************
All the assignments.
*************************************************************/

// Interupts from the sx1509
static const PinID PIN_SX1509_INT = ARDUINO_D2; // Active low

// Shift registor for motor config
static const PinID PIN_SHIFT_LATCH = ARDUINO_D8;  //Pin connected to latch pin (ST_CP) of 74HC595
static const PinID PIN_SHIFT_DATA = ARDUINO_D9;  //Pin connected to Data in (DS) of 74HC595
static const PinID PIN_SHIFT_CLOCK = ARDUINO_D10;  //Pin connected to clock pin (SH_CP) of 74HC595

// Pull high to cut the power to the Raspberry Pi
static const PinID PIN_PI_POWER = ARDUINO_A3;

// I2C bus
static const PinID PIN_I2C_SDA = ARDUINO_A4;
static const PinID PIN_I2C_SCL = ARDUINO_A5;

// Wake/Sleep switch
static const PinID PIN_WAKE_SWITCH = ARDUINO_D3;
static const byte PIN_WAKE_LED = SX1509_B7;

// LED controller
static const PinID PIN_LED_SCIN = ARDUINO_D13;
static const PinID PIN_LED_SDIN = ARDUINO_D12;

// Radio Module
static const PinID PIN_RADIO_RXDATA = ARDUINO_D11;

// Switches for homing and relative encoding
static const PinID PIN_A_SWITCH = SX1509_B10;
static const PinID PIN_B_SWITCH = SX1509_B11;
static const PinID PIN_C_SWITCH = SX1509_B8;
static const PinID PIN_D_SWITCH = SX1509_B9;
static const PinID PIN_E_SWITCH = SX1509_B4;
static const PinID PIN_F_SWITCH = SX1509_B3;

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
static const PinID PIN_MOTOR_YMS1 = SHIFT_S4;
static const PinID PIN_MOTOR_YMS2 = SHIFT_S5;
static const PinID PIN_MOTOR_XMS1 = SHIFT_S6;
static const PinID PIN_MOTOR_XMS2 = SHIFT_S7;
