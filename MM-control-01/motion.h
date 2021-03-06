// motion.h

#ifndef _MOTION_h
#define _MOTION_h

#include "config.h"
#include <inttypes.h>


void cut_filament();
int getIdlerSteps(int current_filament, int next_filament);
void set_positions(int _current_extruder, int _next_extruder);
void eject_filament(int extruder);
void recover_after_eject();

void load_filament_withSensor();
void unload_filament_withSensor();
void load_filament_inPrinter();
void init_Pulley();
void park_idler(bool _unpark);
void home();
bool home_idler(bool toLastFilament);
bool home_selector();
bool checkOk();

int getIdlerDistance(int _current_extruder, int _next_extruder);
int getSelectorDistance(int _current_extruder, int _next_extruder);

extern int8_t filament_type[EXTRUDERS];

#endif

