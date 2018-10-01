/*************************************************************
Motor control
*************************************************************/

#include <AccelStepper.h>
#include <MultiStepper.h>

/*************************************************************
Pins
*************************************************************/

static const char ARDUINO_LATCH_PIN = 8;  //Pin connected to latch pin (ST_CP) of 74HC595
static const char ARDUINO_DATA_PIN = 9;  //Pin connected to Data in (DS) of 74HC595
static const char ARDUINO_CLOCK_PIN = 10;  //Pin connected to clock pin (SH_CP) of 74HC595

static const char SHIFT_SLEEPX_PIN = 1;
static const char SHIFT_ENABLEX_PIN = 2;
static const char SHIFT_SLEEPY_PIN = 3;
static const char SHIFT_ENABLEY_PIN = 4;
static const char SHIFT_YMS_PIN = 5;
static const char SHIFT_XMS_PIN = 7;

static const char  STEPPER_Y_STEP_PIN = 4;
static const char  STEPPER_Y_DIR_PIN = 5;
static const char  STEPPER_X_STEP_PIN = 6;
static const char  STEPPER_X_DIR_PIN = 7;

// Define some steppers and the pins the will use
AccelStepper theta_stepper(AccelStepper::DRIVER, STEPPER_Y_STEP_PIN, STEPPER_Y_DIR_PIN);
AccelStepper radius_stepper(AccelStepper::DRIVER, STEPPER_X_STEP_PIN, STEPPER_X_DIR_PIN);
MultiStepper stepper = MultiStepper();

/*************************************************************
Variables
*************************************************************/

// static const char sleepXMask = 1 << SHIFT_SLEEPX_PIN;
// static const char enXMask = 1 << SHIFT_ENABLEX_PIN;
// static const char sleepYMask = 1 << SHIFT_SLEEPY_PIN;
// static const char enYMask = 1 << SHIFT_ENABLEY_PIN;
// static const char yMSMask = 3 << SHIFT_YMS_PIN; // b11
// static const char xMSMask = 3 << SHIFT_XMS_PIN; // b11

static const char FULL_STEP = 0; // b00
static const char HALF_STEP = 2; // b10
static const char QUARTER_STEP = 1; // b01
static const char EIGHTH_STEP = 3; // b11


/*************************************************************
State
*************************************************************/

const char DATA_STATE_IDLE = 0;
const char DATA_STATE_CHANGED = 1;
const char DATA_STATE_SENDING = 2;
char DATA_STATE = 0;

unsigned long dataSendTimer = 0;
unsigned char motor_settings = 0;
bool motor_settings_changed = true;

static const int MOTOR_STATE_ERROR = 0;
static const int MOTOR_STATE_STOPPING = 6;
static const int MOTOR_STATE_STOPPED = 1;
static const int MOTOR_STATE_MOVING = 2;
static const int MOTOR_STATE_IDLE = 3;
static const int MOTOR_STATE_SLEEP = 4;
static const int MOTOR_STATE_WAKE = 5;

static const int MOTOR_STATE_TO_STOPPING = 16;
static const int MOTOR_STATE_TO_STOPPED = 11;
static const int MOTOR_STATE_TO_MOVING = 12;
static const int MOTOR_STATE_TO_IDLE = 13;
static const int MOTOR_STATE_TO_SLEEP = 14;
static const int MOTOR_STATE_TO_WAKE = 15;

/*═════════╦═══════╦═════════╦══════════╦════════╦══════╦═══════╦══════╗
║          ║ error ║ stopped ║ stopping ║ moving ║ idle ║ sleep ║ wake ║
╠══════════╬═══════╬═════════╬══════════╬════════╬══════╬═══════╬══════╣
║ error    ║       ║         ║          ║        ║      ║       ║      ║
║ stopped  ║       ║         ║        1 ║        ║    1 ║       ║    1 ║
║ stopping ║       ║         ║          ║      1 ║      ║       ║      ║
║ moving   ║       ║       1 ║          ║        ║      ║       ║      ║
║ idle     ║       ║       1 ║          ║        ║      ║       ║      ║
║ sleep    ║       ║         ║          ║        ║    1 ║       ║      ║
║ wake     ║       ║         ║          ║        ║      ║     1 ║      ║
╚══════════╩═══════╩═════════╩══════════╩════════╩══════╩═══════╩═════*/

int MOTOR_STATE = 3;

unsigned int stoppedTimer = 0;
unsigned int idleTimer = 0;

long absolute_position_in_steps[2] = {0, 0};
bool new_position = false;


/*************************************************************
Functions
*************************************************************/

void get_motor_position(motor motors[]){
  motors[0].steps = theta_stepper.currentPosition();
  motors[1].steps = radius_stepper.currentPosition();
}

void set_theta_motor_position(motor motor){
  theta_stepper.moveTo(motor.steps);
}

void set_delta_motor_position(motor motor){
  radius_stepper.moveTo(motor.steps);
}

void set_position_in_motor_steps(long theta, long delta)
{
  absolute_position_in_steps[0] = theta; 
  absolute_position_in_steps[1] = delta;
  new_position = true;
}

void send_state() {
  // turn off the output so the pins don't light up
  // while you're shifting bits:
  digitalWrite(ARDUINO_LATCH_PIN, LOW);

  // shift the bits out:
  shiftOut(ARDUINO_DATA_PIN, ARDUINO_CLOCK_PIN, MSBFIRST, motor_settings);

  // turn on the output so the LEDs can light up:
  digitalWrite(ARDUINO_LATCH_PIN, HIGH);
  
  dataSendTimer = currentMillis + 1;
  motor_settings_changed = false;
}


/*************************************************************
Setup and Main loop
*************************************************************/

void motor_setup() {
  
  theta_stepper.setMaxSpeed(200.0);
  theta_stepper.setMinPulseWidth(20); // Driver supports 1us
  // theta_stepper.setAcceleration(200.0);
  
  radius_stepper.setMaxSpeed(100.0);
  radius_stepper.setMinPulseWidth(20); // Driver supports 1us
  // radius_stepper.setAcceleration(100.0);
  
  stepper.addStepper(theta_stepper);
  stepper.addStepper(radius_stepper);

  /* We use a shift register for the motor settings. */
  // Set pins to output because they are addressed in the main loop
  pinMode(ARDUINO_LATCH_PIN, OUTPUT);
  pinMode(ARDUINO_DATA_PIN, OUTPUT);  
  pinMode(ARDUINO_CLOCK_PIN, OUTPUT);
  
  // Full steps
  bitsWrite(motor_settings, SHIFT_XMS_PIN, FULL_STEP, 2);
  bitsWrite(motor_settings, SHIFT_YMS_PIN, FULL_STEP, 2);
  
  // Enable (Sleep is active low)
  bitWrite(motor_settings, SHIFT_ENABLEX_PIN, 1);
  bitWrite(motor_settings, SHIFT_ENABLEY_PIN, 1);
  
  // Turn off sleep (Sleep is active low)
  bitWrite(motor_settings, SHIFT_SLEEPX_PIN, 1);
  bitWrite(motor_settings, SHIFT_SLEEPY_PIN, 1);
  
  send_state();
}


void motor_loop()
{
  switch (DATA_STATE) {
    case DATA_STATE_IDLE :
      // Only trigger a settings update from idle.
      if (motor_settings_changed){
        DATA_STATE = DATA_STATE_CHANGED;
        /* FALL THROUGH */
      }
      else {
        break;
      }
      
    case DATA_STATE_CHANGED :
      send_state();
      DATA_STATE = DATA_STATE_SENDING;
      break;
      
    case DATA_STATE_SENDING :
      // Hold the state for 1ms for the drivers to pick it up.
      if (currentMillis > dataSendTimer){
         DATA_STATE = DATA_STATE_IDLE;
      }
      break;
    default:
      break;
  }  
  
  switch (MOTOR_STATE) {
    case MOTOR_STATE_ERROR :
      /* TODO: ALARM BELLS*/
      break;
    
    case MOTOR_STATE_TO_MOVING :
      // stepper.moveTo(absolute);
      new_position = false;
      stepper.run();
      break;
    
    case MOTOR_STATE_MOVING :
      stepper.run();
      // if(!stepper.isRunning()){
      //   MOTOR_STATE = MOTOR_STATE_TO_STOPPING;
      // }
      break;
      
    case MOTOR_STATE_TO_STOPPING :
      // stepper.stop();
      MOTOR_STATE = MOTOR_STATE_STOPPING;
      /* FALL THROUGH */
    case MOTOR_STATE_STOPPING :
      // if(!stepper.isRunning()){
      //   MOTOR_STATE = MOTOR_STATE_TO_STOPPED;
      // }
      break;
      
    case MOTOR_STATE_TO_STOPPED :
      stoppedTimer = currentMillis + 1000;
      MOTOR_STATE = MOTOR_STATE_STOPPED;
      /* FALL THROUGH */
    case MOTOR_STATE_STOPPED :
      if(new_position){
        MOTOR_STATE = MOTOR_STATE_TO_MOVING;
      }
      if (currentMillis > stoppedTimer){
        MOTOR_STATE = MOTOR_STATE_TO_IDLE;
      }
      break;
      
    case MOTOR_STATE_TO_IDLE :
      idleTimer = currentMillis + 10000;
      MOTOR_STATE = MOTOR_STATE_IDLE;
      /* FALL THROUGH */
    case MOTOR_STATE_IDLE :
      if(new_position){
        MOTOR_STATE = MOTOR_STATE_STOPPED;
      }
      if (currentMillis > idleTimer){
        MOTOR_STATE = MOTOR_STATE_TO_SLEEP;
      }
      break;
      
    case MOTOR_STATE_TO_SLEEP :
      bitWrite(motor_settings, SHIFT_SLEEPX_PIN, 0);
      bitWrite(motor_settings, SHIFT_SLEEPY_PIN, 0);
      MOTOR_STATE = MOTOR_STATE_SLEEP;
      /* FALL THROUGH */
    case MOTOR_STATE_SLEEP :
      if(new_position){
         MOTOR_STATE = MOTOR_STATE_TO_WAKE;
      }
      break;
      
    case MOTOR_STATE_TO_WAKE :
      bitWrite(motor_settings, SHIFT_SLEEPX_PIN, 1);
      bitWrite(motor_settings, SHIFT_SLEEPY_PIN, 1);
      MOTOR_STATE = MOTOR_STATE_WAKE;
      /* FALL THROUGH */
    case MOTOR_STATE_WAKE :
      MOTOR_STATE = MOTOR_STATE_TO_STOPPED;
      break;
    
    default:
      break;
  }
}
