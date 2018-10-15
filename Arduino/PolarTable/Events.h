/*************************************************************
  Events that can be send between modules
*************************************************************/

#pragma once

/*************************************************************
  All the events in that can be generated
*************************************************************/

typedef char EventID;

static const EventID NULL_EVENT = 0;

static const EventID SLEEP_REQUEST = 10;
static const EventID WAKE_REQUEST = 11;
static const EventID SLEEP = 12;

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

static const EventID LED_ON_REQUEST = 50;
static const EventID LED_OFF_REQUEST = 51;
static const EventID LED_COLOR_CHANGE = 52;
