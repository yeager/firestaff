/*
 * Read-only framing for the CSBWin saved-dungeon tail.
 *
 * Source: CSBWin SaveGame.cpp ReadDatabases()/WriteAndChecksum(), and
 * CSB.h DUNGEONDATINDEX/LEVELDESC.  This deliberately stops before DB0..15:
 * their ownership belongs to the live CSB dungeon/object runtime.
 */
#ifndef FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H
#define FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES = 44u,
    CSB_V1_CSBWIN_LEVEL_DESC_BYTES = 16u,
    CSB_V1_CSBWIN_MAX_SAVE_LEVELS = 64u,
    CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT = 0x08u
};

typedef enum {
    CSB_V1_CSBWIN_DUNGEON_TAIL_OK = 0,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED = -2,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT = -3,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW = -4
} CSB_V1_CSBWinDungeonTailResult;

typedef struct {
    int valid;
    uint16_t sentinel;
    uint16_t legacy_cell_flag_bytes;
    uint8_t level_count;
    uint16_t text_word_count;
    uint16_t object_list_length;
    uint16_t database_entries[16];
    uint16_t level_last_column[CSB_V1_CSBWIN_MAX_SAVE_LEVELS];
    uint16_t column_pointer_count;
    int indirect_text;
    size_t dungeon_index_offset;
    size_t level_descriptors_offset;
    size_t object_list_index_offset;
    size_t object_list_offset;
    size_t text_offset;
    size_t next_database_offset;
} CSB_V1_CSBWinDungeonTailPrefix;

/* Parse the unencrypted tail immediately following the authenticated
 * GAMEBLOCK1/2 streams. `extended_flags` is from Extended Features. */
int csb_v1_csbwin_dungeon_tail_parse_prefix(
    const uint8_t *tail,
    size_t tail_size,
    uint8_t extended_flags,
    CSB_V1_CSBWinDungeonTailPrefix *out);

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
