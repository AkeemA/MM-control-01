//! @file

#include "Buttons.h"
#include "mmctl.h"
#include "motion.h"
#include "permanent_storage.h"
#include "main.h"

#ifdef BTN_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef BTN_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

const int ButtonPin = A2;

void settings_bowden_length();


//!	@brief Select filament for bowden length calibration
//!
//! Filaments are selected by left and right buttons, calibration is activated by middle button.
//! Park position (one behind last filament) can be also selected.
//! Activating calibration in park position exits selector.
//!
void settings_select_filament()
{
  TRACE_LOG("Start");
  while (1)
    {
      manual_extruder_selector();

      if(Btn::middle == buttonClicked())
        {
          led_on(0,GREEN_LED);
          delay(500);
          if (Btn::middle == buttonClicked())
            {
              if (active_extruder < 5) settings_bowden_length();
              else
                {
                  select_extruder(4);
                  select_extruder(0);
                  return;
                }
            }
        }
    }
  TRACE_LOG("End");
}

//!	@brief Show setup menu
//!
//! Items are selected by left and right buttons, activated by middle button.
//!
//! LED indication of states
//!
//! RG | RG | RG | RG | RG | meaning
//! -- | -- | -- | -- | -- | ------------------------
//! 11 | 00 | 00 | 00 | 01 | initial state, no action
//! 11 | 00 | 00 | 01 | 00 | setup bowden length
//! 11 | 00 | 01 | 00 | 00 | erase EEPROM if unlocked
//! 11 | 01 | 00 | 00 | 00 | unlock EEPROM erase
//! 11 | 00 | 00 | 00 | 00 | exit setup menu
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//!
void setupMenu()
{
  TRACE_LOG("Start");
  all_leds_off();
  delay(200);do_idler_step
  all_leds_on(RED_LED);
  delay(1200);
  all_leds_off();
  delay(600);

  int _menu = 4;
  bool _exit = false;
  bool eraseLocked = true;

  do
    {
      led_on(0,RED_LED);
      delay(1);
      led_on(0,GREEN_LED);
      delay(1);
      led_on(_menu,GREEN_LED);
      delay(1);

      switch (buttonClicked())
        {
        case Btn::right:
          if (_menu < 4) { _menu++; delay(800); }
          break;
        case Btn::middle:

          switch (_menu)
            {
            case 3:
              settings_select_filament();
              _exit = true;
              break;
            case 2:
              if (!eraseLocked)
                {
                  BowdenLength::eraseAll();
                  _exit = true;
                }
              break;
            case 1: //unlock erase
              eraseLocked = false;
              break;
            case 0: // exit menu
              _exit = true;
              break;
            }
          break;
        case Btn::left:
          if (_menu > 1) { _menu--; delay(800); }
          break;
        default:
          break;
        }

    } while (!_exit);


  all_leds_off();
  delay(400);
  all_leds_on(GREEN_LED);
  delay(400);
  all_leds_off();
  delay(400);

  all_leds_off();
  led_on(active_extruder,RED_LED);
  TRACE_LOG("End");
}


//!	@brief Show development menu
//!
//! Items are selected by left and right buttons, activated by middle button.
//!
//! LED indication of states
//!
//! RG | RG | RG | RG | RG | meaning
//! -- | -- | -- | -- | -- | ------------------------
//! 11 | 00 | 00 | 00 | 01 | initial state, no action
//! 11 | 00 | 00 | 01 | 00 | home selector
//! 11 | 00 | 01 | 00 | 00 | home idler
//! 11 | 01 | 00 | 00 | 00 | move pulley for 20 mm
//! 11 | 00 | 00 | 00 | 00 | exit setup menu
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//!
// Development menu to test seperated parts of code
void debugMenu()
{
  TRACE_LOG("Start");
  all_leds_off();
  delay(200);
  all_leds_on(RED_LED);
  delay(1200);
  all_leds_off();
  delay(600);

  int _menu = 4;
  bool _exit = false;
  bool eraseLocked = true;

  do
    {
      led_on(0,RED_LED);
      delay(1);
      led_on(0,GREEN_LED);
      delay(1);
      led_on(_menu,GREEN_LED);
      delay(1);

      switch (buttonClicked())
        {
        case Btn::right:
          if (_menu < 4) { led_off(_menu); _menu++; delay(800); }
          break;
        case Btn::middle:

          switch (_menu)
            {
            case 3:
              LOG("homing selector motor");
              home_selector();
              break;
            case 2:
              LOG("Homing idler motor");
              home_idler();
              break;
            case 1:
              LOG("Moving pulley motor for 20 mm");
              move(0,0,20*getPulley_steps_for_mm());
              break;
            case 0: // exit menu
              _exit = true;
              break;
            }
          break;
        case Btn::left:
          if (_menu > 1) { led_off(_menu); _menu--; delay(800); }
          break;
        default:
          break;
        }

    } while (!_exit);


  all_leds_off();
  delay(400);
  all_leds_on(GREEN_LED);
  delay(400);
  all_leds_off();
  delay(400);

  all_leds_off();
  led_on(active_extruder,RED_LED);
  TRACE_LOG("End");
}

//! @brief Set bowden length
//!
//! button | action
//! ------ | ------
//! left   | increase bowden length / feed more filament
//! right  | decrease bowden length / feed less filament
//! middle | store bowden length to EEPROM and exit
//!
//! This state is indicated by following LED pattern:
//!
//! RG | RG | RG | RG | RG
//! -- | -- | -- | -- | --
//! bb | 00 | 00 | 0b | 00
//!
//! @n R - Red LED
//! @n G - Green LED
//! @n 1 - active
//! @n 0 - inactive
//! @n b - blinking
//!
void settings_bowden_length()
{
  TRACE_LOG("Start");
  // load filament above Bondtech gears to check correct length of bowden tube
  if (!isFilamentLoaded)
    {
      BowdenLength bowdenLength;
      load_filament_withSensor();

      do
        {
          switch (buttonClicked())
            {
            case Btn::right:
              if (bowdenLength.decrease())
                {
                  move(0, 0, -bowdenLength.stepSize);
                  delay(400);
                }
              break;

            case Btn::left:
              if (bowdenLength.increase())
                {
                  move(0, 0, bowdenLength.stepSize);
                  delay(400);
                }
              break;
            default:
              break;
            }


          led_on(0,RED_LED);
          delay(10);
          led_on(0,GREEN_LED);
          delay(10);
          led_on(3,GREEN_LED);
          delay(50);


        } while (buttonClicked() != Btn::middle);

      unload_filament_withSensor();
    }
  TRACE_LOG("End");
}

//! @brief Is button pushed?
//!
//! @return button pushed
Btn buttonClicked()
{
  TRACE_LOG("Called, result:");

#ifdef SERIAL_BUTTONS
  int serialButton = -0;
  if(Serial.available() > 0)
    {
      serialButton = Serial.read();
    }

  if (serialButton == 'r')
    {
      LOG("right");
      return Btn::right;
    }
  if (serialButton == 'm')
    {
      LOG("middle");
      return Btn::middle;
    }
  if (serialButton == 'l')
    {
      LOG("left");
      return Btn::left;
    }
#else
  int raw = analogRead(ButtonPin);

  if (raw < 50)
    {
      LOG("right");
      return Btn::right;
    }
  if ((raw > 80 && raw < 100))
    {
      LOG("middle");
      return Btn::middle;
    }
  if ((raw > 160 && raw < 180))
    {
      LOG("left");
      return Btn::left;
    }
  */
#endif
  LOG("none");
  return Btn::none;
}

