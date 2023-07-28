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

  Serial2.begin(9600);

}

void FrameAdd(byte ByteToAdd) {

  FrameIN[CounterIN] = ByteToAdd;

  CounterIN++;

  EscMode = false;

}

int KISS_frame_available () {

  byte ByteIN;
  boolean FrameComing;
  int ReturnValue = 0;

  if (Serial2.available()) { 
    
    if (Serial2.read() == FEND) {

      FrameComing = true;

      EscMode = false;

      CounterIN = 0;

      while (FrameComing) {

        if (Serial2.available()) {

          ByteIN = Serial2.read();

          if (EscMode) {

            if (ByteIN == TFEND) {FrameAdd(FEND);}

            if (ByteIN == TFESC) {FrameAdd(FESC);}

          } else {

            if (ByteIN == FESC) {EscMode = true;} else {

              if (ByteIN == FEND) {FrameComing = false;} else {

                FrameAdd(ByteIN);

              }
            
            }

          }

        }

      }

      CounterIN--;

      ReturnValue = CounterIN;

      CounterIN = 0;

    }

 
  } 

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

    for (int i=0; i<=CounterOUT; i++) {

      Serial2.write(FrameOUT[i]);

    }

    Serial2.write(FEND);

    CounterOUT = 0;

  }

}