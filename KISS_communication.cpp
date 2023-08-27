#include "HardwareSerial.h"
#include "KISS_communication.h"

byte FrameIN [300];
byte FrameOUT [300];
int CounterIN = 0;
int CounterOUT = 0;
boolean EscMode;
byte FEND = 0xC0;
byte FESC = 0xDB;
byte TFEND = 0xDC;
byte TFESC = 0xDD;

void Begin_KISS_port() {

  delay(5000);

  Serial2.begin(9600);

}

void FrameAdd(byte ByteToAdd) {

  FrameIN[CounterIN] = ByteToAdd;

  //if (CounterIN > 0) {Serial.print(ByteToAdd, HEX); Serial.print(" ");}

  CounterIN++;

  EscMode = false;

}

void Flush_RX_Buffer() {

  byte Junk;

  while ( Serial2.available() ) {

    Junk = Serial2.read();

  }

}
/*
int KISS_frame_available () {

  byte ByteIN;
  boolean FrameComing;
  int ReturnValue = 0;

  if (Serial2.available()) {

    ByteIN = Serial2.read(); 
    //Serial.println(ByteIN, HEX);
    
    if (ByteIN == FEND) {

      FrameComing = true;

      EscMode = false;

      CounterIN = 0;

      while (FrameComing) {

        if (Serial2.available()) {

          ByteIN = Serial2.read();

          //Serial.print(ByteIN, HEX); Serial.print(" ");

          if (EscMode) {

            if (ByteIN == TFEND) {FrameAdd(FEND);}

            if (ByteIN == TFESC) {FrameAdd(FESC);}

          } else {

            if (ByteIN == FESC) {EscMode = true;} else {

              if (ByteIN == FEND) {
                
                FrameComing = false; 
                
                //Serial.println(); Serial.println("FEND");
              
              } else {

                FrameAdd(ByteIN);

              }
            
            }

          }

        }

      }

      CounterIN--;

     // Serial.print(CounterIN);

      ReturnValue = CounterIN;

      ByteIN = 0x00;

      CounterIN = 0;

      Serial.println();

    }

 
  } 
  // if (ReturnValue > 0) {Serial.println(ReturnValue);}

  return ReturnValue;

}
*/

int KISS_frame_available () {

  byte ByteIN;
  boolean FrameComing;
  int ReturnValue = 0;
  CounterIN = 0;

  if (Serial2.available()) {

    ByteIN = Serial2.read(); 
    //Serial.println(ByteIN, HEX);
    
    if ( ByteIN == FEND ) {

      FrameComing = true;

      EscMode = false;

      while (FrameComing) {

        if (Serial2.available()) {

          ByteIN = Serial2.read();

          //Serial.print(ByteIN, HEX); Serial.print(" ");

          if (EscMode) {

            if (ByteIN == TFEND) {FrameAdd(FEND);}

            if (ByteIN == TFESC) {FrameAdd(FESC);}

          } else {

            if (ByteIN == FESC) {EscMode = true;} else {

              if (ByteIN == FEND) {
                
                FrameComing = false; 
                
                //Serial.println(); Serial.println("FEND");
              
              } else {

                FrameAdd(ByteIN);

              }
            
            }

          }

        }

      }

      Flush_RX_Buffer();
     // Serial.print(CounterIN);

      ReturnValue = CounterIN-1;

      ByteIN = 0x00;

      CounterIN = 0;

      // Serial.println();

    } else {

      Flush_RX_Buffer();

    }

 
  } 
  // if (ReturnValue > 0) {Serial.println(ReturnValue);}

  return ReturnValue;

}


byte Get_KISS_Frame(int i) {

  return FrameIN[i];

}

void Send_KISS_Frame(byte ByteToSend, boolean LastByte) {

  if (ByteToSend == FEND) {

    FrameOUT[CounterOUT] = FESC;
    CounterOUT++;
    FrameOUT[CounterOUT] = TFEND;
    CounterOUT++;
    
  } else if (ByteToSend == FESC) {

    FrameOUT[CounterOUT] = FESC;
    CounterOUT++;
    FrameOUT[CounterOUT] = TFESC;
    CounterOUT++;

  } else {
    
    FrameOUT[CounterOUT] = ByteToSend;
    CounterOUT++;

  }

  if (LastByte) {

    CounterOUT--;

    Serial2.write(FEND);

    Serial2.write(0);

    for (int i=0; i<=CounterOUT; i++) {

      Serial2.write(FrameOUT[i]);

    }

    Serial2.write(FEND);

    CounterOUT = 0;

  }

}