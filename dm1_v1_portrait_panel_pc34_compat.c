/* DM1 V1 Portrait/Panel Rendering — source-locked from ReDMCSB
 * PORTRAIT.C F0515: Amiga→AtariST planar bitplane conversion for 32x29 portraits
 * PANEL.C: panel layout with 4 champion slots at right side of screen
 * CHAMDRAW.C: portrait draw calls and damage flash overlay */

#include "dm1_v1_portrait_panel_pc34_compat.h"
#include <string.h>

void m11_pp_init(M11_PP_PanelState* state) {
    if (!state) return;
    memset(state, 0, sizeof(M11_PP_PanelState));
    state->selected_index = -1;
}

void m11_pp_set_champion_count(M11_PP_PanelState* state, uint8_t count) {
    if (!state) return;
    if (count > DM1_MAX_CHAMPIONS) count = DM1_MAX_CHAMPIONS;
    state->active_count = count;
}

/* F0515 pattern: load planar portrait data (4 bitplanes, 32x29)
 * Expected size: (32/8) * 29 * 4 = 464 bytes */
bool m11_pp_load_portrait(M11_PP_Portrait* port, const uint8_t* planar_data,
                           uint16_t data_size) {
    if (!port || !planar_data) return false;
    uint16_t expected = (DM1_PORTRAIT_W / 8) * DM1_PORTRAIT_H * DM1_PORTRAIT_BITPLANES;
    if (data_size < expected) return false;

    memcpy(port->planar_data, planar_data, expected);
    port->loaded = true;
    port->injured = false;
    port->damage_flash_timer = 0;
    return true;
}

/* Convert Amiga planar bitplanes to chunky 8-bit indexed pixels
 * F0515: iterate through 4 bitplanes, extract pixel values */
void m11_pp_convert_planar_to_chunky(M11_PP_Portrait* port) {
    if (!port || !port->loaded) return;

    uint16_t plane_size = (DM1_PORTRAIT_W / 8) * DM1_PORTRAIT_H;
    const uint8_t* planes[DM1_PORTRAIT_BITPLANES];
    for (int p = 0; p < DM1_PORTRAIT_BITPLANES; p++) {
        planes[p] = port->planar_data + (p * plane_size);
    }

    for (int y = 0; y < DM1_PORTRAIT_H; y++) {
        for (int x = 0; x < DM1_PORTRAIT_W; x++) {
            int byte_idx = y * (DM1_PORTRAIT_W / 8) + (x / 8);
            int bit = 7 - (x % 8);
            uint8_t pixel = 0;
            for (int p = 0; p < DM1_PORTRAIT_BITPLANES; p++) {
                pixel |= (uint8_t)(((planes[p][byte_idx] >> bit) & 1) << p);
            }
            port->chunky_data[y * DM1_PORTRAIT_W + x] = pixel;
        }
    }
}

void m11_pp_update_bars(M11_PP_ChampionPanel* panel,
                         int16_t hp, int16_t max_hp,
                         int16_t mana, int16_t max_mana,
                         int16_t stamina, int16_t max_stamina,
                         int16_t food, int16_t water) {
    if (!panel) return;
    panel->hp.current = hp;
    panel->hp.max = max_hp;
    panel->hp.color = 8;  /* Red palette index for HP */
    panel->mana.current = mana;
    panel->mana.max = max_mana;
    panel->mana.color = 14; /* Blue palette index for mana */
    panel->stamina.current = stamina;
    panel->stamina.max = max_stamina;
    panel->stamina.color = 5; /* Green palette index for stamina */
    panel->food.current = food;
    panel->food.max = 2048; /* DM1 max food value */
    panel->food.color = 11; /* Yellow for food */
    panel->water.current = water;
    panel->water.max = 2048;
    panel->water.color = 4; /* Cyan for water */
    panel->alive = (hp > 0);
}

void m11_pp_select(M11_PP_PanelState* state, int8_t index) {
    if (!state) return;
    /* Deselect all */
    for (int i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        state->panels[i].selected = false;
    }
    state->selected_index = index;
    if (index >= 0 && index < (int8_t)state->active_count) {
        state->panels[index].selected = true;
    }
}

void m11_pp_damage_flash(M11_PP_PanelState* state, uint8_t champ_idx) {
    if (!state || champ_idx >= DM1_MAX_CHAMPIONS) return;
    state->panels[champ_idx].portrait.injured = true;
    state->panels[champ_idx].portrait.damage_flash_timer = 6; /* 6 frames */
}

void m11_pp_tick(M11_PP_PanelState* state) {
    if (!state) return;
    for (int i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        M11_PP_Portrait* p = &state->panels[i].portrait;
        if (p->damage_flash_timer > 0) {
            p->damage_flash_timer--;
            if (p->damage_flash_timer == 0) {
                p->injured = false;
            }
        }
    }
}

/* Layout champion panels at the right side of screen (224, 33)
 * Original DM1: 4 panels stacked vertically, each ~33px tall */
void m11_pp_layout(M11_PP_PanelState* state, int16_t base_x, int16_t base_y) {
    if (!state) return;
    for (int i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        state->panels[i].panel_x = base_x;
        state->panels[i].panel_y = base_y + (int16_t)(i * 33);
    }
}
