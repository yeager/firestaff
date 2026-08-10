/*
 * Read-only framing for the CSBWin saved-dungeon tail.
 *
 * Source: CSBWin SaveGame.cpp ReadDatabases()/WriteAndChecksum(), CSB.h
 * DUNGEONDATINDEX/LEVELDESC/DB0..DB15, and data.cpp dbEntrySizes. Database
 * bytes remain read-only; this module exposes only their verified spans.
 */
#ifndef FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H
#define FIRESTAFF_CSB_V1_CSBWIN_DUNGEON_TAIL_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_dungeon_loader_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_CSBWIN_DUNGEON_INDEX_BYTES = 44u,
    CSB_V1_CSBWIN_LEVEL_DESC_BYTES = 16u,
    CSB_V1_CSBWIN_MAX_SAVE_LEVELS = 64u,
    CSB_V1_CSBWIN_DATABASE_COUNT = 16u,
    CSB_V1_CSBWIN_MAX_COMPRESSED_TEXT_WORDS = 1000000u,
    CSB_V1_CSBWIN_EXTENDED_FLAG_INDIRECT_TEXT = 0x08u,
    CSB_V1_CSBWIN_EXTENDED_FLAG_BIG_ACTUATORS = 0x80u,
    CSB_V1_CSBWIN_LEGACY_FEATURE_VERSION = '@'
};

typedef enum {
    CSB_V1_CSBWIN_DUNGEON_TAIL_OK = 0,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_ARGUMENT = -1,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TRUNCATED = -2,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LEVEL_COUNT = -3,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_OVERFLOW = -4,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_PREFIX = -5,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_LAYOUT = -6,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_CHECKSUM = -7,
    CSB_V1_CSBWIN_DUNGEON_TAIL_ERR_TEXT_SIZE = -8
} CSB_V1_CSBWinDungeonTailResult;

typedef struct {
    int valid;
    uint16_t sentinel;
    uint16_t legacy_cell_flag_bytes;
    uint8_t level_count;
    uint16_t text_word_count;
    uint16_t object_list_length;
    uint16_t database_entries[CSB_V1_CSBWIN_DATABASE_COUNT];
    uint16_t level_last_column[CSB_V1_CSBWIN_MAX_SAVE_LEVELS];
    uint16_t column_pointer_count;
    int indirect_text;
    uint32_t compressed_text_word_count;
    size_t dungeon_index_offset;
    size_t level_descriptors_offset;
    size_t object_list_index_offset;
    size_t object_list_offset;
    size_t text_offset;
    size_t compressed_text_size_offset;
    size_t compressed_text_offset;
    size_t next_database_offset;
} CSB_V1_CSBWinDungeonTailPrefix;

typedef struct {
    uint8_t database_number;
    uint16_t entry_count;
    uint16_t source_entry_bytes;
    size_t offset;
    size_t byte_count;
} CSB_V1_CSBWinDungeonTailDatabaseSpan;

typedef struct {
    int valid;
    uint8_t extended_features_version;
    uint8_t extended_flags;
    int big_actuators;
    int legacy_scroll_records;
    int cell_flag_size_from_extended_features;
    size_t database_offset;
    size_t database_bytes;
    CSB_V1_CSBWinDungeonTailDatabaseSpan
        database[CSB_V1_CSBWIN_DATABASE_COUNT];
    size_t cell_flags_offset;
    uint32_t cell_flag_bytes;
    size_t checksum_offset;
    uint16_t computed_checksum;
    uint16_t stored_checksum;
} CSB_V1_CSBWinDungeonTailDatabaseLayout;

/* Parse the unencrypted tail immediately following the authenticated
 * GAMEBLOCK1/2 streams. `extended_flags` is from Extended Features. */
int csb_v1_csbwin_dungeon_tail_parse_prefix(
    const uint8_t *tail,
    size_t tail_size,
    uint8_t extended_flags,
    CSB_V1_CSBWinDungeonTailPrefix *out);

/* Verify the DB0..DB15 spans after a parsed prefix without decoding or
 * publishing any record. SaveGame.cpp selects an 8-byte DB3 when
 * BigActuators is clear and a 4-byte DB7 before Extended Features version B;
 * every other source entry size is fixed by data.cpp dbEntrySizes.
 *
 * `extended_cell_flag_bytes` is EXTENDEDFEATURESBLOCK::cellFlagArraySize.
 * Zero selects DUNGEONDATINDEX::LegacyCellFlagArraySize, matching
 * ReadDatabases. The sixteen spans, cell flags, and terminal checksum must
 * consume the tail exactly. On any error `out` is unchanged. */
int csb_v1_csbwin_dungeon_tail_parse_databases(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    uint8_t extended_features_version,
    uint8_t extended_flags,
    uint32_t extended_cell_flag_bytes,
    CSB_V1_CSBWinDungeonTailDatabaseLayout *out);

/* CSBWin SaveGame.cpp WriteAndChecksum()/ReadDatabases() carries a running
 * unsigned-byte checksum over the tail and stores its final u16 in big-endian
 * order. Returns 1 for a verified tail, 0 for a checksum mismatch, and -1
 * for an invalid argument or a tail without the terminal checksum word. */
int csb_v1_csbwin_dungeon_tail_validate_checksum(
    const uint8_t *tail,
    size_t tail_size,
    uint16_t *out_computed,
    uint16_t *out_stored);

/* Materialize a legacy CSBWin saved-dungeon tail into the normal source
 * dungeon reader without publishing it to the global dungeon context.
 *
 * CSBWin SaveGame.cpp writes DUNGEONDATINDEX, LEVELDESC, pointer tables and
 * DB0..DB15 in big-endian source order; ReadDatabases() applies the narrowly
 * defined swaps in data.cpp before ownership is published.  This function
 * reproduces precisely those field swaps into a private temporary buffer and
 * invokes Firestaff's source-byte dungeon loader.  The terminal checksum is
 * not part of DUNGEON.DAT and is never passed through.  The caller must have
 * validated `prefix` and `databases` against the same immutable tail.
 *
 * Only the legacy (non-Extended-Features) tail is admitted.  In particular,
 * indirect text and big actuators have no byte-for-byte equivalence with the
 * legacy DUNGEON.DAT consumer and remain fail-closed.  On failure `out` is
 * zeroed; `tail` is never modified and no global dungeon is changed.
 *
 * Source: CSBWin SaveGame.cpp:1236-1337,2512-2896;
 * data.cpp:DUNGEONDATINDEX::Swap/DATABASES::swap; CSB.h DB0..DB15. */
int csb_v1_csbwin_dungeon_tail_load_legacy_source_dungeon(
    const uint8_t *tail,
    size_t tail_size,
    const CSB_V1_CSBWinDungeonTailPrefix *prefix,
    const CSB_V1_CSBWinDungeonTailDatabaseLayout *databases,
    CSB_V1_DungeonData *out);

const char *csb_v1_csbwin_dungeon_tail_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
