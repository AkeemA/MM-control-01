#ifndef OLEDlib_data_H
#define OLEDlib_data_H

#include <Arduino.h>

#define OLEDlib_Max_X					128	//128 Pixels
#define OLEDlib_Max_Y					32	//64  Pixels

// registers
#define OLEDlib_COMMAND_MODE				0x80
#define OLEDlib_DATA_MODE				0x40

// commands
#define OLEDlib_CMD_DISPLAY_OFF			0xAE
#define OLEDlib_CMD_DISPLAY_ON			0xAF
#define OLEDlib_CMD_NORMAL_DISPLAY		0xA6
#define OLEDlib_CMD_INVERSE_DISPLAY		0xA7
#define OLEDlib_CMD_SET_BRIGHTNESS		0x81

#define OLEDlib_RIGHT_SCROLL				0x26
#define OLEDlib_LEFT_SCROLL				0x27
#define OLEDlib_SET_VERTICAL_SCROLL_AREA 0xA3
#define OLEDlib_VERTICAL_RIGHT_SCROLL	0x29
#define OLEDlib_VERTICAL_LEFT_SCROLL		0x2A
#define OLEDlib_CMD_ACTIVATE_SCROLL		0x2F
#define OLEDlib_CMD_DEACTIVATE_SCROLL	0x2E

#define HORIZONTAL_ADDRESSING	0x00
#define PAGE_ADDRESSING			0x02

#define Scroll_Left				0x00
#define Scroll_Right			0x01
#define Scroll_Up				0x02
#define Scroll_Down				0x03

#define Scroll_2Frames			0x07
#define Scroll_3Frames			0x04
#define Scroll_4Frames			0x05
#define Scroll_5Frames			0x00
#define Scroll_25Frames			0x06
#define Scroll_64Frames			0x01
#define Scroll_128Frames		0x02
#define Scroll_256Frames		0x03



//===========
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22
//===========

#define BLACK 0
#define WHITE 1
#define INVERSE 2

class OLEDlib : public Print{

public :
  int oled_address;

  byte addressingMode;

  virtual size_t write(uint8_t);

  OLEDlib(){};
  ~OLEDlib(){};
  void sendCommand(byte command);
  void sendData(byte Data);

  void printChar(char c, byte X=255, byte Y=255);
  void printSpecialChar(char sChar,byte X=255, byte Y=255);
  void printString(const char *String, byte X=255, byte Y=255, byte numChar=255);
  byte printNumber(long n, byte X=255, byte Y=255);
  byte printNumber(float float_num, byte prec=6, byte Y=255, byte numChar=255);
  void printBigNumber(const char number, float column=0.0, float page=0.0, byte numChar=255);
  void drawBitmap(const byte *bitmaparray, byte X, byte Y, byte width, byte height);
  void drawBitmap2(const byte *bitmaparray, byte X, byte Y, byte width, byte height);
  void drawBitmap3(int x, int y, const byte *bitmap, int w, int h, byte color);
  void drawPixel(int x, int y, byte color);
  void drawGearFrame(const byte *bitmap);

  void init(int address, int width, int height);

  void setCursorXY(float Column, float Row);
  void clearDisplay();
  //void clearPage(byte page);

  void setNormalDisplay(void);
  void setInverseDisplay(void);
  void setPowerOff(void);
  void setPowerOn(void);
  void setPageMode(void);
  void setHorizontalMode(void);
  void setBrightness(byte Brightness);

  void scrollRight(byte start, byte end, byte speed);
  void scrollLeft(byte start, byte end, byte speed);
  void scrollDiagRight(void);
  void scrollDiagLeft(void);
  void setActivateScroll(byte direction, byte startPage, byte endPage, byte scrollSpeed);
  void setDeactivateScroll(void);

  int _width;
  int _height;

};

#endif


