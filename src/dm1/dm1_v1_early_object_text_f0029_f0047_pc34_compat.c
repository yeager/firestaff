#include "dm1_v1_early_object_text_f0029_f0047_pc34_compat.h"

#include <string.h>

static int dm1_v1_has_nonzero_bytes_pc34(const uint8_t *bytes, size_t count)
{
    size_t index;
    for (index = 0u; index < count; ++index) {
        if (bytes[index] != 0u) return 1;
    }
    return 0;
}

uint16_t dm1_v1_f0029_get_2bit_random_number_pc34(uint32_t *in_out_seed)
{
    if (!in_out_seed) return 0u;
    *in_out_seed = *in_out_seed * UINT32_C(0xBB40E62D) + UINT32_C(11);
    return (uint16_t)((*in_out_seed >> 8) & 3u);
}

int dm1_v1_f0031_object_names_admission_pc34(
    const DM1_V1_F0031ObjectNamesRequestPc34 *request,
    DM1_V1_F0031ObjectNamesReceiptPc34 *out_receipt)
{
    size_t index;
    int terminators = 0;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->raw_names || request->raw_names_byte_count == 0u ||
        request->graphics_dat_fingerprint == 0u ||
        request->graphic_index != DM1_V1_F0031_OBJECT_NAMES_GRAPHIC_PC34 ||
        !request->decoded_from_original_graphics_dat || !request->raw_record_verified ||
        !request->no_synthetic_names ||
        !dm1_v1_has_nonzero_bytes_pc34(request->raw_names,
                                        request->raw_names_byte_count)) return 0;
    for (index = 0u; index < request->raw_names_byte_count; ++index) {
        if (request->raw_names[index] == 0u) ++terminators;
    }
    if (terminators < DM1_V1_F0031_OBJECT_NAME_COUNT_PC34) return 0;
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->raw_names_bound = 1;
        out_receipt->name_count = DM1_V1_F0031_OBJECT_NAME_COUNT_PC34;
        out_receipt->suppress_synthetic_fallback = 1;
    }
    return 1;
}

int dm1_v1_f0036_icon_atlas_admission_pc34(
    const DM1_V1_F0036IconAtlasRequestPc34 *request,
    DM1_V1_F0036IconAtlasReceiptPc34 *out_receipt)
{
    int sheet;
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->raw_atlas_pixels || request->raw_atlas_byte_count == 0u ||
        request->graphics_dat_fingerprint == 0u || !request->first_icon_indices ||
        request->icon_index < 0 || !request->decoded_from_original_graphics_dat ||
        !request->raw_record_verified || !request->no_synthetic_surface ||
        !dm1_v1_has_nonzero_bytes_pc34(request->raw_atlas_pixels,
                                        request->raw_atlas_byte_count)) return 0;
    for (sheet = 1; sheet < DM1_V1_F0036_ICON_SHEET_COUNT_PC34; ++sheet) {
        if (request->first_icon_indices[sheet] <= request->first_icon_indices[sheet - 1]) return 0;
    }
    for (sheet = 1; sheet < DM1_V1_F0036_ICON_SHEET_COUNT_PC34; ++sheet) {
        if (request->first_icon_indices[sheet] > (uint16_t)request->icon_index) break;
    }
    --sheet;
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->source_sheet_ordinal = sheet;
        out_receipt->source_local_icon_index =
            request->icon_index - request->first_icon_indices[sheet];
        out_receipt->raw_atlas_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
    }
    return 1;
}

int dm1_v1_f0040_f0047_text_material_admission_pc34(
    const DM1_V1_F0040F0047TextMaterialRequestPc34 *request,
    DM1_V1_F0040F0047TextMaterialReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->raw_font_bytes || request->raw_font_byte_count == 0u ||
        !request->raw_message_bytes || request->raw_message_byte_count == 0u ||
        request->font_fingerprint == 0u || request->message_fingerprint == 0u ||
        !request->original_pc34_font_verified || !request->original_message_route_verified ||
        !request->no_host_font || !request->no_synthetic_text ||
        !dm1_v1_has_nonzero_bytes_pc34(request->raw_font_bytes, request->raw_font_byte_count)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->font_bound = 1;
        out_receipt->message_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
    }
    return 1;
}

const char *dm1_v1_early_object_text_f0029_f0047_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:1688-1730 F0029 LCG; OBJECT.C:25-119 F0031 "
           "C564 object names and :288-342 F0036 seven-sheet icon selection; "
           "TEXT.C:372-1765 F0040-F0047 original font/message-area chain.";
}
