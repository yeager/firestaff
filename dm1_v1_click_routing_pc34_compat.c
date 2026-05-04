/* DM1 V1 Click/Mouse Routing — source-locked from ReDMCSB
 * CLIKVIEW.C F0372: ProcessType80_ClickInDungeonView_TouchFrontWallSensor
 * CLIKCHAM.C F0367: ProcessTypes12To27_ClickInChampionStatusBox
 * CLIKMENU.C: menu click processing
 * COMMAND.C: central command dispatcher from mouse coordinates */

#include "dm1_v1_click_routing_pc34_compat.h"
#include <string.h>

void m11_ck_init(M11_CK_State* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_CK_State));
}

void m11_ck_clear_zones(M11_CK_State* state) {
    if (!state) return;
    state->zone_count = 0;
}

bool m11_ck_add_zone(M11_CK_State* state, M11_CK_ZoneType type,
                      int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t data) {
    if (!state || state->zone_count >= DM1_CK_MAX_ZONES) return false;
    M11_CK_Zone* z = &state->zones[state->zone_count++];
    z->type = type;
    z->x = x; z->y = y; z->w = w; z->h = h;
    z->data = data;
    return true;
}

void m11_ck_update_mouse(M11_CK_State* state, int16_t x, int16_t y,
                          bool left, bool right) {
    if (!state) return;
    state->mouse_x = x;
    state->mouse_y = y;
    state->left_pressed = left;
    state->right_pressed = right;
    state->mouse_visible = true;
}

/* COMMAND.C dispatch pattern: test click against all registered zones */
M11_CK_ClickResult m11_ck_test_click(const M11_CK_State* state,
                                      int16_t x, int16_t y) {
    M11_CK_ClickResult result;
    memset(&result, 0, sizeof(result));

    if (!state) return result;

    for (uint8_t i = 0; i < state->zone_count; i++) {
        const M11_CK_Zone* z = &state->zones[i];
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
void m11_ck_setup_dungeon_zones(M11_CK_State* state) {
    if (!state) return;
    m11_ck_clear_zones(state);

    /* Viewport area — CLIKVIEW.C F0372 */
    m11_ck_add_zone(state, M11_CK_ZONE_VIEWPORT, 0, 0, 224, 136, 0);

    /* Movement arrows — below viewport */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 62, 137, 28, 20, 0);  /* Forward */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 62, 159, 28, 20, 1);  /* Backward */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 30, 148, 28, 20, 2);  /* Turn left */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 94, 148, 28, 20, 3);  /* Turn right */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 30, 137, 28, 20, 4);  /* Strafe left */
    m11_ck_add_zone(state, M11_CK_ZONE_MOVEMENT, 94, 137, 28, 20, 5);  /* Strafe right */

    /* Champion status panels — right side, 4 slots */
    for (uint16_t i = 0; i < 4; i++) {
        m11_ck_add_zone(state, M11_CK_ZONE_CHAMPION,
                         224, (int16_t)(33 + i * 33), 96, 33, i);
    }

    /* Spell casting area */
    m11_ck_add_zone(state, M11_CK_ZONE_SPELL, 130, 137, 88, 43, 0);

    /* Action hand */
    m11_ck_add_zone(state, M11_CK_ZONE_HAND, 0, 137, 28, 43, 0);
}

void m11_ck_setup_inventory_zones(M11_CK_State* state, uint8_t champion_count) {
    if (!state) return;
    m11_ck_clear_zones(state);

    /* Inventory grid — standard DM1 layout */
    int16_t inv_x = 8, inv_y = 8;
    for (uint16_t slot = 0; slot < 30; slot++) {
        int16_t sx = inv_x + (int16_t)((slot % 6) * 32);
        int16_t sy = inv_y + (int16_t)((slot / 6) * 28);
        m11_ck_add_zone(state, M11_CK_ZONE_INVENTORY, sx, sy, 30, 26, slot);
    }

    /* Champion selection tabs at top */
    for (uint16_t i = 0; i < champion_count && i < 4; i++) {
        m11_ck_add_zone(state, M11_CK_ZONE_CHAMPION,
                         (int16_t)(200 + i * 30), 0, 28, 16, i);
    }

    /* Menu button */
    m11_ck_add_zone(state, M11_CK_ZONE_MENU, 0, 180, 60, 18, 0);
}
