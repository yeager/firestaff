#ifndef FIRESTAFF_CSB_V1_F0284_F0289_CHAMPION_HUD_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0284_F0289_CHAMPION_HUD_PC34_COMPAT_H

#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

typedef struct {
    int valid;
    int champion_index;
    int party_direction_before;
    int requested_direction;
    int direction_delta;
    int cell_query;
    int index_in_cell;
    int f0286_runtime_owner_required;
    int bar_heights[3];
    int value_y;
    int value_current;
    int value_maximum;
    char value_current_text[5];
    char value_maximum_text[5];
    const char *source_evidence;
} CSB_V1_F0284F0289ChampionHudReceiptPc34;

/* ReDMCSB CHAMPION.C F0284--F0289.  A value-only contract over an imported
 * PC34 champion and an authenticated CSBgraphics HUD package.  It never
 * changes party direction, champion cells, panel pixels, fonts, or state. */
int csb_v1_f0284_f0289_champion_hud_receipt_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    int champion_index, int requested_direction, int cell_query,
    int value_y, int value_current, int value_maximum,
    CSB_V1_F0284F0289ChampionHudReceiptPc34 *out_receipt);

#endif
