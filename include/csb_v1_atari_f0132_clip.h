#ifndef CSB_V1_ATARI_F0132_CLIP_H
#define CSB_V1_ATARI_F0132_CLIP_H

#include <stdint.h>

typedef struct CSB_V1_AtariF0132VisibleSpan {
    uint16_t destination_offset;
    uint16_t source_offset;
    uint16_t pixel_count;
} CSB_V1_AtariF0132VisibleSpan;

/* ReDMCSB BLIT.C F0132 clips a signed source X against the destination box. */
int csb_v1_atari_f0132_visible_span(int16_t source_x,
                                    uint16_t source_width,
                                    uint16_t box_width,
                                    CSB_V1_AtariF0132VisibleSpan *out_span);

#endif
