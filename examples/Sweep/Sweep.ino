/*******************************************************************************
* RoboCore - Sweep (v1.0)
* 
* Sweep two servos simultaneously but in opposite directions.
* 
* Copyright 2021 RoboCore.
* Written by Francois (22/10/21).
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
// Variables

VespaServo servo1;
VespaServo servo2;

// --------------------------------------------------

void setup(){
  // Note: some servos might require different MIN and MAX pulses (in [ms])
  //       * the default values are {1000,2000}, the maximum values are {500,2500}
  servo1.attach(VESPA_SERVO_S1); // servo on pin 26, with default min and max
  servo2.attach(VESPA_SERVO_S2, 1000, 2000); // servo on pin 25, with min=1000 and max=2000
  
  delay(3000);
}

// --------------------------------------------------

void loop(){
  for(uint8_t i=0 ; i <= 180 ; i += 10){
    servo1.write(i);
    servo2.write(180-i);
    delay(500);
  }
}

// --------------------------------------------------
