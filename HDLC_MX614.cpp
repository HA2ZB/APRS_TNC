#include "HardwareSerial.h"
#include "HDLC_MX614.h"

const int MaxFrameLgth = 300;

byte RX = 2; // RX port number

byte TX = 8;  // TX port number

byte PTT = 10;   // PTT port number

byte RDY = 12;

byte CLK = 6;

byte DET = 22;

byte M0 = 26;

byte M1 = 30;

byte PacketIn [MaxFrameLgth];

int FrameLength;

byte ByteIn;

byte AX25_Flag = 0x7E;

boolean signalT;

boolean signalT_1;

int ReceivedBytes;

unsigned long TimeOut;

unsigned long DET_Time;

long RX_TimeOut;

boolean bitOut;

boolean bitIn;

boolean Flag = false;

boolean FCS_flag = false;

int Stuff = 0;

word FCS = 0xFFFF;



/*

Library internal functions
-------------------------------------------------------------------------------------------------------------------------------------------
*/

boolean Timer_Running () {

  return ((millis() - DET_Time) < RX_TimeOut);

}

void FCScalc(byte MSGbit){

  boolean XOR;

  XOR = (lowByte(FCS)&0x01)^MSGbit == 0x01;

  FCS = FCS >> 1;

  if (XOR){

    FCS = FCS ^ 0x8408;

  }

}

word CalculatedFCS() {

  byte PacketByte;

  FCS = 0xFFFF;

  for (int i=0; i<=FrameLength-2; i++) {

    PacketByte = PacketIn[i];

    for (int j=0; j<=7; j++) {

      FCScalc(PacketByte & 0x01);

      PacketByte = PacketByte >> 1;

    }

  }

  FCS = FCS^0xFFFF;

  Serial.print("Calculated FCS: ");
  Serial.print(lowByte(FCS), HEX);
  Serial.print(highByte(FCS), HEX);

  return FCS;

}

word FrameFCS () {

  Serial.print(" Frame FCS :");
  Serial.print(PacketIn[FrameLength-1], HEX);
  Serial.print(PacketIn[FrameLength], HEX);
  Serial.print(" Frame length: ");
  Serial.println(FrameLength);

  return PacketIn[FrameLength-1]*16 + PacketIn[FrameLength];

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

    while ((RXbyte != 0x7E) && Timer_Running()) {    // so we keep scanning until we have an incoming flag (0x7E)

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
  
}

void Set_RXTimeOut( long RXTimeOutValue) {

  RX_TimeOut = RXTimeOutValue;

}

void Modem_Off() {

  digitalWrite(M0, HIGH);
  digitalWrite(M1, HIGH);

}

void Modem_RX() {

  digitalWrite(M0, LOW);
  digitalWrite(M1, HIGH);

  delay(25);

}

void Modem_TX() {

  digitalWrite(M1, LOW);
  digitalWrite(M0, HIGH);

  delay(25);

}


boolean Carrier_Detected() {

  return (digitalRead(DET));

}

int HDLC_Frame_Available() {

  long FCS_from_Calc;
  long FCS_from_Frame;

  int k = 0;

  Stuff = 0;

  if (Carrier_Detected()) {          //signal detected, we should have flag(s) incoming

    DET_Time = millis();

    ByteIn = ReceiveByte (true);

    Flag = (ByteIn == AX25_Flag); 

    if (Flag) {

      Serial.print("Channel: ");

      while (Flag) {

        ByteIn = ReceiveByte (false);

        Flag = (ByteIn == AX25_Flag);

      }

      while ( (k <= MaxFrameLgth) && !Flag ) {

        PacketIn [k] = ByteIn;

        k++;

        ByteIn = ReceiveByte(false);

        //Serial.print(ByteIn, HEX);
        //Serial.print(" | ");

        Flag = ByteIn == 0x7E;

      }

      Serial.println();

      FrameLength = k-1;

      FCS_from_Calc = CalculatedFCS();
      FCS_from_Frame = FrameFCS();

      Serial.print("from calc: ");
      Serial.print(FCS_from_Calc);
      Serial.print("  from frame: ");
      Serial.println(FCS_from_Frame);

      if (FCS_from_Calc == FCS_from_Frame) {return FrameLength;} else {return FrameLength*(-1);}

    } else return 0;  // no flag received within the timeframe}

  } else return 0;   // no carrier detected
  
}

byte Get_HDLC_Frame(int i) {

  return PacketIn[i];

}

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte) {


}
