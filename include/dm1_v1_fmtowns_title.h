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
 * EDM first displays TITLE_PRESENTS on its own, then presents the 18
 * reverse-order zoom blits at one VBlank each, and finally TITLE_MASTER.
 * The numbering deliberately follows that observable P3 order. */
#define DM1_FMTOWNS_TITLE_WIDTH 320u
#define DM1_FMTOWNS_TITLE_HEIGHT 200u
#define DM1_FMTOWNS_TITLE_PRESENTS_FRAME 0u
#define DM1_FMTOWNS_TITLE_ZOOM_FIRST_FRAME 1u
#define DM1_FMTOWNS_TITLE_ZOOM_FRAME_COUNT 18u
#define DM1_FMTOWNS_TITLE_FINAL_FRAME \
    (DM1_FMTOWNS_TITLE_ZOOM_FIRST_FRAME + DM1_FMTOWNS_TITLE_ZOOM_FRAME_COUNT)

/* Compose one native FM Towns title frame from already-decoded original
 * GRAPHICS.DAT item 1.  `frame` is 0..19.  The caller supplies a 320x200
 * destination and owns the source VBlank presentation cadence and CD track
 * 2 playback. */
int dm1_v1_fmtowns_title_compose_frame(
    const DM1_V1_FmtownsStartupReceipt *startup,
    const uint8_t *title_pixels, uint16_t title_width, uint16_t title_height,
    unsigned int frame, uint8_t *out_pixels, size_t out_capacity);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TITLE_H */
