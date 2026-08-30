#ifndef FIRESTAFF_CSB_V1_ATARI_ST_VBLANK_H
#define FIRESTAFF_CSB_V1_ATARI_ST_VBLANK_H

#include <stdint.h>

/*
 * Atari ST CSB 2.0/2.1 CHANGE7_01_FIX model.
 *
 * ReDMCSB BASE.C:E0017 and VBLANK.C show that every level-4 VBlank first
 * starts its palette-switch setup.  If another VBlank arrives while the
 * handler is active, it increments G0351 and returns.  The outer handler
 * lowers its interrupt mask to 3 and drains that counter before returning.
 *
 * This is a host-side scheduling primitive, not a generic PC VBlank wait.
 * It owns no palette bytes or generated screen data; clients supply their
 * source-backed palette-start and original-handler callbacks.
 */
typedef void (*CSB_V1_AtariStVBlankCallback)(void *context);

typedef struct {
    CSB_V1_AtariStVBlankCallback start_palette_switch;
    CSB_V1_AtariStVBlankCallback run_original_handler;
    void *context;
    uint32_t received_count;
    uint32_t palette_start_count;
    uint32_t original_handler_count;
    uint32_t maximum_concurrent_count;
    uint32_t concurrent_count;
    int handling;
    int interrupt_priority_mask;
} CSB_V1_AtariStVBlank;

void csb_v1_atari_st_vblank_init(
    CSB_V1_AtariStVBlank *state,
    CSB_V1_AtariStVBlankCallback start_palette_switch,
    CSB_V1_AtariStVBlankCallback run_original_handler,
    void *context);

/* Deliver one Atari ST level-4 VBlank. Re-entrant deliveries are retained
 * and drained by the outer call; no VBlank is silently discarded. */
int csb_v1_atari_st_vblank_deliver(CSB_V1_AtariStVBlank *state);

#endif
