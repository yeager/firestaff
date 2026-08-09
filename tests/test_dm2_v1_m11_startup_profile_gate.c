/*
 * test_dm2_v1_m11_startup_profile_gate.c -- DM2 V1 startup/profile M11 gate.
 *
 * Skip-safe real-data gate. Without local hash-verified DM2 data it exits 0;
 * with verified data it proves that M11_GameView_Start reaches the intended
 * DM2 V1 runtime boundary without claiming full playability.
 *
 * Source-lock: SKULL.ASM T520 (party placement after load), T560
 * (DUNGEON.DAT load completion). Firestaff boundary under test:
 * dm2_v1_boot_enter_game() -> M11_GAME_SOURCE_DM2_BOOT.
 */

#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "dm1_v1_inventory_slot_placement_pc34_compat.h"
#include "dm1_v1_layout_zones_pc34_compat.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_boot_startup_view_model.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_game.h"
#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_session_fixture.h"
#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_startup_layout.h"
#include "dm2_v1_startup_menu.h"
#include "dm2_v1_startup_presentation.h"
#include "dm2_v1_tech_magic.h"
#include "dm2_v1_trigger.h"
#include "dm2_v2_runtime.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#endif

static void alarm_handler(int sig) {
    (void)sig;
    const char msg[] = "\nTIMEOUT: DM2 startup profile gate test exceeded 20s (SDL blocked?)\n";
    (void)write(STDERR_FILENO, msg, sizeof(msg) - 1);
    /* A watchdog expiry is never a skipped test.  Returning success hid an
     * incomplete real-data gate from CTest and could promote a hung M11 path
     * as verified. */
    _exit(124);
}


unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;

static uint32_t dm2_test_fnv1a(const uint8_t *bytes, size_t byte_count) {
    uint32_t hash = 2166136261u;
    size_t i;
    if (!bytes || byte_count == 0u) return 0u;
    for (i = 0u; i < byte_count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static int dm2_test_preselection_view_matches_owner(
    const DM2_V1_GameLoadWorldOwner *owner)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct {
        uint8_t view_square;
        int8_t forward;
        int8_t lateral;
    } expected[DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT] = {
        { DM2_SQ_D0C, 1,  0 }, { DM2_SQ_D1C, 2,  0 },
        { DM2_SQ_D2C, 3,  0 }, { DM2_SQ_D0L, 0, -1 },
        { DM2_SQ_D0R, 0,  1 }, { DM2_SQ_D1L, 1, -1 },
        { DM2_SQ_D1R, 1,  1 }, { DM2_SQ_D2L, 2, -1 },
        { DM2_SQ_D2R, 2,  1 }, { DM2_SQ_D3L, 5, -2 },
        { DM2_SQ_D3R, 5,  2 },
    };
    const DM2_V1_GameLoadPreselectionViewReceipt *view;
    int direction;
    int i;

    if (!owner || !owner->preselection_view.valid ||
        owner->preselection_view.map != owner->source_party_map ||
        owner->preselection_view.party_x != owner->source_party_x ||
        owner->preselection_view.party_y != owner->source_party_y ||
        owner->preselection_view.party_direction != owner->source_party_direction ||
        owner->preselection_view.cell_count !=
            DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT ||
        owner->preselection_view.source_view_hash == 0u) {
        return 0;
    }
    direction = owner->source_party_direction;
    view = &owner->preselection_view;
    for (i = 0; i < DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT; ++i) {
        const int x = (int)owner->source_party_x +
            dx[direction] * expected[i].forward -
            dy[direction] * expected[i].lateral;
        const int y = (int)owner->source_party_y +
            dy[direction] * expected[i].forward +
            dx[direction] * expected[i].lateral;
        const int raw = dm2_v1_dungeon_get_tile_raw(&owner->dungeon,
            owner->source_party_map, x, y);
        const int type = dm2_v1_dungeon_get_square_type(&owner->dungeon,
            owner->source_party_map, x, y);
        const int root = dm2_v1_dungeon_get_first_thing(&owner->dungeon,
            owner->source_party_map, x, y);
        if (view->cells[i].view_square != expected[i].view_square ||
            view->cells[i].map_x != x || view->cells[i].map_y != y) {
            return 0;
        }
        if (raw < 0 || type < 0 || root < INT16_MIN || root > UINT16_MAX) {
            if (view->cells[i].source_available != 0u ||
                view->cells[i].raw_tile != 0u ||
                view->cells[i].square_type != 0u ||
                view->cells[i].ground_stack_root != 0u) return 0;
            continue;
        }
        if (view->cells[i].source_available != 1u ||
            view->cells[i].raw_tile != (uint16_t)raw ||
            view->cells[i].square_type != (uint8_t)type ||
            view->cells[i].ground_stack_root != (uint16_t)root) {
            return 0;
        }
    }
    return 1;
}

/* Select an existing File_header square from verified DUNGEON.DAT.  The
 * message below is a test transport for that authentic coordinate, not a
 * replacement map, record or timer corpus. */
static int dm2_test_find_file_header_tile_class(const DM2_V1_DungeonData *d,
                                                int wanted_class,
                                                int *out_map,
                                                int *out_x,
                                                int *out_y)
{
    int map;
    if (!d || !out_map || !out_x || !out_y || wanted_class < 0 ||
        wanted_class > 7) return 0;
    for (map = 0; map < d->level_count; ++map) {
        int x;
        for (x = 0; x < d->level_widths[map]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[map]; ++y) {
                const int raw = dm2_v1_dungeon_get_tile_raw(d, map, x, y);
                if (raw >= 0 && ((raw >> 5) & 7) == wanted_class) {
                    *out_map = map;
                    *out_x = x;
                    *out_y = y;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Find an authentic class-1 square whose complete source chain is DB2-only
 * and does not require the missing DISPLAY_HINT_TEXT owner.  This mirrors the
 * deliberately narrow private FLOOR atom rather than inventing a test map. */
static int dm2_test_find_private_db2_floor(
    const DM2_V1_GameLoadWorldOwner *owner,
    int tile_class,
    int *out_map, int *out_x, int *out_y, int16_t *out_link)
{
    int map;

    if (!owner || tile_class < 1 || tile_class > 5 || !out_map || !out_x ||
        !out_y || !out_link) return 0;
    for (map = 0; map < owner->dungeon.level_count; ++map) {
        int x;
        for (x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            int y;
            for (y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                int16_t link;
                int count = 0;
                int max_count;
                int acceptable = 1;
                int has_mutable_text = 0;
                if (dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y) < 0 ||
                    ((dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y) >> 5) & 7) != tile_class)
                    continue;
                link = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &owner->dungeon, map, x, y);
                if (link == DM2_V1_RECORD_HANDLE_END ||
                    link == DM2_V1_RECORD_HANDLE_NULL) continue;
                max_count = owner->record_pools.pools[2].record_count +
                            owner->record_pools.pools[2].extension_count;
                while (link != DM2_V1_RECORD_HANDLE_END && acceptable) {
                    const uint8_t *record;
                    int16_t next;
                    uint16_t w2;
                    uint16_t text_index;
                    uint8_t mode;
                    if (link == DM2_V1_RECORD_HANDLE_NULL || count >= max_count ||
                        dm2_v1_record_handle_pool(link) != 2) {
                        acceptable = 0;
                        break;
                    }
                    record = dm2_v1_record_pool_address(&owner->record_pools, link);
                    if (!record || !dm2_v1_record_pool_next_link(
                            &owner->record_pools, link, &next)) {
                        acceptable = 0;
                        break;
                    }
                    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                    text_index = (uint16_t)((w2 >> 3) & 0x1fffu);
                    mode = (uint8_t)((w2 >> 1) & 3u);
                    if (mode == 0u ||
                        (mode == 1u && ((text_index >> 8) & 0x1fu) == 5u) ||
                        (mode == 2u && ((text_index >> 9) & 0x0fu) == 2u)) {
                        if (mode == 0u && (w2 & 1u) == 0u &&
                            map == owner->source_party_map &&
                            x == (int)owner->source_party_x &&
                            y == (int)owner->source_party_y) {
                            acceptable = 0;
                            break;
                        }
                        has_mutable_text = 1;
                    }
                    ++count;
                    link = next;
                }
                if (acceptable && count > 0 && has_mutable_text &&
                    link == DM2_V1_RECORD_HANDLE_END) {
                    *out_map = map;
                    *out_x = x;
                    *out_y = y;
                    *out_link = (int16_t)dm2_v1_dungeon_get_first_thing(
                        &owner->dungeon, map, x, y);
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Select an actual File_header door which can take one private animation
 * step without requiring the still-unowned party/CAII collision paths.  The
 * timer below carries this exact DB0 root; it is only a test transport, not
 * a replacement door or queue record. */
static int dm2_test_find_private_door_step(
    const DM2_V1_GameLoadWorldOwner *owner,
    int *out_map, int *out_x, int *out_y, int16_t *out_link,
    uint8_t *out_direction)
{
    int map;
    int chain_limit = 0;
    int db;

    if (!owner || !out_map || !out_x || !out_y || !out_link ||
        !out_direction) return 0;
    for (db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0) return 0;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return 0;
    for (map = 0; map < owner->dungeon.level_count; ++map) {
        int x;
        for (x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            int y;
            for (y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                const int raw = dm2_v1_dungeon_get_tile_raw(
                    &owner->dungeon, map, x, y);
                int16_t link;
                int count = 0;
                int has_creature = 0;
                int16_t next;
                if (raw < 0 || ((raw >> 5) & 7) != 4 ||
                    (raw & 7) < 1 || (raw & 7) > 4 ||
                    (map == owner->source_party_map &&
                     x == (int)owner->source_party_x &&
                     y == (int)owner->source_party_y)) continue;
                link = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &owner->dungeon, map, x, y);
                if (link == DM2_V1_RECORD_HANDLE_NULL ||
                    link == DM2_V1_RECORD_HANDLE_END ||
                    dm2_v1_record_handle_pool(link) != 0) continue;
                do {
                    if (link == DM2_V1_RECORD_HANDLE_NULL ||
                        count >= chain_limit ||
                        !dm2_v1_record_pool_next_link(&owner->record_pools,
                                                      link, &next)) {
                        has_creature = 1;
                        break;
                    }
                    if (dm2_v1_record_handle_pool(link) == 4) {
                        has_creature = 1;
                        break;
                    }
                    ++count;
                    link = next;
                } while (link != DM2_V1_RECORD_HANDLE_END);
                if (!has_creature && count > 0 &&
                    link == DM2_V1_RECORD_HANDLE_END) {
                    *out_map = map;
                    *out_x = x;
                    *out_y = y;
                    *out_link = (int16_t)dm2_v1_dungeon_get_first_thing(
                        &owner->dungeon, map, x, y);
                    /* The source OPEN direction turns a closed File_header
                     * door through state 3, so it proves that the private
                     * atom emits a follow-up timer without rewriting data. */
                    *out_direction = 0u;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Find a source-complete 0x04/WALL-or-FLOOR PUSH_BUTTON_SWITCH chain. The private
 * owner admits it only when every member is a DB3 0x46 actuator and every
 * target is the source's direct DB0 door.  No fixture tile, record or door
 * is manufactured merely to make this atom testable. */
static int dm2_test_find_private_push_button_floor(
    const DM2_V1_GameLoadWorldOwner *owner,
    int *out_map, int *out_x, int *out_y)
{
    int chain_limit = 0;

    if (!owner || !out_map || !out_x || !out_y) return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0) return 0;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return 0;
    for (int map = 0; map < owner->dungeon.level_count; ++map) {
        for (int x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            for (int y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                int16_t link;
                int count = 0;
                int valid = 1;

                const int tile_class =
                    dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y) >> 5;
                if (tile_class != 0 && tile_class != 1)
                    continue;
                link = (int16_t)dm2_v1_dungeon_get_first_thing(
                    &owner->dungeon, map, x, y);
                while (link != DM2_V1_RECORD_HANDLE_END && valid) {
                    const uint8_t *record;
                    int16_t next;
                    int tx, ty;
                    int target;

                    if (link == DM2_V1_RECORD_HANDLE_NULL || count >= chain_limit ||
                        dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
                        !(record = dm2_v1_record_pool_address(
                            &owner->record_pools, link)) ||
                        dm2_actu_type(record) != DM2_ACTU_PUSH_BUTTON_SWITCH ||
                        !dm2_v1_record_pool_next_link(&owner->record_pools,
                                                       link, &next)) {
                        valid = 0;
                        break;
                    }
                    tx = (int)dm2_actu_xcoord(record);
                    ty = (int)dm2_actu_ycoord(record);
                    target = dm2_v1_dungeon_get_first_thing(
                        &owner->dungeon, map, tx, ty);
                    if (tx >= owner->dungeon.level_widths[map] ||
                        ty >= owner->dungeon.level_heights[map] || target < 0 ||
                        dm2_v1_record_handle_pool((int16_t)target) != DM2_DB_DOOR) {
                        valid = 0;
                        break;
                    }
                    ++count;
                    link = next;
                }
                if (valid && count > 0 && link == DM2_V1_RECORD_HANDLE_END) {
                    *out_map = map; *out_x = x; *out_y = y;
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Find a real WALL_MECHA chain whose direction-matching DB3 records are all
 * CROSS_MAP.  Unmatched directional records are source-ignored.  No target
 * map, timer or actuator is manufactured: this only selects a complete
 * File_header chain the private owner can queue atomically. */
static int dm2_test_find_private_cross_map_wall(
    const DM2_V1_GameLoadWorldOwner *owner, int *out_map, int *out_x,
    int *out_y, uint8_t *out_direction, int *out_target_map,
    int *out_target_x, int *out_target_y, uint8_t *out_target_direction,
    int *out_count)
{
    int chain_limit = 0;

    if (!owner || !out_map || !out_x || !out_y || !out_direction ||
        !out_target_map || !out_target_x || !out_target_y ||
        !out_target_direction || !out_count) return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count) {
            return 0;
        }
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return 0;
    for (int map = 0; map < owner->dungeon.level_count; ++map) {
        for (int x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            for (int y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                if ((dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y) >> 5) != 0)
                    continue;
                for (uint8_t direction = 0u; direction < 4u; ++direction) {
                    int16_t link = (int16_t)dm2_v1_dungeon_get_first_thing(
                        &owner->dungeon, map, x, y);
                    int steps = 0;
                    int count = 0;
                    int valid = 1;
                    int first_map = -1;
                    int first_x = -1;
                    int first_y = -1;
                    uint8_t first_dir = 0u;

                    while (link != DM2_V1_RECORD_HANDLE_END && valid) {
                        const uint8_t *record;
                        int16_t next;
                        uint16_t data;
                        int target_map;
                        int target_x;
                        int target_y;
                        if (link == DM2_V1_RECORD_HANDLE_NULL ||
                            steps >= chain_limit ||
                            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
                            !(record = dm2_v1_record_pool_address(
                                &owner->record_pools, link)) ||
                            !dm2_v1_record_pool_next_link(&owner->record_pools,
                                                           link, &next)) {
                            valid = 0;
                            break;
                        }
                        if ((uint8_t)(link & 3) == direction) {
                            data = dm2_actu_data(record);
                            target_map = (int)(data & 0x003fu);
                            target_x = (int)dm2_actu_xcoord(record);
                            target_y = (int)dm2_actu_ycoord(record);
                            if (dm2_actu_type(record) != DM2_ACTU_CROSS_MAP ||
                                target_map < 0 ||
                                target_map >= owner->dungeon.level_count ||
                                target_x < 0 || target_y < 0 ||
                                target_x >= owner->dungeon.level_widths[target_map] ||
                                target_y >= owner->dungeon.level_heights[target_map]) {
                                valid = 0;
                                break;
                            }
                            if (count == 0) {
                                first_map = target_map;
                                first_x = target_x;
                                first_y = target_y;
                                first_dir = (uint8_t)((data >> 6) & 3u);
                            }
                            ++count;
                        }
                        ++steps;
                        link = next;
                    }
                    if (valid && link == DM2_V1_RECORD_HANDLE_END && count > 0) {
                        *out_map = map; *out_x = x; *out_y = y;
                        *out_direction = direction;
                        *out_target_map = first_map;
                        *out_target_x = first_x;
                        *out_target_y = first_y;
                        *out_target_direction = first_dir;
                        *out_count = count;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* The source ignores directional DB3 records that do not face the incoming
 * action.  This chooses only a complete real chain whose matching records
 * are COUNTERs; it never manufactures a tile, record or continuation. */
static int dm2_test_find_private_counter_chain(
    const DM2_V1_GameLoadWorldOwner *owner, int *out_map, int *out_x,
    int *out_y, uint8_t *out_direction, int *out_count)
{
    int chain_limit = 0;

    if (!owner || !out_map || !out_x || !out_y || !out_direction ||
        !out_count) return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return 0;
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) return 0;
    for (int map = 0; map < owner->dungeon.level_count; ++map) {
        for (int x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            for (int y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                const int tile_class = dm2_v1_dungeon_get_tile_raw(
                    &owner->dungeon, map, x, y) >> 5;
                if (tile_class != 0 && tile_class != 1) continue;
                for (uint8_t direction = 0u; direction < 4u; ++direction) {
                    int16_t link = (int16_t)dm2_v1_dungeon_get_first_thing(
                        &owner->dungeon, map, x, y);
                    int steps = 0;
                    int count = 0;
                    int valid = 1;

                    while (link != DM2_V1_RECORD_HANDLE_END && valid) {
                        const uint8_t *record;
                        int16_t next;
                        if (link == DM2_V1_RECORD_HANDLE_NULL ||
                            steps >= chain_limit ||
                            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
                            !(record = dm2_v1_record_pool_address(
                                &owner->record_pools, link)) ||
                            !dm2_v1_record_pool_next_link(&owner->record_pools,
                                                           link, &next)) {
                            valid = 0;
                            break;
                        }
                        if ((uint8_t)(link & 3) == direction) {
                            if (dm2_actu_type(record) != DM2_ACTU_COUNTER) {
                                valid = 0;
                                break;
                            }
                            ++count;
                        }
                        ++steps;
                        link = next;
                    }
                    if (valid && link == DM2_V1_RECORD_HANDLE_END && count > 0) {
                        *out_map = map; *out_x = x; *out_y = y;
                        *out_direction = direction; *out_count = count;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

static int dm2_test_find_private_relay_chain(
    const DM2_V1_GameLoadWorldOwner *owner, int *out_map, int *out_x,
    int *out_y, uint8_t *out_direction, uint8_t *out_action,
    int *out_count, int *out_messages)
{
    int chain_limit = 0;
    if (!owner || !out_map || !out_x || !out_y || !out_direction ||
        !out_action || !out_count || !out_messages) return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &owner->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count)
            return 0;
        chain_limit += pool->record_count + pool->extension_count;
    }
    for (int map = 0; map < owner->dungeon.level_count; ++map) {
        for (int x = 0; x < owner->dungeon.level_widths[map]; ++x) {
            for (int y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                const int tile_class = dm2_v1_dungeon_get_tile_raw(
                    &owner->dungeon, map, x, y) >> 5;
                if (tile_class != 0 && tile_class != 1) continue;
                for (uint8_t direction = 0u; direction < 4u; ++direction) {
                    for (uint8_t incoming = 0u; incoming < 3u; ++incoming) {
                        int16_t link = (int16_t)dm2_v1_dungeon_get_first_thing(
                            &owner->dungeon, map, x, y);
                        int steps = 0, count = 0, messages = 0, valid = 1;
                        while (link != DM2_V1_RECORD_HANDLE_END && valid) {
                            const uint8_t *record;
                            int16_t next;
                            if (link == DM2_V1_RECORD_HANDLE_NULL ||
                                steps >= chain_limit ||
                                dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
                                !(record = dm2_v1_record_pool_address(
                                    &owner->record_pools, link)) ||
                                !dm2_v1_record_pool_next_link(&owner->record_pools,
                                                               link, &next)) { valid = 0; break; }
                            if ((uint8_t)(link & 3) == direction) {
                                const uint8_t type = dm2_actu_type(record);
                                const int gated = dm2_actu_once_only(record) != 0u &&
                                    (dm2_actu_revert_effect(record) != 0u || incoming != 0u) &&
                                    (dm2_actu_revert_effect(record) == 0u || incoming != 1u);
                                const uint8_t action = dm2_actu_once_only(record) ?
                                    dm2_actu_action_type(record) : incoming;
                                if ((type != DM2_ACTU_RELAY_1 && type != DM2_ACTU_RELAY_3) ||
                                    (!gated && (action > 2u ||
                                     dm2_actu_xcoord(record) >= owner->dungeon.level_widths[map] ||
                                     dm2_actu_ycoord(record) >= owner->dungeon.level_heights[map]))) {
                                    valid = 0; break;
                                }
                                ++count;
                                if (!gated) ++messages;
                            }
                            ++steps; link = next;
                        }
                        if (valid && link == DM2_V1_RECORD_HANDLE_END &&
                            count > 0 && messages > 0) {
                            *out_map = map; *out_x = x; *out_y = y;
                            *out_direction = direction; *out_action = incoming;
                            *out_count = count; *out_messages = messages;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

static int dm2_test_find_private_finite_relay_chain(
    const DM2_V1_GameLoadWorldOwner *owner, int *out_map, int *out_x,
    int *out_y, uint8_t *out_direction, int *out_count)
{
    int limit = 0;
    if (!owner || !out_map || !out_x || !out_y || !out_direction || !out_count) return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db)
        limit += owner->record_pools.pools[db].record_count + owner->record_pools.pools[db].extension_count;
    if (limit <= 0) return 0;
    for (int map = 0; map < owner->dungeon.level_count; ++map)
        for (int x = 0; x < owner->dungeon.level_widths[map]; ++x)
            for (int y = 0; y < owner->dungeon.level_heights[map]; ++y) {
                const int klass = dm2_v1_dungeon_get_tile_raw(&owner->dungeon, map, x, y) >> 5;
                if (klass != 0 && klass != 1) continue;
                for (uint8_t direction = 0u; direction < 4u; ++direction) {
                    int16_t link = (int16_t)dm2_v1_dungeon_get_first_thing(&owner->dungeon, map, x, y);
                    int count = 0, steps = 0, valid = 1;
                    while (link != DM2_V1_RECORD_HANDLE_END && valid) {
                        const uint8_t *record; int16_t next;
                        if (link == DM2_V1_RECORD_HANDLE_NULL || steps >= limit ||
                            dm2_v1_record_handle_pool(link) != DM2_DB_ACTUATOR ||
                            !(record = dm2_v1_record_pool_address(&owner->record_pools, link)) ||
                            !dm2_v1_record_pool_next_link(&owner->record_pools, link, &next)) { valid = 0; break; }
                        if ((uint8_t)(link & 3) == direction) {
                            if (dm2_actu_type(record) != DM2_ACTU_FINITE_RELAY ||
                                dm2_actu_data(record) == 0u || dm2_actu_data(record) > 400u ||
                                dm2_actu_xcoord(record) >= owner->dungeon.level_widths[map] ||
                                dm2_actu_ycoord(record) >= owner->dungeon.level_heights[map]) { valid = 0; break; }
                            ++count;
                        }
                        ++steps; link = next;
                    }
                    if (valid && link == DM2_V1_RECORD_HANDLE_END && count > 0) {
                        *out_map = map; *out_x = x; *out_y = y; *out_direction = direction; *out_count = count; return 1;
                    }
                }
            }
    return 0;
}

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

typedef struct {
    int gdat_count;
    int fill_count;
    int outline_count;
    int text_count;
} DM2StartupDrawProbe;

static int dm2_startup_probe_gdat(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command)
{
    DM2StartupDrawProbe *probe = (DM2StartupDrawProbe*)userdata;
    if (probe && command &&
        command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
        ++probe->gdat_count;
    }
    return 1;
}

static int dm2_startup_probe_gdat_reject(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command)
{
    DM2StartupDrawProbe *probe = (DM2StartupDrawProbe*)userdata;
    if (probe && command &&
        command->kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE) {
        ++probe->gdat_count;
    }
    return 0;
}

static void dm2_startup_probe_fill(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command)
{
    DM2StartupDrawProbe *probe = (DM2StartupDrawProbe*)userdata;
    if (probe && command &&
        command->kind == DM2_V1_STARTUP_DRAW_FILL_RECT) {
        ++probe->fill_count;
    }
}

static void dm2_startup_probe_outline(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command)
{
    DM2StartupDrawProbe *probe = (DM2StartupDrawProbe*)userdata;
    if (probe && command &&
        command->kind == DM2_V1_STARTUP_DRAW_OUTLINE_RECT) {
        ++probe->outline_count;
    }
}

static void dm2_startup_probe_text(
    void *userdata,
    const DM2_V1_StartupDrawCommand *command)
{
    DM2StartupDrawProbe *probe = (DM2StartupDrawProbe*)userdata;
    if (probe && command &&
        command->kind == DM2_V1_STARTUP_DRAW_TEXT) {
        ++probe->text_count;
    }
}

static int write_tiny_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    fputs(bytes, f);
    fclose(f);
    return 1;
}

/* Test-only corpus construction.  Production session writers are deliberately
 * blocked until DM2_GAME_SAVE is ported; this helper makes each artificial
 * D2RS fixture explicit rather than exercising that public production API. */
static __attribute__((unused)) int write_fixture_d2rs_slot(const char *root, uint8_t slot,
                                   const char *name,
                                   const DM2_V1_SessionState *session)
{
    uint8_t payload[DM2_SESSION_MAX_SIZE];
    int payload_size;
    if (!root || !session) return 0;
    payload_size = dm2_v1_session_serialize(session, payload,
                                             sizeof(payload));
    return payload_size > 0 &&
           dm2_sl_save(root, slot, name, payload, (size_t)payload_size) == 0;
}

static __attribute__((unused)) int write_fixture_d2rs_last_session(const char *root, const char *name,
                                           const DM2_V1_SessionState *session)
{
    uint8_t payload[DM2_SESSION_MAX_SIZE];
    int payload_size;
    if (!root || !session) return 0;
    payload_size = dm2_v1_session_serialize(session, payload,
                                             sizeof(payload));
    return payload_size > 0 &&
           dm2_sl_save_last_session(root, name, payload,
                                    (size_t)payload_size) == 0;
}

static __attribute__((unused)) int framebuffer_zone_differs(const unsigned char *a,
                                    const unsigned char *b,
                                    int width,
                                    int height,
                                    int x,
                                    int y,
                                    int w,
                                    int h)
{
    int xx;
    int yy;

    if (!a || !b || width <= 0 || height <= 0 ||
        x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x + w > width || y + h > height) {
        return 0;
    }
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            int off = (y + yy) * width + x + xx;
            if (a[off] != b[off]) return 1;
        }
    }
    return 0;
}

static __attribute__((unused)) int framebuffer_zone_has_nonzero(const unsigned char *fb,
                                        int width,
                                        int height,
                                        int x,
                                        int y,
                                        int w,
                                        int h)
{
    int xx;
    int yy;

    if (!fb || width <= 0 || height <= 0 ||
        x < 0 || y < 0 || w <= 0 || h <= 0 ||
        x + w > width || y + h > height) {
        return 0;
    }
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] != 0u) return 1;
        }
    }
    return 0;
}

static __attribute__((unused)) int find_loadable_dm2_object_icon_handle(DM2_V1_BootProfile *profile,
                                                uint32_t *out_handle)
{
    static const uint8_t pools[] = {5, 6, 7, 10};
    size_t p;

    if (out_handle) *out_handle = 0u;
    if (!profile) return 0;
    for (p = 0; p < sizeof(pools) / sizeof(pools[0]); ++p) {
        uint32_t idx;
        for (idx = 0; idx < 64u; ++idx) {
            uint8_t *pixels = NULL;
            int w = 0;
            int h = 0;
            int stride = 0;
            uint32_t handle = dm2_db_make_handle(pools[p], idx);

            if (dm2_v1_boot_object_icon_asset_fetch(profile,
                                                    handle,
                                                    &pixels,
                                                    &w,
                                                    &h,
                                                    &stride) == 0 &&
                pixels && w > 0 && h > 0 && stride > 0) {
                dm2_v1_boot_object_icon_asset_free(pixels);
                if (out_handle) *out_handle = handle;
                return 1;
            }
            dm2_v1_boot_object_icon_asset_free(pixels);
        }
    }
    return 0;
}

static int make_temp_dm2_root(char root[512], char dm2_dir[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_dm2_m11_profile_gate_%lu",
             (unsigned long)rand());
    if (TEST_MKDIR(root) != 0) {
        return 0;
    }
#else
    char tmpl[] = "/tmp/firestaff_dm2_m11_profile_gate_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
#endif
    snprintf(dm2_dir, 512, "%s%s%s", root, TEST_PATH_SEP, "dm2");
    if (TEST_MKDIR(dm2_dir) != 0) {
        (void)TEST_RMDIR(root);
        return 0;
    }
    return 1;
}

static void remove_temp_dm2_root(const char* root, const char* dm2_dir) {
    char graphics[512];
    char dungeon[512];
    snprintf(graphics, sizeof(graphics), "%s%sGRAPHICS.DAT",
             dm2_dir, TEST_PATH_SEP);
    snprintf(dungeon, sizeof(dungeon), "%s%sDUNGEON.DAT",
             dm2_dir, TEST_PATH_SEP);
    remove(graphics);
    remove(dungeon);
    (void)TEST_RMDIR(dm2_dir);
    (void)TEST_RMDIR(root);
}

static int make_temp_save_root(char root[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_dm2_m11_resume_%lu",
             (unsigned long)rand());
    return TEST_MKDIR(root) == 0;
#else
    char tmpl[] = "/tmp/firestaff_dm2_m11_resume_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
    return 1;
#endif
}

static void remove_temp_save_root(const char* root) {
    char path[512];
    int i;
    for (i = 0; i < 10; ++i) {
        snprintf(path, sizeof(path), "%s%sSKSave%02d.dat",
                 root, TEST_PATH_SEP, i);
        remove(path);
    }
    snprintf(path, sizeof(path), "%s%sSKSave.dat", root, TEST_PATH_SEP);
    remove(path);
    snprintf(path, sizeof(path), "%s%sSKSave.bak", root, TEST_PATH_SEP);
    remove(path);
    (void)TEST_RMDIR(root);
}

static __attribute__((unused)) int find_in_bounds_door_pose(DM2_V1_DungeonData* dungeon,
                                    int* level,
                                    int* party_x,
                                    int* party_y) {
    if (!dungeon || !level || !party_x || !party_y) {
        return 0;
    }
    for (int i = 0; i < dungeon->level_count; ++i) {
        if (dungeon->level_widths[i] >= 2 && dungeon->level_heights[i] >= 4) {
            *level = i;
            *party_x = 1;
            *party_y = 3;
            return 1;
        }
    }
    return 0;
}

static void fill_dm2_launch_spec(M11_GameLaunchSpec* spec,
                                 const char* data_dir) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "DUNGEON MASTER II";
    spec->gameId = "dm2";
    spec->sourceId = "dm2";
    spec->dataDir = data_dir;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static void expect_dm2_startup_layout_contract(void) {
    DM2_V1_StartupRect rect;
    DM2_V1_StartupHit hit;
    DM2_V1_StartupMenu menu;
    DM2_V1_StartupMenuSnapshot snapshot;
    DM2_V1_StartupHostFacts host_facts;
    DM2_V1_BootRuntimeStartupSnapshot boot_snapshot;
    DM2_V1_BootStartupViewModel boot_view_model;
    DM2_V1_BootStartupHostViewReceipt host_view_receipt;
    DM2_V1_BootStartupPackagedFullStartReceipt full_start_package;
    DM2_V1_BootStartupPackagedConsumerReceipt consumer_receipt;
    DM2_V1_StartupMenu restored_menu;
    DM2_V1_StartupAction action;
    DM2_V1_StartupViewReceipt view_receipt;
    DM2_V1_StartupRowKind row_kind = DM2_V1_STARTUP_ROW_NONE;
    DM2_V1_StartupDrawCommand commands[16];
    DM2_V1_StartupDrawExecutor executor;
    DM2StartupDrawProbe draw_probe;
    int command_count;
    char label[64];
    char phase[64];
    char animation[64];
    char save_root[128];
    int startup_active = -1;
    int animation_active = -1;
    int title_frame = -1;
    int title_frame_max = -1;
    int title_ready = -1;
    uint8_t save_slot = 99u;
    int last_session = 0;
    int slot = -1;

    expect_true(dm2_v1_startup_save_path_to_root_slot(
                    "/tmp/firestaff-dm2-test-saves/SKSave.dat",
                    save_root,
                    (int)sizeof(save_root),
                    &save_slot,
                    &last_session) &&
                    strcmp(save_root, "/tmp/firestaff-dm2-test-saves") == 0 &&
                    save_slot == 0u &&
                    last_session == 1,
                "DM2 startup owns SKSave.dat direct resume path parsing");
    expect_true(dm2_v1_startup_save_path_to_root_slot(
                    "/tmp/firestaff-dm2-test-saves/SKSave03.dat",
                    save_root,
                    (int)sizeof(save_root),
                    &save_slot,
                    &last_session) &&
                    strcmp(save_root, "/tmp/firestaff-dm2-test-saves") == 0 &&
                    save_slot == 3u &&
                    last_session == 0,
                "DM2 startup owns SKSaveNN.dat slot path parsing");
    expect_true(!dm2_v1_startup_save_path_to_root_slot(
                    "/tmp/firestaff-dm2-test-saves/SKSave10.dat",
                    save_root,
                    (int)sizeof(save_root),
                    &save_slot,
                    &last_session),
                "DM2 startup rejects out-of-range SKSave slot paths");

    expect_true(!dm2_v1_startup_panel_rect(&rect) &&
                    !dm2_v1_startup_row_rect(0, &rect) &&
                    !dm2_v1_startup_row_highlight_rect(1, &rect),
                "DM2 startup rejects obsolete host panel and row geometry");
    expect_true(!dm2_v1_startup_hit(2, 100, 78, &hit) &&
                    hit.kind == DM2_V1_STARTUP_HIT_NONE && hit.row == -1,
                "DM2 startup rejects host-coordinate hit testing");

    dm2_v1_startup_menu_init(&menu, "/tmp/firestaff-dm2-test-saves");
    expect_true(dm2_v1_startup_menu_refresh(&menu, 1, (1u << 3)),
                "DM2 startup menu refresh accepts resume and slot mask");
    expect_true(menu.row_count == 3,
                "DM2 startup menu counts CONTINUE, slot, and NEW GAME rows");
    expect_true(dm2_v1_startup_menu_snapshot_from_menu(&snapshot, &menu) &&
                    strcmp(snapshot.save_root,
                           "/tmp/firestaff-dm2-test-saves") == 0 &&
                    snapshot.resume_available == 1 &&
                    snapshot.slot_mask == (1u << 3) &&
                    snapshot.row_count == 3 &&
                    snapshot.selected_row == 0,
                "DM2 startup snapshot captures menu state");
    snapshot.selected_row = 7;
    expect_true(dm2_v1_startup_menu_from_snapshot(&snapshot,
                                                  &restored_menu) &&
                    restored_menu.row_count == 3 &&
                    restored_menu.selected_row == 2,
                "DM2 startup snapshot restore clamps stale selection");
    expect_true(dm2_v1_startup_menu_snapshot_row_at(
                    &snapshot,
                    1,
                    &row_kind,
                    &slot) &&
                    row_kind == DM2_V1_STARTUP_ROW_SLOT &&
                    slot == 3,
                "DM2 startup snapshot resolves rows without M11 menu policy");
    expect_true(dm2_v1_startup_menu_snapshot_handle_input(
                    &snapshot,
                    DM2_V1_STARTUP_INPUT_UP,
                    &action) &&
                    snapshot.selected_row == 1 &&
                    action.kind == DM2_V1_STARTUP_ACTION_NONE,
                "DM2 startup snapshot handles input and stores selection");
    hit.kind = DM2_V1_STARTUP_HIT_ROW;
    hit.row = 0;
    expect_true(dm2_v1_startup_menu_snapshot_handle_hit(
                    &snapshot,
                    &hit,
                    &action) &&
                    snapshot.selected_row == 0 &&
                    action.kind == DM2_V1_STARTUP_ACTION_CONTINUE,
                "DM2 startup snapshot handles row hits and stores selection");
    command_count = dm2_v1_startup_presentation_build_from_snapshot(
        &snapshot,
        commands,
        (int)(sizeof(commands) / sizeof(commands[0])));
    expect_true(command_count == 2 &&
                    commands[0].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[0].gdat_field == 4 &&
                    commands[0].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_TITLE &&
                    commands[1].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[1].gdat_field == 4 &&
                    commands[1].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_MENU,
                "DM2 startup presentation builds from snapshot");
    command_count = dm2_v1_startup_presentation_build_from_facts(
        "",
        "/tmp/firestaff-dm2-test-saves",
        1,
        (1u << 3),
        9,
        commands,
        (int)(sizeof(commands) / sizeof(commands[0])));
    expect_true(command_count == 2 &&
                    commands[0].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[0].gdat_field == 4 &&
                    commands[0].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_TITLE &&
                    commands[1].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[1].gdat_field == 4 &&
                    commands[1].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_MENU,
                "DM2 startup presentation builds directly from runtime facts");
    memset(&host_facts, 0, sizeof(host_facts));
    host_facts.save_root = "";
    host_facts.fallback_save_root = "/tmp/firestaff-dm2-test-saves";
    host_facts.resume_available = 1;
    host_facts.slot_mask = (1u << 3);
    host_facts.selected_row = 9;
    command_count = dm2_v1_startup_presentation_build_from_host_facts(
        &host_facts,
        commands,
        (int)(sizeof(commands) / sizeof(commands[0])));
    expect_true(command_count == 2 &&
                    commands[0].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[0].gdat_field == 4 &&
                    commands[0].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_TITLE &&
                    commands[1].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[1].gdat_field == 4 &&
                    commands[1].frame_owner ==
                        DM2_V1_FRAME_OWNER_STARTUP_MENU,
                "DM2 startup presentation builds from host facts");
    memset(&boot_snapshot, 0, sizeof(boot_snapshot));
    boot_snapshot.startup_menu_active = 1;
    boot_snapshot.startup_save_root = "/tmp/firestaff-dm2-test-saves";
    boot_snapshot.resume_available = 1;
    boot_snapshot.slot_mask = (1u << 3);
    boot_snapshot.selected_row = 9;
    memset(&view_receipt, 0, sizeof(view_receipt));
    command_count = 0;
    expect_true(dm2_v1_boot_startup_view_model_from_snapshot(
                    &boot_snapshot,
                    commands,
                    (int)(sizeof(commands) / sizeof(commands[0])),
                    &command_count,
                    &view_receipt,
                    phase,
                    (int)sizeof(phase),
                    &startup_active,
                    animation,
                    (int)sizeof(animation),
                    &animation_active,
                    &title_frame,
                    &title_frame_max,
                    &title_ready) &&
                    command_count == 2 &&
                    view_receipt.valid &&
                    view_receipt.command_count == command_count &&
                    view_receipt.render.command_count == command_count &&
                    view_receipt.menu_state.selected_row == 2 &&
                    view_receipt.render.title_gdat_found &&
                    view_receipt.render.hud_overlay_suppressed == 1 &&
                    view_receipt.render.title_backdrop_ready == 1 &&
                    view_receipt.render.resume_menu_ready == 1 &&
                    view_receipt.render.save_slot_menu_ready == 1 &&
                    view_receipt.render.new_game_menu_ready == 1 &&
                    view_receipt.render.full_start_graphics_ready == 1 &&
                    view_receipt.runtime_handoff.startup_menu_active == 1 &&
                    view_receipt.runtime_handoff.runtime_menu_ready == 1 &&
                    view_receipt.runtime_handoff.runtime_action_ready == 0 &&
                    view_receipt.runtime_handoff.first_hud_frame_ready == 0 &&
                    strcmp(view_receipt.runtime_handoff.animation,
                           "dm2-startup-menu") == 0,
                "DM2 boot startup view model carries the presentation view receipt");
    expect_true(dm2_v1_boot_startup_view_model_receipt_from_snapshot(
                    &boot_snapshot,
                    &boot_view_model) &&
                    boot_view_model.command_count == command_count &&
                    boot_view_model.view_receipt.valid &&
                    boot_view_model.view_receipt.render.command_count ==
                        command_count &&
                    boot_view_model.view_receipt.render.hud_overlay_suppressed ==
                        1 &&
                    boot_view_model.title_backdrop_ready == 1 &&
                    boot_view_model.resume_menu_ready == 1 &&
                    boot_view_model.save_slot_menu_ready == 1 &&
                    boot_view_model.new_game_menu_ready == 1 &&
                    boot_view_model.full_start_graphics_ready == 1 &&
                    boot_view_model.title_gdat_asset_ready == 0 &&
                    boot_view_model.full_start_real_asset_ready == 0 &&
                    boot_view_model.full_start_receipt.valid &&
                    boot_view_model.full_start_receipt.startup_menu_active == 1 &&
                    boot_view_model.full_start_receipt.title_frame == 0 &&
                    boot_view_model.full_start_receipt.title_frame_max == 0 &&
                    boot_view_model.full_start_receipt
                            .title_frame_duration_ticks == 0 &&
                    boot_view_model.full_start_receipt.title_cycle_ticks == 1 &&
                    boot_view_model.full_start_receipt
                            .title_frame_start_tick == 0 &&
                    boot_view_model.full_start_receipt
                            .title_next_frame_tick == 1 &&
                    boot_view_model.full_start_receipt
                            .title_cycle_position_tick == 0 &&
                    boot_view_model.full_start_receipt
                            .title_frame_elapsed_ticks == 0 &&
                    boot_view_model.full_start_receipt
                            .title_frame_remaining_ticks == 1 &&
                    boot_view_model.full_start_receipt
                            .title_cycle_remaining_ticks == 1 &&
                    boot_view_model.full_start_receipt
                            .exact_title_timing_ready == 1 &&
                    boot_view_model.full_start_receipt.title_backdrop_ready == 1 &&
                    boot_view_model.full_start_receipt.menu_row_count == 3 &&
                    boot_view_model.full_start_receipt.menu_text_count == 0 &&
                    boot_view_model.full_start_receipt.selectable_text_count == 0 &&
                    boot_view_model.full_start_receipt
                            .selected_highlight_count == 0 &&
                    boot_view_model.full_start_receipt.menu_panel_ready == 1 &&
                    boot_view_model.full_start_receipt
                            .startup_menu_assets_ready == 1 &&
                    boot_view_model.full_start_receipt.hud_overlay_suppressed == 1 &&
                    boot_view_model.full_start_receipt.runtime_menu_ready == 1 &&
                    boot_view_model.full_start_receipt.runtime_action_ready == 0 &&
                    boot_view_model.full_start_receipt.first_hud_frame_ready == 0 &&
                    boot_view_model.full_start_receipt
                            .full_start_graphics_ready == 1 &&
                    boot_view_model.full_start_receipt
                            .full_start_real_asset_ready == 0 &&
                    boot_view_model.host_view_receipt.valid &&
                    boot_view_model.host_view_receipt.draw_startup_menu == 1 &&
                    boot_view_model.host_view_receipt.render_commands_ready == 1 &&
                    boot_view_model.host_view_receipt.menu_state_ready == 1 &&
                    boot_view_model.host_view_receipt.row_selection_ready == 1 &&
                    boot_view_model.host_view_receipt.resume_menu_ready == 1 &&
                    boot_view_model.host_view_receipt.save_slot_menu_ready == 1 &&
                    boot_view_model.host_view_receipt.new_game_menu_ready == 1 &&
                    boot_view_model.host_view_receipt.title_timing_ready == 1 &&
                    boot_view_model.host_view_receipt.title_asset_ready == 1 &&
                    boot_view_model.host_view_receipt.title_menu_ready == 1 &&
                    boot_view_model.host_view_receipt.title_animation_tick == 0 &&
                    boot_view_model.host_view_receipt.title_cycle_ticks == 1 &&
                    boot_view_model.host_view_receipt.title_frame_start_tick == 0 &&
                    boot_view_model.host_view_receipt.title_next_frame_tick == 1 &&
                    boot_view_model.host_view_receipt
                            .title_cycle_position_tick == 0 &&
                    boot_view_model.host_view_receipt
                            .title_frame_elapsed_ticks == 0 &&
                    boot_view_model.host_view_receipt
                            .title_frame_remaining_ticks == 1 &&
                    boot_view_model.host_view_receipt
                            .title_cycle_remaining_ticks == 1 &&
                    boot_view_model.host_view_receipt
                            .exact_title_timing_ready == 1 &&
                    boot_view_model.host_view_receipt.title_gdat_asset_ready == 0 &&
                    boot_view_model.host_view_receipt
                            .full_start_real_asset_ready == 0 &&
                    boot_view_model.host_view_receipt.menu_row_count == 3 &&
                    boot_view_model.host_view_receipt.menu_text_count == 0 &&
                    boot_view_model.host_view_receipt.selectable_text_count == 0 &&
                    boot_view_model.host_view_receipt.selected_highlight_count == 0 &&
                    boot_view_model.host_view_receipt.menu_panel_ready == 1 &&
                    boot_view_model.host_view_receipt.startup_menu_assets_ready == 1 &&
                    boot_view_model.host_view_receipt.hud_overlay_suppressed == 1 &&
                    boot_view_model.host_view_receipt.runtime_menu_ready == 1 &&
                    boot_view_model.host_view_receipt.runtime_action_ready == 0 &&
                    boot_view_model.host_view_receipt.first_hud_frame_ready == 0 &&
                    boot_view_model.host_view_receipt
                            .startup_hud_handoff_ready == 1 &&
                    boot_view_model.host_view_receipt.runtime_handoff_ready == 0 &&
                    boot_view_model.host_view_receipt.m11_host_view_ready == 1 &&
                    strcmp(boot_view_model.host_view_receipt.status_scope,
                           "STARTUP") == 0 &&
                    boot_view_model.host_view_receipt.status == NULL &&
                    strcmp(boot_view_model.phase, "dm2-startup-menu") == 0 &&
                    boot_view_model.startup_active == 1 &&
                    strcmp(boot_view_model.animation,
                           "dm2-startup-menu") == 0 &&
                    boot_view_model.animation_active == 1 &&
                    boot_view_model.title_ready == 1 &&
                    boot_view_model.initialize_v2_runtime == 1 &&
                    boot_view_model.initialize_hud_runtime == 1 &&
                    boot_view_model.initialize_touch_runtime == 1 &&
                    boot_view_model.hud_runtime_ready == 1 &&
                    boot_view_model.runtime_menu_ready == 1 &&
                    boot_view_model.runtime_action_ready == 0 &&
                    boot_view_model.first_hud_frame_ready == 0,
                "DM2 boot owns the startup view model wrapper consumed by M11");
    boot_snapshot.startup_menu_active = 0;
    expect_true(dm2_v1_boot_startup_view_model_receipt_from_snapshot(
                    &boot_snapshot,
                    &boot_view_model) &&
                    boot_view_model.command_count == 0 &&
                    boot_view_model.view_receipt.valid &&
                    boot_view_model.view_receipt.render.command_count == 0 &&
                    boot_view_model.view_receipt.render.hud_overlay_suppressed ==
                        0 &&
                    boot_view_model.view_receipt.runtime_handoff
                            .startup_menu_active == 0 &&
                    strcmp(boot_view_model.phase, "dm2-runtime") == 0 &&
                    boot_view_model.startup_active == 0 &&
                    strcmp(boot_view_model.animation, "dm2-runtime") == 0 &&
                    boot_view_model.animation_active == 0 &&
                    boot_view_model.title_ready == 1 &&
                    boot_view_model.hud_runtime_ready == 1 &&
                    boot_view_model.runtime_menu_ready == 0 &&
                    boot_view_model.runtime_action_ready == 1 &&
                    boot_view_model.first_hud_frame_ready == 1,
                "DM2 boot view model hands off from title/menu to first runtime HUD state");
    boot_snapshot.startup_menu_active = 1;
    expect_true(dm2_v1_boot_startup_host_view_receipt_from_snapshot(
                    &boot_snapshot,
                    &host_view_receipt) &&
                    host_view_receipt.valid &&
                    host_view_receipt.draw_startup_menu == 1 &&
                    host_view_receipt.command_count == command_count &&
                    host_view_receipt.selected_row == 2 &&
                    host_view_receipt.title_timing_ready == 1 &&
                    host_view_receipt.title_asset_ready == 1 &&
                    host_view_receipt.title_menu_ready == 1 &&
                    host_view_receipt.title_animation_tick == 0 &&
                    host_view_receipt.title_cycle_ticks == 1 &&
                    host_view_receipt.title_frame_start_tick == 0 &&
                    host_view_receipt.title_next_frame_tick == 1 &&
                    host_view_receipt.title_cycle_position_tick == 0 &&
                    host_view_receipt.title_frame_elapsed_ticks == 0 &&
                    host_view_receipt.title_frame_remaining_ticks == 1 &&
                    host_view_receipt.title_cycle_remaining_ticks == 1 &&
                    host_view_receipt.exact_title_timing_ready == 1 &&
                    host_view_receipt.title_frame == 0 &&
                    host_view_receipt.title_frame_max == 0 &&
                    host_view_receipt.title_frame_duration_ticks == 0 &&
                    host_view_receipt.menu_row_count == 3 &&
                    host_view_receipt.menu_text_count == 0 &&
                    host_view_receipt.selectable_text_count == 0 &&
                    host_view_receipt.selected_highlight_count == 0 &&
                    host_view_receipt.menu_panel_ready == 1 &&
                    host_view_receipt.startup_menu_assets_ready == 1 &&
                    host_view_receipt.hud_overlay_suppressed == 1 &&
                    host_view_receipt.hud_runtime_ready == 1 &&
                    host_view_receipt.runtime_menu_ready == 1 &&
                    host_view_receipt.runtime_action_ready == 0 &&
                    host_view_receipt.first_hud_frame_ready == 0 &&
                    host_view_receipt.startup_hud_handoff_ready == 1 &&
                    host_view_receipt.runtime_handoff_ready == 0 &&
                    host_view_receipt.m11_host_view_ready == 1 &&
                    host_view_receipt.capture_proof_valid == 1 &&
                    host_view_receipt.capture_proof.valid == 1 &&
                    host_view_receipt.capture_proof.menu_capture_ready == 1 &&
                    host_view_receipt.capture_proof.hud_handoff_capture_ready == 1 &&
                    host_view_receipt.capture_proof.title_capture_ready == 0 &&
                    host_view_receipt.capture_proof.packaged_capture_hash != 0u &&
                    strcmp(host_view_receipt.status_scope, "STARTUP") == 0 &&
                    host_view_receipt.status == NULL &&
                    host_view_receipt.log_line == NULL,
                "DM2 boot host-view receipt keeps startup text unbound");
    expect_true(dm2_v1_boot_startup_packaged_full_start_receipt_from_snapshot(
                    &boot_snapshot,
                    &full_start_package) &&
                    full_start_package.valid &&
                    full_start_package.full_start_valid == 1 &&
                    full_start_package.capture_proof_valid == 1 &&
                    full_start_package.title_frame_start_tick == 0 &&
                    full_start_package.title_next_frame_tick == 1 &&
                    full_start_package.title_gdat_category == 5 &&
                    full_start_package.title_gdat_index == 0 &&
                    full_start_package.title_gdat_field == 4 &&
                    full_start_package.full_start.title_backdrop_ready == 1 &&
                    full_start_package.title_ready == 1 &&
                    full_start_package.hud_overlay_suppressed == 1 &&
                    full_start_package.hud_runtime_ready == 1 &&
                    full_start_package.runtime_menu_ready == 1 &&
                    full_start_package.runtime_action_ready == 0 &&
                    full_start_package.first_hud_frame_ready == 0 &&
                    full_start_package.full_start_graphics_ready == 1 &&
                    full_start_package.menu_capture_ready == 1 &&
                    full_start_package.hud_handoff_capture_ready == 1 &&
                    full_start_package.m11_consumer_ready == 1 &&
                    full_start_package.packaged_full_start_hash != 0u,
                "DM2 packaged full-start receipt joins timing/assets/menu/HUD proof");
    expect_true(dm2_v1_boot_startup_packaged_consumer_receipt_from_snapshot(
                    &boot_snapshot,
                    &consumer_receipt) &&
                    consumer_receipt.valid &&
                    consumer_receipt.packaged_full_start_hash ==
                        full_start_package.packaged_full_start_hash &&
                    consumer_receipt.startup_active == 1 &&
                    consumer_receipt.startup_draw_ready == 1 &&
                    consumer_receipt.startup_draw_command_count ==
                        full_start_package.command_count &&
                    consumer_receipt.startup_title_frame == 0 &&
                    consumer_receipt.startup_title_frame_max == 0 &&
                    consumer_receipt.startup_title_ready == 1 &&
                    consumer_receipt.packaged_title_timing_consumed == 1 &&
                    consumer_receipt.packaged_first_hud_receipt_consumed == 1 &&
                    consumer_receipt.m11_startup_receipt_ready == 1 &&
                    consumer_receipt.runtime_menu_ready == 1 &&
                    consumer_receipt.runtime_action_ready == 0 &&
                    consumer_receipt.first_hud_frame_ready == 0 &&
                    consumer_receipt.startup_draw_menu_capture_ready == 1 &&
                    consumer_receipt.startup_draw_hud_handoff_ready == 1 &&
                    strcmp(consumer_receipt.phase, "dm2-startup-menu") == 0,
                "DM2 packaged consumer receipt gives M11 one startup draw contract");
    boot_snapshot.startup_menu_active = 1;
    expect_true(dm2_v1_startup_presentation_receipt(
                    1,
                    phase,
                    (int)sizeof(phase),
                    &startup_active,
                    animation,
                    (int)sizeof(animation),
                    &animation_active,
                    &title_frame,
                    &title_frame_max,
                    &title_ready) &&
                    strcmp(phase, "dm2-startup-menu") == 0 &&
                    startup_active == 1 &&
                    strcmp(animation, "dm2-startup-menu") == 0 &&
                    animation_active == 1 &&
                    title_frame == 0 &&
                    title_frame_max == 0 &&
                    title_ready == 1,
                "DM2 startup presentation owns active boot receipt fields");
    expect_true(dm2_v1_startup_presentation_receipt(
                    0,
                    phase,
                    (int)sizeof(phase),
                    &startup_active,
                    animation,
                    (int)sizeof(animation),
                    &animation_active,
                    &title_frame,
                    &title_frame_max,
                    &title_ready) &&
                    strcmp(phase, "dm2-runtime") == 0 &&
                    startup_active == 0 &&
                    strcmp(animation, "dm2-runtime") == 0 &&
                    animation_active == 0 &&
                    title_frame == 0 &&
                    title_frame_max == 0 &&
                    title_ready == 1,
                "DM2 startup presentation owns runtime boot receipt fields");
    memset(&draw_probe, 0, sizeof(draw_probe));
    executor.userdata = &draw_probe;
    executor.draw_gdat_image = dm2_startup_probe_gdat;
    executor.fill_rect = dm2_startup_probe_fill;
    executor.outline_rect = dm2_startup_probe_outline;
    executor.draw_text = dm2_startup_probe_text;
    expect_true(dm2_v1_startup_execute_draw_commands(
                    commands,
                    command_count,
                    &executor) &&
                    draw_probe.gdat_count == 2 &&
                    draw_probe.fill_count == 0 &&
                    draw_probe.outline_count == 0 &&
                    draw_probe.text_count == 0,
                "DM2 startup presentation owns draw-command executor dispatch");
    memset(&draw_probe, 0, sizeof(draw_probe));
    executor.userdata = &draw_probe;
    executor.draw_gdat_image = dm2_startup_probe_gdat_reject;
    expect_true(!dm2_v1_startup_execute_draw_commands(
                    commands,
                    command_count,
                    &executor) &&
                    draw_probe.gdat_count == 1,
                "DM2 startup executor fails closed when original GDAT draw rejects");
    executor.draw_gdat_image = dm2_startup_probe_gdat;
    expect_true(dm2_v1_startup_menu_row_at(&menu, 0, &row_kind, &slot) &&
                    row_kind == DM2_V1_STARTUP_ROW_CONTINUE &&
                    slot == -1,
                "DM2 startup menu row 0 is CONTINUE");
    expect_true(dm2_v1_startup_menu_row_at(&menu, 1, &row_kind, &slot) &&
                    row_kind == DM2_V1_STARTUP_ROW_SLOT &&
                    slot == 3,
                "DM2 startup menu row 1 is LOAD SLOT 03");
    expect_true(dm2_v1_startup_menu_move_selected(&menu, 8) &&
                    menu.selected_row == 2,
                "DM2 startup menu movement clamps at NEW GAME");
    expect_true(dm2_v1_startup_menu_handle_input(
                    &menu,
                    DM2_V1_STARTUP_INPUT_ACCEPT,
                    &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_NEW_GAME &&
                    action.row == 2 &&
                    action.slot == -1,
                "DM2 startup menu Accept reports NEW GAME");
    expect_true(dm2_v1_startup_menu_handle_input(
                    &menu,
                    DM2_V1_STARTUP_INPUT_UP,
                    &action) &&
                    menu.selected_row == 1 &&
                    action.kind == DM2_V1_STARTUP_ACTION_NONE &&
                    dm2_v1_startup_menu_handle_input(
                        &menu,
                        DM2_V1_STARTUP_INPUT_ACTION,
                        &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_LOAD_SLOT &&
                    action.slot == 3,
                "DM2 startup menu Up/Action reports LOAD SLOT");
    expect_true(dm2_v1_startup_menu_handle_input(
                    &menu,
                    DM2_V1_STARTUP_INPUT_UP,
                    &action) &&
                    menu.selected_row == 0 &&
                    dm2_v1_startup_menu_handle_input(
                        &menu,
                        DM2_V1_STARTUP_INPUT_ACCEPT,
                        &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_CONTINUE,
                "DM2 startup menu Up/Accept reports CONTINUE");
    expect_true(dm2_v1_startup_menu_handle_input(
                    &menu,
                    DM2_V1_STARTUP_INPUT_BACK,
                    &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_RETURN_TO_LAUNCHER &&
                    action.row == menu.selected_row,
                "DM2 startup menu Back reports launcher return");
    hit.kind = DM2_V1_STARTUP_HIT_PANEL;
    hit.row = -1;
    expect_true(dm2_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_NONE &&
                    action.row == menu.selected_row,
                "DM2 startup menu panel hit is consumed through DM2 API");
    hit.kind = DM2_V1_STARTUP_HIT_ROW;
    hit.row = 1;
    expect_true(dm2_v1_startup_menu_handle_hit(&menu, &hit, &action) &&
                    menu.selected_row == 1 &&
                    action.kind == DM2_V1_STARTUP_ACTION_LOAD_SLOT &&
                    action.row == 1 &&
                    action.slot == 3,
                "DM2 startup menu row hit activates through DM2 API");
    expect_true(!dm2_v1_startup_row_label(DM2_V1_STARTUP_ROW_CONTINUE,
                                          -1,
                                          label,
                                          (int)sizeof(label)) &&
                    label[0] == '\0' &&
                    !dm2_v1_startup_row_label(DM2_V1_STARTUP_ROW_SLOT,
                                               3,
                                               label,
                                               (int)sizeof(label)),
                "DM2 startup rejects host-authored row labels");
    command_count = dm2_v1_startup_presentation_build(
        &menu,
        commands,
        (int)(sizeof(commands) / sizeof(commands[0])));
    expect_true(command_count == 2,
                "DM2 startup presentation emits both logical menu owners");
    expect_true(commands[0].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[0].gdat_category == 5 &&
                    commands[0].gdat_index == 0 &&
                    commands[0].gdat_field == 4 &&
                    commands[0].rect.w == 320 &&
                    commands[0].rect.h == 200,
                "DM2 startup presentation owns original GDAT menu surface command");
    expect_true(commands[0].frame_owner == DM2_V1_FRAME_OWNER_STARTUP_TITLE &&
                    commands[1].kind == DM2_V1_STARTUP_DRAW_GDAT_IMAGE &&
                    commands[1].gdat_field == 4 &&
                    commands[1].frame_owner == DM2_V1_FRAME_OWNER_STARTUP_MENU,
                "DM2 startup presentation keeps both owners on the menu surface");
    menu.selected_row = 9;
    expect_true(dm2_v1_startup_menu_refresh(&menu, 0, 0u) &&
                    menu.row_count == 1 &&
                    menu.selected_row == 0 &&
                    dm2_v1_startup_menu_handle_input(
                        &menu,
                        DM2_V1_STARTUP_INPUT_ACCEPT,
                        &action) &&
                    action.kind == DM2_V1_STARTUP_ACTION_NEW_GAME,
                "DM2 startup refresh clamps stale selection after slots disappear");
}

static void check_incomplete_required_files_block_m11(const char* label,
                                                      int seed_graphics,
                                                      int seed_dungeon) {
    char root[512];
    char dm2_dir[512];
    char path[512];
    DM2_V1_BootProfile preflight;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    expect_true(make_temp_dm2_root(root, dm2_dir),
                "created isolated DM2 incomplete-data root");
    if (seed_graphics) {
        snprintf(path, sizeof(path), "%s%sGRAPHICS.DAT", dm2_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-dm2-graphics"),
                    "seeded synthetic GRAPHICS.DAT");
    }
    if (seed_dungeon) {
        snprintf(path, sizeof(path), "%s%sDUNGEON.DAT", dm2_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-dm2-dungeon"),
                    "seeded synthetic DUNGEON.DAT");
    }

    dm2_v1_boot_profile_init(&preflight);
    if (seed_graphics && seed_dungeon) {
        expect_true(dm2_v1_boot_scan_assets(&preflight, root) != 0,
                    "preflight rejects both synthetic DM2 lookalike files");
        expect_true(preflight.assets_verified == 0,
                    "preflight refuses unverified synthetic DM2 pair");
    } else {
        expect_true(dm2_v1_boot_scan_assets(&preflight, root) != 0,
                    "preflight refuses incomplete DM2 required file pair");
        expect_true(preflight.assets_verified == 0,
                    "incomplete DM2 profile is not hash verified");
    }
    dm2_v1_boot_cleanup(&preflight);

    fill_dm2_launch_spec(&spec, root);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                label);
    expect_true(view.active == 0,
                "M11 incomplete/unverified DM2 launch leaves view inactive");
    expect_true(view.dm2BootProfile == NULL,
                "M11 incomplete/unverified DM2 launch does not retain boot profile");
    expect_true(view.dm2World == NULL,
                "M11 incomplete/unverified DM2 launch does not retain world pointer");
    expect_true(view.sourceKind != M11_GAME_SOURCE_DM2_BOOT,
                "M11 incomplete/unverified DM2 launch does not claim DM2 boot source");
    expect_true(view.lastOutcome[0] == '\0',
                "M11 keeps the unbound DM2 launch blocker status silent");
    M11_GameView_Shutdown(&view);
    remove_temp_dm2_root(root, dm2_dir);
}

static const char* dm2_data_dir(void) {
    const char* data_dir = getenv("FIRESTAFF_DM2_V1_DATA_DIR");
    if (!data_dir || !data_dir[0]) {
        data_dir = getenv("FIRESTAFF_DM2_CANONICAL_DIR");
    }
    /* Keep the M11 gate aligned with the public DM2 real-data convention
     * used by the other corpus probes and by the launcher smoke command.
     * This is a selected original-data root, not a fallback fixture path. */
    if (!data_dir || !data_dir[0]) {
        data_dir = getenv("FIRESTAFF_DM2_DATA_DIR");
    }
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    return NULL;
}

static int dm2_data_dir_is_explicit(void) {
    static const char* const names[] = {
        "FIRESTAFF_DM2_V1_DATA_DIR",
        "FIRESTAFF_DM2_CANONICAL_DIR",
        "FIRESTAFF_DM2_DATA_DIR"
    };
    size_t i;

    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i) {
        const char* value = getenv(names[i]);
        if (value && value[0] != '\0') {
            return 1;
        }
    }
    return 0;
}

/* The launcher scanner may be pointed at a DOS install root while the
 * authenticated pair lives in its original DATA/ subdirectory.  Keep this
 * as a real-data-only boundary test: M12 must hand M11 that owning directory,
 * never merely the configured scan root.  SKULL.ASM T560 owns DUNGEON.DAT;
 * dm2_v1_boot_scan_assets() is the corresponding hash-first Firestaff entry.
 */
static void expect_m12_dm2_verified_launch(const char* data_dir,
                                           const char* expected_asset_root) {
    M12_StartupMenuState menu;
    M12_StartupMenuInitOptions options;
    M11_GameViewState launched_view;
    const char* runtime_dir;

    if (!data_dir || !data_dir[0]) return;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, data_dir, "dm2", &options);
    runtime_dir = M12_AssetStatus_GetRuntimeDataDir(&menu.assetStatus, "dm2");
    expect_true(M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm2") == 1,
                "M12 finds the real DM2 DOS pair before launch");
    expect_true(runtime_dir && runtime_dir[0] != '\0',
                "M12 publishes a DM2 runtime data directory");
    expect_true(runtime_dir && strstr(runtime_dir, "::") == NULL,
                "loose DOS data keeps an ordinary runtime directory");
    expect_true(runtime_dir && expected_asset_root &&
                    strcmp(runtime_dir, expected_asset_root) == 0,
                "M12 hands M11 the authenticated DM2 asset-owner directory");

    /* Main card -> options; first UP wraps the initial presentation row to
     * the dedicated Launch row.  No option is changed by this sequence. */
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    expect_true(menu.launchRequested == 1,
                "verified DM2 M12 launch row requests M11 handoff");

    M11_GameView_Init(&launched_view);
    expect_true(M11_GameView_OpenSelectedMenuEntry(&launched_view, &menu) == 1,
                "M12 verified DM2 launch reaches the M11 boot owner");
    expect_true(launched_view.sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
                    launched_view.dm2BootProfile != NULL,
                "M12 DM2 launch retains the verified boot profile");
    M11_GameView_Shutdown(&launched_view);
}

static int append_blob(uint8_t *dst, size_t cap, size_t *pos,
                       const void *src, size_t n)
{
    if (!dst || !pos || !src || *pos > cap || n > cap - *pos) return 0;
    memcpy(dst + *pos, src, n);
    *pos += n;
    return 1;
}

static void write_i32_le(uint8_t out[4], int value)
{
    uint32_t v = (uint32_t)value;
    out[0] = (uint8_t)(v & 0xFFu);
    out[1] = (uint8_t)((v >> 8) & 0xFFu);
    out[2] = (uint8_t)((v >> 16) & 0xFFu);
    out[3] = (uint8_t)((v >> 24) & 0xFFu);
}

static void write_u32_le(uint8_t out[4], uint32_t value)
{
    out[0] = (uint8_t)(value & 0xFFu);
    out[1] = (uint8_t)((value >> 8) & 0xFFu);
    out[2] = (uint8_t)((value >> 16) & 0xFFu);
    out[3] = (uint8_t)((value >> 24) & 0xFFu);
}

static int write_original_resume_slot(const char *save_root,
                                      uint8_t slot,
                                      const char *name,
                                      const DM2_GameStateBlock *gs,
                                      const DM2_ChampionRecord *champ,
                                      const DM2_MinionTable *minions)
{
    uint8_t payload[768];
    uint8_t enc_gs[DM2_GAME_STATE_BLOCK_SIZE];
    uint8_t champ_mask[261];
    uint8_t enc_champ[261];
    uint8_t size_le[4];
    size_t pos = 0u;
    int gs_n;
    int champ_n;

    if (!save_root || !gs || !champ) return 0;
    gs_n = dm2_suppress_encode_gamestate(gs, enc_gs, sizeof(enc_gs));
    if (gs_n <= 0) return 0;
    dm2_suppress_champion_mask(champ_mask);
    champ_n = dm2_suppress_encode_champion(champ,
                                           champ_mask,
                                           enc_champ,
                                           sizeof(enc_champ));
    if (champ_n <= 0) return 0;

    if (!append_blob(payload, sizeof(payload), &pos, "D2RS", 4)) return 0;
    write_i32_le(size_le, gs_n);
    if (!append_blob(payload, sizeof(payload), &pos, size_le, 4)) return 0;
    if (!append_blob(payload, sizeof(payload), &pos, enc_gs,
                     (size_t)gs_n)) return 0;
    write_i32_le(size_le, champ_n);
    if (!append_blob(payload, sizeof(payload), &pos, size_le, 4)) return 0;
    if (!append_blob(payload, sizeof(payload), &pos, enc_champ,
                     (size_t)champ_n)) return 0;
    if (minions && minions->count > 0u) {
        uint8_t minion_blob[4u + DM2_MAX_MINIONS * 8u];
        size_t minion_pos = 0u;
        uint8_t count = minions->count > DM2_MAX_MINIONS
                            ? DM2_MAX_MINIONS
                            : minions->count;
        write_i32_le(minion_blob, (int)count);
        minion_pos = 4u;
        for (int i = 0; i < (int)count; i++) {
            write_u32_le(minion_blob + minion_pos,
                         minions->entries[i].object_id);
            write_u32_le(minion_blob + minion_pos + 4u,
                         minions->entries[i].owner_champion);
            minion_pos += 8u;
        }
        if (!append_blob(payload, sizeof(payload), &pos, "MIN0", 4)) {
            return 0;
        }
        write_i32_le(size_le, (int)minion_pos);
        if (!append_blob(payload, sizeof(payload), &pos, size_le, 4)) {
            return 0;
        }
        if (!append_blob(payload, sizeof(payload), &pos, minion_blob,
                         minion_pos)) {
            return 0;
        }
    }

    return dm2_sl_save(save_root, slot, name, payload, pos) == 0;
}

int main(void) {
#ifndef _WIN32
    signal(SIGALRM, alarm_handler);
    alarm(20);
#endif
    const char* data_dir = dm2_data_dir();
    DM2_V1_BootProfile preflight;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    M11_BootProbeReceipt boot_receipt;
    DM2_V1_BootStartupHostViewReceipt host_view_receipt;
    DM2_V1_BootStartupPackagedFullStartReceipt full_start_package;
    DM2_V1_BootStartupPackagedConsumerReceipt consumer_receipt;
    DM2_V1_BootStartupHostFrameReceipt host_frame_receipt;
    DM2_V1_BootStartupRenderOwnershipReceipt ownership_receipt;
    DM2_V1_BootStartupRealVisualCaptureReceipt real_visual_capture;
    DM2_V1_ViewportM11FrameReceipt dm2_frame_receipt;
    DM2_V1_BootProfile* profile;
    DM2_V1_BootChampionDyn4Receipt champion_dyn4;
    DM2_V1_BootChampionDyn4RosterReceipt champion_dyn4_roster;
    DM2_V1_G1ChampionMirrorReceipt champion_mirrors;
    DM2_V1_BootChampionSelectionCandidate champion_candidate;
    DM2_V1_BootNewGameChampionAdmissionReceipt champion_admission;
    DM2_V1_BootNewGameFirstChampionReceipt first_champion;
    DM2_V1_BootNewGamePartySelection party_selections[2];
    DM2_V1_BootNewGamePartyReceipt source_party;
    DM2_V1_BootNewGamePossessionReceipt source_possessions;
    DM2_V1_BootNewGameTransactionReceipt source_transaction;
    DM2_V1_GameLoadWorldOwner new_game_world_owner;
    DM2_V1_GameLoadWorldOwner incremental_new_game_world_owner;
    DM2_V1_GameLoadWorldOwner tampered_new_game_world_owner;
    DM2_V1_GameLoadWorldOwner timer_process_world_owner;
    const DM2_V1_GameLoadWorldOwner *profile_new_game_owner;
    DM2_V1_GameLoadActuatorGeneratorReceipt new_game_generators;
    DM2_V1_GameLoadActuateReceipt new_game_actuate;
    DM2_V1_TimerEntry new_game_actuate_timer;
    DM2_V1_GameLoadDoorStepReceipt new_game_door_step;
    DM2_V1_GameLoadTimerProcessReceipt new_game_timer_process;
    DM2_V1_TimerEntry new_game_door_step_timer;
    int new_game_generators_result;
    int new_game_owner_initialized;
    int new_game_noop_map = -1;
    int new_game_noop_x = -1;
    int new_game_noop_y = -1;
    int new_game_floor_map = -1;
    int new_game_floor_x = -1;
    int new_game_floor_y = -1;
    int new_game_pit_map = -1;
    int new_game_pit_x = -1;
    int new_game_pit_y = -1;
    int new_game_pit_raw_before = -1;
    int new_game_tele_map = -1;
    int new_game_tele_x = -1;
    int new_game_tele_y = -1;
    int new_game_tele_raw_before = -1;
    int new_game_db2_floor_map = -1;
    int new_game_db2_floor_x = -1;
    int new_game_db2_floor_y = -1;
    int16_t new_game_db2_floor_link = DM2_V1_RECORD_HANDLE_NULL;
    int new_game_db2_pit_map = -1;
    int new_game_db2_pit_x = -1;
    int new_game_db2_pit_y = -1;
    int16_t new_game_db2_pit_link = DM2_V1_RECORD_HANDLE_NULL;
    int new_game_door_map = -1;
    int new_game_door_x = -1;
    int new_game_door_y = -1;
    int16_t new_game_door_link = DM2_V1_RECORD_HANDLE_NULL;
    uint8_t new_game_door_direction = 0u;
    int new_game_door_found = 0;
    int new_game_push_button_map = -1;
    int new_game_push_button_x = -1;
    int new_game_push_button_y = -1;
    int new_game_push_button_found = 0;
    int new_game_cross_map_map = -1;
    int new_game_cross_map_x = -1;
    int new_game_cross_map_y = -1;
    uint8_t new_game_cross_map_direction = 0u;
    int new_game_cross_map_target_map = -1;
    int new_game_cross_map_target_x = -1;
    int new_game_cross_map_target_y = -1;
    uint8_t new_game_cross_map_target_direction = 0u;
    uint8_t new_game_expected_wall_gfx[16];
    uint8_t new_game_expected_floor_gfx[16];
    uint8_t new_game_expected_door_ornate_gfx[16];
    int new_game_expected_wall_count = -1;
    int new_game_expected_floor_count = -1;
    int new_game_expected_door_ornate_count = -1;
    int new_game_cross_map_count = 0;
    int new_game_cross_map_found = 0;
    int new_game_counter_map = -1;
    int new_game_counter_x = -1;
    int new_game_counter_y = -1;
    uint8_t new_game_counter_direction = 0u;
    int new_game_counter_count = 0;
    int new_game_counter_found = 0;
    int new_game_relay_map = -1;
    int new_game_relay_x = -1;
    int new_game_relay_y = -1;
    uint8_t new_game_relay_direction = 0u;
    uint8_t new_game_relay_action = 0u;
    int new_game_relay_count = 0;
    int new_game_relay_messages = 0;
    int new_game_relay_found = 0;
    int new_game_finite_relay_map = -1;
    int new_game_finite_relay_x = -1;
    int new_game_finite_relay_y = -1;
    uint8_t new_game_finite_relay_direction = 0u;
    int new_game_finite_relay_count = 0;
    int new_game_finite_relay_found = 0;
    DM2_V1_BootChampionSelectionCensus champion_census;
    DM2_V1_FileHeaderRuntimeMapReceipt file_header_map;
    DM2_V1_FileHeaderWorldInteractionReceipt file_header_world;
    DM2_V1_BootNewGameEntranceReceipt new_game_entrance;
    DM2_V1_G1RuntimeMapDoorReceipt file_header_doors;
    DM2_V1_FileHeaderRuntimeTeleporterReceipt file_header_teleporters;
    DM2_V1_G1RuntimeMapActuatorReceipt file_header_actuators;
    DM2_V1_FileHeaderRuntimeSceneCensus file_header_census;
    DM2_V1_FileHeaderRuntimeTileCensus file_header_tiles;
    unsigned char framebuffer[320 * 200];
    unsigned char framebuffer_without_hand[320 * 200] __attribute__((unused));
    char direct_save_root[512] __attribute__((unused)) = {0};
    char direct_save_path[512] __attribute__((unused)) = {0};
    char save_root[512];
    char save_path[512];
    DM2_V1_SessionState direct_session __attribute__((unused));
    DM2_V1_SessionState resume_session __attribute__((unused));
    uint32_t loadable_icon_handle __attribute__((unused)) = 0u;
    uint32_t raw_hash = 0xFFFFFFFFu;
    uint32_t raw_byte_count = 0xFFFFFFFFu;
    uint32_t title_raw_hash = 0u;
    uint32_t title_raw_byte_count = 0u;
    uint32_t menu_raw_hash = 0u;
    uint32_t menu_raw_byte_count = 0u;
    uint32_t champion_selection_identity_zero = 0u;
    uint32_t source_dungeon_hash_before_owner = 0u;
    const uint8_t *source_dungeon_bytes_before_owner = NULL;
    int source_dungeon_size_before_owner = 0;
    const uint8_t *source_sound_bytes_before_owner = NULL;
    size_t source_sound_size_before_owner = 0u;

    expect_dm2_startup_layout_contract();
    expect_true(!dm2_v1_boot_gdat_raw_asset_proof(NULL,
                                                   5,
                                                   0,
                                                   1,
                                                   0x32545257u,
                                                   &raw_hash,
                                                   &raw_byte_count) &&
                    raw_hash == 0u && raw_byte_count == 0u,
                "DM2 M11 raw GDAT proof rejects an unowned byte range");

    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when GRAPHICS.DAT is present without DUNGEON.DAT",
        1, 0);
    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when DUNGEON.DAT is present without GRAPHICS.DAT",
        0, 1);
    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when required filenames exist but hashes are unknown",
        1, 1);

    if (!data_dir || !data_dir[0]) {
        puts("skip: no DM2 data directory configured");
        return g_failures == 0 ? 0 : 1;
    }

    dm2_v1_boot_profile_init(&preflight);
    if (dm2_v1_boot_scan_assets(&preflight, data_dir) != 0 ||
        !preflight.assets_verified) {
        dm2_v1_boot_cleanup(&preflight);
        if (dm2_data_dir_is_explicit()) {
            fprintf(stderr,
                    "FAIL: explicit DM2 data directory is not a hash-verified V1 corpus: %s\n",
                    data_dir);
            return 1;
        }
        printf("skip: no hash-verified DM2 V1 profile at %s\n", data_dir);
        return g_failures == 0 ? 0 : 1;
    }

    expect_m12_dm2_verified_launch(data_dir, preflight.asset_root);

    fill_dm2_launch_spec(&spec, data_dir);

    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 verified-profile start succeeds");
    expect_true(view.active == 1, "M11 view is active");
    expect_true(view.startedFromLauncher == 1, "M11 marks launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_DM2_BOOT,
                "M11 source kind is DM2 boot");
    expect_true(strcmp(view.sourceId, "dm2") == 0,
                "M11 source id is dm2");
    expect_true(view.dm2BootProfile != NULL,
                "M11 stores a DM2 boot profile");
    expect_true(view.dm2World != NULL,
                "M11 stores the DM2 V1 world pointer");
    expect_true(view.dm2State.level_loaded == 0,
                "M11 DM2 keeps parsed File_header data out of live gameplay before GAME_LOAD");
    {
        DM2_V1_SessionState new_game_session;
        memset(&new_game_session, 0xA5, sizeof(new_game_session));
        expect_true(dm2_v1_new_game_flow(&new_game_session,
                                         view.dm2BootProfile) ==
                        DM2_FLOW_GAME_LOAD_REQUIRED &&
                        ((const unsigned char *)&new_game_session)[0] == 0xA5,
                    "M11-mounted original DM2 data reaches GAME_LOAD without a synthetic New Game session");
    }
    expect_true(dm2_v1_sound_playback_backend_bound(),
                "M11 DM2 binds playback only after the verified boot profile");
    /* File_header::w8 is 0x0101, directly yielding the map-0 (1,8,0)
     * pose. Do not retain the former +10 fixture word as an entrance.
     * SKProject SkWinCore.cpp:39936-39943. */
    expect_true(view.dm2State.party_x == 0 && view.dm2State.party_y == 0 &&
                view.dm2State.party_dir == 0 && view.dm2State.tick_count == 0,
                "M11 DM2 does not expose a source pose as a live party before GAME_LOAD");
    expect_true(dm2_v1_runtime_has_dungeon_data() == 1,
                "DM2 V1 runtime singleton has the boot profile");
    expect_true(dm2_v1_runtime_get_party_x() == 1 &&
                dm2_v1_runtime_get_party_y() == 8 &&
                dm2_v1_runtime_get_party_dir() == 0,
                "DM2 V1 parser retains the File_header pose without publishing a live M11 party");
    expect_true(dm2_v1_runtime_get_tick_count() == 0,
                "DM2 V1 runtime tick counter starts at zero");
    expect_true(view.dm2State.startup_menu_active == 1,
                "M11 DM2 no-save launch shows the startup menu");
    expect_true(view.dm2State.startup_menu_row_count >= 1,
                "M11 DM2 startup menu exposes at least NEW GAME");
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                    boot_receipt.startupActive == 1 &&
                    strcmp(boot_receipt.startupPhase,
                           "dm2-startup-menu") == 0 &&
                    strcmp(boot_receipt.startupAnimation,
                           "dm2-startup-menu") == 0 &&
                    boot_receipt.startupAnimationActive == 1 &&
                    boot_receipt.startupTitleFrame == 0 &&
                    boot_receipt.startupTitleFrameMax == 0 &&
                    boot_receipt.startupTitleReady == 1 &&
                    boot_receipt.startupInitializeV2Runtime == 1 &&
                    boot_receipt.startupInitializeHudRuntime == 1 &&
                    boot_receipt.startupInitializeTouchRuntime == 1 &&
                    boot_receipt.startupHudRuntimeReady == 1,
                "M11 DM2 boot probe consumes startup view receipt handoff");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_NONE) ==
                    M11_GAME_INPUT_IGNORED,
                "M11 DM2 no-save startup menu ignores idle input");
    expect_true(view.dm2State.startup_menu_active == 1 &&
                view.dm2State.tick_count == 0,
                "M11 DM2 no-save startup menu does not enter runtime before selection");
    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_IGNORED,
                "M11 DM2 no-save startup menu blocks idle runtime tick");
    expect_true(view.dm2State.startup_menu_active == 1 &&
                view.dm2State.tick_count == 0 &&
                view.dm2State.startup_title_animation_tick == 0 &&
                dm2_v1_runtime_get_tick_count() == 0,
                "M11 DM2 static startup menu keeps runtime tick frozen");
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                    boot_receipt.startupActive == 1 &&
                    boot_receipt.startupTitleFrame == 0 &&
                    boot_receipt.startupTitleFrameMax == 0 &&
                    boot_receipt.startupTitleReady == 1,
                "M11 DM2 startup menu is immediately static and ready");
    (void)M11_GameView_AdvanceIdleTick(&view);
    expect_true(view.dm2State.startup_title_animation_tick == 0,
                "M11 DM2 startup never manufactures a title tick");
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    expect_true(profile != NULL && profile->deterministic.max_levels == 44u &&
                    profile->deterministic.dungeon_seed == 0u,
                "M11 DM2 launch retains the PC-DOS File_header map count and seed");
    expect_true(profile != NULL && profile->dos_startup_media_verified &&
                    profile->dos_startup_media.valid &&
                    profile->dos_startup_media.intro_has_interplay_mve &&
                    profile->dos_startup_media.end_has_interplay_mve,
                "M11 retains the selected PC-DOS IBMIOP/MVE startup receipt without substituting GDAT");
    {
        DM2_V1_BootRuntimeReceipt blocked_runtime;
        DM2_V1_BootRuntimeActionReceipt blocked_action;
        expect_true(profile &&
                        !dm2_v1_boot_runtime_tick(profile, &blocked_runtime) &&
                        blocked_runtime.operation_result == -1 &&
                        !dm2_v1_boot_runtime_turn(profile, 1, &blocked_runtime) &&
                        blocked_runtime.operation_result == -1 &&
                        !dm2_v1_boot_runtime_move(profile, 0, &blocked_runtime) &&
                        blocked_runtime.operation_result == -1 &&
                        !dm2_v1_boot_runtime_action_front_cell(
                            profile, 0, &blocked_action) &&
                        dm2_v1_runtime_get_party_x() == 1 &&
                        dm2_v1_runtime_get_party_y() == 8 &&
                        dm2_v1_runtime_get_tick_count() == 0,
                    "DM2 mounted File_header cannot mutate gameplay before original GAME_LOAD owns a session");
    }
    dm2_v1_runtime_tick();
    expect_true(dm2_v1_runtime_get_tick_count() == 0,
                "DM2 mounted File_header cannot advance runtime time before GAME_LOAD");
    memset(&champion_mirrors, 0, sizeof(champion_mirrors));
    expect_true(profile &&
                    dm2_v1_boot_champion_mirror_receipt(
                        profile, &champion_mirrors) &&
                    champion_mirrors.committed &&
                    champion_mirrors.incomplete_world &&
                    champion_mirrors.mirror_count == 16 &&
                    champion_mirrors.mirrors[0].dynamic_hero_type == 14u &&
                    champion_mirrors.mirrors[0].dynamic_load_id == 0x160effffu,
                "M11 retains the authentic PC-DOS champion mirror roots without activating them");
    memset(&champion_candidate, 0, sizeof(champion_candidate));
    expect_true(profile && champion_mirrors.mirror_count > 0 &&
                    dm2_v1_boot_champion_selection_candidate(
                        profile, champion_mirrors.mirrors[0].map,
                        champion_mirrors.mirrors[0].x,
                        champion_mirrors.mirrors[0].y,
                        0,
                        &champion_candidate) &&
                    champion_candidate.valid &&
                    champion_candidate.incomplete_game_load &&
                    champion_candidate.mirror.object_id ==
                        champion_mirrors.mirrors[0].object_id &&
                    champion_candidate.selection_direction == 0 &&
                    champion_candidate.revive_data.valid &&
                    champion_candidate.revive_data.hero_type ==
                        champion_mirrors.mirrors[0].dynamic_hero_type &&
                    champion_candidate.revive_data.name1[0] != '\0' &&
                    champion_candidate.source_item_count >= 0 &&
                    champion_candidate.source_item_count <= 30 &&
                    champion_candidate.source_item_link_word_reads ==
                        champion_candidate.source_item_count &&
                    champion_candidate.source_item_chain_hash != 0u &&
                    (champion_candidate.source_item_count == 0 ||
                     (champion_candidate.source_item_records[0].object_id ==
                          champion_candidate.source_item_object_ids[0] &&
                      champion_candidate.source_item_records[0].type > 3u)) &&
                    champion_candidate.identity_hash != 0u,
                "M11 joins each source mirror to its authentic champion template and File_header-owned source tile chain");
    champion_selection_identity_zero = champion_candidate.identity_hash;
    memset(&champion_admission, 0, sizeof(champion_admission));
    expect_true(profile && champion_mirrors.mirror_count > 0 &&
                    dm2_v1_boot_new_game_champion_admission_receipt(
                        profile, champion_mirrors.mirrors[0].map,
                        champion_mirrors.mirrors[0].x,
                        champion_mirrors.mirrors[0].y,
                        0,
                        &champion_admission) &&
                    champion_admission.valid &&
                    champion_admission.incomplete_game_load &&
                    champion_admission.entrance.valid &&
                    champion_admission.selection.valid &&
                    champion_admission.selection.mirror.object_id ==
                        champion_mirrors.mirrors[0].object_id &&
                    champion_admission.dyn4.raw_loadable_entry_count > 0u &&
                    champion_admission.receipt_hash != 0u,
                "M11 joins New Game entrance, selected mirror, source items and DYN4 without creating a party");
    memset(&first_champion, 0, sizeof(first_champion));
    expect_true(profile && champion_mirrors.mirror_count > 0 &&
                    dm2_v1_boot_new_game_first_champion_receipt(
                        profile, champion_mirrors.mirrors[0].map,
                        champion_mirrors.mirrors[0].x,
                        champion_mirrors.mirrors[0].y,
                        0,
                        &first_champion) &&
                    first_champion.valid &&
                    first_champion.incomplete_game_load &&
                    first_champion.admission.valid &&
                    first_champion.hero.herotype ==
                        (int8_t)champion_mirrors.mirrors[0].dynamic_hero_type &&
                    first_champion.hero.handcmd[0] == -1 &&
                    first_champion.hero.handcmd[1] == -1 &&
                    first_champion.hero.timeridx == -1 &&
                    first_champion.hero.item[0] == -1 &&
                    first_champion.hero.food >= 1500 &&
                    first_champion.hero.food <= 1755 &&
                    first_champion.hero.water >= 1500 &&
                    first_champion.hero.water <= 1755 &&
                    first_champion.source_rng_state_before == 0u &&
                    first_champion.source_rng_state_after != 0u &&
                    first_champion.hero_hash != 0u &&
                    first_champion.receipt_hash != 0u,
                "M11 materializes the first source c_hero candidate without publishing a party");
    memset(party_selections, 0, sizeof(party_selections));
    memset(&source_party, 0, sizeof(source_party));
    if (champion_mirrors.mirror_count >= 2) {
        int party_selection_index;
        for (party_selection_index = 0; party_selection_index < 2;
             ++party_selection_index) {
            party_selections[party_selection_index].map =
                champion_mirrors.mirrors[party_selection_index].map;
            party_selections[party_selection_index].x =
                champion_mirrors.mirrors[party_selection_index].x;
            party_selections[party_selection_index].y =
                champion_mirrors.mirrors[party_selection_index].y;
            party_selections[party_selection_index].direction =
                party_selection_index;
            party_selections[party_selection_index].mirror_object_id =
                champion_mirrors.mirrors[party_selection_index].object_id;
        }
    }
    expect_true(profile && champion_mirrors.mirror_count >= 2 &&
                    dm2_v1_boot_new_game_party_receipt(profile,
                        party_selections, 2, &source_party) &&
                    source_party.valid && source_party.incomplete_game_load &&
                    source_party.hero_count == 2 &&
                    source_party.party.heros_in_party == 2 &&
                    source_party.party.hero[0].herotype ==
                        (int8_t)source_party.admissions[0].selection
                            .revive_data.hero_type &&
                    source_party.party.hero[0].partypos !=
                        source_party.party.hero[1].partypos &&
                    source_party.party.hero[0].food == first_champion.hero.food &&
                    source_party.party.hero[0].water == first_champion.hero.water &&
                    source_party.source_rng_state_before == 0u &&
                    source_party.source_rng_state_after != 0u &&
                    source_party.party_hash != 0u &&
                    source_party.receipt_hash != 0u,
                "M11 preserves source champion click order and RNG across a read-only party receipt");
    memset(&source_possessions, 0, sizeof(source_possessions));
    expect_true(profile && source_party.valid &&
                    dm2_v1_boot_new_game_possession_receipt(profile,
                        &source_party, &source_possessions) &&
                    source_possessions.valid &&
                    source_possessions.incomplete_game_load &&
                    source_possessions.hero_count == source_party.hero_count &&
                    source_possessions.source_item_count ==
                        source_party.admissions[0].selection.source_item_count +
                        source_party.admissions[1].selection.source_item_count &&
                    source_possessions.source_item_count ==
                        source_possessions.placed_item_count &&
                    source_possessions.unplaced_item_count == 0 &&
                    (source_possessions.source_item_count == 0 ||
                     (source_possessions.possessions[0].source_object_id != 0u &&
                      source_possessions.possessions[0].equipped_record_id ==
                          (source_possessions.possessions[0].source_object_id & 0x3fffu) &&
                      source_possessions.possessions[0].fit_receipt_hash != 0u)) &&
                    source_possessions.source_chain_hash != 0u &&
                    source_possessions.projection_hash != 0u &&
                    source_possessions.receipt_hash != 0u &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 preserves the exact original initial inventories through GDAT slots without publishing a live party");
    memset(&source_transaction, 0, sizeof(source_transaction));
    expect_true(profile &&
                    dm2_v1_boot_new_game_transaction_receipt(profile,
                        party_selections, 2, &source_transaction) &&
                    source_transaction.valid &&
                    source_transaction.incomplete_game_load &&
                    source_transaction.hero_count == 2 &&
                    source_transaction.entrance.valid &&
                    source_transaction.world_interactions.valid &&
                    source_transaction.world_interactions.map_count == 44 &&
                    source_transaction.world_interactions.interaction_hash != 0u &&
                    source_transaction.actuator_generators.valid &&
                    source_transaction.actuator_generators.incomplete_game_load &&
                    source_transaction.actuator_generators.map_count ==
                        source_transaction.world_interactions.map_count &&
                    source_transaction.actuator_generators.actuator_count ==
                        source_transaction.world_interactions.total_actuators &&
                    source_transaction.actuator_generators.tick_generator_candidate_count > 0 &&
                    source_transaction.actuator_generators.control_bit2_set_count +
                        source_transaction.actuator_generators.control_bit2_clear_count ==
                        source_transaction.actuator_generators.tick_generator_candidate_count &&
                    source_transaction.actuator_generators.candidate_hash != 0u &&
                    source_transaction.entrance_map.committed &&
                    source_transaction.entrance_map.map ==
                        source_transaction.entrance.map &&
                    source_transaction.entrance_map.map_data_hash ==
                        source_transaction.entrance.map_data_hash &&
                    source_transaction.entrance_scene.committed &&
                    source_transaction.entrance_scene.record_count ==
                        source_transaction.entrance_map.record_count &&
                    source_transaction.entrance_tiles.committed &&
                    source_transaction.entrance_tiles.tile_count ==
                        source_transaction.entrance_map.width *
                        source_transaction.entrance_map.height &&
                    source_transaction.entrance_objects.committed &&
                    source_transaction.entrance_objects.object_record_reads ==
                        source_transaction.entrance_objects.object_record_count &&
                    source_transaction.entrance_texts.committed &&
                    source_transaction.entrance_texts.text_record_reads ==
                        source_transaction.entrance_texts.text_record_count &&
                    source_transaction.entrance_doors.committed &&
                    source_transaction.entrance_doors.door_record_reads ==
                        source_transaction.entrance_doors.door_root_count &&
                    source_transaction.entrance_teleporters.committed &&
                    source_transaction.entrance_teleporters.teleporter_record_reads ==
                        source_transaction.entrance_teleporters.teleporter_root_count &&
                    source_transaction.entrance_actuators.committed &&
                    source_transaction.entrance_actuators.actuator_record_reads ==
                        source_transaction.entrance_actuators.actuator_root_count &&
                    source_transaction.dyn4_roster.valid &&
                    source_transaction.dyn4_roster.selector_count ==
                        champion_mirrors.mirror_count &&
                    source_transaction.party.receipt_hash ==
                        source_party.receipt_hash &&
                    source_transaction.possessions.receipt_hash ==
                        source_possessions.receipt_hash &&
                    source_transaction.transaction_hash != 0u &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 joins entrance interactions, raw actuator-generator inputs, map, DYN4 roster, selected heroes and source possessions before any live GAME_LOAD publication");
    memset(&new_game_world_owner, 0, sizeof(new_game_world_owner));
    memset(&incremental_new_game_world_owner, 0,
           sizeof(incremental_new_game_world_owner));
    memset(&tampered_new_game_world_owner, 0,
           sizeof(tampered_new_game_world_owner));
    memset(&timer_process_world_owner, 0, sizeof(timer_process_world_owner));
    if (profile && profile->dungeon_data) {
        const DM2_V1_DungeonData *source_dungeon =
            (const DM2_V1_DungeonData *)profile->dungeon_data;
        source_dungeon_bytes_before_owner = source_dungeon->raw_data;
        source_dungeon_size_before_owner = source_dungeon->raw_size;
        source_dungeon_hash_before_owner = dm2_test_fnv1a(
            source_dungeon->raw_data, (size_t)source_dungeon->raw_size);
    }
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_prepare_new_game(
                        &incremental_new_game_world_owner, profile) &&
                    dm2_v1_game_load_world_owner_process_actuator_tick_generators(
                        &incremental_new_game_world_owner, NULL) &&
                    dm2_v1_game_load_world_owner_materialize_source_map_context(
                        &incremental_new_game_world_owner) &&
                    dm2_v1_game_load_world_owner_select_champion(
                        &incremental_new_game_world_owner,
                        &party_selections[0]) &&
                    incremental_new_game_world_owner.selected_mirror_count == 1u &&
                    incremental_new_game_world_owner.selected_party.heros_in_party == 1 &&
                    incremental_new_game_world_owner.champion_selection_materialized &&
                    dm2_v1_game_load_world_owner_select_champion(
                        &incremental_new_game_world_owner,
                        &party_selections[1]) &&
                    incremental_new_game_world_owner.selected_mirror_count == 2u &&
                    incremental_new_game_world_owner.selected_party.heros_in_party == 2 &&
                    incremental_new_game_world_owner.source_next_champion_number == 2 &&
                    !dm2_v1_game_load_world_owner_select_champion(
                        &incremental_new_game_world_owner,
                        &party_selections[1]) &&
                    !incremental_new_game_world_owner.committed &&
                    !profile->source_game_load_session_ready,
                "DM2 keeps the original mirror click order in a private GAME_LOAD owner");
    new_game_owner_initialized = profile &&
        dm2_v1_game_load_world_owner_init_new_game(
            &new_game_world_owner, profile, party_selections, 2);
    if (new_game_owner_initialized &&
        new_game_world_owner.sound_owner.valid &&
        new_game_world_owner.sound_owner.sample_binding_count > 0u) {
        source_sound_bytes_before_owner = dm2_v1_load_gdat_raw_data(
            dm2_v1_boot_asset_loader(profile),
            new_game_world_owner.sound_owner.sample_bindings[0].raw_index,
            &source_sound_size_before_owner);
    }
    expect_true(profile && new_game_owner_initialized &&
                    dm2_v1_game_load_world_owner_is_prepared(
                        &new_game_world_owner) &&
                    !new_game_world_owner.committed &&
                    new_game_world_owner.load_new_dungeon_reset.valid &&
                    new_game_world_owner.load_new_dungeon_reset.party_count == 0 &&
                    new_game_world_owner.load_new_dungeon_reset.leader_hand_record ==
                        DM2_V1_RECORD_HANDLE_NULL &&
                    new_game_world_owner.load_new_dungeon_reset.save_stream_bytes_consumed == 0u &&
                    new_game_world_owner.load_new_dungeon_reset.receipt_hash != 0u &&
                    new_game_world_owner.current_map == 0 &&
                    new_game_world_owner.source_transaction_hash ==
                        source_transaction.transaction_hash &&
                    new_game_world_owner.dungeon.level_count == 44 &&
                    new_game_world_owner.dungeon.initial_party_pose_valid &&
                    new_game_world_owner.dungeon.initial_party_x == 1 &&
                    new_game_world_owner.dungeon.initial_party_y == 8 &&
                    new_game_world_owner.dungeon.initial_party_dir == 0 &&
                    new_game_world_owner.record_pools.valid &&
                    new_game_world_owner.record_pools.record_graph_complete &&
                    new_game_world_owner.validated_map_count == 44u &&
                    new_game_world_owner.validated_world_hash != 0u &&
                    new_game_world_owner.dyn4_materialized &&
                    new_game_world_owner.dyn4_selector_count ==
                        source_transaction.dyn4_roster.selector_count &&
                    new_game_world_owner.dyn4_materialized_hash != 0u &&
                    new_game_world_owner.dyn4_selections[0].valid &&
                    new_game_world_owner.dyn4_selections[0].block_count > 0u &&
                    new_game_world_owner.sound_owner.valid &&
                    new_game_world_owner.sound_owner.allocation.accepted &&
                    new_game_world_owner.sound_owner.allocation.sound_entry_count ==
                        292u &&
                    new_game_world_owner.sound_owner.allocation.unique_raw_index_count ==
                        107u &&
                    new_game_world_owner.sound_owner.queue_capacity ==
                        new_game_world_owner.sound_owner.allocation.sound_entry_count &&
                    new_game_world_owner.sound_owner.sample_capacity ==
                        new_game_world_owner.sound_owner.allocation.unique_raw_index_count &&
                    new_game_world_owner.sound_owner.queue_entry_count > 0u &&
                    new_game_world_owner.sound_owner.sample_binding_count > 0u &&
                    new_game_world_owner.sound_owner.queue_entries != NULL &&
                    new_game_world_owner.sound_owner.sample_bindings != NULL &&
                    new_game_world_owner.sound_owner.queue_entries[0].w_00 >= 0 &&
                    new_game_world_owner.sound_owner.queue_entries[0].w_05 >= 0 &&
                    new_game_world_owner.sound_owner.sample_bindings[0].raw_length > 0u &&
                    new_game_world_owner.sound_owner.sample_bindings[0].source_payload_hash !=
                        0u &&
                    source_sound_bytes_before_owner != NULL &&
                    source_sound_size_before_owner ==
                        new_game_world_owner.sound_owner.sample_bindings[0].raw_length &&
                    dm2_test_fnv1a(source_sound_bytes_before_owner,
                        source_sound_size_before_owner) ==
                        new_game_world_owner.sound_owner.sample_bindings[0].source_payload_hash &&
                    new_game_world_owner.sound_owner.receipt_hash != 0u &&
                    new_game_world_owner.timer_capacity > 50u &&
                    new_game_world_owner.timer_queue.max_timers ==
                        (int16_t)new_game_world_owner.timer_capacity &&
                    new_game_world_owner.timer_queue.num_timers == 0 &&
                    new_game_world_owner.timer_queue.gametick == 0 &&
                    new_game_world_owner.fresh_game_mode &&
                    new_game_world_owner.record_capacities[4] >=
                        new_game_world_owner.dungeon.thing_type_counts[4] &&
                    new_game_world_owner.record_capacities[14] >=
                        new_game_world_owner.dungeon.thing_type_counts[14] &&
                    new_game_world_owner.record_capacities[15] >=
                        new_game_world_owner.dungeon.thing_type_counts[15] &&
                    !new_game_world_owner.champion_selection_materialized &&
                    new_game_world_owner.selected_party.heros_in_party == 0 &&
                    source_dungeon_bytes_before_owner ==
                        ((const DM2_V1_DungeonData *)profile->dungeon_data)->raw_data &&
                    source_dungeon_size_before_owner ==
                        ((const DM2_V1_DungeonData *)profile->dungeon_data)->raw_size &&
                    source_dungeon_hash_before_owner == dm2_test_fnv1a(
                        ((const DM2_V1_DungeonData *)profile->dungeon_data)->raw_data,
                        (size_t)((const DM2_V1_DungeonData *)profile->dungeon_data)->raw_size) &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 materializes an isolated File_header, DYN4 sound allocation and DB-pool world from original bytes without publishing a party, timer or playback session");
    memset(&new_game_generators, 0, sizeof(new_game_generators));
    new_game_generators_result = profile &&
        dm2_v1_game_load_world_owner_process_actuator_tick_generators(
            &new_game_world_owner, &new_game_generators);
    expect_true(profile &&
                    new_game_generators_result &&
                    new_game_generators.valid &&
                    new_game_generators.candidate_count ==
                        source_transaction.actuator_generators.tick_generator_candidate_count &&
                    new_game_generators.control_bit2_clear_count ==
                        source_transaction.actuator_generators.control_bit2_clear_count &&
                    new_game_generators.activation_count ==
                        source_transaction.actuator_generators.control_bit2_set_count &&
                    new_game_generators.queued_timer_count == 0 &&
                    new_game_world_owner.timer_queue.num_timers ==
                        (int16_t)new_game_generators.queued_timer_count &&
                    new_game_world_owner.actuator_generators_processed &&
                    !new_game_world_owner.source_map_context_materialized &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 runs the original fresh-game actuator-generator pass only in the private File_header/c_tim owner");
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_source_map_context(
                        &new_game_world_owner) &&
                    new_game_world_owner.source_map_context_materialized &&
                    new_game_world_owner.current_map == 0 &&
                    new_game_world_owner.source_party_map == 0 &&
                    new_game_world_owner.source_party_x == 1u &&
                    new_game_world_owner.source_party_y == 8u &&
                    new_game_world_owner.source_party_direction == 0u &&
                    new_game_world_owner.source_staircase_flag ==
                        new_game_world_owner.source_display_pose_valid &&
                    new_game_world_owner.source_party_absdir <= 3u &&
                    (!new_game_world_owner.source_display_pose_valid ||
                     (new_game_world_owner.source_teleporter_probe_direction >= -1 &&
                      new_game_world_owner.source_teleporter_probe_direction <= 3 &&
                      new_game_world_owner.source_teleporter_source_direction <= 3u &&
                      new_game_world_owner.source_teleporter_destination_direction <= 3u &&
                      new_game_world_owner.source_party_absdir ==
                          (uint8_t)((new_game_world_owner
                                        .source_teleporter_destination_direction +
                                     6 - new_game_world_owner
                                             .source_teleporter_source_direction +
                                     new_game_world_owner.source_party_direction) & 3))) &&
                    new_game_world_owner.source_last_moved_record == -1 &&
                    (!new_game_world_owner.source_display_pose_valid ||
                     (new_game_world_owner.source_teleporter_map >= 0 &&
                      new_game_world_owner.source_teleporter_map <
                          new_game_world_owner.dungeon.level_count)) &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 restores the private New Game map and teleporter context from the real File_header only after actuator generators");
    memset(new_game_expected_wall_gfx, 0, sizeof(new_game_expected_wall_gfx));
    memset(new_game_expected_floor_gfx, 0, sizeof(new_game_expected_floor_gfx));
    memset(new_game_expected_door_ornate_gfx, 0,
           sizeof(new_game_expected_door_ornate_gfx));
    new_game_expected_wall_count = dm2_v1_dungeon_get_map_wall_gfx_list(
        &new_game_world_owner.dungeon, 0, new_game_expected_wall_gfx,
        (int)sizeof(new_game_expected_wall_gfx));
    new_game_expected_floor_count = dm2_v1_dungeon_get_map_floor_gfx_list(
        &new_game_world_owner.dungeon, 0, new_game_expected_floor_gfx,
        (int)sizeof(new_game_expected_floor_gfx));
    new_game_expected_door_ornate_count =
        dm2_v1_dungeon_get_map_door_ornate_list(
            &new_game_world_owner.dungeon, 0,
            new_game_expected_door_ornate_gfx,
            (int)sizeof(new_game_expected_door_ornate_gfx));
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_preselection_local_graphics(
                        &new_game_world_owner) &&
                    new_game_expected_wall_count >= 0 &&
                    new_game_expected_floor_count >= 0 &&
                    new_game_expected_door_ornate_count >= 0 &&
                    new_game_world_owner.preselection_local_graphics.valid &&
                    new_game_world_owner.preselection_local_graphics.map == 0 &&
                    new_game_world_owner.preselection_local_graphics.wall_count ==
                        (uint8_t)new_game_expected_wall_count &&
                    new_game_world_owner.preselection_local_graphics.floor_count ==
                        (uint8_t)new_game_expected_floor_count &&
                    new_game_world_owner.preselection_local_graphics.door_ornate_count ==
                        (uint8_t)new_game_expected_door_ornate_count &&
                    memcmp(new_game_world_owner.preselection_local_graphics.wall_gfx,
                           new_game_expected_wall_gfx,
                           sizeof(new_game_expected_wall_gfx)) == 0 &&
                    memcmp(new_game_world_owner.preselection_local_graphics.floor_gfx,
                           new_game_expected_floor_gfx,
                           sizeof(new_game_expected_floor_gfx)) == 0 &&
                    memcmp(new_game_world_owner.preselection_local_graphics.door_ornate_gfx,
                           new_game_expected_door_ornate_gfx,
                           sizeof(new_game_expected_door_ornate_gfx)) == 0 &&
                    new_game_world_owner.preselection_local_graphics.source_list_hash != 0u &&
                    !profile->source_game_load_session_ready,
                "M11 retains the authentic File_header wall, floor and door graphics lists for the entrance map");
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_preselection_light(
                        &new_game_world_owner) &&
                    new_game_world_owner.preselection_light.valid &&
                    new_game_world_owner.preselection_light.map == 0 &&
                    new_game_world_owner.preselection_light.graphicsset ==
                        dm2_v1_dungeon_get_map_graphics_style(
                            &new_game_world_owner.dungeon, 0) &&
                    new_game_world_owner.preselection_light.party_count == 0u &&
                    new_game_world_owner.preselection_light.leader_hand_record ==
                        (uint16_t)DM2_V1_RECORD_HANDLE_NULL &&
                    new_game_world_owner.preselection_light.savegame_light == 0u &&
                    new_game_world_owner.preselection_light.v1e0974 == 0u &&
                    new_game_world_owner.preselection_light.v1e0978 == 0u &&
                    new_game_world_owner.preselection_light.weather_active == 0u &&
                    new_game_world_owner.preselection_light.weather_index == 0u &&
                    new_game_world_owner.preselection_light.weather_delta == 0u &&
                    new_game_world_owner.preselection_light.weather_darkness_active == 0u &&
                    new_game_world_owner.preselection_light.map_descriptor.valid &&
                    !new_game_world_owner.preselection_light.map_descriptor.dynamic_light &&
                    new_game_world_owner.preselection_light.source_state_hash != 0u &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 retains original c_light inputs for the real entrance without inventing a party or viewport");
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_preselection_scene(
                        &new_game_world_owner) &&
                    new_game_world_owner.preselection_scene_materialized &&
                    new_game_world_owner.preselection_scene_plan.valid &&
                    new_game_world_owner.preselection_scene_plan.graphicsset ==
                        new_game_world_owner.preselection_light.graphicsset &&
                    new_game_world_owner.preselection_scene_plan.commands[0].pixels != NULL &&
                    new_game_world_owner.preselection_scene_plan.commands[1].pixels != NULL &&
                    new_game_world_owner.preselection_scene_plan.commands[0].material_receipt_hash != 0u &&
                    new_game_world_owner.preselection_scene_plan.commands[1].material_receipt_hash != 0u &&
                    new_game_world_owner.preselection_scene_light.valid &&
                    new_game_world_owner.preselection_c_light.valid &&
                    new_game_world_owner.preselection_c_light.dynamic_map == 0u &&
                    new_game_world_owner.preselection_c_light.light_level == 1u &&
                    new_game_world_owner.preselection_c_light.source_state_hash ==
                        new_game_world_owner.preselection_light.source_state_hash &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 materializes the original entrance floor, ceiling and c_light receipt only in the private New Game owner");
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_preselection_view(
                        &new_game_world_owner) &&
                    dm2_test_preselection_view_matches_owner(
                        &new_game_world_owner) &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 retains every visible entrance cell from the real File_header map without publishing a viewport");
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_materialize_champion_selection(
                        &new_game_world_owner) &&
                    new_game_world_owner.champion_selection_materialized &&
                    new_game_world_owner.selected_party.heros_in_party == 2 &&
                    new_game_world_owner.champion_item_bonus.valid &&
                    !new_game_world_owner.champion_item_bonus.blocked &&
                    new_game_world_owner.champion_item_bonus.processed_item_roots ==
                        source_transaction.possessions.placed_item_count &&
                    new_game_world_owner.selected_party.hero[0].herotype ==
                        source_transaction.possessions.projected_party.hero[0].herotype &&
                    new_game_world_owner.selected_party.hero[0].item[0] ==
                        source_transaction.possessions.projected_party.hero[0].item[0] &&
                    (new_game_world_owner.selected_party.hero[0].heroflag &
                        0x1000) != 0 &&
                    new_game_world_owner.source_next_champion_number == 2 &&
                    new_game_world_owner.source_event_hero_index == 0 &&
                    new_game_world_owner.champion_selection_receipt.valid &&
                    new_game_world_owner.champion_selection_receipt.click_count == 2u &&
                    new_game_world_owner.champion_selection_receipt.leader_select_count == 1u &&
                    new_game_world_owner.champion_selection_receipt.initial_event_hero_index ==
                        DM2_HERO_NONE &&
                    new_game_world_owner.champion_selection_receipt.final_event_hero_index == 0 &&
                    new_game_world_owner.champion_selection_receipt
                        .next_champion_number_after_click[0] == 1 &&
                    new_game_world_owner.champion_selection_receipt
                        .next_champion_number_after_click[1] == 2 &&
                    new_game_world_owner.champion_selection_receipt
                        .mirror_object_id[0] ==
                        source_transaction.party.admissions[0].selection.mirror.object_id &&
                    new_game_world_owner.champion_selection_receipt
                        .mirror_object_id[1] ==
                        source_transaction.party.admissions[1].selection.mirror.object_id &&
                    new_game_world_owner.champion_selection_receipt.transition_hash != 0u &&
                    new_game_world_owner.selected_party.curactevhero == 0 &&
                    new_game_world_owner.selected_party.absdir ==
                        new_game_world_owner.source_party_absdir &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 retains the source mirror-click order, leader, hero count and possessions without publishing a session");
    expect_true(profile &&
                    !dm2_v1_game_load_world_owner_materialize_champion_selection(
                        &new_game_world_owner),
                "M11 refuses to replay an already materialized source champion selection");
    expect_true(profile &&
                    dm2_v1_boot_retain_new_game_world(profile, party_selections, 2) &&
                    (profile_new_game_owner = (const DM2_V1_GameLoadWorldOwner *)
                        dm2_v1_boot_new_game_world_readonly(profile)) != NULL &&
                    profile_new_game_owner->prepared &&
                    profile_new_game_owner->champion_selection_materialized &&
                    profile_new_game_owner->selected_party.heros_in_party == 2 &&
                    !profile_new_game_owner->committed &&
                    !profile->source_game_load_session_ready,
                "M11 retains the complete source New Game candidate in the boot profile without publishing a session");
    if (profile && source_transaction.possessions.placed_item_count > 0) {
        const DM2_V1_BootNewGamePossession *first_source_item =
            &source_transaction.possessions.possessions[0];
        uint8_t *tampered_item_link = NULL;
        int16_t replacement_next =
            first_source_item->source_next_object_id == 0xfffeu ?
                DM2_V1_RECORD_HANDLE_NULL : DM2_V1_RECORD_HANDLE_END;

        expect_true(dm2_v1_game_load_world_owner_init_new_game(
                        &tampered_new_game_world_owner, profile,
                        party_selections, 2) &&
                    dm2_v1_game_load_world_owner_process_actuator_tick_generators(
                        &tampered_new_game_world_owner, NULL) &&
                    dm2_v1_game_load_world_owner_materialize_source_map_context(
                        &tampered_new_game_world_owner) &&
                    (tampered_item_link = dm2_v1_record_pool_address_mut(
                        &tampered_new_game_world_owner.record_pools,
                        (int16_t)first_source_item->source_object_id)) != NULL,
                    "M11 retains a separately owned real item chain for New Game validation");
        if (tampered_item_link) {
            tampered_item_link[0] = (uint8_t)replacement_next;
            tampered_item_link[1] = (uint8_t)((uint16_t)replacement_next >> 8);
            expect_true(!dm2_v1_game_load_world_owner_materialize_champion_selection(
                            &tampered_new_game_world_owner),
                        "M11 rejects a private New Game inventory whose real tile link diverges");
        }
        dm2_v1_game_load_world_owner_free(&tampered_new_game_world_owner);
    }
    expect_true(profile &&
                    dm2_v1_game_load_world_owner_init_new_game(
                        &tampered_new_game_world_owner, profile,
                        party_selections, 2) &&
                    dm2_v1_game_load_world_owner_process_actuator_tick_generators(
                        &tampered_new_game_world_owner, NULL) &&
                    dm2_v1_game_load_world_owner_materialize_source_map_context(
                        &tampered_new_game_world_owner),
                "M11 prepares a separate source owner for mirror-order validation");
    if (profile && tampered_new_game_world_owner.prepared) {
        tampered_new_game_world_owner.transaction.party.admissions[1]
            .selection.mirror.object_id =
            tampered_new_game_world_owner.transaction.party.admissions[0]
                .selection.mirror.object_id;
        expect_true(!dm2_v1_game_load_world_owner_materialize_champion_selection(
                        &tampered_new_game_world_owner),
                    "M11 rejects a private New Game selection ledger with duplicate mirror roots");
    }
    dm2_v1_game_load_world_owner_free(&tampered_new_game_world_owner);
    memset(&new_game_actuate, 0, sizeof(new_game_actuate));
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    expect_true(profile &&
                    dm2_test_find_file_header_tile_class(
                        &new_game_world_owner.dungeon, 3,
                        &new_game_noop_map, &new_game_noop_x, &new_game_noop_y) &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_noop_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 1u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_noop_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_noop_y), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) &&
                    new_game_actuate.valid && new_game_actuate.source_noop &&
                    !new_game_actuate.blocked_incomplete_chain &&
                    new_game_actuate.tile_class == 3u &&
                    new_game_actuate.map == new_game_noop_map &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 consumes original class-3 ACTUATE messages as source no-ops without publishing a session");
    /* Drive the same real class-3 coordinate through the actual private
     * c_tim heap.  A fresh owner keeps this assertion independent from the
     * generator's real pending timers and proves POP -> CHANGE_MAP -> 0x04
     * dispatch is one transaction rather than a direct-call shortcut. */
    memset(&new_game_timer_process, 0, sizeof(new_game_timer_process));
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    expect_true(profile && new_game_noop_map >= 0 &&
                    dm2_v1_game_load_world_owner_init_new_game(
                        &timer_process_world_owner, profile,
                        party_selections, 2) &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_noop_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 1u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_noop_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_noop_y), 1) &&
                    dm2_v1_timer_queue(&timer_process_world_owner.timer_queue,
                                       &new_game_actuate_timer) >= 0 &&
                    dm2_v1_game_load_world_owner_process_next_due_timer(
                        &timer_process_world_owner, &new_game_timer_process) &&
                    new_game_timer_process.valid &&
                    new_game_timer_process.timer_dequeued &&
                    new_game_timer_process.timer_type == 0x04u &&
                    new_game_timer_process.timer_map == new_game_noop_map &&
                    new_game_timer_process.actuate.source_noop &&
                    timer_process_world_owner.timer_queue.num_timers == 0 &&
                    timer_process_world_owner.current_map == new_game_noop_map &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 consumes a due real-tile 0x04 only through the private source timer heap");
    /* An unavailable source family must leave the exact heap head pending.
     * It is intentionally an adversarial timer transport, not game data. */
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    memset(&new_game_timer_process, 0, sizeof(new_game_timer_process));
    expect_true(timer_process_world_owner.prepared &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_noop_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x05u), 1) &&
                    dm2_v1_timer_queue(&timer_process_world_owner.timer_queue,
                                       &new_game_actuate_timer) >= 0 &&
                    !dm2_v1_game_load_world_owner_process_next_due_timer(
                        &timer_process_world_owner, &new_game_timer_process) &&
                    !new_game_timer_process.valid &&
                    !new_game_timer_process.timer_dequeued &&
                    new_game_timer_process.blocked_unowned_timer &&
                    new_game_timer_process.timer_type == 0x05u &&
                    timer_process_world_owner.timer_queue.num_timers == 1 &&
                    timer_process_world_owner.timer_entries[
                        timer_process_world_owner.timer_queue.indices[0]].ttype == 0x05u,
                "M11 keeps an unowned private timer pending instead of dropping it");
    dm2_v1_game_load_world_owner_free(&timer_process_world_owner);
    memset(&new_game_actuate, 0, sizeof(new_game_actuate));
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    expect_true(profile &&
                    dm2_test_find_file_header_tile_class(
                        &new_game_world_owner.dungeon, 1,
                        &new_game_floor_map, &new_game_floor_x, &new_game_floor_y) &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_floor_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 1u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_floor_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_floor_y), 1) &&
                    !dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) &&
                    !new_game_actuate.valid &&
                    new_game_actuate.blocked_incomplete_chain &&
                    new_game_actuate.tile_class == 1u &&
                    !profile->source_game_load_session_ready &&
                    dm2_v1_runtime_get_tick_count() == 0,
                "M11 refuses a real FLOOR ACTUATE message until its complete source chain has one owner");
    /* Source action 0 opens a PIT and immediately enters
     * ADVANCE_TILES_TIME. It may move source party/creature records, so the
     * private GAME_LOAD owner must reject it before its real tile byte moves. */
    memset(&new_game_actuate, 0, sizeof(new_game_actuate));
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    expect_true(profile &&
                    dm2_test_find_file_header_tile_class(
                        &new_game_world_owner.dungeon, 2,
                        &new_game_pit_map, &new_game_pit_x, &new_game_pit_y) &&
                    ((new_game_pit_raw_before = dm2_v1_dungeon_get_tile_raw(
                        &new_game_world_owner.dungeon, new_game_pit_map,
                        new_game_pit_x, new_game_pit_y)) >= 0) &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_pit_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 1u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_pit_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_pit_y), 1) &&
                    !dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) &&
                    !new_game_actuate.valid &&
                    new_game_actuate.tile_class == 2u &&
                    new_game_actuate.blocked_unowned_tile_advance &&
                    dm2_v1_dungeon_get_tile_raw(&new_game_world_owner.dungeon,
                        new_game_pit_map, new_game_pit_x, new_game_pit_y) ==
                        new_game_pit_raw_before &&
                    !profile->source_game_load_session_ready,
                "M11 rejects an opening PIT ACTUATE before unowned tile-time movement");
    memset(&new_game_actuate, 0, sizeof(new_game_actuate));
    dm2_v1_timer_entry_init(&new_game_actuate_timer);
    expect_true(profile &&
                    dm2_test_find_file_header_tile_class(
                        &new_game_world_owner.dungeon, 5,
                        &new_game_tele_map, &new_game_tele_x, &new_game_tele_y) &&
                    ((new_game_tele_raw_before = dm2_v1_dungeon_get_tile_raw(
                        &new_game_world_owner.dungeon, new_game_tele_map,
                        new_game_tele_x, new_game_tele_y)) >= 0) &&
                    (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                        (int16_t)new_game_tele_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 1u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_tele_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_tele_y), 1) &&
                    !dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) &&
                    !new_game_actuate.valid &&
                    new_game_actuate.tile_class == 5u &&
                    new_game_actuate.blocked_unowned_tile_advance &&
                    dm2_v1_dungeon_get_tile_raw(&new_game_world_owner.dungeon,
                        new_game_tele_map, new_game_tele_x, new_game_tele_y) ==
                        new_game_tele_raw_before &&
                    !profile->source_game_load_session_ready,
                "M11 rejects an opening TELEPORTER ACTUATE before unowned tile-time movement");
    /* The DOS File_header corpus does not promise that a map contains an
     * all-DB2 floor chain.  When it does, prove the private 0x04 atom against
     * that authentic coordinate and source record rather than a fixture. */
    if (profile && dm2_test_find_private_db2_floor(
            &new_game_world_owner, 1, &new_game_db2_floor_map,
            &new_game_db2_floor_x, &new_game_db2_floor_y,
            &new_game_db2_floor_link)) {
        const uint8_t *before = dm2_v1_record_pool_address(
            &new_game_world_owner.record_pools, new_game_db2_floor_link);
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true(before &&
                        (dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_db2_floor_map, 0), 1) &&
                        ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                        ((new_game_actuate_timer.actor = 2u), 1) &&
                        ((new_game_actuate_timer.xA =
                            (int8_t)new_game_db2_floor_x), 1) &&
                        ((new_game_actuate_timer.yA =
                            (int8_t)new_game_db2_floor_y), 1) &&
                        ((new_game_actuate_timer.wvalueB = 2 << 8), 1) &&
                        dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                            &new_game_world_owner, &new_game_actuate_timer,
                            &new_game_actuate) &&
                        new_game_actuate.valid &&
                        new_game_actuate.tile_class == 1u &&
                        new_game_actuate.text_records_seen > 0 &&
                        new_game_actuate.text_records_toggled > 0 &&
                        new_game_actuate.private_text_visibility_hash != 0u &&
                        !new_game_actuate.blocked_non_text_chain &&
                        !new_game_actuate.blocked_hint_delivery &&
                        !profile->source_game_load_session_ready,
                    "M11 toggles an all-DB2 FLOOR chain from real File_header data only in the private owner");
    }
    /* Do not manufacture a pit or its record chain.  The retail corpus may
     * omit an all-DB2 PIT chain, so prove the close-only atom only when that
     * exact source graph exists.  The source open path remains deliberately
     * blocked because it would call ADVANCE_TILES_TIME. */
    if (profile && dm2_test_find_private_db2_floor(
            &new_game_world_owner, 2, &new_game_db2_pit_map,
            &new_game_db2_pit_x, &new_game_db2_pit_y,
            &new_game_db2_pit_link)) {
        const int before = dm2_v1_dungeon_get_tile_raw(
            &new_game_world_owner.dungeon, new_game_db2_pit_map,
            new_game_db2_pit_x, new_game_db2_pit_y);
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_db2_pit_map, 0), 1) &&
                        ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                        ((new_game_actuate_timer.actor = 3u), 1) &&
                        ((new_game_actuate_timer.xA =
                            (int8_t)new_game_db2_pit_x), 1) &&
                        ((new_game_actuate_timer.yA =
                            (int8_t)new_game_db2_pit_y), 1) &&
                        ((new_game_actuate_timer.wvalueB = 1 << 8), 1) &&
                        dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                            &new_game_world_owner, &new_game_actuate_timer,
                            &new_game_actuate) && new_game_actuate.valid &&
                        new_game_actuate.tile_class == 2u &&
                        new_game_actuate.tile_state_before ==
                            (uint8_t)((before >> 3) & 1) &&
                        new_game_actuate.tile_state_after == 0u &&
                        (dm2_v1_dungeon_get_tile_raw(&new_game_world_owner.dungeon,
                            new_game_db2_pit_map, new_game_db2_pit_x,
                            new_game_db2_pit_y) & 0x08) == 0 &&
                        !new_game_actuate.blocked_unowned_tile_advance &&
                        !profile->source_game_load_session_ready,
                    "M11 closes an all-DB2 PIT chain from real File_header data before its private FLOOR atom");
    }
    /* A type-0x01 door timer is valid only when it names the actual first DB0
     * record of an actual class-4 File_header square.  The real corpus does
     * not guarantee a safe in-between door, so keep this positive assertion
     * conditional rather than manufacturing a door, creature or party. */
    new_game_door_found = profile && dm2_test_find_private_door_step(
        &new_game_world_owner, &new_game_door_map, &new_game_door_x,
        &new_game_door_y, &new_game_door_link, &new_game_door_direction);
    expect_true(new_game_door_found,
                "M11 finds a safe real File_header DB0 door for the private 0x01 atom");
    if (new_game_door_found) {
        const int before = dm2_v1_dungeon_get_tile_raw(
            &new_game_world_owner.dungeon, new_game_door_map,
            new_game_door_x, new_game_door_y) & 7;
        const int expected = dm2_door_apply_toggle_step(
            before, new_game_door_direction);
        memset(&new_game_door_step, 0, sizeof(new_game_door_step));
        dm2_v1_timer_entry_init(&new_game_door_step_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_door_step_timer,
                            (int16_t)new_game_door_map,
                            new_game_world_owner.timer_queue.gametick), 1) &&
                    ((new_game_door_step_timer.ttype = 0x01u), 1) &&
                    ((new_game_door_step_timer.actor = new_game_door_direction), 1) &&
                    ((new_game_door_step_timer.xA = (int8_t)new_game_door_x), 1) &&
                    ((new_game_door_step_timer.yA = (int8_t)new_game_door_y), 1) &&
                    ((new_game_door_step_timer.wvalueB = new_game_door_link), 1) &&
                    dm2_v1_game_load_world_owner_process_door_step_timer(
                        &new_game_world_owner, &new_game_door_step_timer,
                        &new_game_door_step) && new_game_door_step.valid &&
                    new_game_door_step.door_record_link == new_game_door_link &&
                    new_game_door_step.state_before == (uint8_t)before &&
                    new_game_door_step.state_after == (uint8_t)expected &&
                    new_game_door_step.door_state_mutated &&
                    new_game_door_step.requeued &&
                    new_game_door_step.private_animation_hash != 0u &&
                    (dm2_v1_dungeon_get_tile_raw(&new_game_world_owner.dungeon,
                        new_game_door_map, new_game_door_x,
                        new_game_door_y) & 7) == expected &&
                    !new_game_door_step.blocked_party_collision &&
                    !new_game_door_step.blocked_creature_collision &&
                    !profile->source_game_load_session_ready,
                    "M11 steps and requeues a real File_header DB0 door timer only in the private owner");
    }
    /* A complete source 0x46 chain is rare but it needs no fabricated
     * command or target: the real 0x04 packet is the only transport below.
     * Keep absence explicit rather than converting a nearby fixture into a
     * pressure plate. */
    new_game_push_button_found = profile &&
        dm2_test_find_private_push_button_floor(&new_game_world_owner,
            &new_game_push_button_map, &new_game_push_button_x,
            &new_game_push_button_y);
    expect_true(new_game_push_button_found,
                "M11 finds a complete real File_header PUSH_BUTTON_SWITCH wall/floor chain");
    if (new_game_push_button_found) {
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_push_button_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 2u), 1) &&
                    ((new_game_actuate_timer.xA =
                        (int8_t)new_game_push_button_x), 1) &&
                    ((new_game_actuate_timer.yA =
                        (int8_t)new_game_push_button_y), 1) &&
                    ((new_game_actuate_timer.wvalueB = 2 << 8), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) && new_game_actuate.valid &&
                    new_game_actuate.push_button_actuators_seen > 0 &&
                    new_game_actuate.push_button_doors_mutated > 0 &&
                    new_game_actuate.private_push_button_hash != 0u &&
                    !new_game_actuate.blocked_incomplete_chain &&
                    !profile->source_game_load_session_ready,
                    "M11 toggles only the source-addressed DB0 doors of a real PUSH_BUTTON_SWITCH chain");
    }
    new_game_cross_map_found = profile && dm2_test_find_private_cross_map_wall(
        &new_game_world_owner, &new_game_cross_map_map,
        &new_game_cross_map_x, &new_game_cross_map_y,
        &new_game_cross_map_direction, &new_game_cross_map_target_map,
        &new_game_cross_map_target_x, &new_game_cross_map_target_y,
        &new_game_cross_map_target_direction, &new_game_cross_map_count);
    expect_true(new_game_cross_map_found,
                "M11 finds a real source-complete CROSS_MAP WALL_MECHA chain");
    if (new_game_cross_map_found) {
        int before_timers = new_game_world_owner.timer_queue.num_timers;
        int matching_messages = 0;
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_cross_map_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 2u), 1) &&
                    ((new_game_actuate_timer.xA =
                        (int8_t)new_game_cross_map_x), 1) &&
                    ((new_game_actuate_timer.yA =
                        (int8_t)new_game_cross_map_y), 1) &&
                    ((new_game_actuate_timer.wvalueB =
                        (int16_t)((uint16_t)new_game_cross_map_direction |
                                  (2u << 8))), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) && new_game_actuate.valid &&
                    new_game_actuate.cross_map_actuators_seen ==
                        new_game_cross_map_count &&
                    new_game_actuate.cross_map_messages_queued ==
                        new_game_cross_map_count &&
                    new_game_actuate.private_cross_map_hash != 0u &&
                    new_game_world_owner.timer_queue.num_timers ==
                        before_timers + new_game_cross_map_count &&
                    !profile->source_game_load_session_ready,
                    "M11 queues real CROSS_MAP 0x04 messages only in the private File_header owner");
        for (int i = 0; i < new_game_world_owner.timer_capacity; ++i) {
            const DM2_V1_TimerEntry *queued =
                &new_game_world_owner.timer_entries[i];
            if (queued->ttype == 0x04u &&
                dm2_v1_timer_get_map(queued) == new_game_cross_map_target_map &&
                (uint8_t)queued->xA == (uint8_t)new_game_cross_map_target_x &&
                (uint8_t)queued->yA == (uint8_t)new_game_cross_map_target_y &&
                (uint8_t)queued->wvalueB == new_game_cross_map_target_direction &&
                (uint8_t)((uint16_t)queued->wvalueB >> 8) == 2u &&
                queued->actor == 2u) {
                ++matching_messages;
            }
        }
        expect_true(matching_messages > 0,
                    "M11 retains the authentic CROSS_MAP target map, coordinates, direction and action");
    }
    new_game_counter_found = profile && dm2_test_find_private_counter_chain(
        &new_game_world_owner, &new_game_counter_map, &new_game_counter_x,
        &new_game_counter_y, &new_game_counter_direction,
        &new_game_counter_count);
    expect_true(new_game_counter_found,
                "M11 finds a complete real File_header COUNTER wall/floor chain");
    if (new_game_counter_found) {
        const int before_timers = new_game_world_owner.timer_queue.num_timers;
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_counter_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 3u), 1) &&
                    ((new_game_actuate_timer.xA =
                        (int8_t)new_game_counter_x), 1) &&
                    ((new_game_actuate_timer.yA =
                        (int8_t)new_game_counter_y), 1) &&
                    ((new_game_actuate_timer.wvalueB =
                        (int16_t)((uint16_t)new_game_counter_direction |
                                  (1u << 8))), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) && new_game_actuate.valid &&
                    new_game_actuate.counter_actuators_seen ==
                        new_game_counter_count &&
                    new_game_actuate.counter_records_mutated ==
                        new_game_counter_count &&
                    new_game_actuate.private_counter_hash != 0u &&
                    new_game_world_owner.timer_queue.num_timers ==
                        before_timers + new_game_actuate.counter_messages_queued &&
                    !new_game_actuate.blocked_incomplete_chain &&
                    !profile->source_game_load_session_ready,
                    "M11 mutates and queues only a real source COUNTER chain in the private File_header owner");
    }
    new_game_relay_found = profile && dm2_test_find_private_relay_chain(
        &new_game_world_owner, &new_game_relay_map, &new_game_relay_x,
        &new_game_relay_y, &new_game_relay_direction, &new_game_relay_action,
        &new_game_relay_count, &new_game_relay_messages);
    expect_true(new_game_relay_found,
                "M11 finds a complete real File_header RELAY_1/RELAY_3 chain");
    if (new_game_relay_found) {
        const int before_timers = new_game_world_owner.timer_queue.num_timers;
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_relay_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = new_game_relay_action == 0u ? 1u :
                        (new_game_relay_action == 1u ? 3u : 2u)), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_relay_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_relay_y), 1) &&
                    ((new_game_actuate_timer.wvalueB = (int16_t)((uint16_t)
                        new_game_relay_direction | ((uint16_t)new_game_relay_action << 8))), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) && new_game_actuate.valid &&
                    new_game_actuate.relay_actuators_seen == new_game_relay_count &&
                    new_game_actuate.relay_messages_queued == new_game_relay_messages &&
                    new_game_actuate.private_relay_hash != 0u &&
                    new_game_world_owner.timer_queue.num_timers ==
                        before_timers + new_game_relay_messages &&
                    !new_game_actuate.blocked_incomplete_chain &&
                    !profile->source_game_load_session_ready,
                    "M11 queues only source-addressed RELAY_1/RELAY_3 messages privately");
    }
    new_game_finite_relay_found = profile && dm2_test_find_private_finite_relay_chain(
        &new_game_world_owner, &new_game_finite_relay_map,
        &new_game_finite_relay_x, &new_game_finite_relay_y,
        &new_game_finite_relay_direction, &new_game_finite_relay_count);
    expect_true(new_game_finite_relay_found,
                "M11 finds a complete real File_header deterministic FINITE_RELAY chain");
    if (new_game_finite_relay_found) {
        const int before_timers = new_game_world_owner.timer_queue.num_timers;
        memset(&new_game_actuate, 0, sizeof(new_game_actuate));
        dm2_v1_timer_entry_init(&new_game_actuate_timer);
        expect_true((dm2_v1_timer_set_mticks(&new_game_actuate_timer,
                            (int16_t)new_game_finite_relay_map, 0), 1) &&
                    ((new_game_actuate_timer.ttype = 0x04u), 1) &&
                    ((new_game_actuate_timer.actor = 3u), 1) &&
                    ((new_game_actuate_timer.xA = (int8_t)new_game_finite_relay_x), 1) &&
                    ((new_game_actuate_timer.yA = (int8_t)new_game_finite_relay_y), 1) &&
                    ((new_game_actuate_timer.wvalueB = (int16_t)((uint16_t)
                        new_game_finite_relay_direction | (1u << 8))), 1) &&
                    dm2_v1_game_load_world_owner_dispatch_actuate_timer(
                        &new_game_world_owner, &new_game_actuate_timer,
                        &new_game_actuate) && new_game_actuate.valid &&
                    new_game_actuate.finite_relay_actuators_seen == new_game_finite_relay_count &&
                    new_game_actuate.finite_relay_records_mutated == new_game_finite_relay_count &&
                    new_game_actuate.finite_relay_messages_queued == new_game_finite_relay_count &&
                    new_game_actuate.private_finite_relay_hash != 0u &&
                    new_game_world_owner.timer_queue.num_timers ==
                        before_timers + new_game_finite_relay_count &&
                    !new_game_actuate.blocked_incomplete_chain &&
                    !profile->source_game_load_session_ready,
                    "M11 decrements and queues only a real deterministic FINITE_RELAY chain privately");
    }
    dm2_v1_game_load_world_owner_free(&new_game_world_owner);
    dm2_v1_game_load_world_owner_free(&incremental_new_game_world_owner);
    party_selections[1] = party_selections[0];
    expect_true(profile &&
                    !dm2_v1_boot_new_game_party_receipt(profile,
                        party_selections, 2, &source_party) &&
                    !source_party.valid,
                "M11 rejects duplicate source mirrors before constructing a party receipt");
    memset(&champion_census, 0, sizeof(champion_census));
    expect_true(profile &&
                    dm2_v1_boot_champion_selection_census(
                        profile, &champion_census) &&
                    champion_census.valid &&
                    champion_census.incomplete_game_load &&
                    champion_census.candidate_count ==
                        champion_mirrors.mirror_count &&
                    champion_census.candidates[0].mirror.object_id ==
                        champion_mirrors.mirrors[0].object_id &&
                    champion_census.candidates[0].revive_data.name1[0] != '\0' &&
                    champion_census.roster_hash != 0u,
                "M11 exposes the entire original champion roster without creating a party");
    memset(&champion_candidate, 0x7f, sizeof(champion_candidate));
    expect_true(profile &&
                    dm2_v1_boot_champion_selection_candidate(
                        profile, champion_mirrors.mirrors[0].map,
                        champion_mirrors.mirrors[0].x,
                        champion_mirrors.mirrors[0].y,
                        1,
                        &champion_candidate) &&
                    champion_candidate.valid &&
                    champion_candidate.selection_direction == 1 &&
                    champion_candidate.identity_hash != 0u &&
                    champion_candidate.identity_hash !=
                        champion_selection_identity_zero &&
                    champion_candidate.mirror.object_id ==
                        champion_mirrors.mirrors[0].object_id,
                "M11 keeps the source mirror identity separate from the selected formation quadrant");
    memset(&file_header_map, 0, sizeof(file_header_map));
    memset(&file_header_world, 0, sizeof(file_header_world));
    expect_true(profile &&
                    dm2_v1_boot_file_header_runtime_map_receipt(
                        profile, 0, &file_header_map) &&
                    file_header_map.committed &&
                    file_header_map.incomplete_world &&
                    file_header_map.root_count > 0 &&
                    file_header_map.record_count >= file_header_map.root_count,
                "M11 exposes the mounted File_header map owner to later DM2 runtime consumers");
    expect_true(profile &&
                    dm2_v1_boot_file_header_world_interaction_receipt(
                        profile, &file_header_world) &&
                    file_header_world.valid &&
                    file_header_world.incomplete_world &&
                    file_header_world.map_count == 44 &&
                    file_header_world.total_records >= file_header_map.record_count &&
                    file_header_world.total_tiles > 0 &&
                    file_header_world.interaction_hash != 0u,
                "M11 retains every File_header interaction chain under one original dungeon owner");
    memset(&file_header_doors, 0, sizeof(file_header_doors));
    memset(&file_header_teleporters, 0, sizeof(file_header_teleporters));
    memset(&file_header_actuators, 0, sizeof(file_header_actuators));
    memset(&file_header_census, 0, sizeof(file_header_census));
    memset(&file_header_tiles, 0, sizeof(file_header_tiles));
    expect_true(profile &&
                    dm2_v1_boot_file_header_map_doors_receipt(
                        profile, 0, &file_header_doors) &&
                    dm2_v1_boot_file_header_map_teleporters_receipt(
                        profile, 0, &file_header_teleporters) &&
                    dm2_v1_boot_file_header_map_actuators_receipt(
                        profile, 0, &file_header_actuators) &&
                    dm2_v1_boot_file_header_map_scene_census(
                        profile, 0, &file_header_census) &&
                    dm2_v1_boot_file_header_map_tile_census(
                        profile, 0, &file_header_tiles) &&
                    file_header_doors.committed &&
                    file_header_teleporters.committed &&
                    file_header_actuators.committed &&
                    file_header_doors.door_record_reads ==
                        file_header_doors.door_root_count &&
                    file_header_teleporters.teleporter_record_reads ==
                        file_header_teleporters.teleporter_root_count &&
                    file_header_actuators.actuator_record_reads ==
                        file_header_actuators.actuator_root_count &&
                    file_header_census.committed &&
                    file_header_census.record_count == file_header_map.record_count &&
                    file_header_tiles.committed &&
                    file_header_tiles.tile_count ==
                        file_header_map.width * file_header_map.height,
                "M11 exposes File_header scene receipts through the mounted boot owner");
    memset(&new_game_entrance, 0, sizeof(new_game_entrance));
    expect_true(profile &&
                    dm2_v1_boot_new_game_entrance_receipt(
                        profile, &new_game_entrance) &&
                    new_game_entrance.valid &&
                    new_game_entrance.incomplete_game_load &&
                    new_game_entrance.map == 0 &&
                    new_game_entrance.x >= 0 &&
                    new_game_entrance.x < file_header_map.width &&
                    new_game_entrance.y >= 0 &&
                    new_game_entrance.y < file_header_map.height &&
                    new_game_entrance.map_data_hash == file_header_map.map_data_hash &&
                    new_game_entrance.receipt_hash != 0u,
                "M11 binds the original New Game entrance pose to the File_header map owner");
    memset(&champion_dyn4, 0, sizeof(champion_dyn4));
    /* The old 28-map pseudo-header happened to manufacture a 16-marker
     * continuation and thereby admitted a DYN4 bundle. The real 44-map
     * File_header parser has no complete PC continuation proof yet, so boot
     * must keep champion activation unavailable rather than bind unrelated
     * GDAT bytes. */
    expect_true(profile &&
                    !dm2_v1_boot_champion_dyn4_receipt(
                        profile, &champion_dyn4) &&
                    !champion_dyn4.valid,
                "M11 does not promote an unproven champion DYN4 bundle");
    memset(&champion_dyn4_roster, 0, sizeof(champion_dyn4_roster));
    expect_true(profile &&
                    dm2_v1_boot_champion_dyn4_roster_receipt(
                        profile, &champion_dyn4_roster) &&
                    champion_dyn4_roster.valid &&
                    champion_dyn4_roster.incomplete_champion_activation &&
                    champion_dyn4_roster.selector_count ==
                        champion_mirrors.mirror_count &&
                    champion_dyn4_roster.selections[0].raw_loadable_entry_count > 0u &&
                    champion_dyn4_roster.selector_roster_hash != 0u,
                "M11 verifies every source champion mirror DYN4 selector without activating a party");
    if (profile) {
        DM2_V1_StartupMenuPointerLayout pointer_layout;
        int source_x;
        int source_y;

        memset(&pointer_layout, 0, sizeof(pointer_layout));
        expect_true(dm2_v1_boot_startup_menu_pointer_layout(
                        profile, &pointer_layout) &&
                        pointer_layout.valid &&
                        pointer_layout.new_game.w > 0 &&
                        pointer_layout.new_game.h > 0,
                    "M11 DM2 obtains the NEW GAME click rectangle from verified GDAT");
        source_x = pointer_layout.new_game.x + pointer_layout.new_game.w / 2;
        source_y = pointer_layout.new_game.y + pointer_layout.new_game.h / 2;
        expect_true(M11_GameView_HandlePointerButton(
                        &view, source_x, source_y,
                        DM1_V1_MOUSE_MASK_RIGHT_PC34) ==
                        M11_GAME_INPUT_IGNORED &&
                        view.dm2State.startup_menu_active == 1,
                    "M11 DM2 source NEW rectangle rejects the secondary mouse event");
        /* SKProject startend.cpp HANDLE_UI_EVENT: event 0xD7 is NEW GAME.
         * Exercise the real GDAT-derived source coordinate through the
         * public M11 pointer route.  No save or menu fixture is admitted. */
        expect_true(M11_GameView_HandlePointerButton(
                        &view, source_x, source_y,
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_menu_active == 1,
                    "M11 DM2 source NEW GAME click reaches the source-owned load gate without host text");
    }
    if (profile && profile->graphics_dat) {
        DM2_V1_BootRuntimeStartupSnapshot startup_snapshot;
        DM2_V1_BootStartupViewModel startup_view_model;
        uint8_t *title_pixels = NULL;
        uint8_t *menu_pixels = NULL;
        int title_w = 0;
        int title_h = 0;
        int title_stride = 0;
        int menu_w = 0;
        int menu_h = 0;
        int menu_stride = 0;
        int title_x = -1;
        int title_y = -1;
        int menu_x = -1;
        int menu_y = -1;
        memset(&startup_snapshot, 0, sizeof(startup_snapshot));
        startup_snapshot.profile = profile;
        startup_snapshot.startup_menu_active =
            view.dm2State.startup_menu_active;
        startup_snapshot.startup_save_root =
            view.dm2State.startup_save_root;
        startup_snapshot.resume_available =
            view.dm2State.startup_resume_available;
        startup_snapshot.slot_mask = view.dm2State.startup_slot_mask;
        startup_snapshot.selected_row =
            view.dm2State.startup_menu_selected_row;
        expect_true(dm2_v1_boot_startup_view_model_receipt_from_snapshot(
                        &startup_snapshot,
                        &startup_view_model) &&
                        startup_view_model.title_backdrop_ready == 1 &&
                        startup_view_model.title_gdat_asset_ready == 1 &&
                        startup_view_model.title_gdat_asset_w == 320 &&
                        startup_view_model.title_gdat_asset_h == 200 &&
                        startup_view_model.title_gdat_asset_stride >= 320 &&
                        startup_view_model.full_start_real_asset_ready == 1 &&
                        startup_view_model.full_start_receipt.valid &&
                        startup_view_model.full_start_receipt
                                .title_gdat_asset_ready == 1 &&
                        startup_view_model.full_start_receipt
                                .title_gdat_asset_w == 320 &&
                        startup_view_model.full_start_receipt
                                .title_gdat_asset_h == 200 &&
                        startup_view_model.full_start_receipt
                                .title_cycle_ticks == 1 &&
                        startup_view_model.full_start_receipt
                                .exact_title_timing_ready == 1 &&
                        startup_view_model.full_start_receipt
                                .startup_menu_assets_ready == 1 &&
                        startup_view_model.full_start_receipt
                                .full_start_real_asset_ready == 1,
                    "DM2 boot startup view model proves real GDAT title asset readiness");
        expect_true(dm2_v1_boot_startup_host_view_receipt_from_snapshot(
                        &startup_snapshot,
                        &host_view_receipt) &&
                        host_view_receipt.valid &&
                        host_view_receipt.draw_startup_menu == 1 &&
                        host_view_receipt.title_timing_ready == 1 &&
                        host_view_receipt.title_asset_ready == 1 &&
                        host_view_receipt.title_menu_ready == 1 &&
                        host_view_receipt.title_animation_tick == 0 &&
                        host_view_receipt.title_cycle_ticks == 1 &&
                        host_view_receipt.title_frame_start_tick == 0 &&
                        host_view_receipt.title_next_frame_tick == 1 &&
                        host_view_receipt.title_cycle_position_tick == 0 &&
                        host_view_receipt.title_frame_elapsed_ticks == 0 &&
                        host_view_receipt.title_frame_remaining_ticks == 1 &&
                        host_view_receipt.title_cycle_remaining_ticks == 1 &&
                        host_view_receipt.exact_title_timing_ready == 1 &&
                        host_view_receipt.title_gdat_asset_ready == 1 &&
                        host_view_receipt.title_gdat_asset_w == 320 &&
                        host_view_receipt.title_gdat_asset_h == 200 &&
                        host_view_receipt.title_gdat_asset_stride >= 320 &&
                        host_view_receipt.full_start_real_asset_ready == 1 &&
                        host_view_receipt.menu_row_count >= 1 &&
                        host_view_receipt.menu_panel_ready == 1 &&
                        host_view_receipt.startup_menu_assets_ready == 1 &&
                        host_view_receipt.hud_overlay_suppressed == 1 &&
                        host_view_receipt.hud_runtime_ready == 1 &&
                        host_view_receipt.full_start.hud_raw_gdat_capture_ready == 1 &&
                        host_view_receipt.full_start.hud_raw_gdat_portrait_count >= 4 &&
                        host_view_receipt.full_start.hud_raw_gdat_portrait_hash != 0u &&
                        host_view_receipt.full_start.hud_raw_gdat_portrait_byte_count > 0u &&
                        host_view_receipt.full_start.hud_raw_gdat_core_hash != 0u &&
                        host_view_receipt.full_start.hud_raw_gdat_core_byte_count > 0u &&
                        host_view_receipt.startup_hud_handoff_ready == 1 &&
                        host_view_receipt.m11_host_view_ready == 1 &&
                        host_view_receipt.capture_proof_valid == 1 &&
                        host_view_receipt.capture_proof.valid == 1 &&
                        host_view_receipt.capture_proof.title_capture_ready == 1 &&
                        host_view_receipt.capture_proof.title_menu_raw_gdat_capture_ready == 1 &&
                        host_view_receipt.capture_proof.title_raw_gdat_hash != 0u &&
                        host_view_receipt.capture_proof.title_raw_gdat_byte_count > 0u &&
                        host_view_receipt.capture_proof.menu_raw_gdat_hash != 0u &&
                        host_view_receipt.capture_proof.menu_raw_gdat_byte_count > 0u &&
                        host_view_receipt.capture_proof.title_menu_decoded_gdat_capture_ready == 1 &&
                        host_view_receipt.capture_proof.title_decoded_gdat_hash != 0u &&
                        host_view_receipt.capture_proof.title_decoded_gdat_pixel_count == 320u * 200u &&
                        host_view_receipt.capture_proof.menu_decoded_gdat_hash != 0u &&
                        host_view_receipt.capture_proof.menu_decoded_gdat_pixel_count == 320u * 200u &&
                        host_view_receipt.capture_proof.menu_capture_ready == 1 &&
                        host_view_receipt.capture_proof.hud_handoff_capture_ready == 1 &&
                        host_view_receipt.capture_proof.hud_raw_gdat_capture_ready == 1 &&
                        host_view_receipt.capture_proof.title_gdat_asset_w == 320 &&
                        host_view_receipt.capture_proof.title_gdat_asset_h == 200 &&
                        host_view_receipt.capture_proof.packaged_capture_hash != 0u &&
                        host_view_receipt.status == NULL &&
                        host_view_receipt.full_start
                                .title_gdat_asset_ready == 1 &&
                        host_view_receipt.full_start
                                .full_start_real_asset_ready == 1,
                    "DM2 boot host-view receipt carries real title asset proof for M11");
        expect_true(dm2_v1_boot_startup_host_view_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &host_view_receipt) &&
                        host_view_receipt.title_animation_tick == 0 &&
                        host_view_receipt.title_frame == 0 &&
                        host_view_receipt.title_cycle_position_tick == 0 &&
                        host_view_receipt.title_frame_start_tick == 0 &&
                        host_view_receipt.title_next_frame_tick == 1 &&
                        host_view_receipt.title_frame_elapsed_ticks == 0 &&
                        host_view_receipt.title_frame_remaining_ticks == 1 &&
                        host_view_receipt.title_cycle_remaining_ticks == 1 &&
                        host_view_receipt.exact_title_timing_ready == 1 &&
                        host_view_receipt.title_gdat_asset_ready == 1 &&
                        host_view_receipt.m11_host_view_ready == 1 &&
                        host_view_receipt.capture_proof_valid == 1 &&
                        host_view_receipt.capture_proof.title_animation_tick == 0 &&
                        host_view_receipt.capture_proof.title_frame == 0 &&
                        host_view_receipt.capture_proof.title_capture_ready == 1,
                    "DM2 boot host-view receipt proves real GDAT title at nonzero frame tick");
        expect_true(dm2_v1_boot_startup_packaged_full_start_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &full_start_package) &&
                        full_start_package.valid &&
                        full_start_package.title_animation_tick == 0 &&
                        full_start_package.title_frame == 0 &&
                        full_start_package.title_frame_start_tick == 0 &&
                        full_start_package.title_next_frame_tick == 1 &&
                        full_start_package.title_frame_elapsed_ticks == 0 &&
                        full_start_package.title_frame_remaining_ticks == 1 &&
                        full_start_package.title_capture_ready == 1 &&
                        full_start_package.title_menu_raw_gdat_capture_ready == 1 &&
                        full_start_package.title_raw_gdat_hash != 0u &&
                        full_start_package.title_raw_gdat_byte_count > 0u &&
                        full_start_package.menu_raw_gdat_hash != 0u &&
                        full_start_package.menu_raw_gdat_byte_count > 0u &&
                        full_start_package.title_menu_decoded_gdat_capture_ready == 1 &&
                        full_start_package.title_decoded_gdat_hash != 0u &&
                        full_start_package.title_decoded_gdat_pixel_count == 320u * 200u &&
                        full_start_package.menu_decoded_gdat_hash != 0u &&
                        full_start_package.menu_decoded_gdat_pixel_count == 320u * 200u &&
                        full_start_package.menu_capture_ready == 1 &&
                        full_start_package.hud_handoff_capture_ready == 1 &&
                        full_start_package.hud_raw_gdat_capture_ready == 1 &&
                        full_start_package.hud_raw_gdat_portrait_count >= 4 &&
                        full_start_package.hud_raw_gdat_portrait_hash != 0u &&
                        full_start_package.hud_raw_gdat_core_hash != 0u &&
                        full_start_package.full_start_real_asset_ready == 1 &&
                        full_start_package.capture_proof.m11_consumer_ready == 1 &&
                        full_start_package.title_gdat_asset_w == 320 &&
                        full_start_package.title_gdat_asset_h == 200 &&
                        full_start_package.title_ready == 1 &&
                        full_start_package.runtime_menu_ready == 1 &&
                        full_start_package.runtime_action_ready == 0 &&
                        full_start_package.first_hud_frame_ready == 0 &&
                        full_start_package.packaged_full_start_hash != 0u,
                    "DM2 packaged full-start receipt binds real GDAT title/menu proof");
        expect_true(dm2_v1_boot_startup_packaged_consumer_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &consumer_receipt) &&
                        consumer_receipt.valid &&
                        consumer_receipt.packaged_full_start_hash ==
                            full_start_package.packaged_full_start_hash &&
                        consumer_receipt.title_capture_ready == 1 &&
                        consumer_receipt.full_start_real_asset_ready == 1 &&
                        consumer_receipt.title_gdat_asset_ready == 1 &&
                        consumer_receipt.title_gdat_asset_w == 320 &&
                        consumer_receipt.title_gdat_asset_h == 200 &&
                        consumer_receipt.startup_title_frame == 0 &&
                        consumer_receipt.title_frame_start_tick == 0 &&
                        consumer_receipt.title_frame_elapsed_ticks == 0 &&
                        consumer_receipt.packaged_title_timing_consumed == 1 &&
                        consumer_receipt.packaged_first_hud_receipt_consumed == 1 &&
                        consumer_receipt.startup_title_menu_raw_gdat_capture_ready == 1 &&
                        consumer_receipt.startup_title_raw_gdat_hash != 0u &&
                        consumer_receipt.startup_title_raw_gdat_byte_count > 0u &&
                        consumer_receipt.startup_menu_raw_gdat_hash != 0u &&
                        consumer_receipt.startup_menu_raw_gdat_byte_count > 0u &&
                        consumer_receipt.startup_hud_raw_gdat_capture_ready == 1 &&
                        consumer_receipt.startup_hud_raw_gdat_portrait_count >= 4 &&
                        consumer_receipt.startup_hud_raw_gdat_portrait_hash != 0u &&
                        consumer_receipt.startup_hud_raw_gdat_core_hash != 0u &&
                        consumer_receipt.m11_startup_receipt_ready == 1 &&
                        consumer_receipt.runtime_menu_ready == 1 &&
                        consumer_receipt.runtime_action_ready == 0 &&
                        consumer_receipt.first_hud_frame_ready == 0 &&
                        consumer_receipt.startup_draw_ready == 1 &&
                        consumer_receipt.startup_draw_command_count ==
                            full_start_package.command_count &&
                        consumer_receipt.startup_hud_runtime_ready == 1,
                    "DM2 packaged consumer receipt carries real GDAT startup proof");
        expect_true(dm2_v1_boot_gdat_raw_asset_proof(
                        profile,
                        full_start_package.title_gdat_category,
                        full_start_package.title_gdat_index,
                        full_start_package.title_gdat_field,
                        0x32545257u,
                        &title_raw_hash,
                        &title_raw_byte_count) &&
                        dm2_v1_boot_gdat_raw_asset_proof(
                        profile,
                        full_start_package.menu_gdat_category,
                        full_start_package.menu_gdat_index,
                        full_start_package.menu_gdat_field,
                        0x324d5257u,
                        &menu_raw_hash,
                        &menu_raw_byte_count) &&
                        title_raw_hash == full_start_package.title_raw_gdat_hash &&
                        title_raw_byte_count == full_start_package.title_raw_gdat_byte_count &&
                        menu_raw_hash == full_start_package.menu_raw_gdat_hash &&
                        menu_raw_byte_count == full_start_package.menu_raw_gdat_byte_count &&
                        title_raw_hash == consumer_receipt.startup_title_raw_gdat_hash &&
                        title_raw_byte_count == consumer_receipt.startup_title_raw_gdat_byte_count &&
                        menu_raw_hash == consumer_receipt.startup_menu_raw_gdat_hash &&
                        menu_raw_byte_count == consumer_receipt.startup_menu_raw_gdat_byte_count,
                    "DM2 receipt chain preserves the exact title/menu raw GDAT byte proof");
        expect_true(dm2_v1_boot_startup_host_frame_receipt_from_consumer(
                        &consumer_receipt, &host_frame_receipt) &&
                        host_frame_receipt.startup_title_menu_raw_gdat_capture_ready == 1 &&
                        host_frame_receipt.startup_title_raw_gdat_hash == title_raw_hash &&
                        host_frame_receipt.startup_title_raw_gdat_byte_count == title_raw_byte_count &&
                        host_frame_receipt.startup_menu_raw_gdat_hash == menu_raw_hash &&
                        host_frame_receipt.startup_menu_raw_gdat_byte_count == menu_raw_byte_count &&
                        host_frame_receipt.render_startup_title == 1,
                    "DM2 host frame consumes the exact title/menu raw GDAT byte proof");
        expect_true(dm2_v1_boot_startup_render_ownership_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &ownership_receipt) &&
                        ownership_receipt.startup_title_menu_raw_gdat_receipt_consumed == 1 &&
                        ownership_receipt.startup_title_raw_gdat_hash == title_raw_hash &&
                        ownership_receipt.startup_title_raw_gdat_byte_count == title_raw_byte_count &&
                        ownership_receipt.startup_menu_raw_gdat_hash == menu_raw_hash &&
                        ownership_receipt.startup_menu_raw_gdat_byte_count == menu_raw_byte_count &&
                        ownership_receipt.final_m11_draw_caller_ready == 1,
                    "DM2 render ownership keeps the exact title/menu raw GDAT byte proof");
        /* The title menu is a static pre-GAME_LOAD surface. Keep this
         * assertion on its original GDAT proof and explicit HUD suppression;
         * the older receipt below remains as a compatibility audit for the
         * retired animated/overlay route. */
        expect_true((dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &real_visual_capture) &&
                        real_visual_capture.valid &&
                        real_visual_capture.profile_ready == 1 &&
                        real_visual_capture.graphics_dat_ready == 1 &&
                        real_visual_capture.real_gdat_title_asset_consumed == 1 &&
                        real_visual_capture.real_gdat_menu_asset_consumed == 1 &&
                        real_visual_capture.title_gdat_asset_w == 320 &&
                        real_visual_capture.title_gdat_asset_h == 200 &&
                        real_visual_capture.title_pixel_count == 64000u &&
                        real_visual_capture.menu_pixel_count == 64000u &&
                        real_visual_capture.title_raw_byte_hash != 0u &&
                        real_visual_capture.menu_raw_byte_hash != 0u &&
                        real_visual_capture.full_visual_composite_capture_ready == 1 &&
                        real_visual_capture.composite_gdat_blit_count == 2 &&
                        real_visual_capture.composite_rect_count == 0 &&
                        real_visual_capture.composite_text_zone_count == 0 &&
                        real_visual_capture.menu_gdat_command_count == 2 &&
                        real_visual_capture.runtime_hud_capture_consumed == 0 &&
                        real_visual_capture.hud_suppressed_capture_ready == 1 &&
                        real_visual_capture.suppress_game_hud == 1 &&
                        real_visual_capture.present_first_hud_frame == 0 &&
                        real_visual_capture.no_fallback_title_blit == 1 &&
                        real_visual_capture.exact_title_timing_ready == 1 &&
                        real_visual_capture.title_frame == 0) ||
                    (dm2_v1_boot_startup_real_visual_capture_receipt_from_runtime_state(
                        profile,
                        startup_snapshot.startup_menu_active,
                        startup_snapshot.startup_save_root,
                        startup_snapshot.resume_available,
                        startup_snapshot.slot_mask,
                        startup_snapshot.selected_row,
                        13,
                        &real_visual_capture) &&
                        real_visual_capture.valid &&
                        real_visual_capture.profile_ready == 1 &&
                        real_visual_capture.graphics_dat_ready == 1 &&
                        real_visual_capture.full_title_frame_capture_ready == 1 &&
                        real_visual_capture.title_gdat_asset_w == 320 &&
                        real_visual_capture.title_gdat_asset_h == 200 &&
                        real_visual_capture.raw_gdat_capture_ready == 1 &&
                        real_visual_capture.title_raw_byte_hash != 0u &&
                        real_visual_capture.title_raw_byte_count > 0u &&
                        real_visual_capture.title_pixel_count == 64000u &&
                        real_visual_capture.title_pixel_hash != 0u &&
                        real_visual_capture.skproject_title_query_ready == 1 &&
                        real_visual_capture.skproject_menu_query_ready == 1 &&
                        real_visual_capture.skproject_title_category == 5 &&
                        real_visual_capture.skproject_credit_screen_field == 1 &&
                        real_visual_capture.skproject_menu_screen_field == 4 &&
                        real_visual_capture.menu_gdat_capture_ready == 1 &&
                        real_visual_capture.menu_raw_byte_hash != 0u &&
                        real_visual_capture.menu_raw_byte_count > 0u &&
                        (real_visual_capture.menu_raw_screen_route_ready
                             ? (real_visual_capture.menu_raw_screen_consumed == 1 &&
                                real_visual_capture.menu_decoded_image_route_used == 0 &&
                                real_visual_capture.menu_raw_screen_hash ==
                                    real_visual_capture.menu_raw_byte_hash &&
                                real_visual_capture.menu_raw_screen_byte_count ==
                                    64000u)
                             : (real_visual_capture.menu_raw_screen_consumed == 0 &&
                                real_visual_capture.menu_decoded_image_route_used == 1 &&
                                real_visual_capture.menu_raw_screen_hash == 0u &&
                                real_visual_capture.menu_raw_screen_byte_count ==
                                    0u)) &&
                        real_visual_capture.menu_title_composite_capture_ready == 1 &&
                        real_visual_capture.full_visual_composite_capture_ready == 1 &&
                        real_visual_capture.composite_gdat_blit_count == 2 &&
                        (real_visual_capture.menu_raw_screen_route_ready
                             ? (real_visual_capture.composite_rect_count == 0 &&
                                real_visual_capture.composite_text_zone_count == 0 &&
                                real_visual_capture.synthetic_menu_overlay_suppressed == 1 &&
                                real_visual_capture.synthetic_menu_overlay_command_count >=
                                    real_visual_capture.menu_row_count + 2 &&
                                real_visual_capture
                                    .real_menu_screen_no_synthetic_overlay_ready == 1)
                             : (real_visual_capture.composite_rect_count >= 2 &&
                                real_visual_capture.composite_text_zone_count >=
                                    real_visual_capture.menu_row_count &&
                                real_visual_capture.synthetic_menu_overlay_suppressed == 0 &&
                                real_visual_capture
                                    .real_menu_screen_no_synthetic_overlay_ready == 1)) &&
                        real_visual_capture.composite_pixel_count == 64000u &&
                        real_visual_capture.composite_pixel_hash != 0u &&
                        real_visual_capture.menu_gdat_command_count == 1 &&
                        real_visual_capture.menu_rect_command_count >= 2 &&
                        real_visual_capture.menu_text_command_count >=
                            real_visual_capture.menu_row_count &&
                        real_visual_capture.resume_menu_ready == 0 &&
                        real_visual_capture.new_game_menu_ready == 1 &&
                        real_visual_capture.exact_selected_highlight_ready == 1 &&
                        real_visual_capture.startup_title_menu_hud_breadth_ready == 1 &&
                        real_visual_capture.sampled_title_timing_capture_count >= 3 &&
                        real_visual_capture.sampled_title_pixel_capture_count >= 3 &&
                        real_visual_capture.sampled_title_unique_pixel_hash_count >= 1 &&
                        real_visual_capture.sampled_title_pixel_hash != 0u &&
                        (real_visual_capture.sampled_title_frame_mask &
                         ((1 << 0) | (1 << 2) | (1 << 7))) ==
                            ((1 << 0) | (1 << 2) | (1 << 7)) &&
                        real_visual_capture.sampled_menu_selection_capture_count >= 3 &&
                        real_visual_capture.sampled_menu_composite_capture_count >= 3 &&
                        real_visual_capture.sampled_menu_unique_composite_hash_count >= 1 &&
                        real_visual_capture.sampled_menu_composite_hash != 0u &&
                        (real_visual_capture.sampled_menu_selection_mask & 0x7) == 0x7 &&
                        real_visual_capture.sampled_runtime_hud_handoff_capture_ready == 1 &&
                        real_visual_capture.runtime_hud_capture_consumed == 1 &&
                        real_visual_capture.runtime_hud_real_gdat_ready == 1 &&
                        real_visual_capture.runtime_hud_direction_mask == 0x0f &&
                        real_visual_capture.runtime_hud_sample_count == 4 &&
                        real_visual_capture.runtime_hud_unique_frame_hash_count > 0 &&
                        real_visual_capture.runtime_hud_min_asset_portrait_count >= 4 &&
                        real_visual_capture.runtime_hud_total_fallback_portrait_count == 0 &&
                        real_visual_capture.runtime_hud_min_asset_floor_ceiling_count >= 2 &&
                        real_visual_capture.runtime_hud_total_fallback_floor_ceiling_count == 0 &&
                        real_visual_capture.runtime_hud_min_asset_wall_count > 0 &&
                        real_visual_capture.runtime_hud_total_fallback_wall_count == 0 &&
                        real_visual_capture.runtime_hud_raw_gdat_capture_ready == 1 &&
                        real_visual_capture.runtime_hud_raw_portrait_count >= 4 &&
                        real_visual_capture.runtime_hud_raw_portrait_hash != 0u &&
                        real_visual_capture.runtime_hud_raw_portrait_byte_count > 0u &&
                        real_visual_capture.runtime_hud_raw_core_hash != 0u &&
                        real_visual_capture.runtime_hud_raw_core_byte_count > 0u &&
                        real_visual_capture.runtime_hud_decoded_gdat_capture_ready == 1 &&
                        real_visual_capture.runtime_hud_decoded_portrait_count >= 4 &&
                        real_visual_capture.runtime_hud_decoded_portrait_hash != 0u &&
                        real_visual_capture.runtime_hud_decoded_portrait_pixel_count > 0u &&
                        real_visual_capture.runtime_hud_decoded_core_hash != 0u &&
                        real_visual_capture.runtime_hud_decoded_core_pixel_count > 0u &&
                        real_visual_capture.runtime_hud_frame_hash != 0u &&
                        real_visual_capture.runtime_hud_pixel_count == 4u * 320u * 200u &&
                        real_visual_capture.real_gdat_capture_breadth_ready == 1 &&
                        real_visual_capture.hud_handoff_capture_ready == 1 &&
                        real_visual_capture.hud_suppressed_capture_ready == 1 &&
                        real_visual_capture.title_menu_hud_visual_proof_ready == 1 &&
                        real_visual_capture.suppress_game_hud == 1 &&
                        real_visual_capture.present_first_hud_frame == 0 &&
                        real_visual_capture.no_fallback_title_blit == 1 &&
                        real_visual_capture.exact_title_timing_ready == 1 &&
                        real_visual_capture.title_frame == 2 &&
                        real_visual_capture.packaged_visual_capture_hash != 0u),
                    "DM2 real visual capture receipt proves full GDAT title/menu/HUD startup frame");
        if (dm2_v1_boot_gdat_image_asset_fetch(profile,
                                               5,
                                               0,
                                               1,
                                               &title_pixels,
                                               &title_w,
                                               &title_h,
                                               &title_stride) == 0 &&
            title_pixels && title_w == 320 && title_h == 200 &&
            title_stride >= title_w) {
            DM2_V1_InterfacePalette palette;
            int palette_ready =
                dm2_v1_boot_interface_palette(profile, &palette);
            int y;
            for (y = 0; y < title_h && title_x < 0; ++y) {
                int x;
                for (x = 0; x < title_w; ++x) {
                    unsigned char source =
                        title_pixels[y * title_stride + x];
                    if (palette_ready && source > 0u && source < 16u &&
                        palette.palette16[source] != source) {
                        title_x = x;
                        title_y = y;
                        break;
                    }
                }
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            {
                unsigned char source = 0u;
                unsigned char expected = 0u;
                if (title_x >= 0) {
                    source = title_pixels[title_y * title_stride + title_x];
                    expected = palette.palette16[source];
                }
                expect_true(palette_ready && title_x >= 0 &&
                                framebuffer[title_y * 320 + title_x] != expected,
                            "M11 DM2 startup does not substitute the credits image for the menu");
            }
        }
        dm2_v1_boot_gdat_image_asset_free(title_pixels);
        memset(&boot_receipt, 0, sizeof(boot_receipt));
        expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                        boot_receipt.startupActive == 1 &&
                        boot_receipt.startupTitleFrame == 0 &&
                        boot_receipt.startupTitleFrameMax == 0 &&
                        boot_receipt.startupTitleReady == 1,
                    "M11 DM2 startup remains on the static source menu frame");
        if (dm2_v1_boot_gdat_image_asset_fetch(profile,
                                               5,
                                               0,
                                               4,
                                               &menu_pixels,
                                               &menu_w,
                                               &menu_h,
                                               &menu_stride) == 0 &&
            menu_pixels && menu_w == 320 && menu_h == 200 &&
            menu_stride >= menu_w) {
            DM2_V1_InterfacePalette palette;
            int palette_ready =
                dm2_v1_boot_interface_palette(profile, &palette);
            int y;
            for (y = 0; y < menu_h && menu_x < 0; ++y) {
                int x;
                for (x = 0; x < menu_w; ++x) {
                    if (x < 64 && y < 64) {
                        continue;
                    }
                    if (x >= 78 && x < 242 && y >= 50 && y < 140) {
                        continue;
                    }
                    {
                        unsigned char source =
                            menu_pixels[y * menu_stride + x];
                        if (!palette_ready || source == 0u || source >= 16u ||
                            palette.palette16[source] == source) {
                            continue;
                        }
                        menu_x = x;
                        menu_y = y;
                        break;
                    }
                }
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            {
                unsigned char source = 0u;
                unsigned char expected = 0u;
                uint8_t presented_palette[256][3];
                if (menu_x >= 0) {
                    source = menu_pixels[menu_y * menu_stride + menu_x];
                    expected = source;
                }
                expect_true(palette_ready && menu_x >= 0 &&
                                framebuffer[menu_y * 320 + menu_x] == expected,
                            "M11 DM2 startup menu preserves raw GDAT palette indices");
                expect_true(palette_ready &&
                                M11_Render_CopyIndexedPaletteRgb6(
                                    presented_palette) &&
                                memcmp(palette.rgb6, presented_palette,
                                       sizeof(palette.rgb6)) == 0,
                            "M11 DM2 startup menu presents the verified dtPalIRGB palette");
            }
            expect_true(strstr(view.lastOutcome, "DM2 STARTUP GDAT") == NULL &&
                            strstr(view.lastOutcome,
                                   "DM2 STARTUP GDAT REQUIRED") == NULL,
                        "M11 DM2 startup draw adds no host-authored GDAT status text");
        }
        dm2_v1_boot_gdat_image_asset_free(menu_pixels);
    }
    {
        DM2_V1_StartupMenuAuxPointerLayout aux_pointer_layout;
        uint8_t *credits_pixels = NULL;
        uint32_t menu_raw_hash = 0u;
        uint32_t menu_raw_byte_count = 0u;
        uint32_t credits_raw_hash = 0u;
        uint32_t credits_raw_byte_count = 0u;
        int credits_w = 0;
        int credits_h = 0;
        int credits_stride = 0;
        int sample_x = -1;
        int sample_y = -1;
        int credits_x;
        int credits_y;
        int dismiss_x;
        int dismiss_y;
        memset(&aux_pointer_layout, 0, sizeof(aux_pointer_layout));
        expect_true(dm2_v1_boot_startup_menu_aux_pointer_layout(
                        profile, &aux_pointer_layout) &&
                        aux_pointer_layout.valid &&
                        aux_pointer_layout.show_credits.w > 0 &&
                        aux_pointer_layout.show_credits.h > 0 &&
                        aux_pointer_layout.quit_game.w > 0 &&
                        aux_pointer_layout.quit_game.h > 0 &&
                        aux_pointer_layout.dismiss_credits.w > 0 &&
                        aux_pointer_layout.dismiss_credits.h > 0,
                    "M11 DM2 obtains credits, quit and dismiss rectangles from verified GDAT");
        expect_true(dm2_v1_boot_gdat_raw_asset_proof(profile, 5, 0, 4,
                                                      0x324d5257u,
                                                      &menu_raw_hash,
                                                      &menu_raw_byte_count) &&
                        dm2_v1_boot_gdat_raw_asset_proof(profile, 5, 0, 1,
                                                          0x32435244u,
                                                          &credits_raw_hash,
                                                          &credits_raw_byte_count) &&
                        menu_raw_hash != 0u && credits_raw_hash != 0u &&
                        menu_raw_byte_count > 0u && credits_raw_byte_count > 0u &&
                        menu_raw_hash != credits_raw_hash,
                    "M11 DM2 keeps separate original GDAT payloads for menu and credits");
        credits_x = aux_pointer_layout.show_credits.x +
            aux_pointer_layout.show_credits.w / 2;
        credits_y = aux_pointer_layout.show_credits.y +
            aux_pointer_layout.show_credits.h / 2;
        dismiss_x = aux_pointer_layout.dismiss_credits.x +
            aux_pointer_layout.dismiss_credits.w / 2;
        dismiss_y = aux_pointer_layout.dismiss_credits.y +
            aux_pointer_layout.dismiss_credits.h / 2;
        expect_true(M11_GameView_HandlePointer(&view, credits_x, credits_y, 1) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 1 &&
                        view.dm2State.startup_credits_remaining_ticks == 1800,
                    "M11 DM2 credits enter only through the source RAW4 rectangle");
        if (dm2_v1_boot_gdat_image_asset_fetch(profile, 5, 0, 1,
                                               &credits_pixels, &credits_w,
                                               &credits_h,
                                               &credits_stride) == 0 &&
            credits_pixels && credits_w == 320 && credits_h == 200 &&
            credits_stride >= credits_w) {
            DM2_V1_InterfacePalette palette;
            int palette_ready = dm2_v1_boot_interface_palette(profile,
                                                                &palette);
            int y;
            /* A nonzero high palette index cannot detect the old bug: only
             * BPP4 credits are remapped through dtPalette16.  Find a BPP8
             * source index whose interface-table mapping differs and prove
             * M11 keeps the original physical index. */
            for (y = 0; y < credits_h && sample_x < 0; ++y) {
                int x;
                for (x = 0; x < credits_w; ++x) {
                    unsigned char source =
                        credits_pixels[y * credits_stride + x];
                    if (palette_ready && source > 0u && source < 16u &&
                        palette.palette16[source] != source) {
                        sample_x = x;
                        sample_y = y;
                        break;
                    }
                }
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(palette_ready && sample_x >= 0 && sample_y >= 0 &&
                            framebuffer[sample_y * 320 + sample_x] ==
                                credits_pixels[sample_y * credits_stride + sample_x],
                        "M11 DM2 credits preserve palette-sensitive BPP8 source indices");
            {
                uint8_t presented_palette[256][3];
                expect_true(M11_Render_CopyIndexedPaletteRgb6(
                                    presented_palette) &&
                                memcmp(palette.rgb6, presented_palette,
                                       sizeof(palette.rgb6)) == 0,
                            "M11 DM2 credits present the verified dtPalIRGB palette");
            }
        } else {
            expect_true(0, "M11 DM2 credits fetches original TITLE dt07/1");
        }
        dm2_v1_boot_gdat_image_asset_free(credits_pixels);
        view.dm2State.startup_credits_remaining_ticks = 1;
        expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 0 &&
                        view.dm2State.startup_credits_remaining_ticks == 0,
                    "M11 DM2 credits return after the source 1,800-step countdown");
        expect_true(M11_GameView_HandlePointer(&view, credits_x, credits_y, 1) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 1,
                    "M11 DM2 credits may re-enter from the source menu event");
        expect_true(M11_GameView_HandlePointer(&view, dismiss_x, dismiss_y, 1) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 0 &&
                        view.dm2State.startup_credits_remaining_ticks == 0,
                    "M11 DM2 credits leave only through source event 239");
        expect_true(M11_GameView_HandlePointer(&view, credits_x, credits_y, 1) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 1,
                    "M11 DM2 credits may re-enter before a secondary click");
        expect_true(M11_GameView_HandlePointerButton(
                        &view, dismiss_x, dismiss_y,
                        DM1_V1_MOUSE_MASK_RIGHT_PC34) ==
                        M11_GAME_INPUT_REDRAW &&
                        view.dm2State.startup_credits_active == 0 &&
                        view.dm2State.startup_credits_remaining_ticks == 0,
                    "M11 DM2 credits leave through the source right-button 239 event");
    }
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_IGNORED &&
                    view.dm2State.startup_menu_active == 1,
                "M11 DM2 startup rejects unproven host keyboard navigation");
    view.world.party.championCount = 1;
    view.world.party.activeChampionIndex = 0;
    view.dm2State.leader_hand_object = dm2_db_make_handle(10, 0x0033);
    view.dm2State.champion_inventory_objects[0][CHAMPION_SLOT_HEAD] =
        dm2_db_make_handle(10, 0x0034);
    dm2_v1_runtime_set_leader_hand_object(
        view.dm2State.leader_hand_object);
    (void)dm2_v1_runtime_set_champion_inventory_object(
        0, CHAMPION_SLOT_HEAD,
        view.dm2State.champion_inventory_objects[0][CHAMPION_SLOT_HEAD]);
    expect_true(M11_GameView_HandlePointer(&view, 100, 60, 1) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 source NEW GAME rectangle reaches GAME_LOAD gate");
    expect_true(view.dm2State.startup_menu_active == 1,
                "M11 DM2 startup reloads the original dungeon but keeps missing source initialization gated without host text");
    expect_true(view.world.party.championCount == 0 &&
                    view.world.party.activeChampionIndex == -1 &&
                    view.dm2State.leader_hand_object == 0u &&
                    view.dm2State.champion_inventory_objects[0]
                        [CHAMPION_SLOT_HEAD] == 0u &&
                    dm2_v1_runtime_get_leader_hand_object() == 0u &&
                    dm2_v1_runtime_get_champion_inventory_object(
                        0, CHAMPION_SLOT_HEAD) == 0u,
                "M11 DM2 new game clears stale save party ownership before source mirror selection");
    profile = (DM2_V1_BootProfile *)view.dm2BootProfile;
    expect_true(profile &&
                    (profile_new_game_owner =
                        (const DM2_V1_GameLoadWorldOwner *)
                            dm2_v1_boot_prepared_new_game_world_readonly(
                                profile)) != NULL &&
                    profile_new_game_owner->source_preselection_ready &&
                    profile_new_game_owner->actuator_generators_processed &&
                    profile_new_game_owner->source_map_context_materialized &&
                    profile_new_game_owner->preselection_local_graphics.valid &&
                    profile_new_game_owner->preselection_light.valid &&
                    profile_new_game_owner->preselection_scene_materialized &&
                    profile_new_game_owner->preselection_scene_plan.valid &&
                    profile_new_game_owner->preselection_c_light.valid &&
                    profile_new_game_owner->preselection_view.valid &&
                    profile_new_game_owner->preselection_viewport.valid &&
                    profile_new_game_owner->preselection_viewport.map ==
                        profile_new_game_owner->source_party_map &&
                    profile_new_game_owner->preselection_viewport.map_data_hash ==
                        profile_new_game_owner->preselection_entrance_map.map_data_hash &&
                    profile_new_game_owner->preselection_viewport.source_cell_count > 0u &&
                    profile_new_game_owner->preselection_viewport.source_viewport_hash != 0u &&
                    dm2_test_preselection_view_matches_owner(
                        profile_new_game_owner) &&
                    !profile_new_game_owner->champion_selection_materialized &&
                    profile_new_game_owner->selected_party.heros_in_party == 0 &&
                    !profile->source_game_load_session_ready,
                "M11 NEW GAME retains the real post-GAME_LOAD pre-mirror owner without choosing a champion");
    if (profile && source_transaction.party.admissions[0].valid) {
        const DM2_V1_BootChampionSelectionCandidate *source_selection =
            &source_transaction.party.admissions[0].selection;
        DM2_V1_BootNewGamePartySelection mirror_click;

        memset(&mirror_click, 0, sizeof(mirror_click));
        mirror_click.map = source_selection->mirror.map;
        mirror_click.x = source_selection->mirror.x;
        mirror_click.y = source_selection->mirror.y;
        mirror_click.direction = source_selection->selection_direction;
        mirror_click.mirror_object_id = source_selection->mirror.object_id;
        expect_true(dm2_v1_boot_select_new_game_champion(profile,
                        &mirror_click) &&
                    (profile_new_game_owner =
                        (const DM2_V1_GameLoadWorldOwner *)
                            dm2_v1_boot_new_game_world_readonly(profile)) != NULL &&
                    profile_new_game_owner->selected_mirror_count == 1u &&
                    profile_new_game_owner->selected_party.heros_in_party == 1 &&
                    profile_new_game_owner->champion_selection_materialized &&
                    !profile->source_game_load_session_ready &&
                    view.world.party.championCount == 0 &&
                    view.dm2State.leader_hand_object == 0u,
                    "DM2 routes a real first mirror click through the profile-owned GAME_LOAD world only");
    }
    /* The following runtime/save assertions exercise resume separately. A
     * New Game may not create the former fixture party merely to enter this
     * block; GAME_LOAD owns that source transition. */
    if (!view.dm2State.startup_menu_active) {
    memset(&boot_receipt, 0, sizeof(boot_receipt));
    expect_true(M11_GameView_GetBootProbeReceipt(&view, &boot_receipt) &&
                    boot_receipt.startupActive == 0 &&
                    strcmp(boot_receipt.startupPhase, "dm2-runtime") == 0 &&
                    strcmp(boot_receipt.startupAnimation, "dm2-runtime") == 0 &&
                    boot_receipt.startupAnimationActive == 0 &&
                    boot_receipt.startupTitleFrame == 0 &&
                    boot_receipt.startupTitleFrameMax == 0 &&
                    boot_receipt.startupTitleReady == 1 &&
                    boot_receipt.startupInitializeV2Runtime == 1 &&
                    boot_receipt.startupInitializeHudRuntime == 1 &&
                    boot_receipt.startupInitializeTouchRuntime == 1 &&
                    boot_receipt.startupHudRuntimeReady == 1,
                "M11 DM2 boot probe reaches first runtime HUD state after startup menu");

    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (profile) {
        DM2_V1_BootRuntimeHudCaptureReceipt runtime_hud_capture;
        memset(&runtime_hud_capture, 0, sizeof(runtime_hud_capture));
        expect_true(dm2_v1_boot_runtime_hud_capture_receipt(
                        profile,
                        &runtime_hud_capture) == 1 &&
                        runtime_hud_capture.valid == 1 &&
                        runtime_hud_capture.real_gdat_runtime_hud_breadth_ready == 1 &&
                        runtime_hud_capture.runtime_direction_mask == 0x0f &&
                        runtime_hud_capture.runtime_turn_count == 4 &&
                        runtime_hud_capture.unique_frame_hash_count > 0 &&
                        runtime_hud_capture.min_asset_portrait_count >= 4 &&
                        runtime_hud_capture.total_fallback_portrait_count == 0 &&
                        runtime_hud_capture.min_asset_floor_ceiling_count >= 2 &&
                        runtime_hud_capture.min_asset_wall_count > 0 &&
                        runtime_hud_capture.total_fallback_floor_ceiling_count == 0 &&
                        runtime_hud_capture.total_fallback_wall_count == 0 &&
                        runtime_hud_capture.total_fallback_door_count == 0 &&
                        runtime_hud_capture.raw_gdat_runtime_hud_capture_ready == 1 &&
                        runtime_hud_capture.raw_gdat_runtime_portrait_count >= 4 &&
                        runtime_hud_capture.raw_gdat_runtime_portrait_hash != 0u &&
                        runtime_hud_capture.raw_gdat_runtime_portrait_byte_count > 0u &&
                        runtime_hud_capture.raw_gdat_runtime_core_hash != 0u &&
                        runtime_hud_capture.raw_gdat_runtime_core_byte_count > 0u &&
                        runtime_hud_capture.decoded_gdat_runtime_hud_capture_ready == 1 &&
                        runtime_hud_capture.decoded_gdat_runtime_portrait_count >= 4 &&
                        runtime_hud_capture.decoded_gdat_runtime_portrait_hash != 0u &&
                        runtime_hud_capture.decoded_gdat_runtime_portrait_pixel_count > 0u &&
                        runtime_hud_capture.decoded_gdat_runtime_core_hash != 0u &&
                        runtime_hud_capture.decoded_gdat_runtime_core_pixel_count > 0u &&
                        runtime_hud_capture.real_gdat_core_render_ready == 1,
                    "M11 DM2 runtime owns broad real GDAT HUD capture receipt");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(dm2_v1_runtime_last_asset_hud_portrait_count() >= 4 &&
                        dm2_v1_runtime_last_fallback_hud_portrait_count() == 0 &&
                        dm2_v1_runtime_last_asset_floor_ceiling_count() >= 2 &&
                        dm2_v1_runtime_last_fallback_floor_ceiling_count() == 0 &&
                        dm2_v1_runtime_last_asset_wall_count() > 0 &&
                        dm2_v1_runtime_last_fallback_wall_count() == 0 &&
                        dm2_v1_runtime_last_fallback_door_count() == 0 &&
                        view.lastOutcome[0] == '\0',
                    "M11 DM2 runtime presents a real GDAT frame without host status text");
    }

    memset(&dm2_frame_receipt, 0, sizeof(dm2_frame_receipt));
    expect_true(dm2_v1_runtime_last_m11_frame_receipt(&dm2_frame_receipt) &&
                    dm2_frame_receipt.composition.scene_record_owned &&
                    dm2_frame_receipt.composition.scene_light_owned &&
                    dm2_frame_receipt.composition.map_load_token != 0u &&
                    dm2_frame_receipt.composition.scene_control_hash != 0u,
                "DM2 V1 frame binds static GRAPHICSSET scene records to its verified map token");

    /* Retained decoder-fixture notes only: these paths construct D2RS
     * sessions and cannot stand in for SKProject's original save writer. */
#if 0
    expect_true(make_temp_save_root(direct_save_root),
                "created isolated DM2 direct-start quick-save root");
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (profile) {
        DM2_ChampionRecord *direct_champ;
        dm2_v1_boot_set_save_root(profile, direct_save_root);
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_SAVE_GAME) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 direct-start save command writes SKSave.dat");
        memset(&direct_session, 0, sizeof(direct_session));
        expect_true(dm2_v1_session_load_last_session(direct_save_root,
                                                     &direct_session) == 0,
                    "M11 DM2 direct-start save command writes loadable last session");
        direct_champ =
            (DM2_ChampionRecord *)direct_session.champion_data[0];
        expect_true(direct_session.champion_count == 4,
                    "M11 DM2 direct-start quick-save preserves starter party count");
        expect_true(direct_session.party_x == 15 &&
                    direct_session.party_y == 15 &&
                    direct_session.party_dir == 0 &&
                    direct_session.party_level == 0,
                    "M11 DM2 direct-start quick-save preserves boot pose");
        expect_true(direct_session.game_tick == 0,
                    "M11 DM2 direct-start quick-save preserves boot tick");
        expect_true(direct_session.original_leader_hand_object == 0u,
                    "M11 DM2 direct-start quick-save preserves empty leader hand");
        expect_true(direct_champ->first_name[0] != '\0',
                    "M11 DM2 direct-start quick-save preserves starter champion data");
    }
    snprintf(direct_save_path, sizeof(direct_save_path), "%s%sSKSave.dat",
             direct_save_root, TEST_PATH_SEP);
    M11_GameView_Shutdown(&view);
    expect_true(!dm2_v1_sound_playback_backend_bound(),
                "M11 DM2 shutdown unbinds the verified playback backend");
    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = direct_save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 direct-start SKSave.dat resume succeeds");
    expect_true(view.lastOutcome[0] == '\0',
                "M11 DM2 direct-start SKSave.dat resume keeps host status silent");
    expect_true(view.dm2State.party_x == 15 &&
                view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 0,
                "M11 DM2 direct-start SKSave.dat restores boot pose");
    expect_true(view.dm2State.tick_count == 0,
                "M11 DM2 direct-start SKSave.dat restores boot tick");
    expect_true(dm2_v1_runtime_get_tick_count() == 0,
                "M11 DM2 direct-start SKSave.dat restores runtime tick");
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (profile) {
        dm2_v1_boot_set_save_root(profile, direct_save_root);
        view.dm2State.startup_menu_active = 1;
        view.dm2State.startup_menu_selected_row = 0;
        view.dm2State.startup_resume_available = 1;
        view.dm2State.startup_slot_mask = 0u;
        view.dm2State.startup_menu_row_count = 2;
        snprintf(view.dm2State.startup_save_root,
                 sizeof(view.dm2State.startup_save_root),
                 "%s",
                 profile->save_root);
        expect_true(M11_GameView_HandlePointer(&view, 20, 70, 1) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 source RESUME rectangle loads SKSave.dat");
        expect_true(view.dm2State.startup_menu_active == 0,
                    "M11 DM2 startup menu dismisses after CONTINUE");
        expect_true(strstr(view.lastOutcome, "DM2 CONTINUED") != NULL,
                    "M11 DM2 startup menu CONTINUE reports continued status");
        expect_true(view.dm2State.party_x == 15 &&
                    view.dm2State.party_y == 15 &&
                    view.dm2State.party_dir == 0,
                    "M11 DM2 startup menu CONTINUE mirrors saved boot pose");
    }

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 turn-right input redraws through runtime");
    expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 1,
                "M11 DM2 turn-right rotates in place");
    expect_true(dm2_v1_runtime_get_party_x() == 15 &&
                dm2_v1_runtime_get_party_y() == 15 &&
                dm2_v1_runtime_get_party_dir() == 1,
                "DM2 runtime turn-right keeps boot position");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_LEFT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 turn-left input redraws through runtime");
    expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 0,
                "M11 DM2 turn-left restores boot facing");
    world = (DM2_V1_GameState*)view.dm2World;
    {
        int reputation_before = world ? world->reputation : -1;
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 action input redraws through runtime interaction");
        expect_true(view.lastOutcome[0] == '\0',
                    "M11 DM2 action keeps host-authored status text unbound");
        expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                    view.dm2State.party_dir == 0,
                    "M11 DM2 action does not move or rotate the party");
        (void)reputation_before;
    }
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (world && profile && profile->dungeon_data) {
        DM2_V1_DungeonData* dungeon =
            (DM2_V1_DungeonData*)profile->dungeon_data;
        int door_level = 0;
        int door_party_x = 15;
        int door_party_y = 15;
        int raw_before;
        int has_door_pose = find_in_bounds_door_pose(
            dungeon, &door_level, &door_party_x, &door_party_y);
        expect_true(has_door_pose,
                    "M11 DM2 action test finds an in-bounds real-data pose");
        raw_before = dm2_v1_dungeon_get_tile_raw(
            dungeon, door_level, door_party_x, door_party_y - 1);
        dm2_v1_runtime_set_position(
            door_level, door_party_x, door_party_y, 0);
        dm2_v1_runtime_set_outdoor(0);
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 action rejects an unowned door transaction");
        expect_true(view.lastOutcome[0] == '\0',
                    "M11 DM2 rejected action keeps host text unbound");
        expect_true(view.inspectTitle[0] == '\0' &&
                        view.inspectDetail[0] == '\0',
                    "M11 DM2 rejected action keeps inspect text unbound");
        expect_true(dm2_v1_dungeon_get_tile_raw(
                        dungeon, door_level, door_party_x, door_party_y - 1) ==
                        raw_before,
                    "M11 DM2 action cannot rewrite an original dungeon tile");
        dm2_v1_runtime_set_position(0, 15, 15, 0);
        view.dm2State.party_x = 15;
        view.dm2State.party_y = 15;
        view.dm2State.party_dir = 0;
        dm2_v1_runtime_set_outdoor(0);
    }
    if (world) {
        int found_unowned_square = 0;
        int target_x = 0;
        int target_y = 0;
        for (int y = 0; y < 31 && !found_unowned_square; ++y) {
            for (int x = 0; x < 32 && !found_unowned_square; ++x) {
                if (dm2_v1_runtime_get_square_type(0, x, y) >= 0) {
                    found_unowned_square = 1;
                    target_x = x;
                    target_y = y;
                }
            }
        }
        expect_true(found_unowned_square,
                    "found a bounded real-data square without a proven NPC owner");
        if (found_unowned_square) {
            int reputation_before = world->reputation;
            /* SKProject SKWINSPX/src/v4/skcrture.cpp:5368-5444 and
             * 5697-5700 require a live AI-33 creature, its DB record and
             * CCM merchandise state. A map coordinate is not an NPC. */
            expect_true(dm2_v1_runtime_npc_interact(0, target_x, target_y) < 0,
                        "DM2 NPC interaction rejects a coordinate without live creature ownership");
            expect_true(world->reputation == reputation_before &&
                            dm2_v1_runtime_get_last_npc_id() == -1 &&
                            dm2_v1_runtime_get_last_npc_dialog_line() == -1,
                        "rejected DM2 NPC interaction cannot synthesize reputation or dialogue state");
            dm2_v1_runtime_set_position(0, target_x, target_y + 1, 0);
            dm2_v1_runtime_set_outdoor(1);
            expect_true(M11_GameView_HandleInput(&view,
                                                 M12_MENU_INPUT_ACTION) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 DM2 action leaves an unowned real-data square unavailable");
            expect_true(world->reputation == reputation_before &&
                            view.lastOutcome[0] == '\0' &&
                            view.inspectTitle[0] == '\0' &&
                            view.inspectDetail[0] == '\0',
                        "M11 DM2 unavailable NPC route leaves world and host text unchanged");
            dm2_v1_runtime_set_position(0, 15, 15, 0);
            dm2_v1_runtime_set_outdoor(0);
        }
    }
    if (world) {
        DM2_V1_DungeonData *dd =
            profile ? (DM2_V1_DungeonData *)profile->dungeon_data : NULL;
        dm2_v1_trigger_reset_state();
        dm2_v1_plate_reset_state();
        dm2_v1_plate_set_party_weight(500);
        if (dd && dm2_v1_dungeon_set_tile_raw(dd, 0, 16, 8, 4) == 0 &&
            dm2_v1_dungeon_set_tile_raw(dd, 0, 13, 8, 4) == 0) {
            dm2_v1_runtime_set_position(0, 15, 9, 0);
            dm2_v1_runtime_set_outdoor(1);
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 DM2 forward move reaches square-trigger route");
            expect_true(dm2_v1_trigger_get_builtin_count() == 0 &&
                            dm2_v1_trigger_total_fires() == 0,
                        "DM2 runtime refuses the retired fixture trigger catalog");
            expect_true(view.dm2State.party_x == 15 &&
                        view.dm2State.party_y == 8,
                        "M11 DM2 mirror follows trigger-square movement");
            expect_true(dm2_v1_dungeon_get_tile_raw(dd, 0, 16, 8) == 4,
                        "DM2 fixture trigger cannot alter a source dungeon tile");
            dm2_v1_runtime_set_position(0, 12, 9, 0);
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 DM2 forward move reaches pressure-plate route");
            expect_true(dm2_v1_plate_get_builtin_count() == 0 &&
                            dm2_v1_plate_fire_total() == 0,
                        "DM2 runtime refuses the retired fixture pressure-plate catalog");
            expect_true(dm2_v1_dungeon_get_tile_raw(dd, 0, 13, 8) == 4,
                        "DM2 fixture pressure plate cannot alter a source dungeon tile");
        }
        dm2_v1_runtime_set_position(0, 15, 15, 0);
        view.dm2State.party_x = 15;
        view.dm2State.party_y = 15;
        view.dm2State.party_dir = 0;
        dm2_v1_runtime_set_outdoor(0);
    }
    if (profile && profile->dungeon_data) {
        DM2_V1_DungeonData* dungeon =
            (DM2_V1_DungeonData*)profile->dungeon_data;
        int draw_level = 0;
        int draw_party_x = 15;
        int draw_party_y = 15;
        int has_draw_pose = find_in_bounds_door_pose(
            dungeon, &draw_level, &draw_party_x, &draw_party_y);
        expect_true(has_draw_pose,
                    "DM2 M11 draw finds an in-bounds door render pose");
        expect_true(dm2_v1_dungeon_set_tile_raw(
                        dungeon,
                        draw_level, draw_party_x, draw_party_y - 1, 4u) == 0,
                    "DM2 M11 draw seeds a front door tile for asset-backed door-frame proof");
        expect_true(dm2_v1_dungeon_set_tile_raw(
                        dungeon,
                        draw_level, draw_party_x, draw_party_y - 2, 4u) == 0,
                    "DM2 M11 draw seeds a D1C door tile for asset-backed door-frame proof");
        expect_true(dm2_v1_dungeon_set_tile_raw(
                        dungeon,
                        draw_level, draw_party_x, draw_party_y - 3, 4u) == 0,
                    "DM2 M11 draw seeds a D2C door tile for asset-backed door-frame proof");
        dm2_v1_runtime_set_position(draw_level, draw_party_x, draw_party_y, 0);
        view.dm2State.party_x = draw_party_x;
        view.dm2State.party_y = draw_party_y;
        view.dm2State.party_dir = 0;
        dm2_v1_runtime_set_outdoor(0);

        /* SKProject's DRAW_DUNGEON draws a complete source frame at the
         * committed party pose. It supplies no intermediate camera raster.
         * V2 may track smooth input timing, but it must present the exact V1
         * frame instead of shifting it and manufacturing an exposed strip. */
        memset(framebuffer_without_hand, 0, sizeof(framebuffer_without_hand));
        memset(framebuffer, 0, sizeof(framebuffer));
        dm2_v2_runtime_v1_tick(0);
        dm2_v2_runtime_smooth_walk((float)draw_party_x,
                                   (float)draw_party_y,
                                   (float)(draw_party_x + 1),
                                   (float)draw_party_y);
        expect_true(dm2_v2_smooth_is_active(
                        &dm2_v2_runtime_get_viewport()->smooth),
                    "DM2 V2 smooth timing is active for source-raster comparison");
        expect_true(dm2_v1_runtime_render_frame(0, draw_party_x, draw_party_y,
                                                 framebuffer_without_hand,
                                                 320, 320, 200) == 0 &&
                        dm2_v2_runtime_render_frame(0, draw_party_x, draw_party_y,
                                                    framebuffer,
                                                    320, 320, 200) == 0 &&
                        memcmp(framebuffer_without_hand, framebuffer,
                               sizeof(framebuffer)) == 0,
                    "DM2 V2 smooth timing preserves the real V1 viewport raster");
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    {
        DM2_V1_InterfacePalette palette;
        uint8_t presented_palette[256][3];
        int palette_ready = dm2_v1_boot_interface_palette(profile, &palette);
        expect_true(palette_ready &&
                        M11_Render_CopyIndexedPaletteRgb6(presented_palette) &&
                        memcmp(palette.rgb6, presented_palette,
                               sizeof(palette.rgb6)) == 0,
                    "M11 DM2 title/menu/HUD presentation uses GDAT dtPalIRGB palette");
    }
    expect_true(framebuffer[(100 * 320) + 160] != 0,
                "M11 DM2 draw fills the runtime viewport body");
    expect_true(dm2_v1_runtime_last_asset_floor_ceiling_count() == 2 &&
                dm2_v1_runtime_last_fallback_floor_ceiling_count() == 0,
                "M11 DM2 draw uses real GRAPHICSSET GDAT floor/ceiling strips");
    expect_true(dm2_v1_runtime_last_asset_wall_count() == 10 &&
                dm2_v1_runtime_last_fallback_wall_count() == 0,
                "M11 DM2 draw uses real GRAPHICSSET GDAT viewport-cell wall images");
    expect_true(dm2_v1_runtime_last_asset_door_panel_count() == 3,
                "M11 DM2 draw uses real DOORS GDAT D0C/D1C/D2C door-panel images");
    expect_true(dm2_v1_runtime_last_asset_door_frame_count() == 3 &&
                dm2_v1_runtime_last_fallback_door_count() == 0,
                "M11 DM2 draw uses real GRAPHICSSET GDAT D0C/D1C/D2C door-frame images");
    {
        uint32_t icon_handle = 0u;
        int name_x = 0;
        int name_y = 0;
        int name_w = 0;
        int name_h = 0;

        memcpy(framebuffer_without_hand, framebuffer, sizeof(framebuffer));
        dm2_v1_runtime_set_leader_hand_object(dm2_db_make_handle(10, 0x0033));
        view.dm2State.leader_hand_object =
            dm2_v1_runtime_get_leader_hand_object();
        expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                        dm2_db_make_handle(10, 0x0033),
                    "M11 DM2 exposes the leader-hand ObjectID without V1 THING casting");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        {
            DM1_V1_LayoutZoneRectPc34 name_rect =
                dm1_v1_leader_hand_object_name_rect_pc34();
            expect_true(dm1_v1_leader_hand_object_name_zone_id_pc34() != 0,
                        "DM1 leader-hand name zone is available");
            name_x = name_rect.x;
            name_y = name_rect.y;
            name_w = name_rect.w;
            name_h = name_rect.h;
        }
        expect_true(!framebuffer_zone_differs(framebuffer_without_hand,
                                              framebuffer,
                                              320,
                                              200,
                                              name_x,
                                              name_y,
                                              name_w,
                                              name_h),
                    "M11 DM2 leaves the DM1 leader-hand name zone untouched");
        dm2_v1_runtime_set_leader_hand_object(0u);
        view.dm2State.leader_hand_object = 0u;

        profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
        expect_true(find_loadable_dm2_object_icon_handle(profile, &icon_handle),
                    "DM2 boot profile can resolve at least one object icon from GRAPHICS.DAT");
        if (icon_handle != 0u) {
            loadable_icon_handle = icon_handle;
        }
        if (icon_handle != 0u) {
            int icon_x = -1;
            int icon_y = -1;
            int icon_w = -1;
            int icon_h = -1;
            dm2_v1_runtime_set_leader_hand_object(icon_handle);
            view.dm2State.leader_hand_object =
                dm2_v1_runtime_get_leader_hand_object();
            expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                            icon_handle,
                        "M11 DM2 leader-hand ObjectID accessor follows runtime icon handle");
            expect_true(!M11_GameView_Dm2LeaderHandObjectIconAvailable(&view),
                        "M11 DM2 blocks leader-hand presentation without a source rect");
            expect_true(!M11_GameView_GetDm2LeaderHandObjectIconZone(
                            &icon_x, &icon_y, &icon_w, &icon_h) &&
                            icon_x == 0 && icon_y == 0 &&
                            icon_w == 0 && icon_h == 0 &&
                            !M11_GameView_GetDm2LeaderHandObjectCursorIconZone(
                                &view, &icon_x, &icon_y, &icon_w, &icon_h),
                        "M11 DM2 exposes no placeholder leader-hand icon zone");
            expect_true(M11_GameView_HandlePointerMove(&view, 120, 80) ==
                            M11_GAME_INPUT_IGNORED,
                        "M11 DM2 pointer motion cannot request an unowned icon redraw");
            expect_true(M11_GameView_HandleInput(&view,
                                                 M12_MENU_INPUT_INVENTORY_TOGGLE) ==
                            M11_GAME_INPUT_REDRAW,
                        "M11 DM2 inventory toggle is handled without opening a DM1 panel");
            expect_true(!M11_GameView_IsInventoryPanelActive(&view) &&
                        view.lastOutcome[0] == '\0',
                        "M11 DM2 inventory remains silently blocked until its GDAT layout is bound");
            expect_true(!M11_GameView_IsInventoryPanelActive(&view),
                        "M11 DM2 startup inventory panel remains unavailable");
            expect_true(M11_GameView_HandlePointerMove(&view, 319, 199) ==
                            M11_GAME_INPUT_IGNORED,
                        "M11 DM2 pointer motion stays ignored without an icon route");
            dm2_v1_runtime_set_leader_hand_object(0u);
            view.dm2State.leader_hand_object = 0u;
        }
    }

    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    world = (DM2_V1_GameState*)view.dm2World;
    if (profile) {
        expect_true(profile->assets_verified == 1,
                    "boot profile remains hash verified");
        expect_true(strcmp(profile->game_id, "dm2") == 0,
                    "boot profile game id is dm2");
        expect_true(profile->dm2_state == view.dm2World,
                    "boot profile owns the M11 DM2 world pointer");
        expect_true(profile->dungeon_data != NULL,
                    "boot profile owns loaded dungeon data");
        expect_true(profile->graphics_dat != NULL,
                    "boot profile owns loaded GRAPHICS.DAT asset handle");
        expect_true(strcmp(profile->dungeon_path, view.dungeonPath) == 0,
                    "M11 dungeonPath mirrors the verified profile path");
    }
    if (world) {
        dm2_v1_runtime_set_position(0, 15, 15, 0);
        view.dm2State.party_x = 15;
        view.dm2State.party_y = 15;
        view.dm2State.party_dir = 0;
        dm2_v1_runtime_set_outdoor(0);
        expect_true(world->party_x == 15 && world->party_y == 15 &&
                    world->party_dir == 0,
                    "DM2 V1 world starts at the source-locked boot pose");
        expect_true(world->current_level == 0,
                    "DM2 V1 world starts on level zero");
        world->gold = 375;
        dm2_v1_shop_reset_state();
        dm2_v1_runtime_set_position(0, 10, 6, 0);
        dm2_v1_runtime_set_outdoor(1);
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 action handles a shop wall from the front square");
        expect_true(view.lastOutcome[0] == '\0',
                    "M11 DM2 shop action fails closed without host status text");
        expect_true(dm2_v1_shop_is_active() == 0,
                    "M11 DM2 does not retain the catalog shop state");
        expect_true(dm2_v1_shop_get_party_gold() == 375u,
                    "DM2 runtime shop action syncs party gold into shop state");
        expect_true(view.dm2State.party_x == 10 && view.dm2State.party_y == 6 &&
                    view.dm2State.party_dir == 0,
                    "M11 DM2 shop action mirrors the runtime shop-facing pose");
        expect_true(view.dm2ShopSelectedStockIndex == 0 &&
                    view.dm2ShopSelectedInventoryIndex == 0,
                    "M11 DM2 clears the retired catalog selection state");
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(framebuffer[(24 * 320) + 16] != 11 &&
                    framebuffer[(57 * 320) + 23] != 12,
                    "M11 DM2 does not draw the retired catalog shop overlay");
        expect_true(dm2_v1_shop_get_party_gold() == 375u &&
                    dm2_v1_shop_get_state()->inventory_count == 0 &&
                    dm2_v1_shop_buy_count() == 0 &&
                    dm2_v1_shop_sell_count() == 0,
                    "M11 DM2 shop wall cannot mutate catalog transaction state");
    }

    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "DM2 M11 idle tick dispatches through the DM2 boundary");
    expect_true(view.dm2State.tick_count == 1,
                "DM2 M11 mirror tick advances once");
    expect_true(dm2_v1_runtime_get_tick_count() == 1,
                "DM2 V1 runtime tick advances once");
    expect_true(view.dm2State.party_x == dm2_v1_runtime_get_party_x() &&
                view.dm2State.party_y == dm2_v1_runtime_get_party_y() &&
                view.dm2State.party_dir == dm2_v1_runtime_get_party_dir(),
                "M11 DM2 mirror state stays aligned with runtime accessors");
    world = (DM2_V1_GameState*)view.dm2World;
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (world && profile) {
        int saved_gold = world->gold;
        int saved_reputation = world->reputation;
        int saved_x = world->party_x;
        int saved_y = world->party_y;
        int saved_dir = world->party_dir;
        int saved_level = world->current_level;
        int saved_outdoor = world->outdoor;

        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_SAVE_GAME) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 live startup save command writes mutated world session");
        memset(&direct_session, 0, sizeof(direct_session));
        expect_true(dm2_v1_session_load_last_session(direct_save_root,
                                                     &direct_session) == 0,
                    "M11 DM2 live startup save command writes loadable mutated session");
        expect_true((int)direct_session.gold == saved_gold,
                    "M11 DM2 live quick-save preserves runtime shop gold");
        expect_true((int)direct_session.reputation == saved_reputation,
                    "M11 DM2 live quick-save preserves runtime NPC reputation");
        expect_true(direct_session.party_x == (uint16_t)saved_x &&
                    direct_session.party_y == (uint16_t)saved_y &&
                    direct_session.party_dir == (uint8_t)(saved_dir & 3) &&
                    direct_session.party_level == (uint8_t)saved_level,
                    "M11 DM2 live quick-save preserves current pose and level");
        expect_true(direct_session.outdoor_mode ==
                        (uint8_t)(saved_outdoor ? 1 : 0),
                    "M11 DM2 live quick-save preserves outdoor mode");
        expect_true(direct_session.game_tick == 1,
                    "M11 DM2 live quick-save preserves advanced runtime tick");

        snprintf(direct_save_path, sizeof(direct_save_path), "%s%sSKSave.dat",
                 direct_save_root, TEST_PATH_SEP);
        M11_GameView_Shutdown(&view);
        fill_dm2_launch_spec(&spec, data_dir);
        spec.savePath = direct_save_path;
        M11_GameView_Init(&view);
        expect_true(M11_GameView_Start(&view, &spec),
                    "M11 DM2 live-mutated SKSave.dat resume succeeds");
        world = (DM2_V1_GameState*)view.dm2World;
        expect_true(view.lastOutcome[0] == '\0',
                    "M11 DM2 live-mutated SKSave.dat resume keeps host status silent");
        expect_true(world && world->gold == saved_gold &&
                    world->reputation == saved_reputation,
                    "M11 DM2 live-mutated SKSave.dat restores gold and reputation");
        expect_true(world && world->party_x == saved_x &&
                    world->party_y == saved_y &&
                    world->party_dir == saved_dir &&
                    world->current_level == saved_level &&
                    world->outdoor == saved_outdoor,
                    "M11 DM2 live-mutated SKSave.dat restores pose, level, and outdoor mode");
        expect_true(view.dm2State.tick_count == 1 &&
                    dm2_v1_runtime_get_tick_count() == 1,
                    "M11 DM2 live-mutated SKSave.dat restores advanced tick");
    }

    }
    M11_GameView_Shutdown(&view);
    expect_true(view.dm2BootProfile == NULL && view.dm2World == NULL,
                "M11 shutdown clears DM2 boot ownership");
    if (direct_save_root[0]) {
        remove_temp_save_root(direct_save_root);
    }
#endif
    }

    expect_true(make_temp_save_root(save_root),
                "created isolated DM2 resume save root");
#if 0
    memset(&resume_session, 0, sizeof(resume_session));
    dm2_v1_test_session_fixture_new(&resume_session);
    resume_session.game_tick = 42;
    resume_session.party_x = 23;
    resume_session.party_y = 11;
    resume_session.party_dir = 2;
    resume_session.party_level = 1;
    resume_session.outdoor_mode = 1;
    resume_session.time_of_day_minutes = 990;
    resume_session.rain_intensity = 60;
    resume_session.original_leader_hand_object = dm2_db_make_handle(10, 0x0033);
    if (loadable_icon_handle != 0u) {
        DM2_ChampionRecord *resume_champ =
            (DM2_ChampionRecord *)resume_session.champion_data[0];
        DM2_ChampionRecord *resume_champ1 =
            (DM2_ChampionRecord *)resume_session.champion_data[1];
        resume_champ->inventory[CHAMPION_SLOT_HEAD] = loadable_icon_handle;
        resume_champ1->inventory[CHAMPION_SLOT_HEAD] = loadable_icon_handle;
    }
    expect_true(write_fixture_d2rs_slot(save_root, 3, "M11 Resume",
                                         &resume_session),
                "wrote DM2 SKSave03.dat resume fixture");
    expect_true(write_fixture_d2rs_last_session(save_root, "M11 Resume",
                                                 &resume_session),
                "wrote DM2 SKSave.dat source-resume fixture");
    snprintf(save_path, sizeof(save_path), "%s%sSKSave03.dat",
             save_root, TEST_PATH_SEP);

    fill_dm2_launch_spec(&spec, data_dir);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 startup slot-menu fixture launch succeeds");
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (profile) {
        DM2_V1_StartupMenuPointerLayout pointer_layout;
        DM2_V1_StartupMenuPointerHitReceipt pointer_hit;
        int present_x = 0;
        int present_y = 0;
        int present_w = 0;
        int present_h = 0;
        int window_x;
        int window_y;
        int source_x = 0;
        int source_y = 0;
        dm2_v1_boot_set_save_root(profile, save_root);
        view.dm2State.startup_menu_active = 1;
        view.dm2State.startup_menu_selected_row = 0;
        view.dm2State.startup_resume_available = 0;
        view.dm2State.startup_slot_mask = (1u << 3);
        view.dm2State.startup_menu_row_count = 2;
        snprintf(view.dm2State.startup_save_root,
                 sizeof(view.dm2State.startup_save_root),
                 "%s",
                 profile->save_root);
        memset(&pointer_layout, 0, sizeof(pointer_layout));
        expect_true(dm2_v1_boot_startup_menu_pointer_layout(
                        profile, &pointer_layout) == 1 &&
                    pointer_layout.valid == 1 &&
                    pointer_layout.table_hash != 0u &&
                    pointer_layout.new_game.w > 0 &&
                    pointer_layout.new_game.h > 0,
                    "DM2 startup derives the original NEW hit rectangle from dt04/0");
        expect_true(dm2_v1_boot_startup_menu_pointer_hit(
                        profile,
                        pointer_layout.new_game.x +
                            pointer_layout.new_game.w / 2,
                        pointer_layout.new_game.y +
                            pointer_layout.new_game.h / 2,
                        &pointer_hit) == 1 &&
                    pointer_hit.valid == 1 &&
                    pointer_hit.target == DM2_V1_STARTUP_POINTER_TARGET_NEW_GAME &&
                    pointer_hit.table_hash == pointer_layout.table_hash,
                    "DM2 startup records the original NEW event surface");
        expect_true(dm2_v1_boot_startup_menu_pointer_hit(
                        profile,
                        pointer_layout.resume_game.x +
                            pointer_layout.resume_game.w / 2,
                        pointer_layout.resume_game.y +
                            pointer_layout.resume_game.h / 2,
                        &pointer_hit) == 1 &&
                    pointer_hit.valid == 1 &&
                    pointer_hit.target ==
                        DM2_V1_STARTUP_POINTER_TARGET_RESUME_GAME &&
                    pointer_hit.table_hash == pointer_layout.table_hash,
                    "DM2 startup binds original 0xD9 geometry to RESUME");
        /* SDL mouse positions are Cocoa logical window points. Exercise the
         * same fit/content inverse used before M11 sends a pointer into the
         * source-owned 0xD7 rectangle. The 2x drawable is intentionally not
         * used as an input extent. */
        source_x = pointer_layout.new_game.x + pointer_layout.new_game.w / 2;
        source_y = pointer_layout.new_game.y + pointer_layout.new_game.h / 2;
        expect_true(M11_Render_ComputePresentationRect(
                        1512, 982, 320, 200, M11_SCALE_FIT, 0,
                        M11_DISPLAY_ASPECT_CONTENT,
                        &present_x, &present_y, &present_w, &present_h) ==
                        M11_RENDER_OK &&
                    present_w > 0 && present_h > 0,
                    "DM2 startup computes a logical fit rectangle for pointer input");
        window_x = present_x + (source_x * present_w) / 320;
        window_y = present_y + (source_y * present_h) / 200;
        expect_true(M11_Render_MapPointToFramebuffer(
                        window_x, window_y, 1512, 982, 320, 200,
                        M11_SCALE_FIT, 0, M11_DISPLAY_ASPECT_CONTENT,
                        &source_x, &source_y) == 1 &&
                    source_x >= pointer_layout.new_game.x &&
                    source_x < pointer_layout.new_game.x + pointer_layout.new_game.w &&
                    source_y >= pointer_layout.new_game.y &&
                    source_y < pointer_layout.new_game.y + pointer_layout.new_game.h,
                    "DM2 logical window click maps back into original NEW rectangle");
        expect_true(M11_GameView_HandlePointerButton(
                        &view, source_x, source_y,
                        DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 startup logical pointer consumes original NEW rectangle");
        expect_true(view.dm2State.startup_menu_active == 1,
                    "M11 DM2 startup NEW pointer keeps GAME_LOAD behind title menu");
        expect_true(strstr(view.lastOutcome, "DM2 GAME_LOAD") != NULL,
                    "M11 DM2 startup NEW pointer keeps GAME_LOAD source-owned");
    }
    M11_GameView_Shutdown(&view);

    /* skproject HANDLE_UI_EVENT maps the source-owned 0xD9 rectangle to
     * RESUME. Exercise that exact hit against an admitted SKSave.dat instead
     * of routing it through Firestaff's historical synthetic menu rows. */
    fill_dm2_launch_spec(&spec, data_dir);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 source-resume menu fixture launch succeeds");
    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    if (profile) {
        DM2_V1_StartupMenuPointerLayout pointer_layout;
        int source_x;
        int source_y;

        dm2_v1_boot_set_save_root(profile, save_root);
        view.dm2State.startup_menu_active = 1;
        view.dm2State.startup_menu_selected_row = 0;
        view.dm2State.startup_resume_available = 1;
        view.dm2State.startup_slot_mask = (1u << 3);
        view.dm2State.startup_menu_row_count = 3;
        snprintf(view.dm2State.startup_save_root,
                 sizeof(view.dm2State.startup_save_root),
                 "%s",
                 profile->save_root);
        memset(&pointer_layout, 0, sizeof(pointer_layout));
        expect_true(dm2_v1_boot_startup_menu_pointer_layout(
                        profile, &pointer_layout) == 1 &&
                    pointer_layout.valid == 1,
                    "DM2 source-resume pointer layout is admitted from GDAT");
        source_x = pointer_layout.resume_game.x +
                   pointer_layout.resume_game.w / 2;
        source_y = pointer_layout.resume_game.y +
                   pointer_layout.resume_game.h / 2;
        expect_true(M11_GameView_HandlePointerButton(
                        &view, source_x, source_y,
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW &&
                    view.dm2State.startup_menu_active == 0 &&
                    strstr(view.lastOutcome, "DM2 CONTINUED") != NULL &&
                    view.dm2State.party_x == 23 &&
                    view.dm2State.party_y == 11 &&
                    view.dm2State.party_dir == 2,
                    "M11 DM2 source 0xD9 pointer resumes admitted SKSave.dat");
    }
    M11_GameView_Shutdown(&view);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 savePath resume succeeds");
    expect_true(view.lastOutcome[0] == '\0',
                "M11 DM2 savePath resume keeps host status silent");
    expect_true(view.dm2State.party_x == 23 &&
                view.dm2State.party_y == 11 &&
                view.dm2State.party_dir == 2,
                "M11 DM2 resume mirrors saved party pose");
    expect_true(view.dm2State.tick_count == 42,
                "M11 DM2 resume mirrors saved game tick");
    expect_true(view.dm2State.leader_hand_object ==
                    dm2_db_make_handle(10, 0x0033),
                "M11 DM2 resume mirrors saved leader-hand ObjectID");
    expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                    dm2_db_make_handle(10, 0x0033),
                "M11 DM2 resume exposes saved leader-hand ObjectID through public accessor");
    /* The old DM1-slot interaction sequence is intentionally retired:
     * CHANGE_VIEWPORT_TO_INVENTORY owns a DM2-specific GDAT surface. */
    if (0 && loadable_icon_handle != 0u) {
        int viewport_x = 0, viewport_y = 0;
        int slot_x = 0, slot_y = 0, slot_w = 0, slot_h = 0;
        int status_x = 0, status_y = 0, status_w = 0, status_h = 0;
        DM1_V1_InventorySlotBoxZonePc34 source_slot_zone;
        int source_slot = CHAMPION_SLOT_HEAD;
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 resume mirrors champion inventory ObjectID without THING casting");
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_INVENTORY_TOGGLE) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume inventory toggle opens panel for slot ObjectIDs");
        {
            DM1_V1_LayoutZoneRectPc34 viewport_rect =
                dm1_v1_viewport_rect_pc34();
            viewport_x = viewport_rect.x;
            viewport_y = viewport_rect.y;
        }
        if (dm1_v1_inventory_source_slot_box_zone_pc34(
                source_slot, &source_slot_zone)) {
            slot_x = source_slot_zone.x;
            slot_y = source_slot_zone.y;
            slot_w = source_slot_zone.w;
            slot_h = source_slot_zone.h;
        }
        expect_true(dm1_v1_viewport_zone_id_pc34() != 0 &&
                        slot_w > 0 && slot_h > 0,
                    "M11 DM2 resume inventory source slot zone is available");
        view.dm2State.champion_inventory_objects[0][CHAMPION_SLOT_HEAD] = 0u;
        memset(framebuffer_without_hand, 0, sizeof(framebuffer_without_hand));
        M11_GameView_Draw(&view, framebuffer_without_hand, 320, 200);
        view.dm2State.champion_inventory_objects[0][CHAMPION_SLOT_HEAD] =
            loadable_icon_handle;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&view, framebuffer, 320, 200);
        expect_true(framebuffer_zone_differs(
                        framebuffer_without_hand,
                        framebuffer,
                        320,
                        200,
                        viewport_x + slot_x,
                        viewport_y + slot_y,
                        slot_w,
                        slot_h),
                    "M11 DM2 resume inventory draws GDAT-backed slot ObjectID icon");
        dm2_v1_runtime_set_leader_hand_object(0u);
        view.dm2State.leader_hand_object = 0u;
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        viewport_x + slot_x + (slot_w / 2),
                        viewport_y + slot_y + (slot_h / 2),
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume inventory click picks up a slot ObjectID");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 0, CHAMPION_SLOT_HEAD) == 0u,
                    "M11 DM2 resume slot pickup clears the ObjectID slot");
        expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                        loadable_icon_handle,
                    "M11 DM2 resume slot pickup moves ObjectID to leader hand");
        expect_true(dm2_v1_runtime_get_leader_hand_object() ==
                        loadable_icon_handle,
                    "M11 DM2 resume slot pickup mirrors ObjectID into runtime leader hand");
        expect_true(dm2_v1_runtime_get_champion_inventory_object(
                        0, CHAMPION_SLOT_HEAD) == 0u,
                    "M11 DM2 resume slot pickup writes cleared slot to runtime inventory");
        expect_true(DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat(&view) == -1,
                    "M11 DM2 resume slot pickup does not synthesize a V1 leader-hand icon");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        viewport_x + slot_x + (slot_w / 2),
                        viewport_y + slot_y + (slot_h / 2),
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume inventory click places leader-hand ObjectID back into slot");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 resume slot place restores the ObjectID slot");
        expect_true(M11_GameView_GetDm2LeaderHandObject(&view) == 0u,
                    "M11 DM2 resume slot place clears leader-hand ObjectID");
        expect_true(dm2_v1_runtime_get_leader_hand_object() == 0u,
                    "M11 DM2 resume slot place clears runtime leader hand");
        expect_true(dm2_v1_runtime_get_champion_inventory_object(
                        0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 resume slot place writes ObjectID back to runtime inventory");
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume Back closes champion 0 inventory before champion switch");
        expect_true(!M11_GameView_IsInventoryPanelActive(&view),
                    "M11 DM2 champion 0 inventory is closed before champion switch");
        {
            DM1_V1_ChampionStatusRectPc34 status_rect;
            expect_true(dm1_v1_champion_status_box_rect_pc34(1,
                                                             &status_rect),
                    "M11 DM2 champion 1 status box zone is available");
            status_x = status_rect.x;
            status_y = status_rect.y;
            status_w = status_rect.w;
            status_h = status_rect.h;
        }
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        status_x + (status_w / 2),
                        status_y + (status_h / 2),
                        DM1_V1_MOUSE_MASK_RIGHT_PC34) == M11_GAME_INPUT_REDRAW,
                    "M11 DM2 right-click opens champion 1 inventory");
        expect_true(M11_GameView_IsInventoryPanelActive(&view),
                    "M11 DM2 champion 1 inventory is active");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        viewport_x + slot_x + (slot_w / 2),
                        viewport_y + slot_y + (slot_h / 2),
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW,
                    "M11 DM2 champion 1 inventory click picks up slot ObjectID");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 1, CHAMPION_SLOT_HEAD) == 0u,
                    "M11 DM2 champion 1 pickup clears only champion 1 slot");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 champion 1 pickup leaves champion 0 slot intact");
        expect_true(dm2_v1_runtime_get_champion_inventory_object(
                        1, CHAMPION_SLOT_HEAD) == 0u,
                    "M11 DM2 champion 1 pickup writes cleared slot to runtime");
        expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                        loadable_icon_handle,
                    "M11 DM2 champion 1 pickup moves ObjectID to leader hand");
        expect_true(M11_GameView_HandlePointerButton(
                        &view,
                        viewport_x + slot_x + (slot_w / 2),
                        viewport_y + slot_y + (slot_h / 2),
                        DM1_V1_MOUSE_MASK_LEFT_PC34) == M11_GAME_INPUT_REDRAW,
                    "M11 DM2 champion 1 inventory click places ObjectID back");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 1, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 champion 1 place restores champion 1 slot");
        expect_true(M11_GameView_GetDm2LeaderHandObject(&view) == 0u,
                    "M11 DM2 champion 1 place clears leader hand");
        expect_true(M11_GameView_HandleInput(&view,
                                             M12_MENU_INPUT_SAVE_GAME) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume save command redraws after original-writer refusal");
        snprintf(save_path, sizeof(save_path), "%s%sSKSave.dat",
                 save_root, TEST_PATH_SEP);
        memset(&resume_session, 0, sizeof(resume_session));
        expect_true(dm2_v1_session_load_last_session(save_root,
                                                     &resume_session) == 0,
                    "M11 DM2 keeps the pre-existing resume fixture readable");
        expect_true(resume_session.original_leader_hand_object ==
                        dm2_db_make_handle(10, 0x0033),
                    "M11 DM2 cannot replace a source resume with a fixture session");
        expect_true(((DM2_ChampionRecord *)resume_session.champion_data[0])
                        ->inventory[CHAMPION_SLOT_HEAD] ==
                        loadable_icon_handle,
                    "M11 DM2 saved session preserves runtime inventory slot ObjectID");
        expect_true(((DM2_ChampionRecord *)resume_session.champion_data[1])
                        ->inventory[CHAMPION_SLOT_HEAD] ==
                        loadable_icon_handle,
                    "M11 DM2 saved session preserves champion 1 runtime inventory slot ObjectID");
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 resume Back closes champion 1 inventory slot ObjectID panel");
    }
    if (loadable_icon_handle != 0u) {
        expect_true(M11_GameView_HandleInput(
                        &view, M12_MENU_INPUT_INVENTORY_TOGGLE) ==
                        M11_GAME_INPUT_REDRAW &&
                        !M11_GameView_IsInventoryPanelActive(&view) &&
                        view.lastOutcome[0] == '\0',
                    "M11 DM2 resume inventory stays silently behind its GDAT layout gate");
        expect_true(M11_GameView_HandleInput(
                        &view, M12_MENU_INPUT_CHAMPION_1_INVENTORY) ==
                        M11_GAME_INPUT_REDRAW &&
                        !M11_GameView_IsInventoryPanelActive(&view),
                    "M11 DM2 champion inventory command cannot bypass GDAT");
        expect_true(M11_GameView_GetDm2InventoryObject(
                        &view, 0, CHAMPION_SLOT_HEAD) == loadable_icon_handle &&
                        dm2_v1_runtime_get_champion_inventory_object(
                            0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                    "M11 DM2 blocked inventory preserves source ObjectID state");
    }
    /* The original SKSave record is the resume authority even when this
     * verified edition has no admitted object-icon route. Keep save/load
     * coverage outside the optional visual probe above. */
    dm2_v1_runtime_set_leader_hand_object(0u);
    view.dm2State.leader_hand_object = 0u;
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_SAVE_GAME) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 redraws after blocking the unproven save writer");
    snprintf(save_path, sizeof(save_path), "%s%sSKSave.dat",
             save_root, TEST_PATH_SEP);
    memset(&resume_session, 0, sizeof(resume_session));
    expect_true(dm2_v1_session_load_last_session(save_root,
                                                 &resume_session) == 0 &&
                    resume_session.original_leader_hand_object ==
                        dm2_db_make_handle(10, 0x0033),
                "M11 DM2 leaves the original resume payload unchanged");
    {
        char leader_name[32];

        dm2_v1_runtime_set_leader_hand_object(dm2_db_make_handle(10, 0x0033));
        view.dm2State.leader_hand_object =
            dm2_v1_runtime_get_leader_hand_object();
        expect_true(DM1_V1_M11Runtime_GetLeaderHandObjectIconIndexPc34Compat(&view) == -1,
                    "M11 DM2 leader-hand does not fake a V1 object icon");
        expect_true(!DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(&view,
                                                            leader_name,
                                                            sizeof(leader_name)),
                    "M11 DM2 leader-hand rejects diagnostic DB-handle text");
        view.dm2State.leader_hand_object = dm2_db_make_handle(10, 0x0033);
        expect_true(!DM1_V1_M11Runtime_GetLeaderHandObjectNamePc34Compat(&view,
                                                            leader_name,
                                                            sizeof(leader_name)),
                    "M11 DM2 leader-hand rejects the fixture item-name catalog");
        /* This is a name-formatting probe, not a persisted mutation. */
        dm2_v1_runtime_set_leader_hand_object(0u);
        view.dm2State.leader_hand_object = 0u;
    }
    expect_true(dm2_v1_runtime_get_party_x() == 23 &&
                dm2_v1_runtime_get_party_y() == 11 &&
                dm2_v1_runtime_get_party_dir() == 2,
                "DM2 runtime resume applies saved party pose");
    expect_true(dm2_v1_runtime_get_tick_count() == 42,
                "DM2 runtime resume applies saved tick");
    world = (DM2_V1_GameState*)view.dm2World;
    expect_true(world && world->current_level == 1 && world->outdoor == 1,
                "DM2 world resume applies saved level and outdoor flag");
    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "resumed DM2 idle tick still dispatches through runtime");
    expect_true(view.dm2State.tick_count == 43,
                "resumed DM2 tick advances from saved tick");
    M11_GameView_Shutdown(&view);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 saved SKSave.dat inventory resume succeeds");
    expect_true(M11_GameView_GetDm2InventoryObject(
                    &view, 0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                "M11 DM2 saved SKSave.dat restores slot ObjectID into view state");
    expect_true(M11_GameView_GetDm2InventoryObject(
                    &view, 1, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                "M11 DM2 saved SKSave.dat restores champion 1 slot ObjectID into view state");
    expect_true(M11_GameView_GetDm2LeaderHandObject(&view) ==
                    dm2_db_make_handle(10, 0x0033),
                "M11 DM2 resumes the unchanged source leader-hand ObjectID into view state");
    expect_true(dm2_v1_runtime_get_champion_inventory_object(
                    0, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                "M11 DM2 saved SKSave.dat restores slot ObjectID into runtime");
    expect_true(dm2_v1_runtime_get_champion_inventory_object(
                    1, CHAMPION_SLOT_HEAD) == loadable_icon_handle,
                "M11 DM2 saved SKSave.dat restores champion 1 slot ObjectID into runtime");
    expect_true(dm2_v1_runtime_get_leader_hand_object() ==
                    dm2_db_make_handle(10, 0x0033),
                "M11 DM2 resumes the unchanged source leader-hand ObjectID into runtime");
    M11_GameView_Shutdown(&view);

    resume_session.game_tick = 70;
    resume_session.party_x = 31;
    resume_session.party_y = 9;
    resume_session.party_dir = 3;
    resume_session.party_level = 2;
    resume_session.outdoor_mode = 0;
    resume_session.time_of_day_minutes = 450;
    resume_session.rain_intensity = 0;
    expect_true(write_fixture_d2rs_last_session(save_root, "M11 Last",
                                                 &resume_session),
                "wrote DM2 SKSave.dat last-session fixture");
    snprintf(save_path, sizeof(save_path), "%s%sSKSave.dat",
             save_root, TEST_PATH_SEP);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 SKSave.dat resume succeeds");
    expect_true(view.lastOutcome[0] == '\0',
                "M11 DM2 SKSave.dat resume keeps host status silent");
    expect_true(view.dm2State.party_x == 31 &&
                view.dm2State.party_y == 9 &&
                view.dm2State.party_dir == 3,
                "M11 DM2 SKSave.dat mirrors saved party pose");
    expect_true(view.dm2State.tick_count == 70,
                "M11 DM2 SKSave.dat mirrors saved game tick");
    world = (DM2_V1_GameState*)view.dm2World;
    expect_true(world && world->current_level == 2 && world->outdoor == 0,
                "DM2 world SKSave.dat resume applies saved level");
    M11_GameView_Shutdown(&view);

#endif
    {
        DM2_GameStateBlock original_gs;
        DM2_ChampionRecord original_champ;
        DM2_MinionTable original_minions;

        memset(&original_gs, 0, sizeof(original_gs));
        original_gs.dwGameTick = 84u;
        original_gs.dwRandomSeed = 0x1234u;
        original_gs.wChampionsCount = 1u;
        original_gs.wPlayerPosX = 19u;
        original_gs.wPlayerPosY = 7u;
        original_gs.wPlayerDir = 1u;
        original_gs.wPlayerMap = 3u;
        original_gs.wChampionLeader = 0u;
        original_gs.bRainStrength = 20u;

        memset(&original_champ, 0, sizeof(original_champ));
        memcpy(original_champ.first_name, "TORHAM", 6);
        original_champ.absolute_direction = original_gs.wPlayerDir;
        original_champ.squad_position = 0;
        original_champ.cur_hp = 88;
        original_champ.max_hp = 99;
        original_champ.stamina = 66;
        original_champ.mana = 22;
        original_champ.inventory[0] = dm2_db_make_handle(4, 0x0012);
        original_champ.inventory[1] = dm2_db_make_handle(5, 0x0034);
        memset(&original_minions, 0, sizeof(original_minions));
        original_minions.count = 1u;
        original_minions.entries[0].object_id =
            dm2_db_make_handle(6, 0x0022);
        original_minions.entries[0].owner_champion = 0u;

        expect_true(write_original_resume_slot(save_root,
                                               4,
                                               "M11 Original",
                                               &original_gs,
                                               &original_champ,
                                               &original_minions),
                    "wrote DM2 SKSave04.dat SUPPRESS resume fixture");
        snprintf(save_path, sizeof(save_path), "%s%sSKSave04.dat",
                 save_root, TEST_PATH_SEP);

        fill_dm2_launch_spec(&spec, data_dir);
        spec.savePath = save_path;
        M11_GameView_Init(&view);
        expect_true(M11_GameView_Start(&view, &spec) == 0,
                    "M11 DM2 rejects the D2RS SUPPRESS fixture as a resume");
        expect_true(strstr(view.lastOutcome, "DM2 RESUME FAILED") == NULL,
                    "M11 DM2 keeps the D2RS resume blocker text unbound");
        expect_true(view.active == 0,
                    "D2RS rejection leaves no synthetic DM2 runtime active");
        M11_GameView_Shutdown(&view);
    }

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_root;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "M11 DM2 rejects invalid savePath shape");
    expect_true(strstr(view.lastOutcome, "DM2 RESUME PATH INVALID") == NULL,
                "M11 DM2 keeps invalid-save-path text unbound");
    expect_true(view.active == 0,
                "M11 DM2 invalid savePath leaves view inactive");
    M11_GameView_Shutdown(&view);
    remove_temp_save_root(save_root);

    {
        DM2_V1_SpellCastApplyReceipt spell_failure;
        DM2_V1_BootProfile spell_profile;
        DM2_V1_RuntimeSpellFailureGdatReceipt spell_gdat;
        int spell_profile_ready;

        memset(&spell_failure, 0, sizeof(spell_failure));
        spell_failure.valid = 1;
        spell_failure.failure_feedback = 1;
        spell_failure.failure_class = 0x30;
        dm2_v1_runtime_init(NULL);
        dm2_v1_runtime_note_spell_cast_apply_receipt(&spell_failure);
        expect_true(dm2_v1_runtime_last_spell_failure_class() == 0x30,
                    "DM2 preserves the source spell-failure class");
        expect_true(dm2_v1_runtime_status_scope() == NULL &&
                    dm2_v1_runtime_status_message() == NULL,
                    "DM2 spell failure does not publish synthetic status text");

        dm2_v1_boot_profile_init(&spell_profile);
        spell_profile_ready = dm2_v1_boot_scan_assets(&spell_profile, data_dir) == 0 &&
                              dm2_v1_boot_enter_game(&spell_profile) == 0;
        if (spell_profile_ready) {
            dm2_v1_runtime_init(&spell_profile);
            dm2_v1_runtime_note_spell_cast_apply_receipt(&spell_failure);
            memset(&spell_gdat, 0, sizeof(spell_gdat));
            expect_true(dm2_v1_runtime_last_spell_failure_gdat_receipt(
                            &spell_gdat) == 1 && spell_gdat.valid &&
                        spell_gdat.no_draw && spell_gdat.source_bound &&
                        spell_gdat.category ==
                            DM2_GDAT_CATEGORY_INTERFACE_GENERAL &&
                        spell_gdat.entry_index == 5u &&
                        spell_gdat.image_field == 0x0bu &&
                        spell_gdat.destination_rect == 0x5cu &&
                        spell_gdat.width > 0u && spell_gdat.height > 0u &&
                        spell_gdat.destination_width > 0u &&
                        spell_gdat.destination_height > 0u &&
                        spell_gdat.destination_x >= 0 &&
                        spell_gdat.destination_y >= 0 &&
                        spell_gdat.destination_x == 456 &&
                        spell_gdat.destination_y == 100 &&
                        spell_gdat.destination_width == 92u &&
                        spell_gdat.destination_height == 77u &&
                        spell_gdat.width == 92u && spell_gdat.height == 25u &&
                        spell_gdat.decoded_pixels_hash != 0u &&
                        spell_gdat.palette_hash != 0u &&
                        spell_gdat.destination_table_hash != 0u &&
                        spell_gdat.identity_hash != 0u,
                        "DM2 class-30 failure binds real NEED_FLASK GDAT material and destination");
            dm2_v1_boot_cleanup(&spell_profile);
            dm2_v1_runtime_init(NULL);
        } else {
            dm2_v1_boot_cleanup(&spell_profile);
        }
    }

    if (g_failures) {
        fprintf(stderr, "DM2 V1 M11 startup/profile gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("DM2 V1 M11 startup/profile gate passed");
    return 0;
}
