//mmctl.cpp - multimaterial switcher control
#include "main.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "mmctl.h"
#include "motion.h"
#include "Buttons.h"

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

	set_pulley_dir_push();
	
	do
	{
		do_pulley_step();

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
		set_pulley_dir_pull();
		for (int i = 600; i > 0; i--)   // 570
		{
			do_pulley_step();
			delayMicroseconds(3000);
		}
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
		move(0, -700, 0);
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
			move(0, 700, 0);
		}
		else
		{
			if (previous_extruder == 5)
			{
				move(0, -700, 0);
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
	move(0, 600, 0);

    TRACE_LOG("End, result:");
    TRACE_LOG("true");
	return true;
}


// LED explanation, you can turn on green or red led
// [RG]  [RG]  [RG]  [RG]  [RG]
//  0     1     2     3     4
//
// IN STOCK FIRMWARE:
// shr16_set_led(2 << 2 * (4 - active_extruder)); -> they are counting from right to left (4-active_extruder) -> led_on(active_extruder,green)
// shr16_set_led(1 << 2 * (4 - active_extruder)); -> they are counting from right to left (4-active_extruder) -> led_on(active_extruder,red)
// shr16_set_led(0x155);                          -> all_leds_on(green);
// shr16_set_led(0x2aa);                          -> all_leds_on(red);
// shr16_set_led(0x000);                          -> all_leds_off();

void led_on(int _no, int color)
{
    TRACE_LOG("Start, parameters:");
    TRACE_LOG(_no);
    TRACE_LOG(color);

    if(_no >= 0 && _no < 5 )
    {
        // TODO: implement led on function
    }
    else
    {
        LOG("ERROR: Wrong LED number:");
        LOG(_no);
    }

    TRACE_LOG("End");
}

void led_off(int _no)
{
    TRACE_LOG("Start, parameters:");
    TRACE_LOG(_no);

    if(_no >= 0 && _no < 5 )
    {
        // TODO: implement led off function
    }
    else
    {
        LOG("ERROR: Wrong LED number:");
        LOG(_no);
    }

    TRACE_LOG("End");
}

void all_leds_on(int color)
{
    TRACE_LOG("Start, parameters:");
    TRACE_LOG(color);

    // TODO: implement all_leds_off function

    TRACE_LOG("End");
}

void all_leds_off()
{
    TRACE_LOG("Start");

    // TODO: implement all_leds_off function

    TRACE_LOG("End");
}

void led_blink(int _no)
{
    TRACE_LOG("Start, parameters:");
    TRACE_LOG(_no);

    if(_no >= 0 && _no < 5 )
    {
        // TODO: implement led_blink function
    }
    else
    {
        LOG("ERROR: Wrong LED number:");
        LOG(_no);
    }

    TRACE_LOG("End");
}


