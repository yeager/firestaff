#include "dm1_v1_f0442_f0443_credits_text_material_pc34_compat.h"

#include <string.h>

static int dm1_v1_nonzero_bytes_pc34(const uint8_t *bytes, size_t count)
{
    size_t index;
    for (index = 0u; index < count; ++index) {
        if (bytes[index] != 0u) return 1;
    }
    return 0;
}

static int dm1_v1_palette_rgb6_valid_pc34(const uint8_t *bytes, size_t count)
{
    size_t index;

    if (!bytes || count != DM1_V1_F0442_PALETTE_BYTE_COUNT_PC34 ||
        !dm1_v1_nonzero_bytes_pc34(bytes, count)) return 0;
    for (index = 0u; index < count; ++index) {
        if (bytes[index] > 63u) return 0;
    }
    return 1;
}

static int dm1_v1_f0443_copy_scroll_text_pc34(const char *source, char *out)
{
    size_t index;

    if (!source || !out) return 0;
    for (index = 0u; index <= DM1_V1_F0443_MAX_TEXT_BYTES_PC34; ++index) {
        const unsigned char character = (unsigned char)source[index];
        if (character == 0u) {
            out[index] = '\0';
            return 1;
        }
        if (index == DM1_V1_F0443_MAX_TEXT_BYTES_PC34) return 0;
        out[index] = (character >= (unsigned char)'A' &&
                      character <= (unsigned char)'Z')
            ? (char)(character - 64u)
            : (char)character;
    }
    return 0;
}

int dm1_v1_f0442_entrance_credits_material_admission_pc34(
    const DM1_V1_F0442CreditsMaterialRequestPc34 *request,
    DM1_V1_F0442CreditsMaterialReceiptPc34 *out_receipt)
{
    const size_t required_bytes =
        (size_t)DM1_V1_F0442_SCREEN_WIDTH_PC34 * DM1_V1_F0442_SCREEN_HEIGHT_PC34;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->c005_indexed_pixels ||
        request->c005_pixel_byte_count < required_bytes ||
        request->c005_record_fingerprint == 0u ||
        request->palette_record_fingerprint == 0u ||
        !request->original_graphics_dat_c005 || !request->original_palette_verified ||
        !request->decoded_indexed_pixels_verified || !request->no_synthetic_credits_page ||
        !request->no_host_wrapper ||
        !dm1_v1_nonzero_bytes_pc34(request->c005_indexed_pixels, required_bytes) ||
        !dm1_v1_palette_rgb6_valid_pc34(
            request->credits_palette_rgb6, request->credits_palette_byte_count)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->c005_credits_bound = 1;
        out_receipt->credits_palette_bound = 1;
        out_receipt->wait_vblank_count = DM1_V1_F0442_CREDITS_WAIT_VBLANKS_PC34;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0442_f0443_credits_text_material_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0443_endgame_print_string_material_admission_pc34(
    const DM1_V1_F0443EndgameTextRequestPc34 *request,
    DM1_V1_F0443EndgameTextReceiptPc34 *out_receipt)
{
    DM1_V1_F0443EndgameTextReceiptPc34 receipt;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !request->source_text || !request->scroll_font_bytes ||
        request->scroll_font_byte_count == 0u ||
        request->source_text_fingerprint == 0u || request->scroll_font_fingerprint == 0u ||
        !request->source_text_verified || !request->original_scroll_font_verified ||
        !request->no_host_font || !request->no_synthetic_text) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_f0443_copy_scroll_text_pc34(request->source_text, receipt.scroll_font_text)) {
        return 0;
    }
    receipt.accepted = 1;
    receipt.x = request->x;
    receipt.y = request->y;
    receipt.text_color = request->text_color;
    receipt.background_color = DM1_V1_F0443_BACKGROUND_COLOR_PC34;
    receipt.source_text_bound = 1;
    receipt.scroll_font_bound = 1;
    receipt.suppress_synthetic_fallback = 1;
    receipt.source_evidence =
        dm1_v1_f0442_f0443_credits_text_material_source_evidence_pc34();
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

const char *dm1_v1_f0442_f0443_credits_text_material_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:993-1091 F0442 expands original C005 credits "
           "with G0019 credits palette and records a 1800-VBlank wait; "
           "ENDGAME.C:40-61 F0443 maps only A-Z to scroll-font codes by subtracting "
           "64, then passes the source string to F0053 with background color C12.";
}
