#include "csb_v1_csbwin_dungeon_tail.h"

#include <string.h>

static uint16_t read_be16(const uint8_t *p)
{
    return (uint16_t)(((uint16_t)p[0] << 8) | p[1]);
}

static int add_size(size_t *value, size_t add, size_t limit)
{
    if (!value || add > limit - *value) return 0;
    *value += add;
    return 1;
}

int csb_v1_csbwin_dungeon_tail_parse_prefix(
    const uint8_t *tail,
    size_t tail_size,
    uint8_t extended_flags,
    CSB_V1_CSBWinDungeonTailPrefix *out)
{
    CSB_V1_CSBWinDungeonTailPrefix report;
    size_t offset;
    size_t level_bytes;
    size_t i;
    size_t text_bytes;

    if (!tail || !out) return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT;
    memset(&report, 0, sizeof(report));
    if (tail_size < CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }

    report.dungeon_index_offset = 0u;
    report.sentinel = read_be16(tail + 0u);
    report.legacy_cell_flag_bytes = read_be16(tail + 2u);
    report.level_count = (uint8_t)(read_be16(tail + 4u) >> 8u);
    report.text_word_count = read_be16(tail + 6u);
    report.object_list_length = read_be16(tail + 10u);
    for (i = 0u; i < 16u; ++i) {
        report.database_entries[i] = read_be16(tail + 12u + i * 2u);
    }
    if (report.level_count == 0u ||
        report.level_count > CSB_V1_CSBWIN_MAX_SAVE_LEVELS) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT;
    }

    offset = CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES;
    report.level_descriptors_offset = offset;
    level_bytes = (size_t)report.level_count *
                  CSB_V1_CSBWIN_LEVEL_DESC_BYTES;
    if (!add_size(&offset, level_bytes, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    for (i = 0u; i < report.level_count; ++i) {
        const uint16_t word8 = read_be16(
            tail + report.level_descriptors_offset +
            i * CSB_V1_CSBWIN_LEVEL_DESC_BYTES + 8u);
        const uint16_t columns = (uint16_t)(((word8 >> 6u) & 0x1fu) + 1u);
        if (columns > UINT16_MAX - report.column_pointer_count) {
            return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW;
        }
        report.level_last_column[i] = (uint16_t)(columns - 1u);
        report.column_pointer_count =
            (uint16_t)(report.column_pointer_count + columns);
    }

    report.object_list_index_offset = offset;
    if (!add_size(&offset, (size_t)report.column_pointer_count * 2u,
                  tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    report.object_list_offset = offset;
    if (!add_size(&offset, (size_t)report.object_list_length * 2u,
                  tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }

    report.text_offset = offset;
    report.indirect_text =
        (extended_flags & CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT) != 0u;
    text_bytes = (size_t)report.text_word_count *
                 (report.indirect_text ? 4u : 2u);
    if (!add_size(&offset, text_bytes, tail_size)) {
        return CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED;
    }
    report.next_database_offset = offset;
    report.valid = 1;
    *out = report;
    return CSB_V1_CSBWIN_DUNGEON_TAIL_OK;
}

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void)
{
    return "CSBWin SaveGame.cpp:1236-1337,2536-2840; "
           "CSBWin CSB.h:DUNGEONDATINDEX,LEVELDESC";
}
