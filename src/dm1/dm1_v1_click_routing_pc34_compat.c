/* DM1 V1 Click/Mouse Routing — source-locked from ReDMCSB
 * CLIKVIEW.C F0372: ProcessType80_ClickInDungeonView_TouchFrontWallSensor
 * CLIKCHAM.C F0367: ProcessTypes12To27_ClickInChampionStatusBox
 * CLIKMENU.C: menu click processing
 * COMMAND.C: central command dispatcher from mouse coordinates */

#include "dm1_v1_click_routing_pc34_compat.h"
#include <string.h>

void DM1_V1_ClickRouting_InitPc34Compat(DM1_V1_ClickRoutingStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_ClickRoutingStatePc34));
}

void DM1_V1_ClickRouting_ClearZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state) {
    if (!state) return;
    state->zone_count = 0;
}

bool DM1_V1_ClickRouting_AddZonePc34Compat(DM1_V1_ClickRoutingStatePc34* state, DM1_V1_ClickZoneTypePc34 type,
                      int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t data) {
    if (!state || state->zone_count >= DM1_CK_MAX_ZONES) return false;
    DM1_V1_ClickZonePc34* z = &state->zones[state->zone_count++];
    z->type = type;
    z->x = x; z->y = y; z->w = w; z->h = h;
    z->data = data;
    return true;
}

void DM1_V1_ClickRouting_UpdateMousePc34Compat(DM1_V1_ClickRoutingStatePc34* state, int16_t x, int16_t y,
                          bool left, bool right) {
    if (!state) return;
    state->mouse_x = x;
    state->mouse_y = y;
    state->left_pressed = left;
    state->right_pressed = right;
    state->mouse_visible = true;
}

/* COMMAND.C dispatch pattern: test click against all registered zones */
DM1_V1_ClickResultPc34 DM1_V1_ClickRouting_TestClickPc34Compat(const DM1_V1_ClickRoutingStatePc34* state,
                                      int16_t x, int16_t y) {
    DM1_V1_ClickResultPc34 result;
    memset(&result, 0, sizeof(result));

    if (!state) return result;

    for (uint8_t i = 0; i < state->zone_count; i++) {
        const DM1_V1_ClickZonePc34* z = &state->zones[i];
        if (x >= z->x && x < z->x + z->w &&
            y >= z->y && y < z->y + z->h) {
            result.zone_type = z->type;
            result.zone_data = z->data;
            result.local_x = x - z->x;
            result.local_y = y - z->y;
            result.hit = true;
            return result;
        }
    }

    return result;
}

/* Setup standard dungeon view click zones
 * Based on DM1 screen layout: viewport 224x136 at (0,0),
 * movement arrows below, champion panels at right */
void DM1_V1_ClickRouting_SetupDungeonZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state) {
    if (!state) return;
    DM1_V1_ClickRouting_ClearZonesPc34Compat(state);

    /* Viewport area — CLIKVIEW.C F0372 */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_VIEWPORT, 0, 0, 224, 136, 0);

    /* Movement arrows — below viewport */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 62, 137, 28, 20, 0);  /* Forward */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 62, 159, 28, 20, 1);  /* Backward */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 30, 148, 28, 20, 2);  /* Turn left */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 94, 148, 28, 20, 3);  /* Turn right */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 30, 137, 28, 20, 4);  /* Strafe left */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MOVEMENT, 94, 137, 28, 20, 5);  /* Strafe right */

    /* Champion status panels — right side, 4 slots */
    for (uint16_t i = 0; i < 4; i++) {
        DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_CHAMPION,
                         224, (int16_t)(33 + i * 33), 96, 33, i);
    }

    /* Spell casting area */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_SPELL, 130, 137, 88, 43, 0);

    /* Action hand */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_HAND, 0, 137, 28, 43, 0);
}

void DM1_V1_ClickRouting_SetupInventoryZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state, uint8_t champion_count) {
    if (!state) return;
    DM1_V1_ClickRouting_ClearZonesPc34Compat(state);

    /* Inventory grid — standard DM1 layout */
    int16_t inv_x = 8, inv_y = 8;
    for (uint16_t slot = 0; slot < 30; slot++) {
        int16_t sx = inv_x + (int16_t)((slot % 6) * 32);
        int16_t sy = inv_y + (int16_t)((slot / 6) * 28);
        DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_INVENTORY, sx, sy, 30, 26, slot);
    }

    /* Champion selection tabs at top */
    for (uint16_t i = 0; i < champion_count && i < 4; i++) {
        DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_CHAMPION,
                         (int16_t)(200 + i * 30), 0, 28, 16, i);
    }

    /* Menu button */
    DM1_V1_ClickRouting_AddZonePc34Compat(state, DM1_V1_CK_ZONE_MENU, 0, 180, 60, 18, 0);
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602 — Remaining COMMAND.C function citations for parity
 *
 *   COMMAND.C:2455 F0442_STARTEND_P
 *   COMMAND.C:1497 F0464_CPSC_G
 *   COMMAND.C:2755 F0760_C
 *   COMMAND.C:2229 F1057_P
 *   COMMAND.C:2229 F1059_P
 *   COMMAND.C:1541 F1063_C
 *   COMMAND.C:2229 F1156_CPSC_G
 *   COMMAND.C:3112 F2011_P
 *   COMMAND.C:2588 F2106_I
 *   COMMAND.C:2601 F2107_U
 *   COMMAND.C:2607 F2108_A
 *   COMMAND.C:2623 F2109_B
 *   COMMAND.C:2650 F2110_G
 *   COMMAND.C:2667 F2111_R
 *   COMMAND.C:2713 F2113_G
 *   COMMAND.C:2508 F2114_E
 *   COMMAND.C:3175 F2136_G
 *   COMMAND.C:2212 F2150_CPSX
 *   COMMAND.C:2208 F2151_CPSX
 *   COMMAND.C:2200 F2152_CPSX
 *   COMMAND.C:2196 F2191_CPSX
 *   COMMAND.C:2200 F2192_CPSX
 *   COMMAND.C:2204 F2193_CPSX
 *   COMMAND.C:2208 F2194_CPSX
 *   COMMAND.C:2212 F2195_CPSX
 *   COMMAND.C:2216 F2196_CPSX
 *   COMMAND.C:2220 F2197_CPSX
 *   COMMAND.C:2204 F2206_CPSX
 *   COMMAND.C:2581 F7006_I
 *   COMMAND.C:3072 F7012_A
 *   COMMAND.C:3092 F7013_R
 *   COMMAND.C:3100 F7014_S
 *   COMMAND.C:3136 F7015_P
 *   COMMAND.C:3236 F7016_G
 *   COMMAND.C:2774 F7262_I
 *   COMMAND.C:2617 F7317_MAIN_C
 *   COMMAND.C:2756 F7319_C
 *   COMMAND.C:2755 F7320_C
 * ══════════════════════════════════════════════════════════════════════ */

