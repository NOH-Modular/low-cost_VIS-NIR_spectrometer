#include <SPI.h>

#include <GxEPD2_BW.h>

// UC8151D 2.13" b/w display
// Use B72 variant (GDEH0213B72) — that’s the one Waveshare 2.13" V4 uses
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT> display(
    GxEPD2_213_B72(/*CS=*/ 9, /*DC=*/ 8, /*RST=*/ 12, /*BUSY=*/ 13)
);

#define EPD_SCK_PIN     10
#define EPD_MOSI_PIN    11
#define EPD_CS_PIN      9
#define EPD_DC_PIN      8
#define EPD_RST_PIN     12
#define EPD_BUSY_PIN    13

void setup() {
  // Manual SPI setup on the Pico
  SPI.setSCK(EPD_SCK_PIN);
  SPI.setTX(EPD_MOSI_PIN); // Pico SPI1 uses TX for MOSI
  SPI.begin();

  display.init();
  
  display.setRotation(1);
  display.setTextColor(GxEPD_BLACK);

  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(10, 30);
    display.print("E-Ink, E-Life");
  } while (display.nextPage());
}

void loop() {
  // because the compiler is needy
}