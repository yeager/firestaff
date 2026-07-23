#ifndef FIRESTAFF_DM1_V1_F0442_F0443_CREDITS_TEXT_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0442_F0443_CREDITS_TEXT_MATERIAL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0442_CREDITS_GRAPHIC_PC34 = 5,
    DM1_V1_F0442_SCREEN_WIDTH_PC34 = 320,
    DM1_V1_F0442_SCREEN_HEIGHT_PC34 = 200,
    DM1_V1_F0442_CREDITS_WAIT_VBLANKS_PC34 = 1800,
    DM1_V1_F0442_PALETTE_BYTE_COUNT_PC34 = 48,
    DM1_V1_F0443_MAX_TEXT_BYTES_PC34 = 49,
    DM1_V1_F0443_BACKGROUND_COLOR_PC34 = 12
};

typedef struct DM1_V1_F0442CreditsMaterialRequestPc34 {
    const uint8_t *c005_indexed_pixels;
    size_t c005_pixel_byte_count;
    const uint8_t *credits_palette_rgb6;
    size_t credits_palette_byte_count;
    uint32_t c005_record_fingerprint;
    uint32_t palette_record_fingerprint;
    int original_graphics_dat_c005;
    int original_palette_verified;
    int decoded_indexed_pixels_verified;
    int no_synthetic_credits_page;
    int no_host_wrapper;
} DM1_V1_F0442CreditsMaterialRequestPc34;

typedef struct DM1_V1_F0442CreditsMaterialReceiptPc34 {
    int accepted;
    int c005_credits_bound;
    int credits_palette_bound;
    unsigned int wait_vblank_count;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0442CreditsMaterialReceiptPc34;

typedef struct DM1_V1_F0443EndgameTextRequestPc34 {
    int x;
    int y;
    int text_color;
    const char *source_text;
    const uint8_t *scroll_font_bytes;
    size_t scroll_font_byte_count;
    uint32_t source_text_fingerprint;
    uint32_t scroll_font_fingerprint;
    int source_text_verified;
    int original_scroll_font_verified;
    int no_host_font;
    int no_synthetic_text;
} DM1_V1_F0443EndgameTextRequestPc34;

typedef struct DM1_V1_F0443EndgameTextReceiptPc34 {
    int accepted;
    int x;
    int y;
    int text_color;
    int background_color;
    char scroll_font_text[DM1_V1_F0443_MAX_TEXT_BYTES_PC34 + 1];
    int source_text_bound;
    int scroll_font_bound;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0443EndgameTextReceiptPc34;

int dm1_v1_f0442_entrance_credits_material_admission_pc34(
    const DM1_V1_F0442CreditsMaterialRequestPc34 *request,
    DM1_V1_F0442CreditsMaterialReceiptPc34 *out_receipt);

int dm1_v1_f0443_endgame_print_string_material_admission_pc34(
    const DM1_V1_F0443EndgameTextRequestPc34 *request,
    DM1_V1_F0443EndgameTextReceiptPc34 *out_receipt);

const char *dm1_v1_f0442_f0443_credits_text_material_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0442_F0443_CREDITS_TEXT_MATERIAL_PC34_COMPAT_H */
