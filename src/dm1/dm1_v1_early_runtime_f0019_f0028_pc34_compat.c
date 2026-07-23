#include "dm1_v1_early_runtime_f0019_f0028_pc34_compat.h"

#include <string.h>

static int dm1_v1_early_runtime_graphic_valid_pc34(
    const DM1_V1_EarlyRuntimeGraphicPc34 *graphic)
{
    return graphic && graphic->indexed_pixels && graphic->indexed_pixel_byte_count > 0u &&
        graphic->graphics_dat_fingerprint != 0u &&
        graphic->decoded_from_original_graphics_dat && graphic->raw_record_verified &&
        graphic->no_synthetic_surface;
}

int dm1_v1_f0019_display_error_plan_pc34(
    int16_t error_number,
    int original_pc34_text_route_verified,
    int no_host_termination,
    int no_synthetic_error_surface,
    DM1_V1_F0019ErrorPlanPc34 *out_plan)
{
    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!out_plan || !original_pc34_text_route_verified || !no_host_termination ||
        !no_synthetic_error_surface) return 0;
    out_plan->accepted = 1;
    out_plan->text_color = DM1_V1_F0019_ERROR_TEXT_COLOR_PC34;
    out_plan->tens_character = (char)((error_number / 10) + '0');
    out_plan->ones_character = (char)((error_number % 10) + '0');
    out_plan->require_input_wait = 1;
    out_plan->require_endgame_boundary = 1;
    out_plan->suppress_host_termination = 1;
    return 1;
}

int dm1_v1_f0020_f0021_blit_admission_pc34(
    const DM1_V1_F0020F0021BlitRequestPc34 *request,
    DM1_V1_F0020F0021BlitReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !dm1_v1_early_runtime_graphic_valid_pc34(request->graphic) ||
        !request->source_box_xyxy || !request->source_box_verified ||
        (request->destination_target != DM1_V1_F0020_TARGET_VIEWPORT_PC34 &&
         request->destination_target != DM1_V1_F0021_TARGET_SCREEN_PC34) ||
        !request->no_host_renderer_fallback) return 0;
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->destination_target = request->destination_target;
        out_receipt->transparent_color = request->transparent_color;
        out_receipt->raw_graphic_bound = 1;
        out_receipt->suppress_synthetic_fallback = 1;
    }
    return 1;
}

int dm1_v1_f0022_delay_plan_pc34(
    uint16_t vertical_blank_count,
    int original_vblank_route_verified,
    int no_host_delay_fallback,
    DM1_V1_F0022DelayPlanPc34 *out_plan)
{
    if (out_plan) memset(out_plan, 0, sizeof(*out_plan));
    if (!out_plan || !original_vblank_route_verified || !no_host_delay_fallback) return 0;
    out_plan->accepted = 1;
    out_plan->vertical_blank_count = vertical_blank_count;
    out_plan->require_original_vblank_route = 1;
    out_plan->suppress_host_delay_fallback = 1;
    return 1;
}

int16_t dm1_v1_f0023_get_absolute_value_pc34(int16_t value)
{
    if (value == INT16_MIN) return INT16_MIN;
    return value < 0 ? (int16_t)-value : value;
}

int16_t dm1_v1_f0025_get_maximum_value_pc34(int16_t value1, int16_t value2)
{
    return value1 > value2 ? value1 : value2;
}

uint16_t dm1_v1_f0027_get_16bit_random_number_pc34(uint32_t *in_out_seed)
{
    if (!in_out_seed) return 0u;
    *in_out_seed = *in_out_seed * UINT32_C(0xBB40E62D) + UINT32_C(11);
    return (uint16_t)(*in_out_seed >> 8);
}

uint16_t dm1_v1_f0028_get_1bit_random_number_pc34(uint32_t *in_out_seed)
{
    return (uint16_t)(dm1_v1_f0027_get_16bit_random_number_pc34(in_out_seed) & 1u);
}

const char *dm1_v1_early_runtime_f0019_f0028_source_evidence_pc34(void)
{
    return "ReDMCSB BASE.C:1117-1166 F0019 PC34 text/wait/endgame order; "
           "BASE.C:1246-1288 F0020 and :1394-1446 F0021 raw bitmap blits; "
           "BASE.C:1584-1729 F0022 VBlank delay, F0023/F0025 signed helpers, "
           "and F0027/F0028 LCG 0xBB40E62D plus 11.";
}
