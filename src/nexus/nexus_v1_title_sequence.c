#include "nexus_v1_title_sequence.h"

enum {
    NEXUS_V1_TITLE_MIN_BOOT_FRAMES = 30,
    NEXUS_V1_TITLE_INITIAL_REVEAL_H = 80,
    NEXUS_V1_TITLE_REVEAL_PIXELS_PER_FRAME = 4
};

int nexus_v1_title_min_boot_frames(void)
{
    return NEXUS_V1_TITLE_MIN_BOOT_FRAMES;
}

int nexus_v1_title_frame(int frame,
                         int framebuffer_height,
                         Nexus_V1_TitleFrame *out_frame)
{
    int reveal_h;

    if (!out_frame || framebuffer_height <= 0) {
        return 0;
    }
    if (frame < 0) {
        frame = 0;
    }

    reveal_h = NEXUS_V1_TITLE_INITIAL_REVEAL_H +
               frame * NEXUS_V1_TITLE_REVEAL_PIXELS_PER_FRAME;
    if (reveal_h > framebuffer_height) {
        reveal_h = framebuffer_height;
    }

    out_frame->reveal_h = reveal_h;
    out_frame->reveal_y0 = (framebuffer_height - reveal_h) / 2;
    out_frame->reveal_y1 = out_frame->reveal_y0 + reveal_h;
    out_frame->edge_color = (uint8_t)(12 + ((frame / 4) & 7));
    out_frame->boot_reveal_complete =
        frame >= NEXUS_V1_TITLE_MIN_BOOT_FRAMES;
    return 1;
}
