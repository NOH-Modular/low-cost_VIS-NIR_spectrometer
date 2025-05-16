#ifndef _SPECTROSCOPICO_H
#define _SPECTROSCOPICO_H

/*
Includes
*/
#include <Arduino.h>
#include <Wire.h> //I2C

//GxEPD2 Configuration (MUST BE BEFORE <GxEPD2_BW.h>)
#define ENABLE_GxEPD2_GFX 1
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW
#define MAX_DISPLAY_BUFFER_SIZE 131072ul
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))
#define GxEPD2_DRIVER_CLASS GxEPD2_213_GDEY0213B74 // GDEY0213B74 122x250, SSD1680

#include <GxEPD2_BW.h> //E-Paper display library
#include <RotaryEncoder.h> //Rotary encoder library
#include <SparkFun_AS7265X.h> //AS7265x spectral sensor library (18 channels)
#include <Adafruit_AS7341.h> //AS7341 spectral sensor library (10 channels + flicker detect)

#include "Fonts/FreeMonoBold9pt7b.h" //Medium font
#include "Fonts/FreeMonoBold18pt7b.h" //Large font

#include "18chanbase.h" //Custom bitmap for base UI (AS7265x)
#include "10chanbase.h" //Custom bitmap for base UI (AS7341)

/*
Global Objects and Variables, defined in .ino
*/
extern GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display;
extern SPIClassRP2040 SPIn;
extern RotaryEncoder *encoder;
extern Adafruit_AS7341 as7341;
extern AS7265X sensor;
/*
Global Objects and Variables, defined in .cpp
*/
extern int8_t pos; // Declaration
extern int8_t newpos; // Declaration
extern float readings[18]; // Declaration
extern uint8_t intreadings[18]; // Declaration
extern uint8_t intreadingsmem[9][18]; // Declaration
extern uint8_t ledmode; // Declaration
extern uint8_t sensemode; // Declaration
extern volatile bool ledState; // Declaration
extern volatile bool measuring; // Declaration
extern volatile unsigned long lastInterruptTime; // Declaration
extern const unsigned long debounceDelay; // Declaration

/*
Lookup Tables
*/

// Wavelength Channel names
extern const char* wavelengthNames18[];
extern const char* wavelengthNames10[];

/*
Spectral Sensor Functions
*/
//spectral reading, ledmode 0 for no LEDs, 1 for inbuilt LEDs, (2 for external LEDs, 4 for all LEDs)
void measure();

//should first draw "waiting for reading", then take a reading if sensor connected or generate fake results, then finally draw the data on the screen
void multimeasure();

//determine the dominant colour (based on Sparkfun example)
String detectColour18();
String detectColour10();

/*
Drawing Functions
*/
void bigText(String text);
void drawEmpty(String toptext);
void drawMain(String toptext, uint8_t *finalreadings);

/*
Input Handlers
*/
void checkPosition();

void handleBTNINT();


#endif 