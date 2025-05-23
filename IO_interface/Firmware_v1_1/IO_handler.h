#ifndef IO_HANDLER_H
#define IO_HANDLER_H

#include <Arduino.h>
#include <Wire.h>
#include <RPi_Pico_TimerInterrupt.h>
#include "spectroscopico.h"

static RPI_PICO_Timer button_buffer(0);
static RPI_PICO_Timer encoder_buffer(1);

static const uint16_t      BUT_BUFFER_TIME = 20;
static const uint16_t      ENC_BUFFER_TIME = 20;

static const uint8_t BTN_PIN   = 15;
static const uint8_t ENC_PIN_A = 3;
static const uint8_t ENC_PIN_B = 5;
static const uint8_t ENC_BTN   = 4;

static volatile bool cont_flag = false;

//----------------------------------------------------------------------------------------------------//
// FUNCTIONS
//----------------------------------------------------------------------------------------------------//
void beginIO();

void readSensor(bool print);
bool checkFlag();

bool buttonBuffer(struct repeating_timer *t);
void buttonInterrupt();

bool encoderBuffer(struct repeating_timer *t);
void encoderInterrupt();

bool continuousTimer(struct repeating_timer *t);

#endif