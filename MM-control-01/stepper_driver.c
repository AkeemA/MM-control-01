#include "stepper_driver.h"
#include <avr/io.h>


void driver_enable_motor(uint8_t motor)
{
    digitalWrite(motor, ENABLE);
}

void driver_enable_all()
{
    digitalWrite(idlerEnablePin, ENABLE);
    digitalWrite(pulleyEnablePin, ENABLE);
    digitalWrite(selectorEnablePin, ENABLE);
}

void driver_disable_motor(uint8_t motor)
{
    digitalWrite(motor, DISABLE);
}

void driver_disable_all()
{
    digitalWrite(idlerEnablePin, DISABLE);
    digitalWrite(pulleyEnablePin, DISABLE);
    digitalWrite(selectorEnablePin, DISABLE);
}

void driver_init()
{
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
