/*************************************************************
  Events that can be send between modules
*************************************************************/
#pragma once

#include <Flash.h>
#include "EventTypes.h"

// BuildSystem uses one (high) bit per entry so these values can be ORed and masked.
BuildSystem(SystemID, GLOBAL);
BuildSystem(SystemID, SYSTEM);
BuildSystem(SystemID, MOVEMENT);
BuildSystem(SystemID, CALIBRATION);
BuildSystem(SystemID, REL_MOVEMENT);
BuildSystem(SystemID, ABS_MOVEMENT);
BuildSystem(SystemID, MOTOR);
BuildSystem(SystemID, MOTOR_THETA);
BuildSystem(SystemID, MOTOR_RADIUS);
BuildSystem(SystemID, LIGHTING);
BuildSystem(SystemID, BUTTON);
BuildSystem(SystemID, COMMUNICATION);
BuildSystem(SystemID, ERROR);
BuildSystem(SystemID, RASPBERRY);

// BuildEvents are just sequencial ids, they will be unique, but don't do bit wise opperations on them.
BuildEvent(ActionID, SHUTDOWN);
BuildEvent(ActionID, POWERON);
BuildEvent(ActionID, RESTART);
BuildEvent(ActionID, SLEEP);
BuildEvent(ActionID, WAKE);
BuildEvent(ActionID, PULSE);
BuildEvent(ActionID, START);
BuildEvent(ActionID, STOP);
BuildEvent(ActionID, MOVE);
BuildEvent(ActionID, SETSTATE);
BuildEvent(ActionID, SETCOLOR);
BuildEvent(ActionID, SET);
BuildEvent(ActionID, RED);
BuildEvent(ActionID, GREEN);
BuildEvent(ActionID, BLUE);
BuildEvent(ActionID, TIME);
BuildEvent(ActionID, COLOUR);
BuildEvent(ActionID, BLEND);
BuildEvent(ActionID, SEND);
BuildEvent(ActionID, RECEIVE);
BuildEvent(ActionID, TURN_ON);
BuildEvent(ActionID, TURN_OFF);
BuildEvent(ActionID, PULSE_ON);
BuildEvent(ActionID, PULSE_OFF);
BuildEvent(ActionID, POSITION);
BuildEvent(ActionID, REL_POSITION);
BuildEvent(ActionID, ABS_POSITION);
BuildEvent(ActionID, DIRECTION);
BuildEvent(ActionID, REL_DIRECTION);
BuildEvent(ActionID, ABS_DIRECTION);

BuildEvent(StateID, OFF);
BuildEvent(StateID, ON);
BuildEvent(StateID, ASLEEP);
BuildEvent(StateID, AWAKE);
BuildEvent(StateID, PULSING);
BuildEvent(StateID, BLENDING);
BuildEvent(StateID, STOPPED);
BuildEvent(StateID, MOVING);
BuildEvent(StateID, READY);


/*************************************************************
  All the events in that can be generated
*************************************************************/

// System statemachine states.
static const EventID SYSTEM_STATE = SYSTEM | STATE;
static const EventID SYSTEM_SLEEP_ACTION = SYSTEM | SLEEP;
static const EventID SYSTEM_WAKE_ACTION = SYSTEM | WAKE;

// Button LED stat machine states.
static const EventID BUTTON_ON = BUTTON | TURN_ON;
static const EventID BUTTON_OFF = BUTTON | TURN_OFF;
static const EventID BUTTON_PULSE = BUTTON | PULSE;
static const EventID BUTTON_PULSE_ON = BUTTON | PULSE_ON;
static const EventID BUTTON_PULSE_OFF = BUTTON | PULSE_OFF;

static const EventID MOTOR_STOP = MOVEMENT | STOP;
static const EventID MOTOR_STOPPED = MOVEMENT | STOPPED;
static const EventID MOTOR_MOVE = MOVEMENT | MOVE;
static const EventID MOTOR_READY = MOVEMENT | READY;
static const EventID MOTOR_THETA_POSITION = MOTOR_THETA | POSITION;
static const EventID MOTOR_RADIUS_POSITION = MOTOR_RADIUS | POSITION;
static const EventID MOTOR_THETA_DIRECTION = MOTOR_THETA | DIRECTION;
static const EventID MOTOR_RADIUS_DIRECTION = MOTOR_RADIUS | DIRECTION;
static const EventID MOTOR_THETA_STOP = MOVEMENT | MOTOR_THETA | STOP;
static const EventID MOTOR_RADIUS_STOP = MOVEMENT | MOTOR_RADIUS | STOP;
static const EventID MOTOR_SET = MOVEMENT | SET;
static const EventID CALIBRATION_START = CALIBRATION | START;
static const EventID CALIBRATION_STOP = CALIBRATION | STOP;



// Lighting Actions
static const EventID LIGHTING_SET_STATE = LIGHTING | STATE;
static const EventID LIGHTING_TURN_ON = LIGHTING | TURN_ON;
static const EventID LIGHTING_TURN_OFF = LIGHTING | TURN_OFF;
static const EventID LIGHTING_BLEND = LIGHTING | BLEND;
static const EventID LIGHTING_SET_RED = LIGHTING | RED;
static const EventID LIGHTING_SET_GREEN = LIGHTING | GREEN;
static const EventID LIGHTING_SET_BLUE = LIGHTING | BLUE;
static const EventID LIGHTING_SET_COLOUR = LIGHTING | BLUE;
static const EventID LIGHTING_SET_BLEND_TIME = LIGHTING | SETCOLOR;

// Lighting States
static const EventID LIGHTING_ON = LIGHTING | ON;
static const EventID LIGHTING_OFF = LIGHTING | OFF;
static const EventID LIGHTING_COLOR = LIGHTING | COLOUR;

static const EventID RASPBERRY_STARTUP = RASPBERRY | POWERON;
static const EventID RASPBERRY_SHUTDOWN = RASPBERRY | SHUTDOWN;
static const EventID RASPBERRY_RESTART = RASPBERRY | RESTART;
static const EventID RASPBERRY_HEARTBEAT = RASPBERRY | PULSE;
static const EventID RASPBERRY_ON = RASPBERRY | ON;


static const EventID ERROR_LED_SIGNAL = SystemID(127); // event group
static const EventID ERROR_EVENT = SystemID(128);
static const EventID ERROR_SERIAL = SystemID(129);
static const EventID ERROR_SX1509 = SystemID(130);
static const EventID ERROR_MOTOR = ERROR | MOVEMENT;
static const EventID ERROR_RASPBERRY_SEND = ERROR | RASPBERRY /*| SEND*/;
static const EventID ERROR_EVENT_SYSTEM = SystemID(133);
static const EventID ERROR_REL_DIRECTION = ERROR | REL_DIRECTION;
