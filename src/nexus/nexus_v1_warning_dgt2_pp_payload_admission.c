#include "nexus_v1_warning_dgt2_pp_payload_admission.h"

#include "nexus_v1_ui_surfaces.h"

#include <string.h>

static uint16_t be16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] << 8 | p[1]); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint64_t fnv1a64(const uint8_t *p,size_t n) { uint64_t h=UINT64_C(1469598103934665603); size_t i; for(i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);} return h; }

int nexus_v1_warning_dgt2_pp_payload_admit(
    const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_WarningDgt2DescriptorAdmissionReceipt *descriptor_admission,
    Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2PpPayloadAdmissionReceipt receipt;
    Nexus_UI_Dgt2PpView view;
    uint32_t resource_offset;
    uint32_t resource_end;
    uint32_t pp_offset;
    uint32_t prefix_offset;
    uint32_t body_offset;
    uint32_t body_end;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !descriptor_admission || !descriptor_admission->valid ||
        !descriptor_admission->source_admission_bound ||
        !descriptor_admission->res_header_bound ||
        !descriptor_admission->descriptor_table_bound ||
        !descriptor_admission->selected_descriptor_bound ||
        !descriptor_admission->dgt2_header_bound || !descriptor_admission->pp_header_bound ||
        descriptor_admission->clut_semantics_proven ||
        descriptor_admission->pixel_decode_permitted || descriptor_admission->draw_permitted ||
        source_size != descriptor_admission->declared_size ||
        fnv1a64(source_bytes, source_size) != descriptor_admission->source_fnv1a64 ||
        descriptor_admission->descriptor_count < 2U ||
        descriptor_admission->descriptor_table_offset != 12U ||
        descriptor_admission->descriptor_table_length !=
            (uint32_t)descriptor_admission->descriptor_count * 12U ||
        descriptor_admission->descriptor_offset > source_size ||
        descriptor_admission->descriptor_offset + 8U > source_size ||
        memcmp(source_bytes + descriptor_admission->descriptor_offset, "DGT2", 4) != 0 ||
        be32(source_bytes + descriptor_admission->descriptor_offset + 4U) != 0U ||
        nexus_ui_res_dgt2_pp_view(source_bytes, source_size, 0U, &view) != 0) {
        *out_receipt = receipt; return 0;
    }
    resource_offset = descriptor_admission->descriptor_offset;
    resource_end = be32(source_bytes + descriptor_admission->descriptor_table_offset + 20U);
    pp_offset = resource_offset + 8U;
    prefix_offset = pp_offset + 6U;
    body_offset = prefix_offset + 512U;
    if (resource_end <= resource_offset || resource_end > source_size ||
        resource_end < body_offset || view.clut_bgr555_be != source_bytes + prefix_offset ||
        view.pixels != source_bytes + body_offset || view.pixel_bytes > resource_end - body_offset ||
        memcmp(source_bytes + pp_offset, "PP", 2) != 0) {
        *out_receipt = receipt; return 0;
    }
    body_end = body_offset + (uint32_t)view.pixel_bytes;
    receipt.valid = receipt.descriptor_admission_bound = receipt.resource_boundary_bound =
        receipt.pp_header_bound = receipt.payload_bound = 1;
    receipt.source_fnv1a64 = descriptor_admission->source_fnv1a64;
    receipt.resource_offset = resource_offset;
    receipt.resource_length = resource_end - resource_offset;
    receipt.resource_fnv1a64 = fnv1a64(source_bytes + resource_offset, receipt.resource_length);
    receipt.pp_header_offset = pp_offset;
    receipt.pp_width_field = be16(source_bytes + pp_offset + 2U);
    receipt.pp_height_field = be16(source_bytes + pp_offset + 4U);
    receipt.pp_header_fnv1a64 = fnv1a64(source_bytes + pp_offset, 6U);
    receipt.post_header_prefix_offset = prefix_offset;
    receipt.post_header_prefix_length = 512U;
    receipt.post_header_prefix_fnv1a64 = fnv1a64(source_bytes + prefix_offset, 512U);
    receipt.declared_body_offset = body_offset;
    receipt.declared_body_length = (uint32_t)view.pixel_bytes;
    receipt.declared_body_fnv1a64 = fnv1a64(source_bytes + body_offset, view.pixel_bytes);
    receipt.trailing_offset = body_end;
    receipt.trailing_length = resource_end - body_end;
    receipt.trailing_fnv1a64 = receipt.trailing_length ?
        fnv1a64(source_bytes + body_end, receipt.trailing_length) : 0U;
    *out_receipt = receipt;
    return 1;
}
