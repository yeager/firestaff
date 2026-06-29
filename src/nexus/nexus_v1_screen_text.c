#include "nexus_v1_screen_text.h"

#include <stdlib.h>
#include <string.h>

static void clear_receipt(Nexus_V1_ScreenTextReceipt *receipt,
                          Nexus_V1_ScreenTextStatus status)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
}

static Nexus_V1_ScreenTextStyle default_style(void)
{
    Nexus_V1_ScreenTextStyle style;
    memset(&style, 0, sizeof(style));
    style.x = 0;
    style.y = 0;
    style.fg_index = 1;
    style.bg_index = -1;
    style.letter_spacing_x = 1;
    style.line_height = 0;
    style.tab_stop = 0;
    style.max_chars = 0;
    style.bytes_per_glyph =
        NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
    return style;
}

uint64_t nexus_v1_screen_text_fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(0xcbf29ce484222325);
    size_t i;

    if (!data && size > 0) return hash;
    for (i = 0; i < size; ++i) {
        hash ^= data[i];
        hash *= UINT64_C(0x100000001b3);
    }
    return hash;
}

static int framebuffer_args_valid(uint8_t *framebuffer,
                                  int fb_width,
                                  int fb_height,
                                  int fb_stride)
{
    if (!framebuffer) return 0;
    if (fb_width <= 0 || fb_height <= 0) return 0;
    if (fb_stride < fb_width) return 0;
    return 1;
}

int nexus_v1_screen_text_draw_indexed(
    const Nexus_V1_Font *font,
    const Nexus_V1_FontSections *sections,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text,
    const Nexus_V1_ScreenTextStyle *style_in,
    Nexus_V1_ScreenTextReceipt *receipt)
{
    Nexus_V1_ScreenTextStyle style;
    Nexus_V1_S2D_SectionGlyphMap map;
    Nexus_V1_S2D_TextLayoutConfig cfg;
    Nexus_V1_S2D_TextLayout layout;
    int rc;
    int drawn;
    size_t hash_size;

    clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG);
    if (!font || !sections || !text ||
        !framebuffer_args_valid(framebuffer, fb_width, fb_height, fb_stride)) {
        return NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG;
    }

    style = style_in ? *style_in : default_style();
    if (style.bytes_per_glyph == 0) {
        style.bytes_per_glyph =
            NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
    }
    if (style.letter_spacing_x < 0) {
        return NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG;
    }

    rc = nexus_v1_s2d_section_glyph_map(
        sections, style.bytes_per_glyph, &map);
    if (rc != 0) {
        clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_GLYPH_MAP);
        return NEXUS_V1_SCREEN_TEXT_ERR_GLYPH_MAP;
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.fg_index = style.fg_index;
    cfg.bg_index = style.bg_index;
    cfg.letter_spacing_x = style.letter_spacing_x;
    cfg.line_height = style.line_height;
    cfg.tab_stop = style.tab_stop;
    cfg.bytes_per_glyph = style.bytes_per_glyph;

    rc = nexus_v1_s2d_text_layout_init(&layout, font, &map, &cfg);
    if (rc != 0) {
        clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_LAYOUT_INIT);
        return NEXUS_V1_SCREEN_TEXT_ERR_LAYOUT_INIT;
    }

    layout.cursor.cursor_x = style.x;
    layout.cursor.cursor_y = style.y;
    nexus_v1_s2d_text_layout_set_max_chars(&layout, style.max_chars);

    /* ReDMCSB TEXT.C F0645 lines 164-165 and DUNVIEW.C lines
     * 3695-3706 use fixed character-width accumulation for PC
     * text/inscription draws. This bridge preserves that bounded
     * fixed-advance contract for the Saturn S2D ASCII subset by
     * delegating advance and clipping to nexus_v1_s2d_text_layout. */
    drawn = nexus_v1_s2d_text_layout_draw_string(
        &layout, framebuffer, fb_width, fb_height, fb_stride, text);
    if (drawn < 0) {
        nexus_v1_s2d_text_layout_free(&layout);
        clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_DRAW);
        return NEXUS_V1_SCREEN_TEXT_ERR_DRAW;
    }

    if (receipt) {
        receipt->status = NEXUS_V1_SCREEN_TEXT_OK;
        receipt->range_count = map.range_count;
        receipt->header_char_count = map.header_char_count;
        receipt->char_count = map.char_count;
        receipt->chars_drawn = layout.cursor.chars_drawn;
        receipt->chars_skipped = layout.cursor.chars_skipped;
        receipt->chars_clipped = layout.cursor.chars_clipped;
        receipt->newline_count = layout.cursor.newline_count;
        receipt->tab_count = layout.cursor.tab_count;
        receipt->cursor_x = layout.cursor.cursor_x;
        receipt->cursor_y = layout.cursor.cursor_y;
        receipt->line_count = layout.cursor.line_count;
        receipt->writes = layout.cursor.writes;
        hash_size = (size_t)fb_stride * (size_t)fb_height;
        receipt->framebuffer_hash =
            nexus_v1_screen_text_fnv1a64(framebuffer, hash_size);
    }

    nexus_v1_s2d_text_layout_free(&layout);
    return drawn;
}

int nexus_v1_screen_text_draw_s2d_bytes(
    const uint8_t *s2d_data,
    int s2d_size,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text,
    const Nexus_V1_ScreenTextStyle *style,
    Nexus_V1_ScreenTextReceipt *receipt)
{
    Nexus_V1_Font font;
    Nexus_V1_FontSections sections;
    int rc;

    clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG);
    if (!s2d_data || s2d_size <= 0) {
        return NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG;
    }

    memset(&font, 0, sizeof(font));
    rc = nexus_v1_font_load(&font, s2d_data, s2d_size);
    if (rc <= 0) {
        clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_FONT_LOAD);
        return NEXUS_V1_SCREEN_TEXT_ERR_FONT_LOAD;
    }

    rc = nexus_v1_font_load_sections(s2d_data, s2d_size, &sections);
    if (rc != 0) {
        nexus_v1_font_free(&font);
        clear_receipt(receipt, NEXUS_V1_SCREEN_TEXT_ERR_SECTION_PARSE);
        return NEXUS_V1_SCREEN_TEXT_ERR_SECTION_PARSE;
    }

    rc = nexus_v1_screen_text_draw_indexed(
        &font, &sections, framebuffer, fb_width, fb_height, fb_stride,
        text, style, receipt);
    nexus_v1_font_free(&font);
    return rc;
}

const char *nexus_v1_screen_text_status_name(
    Nexus_V1_ScreenTextStatus status)
{
    switch (status) {
    case NEXUS_V1_SCREEN_TEXT_OK: return "OK";
    case NEXUS_V1_SCREEN_TEXT_ERR_INVALID_ARG: return "INVALID_ARG";
    case NEXUS_V1_SCREEN_TEXT_ERR_FONT_LOAD: return "FONT_LOAD";
    case NEXUS_V1_SCREEN_TEXT_ERR_SECTION_PARSE: return "SECTION_PARSE";
    case NEXUS_V1_SCREEN_TEXT_ERR_GLYPH_MAP: return "GLYPH_MAP";
    case NEXUS_V1_SCREEN_TEXT_ERR_LAYOUT_INIT: return "LAYOUT_INIT";
    case NEXUS_V1_SCREEN_TEXT_ERR_DRAW: return "DRAW";
    default: return "UNKNOWN";
    }
}
