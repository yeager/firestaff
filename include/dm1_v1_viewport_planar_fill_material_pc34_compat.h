#ifndef FIRESTAFF_DM1_V1_VIEWPORT_PLANAR_FILL_MATERIAL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_PLANAR_FILL_MATERIAL_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB ACTIDRAW.C F0134 and FILLBOX.C F0135 consume caller-owned planar
 * bitmap surfaces. This material gate represents an already decoded original
 * viewport surface; no color-only or generated substitute is admitted. */
typedef struct DM1_V1_ViewportPlanarFillMaterialPc34 {
    uint8_t *bitmap;
    size_t bitmap_size;
    size_t row_bytes;
    size_t pixel_height;
    int original_material_verified;
} DM1_V1_ViewportPlanarFillMaterialPc34;

/* F0134 fills the admitted material in 16-pixel planar units. */
int dm1_v1_viewport_fill_material_f0134_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    uint8_t color);

/* F0135 fills one inclusive viewport box on the admitted material. */
int dm1_v1_viewport_fill_material_box_f0135_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    const int16_t box[4],
    uint16_t color);

/* PANEL.C F0344: draw the source proportional food/water bar by filling its
 * black shadow first, then the selected warning/base color. full_box uses
 * inclusive coordinates from the caller-owned original layout material. */
typedef struct DM1_V1_ViewportFoodWaterFillReceiptPc34 {
    int drawn;
    int proportional_units;
    uint16_t bar_color;
    int16_t bar_box[4];
    int16_t shadow_box[4];
} DM1_V1_ViewportFoodWaterFillReceiptPc34;

int dm1_v1_viewport_fill_food_water_bar_f0344_pc34(
    DM1_V1_ViewportPlanarFillMaterialPc34 *material,
    const int16_t full_box[4],
    int amount,
    uint16_t base_color,
    DM1_V1_ViewportFoodWaterFillReceiptPc34 *receipt);

const char *dm1_v1_viewport_planar_fill_material_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
