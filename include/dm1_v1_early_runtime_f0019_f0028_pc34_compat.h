#ifndef FIRESTAFF_DM1_V1_EARLY_RUNTIME_F0019_F0028_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_EARLY_RUNTIME_F0019_F0028_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0019_ERROR_TEXT_COLOR_PC34 = 8,
    DM1_V1_F0020_TARGET_VIEWPORT_PC34 = 1,
    DM1_V1_F0021_TARGET_SCREEN_PC34 = 2
};

typedef struct DM1_V1_EarlyRuntimeGraphicPc34 {
    const uint8_t *indexed_pixels;
    size_t indexed_pixel_byte_count;
    uint32_t graphics_dat_fingerprint;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int no_synthetic_surface;
} DM1_V1_EarlyRuntimeGraphicPc34;

typedef struct DM1_V1_F0019ErrorPlanPc34 {
    int accepted;
    int text_color;
    char tens_character;
    char ones_character;
    int require_input_wait;
    int require_endgame_boundary;
    int suppress_host_termination;
} DM1_V1_F0019ErrorPlanPc34;

typedef struct DM1_V1_F0020F0021BlitRequestPc34 {
    const DM1_V1_EarlyRuntimeGraphicPc34 *graphic;
    const int16_t *source_box_xyxy;
    int source_box_verified;
    int destination_target;
    int transparent_color;
    int no_host_renderer_fallback;
} DM1_V1_F0020F0021BlitRequestPc34;

typedef struct DM1_V1_F0020F0021BlitReceiptPc34 {
    int accepted;
    int destination_target;
    int transparent_color;
    int raw_graphic_bound;
    int suppress_synthetic_fallback;
} DM1_V1_F0020F0021BlitReceiptPc34;

typedef struct DM1_V1_F0022DelayPlanPc34 {
    int accepted;
    uint16_t vertical_blank_count;
    int require_original_vblank_route;
    int suppress_host_delay_fallback;
} DM1_V1_F0022DelayPlanPc34;

int dm1_v1_f0019_display_error_plan_pc34(
    int16_t error_number,
    int original_pc34_text_route_verified,
    int no_host_termination,
    int no_synthetic_error_surface,
    DM1_V1_F0019ErrorPlanPc34 *out_plan);

int dm1_v1_f0020_f0021_blit_admission_pc34(
    const DM1_V1_F0020F0021BlitRequestPc34 *request,
    DM1_V1_F0020F0021BlitReceiptPc34 *out_receipt);

int dm1_v1_f0022_delay_plan_pc34(
    uint16_t vertical_blank_count,
    int original_vblank_route_verified,
    int no_host_delay_fallback,
    DM1_V1_F0022DelayPlanPc34 *out_plan);

int16_t dm1_v1_f0023_get_absolute_value_pc34(int16_t value);
int16_t dm1_v1_f0025_get_maximum_value_pc34(int16_t value1, int16_t value2);
uint16_t dm1_v1_f0027_get_16bit_random_number_pc34(uint32_t *in_out_seed);
uint16_t dm1_v1_f0028_get_1bit_random_number_pc34(uint32_t *in_out_seed);

const char *dm1_v1_early_runtime_f0019_f0028_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_EARLY_RUNTIME_F0019_F0028_PC34_COMPAT_H */
