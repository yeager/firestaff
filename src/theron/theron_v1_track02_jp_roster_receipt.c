#include "theron_v1_track02_jp_roster_receipt.h"

#include "theron_v1_track02.h"
#include "theron_v1_dungeon_handoff.h"

#include <string.h>

#define THERON_JP_ROSTER_RAW_OFFSET 0x0b3d98u
#define THERON_JP_CUE_INDEX01_SECTORS 224u

static const char *const g_names[THERON_TRACK02_JP_ROSTER_COUNT] = {
    "THERON", "MARA", "LINOS", "HEXA", "HAKAR", "TIRAN", "DOTAN",
    "PENTAI"
};

static int read_line(const uint8_t *bytes, size_t size, size_t *cursor,
                     char *out, size_t out_capacity) {
    size_t start;
    size_t length;

    if (!bytes || !cursor || !out || out_capacity == 0u || *cursor >= size) {
        return 0;
    }
    start = *cursor;
    while (*cursor < size && bytes[*cursor] != '\n') {
        ++*cursor;
    }
    if (*cursor >= size) return 0;
    length = *cursor - start;
    ++*cursor;
    if (length >= out_capacity) return 0;
    memcpy(out, bytes + start, length);
    out[length] = '\0';
    return 1;
}

static int decode_nibbles(const char *line, size_t length,
                          uint8_t *out, size_t out_count) {
    if (!line || !out || length != out_count * 2u) return 0;
    for (size_t i = 0u; i < out_count; ++i) {
        unsigned int high = (unsigned int)(unsigned char)line[i * 2u];
        unsigned int low = (unsigned int)(unsigned char)line[i * 2u + 1u];
        if (high < 'A' || high > 'P' || low < 'A' || low > 'P') return 0;
        out[i] = (uint8_t)(((high - 'A') << 4u) | (low - 'A'));
    }
    return 1;
}

static int decode_nibble_values(const char *line, size_t length,
                                uint8_t *out, size_t out_count) {
    if (!line || !out || length != out_count) return 0;
    for (size_t i = 0u; i < out_count; ++i) {
        unsigned int value = (unsigned int)(unsigned char)line[i];
        if (value < 'A' || value > 'P') return 0;
        out[i] = (uint8_t)(value - 'A');
    }
    return 1;
}

int theron_v1_track02_jp_roster_read(
    const uint8_t *track02_data,
    size_t track02_size,
    const char *md5_hex,
    Theron_Track02JpRosterReceipt out_records[
        THERON_TRACK02_JP_ROSTER_COUNT]) {
    size_t cursor = THERON_JP_ROSTER_RAW_OFFSET;
    size_t raw_cursor = THERON_JP_ROSTER_RAW_OFFSET;
    size_t raw_sector = THERON_JP_ROSTER_RAW_OFFSET /
        THERON_TRACK02_RAW_SECTOR_BYTES;
    size_t raw_sector_offset = THERON_JP_ROSTER_RAW_OFFSET %
        THERON_TRACK02_RAW_SECTOR_BYTES;
    int cue_iso = md5_hex &&
        strcmp(md5_hex, THERON_TRACK02_MD5_JP_ISO) == 0;

    if (!out_records) return 0;
    memset(out_records, 0,
           sizeof(Theron_Track02JpRosterReceipt) *
               THERON_TRACK02_JP_ROSTER_COUNT);
    if (!track02_data || !md5_hex ||
        (strcmp(md5_hex, THERON_TRACK02_MD5_JP_BIN) != 0 && !cue_iso) ||
        !theron_v1_track02_raw_bytes_match_md5(
            track02_data, track02_size, md5_hex)) {
        return 0;
    }
    if (cue_iso) {
        size_t cue_sector;
        size_t cue_sector_offset;
        if (raw_sector < THERON_JP_CUE_INDEX01_SECTORS ||
            raw_sector_offset < THERON_TRACK02_RAW_USER_DATA_OFFSET) {
            return 0;
        }
        cue_sector = raw_sector - THERON_JP_CUE_INDEX01_SECTORS;
        cue_sector_offset = raw_sector_offset -
            THERON_TRACK02_RAW_USER_DATA_OFFSET;
        if (cue_sector > SIZE_MAX / THERON_TRACK02_RAW_USER_DATA_BYTES ||
            cue_sector * THERON_TRACK02_RAW_USER_DATA_BYTES >
                SIZE_MAX - cue_sector_offset) {
            return 0;
        }
        cursor = cue_sector * THERON_TRACK02_RAW_USER_DATA_BYTES +
            cue_sector_offset;
        if (cursor >= track02_size ||
            raw_sector_offset >= THERON_TRACK02_RAW_SECTOR_BYTES) {
            return 0;
        }
    } else if (track02_size <= THERON_JP_ROSTER_RAW_OFFSET) {
        return 0;
    }

    for (unsigned int index = 0u;
         index < THERON_TRACK02_JP_ROSTER_COUNT; ++index) {
        Theron_Track02JpRosterReceipt *record = &out_records[index];
        char line[64];
        uint8_t decoded[7];
        size_t view_record_start = cursor;

        record->index = index;
        record->raw_offset = (uint32_t)raw_cursor;
        if (!read_line(track02_data, track02_size, &cursor,
                       record->name, sizeof(record->name)) ||
            strcmp(record->name, g_names[index]) != 0) {
            return 0;
        }
        record->title_raw_offset = record->raw_offset +
            (uint32_t)strlen(record->name) + 1u;
        if (!read_line(track02_data, track02_size, &cursor,
                       record->title, sizeof(record->title)) ||
            !read_line(track02_data, track02_size, &cursor,
                       line, sizeof(line)) ||
            strlen(line) > 1u) {
            return 0;
        }
        record->class_code = line[0] ? line[0] : '\0';
        if (!read_line(track02_data, track02_size, &cursor,
                       line, sizeof(line)) ||
            strlen(line) != 1u || (line[0] != 'M' && line[0] != 'F')) {
            return 0;
        }
        record->sex = line[0];
        if (!read_line(track02_data, track02_size, &cursor,
                       line, sizeof(line)) ||
            !decode_nibbles(line, strlen(line), decoded, 6u)) {
            return 0;
        }
        record->hp = (uint16_t)(((uint16_t)decoded[0] << 8u) | decoded[1]);
        record->stamina =
            (uint16_t)(((uint16_t)decoded[2] << 8u) | decoded[3]);
        record->mana =
            (uint16_t)(((uint16_t)decoded[4] << 8u) | decoded[5]);
        if (!read_line(track02_data, track02_size, &cursor,
                       line, sizeof(line)) ||
            !decode_nibbles(line, strlen(line), record->attributes, 7u) ||
            !read_line(track02_data, track02_size, &cursor,
                       line, sizeof(line)) ||
            !decode_nibble_values(line, strlen(line), record->skills, 16u) ||
            cursor >= track02_size || track02_data[cursor++] != 0u) {
            return 0;
        }
        if (cue_iso) {
            size_t consumed = cursor - view_record_start;
            size_t raw_sector_end =
                (raw_sector + 1u) * THERON_TRACK02_RAW_SECTOR_BYTES;
            if (consumed > raw_sector_end - raw_cursor ||
                cursor > track02_size) {
                return 0;
            }
            raw_cursor += consumed;
        } else {
            raw_cursor = cursor;
        }
        record->next_raw_offset = (uint32_t)raw_cursor;
        record->valid = 1;
    }
    return 1;
}
