/* skproject: c_gfx_pal.cpp / c_gfx_pal.h */
#ifndef FIRESTAFF_DM2_V1_GFX_PAL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_GFX_PAL_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>
#include "dm2_v1_gfx_pixel_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* --- Constants --- */
#define DM2_V1_PAL16  16
#define DM2_V1_PAL256 256
#define DM2_V1_DMFCOL (255.0 / 63.0)

/* --- Types --- */

/* Color conversion lookup entry */
typedef struct {
    uint8_t p;
} DM2_V1_ColorConv;

/* Single palette entry */
typedef struct {
    DM2_V1_Pixel256 c;
} DM2_V1_Palette;

/* Palette data container */
typedef struct {
    DM2_V1_Palette *palette;
    DM2_V1_Palette *pal16to256ptr;
    DM2_V1_Palette small_palette[16];
    DM2_V1_Palette *glbl_pal1;
    DM2_V1_Palette *glbl_pal2;
    bool immediate_colors;
} DM2_V1_PaletteData;

/* --- Callback interface --- */

typedef struct {
    void (*wait_for_vsync)(void *ctx);
    void (*set_screen_color)(void *ctx, int index, float r, float g, float b);
    void (*get_screen_color)(void *ctx, int index, float *r, float *g, float *b);
} DM2_V1_GfxPalCallbacks;

/* --- Receipt structs --- */

typedef struct {
    bool colors_set;
    int16_t colors_count;
} DM2_V1_DriverSetColorsReceipt;

typedef struct {
    bool converted;
    int16_t entries_count;
} DM2_V1_ConvertDriverPaletteReceipt;

typedef struct {
    bool applied;
    int16_t mode;
} DM2_V1_SelectPaletteSetReceipt;

typedef struct {
    bool translated;
    int16_t final_colors;
} DM2_V1_XlatPaletteReceipt;

/* --- Default palette data (768 bytes, 6-bit DAC values) --- */
extern const int8_t DM2_V1_DMPAL_DEFAULT[768];

/* --- Functions --- */

void dm2_v1_color_to_palettecolor(DM2_V1_Palette *pal, DM2_V1_EColor color);
void dm2_v1_ui8_to_palettecolor(DM2_V1_Palette *pal, uint8_t val);
uint8_t dm2_v1_palettecolor_to_ui8(DM2_V1_Palette pal);
DM2_V1_Pixel256 dm2_v1_palettecolor_to_pixel(DM2_V1_Palette pal);
void dm2_v1_palette_conv(DM2_V1_Palette *pal, const DM2_V1_ColorConv *conv);
void dm2_v1_palette_data_init(DM2_V1_PaletteData *pd);

DM2_V1_DriverSetColorsReceipt dm2_v1_driver_setcolors(
    const DM2_V1_GfxPalCallbacks *cb, void *ctx, const int8_t *dmpal);

DM2_V1_ConvertDriverPaletteReceipt dm2_v1_convert_driver_palette(
    const DM2_V1_GfxPalCallbacks *cb, void *ctx,
    const uint8_t *pb, int8_t *dmpal, bool immediate_colors);

DM2_V1_SelectPaletteSetReceipt dm2_v1_select_palette_set(
    const DM2_V1_GfxPalCallbacks *cb, void *ctx,
    int16_t set, const int8_t *dmpal, bool *immediate_colors_ptr);

void dm2_v1_update_blit_palette(DM2_V1_PaletteData *pd, DM2_V1_Palette *pal);

DM2_V1_XlatPaletteReceipt dm2_v1_xlat_palette(
    DM2_V1_Palette *pal, const DM2_V1_ColorConv *conv, int16_t *colors);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_GFX_PAL_PC34_COMPAT_H */
