/*************************************************************
  Motor control
*************************************************************/

#include <AccelStepper.h>
#include <MultiStepper.h>

// Define some steppers and the pins the will use
AccelStepper theta_stepper(AccelStepper::DRIVER, PIN_MOTOR_YSTEP, PIN_MOTOR_YDIR);
AccelStepper radius_stepper(AccelStepper::DRIVER, PIN_MOTOR_XSTEP, PIN_MOTOR_XDIR);
MultiStepper stepper = MultiStepper();

/*************************************************************
  Variables
*************************************************************/

// static const char sleepXMask = 1 << PIN_MOTOR_SLEEPX;
// static const char enXMask = 1 << PIN_MOTOR_ENABLEX;
// static const char sleepYMask = 1 << PIN_MOTOR_SLEEPY;
// static const char enYMask = 1 << PIN_MOTOR_ENABLEY;
// static const char yMSMask = 3 << PIN_MOTOR_YMS1; // 3 == b11
// static const char xMSMask = 3 << PIN_MOTOR_XMS1; // 3 == b11

static const char FULL_STEP = 0; // b00
static const char HALF_STEP = 2; // b10
static const char QUARTER_STEP = 1; // b01
static const char EIGHTH_STEP = 3; // b11


/*************************************************************
  State
*************************************************************/

static const State2 DATA_STATE_IDLE = 0;
static const State2 DATA_STATE_CHANGED = 1;
static const State2 DATA_STATE_SENDING = 2;
State2 DATA_STATE = 0;

unsigned char motor_settings = 0;

static const State2 MOTOR_STATE_ERROR = 0;
static const State2 MOTOR_STATE_STOPPING = 6;
static const State2 MOTOR_STATE_STOPPED = 1;
static const State2 MOTOR_STATE_MOVING = 2;
static const State2 MOTOR_STATE_IDLE = 3;
static const State2 MOTOR_STATE_SLEEP = 4;
static const State2 MOTOR_STATE_WAKE = 5;

static const State2 MOTOR_STATE_TO_STOPPING = 16;
static const State2 MOTOR_STATE_TO_STOPPED = 11;
static const State2 MOTOR_STATE_TO_MOVING = 12;
static const State2 MOTOR_STATE_TO_IDLE = 13;
static const State2 MOTOR_STATE_TO_SLEEP = 14;
static const State2 MOTOR_STATE_TO_WAKE = 15;

/*═════════╦═══════╦═════════╦══════════╦════════╦══════╦═══════╦══════╗
  ║                ║ error      ║ stopped       ║ stopping        ║ moving       ║ idle     ║ sleep      ║ wake     ║
  ╠═════════╬═══════╬═════════╬══════════╬════════╬══════╬═══════╬══════╣
  ║ error    ║       ║         ║          ║        ║      ║       ║      ║
  ║ stopped  ║       ║         ║        1 ║        ║    1 ║       ║    1 ║
  ║ stopping ║       ║         ║          ║      1 ║      ║       ║      ║
  ║ moving   ║       ║       1 ║          ║        ║      ║       ║      ║
  ║ idle     ║       ║       1 ║          ║        ║      ║       ║      ║
  ║ sleep    ║       ║         ║          ║        ║    1 ║       ║      ║
  ║ wake     ║       ║         ║          ║        ║      ║     1 ║      ║
  ╚══════════╩═══════╩═════════╩══════════╩════════╩══════╩═══════╩═════*/

State2 MOTOR_STATE = MOTOR_STATE_IDLE;

long absolute_position_in_steps[2] = {0, 0};
bool new_position = false;


/*************************************************************
  Functions
*************************************************************/

void get_motor_position(Motor motors[]) {
  motors[0].steps = theta_stepper.currentPosition();
  motors[1].steps = radius_stepper.currentPosition();
}

void set_theta_motor_position(Motor motor) {
  theta_stepper.moveTo(motor.steps);
}

void set_radius_motor_position(Motor motor) {
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
  digitalWrite(PIN_SHIFT_LATCH, LOW);

  // TODO: If we need to go faster we could use the SPI hardware...
  // shift the bits out:
  shiftOut(PIN_SHIFT_DATA, PIN_SHIFT_CLOCK, MSBFIRST, motor_settings);

  // turn on the output so the LEDs can light up:
  digitalWrite(PIN_SHIFT_LATCH, HIGH);
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
  pinMode(PIN_SHIFT_LATCH, OUTPUT);
  pinMode(PIN_SHIFT_DATA, OUTPUT);
  pinMode(PIN_SHIFT_CLOCK, OUTPUT);

  // Full steps
  bitsWrite(motor_settings, PIN_MOTOR_XMS1, FULL_STEP, 2);
  bitsWrite(motor_settings, PIN_MOTOR_YMS1, FULL_STEP, 2);

  // Enable (Sleep is active low)
  bitWrite(motor_settings, PIN_MOTOR_ENABLEX, 1);
  bitWrite(motor_settings, PIN_MOTOR_ENABLEY, 1);

  // Turn off sleep (Sleep is active low)
  bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 1);
  bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 1);

  send_state();
}


void motor_loop()
{
  static unsigned long dataSendTimer = 0;
  static bool motor_settings_changed = true;

  switch (DATA_STATE) {
    case DATA_STATE_IDLE :
      // Only trigger a settings update from idle.
      if (motor_settings_changed) {
        DATA_STATE = DATA_STATE_CHANGED;
        /* FALL THROUGH */
      }
      else {
        break;
      }

    case DATA_STATE_CHANGED :
      send_state();
      dataSendTimer = millis() + 10;
      motor_settings_changed = false;
      DATA_STATE = DATA_STATE_SENDING;
      break;

    case DATA_STATE_SENDING :
      // Hold the state for 1ms for the drivers to pick it up.
      if (millis() > dataSendTimer) {
        DATA_STATE = DATA_STATE_IDLE;
      }
      break;

    default:
      break;
  }

  static unsigned long stoppedTimer = 0;
  static unsigned long idleTimer = 0;
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
      stoppedTimer = millis() + 1000;
      MOTOR_STATE = MOTOR_STATE_STOPPED;
    /* FALL THROUGH */
    case MOTOR_STATE_STOPPED :
      if (new_position) {
        MOTOR_STATE = MOTOR_STATE_TO_MOVING;
      }
      if (millis() > stoppedTimer) {
        MOTOR_STATE = MOTOR_STATE_TO_IDLE;
      }
      break;

    case MOTOR_STATE_TO_IDLE :
      idleTimer = millis() + 10000;
      MOTOR_STATE = MOTOR_STATE_IDLE;
    /* FALL THROUGH */
    case MOTOR_STATE_IDLE :
      if (new_position) {
        MOTOR_STATE = MOTOR_STATE_STOPPED;
      }
      if (millis() > idleTimer) {
        MOTOR_STATE = MOTOR_STATE_TO_SLEEP;
      }
      break;

    case MOTOR_STATE_TO_SLEEP :
      bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 0);
      bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 0);
      MOTOR_STATE = MOTOR_STATE_SLEEP;
    /* FALL THROUGH */
    case MOTOR_STATE_SLEEP :
      if (new_position) {
        MOTOR_STATE = MOTOR_STATE_TO_WAKE;
      }
      break;

    case MOTOR_STATE_TO_WAKE :
      bitWrite(motor_settings, PIN_MOTOR_SLEEPX, 1);
      bitWrite(motor_settings, PIN_MOTOR_SLEEPY, 1);
      MOTOR_STATE = MOTOR_STATE_WAKE;
    /* FALL THROUGH */
    case MOTOR_STATE_WAKE :
      MOTOR_STATE = MOTOR_STATE_TO_STOPPED;
      break;

    default:
      break;
  }
}
