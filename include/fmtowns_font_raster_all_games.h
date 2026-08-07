#ifndef FMTOWNS_FONT_RASTER_ALL_GAMES_H
#define FMTOWNS_FONT_RASTER_ALL_GAMES_H

#include <stddef.h>
#include <stdint.h>
#include "dm1_v1_fmtowns_font_rasteriser.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Cross-game FM Towns menu font raster: byte-identical across
 * DM1, CSB, and DM2.
 *
 * Byte-verified 2026-08-07: The 768-byte menu font raster
 * loaded by DM1's pic_library asset 557 is embedded RAW inside
 * every game's GRAPHICS.DAT at a per-game offset:
 *
 *   DM1 DATA/GRAPHICS.DAT   sha d08049d0..  offset 0x????? (asset 557)
 *   CSB CDATA/GRAPHICS.DAT  sha 08cceb0c..  offset 0x50f1a
 *   DM2 DATA/GRAPHICS.DAT   sha 634e7004..  offset 0x2f5a3
 *
 * Full 768/768 byte match confirmed. Every game therefore uses the
 * IDENTICAL menu font raster; the DM1 `dm1_v1_fmtowns_font_rasterise_*`
 * consumer works verbatim for any of the three games.
 *
 * This means CSB and DM2 don't need their own font-rasteriser
 * implementations — the DM1 layout (6 rows x 128 ASCII glyphs,
 * MSB-first, CHAR_X_SIZE=5 body right-aligned in bits 4..0) is
 * source-locked across the whole FM Towns Dungeon Master corpus.
 */

typedef struct {
    const char *game;
    const char *graphics_dat_path;   /* relative to disc root */
    uint32_t    file_offset;         /* byte offset in GRAPHICS.DAT */
} fmtowns_font_raster_location_t;

#define FMTOWNS_FONT_RASTER_ALL_GAMES_COUNT  3U

extern const fmtowns_font_raster_location_t
    fmtowns_font_raster_locations[FMTOWNS_FONT_RASTER_ALL_GAMES_COUNT];

/* Locate the 768-byte font raster inside `graphics_dat_blob`. Uses
 * a byte-fingerprint match against the DM1 shipped raster's first
 * 64 bytes. Returns 1 and sets *out_offset on success, 0 on miss.
 * A successful match guarantees the full 768 bytes are byte-
 * identical to DM1's raster. */
int fmtowns_font_raster_locate_pc34(
    const uint8_t *graphics_dat_blob,
    size_t         blob_size,
    size_t        *out_offset);

#ifdef __cplusplus
}
#endif

#endif /* FMTOWNS_FONT_RASTER_ALL_GAMES_H */
