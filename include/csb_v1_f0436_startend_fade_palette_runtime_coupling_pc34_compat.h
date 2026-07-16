#ifndef FIRESTAFF_CSB_V1_F0436_STARTEND_FADE_PALETTE_RUNTIME_COUPLING_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0436_STARTEND_FADE_PALETTE_RUNTIME_COUPLING_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat.h"
#include "csb_v1_f0439_f0441_f0442_startend_entrance_boundaries_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0436_PALETTE_ENTRY_COUNT_PC34 = 16,
    CSB_V1_F0436_AMIGA_FADE_STEP_COUNT_PC34 = 8,
    CSB_V1_F0436_GENERIC_FADE_STEP_COUNT_PC34 = 16
};

typedef enum CSB_V1_F0436_FadePaletteRoute_PC34 {
    CSB_V1_F0436_ROUTE_TITLE_PRETITLE_PC34 = 1u << 0,
    CSB_V1_F0436_ROUTE_TITLE_PRESENTS_PC34 = 1u << 1,
    CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34 = 1u << 2,
    CSB_V1_F0436_ROUTE_TITLE_STRIKES_BACK_PC34 = 1u << 3,
    CSB_V1_F0436_ROUTE_ENTRANCE_BLACK_PC34 = 1u << 4,
    CSB_V1_F0436_ROUTE_ENTRANCE_SCREEN_PC34 = 1u << 5,
    CSB_V1_F0436_ROUTE_ENTRANCE_CREDITS_PC34 = 1u << 6
} CSB_V1_F0436_FadePaletteRoute_PC34;

#define CSB_V1_F0436_ROUTE_TITLE_ANY_PC34 \
    (CSB_V1_F0436_ROUTE_TITLE_PRETITLE_PC34 | \
     CSB_V1_F0436_ROUTE_TITLE_PRESENTS_PC34 | \
     CSB_V1_F0436_ROUTE_TITLE_CHAOS_PC34 | \
     CSB_V1_F0436_ROUTE_TITLE_STRIKES_BACK_PC34)

#define CSB_V1_F0436_ROUTE_ENTRANCE_ANY_PC34 \
    (CSB_V1_F0436_ROUTE_ENTRANCE_BLACK_PC34 | \
     CSB_V1_F0436_ROUTE_ENTRANCE_SCREEN_PC34 | \
     CSB_V1_F0436_ROUTE_ENTRANCE_CREDITS_PC34)

typedef struct CSB_V1_F0436_FadePaletteFacts_PC34 {
    int valid;
    uint32_t route_mask;
    int target_palette_real_asset_bound;
    int palette_entry_count;
    int fade_step_count;
    int component_masks_source_locked;
    int vertical_blank_synchronized;
    int title_palette_route;
    int entrance_palette_route;
    int credits_palette_route;
    int no_renderer_palette_substitute;
    int no_legacy_palette_wrapper;
    int no_synthetic_palette;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 runtime_coupling;
    CSB_V1_StartEndEntranceBoundaryReceipt_PC34 entrance_boundary;
} CSB_V1_F0436_FadePaletteFacts_PC34;

typedef struct CSB_V1_F0436_FadePaletteReceipt_PC34 {
    int valid;
    uint32_t accepted_route_mask;
    int target_palette_bound;
    int palette_entry_count_source_locked;
    int fade_step_count_source_locked;
    int title_runtime_consumed;
    int entrance_boundary_consumed;
    int component_masks_source_locked;
    int vertical_blank_synchronized;
    int no_renderer_palette_substitute;
    int no_legacy_palette_wrapper;
    int no_synthetic_palette;
    const char *source_evidence;
} CSB_V1_F0436_FadePaletteReceipt_PC34;

void csb_v1_f0436_fade_palette_receipt_init_pc34(
    CSB_V1_F0436_FadePaletteReceipt_PC34 *receipt);

int F0436_STARTEND_FadeToPalette(
    const CSB_V1_F0436_FadePaletteFacts_PC34 *facts,
    CSB_V1_F0436_FadePaletteReceipt_PC34 *out_receipt);

const char *csb_v1_f0436_startend_fade_to_palette_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0436_STARTEND_FADE_PALETTE_RUNTIME_COUPLING_PC34_COMPAT_H */
