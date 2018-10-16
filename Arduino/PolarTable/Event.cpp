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
  _intervalPos = 0;

  _subCount = sizeof(_sub) / sizeof(Subscriber);
  _subPos = 0;
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
void EventManager::triggerInterval(TimedTask task)
{
  if (_intervalCount >= _intervalPos)
  {
    _interval[_intervalPos++] = &task;
  }
}


boolean TimedTask::eval(unsigned long current_ms) {
  if (current_ms >= target_ms and target_ms > 0)
  {
    return true;
  }
  return false;
}

/**
   Tick the EventManager to evaluate any
   timed instances for the manager.
*/
void EventManager::tick()
{
  if (_intervalPos == 0) {
    return;
  }

  unsigned long currentMs = millis();

  for (unsigned int i = 0; i < _intervalCount; ++i)
  {
    TimedTask *task = _interval[i];

    if (task)
    {
      if (task->eval(currentMs))
      {
        // Run the timed event when it evalutes to
        // ready.
        trigger(task->evt);
        _interval[i] = nullptr;
      }
    }
  }
}
