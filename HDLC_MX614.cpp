// HDLC communication functions for Arduino Mega 2560 with MX/FX614 modem IC
// Béla Zagyva, HA2ZB

#include "Arduino.h"
#include "HardwareSerial.h"
#include "HDLC_MX614.h"

const int MaxFrameLgth = 340;  // max. APSR frame length is 332

const byte HDLC_Flag = 0x7E;  // HDLC flag byte (marks beginning and end of HDLC frame)

// Arduino Mega 2560 Digital ports used for modem control

const byte RX = 2;  // RX data input (from modem/radio)

byte TX = 8;  // TX data output (to modem/radio)

byte PTT = 10;  // PTT output on=HIGH, off=LOW

byte RDY = 12;  // RDY input modem handshake signal - ready to receive TX bit

byte CLK = 6;  // CLK output modem handshake signal, latches the next TX byte at modem

byte DET = 22;  // Carrier detect input from modem

byte M0 = 26;  // Modem control output, see below

byte M1 = 30;  // Modem control output, see below

// end of Digital ports definition

byte HDLC_Packet[MaxFrameLgth];  // Byte array variable for HDLC packet to process

int FrameLength;  // internal variable to handle lenght of frame to be sent / received

byte ByteIn;  // variable for received byte

boolean signalT;  // current level of RX signal

boolean signalT_1;  // RX signal level at previous timeslot

int ReceivedBytes;  // variable for received bytes number

unsigned long TimeOut;  // bit level reception timer variable

unsigned long Set_Time; // variable to store the current time when starting a timer

long RX_TimeOut;        // time (ms) we wait after carrier detected for a valid frame (flag)

long TX_TimeOut = 3000; // time (ms) we wait before transmitting in case the channel is busy (carrier detected)

boolean bitOut;         // bit to send

boolean bitIn;          // bit received

boolean Flag = false;   // the byte to send is a flag byte

boolean FCS_flag = false;   // the byte to send is an FCS byte

int Stuff = 0;          // number of consecutive '1's already sent / received

word FCS = 0xFFFF;      // FCS word variable

int TX_Counter = 0;     // counter for incoming bytes

int PTT_Delay = 500;    // PTT delay variable (ms)

/*

Library internal functions
-------------------------------------------------------------------------------------------------------------------------------------------
*/


// timer function, returns a boolean value if the time passed from the Set_Time is shorter than the argument
boolean Timer_Running(long TimerValue) {

  return ((millis() - Set_Time) < TimerValue);    // compares the current time (milliseconds since starting the program) to the set time (global variable, set at the time we want to start the counter) and returns the boolean result
}                                                 // if the difference is smaller waht is provided in the argument (TimerValue) the return value is positive, meaning the timer is still running


// single bit FCS calculation step, this function will be called regularly by the FCS word calculation routine
// argument: the data bit we want to process (lowest bit of MSGbit)
// for detailed explanation see CRC-CCITT and ISO 3309 specifications

void FCScalc(byte MSGbit) {

  boolean XOR;    // internal variable for boolean operation

  XOR = (lowByte(FCS) & 0x01) ^ MSGbit == 0x01;   // XOR operation on the lowest bit of FCS word (global variable) and the message bit we are processing

  FCS = FCS >> 1;   // shift right FCS word by one

  if (XOR) {

    FCS = FCS ^ 0x8408;   // if the result is 1 (true) we apply the generator polynom as a XOR operation
  }
}


// FCS calculator fuction, calculates the FCS word for the whole packet (argument is the number of frame bytes)
// for detailed explanation see CRC-CCITT and ISO 3309 specifications

word CalculatedFCS(int FrameBytes) {

  byte PacketByte;  // internal variable to process a specific byte in the frame

  FCS = 0xFFFF;     // initial value of FCS according to the specification

  for (int i = 0; i <= FrameBytes; i++) {     // we do it for all frame bytes (except flags and incoming FCS)

    PacketByte = HDLC_Packet[i];              // we take the first / next message byte

    for (int j = 0; j <= 7; j++) {            // we do the FCS process for each bits

      FCScalc(PacketByte & 0x01);             // calling the sub-function above

      PacketByte = PacketByte >> 1;           // shift the byte right by one to get the next bit
    }
  }

  FCS = FCS ^ 0xFFFF;                         // XOR mask on the calculated word according to the specification

  return FCS;                                 // return the calculated FCS word
}

// this function gets the incoming FCS word from the received data frame
word FrameFCS() {

  return (HDLC_Packet[FrameLength] * 256 + HDLC_Packet[FrameLength - 1]);   // we simply create a word value from the 2 FCS bytes at the end of the incoming frame, and return as function value

}

// this sub-fuction is doing the sampling on the modem's RXD output digital signal
// and returns the received bit as 1 (true) or 0 (false)

boolean ReceiveBit() {

  boolean bitIn = true;  // internal variable for returning the value. we set this to 'true' for the initial sampling

  while (bitIn && (micros() < TimeOut)) {   // we assume the next incoming bit is 1, and we wait for 1 bit time before sampling

    signalT = (digitalRead(RX) == HIGH);  // sampling - we read the RX signal (modem RXD)
    bitIn = signalT == signalT_1;         // if there is no change related to the previous (T-1) sample, the received bit is 1 (no return to zero coding)
    signalT_1 = signalT;                  // we store this sample as T-1 sample for the next sampling
  }

  if (bitIn) {                            // if we received a 1 it means, there were no switch on the logic values, we did not get an edge to synchronize our sampler timing
    TimeOut = micros() + 830;             // so we use a quasi standard 830 microsec delay for the next sample. this should not be much longer, not to lose the sync if more 1s are coming
  } else {
    TimeOut = micros() + 1000;            // if we received a 0, we had an edge, so we now can lock our timing to that. we apply here a bit longer delay to the next sample
  }                                       // so we position the time somewhere in the midle of the next byte, not ot miss a possible next edge
                                          // these timing ranges work in this solution: 820-840 960-1200
  return bitIn;                           // we return the bit value
}


// sub fuction to get the incoming REAL databit
// manages AX.25 bit-stuffing

boolean GetRXD() {

  boolean RXbit;    // internal variable for return value
  boolean Stuff_Zero_Expected;

  Stuff_Zero_Expected = (Stuff == 5);

  RXbit = ReceiveBit();   // we call the function above to get the PHYSICAL incoming bit

  if (RXbit) {            // if this is 1

    Stuff++;              // we increment the 1's counter

  } else {                // if 0 received

    Stuff = 0;            // we reset the stuff counter

    if (Stuff_Zero_Expected) {     // we check if there were 5 consecutive 1s before

      RXbit = ReceiveBit();   // if yes, this is a stuffing 0, so we drop it and do the next sampling, that will be a REAL HDLC bit

      if (RXbit) {            // if the next bit is 1

        Stuff = 1;            // we start counting the 1 bits again

      }

    }
   
  }

  return RXbit;               // we return the received (real) HDLC bit

}

// byte level HDLC receive function

byte ReceiveByte(boolean LookForFlag) {     // argument: if true, we are looking for a HDLC flag

  byte RXbyte = 0x00;                       // set the initial value of the byte to return

  if (LookForFlag) {                        // if we are looking for a flag, it can come at any time

    while ((RXbyte != 0x7E) && Timer_Running(RX_TimeOut)) {       // so we keep scanning until we have an incoming flag (0x7E), or the timer counts down

      RXbyte = RXbyte >> 1;                 // we shift the byte to the right by 1

      if (GetRXD()) { RXbyte = RXbyte | 0x80; }  // we call the bit reading function
                                                 // if it returns 'true' (1), we set the highest bit to 1 otherwise it remains 0
    }                                            // and keep this doing until we have a flag byte

  }

  else {                                        // if we are not looking for a flag, we read exactly the next 8 bits (w/o zeros to be dropped)

    for (int i = 0; i < 8; i++) {               // we simply read the next 8 bits (the called function will drop zeros if needed) and put it into the byte to be returned

      RXbyte = RXbyte >> 1;                     // shift right the databyte

      if (GetRXD()) { RXbyte = RXbyte | 0x80; }  // read the next databit and set the highest bit of the RXbyte accordingly
    }
  }

  return RXbyte;  // we return the received byte

}

// sub-function for transmitting data, this changes the modem TXD input level if zero to be transmitted (NRZ coding)

void flipOut() {

  Stuff = 0;                              // we reset the stuff counter, as we have a 0 to transmit

  bitOut = !bitOut;                       // negate the bitOut global boolean variable

  if (bitOut) digitalWrite(TX, HIGH);     // if it is 'true' (1), we set the TXD modem input to HIGH
  else digitalWrite(TX, LOW);             // if 'false' (0), we set the TXD to LOW
}


// bit transmission sub-function with modem sync control
// the MX/FX614 RDY-CLK handshake ensures the accurate 1200 bps timing

void transmit() {                   

  while (digitalRead(RDY) == HIGH) {}     // we wait for the modem to drop the RDY output to 0, indicating that it is ready to receive the next bit

  delayMicroseconds(30);                  // if dropped, we wait 30 microsecs (see modem datasheet)

  digitalWrite(CLK, HIGH);                // and we set CLK modem input to HIGH, latching the TXD input to the modem's shift register

  delayMicroseconds(30);                  // we wait 30 microsecs (see modem datasheet)

  digitalWrite(CLK, LOW);                 // and reset CLK
}


// global variable and setter needed to manage the TX counter (counting HDLC frame bytes)

void Reset_TX_Counter() {

  TX_Counter = 0;
}

// sub-fuction to manage HDLC byte transmission and related bit stuffing

void Send_HDLC_Byte(byte TXByte, boolean FlagByte) {      // arguments: byte to send and boolean to indicate if this is a FlagByte (no bit stuffing needed)

  byte sendBit;                                           // internal variable for bit level process

  for (int k = 0; k < 8; k++) {                           // for all the 8 bits...

    sendBit = TXByte & 0x01;                              // we take the last bit of the byte to transmit

    if (sendBit == 0) flipOut();                          // if this is 0, we switch the TXD level (NRZ coding)
    
    else {                                                // if 1

      Stuff++;                                            // we increment the stuff counter

      if (!FlagByte && (Stuff == 5)) {                    // if this is not a flagbyte and we already have 5 consecutive 1s

        transmit();                                       // we transmit this 5th 1-value bit

        flipOut();                                        // and we insert a stuff zero by switching the TXD level
      }
    }

    TXByte = TXByte >> 1;                                 // we shift right the TX byte, next bit will come

    transmit();                                           // we transmit the current bit

  }

}


/*

Public functions
-------------------------------------------------------------------------------------------------------------------------------------------
*/

// modem initialization function

void Begin_Modem() {

  // settings Arduino digital ports to input or output respectively
  pinMode(TX, OUTPUT);    
  pinMode(PTT, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(RDY, INPUT);
  pinMode(RX, INPUT);
  pinMode(DET, INPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  // setting initial output levels
  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);
  digitalWrite(TX, HIGH);
  digitalWrite(CLK, HIGH);    // as per the datasheet, this is recommended for RX mode. however, for TX is should be LOW initially
  digitalWrite(PTT, LOW);

}

// RX timeout setter
// the time in millisecs we are waiting for a valid frame (flagbytes) after data carrier detection

void Set_RXTimeOut(long RXTimeOutValue) {

  RX_TimeOut = RXTimeOutValue;
}

// TX timeout setter
// the time in millisecs we are waiting in case busy channel (carrier detected) before sending the HDLC frame 

void Set_TXTimeOut(long TXTimeOutValue) {

  TX_TimeOut = TXTimeOutValue;
}

// function to set the modem to idle state

void Modem_Off() {

  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);

  delay(200);   // we may not need this delay, but protects switching the modem on-off too fast
}

// function to set the modem to 1200 bps RX mode

void Modem_RX() {

  digitalWrite(CLK, HIGH);    // as per the datasheet, this is recommended for RX mode
  digitalWrite(M0, LOW);
  digitalWrite(M1, HIGH);

  delay(25);
}

// function to set the modem to 1200 bps TX mode

void Modem_TX() {

  bitOut = false;     // we set the TXD to low, so we set this global variable accordingly to 'false'

  digitalWrite(CLK, LOW);   // the initial level should be LOW, otherwise we cannot have a rising edge
  digitalWrite(TX, LOW);

  digitalWrite(M1, LOW);
  digitalWrite(M0, HIGH);

  delay(300);
}

// function to key PTT output

void PTT_On() {

  digitalWrite(PTT, HIGH);

  delay(PTT_Delay);  // PTT delay can be set by function, this is the time in millisecs between keying PTT and starting the transmission
}

// function to unkey PTT output

void PTT_Off() {

  digitalWrite(PTT, LOW);
}

// function to set PTT delay

void Set_PTT_Delay(int PTT_Delay_Value) {

  PTT_Delay = PTT_Delay_Value;    // this is the time in millisecs between keying PTT and starting the transmission
}

// function to return the status of modem DET output
// if 'true', the modem detects a carrier on the channel

boolean Carrier_Detected() {

  boolean CD = false;

  if (digitalRead(DET) == HIGH) {     // first sample

    delay(15);                        // 15 ms delay

    CD = (digitalRead(DET) == HIGH);  // second sample. if both samples are HIGH, we have a real carrir, not just a pin impulse, which can come from the Bluetooth
  
  }

  return CD;      // we return the function value

}

// HDLC frame receiving function
// the return value is a non-zero integer if frame received, the integer value is the number of received bytes (except flags)
// the value is negative in case of FCS error (calculated FCS does not equal with received FCS)

int HDLC_Frame_Available() {

  word FCS_from_Calc;     // internal variable for calculated FCS
  word FCS_from_Frame;    // internal variable for received FCS
  word FCS_XOR;           // internal comparison variable

  int k = 0;              // internal byte counter

  Stuff = 0;              // ve set the global stuff bit counter to zero

  if (Carrier_Detected()) {  //signal detected, we should have flag(s) incoming

    Set_Time = millis();  // we initialize the counter (store the current time to the global variable), the counter will be checked in the ReceiveByte function

    ByteIn = ReceiveByte(true);   // we are looking for a flag, the ReceiveByte function is called accordingly with 'true' argument
                                  // in this case the ReceiveByte function will return either when received a flag byte or the counter counts down

    Flag = (ByteIn == HDLC_Flag); // we check if a flag was received

    if (Flag) {                   // if flag was received

      while (Flag) {              // we go on with receiving the bytes, as normally more flags are sent at the beginning of the frame sequence

        ByteIn = ReceiveByte(false);  // so we check the next byte

        Flag = (ByteIn == HDLC_Flag); // if it is a flag, we continue the while loop until a different byte received
      }

      while ((k <= MaxFrameLgth) && !Flag) {    // we start processing the incoming HDLC bytes, until we receive a flag byte, or exceed the max frame length

        HDLC_Packet[k] = ByteIn;                // we store the received byte to the next position of the global array variable

        k++;                                    // increment the counter for the next byte

        ByteIn = ReceiveByte(false);            // receive the next byte

        Flag = ByteIn == 0x7E;                  // check if flag received
      }

      FrameLength = k - 1;                      // decrement the counter, this is the length of the HDLC frame (without flags)


      FCS_from_Calc = CalculatedFCS(FrameLength - 2);     // we calculate the FCS value of the frame
      FCS_from_Frame = FrameFCS();                        // we get the received FCS bytes from the frame


      FCS_XOR = FCS_from_Calc ^ FCS_from_Frame;           //comparison of the two values

      if (FCS_XOR == 0x0000) {                            // if they are equal
        
        return FrameLength;                               // we return the length of the frame (so NOT the frame itself!)
      
      } else {
      
        return FrameLength * (-1);                        // if theere is a deviation between the two FCS values, we returh the length as a negative integer
      
      }

    } else return 0;  // no flag received within the timeframe, so we return zero

  } else return 0;  // no carrier detected, so we return zero

}

// getter function for the global HDLC array variable

byte Get_HDLC_Frame(int i) {    // argument is the position of databyte we want to get

  return HDLC_Packet[i];        // return value is the byte itself

}

// function to send HDLC frame
// no return value
// arguments: the byte to send IN ORDER, so we call the function as many times as many bytes we have, the first at first, then 2nd and so on
// the second boolean argument marks the last byte, so after that the function will not wait any longer, but sends the bytes out as a HDLC frame

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte) {

  word Frame_FCS;   // internal FCS variable

  HDLC_Packet[TX_Counter] = ByteToSend;   // stores the bytge in the argument to the position of the TX_Counter, which has to be reset by a separate external function

  if (LastByte) {   // if this is the last byte to send
    
    FrameLength = TX_Counter;   // the length of the frame is (without FCS) is the TX_Counter value

    Frame_FCS = CalculatedFCS(FrameLength);   // we calculate the FCS value for the frame before sending

    Set_Time = millis();        // reset the counter

    while (Carrier_Detected() && Timer_Running(TX_TimeOut)) {}    // we check if the channel is busy (carrier detected), but we do not wait longer than TX_TimeOut

    PTT_On();       // we key the radio PTT

    Modem_TX();     // we set the modem to TX mode

    for (int i = 1; i <= 600; i++) {      // we start the transmission with several HDLC flags

      Send_HDLC_Byte(HDLC_Flag, true);    // calling the HDLC byte send function (true argument means no stuffing required)
    
    }

    for (int i = 0; i <= FrameLength; i++) {      // we follow with the databytes

      Send_HDLC_Byte(HDLC_Packet[i], false);      // we call the sending function with the next byte in order, stuffing enabled

    }

    Send_HDLC_Byte(lowByte(Frame_FCS), false);    // finally we send out the 2 FCS bytes
    Send_HDLC_Byte(highByte(Frame_FCS), false);

    Send_HDLC_Byte(HDLC_Flag, true);              // we close the frame with an HDLC flag

    PTT_Off();    // we unkey the radio PTT

    Modem_RX();   // we set back the modem to RX mode

    TX_Counter = 0;   // now we can reset the TX counter

  } else {

    TX_Counter++;   // if it was not the last byte to send, we simply increment the counter and go to the next incoming byte

  }
  
}
