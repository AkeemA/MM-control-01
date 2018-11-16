//OLED_display.h
#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <inttypes.h>
#include <Arduino.h>
#include "config.h"
#include "OLEDlib.h"

extern void LCD_init(int address = 0x3C, int width = 128, int height = 32);
extern void LCD_print(int x, int y, const String text);
extern void LCD_select(int x, int y);

#endif //_DISPLAY_H
