#ifndef NEXUS_V1_TITLE_SEQUENCE_H
#define NEXUS_V1_TITLE_SEQUENCE_H

#include <stdint.h>

typedef enum {
    NEXUS_V1_TITLE_PHASE_BOOT_REVEAL = 0,
    NEXUS_V1_TITLE_PHASE_HOLD = 1,
    NEXUS_V1_TITLE_PHASE_START_READY = 2
} Nexus_V1_TitlePhase;

typedef struct {
    Nexus_V1_TitlePhase phase;
    int frame_in_phase;
    int frames_until_ready;
    int reveal_y0;
    int reveal_y1;
    int reveal_h;
    uint8_t edge_color;
    int boot_reveal_complete;
    int hold_frame;
    int start_ready;
    int prompt_visible;
} Nexus_V1_TitleFrame;

int nexus_v1_title_min_boot_frames(void);
int nexus_v1_title_start_ready_frames(void);
const char *nexus_v1_title_phase_name(Nexus_V1_TitlePhase phase);
int nexus_v1_title_frame(int frame,
                         int framebuffer_height,
                         Nexus_V1_TitleFrame *out_frame);

#endif
