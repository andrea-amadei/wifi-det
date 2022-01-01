# WiFi and Microwaves Detector

**Version: 3.1**<br>
This is an **Arduino-based project**.

The goal of this project is to implement a micro **Wifi and Microwaves detector** with Arduino.

## Required components
  - Arduino Nano
  - U8GLIB SSD1306 OLED Display (128x64 resolution)
  - NRF24 Mini SMD Wireless Module
  - Generic Dimmer module
  - 4.5V Battery and killswitch (Arduino could also be powered via USB)

## Functioning
The WiFi detector can work in two operational modes:
  1. **General mode**: shows an histogram of the power recieved by the WiFi antenna on every channel
  3. **Channel specific mode**: shows a graph of the evolution in time of the power receieved by the WiFi antenna on the selected channel

To toggle between modes or choose the specific channel when in *Channel specific mode*, simply rotate the dimmer until the requested mode appears on screen.
