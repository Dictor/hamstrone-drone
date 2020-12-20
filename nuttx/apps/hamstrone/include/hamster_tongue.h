#ifndef HAMSTERTONGUE_H
#define HAMSTERTONGUE_H

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define HAMSTERTONGUE_MESSAGE_MARKER 0xFF

#define HAMSTERTONGUE_MESSAGE_VERB_HEARTBEAT 0x00
#define HAMSTERTONGUE_MESSAGE_VERB_VALUE 0x01

/*
message serialize format: 
Name        Marker  Length  Verb    Noun    Payload     CRC    
Size(byte)  1       1       1       1       ~255        1
            <-              Message Length              ->
                            <-      Length              ->
*/

typedef struct _HAMSTERTONGUE_Message {
    uint8_t Verb;
    uint8_t Noun;
    uint8_t* Payload;
    uint8_t PayloadLength;
} HAMSTERTONGUE_Message;

HAMSTERTONGUE_Message* HAMSTERTONGUE_NewMessage(uint8_t verb, uint8_t noun, uint8_t payloadLength);
uint16_t HAMSTERTONGUE_GetMessageLength(HAMSTERTONGUE_Message* msg);
uint8_t* HAMSTERTONGUE_SerializeMessage(HAMSTERTONGUE_Message* msg);
ssize_t HAMSTERTONGUE_WriteMessage(HAMSTERTONGUE_Message* msg);
#endif
