//display.cpp - display functionality - LCD, LED...
#include "main.h"
#include <Arduino.h>
#include <stdio.h>
#include <string.h>
#include <avr/io.h>
#include "display.h"
#include "motion.h"
#include "Buttons.h"
#include "OLED_display.h"

#ifdef DISP_LOG
#define LOG(x) DBG_PRINT(x)
#else
#define LOG(x)
#endif

#ifdef DISP_TRACE
#define TRACE_LOG(x) DBG_PRINT(x)
#else
#define TRACE_LOG(x)
#endif

int led_array[5] = {0};

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

#ifdef EMULATED_LEDS
void print_led_frame()
{
  LCD_print(1,0,"( )( )( )( )( )");
}

void led_on(int _no, int color)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_no);
  TRACE_LOG(color);

  if(_no >= 0 && _no < 5 )
    {
      if(led_array[_no]<3 && led_array[_no] != color)
        {
          led_array[_no] = led_array[_no] + color;
          LCD_print(2+3*_no,0,led_as_char(led_array[_no]));
        }
      LOG("new led on: ");
      LOG(led_array[_no]);
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
      if(led_array[_no] != OFF_LED)
        {
          led_array[_no] = OFF_LED;
          LCD_print(2+3*_no,0,led_as_char(led_array[_no]));
        }
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

  for(int i = 0; i<5; i++)
    {
      led_on(i,color);
    }

  TRACE_LOG("End");
}

void all_leds_off()
{
  TRACE_LOG("Start");
  bool changed = false;

  for(int i = 0; i<5; i++)
    {
      led_off(i);
    }

  TRACE_LOG("End");
}

void led_blink(int _no, int color)
{
  TRACE_LOG("Start, parameters:");
  TRACE_LOG(_no);

  led_on(_no, color);
  delay(500);
  led_off(_no);
  delay(500);

  TRACE_LOG("End");
}

String led_as_char(int _no)
{
  if(_no == 0) return " ";
  if(_no == 1) return "R";
  if(_no == 2) return "G";
  if(_no == 3) return "Z";
}

#else

void led_blink(int _no, int color){}
void led_on(int _no, int color){}
void led_off(int _no){}
void all_leds_on(int color){}
void all_leds_off(){}
void print_led_frame(){}
String led_as_char(int _no){}
#endif
