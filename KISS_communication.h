#ifndef KISS_COMMUNICATION_H
#define KISS_COMMUNICATION_H
#include <Arduino.h>

void Begin_KISS_port();           // initialize Bluetooth serial port

int KISS_frame_available();       // reads KISS frame from serial buffer and returns the length of the frame

byte Get_KISS_Frame(int i);       // returns the KISS frame byte from the position given in the argument

void Send_KISS_Frame(byte ByteToSend, boolean LastByte);    // sends KISS frame, frame bytes are to be provided as argument one by one, boolean argument indicates if this is the last byte

#endif