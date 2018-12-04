/*************************************************************
  EventID
*************************************************************/
#pragma once

#include <Flash.h>

struct EventID {

  protected:
    unsigned char id[3];

#if ENABLE_DEBUG_STRINGS == 0
    _FLASH_STRING *debug_name = NULL;
#endif

  public:
    EventID();
    EventID(const long _event_id);
    EventID(const int _event_id);

#if ENABLE_DEBUG_STRINGS == 1
    EventID(const long _event_id, _FLASH_STRING &debug_name);
    EventID(const int _event_id, _FLASH_STRING &debug_name);  
#endif  
  
    bool operator> (const EventID  &f) const;
    bool operator>=(const EventID  &f) const;
    bool operator==(const EventID  &f) const;
    bool operator<=(const EventID  &f) const;
    bool operator< (const EventID  &f) const;
    EventID operator| (const EventID  &f) const;
    EventID operator& (const EventID  &f) const;

    bool operator> (const long   &i) const;
    bool operator>=(const long   &i) const;
    bool operator==(const long   &i) const;
    bool operator<=(const long   &i) const;
    bool operator< (const long   &i) const;
    long operator| (const long   &i) const;
    long operator& (const long   &i) const;

    operator long () const;

};
typedef EventID EventID;
