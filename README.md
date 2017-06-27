# ESP8266DaikinAC
Control Daikin AC unit via IR using ESP8266 - several variants included; http and mqtt communications.

Uses https://github.com/markszabo/IRremoteESP8266 (http version uses a deprecated library build, mqtt version has been written to use more recent library build) 

The MQTT variant supports temperature with DS18B20 One-Wire sensor, and works well with https://github.com/cflurin/homebridge-mqtt and https://github.com/node-red/node-red to be controlled as a Thermostat device




---

Basically a rough and ready copy/pasta of example codes, currently undergoing cleanup. Looking to integrate timer functionality.

Presents the appropriate API for https://github.com/rudders/homebridge-http to switch Daikin AC unit to Heat/Cool/Auto/Off modes via HomeKit using the Homebridge project at https://github.com/nfarina/homebridge

