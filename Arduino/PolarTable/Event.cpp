/*
  Event

  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
*/

#include "Event.h"

/**
   Event structure is the basic Event
   object that can be dispatched by the
   manager.
*/
Event::Event(const EventID cLabel, const void *cExtra) : label(cLabel), extra(cExtra) {}

/**
   The Subscriber is the object that
   encapsulates the Event it's listening for
   and the EventTask to be executed.
*/
Subscriber::Subscriber() : label(0), callback(NULL), callback_data(NULL) {}
Subscriber::Subscriber(const EventID cLabel, void (*callback)()): label(cLabel), callback(callback), callback_data(NULL) {}
Subscriber::Subscriber(const EventID cLabel, void (*callback)(void* userData)): label(cLabel), callback(NULL), callback_data(callback) {}


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

  _next_event_ms = 0;
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
  for (unsigned int i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];

    if (sub and sub->label == label)
    {
      // Execute event
      (sub->callback_data)(evt);
    }
  }
}

void EventManager::trigger(const EventID cLabel)
{
  for (unsigned int i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];
    if (sub and sub->label == cLabel)
    {
      (sub->callback)(); 
    }
  }
}

void EventManager::trigger(const EventID cLabel, void *cExtra)
{
  for (unsigned int i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];

    if (sub and sub->label == cLabel)
    {
      // Execute event
      if(sub->callback_data){
        (sub->callback_data)(&cExtra);        
      }
      else {
        (sub->callback)(); 
      }
    }
  }
}

void EventManager::trigger(const EventID cLabel, const void *cExtra)
{
  for (unsigned int i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];

    if (sub and sub->label == cLabel)
    {
      // Execute event
      if(sub->callback_data){
        (sub->callback_data)(&cExtra);        
      }
      else {
        (sub->callback)(); 
      }
    }
  }
}

void EventManager::trigger(const EventID cLabel, const EventID cExtra)
{
  for (unsigned int i = 0; i < _subCount; ++i)
  {
    Subscriber *sub = _sub[i];

    if (sub and sub->label == cLabel)
    {
      // Execute event
      if(sub->callback_data){
        (sub->callback_data)(&cExtra);        
      }
      else {
        (sub->callback)(); 
      }
    }
  }
}

/**
   Setup a timed trigger that will execute an
   event after a couple of milliseconds.
*/
void EventManager::triggerInterval(TimedTask *task)
{
  int slot = getFreeSlot();
  if (slot != -1){
    _interval[slot] = task;
    _next_event_ms = min(_next_event_ms, task->target_ms);
  }
}

int EventManager::getFreeSlot(){
  TimedTask *task;
  for(unsigned int i = 0; i < _intervalCount ; ++i){
    task = _interval[0];
    if(task == nullptr){
      return i;
    }
  }
//  DEBUG_PRINTLN("No free timer event slots.");
//  error_listener(ERROR_EVENT_SYSTEM); // Event system can't really use it's self...
  return -1;
}

void EventManager::clearSlot(int slot){
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
  if(_next_event_ms != 0  && _next_event_ms <= millis()){
    unsigned long currentMs = millis();
    TimedTask *task;

    _next_event_ms = MAX_LONG;
    
    for(unsigned int i = 0; i < _intervalCount ; ++i){
      task = _interval[i];
      if (currentMs >= task->target_ms){
          trigger(task->evt);
          clearSlot(i);
      }
      else {
        // Re cache our next timer.
        _next_event_ms = min(_next_event_ms, task->target_ms);
      }
    }
  }
}

void EventManager::resetIntervals()
{
  _next_event_ms = MAX_LONG;
  for(unsigned int i = 0; i < _intervalCount ; ++i){
    clearSlot(i);
  }
}
