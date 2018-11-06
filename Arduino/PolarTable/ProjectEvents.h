/*************************************************************
  Events that can be send between modules
*************************************************************/
#pragma once
#include "EventTypes.h"

static int event_id_counter = 0;

#if DEBUG == 1
#define BuildAction(a) a(event_id_counter++, (char*) #a)
#else
#define BuildAction(a) a(event_id_counter++)
#endif // DEBUG

ActionID BuildAction(SHUTDOWN);
ActionID BuildAction(POWERON);
ActionID BuildAction(RESTART);
ActionID BuildAction(SLEEP);
ActionID BuildAction(WAKE);
ActionID BuildAction(PULSE);
ActionID BuildAction(STOP);
ActionID BuildAction(MOVE);
ActionID BuildAction(SETSTATE);
ActionID BuildAction(SETCOLOR);
ActionID BuildAction(BLEND);
ActionID BuildAction(SEND);
ActionID BuildAction(RECEIVE);
ActionID BuildAction(TURN_ON);
ActionID BuildAction(TURN_OFF);
ActionID BuildAction(PULSE_ON);
ActionID BuildAction(PULSE_OFF);

ActionID BuildAction(POSITION);
ActionID BuildAction(REL_POSITION);
ActionID BuildAction(ABS_POSITION);
ActionID BuildAction(DIRECTION);
ActionID BuildAction(REL_DIRECTION);
ActionID BuildAction(ABS_DIRECTION);

StateID OFF(event_id_counter++);
StateID ON(event_id_counter++);
StateID ASLEEP(event_id_counter++);
StateID AWAKE(event_id_counter++);
StateID PULSING(event_id_counter++);
StateID BLENDING(event_id_counter++);
StateID STOPPED(event_id_counter++);
StateID MOVING(event_id_counter++);
StateID READY(event_id_counter++);


/*************************************************************
  All the events in that can be generated
*************************************************************/

// System statemachine states.
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
static const EventID MOTOR_TARGET_THETA = MOTOR_THETA | POSITION;
static const EventID MOTOR_TARGET_RADIUS = MOTOR_RADIUS | POSITION;


// Lighting Actions
static const EventID LIGHTING_TURN_ON = LIGHTING | TURN_ON;
static const EventID LIGHTING_TURN_OFF = LIGHTING | TURN_OFF;
static const EventID LIGHTING_BLEND = LIGHTING | BLEND;
// Lighting States
static const EventID LIGHTING_ON = LIGHTING | ON;
static const EventID LIGHTING_OFF = LIGHTING | OFF;
static const EventID LIGHTING_COLOR = LIGHTING | STATE;

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
static const EventID ERROR_RASPBERRY = SystemID(132);
static const EventID ERROR_EVENT_SYSTEM = SystemID(133);
static const EventID ERROR_REL_DIRECTION = ERROR | REL_DIRECTION;
