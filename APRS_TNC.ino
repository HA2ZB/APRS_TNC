// Main code for APRS TNC with Arduino Mega 2560 board and MX/FX614 modem
// Bela Zagyva, HA2ZB

#include "KISS_communication.h"   // include custom developed library for KISS communication with APRSDroid app via Bluetooth serial interface
#include "HDLC_MX614.h"           // include custom developed library for HDLC layer communication via MX/FX614 modem

const int MaxAPRSFrameLght = 340;     // the total length of an APRS frame is 332 bytes (incl. flags) so we can reduce this a bit if needed

byte APRS_Frame[MaxAPRSFrameLght];    // we devine the byte array variable for processing APRS frames to be sent / received

boolean Last_APRS_Byte;               // we will use this boolean flag to indicate the very last byte in the frame to be sent

boolean FrameError;                   // frame error flag used at the reception

int APRS_Frame_Length = 0;            // variable used for detecting the length of incoming frames

int APRS_Frame_Length_TX = 0;         // variable used for detecting the length of outgoing frames


// setup sequence for TNC initialization

void setup() {
  
  Begin_KISS_port();    // we initialize the BT interface

  Begin_Modem();        // we initialize the modem

  Set_TXTimeOut(500);   // max. time the transmission is on hold in case carrier detected

  Set_RXTimeOut(500);   // max. time we try to find a valid APRS frame after carrier detected

  Set_PTT_Delay(500);   // time between PTT on and strating the AFSK transmission

  Modem_Off();          // we put the modem in idle state

}

// loop sequence for operation

void loop() {

  Modem_RX();   // we set the modem to RX mode - watching the channel

  APRS_Frame_Length = HDLC_Frame_Available();   // the HDLC_Frame_Available() fuction returns a non zero integer in case frame received from the modem (radio)

  if (APRS_Frame_Length != 0) {                 // if the return value is not 0

    FrameError = APRS_Frame_Length < 0;         // if there is an FCS error (the calculated valu is different then the received one), the return value is negative

    if (!FrameError) {                          // if there is no frame error, we start processing the received valid frame

      for (int i=0; i <= APRS_Frame_Length; i++) {  
      
        APRS_Frame[i] = Get_HDLC_Frame(i);      // we read the HDLC frame bytes to our APRS frame array, byte to byte

      }

      for (int i=0; i<= APRS_Frame_Length-3; i++) {         // this loop sends the frame to the KISS BT interface (to APRSDroid client) except the last 2 bytes, as we drop the FCS bytes

        Last_APRS_Byte = (i == (APRS_Frame_Length - 3));    // we flag the last byte

        Send_KISS_Frame(APRS_Frame[i], Last_APRS_Byte);     // we gave over the frame byte and the last byte flag to the sending function

      }   // sending complete
      
    }
    
  }
  
  APRS_Frame_Length_TX = KISS_frame_available();    // the same way we regurarly check if there is any incoming communication from APRSDroid via BT

  if (APRS_Frame_Length_TX > 0) {                   // if the return value is not 0 (will not be nagative as there are no FCS / CRC in KISS)

    delay(500);                                     // 500 microsec delay inserted for testing, to be checked if needed...

    Reset_TX_Counter();                             // we reset the HDLC frame counter to zero (explained in the HDLC library)

    for (int i=1; i<=APRS_Frame_Length_TX; i++) {   // just like we did in the incoming HDLC frame, we forward the KISS frame to the HDLC sending function
                                                    // we start the counter by 1 and not 0 az the first KISS frame byte is 0 (other commands are specified in KISS protocol, but we do not use them, so we assume it is 0)

      Last_APRS_Byte = ( i == APRS_Frame_Length_TX );   // we flag the last byte to send

      Send_HDLC_Frame( Get_KISS_Frame(i), Last_APRS_Byte );   // we give over the byte to send and the last byte flag to the sending function

    }

    APRS_Frame_Length_TX = 0;     // we reset the HDLC counter to zero (ready for a new frame)
    
  } 

}

// End of main code