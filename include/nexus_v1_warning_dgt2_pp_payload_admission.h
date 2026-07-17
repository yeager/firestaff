#ifndef NEXUS_V1_WARNING_DGT2_PP_PAYLOAD_ADMISSION_H
#define NEXUS_V1_WARNING_DGT2_PP_PAYLOAD_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_warning_dgt2_descriptor_admission.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int descriptor_admission_bound;
    int resource_boundary_bound;
    int pp_header_bound;
    int payload_bound;
    int dimension_semantics_proven;
    int clut_semantics_proven;
    int pixel_decode_permitted;
    int draw_permitted;
    uint64_t source_fnv1a64;
    uint32_t resource_offset;
    uint32_t resource_length;
    uint64_t resource_fnv1a64;
    uint32_t pp_header_offset;
    uint16_t pp_width_field;
    uint16_t pp_height_field;
    uint64_t pp_header_fnv1a64;
    uint32_t post_header_prefix_offset;
    uint32_t post_header_prefix_length;
    uint64_t post_header_prefix_fnv1a64;
    uint32_t declared_body_offset;
    uint32_t declared_body_length;
    uint64_t declared_body_fnv1a64;
    uint32_t trailing_offset;
    uint32_t trailing_length;
    uint64_t trailing_fnv1a64;
} Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt;

/* Binds the raw resource-0 PP framing to the next descriptor boundary. Width
 * and height are retained only as header fields, not interpreted dimensions. */
int nexus_v1_warning_dgt2_pp_payload_admit(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2DescriptorAdmissionReceipt *descriptor_admission,
    Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
