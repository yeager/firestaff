#include "dm1_v1_palette_font_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

/* G0347_aui_Palette_TopAndBottomScreen, from the original PC 3.4 data
 * declaration (DATA.C / BASE.C; also source-locked by
 * dm1_v1_palette_top_and_bottom_screen_pc34_compat.c).  The old table mixed
 * RGB565-style constants into this RGB12 API, which could never be emitted
 * unchanged by Amiga OCS COLOR registers.  Keep this as source-owned RGB4
 * nibbles; a dynamic Amiga gameplay palette remains separately capture-gated. */
static const uint16_t default_palette_data[DM1_PALETTE_SIZE] = {
    0x000, 0x666, 0x888, 0x620, 0x0CC, 0x840, 0x080, 0x0C0,
    0xF00, 0xFA0, 0xC86, 0xFF0, 0x444, 0xAAA, 0x00F, 0xFFF
};

void DM1_V1_PaletteFont_PaletteInitPc34Compat(DM1_V1_PaletteFontPaletteStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_PaletteFontPaletteStatePc34));
}

void DM1_V1_PaletteFont_SetPalettePc34Compat(DM1_V1_PaletteFontPaletteStatePc34* state, const uint16_t* colors, int count) {
    if (!state || !colors || count <= 0) return;
    int i;
    for (i = 0; i < count && i < DM1_PALETTE_SIZE; i++) {
        if (i < 8) {
            state->top_bottom[i].rgb12 = (uint16_t)(colors[i] & 0x0FFFu);
            state->update_top_bottom = true;
        } else {
            state->middle[i - 8].rgb12 = (uint16_t)(colors[i] & 0x0FFFu);
            state->update_middle = true;
        }
    }
}

const uint16_t* DM1_V1_PaletteFont_GetDefaultPalettePc34Compat(void) {
    return default_palette_data;
}

void DM1_V1_PaletteFont_FontInitPc34Compat(DM1_V1_PaletteFontFontStatePc34* state) {
    if (!state) return;
    state->base_font = NULL;
    state->custom_font = NULL;
    state->text_color = 0;
    state->bg_color = 0;
}

bool DM1_V1_PaletteFont_FontAllocPc34Compat(DM1_V1_PaletteFontFontStatePc34* state) {
    if (!state) return false;
    state->base_font = (uint8_t*)malloc(1024 * DM1_FONT_CHAR_H);
    state->custom_font = (uint8_t*)malloc(768);
    if (!state->base_font || !state->custom_font) {
        free(state->base_font);
        free(state->custom_font);
        state->base_font = NULL;
        state->custom_font = NULL;
        return false;
    }
    return true;
}

void DM1_V1_PaletteFont_BuildCustomColorsPc34Compat(DM1_V1_PaletteFontFontStatePc34* state, int16_t text, int16_t bg) {
    if (!state || !state->base_font || !state->custom_font) return;
    state->text_color = text;
    state->bg_color = bg;
    memcpy(state->custom_font, state->base_font, 768);
}

void DM1_V1_PaletteFont_DrawCharPc34Compat(const DM1_V1_PaletteFontFontStatePc34* state, uint8_t* fb, int x, int y, unsigned char ch, int scr_w) {
    if (!state || !fb || !state->custom_font) return;
    if ( ch >= DM1_FONT_CHARS) return;
    const uint8_t* glyph = state->custom_font + (ch * DM1_FONT_CHAR_H);
    int i, j;
    for (i = 0; i < DM1_FONT_CHAR_H; i++) {
        uint8_t row = glyph[i];
        int px = x;
        for (j = 0; j < DM1_FONT_CHAR_W; j++) {
            if (px < 0 || px >= scr_w) {
                row <<= 1;
                px++;
                continue;
            }
            int idx = (y + i) * scr_w + px;
            if (row & 0x80) {
                fb[idx] = (uint8_t)state->text_color;
            } else {
                fb[idx] = (uint8_t)state->bg_color;
            }
            row <<= 1;
            px++;
        }
    }
}

void DM1_V1_PaletteFont_DrawStringPc34Compat(const DM1_V1_PaletteFontFontStatePc34* state, uint8_t* fb, int x, int y, const char* str, int scr_w) {
    if (!str) return;
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += DM1_FONT_CHAR_H;
        } else {
            DM1_V1_PaletteFont_DrawCharPc34Compat(state, fb, cx, y, *str, scr_w);
            cx += DM1_FONT_CHAR_W;
        }
        str++;
    }
}

static const char* skill_names_data[DM1_SKILL_LEVEL_COUNT] = {
    "NEOPHYTE", "NOVICE", "APPRENTICE", "JOURNEYMAN", "CRAFTSMAN",
    "ARTISAN", "ADEPT", "EXPERT", "LO MASTER", "UM MASTER",
    "ON MASTER", "EE MASTER", "HI MASTER", "SU MASTER", "ARCHMASTER"
};

const char* DM1_V1_PaletteFont_GetSkillNamePc34Compat(int level) {
    if (level < 0 || level >= DM1_SKILL_LEVEL_COUNT) return "Unknown";
    return skill_names_data[level];
}

void DM1_V1_PaletteFont_ConvertPortraitPlanarPc34Compat(uint8_t* buf) {
    if (!buf) return;
    int w = DM1_PORTRAIT_W;
    int h = DM1_PORTRAIT_H;
    /* Amiga bitplanes store eight horizontal pixels in each byte.  The
     * former w*h byte stride addressed three planes beyond the supplied
     * 32x29x4 source and made this conversion depend on adjacent memory.
     * Keep the source layout explicit: four 116-byte planes become one
     * 928-byte indexed-pixel surface. */
    int bytes_per_row = (w + 7) / 8;
    int plane_size = bytes_per_row * h;
    uint8_t* out = (uint8_t*)malloc((size_t)w * (size_t)h);
    if (!out) return;
    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int byte_idx = i * bytes_per_row + j / 8;
            int bit = 7 - (j & 7);
            uint8_t pixel = 0;
            int p;
            for (p = 0; p < 4; p++) {
                pixel |= ((buf[p * plane_size + byte_idx] >> bit) & 1) << p;
            }
            out[i * w + j] = pixel;
        }
    }
    memcpy(buf, out, w * h);
    free(out);
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — PALETTE.C remaining function citations
 *
 *   PALETTE.C:395 F0508_AMIGA_B
 *   PALETTE.C:21 F1014_U
 *   PALETTE.C:24 F1015_U
 *   PALETTE.C:399 F1016_S
 *   PALETTE.C:136 F1127_T
 *   PALETTE.C:428 F1779_S
 * ══════════════════════════════════════════════════════════════════════ */
