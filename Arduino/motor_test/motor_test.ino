/*************************************************************
  Motor control
*************************************************************/

#include <AccelStepper.h>
#include <MultiStepper.h>

// The X Stepper pins
#define PIN_MOTOR_XSTEP 2
#define PIN_MOTOR_XDIR 3
#define PIN_MOTOR_XMS1 4
#define PIN_MOTOR_XMS2 5

// Define some steppers and the pins the will use
AccelStepper stepper(AccelStepper::DRIVER, PIN_MOTOR_XSTEP, PIN_MOTOR_XDIR);


static const char FULL_STEP = 0; // b00
static const char HALF_STEP = 2; // b10
static const char QUARTER_STEP = 1; // b01
static const char EIGHTH_STEP = 3; // b11

void setup() {
  Serial.begin(115200);
  Serial.write("Setup\n");
  // put your setup code here, to run once:

  pinMode(PIN_MOTOR_XMS1, OUTPUT);
  pinMode(PIN_MOTOR_XMS2, OUTPUT);

  digitalWrite(PIN_MOTOR_XMS1, HIGH);
  digitalWrite(PIN_MOTOR_XMS2, HIGH);

   stepper.setMaxSpeed(650*9);
  stepper.setMinPulseWidth(20); // Driver supports 1us
  stepper.setAcceleration(400.0*8);

   stepper.setSpeed(650*9);  
   Serial.write("Setup done\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  stepper.runSpeed();
}
