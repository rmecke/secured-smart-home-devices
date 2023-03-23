# About this repository
This repository contains the code basis for the self built smart home devices.
Every "mqtt_*" folder contains the code for one device. 
Each device can act as sensor and/or actor depending on it's use case.
The devices itself shouldn't contain logic to control other smart home devices,
instead the sensors just collect and maybe count inputs and send the final retrieved data to iobroker.

## Usage
The ESP-8266 is being programmed using the Arduino development environment, therefore every "mqtt_*" folder represents one project.
Since every device needs to communicate with the ioBroker, we use MQTT as communication protocol.

Therefore the SSID and password of the home WLAN needs to be specified in every file. Also the IP of the iobroker device needs to be configured.
Ensure in your router options, that this iobroker IP is fixed. Otherwise the devices won't be able to communicate with iobroker.