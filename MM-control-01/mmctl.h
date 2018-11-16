//mmctl.h - multimaterial switcher control
#ifndef _MMCTL_H
#define _MMCTL_H

#include <inttypes.h>
#include "config.h"

extern bool isPrinting;
extern bool isHomed;
extern int toolChanges;

extern int active_extruder;
extern int previous_extruder;
extern bool isFilamentLoaded; // Filament loaded just above BondTech gear in MK3 extruder
extern bool isIdlerParked;

extern int check_finda();
extern int check_extruder_sensor();

extern bool home_idler();
extern bool home_selector();

extern bool switch_extruder_withSensor(int new_extruder);

extern bool select_extruder(int new_extruder);
extern bool service_position();

extern bool feed_filament();

extern void led_blink(int _no);
extern void led_on(int _no, int color);
extern void led_off(int _no);
extern void all_leds_on(int color);
extern void all_leds_off();

#endif //_MMCTL_H
