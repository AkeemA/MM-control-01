//mmctl.cpp - multimaterial switcher control
#include "main.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "mmctl.h"
#include "motion.h"
#include "display.h"
#include "stepper_driver.h"
#include "Buttons.h"
#include "OLED_display.h"

#ifdef MMCTL_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef MMCTL_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

int active_extruder = 0;
int previous_extruder = -1;
bool isFilamentLoaded = false;
bool isIdlerParked = false;
int toolChanges = 0;

bool isPrinting = false;
bool isHomed = false;

int check_finda() // output: 1 - there is filament; 0 - no filament
{
  TRACE_LOG("Start");
#ifdef UNSTOCK_FINDA
  int reading = analogRead(findaPin);
  reading = reading - 110;
  if(reading<0) reading = reading * -1;

  TRACE_LOG("End, result:");
  if(reading>4)
    {
      TRACE_LOG("true)");
      return 1;
    }
  else
    {
      TRACE_LOG("false)");
      return 0;
    }
#else
  int _return = digitalRead(findaPin);
  TRACE_LOG("End, result:");
  TRACE_LOG(_return);
  return _return;
#endif
}

int check_extruder_sensor() // output: 1 - there is filament; 0 - no filament
{
  TRACE_LOG("Start");
#ifdef UNSTOCK_pinda
  int reading = analogRead(findaPin);
  reading = reading - 110;
  if(reading<0) reading = reading * -1;

  TRACE_LOG("End, result:");
  if(reading>4)
    {
      TRACE_LOG("true)");
      return 1;
    }
  else
    {
      TRACE_LOG("false)");
      return 0;
    }
#else
  int _return = digitalRead(extruderSensorPin);
  TRACE_LOG("End, result:");
  TRACE_LOG(_return);
  return _return;
#endif
}

bool feed_filament()
{
  LOG("Start");
  bool _feed = true;
  bool _loaded = false;

  int _c = 0;
  int _delay = 0;
  park_idler(true);

  set_direction(pulleyMotor, PUSH);

  do
    {
      do_step(pulleyMotor);

      _c++;
      if (_c > 50) { led_on(active_extruder,RED_LED); };
      if (_c > 100) { all_leds_off(); _c = 0; _delay++; };

      if (check_finda() == 1) { _loaded = true; _feed = false; };
      if (buttonClicked() != Btn::none && _delay > 10) { _loaded = false; _feed = false; }
      delayMicroseconds(4000);
    } while (_feed);

  if (_loaded)
    {
      // unload to PTFE tube
      moveMotor(pulleyMotor, 600 * PULL, 3000);
    }

  park_idler(false);
  led_on(active_extruder,GREEN_LED);

  TRACE_LOG("End, result:");
  TRACE_LOG("true");
  return true;
}

//! @brief Change filament
//!
//! Unload filament, if different filament than requested is currently loaded,
//! or homing wasn't done yet.
//! Home if not homed.
//! Switch to requested filament (this does nothing if requested filament is currently selected).
//! Load filament if not loaded.
//! @par new_extruder Filament to be selected
void switch_extruder_withSensor(int new_extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(new_extruder);

  isPrinting = true;

  if (active_extruder == 5)
    {
      moveMotor(selectorMotor, -700);
      active_extruder = 4;
    }

  toolChanges++;

  led_on(active_extruder,RED_LED);

  previous_extruder = active_extruder;
  active_extruder = new_extruder;

  if (isFilamentLoaded && ((previous_extruder != active_extruder) || !isHomed))
    {
      unload_filament_withSensor();
    }

  if (!isHomed) 
  {
    home();
  }

  set_positions(previous_extruder, active_extruder);

  led_on(active_extruder,RED_LED);

  if (!isFilamentLoaded)
    {
      load_filament_withSensor();
    }

  all_leds_off();
  led_on(active_extruder,GREEN_LED);

  TRACE_LOG("End");
}

//! @brief Select filament
//!
//! Does not unload or load filament, just moves selector and idler,
//! caller is responsible for ensuring, that filament is not loaded when caled.
//!
//! @param new_extruder Filament to be selected
void select_extruder(int new_extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(new_extruder);

  if (!isHomed) 
  {
    home();
  }

  led_on(active_extruder,GREEN_LED);

  int previous_extruder = active_extruder;
  active_extruder = new_extruder;

  if (previous_extruder != active_extruder)
    {
      if (new_extruder == 5)
        {
          moveMotor(selectorMotor, 700);
          ++previous_extruder;
        }
      else if (previous_extruder == 5)
        {
          moveMotor(selectorMotor, -700);
          previous_extruder = 4;
        }
      
      if (isIdlerParked) park_idler(true);
      set_positions(previous_extruder, active_extruder); // move idler and selector to new filament position
      park_idler(false);
    }

  all_leds_off();
  led_on(active_extruder,RED_LED);
  TRACE_LOG("End");
}
