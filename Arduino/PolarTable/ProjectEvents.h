/*************************************************************
  Events that can be send between modules
*************************************************************/
#pragma once

/*************************************************************
  All the events in that can be generated
*************************************************************/

typedef char EventID;

static const EventID NULL_EVENT = 0;

static const EventID SYSTEM_EVENT = 5;
// System statemachine states.
static const EventID SYSTEM_SLEEP = 10;
static const EventID SYSTEM_WAKE = 11;


static const EventID BUTTON_LED = 14;

static const EventID BUTTON_ON = 15;
static const EventID BUTTON_OFF = 16;
static const EventID BUTTON_PULSE = 17;
static const EventID BUTTON_PULSE_ON = 18;
static const EventID BUTTON_PULSE_OFF = 19;

static const EventID MOTOR_SET_THETA_POS_REQUEST = 30;
static const EventID MOTOR_SET_RADIUS_POS_REQUEST = 31;
static const EventID MOTOR_SET_THETA_DIR_REQUEST = 32;
static const EventID MOTOR_SET_RADIUS_DIR_REQUEST = 33;
static const EventID MOTOR_SET_SETTINGS_REQUEST = 34;

static const EventID MOTOR_NEW_POSITION = 35;
static const EventID MOTOR_STOP = 36;
static const EventID MOTOR_TARGET_THETA = 37;
static const EventID MOTOR_TARGET_RADIUS = 38;
static const EventID MOTOR_READY_FOR_DATA = 39;


static const EventID LED_ON_REQUEST = 50;
static const EventID LED_OFF_REQUEST = 51;
static const EventID LED_COLOR_CHANGE = 52;

static const EventID LED_SHUTDOWN = 53;
static const EventID LED_OFF = 54;
static const EventID LED_ON = 55;
static const EventID LED_BLEND = 56;
static const EventID LIGHTING_STATE = 57;
static const EventID LIGHTING_COLOR = 58;

static const EventID RASPBERRY_EVENT = 60;
static const EventID RASPBERRY_SHUTDOWN = 61;
static const EventID RASPBERRY_RESTART = 62;
static const EventID RASPBERRY_STARTUP = 63;
static const EventID RASPBERRY_HEARTBEAT = 64;


static const EventID ERROR_LED_SIGNGAL = 127;
static const EventID ERROR_EVENT = 128;
static const EventID ERROR_SERIAL = 129;
static const EventID ERROR_SX1509 = 130;
static const EventID ERROR_MOTOR = 131;
static const EventID ERROR_RASPBERRY = 132;
static const EventID ERROR_EVENT_SYSTEM = 133;
static const EventID ERROR_RELATIVE_ENCODER = 134;
