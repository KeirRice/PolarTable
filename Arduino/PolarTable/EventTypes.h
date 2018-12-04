/*************************************************************
  Our event types.
*************************************************************/
#pragma once

#include <Flash.h>
#include <kEvent.h>

unsigned char event_id_counter = 0;
unsigned char system_id_offset = 6;

/*
Macros for setting up all of the global event variables.
 t is the variable type you want (StateID, ActionID, etc)
 a is the name of the variable that will get created as well as the debug string (if enabled);

The debug strings are all stored in PROGMEM.
*/
#if DEBUG == 1 && ENABLE_DEBUG_STRINGS == 1
#define BuildEvent(t, a) \
  FLASH_STRING(prog_##a, #a); \
  const t a(event_id_counter++, prog_##a); 
#define BuildSystem(t, a) \
  FLASH_STRING(prog_##a, #a); \
  const t a(system_id_offset++, prog_##a); 
#else
#define BuildEvent(t, a) \
  const t a(event_id_counter++)
#define BuildSystem(t, a) \
  const t a(system_id_offset++)
#endif // DEBUG == 1


typedef struct SystemID : public EventID {
  SystemID(const long _bit_offset): EventID(1 << _bit_offset) {}
#if ENABLE_DEBUG_STRINGS == 1
  SystemID(const long _bit_offset, _FLASH_STRING &_debug_name): EventID(1 << _bit_offset, _debug_name) {}
#endif
} SystemID;


// Two bits to seperate the actions from the states if we need to.
typedef SystemID EventType;
BuildSystem(EventType, ACTION);
BuildSystem(EventType, STATE);

typedef struct ActionID : public EventID {
  ActionID(const long _event_id): EventID(_event_id | ACTION) {}
#if ENABLE_DEBUG_STRINGS == 1
  ActionID(const long _event_id, _FLASH_STRING &_debug_name): EventID(_event_id | ACTION, _debug_name) {}  
#endif
} ActionID;

typedef struct StateID : public EventID {
  StateID(const long _event_id): EventID(_event_id | STATE) {}
#if ENABLE_DEBUG_STRINGS == 1
  StateID(const long _event_id, _FLASH_STRING &_debug_name): EventID(_event_id | ACTION, _debug_name) {}  
#endif
} StateID;

