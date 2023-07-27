# About this repository
This repository contains the code basis for the self built smart home devices.
Every "mqtt_*" folder contains the code for one device. 
Each device can act as sensor and/or actor depending on it's use case.
The devices itself shouldn't contain logic to control other smart home devices,
instead the sensors just collect and maybe count inputs and send the final retrieved data to iobroker.

## Usage
The ESP-8266 is being programmed using the Arduino development environment, therefore every "mqtt_*" folder represents one project.

Since every device needs to communicate with the ioBroker, we use MQTT as communication protocol. Also configure the mqtt-topics if necessary.

Therefore the SSID and password of the home WLAN needs to be specified in every file. Also the IP of the iobroker device needs to be configured.
Ensure in your router options, that this iobroker IP is fixed. Otherwise the devices won't be able to communicate with iobroker.

## Devices
### RGB-LED
A simple rgb led, which can be used for demo usecases. This project could also be used as template for other projects.

### MQTT-Light-1
A simple relais switch, which can open or close the circuit to light up an AC-lamp. It includes also a touch sensor and counts the interactions.

### MQTT-Light-2
A simple relais switch, which can open or close the circuit to light up an AC-lamp.

### MQTT-Plant
Check the watering status of a plant.

### MQTT-Room-Data
Get some room statistics, like temperature and humidity. It includes also a audio sensor and counts the clap interactions.

## References

Also I would like to mention the following sources, which provided some fritzing resources I could use for visualization of the breadboards:
- https://forum.fritzing.org/t/humidity-and-temperature-sensor-dht11/6307
- https://github.com/adafruit/Fritzing-Library/blob/master/parts/DHT11%20Humitidy%20and%20Temperature%20Sensor.fzpz
- https://github.com/adafruit/Fritzing-Library
- https://github.com/adafruit/DHT-sensor-library
- https://github.com/roman-miniailov/fritzing-parts
- https://forum.fritzing.org/t/part-request-for-this-ac-dc-converter/6795/4
- https://arduinomodules.info/download/ky-019-5v-relay-module-zip-file/
- https://fritzing.org/projects/iot-switch-onoff-220-240v-device-with-nodemcu-5v-r
- https://fritzing.org/projects/capacitive-sensor-with-ttp223b-chip (TTP223)
- https://github.com/OgreTransporter/fritzing-parts-extra (Capacative Moisture)