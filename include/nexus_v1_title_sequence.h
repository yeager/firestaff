#ifndef NEXUS_V1_TITLE_SEQUENCE_H
#define NEXUS_V1_TITLE_SEQUENCE_H

#include <stdint.h>

typedef struct {
    int reveal_y0;
    int reveal_y1;
    int reveal_h;
    uint8_t edge_color;
    int boot_reveal_complete;
} Nexus_V1_TitleFrame;

int nexus_v1_title_min_boot_frames(void);
int nexus_v1_title_frame(int frame,
                         int framebuffer_height,
                         Nexus_V1_TitleFrame *out_frame);

#endif
