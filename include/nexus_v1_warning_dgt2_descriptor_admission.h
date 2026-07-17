#ifndef NEXUS_V1_WARNING_DGT2_DESCRIPTOR_ADMISSION_H
#define NEXUS_V1_WARNING_DGT2_DESCRIPTOR_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_warning_dgt2_source_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int source_admission_bound;
    int res_header_bound;
    int descriptor_table_bound;
    int selected_descriptor_bound;
    int dgt2_header_bound;
    int pp_header_bound;
    int clut_semantics_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t declared_size;
    uint16_t descriptor_count;
    uint32_t descriptor_table_offset;
    uint32_t descriptor_table_length;
    uint64_t descriptor_table_fnv1a64;
    uint32_t descriptor_index;
    uint32_t descriptor_id;
    uint32_t descriptor_offset;
    uint64_t descriptor_fnv1a64;
    uint64_t dgt2_header_fnv1a64;
    uint64_t pp_header_fnv1a64;
} Nexus_V1_WarningDgt2DescriptorAdmissionReceipt;

/* Attests only the source-backed RES-star/DGT2 descriptor framing already
 * accepted by the existing lookup. It does not assign image/palette meaning. */
int nexus_v1_warning_dgt2_descriptor_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2SourceAdmissionReceipt *source_admission,
    Nexus_V1_WarningDgt2DescriptorAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
