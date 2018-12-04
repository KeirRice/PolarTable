/*************************************************************
  Event

  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
*************************************************************/

#pragma once

#include "Arduino.h"
#include "EventID.h"

#ifndef INTERVAL_SLOT_COUNT
#define INTERVAL_SLOT_COUNT 1
#endif  // INTERVAL_SLOT_COUNT
#ifndef SUBSCRIBER_SLOT_COUNT
#define SUBSCRIBER_SLOT_COUNT 3
#endif  // SUBSCRIBER_SLOT_COUNT

#ifndef ENABLE_DEBUG_STRINGS
#define ENABLE_DEBUG_STRINGS 0
#endif  // ENABLE_DEBUG_STRINGS

#ifndef MAX_LONG
#define MAX_LONG 2147483647
#endif  // MAX_LONG


static long event_id_mask =   0b00000000000000000000000001111111;
static long action_id_mask =  0b00000000000000000000000010000000;
static long state_id_mask =   0b00000000000000000000000100000000;
static long system_id_mask =  0b11111111111111111111111000000000;


/**
 * Event structure is the basic Event
 * object that can be dispatched by the
 * manager.
 */
struct Event
{
  Event(const EventID cLabel, const void *cExtra=0);
  const EventID label;
  const void *extra;
};


/**
 * EventTask is a structure that serves as an
 * abstract class of a "dispatchable" object.
 */
struct EventTask
{
  virtual void execute(Event *evt) = 0;
};


/**
 * The Subscriber is the object that
 * encapsulates the Event it's listening for
 * and the EventTask to be executed.
 */
struct Subscriber
{
  Subscriber();
  Subscriber(const EventID cLabel, EventTask *task);
  
  const EventID label;
  EventTask *task;
};


/**
 * TimedTask is an Event that executes after a certain
 * amount of milliseconds.
 */
struct TimedTask
{
  TimedTask();
  TimedTask(unsigned long t_ms, Event *cEvt);
  
  const unsigned long target_ms;
  Event *evt;
};


/**
 * The EventManager is responsible for gathering subscribers
 * and dispatching them when the requested Event is
 * triggered.
 */
class EventManager
{
  public:
    EventManager();
    void subscribe(Subscriber sub);

    void trigger(Event *evt);
    void trigger(const EventID cLabel);
    void trigger(const EventID cLabel, void *cExtra);
    void trigger(const EventID cLabel, const void *cExtra);
    void trigger(const EventID cLabel, const EventID cExtra);
    
    void triggerInterval(TimedTask *timed);
    void tick();

    // After waking up none of our intervals make sense any more.
    void resetIntervals();
    
  private:
    unsigned char getFreeSlot();
    void clearSlot(unsigned char slot);
    
    TimedTask* _interval[INTERVAL_SLOT_COUNT];
    unsigned int _intervalCount;

    // Keep a cache of the next timer up so we don't need to test them all.
    inline void setNextEventMS(long next_event_ms);
    // Set only if smaller than existing valuec
    inline void minSetNextEventMS(long next_event_ms);
    unsigned long _next_event_ms;
    
    Subscriber* _sub[SUBSCRIBER_SLOT_COUNT];
    unsigned char _subCount;
    unsigned char _subPos;
};
