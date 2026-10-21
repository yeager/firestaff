#ifndef DM1_V1_IIGS_TITLE_F0437_H
#define DM1_V1_IIGS_TITLE_F0437_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { DM1_V1_IIGS_TITLE_WIDTH=320, DM1_V1_IIGS_TITLE_HEIGHT=200,
       DM1_V1_IIGS_TITLE_ZOOM_FRAMES=18 };

/* Compose one bounded F0437 zoom frame from native C001_GRAPHIC_TITLE
 * palette indices. This first port uses nearest-neighbour sampling; original
 * F0129's GS scaler is not yet reproduced, so this is not visual parity. */
int dm1_v1_iigs_title_f0437_compose(const uint8_t title[320u*200u],
    unsigned frame, uint8_t screen[320u*200u], uint16_t palette_rgb4[16]);

#ifdef __cplusplus
}
#endif
#endif
