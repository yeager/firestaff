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
    M11_CK_ZONE_NONE = 0,
    M11_CK_ZONE_VIEWPORT,       /* CLIKVIEW.C: dungeon view area */
    M11_CK_ZONE_CHAMPION,       /* CLIKCHAM.C: champion panel */
    M11_CK_ZONE_INVENTORY,      /* CLIKMENU.C: inventory slots */
    M11_CK_ZONE_MOVEMENT,       /* Movement arrows */
    M11_CK_ZONE_SPELL,          /* Spell casting symbols */
    M11_CK_ZONE_HAND,           /* Action hand area */
    M11_CK_ZONE_MENU            /* Menu buttons */
} M11_CK_ZoneType;

typedef struct {
    M11_CK_ZoneType type;
    int16_t x, y, w, h;         /* Screen rectangle */
    uint16_t data;               /* Zone-specific identifier (champion index, slot, etc.) */
} M11_CK_Zone;

/* Click result */
typedef struct {
    M11_CK_ZoneType zone_type;
    uint16_t zone_data;
    int16_t local_x, local_y;   /* Position within zone */
    bool    hit;
} M11_CK_ClickResult;

typedef struct {
    M11_CK_Zone zones[DM1_CK_MAX_ZONES];
    uint8_t zone_count;
    int16_t mouse_x, mouse_y;
    bool    mouse_visible;
    bool    left_pressed;
    bool    right_pressed;
} M11_CK_State;

void m11_ck_init(M11_CK_State* state);
void m11_ck_clear_zones(M11_CK_State* state);
bool m11_ck_add_zone(M11_CK_State* state, M11_CK_ZoneType type,
                      int16_t x, int16_t y, int16_t w, int16_t h,
                      uint16_t data);
void m11_ck_update_mouse(M11_CK_State* state, int16_t x, int16_t y,
                          bool left, bool right);
M11_CK_ClickResult m11_ck_test_click(const M11_CK_State* state,
                                      int16_t x, int16_t y);
void m11_ck_setup_dungeon_zones(M11_CK_State* state);
void m11_ck_setup_inventory_zones(M11_CK_State* state, uint8_t champion_count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CLICK_ROUTING_PC34_COMPAT_H */
