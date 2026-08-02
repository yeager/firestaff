#include "theron_v1_track02_level_data_blocks.h"

/* Source: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76).
 *
 * 7 level data blocks found by full-track scan for a repeated 232-byte
 * (0xE8) prologue signature. Each block has:
 *   - 0xE8 bytes: shared resource table (identical across all 7 levels)
 *   - 8 bytes: per-level metadata (bytes 0xE8-0xEF)
 *   - Compressed level data starting at offset 0xF0
 *
 * Compression algorithm: under investigation (not standard Okumura LZSS).
 * The decompression routine is in Track 02 code banks (not Track 01,
 * which is CD-DA audio). */

static const Theron_LevelDataBlockDesc g_level_blocks[THERON_TRACK02_LEVEL_COUNT] = {
    /* Level 1 */ { 0x09F000, { 0x07, 0x87, 0x18, 0x10, 0x10, 0x10, 0x10, 0x20 } },
    /* Level 2 */ { 0x0DF342, { 0x03, 0x83, 0x3F, 0x3F, 0x3F, 0x07, 0x1F, 0x0F } },
    /* Level 3 */ { 0x11F000, { 0x0F, 0x87, 0x0F, 0x04, 0x00, 0x01, 0x01, 0x01 } },
    /* Level 4 */ { 0x15F6A8, { 0x07, 0x87, 0x3F, 0x3F, 0x3F, 0x1F, 0x1F, 0x1F } },
    /* Level 5 */ { 0x19F373, { 0x07, 0x87, 0x0F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF } },
    /* Level 6 */ { 0x1DF000, { 0x07, 0x87, 0x07, 0x0C, 0x10, 0x30, 0x20, 0x20 } },
    /* Level 7 */ { 0x21F000, { 0x0D, 0x86, 0x06, 0x30, 0x0F, 0x0E, 0x07, 0x0E } },
};

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block(unsigned int level) {
    if (level >= THERON_TRACK02_LEVEL_COUNT) return NULL;
    return &g_level_blocks[level];
}
