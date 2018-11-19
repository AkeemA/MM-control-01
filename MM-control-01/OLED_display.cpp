//OLED_display.c

#include "OLED_display.h"
#include <avr/io.h>

OLEDlib OLED_disp;  // OLEDlib object 

void LCD_init(int address, int width, int height)
{
  OLED_disp.init(address, width, height);
}

void LCD_print(int x, int y, const String text)
{
  OLED_disp.setCursorXY(x,y);
  OLED_disp.print(text);
}

void LCD_select(int x, int y)
{

}
