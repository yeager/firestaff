
#include "nexus_v1_saturn_font.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static uint32_t rb32(const uint8_t *p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}

int nexus_v1_font_load(Nexus_V1_Font *font, const uint8_t *data, int size) {
    if (!font || !data || size < 48) return -1;
    memset(font, 0, sizeof(*font));

    /* Verify SEGA SATURN SCR header */
    if (memcmp(data, "SEGA SATURN SCR", 15) != 0) return -1;

    /* Parse header fields (big-endian) */
    font->char_count = rb32(data + 16) & 0xFFFF;
    if (font->char_count <= 0 || font->char_count > 512)
        font->char_count = 256;

    /* Estimate char dimensions from data size */
    int glyph_data_size = size - 48;  /* skip header */
    int glyph_size = glyph_data_size / font->char_count;
    /* Common Saturn font: 12x12, 16x16, 8x16 */
    if (glyph_size >= 32) { font->char_width = 16; font->char_height = 16; }
    else if (glyph_size >= 18) { font->char_width = 12; font->char_height = 12; }
    else { font->char_width = 8; font->char_height = glyph_size; }

    font->bitmap_data = malloc(glyph_data_size);
    if (font->bitmap_data) {
        memcpy(font->bitmap_data, data + 48, glyph_data_size);
        font->bitmap_size = glyph_data_size;
    }

    printf("Saturn font: %d chars, %dx%d, %d bytes\n",
        font->char_count, font->char_width, font->char_height, glyph_data_size);
    return font->char_count;
}

void nexus_v1_font_free(Nexus_V1_Font *font) {
    if (font) { free(font->bitmap_data); font->bitmap_data = NULL; }
}

const uint8_t *nexus_v1_font_get_glyph(const Nexus_V1_Font *font, int idx) {
    int glyph_size;
    if (!font || !font->bitmap_data || idx < 0 || idx >= font->char_count)
        return NULL;
    glyph_size = font->bitmap_size / font->char_count;
    return font->bitmap_data + idx * glyph_size;
}

static int nexus_v1_font_glyph_layout(const Nexus_V1_Font *font,
                                      int *out_glyph_size,
                                      int *out_row_stride,
                                      int *out_required_bytes) {
    int glyph_size, row_stride, required_bytes;
    if (!font || !font->bitmap_data || font->char_count <= 0 ||
        font->char_width <= 0 || font->char_height <= 0 ||
        font->bitmap_size <= 0) {
        return 0;
    }

    glyph_size = font->bitmap_size / font->char_count;
    row_stride = (font->char_width + 7) / 8;
    required_bytes = row_stride * font->char_height;
    if (glyph_size < required_bytes) {
        return 0;
    }

    if (out_glyph_size) *out_glyph_size = glyph_size;
    if (out_row_stride) *out_row_stride = row_stride;
    if (out_required_bytes) *out_required_bytes = required_bytes;
    return 1;
}

int nexus_v1_font_get_glyph_pixel(const Nexus_V1_Font *font,
                                  int char_index,
                                  int x,
                                  int y) {
    int row_stride;
    const uint8_t *glyph;
    const uint8_t *row;

    if (!nexus_v1_font_glyph_layout(font, NULL, &row_stride, NULL) ||
        x < 0 || y < 0 || x >= font->char_width || y >= font->char_height) {
        return 0;
    }

    glyph = nexus_v1_font_get_glyph(font, char_index);
    if (!glyph) {
        return 0;
    }

    row = glyph + y * row_stride;
    return (row[x / 8] & (uint8_t)(0x80u >> (x & 7))) ? 1 : 0;
}

int nexus_v1_font_expand_glyph_bitmap(const Nexus_V1_Font *font,
                                      int char_index,
                                      uint8_t *out_bitmap,
                                      int out_width,
                                      int out_height,
                                      int out_stride) {
    int x, y;

    if (!font || !out_bitmap ||
        out_width < font->char_width ||
        out_height < font->char_height ||
        out_stride < out_width ||
        !nexus_v1_font_get_glyph(font, char_index) ||
        !nexus_v1_font_glyph_layout(font, NULL, NULL, NULL)) {
        return -1;
    }

    for (y = 0; y < font->char_height; ++y) {
        for (x = 0; x < font->char_width; ++x) {
            out_bitmap[y * out_stride + x] =
                (uint8_t)nexus_v1_font_get_glyph_pixel(font, char_index, x, y);
        }
    }

    return 1;
}

int nexus_v1_font_draw_glyph_indexed(const Nexus_V1_Font *font,
                                     uint8_t *framebuffer,
                                     int fb_width,
                                     int fb_height,
                                     int fb_stride,
                                     int dst_x,
                                     int dst_y,
                                     int char_index,
                                     uint8_t fg_index,
                                     int bg_index) {
    int x, y, writes;

    if (!font || !framebuffer ||
        fb_width <= 0 || fb_height <= 0 || fb_stride < fb_width ||
        bg_index > 255 ||
        !nexus_v1_font_get_glyph(font, char_index) ||
        !nexus_v1_font_glyph_layout(font, NULL, NULL, NULL)) {
        return -1;
    }

    writes = 0;
    for (y = 0; y < font->char_height; ++y) {
        int py = dst_y + y;
        if (py < 0 || py >= fb_height) {
            continue;
        }
        for (x = 0; x < font->char_width; ++x) {
            int px = dst_x + x;
            int pixel;
            if (px < 0 || px >= fb_width) {
                continue;
            }

            pixel = nexus_v1_font_get_glyph_pixel(font, char_index, x, y);
            if (pixel) {
                framebuffer[py * fb_stride + px] = fg_index;
                ++writes;
            } else if (bg_index >= 0) {
                framebuffer[py * fb_stride + px] = (uint8_t)bg_index;
                ++writes;
            }
        }
    }

    return writes;
}

/* ── SEGA SATURN SCR section-table parser ──────────────────────────────
 *
 * Walks the 32-entry section table that immediately follows the 32-byte
 * SEGA SATURN SCR header. Each entry is 8 bytes — a 32-bit big-endian
 * file offset followed by a 32-bit big-endian size — describing one
 * section of the file. Empty entries (offset=0 AND size=0) are reserved
 * / unused and skipped. Real FONT256.S2D (25,012 bytes) populates the
 * even indices 0, 2, 4, 6 with four contiguous on-disk sections; entries
 * 28..31 carry non-zero bytes that fail bounds checks (the file's tail
 * padding — a region the flat 1bpp loader silently consumes) and the
 * parser skips them rather than rejecting the whole table.
 *
 * This is the asset-backed handoff the original 1bpp flat bitmap path
 * skipped. `nexus_v1_font_load()` still consumes the bytes after offset
 * 48 as a flat 1bpp glyph stream; `nexus_v1_font_load_sections()` is the
 * section-aware complement, and it is what the SCR section-table row in
 * docs/FIRESTAFF_GAP_LIST.md needs to claim real on-disk evidence.
 *
 * Source-lock notes:
 *   - SEGA SATURN SCR header is 32 bytes (16-byte magic + 16-byte
 *     descriptor block: char_count u32 BE, descriptor u32 BE, reserved).
 *   - Section table begins at offset 0x20 and holds 32 entries of 8 bytes
 *     each (offset u32 BE + size u32 BE) — 256 bytes total — ending at
 *     offset 0x120. Verified against local FONT256.S2D, 25,012 bytes,
 *     populated entries at indices 0/2/4/6.
 *   - The flat bitmap path treats "data after offset 48" as one glyph
 *     stream; the section-aware path exposes the four on-disk sections
 *     so future layout/runtime drawing can target the right section
 *     instead of guessing. */
int nexus_v1_font_load_sections(const uint8_t *data,
                                int size,
                                Nexus_V1_FontSections *out) {
    int populated_count = 0;
    int i;
    int section_table_end;

    if (!data || !out || size <= 0) {
        return -1;
    }

    /* Header must hold at least the magic + char_count + descriptor +
     * the full 32-entry section table (32 * 8 = 256 bytes). */
    if (size < (int)NEXUS_V1_FONT_SCR_HEADER_SIZE +
              (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX *
              (int)NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE) {
        return -1;
    }

    if (memcmp(data, NEXUS_V1_FONT_SCR_MAGIC,
               NEXUS_V1_FONT_SCR_MAGIC_SIZE) != 0) {
        return -1;
    }

    memset(out, 0, sizeof(*out));

    /* char_count lives at offset 0x10 as a 32-bit BE word; the Saturn
     * SCR loader keeps only the low 16 bits, which is the glyph count
     * (256 for FONT256.S2D). The descriptor at 0x14 is a 32-bit BE word
     * we expose verbatim (0x12 for FONT256.S2D per the verified asset). */
    out->char_count = (int)(rb32(data + 0x10u) & 0xFFFFu);
    out->header_descriptor = rb32(data + 0x14u);

    section_table_end = (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET +
                        (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX *
                        (int)NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE;

    for (i = 0; i < (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX; ++i) {
        const uint8_t *entry = data +
            (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_OFFSET +
            i * (int)NEXUS_V1_FONT_SCR_SECTION_ENTRY_SIZE;
        uint32_t offset = rb32(entry);
        uint32_t sz = rb32(entry + 4);

        /* Reserved / unused entries are encoded as offset=0 AND size=0.
         * Skip them so callers only see populated sections. */
        if (offset == 0 && sz == 0) {
            continue;
        }

        /* Bounds-check the section window against the file size. We
         * require [offset, offset+sz) ⊆ [0, size). If a real on-disk
         * file has stray non-zero bytes that fail bounds (e.g. tail
         * padding from a previous section that the flat 1bpp loader
         * silently consumes), we skip them rather than reject the
         * whole table — the parser stays bounded by skipping every
         * entry that is not a well-formed (offset, size) window. */
        if (offset >= (uint32_t)size) {
            continue;
        }
        if (sz > (uint32_t)(size - (int)offset)) {
            continue;
        }

        /* Section data must not overlap the section table itself,
         * otherwise the table bytes would self-modify as we walk. */
        if ((int)offset < section_table_end &&
            (int)offset + (int)sz > section_table_end) {
            continue;
        }

        if (populated_count >= (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX) {
            /* Already filled the bounded output; this should be
             * unreachable because we only iterate MAX entries. */
            return -1;
        }

        out->sections[populated_count].index = i;
        out->sections[populated_count].file_offset = offset;
        out->sections[populated_count].size = sz;
        ++populated_count;
    }

    out->section_count = populated_count;
    return 0;
}

int nexus_v1_font_section_count(const Nexus_V1_FontSections *sections) {
    if (!sections) return -1;
    if (sections->section_count < 0) return 0;
    if (sections->section_count >
        (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX) {
        return (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX;
    }
    return sections->section_count;
}

const Nexus_V1_FontSection *nexus_v1_font_get_section(
    const Nexus_V1_FontSections *sections, int index) {
    if (!sections) return NULL;
    if (index < 0 || index >= sections->section_count) return NULL;
    if (index >= (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX) return NULL;
    return &sections->sections[index];
}

int nexus_v1_font_section_table_index(const Nexus_V1_FontSections *sections,
                                      int parsed_index) {
    if (!sections) return -1;
    if (parsed_index < 0 || parsed_index >= sections->section_count) {
        return -1;
    }
    if (parsed_index >= (int)NEXUS_V1_FONT_SCR_SECTION_TABLE_MAX) {
        return -1;
    }
    return sections->sections[parsed_index].index;
}

int nexus_v1_font_section_in_bounds(const Nexus_V1_FontSection *section,
                                    int file_size) {
    uint64_t end;

    if (!section || file_size <= 0) return -1;

    if ((int64_t)section->size < 0) return -1;
    if ((int64_t)section->file_offset < 0) return -1;

    /* Empty entries (offset=0 AND size=0) are reserved and report
     * "unpopulated" rather than "in bounds". */
    if (section->file_offset == 0 && section->size == 0) {
        return 0;
    }

    end = (uint64_t)section->file_offset + (uint64_t)section->size;
    if (end > (uint64_t)file_size) return -1;
    return 1;
}
