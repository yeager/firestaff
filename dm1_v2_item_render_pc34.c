#include <stdint.h>
#include "dm1_v2_item_render_pc34.h"

/*
 * DM1 V2 item render binding metadata.
 *
 * Source anchors:
 * - ReDMCSB DUNVIEW.C F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF
 *   walks each visible cell thing list and draws open-square objects before the
 *   creature, projectile, explosion, and final Fluxcage passes.
 * - ReDMCSB DUNVIEW.C object section around line 4820 uses per-cell object
 *   positions from G0219 coordinate sets; V2 keeps this as metadata for the
 *   modern renderer instead of changing V1 gameplay/object semantics.
 * - Firestaff V1 parity layer dm1_v1_viewport_floor_ceiling_items_pc34_compat.h
 *   documents the same F0115 order and alcove-object special path.
 * - V2 action/inventory hand semantics are routed through the existing
 *   dm1_v2_hud_interaction_pc34 bridge, which consumes the V1 champion/action
 *   hand route matrix rather than remapping commands.
 */

static const DM1_V2_ItemRenderBinding s_empty_hand = {
    "fs.v2.item.starter.empty-hand",
    DM1_V2_ITEM_SURFACE_ACTION_HAND,
    0,
    0,
    0,
};

static const DM1_V2_ItemRenderBinding s_floor_item = {
    "fs.v2.item.starter.floor-item-placeholder",
    DM1_V2_ITEM_SURFACE_FLOOR,
    DM1_V2_CELL_LAYER_FLOOR_ITEM,
    1,
    1,
};

const DM1_V2_ItemRenderBinding* dm1_v2_item_render_empty_hand_binding(void) {
    return &s_empty_hand;
}

const DM1_V2_ItemRenderBinding* dm1_v2_item_render_floor_item_binding(void) {
    return &s_floor_item;
}

int dm1_v2_item_render_layer_precedes(DM1_V2_CellLayer earlier, DM1_V2_CellLayer later) {
    if (earlier < DM1_V2_CELL_LAYER_FLOOR_ITEM || earlier > DM1_V2_CELL_LAYER_FLUXCAGE) return 0;
    if (later < DM1_V2_CELL_LAYER_FLOOR_ITEM || later > DM1_V2_CELL_LAYER_FLUXCAGE) return 0;
    return earlier < later;
}

const char* dm1_v2_item_render_source_evidence(void) {
    return "ReDMCSB DUNVIEW.C F0115/l4820 object pass + G0219 sub-cell positions; "
           "Firestaff dm1_v1_viewport_floor_ceiling_items_pc34_compat.h F0115 draw order; "
           "V2 hand routes via dm1_v2_hud_interaction_pc34.";
}

/* V2 Item Render — enhanced item sprites with glow/rarity effects */

typedef struct {
    int icon_index;
    int rarity; /* 0=common, 1=uncommon, 2=rare, 3=legendary */
    uint32_t glow_color;
    float glow_intensity;
    int stack_count;
} V2_ItemRenderInfo;

uint32_t v2_item_rarity_color(int rarity) {
    switch (rarity) {
        case 0: return 0xFFFFFFFF; /* white */
        case 1: return 0xFF00FF00; /* green */
        case 2: return 0xFF0088FF; /* blue */
        case 3: return 0xFFFF8800; /* orange */
        default: return 0xFFFFFFFF;
    }
}

float v2_item_glow_pulse(uint32_t tick, int rarity) {
    if (rarity < 2) return 0.0f;
    float t = (float)(tick % 60) / 60.0f;
    /* Sinusoidal pulse for rare+ items */
    float s = 0.5f + 0.5f * (float)((t * 6.283185f < 3.14159f) ?
        (t * 6.283185f / 3.14159f) : (2.0f - t * 6.283185f / 3.14159f));
    return s * (rarity == 3 ? 0.8f : 0.4f);
}

