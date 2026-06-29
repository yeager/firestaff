/*
 * nexus_v1_screen_text.h
 * ======================
 *
 * Bounded Nexus V1 screen-text bridge for FONT256.S2D.
 *
 * This module is intentionally small: it binds the already-parsed
 * SEGA SATURN SCR section table to the existing S2D text-layout cursor
 * and draws one ASCII text run into a caller-owned indexed framebuffer.
 * It is the runtime-facing wrapper above nexus_v1_s2d_text_layout, not
 * a full Saturn UI renderer.
 */

#ifndef NEXUS_V1_SCREEN_TEXT_H
#define NEXUS_V1_SCREEN_TEXT_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_s2d_text_layout.h"
#include "nexus_v1_saturn_font.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NEXUS_V1_SCREEN_TEXT_OK = 0,
    NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG = -1,
    NEXUS_V1_SCREEN_TEXT_ERR_FONT_LOAD = -2,
    NEXUS_V1_SCREEN_TEXT_ERR_SECTION_PARSE = -3,
    NEXUS_V1_SCREEN_TEXT_ERR_GLYPH_MAP = -4,
    NEXUS_V1_SCREEN_TEXT_ERR_LAYOUT_INIT = -5,
    NEXUS_V1_SCREEN_TEXT_ERR_DRAW = -6
} Nexus_V1_ScreenTextStatus;

typedef struct {
    int      x;
    int      y;
    uint8_t  fg_index;
    int      bg_index;          /* -1 = transparent */
    int      letter_spacing_x;
    int      line_height;       /* 0 = font->char_height */
    int      tab_stop;          /* 0 = disabled */
    int      max_chars;         /* <= 0 = no cap */
    uint32_t bytes_per_glyph;   /* 0 = default 16x16 1bpp glyph */
} Nexus_V1_ScreenTextStyle;

typedef struct {
    Nexus_V1_ScreenTextStatus status;
    int range_count;
    int header_char_count;
    int char_count;
    int chars_drawn;
    int chars_skipped;
    int chars_clipped;
    int newline_count;
    int tab_count;
    int cursor_x;
    int cursor_y;
    int line_count;
    long writes;
    uint64_t framebuffer_hash;
} Nexus_V1_ScreenTextReceipt;

uint64_t nexus_v1_screen_text_fnv1a64(const uint8_t *data, size_t size);

int nexus_v1_screen_text_draw_indexed(
    const Nexus_V1_Font *font,
    const Nexus_V1_FontSections *sections,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text,
    const Nexus_V1_ScreenTextStyle *style,
    Nexus_V1_ScreenTextReceipt *receipt);

int nexus_v1_screen_text_draw_s2d_bytes(
    const uint8_t *s2d_data,
    int s2d_size,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text,
    const Nexus_V1_ScreenTextStyle *style,
    Nexus_V1_ScreenTextReceipt *receipt);

const char *nexus_v1_screen_text_status_name(
    Nexus_V1_ScreenTextStatus status);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_SCREEN_TEXT_H */
