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
/* Keep the final, fully legible Dungeon Master title on screen before the
 * shared Entrance presentation replaces it. */
#define DM1_FMTOWNS_TITLE_MASTER_HOLD_MS 1200u

/* Compose one native FM Towns title frame from already-decoded original
 * GRAPHICS.DAT item 1.  `frame` is 0..19.  The caller supplies a 320x200
 * destination and owns the source VBlank presentation cadence and CD track
 * 2 playback. */
int dm1_v1_fmtowns_title_compose_frame(
    const DM1_V1_FmtownsStartupReceipt *startup,
    const uint8_t *title_pixels, uint16_t title_width, uint16_t title_height,
    unsigned int frame, uint8_t *out_pixels, size_t out_capacity);

/* TITLE.C waits one 60 Hz VBlank between PRESENTS and each of the 18 zoom
 * blits, then two more before TITLE_MASTER and one after it. */
unsigned int dm1_v1_fmtowns_title_frame_wait_vblanks(unsigned int frame);

/* Return the palette active for one title frame.  FM Towns installs both
 * C13_DUNGEON and C14_MASTER before the zoom loop; only the initial PRESENTS
 * frame uses the separate C12 palette. */
int dm1_v1_fmtowns_title_palette_for_frame(
    const DM1_V1_FmtownsStartupReceipt *startup, unsigned int frame,
    uint8_t out_rgb6[16][3]);

/* Convert a cumulative 60 Hz VBlank ordinal to an integer host delay without
 * accumulating the +2% drift produced by rounding every blank to 17 ms. */
unsigned int dm1_v1_fmtowns_title_vblank_delay_ms(unsigned int ordinal);

/* ReDMCSB ENTRANCE.C F0438: FM Towns advances an opening-door step after
 * three 60 Hz VBlank intervals. */
unsigned int dm1_v1_fmtowns_door_step_delay_ms(unsigned int ordinal);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_FMTOWNS_TITLE_H */
