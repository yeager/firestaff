/*
 * DM1 V1 Central Dungeon Data Store — implementation.
 *
 * Pure data aggregator with convenience accessors.  Delegates to
 * dm1_v1_dungeon_loader, dm1_v1_event_timer, and dm1_v1_object_world
 * for actual logic.
 *
 * Creature group plumbing (M11_GroupState / m11_group_*) was removed
 * on 2026-05-26: the orchestrator drives groups directly through
 * world->creatureAI[]/world->groups in GameWorld_Compat, and the
 * dungeon-data store was never wired into that path.  See the
 * commit message for full audit (dm1_v1_engine modules list stays
 * the central catalogue of in-use modules).
 *
 * Source lock: see header for ReDMCSB global variable references.
 */

#include "dm1_v1_dungeon_data_pc34_compat.h"
#include <string.h>

/* ── Initialization / teardown ────────────────────────────────────── */

void m11_dd_init(M11_DD_DungeonData *dd)
{
    memset(dd, 0, sizeof(*dd));
    m11_dl_init(&dd->dungeon);
    dm1v1_event_queue_init(&dd->events, 0);
    m11_ow_init(&dd->objects);
    dd->currentMapIndex = -1;
    dd->party.mapIndex  = -1;
    dd->party.facing    = 0;
}

bool m11_dd_load_dungeon(M11_DD_DungeonData *dd, const char *dungeon_dat_path)
{
    if (!dd || !dungeon_dat_path) return false;
    if (!m11_dl_load_from_file(&dd->dungeon, dungeon_dat_path)) return false;

    /* Default to first map */
    dd->currentMapIndex = 0;
    if (dd->dungeon.header.level_count > 0) {
        dd->currentMapWidth  = dd->dungeon.header.levels[0].width;
        dd->currentMapHeight = dd->dungeon.header.levels[0].height;
    }
    dd->loaded = true;
    return true;
}

bool m11_dd_load_objects(M11_DD_DungeonData *dd,
                         const char *graphics_dat_path)
{
    /* Object table load is a future integration point.
     * For now, just init the state so it is safe to query. */
    (void)graphics_dat_path;
    if (!dd) return false;
    m11_ow_init(&dd->objects);
    return true;
}

void m11_dd_shutdown(M11_DD_DungeonData *dd)
{
    if (!dd) return;
    m11_dl_cleanup(&dd->dungeon);
    dd->loaded = false;
}

/* ── Map access ───────────────────────────────────────────────────── */

bool m11_dd_set_current_map(M11_DD_DungeonData *dd, int16_t mapIndex)
{
    if (!dd || !dd->loaded) return false;
    if (mapIndex < 0 || mapIndex >= (int16_t)dd->dungeon.header.level_count)
        return false;

    dd->currentMapIndex  = mapIndex;
    dd->currentMapWidth  = dd->dungeon.header.levels[mapIndex].width;
    dd->currentMapHeight = dd->dungeon.header.levels[mapIndex].height;
    return true;
}

const M11_DL_Tile *m11_dd_get_tile(const M11_DD_DungeonData *dd,
                                    uint8_t level, uint8_t x, uint8_t y)
{
    if (!dd || !dd->loaded) return NULL;
    return m11_dl_get_tile(&dd->dungeon, level, x, y);
}

const M11_DL_Tile *m11_dd_get_current_tile(M11_DD_DungeonData *dd,
                                            int16_t x, int16_t y)
{
    if (!dd || !dd->loaded || dd->currentMapIndex < 0) return NULL;

    /* Out-of-bounds → WALL, matching F0151 behavior */
    if (x < 0 || y < 0 ||
        x >= dd->currentMapWidth || y >= dd->currentMapHeight) {
        dd->squareAheadElement = DM1_ELEMENT_WALL;
        return NULL;
    }
    return m11_dl_get_tile(&dd->dungeon,
                           (uint8_t)dd->currentMapIndex,
                           (uint8_t)x, (uint8_t)y);
}

/* ── Party access ─────────────────────────────────────────────────── */

void m11_dd_set_party_pos(M11_DD_DungeonData *dd,
                           int16_t mapIndex, int16_t x, int16_t y,
                           uint8_t facing)
{
    if (!dd) return;
    dd->party.mapIndex = mapIndex;
    dd->party.posX     = x;
    dd->party.posY     = y;
    dd->party.facing   = facing;
}

const M11_DD_PartyPos *m11_dd_get_party_pos(const M11_DD_DungeonData *dd)
{
    return dd ? &dd->party : NULL;
}

void m11_dd_set_champion_count(M11_DD_DungeonData *dd, int count)
{
    if (!dd) return;
    if (count < 0) count = 0;
    if (count > M11_DD_MAX_CHAMPIONS) count = M11_DD_MAX_CHAMPIONS;
    dd->championCount = count;
}

/* ── Time ─────────────────────────────────────────────────────────── */

void m11_dd_advance_tick(M11_DD_DungeonData *dd)
{
    if (!dd) return;
    dd->gameTime++;
    dm1v1_event_advance_tick(&dd->events);
}

uint32_t m11_dd_get_game_time(const M11_DD_DungeonData *dd)
{
    return dd ? dd->gameTime : 0;
}

/* ── Event queue convenience ──────────────────────────────────────── */

int m11_dd_add_event(M11_DD_DungeonData *dd,
                     const struct DM1_Event_V1 *event)
{
    if (!dd || !event) return -1;
    return dm1v1_event_add(&dd->events, event);
}

bool m11_dd_has_expired_events(const M11_DD_DungeonData *dd)
{
    if (!dd) return false;
    return dm1v1_event_is_first_expired(&dd->events);
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *m11_dd_source_evidence(void)
{
    return
        "DM1 V1 Central Dungeon Data Store\n"
        "Aggregates: DUNGEON.C (G0271,G0273,G0274,G0283,G0285,G0286), "
        "TIMELINE.C (G0370-G0373), "
        "GAMELOOP.C (G0310,G0303), CHAMPION.C (G0410,G0411)\n"
        "Source: ReDMCSB WIP20210206 Toolchains/Common/Source/\n";
}
