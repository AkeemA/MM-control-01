//physical_des.cpp - multimaterial body description
#include "main.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "mmctl.h"
#include "motion.h"
#include "Buttons.h"
#include "OLED_display.h"

#ifdef PHYDES_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef PHYDES_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

unsigned long int length_spool_finda = 1500;
unsigned long int length_finda_extSens = 10000;
unsigned long int length_extSens_BondTech = 1000;
unsigned long int length_BondTech_extruder = 500;

// --------------------------------------------------
// |LH   O       O       O       O       O        RH|
// |  -1 |   14  |   14  |   14  |   14  |    18    |
// |  mm |   mm  |   mm  |   mm  |   mm  |    mm    |
// --------------------------------------------------
int selector_pos_distance = 14; // distance from every selector position (from 1 to 2, 2 to 3...) in mm
int selector_1_from_rHome = 18; // distance from home position on right side to 1 position in mm
int selector_5_from_lHome = -1; // distance from home position on left side to 5 position in mm

unsigned long int getPulley_steps_for_mm()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(PULLEY_STEPS_MM);
  return PULLEY_STEPS_MM;
}

unsigned long int getSelector_steps_for_mm()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(SELECTOR_STEPS_MM);
  return SELECTOR_STEPS_MM;
}

unsigned long int getIdler_steps_for_position()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(IDLER_STEPS_TO_CHANGE);
  return IDLER_STEPS_TO_CHANGE;
}

unsigned long int getLength_spool_finda()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(length_spool_finda);
  return length_spool_finda;
}

unsigned long int getLength_finda_extSens()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(length_finda_extSens);
  return length_finda_extSens;
}

unsigned long int getLength_extSens_BondTech()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(length_extSens_BondTech);
  return length_extSens_BondTech;
}

unsigned long int getLength_BondTech_extruder()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(length_BondTech_extruder);
  return length_BondTech_extruder;
}

int getSelector_pos_dis()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(selector_pos_distance);
  return selector_pos_distance;
}


int getSelector_dis_from_rH()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(selector_1_from_rHome);
  return selector_1_from_rHome;
}

int getSelector_dis_from_lH()
{
  TRACE_LOG("Called, result:");
  TRACE_LOG(selector_5_from_lHome);
  return selector_5_from_lHome;
}

