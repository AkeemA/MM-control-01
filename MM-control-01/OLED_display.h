//OLED_display.h
#ifndef _OLEDDISPLAY_H
#define _OLEDDISPLAY_H

#include <inttypes.h>
#include <Arduino.h>
#include "config.h"
#include "OLEDlib.h"

void LCD_init(int address = 0x3C, int width = 128, int height = 32);
void LCD_print(int x, int y, const String text);
void LCD_select(int x, int y);

#endif //_OLEDDISPLAY_H
