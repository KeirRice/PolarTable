/*************************************************************
  Lighting
*************************************************************/

#include <FastLED.h>
#include "ProjectEvents.h"

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

static const uint8_t blendRate = 50;  // How fast to blend.  Higher is slower.  [milliseconds]

CHSV colorStart = CHSV(96, 255, 255); // starting color
CHSV colorTarget = CHSV(192, 255, 255); // target color
CHSV colorCurrent = colorStart;
CHSV incomingColorTarget;

/*************************************************************
  Access
*************************************************************/

CHSV get_color() {
  return colorTarget;
}

bool blend(bool reset = false)
{
  EVERY_N_MILLISECONDS(blendRate) {
    static uint8_t k;
    if ( colorCurrent.h == colorTarget.h or reset ) {  // Check if target has been reached
      k = 0;  // reset k value
      return true;
    }

    colorCurrent = blend(colorStart, colorTarget, k, SHORTEST_HUES);
    fill_solid( leds, NUM_LEDS, colorCurrent );
    k++;
  }

  FastLED.show();  // update the display
  return false;
}


/*************************************************************
  State Machine
*************************************************************/

void lighting_shutdown_on_enter();
void lighting_shutdown_on_state();
void lighting_off_on_enter();
void lighting_blend_on_state();

State state_lighting_on(NULL, NULL, NULL);
State state_lighting_blend(NULL, &lighting_blend_on_state, NULL);
State state_lighting_shutdown(&lighting_off_on_enter, &lighting_shutdown_on_state, NULL);
State state_lighting_off(&lighting_off_on_enter, NULL, NULL);

Fsm fsm_lighting(&state_lighting_on);

void lighting_shutdown_on_enter(){
  blend(true); // Reset blending
  colorTarget.setHSV(0.0, 0.0, 0.0); // Black
}
void lighting_shutdown_on_state(){
  if(blend()){
    // Done
    fsm_lighting.trigger(LED_OFF);
  }
}

void lighting_off_on_enter(){
  leds[0] = CHSV(0, 0, 0);
  FastLED.show();
}

void lighting_blend_on_state(){
  if(blend()){
    // Done
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

  evtManager.subscribe(Subscriber(LIGHTING_STATE, lighting_listener));
  evtManager.subscribe(Subscriber(LIGHTING_COLOR, lighting_color_listener));
}

void lighting_loop() {
  fsm_lighting.run_machine();
}
