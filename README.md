# IoTFanControlerW5000
IOT Fan Controller based on RP2040 W5000 board from WizNet([WizNet W5100S Rp2040](https://www.wiznet.io/product-item/w5100s-evb-pico/))

Controls PWM fan based on environment temperature read through a DS18B20 sensor. Uses three presets to select fan speed, and a night time override speed to reduce night time noise.

Device is based on my TwinThing framework (see [twinThingRP2040W5x00](https://github.com/jondurrant/twinThingRP2040W5x00) and [TwinThingPyMQTT](https://github.com/jondurrant/TwinThingPyMQTT)). The device does not persist status or configuration in flash memory but instead publishes its state to a MQTT shadow service, which can update the device to current configuration when it attaches to the network.

# Hardware
+ [WizNet W5100S Rp2040](https://www.wiznet.io/product-item/w5100s-evb-pico/)
+ RGB Common Cathode LED
+ 3x Resistors 91R
+ DS20B18 temperature sensor
+ Resistor 4.7K
+ RotaryEncoder with Push Switch
+ 2x 10nF Capacitor for smoothing
+ OLED display 128x32 SSD1306
+ 2x 80mm 12V PWM controllable fans

# Dependencies
## C++
+ [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel): Task management
+ [FreeRTOS-CoreMQTT-Agent](https://www.freertos.org/mqtt-agent/index.html): MQTT Library
+ [ioLibary_Driver](https://github.com/Wiznet/ioLibrary_Driver): Drivers for W5100s
+ [twinThingRP2040W5x00](https://github.com/jondurrant/twinThingRP2040W5x00)
+ [json-maker](https://github.com/rafagafe/json-maker): JSON writing for comms
+ [tiny-json](https://github.com/rafagafe/tiny-json): JSON parser
+ [RP2040PIO-DS18B20](https://github.com/jondurrant/RP2040PIO-DS18B20): Sensor library
+ [pico-ssd1306](https://github.com/daschr/pico-ssd1306): Oled Driver
+ [RtosComponents](https://github.com/jondurrant/RtosComponents): Watchdog module


## MQTT hub
+ [EMQX](https://www.emqx.io/)

## MQTT Services
+ [TwinManager](https://github.com/jondurrant/TwinThingPyMQTT)

# Folder Structur

+ src - source code for the project
+ lib - libraries used by the project
+ port - port code needed for the libraries used by the project
+ patches - patch files produced by WizNet for the ioLibrary_Driver lib
+ py - test python to interact with the IOT devices
+ schematic - Circuit schematic (no PCB design as build on prototype board)
+ 3dModels - STL files for PCB bracket, fan cover, OLED housing and Rotary Encoder housing

# Clone and Build
## Clone
'''
git clone --recurse-submodules https://github.com/jondurrant/IoTFanControlerW5000
'''

## Build
'''
mkdir build
cd build
cmake ..
make
'''
