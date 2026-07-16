/* DM1 V1 Portrait/Panel Rendering — source-locked from ReDMCSB
 * PORTRAIT.C F0515: Amiga→AtariST planar bitplane conversion for 32x29 portraits
 * PANEL.C: panel layout with 4 champion slots at right side of screen
 * CHAMDRAW.C: portrait draw calls and damage flash overlay */

#include "dm1_v1_portrait_panel_pc34_compat.h"
#include <string.h>

void DM1_V1_PortraitPanel_InitPc34Compat(DM1_V1_PortraitPanelStatePc34* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_V1_PortraitPanelStatePc34));
    state->selected_index = -1;
}

void DM1_V1_PortraitPanel_SetChampionCountPc34Compat(DM1_V1_PortraitPanelStatePc34* state, uint8_t count) {
    if (!state) return;
    if (count > DM1_MAX_CHAMPIONS) count = DM1_MAX_CHAMPIONS;
    state->active_count = count;
}

/* F0515 pattern: load planar portrait data (4 bitplanes, 32x29)
 * Expected size: (32/8) * 29 * 4 = 464 bytes */
bool DM1_V1_PortraitPanel_LoadPortraitPc34Compat(DM1_V1_PortraitPanelPortraitPc34* port, const uint8_t* planar_data,
                           uint16_t data_size) {
    if (!port || !planar_data) return false;
    uint16_t expected = DM1_PORTRAIT_PLANAR_BYTES;
    if (data_size < expected) return false;

    memcpy(port->planar_data, planar_data, expected);
    port->loaded = true;
    port->injured = false;
    port->damage_flash_timer = 0;
    return true;
}

bool DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
    const uint8_t* planar_data, uint16_t planar_size,
    uint8_t* chunky_data, uint16_t chunky_size) {
    uint16_t plane_size = (DM1_PORTRAIT_W / 8) * DM1_PORTRAIT_H;
    const uint8_t* planes[DM1_PORTRAIT_BITPLANES];

    if (!planar_data || !chunky_data) return false;
    if (planar_size < DM1_PORTRAIT_PLANAR_BYTES ||
        chunky_size < DM1_PORTRAIT_CHUNKY_BYTES) {
        return false;
    }

    for (int p = 0; p < DM1_PORTRAIT_BITPLANES; p++) {
        planes[p] = planar_data + (p * plane_size);
    }

    for (int y = 0; y < DM1_PORTRAIT_H; y++) {
        for (int x = 0; x < DM1_PORTRAIT_W; x++) {
            int byte_idx = y * (DM1_PORTRAIT_W / 8) + (x / 8);
            int bit = 7 - (x % 8);
            uint8_t pixel = 0;
            for (int p = 0; p < DM1_PORTRAIT_BITPLANES; p++) {
                pixel |= (uint8_t)(((planes[p][byte_idx] >> bit) & 1) << p);
            }
            chunky_data[y * DM1_PORTRAIT_W + x] = pixel;
        }
    }
    return true;
}

bool DM1_V1_PortraitPanel_ConvertChunkyBufferToPlanarPc34Compat(
    const uint8_t* chunky_data, uint16_t chunky_size,
    uint8_t* planar_data, uint16_t planar_size) {
    uint16_t plane_size = (DM1_PORTRAIT_W / 8) * DM1_PORTRAIT_H;

    if (!chunky_data || !planar_data) return false;
    if (chunky_size < DM1_PORTRAIT_CHUNKY_BYTES ||
        planar_size < DM1_PORTRAIT_PLANAR_BYTES) {
        return false;
    }

    memset(planar_data, 0, DM1_PORTRAIT_PLANAR_BYTES);
    for (int y = 0; y < DM1_PORTRAIT_H; y++) {
        for (int x = 0; x < DM1_PORTRAIT_W; x++) {
            int byte_idx = y * (DM1_PORTRAIT_W / 8) + (x / 8);
            int bit = 7 - (x % 8);
            uint8_t pixel = (uint8_t)(chunky_data[y * DM1_PORTRAIT_W + x] & 0x0F);
            for (int p = 0; p < DM1_PORTRAIT_BITPLANES; p++) {
                planar_data[p * plane_size + byte_idx] |=
                    (uint8_t)(((pixel >> p) & 1u) << bit);
            }
        }
    }
    return true;
}

/* Convert planar bitplanes to chunky 8-bit indexed pixels.
 * F0515/F2105: iterate through 4 bitplanes, extract pixel values. */
void DM1_V1_PortraitPanel_ConvertPlanarToChunkyPc34Compat(DM1_V1_PortraitPanelPortraitPc34* port) {
    if (!port || !port->loaded) return;
    (void)DM1_V1_PortraitPanel_ConvertPlanarBufferToChunkyPc34Compat(
        port->planar_data, DM1_PORTRAIT_PLANAR_BYTES,
        port->chunky_data, DM1_PORTRAIT_CHUNKY_BYTES);
}

void DM1_V1_PortraitPanel_UpdateBarsPc34Compat(DM1_V1_PortraitPanelChampionPc34* panel,
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

void DM1_V1_PortraitPanel_SelectPc34Compat(DM1_V1_PortraitPanelStatePc34* state, int8_t index) {
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

void DM1_V1_PortraitPanel_DamageFlashPc34Compat(DM1_V1_PortraitPanelStatePc34* state, uint8_t champ_idx) {
    if (!state || champ_idx >= DM1_MAX_CHAMPIONS) return;
    state->panels[champ_idx].portrait.injured = true;
    state->panels[champ_idx].portrait.damage_flash_timer = 6; /* 6 frames */
}

void DM1_V1_PortraitPanel_TickPc34Compat(DM1_V1_PortraitPanelStatePc34* state) {
    if (!state) return;
    for (int i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        DM1_V1_PortraitPanelPortraitPc34* p = &state->panels[i].portrait;
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
void DM1_V1_PortraitPanel_LayoutPc34Compat(DM1_V1_PortraitPanelStatePc34* state, int16_t base_x, int16_t base_y) {
    if (!state) return;
    for (int i = 0; i < DM1_MAX_CHAMPIONS; i++) {
        state->panels[i].panel_x = base_x;
        state->panels[i].panel_y = base_y + (int16_t)(i * 33);
    }
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass602b — PORTRAIT.C remaining function citations
 *
 *   PORTRAIT.C:152 F2095_A
 *   PORTRAIT.C:137 F2104_CHAMPION_C
 *   PORTRAIT.C:165 F2105_CHAMPION_C
 *   PORTRAIT.C:162 F7024_F
 *   PORTRAIT.C:192 F7251_C
 *   PORTRAIT.C:212 F7252_C
 * ══════════════════════════════════════════════════════════════════════ */
