/*************************************************************
  Helper functions and macros
*************************************************************/
#pragma once

#include <stdarg.h>
#include <avr/sleep.h>

#define UNUSED(x) (void)(x)

template <typename T> inline constexpr
int sign(T x) {
    return (T(0) < x) - (x < T(0));
}

static constexpr long bitmask(const byte onecount)
{
    return static_cast<long>(-(onecount != 0))
        & (static_cast<long>(-1) >> ((sizeof(long) * 8) - onecount));
}

static inline constexpr long offset_bitmask(const byte onecount, const byte offset)
{
  return bitmask(onecount) << offset;
}


void(* resetFunc) (void) = 0; //declare reset function @ address 0

void doReset() {
  resetFunc();  //call reset
}

// Interrupt handler for the wakeup
void wakeUpNow(){}

void sleepNow()
{
    /* Now is the time to set the sleep mode. In the Atmega8 datasheet
     * http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
     * there is a list of sleep modes which explains which clocks and
     * wake up sources are available in which sleep mode.
     *
     * In the avr/sleep.h file, the call names of these sleep modes are to be found:
     *
     * The 5 different modes are:
     *     SLEEP_MODE_IDLE         -the least power savings
     *     SLEEP_MODE_ADC
     *     SLEEP_MODE_PWR_SAVE
     *     SLEEP_MODE_STANDBY
     *     SLEEP_MODE_PWR_DOWN     -the most power savings
     *
     */  
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 
    sleep_enable();          // enables the sleep bit in the mcucr register
                             // so sleep is possible. just a safety pin
 
    /* Now it is time to enable an interrupt. We do it here so an
     * accidentally pushed interrupt button doesn't interrupt
     * our running program. if you want to be able to run
     * interrupt code besides the sleep function, place it in
     * setup() for example.
     *
     * In the function call attachInterrupt(A, B, C)
     * A   can be either 0 or 1 for interrupts on pin 2 or 3.  
     *
     * B   Name of a function you want to execute at interrupt for A.
     *
     * C   Trigger mode of the interrupt pin. can be:
     *             LOW        a low level triggers
     *             CHANGE     a change in level triggers
     *             RISING     a rising edge of a level triggers
     *             FALLING    a falling edge of a level triggers
     *
     * In all but the IDLE sleep modes only LOW can be used.
     */ 
    attachInterrupt(digitalPinToInterrupt(PIN_INTERUPT), wakeUpNow, LOW); 
    sleep_mode();            // here the device is actually put to sleep!!
                             // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
    sleep_disable();         // first thing after waking from sleep:
                             // disable sleep...
    detachInterrupt(1);      // disables interrupt 1 on pin 3 so the
                             // wakeUpNow code will not be executed
                             // during normal running time.
}


template<typename T, typename T2, typename T3>
void bitsWrite(T &x, const T2 &n, const T3 b, const int w)
/* Extention to bitWrite that supports more than one bit getting set at a time.
  x: the numeric variable to which to write
  n: which bit of the number to write, starting at 0 for the least-significant (rightmost) bit
  b: the value to write to the bits
  w: the width of the in bits of b
*/
{
  int mask = ((1 << (w + 1)) - 1) << b; // Build a mask w wide, move it into position b
  x = (x & ~mask) | ((n << b) & mask); // Clear the bits under mask and apply the value
}
