#include "nexus_v1_font256_s2d_section_witness.h"

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

int nexus_v1_font256_s2d_first_section_witness(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSectionWitnessReceipt *out_receipt)
{
    Nexus_V1_Font256S2DSectionWitnessReceipt receipt;
    const Nexus_V1_FontSection *section;
    uint64_t source_fnv;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.preamble_capture_required = 1;
    if (!source_bytes || !admission || !admission->valid ||
        !admission->source_identity_bound || !admission->scr_header_bound ||
        !admission->section_table_bound || source_size != admission->source_bytes ||
        admission->section_count != 4U ||
        admission->glyph_layout_proven || admission->pixel_decode_permitted ||
        admission->draw_permitted) {
        *out_receipt = receipt;
        return 0;
    }
    source_fnv = fnv1a64(source_bytes, source_size);
    section = &admission->sections[NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX];
    if (source_fnv != admission->source_fnv1a64 ||
        section->index != (int)NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX ||
        section->file_offset > source_size || section->size > source_size - section->file_offset ||
        section->size < NEXUS_V1_FONT256_S2D_FIRST_PREAMBLE_BYTES ||
        fnv1a64(source_bytes + section->file_offset, section->size) !=
            admission->section_fnv1a64[NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX] ||
        fnv1a64(source_bytes + NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET,
                 NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX *
                     NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE) !=
            admission->section_table_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.valid = 1;
    receipt.source_admission_bound = 1;
    receipt.selected_section_bound = 1;
    receipt.source_fnv1a64 = source_fnv;
    receipt.section_table_index = NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX;
    receipt.section_offset = section->file_offset;
    receipt.section_length = section->size;
    receipt.section_fnv1a64 = admission->section_fnv1a64[0];
    receipt.preamble_offset = section->file_offset;
    receipt.preamble_length = NEXUS_V1_FONT256_S2D_FIRST_PREAMBLE_BYTES;
    receipt.preamble_fnv1a64 = fnv1a64(source_bytes + section->file_offset,
                                        receipt.preamble_length);
    *out_receipt = receipt;
    return 1;
}
