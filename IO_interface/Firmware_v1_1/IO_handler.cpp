#include "IO_handler.h"

//----------------------------------------------------------------------------------------------------//
// Set-Up & Read Function
//----------------------------------------------------------------------------------------------------//
void beginIO()
{
  //Pin Definition
  pinMode(BTN_PIN, INPUT_PULLUP);

  pinMode(ENC_PIN_A,INPUT_PULLUP);
  pinMode(ENC_PIN_B,INPUT_PULLUP);
  pinMode(ENC_BTN,INPUT_PULLUP);

  //Buffer Timer Definition
  button_buffer.attachInterrupt(BUT_BUFFER_TIME, buttonBuffer);
  button_buffer.disableTimer();
  encoder_buffer.attachInterrupt(ENC_BUFFER_TIME, encoderBuffer);
  encoder_buffer.disableTimer();

  //Interrupt Definition
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), encoderInterrupt, CHANGE);
  
}

void readSensor(bool print)
{
  if(measuring)
  {
    measuring=false;
  }
  else
  {
    measuring=true;
    if(print){bigText(false, "Measuring...");}
    multimeasure();
  }
}

bool checkFlag()
{
  return cont_flag;
}

//----------------------------------------------------------------------------------------------------//
// Button Interrupt & Timer Handlers
//----------------------------------------------------------------------------------------------------//
void buttonInterrupt()
{
  //disable the interrupt
  detachInterrupt(digitalPinToInterrupt(BTN_PIN));

  //trigger the timer
  button_buffer.enableTimer();
}

bool buttonBuffer(struct repeating_timer *t)
{
  //read one time, then toggle the continuous flag if in continuous mode
  if(!digitalRead(BTN_PIN))
  {
    if(sensemode)
    {
      cont_flag = !cont_flag;
      cont_flag_draw = cont_flag;
      if(!cont_flag){
        if(sensecon == 2){drawMain(false, detectColour10(), intreadings);}
        else{drawMain(false, detectColour18(), intreadings);}
        return true;
      }
    }
    readSensor(true);
  }
  
  //re-enable int & disable buffer
  attachInterrupt(digitalPinToInterrupt(BTN_PIN), buttonInterrupt, CHANGE);
  button_buffer.disableTimer();
  return true;
}

//----------------------------------------------------------------------------------------------------//
// Encoder Interrupt & Timer Handlers
//----------------------------------------------------------------------------------------------------//
void encoderInterrupt()
{
  //disable the interrupt
  detachInterrupt(digitalPinToInterrupt(ENC_PIN_A));

  //trigger the timer
  encoder_buffer.enableTimer();
}

bool encoderBuffer(struct repeating_timer *t)
{
  //PROCESS
  if(!digitalRead(ENC_BTN))
  {
    if(ledmode==3){ledmode=0;}
    else{ledmode++;}
  }
  else
  {
    sensemode = !sensemode;
    if(!sensemode){cont_flag=false;cont_flag_draw = cont_flag;}
  }
  
  if(sensecon == 2){
    drawMain(false, detectColour10(), intreadings);
  }
  else{
    drawMain(false, detectColour18(), intreadings);
  }

  //re-enable int & disable buffer
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), encoderInterrupt, CHANGE);
  encoder_buffer.disableTimer();
  return true;
}