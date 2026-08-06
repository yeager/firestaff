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

/* FNV-1a identities of the complete post-prologue spans in the supplied
 * retail Track 02 BINs. These are media admission gates, not compression
 * output claims. */
static const uint32_t g_us_compressed_fnv1a[THERON_TRACK02_LEVEL_COUNT] = {
    0xf7ccbfe9u, 0xf1a6b37au, 0x3c56f832u, 0xdf34534bu,
    0xa1928360u, 0x64749f2fu, 0x33b93910u
};
static const uint32_t g_jp_compressed_fnv1a[THERON_TRACK02_LEVEL_COUNT] = {
    0xa8818e93u, 0x13142c8fu, 0x4087881au, 0x5bc73358u,
    0x326eff1fu, 0xff96a9afu, 0x930a5bf6u
};

static const uint32_t g_us_iso_compressed_fnv1a[THERON_TRACK02_LEVEL_COUNT] = {
    0xf7ccbfe9u, 0xf1a6b37au, 0x3c56f832u, 0xdf34534bu,
    0xa1928360u, 0x64749f2fu, 0x5c09952du
};
static const uint32_t g_jp_iso_compressed_fnv1a[THERON_TRACK02_LEVEL_COUNT] = {
    0xa8818e93u, 0x13142c8fu, 0x4087881au, 0x5bc73358u,
    0x326eff1fu, 0xff96a9afu, 0xdce9bf6u
};

/* Both authenticated BINs carry the same byte-exact shared 0xE8-byte
 * resource prologue at every later-level anchor.  Keep this as an admission
 * invariant: a compressed-span hash alone would let a damaged shared table
 * masquerade as a valid level record. */
static const uint32_t g_shared_prologue_fnv1a = 0xa6268637u;

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

/* Direct MODE1/2048 ISO projections. These offsets are not raw BIN offsets:
 * the raw Track 02 pregap is absent from the supplied TQUS19.iso/TQJP19.iso
 * projections. The final spans intentionally use each ISO's own EOF and
 * hash; no truncated US or JP ISO payload is silently compared with BIN. */
static const Theron_LevelDataBlockDesc g_us_iso_level_blocks[THERON_TRACK02_LEVEL_COUNT] = {
    { 0x02E800, { 0x07, 0x87, 0x18, 0x10, 0x10, 0x10, 0x10, 0x20 } },
    { 0x06EB42, { 0x03, 0x83, 0x3F, 0x3F, 0x3F, 0x07, 0x1F, 0x0F } },
    { 0x0AE800, { 0x0F, 0x87, 0x0F, 0x04, 0x00, 0x01, 0x01, 0x01 } },
    { 0x0EEEA8, { 0x07, 0x87, 0x3F, 0x3F, 0x3F, 0x1F, 0x1F, 0x1F } },
    { 0x12EB73, { 0x07, 0x87, 0x0F, 0x3F, 0x7F, 0xFF, 0xFF, 0xFF } },
    { 0x16E800, { 0x07, 0x87, 0x07, 0x0C, 0x10, 0x30, 0x20, 0x20 } },
    { 0x1AE800, { 0x0D, 0x86, 0x06, 0x30, 0x0F, 0x0E, 0x07, 0x0E } },
};

static const Theron_LevelDataBlockDesc g_jp_iso_level_blocks[THERON_TRACK02_LEVEL_COUNT] = {
    { 0x02E82F, { 0x07, 0x87, 0x18, 0x10, 0x10, 0x10, 0x10, 0x20 } },
    { 0x06EB71, { 0x07, 0x87, 0x04, 0x02, 0x04, 0x02, 0x04, 0x02 } },
    { 0x0AE82F, { 0x0F, 0x87, 0x0F, 0x04, 0x00, 0x01, 0x01, 0x01 } },
    { 0x0EEED7, { 0x03, 0x83, 0x3F, 0x3F, 0x3F, 0x07, 0x1F, 0x0F } },
    { 0x12EE9B, { 0x0F, 0x87, 0x1E, 0x1E, 0x1E, 0x1F, 0x3F, 0x3F } },
    { 0x16E82F, { 0x07, 0x87, 0x07, 0x0C, 0x10, 0x30, 0x20, 0x20 } },
    { 0x1AE82F, { 0x0F, 0x87, 0x10, 0x10, 0x10, 0x00, 0x20, 0x01 } },
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
    if (variant == THERON_TRACK02_VARIANT_US_ISO)
        return &g_us_iso_level_blocks[level];
    if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO)
        return &g_jp_iso_level_blocks[level];
    return NULL;
}

int theron_v1_track02_level_data_block_read(
    const uint8_t *user_data, size_t user_data_size,
    Theron_Track02Variant variant, unsigned int level,
    Theron_LevelDataBlockReceipt *out) {
    const Theron_LevelDataBlockDesc *block;
    const uint32_t *expected_hashes;
    size_t next_offset;
    size_t compressed_offset;
    size_t compressed_end;
    uint16_t resource_length;
    size_t resource_bitstream_bytes;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!user_data) return 0;
    if (variant == THERON_TRACK02_VARIANT_US_BIN)
        expected_hashes = g_us_compressed_fnv1a;
    else if (variant == THERON_TRACK02_VARIANT_JP_BIN)
        expected_hashes = g_jp_compressed_fnv1a;
    else if (variant == THERON_TRACK02_VARIANT_US_ISO)
        expected_hashes = g_us_iso_compressed_fnv1a;
    else if (variant == THERON_TRACK02_VARIANT_JP_REV1_ISO)
        expected_hashes = g_jp_iso_compressed_fnv1a;
    else
        return 0;
    if (!theron_v1_track02_level_data_block_for_variant(variant, level)) return 0;
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
    if (compressed_end - compressed_offset <
            THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE) {
        return 0;
    }
    resource_length = (uint16_t)(user_data[compressed_offset + 2u] |
                                 ((uint16_t)user_data[compressed_offset + 3u] << 8));
    /* Retail bank-$1f $23ad reads the word at source+2, subtracts five,
     * then advances the source pointer by six before the variable-bit reader
     * starts. This is framing only; bank mappings and output meaning remain
     * unauthenticated. */
    if (resource_length < 5u ||
        (size_t)(resource_length - 5u) >
            compressed_end - compressed_offset -
                THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE) {
        return 0;
    }
    resource_bitstream_bytes = (size_t)(resource_length - 5u);
    out->valid = 1;
    out->variant = variant;
    out->level = level;
    out->block_ud_offset = block->ud_offset;
    out->compressed_ud_offset = (uint32_t)compressed_offset;
    out->resource_end_ud_offset = (uint32_t)(compressed_offset +
                                             THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE +
                                             resource_bitstream_bytes);
    out->compressed_bytes = compressed_end - compressed_offset;
    out->resource_length = resource_length;
    out->resource_bitstream_bytes = resource_bitstream_bytes;
    out->compressed = user_data + compressed_offset;
    out->compressed_fnv1a = fnv1a(out->compressed, out->compressed_bytes);
    memcpy(out->resource_header, out->compressed,
           THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE);
    out->resource_header_verified = 1;
    out->resource_bitstream = out->compressed +
                              THERON_TRACK02_LEVEL_RESOURCE_HEADER_SIZE;
    out->shared_prologue_fnv1a = fnv1a(
        user_data + block->ud_offset,
        THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE);
    memcpy(out->per_level_meta, block->per_level_meta,
           sizeof(out->per_level_meta));
    return out->compressed_bytes != 0u &&
           out->compressed_fnv1a == expected_hashes[level] &&
           out->shared_prologue_fnv1a == g_shared_prologue_fnv1a &&
           memcmp(user_data + block->ud_offset +
                      THERON_TRACK02_LEVEL_SHARED_PROLOGUE_SIZE,
                  block->per_level_meta,
                  sizeof(block->per_level_meta)) == 0;
}
