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
} DM1_V1_ClickZoneIdPc34;

/* Viewport view cell — from CLIKVIEW.C */
typedef enum {
    DM1_VIEW_CELL_FRONT_LEFT = 0,
    DM1_VIEW_CELL_FRONT_RIGHT = 1,
    DM1_VIEW_CELL_BACK_RIGHT = 2,
    DM1_VIEW_CELL_BACK_LEFT = 3,
    DM1_VIEW_CELL_COUNT
} DM1_V1_ViewCellPc34;

/* Click zone rectangle */
typedef struct {
    int x, y, w, h;
    DM1_V1_ClickZoneIdPc34 zoneId;
    int enabled;
} DM1_V1_ClickZonePc34;

#define DM1_V1_MAX_CLICK_ZONES_PC34 64

/* Click routing state */
typedef struct {
    DM1_V1_ClickZonePc34 zones[DM1_V1_MAX_CLICK_ZONES_PC34];
    int zoneCount;
    int lastClickX;
    int lastClickY;
    DM1_V1_ClickZoneIdPc34 lastClickZone;
    int mouseDown;
    int mouseButton;        /* 1=right, 2=left (DM1 convention) */
} DM1_V1_ClickStatePc34;

/* Viewport click result (from CLIKVIEW.C processing) */
typedef struct {
    DM1_V1_ViewCellPc34 viewCell;          /* which cell was clicked */
    int wallSensorTriggered;        /* F0372: front wall sensor activated */
    int objectGrabbed;              /* F0373: picked up an object */
    int pileTopObjectId;            /* G0292_aT_PileTopObject[viewCell] */
    int objectThrown;               /* F0374: threw/placed an object */
    int creatureAttacked;           /* F0375: attack in viewport */
    int targetMapX;                 /* world coords of click target */
    int targetMapY;
    int stopWaitingForInput;        /* G0321 flag set */
} DM1_V1_ViewportClickResultPc34;

typedef struct {
    uint8_t grabbableCellMask;
    int pileTopObjectId[DM1_VIEW_CELL_COUNT];
} DM1_V1_ViewportGrabbableStatePc34;

/*
 * Initialize click routing state.
 */
void DM1_V1_Click_InitPc34Compat(DM1_V1_ClickStatePc34 *s);

/*
 * Add a click zone. Returns index or -1 if full.
 */
int DM1_V1_Click_AddZonePc34Compat(DM1_V1_ClickStatePc34 *s, int x, int y, int w, int h,
                       DM1_V1_ClickZoneIdPc34 zoneId);

/*
 * Enable/disable a zone by ID.
 */
void DM1_V1_Click_EnableZonePc34Compat(DM1_V1_ClickStatePc34 *s, DM1_V1_ClickZoneIdPc34 zoneId,
                           int enabled);

/*
 * Hit test: which zone does (mx, my) fall in?
 * Returns DM1_ZONE_NONE if no hit.
 */
DM1_V1_ClickZoneIdPc34 DM1_V1_Click_HitTestPc34Compat(const DM1_V1_ClickStatePc34 *s, int mx, int my);

/*
 * Process a mouse click. Returns the zone hit.
 */
DM1_V1_ClickZoneIdPc34 DM1_V1_Click_MouseDownPc34Compat(DM1_V1_ClickStatePc34 *s,
                                      int mx, int my, int button);

/*
 * Process mouse release. Returns the zone where the click occurred.
 */
DM1_V1_ClickZoneIdPc34 DM1_V1_Click_MouseUpPc34Compat(DM1_V1_ClickStatePc34 *s, int mx, int my);

/*
 * Set up standard DM1 game screen zones.
 * Viewport: 0,0 224x136
 * Movement arrows: 234,124 86x76
 * Spell area: 233,2 85x70
 * Action area: 233,72 85x50
 * Champion panels: 0,136 80x16 each
 */
void DM1_V1_Click_SetupGameZonesPc34Compat(DM1_V1_ClickStatePc34 *s);

/*
 * Set up inventory screen zones (slot grid).
 */
void DM1_V1_Click_SetupInventoryZonesPc34Compat(DM1_V1_ClickStatePc34 *s);

/*
 * Clear all zones.
 */
void DM1_V1_Click_ClearZonesPc34Compat(DM1_V1_ClickStatePc34 *s);

/*
 * Resolve a viewport click to a view cell (F0372-F0375).
 * mx, my: click position within the 224x136 viewport.
 * partyDir: current party direction (0-3).
 * partyX, partyY: party map position.
 * hasLeader: 1 if party has a leader.
 * leaderHandEmpty: 1 if leader's hand is empty.
 */
DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickPc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty);

DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickWithGrabbableMaskPc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty, uint8_t grabbableCellMask);

DM1_V1_ViewportClickResultPc34 DM1_V1_Viewport_ResolveClickWithGrabbableStatePc34Compat(
    int mx, int my, int partyDir, int partyX, int partyY,
    int hasLeader, int leaderHandEmpty,
    const DM1_V1_ViewportGrabbableStatePc34 *grabbableState);

void DM1_V1_ViewportGrabbable_InitPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state);
void DM1_V1_ViewportGrabbable_ClearPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state);
int DM1_V1_ViewportGrabbable_SetPileTopPc34Compat(DM1_V1_ViewportGrabbableStatePc34 *state,
                                        DM1_V1_ViewCellPc34 cell,
                                        int pileTopObjectId);
int DM1_V1_ViewportGrabbable_PileTopPc34Compat(
    const DM1_V1_ViewportGrabbableStatePc34 *state, DM1_V1_ViewCellPc34 cell);

/*
 * Source evidence string.
 */
const char *DM1_V1_ViewportClick_SourceEvidencePc34Compat(void);

#define DM1_VIEWPORT_GRABBABLE_CELL_MASK(cell) ((uint8_t)(1u << (cell)))
#define DM1_VIEWPORT_GRABBABLE_NO_CELLS ((uint8_t)0x00u)
#define DM1_VIEWPORT_GRABBABLE_ALL_CELLS ((uint8_t)0x0fu)
#define DM1_VIEWPORT_NO_PILE_TOP_OBJECT (-1)

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_VIEWPORT_CLICK_PC34_COMPAT_H */
