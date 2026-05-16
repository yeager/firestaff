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
} M11_PF_Color;

typedef struct {
    M11_PF_Color top_bottom[16];
    M11_PF_Color middle[16];
    bool update_middle;
    bool update_top_bottom;
} M11_PF_PaletteState;

typedef struct {
    uint8_t* base_font;
    uint8_t* custom_font;
    int16_t text_color;
    int16_t bg_color;
} M11_PF_FontState;

void m11_pf_palette_init(M11_PF_PaletteState* state);
void m11_pf_set_palette(M11_PF_PaletteState* state, const uint16_t* colors, int count);
const uint16_t* m11_pf_get_default_palette(void);

void m11_pf_font_init(M11_PF_FontState* state);
bool m11_pf_font_alloc(M11_PF_FontState* state);
void m11_pf_build_custom_colors(M11_PF_FontState* state, int16_t text, int16_t bg);

void m11_pf_draw_char(const M11_PF_FontState* state, uint8_t* fb, int x, int y, unsigned char ch, int scr_w);
void m11_pf_draw_string(const M11_PF_FontState* state, uint8_t* fb, int x, int y, const char* str, int scr_w);

const char* m11_pf_get_skill_name(int level);
void m11_pf_convert_portrait_planar(uint8_t* buf);

#ifdef __cplusplus
}
#endif

#endif
