#ifndef FIRESTAFF_DM1_V1_EARLY_OBJECT_TEXT_F0029_F0047_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_EARLY_OBJECT_TEXT_F0029_F0047_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0031_OBJECT_NAMES_GRAPHIC_PC34 = 564,
    DM1_V1_F0031_OBJECT_NAME_COUNT_PC34 = 199,
    DM1_V1_F0036_ICON_SHEET_COUNT_PC34 = 7
};

typedef struct DM1_V1_F0031ObjectNamesRequestPc34 {
    const uint8_t *raw_names;
    size_t raw_names_byte_count;
    uint32_t graphics_dat_fingerprint;
    int graphic_index;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int no_synthetic_names;
} DM1_V1_F0031ObjectNamesRequestPc34;

typedef struct DM1_V1_F0031ObjectNamesReceiptPc34 {
    int accepted;
    int raw_names_bound;
    int name_count;
    int suppress_synthetic_fallback;
} DM1_V1_F0031ObjectNamesReceiptPc34;

typedef struct DM1_V1_F0036IconAtlasRequestPc34 {
    const uint8_t *raw_atlas_pixels;
    size_t raw_atlas_byte_count;
    uint32_t graphics_dat_fingerprint;
    const uint16_t *first_icon_indices;
    int icon_index;
    int decoded_from_original_graphics_dat;
    int raw_record_verified;
    int no_synthetic_surface;
} DM1_V1_F0036IconAtlasRequestPc34;

typedef struct DM1_V1_F0036IconAtlasReceiptPc34 {
    int accepted;
    int source_sheet_ordinal;
    int source_local_icon_index;
    int raw_atlas_bound;
    int suppress_synthetic_fallback;
} DM1_V1_F0036IconAtlasReceiptPc34;

typedef struct DM1_V1_F0040F0047TextMaterialRequestPc34 {
    const uint8_t *raw_font_bytes;
    size_t raw_font_byte_count;
    uint32_t font_fingerprint;
    const uint8_t *raw_message_bytes;
    size_t raw_message_byte_count;
    uint32_t message_fingerprint;
    int original_pc34_font_verified;
    int original_message_route_verified;
    int no_host_font;
    int no_synthetic_text;
} DM1_V1_F0040F0047TextMaterialRequestPc34;

typedef struct DM1_V1_F0040F0047TextMaterialReceiptPc34 {
    int accepted;
    int font_bound;
    int message_bound;
    int suppress_synthetic_fallback;
} DM1_V1_F0040F0047TextMaterialReceiptPc34;

uint16_t dm1_v1_f0029_get_2bit_random_number_pc34(uint32_t *in_out_seed);
int dm1_v1_f0031_object_names_admission_pc34(
    const DM1_V1_F0031ObjectNamesRequestPc34 *request,
    DM1_V1_F0031ObjectNamesReceiptPc34 *out_receipt);
int dm1_v1_f0036_icon_atlas_admission_pc34(
    const DM1_V1_F0036IconAtlasRequestPc34 *request,
    DM1_V1_F0036IconAtlasReceiptPc34 *out_receipt);
int dm1_v1_f0040_f0047_text_material_admission_pc34(
    const DM1_V1_F0040F0047TextMaterialRequestPc34 *request,
    DM1_V1_F0040F0047TextMaterialReceiptPc34 *out_receipt);
const char *dm1_v1_early_object_text_f0029_f0047_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_EARLY_OBJECT_TEXT_F0029_F0047_PC34_COMPAT_H */
