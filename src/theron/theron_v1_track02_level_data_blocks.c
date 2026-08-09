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
 * Compression algorithm: the exact variable-width HuC6280 routine is
 * byte-locked in bank-$1f at $23AD-$252A.  Its stage-2 MPR setup and the
 * later level/object consumer remain separate runtime-admission gates. */

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

static void theron_huc6280_receipt_init(Theron_Huc6280DecodeReceipt *out) {
    if (out) memset(out, 0, sizeof(*out));
}

static int theron_huc6280_read_bits(const uint8_t *bits, size_t bit_count,
                                    size_t *bit_cursor, unsigned int width,
                                    uint16_t *value) {
    uint16_t result = 0u;
    unsigned int i;

    if (!bits || !bit_cursor || !value || width == 0u || width > 16u ||
        *bit_cursor > bit_count || width > bit_count - *bit_cursor) {
        return 0;
    }
    for (i = 0u; i < width; ++i) {
        size_t bit = *bit_cursor;
        result = (uint16_t)((result << 1u) |
                            ((bits[bit / 8u] >> (7u - (bit % 8u))) & 1u));
        *bit_cursor = bit + 1u;
    }
    *value = result;
    return 1;
}

static int theron_huc6280_address_to_offset(uint16_t address,
                                            uint16_t destination_address,
                                            size_t capacity, size_t *offset) {
    uint32_t delta;

    if (!offset || address < destination_address) return 0;
    delta = (uint32_t)address - (uint32_t)destination_address;
    if ((size_t)delta >= capacity) return 0;
    *offset = (size_t)delta;
    return 1;
}

static int theron_huc6280_emit_token(
    uint16_t token, uint8_t *destination, size_t destination_capacity,
    uint16_t destination_address, uint16_t *pointer_table,
    size_t pointer_table_capacity, size_t pointer_table_available,
    size_t *output_offset,
    Theron_Huc6280DecodeReceipt *receipt) {
    uint8_t high = (uint8_t)(token >> 8u);

    if (!destination || !pointer_table || !output_offset || !receipt) return 0;
    if (*output_offset >= destination_capacity ||
        *output_offset > 0xFFFFu - (size_t)destination_address) {
        receipt->status = THERON_HUC6280_DECODE_DESTINATION;
        return 0;
    }
    if (high == 0u || (high & 0x80u) != 0u) {
        destination[*output_offset] = (uint8_t)token;
        ++*output_offset;
        ++receipt->literal_tokens;
        return 1;
    }

    /* $2496: (token - 1) << 1 indexes the output-address table rooted at
     * $32/$33.  The table is passed separately because the stage-2 MPR
     * projection is not equivalent to a host pointer. */
    {
        size_t index = (size_t)token - 1u;
        uint16_t source_address;
        uint16_t next_address;
        uint16_t length;
        size_t source_offset;
        size_t i;

        if (index + 1u >= pointer_table_capacity ||
            index + 1u >= pointer_table_available) {
            receipt->status = THERON_HUC6280_DECODE_POINTER_TABLE;
            return 0;
        }
        source_address = pointer_table[index];
        next_address = pointer_table[index + 1u];
        length = (uint16_t)(next_address - source_address);
        if (length == 0u) {
            /* The retail routine accepts a zero-length TII/copy. */
            ++receipt->backreference_tokens;
            return 1;
        }
        if (!theron_huc6280_address_to_offset(source_address,
                                               destination_address,
                                               destination_capacity,
                                               &source_offset) ||
            (size_t)length > destination_capacity - *output_offset ||
            source_offset > destination_capacity - (size_t)length) {
            receipt->status = THERON_HUC6280_DECODE_DESTINATION;
            return 0;
        }
        /* The HuC6280 copy path is allowed to cross the low-byte boundary;
         * byte-at-a-time copying preserves its already-produced source and
         * is safe for the overlapping form of a back-reference. */
        for (i = 0u; i < (size_t)length; ++i) {
            destination[*output_offset + i] = destination[source_offset + i];
        }
        *output_offset += (size_t)length;
        ++receipt->backreference_tokens;
    }
    return 1;
}

int theron_v1_huc6280_decode_resource(
    const uint8_t *resource, size_t resource_bytes,
    uint8_t *destination, size_t destination_capacity,
    uint16_t destination_address,
    uint16_t *pointer_table, size_t pointer_table_capacity,
    size_t pointer_table_seed_count,
    Theron_Huc6280DecodeReceipt *out) {
    uint16_t resource_length;
    size_t bitstream_bytes;
    const uint8_t *bitstream;
    size_t bit_count;
    size_t bit_cursor = 0u;
    size_t output_offset = 0u;
    size_t pointer_entries = 0u;
    unsigned int width = 9u;
    size_t token_count = 0u;
    size_t pointer_table_available;

    theron_huc6280_receipt_init(out);
    if (!out) return 0;
    if (!resource || !destination || !pointer_table || resource_bytes < 6u ||
        destination_capacity == 0u || pointer_table_capacity < 2u ||
        pointer_table_seed_count > pointer_table_capacity) {
        out->status = THERON_HUC6280_DECODE_INVALID_ARGUMENT;
        return 0;
    }
    resource_length = (uint16_t)(resource[2] |
                                 ((uint16_t)resource[3] << 8u));
    if (resource_length < 5u) {
        out->status = THERON_HUC6280_DECODE_TRUNCATED;
        return 0;
    }
    bitstream_bytes = (size_t)resource_length - 5u;
    if (bitstream_bytes > resource_bytes - 6u) {
        out->status = THERON_HUC6280_DECODE_TRUNCATED;
        return 0;
    }
    if (destination_capacity > 0x10000u - (size_t)destination_address) {
        out->status = THERON_HUC6280_DECODE_UNSUPPORTED;
        return 0;
    }
    bitstream = resource + 6u;
    bit_count = bitstream_bytes * 8u;
    pointer_table_available = pointer_table_seed_count;
    out->resource_length = resource_length;
    out->resource_bitstream_bytes = bitstream_bytes;

    /* $0F=$01 and $14=$09 come from the authenticated $23A4 helper.  The
     * first pointer entry is written before the first token; two tokens are
     * consumed per L23DE/L240D output group, with marker tokens only widening
     * the reader. */
    while (bit_cursor < bit_count) {
        uint16_t token;

        if (pointer_entries >= pointer_table_capacity) {
            out->status = THERON_HUC6280_DECODE_POINTER_TABLE;
            return 0;
        }
        pointer_table[pointer_entries++] =
            (uint16_t)((uint32_t)destination_address +
                       (uint32_t)output_offset);
        if (pointer_table_available < pointer_entries) {
            pointer_table_available = pointer_entries;
        }

        do {
            if (!theron_huc6280_read_bits(bitstream, bit_count, &bit_cursor,
                                          width, &token)) {
                out->status = THERON_HUC6280_DECODE_TRUNCATED;
                return 0;
            }
            if (token == 0x0100u) {
                if (width == 16u) {
                    out->status = THERON_HUC6280_DECODE_UNSUPPORTED;
                    return 0;
                }
                ++width;
                ++out->width_markers;
            }
        } while (token == 0x0100u);
        if (!theron_huc6280_emit_token(
                token, destination, destination_capacity, destination_address,
                pointer_table, pointer_table_capacity, pointer_table_available,
                &output_offset, out)) {
            return 0;
        }
        ++token_count;

        {
            int second_token_read = 1;
            do {
                if (!theron_huc6280_read_bits(bitstream, bit_count,
                                              &bit_cursor, width, &token)) {
                    /* The retail helper tests the byte countdown after this
                     * second read and returns before emitting an incomplete
                     * trailing token. */
                    second_token_read = 0;
                    break;
                }
                if (token == 0x0100u) {
                    if (width == 16u) {
                        out->status = THERON_HUC6280_DECODE_UNSUPPORTED;
                        return 0;
                    }
                    ++width;
                    ++out->width_markers;
                }
            } while (token == 0x0100u);
            if (!second_token_read || bit_cursor >= bit_count ||
                token == 0x0100u) break;
            if (!theron_huc6280_emit_token(
                    token, destination, destination_capacity,
                    destination_address, pointer_table,
                    pointer_table_capacity, pointer_table_available,
                    &output_offset, out)) {
                return 0;
            }
            ++token_count;
        }
    }

    out->status = THERON_HUC6280_DECODE_READY;
    out->bits_consumed = bit_cursor;
    out->output_bytes = output_offset;
    out->pointer_entries = pointer_entries;
    out->tokens = token_count;
    out->final_bit_width = (uint8_t)width;
    return 1;
}
