#include "theron_v1_track02_us_roster_receipt.h"

#include "theron_v1_track02.h"
#include "theron_v1_dungeon_handoff.h"

#include <string.h>

#define THERON_US_ROSTER_RAW_OFFSET 0x0b46c8u
#define THERON_US_ROSTER_RAW_END    0x0b4830u
#define THERON_US_FIELD_SEPARATOR   28u
#define THERON_US_RECORD_END        31u
#define THERON_US_ROSTER_FNV1A      0x39d95c9eu

static const char *const g_names[THERON_TRACK02_US_ROSTER_COUNT] = {
    "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN",
    "PENTAI"
};

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t raw_offset;
    size_t raw_end;
    unsigned int slot;
} UsGlyphCursor;

static uint32_t fnv1a32(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int read_glyph(UsGlyphCursor *cursor, uint8_t *out) {
    uint16_t word;
    unsigned int shift;
    if (!cursor || !out || cursor->raw_offset + 1u >= cursor->size ||
        cursor->raw_offset >= cursor->raw_end || cursor->slot > 2u) {
        return 0;
    }
    word = (uint16_t)cursor->bytes[cursor->raw_offset] |
        (uint16_t)cursor->bytes[cursor->raw_offset + 1u] << 8u;
    shift = 10u - cursor->slot * 5u;
    *out = (uint8_t)((word >> shift) & 0x1fu);
    if (++cursor->slot == 3u) {
        cursor->slot = 0u;
        cursor->raw_offset += 2u;
    }
    return 1;
}

static int read_field(UsGlyphCursor *cursor, uint8_t *out,
                      size_t capacity, size_t *out_count,
                      int *out_record_end) {
    size_t count = 0u;
    uint8_t glyph;
    if (!cursor || !out_count || !out_record_end) return 0;
    *out_record_end = 0;
    while (read_glyph(cursor, &glyph)) {
        if (glyph == THERON_US_FIELD_SEPARATOR ||
            glyph == THERON_US_RECORD_END) {
            *out_count = count;
            *out_record_end = glyph == THERON_US_RECORD_END;
            return 1;
        }
        if (count >= capacity) return 0;
        if (out) out[count] = glyph;
        ++count;
    }
    return 0;
}

static int decode_letters(const uint8_t *glyphs, size_t count,
                          char *out, size_t capacity) {
    if (!glyphs || !out || count + 1u > capacity) return 0;
    for (size_t i = 0u; i < count; ++i) {
        if (glyphs[i] > 25u) return 0;
        out[i] = (char)('A' + glyphs[i]);
    }
    out[count] = '\0';
    return 1;
}

static int decode_pairs(const uint8_t *glyphs, size_t glyph_count,
                        uint8_t *out, size_t out_count) {
    if (!glyphs || !out || glyph_count != out_count * 2u) return 0;
    for (size_t i = 0u; i < out_count; ++i) {
        if (glyphs[i * 2u] > 15u || glyphs[i * 2u + 1u] > 15u) return 0;
        out[i] = (uint8_t)((glyphs[i * 2u] << 4u) |
                           glyphs[i * 2u + 1u]);
    }
    return 1;
}

int theron_v1_track02_us_roster_read(
    const uint8_t *track02_data, size_t track02_size, const char *md5_hex,
    Theron_Track02UsRosterReceipt out_records[
        THERON_TRACK02_US_ROSTER_COUNT]) {
    UsGlyphCursor cursor;
    size_t offset_shift;
    size_t source_offset = THERON_US_ROSTER_RAW_OFFSET;
    size_t source_end = THERON_US_ROSTER_RAW_END;

    if (!out_records) return 0;
    memset(out_records, 0,
           sizeof(*out_records) * THERON_TRACK02_US_ROSTER_COUNT);
    if (!track02_data || !md5_hex ||
        (strcmp(md5_hex, THERON_TRACK02_MD5_US_BIN) != 0 &&
         strcmp(md5_hex, THERON_TRACK02_MD5_US_CLONECD_BIN) != 0 &&
         strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) != 0) ||
        !theron_v1_track02_raw_bytes_match_md5(
            track02_data, track02_size, md5_hex)) {
        return 0;
    }
    offset_shift = strcmp(md5_hex, THERON_TRACK02_MD5_US_CLONECD_BIN) == 0
        ? 225u * THERON_TRACK02_RAW_SECTOR_BYTES
        : 0u;
    if (strcmp(md5_hex, THERON_TRACK02_MD5_US_ISO) == 0) {
        const size_t pregap_sectors = 225u;
        const size_t raw_offsets[2] = {
            THERON_US_ROSTER_RAW_OFFSET, THERON_US_ROSTER_RAW_END
        };
        size_t mapped[2];
        for (size_t i = 0u; i < 2u; ++i) {
            size_t sector = raw_offsets[i] / THERON_TRACK02_RAW_SECTOR_BYTES;
            size_t in_sector = raw_offsets[i] % THERON_TRACK02_RAW_SECTOR_BYTES;
            if (sector < pregap_sectors || in_sector < 16u ||
                in_sector >= 16u + THERON_TRACK02_RAW_USER_DATA_BYTES) {
                return 0;
            }
            mapped[i] = (sector - pregap_sectors) *
                            THERON_TRACK02_RAW_USER_DATA_BYTES +
                        in_sector - 16u;
        }
        source_offset = mapped[0];
        source_end = mapped[1];
    } else {
        source_offset -= offset_shift;
        source_end -= offset_shift;
    }
    if (source_end < source_offset || track02_size < source_end ||
        fnv1a32(track02_data + source_offset,
                THERON_US_ROSTER_RAW_END - THERON_US_ROSTER_RAW_OFFSET) !=
            THERON_US_ROSTER_FNV1A) {
        return 0;
    }
    cursor.bytes = track02_data;
    cursor.size = track02_size;
    cursor.raw_offset = source_offset;
    cursor.raw_end = source_end;
    cursor.slot = 0u;

    for (unsigned int index = 0u;
         index < THERON_TRACK02_US_ROSTER_COUNT; ++index) {
        Theron_Track02UsRosterReceipt *record = &out_records[index];
        uint8_t field[64];
        uint8_t decoded[7];
        size_t count;
        int ended;

        record->index = index;
        record->raw_offset = (uint32_t)cursor.raw_offset;
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended ||
            !decode_letters(field, count, record->name,
                            sizeof(record->name)) ||
            strcmp(record->name, g_names[index]) != 0) return 0;
        /* Title: its 26/27/30 controls remain unresolved. */
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended)
            return 0;
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended ||
            count > 1u || (count == 1u && field[0] > 25u)) return 0;
        record->class_code = count == 1u ? (char)('A' + field[0]) : '\0';
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended ||
            count != 1u || (field[0] != ('M' - 'A') &&
                            field[0] != ('F' - 'A'))) return 0;
        record->sex = (char)('A' + field[0]);
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended ||
            !decode_pairs(field, count, decoded, 6u)) return 0;
        record->hp = (uint16_t)((uint16_t)decoded[0] << 8u | decoded[1]);
        record->stamina =
            (uint16_t)((uint16_t)decoded[2] << 8u | decoded[3]);
        record->mana = (uint16_t)((uint16_t)decoded[4] << 8u | decoded[5]);
        if (!read_field(&cursor, field, sizeof(field), &count, &ended) || ended ||
            !decode_pairs(field, count, record->attributes, 7u) ||
            !read_field(&cursor, field, sizeof(field), &count, &ended) || !ended ||
            count != 16u) return 0;
        for (size_t i = 0u; i < 16u; ++i) {
            if (field[i] > 15u) return 0;
            record->skills[i] = field[i];
        }
        /* An end marker can occur in slot 0 or 1.  The rest of that packed
         * word is padding, and every next record begins at a word boundary. */
        if (cursor.slot != 0u) {
            cursor.slot = 0u;
            cursor.raw_offset += 2u;
        }
        record->next_raw_offset = (uint32_t)cursor.raw_offset;
        record->valid = 1;
    }
    return cursor.raw_offset == cursor.raw_end;
}
