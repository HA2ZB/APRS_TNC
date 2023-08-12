#ifndef HDLC_MX614_H
#define HDLC_MX614_H
#include <Arduino.h>

void Begin_Modem();

void Set_RXTimeOut( long RXTimeOutValue);

void Set_TXTimeOut( long TXTimeoutValue);

void Modem_Off();

void Modem_RX();

void Modem_TX();

void PTT_On();

void PTT_Off();

void Set_PTT_Delay(int PTT_Delay_Value);

boolean Carrier_Detected();

int HDLC_Frame_Available();

byte Get_HDLC_Frame(int i);

void Reset_TX_Counter();

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte);

#endif