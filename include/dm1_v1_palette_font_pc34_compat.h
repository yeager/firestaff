#ifndef FIRESTAFF_DM1_V1_PALETTE_FONT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PALETTE_FONT_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_PALETTE_SIZE 16
#define DM1_FONT_CHAR_W 8
#define DM1_FONT_CHAR_H 6
#define DM1_FONT_CHARS 128
#define DM1_PORTRAIT_W 32
#define DM1_PORTRAIT_H 29
#define DM1_SKILL_LEVEL_COUNT 15

typedef struct {
    uint16_t rgb12;
} DM1_V1_PaletteFontColorPc34;

typedef struct {
    DM1_V1_PaletteFontColorPc34 top_bottom[16];
    DM1_V1_PaletteFontColorPc34 middle[16];
    bool update_middle;
    bool update_top_bottom;
} DM1_V1_PaletteFontPaletteStatePc34;

typedef struct {
    uint8_t* base_font;
    uint8_t* custom_font;
    int16_t text_color;
    int16_t bg_color;
} DM1_V1_PaletteFontFontStatePc34;

void DM1_V1_PaletteFont_PaletteInitPc34Compat(DM1_V1_PaletteFontPaletteStatePc34* state);
void DM1_V1_PaletteFont_SetPalettePc34Compat(DM1_V1_PaletteFontPaletteStatePc34* state, const uint16_t* colors, int count);
const uint16_t* DM1_V1_PaletteFont_GetDefaultPalettePc34Compat(void);

void DM1_V1_PaletteFont_FontInitPc34Compat(DM1_V1_PaletteFontFontStatePc34* state);
bool DM1_V1_PaletteFont_FontAllocPc34Compat(DM1_V1_PaletteFontFontStatePc34* state);
void DM1_V1_PaletteFont_BuildCustomColorsPc34Compat(DM1_V1_PaletteFontFontStatePc34* state, int16_t text, int16_t bg);

void DM1_V1_PaletteFont_DrawCharPc34Compat(const DM1_V1_PaletteFontFontStatePc34* state, uint8_t* fb, int x, int y, unsigned char ch, int scr_w);
void DM1_V1_PaletteFont_DrawStringPc34Compat(const DM1_V1_PaletteFontFontStatePc34* state, uint8_t* fb, int x, int y, const char* str, int scr_w);

const char* DM1_V1_PaletteFont_GetSkillNamePc34Compat(int level);
void DM1_V1_PaletteFont_ConvertPortraitPlanarPc34Compat(uint8_t* buf);

/* Compatibility aliases for older M11 call sites. */
typedef DM1_V1_PaletteFontColorPc34 M11_PF_Color;
typedef DM1_V1_PaletteFontPaletteStatePc34 M11_PF_PaletteState;
typedef DM1_V1_PaletteFontFontStatePc34 M11_PF_FontState;
#define m11_pf_palette_init DM1_V1_PaletteFont_PaletteInitPc34Compat
#define m11_pf_set_palette DM1_V1_PaletteFont_SetPalettePc34Compat
#define m11_pf_get_default_palette DM1_V1_PaletteFont_GetDefaultPalettePc34Compat
#define m11_pf_font_init DM1_V1_PaletteFont_FontInitPc34Compat
#define m11_pf_font_alloc DM1_V1_PaletteFont_FontAllocPc34Compat
#define m11_pf_build_custom_colors DM1_V1_PaletteFont_BuildCustomColorsPc34Compat
#define m11_pf_draw_char DM1_V1_PaletteFont_DrawCharPc34Compat
#define m11_pf_draw_string DM1_V1_PaletteFont_DrawStringPc34Compat
#define m11_pf_get_skill_name DM1_V1_PaletteFont_GetSkillNamePc34Compat
#define m11_pf_convert_portrait_planar DM1_V1_PaletteFont_ConvertPortraitPlanarPc34Compat

#ifdef __cplusplus
}
#endif

#endif
