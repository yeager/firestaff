
#include "nexus_v1_text.h"
#include <string.h>
#include <stdio.h>

/* Basic Shift-JIS to UTF-8 converter.
 * Handles ASCII (0x20-0x7E) and katakana (0xA1-0xDF).
 * Full JIS X 0208 conversion would require lookup tables. */
int nexus_v1_sjis_to_utf8(const uint8_t *sjis, int sjis_len,
    char *utf8_out, int utf8_max)
{
    int si = 0, ui = 0;
    if (!sjis || !utf8_out) return 0;

    while (si < sjis_len && ui < utf8_max - 3) {
        uint8_t b = sjis[si];
        if (b == 0) break;

        if (b >= 0x20 && b <= 0x7E) {
            /* ASCII */
            utf8_out[ui++] = (char)b;
            si++;
        } else if (b >= 0xA1 && b <= 0xDF) {
            /* Half-width katakana -> UTF-8 (U+FF61-U+FF9F) */
            uint16_t cp = 0xFF61 + (b - 0xA1);
            utf8_out[ui++] = (char)(0xE0 | (cp >> 12));
            utf8_out[ui++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            utf8_out[ui++] = (char)(0x80 | (cp & 0x3F));
            si++;
        } else if ((b >= 0x81 && b <= 0x9F) || (b >= 0xE0 && b <= 0xEF)) {
            /* Double-byte Shift-JIS (skip for now — needs lookup table) */
            utf8_out[ui++] = '?';
            si += 2;
        } else {
            si++;
        }
    }
    utf8_out[ui] = 0;
    return ui;
}

/* Extract printable strings from binary data */
int nexus_v1_extract_strings(const uint8_t *data, int size,
    char **out_strings, int max_strings)
{
    int count = 0, i = 0;
    if (!data || !out_strings) return 0;

    while (i < size && count < max_strings) {
        /* Find runs of printable bytes */
        int start = i;
        while (i < size && ((data[i] >= 0x20 && data[i] <= 0x7E) ||
               (data[i] >= 0xA1 && data[i] <= 0xDF) ||
               (data[i] >= 0x81 && data[i] <= 0x9F) ||
               (data[i] >= 0xE0 && data[i] <= 0xEF)))
            i++;
        if (i - start >= 4) {
            static char buf[256];
            nexus_v1_sjis_to_utf8(data + start, i - start, buf, sizeof(buf));
            out_strings[count++] = buf; /* Note: reuses buffer — copy in real use */
        }
        i++;
    }
    return count;
}

static uint64_t nexus_v1_screen_text_fnv1a64(const uint8_t *data,
                                             int size)
{
    uint64_t h = UINT64_C(0xcbf29ce484222325);
    int i;

    if (!data || size <= 0) return h;

    for (i = 0; i < size; ++i) {
        h ^= (uint64_t)data[i];
        h *= UINT64_C(0x100000001b3);
    }
    return h;
}

int nexus_v1_screen_text_init(
    Nexus_V1_ScreenTextRuntime *runtime,
    const Nexus_V1_Font *font,
    const Nexus_V1_FontSections *sections,
    const Nexus_V1_S2D_TextLayoutConfig *config)
{
    Nexus_V1_S2D_TextLayoutConfig defaults;
    const Nexus_V1_S2D_TextLayoutConfig *active_config = config;
    uint32_t bytes_per_glyph =
        NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
    int rc;

    if (!runtime || !font || !sections) return -1;

    memset(runtime, 0, sizeof(*runtime));

    if (config && config->bytes_per_glyph != 0) {
        bytes_per_glyph = config->bytes_per_glyph;
    }

    rc = nexus_v1_s2d_section_glyph_map(
        sections,
        bytes_per_glyph,
        &runtime->glyph_map);
    if (rc != 0) {
        memset(runtime, 0, sizeof(*runtime));
        return -1;
    }
    if (runtime->glyph_map.char_count <= 0) {
        memset(runtime, 0, sizeof(*runtime));
        return -1;
    }

    if (!active_config) {
        memset(&defaults, 0, sizeof(defaults));
        defaults.fg_index = 15;
        defaults.bg_index = -1;
        defaults.letter_spacing_x = 1;
        defaults.line_height = 0;
        defaults.tab_stop = 0;
        defaults.bytes_per_glyph =
            NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
        active_config = &defaults;
    }

    rc = nexus_v1_s2d_text_layout_init(
        &runtime->layout, font, &runtime->glyph_map, active_config);
    if (rc != 0) {
        memset(runtime, 0, sizeof(*runtime));
        return -1;
    }

    runtime->initialized = 1;
    return 0;
}

void nexus_v1_screen_text_reset(Nexus_V1_ScreenTextRuntime *runtime)
{
    if (!runtime || !runtime->initialized) return;
    nexus_v1_s2d_text_layout_reset_cursor(&runtime->layout);
    memset(&runtime->last_receipt, 0, sizeof(runtime->last_receipt));
}

void nexus_v1_screen_text_free(Nexus_V1_ScreenTextRuntime *runtime)
{
    if (!runtime) return;
    nexus_v1_s2d_text_layout_free(&runtime->layout);
    memset(runtime, 0, sizeof(*runtime));
}

int nexus_v1_screen_text_draw(
    Nexus_V1_ScreenTextRuntime *runtime,
    Nexus_Framebuffer *framebuffer,
    int x,
    int y,
    const char *text,
    Nexus_V1_ScreenTextReceipt *out_receipt)
{
    Nexus_V1_ScreenTextReceipt receipt;
    int drawn;

    if (!runtime || !runtime->initialized || !framebuffer || !text) {
        return -1;
    }

    nexus_v1_s2d_text_layout_reset_cursor(&runtime->layout);
    runtime->layout.cursor.cursor_x = x;
    runtime->layout.cursor.cursor_y = y;

    /* Runtime binding: draw into the actual Nexus 320x200 indexed
     * framebuffer backing store. The underlying glyph rasterizer still
     * owns clipping and per-pixel writes. */
    drawn = nexus_v1_s2d_text_layout_draw_string(
        &runtime->layout,
        framebuffer->color_buffer,
        NEXUS_FB_W,
        NEXUS_FB_H,
        NEXUS_FB_W,
        text);
    if (drawn < 0) {
        return -1;
    }

    memset(&receipt, 0, sizeof(receipt));
    receipt.glyphs_drawn = drawn;
    receipt.chars_skipped = runtime->layout.cursor.chars_skipped;
    receipt.chars_clipped = runtime->layout.cursor.chars_clipped;
    receipt.newline_count = runtime->layout.cursor.newline_count;
    receipt.tab_count = runtime->layout.cursor.tab_count;
    receipt.final_cursor_x = runtime->layout.cursor.cursor_x;
    receipt.final_cursor_y = runtime->layout.cursor.cursor_y;
    receipt.framebuffer_writes = runtime->layout.cursor.writes;
    receipt.map_range_count = runtime->glyph_map.range_count;
    receipt.map_char_count = runtime->glyph_map.char_count;
    receipt.framebuffer_hash = nexus_v1_screen_text_fnv1a64(
        framebuffer->color_buffer,
        NEXUS_FB_W * NEXUS_FB_H);

    runtime->last_receipt = receipt;
    if (out_receipt) *out_receipt = receipt;

    return drawn;
}

const Nexus_V1_ScreenTextReceipt *nexus_v1_screen_text_last_receipt(
    const Nexus_V1_ScreenTextRuntime *runtime)
{
    if (!runtime || !runtime->initialized) return NULL;
    return &runtime->last_receipt;
}
