/*************************************************************
  Debug functions that can be compiled out on release
*************************************************************/
#pragma once

#define DEBUG 1
// #define ENABLE_MOCK_WIRE_LIBRARY
// #define ENABLE_MOCK_SX1509_LIBRARY

//#define DISABLE_ENCODER_ABSOLUTE
//#define DISABLE_ENCODER_RELATIVE
//#define DISABLE_BUTTON
//#define DISABLE_BUTTON_LED
//#define DISABLE_CALIBRATION
//#define DISABLE_LIGHTING
//#define DISABLE_MOTORS
//#define DISABLE_RASPBERRY_COMS
//#define DISABLE_RASPBERRY_MANAGER

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

#define DEBUG_PRINT_VAR1(a) \
  do { \
    if (DEBUG){ \
      Serial.println(F(#a ":\t" STRINGIZE(a))); \
      Serial.flush(); \
    }\
  } while (0)
#define DEBUG_PRINT_VAR2(a, b) \
  do { \
    if (DEBUG){ \
      Serial.println(F(#a ":\t" STRINGIZE(a) "\t" #b ":\t" STRINGIZE(b))); \
      Serial.flush(); \
    }\
  } while (0)
#define DEBUG_PRINT_VAR3(a, b, c) \
  do { \
    if (DEBUG){ \
      Serial.println(F(#a ":\t" STRINGIZE(a) "\t" #b ":\t" STRINGIZE(b) "\t" #c ":\t" STRINGIZE(c))); \
      Serial.flush(); \
    }\
  } while (0)

#define GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define FOO(...) GET_MACRO(__VA_ARGS__, DEBUG_PRINT_VAR3, DEBUG_PRINT_VAR2, DEBUG_PRINT_VAR1)(__VA_ARGS__)

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
      Serial.println(F("At " __FILE__ ": " STRINGIZE(__LINE__))); \
      Serial.flush(); \
    }\
  } while (0)
