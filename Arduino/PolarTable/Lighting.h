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
#include <kEvent.h>
#include "ProjectEvents.h"

extern EventManager evtManager;

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

static const uint8_t blendRate = 50;  // How fast to blend.  Higher is slower.  [milliseconds]

static CHSV colorCurrent;
static CHSV colorSource = CHSV(255, 255, 255); // starting color
static CHSV colorTarget = CHSV(128, 255, 255); // target color
static CHSV incomingColorTarget;

bool lighting_on = false;

/*************************************************************
  Access
*************************************************************/

bool get_lighting_state(){
  return lighting_on;
}

char* get_lighting_color() {
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
    colorCurrent = blend(colorSource, colorTarget, k++, SHORTEST_HUES);
    leds[0] = colorCurrent;
    FastLED.show();  // update the display
  }
  return false;
}


/*************************************************************
  State Machine
*************************************************************/

void lighting_on_enter();
void lighting_shutdown_enter();
void lighting_shutdown_state();
void lighting_off_enter();
void lighting_blend_enter();
void lighting_blend_state();

State state_lighting_on(&lighting_on_enter, NULL, NULL);
State state_lighting_blend(&lighting_blend_enter, &lighting_blend_state, NULL);
State state_lighting_shutdown(&lighting_off_enter, &lighting_shutdown_state, NULL);
State state_lighting_off(&lighting_off_enter, NULL, NULL);

Fsm fsm_lighting(&state_lighting_on);

void lighting_on_enter(){
  lighting_on = true;
}

void lighting_shutdown_enter(){
  // Reset blending and set the target to black (AKA Off)
  blend(true);
  colorTarget = CHSV(0, 0, 0);
}
void lighting_shutdown_state(){
  if(blend()){
    // Done
    fsm_lighting.trigger(LIGHTING_OFF);
  }
}

void lighting_off_enter(){
  // Set the lights to off.
  leds[0] = CRGB::Black;
  FastLED.show();
  lighting_on = false;
}

void lighting_blend_enter(){
  // Reset any current blending.
  blend(true);
  colorSource = colorCurrent;
  colorTarget = incomingColorTarget;
}
void lighting_blend_state(){
  // Run the blend until we are done and transistion to on
  if(blend()){
    fsm_lighting.trigger(LIGHTING_ON);
  }
}

struct LightingEventDriver : public FsmEventDriver
{
  LightingEventDriver(Fsm *statemachine) : FsmEventDriver(statemachine) {}

  using EventTask::execute;
  
  void execute(Event *evt)
  {
    if(evt->label == LIGHTING_SET_STATE) {
      byte data = *(byte*)evt->extra;
      if(data){
        fsm->trigger(LIGHTING_TURN_ON);
      }
      else {
        fsm->trigger(LIGHTING_TURN_OFF);
      }
    }
    else{
      CRGB new_color = CRGB(leds[0]);
      if (evt->label == LIGHTING_SET_RED && evt->label == LIGHTING_SET_GREEN && evt->label == LIGHTING_SET_BLUE) {      
        uint8_t *data = (uint8_t*)evt->extra;
        new_color.r = data[0];
        new_color.g = data[1];
        new_color.b = data[2];
      }
      else if (evt->label == LIGHTING_SET_RED) {      
        new_color.r = *(byte*)evt->extra;
      }
      else if (evt->label == LIGHTING_SET_GREEN) {      
        new_color.g = *(byte*)evt->extra;
      }
      else if (evt->label == LIGHTING_SET_BLUE) {      
        new_color.b = *(byte*)evt->extra;
      }
      incomingColorTarget = rgb2hsv_approximate(new_color);
      fsm->trigger(LIGHTING_BLEND);      
    }
    
//    else if(evt->label == LIGHTING_SET_BLEND_TIME) {
//        
//    }
//    else if(evt->label == LIGHTING_BLEND) {
//        
//    }
//    else {
//      fsm->trigger(*(int*)evt->extra);
//    }
  }
};


/*************************************************************
  Setup and main loop
*************************************************************/

void lighting_setup() {
  assert(PIN_LED_SDIN == 51);
  assert(PIN_LED_SCIN == 52);
  FastLED.addLeds<P9813, 51 /*PIN_LED_SDIN.pin*/, 52 /*PIN_LED_SCIN.pin*/, RGB>(leds, NUM_LEDS);

  fsm_button_led.add_transition(&state_lighting_on, &state_lighting_blend, LIGHTING_BLEND, NULL);
  fsm_button_led.add_transition(&state_lighting_blend, &state_lighting_blend, LIGHTING_BLEND, NULL);
  fsm_button_led.add_transition(&state_lighting_blend, &state_lighting_on, LIGHTING_ON, NULL);
  
  fsm_button_led.add_transition(&state_lighting_on, &state_lighting_shutdown, LIGHTING_TURN_OFF, NULL);
  fsm_button_led.add_transition(&state_lighting_blend, &state_lighting_shutdown, LIGHTING_TURN_OFF, NULL);
  
  fsm_button_led.add_transition(&state_lighting_shutdown, &state_lighting_off, LIGHTING_OFF, NULL);
  fsm_button_led.add_transition(&state_lighting_off, &state_lighting_on, LIGHTING_TURN_ON, NULL);

  // Create the bridge from the event system to the system fsm.
  struct LightingEventDriver lighting_color_event_listner = LightingEventDriver(&fsm_system);
  evtManager.subscribe(Subscriber(LIGHTING, &lighting_color_event_listner));
}

void lighting_loop() {
  fsm_lighting.run_machine();
}

#endif // DISABLE_LIGHTING
