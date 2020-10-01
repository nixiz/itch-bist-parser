/*
 * MoldUDP protocol messages
 *
 * The implementation is based on the following specifications provided
 * by NASDAQ OMX:
 *
 *   MoldUDP
 *   Version 1.02a
 *   October 19, 2006
 *
 * and
 *
 *   MoldUDP for NASDAQ OMX Nordic
 *   Version 1.0.1
 *   February 10, 2014
 */

#ifndef HELIX_NASDAQ_MOLDUDP_PROTO_H
#define HELIX_NASDAQ_MOLDUDP_PROTO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

#ifdef _WIN32
#pragma pack(push,1)
#define pack_attr
#else
#define pack_attr __attribute__((packed))
#endif // WIN32


struct moldudp_header {
    char     Session[10];
    uint32_t SequenceNumber;
    uint16_t MessageCount;
} pack_attr;

struct moldudp_message_block {
    uint16_t MessageLength;
} pack_attr;

#ifdef _WIN32
#pragma pack(pop)
#endif // WIN32

#ifdef __cplusplus
}
#endif

#endif