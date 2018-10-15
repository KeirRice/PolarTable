/*
  Event
  
  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
 */ 
 
#ifndef Event_h
#define Event_h

#include "Arduino.h"

typedef char EventID;
#define INTERVAL_SLOT_COUNT 0
#define SUBSCRIBER_SLOT_COUNT 2

#include "ProjectEvents.h"

/**
 * Event structure is the basic Event
 * object that can be dispatched by the
 * manager.
 */
struct Event
{
  Event(const EventID cLabel, const void *cExtra=0) : label(cLabel), extra(cExtra) {}
  const EventID label;
  const void *extra;
};

/**
 * EventTask is a structure that serves as an
 * abstract class of a "dispatchable" object.
 */
struct EventTask
{
  virtual void execute(Event evt) = 0;
};

/**
 * The Subscriber is the object that
 * encapsulates the Event it's listening for
 * and the EventTask to be executed.
 */
struct Subscriber
{
  Subscriber(const EventID *cLabel, void (*func)(Event evt)) : label(cLabel), task(func) {}
  const EventID *label;
  const void (*task)(Event evt);
};

/**
 * TimedTask is an Event that executes after a certain
 * amount of milliseconds.
 */
struct TimedTask
{
  TimedTask() : target_ms(0), evt(NULL)  {}
  TimedTask(unsigned long t_ms, Event cEvt) : target_ms(t_ms), evt(cEvt) {}
  
  /**
   * Evaluates the state of the timed task and if
   * it's time to execute it or not. Resets the current
   * counter if it reaches the timed threshold.
   */
  boolean eval(unsigned long current_ms);
  
  const unsigned long target_ms;
  Event evt;
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
    void trigger(const Event evt);
    void trigger(const EventID cLabel, const void *cExtra=0);
    void triggerInterval(TimedTask timed);
    void tick();
    
  private:
    TimedTask* _interval[INTERVAL_SLOT_COUNT];
    unsigned int _intervalCount;
    unsigned int _intervalPos;
    
    Subscriber* _sub[SUBSCRIBER_SLOT_COUNT];
    unsigned int _subCount;
    unsigned int _subPos;
};

#endif
