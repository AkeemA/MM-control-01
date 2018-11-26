#ifndef _STEPDRV_H
#define _STEPDRV_H

#include <inttypes.h>
#include "config.h"
#include "main.h"


#if defined(__cplusplus)
extern "C" {
#endif //defined(__cplusplus)

void driver_init();
void driver_enable_motor(uint8_t motor);
void driver_enable_all();
void driver_disable_motor(uint8_t motor);
void driver_disable_all();

long set_direction(const int (&_motor)[4], long _steps);

void do_step(const int (&_motor)[4]);
void move_proportional(int _idler, int _selector);
void moveMotor(const int (&_motor)[4], long _steps, int _speed = EMPTY_SPEED);
void moveMotorWithSensor(const int (&_motor)[4], long _steps, int _sensor, int _speed = EMPTY_SPEED);

#if defined(__cplusplus)
}
#endif //defined(__cplusplus)
#endif //_STEPDRV_H
