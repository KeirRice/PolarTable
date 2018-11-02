/*************************************************************
  Events that can be send between modules
*************************************************************/
#pragma once

/*************************************************************
  All the events in that can be generated
*************************************************************/

// EventID
//  * Group
//  * Action
//  * State

typedef int EventID;
typedef int SystemID;
typedef int ActionID;
typedef int StateID;

ActionID Shutdown = 0;
ActionID PowerOn = 1;
ActionID Sleep = 2;
ActionID Wake = 3;
ActionID Pulse = 4;
ActionID Stop = 5;
ActionID SetPosition = 6;
ActionID SetDirection = 7;
ActionID SetState = 8;
ActionID SetColor = 9;
ActionID Blend = 10;
ActionID Send = 11;
ActionID Receive = 12;
StateID Off = 13;
StateID On = 14;
StateID Asleep = 15;
StateID Awake = 16;
StateID Pulsing = 17;
StateID Stopped = 18;

// bit 8 state vs action
int TypeAction = 1 << 8;
int TypeState = 2 << 8;

// 7 high bits for system
int system_id_base = 1 << 9;
SystemID Global = system_id_base << 1;
SystemID System = system_id_base << 2;
SystemID Movement = system_id_base << 3;
SystemID Lighting = system_id_base << 4;
SystemID Communication = system_id_base << 5;
SystemID Error = system_id_base << 6;



enum System {
#   define X(a, b) a,
// #   include "enum/system.h"
    X(On, "on")
    X(Off, "off")
#   undef X
    SystemCount
};

char const* const system_str[] = {
#   define X(a, b) #b,
// #   include "enum/system.h"
    X(On, "on")
    X(Off, "off")
#   undef X
    0
};

// Group, A high level event to narrow down which system need the real event.
// Action: A thing that is active, a verb
// State: They way things are now, a noun.



static const GroupID NULL_EVENT = 0;




static const EventID NULL_EVENT = 0;

static const EventID SYSTEM_EVENT = 5; // Event group
// System statemachine states.
static const EventID SYSTEM_SLEEP = 10; // action
static const EventID SYSTEM_WAKE = 11; // action


static const EventID BUTTON_LED = 14; // Event group
// Button LED stat machine states.
static const EventID BUTTON_ON = 15; // state/action
static const EventID BUTTON_OFF = 16; // state/action
static const EventID BUTTON_PULSE = 17; // state/action
static const EventID BUTTON_PULSE_ON = 18; // action
static const EventID BUTTON_PULSE_OFF = 19; // action

static const EventID MOTOR_SET_THETA_POS_REQUEST = 30; // action
static const EventID MOTOR_SET_RADIUS_POS_REQUEST = 31; // action
static const EventID MOTOR_SET_THETA_DIR_REQUEST = 32; // action
static const EventID MOTOR_SET_RADIUS_DIR_REQUEST = 33; // action
static const EventID MOTOR_SET_SETTINGS_REQUEST = 34; // action

static const EventID MOTOR_NEW_POSITION = 35; // action
static const EventID MOTOR_STOP = 36; // state/action
static const EventID MOTOR_TARGET_THETA = 37; // action
static const EventID MOTOR_TARGET_RADIUS = 38; // action
static const EventID MOTOR_READY_FOR_DATA = 39; // state


static const EventID LED_ON_REQUEST = 50; // action
static const EventID LED_OFF_REQUEST = 51; // action
static const EventID LED_COLOR_CHANGE = 52; // action

static const EventID LED_SHUTDOWN = 53; // action
static const EventID LED_OFF = 54; // action/state
static const EventID LED_ON = 55; // action/state
static const EventID LED_BLEND = 56; // action/state
static const EventID LIGHTING_STATE = 57; // ?
static const EventID LIGHTING_COLOR = 58; // ?

static const EventID RASPBERRY_EVENT = 60; // event group
static const EventID RASPBERRY_SHUTDOWN = 61; // action
static const EventID RASPBERRY_RESTART = 62; // action
static const EventID RASPBERRY_STARTUP = 63; // action
static const EventID RASPBERRY_HEARTBEAT = 64; // action


static const EventID ERROR_LED_SIGNAL = 127; // event group
static const EventID ERROR_EVENT = 128;
static const EventID ERROR_SERIAL = 129;
static const EventID ERROR_SX1509 = 130;
static const EventID ERROR_MOTOR = 131;
static const EventID ERROR_RASPBERRY = 132;
static const EventID ERROR_EVENT_SYSTEM = 133;
static const EventID ERROR_RELATIVE_ENCODER = 134;
