#ifndef DM1_V1_FMTOWNS_TITLE_H
#define DM1_V1_FMTOWNS_TITLE_H

#include <stddef.h>
#include <stdint.h>

#include "dm1_v1_fmtowns_startup.h"

#ifdef __cplusplus
extern "C" {
#endif

/* HMA-240 EDM.EXP DO_TITLE_ANIMATION owns a 320x200 indexed title source.
 * The source is GRAPHICS.DAT item 1, not the PC 3.4 C001/TITLE.DAT path.
 * Frames 0..17 are the executable's 18 reverse-order zoom blits; frame 18
 * adds TITLE_MASTER after the zoom has settled. */
#define DM1_FMTOWNS_TITLE_WIDTH 320u
#define DM1_FMTOWNS_TITLE_HEIGHT 200u
#define DM1_FMTOWNS_TITLE_ZOOM_FRAME_COUNT 18u
#define DM1_FMTOWNS_TITLE_FINAL_FRAME DM1_FMTOWNS_TITLE_ZOOM_FRAME_COUNT

/* Compose one native FM Towns title frame from already-decoded original
 * GRAPHICS.DAT item 1.  `frame` is 0..18.  The caller supplies a 320x200
 * destination and owns its presentation cadence and CD track 2 playback. */
int dm1_v1_fmtowns_title_compose_frame(
    const DM1_V1_FmtownsStartupReceipt *startup,
    const uint8_t *title_pixels, uint16_t title_width, uint16_t title_height,
    unsigned int frame, uint8_t *out_pixels, size_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TITLE_H */
