#include "Debug.h"

#define MEGA

typedef unsigned int uint;

#define THETA_MAX_TRAVEL 1000
#define RADIUS_MAX_TRAVEL 1000

#ifdef MEGA
#include "PinsMega.h"

#include <kEvent.h>
#include "EventTypes.h"
#endif // MEGA

#include "ProjectEvents.h"

#include "Helpers.h"

EventManager evtManager;
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

#include "blah.h"
#include "ButtonLED.h"
#include "Button.h"
#include "Calibration.h"
#include "EncoderAbsolute.h"
#include "EncoderRelative.h"
#include "Lighting.h"
#include "Motors.h"
#include "Commands.h"


void setup()
{
  if (DEBUG) {
    Serial.begin(115200);
  }
  DEBUG_PRINTLN("Setup serial.");
  DEBUG_WHERE();

  esp_setup();
  pin_setup();
  button_setup();
  button_led_setup();
  encoder_absolute_setup();
  encoder_relative_setup();
  lighting_setup();
  motor_setup();
  calibration_setup();
  command_setup();
  
  // testing_setup();
  DEBUG_WHERE();
}

void loop()
{
  esp_loop();
  button_loop();
  button_led_loop();
  lighting_loop();
  encoder_relative_loop();
  encoder_absolute_loop();
  motor_loop();
  calibration_loop();
  command_loop();
}
//void setup(){}
//void loop(){}
