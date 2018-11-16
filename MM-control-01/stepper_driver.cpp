#include "stepper_driver.h"
#include "main.h"
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
