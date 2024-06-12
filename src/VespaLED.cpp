/*******************************************************************************
* RoboCore Vespa LED Library
* 
* Library to control LED within the Vespa board and external ones.
* 
* Copyright 2024 RoboCore.
* 
* 
* This file is part of the Vespa library by RoboCore ("RoboCore-Vespa-lib").
* 
* "RoboCore-Vespa-lib" is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* "RoboCore-Vespa-lib" is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
* 
* You should have received a copy of the GNU Lesser General Public License
* along with "RoboCore-Vespa-lib". If not, see <https://www.gnu.org/licenses/>
*******************************************************************************/

// --------------------------------------------------
// Libraries

#include "RoboCore_Vespa.h"

// --------------------------------------------------
// --------------------------------------------------

// Constructor (default)
VespaLED::VespaLED(void) :
  VespaLED(VESPA_LED_PIN) // call the main constructor
{
  // nothing to do here
}

// --------------------------------------------------

// Constructor
//  @param (pin) : the pin assigned to the LED [uint8_t]
VespaLED::VespaLED(uint8_t pin) :
  _pin(pin),
  _state(LOW),
  _stop_time(0)
{
  // configure the pin
  pinMode(this->_pin, OUTPUT);
  digitalWrite(this->_pin, this->_state);
}

// --------------------------------------------------

// Destructor
VespaLED::~VespaLED(void){
  // set the pin as input
  pinMode(this->_pin, INPUT);
}

// --------------------------------------------------
// --------------------------------------------------

// Set the LED to blink
//  @param (duration) : the delay for the blink [ms] [uint32_t]
//  Note: the method <update()> must be called to check and toggle the state of the pin.
void VespaLED::blink(uint32_t duration){
  this->_delay = duration;

  if (this->_delay == 0){
    this->_stop_time = 0; // reset
  } else {
    this->_stop_time = millis() + this->_delay;
  }
}

// --------------------------------------------------

// Turn the LED on
void VespaLED::on(void){
  this->_stop_time = 0; // reset
  this->_state = HIGH;
  digitalWrite(this->_pin, this->_state);
}

// --------------------------------------------------

// Turn the LED off
void VespaLED::off(void){
  this->_stop_time = 0; // reset
  this->_state = LOW;
  digitalWrite(this->_pin, this->_state);
}

// --------------------------------------------------

// Toggle the state of the LED
void VespaLED::toggle(void){
  if (this->_state == LOW){
    this->on();
  } else {
    this->off();
  }
}

// --------------------------------------------------

// Update the state of the pin (when blinking)
void VespaLED::update(void){
  // check the stop time
  if (this->_stop_time == 0){
    return;
  }

  if (millis() >= this->_stop_time){
    this->toggle(); // update the LED
    this->_stop_time = millis() + this->_delay; // update the stop time
  }
}

// --------------------------------------------------
// --------------------------------------------------
