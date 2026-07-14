#include "csb_v1_csbwin_512_xor_pad_classify.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void write_le16(uint8_t *bytes, size_t offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
}

static void write_le32(uint8_t *bytes, size_t offset, uint32_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1u] = (uint8_t)(value >> 8);
    bytes[offset + 2u] = (uint8_t)(value >> 16);
    bytes[offset + 3u] = (uint8_t)(value >> 24);
}

static uint32_t form_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t result = 0u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        result = result * 0xbb40e62du + 11u + (uint32_t)bytes[i];
    }
    return result;
}

static uint32_t rcs_checksum(const uint8_t *bytes, size_t size)
{
    uint32_t result = 0xffffu;
    size_t i;

    for (i = 0u; i < size; ++i) {
        result = result * 0xbb40e62du + 11u + (uint32_t)bytes[i];
    }
    return result;
}

static uint8_t *build_dsa_save(const uint32_t *program_words,
                               size_t action_count,
                               size_t *out_size)
{
    const size_t dsa_offset = CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES;
    const size_t state_offset = dsa_offset + 108u;
    const size_t action_offset = state_offset + 8u;
    size_t checksum_offset = action_offset;
    uint8_t *bytes;
    size_t i;

    if (!program_words || action_count == 0u) return NULL;
    for (i = 0u; i < action_count; ++i) {
        checksum_offset += 8u + (size_t)program_words[i] * 2u;
    }
    bytes = calloc(checksum_offset + 4u, 1u);
    if (!bytes) return NULL;
    memcpy(bytes, " Extended Features ", 20u);
    bytes[36u] = 1u;
    write_le16(bytes, 38u, 1u);
    write_le32(bytes, dsa_offset, 0u);
    write_le32(bytes, dsa_offset + 96u, 1u);
    write_le32(bytes, dsa_offset + 104u, 1u);
    write_le32(bytes, state_offset, 0u);
    write_le32(bytes, state_offset + 4u, (uint32_t)action_count);
    {
        size_t offset = action_offset;
        size_t action_index;
        for (action_index = 0u; action_index < action_count; ++action_index) {
            size_t word_index;
            write_le32(bytes, offset, (uint32_t)action_index);
            write_le32(bytes, offset + 4u, program_words[action_index]);
            offset += 8u;
            for (word_index = 0u;
                 word_index < (size_t)program_words[action_index];
                 ++word_index) {
                write_le16(bytes, offset + word_index * 2u,
                           (uint16_t)word_index);
            }
            offset += (size_t)program_words[action_index] * 2u;
        }
    }
    write_le32(bytes, checksum_offset,
               rcs_checksum(bytes + dsa_offset, checksum_offset - dsa_offset));
    write_le32(bytes, 32u, 0u);
    write_le32(bytes, 32u, form_checksum(bytes,
                                          CSB_V1_CSBWIN_EXTENDED_FEATURES_BYTES));
    *out_size = checksum_offset + 4u;
    return bytes;
}

int main(void)
{
    CSB_V1_CSBWinExtendedDSAReport report;
    CSB_V1_CSBWinExtendedFeaturesReport features;
    uint8_t *bytes;
    const uint32_t ceiling_program[] = {
        CSB_V1_CSBWIN_MAX_EXTENDED_DSA_PROGRAM_WORDS
    };
    const uint32_t oversized_programs[] = { 16384u, 16385u };
    size_t size;
    int rc;

    bytes = build_dsa_save(ceiling_program, 1u, &size);
    expect(bytes != NULL, "ceiling fixture allocates");
    if (bytes) {
        rc = csb_v1_csbwin_512_inspect_extended_dsa_section(
            bytes, size, &report, &features);
        expect(rc == CSB_V1_CSBWIN_EXTENDED_OK && report.valid,
               "aggregate bytecode ceiling remains admissible");
        expect(report.program_word_count ==
                   CSB_V1_CSBWIN_MAX_EXTENDED_DSA_PROGRAM_WORDS,
               "ceiling report retains the authenticated word count");
        free(bytes);
    }

    bytes = build_dsa_save(oversized_programs, 2u, &size);
    expect(bytes != NULL, "overflow fixture allocates");
    if (bytes) {
        rc = csb_v1_csbwin_512_inspect_extended_dsa_section(
            bytes, size, &report, &features);
        expect(rc == CSB_V1_CSBWIN_EXTENDED_ERR_DSA && !report.valid,
               "aggregate oversized DSA payload rejects before staging");
        free(bytes);
    }

    return failures == 0 ? 0 : 1;
}
