#include "KISS_communication.h"
#include "HDLC_MX614.h"

byte APRS_Frame[] = {0x82, 0xA0, 0x88, 0xA4, 0x62, 0x6C, 0xE0, 0x90, 0x82, 0x64, 0xB4, 0x84, 0x40, 0x62, 0xAE, 0x92, 0x88, 0x8A, 0x62, 0x40, 0x62, 0xAE, 0x92, 0x88, 0x8A, 0x64, 0x40, 0x65, 0x03, 0xF0, 0x3A, 0x48, 0x41, 0x35, 0x42, 0x52, 0x20, 0x20, 0x20, 0x20, 0x3A, 0x4F, 0x6B, 0x7B, 0x31};

boolean Last_APRS_Byte;

boolean FrameError;

int APRS_Frame_Length = 45;

void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);

Serial.println("Initializing...");

Begin_KISS_port();

Serial.println("KISS port ready");

delay(10000);

// Begin_Modem();

// Set_RXTimeOut(2000);

// Modem_Off();

// Modem_RX();

// Serial.println("Sending KISS frames");

}

void loop() {
  // put your main code here, to run repeatedly:

  /*
  
  APRS_Frame_Length = HDLC_Frame_Available();

  if (APRS_Frame_Length != 0) {

    FrameError = APRS_Frame_Length < 0;

    if (!FrameError) {
      
      Serial.print(" Frame[");
      Serial.print(APRS_Frame_Length);
      Serial.print("] ");

      for (int i=0; i <= APRS_Frame_Length; i++) {
      
        APRS_Frame[i] = Get_HDLC_Frame(i);

        Serial.print(APRS_Frame[i], HEX);
        Serial.print(" ");

      }

      Serial.println();
      Serial.println("Sending to KISS port...");

      for (int i=0; i<= APRS_Frame_Length-2; i++) {

        Last_APRS_Byte = (i == (APRS_Frame_Length - 2));

        Send_HDLC_Frame(APRS_Frame[i], Last_APRS_Byte);

      } 
      
    } else { //Serial.println("Frame error");
    
      }
    
  }

  */

  Serial.println();
  Serial.print("Sending to KISS port... ");
  Serial.println(APRS_Frame_Length);

  for (int i=0; i<= APRS_Frame_Length-1; i++) {

    Last_APRS_Byte = (i == (APRS_Frame_Length - 1));

    Send_KISS_Frame(APRS_Frame[i], Last_APRS_Byte);

  } 

  delay(5000);

}
