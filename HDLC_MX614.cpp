#include "Arduino.h"
#include "HardwareSerial.h"
#include "HDLC_MX614.h"

const int MaxFrameLgth = 340;

const byte HDLC_Flag = 0x7E;

byte RX = 2; // RX port number

byte TX = 8;  // TX port number

byte PTT = 10;   // PTT port number

byte RDY = 12;

byte CLK = 6;

byte DET = 22;

byte M0 = 26;

byte M1 = 30;

byte HDLC_Packet [MaxFrameLgth];

int FrameLength;

byte ByteIn;

byte AX25_Flag = 0x7E;

boolean signalT;

boolean signalT_1;

int ReceivedBytes;

unsigned long TimeOut;

unsigned long DET_Time;

long RX_TimeOut;

long TX_TimeOut = 3000;

boolean bitOut;

boolean bitIn;

boolean Flag = false;

boolean FCS_flag = false;

int Stuff = 0;

word FCS = 0xFFFF;

int TX_Counter = 0;

int PTT_Delay = 500;

/*

Library internal functions
-------------------------------------------------------------------------------------------------------------------------------------------
*/

boolean Timer_Running (long TimerValue) {

  return ((millis() - DET_Time) < TimerValue);

}

void FCScalc(byte MSGbit){

  boolean XOR;

  XOR = (lowByte(FCS)&0x01)^MSGbit == 0x01;

  FCS = FCS >> 1;

  if (XOR){

    FCS = FCS ^ 0x8408;

  }

}

word CalculatedFCS(int FrameBytes) {

  byte PacketByte;

  FCS = 0xFFFF;

  for (int i=0; i<=FrameBytes; i++) {

    PacketByte = HDLC_Packet[i];

    for (int j=0; j<=7; j++) {

      FCScalc(PacketByte & 0x01);

      PacketByte = PacketByte >> 1;

    }

  }

  FCS = FCS^0xFFFF;

  return FCS;

}

word FrameFCS () {

  word Frame_FCS_value;

  Frame_FCS_value = HDLC_Packet[FrameLength]*256 + HDLC_Packet[FrameLength-1];

  return Frame_FCS_value;

}



boolean ReceiveBit() {

  boolean bitIn = true;          // internal variable for returning the value
  
  while (bitIn && (micros() < TimeOut)) {

    signalT = (digitalRead(RX) == HIGH);    // we read the RX signal (modem RXD)
    bitIn = signalT == signalT_1;
    signalT_1 = signalT;

  }

  if (bitIn) { TimeOut = micros() + 830; } else { TimeOut = micros() + 1000; }     // 820-840 960-1200

  return bitIn;                         // we return the bit value

}

boolean GetRXD () {

  boolean RXbit;

  RXbit = ReceiveBit();

  if (RXbit) {

    Stuff++;                            // we increment the 1's counter
    
  } else {

      if (Stuff == 5) {

        RXbit = ReceiveBit();

      }

      Stuff = 0;

    }

  return RXbit;

}

byte ReceiveByte(boolean LookForFlag) {

  byte RXbyte = 0x00;   // set the initial value of the byte to return

  int i;                // define for cycle variable

  if (LookForFlag) {            // if we are looking for a flag, it can come at any time

    while ((RXbyte != 0x7E) && Timer_Running(RX_TimeOut)) {    // so we keep scanning until we have an incoming flag (0x7E)

      RXbyte = RXbyte >> 1;     // we shift the byte to the right by 1

      if ( GetRXD() ) { RXbyte = RXbyte | 0x80; }  // we call the bit reading function and we tell to it that we are looking for a flag, no 0 dropping needed
                                                                  // if it returns 'true' (1), we set the highest bit to 1 otherwise it remains 0
    }                          // and keep this doing until we have a flag byte

  }

  else {                       // if we are not looking for a flag, we read exactly the next 8 bits (w/o zeros to be dropped)
  
    for (i=0; i<8; i++) {      // we simply read the next 8 bits (the called function will drop zeros if needed) and put it into the byte to be returned
      
      RXbyte = RXbyte >> 1;      
      
      if ( GetRXD() ) { RXbyte = RXbyte | 0x80; }    // false argument tells it should drop zero if needed

    } 

  }

  return RXbyte;      // we return the received byte

}


void flipOut() {

  Stuff = 0;

  bitOut = !bitOut;

  if (bitOut) digitalWrite(TX, HIGH); else digitalWrite(TX, LOW);
  
}

void transmit (){

  while (digitalRead(RDY) == HIGH){}

  delayMicroseconds(30);

  digitalWrite (CLK, HIGH);

  delayMicroseconds(30);

  digitalWrite(CLK, LOW);

}

void Reset_TX_Counter() {

  TX_Counter = 0;

}

void Send_HDLC_Byte(byte TXByte, boolean FlagByte){

  int k;
  byte sendBit;

   for (k=0; k<8; k++){

    sendBit = TXByte & 0x01;

    if (sendBit == 0) flipOut(); else {

      Stuff++;

      if (!FlagByte && (Stuff == 5)){

        transmit();

        flipOut();

      }

    }

    TXByte = TXByte >> 1;

    transmit();

  }

}


/*

Public functions
-------------------------------------------------------------------------------------------------------------------------------------------
*/


void Begin_Modem() {

  pinMode(TX, OUTPUT);
  pinMode(PTT, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(RDY, INPUT);
  pinMode(RX, INPUT);
  pinMode(DET, INPUT);
  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);

  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);
  digitalWrite(TX, HIGH);
  digitalWrite(CLK, HIGH);
  digitalWrite(PTT, LOW);
  
  // delay(1000);

}

void Set_RXTimeOut( long RXTimeOutValue) {

  RX_TimeOut = RXTimeOutValue;

}

void Set_TXTimeOut( long TXTimeOutValue) {

  TX_TimeOut = TXTimeOutValue;

}

void Modem_Off() {

  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);

  delay(200);

}

void Modem_RX() {

  digitalWrite(CLK, HIGH);
  digitalWrite(M0, LOW);
  digitalWrite(M1, HIGH);

  delay(25);

}

void Modem_TX() {

  bitOut = false;

  digitalWrite(CLK, LOW);
  digitalWrite(TX, LOW);

  digitalWrite(M1, LOW);
  digitalWrite(M0, HIGH);

  delay(300);

}

void PTT_On() {

  digitalWrite(PTT, HIGH);
  
  delay(PTT_Delay);

}

void PTT_Off() {

  digitalWrite(PTT, LOW);

}

void Set_PTT_Delay(int PTT_Delay_Value) {

  PTT_Delay = PTT_Delay_Value;

}

boolean Carrier_Detected() {

  boolean CD = false;

  if (digitalRead(DET) == HIGH) {

    delay(15);

    CD = (digitalRead(DET) == HIGH);

  }

  return CD;

}

int HDLC_Frame_Available() {

  word FCS_from_Calc;
  word FCS_from_Frame;
  word FCS_XOR;

  int k = 0;

  Stuff = 0;

  if (Carrier_Detected()) {          //signal detected, we should have flag(s) incoming

    DET_Time = millis();

    ByteIn = ReceiveByte (true);

    //Serial.println(ByteIn, HEX);

    Flag = (ByteIn == AX25_Flag); 

    if (Flag) {

      while (Flag) {

        ByteIn = ReceiveByte (false);

        Flag = (ByteIn == AX25_Flag);

      }

      while ( (k <= MaxFrameLgth) && !Flag ) {

        HDLC_Packet [k] = ByteIn;

        k++;

        ByteIn = ReceiveByte(false);

        Flag = ByteIn == 0x7E;

      }

      FrameLength = k-1;

      
      FCS_from_Calc = CalculatedFCS(FrameLength-2);
      FCS_from_Frame = FrameFCS();

     
      FCS_XOR = FCS_from_Calc ^ FCS_from_Frame;

      //Serial.println(FrameLength);

      if (FCS_XOR == 0x0000) {return FrameLength;} else {return FrameLength*(-1);}

    } else return 0;  // no flag received within the timeframe}

  } else return 0;   // no carrier detected
  
}

byte Get_HDLC_Frame(int i) {

  return HDLC_Packet[i];

}

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte) {

  word Frame_FCS;

  HDLC_Packet[TX_Counter] = ByteToSend;

  if (LastByte) {
    //Serial.println("Last byte");
    FrameLength = TX_Counter;

    Frame_FCS = CalculatedFCS(FrameLength);

    DET_Time = millis();

    while ( Carrier_Detected() && Timer_Running(TX_TimeOut) ) {}

    // Modem_Off();

    PTT_On();

    Modem_TX();
    //Serial.println("TX");

    for (int i=1; i<=600; i++) {

      Send_HDLC_Byte(HDLC_Flag, true);

    }
    
    for (int i=0; i<=FrameLength; i++) {

      Send_HDLC_Byte(HDLC_Packet[i], false);

    }

    Send_HDLC_Byte(lowByte(Frame_FCS), false);
    Send_HDLC_Byte(highByte(Frame_FCS), false);    

    Send_HDLC_Byte(HDLC_Flag, true);

    PTT_Off();

    Modem_RX();

    //Serial.println(Frame_FCS, HEX);

    TX_Counter = 0;

  } else {

    TX_Counter++;

  }

}
