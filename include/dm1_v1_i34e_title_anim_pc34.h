#ifndef DM1_V1_I34E_TITLE_ANIM_PC34_H
#define DM1_V1_I34E_TITLE_ANIM_PC34_H

#include "csb_v1_fmtowns_anm.h"

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t offset;
    uint32_t frame_index;
    uint16_t width;
    uint16_t height;
    int break_allowed;
    int finished;
    int valid;
    CSB_V1_FmtownsAnmColor palette[CSB_FMTOWNS_ANM_PALETTE_SIZE];
    uint8_t pixels[CSB_FMTOWNS_ANM_FRAME_PIXELS];
} DM1_V1_I34ETitleAnimPc34;

typedef struct {
    const uint8_t *pixels;
    const CSB_V1_FmtownsAnmColor *palette;
    uint16_t width;
    uint16_t height;
    uint16_t source_delay_vblanks;
    uint32_t frame_index;
    int break_allowed;
} DM1_V1_I34ETitleAnimFramePc34;

/* ReDMCSB ANIM.C F2275 / ANIMIMG.C F8288, I34E PC34 TITLE command. */
int dm1_v1_i34e_title_anim_pc34_init(DM1_V1_I34ETitleAnimPc34 *state,
                                     const uint8_t *data, size_t size);
/* Returns 1 for a decoded frame, 0 after DO/end, and -1 for malformed data. */
int dm1_v1_i34e_title_anim_pc34_step(
    DM1_V1_I34ETitleAnimPc34 *state,
    DM1_V1_I34ETitleAnimFramePc34 *out_frame);

#endif
