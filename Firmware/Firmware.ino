#include "spectroscopico.h"

#define PIN_IN1 5
#define PIN_IN2 3

Adafruit_AS7341 as7341;
AS7265X sensor;
RotaryEncoder *encoder = nullptr;
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ 9, /*DC=*/ 8, /*RST=*/ 12, /*BUSY=*/ 13)); // Waveshare Pico-ePaper-2.9
SPIClassRP2040 SPIn(spi1, -1, 13, 10, 11);

void setup() {
  //for RNG of sensor data, disable if not required
  randomSeed(analogRead(A0));

  //button interrupt
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(15, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(15), handleBTNINT, FALLING);

  //display initialisation
  // pinMode(13, INPUT_PULLDOWN); //pull down busy pin
  display.epd2.selectSPI(SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  display.init(115200, true, 10, false, SPIn, SPISettings(4000000, MSBFIRST, SPI_MODE0));
  //full refresh to begin
  display.clearScreen();

  //encoder initialisation
  encoder = new RotaryEncoder(PIN_IN1, PIN_IN2, RotaryEncoder::LatchMode::FOUR3);
  attachInterrupt(digitalPinToInterrupt(PIN_IN1), checkPosition, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_IN2), checkPosition, CHANGE);
  
  //spectrometer initialisation
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  Wire.setSCL(1);
  Wire.setSDA(0);
  delay(750); //750ms gives E-Paper enough time to initialise
  if (sensor.begin() == false){ //no sensor detected
    bigText("No Sensor Connected");
  }
  else{
    bigText("Valid Sensor Connected");
    sensor.disableIndicator();
  }
  delay(750);
  drawEmpty("Booted");
}

void loop() {

}
