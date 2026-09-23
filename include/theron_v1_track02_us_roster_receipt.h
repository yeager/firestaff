#ifndef THERON_V1_TRACK02_US_ROSTER_RECEIPT_H
#define THERON_V1_TRACK02_US_ROSTER_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_US_ROSTER_COUNT 8u
#define THERON_TRACK02_US_ROSTER_NAME_CAPACITY 16u

typedef struct {
    int valid;
    unsigned int index;
    uint32_t raw_offset;
    uint32_t next_raw_offset;
    char name[THERON_TRACK02_US_ROSTER_NAME_CAPACITY];
    char class_code;
    char sex;
    uint16_t hp;
    uint16_t stamina;
    uint16_t mana;
    uint8_t attributes[7];
    uint8_t skills[16];
} Theron_Track02UsRosterReceipt;

/* Reads the eight complete champion records from the authenticated US raw
 * MODE1/2352 Track 02 BIN.  The source packs three 5-bit values per
 * little-endian word in the 360-byte span 0xB46C8..0xB4830 (FNV-1a
 * 0x39D95C9E). Title/control fields are walked for framing only and are
 * deliberately not published by this receipt. */
int theron_v1_track02_us_roster_read(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02UsRosterReceipt out_records[
        THERON_TRACK02_US_ROSTER_COUNT]);

#endif
