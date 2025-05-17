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
    bigText(false, "No Sensor Detected");
  }
}


void loop() {
  // Your encoder check is interrupt-driven, so no need to call checkPosition() here normally.

  if (buttonpress) {
    Serial.print("LOOP: buttonpress TRUE. Current 'measuring': "); Serial.println(measuring); // DEBUG
    buttonpress = false; // Consume the flag *immediately*

    if (measuring) {
      // If currently measuring, this press means STOP.
      measuring = false; // Signal the currently running multimeasure to stop.
      Serial.println("LOOP: Set 'measuring' to FALSE (requesting stop).");
      // multimeasure's internal 'while(measuring)' should catch this.
      // No need to call multimeasure() again here.
      // The screen update to "Stopped" or "Ready" will happen after multimeasure returns.
    } else {
      // Not measuring, so this press means START.
      measuring = true;
      Serial.print("LOOP: Set 'measuring' to TRUE (requesting start). Mode: "); Serial.println(sensemode);

      multimeasure(); // Call the function. It will perform its action(s).
                      // For continuous/burst, it loops internally until 'measuring' is false.

      // After multimeasure() returns (it completed, or was stopped by 'measuring' becoming false):
      Serial.println("LOOP: multimeasure() has returned.");
      measuring = false; // Ensure state is consistently 'not measuring' after any operation.

      // Update display to reflect the end of operation.
      // If multimeasure showed results for single/burst, that's fine.
      // If continuous was stopped, or any mode finished, show "Ready".
      drawEmpty(false, "Ready");
    }
  }

  rp2040.idleOtherCore();
  delay(20); // A small delay is good practice.
}
