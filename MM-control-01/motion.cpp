#include "motion.h"
#include "stepper_driver.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <Arduino.h>
#include "main.h"
#include "mmctl.h"
#include "display.h"
#include "physical_des.h"
#include "Buttons.h"
#include "permanent_storage.h"

#ifdef MOTION_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef MOTION_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif


const int selector_steps_after_homing = -3700;
const int idler_steps_after_homing = -130;

const int selector_steps = 2790/4;
const int idler_steps = 1420 / 4;    // 2 msteps = 180 / 4
const int idler_parking_steps = (idler_steps / 2) + 40;  // 40

const int bowden_length = 1000;
// endstop to tube  - 30 mm, 550 steps

int selector_steps_for_eject = 0;
int idler_steps_for_eject = 0;

int8_t filament_type[EXTRUDERS] = {-1, -1, -1, -1, -1};

void cut_filament()
{
  TRACE_LOG("Called: cut_filament");
}

//! @brief Compute steps for idler needed to change filament
//! @par _current_extruder Currently selected extruder
//! @par _next_extruder Extruder to be selected
//! @return idler steps
int getIdlerDistance(int _current_extruder, int _next_extruder)
{
    return ((_current_extruder - _next_extruder) * getIdler_steps_for_position());
}

//! @brief Compute steps for selector needed to change filament
//! @par _current_extruder Currently selected extruder
//! @par _next_extruder Extruder to be selected
//! @return selector steps
int getSelectorDistance(int _current_extruder, int _next_extruder)
{
    return (((_current_extruder - _next_extruder) * getSelector_steps_for_mm()) * -1);
}

void set_positions(int _current_extruder, int _next_extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_current_extruder);
  TRACE_LOG(_next_extruder);
  // steps to move to new position of idler and selector
  int _selector_steps = getSelectorDistance(_current_extruder, _next_extruder);
  int _idler_steps = getIdlerDistance(_current_extruder, _next_extruder);

  // move both to new position
  move_proportional(_idler_steps, _selector_steps);
  TRACE_LOG("End");
}

void eject_filament(int extruder)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(extruder);
  //move selector sideways and push filament forward little bit, so user can catch it, unpark idler at the end to user can pull filament out
  int selector_position = 0;

  int8_t selector_offset_for_eject = 0;
  int8_t idler_offset_for_eject = 0;

  //if there is still filament detected by PINDA unload it first
  if (isFilamentLoaded)  unload_filament_withSensor();

  select_extruder(active_extruder); //Enforce home idler and selector.

  if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament


  //if we are want to eject fil 0-2, move seelctor to position 4 (right), if we want to eject filament 3 - 4, move selector to position 0 (left)
  //maybe we can also move selector to service position in the future?
  if (extruder <= 2) selector_position = 4;
  else selector_position = 0;

  //count offset (number of positions) for desired selector and idler position for ejecting
  selector_offset_for_eject = active_extruder - selector_position;
  idler_offset_for_eject = active_extruder - extruder;

  //count number of desired steps for selector and idler and store it in static variable
  selector_steps_for_eject = (selector_offset_for_eject * selector_steps) * -1;
  idler_steps_for_eject = idler_offset_for_eject * idler_steps;

  //move selector and idler to new position
  move_proportional(idler_steps_for_eject, selector_steps_for_eject);

  //push filament forward
  moveMotor(pulleyMotor, 2500, 1500);

  //unpark idler so user can easily remove filament
  park_idler(false);
  TRACE_LOG("End");
}

void recover_after_eject()
{
  TRACE_LOG("Start");
  move_proportional(-idler_steps_for_eject, -selector_steps_for_eject);
  TRACE_LOG("End");
}

void load_filament_withSensor()
{
  TRACE_LOG("Start");

  FilamentLoaded::set(active_extruder);
  if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament


  // load filament until FINDA senses end of the filament, means correctly loaded into the selector
  // we can expect something like 570 steps to get in sensor
  unsigned long int _loadSteps = getLength_spool_finda(); // 1500 - default from PRUSA
  moveMotorWithSensor(pulleyMotor, _loadSteps, FINDA_SENSOR, 5500);

  // filament did not arrived at FINDA, let's try to correct that
  if (check_finda() == 0)
    {
      for (int i = 6; i > 0; i--)
        {
          if (check_finda() == 0)
            {
              // attempt to correct
              moveMotor(pulleyMotor, -200, 1500);
              moveMotorWithSensor(pulleyMotor, 500, FINDA_SENSOR, 4000);
            }
        }
    }

  // still not at FINDA, error on loading, let's wait for user input
  if (check_finda() == 0)
    {
      bool _continue = false;
      bool _isOk = false;



      park_idler(false);
      do
        {
          all_leds_off();
          delay(800);
          if (!_isOk)
            {
              led_on(active_extruder,GREEN_LED);
            }
          else
            {
              led_on(active_extruder,RED_LED);
              delay(100);
              led_on(active_extruder,GREEN_LED);
              delay(100);
            }
          delay(800);

          switch (buttonClicked())
            {
            case Btn::left:
              // just move filament little bit
              park_idler(true);
              moveMotor(pulleyMotor, 200, 5500);
              park_idler(false);
              break;
            case Btn::middle:
              // check if everything is ok
              park_idler(true);
              _isOk = checkOk();
              park_idler(false);
              break;
            case Btn::right:
              // continue with loading
              park_idler(true);
              _isOk = checkOk();
              park_idler(false);

              if (_isOk) //pridat do podminky flag ze od tiskarny prislo continue
                {
                  _continue = true;
                }
              break;
            default:
              break;
            }

        } while ( !_continue );

      park_idler(true);
      // TODO: do not repeat same code, try to do it until succesfull load
      _loadSteps = getLength_spool_finda();
      moveMotorWithSensor(pulleyMotor, _loadSteps, FINDA_SENSOR, 5500);
    }
  else
    {
      // nothing
    }

  float _speed = 4500;
  _loadSteps = getLength_finda_extSens(); // lengt between 2 sensors
  int i = 0;
  do
    {
      do_step(pulleyMotor);
      i++;
      if (i > round(_loadSteps*0.001) && i < round(_loadSteps*0.45) && _speed > 650) _speed = _speed - 4;
      if (i > round(_loadSteps*0.01) && i < round(_loadSteps*0.45) && _speed > 650) _speed = _speed - 1;
      if (i > round(_loadSteps*0.9) && _speed < 3000) _speed = _speed + 2;
      delayMicroseconds(_speed);
    } while (check_extruder_sensor() == 0 && i < _loadSteps);

  _loadSteps = getLength_extSens_BondTech();
  moveMotor(pulleyMotor, _loadSteps, 3000);

  isFilamentLoaded = true;  // filament loaded
  TRACE_LOG("End");
}

void unload_filament_withSensor()
{
  TRACE_LOG("Start");
  // unloads filament from extruder - filament is above Bondtech gears

  if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament

  float _speed = 2000;
  int _endstop_hit = 0;

  // I stage: unload from hotend to filament sensor on top of the mk3 extruder
  // Speed of unload is 2000 microseconds/step = 2 miliseconds/step try for 2s, if after that sensor will still sense filament we will raise the error
  int _unloadSteps = getLength_extSens_BondTech() + 10 * getPulley_steps_for_mm() * -1; // move for BONDTECH gear --- extruder sensor distance + additional 10 mm

  moveMotorWithSensor(pulleyMotor, _unloadSteps, EXTRUDER_SENSOR, _speed);

  if(check_extruder_sensor() == 0) // Check if unloading from hotend to extruder filamend sensor was successful, if yes extrude to FINDA sensor on MMU2
    {

      // II stage: filament reached extruder sensor -> unload until FINDA senses end of the filament
      _unloadSteps = getLength_finda_extSens() + 20 * getPulley_steps_for_mm(); // move for BONDTECH gear --- extruder sensor distance + additional 20 mm
      set_direction(pulleyMotor, PULL);
      unsigned long i = _unloadSteps;
      do
        {
          do_step(pulleyMotor);
          i--;

          if (i < round(_unloadSteps*0.15) && _speed < 6000) _speed = _speed + 3;
          if (i < round(_unloadSteps*0.20) && _speed < 2500) _speed = _speed + 2;
          if (i < round(_unloadSteps*0.90) && i > round(_unloadSteps*0.50) && _speed > 550) _speed = _speed - 2;

          delayMicroseconds(_speed);
          if (check_finda() == 0) _endstop_hit++;

        } while (_endstop_hit < 100 && i > 0);

      // move a little bit so it is not a grinded hole in filament
      moveMotor(pulleyMotor, 100, 5000);

      // FINDA is still sensing filament, let's try to unload it once again // TODO change it for steps_for_mm
      if (check_finda() == 1)
        {
          int i = 0;
          do
          {
              moveMotor(pulleyMotor, 150, 4000);
              moveMotorWithSensor(pulleyMotor, -4000, FINDA_SENSOR, 3000);
              i++;
              delay(100);
          } while( check_finda() == 1 && i < 6);
        }
    }


  // error, wait for user input
  if (check_finda() == 1 || check_extruder_sensor() == 1)
    {
      bool _continue = false;
      bool _isOk = false;

      park_idler(false);
      do
        {
          all_leds_off();
          delay(100);
          if (!_isOk)
            {
              led_on(active_extruder,GREEN_LED);
            }
          else
            {
              led_on(active_extruder,RED_LED);
              delay(100);
              led_on(active_extruder,GREEN_LED);
              delay(100);
            }
          delay(100);

          switch (buttonClicked())
            {
            case Btn::left:
              // just move filament little bit
              park_idler(true);
              moveMotor(pulleyMotor, -200, 5500); // TODO: change to the steps per mm and check speed
              park_idler(false);
              break;
            case Btn::middle:
              // check if everything is ok
              park_idler(true);
              _isOk = checkOk();
              park_idler(false);
              break;
            case Btn::right:
              // continue with unloading
              park_idler(true);
              _isOk = checkOk();
              park_idler(false);

              if (_isOk)
                {
                  _continue = true;
                }
              break;
            default:
              break;
            }


        } while (!_continue);

      led_on(previous_extruder,RED_LED);
      park_idler(true);
    }
  else
    {
      // correct unloading
      _speed = 5000;
      // unload to PTFE tube
      moveMotor(pulleyMotor, -450, _speed); // TODO: change to the steps per mm and check speed
    }
  park_idler(false);
  isFilamentLoaded = false; // filament unloaded
  TRACE_LOG("End");
}

//! @brief Do 320 pulley steps slower and 450 steps faster with decreasing motor current.
//!
//! @n d = 6.3 mm        pulley diameter
//! @n c = pi * d        pulley circumference
//! @n FSPR = 200        full steps per revolution (stepper motor constant) (1.8 deg/step)
//! @n mres = 2          microstep resolution (uint8_t __res(AX_PUL))
//! @n SPR = FSPR * mres steps per revolution
//! @n T1 = 2600 us      step period first segment
//! @n T2 = 2200 us      step period second segment
//! @n v1 = (1 / T1) / SPR * c = 19.02 mm/s  speed first segment
//! @n s1 =   320    / SPR * c = 15.80 mm    distance first segment
//! @n v2 = (1 / T2) / SPR * c = 22.48 mm/s  speed second segment
//! @n s2 =   450    / SPR * c = 22.26 mm    distance second segment
void load_filament_inPrinter() // TODO: think about changing it also to configurable value to be independedn from motor steps/rev
{
  TRACE_LOG("Start");
  // loads filament after confirmed by printer into the Bontech pulley gears so they can grab them

  if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament
  //PLA
  moveMotor(pulleyMotor, getLength_BondTech_extruder(), 2600);

  //PLA
  moveMotor(pulleyMotor, getLength_BondTech_extruder(), 2200);// TODO: check why it is used twice -> different speed ?

  park_idler(false);
  TRACE_LOG("End");
}

void init_Pulley()
{
  TRACE_LOG("Start");
  int led = -1;

	int _speed = 3000;
  
  set_direction(pulleyMotor, PUSH);
  for (int i = 50; i > 0; i--)
    {
      do_step(pulleyMotor);
      delayMicroseconds(_speed);
      led = 4-(int)(i/10);
      if(led >=0) led_on(4-(int)(i/10),RED_LED);
    }

  all_leds_off();
  set_direction(pulleyMotor, PULL);
  for (int i = 50; i > 0; i--)
    {
      do_step(pulleyMotor);
      delayMicroseconds(_speed);
      led = 4-(int)(i/10);
      if(led >=0) led_on(4-(int)(i/10),RED_LED);
    }

  TRACE_LOG("End");
}

void park_idler(bool _unpark)
{
  TRACE_LOG("Start");

  if (_unpark) // get idler in contact with filament
    {
      moveMotor(idlerMotor,idler_parking_steps);
      isIdlerParked = false;
    }
  else // park idler so filament can move freely
    {
      moveMotor(idlerMotor,-1 * idler_parking_steps);
      isIdlerParked = true;
    }

  TRACE_LOG("End");
}

//! @brief home idler
//!
//! @par toLastFilament
//!   - true
//! Move idler to previously loaded filament and disengage. Returns true.
//! Does nothing if last filament used is not known and returns false.
//!   - false (default)
//! Move idler to filament 0 and disengage. Returns true.
//!
//! @retval true Succeeded
//! @retval false Failed
bool home_idler(bool toLastFilament)
{
  TRACE_LOG("Start");

  uint8_t filament = 0; //Not needed, just to suppress compiler warning.
  if(toLastFilament)
  {
    if(!FilamentLoaded::get(filament)) return false;
  }

  int i = 0;
  int dir = 1;
  while (i < 10) // Count of idler positions + extro for homing TODO: get it from physical description
  {
    if(dir == -1) dir = 1; else dir = -1;
    moveMotor(idlerMotor, 1*getIdler_steps_for_position());
    i += 2;
  }

  moveMotor(idlerMotor, idler_steps_after_homing);

  if (toLastFilament)
    {
      int idlerSteps = getIdlerDistance(0,filament);
      move_proportional(idlerSteps, 0);
    }

	park_idler(false);
  TRACE_LOG("End, result:");
  TRACE_LOG("true");
  return true;
}

bool home_selector()
{
  TRACE_LOG("Start");

  while (check_finda() == 1)
  {
    while (Btn::right != buttonClicked())
    {
      if (check_finda() == 1)
      {
        all_leds_on(RED_LED);
      }
      else
      {
        all_leds_on(GREEN_LED);
      }
      delay(300);
      all_leds_off();
      delay(300);
    }
  }

  int i = 0;
  int dir = 1;
  while (i < 76) // length of selector TODO: get it from physical description
  {
    if(dir == -1) dir = 1; else dir = -1;
    moveMotor(selectorMotor, 1*getSelector_steps_for_mm());
    i += 4;
  }

  moveMotor(selectorMotor, selector_steps_after_homing); // move to initial position

  TRACE_LOG("End, result:");
  TRACE_LOG(true);
  return true;
}

void home()
{
  TRACE_LOG("Start");

  all_leds_off();
  
  // home both idler and selector
  home_idler(false);
  home_selector();

  all_leds_on(GREEN_LED);
  delay(100);
  all_leds_off();

  isFilamentLoaded = false;
  led_on(active_extruder,RED_LED);
  isHomed = true;

  TRACE_LOG("End");
}

bool checkOk()
{
  TRACE_LOG("Start");
  bool _ret = false;
  int _steps = 0;
  int _endstop_hit = 0;


  // filament in FINDA, let's try to unload it
  set_direction(pulleyMotor, PULL);
  if (check_finda() == 1)
    {
      _steps = 3000;
      _endstop_hit = 0;
      do
        {
          do_step(pulleyMotor);
          delayMicroseconds(3000);
          if (check_finda() == 0) _endstop_hit++;
          _steps--;
        } while (_steps > 0 && _endstop_hit < 50);
    }

  if (check_finda() == 0)
    {
      // looks ok, load filament to FINDA
      set_direction(pulleyMotor, PUSH);

      _steps = 3000;
      _endstop_hit = 0;
      do
        {
          do_step(pulleyMotor);
          delayMicroseconds(3000);
          if (check_finda() == 1) _endstop_hit++;
          _steps--;
        } while (_steps > 0 && _endstop_hit < 50);

      if (_steps == 0)
        {
          // we ran out of steps, means something is again wrong, abort
          _ret = false;
        }
      else
        {
          // looks ok !
          // unload to PTFE tube
          moveMotor(pulleyMotor, -600, 3000);
          _ret = true;
        }

    }
  else
    {
      // something is wrong, abort
      _ret = false;
    }

  TRACE_LOG("End, result:");
  TRACE_LOG(_ret);
  return _ret;
}
