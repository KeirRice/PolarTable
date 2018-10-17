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
  io.ledDriverInit(SX1509_B13);
}

const int Off = 0;
const int On = 1;
const int Breath = 2;
const int Panic = 3;

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
  static int forward = 1;
  static int i = 10;

  if(i < 1 or i >= 255){
    forward *= -1;
  }
  io.analogWrite(SX1509_B13, i);
  i += forward;
  delay(5);
  
  
//  delay(3000);
//  light(Off);
//  delay(8000);
//  io.analogWrite(SX1509_B13, 255);
  
//  digitalWrite(ARDUINO_D13, HIGH);
//  delay(500);
//  digitalWrite(ARDUINO_D13, LOW);

//  light(On);
//  delay(8000);
//  io.analogWrite(SX1509_B13, 0);
//  delay(3000);
//  light(Off);
//  delay(8000);
//  io.analogWrite(SX1509_B13, 255);
//  delay(3000);
}

#endif // ENABLE_TESTING
