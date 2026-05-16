#ifndef FIRESTAFF_DM1_V1_VIEWPORT_CLICK_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_VIEWPORT_CLICK_PC34_COMPAT_H

/*
 * DM1 V1 Viewport & Menu Click Routing — source-locked to ReDMCSB
 *   CLIKVIEW.C and CLIKMENU.C
 *
 * Viewport click processing: wall sensor activation, item pickup/drop,
 * creature attack targeting. Menu click zones: movement arrows, spell
 * area, action area, champion panels, inventory grid.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   CLIKVIEW.C: F0372 (click viewport — wall sensor), F0373 (grab object),
 *               F0374 (throw/put object), F0375 (attack creature)
 *   CLIKMENU.C: F0365 (turn dispatch), F0366 (step dispatch),
 *               F0367 (champion panel click), F0368 (spell click),
 *               F0369 (action area click)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Click zone identifiers */
typedef enum {
    DM1_ZONE_NONE = 0,
    DM1_ZONE_VIEWPORT,
    DM1_ZONE_MOVEMENT_FORWARD,
    DM1_ZONE_MOVEMENT_LEFT,
    DM1_ZONE_MOVEMENT_RIGHT,
    DM1_ZONE_MOVEMENT_BACKWARD,
    DM1_ZONE_MOVEMENT_TURN_LEFT,
    DM1_ZONE_MOVEMENT_TURN_RIGHT,
    DM1_ZONE_SPELL_AREA,
    DM1_ZONE_ACTION_AREA,
    DM1_ZONE_CHAMPION_0,
    DM1_ZONE_CHAMPION_1,
    DM1_ZONE_CHAMPION_2,
    DM1_ZONE_CHAMPION_3,
    DM1_ZONE_INVENTORY,
    DM1_ZONE_MAP,
    DM1_ZONE_OPTIONS,
    DM1_ZONE_COUNT
} M11_ClickZoneId;

/* Viewport view cell — from CLIKVIEW.C */
typedef enum {
    DM1_VIEW_CELL_FRONT_LEFT = 0,
    DM1_VIEW_CELL_FRONT_RIGHT = 1,
    DM1_VIEW_CELL_BACK_RIGHT = 2,
    DM1_VIEW_CELL_BACK_LEFT = 3,
    DM1_VIEW_CELL_COUNT
} M11_ViewCell;

/* Click zone rectangle */
typedef struct {
    int x, y, w, h;
    M11_ClickZoneId zoneId;
    int enabled;
} M11_ClickZone;

#define M11_MAX_CLICK_ZONES 64

/* Click routing state */
typedef struct {
    M11_ClickZone zones[M11_MAX_CLICK_ZONES];
    int zoneCount;
    int lastClickX;
    int lastClickY;
    M11_ClickZoneId lastClickZone;
    int mouseDown;
    int mouseButton;        /* 1=right, 2=left (DM1 convention) */
} M11_ClickState;

/* Viewport click result (from CLIKVIEW.C processing) */
typedef struct {
    M11_ViewCell viewCell;          /* which cell was clicked */
    int wallSensorTriggered;        /* F0372: front wall sensor activated */
    int objectGrabbed;              /* F0373: picked up an object */
    int objectThrown;               /* F0374: threw/placed an object */
    int creatureAttacked;           /* F0375: attack in viewport */
    int targetMapX;                 /* world coords of click target */
    int targetMapY;
    int stopWaitingForInput;        /* G0321 flag set */
} M11_ViewportClickResult;

/*
 * Initialize click routing state.
 */
void m11_click_init(M11_ClickState *s);

/*
 * Add a click zone. Returns index or -1 if full.
 */
int m11_click_add_zone(M11_ClickState *s, int x, int y, int w, int h,
                       M11_ClickZoneId zoneId);

/*
 * Enable/disable a zone by ID.
 */
void m11_click_enable_zone(M11_ClickState *s, M11_ClickZoneId zoneId,
                           int enabled);

/*
 * Hit test: which zone does (mx, my) fall in?
 * Returns DM1_ZONE_NONE if no hit.
 */
M11_ClickZoneId m11_click_hit_test(const M11_ClickState *s, int mx, int my);

/*
 * Process a mouse click. Returns the zone hit.
 */
M11_ClickZoneId m11_click_mouse_down(M11_ClickState *s,
                                      int mx, int my, int button);

/*
 * Process mouse release. Returns the zone where the click occurred.
 */
M11_ClickZoneId m11_click_mouse_up(M11_ClickState *s, int mx, int my);

/*
 * Set up standard DM1 game screen zones.
 * Viewport: 0,0 224x136
 * Movement arrows: 234,124 86x76
 * Spell area: 233,2 85x70
 * Action area: 233,72 85x50
 * Champion panels: 0,136 80x16 each
 */
void m11_click_setup_game_zones(M11_ClickState *s);

/*
 * Set up inventory screen zones (slot grid).
 */
void m11_click_setup_inventory_zones(M11_ClickState *s);

/*
 * Clear all zones.
 */
void m11_click_clear_zones(M11_ClickState *s);

/*
 * Resolve a viewport click to a view cell (F0372-F0375).
 * mx, my: click position within the 224x136 viewport.
 * partyDir: current party direction (0-3).
 * partyX, partyY: party map position.
 * hasLeader: 1 if party has a leader.
 * leaderHandEmpty: 1 if leader's hand is empty.
 */
M11_ViewportClickResult m11_viewport_resolve_click(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty);

/*
 * Source evidence string.
 */
const char *m11_viewport_click_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_CLICK_PC34_COMPAT_H */
