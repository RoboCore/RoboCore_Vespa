/*******************************************************************************
* RoboCore Vespa Servo Library (v1.1)
* 
* Library to use servo motors with the Vespa board.
* 
* Copyright 2021 RoboCore.
* Written by Francois (22/10/21).
* Based on the library by John K. Bennett (@jkb-git).
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
// Static variables

uint8_t VespaServo::_servo_count = 0; // default
VespaServo *VespaServo::_servos[VESPA_SERVO_QTY]; // create the array

// --------------------------------------------------
// --------------------------------------------------

// Constructor (default)
VespaServo::VespaServo(void) :
  VespaServo(VESPA_SERVO_DEFAULT_CHANNEL) // call the main constructor
{
  // nothing to do here
}

// --------------------------------------------------

// Constructor
//  @param (channel) : the LEDC channel to use [uint8_t]
VespaServo::VespaServo(uint8_t channel) :
  _pin(0xFF),
  _pwm_frequency(50), // 50 Hz -> 20 ms
  _pwm_resolution(10)  // 10 bits
{ 
  // set the default values if first time
  if(_servo_count == 0){
    for(uint8_t i=0 ; i < VESPA_SERVO_QTY ; i++){
      _servos[i] = nullptr; // default to null pointer
    }
  }

  // check if max servos
  if(_servo_count >= VESPA_SERVO_QTY){
    this->_pwm_channel = 0xFF; // no channel
    return;
  }

  // check the given channel
  if(channel > 13){ // with the Vespa, channels 14 and 15 are not available
    this->_pwm_channel = 0xFF; // no channel
    return;
  }

  // find a channel to use
  for(uint8_t i=0 ; i < VESPA_SERVO_QTY ; i++){
    // find an empty servo slot
    if(_servos[i] == nullptr){
      this->_pwm_channel = channel; // set the channel
      _servos[i] = this; // add the servo to the list
      _servo_count++; // update the count
      break; // exit
    } else {
      // update the channel if already in use
      if(_servos[i]->_pwm_channel == channel){
        if(channel > 0){
          channel--; // update
        } else { // reached the maximum
          this->_pwm_channel = 0xFF; // no channel
          return;
        }
      }
    }
  }
}

// --------------------------------------------------

// Destructor
VespaServo::~VespaServo(void){
  // detach the servo
  this->detach();
  
  // remove the servo from list
  for(uint8_t i=0 ; i < VESPA_SERVO_QTY ; i++){
    if(_servos[i] == this){
      _servos[i] = nullptr; // reset
      break; // exit
    }
  }
  _servo_count--; // update
}

// --------------------------------------------------
// --------------------------------------------------

// Attach a pin to the servo
//  @param (pin) : the pin to attach [uint8_t]
//  @returns true if the pin was attached [bool]
bool VespaServo::attach(uint8_t pin){
  return this->attach(pin, VESPA_SERVO_PULSE_WIDTH_DEFAULT_MIN, VESPA_SERVO_PULSE_WIDTH_DEFAULT_MAX);
}

// --------------------------------------------------

// Attach a pin to the servo
//  @param (pin) : the pin to attach [uint8_t]
//         (min) : the minimum pulse width in [us] [uint16_t]
//         (max) : the maximum pulse width in [us] [uint16_t]
//  @returns true if the pin was attached [bool]
bool VespaServo::attach(uint8_t pin, uint16_t min, uint16_t max){
  // check if the servo is already attached
  if(this->attached()){
    return false;
  }

  // check if valid channel
  if(this->_pwm_channel == 0xFF){
    return false;
  }

  // check if valid pin
  uint8_t pins[] = { 25, 26, 32, 33, 5, 16, 17, 18, 19, 21, 22, 23 }; // available pins for the Vespa board
  size_t pin_count = sizeof(pins) / sizeof(uint8_t);
  bool valid = false; // default
  for(uint8_t i=0 ; i < pin_count ; i++){
    if(pins[i] == pin){
      valid = true; // set
      this->_pin = pin;
      break; // exit
    }
  }
  if(!valid){
    return false;
  }

  // verify and set the minimum and maximum pulse values
  this->_min = (min < VESPA_SERVO_PULSE_WIDTH_MIN) ? VESPA_SERVO_PULSE_WIDTH_MIN : min;
  this->_max = (max > VESPA_SERVO_PULSE_WIDTH_MAX) ? VESPA_SERVO_PULSE_WIDTH_MAX : max;
  
  // configure the LEDC driver
  this->_pwm_frequency = ledcSetup(this->_pwm_channel, this->_pwm_frequency, this->_pwm_resolution); // configure the channels and timers
  this->_max_duty_cyle = (uint16_t)(pow(2, this->_pwm_resolution) - 1); // calculate the maximum duty cycle
  ledcAttachPin(this->_pin, this->_pwm_channel); // attach the pin
  this->write(90); // set the default position (90 degrees)
}

// --------------------------------------------------

// Check if the servo is attached to a pin
//  @returns true if attached [bool]
bool VespaServo::attached(void){
  return (this->_pin == 0xFF) ? false : true;
}

// --------------------------------------------------

// Detach the current servo
void VespaServo::detach(void){
  // check if the servo is attached to a pin
  if(this->attached()){
    ledcDetachPin(this->_pin); // detach the pin from the LEDC driver
    pinMode(this->_pin, INPUT); // set the pin as input
    this->_pin = 0xFF; // reset
  }
}
// --------------------------------------------------

// Get the associated channel
//  @returns the associated channel [uint8_t] (default 0xFF)
uint8_t VespaServo::getChannel(void){
  return this->_pwm_channel;
}
// --------------------------------------------------

// Write a value to the servo
//  @param (value) : the value to write in [degrees or us] [uint16_t]
void VespaServo::write(uint16_t value){
  // check if the servo is attached
  if(!this->attached()){
    return; // exit
  }

  // check if given in degrees
  if(value < VESPA_SERVO_PULSE_WIDTH_MIN){
    value = map(value, 0, 180, this->_min, this->_max); // map to [us]
  }
  
  // check the limits
  if(value > this->_max){
    value = this->_max;
  }
  if(value < this->_min){
    value = this->_min;
  }

  // update the value to ticks
  double ticks = 1000000.0 / this->_pwm_frequency; // get the period [us]
  ticks /= value; // proportional value [1]
  ticks = this->_max_duty_cyle / ticks; // final value [1 = ticks]
  
  // update the duty cycle
  ledcWrite(this->_pwm_channel, (uint32_t)ticks);
}
    
// --------------------------------------------------
// --------------------------------------------------
