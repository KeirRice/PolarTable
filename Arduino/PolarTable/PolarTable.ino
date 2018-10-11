#define DEBUG 1
// #define ENABLE_SX1509
#define ENABLE_I2C_WIRE
#define SLAVE_ADDRESS 0x04

const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)

typedef unsigned int uint;
typedef unsigned char State;

#include "Debug.h"

#include "Pins.h"
#include "Events.h"
#include "Helpers.h"

#include <FastLED.h>
#include <Bounce2.h>
#include "SX1509.h"
#include "Wire.h"
#include <stdarg.h>
#include <avr/sleep.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <util/atomic.h> // this library includes the ATOMIC_BLOCK macro.

#include "WireTypes.h"
#include "pixeltypes.h"

#include "Button.h"
#include "Calibration.h"
#include "Encoder.h"
#include "Lighting.h"
#include "Motors.h"
#include "Raspberry.h"


SX1509 io;
extern long startMillis;

void setup()
{
  if(DEBUG){
    Serial.begin(11500);
  }  
  DEBUG_PRINTLN("Setup serial.");
  DEBUG_WHERE();
  
  resetMillis();
  if (!io.begin(SX1509_ADDRESS))
  {
    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
    while (1);
  }
  raspberry_setup();
  button_setup();
  encoder_setup();
  lighting_setup();
  motor_setup();
}

void loop()
{
  raspberry_loop();
  button_loop();
  encode_loop();
  lighting_loop();
  motor_loop();
}
