#ifndef KISS_COMMUNICATION_H
#define KISS_COMMUNICATION_H
#include <Arduino.h>

void Begin_KISS_port();

int KISS_frame_available();

byte Get_KISS_Frame(int i);

void Send_KISS_Frame(byte ByteToSend, boolean LastByte);

#endif