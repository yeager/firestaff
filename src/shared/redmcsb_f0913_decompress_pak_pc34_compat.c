#include "redmcsb_f0913_decompress_pak_pc34_compat.h"

#include <stddef.h>

static uint16_t redmcsb_f0913_read_be16(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

static uint32_t redmcsb_f0913_read_be32(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) |
           ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) |
           (uint32_t)bytes[3];
}

static bool redmcsb_f0913_read_nibble(const uint8_t *pak_bytes,
                                       size_t pak_size,
                                       size_t *nibble_index,
                                       uint16_t *out_nibble)
{
    size_t byte_offset;

    byte_offset = REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34 +
                  *nibble_index / 2u;
    if (byte_offset >= pak_size) {
        return false;
    }

    if ((*nibble_index & 1u) == 0u) {
        *out_nibble = (uint16_t)(pak_bytes[byte_offset] >> 4);
    } else {
        *out_nibble = (uint16_t)(pak_bytes[byte_offset] & 0x0fu);
    }
    (*nibble_index)++;
    return true;
}

static bool redmcsb_f0913_read_nibbles(const uint8_t *pak_bytes,
                                        size_t pak_size,
                                        size_t *nibble_index,
                                        unsigned int count,
                                        uint16_t *out_value)
{
    uint16_t value = 0u;
    uint16_t nibble;
    unsigned int index;

    for (index = 0u; index < count; index++) {
        if (!redmcsb_f0913_read_nibble(pak_bytes, pak_size, nibble_index,
                                        &nibble)) {
            return false;
        }
        value = (uint16_t)((value << 4) | nibble);
    }
    *out_value = value;
    return true;
}

int redmcsb_f0913_decompress_pak_pc34_compat(
    const uint8_t *pak_bytes,
    size_t pak_size,
    uint16_t *decompressed_words,
    size_t decompressed_word_capacity)
{
    uint32_t decompressed_word_count;
    size_t nibble_index = 0u;
    size_t output_index;

    if (pak_bytes == NULL ||
        (decompressed_words == NULL && decompressed_word_capacity != 0u)) {
        return REDMCSB_F0913_DECOMPRESS_PAK_HOST_INVALID_ARGUMENT_PC34;
    }
    if (pak_size < REDMCSB_F0913_PAK_COMPRESSED_DATA_OFFSET_PC34) {
        return REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34;
    }
    if (redmcsb_f0913_read_be16(pak_bytes) !=
        REDMCSB_F0913_PAK_SIGNATURE_PC34) {
        return REDMCSB_F0913_DECOMPRESS_PAK_BAD_SIGNATURE_PC34;
    }

    decompressed_word_count = redmcsb_f0913_read_be32(pak_bytes + 4u);
    if ((size_t)decompressed_word_count > decompressed_word_capacity ||
        (decompressed_word_count != 0u && decompressed_words == NULL)) {
        return REDMCSB_F0913_DECOMPRESS_PAK_HOST_OUTPUT_TOO_SMALL_PC34;
    }

    for (output_index = 0u; output_index < (size_t)decompressed_word_count;
         output_index++) {
        uint16_t control_code;
        uint16_t table_index;

        if (!redmcsb_f0913_read_nibbles(pak_bytes, pak_size, &nibble_index,
                                         1u, &control_code)) {
            return REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34;
        }
        if (control_code < 8u) {
            if (!redmcsb_f0913_read_nibbles(pak_bytes, pak_size,
                                             &nibble_index, 1u,
                                             &table_index)) {
                return REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34;
            }
            table_index = (uint16_t)((control_code << 4) | table_index);
        } else if (control_code < 15u) {
            if (!redmcsb_f0913_read_nibbles(pak_bytes, pak_size,
                                             &nibble_index, 2u,
                                             &table_index)) {
                return REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34;
            }
            table_index = (uint16_t)(((control_code << 8) | table_index) -
                                      1920u);
        } else {
            if (!redmcsb_f0913_read_nibbles(pak_bytes, pak_size,
                                             &nibble_index, 4u,
                                             &decompressed_words[output_index])) {
                return REDMCSB_F0913_DECOMPRESS_PAK_HOST_TRUNCATED_INPUT_PC34;
            }
            continue;
        }
        decompressed_words[output_index] = redmcsb_f0913_read_be16(
            pak_bytes + REDMCSB_F0913_PAK_WORD_TABLE_OFFSET_PC34 +
            (size_t)table_index * 2u);
    }
    return REDMCSB_F0913_DECOMPRESS_PAK_OK_PC34;
}

bool redmcsb_f0913_install_prim_decompress_hook_pc34_compat(
    redmcsb_f0913_prim_hook_callback_pc34_compat hook_callback,
    void *hook_context)
{
    if (hook_callback == NULL) {
        return false;
    }
    return hook_callback(
        hook_context, REDMCSB_F0913_PRIM_DECOMPRESS_CODE_SEGMENT_SLOT_PC34,
        redmcsb_f0913_decompress_pak_pc34_compat);
}

const char *redmcsb_f0913_decompress_pak_source_evidence_pc34(void)
{
    return "ReDMCSB DECOMPCO.C:11-109 defines F0913: it rejects a word "
           "other than 0x5223, reads the long word count at +4, the 1920-"
           "word table at +8, and decodes from +3848. APPA.C:39-41 installs "
           "F0913 through PRIM_04_Library_HookFunction in PRIM slot 25.";
}
