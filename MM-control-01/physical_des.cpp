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
