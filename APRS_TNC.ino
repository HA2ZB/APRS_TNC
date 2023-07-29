#include "KISS_communication.h"
#include "HDLC_MX614.h"

byte APRS_Frame[300];

boolean Last_APRS_Byte;

boolean FrameError;

int APRS_Frame_Length;

void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);

Serial.println("Initializing...");

Begin_KISS_port();

Begin_Modem();

Set_RXTimeOut(2000);

Modem_Off();

Modem_RX();

Serial.println("Watching audio interface...");

}

void loop() {
  // put your main code here, to run repeatedly:

  APRS_Frame_Length = HDLC_Frame_Available();

  if (APRS_Frame_Length != 0) {

    FrameError = APRS_Frame_Length < 0;

    if (FrameError) {
      
      APRS_Frame_Length = (-1) * APRS_Frame_Length;

      Serial.print(" Frame - : ");

    } else Serial.print(" Frame + : ");

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

  }

}
