/*******************************************************************************
* RoboCore Vespa Button Library
* 
* Library to read the button within the Vespa board and external ones.
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
VespaButton::VespaButton(void) :
  VespaButton(VESPA_BUTTON_PIN, INPUT) // call the main constructor
{
  // nothing to do here
}

// --------------------------------------------------

// Constructor
//  @param (pin) : the pin assigned to the button [uint8_t]
VespaButton::VespaButton(uint8_t pin, uint8_t mode) :
  _pin(pin),
  _active_mode(LOW),
  _debounce(20),
  on_change(nullptr)
{
  if ((mode != INPUT) && (mode != INPUT_PULLUP)){
    mode = INPUT; // force a valid mode
  }

  // configure the pin
  pinMode(this->_pin, mode);
  this->_last_state = (digitalRead(this->_pin) == this->_active_mode) ? true : false;
}

// --------------------------------------------------

// Destructor
VespaButton::~VespaButton(void){
  // nothing to do here
}

// --------------------------------------------------
// --------------------------------------------------

// Check if the button is pressed
bool VespaButton::pressed(void){
  uint8_t state = digitalRead(this->_pin);
  delay(this->_debounce); // debounce
  if (state == digitalRead(this->_pin)){
    bool res = (state == this->_active_mode) ? true : false;

    if (res != this->_last_state){
      if (this->on_change != nullptr){
        this->on_change(res);
      }
    }

    this->_last_state = res;
  }

  return this->_last_state;
}

// --------------------------------------------------

// Set the active mode
//  @param (mode) : HIGH or LOW [uint8_t]
//  @returns false if an invalid mode was given
bool VespaButton::setActiveMode(uint8_t mode){
  if ((mode != LOW) && (mode != HIGH)){
    return false;
  }

  this->_active_mode = mode;

  return true;
}

// --------------------------------------------------

// Set the debounce for the reading
//  @param (debounce) : the debounce [ms] [uint16_t]
void VespaButton::setDebounce(uint16_t debounce){
  this->_debounce = debounce;
}

// --------------------------------------------------
// --------------------------------------------------
