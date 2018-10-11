#pragma once

/*************************************************************
All the events in that can be generated
*************************************************************/

typedef unsigned char Event;

static const Event NULL_EVENT = 0;

static const Event SLEEP_REQUEST = 10;
static const Event WAKE_REQUEST = 11;

static const Event MOTOR_SET_THETA_POS_REQUEST = 30;
static const Event MOTOR_SET_RADIUS_POS_REQUEST = 31;
static const Event MOTOR_SET_THETA_DIR_REQUEST = 32;
static const Event MOTOR_SET_RADIUS_DIR_REQUEST = 33;
static const Event MOTOR_SET_SETTINGS_REQUEST = 34;

static const Event LED_ON_REQUEST = 50;
static const Event LED_OFF_REQUEST = 51;
static const Event LED_COLOR_CHANGE = 52;
