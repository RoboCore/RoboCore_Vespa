/*******************************************************************************
* RoboCore Vespa Motors Library (v1.1)
* 
* Library to use the motors of the Vespa board.
* 
* Copyright 2021 RoboCore.
* Written by Francois (25/10/21).
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

// Constructor
VespaMotors::VespaMotors(void) :
  _pinMA1(13),
  _pinMA2(14),
  _pinMB1(27),
  _pinMB2(4),
  _pwm_channel_A(VESPA_MOTORS_CHANNEL_A),
  _pwm_channel_B(VESPA_MOTORS_CHANNEL_B),
  _pwm_frequency(5000), // 5 kHz
  _pwm_resolution(10)  // 10 bits
{
  // configure the pins
  pinMode(this->_pinMA1, OUTPUT);
  pinMode(this->_pinMA2, OUTPUT);
  pinMode(this->_pinMB1, OUTPUT);
  pinMode(this->_pinMB2, OUTPUT);

  // turn all channels off
  digitalWrite(this->_pinMA1, LOW);
  digitalWrite(this->_pinMA2, LOW);
  digitalWrite(this->_pinMB1, LOW);
  digitalWrite(this->_pinMB2, LOW);
  
  // configure the PWM
  this->_configurePWM();

  // default to stopped
  this->stop();
}

// --------------------------------------------------

// Destructor
VespaMotors::~VespaMotors(void){
  // detach the pins from the PWM
  ledcDetachPin(*this->_active_pin_A);
  ledcDetachPin(*this->_active_pin_B);

  // set all pins as inputs
  pinMode(this->_pinMA1, INPUT);
  pinMode(this->_pinMA2, INPUT);
  pinMode(this->_pinMB1, INPUT);
  pinMode(this->_pinMB2, INPUT);
}

// --------------------------------------------------
// --------------------------------------------------

// Set the motors to move backwards
//  @param (speed) : the speed of the motor (0-100) [uint8_t]
void VespaMotors::backward(uint8_t speed){
  // constrain the value
  if(speed > 100){
    speed = 100;
  }

  // update the directions
  this->_attachPin(&this->_pinMA2);
  this->_attachPin(&this->_pinMB2);

  this->_pwmA = map(speed, 0, 100, 0, this->_max_duty_cyle); // transform to the current configuration
  this->_pwmB = this->_pwmA;
  ledcWrite(this->_pwm_channel_A, this->_pwmA); // update
  ledcWrite(this->_pwm_channel_B, this->_pwmB); // update
}

// --------------------------------------------------

// Set the motors to move forwards
//  @param (speed) : the speed of the motor (0-100%) [uint8_t]
void VespaMotors::forward(uint8_t speed){
  // constrain the value
  if(speed > 100){
    speed = 100;
  }

  // update the directions
  this->_attachPin(&this->_pinMA1);
  this->_attachPin(&this->_pinMB1);

  this->_pwmA = map(speed, 0, 100, 0, this->_max_duty_cyle); // transform to the current configuration
  this->_pwmB = this->_pwmA;
  ledcWrite(this->_pwm_channel_A, this->_pwmA); // update
  ledcWrite(this->_pwm_channel_B, this->_pwmB); // update
}

// --------------------------------------------------

// Set the left motor speed
//  @param (speed) : the speed of the motor (-100-100%) [int8_t]
void VespaMotors::setSpeedLeft(int8_t speed){
  // update the directions
  if(speed >= 0){
    this->_setDirectionLeft(VespaMotors::FORWARD);
  } else {
    speed *= -1; // update
    this->_setDirectionLeft(VespaMotors::BACKWARD);
  }
  
  // constrain the value
  if(speed > 100){
    speed = 100;
  }

  this->_pwmA = map(speed, 0, 100, 0, this->_max_duty_cyle); // transform to the current configuration
  ledcWrite(this->_pwm_channel_A, this->_pwmA); // update
}

// --------------------------------------------------

// Set the right motor speed
//  @param (speed) : the speed of the motor (-100-100%) [int8_t]
void VespaMotors::setSpeedRight(int8_t speed){
  // update the directions
  if(speed >= 0){
    this->_setDirectionRight(VespaMotors::FORWARD);
  } else {
    speed *= -1; // update
    this->_setDirectionRight(VespaMotors::BACKWARD);
  }
  
  // constrain the value
  if(speed > 100){
    speed = 100;
  }

  this->_pwmB = map(speed, 0, 100, 0, this->_max_duty_cyle); // transform to the current configuration
  ledcWrite(this->_pwm_channel_B, this->_pwmB); // update
}

// --------------------------------------------------

// Stop both motors
void VespaMotors::stop(void){
  this->_pwmA = 0; // reset
  this->_pwmB = 0; // reset

  ledcWrite(this->_pwm_channel_A, this->_pwmA); // update
  ledcWrite(this->_pwm_channel_B, this->_pwmB); // update
}

// --------------------------------------------------

// Set the motors to turn
//  @param (speedA) : the speed of the left motor (-100-100%) [int8_t]
//         (speedB) : the speed of the right motor (-100-100%) [int8_t]
//  Note: a negative value sets the motor to move backwards
void VespaMotors::turn(int8_t speedA, int8_t speedB){
  // update both speeds (the values and the directions are automatically constrained)
  this->setSpeedLeft(speedA);
  this->setSpeedRight(speedB);
}

// --------------------------------------------------
// --------------------------------------------------

// Attach a pin to the active PWM channel
//  @param (pin) : the new pin to attach [uint8_t *]
//  Note: the current active pin is then set to LOW.
void VespaMotors::_attachPin(uint8_t * pin){
  // check if is already the active pin
  if((pin == this->_active_pin_A) || (pin == this->_active_pin_B)){
    return;
  }

  // motor A
  if((pin == &this->_pinMA1) || (pin == &this->_pinMA2)){
    ledcDetachPin(*this->_active_pin_A);
    digitalWrite(*this->_active_pin_A, LOW);
    this->_active_pin_A = pin;
    ledcAttachPin(*this->_active_pin_A, this->_pwm_channel_A);
  }
  // motor B
  if((pin == &this->_pinMB1) || (pin == &this->_pinMB2)){
    ledcDetachPin(*this->_active_pin_B);
    digitalWrite(*this->_active_pin_B, LOW);
    this->_active_pin_B = pin;
    ledcAttachPin(*this->_active_pin_B, this->_pwm_channel_B);
  }
}

// --------------------------------------------------

// Set the direction of the left motor
//  @param (direction) : the direction of the motor (0-1) [uint8_t]
void VespaMotors::_setDirectionLeft(uint8_t direction){
  if(direction == VespaMotors::FORWARD){
    this->_attachPin(&this->_pinMA1);
  } else {
    this->_attachPin(&this->_pinMA2);
  }
}

// --------------------------------------------------

// Set the direction of the right motor
//  @param (direction) : the direction of the motor (0-1) [uint8_t]
void VespaMotors::_setDirectionRight(uint8_t direction){
  if(direction == VespaMotors::FORWARD){
    this->_attachPin(&this->_pinMB1);
  } else {
    this->_attachPin(&this->_pinMB2);
  }
}

// --------------------------------------------------

// Configure the PWM channels
void VespaMotors::_configurePWM(void){
  // configure the channels and timers (use the same frequency)
  this->_pwm_frequency = ledcSetup(this->_pwm_channel_A, this->_pwm_frequency, this->_pwm_resolution);
  this->_pwm_frequency = ledcSetup(this->_pwm_channel_B, this->_pwm_frequency, this->_pwm_resolution);

  // calculate the maximum duty cycle
  this->_max_duty_cyle = (uint16_t)(pow(2, this->_pwm_resolution) - 1);

  // attach the pins
  this->_active_pin_A = &this->_pinMA1;
  this->_active_pin_B = &this->_pinMB1;
  ledcAttachPin(*this->_active_pin_A, this->_pwm_channel_A);
  ledcAttachPin(*this->_active_pin_B, this->_pwm_channel_B);
}

// --------------------------------------------------
// --------------------------------------------------
