/*
 * nexus_v1_s2d_text_layout.c
 * ==========================
 *
 * Implementation of the runtime S2D font layout API declared in
 * include/nexus_v1_s2d_text_layout.h.
 *
 * The layout layer sits between `nexus_v1_font_draw_glyph_indexed()`
 * (single-glyph draw) and a future M11 text primitive. It owns:
 *
 *   - a section→glyph-range map computed once at init time from the
 *     parsed SCR section table (no per-glyph parser calls on the
 *     hot path);
 *   - a cursor that advances by (char_width + letter_spacing_x)
 *     per glyph, breaks on '\n', and steps on '\t';
 *   - monotonic counters the probe can hash for a deterministic
 *     screen-text receipt (chars_drawn, writes, line_count).
 *
 * The hot-path draw call still goes through
 * `nexus_v1_font_draw_glyph_indexed()`, which the existing render
 * probe already locks at 32/32 PASS. This module therefore adds the
 * "section-bound text layout" row to the gap-list without
 * re-decoding glyph bytes.
 *
 * Source-lock:
 *   include/nexus_v1_saturn_font.h
 *   src/nexus/nexus_v1_saturn_font.c (load_sections, draw_glyph_indexed)
 *   include/nexus_v1_s2d_text_layout.h (this module's API)
 *
 * Non-claim:
 *   The map only walks populated sections in order, so the per-section
 *   char ranges form a contiguous partition from char 0 up to
 *   `sum(char_count)` (capped at the SCR header char_count). This is
 *   enough to route a glyph index to a specific section without
 *   claiming which bytes inside that section hold the actual glyph
 *   payload — that decode stays with the existing flat 1bpp loader.
 */

#include "nexus_v1_s2d_text_layout.h"

#include <string.h>

/* ── Section → glyph range map ─────────────────────────────────────── */

int nexus_v1_s2d_section_glyph_map(
    const Nexus_V1_FontSections *sections,
    uint32_t bytes_per_glyph,
    Nexus_V1_S2D_SectionGlyphMap *out)
{
    int section_count;
    int next_char = 0;
    int header_char_count;
    int i;

    if (!sections || !out) return -1;
    if (bytes_per_glyph == 0) return -1;

    memset(out, 0, sizeof(*out));
    out->bytes_per_glyph = bytes_per_glyph;
    out->header_char_count = sections->char_count;
    out->char_count = 0;
    header_char_count = sections->char_count;

    section_count = sections->section_count;
    if (section_count < 0) section_count = 0;
    if (section_count > (int)NEXUS_V1_S2D_TEXT_LAYOUT_MAX_RANGES) {
        return -1;
    }

    for (i = 0; i < section_count; ++i) {
        const Nexus_V1_FontSection *sec = &sections->sections[i];
        uint32_t section_size = sec->size;
        uint32_t glyph_budget;
        int remaining_chars;
        int covered;

        if (section_size == 0) continue;
        if (bytes_per_glyph == 0) continue;

        glyph_budget = section_size / bytes_per_glyph;

        /* The SCR header's char_count is the cap. Anything past the
         * cap is treated as trailing padding (the FONT256.S2D gate
         * confirms section [3] holds 0x01e4 bytes that the header
         * does not need). */
        remaining_chars = header_char_count - next_char;
        if (remaining_chars <= 0) {
            break;
        }

        covered = (int)glyph_budget;
        if (covered > remaining_chars) covered = remaining_chars;
        if (covered <= 0) continue;

        out->ranges[out->range_count].parsed_section_index = i;
        out->ranges[out->range_count].table_index = sec->index;
        out->ranges[out->range_count].file_offset = sec->file_offset;
        out->ranges[out->range_count].size_bytes = sec->size;
        out->ranges[out->range_count].char_start = next_char;
        out->ranges[out->range_count].char_count = covered;
        ++out->range_count;

        next_char += covered;
        out->char_count = next_char;
    }

    /* `out->char_count` is the running sum of populated ranges,
     * already capped per-range against `header_char_count`. We do
     * NOT clamp it down further: an empty table leaves char_count
     * at 0 (no coverage) while header_char_count still reports the
     * SCR header's nominal value. */

    return 0;
}

int nexus_v1_s2d_glyph_range_lookup(
    const Nexus_V1_S2D_SectionGlyphMap *map,
    int char_index)
{
    int i;

    if (!map) return -1;
    if (char_index < 0 || char_index >= map->char_count) return -1;

    for (i = 0; i < map->range_count; ++i) {
        int start = map->ranges[i].char_start;
        int end = start + map->ranges[i].char_count;
        if (char_index >= start && char_index < end) {
            return i;
        }
    }
    return -1;
}

int nexus_v1_s2d_glyph_map_total_chars(
    const Nexus_V1_S2D_SectionGlyphMap *map)
{
    int i;
    int total = 0;

    if (!map) return 0;

    for (i = 0; i < map->range_count; ++i) {
        total += map->ranges[i].char_count;
    }
    return total;
}

/* ── Runtime text layout ───────────────────────────────────────────── */

void nexus_v1_s2d_text_layout_reset_cursor(Nexus_V1_S2D_TextLayout *layout) {
    if (!layout) return;
    memset(&layout->cursor, 0, sizeof(layout->cursor));
}

int nexus_v1_s2d_text_layout_init(
    Nexus_V1_S2D_TextLayout *layout,
    const Nexus_V1_Font *font,
    const Nexus_V1_S2D_SectionGlyphMap *glyph_map,
    const Nexus_V1_S2D_TextLayoutConfig *config)
{
    if (!layout || !font || !glyph_map) return -1;
    if (!font->bitmap_data) return -1;
    if (glyph_map->bytes_per_glyph == 0) return -1;

    memset(layout, 0, sizeof(*layout));
    layout->font = font;
    layout->glyph_map = glyph_map;

    if (config) {
        layout->config = *config;
    } else {
        Nexus_V1_S2D_TextLayoutConfig defaults;
        memset(&defaults, 0, sizeof(defaults));
        defaults.fg_index = 1;
        defaults.bg_index = -1;
        defaults.letter_spacing_x = 1;
        defaults.line_height = 0;
        defaults.tab_stop = 0;
        defaults.bytes_per_glyph =
            NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
        layout->config = defaults;
    }

    if (layout->config.bytes_per_glyph == 0) {
        layout->config.bytes_per_glyph =
            NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH;
    }

    layout->initialized = 1;
    return 0;
}

void nexus_v1_s2d_text_layout_free(Nexus_V1_S2D_TextLayout *layout) {
    if (!layout) return;
    /* Note: do NOT use `&layout` here. On macOS, the secure memset
     * macro expands to __builtin___memset_chk which uses the
     * declared type of the destination pointer — and `&layout` has
     * type `Nexus_V1_S2D_TextLayout **` (pointer-to-pointer), so the
     * size guard would be 8 bytes (sizeof pointer), not
     * sizeof(Nexus_V1_S2D_TextLayout). Passing `layout` (already a
     * pointer to the struct) gives the right size and avoids the
     * trap. */
    memset(layout, 0, sizeof(*layout));
}

void nexus_v1_s2d_text_layout_set_max_chars(
    Nexus_V1_S2D_TextLayout *layout,
    int max_chars)
{
    if (!layout) return;
    layout->cursor.max_chars = max_chars;
}

/* Draw one glyph at the current cursor. Updates the cursor on
 * success. Returns the number of framebuffer writes from the
 * underlying draw call, or -1 when the glyph is out of coverage
 * (caller increments the cursor's chars_skipped). */
static int draw_one_glyph(
    Nexus_V1_S2D_TextLayout *layout,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    int char_index)
{
    int range_index;
    int writes;

    if (!layout || !layout->font || !layout->glyph_map) return -1;

    range_index = nexus_v1_s2d_glyph_range_lookup(
        layout->glyph_map, char_index);
    if (range_index < 0) {
        return -1;
    }

    writes = nexus_v1_font_draw_glyph_indexed(
        layout->font,
        framebuffer,
        fb_width,
        fb_height,
        fb_stride,
        layout->cursor.cursor_x,
        layout->cursor.cursor_y,
        char_index,
        layout->config.fg_index,
        layout->config.bg_index);

    if (writes > 0) {
        layout->cursor.writes += writes;
        if (writes < layout->font->char_width * layout->font->char_height) {
            layout->cursor.chars_clipped++;
        }
    }

    layout->cursor.cursor_x += layout->font->char_width +
                               layout->config.letter_spacing_x;
    return writes;
}

int nexus_v1_s2d_text_layout_draw_string(
    Nexus_V1_S2D_TextLayout *layout,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text)
{
    const char *p;
    int drawn = 0;
    int line_height;

    if (!layout || !layout->initialized || !framebuffer || !text) return -1;
    if (fb_width <= 0 || fb_height <= 0 || fb_stride < fb_width) return -1;

    line_height = layout->config.line_height;
    if (line_height <= 0) line_height = layout->font->char_height;
    if (line_height <= 0) line_height = 1;

    for (p = text; *p != '\0'; ++p) {
        unsigned char c = (unsigned char)*p;

        if (layout->cursor.max_chars > 0 &&
            drawn >= layout->cursor.max_chars) {
            break;
        }

        if (c == '\n') {
            layout->cursor.cursor_x = 0;
            layout->cursor.cursor_y += line_height;
            layout->cursor.line_count++;
            layout->cursor.newline_count++;
            continue;
        }
        if (c == '\t' && layout->config.tab_stop > 0) {
            int step;
            int target;
            layout->cursor.tab_count++;
            step = layout->font->char_width +
                   layout->config.letter_spacing_x;
            if (step <= 0) step = 1;
            target = layout->cursor.cursor_x +
                     (layout->config.tab_stop -
                      (layout->cursor.cursor_x /
                       layout->config.tab_stop)) *
                     layout->config.tab_stop;
            layout->cursor.cursor_x = target;
            continue;
        }
        if (c < 0x20 || c > 0x7E) {
            /* ASCII only; skip control bytes and any non-printable
             * byte without advancing the cursor or counting as
             * drawn. */
            continue;
        }

        /* Map printable ASCII '!' (0x21) to glyph 1 so glyph 0
         * stays reserved for "no character" semantics. That matches
         * the typical 1bpp bitmap font convention used by the
         * DM1/CSB M648 family and keeps the layout ASCII
         * round-trippable. */
        {
            int char_index = (int)(c - 0x20u);
            int writes = draw_one_glyph(layout, framebuffer,
                                        fb_width, fb_height, fb_stride,
                                        char_index);
            if (writes < 0) {
                layout->cursor.chars_skipped++;
                continue;
            }
            drawn++;
            layout->cursor.chars_drawn++;
        }
    }

    return drawn;
}
