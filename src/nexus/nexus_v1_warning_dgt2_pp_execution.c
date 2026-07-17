#include "nexus_v1_warning_dgt2_pp_execution.h"

#include <string.h>

static uint16_t be16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] << 8 | p[1]);
}

static uint64_t fnv1a64(const uint8_t *p, size_t n)
{
    uint64_t h = UINT64_C(1469598103934665603);
    size_t i;

    for (i = 0; i < n; ++i) {
        h ^= p[i];
        h *= UINT64_C(1099511628211);
    }
    return h;
}

int nexus_v1_warning_dgt2_pp_execute(
    const uint8_t *source_bytes,
    size_t source_size,
    const Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt *payload_admission,
    uint8_t *indexed_pixels_out,
    size_t indexed_pixels_out_size,
    uint16_t bgr555_words_out[256],
    Nexus_V1_WarningDgt2PpPresentFn present,
    void *present_context,
    Nexus_V1_WarningDgt2PpExecutionReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2PpExecutionReceipt receipt;
    uint32_t i;
    uint64_t palette_fnv;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !payload_admission || !indexed_pixels_out ||
        !bgr555_words_out || !present || !payload_admission->valid ||
        !payload_admission->descriptor_admission_bound ||
        !payload_admission->resource_boundary_bound ||
        !payload_admission->pp_header_bound || !payload_admission->payload_bound ||
        source_size != 101256U ||
        fnv1a64(source_bytes, source_size) != payload_admission->source_fnv1a64 ||
        payload_admission->resource_offset != 0x48U ||
        payload_admission->resource_length != 0x5c10U ||
        payload_admission->pp_header_offset != 0x50U ||
        payload_admission->pp_width_field != 240U ||
        payload_admission->pp_height_field != 96U ||
        payload_admission->post_header_prefix_offset != 0x56U ||
        payload_admission->post_header_prefix_length != 512U ||
        payload_admission->declared_body_offset != 0x256U ||
        payload_admission->declared_body_length != 23040U ||
        payload_admission->trailing_offset != 0x5c56U ||
        payload_admission->trailing_length != 2U ||
        indexed_pixels_out_size != payload_admission->declared_body_length ||
        payload_admission->post_header_prefix_offset > source_size ||
        payload_admission->post_header_prefix_length >
            source_size - payload_admission->post_header_prefix_offset ||
        payload_admission->declared_body_offset > source_size ||
        payload_admission->declared_body_length >
            source_size - payload_admission->declared_body_offset ||
        payload_admission->trailing_offset > source_size ||
        payload_admission->trailing_length > source_size - payload_admission->trailing_offset ||
        fnv1a64(source_bytes + payload_admission->resource_offset,
                 payload_admission->resource_length) != payload_admission->resource_fnv1a64 ||
        memcmp(source_bytes + payload_admission->pp_header_offset, "PP", 2) != 0 ||
        be16(source_bytes + payload_admission->pp_header_offset + 2U) !=
            payload_admission->pp_width_field ||
        be16(source_bytes + payload_admission->pp_header_offset + 4U) !=
            payload_admission->pp_height_field ||
        fnv1a64(source_bytes + payload_admission->pp_header_offset, 6U) !=
            payload_admission->pp_header_fnv1a64 ||
        fnv1a64(source_bytes + payload_admission->post_header_prefix_offset,
                 payload_admission->post_header_prefix_length) !=
            payload_admission->post_header_prefix_fnv1a64 ||
        fnv1a64(source_bytes + payload_admission->declared_body_offset,
                 payload_admission->declared_body_length) !=
            payload_admission->declared_body_fnv1a64 ||
        fnv1a64(source_bytes + payload_admission->trailing_offset,
                 payload_admission->trailing_length) != payload_admission->trailing_fnv1a64) {
        *out_receipt = receipt;
        return 0;
    }

    for (i = 0; i < 256U; ++i) {
        bgr555_words_out[i] = be16(source_bytes +
            payload_admission->post_header_prefix_offset + i * 2U);
    }
    memcpy(indexed_pixels_out, source_bytes + payload_admission->declared_body_offset,
           payload_admission->declared_body_length);
    palette_fnv = fnv1a64(source_bytes + payload_admission->post_header_prefix_offset,
                           payload_admission->post_header_prefix_length);
    if (!present(present_context, indexed_pixels_out, 240U, 96U, 240U,
                 bgr555_words_out, 256U)) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.valid = receipt.payload_admission_bound = receipt.pp_256_indexed_proven = 1;
    receipt.bgr555_clut_proven = receipt.stride_proven = receipt.index_plane_copied = 1;
    receipt.presented = 1;
    receipt.source_fnv1a64 = payload_admission->source_fnv1a64;
    receipt.width = 240U;
    receipt.height = 96U;
    receipt.stride = 240U;
    receipt.bgr555_word_count = 256U;
    receipt.index_plane_fnv1a64 = payload_admission->declared_body_fnv1a64;
    receipt.bgr555_words_fnv1a64 = palette_fnv;
    receipt.trailing_raw_bytes = payload_admission->trailing_length;
    *out_receipt = receipt;
    return 1;
}
