#ifndef FIRESTAFF_DM2_V1_MVE_VIDEO_H
#define FIRESTAFF_DM2_V1_MVE_VIDEO_H

#include "dm2_v1_mve_stream.h"

#include <stdint.h>

/* Private PAL8 Interplay-MVE v3 video owner for the verified PC-DOS DM2
 * INTRO/END streams.  The three 320x200 pages are RAM only: the original
 * movie bytes remain caller-owned and are never extracted or rewritten. */
#define DM2_V1_MVE_VIDEO_WIDTH 320u
#define DM2_V1_MVE_VIDEO_HEIGHT 200u
#define DM2_V1_MVE_VIDEO_PIXELS (DM2_V1_MVE_VIDEO_WIDTH * DM2_V1_MVE_VIDEO_HEIGHT)

typedef struct {
    uint8_t second_last[DM2_V1_MVE_VIDEO_PIXELS];
    uint8_t last[DM2_V1_MVE_VIDEO_PIXELS];
    uint8_t current[DM2_V1_MVE_VIDEO_PIXELS];
    uint8_t palette_rgb[256u * 3u]; /* RGB6 expanded to RGB8 on update. */
    uint32_t decoded_presentations;
    int initialized;
} DM2_V1_MveVideo;

void dm2_v1_mve_video_init(DM2_V1_MveVideo *video);

/* Decodes one 0x0f/0x11 v3 presentation delivered by the strict MVE
 * iterator. Returns 1 only for an exact, fully consumed PAL8 frame. */
int dm2_v1_mve_video_decode_presentation(
    DM2_V1_MveVideo *video, const DM2_V1_MvePresentation *presentation,
    const uint8_t *movie_bytes, size_t movie_byte_count);

const uint8_t *dm2_v1_mve_video_pixels(const DM2_V1_MveVideo *video);
const uint8_t *dm2_v1_mve_video_palette_rgb(const DM2_V1_MveVideo *video);

#endif /* FIRESTAFF_DM2_V1_MVE_VIDEO_H */
