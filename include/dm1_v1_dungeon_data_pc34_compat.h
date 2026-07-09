#ifndef FIRESTAFF_DM1_V1_DUNGEON_DATA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_DATA_PC34_COMPAT_H

/*
 * DM1 V1 Central Dungeon Data Store — pc34 compat layer.
 *
 * Single shared state that holds the current level map, creature group
 * list, object/thing data, and event timer queue.  All other dm1_v1_*
 * modules read/write through this store instead of keeping private
 * copies.
 *
 * The struct mirrors the ReDMCSB global variable layout:
 *
 *   DUNGEON.C globals:
 *     G0271_i_CurrentMapIndex, G0273_i_CurrentMapWidth,
 *     G0274_i_CurrentMapHeight, G0283_pT_SquareFirstThings,
 *     G0284_apuc_ThingData[16], G0285_i_SquareAheadElement,
 *     G0286_B_FacingAlcove
 *
 *   GROUP.C globals:
 *     G0375_as_ActiveGroups, G0376_i_MaximumActiveGroupCount,
 *     G0377_i_ActiveGroupCount
 *
 *   TIMELINE.C globals:
 *     G0370_ps_Events, G0371_pui_Timeline,
 *     G0372_ui_EventCount, G0373_ui_FirstUnusedEventIndex
 *
 *   GAMELOOP.C:
 *     G0310_ul_GameTime, G0303_i_PartyDeath
 *
 *   CHAMPION.C:
 *     G0410_apc_Champions[4], G0411_i_PartyChampionCount
 *
 * The store does NOT own subsystem logic — it is pure data with
 * accessors.  Each subsystem (movement, rendering, AI, etc.) operates
 * on pointers/references obtained from this store.
 *
 * Source lock (ReDMCSB WIP20210206, Toolchains/Common/Source):
 *   DUNGEON.C, GROUP.C, TIMELINE.C, GAMELOOP.C, CHAMPION.C
 */

#include <stdint.h>
#include <stdbool.h>

#include "dm1_v1_dungeon_loader_pc34_compat.h"
#include "dm1_v1_dungeon_square_structs_pc34_compat.h"
#include "dm1_v1_event_timer_pc34_compat.h"
#include "dm1_v1_object_world_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Party position (from DUNGEON.C / GAMELOOP.C) ────────────────── */
typedef struct {
    int16_t  mapIndex;           /* G0271_i_CurrentMapIndex */
    int16_t  posX;               /* G0306_i_PartyMapX */
    int16_t  posY;               /* G0307_i_PartyMapY */
    uint8_t  facing;             /* G0308_i_PartyDirection (0=N,1=E,2=S,3=W) */
} DM1_V1_DungeonDataPartyPosPc34;

/* ── Champion slot (minimal — full state lives in champion modules) ─ */
#define DM1_V1_DUNGEON_DATA_MAX_CHAMPIONS_PC34   4

typedef struct {
    bool     alive;
    bool     recruited;
    int16_t  currentHP;
    int16_t  maxHP;
    uint16_t attributes;         /* packed flags from CHAMPION.C */
} DM1_V1_DungeonDataChampionSlotPc34;

/* ── Central dungeon data store ───────────────────────────────────── */
typedef struct {
    /* ─ Dungeon structure (from dungeon_loader) ─ */
    M11_DL_DungeonState          dungeon;

    /* ─ Current map metadata ─ */
    int16_t  currentMapIndex;     /* G0271 */
    int16_t  currentMapWidth;     /* G0273 */
    int16_t  currentMapHeight;    /* G0274 */
    int16_t  squareAheadElement;  /* G0285 */
    bool     facingAlcove;        /* G0286 */

    /* ─ Party ─ */
    DM1_V1_DungeonDataPartyPosPc34              party;
    DM1_V1_DungeonDataChampionSlotPc34          champions[DM1_V1_DUNGEON_DATA_MAX_CHAMPIONS_PC34];
    int                          championCount; /* G0411 */

    /* ─ Event queue / timeline ─ */
    struct DM1_EventQueue_V1     events;

    /* ─ Object tables (shared read-only after load) ─ */
    M11_OW_WorldState                 objects;

    /* ─ Global time ─ */
    uint32_t  gameTime;           /* G0310_ul_GameTime — ticks since start */
    uint32_t  realTimeMs;         /* Wall-clock ms at last tick */

    /* ─ Game flags ─ */
    bool     partyDead;           /* G0303 */
    bool     newGame;             /* G0298 */
    bool     gameWon;             /* G0302 */

    /* ─ Initialization gate ─ */
    bool     loaded;              /* True after successful init */
} DM1_V1_DungeonDataPc34;

/* ── Initialization / teardown ────────────────────────────────────── */

/*
 * Zero-initialize the entire store.  Does NOT load any files.
 */
void DM1_V1_DungeonData_InitPc34Compat(DM1_V1_DungeonDataPc34 *dd);

/*
 * Load dungeon from DUNGEON.DAT and populate the store.
 * Calls m11_dl_load_from_file internally, sets currentMapIndex to 0.
 * Returns true on success.
 */
bool DM1_V1_DungeonData_LoadDungeonPc34Compat(DM1_V1_DungeonDataPc34 *dd, const char *dungeon_dat_path);

/*
 * Load object tables from GRAPHICS.DAT metadata.
 * (Delegates to m11_ow_init.)
 */
bool DM1_V1_DungeonData_LoadObjectsPc34Compat(DM1_V1_DungeonDataPc34 *dd, const char *graphics_dat_path);

/*
 * Release any heap allocations in the store.
 */
void DM1_V1_DungeonData_ShutdownPc34Compat(DM1_V1_DungeonDataPc34 *dd);

/* ── Map access ───────────────────────────────────────────────────── */

/*
 * Switch to a new map level.  Updates currentMapIndex/Width/Height.
 * Returns true if the map index is valid.
 */
bool DM1_V1_DungeonData_SetCurrentMapPc34Compat(DM1_V1_DungeonDataPc34 *dd, int16_t mapIndex);

/*
 * Get tile at (level, x, y).  Returns NULL if out of bounds.
 */
const M11_DL_Tile *DM1_V1_DungeonData_GetTilePc34Compat(const DM1_V1_DungeonDataPc34 *dd,
                                    uint8_t level, uint8_t x, uint8_t y);

/*
 * Get tile on the current map at (x, y).  Returns NULL if out of bounds.
 * For out-of-bounds, squareAheadElement is set to WALL per F0151.
 */
const M11_DL_Tile *DM1_V1_DungeonData_GetCurrentTilePc34Compat(DM1_V1_DungeonDataPc34 *dd,
                                            int16_t x, int16_t y);

/* ── Party access ─────────────────────────────────────────────────── */

void DM1_V1_DungeonData_SetPartyPosPc34Compat(DM1_V1_DungeonDataPc34 *dd,
                           int16_t mapIndex, int16_t x, int16_t y,
                           uint8_t facing);

const DM1_V1_DungeonDataPartyPosPc34 *DM1_V1_DungeonData_GetPartyPosPc34Compat(const DM1_V1_DungeonDataPc34 *dd);

void DM1_V1_DungeonData_SetChampionCountPc34Compat(DM1_V1_DungeonDataPc34 *dd, int count);

/* ── Time ─────────────────────────────────────────────────────────── */

/*
 * Advance game time by one tick.  Does NOT dispatch events.
 */
void DM1_V1_DungeonData_AdvanceTickPc34Compat(DM1_V1_DungeonDataPc34 *dd);

/*
 * Get current game time.
 */
uint32_t DM1_V1_DungeonData_GetGameTimePc34Compat(const DM1_V1_DungeonDataPc34 *dd);

/* ── Event queue convenience (delegates to dm1_v1_event_timer) ────── */

int DM1_V1_DungeonData_AddEventPc34Compat(DM1_V1_DungeonDataPc34 *dd,
                     const struct DM1_Event_V1 *event);

bool DM1_V1_DungeonData_HasExpiredEventsPc34Compat(const DM1_V1_DungeonDataPc34 *dd);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *DM1_V1_DungeonData_SourceEvidencePc34Compat(void);

typedef DM1_V1_DungeonDataPartyPosPc34 M11_DD_PartyPos;
typedef DM1_V1_DungeonDataChampionSlotPc34 M11_DD_ChampionSlot;
typedef DM1_V1_DungeonDataPc34 M11_DD_DungeonData;

#define M11_DD_MAX_CHAMPIONS DM1_V1_DUNGEON_DATA_MAX_CHAMPIONS_PC34

#define m11_dd_init DM1_V1_DungeonData_InitPc34Compat
#define m11_dd_load_dungeon DM1_V1_DungeonData_LoadDungeonPc34Compat
#define m11_dd_load_objects DM1_V1_DungeonData_LoadObjectsPc34Compat
#define m11_dd_shutdown DM1_V1_DungeonData_ShutdownPc34Compat
#define m11_dd_set_current_map DM1_V1_DungeonData_SetCurrentMapPc34Compat
#define m11_dd_get_tile DM1_V1_DungeonData_GetTilePc34Compat
#define m11_dd_get_current_tile DM1_V1_DungeonData_GetCurrentTilePc34Compat
#define m11_dd_set_party_pos DM1_V1_DungeonData_SetPartyPosPc34Compat
#define m11_dd_get_party_pos DM1_V1_DungeonData_GetPartyPosPc34Compat
#define m11_dd_set_champion_count DM1_V1_DungeonData_SetChampionCountPc34Compat
#define m11_dd_advance_tick DM1_V1_DungeonData_AdvanceTickPc34Compat
#define m11_dd_get_game_time DM1_V1_DungeonData_GetGameTimePc34Compat
#define m11_dd_add_event DM1_V1_DungeonData_AddEventPc34Compat
#define m11_dd_has_expired_events DM1_V1_DungeonData_HasExpiredEventsPc34Compat
#define m11_dd_source_evidence DM1_V1_DungeonData_SourceEvidencePc34Compat

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DUNGEON_DATA_PC34_COMPAT_H */
