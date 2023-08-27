#include "KISS_communication.h"
#include "HDLC_MX614.h"

byte APRS_Frame[300];

boolean Last_APRS_Byte;

boolean FrameError;

int APRS_Frame_Length = 0;

int APRS_Frame_Length_TX = 0;

void setup() {
  // put your setup code here, to run once:

// Serial.begin(9600);

Begin_KISS_port();

Begin_Modem();

Set_TXTimeOut(500);

Set_RXTimeOut(500);

Set_PTT_Delay(500);

Modem_Off();

}

void loop() {
  // put your main code here, to run repeatedly:

  Modem_RX();

  //*
  
  APRS_Frame_Length = HDLC_Frame_Available();


  if (APRS_Frame_Length != 0) {

    //Serial.print("Frame!");

    FrameError = APRS_Frame_Length < 0;

    if (!FrameError) {

      //Serial.println("HDLC frame");

      for (int i=0; i <= APRS_Frame_Length; i++) {
      
        APRS_Frame[i] = Get_HDLC_Frame(i);

      }

      for (int i=0; i<= APRS_Frame_Length-3; i++) {

        Last_APRS_Byte = (i == (APRS_Frame_Length - 3));

        Send_KISS_Frame(APRS_Frame[i], Last_APRS_Byte);

      } 
      
    }
    
  }
  

  //*
  APRS_Frame_Length_TX = KISS_frame_available();

  //Serial.print(APRS_Frame_Length);

  if (APRS_Frame_Length_TX > 0) {

    //Serial.println(APRS_Frame_Length_TX);

    delay(500);

    //Serial.print(APRS_Frame_Length);
    //Serial.print(" - ");

    Reset_TX_Counter();

    for (int i=1; i<=APRS_Frame_Length_TX; i++) {

      Last_APRS_Byte = ( i == APRS_Frame_Length_TX );

      Send_HDLC_Frame( Get_KISS_Frame(i), Last_APRS_Byte );

      //Serial.print(Get_KISS_Frame(i), HEX);
      //Serial.print(" ");
    }

    APRS_Frame_Length_TX = 0;
    
  } 
  //*/

}
