#ifndef REDMCSB_CEDTINC8_DUNGEON_WRITE_PC34_COMPAT_H
#define REDMCSB_CEDTINC8_DUNGEON_WRITE_PC34_COMPAT_H

#include "redmcsb_f7063_dungeon_stream_pc34_compat.h"

#include <stddef.h>

/* ReDMCSB CEDTINC8.C SAVE_GAME + CEDTINC6.C F7060, PC34 dungeon output. */

/*
 * Emits F7060's fixed 22-part dungeon stream followed by its little-endian
 * 16-bit checksum. All parts stay opaque; no dungeon layout is decoded.
 */
int redmcsb_cedtinc8_write_dungeon_stream_pc34(
    const RedmcsbF7063DungeonPart parts[REDMCSB_F7063_DUNGEON_PART_COUNT],
    uint8_t *written_bytes, size_t written_bytes_size, size_t *written_size);

const char *redmcsb_cedtinc8_dungeon_write_pc34_source_evidence(void);

#endif
