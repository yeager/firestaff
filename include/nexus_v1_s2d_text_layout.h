/*
 * nexus_v1_s2d_text_layout.h
 * ==========================
 *
 * Nexus V1 S2D font runtime text-layout API.
 *
 * Background
 * ----------
 * The existing `nexus_v1_saturn_font` module already exposes:
 *   - the flat 1bpp glyph-loader (`nexus_v1_font_load()`),
 *   - a per-pixel and per-glyph indexed-framebuffer draw API
 *     (`nexus_v1_font_get_glyph_pixel`, `nexus_v1_font_draw_glyph_indexed`),
 *   - a bounded 32-entry SEGA SATURN SCR section-table parser
 *     (`nexus_v1_font_load_sections()` + helpers).
 *
 * What that module still does NOT expose is a layout API on top of
 * the parsed sections: callers have to manage a cursor, advance the
 * x/y position by (char_width + letter_spacing) per glyph, handle
 * line breaks, and decide which bytes inside a populated section
 * actually hold glyphs. Without that, the S2D gap-list row
 * "bind the section parser to runtime text layout" stays open.
 *
 * Shift-JIS lead-byte skip gate (2026-06-29)
 * ------------------------------------------
 * The S2D gap-list row explicitly calls out "support real Shift-JIS
 * double-byte characters" as the next step. The flat 1bpp loader and
 * the section→glyph-range map are still indexed by glyph ordinal, so
 * they cannot render a real SJIS pair without a separate glyph-index
 * decoder that is out of scope for the bounded layout layer. This
 * module instead offers a deterministic Shift-JIS LEAD-BYTE SKIP
 * gate: when the byte stream contains a Shift-JIS lead
 * (0x81..0x9F, 0xE0..0xFC), the layout consumes it (and a valid
 * trail byte, if present) without drawing or advancing the cursor,
 * bumps `cursor.sjis_leads_seen` + `cursor.sjis_leads_skipped`, and
 * stamps the skip into a per-reason histogram. The framebuffer hash
 * stays bit-identical across two runs that include the same
 * embedded Shift-JIS bytes — the deterministic receipt the
 * probe locks.
 *
 * This is not a Shift-JIS RENDERER. It is the gate the gap-list
 * row asks for: a deterministic, hash-stable input filter so that
 * real Shift-JIS byte streams flowing through the same
 * parser→map→layout chain do not corrupt the per-line receipt. A
 * future SJIS-glyph decoder would consume the SJIS pair *before*
 * this gate (indexing the corresponding glyph directly via the
 * SCR section/glyph-range map) and skip the byte-level trail
 * consumed here.
 *
 * What this module does
 * ---------------------
 * 1. Computes a bounded glyph-range map from the parsed SCR section
 *    table: each populated section holds a contiguous slice of the
 *    font's character table. The first populated section starts at
 *    character 0, the second picks up where the first stopped, and
 *    so on. The total covered range is the smaller of the section
 *    byte budget (sum of populated section sizes / glyph size) and
 *    the SCR header's `char_count` (low 16 bits of the u32 BE at
 *    offset 0x10).
 * 2. Exposes a minimal text-layout cursor that walks an ASCII string
 *    one glyph at a time and dispatches each glyph to the right
 *    section by character index. Letters advance the cursor,
 *    '\n' breaks a line, '\t' advances to the next tab stop.
 * 3. The draw API is bounded: it clips against a framebuffer rect
 *    and refuses to draw past `max_chars` even if the input string
 *    is longer. No SDL, no real Saturn asset, no real Nexus screen
 *    capture — it produces per-line stats (chars_drawn, glyph_bytes
 *    touched, framebuffer writes) that the probe can hash for a
 *    deterministic screen-text receipt.
 *
 * What this module does NOT do
 * -----------------------------
 * - It does NOT decode the actual glyph payload inside a populated
 *   section. The flat 1bpp loader (`nexus_v1_font_load`) already
 *   handles that, and `nexus_v1_font_draw_glyph_indexed` already
 *   draws bytes from a glyph into an indexed framebuffer. This
 *   module layers layout on top of those APIs and never re-decodes
 *   a glyph.
 * - It does NOT claim full Saturn SCR font parity. Real Shift-JIS
 *   double-byte characters are not RENDERED; they are skipped via
 *   a deterministic lead-byte gate (see "Shift-JIS lead-byte skip
 *   gate" above). Vertical text and proportional advance widths
 *   are still NOT supported; the layout API advances by a fixed
 *   width derived from `Nexus_V1_Font.char_width`.
 * - It does NOT touch a real Nexus screen or render against a real
 *   DGN framebuffer. The probe renders into a synthetic indexed
 *   framebuffer that lives entirely in test memory; the optional
 *   real-asset branch just feeds the same layout over the real
 *   FONT256.S2D when the operator has staged the file.
 *
 * Source-lock
 * -----------
 *   src/nexus/nexus_v1_saturn_font.c (load_sections, draw_glyph_indexed)
 *   include/nexus_v1_saturn_font.h   (Nexus_V1_Font, Nexus_V1_FontSections)
 *   ReDMCSB DUNVIEW.C (drawing into the DM1 viewport) — the Saturn
 *     analogue of `DrawString` for Japanese text uses the same cursor
 *     advance; we model the same fixed-width advance here so the
 *     layout path stays compatible with both PC and Saturn family.
 *
 * On-disk layout (FONT256.S2D, 25,012 bytes):
 *   The four populated sections at indices 0, 2, 4, 6 of the SCR
 *   section table hold 0x2010 + 0x3c90 + 0x0210 + 0x01e4 = 24,976
 *   bytes of payload. With a 16x16 1bpp glyph (32 bytes per glyph)
 *   that covers 24,976 / 32 = 780 glyph slots — comfortably beyond
 *   the SCR header's `char_count=256`, so the layout caps at 256
 *   and leaves the tail of section [3] as trailing padding (which
 *   matches what the existing `nexus_v1_font_load_sections` gate
 *   observes on real FONT256.S2D).
 */

#ifndef NEXUS_V1_S2D_TEXT_LAYOUT_H
#define NEXUS_V1_S2D_TEXT_LAYOUT_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_saturn_font.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Section → glyph range map ─────────────────────────────────────── */

/* Maximum number of populated SCR sections the layout layer keeps
 * a range entry for. Matches NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX so
 * the bounded layer never exceeds the parser's own output. */
#define NEXUS_V1_S2D_TEXT_LAYOUT_MAX_RANGES \
    NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX

/* Bytes per 16x16 1bpp glyph. The layout layer needs this constant
 * to convert a section byte budget into a glyph count; it matches
 * `row_stride * char_height` when char_width=16 and char_height=16.
 * Layout callers with a different glyph size must override
 * `bytes_per_glyph` in `Nexus_V1_S2D_TextLayoutConfig`. */
#define NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH 32u

/* One populated section's contribution to the character table. */
typedef struct {
    int      parsed_section_index;   /* index into Nexus_V1_FontSections.sections[] */
    int      table_index;            /* original SCR table index (0..31) */
    uint32_t file_offset;            /* mirror of Nexus_V1_FontSection.file_offset */
    uint32_t size_bytes;             /* mirror of Nexus_V1_FontSection.size */
    int      char_start;             /* first char index covered by this section */
    int      char_count;             /* number of consecutive char indices */
} Nexus_V1_S2D_GlyphRange;

/* Aggregate result of binding parsed sections to a character table. */
typedef struct {
    int                       header_char_count;   /* SCR header.char_count (the cap) */
    int                       char_count;          /* effective coverage (sum of populated range char_counts) */
    uint32_t                  bytes_per_glyph;     /* used to slice each section */
    int                       range_count;         /* number of populated ranges */
    Nexus_V1_S2D_GlyphRange   ranges[NEXUS_V1_S2D_TEXT_LAYOUT_MAX_RANGES];
} Nexus_V1_S2D_SectionGlyphMap;

/* Build a section → glyph-range map from already-parsed
 * `Nexus_V1_FontSections`. The map is bounded by the parser's
 * `section_count` and by the SCR header's `char_count`, whichever
 * is smaller; the smaller of the two is the effective coverage.
 * Returns 0 on success and fills `out`. Returns -1 on NULL input,
 * invalid `bytes_per_glyph`, or when the section count exceeds the
 * bounded output. */
int nexus_v1_s2d_section_glyph_map(
    const Nexus_V1_FontSections *sections,
    uint32_t bytes_per_glyph,
    Nexus_V1_S2D_SectionGlyphMap *out);

/* Look up which populated range covers `char_index`, or -1 if the
 * index falls outside the bounded coverage or `map` is NULL. */
int nexus_v1_s2d_glyph_range_lookup(
    const Nexus_V1_S2D_SectionGlyphMap *map,
    int char_index);

/* ── Shift-JIS lead-byte skip gate ────────────────────────────────────
 *
 * Why: real (and synthetic) FONT256.S2D byte streams may carry
 * Shift-JIS double-byte characters whose byte ranges
 * (lead 0x81..0x9F, lead 0xE0..0xFC, trail 0x40..0x7E or
 * 0x80..0xFC) overlap the printable ASCII range that the layout
 * cursor walks one byte at a time. Until a real Shift-JIS glyph
 * decoder is wired into the SCR section→glyph-range map, the
 * layout layer has to filter those bytes deterministically
 * without corrupting the framebuffer hash. The classifier below
 * is the deterministic gate; the cursor's
 * `sjis_leads_seen`/`sjis_leads_skipped` counters and the
 * `skip_reasons` histogram let the probe prove the
 * hash-stability invariant.
 *
 * Encoding reference (Shift-JIS):
 *   lead  byte ∈ { 0x81..0x9F } ∪ { 0xE0..0xFC }
 *   trail byte ∈ { 0x40..0x7E } ∪ { 0x80..0xFC }
 * Source-lock notes:
 *   - The trail range excludes 0x7F (DEL) and the byte region
 *     past 0xFC; both gaps are standard Shift-JIS overhead.
 *   - This module intentionally does NOT compute a glyph index
 *     for the (lead, trail) pair — that is a separate decoder
 *     step that the existing 1bpp loader still owns.
 */

/* Returns 1 when `c` is a Shift-JIS lead byte (lead range only;
 * the trail-range check is exposed inline below). Returns 0
 * otherwise (and unconditionally for `c == 0`). */
int nexus_v1_s2d_shift_jis_lead_p(unsigned char c);

/* Returns 1 when `c` is a valid Shift-JIS trail byte. Trailing
 * DEL (0x7F) and bytes past 0xFC are not part of Shift-JIS and
 * return 0. `c == 0` returns 0 because the layout cursor uses
 * the NUL terminator as a sentinel. */
int nexus_v1_s2d_shift_jis_trail_p(unsigned char c);

/* Sum of `char_count` across all populated ranges. Returns 0 when
 * `map` is NULL. */
int nexus_v1_s2d_glyph_map_total_chars(
    const Nexus_V1_S2D_SectionGlyphMap *map);

/* ── Runtime text layout cursor ────────────────────────────────────── */

/* Layout-time configuration. The defaults map onto a 16x16 font
 * with a 1px letter spacing and 1-line height. */
typedef struct {
    uint8_t fg_index;            /* index passed to draw_glyph_indexed */
    int     bg_index;            /* -1 = transparent (do not paint bg) */
    int     letter_spacing_x;    /* extra pixels per glyph advance (>= 0) */
    int     line_height;         /* 0 = auto (use font->char_height) */
    int     tab_stop;            /* columns per '\t'; 0 disables tabs */
    uint32_t bytes_per_glyph;    /* 0 = NEXUS_V1_S2D_TEXT_LAYOUT_DEFAULT_BYTES_PER_GLYPH */
} Nexus_V1_S2D_TextLayoutConfig;

/* Per-reason skip bucket. Used both for the legacy ASCII path
 * (NON_PRINTABLE / OUT_OF_RANGE / MAX_CHARS) and the Shift-JIS
 * gate (SHIFT_JIS_LEAD / SHIFT_JIS_PAIR). The probe reads the
 * histogram to lock the deterministic-input-receipt invariant. */
typedef enum {
    NEXUS_V1_S2D_SKIP_NONE = 0,        /* placeholder so the enum starts at 0 */
    NEXUS_V1_S2D_SKIP_NON_PRINTABLE,    /* control bytes and ASCII beyond 0x7E that are NOT a SJIS lead */
    NEXUS_V1_S2D_SKIP_OUT_OF_RANGE,     /* printable ASCII whose glyph index falls outside map coverage */
    NEXUS_V1_S2D_SKIP_SHIFT_JIS_LEAD,   /* lone SJIS lead (no valid trail) — lead byte only */
    NEXUS_V1_S2D_SKIP_SHIFT_JIS_PAIR,   /* SJIS lead + valid trail pair skipped together */
    NEXUS_V1_S2D_SKIP_MAX_CHARS,        /* max_chars cap stopped the run mid-string */
    NEXUS_V1_S2D_SKIP_REASON_COUNT
} Nexus_V1_S2D_SkipReason;

/* Live cursor state. `char_count_drawn` and `writes` are monotonic
 * counters callers can read for a deterministic per-line receipt. */
typedef struct {
    int cursor_x;
    int cursor_y;
    int line_count;
    int chars_drawn;
    int newline_count;
    int tab_count;
    int chars_skipped;           /* out-of-coverage chars dropped */
    int chars_clipped;           /* chars drawn but pixels clipped */
    long writes;                 /* total framebuffer writes */
    int max_chars;               /* hard cap; <= 0 = no cap */

    /* Shift-JIS gate counters (2026-06-29). `sjis_leads_seen` is
     * every lead byte the layout peeked; `sjis_leads_skipped`
     * counts leads that were dropped (drawn=0). A lead+trail pair
     * bumps both by 1 because only one lead byte was seen. */
    int sjis_leads_seen;
    int sjis_leads_skipped;

    /* Per-reason histogram. Index by `Nexus_V1_S2D_SkipReason`. */
    int skip_reasons[NEXUS_V1_S2D_SKIP_REASON_COUNT];
} Nexus_V1_S2D_TextCursor;

/* Owned by the caller; the layout layer keeps borrowed pointers
 * to the font + glyph map + config and never frees them. */
typedef struct {
    const Nexus_V1_Font                 *font;
    const Nexus_V1_S2D_SectionGlyphMap  *glyph_map;
    Nexus_V1_S2D_TextLayoutConfig        config;
    Nexus_V1_S2D_TextCursor              cursor;
    int                                  initialized;
} Nexus_V1_S2D_TextLayout;

/* Initialize a layout object. `font`, `glyph_map`, and `config` must
 * outlive the layout object; the layout keeps borrowed pointers.
 * Returns 0 on success and -1 on NULL/missing input or invalid
 * `bytes_per_glyph`. */
int nexus_v1_s2d_text_layout_init(
    Nexus_V1_S2D_TextLayout *layout,
    const Nexus_V1_Font *font,
    const Nexus_V1_S2D_SectionGlyphMap *glyph_map,
    const Nexus_V1_S2D_TextLayoutConfig *config);

/* Free the layout's internal scratch state. After this call the
 * layout object is reset to zero but the borrowed pointers are NOT
 * released — the caller still owns them. */
void nexus_v1_s2d_text_layout_free(Nexus_V1_S2D_TextLayout *layout);

/* Reset the cursor + counters without freeing the borrowed pointers.
 * Useful for drawing multiple lines into the same framebuffer. */
void nexus_v1_s2d_text_layout_reset_cursor(Nexus_V1_S2D_TextLayout *layout);

/* Draw a null-terminated ASCII string into `framebuffer`. The draw
 * cursor advances by (char_width + letter_spacing_x) per glyph.
 * Returns the number of glyphs actually drawn (0..chars_drawn +
 * chars_skipped). Returns -1 on NULL/missing inputs. */
int nexus_v1_s2d_text_layout_draw_string(
    Nexus_V1_S2D_TextLayout *layout,
    uint8_t *framebuffer,
    int fb_width,
    int fb_height,
    int fb_stride,
    const char *text);

/* Total SJIS leads seen since the last cursor reset (or init).
 * Returns 0 when `layout` is NULL. */
int nexus_v1_s2d_text_layout_sjis_leads_seen(
    const Nexus_V1_S2D_TextLayout *layout);

/* Total SJIS leads skipped since the last cursor reset (or init).
 * Returns 0 when `layout` is NULL. */
int nexus_v1_s2d_text_layout_sjis_leads_skipped(
    const Nexus_V1_S2D_TextLayout *layout);

/* Number of bytes the layout observed that fell into a given
 * skip reason bucket since the last cursor reset (or init).
 * Returns 0 when `layout` is NULL or `reason` is out of range. */
int nexus_v1_s2d_text_layout_skip_reason_count(
    const Nexus_V1_S2D_TextLayout *layout,
    int reason);

/* Hard cap the cursor's `max_chars` so a long string stops drawing
 * at the cap. Use 0 (or negative) to disable the cap. */
void nexus_v1_s2d_text_layout_set_max_chars(
    Nexus_V1_S2D_TextLayout *layout,
    int max_chars);

#ifdef __cplusplus
}
#endif

#endif /* NEXUS_V1_S2D_TEXT_LAYOUT_H */
