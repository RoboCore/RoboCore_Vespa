RoboCore Vespa Arduino Library
==============================

Arduino library for the [*RoboCore Vespa v1.0*](https://www.robocore.net/loja/produtos/1439).


This file is part of the Vespa library by RoboCore ("RoboCore-Vespa-lib").

Change log
----------

**v1.3**
* Contributors: @Francois.
* Updated to be compatible with the Arduino ESP release 3.0.1.
	* ( https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html )
	* `ledcSetup()` and `ledcAttachPin()` replaced by `ledcAttach()`.
	* `ledcDetachPin()` replaced by `ledcDetach()`.
	* *For the previous version of the Arduino ESP package (v2.0.x), use the Vespa library v1.2.*
* `VespaBattery`
	* Updated to use the Arduino ESP package instead of the IDF functions.
	* Removed the constructor `VespaBattery(uint32_t)`.
* `VespaMotors`
	* `_attachPin()` now returns a boolean.
	* `_configurePWM()` now returns a boolean.
* `VespaServo`
	* `VespaServo(channel)` was removed because there was no more use for it.
	* `getChannel()` was removed because there was no more use for it.
	* Added `_attached` for a better handling of the servo.
	* The channel is now selected automatically when using the LEDC API.
		* *It seemed correct during the tests, but it might be better to define the channels in the library (see v1.2).*
* In `VespaMotors` and `VespaServo`, for all LEDC functions, input parameter `channel` has been changed to `pin`.
* Added `VespaButton` to the library, to read the button of the board and external ones as well.
	* Updated the example `Button` to use the class `VespaButton`.
* Added `VespaLED` to the library, to control the LED of the board and external ones as well.
	* Added the example `Blink_Vespa` to show how to use the class `VespaLED`.

**v1.2**
* Contributors: @Francois.
* Updated `VespaServo::attach()` to use by default `PULSE_WIDTH_MAX` and `PULSE_WIDTH_MIN` instead of `PULSE_WIDTH_DEFAULT_xxx`.

**v1.1**
* Contributors: @Francois.
* Updated `VespaBattery::readVoltage()` to always use the bit width defined in the library and to not interfere in external analog readings (`analogRead()` uses 12 bits by default).

**v1.0**
* Contributors: @Francois.
* Based on the Arduino implementation of the ESP32.
	* ( https://github.com/espressif/arduino-esp32/blob/11f89cddf6f331bde53c09b2edf6107952867269/cores/esp32/esp32-hal-ledc.h )
* There is no option to change the PWM channels, but it might be useful to update them without needing to recompile.
* By default, channels 14 and 15 are used so that there is a lesser chance of collision with other libraries (e.g. Servo).
* By default, the servos use channel 13 or lower to decrease the chance of collision with other libraries.
* Only four servos can be used simultaneously at the moment (S1 to S4).
* The ADC1 is configured internally for reading the pin 34 (Vbat). This means that any changes in the library or externally (user code) might interfere with the reading within the library.

