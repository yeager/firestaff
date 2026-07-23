#ifndef FIRESTAFF_DM1_V1_F0437_F0438_F0439_STARTUP_VISUAL_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0437_F0438_F0439_STARTUP_VISUAL_ADMISSION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_STARTUP_PALETTE_COLOR_COUNT_PC34 = 16,
    DM1_V1_STARTUP_PALETTE_BYTE_COUNT_PC34 = 48,
    DM1_V1_STARTUP_C001_WIDTH_PC34 = 320,
    DM1_V1_STARTUP_C001_HEIGHT_PC34 = 200,
    DM1_V1_STARTUP_C004_WIDTH_PC34 = 320,
    DM1_V1_STARTUP_C004_HEIGHT_PC34 = 200,
    DM1_V1_STARTUP_DOOR_WIDTH_PC34 = 256,
    DM1_V1_STARTUP_DOOR_HEIGHT_PC34 = 161
};

typedef enum DM1_V1_StartupVisualGraphicRolePc34 {
    DM1_V1_STARTUP_VISUAL_C001_TITLE_PC34 = 1,
    DM1_V1_STARTUP_VISUAL_C002_LEFT_DOORS_PC34 = 2,
    DM1_V1_STARTUP_VISUAL_C003_RIGHT_DOORS_PC34 = 3,
    DM1_V1_STARTUP_VISUAL_C004_ENTRANCE_PC34 = 4
} DM1_V1_StartupVisualGraphicRolePc34;

/* Decoder output only. A host-created page, font, or wrapper cannot qualify. */
typedef struct DM1_V1_StartupVisualGraphicPc34 {
    DM1_V1_StartupVisualGraphicRolePc34 role;
    const uint8_t *pixels;
    size_t pixel_byte_count;
    unsigned int width;
    unsigned int height;
    uint32_t graphics_dat_record_fingerprint;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int indexed_pixels_verified;
    int no_synthetic_wrapper;
    int no_replacement_font;
} DM1_V1_StartupVisualGraphicPc34;

typedef struct DM1_V1_StartupVisualPalettePc34 {
    const uint8_t *rgb6_bytes;
    size_t byte_count;
    uint32_t source_fingerprint;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int no_synthetic_palette;
} DM1_V1_StartupVisualPalettePc34;

typedef struct DM1_V1_F0437TitleMaterialReceiptPc34 {
    int accepted;
    int c001_title_bound;
    int presents_region_bound;
    int dungeon_region_bound;
    int master_region_bound;
    int presents_palette_bound;
    int title_palette_bound;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0437TitleMaterialReceiptPc34;

typedef struct DM1_V1_F0438DoorMaterialReceiptPc34 {
    int accepted;
    int c004_entrance_bound;
    int c002_left_door_bound;
    int c003_right_door_bound;
    int entrance_palette_bound;
    unsigned int source_step_count;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0438DoorMaterialReceiptPc34;

typedef struct DM1_V1_F0439EntranceMaterialReceiptPc34 {
    int accepted;
    int c004_entrance_bound;
    int entrance_palette_bound;
    int no_title_material_substitution;
    int suppress_synthetic_fallback;
    const char *source_evidence;
} DM1_V1_F0439EntranceMaterialReceiptPc34;

int dm1_v1_f0437_title_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c001_title,
    const DM1_V1_StartupVisualPalettePc34 *presents_palette,
    const DM1_V1_StartupVisualPalettePc34 *title_palette,
    DM1_V1_F0437TitleMaterialReceiptPc34 *out_receipt);

int dm1_v1_f0438_door_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c004_entrance,
    const DM1_V1_StartupVisualGraphicPc34 *c002_left_doors,
    const DM1_V1_StartupVisualGraphicPc34 *c003_right_doors,
    const DM1_V1_StartupVisualPalettePc34 *entrance_palette,
    DM1_V1_F0438DoorMaterialReceiptPc34 *out_receipt);

int dm1_v1_f0439_entrance_material_admission_pc34(
    const DM1_V1_StartupVisualGraphicPc34 *c004_entrance,
    const DM1_V1_StartupVisualPalettePc34 *entrance_palette,
    DM1_V1_F0439EntranceMaterialReceiptPc34 *out_receipt);

const char *dm1_v1_f0437_f0438_f0439_startup_visual_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0437_F0438_F0439_STARTUP_VISUAL_ADMISSION_PC34_COMPAT_H */
