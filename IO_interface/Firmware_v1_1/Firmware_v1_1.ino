#include <Adafruit_GFX.h>
#include "spectroscopico.h"
#include "IO_handler.h"
#include "RPi_Pico_ISR_Timer.h"

//----------------------------------------------------------------------------------------------------//
// Objects & Constants Definition
//----------------------------------------------------------------------------------------------------//
#define PIN_IN1 5
#define PIN_IN2 3



Adafruit_AS7341 as7341;
AS7265X sensor;
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 9, /*DC=*/ 8, /*RST=*/ 12, /*BUSY=*/ 13)); // Waveshare Pico-ePaper-2.9
SPIClassRP2040 SPIn(spi1, -1, 13, 10, 11);

//----------------------------------------------------------------------------------------------------//
// Set-Up Function
//----------------------------------------------------------------------------------------------------//
void setup() 
{
  beginIO();
  //for RNG of sensor data, disable if not required
  randomSeed(analogRead(A0));

  //display initialisation
  display.epd2.selectSPI(SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200, true, 10, false, SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  //full refresh to begin
  display.clearScreen();
  
  //spectrometer initialisation
  pinMode(0, OUTPUT); //SDA
  pinMode(1, OUTPUT); //SCL
  pinMode(16, OUTPUT); //EXTERNAL LED ENABLE
  Wire.setSCL(1);
  Wire.setSDA(0);
  delay(750); //750ms gives E-Paper enough time to initialise
  if (sensor.begin() == true){ //first check for AS7265x
    sensecon = 1;
    bigText(false, "AS7265x Connected");
    sensor.disableIndicator();
  }
  else if (as7341.begin() == true){ //if no AS7265x then check for AS7341
    sensecon = 2;
    bigText(false, "AS7341 Connected");
    as7341.setATIME(100);
    as7341.setASTEP(999);
    as7341.setGain(AS7341_GAIN_256X);
    as7341.setLEDCurrent(20); //mA
  }
  else{
    sensecon = 0;
    bigText(false, "No Sensor Detected, Generating Random Results");
  }
  delay(1000);
  drawEmpty(false, "Booted");
}

//----------------------------------------------------------------------------------------------------//
// Loop Function (SHOULD HAVE NOTHING)
//----------------------------------------------------------------------------------------------------//
void loop() {
  //delay(4000);
  if(checkFlag()){readSensor(false);}
}
