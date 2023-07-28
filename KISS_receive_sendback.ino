#include "KISS_communication.h"


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  Begin_KISS_port();

}

void loop() {
  // put your main code here, to run repeatedly:

  int Frame_length = 0;

  byte KISS_Frame_Received[300];

  Frame_length = KISS_frame_available();

  if (Frame_length != 0) {

    for (int i=0; i <= Frame_length; i++) {

      KISS_Frame_Received[i] = Get_KISS_Frame(i);
      
      Serial.print(KISS_Frame_Received[i], HEX);

      Serial.print(' ');

    }

        Serial.println();

    delay(5000);

    Serial.print("Sending frame back. Length: ");

    Serial.println(Frame_length);


    for (int i=0; i<= Frame_length; i++) {

      Send_KISS_Frame (KISS_Frame_Received[i], i==Frame_length);

    }

    Serial.println("Frame sent");
    
  }

}
