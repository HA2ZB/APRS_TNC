# APRS_TNC
Arduino Mega 2560 based APRS TNC with MX614 (FX614) modem

This is a very initial stage of the development of an Arduino based APRS TNC. The current functionality connects Android APRSDroid app to the radio audio interface via Bluetooth (HC-05 module). I am using the Elegoo Mega 2560 Arduio clone in this project.

Basic functionality is tested, working, but I cannot guarantee the bug free operation of course, use it on your own risk.

Why MX/FX614? This is a proven, legacy, but still available chip. Using this as coder/decoder reduces the workload of Arduino, and provides reasonable signal processing performance.

You can find the modem circuit schemtics in the Hardware folder. I have it on breadboard, once a final construcion is ready, I will upload the PCB layout and other documents as well.

If you want to implement, here are important things to start with and understand:

1. Arduino Serial2 is used for serial BT communication, so connect HC-05 to the Serial2 interface - or change the interface EVERYWHERE in KISS_communication.cpp
2. Change the RX buffer size from the standard 64 byte to at least 332 bytes in the Arduino IDE Hardwareserial.h file


Credits to John Hansen, W2FS (https://www.tapr.org/pdf/DCC1998-PICet-W2FS.pdf) for the basic principles of APRS transmission code.
