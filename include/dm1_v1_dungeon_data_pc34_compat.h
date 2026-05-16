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
#include "dm1_v1_group_management_pc34_compat.h"
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
} M11_DD_PartyPos;

/* ── Champion slot (minimal — full state lives in champion modules) ─ */
#define M11_DD_MAX_CHAMPIONS   4

typedef struct {
    bool     alive;
    bool     recruited;
    int16_t  currentHP;
    int16_t  maxHP;
    uint16_t attributes;         /* packed flags from CHAMPION.C */
} M11_DD_ChampionSlot;

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
    M11_DD_PartyPos              party;
    M11_DD_ChampionSlot          champions[M11_DD_MAX_CHAMPIONS];
    int                          championCount; /* G0411 */

    /* ─ Creature groups ─ */
    M11_GroupState               groups;

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
} M11_DD_DungeonData;

/* ── Initialization / teardown ────────────────────────────────────── */

/*
 * Zero-initialize the entire store.  Does NOT load any files.
 */
void m11_dd_init(M11_DD_DungeonData *dd);

/*
 * Load dungeon from DUNGEON.DAT and populate the store.
 * Calls m11_dl_load_from_file internally, sets currentMapIndex to 0.
 * Returns true on success.
 */
bool m11_dd_load_dungeon(M11_DD_DungeonData *dd, const char *dungeon_dat_path);

/*
 * Load object tables from GRAPHICS.DAT metadata.
 * (Delegates to m11_ow_init.)
 */
bool m11_dd_load_objects(M11_DD_DungeonData *dd, const char *graphics_dat_path);

/*
 * Release any heap allocations in the store.
 */
void m11_dd_shutdown(M11_DD_DungeonData *dd);

/* ── Map access ───────────────────────────────────────────────────── */

/*
 * Switch to a new map level.  Updates currentMapIndex/Width/Height.
 * Returns true if the map index is valid.
 */
bool m11_dd_set_current_map(M11_DD_DungeonData *dd, int16_t mapIndex);

/*
 * Get tile at (level, x, y).  Returns NULL if out of bounds.
 */
const M11_DL_Tile *m11_dd_get_tile(const M11_DD_DungeonData *dd,
                                    uint8_t level, uint8_t x, uint8_t y);

/*
 * Get tile on the current map at (x, y).  Returns NULL if out of bounds.
 * For out-of-bounds, squareAheadElement is set to WALL per F0151.
 */
const M11_DL_Tile *m11_dd_get_current_tile(M11_DD_DungeonData *dd,
                                            int16_t x, int16_t y);

/* ── Party access ─────────────────────────────────────────────────── */

void m11_dd_set_party_pos(M11_DD_DungeonData *dd,
                           int16_t mapIndex, int16_t x, int16_t y,
                           uint8_t facing);

const M11_DD_PartyPos *m11_dd_get_party_pos(const M11_DD_DungeonData *dd);

void m11_dd_set_champion_count(M11_DD_DungeonData *dd, int count);

/* ── Time ─────────────────────────────────────────────────────────── */

/*
 * Advance game time by one tick.  Does NOT dispatch events.
 */
void m11_dd_advance_tick(M11_DD_DungeonData *dd);

/*
 * Get current game time.
 */
uint32_t m11_dd_get_game_time(const M11_DD_DungeonData *dd);

/* ── Event queue convenience (delegates to dm1_v1_event_timer) ────── */

int m11_dd_add_event(M11_DD_DungeonData *dd,
                     const struct DM1_Event_V1 *event);

bool m11_dd_has_expired_events(const M11_DD_DungeonData *dd);

/* ── Group convenience (delegates to dm1_v1_group_management) ────── */

M11_Group *m11_dd_get_group_at(M11_DD_DungeonData *dd,
                                int mapX, int mapY);

int m11_dd_active_group_count(const M11_DD_DungeonData *dd);

/* ── Source evidence ──────────────────────────────────────────────── */
const char *m11_dd_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_DUNGEON_DATA_PC34_COMPAT_H */
