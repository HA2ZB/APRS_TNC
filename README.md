# APRS_TNC
Arduino Mega 2560 based APRS TNC with MX614 (FX614) modem

This is a very initial stage of the development of an Arduino based APRS TNC. The current functionality connects Android APRSdroid app to the radio audio interface via Bluetooth (HC-05 module). It should obviously work with other serial KISS terminals as well (not tested). I am using the Elegoo Mega 2560 Arduio clone in this project.

Basic functionality is tested, working, but I cannot guarantee the bug free operation of course, use it on your own risk.

Why MX/FX614? This is a proven, legacy, but still available chip. Using this as coder/decoder reduces the workload of Arduino, and provides reasonable signal processing performance.

You can find the modem circuit schematics in the Hardware folder. I still have the device as a prototype, once a final construcion is ready, I will upload the PCB layout and other documents as well. Until than, the proto device mechanical construction is documented also in the Hardware folder (PDF).

If you want to implement, here are important things to start with and understand:

1. Arduino Serial2 is used for serial BT communication, so connect HC-05 to the Serial2 interface - or change the interface EVERYWHERE in KISS_communication.cpp
2. Change the RX buffer size from the standard 64 byte to at least 340 bytes in the Arduino IDE Hardwareserial.h file
3. The HC-05 module caused EMI issues during the tests, so I applied ferrite beads on connecting wires between Mega 2560 5V-GND connections to modem and HC-05, and also on the serial data wires between Mega 2560 and HC-05. Same for the GPS module.

Credits to John Hansen, W2FS (https://www.tapr.org/pdf/DCC1998-PICet-W2FS.pdf) for the basic principles of APRS transmission code.


Useful links:

MX614 datasheet: https://www.cmlmicro.com/wp-content/uploads/2017/06/MX614_ds.pdf

APRS Protocol Reference: http://www.aprs.org/doc/APRS101.PDF

Understanding HDLC/AX.25 and KISS: https://notblackmagic.com/bitsnpieces/ax.25/?utm_source=pocket_reader

KISS TNC reference: https://www.ax25.net/kiss.aspx

The Cyclic Redundancy Check (CRC) for AX.25 (FCS): http://practicingelectronics.com/articles/article-100003/article.php

How to change RX buffer size in Hardwareserial.h: https://forum.arduino.cc/t/solved-serial-buffer-size/581828/4

Arduino HC-05 tutorial: https://howtomechatronics.com/tutorials/arduino/arduino-and-hc-05-bluetooth-module-tutorial/
