#include "motion.h"
#include "shr16.h"
#include "stepper_driver.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <Arduino.h>
#include "main.h"
#include "mmctl.h"
#include "Buttons.h"
#include "permanent_storage.h"

const int selector_steps_after_homing = -3700;
const int idler_steps_after_homing = -130;

const int selector_steps = 2790/4;
const int idler_steps = 1420 / 4;    // 2 msteps = 180 / 4
const int idler_parking_steps = (idler_steps / 2) + 40;  // 40
#define STEPS_FOR_MM 500 // TODO: CHECK THE REAL VALUE / CONFIGURE

const int bowden_length = 1000;
// endstop to tube  - 30 mm, 550 steps

int selector_steps_for_eject = 0;
int idler_steps_for_eject = 0;

int8_t filament_type[EXTRUDERS] = {-1, -1, -1, -1, -1};

int set_idler_direction(int _steps);
int set_selector_direction(int _steps);
int set_pulley_direction(int _steps);

void cut_filament();

void park_idler(bool _unpark);

void load_filament_inPrinter();
void load_filament_withSensor();

void do_pulley_step();
void do_idler_step();

void set_positions(int _current_extruder, int _next_extruder);

bool checkOk();


unsigned long int length_spool_finda = 1500;
unsigned long int length_finda_extSens = 10000;
unsigned long int length_extSens_BondTech = 1000;
unsigned long int length_BondTech_extruder = 500;

// Preparation for getting info from menu/calibration/user input/storage
unsigned long int getSteps_for_mm()
{
    return STEPS_FOR_MM;
}

// Preparation for getting info from menu/calibration/user input/storage
unsigned long int getLength_spool_finda()
{
    return length_spool_finda;
}

// Preparation for getting info from menu/calibration/user input/storage
unsigned long int getLength_finda_extSens()
{
    return length_finda_extSens;
}

// Preparation for getting info from menu/calibration/user input/storage
unsigned long int getLength_extSens_BondTech()
{
    return length_extSens_BondTech;
}

// Preparation for getting info from menu/calibration/user input/storage
unsigned long int getLength_BondTech_extruder()
{
    return length_BondTech_hotend;
}

void cut_filament()
{
}

void set_positions(int _current_extruder, int _next_extruder)
{
	// steps to move to new position of idler and selector
	int _selector_steps = ((_current_extruder - _next_extruder) * selector_steps) * -1;
	int _idler_steps = (_current_extruder - _next_extruder) * idler_steps;

	// move both to new position
	move_proportional(_idler_steps, _selector_steps);
}

void eject_filament(int extruder)
{
	//move selector sideways and push filament forward little bit, so user can catch it, unpark idler at the end to user can pull filament out
	int selector_position = 0;
	int steps = 0;


	int8_t selector_offset_for_eject = 0;
	int8_t idler_offset_for_eject = 0;
		
	//if there is still filament detected by PINDA unload it first
	if (isFilamentLoaded)  unload_filament_withSensor();

        //??driver_enable_motor(idlerEnablePin);
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
        //??driver_enable_motor(selectorEnablePin);
	move_proportional(idler_steps_for_eject, selector_steps_for_eject);

	//push filament forward
	do
	{
		do_pulley_step();
		steps++;
		delayMicroseconds(1500);
	} while (steps < 2500);

	//unpark idler so user can easily remove filament
	park_idler(false);
        //??driver_disable_all();
}

void recover_after_eject()
{
    //restore state before eject filament
    //??driver_enable_motor(idlerEnablePin);
    //??driver_enable_motor(selectorEnablePin);
    move_proportional(-idler_steps_for_eject, -selector_steps_for_eject);
    //??driver_disable_all();
}

void load_filament_withSensor()
{
    //??driver_enable_motor(idlerEnablePin);
	if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament

    //??driver_enable_motor(pulleyEnablePin);
	set_pulley_dir_push();

	int _endstop_hit = 0;

	// load filament until FINDA senses end of the filament, means correctly loaded into the selector
	// we can expect something like 570 steps to get in sensor
    unsigned long int _loadSteps = getLength_spool_finda(); // 1500 - default from PRUSA
	do
	{
		do_pulley_step();
        _loadSteps--;
		delayMicroseconds(5500);
    } while (check_finda() == 0 && _loadSteps > 0);


	// filament did not arrived at FINDA, let's try to correct that
    if (check_finda() == 0)
	{
		for (int i = 6; i > 0; i--)
		{
            if (check_finda() == 0)
			{
				// attempt to correct
				set_pulley_dir_pull();
				for (int i = 200; i >= 0; i--)
				{
					do_pulley_step();
					delayMicroseconds(1500);
				}

				set_pulley_dir_push();
                _loadSteps = 500;
				do
				{
					do_pulley_step();
                    _loadSteps--;
					delayMicroseconds(4000);
                    if (check_finda() == 1) _endstop_hit++;
                } while (_endstop_hit<100 && _loadSteps > 0);
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
			shr16_set_led(0x000);
			delay(800);
			if (!_isOk)
			{
				shr16_set_led(2 << 2 * (4 - active_extruder));
			}
			else
			{
				shr16_set_led(1 << 2 * (4 - active_extruder));
				delay(100);
				shr16_set_led(2 << 2 * (4 - active_extruder));
				delay(100);
			}
			delay(800);


// WK TODO wrap button functions
            switch (buttonClicked())
			{
				case Btn::left:
					// just move filament little bit
					park_idler(true);
					set_pulley_dir_push();

					for (int i = 0; i < 200; i++)
					{
						do_pulley_step();
						delayMicroseconds(5500);
					}
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
		do
		{
			do_pulley_step();
            _loadSteps--;
			delayMicroseconds(5500);
        } while (check_finda() == 0 && _loadSteps > 0);
		// ?
	}
	else
	{
		// nothing
	}

	{

        float _speed = 4500;
        _loadSteps = getLength_finda_extSens(); // lengt between 2 sensors
        int i = 0;
        do
        {
            do_pulley_step();
            i++;
            if (i > round(_loadSteps*0.001) && i < round(_loadSteps*0.45) && _speed > 650) _speed = _speed - 4;
            if (i > round(_loadSteps*0.01) && i < round(_loadSteps*0.45) && _speed > 650) _speed = _speed - 1;
            if (i > round(_loadSteps*0.9) && _speed < 3000) _speed = _speed + 2;
            delayMicroseconds(_speed);
        } while (check_extruder_sensor() == 0 && i < _loadSteps);


        _speed = 3000;

        _loadSteps = getLength_extSens_BondTech();
        for (uint16_t i = 0; i < _loadSteps; i++)
		{
			do_pulley_step();
			delayMicroseconds(_speed);
		}
	}

    	//??driver_disable_all();
	isFilamentLoaded = true;  // filament loaded 
}

void unload_filament_withSensor()
{
    // unloads filament from extruder - filament is above Bondtech gears
    //??driver_enable_motor(pulleyEnablePin);
    //??driver_enable_motor(idlerEnablePin);

	if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament

	set_pulley_dir_pull();

	float _speed = 2000;
	float _first_point = 1800;
	float _second_point = 8700;   
	int _endstop_hit = 0;

// I stage: unload from hotend to filament sensor on top of the mk3 extruder
// Speed of unload is 2000 microseconds/step = 2 miliseconds/step try for 2s, if after that sensor will still sense filament we will raise the error
    int _unloadSteps = getLength_extSens_BondTech() + 10 * getSteps_for_mm(); // move for BONDTECH gear --- extruder sensor distance + additional 10 mm
    while(check_extruder_sensor() == 1 && _unloadSteps > 0)
    {
        do_pulley_step();
        _unloadSteps--;
        delayMicroseconds(_speed);
    }


    if(check_extruder_sensor() == 0) // Check if unloading from hotend to extruder filamend sensor was successful, if yes extrude to FINDA sensor on MMU2
    {

// II stage: filament reached extruder sensor -> unload until FINDA senses end of the filament
        _unloadSteps = getLength_finda_extSens() + 20 * getSteps_for_mm(); // move for BONDTECH gear --- extruder sensor distance + additional 20 mm
        unsigned long i = _unloadSteps;
        do
        {
            do_pulley_step();
            i--;

            if (i < round(_unloadSteps*0.15) && _speed < 6000) _speed = _speed + 3;
            if (i < round(_unloadSteps*0.20) && _speed < 2500) _speed = _speed + 2;
            if (i < round(_unloadSteps*0.90) && i > round(_unloadSteps*0.50) && _speed > 550) _speed = _speed - 2;

            delayMicroseconds(_speed);
            if (check_finda() == 0 && i < round(_unloadSteps*0.25)) _endstop_hit++;

        } while (_endstop_hit < 100 && i > 0);

        // move a little bit so it is not a grinded hole in filament
        for (int i = 100; i > 0; i--) // TODO: change for steps_for_mm
        {
            do_pulley_step();
            delayMicroseconds(5000);
        }

        // FINDA is still sensing filament, let's try to unload it once again // TODO change it for steps_for_mm
        if (check_finda() == 1)
        {
            for (int i = 6; i > 0; i--)
            {
                if (check_finda() == 1)
                {
                    set_pulley_dir_push();
                    for (int i = 150; i > 0; i--)
                    {
                        do_pulley_step();
                        delayMicroseconds(4000);
                    }

                    set_pulley_dir_pull();
                    int _steps = 4000;
                    _endstop_hit = 0;
                    do
                    {
                        do_pulley_step();
                        _steps--;
                        delayMicroseconds(3000);
                        if (check_finda() == 0) _endstop_hit++;
                    } while (_endstop_hit < 100 && _steps > 0);
                }
                delay(100);
            }

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
			shr16_set_led(0x000);
			delay(100);
			if (!_isOk)
			{
				shr16_set_led(2 << 2 * (4 - active_extruder));
			}
			else
			{
				shr16_set_led(1 << 2 * (4 - active_extruder));
				delay(100);
				shr16_set_led(2 << 2 * (4 - active_extruder));
				delay(100);
			}
			delay(100);


// WK TODO wrap button functions
			switch (buttonClicked())
			{
			case Btn::left:
				// just move filament little bit
				park_idler(true);
				set_pulley_dir_pull();

				for (int i = 0; i < 200; i++)
				{
					do_pulley_step();
					delayMicroseconds(5500);
				}
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
		
		shr16_set_led(1 << 2 * (4 - previous_extruder));
		park_idler(true);
	}
	else
	{
		// correct unloading
		_speed = 5000;
		// unload to PTFE tube
		set_pulley_dir_pull();
        for (int i = 450; i > 0; i--)   // TODO change for steps_for_mm
		{
			do_pulley_step();
			delayMicroseconds(_speed);
		}
	}
	park_idler(false);
    //??driver_disable_all();
	isFilamentLoaded = false; // filament unloaded 
}

void load_filament_inPrinter() // TODO: think about changing it also to configurable value to be independedn from motor steps/rev
{
    // loads filament after confirmed by printer into the Bontech pulley gears so they can grab them

    //??driver_init_motor(idlerEnablePin);
	if (isIdlerParked) park_idler(true); // if idler is in parked position un-park him get in contact with filament
    //??driver_init_motor(pulleyEnablePin);
	set_pulley_dir_push();

    //PLA
    for (int i = 0; i <= getLength_BondTech_extruder(); i++)
	{
		do_pulley_step();
		delayMicroseconds(2600);
	}

	//PLA

    for (int i = 0; i <= getLength_BondTech_extruder(); i++) // TODO: check why it is used twice -> different speed ?
	{
		do_pulley_step();
		delayMicroseconds(2200); 
	}
		
    park_idler(false);
    //??driver_disable_all();
}

void init_Pulley()
{
    float _speed = 3000;
	
	set_pulley_dir_push();
	for (int i = 50; i > 0; i--)
	{
		do_pulley_step();
		delayMicroseconds(_speed);
		shr16_set_led(1 << 2 * (int)(i/50));
	}

	set_pulley_dir_pull();
	for (int i = 50; i > 0; i--)
	{
		do_pulley_step();
		delayMicroseconds(_speed);
		shr16_set_led(1 << 2 * (4-(int)(i / 50)));
	}

}

void do_pulley_step()
{
    digitalWrite(pulleyStepPin, HIGH);
    asm("nop");
    digitalWrite(pulleyStepPin, LOW);
	asm("nop");
}

void do_idler_step()
{
    digitalWrite(idlerStepPin, HIGH);
	asm("nop");
    digitalWrite(idlerStepPin, LOW);
	asm("nop");
}

void park_idler(bool _unpark)
{

	if (_unpark) // get idler in contact with filament
	{
		move(idler_parking_steps, 0,0);
		isIdlerParked = false;
	} 
	else // park idler so filament can move freely
	{
		move(idler_parking_steps*-1, 0,0);
		isIdlerParked = true;
	}
	 
}

bool home_idler()
{
	int _c = 0;
	int _l = 0;

	for (int c = 1; c > 0; c--)  // not really functional, let's do it rather more times to be sure
	{
		move(0, (c * 5) * -1,0);
		delay(50);
		for (int i = 0; i < 2000; i++)
		{
			move(1, 0,0);
            delayMicroseconds(100);

			_c++;
			if (i == 1000) { _l++; }
			if (_c > 100) { shr16_set_led(1 << 2 * _l); };
			if (_c > 200) { shr16_set_led(0x000); _c = 0; };
		}
	}
	return true;
}

bool home_selector()
{
	 
	int _c = 0;
	int _l = 2;

	for (int c = 5; c > 0; c--)   // not really functional, let's do it rather more times to be sure
	{
		move(0, (c*20) * -1,0);
		delay(50);
		for (int i = 0; i < 4000; i++)
		{
			move(0, 1,0);
			//uint16_t sg = tmc2130_read_sg(1);
			//if ((i > 16) && (sg < 6))	break;

			_c++;
			if (i == 3000) { _l++; }
			if (_c > 100) { shr16_set_led(1 << 2 * _l); };
			if (_c > 200) { shr16_set_led(0x000); _c = 0; };
		}
	}
	
	return true;
}

void home()
{
	move(-10, -100,0); // move a bit in opposite direction
	
	// home both idler and selector
	home_idler();
    home_selector();
	
	shr16_set_led(0x155);
	move(idler_steps_after_homing, selector_steps_after_homing,0); // move to initial position

	active_extruder = 0;

	park_idler(false);
	shr16_set_led(0x000);
	
	isFilamentLoaded = false; 
	shr16_set_led(1 << 2 * (4-active_extruder));

  isHomed = true;
}
 

void move_proportional(int _idler, int _selector)
{
	// gets steps to be done and set direction
	_idler = set_idler_direction(_idler);
	_selector = set_selector_direction(_selector);

	float _idler_step = (float)_idler/(float)_selector;
	float _idler_pos = 0;
	int _speed = 2500;
	int _start = _selector - 250;
	int _end = 250;

	do
	{
		if (_idler_pos >= 1)
		{
            if (_idler > 0) { digitalWrite(idlerStepPin, HIGH); } // D12 - step on one driver
		}
        if (_selector > 0) { digitalWrite(selectorStepPin, HIGH); }  // D4 - step on second driver
		
		asm("nop");
		
		if (_idler_pos >= 1)
		{
            if (_idler > 0) { digitalWrite(idlerStepPin, LOW); _idler--;  }
		}

        if (_selector > 0) { digitalWrite(selectorStepPin, LOW); _selector--; }
		asm("nop");

		if (_idler_pos >= 1)
		{
			_idler_pos = _idler_pos - 1;
		}


		_idler_pos = _idler_pos + _idler_step;

		delayMicroseconds(_speed);
		if (_speed > 900 && _selector > _start) { _speed = _speed - 10; }
		if (_speed < 2500 && _selector < _end) { _speed = _speed + 10; }

	} while (_selector != 0 || _idler != 0 );
}

void move(int _idler, int _selector, int _pulley)
{
	int _acc = 50;

	// gets steps to be done and set direction
	_idler = set_idler_direction(_idler); 
	_selector = set_selector_direction(_selector);
	_pulley = set_pulley_direction(_pulley);
	

	do
	{
        if (_idler > 0) { digitalWrite(idlerStepPin, HIGH); }
        if (_selector > 0) { digitalWrite(selectorStepPin, HIGH); }
        if (_pulley > 0) { digitalWrite(pulleyStepPin, HIGH); }
		asm("nop");
        if (_idler > 0) { digitalWrite(idlerStepPin, LOW); _idler--; delayMicroseconds(1000); }
        if (_selector > 0) { digitalWrite(selectorStepPin, LOW); _selector--;  delayMicroseconds(800); }
        if (_pulley > 0) { digitalWrite(pulleyStepPin, LOW); _pulley--;  delayMicroseconds(700); }
		asm("nop");

		if (_acc > 0) { delayMicroseconds(_acc*10); _acc = _acc - 1; }; // super pseudo acceleration control

	} while (_selector != 0 || _idler != 0 || _pulley != 0);
}


void set_idler_dir_down()
{
    digitalWrite(idlerDirPin, LOW);
}
void set_idler_dir_up()
{
    digitalWrite(idlerDirPin, HIGH);
}


int set_idler_direction(int _steps)
{
	if (_steps < 0)
	{
		_steps = _steps * -1;
		set_idler_dir_down();
	}
	else 
	{
		set_idler_dir_up();
	}
	return _steps;
}
int set_selector_direction(int _steps)
{
	if (_steps < 0)
	{
        _steps = _steps * -1;
        digitalWrite(selectorDirPin, LOW);
	}
	else
	{
        digitalWrite(selectorDirPin, HIGH);
    }
	return _steps;
}
int set_pulley_direction(int _steps)
{
	if (_steps < 0)
	{
		_steps = _steps * -1;
		set_pulley_dir_pull();
	}
	else
	{
		set_pulley_dir_push();
	}
	return _steps;
}

void set_pulley_dir_push()
{
    digitalWrite(pulleyDirPin, LOW);
}
void set_pulley_dir_pull()
{
    digitalWrite(pulleyDirPin, HIGH);
}


bool checkOk()
{
	bool _ret = false;
	int _steps = 0;
	int _endstop_hit = 0;


	// filament in FINDA, let's try to unload it
	set_pulley_dir_pull();
    if (check_finda() == 1)
	{
		_steps = 3000;
		_endstop_hit = 0;
		do
		{
			do_pulley_step();
			delayMicroseconds(3000);
            if (check_finda() == 0) _endstop_hit++;
			_steps--;
		} while (_steps > 0 && _endstop_hit < 50);
	}

    if (check_finda() == 0)
	{
		// looks ok, load filament to FINDA
		set_pulley_dir_push();
		
		_steps = 3000;
		_endstop_hit = 0;
		do
		{
			do_pulley_step();
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
			set_pulley_dir_pull();
			for (int i = 600; i > 0; i--)   // 570
			{
				do_pulley_step();
				delayMicroseconds(3000);
			}
			_ret = true;
		}

	}
	else
	{
		// something is wrong, abort
		_ret = false;
	}

	return _ret;
}
