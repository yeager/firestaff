/*
 * nexus_v1_s2d_glyph_decode.c
 * ===========================
 *
 * Implementation of the per-glyph byte-window decoder declared in
 * include/nexus_v1_s2d_glyph_decode.h.
 *
 * The decoder sits between the parsed SCR section table
 * (`nexus_v1_font_load_sections`) and the existing flat 1bpp
 * bitmap path (`nexus_v1_font_get_glyph_pixel`). It does NOT
 * re-decode the 1bpp bits — instead it tells a caller which bytes
 * inside each populated section correspond to a given glyph index,
 * and lets the caller copy those bytes straight out of the SCR file.
 *
 * On FONT256.S2D (25,012 bytes), section [0] holds 8208 bytes =
 * 256 glyphs at 32 bytes/glyph, so every glyph index 0..255 maps
 * into section [0] at `local_offset = glyph_index * 32`. Sections
 * [2]/[4]/[6] are trailing padding that the layout cursor does not
 * reach (their 0x3c90/0x0210/0x01e4 bytes sit past the SCR header's
 * `char_count=256` cap).
 *
 * Source-lock:
 *   include/nexus_v1_saturn_font.h
 *   src/nexus/nexus_v1_saturn_font.c (load_sections, get_glyph)
 *   include/nexus_v1_s2d_text_layout.h (SectionGlyphMap sibling)
 *   include/nexus_v1_s2d_glyph_decode.h (this module's API)
 *
 * Non-claim:
 *   The decoder does NOT assert that every populated section is
 *   fully covered by glyph bytes. A populated section that the
 *   header `char_count` cap doesn't reach is silently skipped on
 *   the per-section branch — that's the same treatment the layout
 *   cursor applies. It does NOT claim which 1bpp bit-pairs decode
 *   to which pixel either; that stays with the existing flat
 *   decoder.
 */

#include "nexus_v1_s2d_glyph_decode.h"

#include <string.h>

int nexus_v1_s2d_glyph_byte_map_build(
    const Nexus_V1_FontSections *sections,
    const Nexus_V1_S2D_SectionGlyphMap *glyph_map,
    Nexus_V1_S2D_GlyphByteMap *out)
{
    int i;
    int w;

    if (!sections || !glyph_map || !out) return -1;
    if (glyph_map->bytes_per_glyph == 0) return -1;

    memset(out, 0, sizeof(*out));
    out->bytes_per_glyph = glyph_map->bytes_per_glyph;
    out->header_char_count = glyph_map->header_char_count;
    out->char_count = glyph_map->char_count;

    if (out->char_count <= 0) {
        out->window_count = 0;
        return 0;
    }

    if (out->char_count > (int)NEXUS_V1_S2D_GLYPH_DECODE_MAX_WINDOWS) {
        return -1;
    }

    w = 0;
    for (i = 0; i < glyph_map->range_count; ++i) {
        const Nexus_V1_S2D_GlyphRange *range = &glyph_map->ranges[i];
        int j;
        /* `range->parsed_section_index` indexes into
         * `sections->sections[]`, which holds the parsed-order
         * entries produced by nexus_v1_font_load_sections. We use
         * the parsed order (NOT the original SCR table index) so
         * the decoder stays consistent with the layout cursor. */
        const Nexus_V1_FontSection *sec =
            &sections->sections[range->parsed_section_index];

        for (j = 0; j < range->char_count; ++j) {
            int char_index = range->char_start + j;
            uint32_t local_offset = (uint32_t)j * out->bytes_per_glyph;
            uint32_t window_size = out->bytes_per_glyph;
            uint32_t file_offset = sec->file_offset + local_offset;

            if (w >= (int)NEXUS_V1_S2D_GLYPH_DECODE_MAX_WINDOWS) return -1;

            out->windows[w].parsed_section_index = range->parsed_section_index;
            out->windows[w].table_index = range->table_index;
            out->windows[w].file_offset = file_offset;
            out->windows[w].local_offset = local_offset;
            out->windows[w].size_bytes = window_size;
            out->windows[w].char_index = char_index;

            ++w;
        }
    }

    out->window_count = w;
    out->bytes_total = w * (int)out->bytes_per_glyph;
    return 0;
}

const Nexus_V1_S2D_GlyphByteWindow *nexus_v1_s2d_glyph_byte_window_lookup(
    const Nexus_V1_S2D_GlyphByteMap *map,
    int char_index)
{
    int i;

    if (!map) return NULL;
    if (char_index < 0 || char_index >= map->char_count) return NULL;

    /* Windows are stored in glyph-index order, so we can index
     * directly. We still walk explicitly to make the function
     * robust to a future change in storage order. */
    for (i = 0; i < map->window_count; ++i) {
        if (map->windows[i].char_index == char_index) {
            return &map->windows[i];
        }
    }
    return NULL;
}

int nexus_v1_s2d_glyph_byte_decode(
    const uint8_t *data,
    int data_size,
    const Nexus_V1_S2D_GlyphByteMap *map,
    int char_index,
    uint8_t *out_bytes,
    int out_size)
{
    const Nexus_V1_S2D_GlyphByteWindow *win;
    uint32_t end;

    if (!data || data_size <= 0 || !map || !out_bytes || out_size <= 0) {
        return -1;
    }
    if (char_index < 0 || char_index >= map->char_count) return -1;

    win = nexus_v1_s2d_glyph_byte_window_lookup(map, char_index);
    if (!win) return -1;

    /* Bounds: window must fit entirely inside the file. The
     * section-window bounds check is implicit here because each
     * window is a strict sub-window of its section. We still
     * check `data_size` defensively to guard against callers that
     * pass a partial buffer. */
    if (win->size_bytes == 0) return -1;
    if (win->file_offset > (uint32_t)data_size) return -1;
    end = win->file_offset + win->size_bytes;
    if (end > (uint32_t)data_size) return -1;
    if (win->size_bytes > (uint32_t)out_size) return -1;

    memcpy(out_bytes, data + win->file_offset, win->size_bytes);
    return (int)win->size_bytes;
}

int nexus_v1_s2d_glyph_byte_map_total_bytes(
    const Nexus_V1_S2D_GlyphByteMap *map)
{
    int i;
    int total = 0;

    if (!map) return 0;

    for (i = 0; i < map->window_count; ++i) {
        total += (int)map->windows[i].size_bytes;
    }
    return total;
}

int nexus_v1_s2d_glyph_byte_map_section_byte_count(
    const Nexus_V1_S2D_GlyphByteMap *map,
    int section_parsed_index)
{
    int i;
    int total = 0;

    if (!map) return 0;

    for (i = 0; i < map->window_count; ++i) {
        if (map->windows[i].parsed_section_index == section_parsed_index) {
            total += (int)map->windows[i].size_bytes;
        }
    }
    return total;
}
