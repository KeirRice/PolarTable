/*************************************************************
  Event

  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
*************************************************************/

#include "kEvent.h"

#if INTERVAL_SLOT_COUNT > 10
#define ENABLE_FANCY_TIMER
#endif //INTERVAL_SLOT_COUNT > 10


/**
   Event structure is the basic Event
   object that can be dispatched by the
   manager.
*/
Event::Event(const EventID cLabel, const void *cExtra /*=0*/) : label(cLabel), extra(cExtra) {}


/**
   The Subscriber is the object that
   encapsulates the Event it's listening for
   and the EventTask to be executed.
*/
Subscriber::Subscriber() : label(0), task(NULL) {}
Subscriber::Subscriber(const EventID cLabel, EventTask *task): label(cLabel), task(task) {}



/**
   TimedTask is an Event that executes after a certain
   amount of milliseconds.
*/
TimedTask::TimedTask() : target_ms(0), evt(NULL) {}
TimedTask::TimedTask(unsigned long t_ms, Event *cEvt) : target_ms(t_ms), evt(cEvt) {}


/**
   Constructs a new EventManager and
   figures out the size of the available
   array slots.
*/
EventManager::EventManager()
{
  _intervalCount = sizeof(_interval) / sizeof(TimedTask);
  
  _subCount = sizeof(_sub) / sizeof(Subscriber);
  _subPos = 0;

  setNextEventMS(0);
}

/**
   Subscribes a new Subscriber to the
   event manager.
*/
void EventManager::subscribe(Subscriber sub)
{
  if (_subCount >= _subPos)
  {
    _sub[_subPos++] = &sub;
  }
}

/**
   Triggers a specified event which will find the applicable
   Subscriber and execute it's EventTask
*/
void EventManager::trigger(Event *evt)
{
  const EventID label = evt->label;
  long system_label = (label & system_id_mask);
  
  for (unsigned char i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];
    if (sub)
    {
      if (sub->label == label)  // Check for exact match
      {
        // Execute event
        (sub->task->execute)(evt);
      }
      else {
        // Check for match in just the system_id bits.
        bool sub_wants_system_labels = (sub->label & system_id_mask) == 0;
        if(sub_wants_system_labels and (system_label == label)){
          // Execute event
          (sub->task->execute)(evt);            
        }
      }    
    }
  }
}

void EventManager::trigger(const EventID cLabel)
{
  Event evt = Event(cLabel);
  EventManager::trigger(&evt);
}

void EventManager::trigger(const EventID cLabel, void *cExtra)
{
  Event evt = Event(cLabel, cExtra);
  EventManager::trigger(&evt);
}

void EventManager::trigger(const EventID cLabel, const void *cExtra)
{
  Event evt = Event(cLabel, cExtra);
  EventManager::trigger(&evt);
}

void EventManager::trigger(const EventID cLabel, const EventID fsm_event)
{
  Event evt = Event(cLabel, (void*)&fsm_event);
  EventManager::trigger(&evt);
}

/**
   Setup a timed trigger that will execute an
   event after a couple of milliseconds.
*/
void EventManager::triggerInterval(TimedTask *task)
{
  unsigned char slot = getFreeSlot();
  if (slot != 255){
    _interval[slot] = task;
    minSetNextEventMS(task->target_ms);
  }
}

unsigned char EventManager::getFreeSlot(){
  TimedTask *task;
  for(unsigned char i = 0; i < _intervalCount ; ++i){
    task = _interval[0];
    if(task == nullptr){
      return i;
    }
  }
  // DEBUG_PRINTLN("No free timer event slots.");
  return 255;
}

void EventManager::clearSlot(unsigned char  slot){
  TimedTask *task = _interval[slot];
  delete task;
  _interval[slot] = nullptr;
}

/**
   Tick the EventManager to evaluate any
   timed instances for the manager.
*/
void EventManager::tick()
{
  // Fancy sorting and caching of times only makes sense if there are heaps of misses per tick
#if INTERVAL_SLOT_COUNT > 0
#if INTERVAL_SLOT_COUNT > 10
  if(_next_event_ms != 0  && _next_event_ms <= millis()){
    unsigned long currentMs = millis();
    TimedTask *task;

    setNextEventMS(MAX_LONG);
    
    for(unsigned char i = 0; i < _intervalCount ; ++i){
      task = _interval[i];
      if (currentMs >= task->target_ms){
          trigger(task->evt);
          clearSlot(i);
      }
      else {
        // Re cache our next timer.
        minSetNextEventMS(task->target_ms);
      }
    }
  }
#else // INTERVAL_SLOT_COUNT > 10
  unsigned long currentMs = millis();
  TimedTask *task;
  for(unsigned char i = 0; i < _intervalCount ; ++i){
    task = _interval[i];
    if (currentMs >= task->target_ms){
        trigger(task->evt);
        clearSlot(i);
    }
  }
#endif // INTERVAL_SLOT_COUNT > 10
#endif // INTERVAL_SLOT_COUNT > 0
}

void EventManager::resetIntervals()
{
  setNextEventMS(MAX_LONG);
  for(unsigned char i = 0; i < _intervalCount ; ++i){
    clearSlot(i);
  }
}


inline void EventManager::setNextEventMS(long next_event_ms){
#ifdef ENABLE_FANCY_TIMER
  _next_event_ms = next_event_ms;
#endif // ENABLE_FANCY_TIMER
}
inline void EventManager::minSetNextEventMS(long next_event_ms){
#ifdef ENABLE_FANCY_TIMER
  _next_event_ms = min(_next_event_ms, next_event_ms);
#endif // ENABLE_FANCY_TIMER
}