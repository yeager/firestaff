#include "theron_v1_track02_creature_spawn.h"

#include <string.h>

/* Source: authenticated raw MODE1/2352 Track 02 BIN.  These are user-data
 * offsets after removing each sector's 16-byte physical header. */
enum {
    THERON_SPAWN_SOURCE_POINTER_US_UD = 0x274018u,
    THERON_SPAWN_SOURCE_POINTER_JP_UD = 0x273818u
};

static const uint32_t g_spawn_source_zone_us_ud[THERON_TRACK02_SPAWN_ZONE_COUNT] = {
    0x274058u, 0x2740d7u, 0x274102u, 0x274129u, 0x274150u
};

static const uint32_t g_spawn_source_zone_jp_ud[THERON_TRACK02_SPAWN_ZONE_COUNT] = {
    0x273858u, 0x2738d7u, 0x273902u, 0x273929u, 0x273950u
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

static int theron_track02_source_marker(const uint8_t *raw, size_t raw_bytes,
                                        Theron_V1Track02Variant variant) {
    static const uint8_t us_roster[] = {
        'A','K','U','T','U','B','A',0x01,
        'D','R','A','T','O','R',' ',0x01,
        'F','O','R','M','I','C',' ',0x01,
        'S','A','R','M','O','N',' ',0x01,
        'S','H','A','D','O',' ',' ',0x01,
        'T','H','I','E','F',' ',' ',0x01,
        'D','E','M','O','N',' ',' ',0x00
    };
    /* First three complete Shift-JIS display-name records and their original
     * $8f terminators from the authenticated JP retail BIN. */
    static const uint8_t jp_roster_prefix[] = {
        0x4e,0x82,0x60,0x82,0x6a,0x82,0x74,0x82,0x73,0x82,0x74,0x82,
        0x61,0x82,0x60,0x81,0x8f,0x82,0x63,0x82,0x71,0x82,0x60,0x82,
        0x73,0x82,0x6e,0x82,0x71,0x81,0x40,0x81,0x8f,0x82,0x65,0x82,
        0x6e,0x82,0x71,0x82,0x6c,0x82,0x68,0x82,0x62,0x81,0x40,0x81,
        0x8f
    };
    const uint8_t *roster;
    size_t roster_size;
    uint32_t roster_offset;
    size_t i;
    uint8_t value;

    if (variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        roster = us_roster;
        roster_size = sizeof(us_roster);
        roster_offset = 0x2741efu;
    } else if (variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        roster = jp_roster_prefix;
        roster_size = sizeof(jp_roster_prefix);
        roster_offset = 0x2739efu;
    } else return 0;
    if (!theron_track02_source_span(raw, raw_bytes, roster_offset,
                                    roster_size)) return 0;
    for (i = 0; i < roster_size; ++i) {
        if (!theron_track02_raw_user_byte(raw, raw_bytes,
                                          roster_offset + (uint32_t)i, &value) ||
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
    const uint32_t *zone_offsets = NULL;
    uint32_t pointer_offset = 0u;
    unsigned int i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        expected_md5 = THERON_V1_TRACK02_MD5_US_BIN;
        pointer_offset = THERON_SPAWN_SOURCE_POINTER_US_UD;
        zone_offsets = g_spawn_source_zone_us_ud;
    } else if (variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        expected_md5 = THERON_V1_TRACK02_MD5_JP_BIN;
        pointer_offset = THERON_SPAWN_SOURCE_POINTER_JP_UD;
        zone_offsets = g_spawn_source_zone_jp_ud;
    } else return 0;
    if (!raw_track02 || !theron_v1_track02_raw_bytes_match_md5(
            raw_track02, raw_track02_bytes, expected_md5) ||
        !theron_track02_source_span(raw_track02, raw_track02_bytes,
                                    pointer_offset,
                                    THERON_TRACK02_SPAWN_POINTER_COUNT * 8u) ||
        !theron_track02_source_marker(raw_track02, raw_track02_bytes, variant))
        return 0;

    for (i = 0; i < THERON_TRACK02_SPAWN_POINTER_COUNT; ++i) {
        uint32_t source = pointer_offset + i * 8u;
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
        uint32_t source = zone_offsets[i];
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

int theron_v1_track02_bind_spawn_consumer_source(
    const uint8_t *raw_track02,
    size_t raw_track02_bytes,
    Theron_V1Track02Variant variant,
    Theron_Track02SpawnConsumerSourceReceipt *out) {
    const char *expected_md5;
    uint32_t offset;
    uint32_t expected_checksum;
    uint32_t hash = 2166136261u;
    uint8_t value;
    unsigned int i;
    enum { CONSUMER_BYTES = 0x10du };

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (variant == THERON_V1_TRACK02_VARIANT_US_BIN) {
        expected_md5 = THERON_V1_TRACK02_MD5_US_BIN;
        offset = 0x0870e5u;
        expected_checksum = 0xeb241d19u;
    } else if (variant == THERON_V1_TRACK02_VARIANT_JP_BIN) {
        expected_md5 = THERON_V1_TRACK02_MD5_JP_BIN;
        offset = 0x0868d2u;
        expected_checksum = 0x7dc1e453u;
    } else return 0;
    if (!raw_track02 || !theron_v1_track02_raw_bytes_match_md5(
            raw_track02, raw_track02_bytes, expected_md5) ||
        !theron_track02_source_span(raw_track02, raw_track02_bytes,
                                    offset, CONSUMER_BYTES)) return 0;
    for (i = 0u; i < CONSUMER_BYTES; ++i) {
        if (!theron_track02_raw_user_byte(raw_track02, raw_track02_bytes,
                                          offset + i, &value)) return 0;
        hash ^= value;
        hash *= 16777619u;
    }
    if (hash != expected_checksum) return 0;
    out->valid = 1;
    out->variant = variant;
    out->user_data_offset = offset;
    out->byte_count = CONSUMER_BYTES;
    out->checksum = hash;
    out->regional_code_verified = 1;
    out->runtime_execution_proven = 0;
    out->category_semantics_proven = 0;
    return 1;
}
