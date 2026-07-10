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
} DM1_V1_PortraitPanelPortraitPc34;

/* Status bar */
typedef struct {
    int16_t  current;
    int16_t  max;
    uint8_t  color;     /* bar color index (red=HP, blue=mana, green=stamina) */
} DM1_V1_PortraitPanelStatusBarPc34;

/* Champion panel slot */
typedef struct {
    DM1_V1_PortraitPanelPortraitPc34 portrait;
    DM1_V1_PortraitPanelStatusBarPc34 hp;
    DM1_V1_PortraitPanelStatusBarPc34 mana;
    DM1_V1_PortraitPanelStatusBarPc34 stamina;
    DM1_V1_PortraitPanelStatusBarPc34 food;
    DM1_V1_PortraitPanelStatusBarPc34 water;
    int16_t  panel_x;   /* screen x of this panel slot */
    int16_t  panel_y;   /* screen y */
    bool     selected;  /* is this the active champion? */
    bool     alive;
} DM1_V1_PortraitPanelChampionPc34;

typedef struct {
    DM1_V1_PortraitPanelChampionPc34 panels[DM1_MAX_CHAMPIONS];
    uint8_t  active_count;
    int8_t   selected_index; /* -1 = none selected */
} DM1_V1_PortraitPanelStatePc34;

void DM1_V1_PortraitPanel_InitPc34Compat(DM1_V1_PortraitPanelStatePc34* state);
void DM1_V1_PortraitPanel_SetChampionCountPc34Compat(DM1_V1_PortraitPanelStatePc34* state, uint8_t count);
bool DM1_V1_PortraitPanel_LoadPortraitPc34Compat(DM1_V1_PortraitPanelPortraitPc34* port, const uint8_t* planar_data,
                           uint16_t data_size);
void DM1_V1_PortraitPanel_ConvertPlanarToChunkyPc34Compat(DM1_V1_PortraitPanelPortraitPc34* port);
void DM1_V1_PortraitPanel_UpdateBarsPc34Compat(DM1_V1_PortraitPanelChampionPc34* panel,
                         int16_t hp, int16_t max_hp,
                         int16_t mana, int16_t max_mana,
                         int16_t stamina, int16_t max_stamina,
                         int16_t food, int16_t water);
void DM1_V1_PortraitPanel_SelectPc34Compat(DM1_V1_PortraitPanelStatePc34* state, int8_t index);
void DM1_V1_PortraitPanel_DamageFlashPc34Compat(DM1_V1_PortraitPanelStatePc34* state, uint8_t champ_idx);
void DM1_V1_PortraitPanel_TickPc34Compat(DM1_V1_PortraitPanelStatePc34* state); /* update flash timers */
void DM1_V1_PortraitPanel_LayoutPc34Compat(DM1_V1_PortraitPanelStatePc34* state, int16_t base_x, int16_t base_y);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_PORTRAIT_PANEL_PC34_COMPAT_H */
