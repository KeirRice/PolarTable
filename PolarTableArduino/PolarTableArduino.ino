#define DEBUG 1
#include "Debug.h"

// #define ENABLE_SX1509

#include "HelpersTemplate.h"
#include <FastLED.h>
#include "pixeltypes.h"
#include "WireTypes.h"
#include "SX1509.h"

typedef unsigned int uint;

const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
SX1509 io;

long startMillis;

void resetMillis(){
  startMillis = millis();
}

void setup()
{
  if(DEBUG){
    Serial.begin(9600);
  }
  
  DEBUG_PRINTLN("setup coms.");
  DEBUG_WHERE();
  resetMillis();
  if (!io.begin(SX1509_ADDRESS))
  {
    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
    while (1);
  }
  
}

void loop()
{
  
}
