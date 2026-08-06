#ifndef THERON_V1_TRACK02_JP_ROSTER_RECEIPT_H
#define THERON_V1_TRACK02_JP_ROSTER_RECEIPT_H

#include <stddef.h>
#include <stdint.h>

#define THERON_TRACK02_JP_ROSTER_COUNT 8u
#define THERON_TRACK02_JP_ROSTER_NAME_CAPACITY 16u
#define THERON_TRACK02_JP_ROSTER_TITLE_CAPACITY 32u

/* Source-only receipt for the real JP Track 02 champion cluster.  The
 * decoded fields are byte-format evidence; this API does not authorize a
 * US text route, portraits, or live gameplay ownership. */
typedef struct {
    int valid;
    unsigned int index;
    uint32_t raw_offset;
    uint32_t next_raw_offset;
    char name[THERON_TRACK02_JP_ROSTER_NAME_CAPACITY];
    char title[THERON_TRACK02_JP_ROSTER_TITLE_CAPACITY];
    char class_code;
    char sex;
    uint16_t hp;
    uint16_t stamina;
    uint16_t mana;
    uint8_t attributes[7];
    uint8_t skills[16];
} Theron_Track02JpRosterReceipt;

/* Reads all eight records from the authenticated raw JP MODE1/2352 BIN.
 * The expected cluster is at raw offset 0xB3D98 and every field is bounded
 * by its newline/NUL framing before it is admitted to the receipt. */
int theron_v1_track02_jp_roster_read(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02JpRosterReceipt out_records[
        THERON_TRACK02_JP_ROSTER_COUNT]);

#endif /* THERON_V1_TRACK02_JP_ROSTER_RECEIPT_H */
