#include "nexus_v1_warning_dgt2_source_admission.h"

#include "nexus_v1_ui_surfaces.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *bytes, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t index;
    for (index = 0; index < size; ++index) { value ^= bytes[index]; value *= UINT64_C(1099511628211); }
    return value;
}

int nexus_v1_warning_dgt2_source_admit(
    const uint8_t *source_bytes, size_t source_size,
    const Nexus_V1_WarningDgt2SourceIdentity *identity,
    Nexus_V1_WarningDgt2SourceAdmissionReceipt *out_receipt)
{
    Nexus_V1_WarningDgt2SourceAdmissionReceipt receipt;
    Nexus_UI_Dgt2PpView view;
    uint64_t source_fnv;
    size_t clut_offset;
    size_t pixel_offset;
    size_t dgt2_offset;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!source_bytes || !identity || source_size != NEXUS_V1_WARNING_BIN_BYTES ||
        !identity->source_fnv1a64 ||
        (!((identity->sha256_verified && identity->sha256_hex &&
            strcmp(identity->sha256_hex, NEXUS_V1_WARNING_BIN_SHA256) == 0) ||
           identity->canonical_fnv1a64_verified))) {
        *out_receipt = receipt;
        return 0;
    }
    source_fnv = fnv1a64(source_bytes, source_size);
    if (source_fnv != identity->source_fnv1a64 ||
        (source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_JA &&
         source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_EN &&
         source_fnv != NEXUS_V1_WARNING_BIN_FNV1A64_FR) ||
        nexus_ui_res_dgt2_pp_view(source_bytes, source_size, 0U, &view) != 0 ||
        !view.clut_bgr555_be || !view.pixels || view.pixel_bytes == 0U) {
        *out_receipt = receipt; return 0;
    }
    clut_offset = (size_t)(view.clut_bgr555_be - source_bytes);
    pixel_offset = (size_t)(view.pixels - source_bytes);
    if (clut_offset < 14U || pixel_offset != clut_offset + 512U ||
        pixel_offset > source_size || view.pixel_bytes > source_size - pixel_offset) {
        *out_receipt = receipt; return 0;
    }
    dgt2_offset = clut_offset - 14U;
    if (dgt2_offset > source_size || pixel_offset + view.pixel_bytes < dgt2_offset ||
        memcmp(source_bytes + dgt2_offset, "DGT2", 4) != 0 ||
        memcmp(source_bytes + dgt2_offset + 8U, "PP", 2) != 0) {
        *out_receipt = receipt; return 0;
    }
    receipt.valid = receipt.source_identity_bound = receipt.res_directory_bound =
        receipt.dgt2_resource_bound = 1;
    receipt.source_fnv1a64 = source_fnv;
    receipt.resource_id = 0U;
    receipt.dgt2_offset = (uint32_t)dgt2_offset;
    receipt.dgt2_length = (uint32_t)(pixel_offset + view.pixel_bytes - dgt2_offset);
    receipt.dgt2_fnv1a64 = fnv1a64(source_bytes + dgt2_offset, receipt.dgt2_length);
    receipt.clut_offset = (uint32_t)clut_offset;
    receipt.clut_length = 512U;
    receipt.clut_fnv1a64 = fnv1a64(source_bytes + clut_offset, 512U);
    receipt.pixel_offset = (uint32_t)pixel_offset;
    receipt.pixel_length = (uint32_t)view.pixel_bytes;
    receipt.pixel_fnv1a64 = fnv1a64(source_bytes + pixel_offset, view.pixel_bytes);
    *out_receipt = receipt;
    return 1;
}
