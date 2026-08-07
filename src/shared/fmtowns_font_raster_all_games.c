#include "fmtowns_font_raster_all_games.h"
#include <string.h>

const fmtowns_font_raster_location_t
fmtowns_font_raster_locations[FMTOWNS_FONT_RASTER_ALL_GAMES_COUNT] = {
    /* DM1's raster comes via pic_library asset 557; the raw file
     * offset varies with the DECODEGRAPHIC record layout so we
     * leave it 0 and route DM1 consumers through the loader. */
    { "DM1", "DATA/GRAPHICS.DAT",   0 },
    { "CSB", "CDATA/GRAPHICS.DAT",  0x50f1a },
    { "DM2", "DATA/GRAPHICS.DAT",   0x2f5a3 }
};

/* First 64 bytes of the DM1 shipped raster (row 0 of ASCII chars
 * 0x20..0x5f). Used as an anchor pattern to locate the same raster
 * in any GRAPHICS.DAT. */
static const uint8_t k_font_raster_prefix[64] = {
    0x00,0x1e,0x16,0x0f, 0x1e,0x0f,0x1f,0x0e, 0x19,0x1f,0x1f,0x19, 0x0c,0x11,0x19,0x0e,
    0x16,0x0e,0x1e,0x0f, 0x1f,0x19,0x19,0x11, 0x19,0x11,0x1f,0x00, 0x00,0x06,0x00,0x00,
    0x00,0x04,0x05,0x0a, 0x0f,0x09,0x04,0x02, 0x04,0x04,0x15,0x00, 0x00,0x00,0x00,0x01,
    0x0e,0x04,0x0e,0x1e, 0x12,0x1f,0x06,0x1f, 0x0e,0x0e,0x00,0x00, 0x02,0x00,0x08,0x0e
};

int fmtowns_font_raster_locate_pc34(
        const uint8_t *blob, size_t blob_size, size_t *out_offset) {
    if (!blob || !out_offset) return 0;
    if (blob_size < 64) return 0;
    for (size_t i = 0; i + 64 <= blob_size; ++i) {
        if (blob[i] == k_font_raster_prefix[0] &&
            memcmp(blob + i, k_font_raster_prefix, 64) == 0) {
            *out_offset = i;
            return 1;
        }
    }
    return 0;
}
