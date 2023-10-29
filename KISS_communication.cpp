// KISS serial Buetooth communication functions for Arduino Mega 2560 and HC-05 module
// Béla Zagyva, HA2ZB

// IMPORTANT: this library uses the Arduino serial communication port to connect the HC-05 Bluetooth module
// Please apply the followings:
// 1) Serial2 is used, so connect HC-05 to the Serial2 interface - or change the interface EVERYWHERE in this code
// 2) Change the RX buffer size from the standard 64 byte to at least 340 bytes in the Arduino IDE Hardwareserial.h file

// If you are not familiar, check APRS KISS reference for the specification details

#include "HardwareSerial.h"
#include "KISS_communication.h"

const int MaxKISSFrameLgth = 340;   // the maximal length of an APRS frame is 332 bytes, so we set this around that value (a bit more)

byte FrameIN [MaxKISSFrameLgth];    // global variable for incoming frames (from Bluetooth)

byte FrameOUT [MaxKISSFrameLgth];   // global variable for outgoing frames (to Bluetooth)

int CounterIN = 0;      // global variable for incoming frame byte counter

int CounterOUT = 0;     // global variable for outgoing frame byte counter

boolean EscMode;        // global booleanv variable for indicating if the KISS communication is in escape mode

const byte FEND = 0xC0;       // frame end constant
const byte FESC = 0xDB;       // frame escape constant
const byte TFEND = 0xDC;      // transposed frame end constant
const byte TFESC = 0xDD;      // transposed frame escape constant

const byte BT_STATE = 36;     // HC-05 Bluetooth module STATE pin connected to D36 port to monitor the conncetion status

// function to initialize KISS port (Serial2 in this case)

void Begin_KISS_port() {

  delay(500);   // delay in ms - may not needed

  pinMode(BT_STATE, INPUT);   // setting D36 port to input

  Serial2.begin(9600);  // initialize Serial2

}

// function to add the byte in argument to the CounterIN position of the incoming frame array global variable

void FrameAdd(byte ByteToAdd) {

  FrameIN[CounterIN] = ByteToAdd;   // writes the byte to the array

  CounterIN++;      // increments the counter

  EscMode = false;  // sets the escape mode indicator to false

}

// it may happen that HC-05 comes up with random junk byte series, this function is to flush the buffer

void Flush_RX_Buffer() {

  byte Junk;    // internal byte variable to read the serial port

  while ( Serial2.available() ) {     // while data available in the buffer

    Junk = Serial2.read();            // we simply read it and do nothing with it

  }

}

// function to receive kiss frame from the Bluetooth interface
// returns th number of received bytes

int KISS_frame_available () {

  byte ByteIN;              // internal variable to read the data

  boolean FrameComing;      // boolean value indicating if data is available and we are still waitning for a FEND

  int ReturnValue = 0;      // internal variable for the return value

  CounterIN = 0;            // reset incoming data counter (global variable)

  if (Serial2.available()) {    // if data is available in serial buffer

    ByteIN = Serial2.read();    // we read the first byte, according to the specification, it is always a FEND
        
    if ( ByteIN == FEND ) {     // if that is the case, we have a KISS frame

      FrameComing = true;       // so we expect KISS data bytes

      EscMode = false;          // and we start with the normal (no escape) mode

      while (FrameComing) {     // while we see a valid KISS frame

        if (Serial2.available()) {    // check the buffer if data available

          ByteIN = Serial2.read();    // read the next byte from the buffer

          if (EscMode) {              // if we are in escape mode (the previous byte was a FESC)

            if (ByteIN == TFEND) {FrameAdd(FEND);}    // if the byte is a TFEND, we change it back to FEND, and add to the end of the frame array (so this is NOT the end of the frame)

            if (ByteIN == TFESC) {FrameAdd(FESC);}    // if the byte is a TFESC, we change it back to FESC, and add ... and there are no other valid cases to handle

          } else {      // if we are not in escape mode

            if (ByteIN == FESC) {EscMode = true;} else {    // if the byte is FESC, we switch to escape mode

              if (ByteIN == FEND) {       // or we check if this is a FEND
                
                FrameComing = false;      // in that case we set this flag to false, this will stop the loop (see while condition)
              
              } else {

                FrameAdd(ByteIN);         // if neither FEND nor FESC received this is a normal databyte, so we add it to the end of the frame array

              }
            
            }

          }

        }

      }

      Flush_RX_Buffer();            // we may have some junk in the buffer after FEND, so we flush it

      ReturnValue = CounterIN-1;    // receiving counter is incremented in the FrameAdd function, so we decrement, and that is the lenght of the frame

      ByteIN = 0x00;                // we reset the byte variable - may not needed

      CounterIN = 0;                // reset the byte counter


    } else {          // first byte was not FEND, this is not a valid KISS frame 

      Flush_RX_Buffer();    // so we simply flush the buffer

    }

 
  } 

  return ReturnValue;     // we return the length of the frame

}

// getter function to the incoming frame global array variable

byte Get_KISS_Frame(int i) {      // the argument is the position of the byte we want to get

  return FrameIN[i];              // and the function returns that specific byte

}

// function to send KISS frame to HC-05 Bluetooth module
// we call this function as many times as many bytes we want to send

void Send_KISS_Frame(byte ByteToSend, boolean LastByte) {     // arguments: the byte to send and boolean indicating if this is the last byte of the frame

  if (ByteToSend == FEND) {     // if the databyte happens to equal the FEND byte, we do an escape sequence

    FrameOUT[CounterOUT] = FESC;      // in the global frame array variable we add a FESC byte
    CounterOUT++;                     // increment the byte counter
    FrameOUT[CounterOUT] = TFEND;     // add the transposed FEND (TFEND)
    CounterOUT++;                     // increment the byte counter
    
  } else if (ByteToSend == FESC) {    // if it is a FESC

    FrameOUT[CounterOUT] = FESC;      // we do the same sequence, see above
    CounterOUT++;
    FrameOUT[CounterOUT] = TFESC;
    CounterOUT++;

  } else {      // if it is neither FEND nor FESC
    
    FrameOUT[CounterOUT] = ByteToSend;    // we simply add this byte to the outgoing frame array
    CounterOUT++;                         // and increment the counter

  }

  if (LastByte) {       // if that was the last byte of the frame

    CounterOUT--;       // we decrement the counter (now the value is the length of the frame)

    Serial2.write(FEND);    //  we start sending to serial port with a FEND

    Serial2.write(0);       // next is a zero byte according to the specification (data frame, not command frame)

    for (int i=0; i<=CounterOUT; i++) {     // than we one by one send out the frame bytes

      Serial2.write(FrameOUT[i]);           // writing them to the serial port

    }

    Serial2.write(FEND);                    // we close the frame with a FEND

    CounterOUT = 0;                         // reset the byte counter

  }

}