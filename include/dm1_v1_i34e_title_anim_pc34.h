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
    uint16_t source_counter_units;
    uint32_t frame_index;
    int break_allowed;
} DM1_V1_I34ETitleAnimFramePc34;

typedef struct {
    uint16_t lfsr;
    uint32_t lfsr_steps;
    uint32_t pixels_cleared;
    int finished;
    int valid;
} DM1_V1_I34ETitleErasePc34;

/* ReDMCSB ANIM.C main:2043-2060 / ANIMIMG.C F8288, I34E TITLE. */
int dm1_v1_i34e_title_anim_pc34_init(DM1_V1_I34ETitleAnimPc34 *state,
                                     const uint8_t *data, size_t size);
/* Returns 1 for a decoded frame, 0 after DO/end, and -1 for malformed data. */
int dm1_v1_i34e_title_anim_pc34_step(
    DM1_V1_I34ETitleAnimPc34 *state,
    DM1_V1_I34ETitleAnimFramePc34 *out_frame);

/* Default IBMIO callback scheduler: PIT input 1193182 Hz, divisor 65536,
 * decrement 3. epoch_ms/frame_started_ms are the same active (focus-paused)
 * clock. Reset the counter before decode, then compare this deadline after
 * decode/presentation. Returns UINT64_MAX for invalid/overflowing inputs.
 * The host epoch selects a phase; it does not recover historical BIOS phase.
 * Debug driver -D polling mode is not modeled by this default-only helper. */
uint64_t dm1_v1_i34e_title_pc34_deadline_ms(uint64_t epoch_ms,
    uint64_t frame_started_ms, uint16_t counter_units);

/* ReDMCSB VIDEODRV.C F8169 clears VGA pixels in this 16-bit LFSR order. */
void dm1_v1_i34e_title_erase_pc34_init(DM1_V1_I34ETitleErasePc34 *state);
/* Process at most max_steps LFSR states; returns 1 while work remains, 0 when
 * the complete 320x200 screen is black, and -1 for invalid input. */
int dm1_v1_i34e_title_erase_pc34_step(DM1_V1_I34ETitleErasePc34 *state,
                                      uint8_t *pixels, size_t pixel_count,
                                      uint32_t max_steps);

#endif
