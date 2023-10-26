// APRS packet coding and decoding functions for Arduino Mega 2560 with MX/FX614 modem IC
// Béla Zagyva, HA2ZB

// DO NOT USE - code is under development

#include "Arduino.h"
#include "HardwareSerial.h"
#include "APRS_communication.h"
#include <TinyGPS++.h>

const int MaxAPRSLength = 340;

byte APRS_Frame_Out[MaxAPRSLength];

byte APRS_Frame_In[MaxAPRSLength];

byte Source[7];

byte Source_SSID;

byte Destination[7];

byte Destination_SSID;

byte Digipeaters[8] [7];


