/* DM1 V1 Portrait/Panel Rendering — source-locked from ReDMCSB
 * PORTRAIT.C: F0515 portrait bitplane conversion (32x29 pixels)
 * PANEL.C: champion panel layout, status bars, skill level display
 * CHAMDRAW.C: champion portrait drawing in HUD area */
#ifndef FIRESTAFF_DM1_V1_PORTRAIT_PANEL_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PORTRAIT_PANEL_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_PORTRAIT_W         32
#define DM1_PORTRAIT_H         29
#define DM1_PORTRAIT_BITPLANES  4
#define DM1_MAX_CHAMPIONS       4
#define DM1_PANEL_BAR_W        25   /* HP/mana/stamina bar width */
#define DM1_PANEL_BAR_H         3   /* Bar height in pixels */

/* Champion portrait state */
typedef struct {
    uint8_t  planar_data[DM1_PORTRAIT_W / 8 * DM1_PORTRAIT_H * DM1_PORTRAIT_BITPLANES];
    uint8_t  chunky_data[DM1_PORTRAIT_W * DM1_PORTRAIT_H]; /* converted for blitting */
    bool     loaded;
    bool     injured;  /* flash portrait red when damaged */
    uint8_t  damage_flash_timer;
} M11_PP_Portrait;

/* Status bar */
typedef struct {
    int16_t  current;
    int16_t  max;
    uint8_t  color;     /* bar color index (red=HP, blue=mana, green=stamina) */
} M11_PP_StatusBar;

/* Champion panel slot */
typedef struct {
    M11_PP_Portrait portrait;
    M11_PP_StatusBar hp;
    M11_PP_StatusBar mana;
    M11_PP_StatusBar stamina;
    M11_PP_StatusBar food;
    M11_PP_StatusBar water;
    int16_t  panel_x;   /* screen x of this panel slot */
    int16_t  panel_y;   /* screen y */
    bool     selected;  /* is this the active champion? */
    bool     alive;
} M11_PP_ChampionPanel;

typedef struct {
    M11_PP_ChampionPanel panels[DM1_MAX_CHAMPIONS];
    uint8_t  active_count;
    int8_t   selected_index; /* -1 = none selected */
} M11_PP_PanelState;

void m11_pp_init(M11_PP_PanelState* state);
void m11_pp_set_champion_count(M11_PP_PanelState* state, uint8_t count);
bool m11_pp_load_portrait(M11_PP_Portrait* port, const uint8_t* planar_data,
                           uint16_t data_size);
void m11_pp_convert_planar_to_chunky(M11_PP_Portrait* port);
void m11_pp_update_bars(M11_PP_ChampionPanel* panel,
                         int16_t hp, int16_t max_hp,
                         int16_t mana, int16_t max_mana,
                         int16_t stamina, int16_t max_stamina,
                         int16_t food, int16_t water);
void m11_pp_select(M11_PP_PanelState* state, int8_t index);
void m11_pp_damage_flash(M11_PP_PanelState* state, uint8_t champ_idx);
void m11_pp_tick(M11_PP_PanelState* state); /* update flash timers */
void m11_pp_layout(M11_PP_PanelState* state, int16_t base_x, int16_t base_y);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_PORTRAIT_PANEL_PC34_COMPAT_H */
