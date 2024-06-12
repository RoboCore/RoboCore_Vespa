/*******************************************************************************
* RoboCore Vespa Battery Library
* 
* Library to read the battery voltage of the Vespa board.
* 
* Copyright 2024 RoboCore.
* [v1.0] Based on the example from @DaveCalaway (https://github.com/espressif/arduino-esp32/issues/1804)
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

// References
//  - https://docs.espressif.com/projects/arduino-esp32/en/latest/api/adc.html
//  - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_oneshot.html
//  - https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc_calibration.html

// --------------------------------------------------
// Libraries

#include "RoboCore_Vespa.h"

// --------------------------------------------------
// --------------------------------------------------

// Constructor (default)
VespaBattery::VespaBattery(void) :
  _pin(VESPA_BATTERY_PIN),
  _battery_type(BATTERY_UNDEFINED),
  handler_critical(nullptr)
{
  // configure the pin
  pinMode(this->_pin, INPUT);

  // configure the ADC
  /*
  * For the ESP32
  *   - ADC_ATTEN_DB_0 gives 100 mV ~ 950 mV
  *   - ADC_ATTEN_DB_2_5 gives 100 mV ~ 1250 mV
  *   - ADC_ATTEN_DB_6 gives 150 mV ~ 1750 mV
  *   - ADC_ATTEN_DB_11 gives 150 mV ~ 3100 mV
  * 
  * Note: 11 db attenuation is deprecated in ESP IDF v5.2.2.
  */
  analogSetPinAttenuation(this->_pin, VESPA_BATTERY_ADC_ATTENUATION);
}

// --------------------------------------------------

// Destructor
VespaBattery::~VespaBattery(void){
  // nothing to do here
}

// --------------------------------------------------
// --------------------------------------------------

// Read the remaining capacity of the battery
//  @returns the remaining capacity (in %) [uint8_t]
uint8_t VespaBattery::readCapacity(void){
  // check for LiPo
  //  (Vnominal = 3,7 V / Vmax = 4,2 V)
  if(this->_battery_type == BATTERY_LIPO){
    // read the voltage
    uint32_t voltage = this->readVoltage();

    // create the lookup table
    //  (from https://blog.ampow.com/lipo-voltage-chart/ )
    uint8_t lookup_size = 21;
    uint32_t lookup[2][lookup_size] = {{  8400,  8300,  8220,  8160,  8050,  7970,  7910,  7830,  7750,  7710,  7670,  7630,  7590,  7570,  7530,  7490,  7450,  7410,  7370,  7220, 6550 },
                                       { 12600, 12450, 12330, 12250, 12070, 11950, 11860, 11740, 11620, 11560, 11510, 11450, 11390, 11360, 11300, 11240, 11180, 11120, 11060, 10830, 9820 }};
                                // [%] {   100,    95,    90,    85,    80,    75,    70,    65,    60,    55,    50,    45,    40,    35,    30,    25,    20,    15,    10,     5,    0  }

    // get the number of cells
    uint8_t cells = (voltage < 9000)? 0 : 1;
    // constrain the voltage
    if(voltage > lookup[cells][0]){
      voltage = lookup[cells][0];
    }
    
    // find the corresponding voltage in the table
    uint8_t index = 0;
    for(uint8_t i=0 ; i < lookup_size ; i++){
      if(lookup[cells][i] >= voltage){
        index = i; // update
      } else {
        break; // exit the loop
      }
    }

    // calculate the percentage
    uint32_t percentage = 100 - index * 5; // raw attribution
    if(index < (lookup_size - 1)){
      uint32_t diff1 = lookup[cells][index] - lookup[cells][index + 1];
      uint32_t diff2 = lookup[cells][index] - voltage;
      uint32_t correction = map(diff2, 0, diff1, 0, 5);
      percentage -= correction; // update
    }

    // check if the voltage is critical
    if(percentage <= 15){
      if(this->handler_critical != nullptr){
        this->handler_critical(percentage); // call the handler
      }
    }

    // return the value calculated
    return percentage;
  }

  // default value (100 %)
  return 100;
}

// --------------------------------------------------

// Read the voltage of the battery (in mV)
//  @returns the voltage of the battery (in mV) [uint32_t]
uint32_t VespaBattery::readVoltage(void){
  // convert the voltage based on the circuit factor
  uint32_t voltage = analogReadMilliVolts(this->_pin);
  voltage *= VESPA_BATTERY_VOLTAGE_CONVERSION;
  voltage /= 1000;

  return voltage;
}

// --------------------------------------------------

// Set the type of the battery
//  @param (type) : the type of the battery [uint8_t]
//  @returns true if a valid type was given [bool]
bool VespaBattery::setBatteryType(uint8_t type){
  // check if the type exists and assign
  if((type >= BATTERY_UNDEFINED) && (type <= BATTERY_LIPO)){
    this->_battery_type = type;
    return true;
  }
  return false;
}

// --------------------------------------------------
// --------------------------------------------------
