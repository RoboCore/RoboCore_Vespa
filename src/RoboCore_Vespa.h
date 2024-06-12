#ifndef VESPA_H
#define VESPA_H

/*******************************************************************************
* RoboCore - Vespa Library (v1.3)
* 
* Library to use the functions of the Vespa board.
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

  #include <esp_arduino_version.h>

  #include <esp32-hal-adc.h>
  #include <esp32-hal-ledc.h>
}

#ifdef ESP_ARDUINO_VERSION_MAJOR
#if ESP_ARDUINO_VERSION_MAJOR < 3
#warning RoboCore Vespa v1.3 is meant to use the Arduino ESP package v3.0+
#endif
#endif

// --------------------------------------------------
// Macros

#define VESPA_VERSION_MAJOR 1 // (X.x.x)
#define VESPA_VERSION_MINOR 3 // (x.X.x)
#define VESPA_VERSION_PATCH 0 // (x.x.X)

#define VESPA_BATTERY_ADC_ATTENUATION (ADC_11db)
#define VESPA_BATTERY_PIN (34)
#define VESPA_BATTERY_VOLTAGE_CONVERSION (5702) // Vin = Vout * (R1+R2)/R2

#define VESPA_BUTTON_PIN (35)

#define VESPA_LED_PIN (15)

#define VESPA_MOTORS_CHANNEL_A (14)
#define VESPA_MOTORS_CHANNEL_B (15)

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
    VespaBattery(void);
    ~VespaBattery(void);
    uint8_t readCapacity(void);
    uint32_t readVoltage(void);
    bool setBatteryType(uint8_t);

    void (*handler_critical)(uint8_t); // critical voltage (capacity)

  private:
    uint8_t _pin;
    uint8_t _battery_type;
};

// --------------------------------------------------
// Class - Vespa Button

class VespaButton {
  public:
    VespaButton(void);
    VespaButton(uint8_t, uint8_t = INPUT);
    ~VespaButton(void);
    bool pressed(void);
    bool setActiveMode(uint8_t);
    void setDebounce(uint16_t);

    void (*on_change)(bool);

  private:
    uint8_t _pin, _active_mode;
    uint16_t _debounce;
    bool _last_state;
};

// --------------------------------------------------
// Class - Vespa LED

class VespaLED {
  public:
    VespaLED(void);
    VespaLED(uint8_t);
    ~VespaLED(void);
    void blink(uint32_t);
    void on(void);
    void off(void);
    void toggle(void);
    void update(void);

  private:
    uint8_t _pin, _state;
    uint32_t _stop_time, _delay;
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

    bool _attachPin(uint8_t *);
    bool _configurePWM(void);
    void _setDirectionLeft(uint8_t);
    void _setDirectionRight(uint8_t);
};

// --------------------------------------------------
// Class - Vespa Servo

class VespaServo {
  public:
    VespaServo(void);
    ~VespaServo(void);
    bool attach(uint8_t);
    bool attach(uint8_t, uint16_t, uint16_t);
    bool attached(void);
    void detach(void);
    void write(uint16_t);
    
  private:
    static uint8_t _servo_count;
    static VespaServo *_servos[];

    bool _attached;
    uint8_t _pin;
    uint16_t _max, _min; // [us]
    double _pwm_frequency; // [Hz]
    uint8_t _pwm_resolution;
    uint16_t _max_duty_cyle;
};

// --------------------------------------------------

#endif // VESPA_H
