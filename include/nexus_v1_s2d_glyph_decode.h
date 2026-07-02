/*
 * nexus_v1_s2d_glyph_decode.h
 * ============================
 *
 * Nexus V1 S2D font section→glyph-byte-window decoder API.
 *
 * Background
 * ----------
 * The existing modules cover the SCR section-table parse
 * (`nexus_v1_font_load_sections`) and a section→glyph-range map +
 * runtime text-layout cursor (`nexus_v1_s2d_text_layout`). What is
 * still missing is a bounded decoder that takes the parsed sections
 * and tells a caller EXACTLY which bytes inside each populated
 * section correspond to a given glyph index, and lets a caller copy
 * those glyph bytes out of the SCR file directly (without going
 * through the flat 1bpp loader's "data after offset 48" stream).
 *
 * Why this matters
 * ----------------
 * The flat 1bpp loader (`nexus_v1_font_load`) treats every byte
 * after offset 48 as one glyph stream. That works for FONT256.S2D
 * because the 32-byte header + 16-byte descriptor area overlap the
 * section-table start, and section [0]'s 8208 bytes start at offset
 * 0x120 — past the section-table end. But it bypasses the SCR
 * section table entirely, so the gap-list row
 *
 *   "decode which bytes inside each populated section hold the
 *    actual glyph payload"
 *
 * stays open. This module layers a bounded byte-window decoder on
 * top of `Nexus_V1_FontSections` so a caller can answer:
 *
 *   - which populated section holds glyph N?
 *   - which byte window (offset within the file, size in bytes)
 *     inside that section holds glyph N's payload?
 *   - does that byte window match what the flat 1bpp loader reads
 *     for the same glyph index (consistency check)?
 *
 * The decode is bounded: every window is checked against the SCR
 * file size and against the section's own `(file_offset, size)`
 * pair. The decoder never claims which 1bpp bits inside a window
 * correspond to which pixel — that decode stays with
 * `nexus_v1_font_get_glyph_pixel()` / `nexus_v1_font_draw_glyph_indexed()`.
 *
 * What this module does NOT do
 * ----------------------------
 * - It does NOT decode 1bpp bit-packing. The flat 1bpp loader
 *   already owns that decode (row-major, MSB-first per byte).
 * - It does NOT support proportional advance, vertical text, or
 *   Shift-JIS double-byte characters. The decoder is bounded by
 *   `bytes_per_glyph` (default 32 for a 16x16 1bpp glyph) just
 *   like the existing text-layout cursor.
 * - It does NOT touch a real Nexus screen or render against a real
 *   DGN framebuffer. The probe renders into synthetic byte arrays
 *   that live entirely in test memory.
 *
 * Source-lock
 * -----------
 *   include/nexus_v1_saturn_font.h
 *   src/nexus/nexus_v1_saturn_font.c (load_sections, get_glyph)
 *   include/nexus_v1_s2d_text_layout.h (sibling range-map API)
 *   SEGA SATURN SCR (16-byte magic + 16-byte descriptor + 256-byte
 *     section table, populated entries at indices 0/2/4/6 in the
 *     verified 25,012-byte FONT256.S2D asset).
 */

#ifndef NEXUS_V1_S2D_GLYPH_DECODE_H
#define NEXUS_V1_S2D_GLYPH_DECODE_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_saturn_font.h"
#include "nexus_v1_s2d_text_layout.h"   /* Nexus_V1_S2D_SectionGlyphMap */

#ifdef __cplusplus
extern "C" {
#endif

/* One glyph's worth of byte-level coverage inside a populated
 * section. `file_offset` points at the first byte of the glyph's
 * payload inside the SCR file; `size_bytes` is the byte window
 * length (always `bytes_per_glyph` for an in-range glyph index).
 * `local_offset` is the same byte offset relative to the section's
 * own `file_offset`, useful for relative addressing. */
typedef struct {
    int      parsed_section_index;   /* index into Nexus_V1_FontSections.sections[] */
    int      table_index;            /* original SCR table index (0..31) */
    uint32_t file_offset;            /* absolute offset inside the SCR file */
    uint32_t local_offset;           /* offset relative to the section's file_offset */
    uint32_t size_bytes;             /* bytes covered for this glyph (== bytes_per_glyph) */
    int      char_index;             /* glyph index this window covers */
} Nexus_V1_S2D_GlyphByteWindow;

/* Aggregate result of binding parsed sections to per-glyph byte
 * windows. The same `bytes_per_glyph` constraint as the layout
 * cursor applies. `windows` is indexed by glyph index 0..char_count-1
 * so callers can directly index `windows[char_index]` without a
 * separate lookup. */
typedef struct {
    int                              header_char_count;
    int                              char_count;          /* effective coverage */
    uint32_t                         bytes_per_glyph;
    int                              window_count;        /* == char_count when full coverage */
    int                              bytes_total;         /* sum of all window sizes */
    Nexus_V1_S2D_GlyphByteWindow     windows[NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX * 64];
} Nexus_V1_S2D_GlyphByteMap;

#define NEXUS_V1_S2D_GLYPH_DECODE_MAX_WINDOWS \
    (NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX * 64)

/* Build a per-glyph byte-window map from the parsed SCR sections
 * and an existing `Nexus_V1_S2D_SectionGlyphMap`. The windows are
 * computed once and stay valid as long as the input `sections` and
 * `glyph_map` stay valid. Returns 0 on success and fills `out`.
 * Returns -1 on NULL input, invalid `bytes_per_glyph`, or when the
 * window count exceeds the bounded output. */
int nexus_v1_s2d_glyph_byte_map_build(
    const Nexus_V1_FontSections *sections,
    const Nexus_V1_S2D_SectionGlyphMap *glyph_map,
    Nexus_V1_S2D_GlyphByteMap *out);

/* Look up the byte window for `char_index`, or NULL if the index
 * falls outside the bounded coverage or `map` is NULL. The pointer
 * is borrowed and stays valid as long as the map does. */
const Nexus_V1_S2D_GlyphByteWindow *nexus_v1_s2d_glyph_byte_window_lookup(
    const Nexus_V1_S2D_GlyphByteMap *map,
    int char_index);

/* Decode one glyph's byte window straight out of `data`. The caller
 * passes a destination buffer of at least `bytes_per_glyph` bytes;
 * on success, the window is copied into `out_bytes` and the function
 * returns the number of bytes copied (== bytes_per_glyph). Returns
 * -1 on NULL/bad-size/zero-length input, out-of-coverage char_index,
 * or when the window escapes the file. */
int nexus_v1_s2d_glyph_byte_decode(
    const uint8_t *data,
    int data_size,
    const Nexus_V1_S2D_GlyphByteMap *map,
    int char_index,
    uint8_t *out_bytes,
    int out_size);

/* Sum of `size_bytes` across all populated windows. Returns 0 when
 * `map` is NULL. */
int nexus_v1_s2d_glyph_byte_map_total_bytes(
    const Nexus_V1_S2D_GlyphByteMap *map);

/* Number of populated windows whose `parsed_section_index` matches
 * `section_parsed_index`. Returns 0 when `map` is NULL or the
 * section is not in the map. */
int nexus_v1_s2d_glyph_byte_map_section_byte_count(
    const Nexus_V1_S2D_GlyphByteMap *map,
    int section_parsed_index);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_S2D_GLYPH_DECODE_H */
