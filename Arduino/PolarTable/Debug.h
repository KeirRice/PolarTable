/*************************************************************
  Debug functions that can be compiled out on release
*************************************************************/
#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG 1

#ifndef DEBUG
#define NDEBUG
#endif // DEBUG

#include "Arduino.h"
#include <assert.h>

// #define ENABLE_MOCK_WIRE_LIBRARY
// #define ENABLE_MOCK_SX1509_LIBRARY
// #define ENABLE_TESTING
// #define ENABLE_SERIAL_UI

//#define DISABLE_ENCODER_ABSOLUTE
//#define DISABLE_ENCODER_RELATIVE
// #define DISABLE_BUTTON
// #define DISABLE_BUTTON_LED
//#define DISABLE_CALIBRATION
//#define DISABLE_LIGHTING
//#define DISABLE_MOTORS
//#define DISABLE_RASPBERRY_COMS
//#define DISABLE_RASPBERRY_MANAGER

// Convert a byte to text we can print
void print_byte(byte code)
{
  char binstr[(sizeof(code) * 8) - 1] = {(char) '0'};
  for(unsigned int i = (sizeof(code) * 8); i > 0 ; --i)
  {
    binstr[i-1] = (char) (code & (1 << (i - 1))) ? '1' : '0';
  }
  Serial.print(binstr);
}

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)
/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)

#define DEBUG_PRINT(...) \
  do { \
    if (DEBUG){ \
      Serial.print(F("DEBUG: " __VA_ARGS__)); \
      Serial.flush(); \
    }\
  } while (0)

#define DEBUG_PRINTLN(...) \
  do { \
    if (DEBUG){ \
      Serial.println(F("DEBUG: " __VA_ARGS__)); \
      Serial.flush(); \
    }\
  } while (0)

#define DEBUG_WHERE() \
  do { \
    if (DEBUG){ \
      Serial.print(F("At "));\
      Serial.print(__func__);\
      Serial.println(F(" in " __FILE__ ": " STRINGIZE(__LINE__))); \
      Serial.flush(); \
    }\
  } while (0)

#define DEBUG_PRINT_VAR1(a) \
  do { \
    if (DEBUG){ \
      Serial.print(F(#a ":\t")); \
      Serial.println(a); \
      Serial.flush(); \
    }\
  } while (0)
#define DEBUG_PRINT_VAR2(a, b) \
  do { \
    if (DEBUG){ \
      Serial.print(F(#a ":\t")); \
      Serial.print(a); \
      Serial.print(F("\t" #b ":\t")); \
      Serial.println(b); \
      Serial.flush(); \
    }\
  } while (0)
#define DEBUG_PRINT_VAR3(a, b, c) \
  do { \
    if (DEBUG){ \
      Serial.print(F(#a ":\t")); \
      Serial.print(a); \
      Serial.print(F("\t" #b ":\t")); \
      Serial.print(b); \
      Serial.print(F("\t" #c ":\t")); \
      Serial.println(c); \
      Serial.flush(); \
    }\
  } while (0)

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define DEBUG_PRINT_VAR(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_VAR3, DEBUG_PRINT_VAR2, DEBUG_PRINT_VAR1)(__VA_ARGS__)

#endif // _DEBUG_H
