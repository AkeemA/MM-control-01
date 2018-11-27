//mmctl.h - multimaterial switcher control
#ifndef _MMCTL_H
#define _MMCTL_H

#include <inttypes.h>
#include "config.h"
#include <Arduino.h>

extern bool isPrinting;
extern bool isHomed;
extern int toolChanges;

extern int active_extruder;
extern int previous_extruder;
extern bool isFilamentLoaded; // Filament loaded just above BondTech gear in MK3 extruder
extern bool isIdlerParked;

int check_finda();
int check_extruder_sensor();

void switch_extruder_withSensor(int new_extruder);
void select_extruder(int new_extruder);
bool feed_filament();

#endif //_MMCTL_H
