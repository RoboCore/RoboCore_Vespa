#ifndef VESPA_H
#define VESPA_H

/*******************************************************************************
* RoboCore - Vespa Library (v1.1)
* 
* Library to use the functions of the Vespa board.
* 
* Copyright 2021 RoboCore.
* Written by Francois (08/10/21).
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
* 
* Versions
*   v1.1  - Updated <VespaBattery::readVoltage()> to always use the bit width
*           defined in the library and to not interfere in external analog
*           readings (<analogRead()> uses 12 bits by default).
*   v1.0  - Based on the Arduino implementation of the ESP32.
*           ( https://github.com/espressif/arduino-esp32/blob/11f89cddf6f331bde53c09b2edf6107952867269/cores/esp32/esp32-hal-ledc.h )
*         - There is no option to change the PWM channels, but it might be
*           useful to update them without needing to recompile.
*         - By default, channels 14 and 15 are used so that there is a lesser
*           chance of collision with other libraries (e.g. Servo).
*         - By default, the servos use channel 13 or lower to decrease the chance
*           of collision with other libraries.
*         - Only four servos can be used simultaneously at the moment (S1 to S4).
*         - The ADC1 is configured internally for reading the pin 34 (Vbat).
*           This means that any changes in the library or externally (user code)
*           might interfere with the reading within the library.
*
*******************************************************************************/

#if !defined(ARDUINO_ESP32_DEV) // ESP32
#error Use this library with the ESP32
#endif

// --------------------------------------------------
// Libraries

#include <Arduino.h>

extern "C" {
  #include <stdarg.h>
  #include <stdint.h>
  #include <stdlib.h>
  
  #include <esp_adc_cal.h>
  #include <esp32-hal-ledc.h>
}

// --------------------------------------------------
// Macros

#define VESPA_BATTERY_ADC_ATTENUATION (ADC_ATTEN_DB_11)
#define VESPA_BATTERY_ADC_CHANNEL (ADC1_CHANNEL_6)
#define VESPA_BATTERY_ADC_UNIT (ADC_UNIT_1)
#define VESPA_BATTERY_ADC_WIDTH (ADC_WIDTH_BIT_11)
#define VESPA_BATTERY_PIN (34)
#define VESPA_BATTERY_VOLTAGE_CONVERSION (5702) // Vin = Vout * (R1+R2)/R2

#define VESPA_MOTORS_CHANNEL_A (14)
#define VESPA_MOTORS_CHANNEL_B (15)

#define VESPA_SERVO_DEFAULT_CHANNEL (13)
#define VESPA_SERVO_PULSE_WIDTH_DEFAULT_MAX (2000) // [us]
#define VESPA_SERVO_PULSE_WIDTH_DEFAULT_MIN (1000) // [us]
#define VESPA_SERVO_PULSE_WIDTH_MAX (2500) // [us]
#define VESPA_SERVO_PULSE_WIDTH_MIN (500) // [us]
#define VESPA_SERVO_QTY (4)

// helper macros
#define VESPA_SERVO_S1 (26)
#define VESPA_SERVO_S2 (25)
#define VESPA_SERVO_S3 (33)
#define VESPA_SERVO_S4 (32)

// --------------------------------------------------
// Enumerators

enum BatteryType : uint8_t {
  BATTERY_UNDEFINED = 0, 
  BATTERY_LIPO
};

// --------------------------------------------------
// Class - Vespa Battery

class VespaBattery {
  public:
    void (*handler_critical)(uint8_t); // critical voltage (capacity)
    VespaBattery(void);
    VespaBattery(uint32_t);
    ~VespaBattery(void);
    uint8_t getCalibrationType(void);
    uint32_t getReferenceVoltage(void);
    uint8_t readCapacity(void);
    uint32_t readVoltage(void);
    bool setBatteryType(uint8_t);

  private:
    esp_adc_cal_characteristics_t *_adc_characteristics;
    esp_adc_cal_value_t _adc_calibration_type;
    uint8_t _pin;
    uint8_t _battery_type;
};

// --------------------------------------------------
// Class - Vespa Motors

class VespaMotors {
  public:
    VespaMotors(void);
    ~VespaMotors(void);
    void backward(uint8_t);
    void forward(uint8_t);
    void setSpeedLeft(int8_t);
    void setSpeedRight(int8_t);
    void stop(void);
    void turn(int8_t, int8_t);

    const static uint8_t FORWARD = HIGH;  // MA1 & MB1
    const static uint8_t BACKWARD = LOW; // MA2 & MB2 (opposite of FORWARD)

  private:
    uint8_t _pinMA1, _pinMA2, _pinMB1, _pinMB2;
    uint8_t *_active_pin_A, *_active_pin_B;
    uint16_t _pwmA, _pwmB;
    uint8_t _pwm_channel_A, _pwm_channel_B;
    double _pwm_frequency; // [Hz]
    uint8_t _pwm_resolution;
    uint16_t _max_duty_cyle;

    void _attachPin(uint8_t *);
    void _setDirectionLeft(uint8_t);
    void _setDirectionRight(uint8_t);
    void _configurePWM(void);
};

// --------------------------------------------------
// Class - Vespa Servo

class VespaServo {
  public:
    VespaServo(void);
    VespaServo(uint8_t);
    ~VespaServo(void);
    bool attach(uint8_t);
    bool attach(uint8_t, uint16_t, uint16_t);
    bool attached(void);
    void detach(void);
    uint8_t getChannel(void);
    void write(uint16_t);
    
  private:
    static uint8_t _servo_count;
    static VespaServo *_servos[];

    uint8_t _pin;
    uint16_t _max, _min; // [us]
    uint8_t _pwm_channel;
    double _pwm_frequency; // [Hz]
    uint8_t _pwm_resolution;
    uint16_t _max_duty_cyle;
};

// --------------------------------------------------

#endif // VESPA_H
