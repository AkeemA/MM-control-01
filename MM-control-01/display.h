//display.h - display functionality - LCD, LED...
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <inttypes.h>
#include "config.h"
#include <Arduino.h>

void led_blink(int _no, int color);
void led_on(int _no, int color);
void led_off(int _no);
void all_leds_on(int color);
void all_leds_off();
void print_led_frame();
String led_as_char(int _no);

#endif //_DISPLAY_H
