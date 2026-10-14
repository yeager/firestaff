#ifndef DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_H
#define DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_H

#include <stddef.h>
#include <stdint.h>
#include "dm1_v1_fmtowns_graphics_dat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_WIDTH = 224,
    DM1_V1_FMTOWNS_ENTRANCE_VIEWPORT_HEIGHT = 136,
    DM1_V1_FMTOWNS_ENTRANCE_SOURCE_FLOOR = 78,
    DM1_V1_FMTOWNS_ENTRANCE_SOURCE_CEILING = 79,
    DM1_V1_FMTOWNS_ENTRANCE_SOURCE_WALL_SET_ZERO_FIRST = 86,
    DM1_V1_FMTOWNS_ENTRANCE_SOURCE_WALL_SET_ZERO_LAST = 107
};

/* Supplies one expanded IMG2 image from the authenticated DM1 FM Towns
 * GRAPHICS.DAT. source_graphic is a ReDMCSB MEDIA720 record number. */
typedef int (*DM1_V1_FmtownsEntranceGraphicProvider)(
    void *user_data, unsigned int source_graphic, const uint8_t **pixels,
    int *width, int *height);

typedef struct DM1_V1_FmtownsEntranceViewportReceipt {
    int valid;
    /* Bit 0 = floor 78, bit 1 = ceiling 79, bits 2..23 = records 86..107;
     * bit 31 reports a request outside the source-bounded F0797 renderer. */
    uint32_t requested_source_graphics;
    uint32_t loaded_source_graphics;
    uint32_t missing_source_graphics;
    /* FNV-1a over only the 224x136 viewport pixels, excluding row padding. */
    uint32_t pixel_hash_fnv1a;
    /* COORD.C G2067/G2068 origin for the authenticated Towns language. */
    int16_t aperture_x;
    int16_t aperture_y;
} DM1_V1_FmtownsEntranceViewportReceipt;

/* Render the transient C255 map used by DM1 FM Towns ENTRANCE.C F0797.
 * Output remains untouched unless every requested source record was supplied.
 * The function owns no game state and accepts only the authentic Towns
 * source-record provider; it never falls back to synthesized art. */
int dm1_v1_fmtowns_entrance_render_c255(
    uint8_t *viewport_pixels, size_t viewport_pixel_count,
    int viewport_stride, DM1_V1_FmtownsEntranceGraphicProvider provider,
    void *provider_user_data, DM1_V1_FmtownsLang language,
    DM1_V1_FmtownsEntranceViewportReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
