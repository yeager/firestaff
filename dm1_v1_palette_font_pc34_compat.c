#include "dm1_v1_palette_font_pc34_compat.h"
#include <string.h>
#include <stdlib.h>

static const uint16_t default_palette_data[DM1_PALETTE_SIZE] = {
    0x0000, 0x001F, 0x07E0, 0x0400, 0x0004, 0x0001, 0x0002, 0x0008,
    0x0010, 0x0020, 0x0040, 0x0080, 0x0100, 0x0200, 0x0400, 0x0800
};

void m11_pf_palette_init(M11_PF_PaletteState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_PF_PaletteState));
}

void m11_pf_set_palette(M11_PF_PaletteState* state, const uint16_t* colors, int count) {
    if (!state || !colors || count <= 0) return;
    int i;
    for (i = 0; i < count && i < DM1_PALETTE_SIZE; i++) {
        if (i < 8) {
            state->top_bottom[i].rgb12 = colors[i];
            state->update_top_bottom = true;
        } else {
            state->middle[i - 8].rgb12 = colors[i];
            state->update_middle = true;
        }
    }
}

const uint16_t* m11_pf_get_default_palette(void) {
    return default_palette_data;
}

void m11_pf_font_init(M11_PF_FontState* state) {
    if (!state) return;
    state->base_font = NULL;
    state->custom_font = NULL;
    state->text_color = 0;
    state->bg_color = 0;
}

bool m11_pf_font_alloc(M11_PF_FontState* state) {
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

void m11_pf_build_custom_colors(M11_PF_FontState* state, int16_t text, int16_t bg) {
    if (!state || !state->base_font || !state->custom_font) return;
    state->text_color = text;
    state->bg_color = bg;
    memcpy(state->custom_font, state->base_font, 768);
}

void m11_pf_draw_char(const M11_PF_FontState* state, uint8_t* fb, int x, int y, unsigned char ch, int scr_w) {
    if (!state || !fb || !state->custom_font) return;
    if ( ch >= DM1_FONT_CHARS) return;
    const uint8_t* glyph = state->custom_font + (ch * DM1_FONT_CHAR_H);
    int i, j;
    for (i = 0; i < DM1_FONT_CHAR_H; i++) {
        uint8_t row = glyph[i];
        int px = x;
        for (j = 0; j < DM1_FONT_CHAR_W; j++) {
            if (px < 0 || px >= scr_w) {
                row >>= 1;
                px++;
                continue;
            }
            int idx = (y + i) * scr_w + px;
            if (row & 0x80) {
                fb[idx] = (uint8_t)state->text_color;
            } else {
                fb[idx] = (uint8_t)state->bg_color;
            }
            row >>= 1;
            px++;
        }
    }
}

void m11_pf_draw_string(const M11_PF_FontState* state, uint8_t* fb, int x, int y, const char* str, int scr_w) {
    if (!str) return;
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            cx = x;
            y += DM1_FONT_CHAR_H;
        } else {
            m11_pf_draw_char(state, fb, cx, y, *str, scr_w);
            cx += DM1_FONT_CHAR_W;
        }
        str++;
    }
}

static const char* skill_names_data[DM1_SKILL_LEVEL_COUNT] = {
    "Novice", "Apprentice", "Journeyman", "Expert", "Master",
    "Adept", "Virtuoso", "Grandmaster", "Legend", "Mythic",
    "Divine", "Transcendent", "Eternal", "Omnipotent", "Cosmic"
};

const char* m11_pf_get_skill_name(int level) {
    if (level < 0 || level >= DM1_SKILL_LEVEL_COUNT) return "Unknown";
    return skill_names_data[level];
}

void m11_pf_convert_portrait_planar(uint8_t* buf) {
    if (!buf) return;
    int w = DM1_PORTRAIT_W;
    int h = DM1_PORTRAIT_H;
    int plane_size = w * h;
    uint8_t* out = (uint8_t*)malloc(w * h);
    if (!out) return;
    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            int idx = i * w + j;
            out[idx] = (buf[0 * plane_size + idx] & 0x0F) |
                       (buf[1 * plane_size + idx] & 0x0F) << 4;
        }
    }
    memcpy(buf, out, w * h);
    free(out);
}
