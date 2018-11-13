#include "Debug.h"

const byte ARDUINO_I2C_ADDRESS = 0x04;
const byte RASPBERRY_I2C_ADDRESS = 0x14;
const byte SX1509_I2C_ADDRESS = 0x3E;

typedef unsigned int uint;

#define THETA_MAX_TRAVEL 1000
#define RADIUS_MAX_TRAVEL 1000

#include "Pins.h"
#include "ProjectEvents.h"

#include "Helpers.h"

#include "Event.h"
EventManager evtManager;

#include "SparkFunSX1509.h"
SX1509 io;
#include <Fsm.h>

struct FsmEventDriver : public EventTask
{
  Fsm *fsm;
  FsmEventDriver();
  FsmEventDriver(Fsm *statemachine) : fsm(statemachine) {}

  using EventTask::execute;
  
  void execute(Event *evt)
  {
    // DEBUG_PRINT_VAR("FsmEventDriver ", *(EventID*)evt->extra);
    fsm->trigger(*(EventID*)evt->extra);
  }
};

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
#include "I2C.h"


void setup()
{
  error_setup();
  
  if (DEBUG) {
    Serial.begin(19200);
  }
  DEBUG_PRINTLN("Setup serial.");
  DEBUG_WHERE();

  if (!io.begin(SX1509_I2C_ADDRESS))
  {
    DEBUG_PRINTLN("Failed to begin SX1509 coms.");
    evtManager.trigger(ERROR_EVENT, ERROR_SX1509);
    while (1);
  }
  
  raspberry_setup();
  raspberry_manager_setup();
  button_setup();
  button_led_setup();
  encoder_absolute_setup();
  encoder_relative_setup();
  lighting_setup();
  motor_setup();
  i2c_setup();
  calibration_setup();
  
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
  i2c_loop();
  calibration_loop();
}
//void setup(){}
//void loop(){}
