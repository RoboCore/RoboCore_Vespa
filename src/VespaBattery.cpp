/*******************************************************************************
* RoboCore Vespa Battery Library (v1.1)
* 
* Library to read the battery voltage of the Vespa board.
* 
* Copyright 2021 RoboCore.
* Written by Francois (03/12/21).
* Based on the example from @DaveCalaway (https://github.com/espressif/arduino-esp32/issues/1804)
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

extern "C" {
  #include <soc/sens_reg.h>
}

// --------------------------------------------------
// --------------------------------------------------

// Constructor (default)
VespaBattery::VespaBattery(void) :
  VespaBattery(1100) // call the main constructor (default Vref = 1100 mV)
{
  // nothing to do here
}

// --------------------------------------------------

// Constructor
//  @param (vref) : the reference voltage for the ADC calibration [uint32_t]
VespaBattery::VespaBattery(uint32_t vref) :
  _pin(VESPA_BATTERY_PIN),
  _battery_type(BATTERY_UNDEFINED),
  handler_critical(nullptr)
{
  // configure the pin
  pinMode(this->_pin, INPUT);

  // configure the ADC
  /*
   * 4095 for 12-bits -> VDD_A / 4095 = 805uV  too jittery
   * 2048 for 11-bits -> 3.9 / 2048 = 1.9mV looks fine
   * 
   * The maximum voltage is limited by VDD_A
   *  - 0dB attenuaton (ADC_ATTEN_DB_0) gives full-scale voltage 1.1V
   *  - 2.5dB attenuation (ADC_ATTEN_DB_2_5) gives full-scale voltage 1.5V
   *  - 6dB attenuation (ADC_ATTEN_DB_6) gives full-scale voltage 2.2V
   *  - 11dB attenuation (ADC_ATTEN_DB_11) gives full-scale voltage 3.9V
   */
  adc1_config_width(VESPA_BATTERY_ADC_WIDTH);
  adc1_config_channel_atten(VESPA_BATTERY_ADC_CHANNEL, VESPA_BATTERY_ADC_ATTENUATION);
  
  // create a new characteristics object
  this->_adc_characteristics = new esp_adc_cal_characteristics_t;

  // configure the ADC characteristics
  this->_adc_calibration_type = esp_adc_cal_characterize(VESPA_BATTERY_ADC_UNIT, VESPA_BATTERY_ADC_ATTENUATION, VESPA_BATTERY_ADC_WIDTH, vref, this->_adc_characteristics);
}

// --------------------------------------------------

// Destructor
VespaBattery::~VespaBattery(void){
  // free the characteristics pointer
  delete this->_adc_characteristics;
}

// --------------------------------------------------
// --------------------------------------------------

// Get the calibration type of the ADC
//  @returns the calibration type [uint8_t]
//  Note: from <esp_adc_cal.h>
//        0 > eFuse Vref
//        1 > eFuse Two Point
//        2 > default Vref
//        3 > eFuse Two Point Fit
uint8_t VespaBattery::getCalibrationType(void){
  return static_cast<uint8_t> (this->_adc_calibration_type);
}
// --------------------------------------------------

// Get the reference voltage for the ADC (in mV)
//  @returns the reference voltage [uint32_t]
uint32_t VespaBattery::getReferenceVoltage(void){
  return this->_adc_characteristics->vref;
}

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
  // get the current ADC configuration
  uint32_t adc1_ctrl_register = READ_PERI_REG(SENS_SAR_READ_CTRL_REG);
  uint32_t adc1_bit_width = adc1_ctrl_register & 0x00030000; // get the current bit width
  adc1_bit_width >>= 16; // convert to LSB[0:1]

  // set the ADC to the bit width for the Vespa
  adc1_config_width(VESPA_BATTERY_ADC_WIDTH);

  // read the analog pin
  int32_t value = adc1_get_raw(VESPA_BATTERY_ADC_CHANNEL);

  // Note: the analog pin can be read with <analogRead()>, but it
  //       requires the resolution to be configured accordingly
  //       with <analogReadResolution()>, otherwise the result
  //       can be misinterpreted.

  // check the value read
  if(value < 0){
    return 0;
  }
  
  // Note: although <esp_adc_cal_get_voltage()> would be a better fit, it is best to use <adc1_channel_t>
  //       instead of <adc_channel_t>. therefore <esp_adc_cal_raw_to_voltage()> is used.
  uint32_t voltage = esp_adc_cal_raw_to_voltage(value, this->_adc_characteristics);
  voltage *= VESPA_BATTERY_VOLTAGE_CONVERSION; // convert the value considering the internal circuit
  voltage /= 1000;

  // return the ADC to the previous configuration
  adc1_config_width((adc_bits_width_t)adc1_bit_width);

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
