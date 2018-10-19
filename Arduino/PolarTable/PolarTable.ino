
#include "Debug.h"

const byte ARDUINO_I2C_ADDRESS = 0x04;
const byte SX1509_I2C_ADDRESS = 0x3E;  // SX1509 I2C address (00)

typedef unsigned int uint;
typedef unsigned char State2;

#include "Pins.h"
#include "ProjectEvents.h"
#include "Helpers.h"

#include "Event.h"
EventManager evtManager;

#include "SX1509.h"
SX1509 io;

#include "WireTypes.h"
#include "Error.h"
#include "ButtonLED.h"
#include "Button.h"
#include "Calibration.h"
#include "EncoderAbsolute.h"
#include "EncoderRelative.h"
#include "Lighting.h"
#include "Motors.h"

#include "RaspberryManager.h"
#include "Raspberry.h"

#include "Testing.h"

extern long startMillis;

void setup()
{
  error_setup();
  resetMillis();
  
  if (DEBUG) {
    Serial.begin(115200);
  }
  DEBUG_PRINTLN("Setup serial.");
  DEBUG_WHERE();

//  if (!io.begin(SX1509_I2C_ADDRESS))
//  {
//    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
//    evtManager.trigger(ERROR_EVENT, ERROR_SX1509);
//    while (1);
//  }
  raspberry_setup();
  raspberry_manager_setup();
  button_setup();
  button_led_setup();
  encoder_absolute_setup();
  encoder_relative_setup();
  lighting_setup();
  motor_setup();
  // testing_setup();
  DEBUG_WHERE();
}

void loop()
{
  raspberry_loop();
  raspberry_manager_loop();
  button_loop();
  button_led_loop();
  lighting_loop();
  encoder_relative_loop();
  encoder_absolute_loop();
  motor_loop();
}
