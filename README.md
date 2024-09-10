# ESP32-LightSwitch
Using an ESP32 to control Govee RGB Lights.

## Description

A simple 4 key macropad that manages my lights.

## Getting Started

### Dependencies

- ESP32 microcontroller
- 4 cherry mx style mechanical switches
- 3D printed Case (LightController-Case.stl)
- Govee API key

### Installing

Open light-controller.ino in Arduino IDE.
Change lines 14-17 to match your scenario.
```
const char *ssid = "wifi-ssid";
const char *password = "wifi-password";

const char *apiKey = "govee-api-key"; // get api key from govee
```
Upload to ESP32.

## Authors

Valentin Thevoz

## License

This project is licensed under the [NAME HERE] License - see the LICENSE.md file for details

## Acknowledgments
The code in this project is modified from Boulama's code in his motion sensor govee esp32 project.
https://www.boulama.com/blog/posts/controlling-govee-devices-with-esp32-motion-sensor.html
Boulama K. (https://github.com/boulama)
