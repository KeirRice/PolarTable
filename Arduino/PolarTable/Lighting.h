/*************************************************************
  Lighting
*************************************************************/
#pragma once

#ifdef DISABLE_LIGHTING

void lighting_setup() {}
void lighting_loop() {}

#else

#include <FastLED.h>
#include <Fsm.h>
#include "Event.h"
#include "ProjectEvents.h"

extern EventManager evtManager;

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

static const uint8_t blendRate = 50;  // How fast to blend.  Higher is slower.  [milliseconds]

CHSV colorStart = CHSV(255, 255, 255); // starting color
CHSV colorTarget = CHSV(128, 255, 255); // target color

CHSV incomingColorTarget;
CHSV colorCurrent;

/*************************************************************
  Access
*************************************************************/

char* get_color() {
  return (char*)(&leds[0]);
}

bool blend(bool reset = false)
{
  EVERY_N_MILLISECONDS(blendRate) {
    static uint8_t k = 0;
    if ( colorCurrent.h == colorTarget.h or reset ) {  // Check if target has been reached
      k = 0;  // reset k value
      return true;
    }
    colorCurrent = blend(colorStart, colorTarget, k++, SHORTEST_HUES);
    leds[0] = colorCurrent;
    FastLED.show();  // update the display
  }
  return false;
}


/*************************************************************
  State Machine
*************************************************************/

void lighting_shutdown_enter();
void lighting_shutdown_state();
void lighting_off_enter();
void lighting_blend_enter();
void lighting_blend_state();

State state_lighting_on(NULL, NULL, NULL);
State state_lighting_blend(&lighting_blend_enter, &lighting_blend_state, NULL);
State state_lighting_shutdown(&lighting_off_enter, &lighting_shutdown_state, NULL);
State state_lighting_off(&lighting_off_enter, NULL, NULL);

Fsm fsm_lighting(&state_lighting_on);

void lighting_shutdown_enter(){
  // Reset blending and set the target to black (AKA Off)
  blend(true);
  colorTarget = CHSV(0, 0, 0);
}
void lighting_shutdown_state(){
  if(blend()){
    // Done
    fsm_lighting.trigger(LED_OFF);
  }
}

void lighting_off_enter(){
  // Set the lights to off.
  leds[0] = CRGB::Black;
  FastLED.show();
}

void lighting_blend_enter(){
  // Reset any current blending.
  blend(true);
}
void lighting_blend_state(){
  // Run the blend until we are done and transistion to on
  if(blend()){
    fsm_lighting.trigger(LED_ON);
  }
}

void lighting_listener(void* data){
  fsm_lighting.trigger((int) data);
}

void lighting_color_listener(void* data){
  incomingColorTarget = *(static_cast<CHSV *>(data));
  fsm_lighting.trigger(LED_BLEND);
}

struct LightingEventDriver : public FsmEventDriver
{
  Fsm *fsm;
  LightingEventDriver();
  LightingEventDriver(Fsm *statemachine) : fsm(statemachine) {}

  using EventTask::execute;
  
  void execute(Event *evt)
  {
    incomingColorTarget = (*(CHSV*)evt->extra);
    fsm->trigger(LED_BLEND);
  }
};


/*************************************************************
  Setup and main loop
*************************************************************/

void lighting_setup() {
  FastLED.addLeds<P9813, PIN_LED_SDIN, PIN_LED_SCIN, RGB>(leds, NUM_LEDS);

  fsm_button_led.add_transition(&state_lighting_on, &state_lighting_blend, LED_BLEND, NULL);
  fsm_button_led.add_transition(&state_lighting_blend, &state_lighting_on, LED_ON, NULL);
  
  fsm_button_led.add_transition(&state_lighting_on, &state_lighting_shutdown, LED_SHUTDOWN, NULL);
  fsm_button_led.add_transition(&state_lighting_blend, &state_lighting_shutdown, LED_SHUTDOWN, NULL);
  
  fsm_button_led.add_transition(&state_lighting_shutdown, &state_lighting_off, LED_OFF, NULL);
  fsm_button_led.add_transition(&state_lighting_off, &state_lighting_on, LED_ON, NULL);

  // Create the bridge from the event system to the system fsm.
  struct FsmEventDriver lighting_event_listner = FsmEventDriver(&fsm_system);
  evtManager.subscribe(Subscriber(LIGHTING_STATE, &lighting_event_listner));
  struct LightingEventDriver lighting_color_event_listner = LightingEventDriver(&fsm_system);
  evtManager.subscribe(Subscriber(LIGHTING_COLOR, &lighting_color_event_listner));
}

void lighting_loop() {
  fsm_lighting.run_machine();
}

#endif // DISABLE_LIGHTING
