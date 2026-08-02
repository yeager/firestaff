#ifndef THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H
#define THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * 7 per-level graphics/tile banks (~256KB each = one PCE HuCard bank).
 * Identified by byte-exact 232-byte shared prologue signature.
 * Each block starts with a 0xF0-byte prologue (0xE8 bytes shared resource
 * table + 8 bytes per-level metadata), followed by level-specific tile
 * pattern data for the PCE VDC (4bpp 8x8 tiles).
 *
 * These are NOT the dungeon map layout (wall/floor topology) — that data
 * format is still unidentified within Track 02.
 *
 * UD offsets discovered via full-track signature scan. */

#define THERON_TRACK02_LEVEL_COUNT  7u
#define THERON_TRACK02_LEVEL_PROLOGUE_SIZE  0xF0u
#define THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE  0xE8u

typedef struct {
    uint32_t ud_offset;
    uint8_t  per_level_meta[8];
} Theron_LevelDataBlockDesc;

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block(unsigned int level);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_TRACK02_LEVEL_DATA_BLOCKS_H */
