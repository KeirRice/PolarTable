#include "Debug.h"

#define MEGA

typedef unsigned int uint;

#define THETA_MAX_TRAVEL 1000
#define RADIUS_MAX_TRAVEL 1000

#include "PinsMega.h"

#include <kPin.h>

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

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <kEncoder.h>
kEncoder::AbsoluteEncoder abs_encoder;
kEncoder::RelativeEncoder rel_encoder;


void abs_encoder_isr(){
  abs_encoder.interputHandler();
}
void rel_encoder_isr(){
  rel_encoder.interputHandler();
}

#include "ProjectEvents.h"
#include "Helpers.h"

#include "ESP.h"
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
  button_setup();
  button_led_setup();
  lighting_setup();
  motor_setup();
  calibration_setup();
  command_setup();

  // Encode setup.
#ifndef DISABLE_ENCODER_ABSOLUTE
  kPin::Group::Port *asb_port = new kPin::Group::Port(kPin::PORT_K, 0b00001111);
  abs_encoder.setup((*asb_port), &abs_encoder_isr);
#endif // DISABLE_ENCODER_ABSOLUTE
#ifndef DISABLE_ENCODER_RELATIVE
  kPin::Group::Port *rel_port = new kPin::Group::Port(kPin::PORT_K, 0b00110000);
  rel_encoder.setup((*rel_port), &rel_encoder_isr);
#endif // DISABLE_ENCODER_RELATIVE
  
  // testing_setup();
  DEBUG_WHERE();
}

void loop()
{
//  esp_loop();
//  button_loop();
//  button_led_loop();
//  lighting_loop();
//  motor_loop();
//  calibration_loop();
//  command_loop();
}
//void setup(){}
//void loop(){}
