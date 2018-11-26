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

int active_extruder = -1;
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

bool switch_extruder_withSensor(int new_extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(new_extruder);

  isPrinting = true;
  bool _return = false;
  if (!isHomed) { home(); }

  if (active_extruder == 5)
    {
      moveMotor(selectorMotor, -700);
      active_extruder = 4;
    }


  toolChanges++;

  led_on(active_extruder,RED_LED);

  previous_extruder = active_extruder;
  active_extruder = new_extruder;

  if (previous_extruder == active_extruder)
    {
      if (!isFilamentLoaded)
        {
          led_on(active_extruder,RED_LED);
          load_filament_withSensor(); // just load filament if not loaded
          _return = true;
        }
      else
        {
          _return = false;  // nothing really happened
        }
    }
  else
    {
      if (isFilamentLoaded) { unload_filament_withSensor(); } // unload filament first
      set_positions(previous_extruder, active_extruder); // move idler and selector to new filament position

      led_on(active_extruder,RED_LED);
      load_filament_withSensor(); // load new filament
      _return = true;
    }

  all_leds_off();
  led_on(active_extruder,GREEN_LED);
  TRACE_LOG("End, result:");
  TRACE_LOG(_return);
  return _return;
}

//! @brief select extruder
//!
//! Known limitation is, that if extruder 5 - service position was selected before
//! it is not possible to select any other extruder than extruder 4.
//!
//! @param new_extruder Extruder to be selected
//! @return
bool select_extruder(int new_extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(new_extruder);

  bool _return = false;
  if (!isHomed) { home(); }

  led_on(active_extruder,GREEN_LED);

  int previous_extruder = active_extruder;
  active_extruder = new_extruder;

  if (previous_extruder == active_extruder)
    {
      if (!isFilamentLoaded)
        {
          _return = true;
        }
    }
  else
    {
      if (new_extruder == 5)
        {
          moveMotor(selectorMotor, 700);
        }
      else
        {
          if (previous_extruder == 5)
            {
              moveMotor(selectorMotor, -700);
            }
          else
            {
              if (isIdlerParked) park_idler(true);
              set_positions(previous_extruder, active_extruder); // move idler and selector to new filament position
              park_idler(false);
            }
        }
      _return = true;
    }

  all_leds_off();
  led_on(active_extruder,RED_LED);
  TRACE_LOG("End, result:");
  TRACE_LOG(_return);
  return _return;
}

bool service_position()
{
  TRACE_LOG("Start");

  moveMotor(selectorMotor, 600);

  TRACE_LOG("End, result:");
  TRACE_LOG("true");
  return true;
}
