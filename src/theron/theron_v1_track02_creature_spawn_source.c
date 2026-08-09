#include "theron_v1_track02_creature_spawn.h"

#include <string.h>

/* Source: authenticated raw MODE1/2352 Track 02 BIN.  These are user-data
 * offsets after removing each sector's 16-byte physical header. */
enum {
    THERON_SPAWN_SOURCE_POINTER_UD = 0x274018u
};

static const uint32_t g_spawn_source_zone_ud[THERON_TRACK02_SPAWN_ZONE_COUNT] = {
    0x274058u, 0x2740d7u, 0x274102u, 0x274129u, 0x274150u
};

static int theron_track02_raw_user_byte(const uint8_t *raw, size_t raw_bytes,
                                         uint32_t user_offset,
                                         uint8_t *out) {
    size_t user_bytes;
    size_t sector;
    size_t in_sector;
    size_t raw_offset;

    if (!raw || !out || raw_bytes == 0u ||
        raw_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u) return 0;
    user_bytes = (raw_bytes / THERON_V1_TRACK02_RAW_SECTOR_BYTES) * 2048u;
    if ((size_t)user_offset >= user_bytes) return 0;
    sector = (size_t)user_offset / 2048u;
    in_sector = (size_t)user_offset % 2048u;
    raw_offset = sector * THERON_V1_TRACK02_RAW_SECTOR_BYTES +
                 THERON_V1_TRACK02_MODE1_HEADER_BYTES + in_sector;
    if (raw_offset >= raw_bytes) return 0;
    *out = raw[raw_offset];
    return 1;
}

static int theron_track02_raw_user_le16(const uint8_t *raw, size_t raw_bytes,
                                        uint32_t user_offset,
                                        uint16_t *out) {
    uint8_t lo, hi;
    if (!out || !theron_track02_raw_user_byte(raw, raw_bytes, user_offset,
                                               &lo) ||
        !theron_track02_raw_user_byte(raw, raw_bytes, user_offset + 1u,
                                      &hi)) return 0;
    *out = (uint16_t)lo | ((uint16_t)hi << 8u);
    return 1;
}

static int theron_track02_source_span(const uint8_t *raw, size_t raw_bytes,
                                      uint32_t user_offset,
                                      size_t span_bytes) {
    size_t user_bytes;
    if (!raw || raw_bytes % THERON_V1_TRACK02_RAW_SECTOR_BYTES != 0u)
        return 0;
    user_bytes = (raw_bytes / THERON_V1_TRACK02_RAW_SECTOR_BYTES) * 2048u;
    if ((size_t)user_offset > user_bytes) return 0;
    return span_bytes <= user_bytes - (size_t)user_offset;
}

static int theron_track02_source_marker(const uint8_t *raw, size_t raw_bytes) {
    static const uint8_t roster[] = {
        'A','K','U','T','U','B','A',0x01,
        'D','R','A','T','O','R',' ',0x01,
        'F','O','R','M','I','C',' ',0x01,
        'S','A','R','M','O','N',' ',0x01,
        'S','H','A','D','O',' ',' ',0x01,
        'T','H','I','E','F',' ',' ',0x01,
        'D','E','M','O','N',' ',' ',0x00
    };
    size_t i;
    uint8_t value;

    if (!theron_track02_source_span(raw, raw_bytes, 0x2741efu,
                                    sizeof(roster))) return 0;
    for (i = 0; i < sizeof(roster); ++i) {
        if (!theron_track02_raw_user_byte(raw, raw_bytes,
                                          0x2741efu + (uint32_t)i, &value) ||
            value != roster[i]) return 0;
    }
    return 1;
}

int theron_v1_track02_decode_spawn_source(
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    Theron_V1Track02Variant variant,
    Theron_Track02SpawnSource *out) {
    const char *expected_md5 = NULL;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    /* The currently authenticated pointer/regular-spawn offsets are the US
     * Track 02 layout documented by the disassembly.  JP has a different
     * asset layout at these offsets and must not be interpreted as US. */
    if (variant == THERON_V1_TRACK02_VARIANT_US_BIN)
        expected_md5 = THERON_V1_TRACK02_MD5_US_BIN;
    else return 0;
    if (!raw_track02 || !theron_v1_track02_raw_bytes_match_md5(
            raw_track02, raw_track02_bytes, expected_md5) ||
        !theron_track02_source_span(raw_track02, raw_track02_bytes,
                                    THERON_SPAWN_SOURCE_POINTER_UD,
                                    THERON_TRACK02_SPAWN_POINTER_COUNT * 8u) ||
        !theron_track02_source_marker(raw_track02, raw_track02_bytes))
        return 0;

    for (i = 0; i < THERON_TRACK02_SPAWN_POINTER_COUNT; ++i) {
        uint32_t source = THERON_SPAWN_SOURCE_POINTER_UD + i * 8u;
        Theron_CreaturePointerEntry *pointer = &out->pointers[i];
        if (!theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 0u, &pointer->sprite_desc_offset) ||
            !theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 2u, &pointer->constant_278a) ||
            !theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 4u, &pointer->spawn_data_offset) ||
            !theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 6u, &pointer->constant_016b)) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
    }
    for (i = 0; i < THERON_TRACK02_SPAWN_ZONE_COUNT; ++i) {
        uint32_t source = g_spawn_source_zone_ud[i];
        Theron_SpawnZoneDesc *zone = &out->zones[i];
        if (!theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 0u, &zone->map_width) ||
            !theron_track02_raw_user_le16(raw_track02, raw_track02_bytes,
                                          source + 2u, &zone->map_height) ||
            !theron_track02_raw_user_byte(raw_track02, raw_track02_bytes,
                                          source + 4u, &zone->category) ||
            !theron_track02_raw_user_byte(raw_track02, raw_track02_bytes,
                                          source + 5u, &zone->count) ||
            !theron_track02_raw_user_byte(raw_track02, raw_track02_bytes,
                                          source + 6u, &zone->param1) ||
            !theron_track02_raw_user_byte(raw_track02, raw_track02_bytes,
                                          source + 7u, &zone->param2)) {
            memset(out, 0, sizeof(*out));
            return 0;
        }
    }
    out->authenticated = 1;
    out->variant = variant;
    return 1;
}
