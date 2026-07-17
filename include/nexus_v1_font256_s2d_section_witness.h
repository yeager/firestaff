#ifndef NEXUS_V1_FONT256_S2D_SECTION_WITNESS_H
#define NEXUS_V1_FONT256_S2D_SECTION_WITNESS_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_font256_s2d_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The first observed bytes after the canonical SCR section table are the
 * 16-byte preamble at section-table index zero. They are a capture target
 * only: no field within them has text, glyph, palette, or pixel semantics. */
#define NEXUS_V1_FONT256_S2D_FIRST_SECTION_INDEX 0U
#define NEXUS_V1_FONT256_S2D_FIRST_PREAMBLE_BYTES 16U

typedef struct {
    int valid;
    int source_admission_bound;
    int selected_section_bound;
    int preamble_capture_required;
    int glyph_layout_proven;
    int palette_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t section_table_index;
    uint32_t section_offset;
    uint32_t section_length;
    uint64_t section_fnv1a64;
    uint32_t preamble_offset;
    uint32_t preamble_length;
    uint64_t preamble_fnv1a64;
} Nexus_V1_Font256S2DSectionWitnessReceipt;

/* Rechecks the canonical admission against the live source and publishes only
 * the bounded raw section-zero preamble as an external-capture target. */
int nexus_v1_font256_s2d_first_section_witness(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_Font256S2DAdmissionReceipt *admission,
    Nexus_V1_Font256S2DSectionWitnessReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
