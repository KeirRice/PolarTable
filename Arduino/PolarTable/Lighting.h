/*************************************************************
Lighting
*************************************************************/

#include <FastLED.h>
#include "Events.h"

#define NUM_LEDS 1
CRGB leds[NUM_LEDS];

static const State LIGHTING_STATE_IDLE = 0;
static const State LIGHTING_STATE_TO_TARGET = 1;
static const State LIGHTING_STATE_TO_OFF = 2;
State current_lighting_state = LIGHTING_STATE_IDLE;

static const uint8_t blendRate = 50;  // How fast to blend.  Higher is slower.  [milliseconds]

CHSV colorStart = CHSV(96,255,255);  // starting color
CHSV colorTarget = CHSV(192,255,255);  // target color
CHSV colorCurrent = colorStart;
CHSV incomingColorTarget;

Event active_lighting_event = 0;

/*************************************************************
Access
*************************************************************/

void set_color(CHSV &color) {
  incomingColorTarget = color;
  active_lighting_event = LED_COLOR_CHANGE;
}

CHSV get_color() {
  return colorTarget;
}

void lighting_event(Event new_event)
{
  active_lighting_event = new_event;
}

bool blend(bool reset=false)
{
  EVERY_N_MILLISECONDS(blendRate){
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
Setup and main loop
*************************************************************/

void lighting_setup() {
  FastLED.addLeds<P9813, PIN_LED_SDIN, PIN_LED_SCIN, RGB>(leds, NUM_LEDS);
}

void lighting_loop() {
  switch(active_lighting_event){
    case LED_COLOR_CHANGE :
        colorStart = colorCurrent;
        colorTarget = incomingColorTarget;
        current_lighting_state = LIGHTING_STATE_TO_TARGET;
        break;
        
    case LED_OFF_REQUEST :
      active_lighting_event = NULL_EVENT;
      blend(true);
      colorTarget.setHSV(0.0, 0.0, 0.0); // Black
      current_lighting_state = LIGHTING_STATE_TO_OFF;
      break;
      
    case LED_ON_REQUEST :
      active_lighting_event = NULL_EVENT;
      blend(true);
      colorTarget = colorCurrent;
      current_lighting_state = LIGHTING_STATE_TO_TARGET;
      break;
      
    default :
      break;  
  }
  active_lighting_event = 0;

  switch(current_lighting_state){
    case LIGHTING_STATE_IDLE :
      break;
      
    case LIGHTING_STATE_TO_TARGET :
      EVERY_N_MILLISECONDS(blendRate) {                           // FastLED based non-blocking delay to update/display the sequence.
        if(blend()){
          current_lighting_state = LIGHTING_STATE_IDLE;
        }
      }
      break;
    
    case LIGHTING_STATE_TO_OFF :
      EVERY_N_MILLISECONDS(10) {                                  // FastLED based non-blocking delay to update/display the sequence.
        if(blend()){
          current_lighting_state = LIGHTING_STATE_IDLE;
        }
      }
    
    default :
      break;
  }
}
