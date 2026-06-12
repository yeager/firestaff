#ifndef FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT_H

/*
 * DM1 V1 champion-panel portrait box blit dispatch gate
 *
 * This contract-only fixture pins the F0292 -> F0354 dispatch predicate
 * that controls when the inventory-champion portrait is re-blitted into
 * the 32x29 status-box portrait zone during the V1 status-box redraw:
 *
 *  - ReDMCSB CHAMDRAW.C F0292:757-760 dispatches the nine redraw-mask bits
 *    (NAME_TITLE | STATISTICS | LOAD | ICON | PANEL | STATUS_BOX | WOUNDS
 *    | VIEWPORT | ACTION_HAND) and short-circuits to the end of F0292 when
 *    none of those bits is set in the champion Attributes field.
 *  - ReDMCSB CHAMDRAW.C F0292:767-770 defers to F0355 when the champion
 *    is the inventory champion and G0297_B_DrawFloorAndCeilingRequested
 *    is true (the F0355 toggle is the "champion was clicked" precondition).
 *  - ReDMCSB CHAMDRAW.C F0292:771 enters the status-box branch only when
 *    MASK0x1000_STATUS_BOX is set in the redraw mask; the branch then
 *    resolves the C151+C151+championIndex status-box zone rectangle and
 *    fills the live champion status box with C12_COLOR_DARKEST_GRAY.
 *  - ReDMCSB CHAMDRAW.C F0292:784 forks the dead branch
 *    (L0865_ps_Champion->CurrentHealth == 0) before reaching the
 *    F0354 inventory-portrait call site.
 *  - ReDMCSB CHAMDRAW.C F0292:810-812 calls
 *    F0354_INVENTORY_DrawStatusBoxPortrait(P0615_ui_ChampionIndex) only
 *    when L0863_B_IsInventoryChampion is true; F0354 is therefore the
 *    *narrow* inventory-champion-only blit path, not a per-champion
 *    redraw. After F0354 returns, F0292 sets only
 *    MASK0x0100_STATISTICS in L0862_ui_ChampionAttributes so the rest
 *    of F0292 continues with the bar graph, food/water, eye/mouth,
 *    load, and viewport-redraw chain.
 *  - ReDMCSB CHAMDRAW.C F0292:813-814 takes the non-inventory-champion
 *    fallback path, which sets NAME_TITLE | STATISTICS | WOUNDS |
 *    ACTION_HAND for the non-inventory champion and never reaches F0354.
 *  - ReDMCSB CHAMDRAW.C F0292:1110 clears the nine redraw-mask bits
 *    (including the MASK0x0100_STATISTICS that was set at line 812)
 *    when the function returns, so the next call sees a clean mask.
 *  - ReDMCSB TIMELINE.C F0254_TIMELINE_ProcessEvent12_HideDamageReceived
 *    lines 1614-1637 owns the secondary F0354 dispatch
 *    (TIMELINE.C F0254:1614-1637): the function short-circuits at the
 *    dead-champion gate (CurrentHealth == 0), then routes the inventory
 *    champion through F0354 between F0077_MOUSE_EnableScreenUpdate_CPSE
 *    and F0078_MOUSE_DisableScreenUpdate and routes the non-inventory
 *    champion through F0292 with only MASK0x0080_NAME_TITLE set.
 *  - ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143
 *    ORs the per-call P2062_ui_ argument into every active champion's
 *    Attributes before delegating to F0292 in champion-index order
 *    (CHAMDRAW.C F0293:1117-1143); the dispatch order is therefore
 *    0..G0305-1 (the active party champion count), and the inventory
 *    champion's F0354 call lands on the same tick as the non-inventory
 *    champions' F0292 fallback.
 *
 * Contract only: this helper is a synthetic, no-asset fixture for the
 * F0292 -> F0354 dispatch predicate, the post-call Attributes mask, and
 * the F0254 / F0293 call-site parity. It does not render bitmaps, does
 * not load GRAPHICS.DAT / DUNGEON.DAT, and does not claim real-asset
 * pixel parity.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_V1_CPBBG_REDRAW_MASK_COUNT_PC34 9
#define DM1_V1_CPBBG_CHAMPION_COUNT_PC34 4

/* ReDMCSB CHAMDRAW.C F0292:757 redraw-mask bits. */
#define DM1_V1_CPBBG_MASK_NAME_TITLE_PC34     0x0080u
#define DM1_V1_CPBBG_MASK_STATISTICS_PC34     0x0100u
#define DM1_V1_CPBBG_MASK_LOAD_PC34           0x0200u
#define DM1_V1_CPBBG_MASK_ICON_PC34           0x0400u
#define DM1_V1_CPBBG_MASK_PANEL_PC34          0x0800u
#define DM1_V1_CPBBG_MASK_STATUS_BOX_PC34     0x1000u
#define DM1_V1_CPBBG_MASK_WOUNDS_PC34         0x2000u
#define DM1_V1_CPBBG_MASK_VIEWPORT_PC34       0x4000u
#define DM1_V1_CPBBG_MASK_ACTION_HAND_PC34    0x8000u

/* ReDMCSB CHAMDRAW.C F0292:812 post-F0354 continuation mask. */
#define DM1_V1_CPBBG_POST_F0354_MASK_PC34     DM1_V1_CPBBG_MASK_STATISTICS_PC34

/* ReDMCSB CHAMDRAW.C F0292:813-814 non-inventory fallback mask. */
#define DM1_V1_CPBBG_NON_INVENTORY_MASK_PC34 \
    (DM1_V1_CPBBG_MASK_NAME_TITLE_PC34 | \
     DM1_V1_CPBBG_MASK_STATISTICS_PC34 | \
     DM1_V1_CPBBG_MASK_WOUNDS_PC34 | \
     DM1_V1_CPBBG_MASK_ACTION_HAND_PC34)

/* ReDMCSB TIMELINE.C F0254:1635 non-inventory Attributes mask. */
#define DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34 \
    DM1_V1_CPBBG_MASK_NAME_TITLE_PC34

/* ReDMCSB CHAMDRAW.C F0292:1110 post-F0292 clear mask (all 9 bits). */
#define DM1_V1_CPBBG_CLEAR_MASK_PC34 \
    (DM1_V1_CPBBG_MASK_NAME_TITLE_PC34 | \
     DM1_V1_CPBBG_MASK_STATISTICS_PC34 | \
     DM1_V1_CPBBG_MASK_LOAD_PC34 | \
     DM1_V1_CPBBG_MASK_ICON_PC34 | \
     DM1_V1_CPBBG_MASK_PANEL_PC34 | \
     DM1_V1_CPBBG_MASK_STATUS_BOX_PC34 | \
     DM1_V1_CPBBG_MASK_WOUNDS_PC34 | \
     DM1_V1_CPBBG_MASK_VIEWPORT_PC34 | \
     DM1_V1_CPBBG_MASK_ACTION_HAND_PC34)

/*
 * ReDMCSB DEFS.H:3783-3793: status-box C151+C151+championIndex zone stride
 * and the C151 base zone (C151_ZONE_CHAMPION_0_STATUS_BOX_NAME_HANDS).
 */
#define DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 151
#define DM1_V1_CPBBG_STATUS_BOX_ZONE_STRIDE_PC34 1

/* ReDMCSB DEFS.H:3793: C175_ZONE_FIRST_CHAMPION_STATUS_BOX. */
#define DM1_V1_CPBBG_PORTRAIT_ZONE_BASE_PC34 175

typedef enum {
    DM1_V1_CPBBG_DISPATCH_F0292_PC34 = 0,
    DM1_V1_CPBBG_DISPATCH_F0254_PC34 = 1,
    DM1_V1_CPBBG_DISPATCH_F0293_PC34 = 2
} DM1_V1_CPBBG_DispatchSitePc34Compat;

typedef enum {
    DM1_V1_CPBBG_PATH_NOT_REACHED_PC34 = 0,
    DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34 = 1,
    DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34 = 2,
    DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34 = 3
} DM1_V1_CPBBG_PathPc34Compat;

typedef struct {
    /* Inputs */
    uint16_t input_redraw_mask;
    bool champion_alive;
    bool is_inventory_champion;
    bool draw_floor_and_ceiling_requested;
    int champion_index;
    int inventory_champion_ordinal;
    DM1_V1_CPBBG_DispatchSitePc34Compat dispatch_site;

    /* Outputs */
    bool f0292_short_circuits;
    bool f0292_calls_f0355;
    bool f0292_enters_status_box_branch;
    bool f0292_calls_f0354;
    uint16_t post_f0292_attributes_mask;
    uint16_t post_f0292_cleared_mask;
    DM1_V1_CPBBG_PathPc34Compat path;
    int status_box_zone;
    int portrait_zone;
    int champion_index_dispatch_order; /* 0-based, only set for F0293 dispatch */
} DM1_V1_CPBBG_GateResultPc34Compat;

void DM1_V1_CPBBG_DefaultInputPc34Compat(
    DM1_V1_CPBBG_GateResultPc34Compat *out_result);

void DM1_V1_CPBBG_BuildGatePc34Compat(
    uint16_t input_redraw_mask,
    bool champion_alive,
    bool is_inventory_champion,
    bool draw_floor_and_ceiling_requested,
    int champion_index,
    int inventory_champion_ordinal,
    DM1_V1_CPBBG_DispatchSitePc34Compat dispatch_site,
    DM1_V1_CPBBG_GateResultPc34Compat *out_result);

const char *DM1_V1_CPBBG_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_BLIT_GATE_PC34_COMPAT_H */
