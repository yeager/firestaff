/* DM1 V1 Click/Mouse Routing — source-locked from ReDMCSB
 * CLIKVIEW.C F0372: click in dungeon view (touch front wall sensor)
 * CLIKCHAM.C F0367: click in champion status box
 * CLIKMENU.C: click in menu/inventory area
 * MOUSESET.C: mouse position tracking, cursor visibility
 * COMMAND.C: command type dispatching from click coordinates */
#ifndef FIRESTAFF_DM1_V1_CLICK_ROUTING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CLICK_ROUTING_PC34_COMPAT_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM1_CK_MAX_ZONES    32

/* Click target types — from COMMAND.C command type dispatching */
typedef enum {
    DM1_V1_CK_ZONE_NONE = 0,
    DM1_V1_CK_ZONE_VIEWPORT,       /* CLIKVIEW.C: dungeon view area */
    DM1_V1_CK_ZONE_CHAMPION,       /* CLIKCHAM.C: champion panel */
    DM1_V1_CK_ZONE_INVENTORY,      /* CLIKMENU.C: inventory slots */
    DM1_V1_CK_ZONE_MOVEMENT,       /* Movement arrows */
    DM1_V1_CK_ZONE_SPELL,          /* Spell casting symbols */
    DM1_V1_CK_ZONE_HAND,           /* Action hand area */
    DM1_V1_CK_ZONE_MENU            /* Menu buttons */
} DM1_V1_ClickZoneTypePc34;

typedef struct {
    DM1_V1_ClickZoneTypePc34 type;
    int16_t x, y, w, h;         /* Screen rectangle */
    uint16_t data;               /* Zone-specific identifier (champion index, slot, etc.) */
} DM1_V1_ClickZonePc34;

/* Click result */
typedef struct {
    DM1_V1_ClickZoneTypePc34 zone_type;
    uint16_t zone_data;
    int16_t local_x, local_y;   /* Position within zone */
    bool    hit;
} DM1_V1_ClickResultPc34;

typedef struct {
    DM1_V1_ClickZonePc34 zones[DM1_CK_MAX_ZONES];
    uint8_t zone_count;
    int16_t mouse_x, mouse_y;
    bool    mouse_visible;
    bool    left_pressed;
    bool    right_pressed;
} DM1_V1_ClickRoutingStatePc34;

void DM1_V1_ClickRouting_InitPc34Compat(DM1_V1_ClickRoutingStatePc34* state);
void DM1_V1_ClickRouting_ClearZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state);
bool DM1_V1_ClickRouting_AddZonePc34Compat(DM1_V1_ClickRoutingStatePc34* state, DM1_V1_ClickZoneTypePc34 type,
                      int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t data);
void DM1_V1_ClickRouting_UpdateMousePc34Compat(DM1_V1_ClickRoutingStatePc34* state, int16_t x, int16_t y,
                          bool left, bool right);
DM1_V1_ClickResultPc34 DM1_V1_ClickRouting_TestClickPc34Compat(const DM1_V1_ClickRoutingStatePc34* state,
                                      int16_t x, int16_t y);
void DM1_V1_ClickRouting_SetupDungeonZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state);
void DM1_V1_ClickRouting_SetupInventoryZonesPc34Compat(DM1_V1_ClickRoutingStatePc34* state, uint8_t champion_count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CLICK_ROUTING_PC34_COMPAT_H */
