#include "stepper_driver.h"
#include "main.h"
#include "mmctl.h"
#include <avr/io.h>

#ifdef STEPPER_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef STEPPER_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

void driver_enable_motor(uint8_t motor)
{
  TRACE_LOG("Called: driver_enable_motor(), parameter:");
  TRACE_LOG(motor);
  digitalWrite(motor, ENABLE);
}

void driver_enable_all()
{
  TRACE_LOG("Called: driver_enable_all()");
  digitalWrite(idlerEnablePin, ENABLE);
  digitalWrite(pulleyEnablePin, ENABLE);
  digitalWrite(selectorEnablePin, ENABLE);
}

void driver_disable_motor(uint8_t motor)
{
  TRACE_LOG("Called: driver_disable_motor(), parameter:");
  TRACE_LOG(motor);
  digitalWrite(motor, DISABLE);
}

void driver_disable_all()
{
  TRACE_LOG("Called: driver_disable_all()");
  digitalWrite(idlerEnablePin, DISABLE);
  digitalWrite(pulleyEnablePin, DISABLE);
  digitalWrite(selectorEnablePin, DISABLE);
}

void driver_init()
{
  TRACE_LOG("Called: driver_init()");
  pinMode(idlerDirPin, OUTPUT);
  pinMode(idlerStepPin, OUTPUT);
  pinMode(idlerEnablePin, OUTPUT);

  pinMode(pulleyEnablePin, OUTPUT);
  pinMode(pulleyDirPin, OUTPUT);
  pinMode(pulleyStepPin, OUTPUT);

  pinMode(selectorEnablePin, OUTPUT);
  pinMode(selectorDirPin, OUTPUT);
  pinMode(selectorStepPin, OUTPUT);
}

void do_step(const int (&_motor)[4])
{
  TRACE_LOG("Start");
  digitalWrite(_motor[STEP_PIN], HIGH);
  asm("nop");
  digitalWrite(_motor[STEP_PIN], LOW);
  asm("nop");
  TRACE_LOG("End");
}

void move_proportional(int _idler, int _selector) // TODO: check how to refactor it, dont like the speed value
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_idler);
  TRACE_LOG(_selector);
  // gets steps to be done and set direction
  _idler = set_direction(idlerMotor, _idler);
  _selector = set_direction(selectorMotor, _selector);

  float _idler_step = _selector ? (float)_idler/(float)_selector : 1.0;
  float _idler_pos = 0;
  int _speed = 2500;
  int _start = _selector - 250;
  int _end = 250;

  while (_selector != 0 || _idler != 0 )
    {
      if (_idler_pos >= 1)
        {
          if (_idler > 0) { digitalWrite(idlerMotor[STEP_PIN], HIGH); }
        }
      if (_selector > 0) { digitalWrite(selectorMotor[STEP_PIN], HIGH); }

      asm("nop");

      if (_idler_pos >= 1)
        {
          if (_idler > 0) { digitalWrite(idlerMotor[STEP_PIN], LOW); _idler--;  }
        }

      if (_selector > 0) { digitalWrite(selectorMotor[STEP_PIN], LOW); _selector--; }
      asm("nop");

      if (_idler_pos >= 1)
        {
          _idler_pos = _idler_pos - 1;
        }


      _idler_pos = _idler_pos + _idler_step;

      delayMicroseconds(_speed);
      if (_speed > 900 && _selector > _start) { _speed = _speed - 10; }
      if (_speed < 2500 && _selector < _end) { _speed = _speed + 10; }

    }

  TRACE_LOG("End");
}

void moveMotor(const int (&_motor)[4], long _steps, int _speed)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_motor);
  TRACE_LOG(_steps);
  if(_speed == EMPTY_SPEED) _speed = _motor[SPEED_VALUE];
  int _acc = 50;

  // gets steps to be done and set direction
  _steps = set_direction(_motor, _steps);

  do
    {
      if (_steps > 0) { digitalWrite(_motor[STEP_PIN], HIGH); }
      asm("nop");
      if (_steps > 0) { digitalWrite(_motor[STEP_PIN], LOW); _steps--; delayMicroseconds(_speed); }
      asm("nop");

      if (_acc > 0) { delayMicroseconds(_acc*10); _acc = _acc - 1; }; // super pseudo acceleration control

    } while (_steps != 0);

  TRACE_LOG("End, parameters:");
}

void moveMotorWithSensor(const int (&_motor)[4], long _steps, int _sensor, int _speed)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_motor);
  TRACE_LOG(_steps);
  if(_speed == EMPTY_SPEED) _speed = _motor[SPEED_VALUE];

  // gets steps to be done and set direction
  _steps = set_direction(_motor, _steps);
  int _endstop_hit = 0;

  do
    {
      do_step(_motor);
      _steps--;
      delayMicroseconds(_speed);

      if ( _sensor == FINDA_SENSOR && check_finda() == 1 ) _endstop_hit++;
      else if ( _sensor == EXTRUDER_SENSOR && check_extruder_sensor() == 1 ) _endstop_hit++;
    } while ( _endstop_hit < 100 && _steps > 0);

  TRACE_LOG("End, parameters:");
}

void move(long _idler, long _selector, long _pulley)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_idler);
  TRACE_LOG(_selector);
  TRACE_LOG(_pulley);
  int _acc = 50;

  // gets steps to be done and set direction
  _idler = set_direction(idlerMotor, _idler);
  _selector = set_direction(selectorMotor, _selector);
  _pulley = set_direction(pulleyMotor, _pulley);

  do
    {
      if (_idler > 0) { digitalWrite(idlerMotor[STEP_PIN], HIGH); }
      if (_selector > 0) { digitalWrite(selectorMotor[STEP_PIN], HIGH); }
      if (_pulley > 0) { digitalWrite(pulleyMotor[STEP_PIN], HIGH); }
      asm("nop");
      if (_idler > 0) { digitalWrite(idlerMotor[STEP_PIN], LOW); _idler--; delayMicroseconds(idlerMotor[SPEED_VALUE]); }
      if (_selector > 0) { digitalWrite(selectorMotor[STEP_PIN], LOW); _selector--;  delayMicroseconds(selectorMotor[SPEED_VALUE]); }
      if (_pulley > 0) { digitalWrite(pulleyMotor[STEP_PIN], LOW); _pulley--;  delayMicroseconds(pulleyMotor[SPEED_VALUE]); }
      asm("nop");

      if (_acc > 0) { delayMicroseconds(_acc*10); _acc = _acc - 1; }; // super pseudo acceleration control

    } while (_selector != 0 || _idler != 0 || _pulley != 0);

  TRACE_LOG("End, parameters:");
}

long set_direction(const int (&_motor)[4], long _steps)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_steps);
  if (_steps < 0)
    {
      _steps = _steps * -1;
      digitalWrite(_motor[DIR_PIN], LOW);
    }
  else
    {
      digitalWrite(_motor[DIR_PIN], HIGH);
    }
  TRACE_LOG("End, result:");
  TRACE_LOG(_steps);
  return _steps;
}

