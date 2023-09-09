#ifndef HDLC_MX614_H
#define HDLC_MX614_H
#include <Arduino.h>

void Begin_Modem();       // initializes modem and connecting control ports

void Set_RXTimeOut( long RXTimeOutValue);   // sets the time in ms the TNC waits for HDLC frame after carrier detected

void Set_TXTimeOut( long TXTimeoutValue);   // sets the time in ms the TNC waits to send HDLC frame if cahnnel is busy (carrier detected)

void Modem_Off();   // puts the modem to idle state

void Modem_RX();    // puts the modem to RX mode

void Modem_TX();    // puts teh modem to TX mode

void PTT_On();      // keys radio PTT connection

void PTT_Off();     // unkeys radio PTT connection

void Set_PTT_Delay(int PTT_Delay_Value);    // delay in ms between PTT and transmission

boolean Carrier_Detected();       // true if carrier detected in the voice channel

int HDLC_Frame_Available();       // checks for and receives HDLC frame, returns the length of the received HDLC frame

byte Get_HDLC_Frame(int i);       // returns a HDLC byte from the position given in the argument from received frame 

void Reset_TX_Counter();          // resets (to zero) the TX byte counter

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte);    // sends HDLC frame, frame bytes are to be provided as argument one by one, boolean argument indicates if this is the last byte

#endif