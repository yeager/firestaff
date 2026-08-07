#include "dm1_v2_item_render_pc34.h"

/*
 * DM1 V2 item render binding metadata.
 *
 * Source anchors:
 * - ReDMCSB DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
 *   walks each visible cell thing list and draws open-square objects before the
 *   creature, projectile, explosion, and final Fluxcage passes.
 * - ReDMCSB OBJECT.C F0033 resolves object icons from GRAPHICS.DAT 42..48.
 *   C201_ICON_ACTION_ICON_EMPTY_HAND is graphic 48, icon-cell 9, at
 *   (144, 0, 16, 16). The local PC34 archive verifies graphic 48 as a
 *   256x32 bitmap under GRAPHICS.DAT SHA-256
 *   2c3aa836925c64c09402bafb03c645932bd03c4f003ad9a86542383b078ecf8e.
 * - DUNVIEW.C's floor-object pass selects an aspect from each live Thing.
 *   It has no one generic floor-item bitmap, so this helper deliberately
 *   returns no V2 floor binding rather than substitute a made-up asset.
 * - Firestaff V1 parity layer include/dm1_v1_viewport_floor_ceiling_items_pc34_compat.h
 *   documents the same F0115 order and alcove-object special path.
 * - V2 action/inventory hand semantics are routed through the existing
 *   dm1_v2_hud_interaction_pc34 bridge, which consumes the V1 champion/action
 *   hand route matrix rather than remapping commands.
 */

static const DM1_V2_ItemRenderBinding s_empty_hand = {
    "C201_ICON_ACTION_ICON_EMPTY_HAND",
    DM1_V2_ITEM_SURFACE_ACTION_HAND,
    0,
    0,
    0,
    48,
    201,
    144,
    0,
    16,
    16,
};

const DM1_V2_ItemRenderBinding* dm1_v2_item_render_empty_hand_binding(void) {
    return &s_empty_hand;
}

const DM1_V2_ItemRenderBinding* dm1_v2_item_render_floor_item_binding(void) {
    /* F0115 obtains a distinct G0209/G0219 aspect for every live Thing.
     * Without that Thing-to-source-graphic route, no generic V2 material is
     * source-backed. The V1 F0115 renderer remains the pixel owner. */
    return 0;
}

int dm1_v2_item_render_layer_precedes(DM1_V2_CellLayer earlier, DM1_V2_CellLayer later) {
    if (earlier < DM1_V2_CELL_LAYER_FLOOR_ITEM || earlier > DM1_V2_CELL_LAYER_FLUXCAGE) return 0;
    if (later < DM1_V2_CELL_LAYER_FLOOR_ITEM || later > DM1_V2_CELL_LAYER_FLUXCAGE) return 0;
    return earlier < later;
}

const char* dm1_v2_item_render_source_evidence(void) {
    return "ReDMCSB DUNVIEW.C F0115/l4820 object pass + G0219 sub-cell positions; "
           "ReDMCSB OBJECT.C F0033 + DATA.C G0026: C201 -> GRAPHICS.DAT 48 "
           "at (144,0,16,16); "
           "Firestaff dm1_v1_viewport_floor_ceiling_items_pc34_compat.h F0115 draw order; "
           "V2 hand routes via dm1_v2_hud_interaction_pc34.";
}
