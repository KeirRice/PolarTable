/*************************************************************
  Event

  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
*************************************************************/
#pragma once

#include "Arduino.h"
#include "EventTypes.h"

#define INTERVAL_SLOT_COUNT 5
#define SUBSCRIBER_SLOT_COUNT 10

#define MAX_LONG 2147483647


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


struct EventListener : public EventTask
{
  using EventTask::execute;
  
  void execute(Event *evt){
    // fsm_system.trigger((int)*(evt->extra));
  }

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
    int getFreeSlot();
    void clearSlot(int slot);
    
    TimedTask* _interval[INTERVAL_SLOT_COUNT];
    unsigned int _intervalCount;

    // Keep a cache of the next timer up so we don't need to test them all.
    unsigned long _next_event_ms;
    
    Subscriber* _sub[SUBSCRIBER_SLOT_COUNT];
    unsigned int _subCount;
    unsigned int _subPos;
};

