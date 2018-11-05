/*************************************************************
  Events that can be send between modules
*************************************************************/
#pragma once
#include "EventTypes.h"

static int event_id_counter = 0;

ActionID SHUTDOWN(event_id_counter++);
ActionID POWERON(event_id_counter++);
ActionID SLEEP(event_id_counter++);
ActionID WAKE(event_id_counter++);
ActionID PULSE(event_id_counter++);
ActionID STOP(event_id_counter++);
ActionID SETPOSITION(event_id_counter++);
ActionID SETDIRECTION(event_id_counter++);
ActionID SETSTATE(event_id_counter++);
ActionID SETCOLOR(event_id_counter++);
ActionID BLEND(event_id_counter++);
ActionID SEND(event_id_counter++);
ActionID RECEIVE(event_id_counter++);
ActionID TURN_ON(event_id_counter++);
ActionID TURN_OFF(event_id_counter++);
ActionID PULSE_ON(event_id_counter++);
ActionID PULSE_OFF(event_id_counter++);

ActionID POSITION(event_id_counter++);
ActionID REL_POSITION(event_id_counter++);
ActionID ABS_POSITION(event_id_counter++);
ActionID DIRECTION(event_id_counter++);
ActionID REL_DIRECTION(event_id_counter++);
ActionID ABS_DIRECTION(event_id_counter++);

StateID OFF(event_id_counter++);
StateID ON(event_id_counter++);
StateID ASLEEP(event_id_counter++);
StateID AWAKE(event_id_counter++);
StateID PULSING(event_id_counter++);
StateID STOPPED(event_id_counter++);
StateID BLENDING(event_id_counter++);


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

static const EventID MOTOR_SET_THETA_POS_REQUEST = SystemID(30); // action
static const EventID MOTOR_SET_RADIUS_POS_REQUEST = SystemID(31); // action
static const EventID MOTOR_SET_THETA_DIR_REQUEST = SystemID(32); // action
static const EventID MOTOR_SET_RADIUS_DIR_REQUEST = SystemID(33); // action
static const EventID MOTOR_SET_SETTINGS_REQUEST = SystemID(34); // action

static const EventID MOTOR_NEW_POSITION = SystemID(35); // action
static const EventID MOTOR_STOP = SystemID(36); // state/action
static const EventID MOTOR_TARGET_THETA = SystemID(37); // action
static const EventID MOTOR_TARGET_RADIUS = SystemID(38); // action
static const EventID MOTOR_READY_FOR_DATA = SystemID(39); // state


// Lighting Actions
static const EventID LIGHTING_TURN_ON = LIGHTING | TURN_ON;
static const EventID LIGHTING_TURN_OFF = LIGHTING | TURN_OFF;
static const EventID LIGHTING_BLEND = LIGHTING | BLEND;
// Lighting States
static const EventID LIGHTING_ON = LIGHTING | ON;
static const EventID LIGHTING_OFF = LIGHTING | OFF;

static const EventID LIGHTING_COLOR = LIGHTING | STATE; // ?

static const EventID RASPBERRY_EVENT = SystemID(60); // event group
static const EventID RASPBERRY_SHUTDOWN = SystemID(61); // action
static const EventID RASPBERRY_RESTART = SystemID(62); // action
static const EventID RASPBERRY_STARTUP = SystemID(63); // action
static const EventID RASPBERRY_HEARTBEAT = SystemID(64); // action


static const EventID ERROR_LED_SIGNAL = SystemID(127); // event group
static const EventID ERROR_EVENT = SystemID(128);
static const EventID ERROR_SERIAL = SystemID(129);
static const EventID ERROR_SX1509 = SystemID(130);
static const EventID ERROR_MOTOR = SystemID(131);
static const EventID ERROR_RASPBERRY = SystemID(132);
static const EventID ERROR_EVENT_SYSTEM = SystemID(133);
static const EventID ERROR_REL_DIRECTION = ERROR | REL_DIRECTION;
