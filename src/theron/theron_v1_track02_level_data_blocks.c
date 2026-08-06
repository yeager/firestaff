#include "theron_v1_track02_level_data_blocks.h"
#include <string.h>

static uint32_t fnv1a(const uint8_t *bytes, size_t count) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0u; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

/* Sources: US Track 02 BIN (MD5 f23601102138f87c33025877767ebf76) and JP
 * Track 02 BIN (MD5 b7afb338ad31be1025b53f9aff12d73a).
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

static const Theron_LevelDataBlockDesc g_jp_level_blocks[THERON_TRACK02_LEVEL_COUNT] = {
    /* Level 1 */ { 0x09E82F, { 0x07, 0x87, 0x18, 0x10, 0x10, 0x10, 0x10, 0x20 } },
    /* Level 2 */ { 0x0DEB71, { 0x07, 0x87, 0x04, 0x02, 0x04, 0x02, 0x04, 0x02 } },
    /* Level 3 */ { 0x11E82F, { 0x0F, 0x87, 0x0F, 0x04, 0x00, 0x01, 0x01, 0x01 } },
    /* Level 4 */ { 0x15EED7, { 0x03, 0x83, 0x3F, 0x3F, 0x3F, 0x07, 0x1F, 0x0F } },
    /* Level 5 */ { 0x19EE9B, { 0x0F, 0x87, 0x1E, 0x1E, 0x1E, 0x1F, 0x3F, 0x3F } },
    /* Level 6 */ { 0x1DE82F, { 0x07, 0x87, 0x07, 0x0C, 0x10, 0x30, 0x20, 0x20 } },
    /* Level 7 */ { 0x21E82F, { 0x0F, 0x87, 0x10, 0x10, 0x10, 0x00, 0x20, 0x01 } },
};

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block(unsigned int level) {
    return theron_v1_track02_level_data_block_for_variant(
        THERON_TRACK02_VARIANT_US_BIN, level);
}

const Theron_LevelDataBlockDesc *theron_v1_track02_level_data_block_for_variant(
    Theron_Track02Variant variant, unsigned int level) {
    if (level >= THERON_TRACK02_LEVEL_COUNT) return NULL;
    if (variant == THERON_TRACK02_VARIANT_US_BIN) return &g_level_blocks[level];
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) return &g_jp_level_blocks[level];
    return NULL;
}

int theron_v1_track02_level_data_block_read(
    const uint8_t *user_data, size_t user_data_size,
    Theron_Track02Variant variant, unsigned int level,
    Theron_LevelDataBlockReceipt *out) {
    const Theron_LevelDataBlockDesc *block;
    size_t next_offset;
    size_t compressed_offset;
    size_t compressed_end;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!user_data || !theron_v1_track02_level_data_block_for_variant(
            variant, level)) return 0;
    block = theron_v1_track02_level_data_block_for_variant(variant, level);
    if ((size_t)block->ud_offset > user_data_size ||
        THERON_TRACK02_LEVEL_PROLOGUE_SIZE >
            user_data_size - block->ud_offset) return 0;
    next_offset = user_data_size;
    if (level + 1u < THERON_TRACK02_LEVEL_COUNT) {
        const Theron_LevelDataBlockDesc *next =
            theron_v1_track02_level_data_block_for_variant(variant, level + 1u);
        if (!next || next->ud_offset <= block->ud_offset) return 0;
        next_offset = next->ud_offset;
    }
    if (next_offset > user_data_size ||
        next_offset - block->ud_offset < THERON_TRACK02_LEVEL_PROLOGUE_SIZE) {
        return 0;
    }
    compressed_offset = (size_t)block->ud_offset +
                        THERON_TRACK02_LEVEL_PROLOGUE_SIZE;
    compressed_end = next_offset;
    out->valid = 1;
    out->variant = variant;
    out->level = level;
    out->block_ud_offset = block->ud_offset;
    out->compressed_ud_offset = (uint32_t)compressed_offset;
    out->compressed_bytes = compressed_end - compressed_offset;
    out->compressed = user_data + compressed_offset;
    out->compressed_fnv1a = fnv1a(out->compressed, out->compressed_bytes);
    out->shared_prologue_fnv1a = fnv1a(
        user_data + block->ud_offset,
        THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE);
    memcpy(out->per_level_meta, block->per_level_meta,
           sizeof(out->per_level_meta));
    return out->compressed_bytes != 0u && out->compressed_fnv1a != 0u;
}
