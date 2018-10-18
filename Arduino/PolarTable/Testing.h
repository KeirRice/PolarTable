/*************************************************************
  Testing playground
*************************************************************/
#pragma once

#ifndef ENABLE_TESTING

void testing_setup() {}
void testing_loop() {}

#else

#include "SX1509.h"

extern SX1509 io;

void testing_setup()
{
  pinMode(ARDUINO_D13, OUTPUT);
  if (!io.begin(SX1509_I2C_ADDRESS, ARDUINO_D13))
  {
    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
    evtManager.trigger(ERROR_EVENT, ERROR_SX1509);
    while (1);
  } 

  io.reset(1);
  io.clock(INTERNAL_CLOCK_2MHZ, 1);
  io.pinMode(SX1509_B13, ANALOG_OUTPUT);
  io.ledDriverInit(SX1509_B13, 1, true);
}

const int Off = 0;
const int On = 1;
const int Breath = 2;
const int Panic = 3;

//#define   REG_T_ON_13       0x5A  //  RegTOn13 ON time register for I/O[13] 0000 0000
//#define   REG_I_ON_13       0x5B  //  RegIOn13 ON intensity register for I/O[13] 1111 1111
//#define   REG_OFF_13        0x5C  //  RegOff13 OFF time/intensity register for I/O[13] 0000 0000
//#define   REG_T_RISE_13     0x5D  //  RegTRise13 Fade in register for I/O[13] 0000 0000
//#define   REG_T_FALL_13     0x5E  //  RegTFall13 Fade out register for I/O[13] 0000 0000
// setupBlink(byte pin, byte tOn, byte tOff, byte onIntensity, byte offIntensity, byte tRise, byte tFall, bool log)

//0x5A RegTOn13 ON time register for I/O[13] 0000 0000
//0x5B RegIOn13 ON intensity register for I/O[13] 1111 1111
//0x5C RegOff13 OFF time/intensity register for I/O[13] 0000 0000
//0x5D RegTRise13 Fade in register for I/O[13] 0000 0000
//0x5E RegTFall13 Fade out register for I/O[13] 0000 0000 





//Each IO[X] has its own set of programmable registers (Cf §5 for more detailed information):
// RegTOnX (blinking capable I/Os only): TOnX, ON time of IO[X]
// RegIOnX (all I/Os): IOnX, ON intensity of IO[X]
// RegOffX (blinking capable I/Os only): TOffX and IOffX, OFF time and intensity of IO[X]
// RegTRiseX(breathing capable I/Os only): TRiseX, fade in time of IO[X]
// RegTFallX(breathing capable I/Os only): TFallX, fade out time of IO[X] 


/*
| Addr |   Name    | Default | Bits |                                      Description                                       |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|
| 0xXX | RegTOnX   | 0x00    | 4:0  | ON Time of IO[X]:                                                                      |
|      |           |         |      | 0 : Infinite (Static mode, TOn directly controlled by RegData, Cf §4.9.2)              |
|      |           |         |      | 1 - 15 : TOnX = 64 * RegTOnX * (255/ClkX)                                              |
|      |           |         |      | 16 - 31 : TOnX = 512 * RegTOnX * (255/ClkX)                                            |
|      |           |         |      |                                                                                        |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|
| 0xXX | RegIOnX   | 0xFF    | 7:0  | ON Intensity of IO[X]                                                                  |
|      |           |         |      | - Linear mode : IOnX = RegIOnX                                                         |
|      |           |         |      | - Logarithmic mode (fading capable IOs only) : IOnX = f(RegIOnX)                       |
|      |           |         |      | 16 - 31 : TOffX = 512 * RegOffX[7:3] * (255/ClkX)                                      |
|      |           |         |      |                                                                                        |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|
| 0xXX | RegOffX   | 0x00    | 7:3  | OFF Time of IO[X]:                                                                     |
|      |           |         |      | 0 : Infinite (Single shot mode, TOff directly controlled by RegData, Cf §4.9.3)        |
|      |           |         |      | 1 - 15 : TOffX = 64 * RegOffX[7:3] * (255/ClkX)                                        |
|      |           |         |      |                                                                                        |
|      |           |         |------|----------------------------------------------------------------------------------------|
|      |           |         | 2:0  | OFF Intensity of IO[X]                                                                 |
|      |           |         |      | - Linear mode : IOffX = 4 x RegOff[2:0]                                                |
|      |           |         |      | - Logarithmic mode (fading capable IOs only) : IOffX = f(4 x RegOffX[2:0]) , Cf §4.9.5 |
|      |           |         |      |                                                                                        |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|
| 0xXX | RegTRiseX | 0x00    | 7:5  | Unused                                                                                 |
|      |           |         |      |                                                                                        |
|      |           |         |------|----------------------------------------------------------------------------------------|
|      |           |         | 4:0  | Fade In setting of IO[X]                                                               |
|      |           |         |      | 0 : OFF                                                                                |
|      |           |         |      | 1 - 15 : TRiseX = (RegIOnX-(4xRegOffX[2:0])) * RegTRiseX * (255/ClkX)                  |
|      |           |         |      | 16 - 31 : TRiseX = 16 * (RegIOnX-(4xRegOffX[2:0])) * RegTRiseX * (255/ClkX)            |
|      |           |         |      |                                                                                        |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|
| 0xXX | RegTFallX | 0x00    | 7:5  | Unused                                                                                 |
|      |           |         |      |                                                                                        |
|      |           |         |------|----------------------------------------------------------------------------------------|
|      |           |         | 4:0  | Fade Out setting of IO[X]                                                              |
|      |           |         |      | 0 : OFF                                                                                |
|      |           |         |      | 1 - 15 : TFallX = (RegIOnX-(4xRegOffX[2:0])) * RegTFallX * (255/ClkX)                  |
|      |           |         |      | 16 - 31 : TFallX = 16 * (RegIOnX-(4xRegOffX[2:0])) * RegTFallX * (255/ClkX)            |
|      |           |         |      |                                                                                        |
|------|-----------|---------|------|----------------------------------------------------------------------------------------|

*/

void fade_on(){
  
  byte pin = SX1509_B13;
  byte tOn = 0; // Infinite (Static mode, TOn directly controlled by RegData (digitalWrite)
  byte tOff = 0; // Infinite (Single shot mode, TOff directly controlled by RegData (digitalWrite)
  byte onIntensity = 255; // 0 - 255, 0 is off
  byte offIntensity = 7; // 0 - 7, 0 is off
  byte tRise = 10; // 0 and 31 
  byte tFall = 0; // 0 and 31
  bool log = false;
  byte freq = 1;

  // tRise
  // 0: Off
  // 1-15:  TRise =      (regIOn - (4 * offIntensity)) * tRise * (255/ClkX)
  // 16-31: TRise = 16 * (regIOn - (4 * offIntensity)) * tRise * (255/ClkX)

   // ledDriverInit(pin, log);
  // RegOff = tOff<<3) | offIntensity
  // REG_I_ON = onIntensity
  // REG_T_RISE = tRise
  // REG_T_FALL = tFall
  
  io.setupBlink(pin, tOn, tOff, onIntensity, offIntensity, tRise, tFall, log);
}


void light(int newStatus) {
  // Setup for an LED in sinking current mode.  
  switch (newStatus) {

    case Off: {     //Fade out
        Serial.println("Off");
        io.breathe(SX1509_B13,    //pin
                0,  // TON , time on ms needs to be 0 for static mode
                1,  // T0FF, time off ms, needs to be !=0 for static mode
                0,  // fade in, raise time / speed ms
                8000  // fade out, fall time / speed ms
               );
        io.digitalWrite(SX1509_B13, 1); // turn led off, it will slowly fade out
      }; break;

    case On: {    // Fade in
        Serial.println("On");
        io.breathe(SX1509_B13,    //pin
                0,  // tOn , time on ms, needs to be 0 for static mode
                1,  // tOff, time off ms, need to be != 0 for static mode
                8000, // fade in, raise time in ms
                0 // fade out fall time in ms
               );
        io.digitalWrite(SX1509_B13, 0); // turn led on, it will slowly fade in
      }; break;

    case Breath: {
        Serial.println("Breathing");
        io.breathe(SX1509_B13,    //pin
                500,  // tOn , time on ms , needs to !=0 for blink/breath mode
                500,  // tOff, time off ms
                4800, // rise,  raise time / speed ms
                1000  // fall fall time / speed ms
               );
        io.digitalWrite(SX1509_B13, 0); // turn led on
      }; break;

    case Panic: {
        Serial.println("Panic");
        io.breathe(SX1509_B13,    //pin
                500,  // tOn , time on ms  //neds to be !=0 for breath/blink mode
                500,  // tOff, time off ms
                600,  // rise,  raise time / speed ms
                600 // fall fall time / speed ms
               );
        io.digitalWrite(SX1509_B13, 0); // turn led on
      }; break;

  }
}


void testing_loop()
{
//
//  fade_on();
//  delay(8000);
  
//  static int forward = 1;
//  static int i = 10;
//
//  if(i < 1 or i >= 255){
//    forward *= -1;
//  }
//  io.analogWrite(SX1509_B13, i);
//  i += forward;
//  delay(5);
  
  
//  delay(3000);
//  light(Off);
//  delay(8000);
//  io.analogWrite(SX1509_B13, 255);
  
//  digitalWrite(ARDUINO_D13, HIGH);
//  delay(500);
//  digitalWrite(ARDUINO_D13, LOW);

  light(On);
  delay(8000);
//  io.analogWrite(SX1509_B13, 0);
//  delay(3000);
  light(Off);
  delay(8000);
//  io.analogWrite(SX1509_B13, 255);
  delay(3000);
}

#endif // ENABLE_TESTING
