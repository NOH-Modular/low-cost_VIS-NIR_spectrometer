#include "spectroscopico.h"

/*
Global Variable Definitions
*/
int8_t pos = 0;
int8_t newpos = 0;
float readings18[18];
uint16_t readings10[10];
uint8_t intreadings[18];
uint8_t intreadingsmem[9][18];
uint8_t sensecon;
uint8_t ledmode = 1;
uint8_t sensemode = 0;
volatile bool ledState = LOW;
volatile bool measuring = false;
volatile bool buttonpress = false;
volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 200; //ms

const char* wavelengthNames18[] = {
  "410nm (A) - Violet  ", "435nm (B) - Indigo  ", "460nm (C) - Blue    ",
  "485nm (D) - Cyan    ", "510nm (E) - Green   ", "535nm (F) - Lime    ",
  "560nm (G) - Yellow  ", "585nm (H) - Orange  ", "610nm (R) - Orange+ ",
  "645nm (I) - Red     ", "680nm (S) - Deep Red", "705nm (J) - Red+    ",
  "730nm (T) - NIR     ", "760nm (U) - NIR     ", "810nm (V) - NIR     ",
  "860nm (W) - NIR     ", "900nm (K) - NIR     ", "940nm (L) - NIR     "
};
const char* wavelengthNames10[] = {
  "415nm (F1) - Violet", "445nm (F2) - Deep Blue", "480nm (F3) - Light Blue",
  "515nm (F4) - Green", "555nm (F5) - Yellow-Green", "590nm (F8) - Yellow",
  "630nm (F1) - Deep Orange", "680nm (F2) - Red",
  "Clear", "940nm (NIR) - NIR"
};

/*
Spectral Sensor Functions
*/
//spectral reading, ledmode 0 for no LEDs, 1 for inbuilt LEDs, (2 for external LEDs, 4 for all LEDs)
void measure(){
  if(sensecon == 1){ //AS7265x 18 channels
    if(ledmode == 0){ //measure with no LEDs
      sensor.takeMeasurements();
    }
    else if(ledmode == 1){ //measure with inbuilt LEDs
      sensor.takeMeasurementsWithBulb();
    }
    else if(ledmode == 2){ //measure with extenal LEDs
      //set GPIO16 high
      sensor.takeMeasurements();
      //set GPIO16 low
    }
    else if(ledmode == 3){ //measure with inbuilt and external LEDs
      //set GPIO16 high
      sensor.takeMeasurementsWithBulb();
      //set GPIO16 low
    }
    else{ //measure with no LEDs if invalid mode
      sensor.takeMeasurements();
    }
    readings18[0] = sensor.getCalibratedA();  // 410nm
    readings18[1] = sensor.getCalibratedB();  // 435nm
    readings18[2] = sensor.getCalibratedC();  // 460nm
    readings18[3] = sensor.getCalibratedD();  // 485nm
    readings18[4] = sensor.getCalibratedE();  // 510nm
    readings18[5] = sensor.getCalibratedF();  // 535nm
    readings18[6] = sensor.getCalibratedG();  // 560nm
    readings18[7] = sensor.getCalibratedH();  // 585nm
    readings18[8] = sensor.getCalibratedR();  // 610nm
    readings18[9] = sensor.getCalibratedI();  // 645nm
    readings18[10] = sensor.getCalibratedS(); // 680nm
    readings18[11] = sensor.getCalibratedJ(); // 705nm
    readings18[12] = sensor.getCalibratedT(); // 730nm
    readings18[13] = sensor.getCalibratedU(); // 760nm
    readings18[14] = sensor.getCalibratedV(); // 810nm
    readings18[15] = sensor.getCalibratedW(); // 860nm
    readings18[16] = sensor.getCalibratedK(); // 900nm
    readings18[17] = sensor.getCalibratedL(); // 940nm
    float maxReading = 0;
    for (int i = 0; i < 18; i++) {
      if (readings18[i] > maxReading) maxReading = readings18[i];
    }
    for (int i = 0; i < 18; i++) {
      intreadings[i] = readings18[i] / maxReading * 69;
    }
  }
  else if(sensecon == 2){ //AS7341 10 channels
    if(ledmode == 0){ //measure with no LEDs
      as7341.readAllChannels(readings10);
    }
    else if(ledmode == 1){ //measure with inbuilt LEDs
      as7341.enableLED(true);
      as7341.readAllChannels(readings10);
      as7341.enableLED(false);
    }
    else if(ledmode == 2){ //measure with extenal LEDs
      digitalWrite(16, HIGH);
      sensor.takeMeasurements();
      digitalWrite(16, LOW);
    }
    else if(ledmode == 3){ //measure with inbuilt and external LEDs
      digitalWrite(16, HIGH);
      as7341.enableLED(true);
      as7341.readAllChannels(readings10);
      as7341.enableLED(false);
      digitalWrite(16, LOW);
    }
    else{ //measure with no LEDs if invalid mode
      as7341.readAllChannels(readings10);
    }
    //need to re-order from F1,2,3,4,CLR,NIR,5,6,7,8 to F1,2,3,4,5,6,7,8,NIR,CLR
    uint16_t CLR = readings10[4];
    uint16_t NIR = readings10[5];
    readings10[4] = readings10[6];
    readings10[5] = readings10[7];
    readings10[6] = readings10[8];
    readings10[7] = readings10[9];
    readings10[8] = NIR;
    readings10[9] = CLR;

    //scales to 0-68 for display (65535 is largest value for uint16_t)
    float maxReading = 0;
    for (int i = 0; i < 10; i++) {
      if (readings10[i] > maxReading) maxReading = readings10[i];
    }
    for (int i = 0; i < 10; i++) {
      intreadings[i] = readings10[i] / maxReading * 69;
    }
  }
  else{ //randomly generates bogus data if no sensor connected
    for(uint8_t i = 0; i < 18; i++){ 
      intreadings[i] = random(68);
      readings18[i] = intreadings[i];
    }
    delay(750); //simulates integration time
  }
}

// spectroscopico.cpp
void multimeasure(){
  bigText(false, "Measuring..."); // Show "Measuring..." screen once at the start

  if(sensemode == 0){ // Single fire
    if (!measuring) return; // Should have been set true by main loop to start
    measure();
    if(sensecon == 2){
      // drawMain(false, detectColour10(), intreadings);
      drawMainRipe(false, bananaRipeness(), intreadings);
    }
    else{
      // drawMainRipe(false, detectColour18(), intreadings);
      drawMainRipe(false, bananaRipeness(), intreadings);
    }
    measuring = false; // Signal that this single measurement is done
  }
  else if(sensemode == 1){ // Continuous fire
    while(measuring){ // Continues until 'measuring' is set to false by the main loop
      measure();
      if(sensecon == 2){
        // drawMain(false, detectColour10(), intreadings);
        drawMainRipe(false, bananaRipeness(), intreadings);
      }
      else{
        // drawMain(false, detectColour18(), intreadings);
        drawMainRipe(false, bananaRipeness(), intreadings);
      }
      rp2040.idleOtherCore(); // Allow other core to run (e.g. for USB serial)
      delay(50); // Small delay to prevent hammering sensor/display
    }
  }
  else if(sensemode > 1){ // Burst fire (sensemode indicates number of measurements)
    if (!measuring) return;

    float tempReadings[18]; // Or 10, depending on sensor
    uint8_t numChannels = (sensecon == 2) ? 10 : 18;

    // Initialize intreadingsmem for averaging if needed, or average directly
    for (uint8_t ch = 0; ch < numChannels; ++ch) {
        if (numChannels == 18) intreadingsmem[0][ch] = 0; // Use a specific row or a temp sum array
        else { /* similar for 10 channels if you have a different averaging buffer */ }
    }
    // Or better, create a temporary sum array for the current burst:
    float channelSums[numChannels];
    for(uint8_t ch=0; ch<numChannels; ++ch) channelSums[ch] = 0.0;


    for(uint8_t i = 0; i < sensemode && measuring; i++){
      bigText(false, "Measure " + String(i+1) + "/" + String(sensemode));
      measure(); // Populates readings18/readings10 and intreadings

      // Accumulate readings for averaging
      for (uint8_t ch = 0; ch < numChannels; ++ch) {
        if (numChannels == 18) {
            channelSums[ch] += readings18[ch];
        } else {
            channelSums[ch] += readings10[ch];
        }
      }

       rp2040.idleOtherCore();
       delay(50); // Allow button check
    }

    if (measuring) { // Only process and display if not stopped mid-burst
        // Calculate averages
        for (uint8_t ch = 0; ch < numChannels; ++ch) {
            if (numChannels == 18) {
                readings18[ch] = channelSums[ch] / sensemode;
            } else {
                readings10[ch] = (uint16_t)(channelSums[ch] / sensemode);
            }
        }
        // Normalize the averaged readings18/readings10 into intreadings for display
        float maxReading = 0;
        if (numChannels == 18) {
            for (int i = 0; i < 18; i++) {
                if (readings18[i] > maxReading) maxReading = readings18[i];
            }
            for (int i = 0; i < 18; i++) {
                intreadings[i] = (maxReading == 0) ? 0 : readings18[i] / maxReading * 69;
            }
        } else { // AS7341 10 channels
            for (int i = 0; i < 10; i++) {
                if (readings10[i] > maxReading) maxReading = readings10[i];
            }
            for (int i = 0; i < 10; i++) {
                intreadings[i] = (maxReading == 0) ? 0 : readings10[i] / maxReading * 69;
            }
        }
        drawMain(false, (sensecon == 2 ? detectColour10() : detectColour18()), intreadings);
    }
    measuring = false; // Signal that burst is done or was stopped
  }
}

//determine the dominant colour (based on Sparkfun example)
String detectColour18() {
  int maxIndex = 0;
  float maxVal;
  
  // Find the peak wavelength
  for (int i = 1; i < 18; i++) {
    if (readings18[i] > maxVal) {
      maxVal = readings18[i];
      maxIndex = i+1; //number of the column that appears on screen
    }
  }
  
  // Determine dominant colour based on peak wavelength
  if (maxVal <= 1) return "No Light";
  else if (maxIndex <= 1) return "Violet";
  else if (maxIndex <= 3) return "Blue";
  else if (maxIndex <= 5) return "Green";
  else if (maxIndex <= 7) return "Yellow";
  else if (maxIndex <= 9) return "Orange";
  else if (maxIndex <= 14) return "Red";
  else return "NIR";
}
String detectColour10() {
  int maxIndex = 0;
  float maxVal;
  
  // Find the peak wavelength
  for (int i = 0; i < 9; i++) { //skips 10th channel as this is "clear"
    if (readings10[i] > maxVal) {
      maxVal = readings10[i];
      maxIndex = i+1;
    }
  }
  
  // Determine dominant colour based on peak wavelength
  if (maxVal <= 1) return "No Light";
  else if (maxIndex <= 1) return "Violet";
  else if (maxIndex <= 3) return "Blue";
  else if (maxIndex <= 5) return "Green";
  else if (maxIndex <= 6) return "Yellow";
  else if (maxIndex <= 7) return "Orange";
  else if (maxIndex <= 8) return "Red";
  else return "NIR";
}

uint8_t bananaRipeness(){ //compare prominence of ~650nm to ~550nm. Ratios close to 0.5 are unripe, 1 are ripe, 1.5 are overripe
  float ripeness;
  if(sensecon == 2){
    ripeness = (static_cast<float>(intreadings[7]) / intreadings[4]) - 0.7; 
    // ripeness = intreadings[6] / intreadings[4];
  }
  else{
    ripeness = (static_cast<float>(intreadings[7]) / intreadings[4]) - 0.7;
  }
  if(ripeness<0){
    ripeness = 0;
  }
  else if(ripeness>1){
    ripeness = 1;
  }
  ripeness *= 158; //(scale so 0.5 ratio corresponds to 0 and 1.5 corresponds to 160 (full scale) with 1 in the middle)
  return (uint8_t) ripeness;
}

/*
Drawing Functions
*/
void bigText(bool full, String text) {
  display.setRotation(1); //sets landscape rotation
  if(full){
    display.setFullWindow(); //sets full refresh mode
  }
  else{
    display.setPartialWindow(0, 0, display.width(), display.height()); //sets partial refresh mode 
  }
  display.firstPage();     //start paged drawing

  //Variables to store text bounds
  int16_t x1, y1;
  uint16_t w, h;

  do {
    //Set background for the current page
    display.fillScreen(GxEPD_WHITE);
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_BLACK);

    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    display.setCursor(((display.width() - w) / 2 - x1), ((display.height() - h) / 2 - y1));
    display.print(text);

  } while (display.nextPage()); // Send page buffer & check if more pages
}

void drawEmpty(bool full, String toptext) {
  display.setRotation(1); //sets landscape rotation
  if(full){
    display.setFullWindow(); //sets full refresh mode
  }
  else{
    display.setPartialWindow(0, 0, display.width(), display.height()); //sets partial refresh mode 
  }
  display.firstPage();     //start paged drawing

  do {
    //Set background for the current page
    display.fillScreen(GxEPD_WHITE);

    //Draw empty bars (Bitmap)
    display.drawBitmap(2, 37, base18, 245, 91, GxEPD_BLACK);
    
    //border to check boundaries
    // display.drawRect(0, 6, 250, 122, GxEPD_BLACK);

    //Draw measurement mode
    display.setFont(); //default 5x7 font
    display.setTextColor(GxEPD_BLACK);
    if(sensemode == 0){
      display.setCursor(181, 10);
      display.print("Single Fire");
    }
    else if(sensemode == 1){
      display.setCursor(187, 10);
      display.print("Continuous");
    }
    else{
      display.setCursor(205, 10);
      display.print("Burst ");
      display.print(sensemode);
    }
    
    //draw led mode
    if(ledmode == 0){
      display.setCursor(205, 23);
      display.print("No LEDs");
    }
    else if(ledmode == 1){
      display.setCursor(169, 23);
      display.print("Internal LEDs");
    }
    else if(ledmode == 2){
      display.setCursor(169, 23);
      display.print("External LEDs");
    }
    else if(ledmode == 3){
      display.setCursor(199, 23);
      display.print("All LEDs");
    }
    else{
      display.setCursor(175, 23);
      display.print("Invalid Mode");
    }

    //Draw Title text (Set font, color, cursor, print)
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 27);
    display.print(toptext);

  } while (display.nextPage()); // Send page buffer & check if more pages
}

void drawMain(bool full, String toptext, uint8_t *finalreadings) {
  display.setRotation(1); //sets landscape rotation
  if(full){
    display.setFullWindow(); //sets full refresh mode
  }
  else{
    display.setPartialWindow(0, 0, display.width(), display.height()); //sets partial refresh mode 
  }
  display.firstPage();     //start paged drawing

  do {
    //Set background for the current page
    display.fillScreen(GxEPD_WHITE);
    if(sensecon <= 1){ //for bogus data or AS7265x (18 channels)
      display.drawBitmap(2, 37, base18, 245, 91, GxEPD_BLACK);
      //Draw Filled bars (Rectangles)
      display.fillRect(3, 107, 9, -finalreadings[0], GxEPD_BLACK);
      display.fillRect(16, 107, 9, -finalreadings[1], GxEPD_BLACK);
      display.fillRect(30, 107, 9, -finalreadings[2], GxEPD_BLACK);
      display.fillRect(44, 107, 9, -finalreadings[3], GxEPD_BLACK);
      display.fillRect(58, 107, 9, -finalreadings[4], GxEPD_BLACK);
      display.fillRect(72, 107, 9, -finalreadings[5], GxEPD_BLACK);
      display.fillRect(85, 107, 9, -finalreadings[6], GxEPD_BLACK);
      display.fillRect(99, 107, 9, -finalreadings[7], GxEPD_BLACK);
      display.fillRect(113, 107, 9, -finalreadings[8], GxEPD_BLACK);
      display.fillRect(127, 107, 9, -finalreadings[9], GxEPD_BLACK);
      display.fillRect(140, 107, 9, -finalreadings[10], GxEPD_BLACK);
      display.fillRect(154, 107, 9, -finalreadings[11], GxEPD_BLACK);
      display.fillRect(168, 107, 9, -finalreadings[12], GxEPD_BLACK);
      display.fillRect(182, 107, 9, -finalreadings[13], GxEPD_BLACK);
      display.fillRect(195, 107, 9, -finalreadings[14], GxEPD_BLACK);
      display.fillRect(209, 107, 9, -finalreadings[15], GxEPD_BLACK);
      display.fillRect(223, 107, 9, -finalreadings[16], GxEPD_BLACK);
      display.fillRect(237, 107, 9, -finalreadings[17], GxEPD_BLACK);
    }
    else{ //for AS7341 (10 channels)
      display.drawBitmap(2, 37, base10, 246, 90, GxEPD_BLACK);
      
      for(uint8_t i = 0; i < 10; i++){
        display.fillRect((3+(i*25)), 107, 19, -finalreadings[i], GxEPD_BLACK);
      }
    }

    //Draw measurement mode
    display.setFont(); //default 5x7 font
    display.setTextColor(GxEPD_BLACK);
    if(sensemode == 0){
      display.setCursor(181, 10);
      display.print("Single Fire");
    }
    else if(sensemode == 1){
      display.setCursor(187, 10);
      display.print("Continuous");
    }
    else{
      display.setCursor(205, 10);
      display.print("Burst ");
      display.print(sensemode);
    }
    
    //draw led mode
    if(ledmode == 0){
      display.setCursor(205, 23);
      display.print("No LEDs");
    }
    else if(ledmode == 1){
      display.setCursor(169, 23);
      display.print("Internal LEDs");
    }
    else if(ledmode == 2){
      display.setCursor(169, 23);
      display.print("External LEDs");
    }
    else if(ledmode == 3){
      display.setCursor(199, 23);
      display.print("All LEDs");
    }
    else{
      display.setCursor(175, 23);
      display.print("Invalid Mode");
    }

    //Draw Title text (Set font, color, cursor, print)
    display.setFont(&FreeMonoBold18pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(0, 27);
    display.print(toptext);

  } while (display.nextPage()); // Send page buffer & check if more pages
}

void drawMainRipe(bool full, uint8_t ripeness, uint8_t *finalreadings) {
  display.setRotation(1); //sets landscape rotation
  if(full){
    display.setFullWindow(); //sets full refresh mode
  }
  else{
    display.setPartialWindow(0, 0, display.width(), display.height()); //sets partial refresh mode 
  }
  display.firstPage();     //start paged drawing

  do {
    //Set background for the current page
    display.fillScreen(GxEPD_WHITE);
    if(sensecon <= 1){ //for bogus data or AS7265x (18 channels)
      display.drawBitmap(2, 37, base18, 245, 91, GxEPD_BLACK);
      //Draw Filled bars (Rectangles)
      display.fillRect(3, 107, 9, -finalreadings[0], GxEPD_BLACK);
      display.fillRect(16, 107, 9, -finalreadings[1], GxEPD_BLACK);
      display.fillRect(30, 107, 9, -finalreadings[2], GxEPD_BLACK);
      display.fillRect(44, 107, 9, -finalreadings[3], GxEPD_BLACK);
      display.fillRect(58, 107, 9, -finalreadings[4], GxEPD_BLACK);
      display.fillRect(72, 107, 9, -finalreadings[5], GxEPD_BLACK);
      display.fillRect(85, 107, 9, -finalreadings[6], GxEPD_BLACK);
      display.fillRect(99, 107, 9, -finalreadings[7], GxEPD_BLACK);
      display.fillRect(113, 107, 9, -finalreadings[8], GxEPD_BLACK);
      display.fillRect(127, 107, 9, -finalreadings[9], GxEPD_BLACK);
      display.fillRect(140, 107, 9, -finalreadings[10], GxEPD_BLACK);
      display.fillRect(154, 107, 9, -finalreadings[11], GxEPD_BLACK);
      display.fillRect(168, 107, 9, -finalreadings[12], GxEPD_BLACK);
      display.fillRect(182, 107, 9, -finalreadings[13], GxEPD_BLACK);
      display.fillRect(195, 107, 9, -finalreadings[14], GxEPD_BLACK);
      display.fillRect(209, 107, 9, -finalreadings[15], GxEPD_BLACK);
      display.fillRect(223, 107, 9, -finalreadings[16], GxEPD_BLACK);
      display.fillRect(237, 107, 9, -finalreadings[17], GxEPD_BLACK);
    }
    else{ //for AS7341 (10 channels)
      display.drawBitmap(2, 37, base10, 246, 90, GxEPD_BLACK);
      
      for(uint8_t i = 0; i < 10; i++){
        display.fillRect((3+(i*25)), 107, 19, -finalreadings[i], GxEPD_BLACK);
      }
    }

    //Draw measurement mode
    display.setFont(); //default 5x7 font
    display.setTextColor(GxEPD_BLACK);
    if(sensemode == 0){
      display.setCursor(181, 10);
      display.print("Single Fire");
    }
    else if(sensemode == 1){
      display.setCursor(187, 10);
      display.print("Continuous");
    }
    else{
      display.setCursor(205, 10);
      display.print("Burst ");
      display.print(sensemode);
    }
    
    //draw led mode
    if(ledmode == 0){
      display.setCursor(205, 23);
      display.print("No LEDs");
    }
    else if(ledmode == 1){
      display.setCursor(169, 23);
      display.print("Internal LEDs");
    }
    else if(ledmode == 2){
      display.setCursor(169, 23);
      display.print("External LEDs");
    }
    else if(ledmode == 3){
      display.setCursor(199, 23);
      display.print("All LEDs");
    }
    else{
      display.setCursor(175, 23);
      display.print("Invalid Mode");
    }

    //Draw Ripeness Scale
    display.drawBitmap(3, 8, ripescale, 170, 16, GxEPD_BLACK); //scale with labels
    display.drawBitmap(ripeness, 25, arrow, 7, 10, GxEPD_BLACK); //arrow

  } while (display.nextPage()); // Send page buffer & check if more pages
}

/*
Input Handlers
*/
void checkPosition() //called upon whenever pin changes on encoder
{
  encoder->tick(); //call tick() to check the state.
  newpos = encoder->getPosition();
  if (pos != newpos) {
      if ((int)(encoder->getDirection()) > 0 && sensemode < 10){
        sensemode++;
      }
      else if((int)(encoder->getDirection()) < 0 && sensemode > 0){
        sensemode--;
      }
      drawMain(false, "Ready", intreadings);
  }
}

void handleBTNINT() {
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceDelay) {
    // Toggle LED for debug purposes
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState);

    buttonpress = true; // Set the flag for the main loop
    lastInterruptTime = interruptTime;
  }
}
