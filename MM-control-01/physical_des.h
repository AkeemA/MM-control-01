//physical_des.h - multimaterial body description
#ifndef _PHYSICAL_H
#define _PHYSICAL_H

#include <inttypes.h>
#include "config.h"
#include <Arduino.h>

unsigned long int getPulley_steps_for_mm();
unsigned long int getSelector_steps_for_mm();
unsigned long int getIdler_steps_for_position();
unsigned long int getLength_spool_finda();
unsigned long int getLength_finda_extSens();
unsigned long int getLength_extSens_BondTech();
unsigned long int getLength_BondTech_extruder();

int getSelector_pos_dis();
int getSelector_dis_from_rH();
int getSelector_dis_from_lH();

#endif //_PHYSICAL_H
