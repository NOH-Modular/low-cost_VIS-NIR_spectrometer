/*
  Bar graph for AS7265x Spectral Triad
  Modified from SparkFun's Example2_BasicReadingsWithLEDs.ino
  
  This program takes all 18 spectral readings from the AS7265x Triad sensor and
  displays them in the serial monitor with a visual representation.
  
  Features:
  - Wavelength labels for each reading
  - Bar chart visualisation of readings in the serial monitor
  - Colour spectrum 
  - Colour identification based on the dominant wavelengths
  
  Hardware Connections:
  Open the serial monitor at 115200 baud to see the visualisation
*/

#include "SparkFun_AS7265X.h" //http://librarymanager/All#SparkFun_AS7265X
AS7265X sensor;

#include <Wire.h>

// Wavelengths
const char* wavelengthNames[] = {
  "410nm (A) - Violet  ", "435nm (B) - Indigo  ", "460nm (C) - Blue    ",
  "485nm (D) - Cyan    ", "510nm (E) - Green   ", "535nm (F) - Lime    ",
  "560nm (G) - Yellow  ", "585nm (H) - Orange  ", "610nm (R) - Orange+ ",
  "645nm (I) - Red     ", "680nm (S) - Deep Red", "705nm (J) - Red+    ",
  "730nm (T) - NIR     ", "760nm (U) - NIR     ", "810nm (V) - NIR     ",
  "860nm (W) - NIR     ", "900nm (K) - NIR     ", "940nm (L) - NIR     "
};

// Store the sensor readings in an array
float readings[18];

// Determine the dominant colour range
String detectColour() {
  int maxIndex = 0;
  float maxVal = readings[0];
  
  // Find the peak wavelength
  for (int i = 1; i < 18; i++) {
    if (readings[i] > maxVal) {
      maxVal = readings[i];
      maxIndex = i;
    }
  }
  
  // Determine colour based on peak wavelength
  if (maxIndex <= 2) return "VIOLET/BLUE";
  else if (maxIndex <= 4) return "CYAN/GREEN";
  else if (maxIndex <= 6) return "YELLOW/LIME";
  else if (maxIndex <= 9) return "ORANGE/RED";
  else if (maxIndex <= 11) return "DEEP RED";
  else return "INFRARED (NOT VISIBLE)";
}

void setup() {
  Serial.begin(115200);
  Serial.println("AS7265x Spectral Triad Enhanced Visualisation");
  Serial.println();
  
  Serial.println("Point the Triad at your sample and press any key to begin...");
  while (Serial.available() == false) {
    // Wait for user to press a key
  }
  Serial.read(); 
  
  if (sensor.begin() == false) {
    Serial.println("Sensor not detected. Please check wiring. Freezing...");
    while (1);
  }
  
  sensor.disableIndicator(); 
}

void loop() {
  // Take measurements and store in array
  sensor.takeMeasurementsWithBulb();
  
  readings[0] = sensor.getCalibratedA();  // 410nm
  readings[1] = sensor.getCalibratedB();  // 435nm
  readings[2] = sensor.getCalibratedC();  // 460nm
  readings[3] = sensor.getCalibratedD();  // 485nm
  readings[4] = sensor.getCalibratedE();  // 510nm
  readings[5] = sensor.getCalibratedF();  // 535nm
  readings[6] = sensor.getCalibratedG();  // 560nm
  readings[7] = sensor.getCalibratedH();  // 585nm
  readings[8] = sensor.getCalibratedR();  // 610nm
  readings[9] = sensor.getCalibratedI();  // 645nm
  readings[10] = sensor.getCalibratedS(); // 680nm
  readings[11] = sensor.getCalibratedJ(); // 705nm
  readings[12] = sensor.getCalibratedT(); // 730nm
  readings[13] = sensor.getCalibratedU(); // 760nm
  readings[14] = sensor.getCalibratedV(); // 810nm
  readings[15] = sensor.getCalibratedW(); // 860nm
  readings[16] = sensor.getCalibratedK(); // 900nm
  readings[17] = sensor.getCalibratedL(); // 940nm
  
  // Clear the screen 
  Serial.write(27);      // ESC command
  Serial.print("[2J");   // Clear screen command
  Serial.write(27);
  Serial.print("[H");    // Cursor to home command
  
  // Print header
  Serial.println(F("------------------------------------------------"));
  Serial.println(F("   AS7265x SPECTRAL TRIAD - VISUAL READINGS     "));
  Serial.println(F("------------------------------------------------"));
  
  // Find the maximum value for scaling the bar chart
  float maxReading = 0;
  for (int i = 0; i < 18; i++) {
    if (readings[i] > maxReading) maxReading = readings[i];
  }
  
  // Ensure we have a reasonable max for the graph
  if (maxReading < 10) maxReading = 10;
  
  // Print the readings with bar graphs
  for (int i = 0; i < 18; i++) {
    // Print the wavelength name and the value
    Serial.print(wavelengthNames[i]);
    Serial.print(": ");
    Serial.print(readings[i], 2);
    Serial.print("\t");
    
    // Print a bar graph using ASCII characters
    int barLength = map(readings[i] * 100, 0, maxReading * 100, 0, 40);
    
    // Choose character based on wavelength range (to indicate colour)
    char barChar;
    if (i <= 2) barChar = '~';      // Violet/Blue range
    else if (i <= 4) barChar = '=';  // Green range
    else if (i <= 6) barChar = '#';  // Yellow range
    else if (i <= 11) barChar = '+'; // Red range
    else barChar = '.';              // Infrared (non-visible)
    
    for (int j = 0; j < barLength; j++) {
      Serial.print(barChar);
    }
    Serial.println();
  }
  
  // Identify and display the detected colour
  Serial.println(F("------------------------------------------------"));
  Serial.print(F("DETECTED COLOUR RANGE: "));
  Serial.println(detectColour());
  Serial.println(F("------------------------------------------------"));
  
  // Print a simple spectral visualisation
  Serial.println();
  Serial.println(F("SPECTRUM: [VIOLET]---[BLUE]---[GREEN]---[YELLOW]---[RED]---[INFRARED]"));
  Serial.println();
  
  // Display note about updating
  Serial.println(F("Updating every 2 seconds... Press any key to stop"));
  
  // Check if user wants to stop
  if (Serial.available()) {
    Serial.read();
    Serial.println("Stopped. Press any key to resume...");
    while (!Serial.available());
    Serial.read(); // Clear the input buffer
  }
  
  delay(2000); // Update every 2 seconds
}