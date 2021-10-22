/*******************************************************************************
* RoboCore - Supply Voltage (v1.0)
* 
* Read the voltage of the power supply.
* 
* Copyright 2021 RoboCore.
* Written by Francois (07/10/21).
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

VespaBattery vbat;

// --------------------------------------------------

void setup() {
  Serial.begin(115200);
}

// --------------------------------------------------

void loop() {
  Serial.print("Battery voltage: ");
  Serial.print(vbat.readVoltage());
  Serial.println(" mV");

  delay(1000);
}

// --------------------------------------------------
