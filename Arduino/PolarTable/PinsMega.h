
/*************************************************************
  All the pins.
*************************************************************/
#pragma once

#include <kPins.h>

using namespace kPins;


/*************************************************************
  All the assignments.
*************************************************************/

// Pull high to cut the power to the Raspberry Pi
constexpr static const PinID PIN_PI_POWER(ARDUINO_A3);

// I2C bus
constexpr static const PinID PIN_I2C_SDA(ARDUINO_I2C_SDA);
constexpr static const PinID PIN_I2C_SCL(ARDUINO_I2C_SCL);

// Wake/Sleep switch
constexpr static const PinID PIN_WAKE_SWITCH(ARDUINO_INT5);  // ARDUINO_D3
constexpr static const PinID PIN_INTERUPT(PIN_WAKE_SWITCH);
constexpr static const PinID PIN_WAKE_LED(ARDUINO_D4);

// Motor control
constexpr static const PinID PIN_MOTOR_YSTEP(ARDUINO_D5);
constexpr static const PinID PIN_MOTOR_YDIR(ARDUINO_D6);
constexpr static const PinID PIN_MOTOR_XSTEP(ARDUINO_D7);
constexpr static const PinID PIN_MOTOR_XDIR(ARDUINO_D8);

// Radio Module
constexpr static const PinID PIN_RADIO_RXDATA(ARDUINO_D9);

// Switches for homing
constexpr static const PinID PIN_A_SWITCH(ARDUINO_D68);
constexpr static const PinID PIN_B_SWITCH(ARDUINO_D69);
//constexpr static const PinID PIN_C_SWITCH(ARDUINO_D12);
//constexpr static const PinID PIN_D_SWITCH(ARDUINO_D13);

// LED controller
constexpr static const PinID PIN_LED_SCIN(ARDUINO_ICSP_SCK);
constexpr static const PinID PIN_LED_SDIN(ARDUINO_ICSP_MOSI);

// Switches for relative encoding
constexpr static const PinID PIN_E_SWITCH(ARDUINO_D66);
constexpr static const PinID PIN_F_SWITCH(ARDUINO_D67);

// Motor Settings
constexpr static const PinID PIN_MOTOR_SLEEPX(ARDUINO_D22);
constexpr static const PinID PIN_MOTOR_ENABLEX(ARDUINO_D23);
constexpr static const PinID PIN_MOTOR_SLEEPY(ARDUINO_D24);
constexpr static const PinID PIN_MOTOR_ENABLEY(ARDUINO_D25);
constexpr static const PinID PIN_MOTOR_XMS1(ARDUINO_D26);
constexpr static const PinID PIN_MOTOR_XMS2(ARDUINO_D27);
constexpr static const PinID PIN_MOTOR_YMS1(ARDUINO_D28);
constexpr static const PinID PIN_MOTOR_YMS2(ARDUINO_D29);

// Error Light
constexpr static const PinID PIN_ERROR_LED(ARDUINO_D30);

// IR sensors for absolute encoding
constexpr static const PinID PIN_G_IR(ARDUINO_A8);
constexpr static const PinID PIN_H_IR(ARDUINO_A9);
constexpr static const PinID PIN_I_IR(ARDUINO_A10);
constexpr static const PinID PIN_J_IR(ARDUINO_A11);
