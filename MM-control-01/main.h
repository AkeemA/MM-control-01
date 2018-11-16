#ifndef _MAIN_H
#define _MAIN_H


#include <inttypes.h>
#include "config.h"
#include <string.h>
#include <Arduino.h>
//GENERAL
#define ENABLE LOW                // stepper motor enable is active low
#define DISABLE HIGH              // stepper motor disable is active high

#define GREEN_LED 1
#define RED_LED 2

void LOG_println(String text);

void manual_extruder_selector();

// system state
extern int8_t sys_state;

// signals from interrupt to main loop
extern uint8_t sys_signals;

// get state of signal (main loop or interrupt)
#define SIG_GET(id) (sys_signals & (1 << id))
// set state of signal (interrupt only)
#define SIG_SET(id) (sys_signals |= (1 << id))
// get state of signal (main loop only)
#define SIG_CLR(id) asm("cli"); sys_signals &= ~(1 << id); asm("sei")

#ifdef DEBUG
#define DBG_PRINT(...) Serial.print(millis());     \
    Serial.print(": ");    \
    Serial.print(__PRETTY_FUNCTION__); \
    Serial.print(": ");    \
    Serial.println(__VA_ARGS__)
#else
#define DBG_PRINT(x)
#endif

#endif //_MAIN_H
