#include "nexus_v1_warning_dgt2_descriptor_admission.h"

#include "nexus_v1_ui_surfaces.h"

#include <string.h>

static uint16_t be16(const uint8_t *p) { return (uint16_t)((uint16_t)p[0] << 8 | p[1]); }
static uint32_t be32(const uint8_t *p) { return (uint32_t)p[0] << 24 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 8 | p[3]; }
static uint64_t fnv1a64(const uint8_t *p,size_t n) { uint64_t h=UINT64_C(1469598103934665603); size_t i; for(i=0;i<n;++i){h^=p[i];h*=UINT64_C(1099511628211);} return h; }

int nexus_v1_warning_dgt2_descriptor_admit(
    const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_WarningDgt2SourceAdmissionReceipt *source_admission,
    Nexus_V1_WarningDgt2DescriptorAdmissionReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2DescriptorAdmissionReceipt receipt;
    Nexus_UI_Dgt2PpView view;
    uint16_t count;
    uint32_t selected_offset = 0U;
    uint32_t index;
    size_t table_length;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !source_admission || !source_admission->valid ||
        !source_admission->source_identity_bound || !source_admission->res_directory_bound ||
        !source_admission->dgt2_resource_bound || source_admission->clut_semantics_proven ||
        source_admission->pixel_decode_permitted || source_admission->draw_permitted ||
        source_size != NEXUS_V1_WARNING_BIN_BYTES ||
        fnv1a64(source_bytes, source_size) != source_admission->source_fnv1a64 ||
        source_size < 12U || memcmp(source_bytes, "RES*", 4) != 0 ||
        be32(source_bytes + 4) != source_size) { *out_receipt = receipt; return 0; }
    count = be16(source_bytes + 8);
    table_length = (size_t)count * 12U;
    if (!count || table_length > source_size - 12U ||
        nexus_ui_res_dgt2_pp_view(source_bytes, source_size, 0U, &view) != 0) {
        *out_receipt = receipt; return 0;
    }
    for (index = 0U; index < count; ++index) {
        const uint8_t *descriptor = source_bytes + 12U + (size_t)index * 12U;
        uint32_t offset = be32(descriptor + 8);
        uint32_t next_offset = index + 1U < count ? be32(descriptor + 20U) : (uint32_t)source_size;
        if (memcmp(descriptor, "DGT2", 4) != 0 || be32(descriptor + 4) != index ||
            offset < 12U + table_length || offset >= source_size || next_offset <= offset ||
            next_offset > source_size) { *out_receipt = receipt; return 0; }
        if (index == 0U) selected_offset = offset;
    }
    if (!selected_offset || selected_offset != source_admission->dgt2_offset ||
        source_admission->dgt2_length > source_size - selected_offset ||
        memcmp(source_bytes + selected_offset, "DGT2", 4) != 0 ||
        be32(source_bytes + selected_offset + 4U) != 0U ||
        memcmp(source_bytes + selected_offset + 8U, "PP", 2) != 0 ||
        source_admission->clut_offset != selected_offset + 14U ||
        source_admission->pixel_offset != selected_offset + 526U) {
        *out_receipt = receipt; return 0;
    }
    receipt.valid = receipt.source_admission_bound = receipt.res_header_bound =
        receipt.descriptor_table_bound = receipt.selected_descriptor_bound =
        receipt.dgt2_header_bound = receipt.pp_header_bound = 1;
    receipt.source_fnv1a64 = source_admission->source_fnv1a64;
    receipt.declared_size = (uint32_t)source_size;
    receipt.descriptor_count = count;
    receipt.descriptor_table_offset = 12U;
    receipt.descriptor_table_length = (uint32_t)table_length;
    receipt.descriptor_table_fnv1a64 = fnv1a64(source_bytes + 12U, table_length);
    receipt.descriptor_index = 0U;
    receipt.descriptor_id = 0U;
    receipt.descriptor_offset = selected_offset;
    receipt.descriptor_fnv1a64 = fnv1a64(source_bytes + 12U, 12U);
    receipt.dgt2_header_fnv1a64 = fnv1a64(source_bytes + selected_offset, 8U);
    receipt.pp_header_fnv1a64 = fnv1a64(source_bytes + selected_offset + 8U, 6U);
    *out_receipt = receipt;
    return 1;
}
