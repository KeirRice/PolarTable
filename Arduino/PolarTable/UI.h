/*************************************************************
  Make a serial UI for interactive debug
*************************************************************/
#pragma once

#ifndef ENABLE_SERIAL_UI

void ui_setup(){}
void ui_loop(){}

#else

/* we need the SerialUI lib */
#include <SerialUI.h>

/*************************************************************
  Setup
*************************************************************/

/*
Greating

RASP_LIGHTING_ON = 1;
RASP_LIGHTING_OFF = 2;
RASP_LIGHTING_COLOR = 3;

RASP_THETA = 5;
RASP_RADIUS = 6;

RASP_SLEEP = 21;
RASP_WAKE = 22;

*/

// serial_baud_rate -- connect to device at this baud rate, using druid
#define serial_baud_rate      115200

// serial_maxidle_ms -- how long before we consider the user
// gone, for lack of activity (milliseconds)
#define serial_maxidle_ms     30000

// serial_readtimeout_ms -- timeout when we're expecting input
// for a specific request/read (milliseconds)
#define serial_readtimeout_ms   20000

#define serial_ui_greeting_str  "+++ Basic +++\r\nEnter '?' for options."
// serial_input_terminator -- what we consider an <ENTER> (i.e. newline)
#define serial_input_terminator   '\n'

// if you included requests for "strings",
// request_inputstring_maxlen will set the max length allowable
// (bigger need more RAM)
#define request_inputstring_maxlen  50

#define DIE_HORRIBLY(msg)  for(;;){ MySUI.println(msg); delay(1000); }


/*************************************************************
  Containers
*************************************************************/
typedef struct MyInputsContainerStruct {

  long int Custom;
  // constructor to set sane startup vals
  MyInputsContainerStruct() :
    Custom(0)
  {}
} MyInputsContainerSt;

typedef struct MyTrackedVarsContainerStruct {

  unsigned long ClickCount;
  // constructor to set sane startup vals
  MyTrackedVarsContainerStruct() :
    ClickCount(0)
  {}
} MyTrackedVarsContainerSt;



/*************************************************************
  Variables
*************************************************************/

SUI::SerialUI MySUI;
MyInputsContainerSt MyInputs;
MyTrackedVarsContainerSt MyTracked;

/*************************************************************
  Callbacks
*************************************************************/

/* *** Main *** */
namespace Main {
 
  void doDetails() {
    // output some info
    MySUI.println(F("Super basic example... manage and track a click counter"));
    MySUI.print(F("The counter is currently at:"));
    MySUI.println(MyTracked.ClickCount);
  }
  
  void doIncrementClicks() {
    MyTracked.ClickCount ++;
  }
  
  /* *** Main -> Manage *** */
  namespace Manage {

    void doReset() {
      MyTracked.ClickCount = 0;
    }
    
    bool CustomIsOk(long int& newVal) {
      // we'll limit this to 1000, for no particular reason
      if (newVal > 1000)
      {
        return false;
      }
      return true;
    }
    
    void CustomChanged() {
      // the value has been changed, so let's update the tracker
      // accordingly
      MyTracked.ClickCount = MyInputs.Custom;
    }
  } /* namespace Manage */
} /* namespace Main */



bool SetupSerialUI() {

  MySUI.setGreeting(F(serial_ui_greeting_str));
  // SerialUI acts just like (is actually a facade for)
  // Serial.  Use _it_, rather than Serial, throughout the
  // program.
  // basic setup of SerialUI:
  MySUI.begin(serial_baud_rate); // serial line open/setup
  MySUI.setTimeout(serial_readtimeout_ms);   // timeout for reads (in ms), same as for Serial.
  MySUI.setMaxIdleMs(serial_maxidle_ms);    // timeout for user (in ms)
  MySUI.setReadTerminator(serial_input_terminator);

  // Add variable state tracking
  MySUI.trackState(SUI_STR("ClickCount"), &(MyTracked.ClickCount));

  // a few error messages we hopefully won't need
  SUI_FLASHSTRING CouldntCreateMenuErr = F("Could not create menu?");
  SUI_FLASHSTRING CouldntAddItemErr = F("Could not add item?");

  // get top level menu, to start adding items
  SUI::Menu *topMenu = MySUI.topLevelMenu();
  if (! topMenu ) {
    // well, that can't be good...
    MySUI.returnError(F("Very badness in sEriALui!1"));
    return false;
  }

  /* *** Main *** */
  if ( ! topMenu->addCommand(SUI_STR("details"), Main::doDetails, SUI_STR("Program deets"))) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }

  if ( ! topMenu->addCommand(SUI_STR("Increment Clicks"), Main::doIncrementClicks, SUI_STR("increment the click counter"))) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }
  SUI::Menu * submen1 = topMenu->subMenu(
                          SUI_STR("Manage"),
                          SUI_STR("Manage the counter"), 
        2);
  if (! submen1 ) {
    DIE_HORRIBLY(CouldntCreateMenuErr);
  }

  /* *** Main -> Manage *** */

  if ( ! submen1->addCommand(
         SUI_STR("reset"),
         Main::Manage::doReset,
         SUI_STR("set the counter to 0"))) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }

  if ( ! submen1->addRequest(
         &(MyInputs.Custom),
         SUI_STR("custom"),
         SUI_STR("set the counter to a value"),
         Main::Manage::CustomIsOk,
         Main::Manage::CustomChanged)) {
    MySUI.returnError(CouldntAddItemErr);
    return false;
  }

  return true;
}

void ui_setup() {
  if (!SetupSerialUI()) {
    DIE_HORRIBLY(F("Problem during setup"));
  }
}


void ui_loop() {

  /* We checkForUser() periodically, to see
  ** if anyone is attempting to send us some
  ** data through SerialUI.
  **
  ** This code checks at every pass of the main
  ** loop, meaning a user can interact with the
  ** system at any time.  Should you want to
  ** check for user access only once (say have a
  ** N second wait on startup, and then forgo
  ** allowing SerialUI access), then increase the
  ** delay parameter and use checkForUserOnce(), e.g.
  **
  **    mySUI.checkForUserOnce(15000);
  **
  ** to allow 15 seconds to connect on startup only.
  **
  ** Called without parameters, or with 0, checkForUser
  ** won't delay the program, as it won't block at all.
  ** Using a parameter > 0:
  **  checkForUser(MAX_MS);
  ** will wait for up to MAX_MS milliseconds for a user,
  ** so is equivalent to having a delay(MAX_MS) in the loop,
  ** when no user is present.
  */
  if (MySUI.checkForUser()) {

    /* Now we keep handling the serial user's
    ** requests until they exit or timeout.
    */
    while (MySUI.userPresent()) {
      // actually respond to requests, using
      MySUI.handleRequests();

      // you could add a quick task here, to perform
      // after *every* request, but it's better to use
      // the setUserPresenceHeartbeat-related methods
    }
  } /* end if we had a user on the serial line */
}

#endif //ENABLE_SERIAL_UI
