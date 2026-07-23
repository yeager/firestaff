#include "dm1_v1_early_runtime_f0019_f0028_pc34_compat.h"
#include "dm1_v1_main_math_f0024_f0026_f0030_pc34_compat.h"

#include <limits.h>
#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    const uint8_t pixels[8] = {1};
    const int16_t box[4] = {0, 0, 1, 1};
    DM1_V1_EarlyRuntimeGraphicPc34 graphic;
    DM1_V1_F0019ErrorPlanPc34 error_plan;
    DM1_V1_F0020F0021BlitRequestPc34 blit;
    DM1_V1_F0020F0021BlitReceiptPc34 blit_receipt;
    DM1_V1_F0022DelayPlanPc34 delay_plan;
    uint32_t seed_a = 1u;
    uint32_t seed_b = 1u;

    CHECK(dm1_v1_f0019_display_error_plan_pc34(42, 1, 1, 1, &error_plan));
    CHECK(error_plan.accepted && error_plan.text_color == 8 &&
          error_plan.tens_character == '4' && error_plan.ones_character == '2' &&
          error_plan.require_input_wait && error_plan.require_endgame_boundary &&
          error_plan.suppress_host_termination);
    CHECK(!dm1_v1_f0019_display_error_plan_pc34(42, 1, 0, 1, &error_plan));

    memset(&graphic, 0, sizeof(graphic));
    graphic.indexed_pixels = pixels;
    graphic.indexed_pixel_byte_count = sizeof(pixels);
    graphic.graphics_dat_fingerprint = 1u;
    graphic.decoded_from_original_graphics_dat = 1;
    graphic.raw_record_verified = 1;
    graphic.no_synthetic_surface = 1;
    memset(&blit, 0, sizeof(blit));
    blit.graphic = &graphic;
    blit.source_box_xyxy = box;
    blit.source_box_verified = 1;
    blit.destination_target = DM1_V1_F0020_TARGET_VIEWPORT_PC34;
    blit.transparent_color = -1;
    blit.no_host_renderer_fallback = 1;
    CHECK(dm1_v1_f0020_f0021_blit_admission_pc34(&blit, &blit_receipt));
    CHECK(blit_receipt.accepted && blit_receipt.raw_graphic_bound &&
          blit_receipt.destination_target == DM1_V1_F0020_TARGET_VIEWPORT_PC34 &&
          blit_receipt.suppress_synthetic_fallback);
    blit.destination_target = DM1_V1_F0021_TARGET_SCREEN_PC34;
    CHECK(dm1_v1_f0020_f0021_blit_admission_pc34(&blit, &blit_receipt));
    blit.graphic = 0;
    CHECK(!dm1_v1_f0020_f0021_blit_admission_pc34(&blit, &blit_receipt));

    CHECK(dm1_v1_f0022_delay_plan_pc34(20u, 1, 1, &delay_plan));
    CHECK(delay_plan.accepted && delay_plan.vertical_blank_count == 20u &&
          delay_plan.require_original_vblank_route && delay_plan.suppress_host_delay_fallback);
    CHECK(!dm1_v1_f0022_delay_plan_pc34(20u, 0, 1, &delay_plan));

    CHECK(dm1_v1_f0023_get_absolute_value_pc34(-14) == 14);
    CHECK(dm1_v1_f0023_get_absolute_value_pc34(INT16_MIN) == INT16_MIN);
    CHECK(F0024_MAIN_GetMinimumValue(9, 4) == 4);
    CHECK(dm1_v1_f0025_get_maximum_value_pc34(9, 4) == 9);
    CHECK(F0026_MAIN_GetBoundedValue(0, 101, 100) == 100);
    CHECK(dm1_v1_f0027_get_16bit_random_number_pc34(&seed_a) == 16614u);
    CHECK(dm1_v1_f0028_get_1bit_random_number_pc34(&seed_a) == 1u);
    CHECK(dm1_v1_f0027_get_16bit_random_number_pc34(&seed_b) == 16614u);
    CHECK(dm1_v1_f0027_get_16bit_random_number_pc34(0) == 0u);
    CHECK(strstr(dm1_v1_early_runtime_f0019_f0028_source_evidence_pc34(), "F0027/F0028") != NULL);

    printf("test_dm1_v1_early_runtime_f0019_f0028_pc34_compat: %d assertions, %d failures\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
