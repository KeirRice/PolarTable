#include "Debug.h"

#define MEGA

typedef unsigned int uint;

#define THETA_MAX_TRAVEL 1000
#define RADIUS_MAX_TRAVEL 1000

#include "PinsMega.h"

#include <kEvent.h>
#include "EventTypes.h"
EventManager evtManager;
#include <Fsm.h>

typedef struct FsmEventDriver : public EventTask
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
} FsmEventDriver;

// #define LIBCALL_ENABLEINTERRUPT  // Shouldn't need this but I think I have a library conflict.
#include <EnableInterrupt.h>
#include <kEncoder.h>
kEncoder::AbsoluteEncoder abs_encoder((kEncoder::PinCollectionInterface) kEncoder::PinBank<PIN_G_IR, PIN_H_IR, PIN_I_IR, PIN_J_IR>());
kEncoder::RelativeEncoder rel_encoder((kEncoder::PinCollectionInterface) kEncoder::PinBank<PIN_E_SWITCH, PIN_F_SWITCH>());

void abs_encoder_isr(){
  abs_encoder.interputHandler();
}
void rel_encoder_isr(){
  rel_encoder.interputHandler();
}

#include "ProjectEvents.h"
#include "Helpers.h"

#include "ESP.h"
#include "ButtonLED.h"
#include "Button.h"
#include "Calibration.h"
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
  lighting_setup();
  motor_setup();
  calibration_setup();
  command_setup();

  // Encode setup.
#ifndef DISABLE_ENCODER_ABSOLUTE
  abs_encoder.setup(&abs_encoder_isr);
#endif // DISABLE_ENCODER_ABSOLUTE
#ifndef DISABLE_ENCODER_RELATIVE
  rel_encoder.setup(&rel_encoder_isr);
#endif // DISABLE_ENCODER_RELATIVE
  
  // testing_setup();
  DEBUG_WHERE();
}

void loop()
{
  esp_loop();
  button_loop();
  button_led_loop();
  lighting_loop();
  motor_loop();
  calibration_loop();
  command_loop();
}
//void setup(){}
//void loop(){}
