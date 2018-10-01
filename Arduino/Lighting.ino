/*************************************************************
Lighting
*************************************************************/

#include <FastLED.h>

#define NUM_LEDS 1

static const char DATA_PIN = 12;
static const char CLOCK_PIN = 13;

CRGB leds[NUM_LEDS];

const unsigned char LIGHTING_STATE_IDLE = 0;
const unsigned char LIGHTING_STATE_TO_TARGET = 1;
const unsigned char LIGHTING_STATE_TO_OFF = 2;
unsigned char LIGHTING_STATE = LIGHTING_STATE_IDLE;

uint8_t blendRate = 50;  // How fast to blend.  Higher is slower.  [milliseconds]

CHSV colorStart = CHSV(96,255,255);  // starting color
CHSV colorTarget = CHSV(192,255,255);  // target color
CHSV colorCurrent = colorStart;

bool colorChanged = true;
bool offEvent = false;
CHSV incomingColorTarget;

/*************************************************************
Access
*************************************************************/

const void set_color(const CHSV &color) {
  incomingColorTarget = color;
  colorChanged = true;
}

const CHSV get_color() {
  return colorTarget;
}

const void lights_off()
{
  offEvent = true;
}


/*************************************************************
Setup and main loop
*************************************************************/


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

void lighting_setup() {
  FastLED.addLeds<P9813, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
}

void lighting_loop() {
  if(offEvent){
    offEvent = false;
    blend(true);
    colorTarget.setHSV(0.0, 0.0, 0.0); // Black
    LIGHTING_STATE = LIGHTING_STATE_TO_OFF;
  }

  switch(LIGHTING_STATE){
    case LIGHTING_STATE_IDLE :
      if(colorChanged){
        colorChanged = false;
        colorStart = colorCurrent;
        colorTarget = incomingColorTarget;
        LIGHTING_STATE = LIGHTING_STATE_TO_TARGET;
      }
      break;
      
    case LIGHTING_STATE_TO_TARGET :
      EVERY_N_MILLISECONDS(blendRate) {                           // FastLED based non-blocking delay to update/display the sequence.
        if(blend()){
          LIGHTING_STATE = LIGHTING_STATE_IDLE;
        }
      }
      break;
    
    case LIGHTING_STATE_TO_OFF :
      EVERY_N_MILLISECONDS(10) {                           // FastLED based non-blocking delay to update/display the sequence.
        if(blend()){
          LIGHTING_STATE = LIGHTING_STATE_IDLE;
        }
      }
    
    default :
      break;
  }
}


