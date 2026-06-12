/*
 * DM1 V1 champion-panel portrait box blit dispatch gate implementation.
 *
 * Source-lock anchors (ReDMCSB WIP 20210206):
 *  - CHAMDRAW.C F0292:757-760 — nine redraw-mask short-circuit
 *  - CHAMDRAW.C F0292:767-770 — F0355 INVENTORY_Toggle_CPSE pre-route
 *  - CHAMDRAW.C F0292:771     — MASK0x1000_STATUS_BOX branch gate
 *  - CHAMDRAW.C F0292:784     — dead champion short-circuit
 *  - CHAMDRAW.C F0292:810-812 — F0354 portrait box blit + post-call mask
 *  - CHAMDRAW.C F0292:813-814 — non-inventory champion fallback mask
 *  - CHAMDRAW.C F0292:1110    — full nine-bit clear on return
 *  - TIMELINE.C  F0254:1614-1637 — HideDamageReceived inventory/non-inventory split
 *  - CHAMDRAW.C F0293:1117-1143 — DrawAllChampionStates OR-into-Attributes
 *                                   and champion-index dispatch order
 *  - DEFS.H      3783-3793 — C151..C154 status-box zone stride + C175 portrait zone base
 *  - DEFS.H      3793      — C175_ZONE_FIRST_CHAMPION_STATUS_BOX
 *
 * No bitmap sampling, no GRAPHICS.DAT/DUNGEON.DAT load, no real-asset parity claim.
 */

#include "dm1_v1_champion_panel_portrait_box_blit_gate_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock contract only.
 *
 * CHAMDRAW.C F0292:757-760 reads L0862_ui_ChampionAttributes and short-
 * circuits the entire F0292 call when none of the nine redraw-mask bits
 * (NAME_TITLE | STATISTICS | LOAD | ICON | PANEL | STATUS_BOX | WOUNDS
 * | VIEWPORT | ACTION_HAND) is set; the dispatch is therefore "any bit
 * set -> continue" and "no bit set -> end of F0292".
 *
 * CHAMDRAW.C F0292:767-770 fires F0355_INVENTORY_Toggle_CPSE
 * (C05_CHAMPION_SPECIAL_INVENTORY) before the status-box branch whenever
 * the inventory champion is the current champion and
 * G0297_B_DrawFloorAndCeilingRequested is true; this is the "champion
 * was clicked while the floor/ceiling was dirty" precondition.
 *
 * CHAMDRAW.C F0292:771 enters the status-box branch only when
 * MASK0x1000_STATUS_BOX is set, then resolves the C151+championIndex
 * status-box zone rectangle. The status-box branch fills the live
 * status box with C12_COLOR_DARKEST_GRAY before the F0354 inventory
 * portrait blit.
 *
 * CHAMDRAW.C F0292:784 forks the dead branch when
 * L0865_ps_Champion->CurrentHealth == 0. The dead branch draws
 * C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION into the C151 zone and prints
 * the champion name + champion action icon; it never reaches F0354.
 *
 * CHAMDRAW.C F0292:810-812 is the F0354 call site: the call only fires
 * when L0863_B_IsInventoryChampion is true, and immediately after the
 * call returns F0292 sets only MASK0x0100_STATISTICS in
 * L0862_ui_ChampionAttributes so the bar graph + food/water + eye/mouth
 * + load + icon + panel + action hand + viewport redraw chain still
 * runs for the inventory champion.
 *
 * CHAMDRAW.C F0292:813-814 is the non-inventory fallback: it sets
 * NAME_TITLE | STATISTICS | WOUNDS | ACTION_HAND so the non-inventory
 * champion's status-box redraw continues with name title, statistics,
 * wounds, and the action-hand icon, but no portrait blit.
 *
 * CHAMDRAW.C F0292:1110 clears all nine redraw-mask bits at the end of
 * F0292 (the label is T0292042); the next F0292 call therefore sees a
 * clean Attributes field.
 *
 * TIMELINE.C F0254_TIMELINE_ProcessEvent12_HideDamageReceived:1614-1637
 * is the secondary F0354 dispatch: the function short-circuits at the
 * dead-champion gate (CurrentHealth == 0), routes the inventory
 * champion through F0354 between
 * F0077_MOUSE_EnableScreenUpdate_CPSE and F0078_MOUSE_DisableScreenUpdate,
 * and routes the non-inventory champion through F0292 with only
 * MASK0x0080_NAME_TITLE set in Attributes.
 *
 * CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143 ORs the
 * per-call P2062_ui_ argument into every active champion's Attributes
 * and dispatches F0292 in champion-index order
 * (L0873_ui_ChampionIndex = C00_CHAMPION_FIRST..G0305-1).
 *
 * DEFS.H:3783-3793 anchors the C151..C154 status-box zone stride and
 * DEFS.H:3793 anchors C175_ZONE_FIRST_CHAMPION_STATUS_BOX used by
 * F0354_INVENTORY_DrawStatusBoxPortrait.
 */
static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. CHAMDRAW.C F0292:757-760 short-circuits F0292 when "
    "none of the nine redraw-mask bits (NAME_TITLE | STATISTICS | LOAD | "
    "ICON | PANEL | STATUS_BOX | WOUNDS | VIEWPORT | ACTION_HAND) is set. "
    "CHAMDRAW.C F0292:767-770 routes the inventory champion through "
    "F0355_INVENTORY_Toggle_CPSE(C05_CHAMPION_SPECIAL_INVENTORY) when "
    "G0297_B_DrawFloorAndCeilingRequested is set. CHAMDRAW.C F0292:771 "
    "enters the status-box branch when MASK0x1000_STATUS_BOX is set and "
    "resolves the C151+championIndex status-box zone rectangle. CHAMDRAW.C "
    "F0292:784 forks the dead branch when CurrentHealth == 0. CHAMDRAW.C "
    "F0292:810-812 calls F0354_INVENTORY_DrawStatusBoxPortrait only when "
    "L0863_B_IsInventoryChampion is true and sets only MASK0x0100_STATISTICS "
    "in L0862_ui_ChampionAttributes so the bar graph + food/water + "
    "eye/mouth + load + icon + panel + action hand + viewport redraw "
    "chain continues. CHAMDRAW.C F0292:813-814 takes the non-inventory "
    "fallback with NAME_TITLE | STATISTICS | WOUNDS | ACTION_HAND. "
    "CHAMDRAW.C F0292:1110 (T0292042) clears all nine redraw-mask bits "
    "at function exit. TIMELINE.C F0254:1614-1637 routes the inventory "
    "champion through F0354 between F0077/F0078 and the non-inventory "
    "champion through F0292 with only MASK0x0080_NAME_TITLE. CHAMDRAW.C "
    "F0293:1117-1143 OR-s the per-call P2062_ui_ argument into every "
    "active champion's Attributes and dispatches F0292 in champion-index "
    "order from C00_CHAMPION_FIRST to G0305-1. DEFS.H:3783-3793 anchors "
    "the C151 status-box zone stride; DEFS.H:3793 anchors "
    "C175_ZONE_FIRST_CHAMPION_STATUS_BOX used by F0354.";

void DM1_V1_CPBBG_DefaultInputPc34Compat(
    DM1_V1_CPBBG_GateResultPc34Compat *out_result)
{
    if (!out_result) {
        return;
    }
    memset(out_result, 0, sizeof(*out_result));
    out_result->input_redraw_mask = DM1_V1_CPBBG_MASK_STATUS_BOX_PC34;
    out_result->champion_alive = true;
    out_result->is_inventory_champion = true;
    out_result->draw_floor_and_ceiling_requested = false;
    out_result->champion_index = 0;
    out_result->inventory_champion_ordinal = 1;
    out_result->dispatch_site = DM1_V1_CPBBG_DISPATCH_F0292_PC34;
    out_result->f0292_short_circuits = false;
    out_result->f0292_calls_f0355 = false;
    out_result->f0292_enters_status_box_branch = true;
    out_result->f0292_calls_f0354 = true;
    out_result->post_f0292_attributes_mask = DM1_V1_CPBBG_POST_F0354_MASK_PC34;
    out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;
    out_result->path = DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34;
    out_result->status_box_zone =
        DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + 0;
    out_result->portrait_zone = DM1_V1_CPBBG_PORTRAIT_ZONE_BASE_PC34 + 0;
    out_result->champion_index_dispatch_order = -1;
}

static bool is_valid_champion_index(int champion_index)
{
    return champion_index >= 0 &&
           champion_index < DM1_V1_CPBBG_CHAMPION_COUNT_PC34;
}

void DM1_V1_CPBBG_BuildGatePc34Compat(
    uint16_t input_redraw_mask,
    bool champion_alive,
    bool is_inventory_champion,
    bool draw_floor_and_ceiling_requested,
    int champion_index,
    int inventory_champion_ordinal,
    DM1_V1_CPBBG_DispatchSitePc34Compat dispatch_site,
    DM1_V1_CPBBG_GateResultPc34Compat *out_result)
{
    if (!out_result) {
        return;
    }

    /* Initialize with the default portrait-blit path; mutate as gates
     * short-circuit or fall through.
     */
    DM1_V1_CPBBG_DefaultInputPc34Compat(out_result);

    out_result->input_redraw_mask = input_redraw_mask;
    out_result->champion_alive = champion_alive;
    out_result->is_inventory_champion = is_inventory_champion;
    out_result->draw_floor_and_ceiling_requested =
        draw_floor_and_ceiling_requested;
    out_result->champion_index = champion_index;
    out_result->inventory_champion_ordinal = inventory_champion_ordinal;
    out_result->dispatch_site = dispatch_site;

    /*
     * ReDMCSB TIMELINE.C F0254:1614-1637 secondary F0354 dispatch.
     * F0254 has its own short-circuit tree: the dead-champion gate
     * fires first (line 1624), then the inventory-champion branch
     * routes the live champion through F0354 (line 1630) and the
     * non-inventory branch routes through F0292 with only
     * MASK0x0080_NAME_TITLE in Attributes (line 1635). F0254 does not
     * consume the F0292 redraw-mask bits, so the F0292 short-circuit
     * is not applied to the F0254 dispatch.
     */
    if (dispatch_site == DM1_V1_CPBBG_DISPATCH_F0254_PC34) {
        out_result->f0292_short_circuits = false;
        out_result->f0292_calls_f0355 = false;
        out_result->f0292_enters_status_box_branch = false;
        out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;

        if (!champion_alive) {
            /* TIMELINE.C F0254:1624 dead champion short-circuit. */
            out_result->f0292_calls_f0354 = false;
            out_result->path = DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34;
            out_result->post_f0292_attributes_mask = 0;
            if (is_valid_champion_index(champion_index)) {
                out_result->status_box_zone =
                    DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
            }
            out_result->champion_index_dispatch_order = -1;
            return;
        }

        if (!is_inventory_champion) {
            /* TIMELINE.C F0254:1635 non-inventory champion -> F0292
             * with only MASK0x0080_NAME_TITLE in Attributes.
             */
            out_result->f0292_calls_f0354 = false;
            out_result->path = DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34;
            out_result->post_f0292_attributes_mask =
                DM1_V1_CPBBG_F0254_NON_INVENTORY_MASK_PC34;
            if (is_valid_champion_index(champion_index)) {
                out_result->status_box_zone =
                    DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
            }
            out_result->champion_index_dispatch_order = -1;
            return;
        }

        /* TIMELINE.C F0254:1630 inventory champion -> F0354. */
        out_result->f0292_calls_f0354 = true;
        out_result->path = DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34;
        out_result->post_f0292_attributes_mask =
            DM1_V1_CPBBG_POST_F0354_MASK_PC34;
        if (is_valid_champion_index(champion_index)) {
            out_result->status_box_zone =
                DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
            out_result->portrait_zone =
                DM1_V1_CPBBG_PORTRAIT_ZONE_BASE_PC34 + champion_index;
        } else {
            out_result->status_box_zone = -1;
            out_result->portrait_zone = -1;
        }
        out_result->champion_index_dispatch_order = -1;
        return;
    }

    /* ReDMCSB CHAMDRAW.C F0292:757 — short-circuit when no redraw-mask bit
     * is set. Only the inventory-champion F0354 path remains "off the
     * record" when the entire mask is zero, because no F0292 body runs.
     */
    if ((input_redraw_mask & DM1_V1_CPBBG_CLEAR_MASK_PC34) == 0) {
        out_result->f0292_short_circuits = true;
        out_result->f0292_calls_f0355 = false;
        out_result->f0292_enters_status_box_branch = false;
        out_result->f0292_calls_f0354 = false;
        out_result->path = DM1_V1_CPBBG_PATH_NOT_REACHED_PC34;
        out_result->post_f0292_attributes_mask = 0;
        out_result->post_f0292_cleared_mask = 0;
        out_result->champion_index_dispatch_order = -1;
        return;
    }

    /* ReDMCSB CHAMDRAW.C F0292:767-770 — F0355 pre-route fires only for
     * the inventory champion and only when G0297 is set. The status-box
     * branch still runs after F0355 returns.
     */
    if (is_inventory_champion && draw_floor_and_ceiling_requested) {
        out_result->f0292_calls_f0355 = true;
    } else {
        out_result->f0292_calls_f0355 = false;
    }

    /* ReDMCSB CHAMDRAW.C F0292:771 — status-box branch only enters when
     * MASK0x1000_STATUS_BOX is set. Without that bit, the F0354 call
     * site at line 810-812 is unreachable, regardless of champion state.
     */
    if ((input_redraw_mask & DM1_V1_CPBBG_MASK_STATUS_BOX_PC34) == 0) {
        out_result->f0292_enters_status_box_branch = false;
        out_result->f0292_calls_f0354 = false;
        out_result->path = DM1_V1_CPBBG_PATH_NOT_REACHED_PC34;
        out_result->post_f0292_attributes_mask = 0;
        out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;
        out_result->champion_index_dispatch_order = -1;
        return;
    }

    out_result->f0292_enters_status_box_branch = true;

    /* ReDMCSB CHAMDRAW.C F0292:784 — dead champion short-circuit. The
     * dead branch draws C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION and the
     * champion name + action icon; it never reaches F0354.
     */
    if (!champion_alive) {
        out_result->f0292_calls_f0354 = false;
        out_result->path = DM1_V1_CPBBG_PATH_DEAD_STATUS_BOX_PC34;
        out_result->post_f0292_attributes_mask = 0;
        out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;
        if (is_valid_champion_index(champion_index)) {
            out_result->status_box_zone =
                DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
        }
        out_result->champion_index_dispatch_order = -1;
        return;
    }

    /* ReDMCSB CHAMDRAW.C F0292:810-812 — F0354 call only fires when
     * the champion is the inventory champion. The non-inventory
     * champion fallback at line 813-814 takes the redraw-mask branch
     * instead.
     */
    if (!is_inventory_champion) {
        out_result->f0292_calls_f0354 = false;
        out_result->path = DM1_V1_CPBBG_PATH_NON_INVENTORY_REDRAW_PC34;
        out_result->post_f0292_attributes_mask =
            DM1_V1_CPBBG_NON_INVENTORY_MASK_PC34;
        out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;
        if (is_valid_champion_index(champion_index)) {
            out_result->status_box_zone =
                DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
        }
        out_result->champion_index_dispatch_order = -1;
        return;
    }

    /* Inventory champion + alive + MASK0x1000_STATUS_BOX -> F0354
     * inventory-portrait blit, post-call MASK0x0100_STATISTICS only.
     */
    out_result->f0292_calls_f0354 = true;
    out_result->path = DM1_V1_CPBBG_PATH_F0354_PORTRAIT_BLIT_PC34;
    out_result->post_f0292_attributes_mask = DM1_V1_CPBBG_POST_F0354_MASK_PC34;
    out_result->post_f0292_cleared_mask = DM1_V1_CPBBG_CLEAR_MASK_PC34;
    if (is_valid_champion_index(champion_index)) {
        out_result->status_box_zone =
            DM1_V1_CPBBG_STATUS_BOX_ZONE_BASE_PC34 + champion_index;
        out_result->portrait_zone =
            DM1_V1_CPBBG_PORTRAIT_ZONE_BASE_PC34 + champion_index;
    } else {
        out_result->status_box_zone = -1;
        out_result->portrait_zone = -1;
    }

    /*
     * ReDMCSB CHAMDRAW.C F0293:1117-1143 champion-index dispatch order.
     * The dispatch order is the same as the F0293 loop counter
     * (C00_CHAMPION_FIRST + champion_index); the helper records the
     * order alongside the gate so callers can pin it against the
     * F0293 champion-index iteration.
     */
    if (dispatch_site == DM1_V1_CPBBG_DISPATCH_F0293_PC34 &&
        is_valid_champion_index(champion_index)) {
        out_result->champion_index_dispatch_order = champion_index;
    } else {
        out_result->champion_index_dispatch_order = -1;
    }
}

const char *DM1_V1_CPBBG_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}
