
#ifndef NEXUS_V1_SATURN_FONT_H
#define NEXUS_V1_SATURN_FONT_H
#include <stdint.h>

/* SEGA SATURN SCR — Saturn Screen Resource font format.
 * FONT256.S2D contains 256 character glyphs for Japanese text.
 *
 * On-disk layout (asset-backed, FONT256.S2D, 25,012 bytes):
 *   offset 0x00: "SEGA SATURN SCR\0" (16-byte magic)
 *   offset 0x10: char_count (4-byte big-endian; low 16 bits = 256 for FONT256)
 *   offset 0x14: header/section-table descriptor (4-byte BE; 0x12 observed)
 *   offset 0x18: 8 bytes reserved / padding
 *   offset 0x20: section table — 32 entries of 8 bytes (offset BE u32, size BE u32)
 *   offset 0x120: section data
 * Real FONT256.S2D populates section indices 0, 2, 4, 6 with four
 * contiguous on-disk sections (24,976 bytes total), and the last
 * four entries (28..31) carry non-zero bytes that fail bounds checks
 * — the bounded parser treats those as "skipped" rather than rejecting
 * the whole table. Indices 1, 3, 5, 7..27 are reserved/zero.
 * Header: "SEGA SATURN SCR\0" (16 bytes) */

#define NEXUS_V1_FONT_SCR_MAGIC_SIZE 16
#define NEXUS_V1_FONT_SCR_MAGIC      "SEGA SATURN SCR"
#define NEXUS_V1_FONT_SCR_HEADER_SIZE 0x20u            /* 32 bytes */
#define NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET 0x20u   /* right after header */
#define NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE 8u         /* offset (u32 BE) + size (u32 BE) */
#define NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX 32u         /* 32 entries observed in FONT256.S2D */

typedef struct {
    int char_count;
    int char_width;
    int char_height;
    uint8_t *bitmap_data;
    int bitmap_size;
} Nexus_V1_Font;

/* One populated entry from the on-disk SEGA SATURN SCR section table. */
typedef struct {
    int      index;        /* entry index in the section table (0..NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX-1) */
    uint32_t file_offset;  /* absolute offset of section data inside the SCR file */
    uint32_t size;         /* size of section data in bytes */
} Nexus_V1_FontSection;

/* Aggregate result of walking the SCR section table.
 * Bounded by NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX populated entries. */
typedef struct {
    int                    char_count;            /* parsed from header[0x10..0x13] (low 16 bits) */
    uint32_t               header_descriptor;     /* parsed from header[0x14..0x17] (e.g. 0x12 for FONT256.S2D) */
    int                    section_count;         /* number of populated entries actually walked */
    Nexus_V1_FontSection   sections[NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX];
} Nexus_V1_FontSections;

int nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size);
void nexus_v1_font_free(Nexus_V1_Font *font);
const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int char_index);
int nexus_v1_font_get_glyph_pixel(const Nexus_V1_Font *font,
                                  int char_index,
                                  int x,
                                  int y);
int nexus_v1_font_expand_glyph_bitmap(const Nexus_V1_Font *font,
                                      int char_index,
                                      uint8_t *out_bitmap,
                                      int out_width,
                                      int out_height,
                                      int out_stride);
int nexus_v1_font_draw_glyph_indexed(const Nexus_V1_Font *font,
                                     uint8_t *framebuffer,
                                     int fb_width,
                                     int fb_height,
                                     int fb_stride,
                                     int dst_x,
                                     int dst_y,
                                     int char_index,
                                     uint8_t fg_index,
                                     int bg_index);

/* ── Bounded SCR section-table parser ───────────────────────────────────
 *
 * Walks the 32-entry section table at offset 0x20 of an SEGA SATURN SCR
 * payload. Each entry is 8 bytes (offset BE u32, size BE u32). Entries with
 * offset=0 AND size=0 are treated as "reserved/empty" and skipped. Entries
 * whose (offset, size) window fails bounds checks (escapes the file or
 * overlaps the section table itself) are also skipped — the parser stays
 * bounded by reporting only well-formed sections rather than rejecting the
 * whole table on a single bad entry. This is what the real FONT256.S2D
 * tail-padding (entries 28..31 with non-zero but out-of-bounds values)
 * needs to coexist with a parser that still walks the four real sections.
 *
 * Returns 0 on success and populates `out` with up to
 * NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX populated entries. Returns -1 when
 * `data`/`out` is NULL, the buffer is too small to fit the header + table,
 * or the magic does not match.
 *
 * This is the asset-backed step the existing 1bpp glyph-decode path
 * skipped; the existing `nexus_v1_font_load()` still does its flat
 * 1bpp bitmap pass over the bytes after offset 48, and remains the
 * path the runtime takes. `nexus_v1_font_load_sections()` is the
 * section-aware complement that the SCR section-table gap-list row
 * needs to claim real on-disk evidence. */
int nexus_v1_font_load_sections(const uint8_t *data, int size,
                                Nexus_V1_FontSections *out);

/* Number of populated sections (0..NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX).
 * Returns -1 if `sections` is NULL. */
int nexus_v1_font_section_count(const Nexus_V1_FontSections *sections);

/* Return a pointer to the populated section at `index` in the section
 * table, or NULL if the index is out of range or `sections` is NULL.
 * Indices refer to the parsed order (0..section_count-1), NOT the
 * original SCR table index. */
const Nexus_V1_FontSection *nexus_v1_font_get_section(
    const Nexus_V1_FontSections *sections, int index);

/* Original SCR table index of a populated section, given its parsed
 * order. Returns -1 when out of range or NULL. */
int nexus_v1_font_section_table_index(const Nexus_V1_FontSections *sections,
                                      int parsed_index);

/* Validate that `parsed_index` is a populated section whose
 * (file_offset, size) window is fully inside `[0, file_size)`. Returns 1
 * when the window is valid, 0 when the index is empty/unpopulated, and
 * -1 on bad arguments or when the window escapes the file bounds. */
int nexus_v1_font_section_in_bounds(const Nexus_V1_FontSection *section,
                                    int file_size);

#endif
