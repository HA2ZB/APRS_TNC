#ifndef HDLC_MX614_H
#define HDLC_MX614_H
#include <Arduino.h>

void Begin_Modem();

void Set_RXTimeOut( long RXTimeOutValue);

void Modem_Off();

void Modem_RX();

void Modem_TX();

boolean Carrier_Detected();

int HDLC_Frame_Available();

byte Get_HDLC_Frame(int i);

void Send_HDLC_Frame(byte ByteToSend, boolean LastByte);

#endif