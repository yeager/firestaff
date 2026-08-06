#include "nexus_v1_font256_s2d_admission.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) {
        value ^= bytes[index];
        value *= UINT64_C(1099511628211);
    }
    return value;
}

static int admitted_sha256(const char *sha256_hex)
{
    return sha256_hex &&
        (strcmp(sha256_hex, NEXUS_V1_FONT256_S2D_SHA256) == 0 ||
         strcmp(sha256_hex, NEXUS_V1_FONT256_S2D_SHA256_ENGLISH) == 0);
}

int nexus_v1_font256_s2d_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DSourceIdentity *identity,
    Nexus_V1_Font256S2DAdmissionReceipt *out_receipt)
{
    static const int expected_indices[4] = { 0, 2, 4, 6 };
    static const uint32_t expected_offsets[4] = { 0x0120U, 0x2130U, 0x5dc0U, 0x5fd0U };
    static const uint32_t expected_sizes[4] = { 0x2010U, 0x3c90U, 0x0210U, 0x01e4U };
    Nexus_V1_Font256S2DAdmissionReceipt receipt;
    Nexus_V1_FontSections parsed;
    uint64_t source_fnv;
    int index;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !identity || source_size != NEXUS_V1_FONT256_S2D_BYTES ||
        !identity->sha256_verified || !identity->sha256_hex ||
        !admitted_sha256(identity->sha256_hex) ||
        !identity->source_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }

    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != identity->source_fnv1a64 ||
        nexus_v1_font_load_sections(source_bytes, (int)source_size, &parsed) != 0 ||
        parsed.char_count != 256 || parsed.header_descriptor != 0x12U ||
        parsed.section_count != 4) {
        *out_receipt = receipt;
        return 0;
    }

    for (index = 0; index < 4; ++index) {
        const Nexus_V1_FontSection *section =
            nexus_v1_font_get_section(&parsed, index);
        if (!section || section->index != expected_indices[index] ||
            section->file_offset != expected_offsets[index] ||
            section->size != expected_sizes[index] ||
            nexus_v1_font_section_in_bounds(section, (int)source_size) != 1) {
            *out_receipt = receipt;
            return 0;
        }
        receipt.sections[index] = *section;
        receipt.section_fnv1a64[index] = fnv1a64(
            source_bytes + section->file_offset, section->size);
    }

    receipt.valid = 1;
    receipt.source_identity_bound = 1;
    receipt.scr_header_bound = 1;
    receipt.section_table_bound = 1;
    receipt.source_bytes = (uint32_t)source_size;
    receipt.source_fnv1a64 = source_fnv;
    receipt.char_count = (uint32_t)parsed.char_count;
    receipt.header_descriptor = parsed.header_descriptor;
    receipt.section_count = (uint32_t)parsed.section_count;
    receipt.section_table_fnv1a64 = fnv1a64(
        source_bytes + NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET,
        NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX *
            NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE);
    *out_receipt = receipt;
    return 1;
}
