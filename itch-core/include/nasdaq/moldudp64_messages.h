/*
 * MoldUDP64 protocol messages
 *
 * The implementation is based on the following specifications provided
 * by NASDAQ:
 *
 *   MoldUDP64 Protocol Specification
 *   V 1.00
 *   07/07/2009
 */

#ifndef HELIX_NASDAQ_MOLDUDP64_MESSAGES_H
#define HELIX_NASDAQ_MOLDUDP64_MESSAGES_H

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

struct moldudp64_header {
    char     Session[10];
    uint64_t SequenceNumber;
    uint16_t MessageCount;
} pack_attr;

struct moldudp64_message_block {
    uint16_t MessageLength;
} pack_attr;

struct moldudp64_request_packet {
    char     Session[10];
    uint64_t SequenceNumber;
    uint16_t MessageCount;
} pack_attr;

#ifdef _WIN32
#pragma pack(pop)
#endif // WIN32

#ifdef __cplusplus
}
#endif

#endif
