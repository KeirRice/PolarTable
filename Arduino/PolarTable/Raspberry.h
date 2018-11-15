/*************************************************************
  Communicate with the Rasberry Pi
*************************************************************/
#pragma once

#ifdef DISABLE_RASPBERRY_COMS

void raspberry_setup() {}
void raspberry_loop() {}

#else

#include "Wire.h"
#include "Helpers.h"


/*************************************************************
  Setup and loop
*************************************************************/

struct RaspberryEventDriver : public EventTask
{
  RaspberryEventDriver() {};

  using EventTask::execute;

  void execute(Event *evt)
  {
    EventID event_message = *(EventID*)evt->extra;
    if (event_message == RASPBERRY_SHUTDOWN)
    {
      send_raspberry_shutdown();
    }
    else if (event_message == MOTOR_READY)
    {
      send_motor_ready();
    }
  }
};

void raspberry_setup() {
  struct RaspberryEventDriver raspberry_event_listner = RaspberryEventDriver();
  evtManager.subscribe(Subscriber(RASBERRY_MESSAGE, &raspberry_event_listner));
}

void raspberry_loop() {}


#endif // DISABLE_RASPBERRY_COMS
