#ifndef FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H

/*
 * DM1 V1 Entrance & Champion Selection — source-locked to ReDMCSB ENTRANCE.C
 *
 * Mirror hall champion selection, resurrection, reincarnation, door animation.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   ENTRANCE.C: F0438 (open entrance doors), F0439 (draw entrance door),
 *               F0797 (draw micro dungeon behind doors),
 *               F0440/F0441 (champion selection/mirror interaction)
 *   CHAMPION.C: F0280 (add champion to party from mirror)
 *   REVIVE.C:   resurrection/reincarnation flows
 */

#include <stdint.h>

#include "dm1_v1_hoc_mirror_candidate_click_admission_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum champions in DM1: 4 party slots, 24 mirrors total. */
#define DM1_V1_MAX_CHAMPIONS_PC34          4
#define DM1_V1_MAX_MIRROR_SLOTS_PC34      24
#define DM1_V1_ENTRANCE_MICRO_DUNGEON_SIZE_PC34 25
#define DM1_V1_ENTRANCE_MAP_INDEX_PC34 255
#define DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 5
#define DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 5
#define DM1_V1_ENTRANCE_DIRECTION_SOUTH_PC34 2
#define DM1_V1_ENTRANCE_ELEMENT_WALL_PC34 0
#define DM1_V1_ENTRANCE_ELEMENT_CORRIDOR_PC34 1
#define DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_GRAPHIC_PC34 26
#define DM1_V1_ENTRANCE_RESURRECT_PANEL_GRAPHIC_PC34 40
#define DM1_V1_ENTRANCE_DOOR_ANIMATION_GRAPHIC_PC34 562
#define DM1_V1_ENTRANCE_SCREEN_GRAPHIC_PC34 563
#define DM1_V1_ENTRANCE_SCREEN_W_PC34 320
#define DM1_V1_ENTRANCE_SCREEN_H_PC34 200
#define DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_W_PC34 32
#define DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_H_PC34 29
#define DM1_V1_ENTRANCE_CHAMPION_PORTRAIT_ATLAS_COLS_PC34 8
#define DM1_V1_ENTRANCE_WALL_PORTRAIT_X_PC34 96
#define DM1_V1_ENTRANCE_WALL_PORTRAIT_Y_PC34 35
#define DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34 4

/* Entrance states */
typedef enum {
    DM1_ENTRANCE_IDLE = 0,        /* Not in entrance */
    DM1_ENTRANCE_DOOR_OPENING,    /* Door animation playing */
    DM1_ENTRANCE_VIEWING,         /* Viewing mirror hall */
    DM1_ENTRANCE_SELECTING,       /* Clicked on a mirror, viewing champion */
    DM1_ENTRANCE_RESURRECTING,    /* Resurrect dialog active */
    DM1_ENTRANCE_REINCARNATING,   /* Reincarnate dialog active */
    DM1_ENTRANCE_DONE             /* All champions selected, entering dungeon */
} DM1_V1_EntranceStatePc34;

typedef enum {
    DM1_V1_ENTRANCE_MENU_ROUTE_NONE_PC34 = 0,
    DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 = 1,
    DM1_V1_ENTRANCE_MENU_ROUTE_LIVE_CHAMPION_PC34 = 2,
    DM1_V1_ENTRANCE_MENU_ROUTE_DEAD_CHAMPION_PC34 = 3,
    DM1_V1_ENTRANCE_MENU_ROUTE_PARTY_FULL_PC34 = 4,
    DM1_V1_ENTRANCE_MENU_ROUTE_ENTER_DUNGEON_PC34 = 5
} DM1_V1_EntranceMenuRoutePc34;

typedef enum {
    DM1_V1_ENTRANCE_OVERLAY_NONE_PC34 = 0,
    DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 = 1,
    DM1_V1_ENTRANCE_OVERLAY_CHAMPION_PORTRAIT_PC34 = 2,
    DM1_V1_ENTRANCE_OVERLAY_RESURRECT_REINCARNATE_PC34 = 3,
    DM1_V1_ENTRANCE_OVERLAY_ENTER_DUNGEON_PC34 = 4
} DM1_V1_EntranceOverlayKindPc34;

typedef struct {
    int valid;
    DM1_V1_EntranceOverlayKindPc34 kind;
    int mirrorIndex;
    int championIndex;
    int mirrorMapX;
    int mirrorMapY;
    int mirrorFacing;
    int graphicIndex;
    int sourceX;
    int sourceY;
    int width;
    int height;
    int viewportX;
    int viewportY;
    int clearStalePanelFirst;
    int suppressThingPayloads;
    int blockEnterUntilChampionSelected;
    int realAssetCaptureProof;
    int requiresGraphicsDat;
    int noHostRenderInference;
    const char *reason;
} DM1_V1_EntranceRenderOverlayCommandPc34;

/* Mirror slot state */
typedef struct {
    int occupied;           /* 1 if a champion is in this mirror */
    int championIndex;      /* index into champion data, -1 if empty */
    int selected;           /* 1 if already recruited to party */
    int dead;               /* 1 if bones (needs resurrect/reincarnate) */
    int mapX;               /* mirror position X */
    int mapY;               /* mirror position Y */
    int facing;             /* direction mirror faces (0-3) */
} DM1_V1_MirrorSlotPc34;

/* Door animation state */
typedef struct {
    int animationStep;      /* current step (0..9 for 10 frames) */
    int totalSteps;         /* 10 in original */
    int frameDelayMs;       /* delay between frames */
    uint32_t lastFrameMs;   /* timestamp of last frame */
    int complete;           /* 1 when animation finished */
} DM1_V1_DoorAnimationPc34;

typedef struct {
    int valid;
    int mapIndex;
    int width;
    int height;
    int partyX;
    int partyY;
    int partyDirection;
    int drawFloorAndCeilingRequested;
    unsigned char squares[DM1_V1_ENTRANCE_MICRO_DUNGEON_SIZE_PC34];
    int corridorCount;
    int doorFrameIndex;
    int doorSourceStep;
    int drawDoorFrame;
    int playDoorRattleSound;
    int entranceMusicRequested;
    int realAssetCaptureProof;
    int entranceScreenGraphicIndex;
    int doorAnimationGraphicIndex;
    int doorFrameCount;
    int requiresGraphicsDat;
    int noHostRenderInference;
    const char *captureProofReason;
} DM1_V1_EntranceFullStartRenderReceiptPc34;

typedef struct {
    int handled;
    DM1_V1_EntranceMenuRoutePc34 route;
    DM1_V1_EntranceStatePc34 state;
    int selectedMirrorIndex;
    int selectedChampionIndex;
    int selectedMirrorDead;
    int selectedMirrorMapX;
    int selectedMirrorMapY;
    int selectedMirrorFacing;
    int partyChampionCount;
    int partyFull;
    int showHall;
    int showChampionPanel;
    int showResurrectReincarnateChoices;
    int canRecruit;
    int canResurrect;
    int canReincarnate;
    int canCancelSelection;
    int canEnterDungeon;
    int renderHallMirrorOverlay;
    int renderChampionMirrorOverlay;
    int renderResurrectReincarnateOverlay;
    int renderEnterDungeonOverlay;
    int clearStaleChampionMirrorOverlay;
    int blockEnterUntilChampionSelected;
    int renderOverlayCommandCount;
    DM1_V1_EntranceRenderOverlayCommandPc34
        renderOverlayCommands[DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34];
    int needsRedraw;
    const char *reason;
} DM1_V1_EntranceMenuRouteReceiptPc34;

/* Entrance persistent state */
typedef struct {
    DM1_V1_EntranceStatePc34 state;
    DM1_V1_MirrorSlotPc34 mirrors[DM1_V1_MAX_MIRROR_SLOTS_PC34];
    int mirrorCount;                    /* actual number of occupied mirrors */
    int selectedMirrorIndex;            /* currently viewed mirror, -1 if none */
    int partyChampionCount;             /* champions recruited so far */
    int partyChampionIndices[DM1_V1_MAX_CHAMPIONS_PC34]; /* recruited champion indices */
    DM1_V1_DoorAnimationPc34 doorAnim;
    int microDungeonBuilt;              /* F0797 micro dungeon constructed */
    uint32_t lastInteractionMs;
} DM1_V1_EntranceCtxPc34;

/* Result of an entrance tick */
typedef struct {
    int stateChanged;                   /* entrance state transitioned */
    DM1_V1_EntranceStatePc34 newState;
    int doorAnimationAdvanced;          /* door frame progressed */
    int doorAnimationComplete;
    int mirrorSelected;                 /* a mirror was clicked */
    int mirrorIndex;                    /* which mirror */
    int championRecruited;              /* a champion joined the party */
    int championIndex;                  /* which champion */
    int needsRedraw;                    /* screen needs refresh */
    int entranceComplete;               /* all done, enter dungeon */
} DM1_V1_EntranceTickResultPc34;

/*
 * Initialize entrance context. Call when entering the mirror hall.
 * mirrorData: array of mirror slot descriptions from dungeon data.
 */
void DM1_V1_Entrance_InitPc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Add a mirror slot (champion in a mirror).
 * Returns slot index or -1 if full.
 */
int DM1_V1_Entrance_AddMirrorPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, int championIndex,
                            int mapX, int mapY, int facing, int isDead);

/*
 * Start the door opening animation (F0438).
 */
void DM1_V1_Entrance_StartDoorAnimationPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, uint32_t nowMs);

/*
 * Tick the door animation. Returns 1 if a new frame is ready.
 */
int DM1_V1_Entrance_TickDoorAnimationPc34Compat(DM1_V1_EntranceCtxPc34 *ctx, uint32_t nowMs);

/*
 * Handle a click on a mirror (F0440/F0441).
 * mirrorIndex: which mirror was clicked.
 * Returns result with selection info.
 */
DM1_V1_EntranceTickResultPc34 DM1_V1_Entrance_ClickMirrorPc34Compat(DM1_V1_EntranceCtxPc34 *ctx,
                                                  int mirrorIndex,
                                                  uint32_t nowMs);

/* Source-bound C127 -> C026 entrance selection. The raw index-only helper
 * remains for non-PC34 tooling; original-data runtime input must use this
 * route so a live DUNGEON.DAT C127 sensor and matching GRAPHICS.DAT C026/C040
 * material are required before a champion panel can open. */
int DM1_V1_Entrance_ClickMirrorFromSourceReceiptPc34Compat(
    DM1_V1_EntranceCtxPc34 *ctx,
    int mirrorIndex,
    uint32_t nowMs,
    const DM1_V1_HocMirrorCandidateClickAdmissionReceiptPc34 *sourceReceipt,
    DM1_V1_EntranceTickResultPc34 *outResult);

/*
 * Recruit the currently selected champion to the party (F0280).
 * Returns 1 on success, 0 if party full or no champion selected.
 */
int DM1_V1_Entrance_RecruitChampionPc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Attempt resurrection of the selected dead champion.
 * Returns 1 on success, 0 if not applicable.
 */
int DM1_V1_Entrance_ResurrectPc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Attempt reincarnation of the selected dead champion.
 * Returns 1 on success, 0 if not applicable.
 */
int DM1_V1_Entrance_ReincarnatePc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Cancel current selection, return to viewing state.
 */
void DM1_V1_Entrance_CancelSelectionPc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Finalize entrance — mark all done, transition to dungeon.
 * Call when player has selected champions and walks past the hall.
 */
DM1_V1_EntranceTickResultPc34 DM1_V1_Entrance_FinalizePc34Compat(DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Query: is entrance complete?
 */
int DM1_V1_Entrance_IsCompletePc34Compat(const DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Get current party count in entrance.
 */
int DM1_V1_Entrance_GetPartyCountPc34Compat(const DM1_V1_EntranceCtxPc34 *ctx);

/*
 * Build the full-start graphics receipt for the entrance micro-dungeon and
 * currently visible door-animation frame.
 */
int DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(
    const DM1_V1_EntranceCtxPc34 *ctx,
    DM1_V1_EntranceFullStartRenderReceiptPc34 *outReceipt);

/*
 * Build the HoC champion-selection menu route for M11/M12 without host-side
 * state inference.
 */
int DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
    const DM1_V1_EntranceCtxPc34 *ctx,
    DM1_V1_EntranceMenuRouteReceiptPc34 *outReceipt);

/*
 * Source evidence string.
 */
const char *DM1_V1_Entrance_SourceEvidencePc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_ENTRANCE_CHAMPION_SELECT_PC34_COMPAT_H */
