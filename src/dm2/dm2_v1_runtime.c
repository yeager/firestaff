/*
 * dm2_v1_runtime.c — DM2 V1 Runtime Stub
 *
 * Phase 1: Provides the game tick path for DM2 when launched.
 * The actual game logic (movement, combat, spells) is Phases 2-6.
 * This stub wires the DM2 viewport into the Firestaff game loop
 * so that a DM2 launch can display a viewport frame without crashes.
 *
 * The CSB path in firestaff_game_loop.c provides the reference pattern:
 *   FS_GAME_CSB → csb_v1_viewport_render_frame() → DM1 viewport engine
 *   FS_GAME_DM2 → dm2_v1_runtime_render_frame()   → DM2 viewport engine
 *
 * Source: SKULL.ASM T560  — dungeon tick
 *         SKULL.ASM T600  — outdoor tick
 *         SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T048  — input dispatch
 */

#include "dm2_v1_game.h"
#include "dm2_v1_g1_scene_runtime_bridge.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_projectile_step_pc34_compat.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_timeline.h"
#include "dm2_v1_trigger.h"
#include "dm2_v1_world_model.h"
#include "fs_portable_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── DM2 V1 Runtime State ─────────────────────────────────────────── */

typedef struct {
    DM2_V1_BootProfile *boot;
    int outdoor;              /* 1=outdoor mode, 0=dungeon mode */
    int tick_count;
    int paused;
    /* Movement state */
    int move_cooldown_ticks;
    /* Weather state (outdoor) */
    DM2_V1_WeatherState weather;
    int time_of_day_minutes;  /* 0-1439 */
    /* Dungeon state */
    int dungeon_level;
    int view_dir;
    uint32_t leader_hand_object;
    uint32_t champion_inventory_objects[4][30];
    int session_snapshot_valid;
    DM2_V1_SessionState session_snapshot;
    DM2_MinionTable minions;
    int last_npc_level;
    int last_npc_x;
    int last_npc_y;
    int last_npc_id;
    int last_npc_dialog_line;
    char last_target_message[160];
    int last_spawn_instance_id;
    int last_spawn_ai;
    int last_spawn_x;
    int last_spawn_y;
    int last_spawn_level;
    int spawn_count;
    int last_actuator_type;
    int last_actuator_x;
    int last_actuator_y;
    int last_actuator_level;
    int actuator_count;
    uint32_t last_generated_object;
    int last_projectile_slot;
    int projectile_actuator_count;
    /* Consumed by the next V1 frame only. It represents an accepted party
     * move, never a cooldown, blocked move, or host-supplied animation. */
    int scene_movement_pending;
    /* V2 smooth movement callbacks — registered by dm2_v2_runtime */
    DM2_V2_MoveCallback  move_callback;
    DM2_V2_TurnCallback  turn_callback;
    DM2_V2_StairsCallback stairs_callback;
    /* Startup/render asset boundary owned by the runtime handoff. */
    DM2_V1_ViewportAssetFetch viewport_asset_fetch;
    void *viewport_asset_user;
    DM2_V1_ViewportAssetPaletteFetch viewport_asset_palette_fetch;
    void *viewport_asset_palette_user;
    uint8_t map_wall_gfx_list[16];
    int map_wall_gfx_count;
    uint8_t map_floor_gfx_list[16];
    int map_floor_gfx_count;
    uint8_t map_door_gfx_list[2];
    int map_graphics_style;
    DM2_V1_GdatSceneM11CommandPlan gdat_scene_material_plan;
    DM2_V1_GdatSceneLightM11Receipt gdat_scene_light_receipt;
    DM2_V1_GdatWallM11CommandPlan gdat_wall_material_plan;
    DM2_V1_GdatDoorOverlayM11CommandPlan gdat_door_material_plan;
    int gdat_scene_control_ready;
    uint32_t gdat_scene_control_hash;
    uint32_t gdat_scene_control_present_mask;
    uint32_t gdat_scene_control_query_count;
    uint16_t gdat_scene_colorkey;
    uint16_t gdat_scene_flags;
    uint16_t gdat_scene_ambient_light;
    uint16_t gdat_scene_highest_light_level;
    uint16_t gdat_scene_void_random_fall;
    uint16_t gdat_scene_animated_floor;
    uint16_t gdat_scene_rain;
    uint16_t gdat_misty_map;
    uint16_t gdat_thunder_position;
    uint16_t gdat_ambient_darkness;
    int gdat_weather_receipt_ready;
    uint32_t gdat_weather_receipt_hash;
    uint32_t gdat_weather_material_mask;
    int gdat_weather_destination_ready;
    uint32_t gdat_weather_destination_hash;
    uint32_t gdat_weather_destination_mask;
    int gdat_dialogue_shell_receipt_ready;
    uint32_t gdat_dialogue_shell_receipt_hash;
    int gdat_interface_palette_ready;
    uint32_t gdat_interface_palette_hash;
    uint8_t gdat_interface_palette16[16];
    int gdat_interface_action_palette_ready;
    uint32_t gdat_interface_action_palette_hash;
    uint8_t gdat_interface_action_palette_darkness;
    uint8_t gdat_interface_action_palette16[16];
    DM2_V1_G1FirstMapRuntimeReceipt g1_first_map_runtime;
    DM2_V1_G1TeleporterTransitionReceipt g1_map0_teleporter_transition;
    DM2_V1_G1Map5TextRuntimeReceipt g1_map5_text_runtime;
    DM2_V1_G1TextMessageRuntimeReceipt g1_map5_text_messages_runtime;
    DM2_V1_G1GdatTextMessageRuntimeReceipt g1_map5_gdat_text_messages_runtime;
    DM2_V1_G1TextWallGfxRuntimeReceipt g1_map5_text_wall_gfx_runtime;
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt g1_actuator_wall_gfx_runtime;
    DM2_V1_G1CreatureMapChipRuntimeReceipt g1_creature_map_chip_runtime;
    DM2_V1_G1SceneRuntimeHandoffReceipt g1_scene_runtime_handoff;
    int g1_first_map_viewport_consumed;
    int g1_map0_teleporter_transition_viewport_consumed;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_post_load;
} DM2_V1_RuntimeState;

static DM2_V1_RuntimeState g_dm2_runtime;
static int g_dm2_last_asset_floor_ceiling_count = 0;
static int g_dm2_last_fallback_floor_ceiling_count = 0;
static int g_dm2_last_asset_wall_count = 0;
static int g_dm2_last_fallback_wall_count = 0;
static int g_dm2_last_asset_door_panel_count = 0;
static int g_dm2_last_asset_door_overlay_count = 0;
static int g_dm2_last_asset_door_frame_count = 0;
static int g_dm2_last_asset_door_button_count = 0;
static int g_dm2_last_fallback_door_count = 0;
static DM2_V1_RuntimeDoorRenderReceipt g_dm2_last_door_render;
static int g_dm2_last_asset_creature_count = 0;
static int g_dm2_last_fallback_creature_count = 0;
static DM2_V1_RuntimeCreatureRenderReceipt g_dm2_last_creature_render;
static DM2_V1_RuntimeItemRenderReceipt g_dm2_last_item_render;
static int g_dm2_last_asset_item_count = 0;
static int g_dm2_last_fallback_item_count = 0;
static int g_dm2_last_asset_creature_possession_item_count = 0;
static int g_dm2_last_fallback_creature_possession_item_count = 0;
static int g_dm2_last_asset_carried_item_count = 0;
static int g_dm2_last_fallback_carried_item_count = 0;
static int g_dm2_last_asset_projectile_count = 0;
static int g_dm2_last_fallback_projectile_count = 0;
static DM2_V1_RuntimeProjectileRenderReceipt g_dm2_last_projectile_render;
static int g_dm2_last_asset_hud_portrait_count = 0;
static int g_dm2_last_fallback_hud_portrait_count = 0;
static DM2_V1_RuntimeFrameOwnershipReceipt g_dm2_frame_ownership;
static DM2_V1_ViewportM11FrameReceipt g_dm2_last_m11_frame;
static int g_dm2_runtime_restore_in_progress = 0;

enum {
    DM2_SKPROJECT_TTY_CHAMPION = 0x0c,
    DM2_SKPROJECT_TTY_MISSILE_0 = 0x1d,
    DM2_SKPROJECT_TTY_MISSILE_1 = 0x1e
};

static uint32_t dm2_runtime_timer_tick(const DM2_TimerEntry *timer)
{
    uint8_t raw[DM2_TIMER_ENTRY_SIZE];

    memcpy(raw, timer, sizeof(raw));
    return (uint32_t)raw[0] |
           ((uint32_t)raw[1] << 8) |
           ((uint32_t)raw[2] << 16);
}

/* skproject/SKULLWIN/c_timer.cpp::DM2_cmp_timers compares the saved timer
 * table, not a decoded compatibility view.  The final pointer comparison is
 * the original table index here, because this array preserves that order. */
static int dm2_runtime_timer_precedes(const DM2_V1_SessionState *session,
                                      uint8_t lhs_index,
                                      uint8_t rhs_index)
{
    uint8_t lhs[DM2_TIMER_ENTRY_SIZE];
    uint8_t rhs[DM2_TIMER_ENTRY_SIZE];
    uint32_t lhs_tick;
    uint32_t rhs_tick;

    memcpy(lhs, &session->original_timers[lhs_index], sizeof(lhs));
    memcpy(rhs, &session->original_timers[rhs_index], sizeof(rhs));
    lhs_tick = (uint32_t)lhs[0] | ((uint32_t)lhs[1] << 8) |
               ((uint32_t)lhs[2] << 16);
    rhs_tick = (uint32_t)rhs[0] | ((uint32_t)rhs[1] << 8) |
               ((uint32_t)rhs[2] << 16);
    if (lhs_tick != rhs_tick) return lhs_tick < rhs_tick;
    if (lhs[4] != rhs[4]) return lhs[4] > rhs[4];
    if (lhs[5] != rhs[5]) return lhs[5] > rhs[5];
    return lhs_index <= rhs_index;
}

static uint32_t dm2_runtime_timer_heap_hash(
    const DM2_V1_SessionState *session,
    const DM2_V1_RuntimeTimerPostLoadReceipt *receipt)
{
    uint32_t hash = 2166136261u;

    for (uint8_t i = 0u; i < receipt->timer_heap_count; ++i) {
        const uint8_t timer_index = receipt->timer_heap_index[i];
        const uint8_t *raw = (const uint8_t *)&session->original_timers[timer_index];
        hash ^= timer_index;
        hash *= 16777619u;
        for (uint8_t byte = 0u; byte < DM2_TIMER_ENTRY_SIZE; ++byte) {
            hash ^= raw[byte];
            hash *= 16777619u;
        }
    }
    return hash;
}

static void dm2_runtime_heapify_original_timers(
    const DM2_V1_SessionState *session,
    DM2_V1_RuntimeTimerPostLoadReceipt *out)
{
    uint8_t count = out->timer_count;

    out->timer_heap_count = count;
    out->next_timer_index = 0xffu;
    for (uint8_t i = 0u; i < count; ++i) {
        out->timer_heap_index[i] = i;
    }

    /* This is the bottom-up heap pass from DM2_SORT_TIMERS.  It deliberately
     * leaves the raw table untouched: later DB-backed timer dispatch remains
     * unavailable until its original DB address owner is proven. */
    for (int parent = ((int)count - 2) / 2; parent >= 0; --parent) {
        int current = parent;
        for (;;) {
            int child = current * 2 + 1;
            uint8_t saved_index;
            if (child >= (int)count) break;
            if (child + 1 < (int)count &&
                dm2_runtime_timer_precedes(session,
                    out->timer_heap_index[child + 1],
                    out->timer_heap_index[child])) {
                child++;
            }
            if (dm2_runtime_timer_precedes(session,
                out->timer_heap_index[current], out->timer_heap_index[child])) {
                break;
            }
            saved_index = out->timer_heap_index[current];
            out->timer_heap_index[current] = out->timer_heap_index[child];
            out->timer_heap_index[child] = saved_index;
            current = child;
        }
    }
    if (count != 0u) {
        out->next_timer_index = out->timer_heap_index[0];
        out->next_timer_tick = dm2_runtime_timer_tick(
            &session->original_timers[out->next_timer_index]);
    }
    out->timer_heap_hash = dm2_runtime_timer_heap_hash(session, out);
}

/* skproject/SKWIN/SkWinCore.cpp::_3a15_020f.  DM2_TimerEntry is retained as
 * the original ten-byte wire image by the SUPPRESS decoder, so inspect its
 * bytes rather than its older compatibility field names. */
static int dm2_runtime_rebuild_original_timer_owners(
    const DM2_V1_SessionState *session,
    DM2_V1_RuntimeTimerPostLoadReceipt *out)
{
    uint8_t count;

    if (!session || !out || session->champion_count > 4u ||
        session->original_timer_count > DM2_MAX_TIMERS) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(out->champion_timer_index, 0xff,
           sizeof(out->champion_timer_index));
    count = session->original_timer_count;
    out->timer_count = count;
    memset(out->timer_heap_index, 0xff, sizeof(out->timer_heap_index));

    /* The source returns immediately for an empty queue.  There are no
     * source timer-owner facts to manufacture in that case. */
    if (count == 0u) {
        out->next_timer_index = 0xffu;
        out->timer_heap_hash = dm2_runtime_timer_heap_hash(session, out);
        out->valid = 1;
        return 1;
    }

    dm2_runtime_heapify_original_timers(session, out);

    for (uint8_t i = 0u; i < count; ++i) {
        uint8_t raw[DM2_TIMER_ENTRY_SIZE];
        uint8_t timer_type;
        uint8_t actor;

        memcpy(raw, &session->original_timers[i], sizeof(raw));
        timer_type = raw[4];
        actor = raw[5];
        switch (timer_type) {
            case DM2_SKPROJECT_TTY_CHAMPION:
                /* _3a15_020f writes the current table index.  Reject a bad
                 * actor before publish instead of writing beyond four saved
                 * champion records. */
                if (actor >= session->champion_count || actor >= 4u) {
                    return 0;
                }
                out->champion_timer_index[actor] = i;
                out->champion_timer_bound_mask |= (uint8_t)(1u << actor);
                break;
            case DM2_SKPROJECT_TTY_MISSILE_0:
            case DM2_SKPROJECT_TTY_MISSILE_1:
                /* Source calls GET_ADDRESS_OF_RECORDE(timer->value).  The
                 * exact ObjectID is intentionally not dereferenced until
                 * raw saved DB pools have a verified address owner. */
                out->unresolved_record_timer_count++;
                break;
            default:
                out->other_timer_count++;
                break;
        }
    }
    out->valid = 1;
    return 1;
}

static int dm2_runtime_resolve_g1_scene_gdat(
    void *user,
    DM2_V1_G1SceneTileClass tile_class,
    DM2_V1_G1SceneRootClass root_class,
    int *out_gdat_index)
{
    const DM2_V1_RuntimeState *rt = (const DM2_V1_RuntimeState *)user;

    if (!rt || !out_gdat_index || !rt->gdat_scene_control_ready ||
        rt->map_graphics_style < 0 || rt->map_graphics_style > 0xff ||
        root_class == DM2_V1_G1_SCENE_ROOT_DOOR ||
        root_class == DM2_V1_G1_SCENE_ROOT_CREATURE) {
        return 0;
    }
    if (tile_class == DM2_V1_G1_SCENE_TILE_FLOOR) {
        *out_gdat_index = dm2_v1_viewport_scene_material_graphic_index(
            rt->map_graphics_style,
            DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR);
        return *out_gdat_index != 0;
    }
    if (tile_class == DM2_V1_G1_SCENE_TILE_WALL) {
        *out_gdat_index = dm2_v1_viewport_wall_graphic_index_for_graphicsset(
            rt->map_graphics_style, DM2_SQ_D0L);
        return *out_gdat_index != 0;
    }
    /* A door tile can begin with DB1 and needs the later DB0 payload route.
     * Do not turn its terrain tag into a guessed GDAT panel index. */
    return 0;
}

static void dm2_runtime_refresh_g1_scene_handoff(
    DM2_V1_RuntimeState *rt, int level, int x, int y)
{
    const DM2_V1_DungeonData *dungeon;

    if (!rt) return;
    memset(&rt->g1_scene_runtime_handoff, 0,
           sizeof(rt->g1_scene_runtime_handoff));
    if (!rt->boot || !rt->boot->dungeon_data || rt->outdoor) return;
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    (void)dm2_v1_g1_scene_runtime_handoff(
        dungeon, level, x, y, dm2_runtime_resolve_g1_scene_gdat, rt,
        rt->viewport_asset_fetch, rt->viewport_asset_user,
        rt->viewport_asset_palette_fetch, rt->viewport_asset_palette_user,
        &rt->g1_scene_runtime_handoff);
}

#define DM2_RUNTIME_SAVE_MAGIC "FS2RT01"
#define DM2_RUNTIME_SAVE_VERSION 4u

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t session_size;
    uint32_t creature_size;
    uint32_t dungeon_size;
    uint32_t graphics_size;
    char graphics_md5[33];
    uint8_t map_wall_gfx_list[16];
    uint8_t map_floor_gfx_list[16];
    uint8_t map_door_gfx_list[2];
    int32_t map_wall_gfx_count;
    int32_t map_floor_gfx_count;
    int32_t move_cooldown_ticks;
    int32_t paused;
    int32_t map_graphics_style;
    int32_t gdat_scene_control_ready;
    uint32_t gdat_scene_control_hash;
    uint32_t gdat_scene_control_present_mask;
    uint32_t gdat_scene_control_query_count;
    uint16_t gdat_scene_colorkey;
    uint16_t gdat_scene_flags;
    uint16_t gdat_scene_ambient_light;
    uint16_t gdat_scene_highest_light_level;
    uint16_t gdat_scene_void_random_fall;
    uint16_t gdat_scene_animated_floor;
    uint16_t gdat_scene_rain;
    uint16_t gdat_misty_map;
    uint16_t gdat_thunder_position;
    uint16_t gdat_ambient_darkness;
} DM2_V1_RuntimeSaveHeader;

static int dm2_runtime_live_header_valid(const DM2_V1_RuntimeSaveHeader *header,
                                         const DM2_V1_DungeonData *dungeon)
{
    if (!header || !dungeon ||
        memcmp(header->magic, DM2_RUNTIME_SAVE_MAGIC, 8) != 0 ||
        header->version != DM2_RUNTIME_SAVE_VERSION ||
        header->session_size == 0 ||
        header->creature_size != sizeof(DM2_V1_CreatureLiveState) ||
        header->dungeon_size != (uint32_t)dungeon->raw_size ||
        header->graphics_size != (uint32_t)g_dm2_runtime.boot->graphics_size ||
        header->map_wall_gfx_count < 0 || header->map_wall_gfx_count > 16 ||
        header->map_floor_gfx_count < 0 || header->map_floor_gfx_count > 16 ||
        header->move_cooldown_ticks < 0) {
        return 0;
    }
    return strncmp(header->graphics_md5,
                   g_dm2_runtime.boot->graphics_md5,
                   sizeof(header->graphics_md5)) == 0;
}

static int dm2_runtime_write_live_sidecar(const char *save_root);
static void dm2_runtime_restore_live_sidecar(const char *save_root,
                                             const DM2_V1_SessionState *session);

static void dm2_runtime_add_viewport_asset_evidence(
    DM2_V1_RuntimeFrameOwnershipReceipt *receipt, int gdat_index)
{
    DM2_V1_BootViewportAssetEvidence evidence;
    if (!receipt || g_dm2_runtime.viewport_asset_fetch !=
                         dm2_v1_boot_viewport_asset_fetch ||
        !g_dm2_runtime.viewport_asset_user ||
        !dm2_v1_boot_viewport_asset_evidence(
            (DM2_V1_BootProfile *)g_dm2_runtime.viewport_asset_user,
            gdat_index, &evidence)) {
        return;
    }
    receipt->viewport_raw_gdat_hash =
        receipt->viewport_raw_gdat_hash * 33u + evidence.raw_hash;
    receipt->viewport_raw_gdat_byte_count += evidence.raw_byte_count;
    receipt->viewport_decoded_gdat_hash =
        receipt->viewport_decoded_gdat_hash * 33u + evidence.decoded_hash;
    receipt->viewport_decoded_gdat_pixel_count += evidence.decoded_pixel_count;
    ++receipt->viewport_raw_gdat_asset_count;
    ++receipt->viewport_decoded_gdat_asset_count;
}

static int dm2_runtime_door_state(uint16_t square_raw) {
    return (int)(square_raw & 0x0007u);
}

static int dm2_runtime_raw_is_door_square(uint16_t square_raw) {
    int square_type = (int)(square_raw & DM2_SQUARE_TYPE_MASK);
    enum { DM2_RUNTIME_DOOR_RAW_CLOSED_SENTINEL = 4 };
    /* Current startup/render tests exercise DM2 door cells in two bounded
     * forms: DM2_SQUARE_DOOR for viewport asset binding and C4 closed-door
     * low bits for the action/state path.  Keep the bridge narrow until the
     * square-first door record chain is decoded into runtime state. */
    return square_type == DM2_SQUARE_DOOR ||
           square_type == DM2_RUNTIME_DOOR_RAW_CLOSED_SENTINEL;
}

static int dm2_runtime_square_type_at(const DM2_V1_DungeonData *dd,
                                      int level,
                                      int x,
                                      int y,
                                      int raw) {
    int square_type;

    if (!dd) return raw & DM2_SQUARE_TYPE_MASK;
    square_type = dm2_v1_dungeon_get_square_type(dd, level, x, y);
    if (square_type >= 0) return square_type;
    return raw & DM2_SQUARE_TYPE_MASK;
}

static int dm2_runtime_has_door_record_at(const DM2_V1_DungeonData *dd,
                                          int level,
                                          int x,
                                          int y) {
    int thing;
    if (!dd) return 0;
    thing = dm2_v1_dungeon_get_first_thing(dd, level, x, y);
    if (thing < 0) return 0;
    return dm2_v1_dungeon_find_thing_of_type(dd, (uint16_t)thing, 0, 8) >= 0;
}

/* skproject SKWIN/SkWinCore.cpp lines 10351-10358 and 16927-16931
 * route DB0 door records through tile state; state 0/open and
 * state 5/destroyed remain passable/render-open even when the raw low
 * bits no longer match the closed-door sentinel. */
static int dm2_runtime_is_door_at(const DM2_V1_DungeonData *dd,
                                  int level,
                                  int x,
                                  int y,
                                  int raw) {
    int square_type = dm2_runtime_square_type_at(dd, level, x, y, raw);
    return square_type == DM2_SQUARE_DOOR ||
           dm2_runtime_has_door_record_at(dd, level, x, y) ||
           dm2_runtime_raw_is_door_square((uint16_t)raw);
}

static uint16_t dm2_runtime_door_attributes_at(DM2_V1_DungeonData *dd,
                                               int level,
                                               int x,
                                               int y) {
    int thing;
    int door_thing;
    int type = -1;
    int index = -1;
    int size = 0;
    const uint8_t *record;
    uint16_t w2;
    int door_type;

    if (!dd) return 0;
    thing = dm2_v1_dungeon_get_first_thing(dd, level, x, y);
    if (thing < 0) return 0;
    door_thing = dm2_v1_dungeon_find_thing_of_type(dd, (uint16_t)thing, 0, 8);
    if (door_thing < 0) return 0;
    record = dm2_v1_dungeon_get_thing_record(
        dd, (uint16_t)door_thing, &type, &index, &size);
    (void)index;
    if (!record || type != 0 || size < 4) return 0;

    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    door_type = (int)(w2 & 3u);
    return dm2_door_get_attributes(door_type);
}

static int dm2_runtime_creature_read_door(void *user,
                                          int level,
                                          int x,
                                          int y,
                                          int *out_state,
                                          uint16_t *out_attributes) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dd;
    int raw;

    if (!rt || !rt->boot || !rt->boot->dungeon_data) return 0;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0 || !dm2_runtime_is_door_at(dd, level, x, y, raw)) return 0;
    if (out_state) *out_state = dm2_runtime_door_state((uint16_t)raw);
    if (out_attributes) {
        *out_attributes = dm2_runtime_door_attributes_at(dd, level, x, y);
    }
    return 1;
}

static void dm2_runtime_apply_door_record_metadata(
    DM2_V1_DungeonData *dd,
    int level,
    int x,
    int y,
    int view_dir,
    const uint8_t *wall_gfx_list,
    int wall_gfx_count,
    const uint8_t *door_gfx_list,
    DM2_ViewSquare *door) {
    int thing;
    int door_thing;
    int type = -1;
    int index = -1;
    int size = 0;
    const uint8_t *record;
    uint16_t w2;
    uint16_t wall_button_object_id = 0xffffu;
    int wall_gfx_index = -1;
    int wall_gfx_field = -1;

    if (!dd || !door) return;
    thing = dm2_v1_dungeon_get_first_thing(dd, level, x, y);
    if (thing < 0) return;
    door_thing = dm2_v1_dungeon_find_thing_of_type(dd, (uint16_t)thing, 0, 8);
    if (door_thing < 0) return;
    record = dm2_v1_dungeon_get_thing_record(
        dd, (uint16_t)door_thing, &type, &index, &size);
    (void)index;
    if (!record || type != 0 || size < 4) return;
    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    /* skproject SKWIN/DME.h Door::Button/ButtonState/DoorType/
     * OpeningDir expose these fields from the DB0 door record's w2 word;
     * SkWinCore.cpp DRAW_DOOR_FRAMES lines ~46340-46349 uses Button()
     * and ButtonState() to choose the default button image. */
    door->door_button = (uint8_t)((w2 >> 6) & 1u);
    door->door_button_state = (uint8_t)((w2 >> 11) & 1u);
    door->door_record_type = (uint8_t)(w2 & 1u);
    door->door_opening_dir = (uint8_t)((w2 >> 5) & 1u);
    door->ornament_index = (uint8_t)((w2 >> 1) & 0x0fu);
    if (door_gfx_list) {
        door->door_gfx_index = door_gfx_list[door->door_record_type & 1u];
    }
    if (!door->door_button &&
        dm2_v1_dungeon_find_text_wall_gfx_owner(
            dd, (uint16_t)thing, view_dir, 2, 8,
            &wall_gfx_index, &wall_gfx_field,
            &wall_button_object_id) == 0) {
        door->door_wall_button = 1;
        door->door_wall_button_index = (uint8_t)wall_gfx_index;
        door->door_wall_button_field = (uint8_t)wall_gfx_field;
        door->door_wall_button_x = (int16_t)x;
        door->door_wall_button_y = (int16_t)y;
        door->door_wall_button_object_id = wall_button_object_id;
    } else if (!door->door_button &&
               dm2_v1_dungeon_resolve_actuator_wall_gfx_owner(
                   dd, (uint16_t)thing, view_dir, 2, 8,
                   wall_gfx_list, wall_gfx_count,
                   &wall_gfx_index, &wall_gfx_field,
                   &wall_button_object_id) == 0) {
        door->door_wall_button = 1;
        door->door_wall_button_index = (uint8_t)wall_gfx_index;
        door->door_wall_button_field = (uint8_t)wall_gfx_field;
        door->door_wall_button_x = (int16_t)x;
        door->door_wall_button_y = (int16_t)y;
        door->door_wall_button_object_id = wall_button_object_id;
    }
}

static uint16_t dm2_runtime_door_set_state(uint16_t square_raw, int state) {
    return (uint16_t)((square_raw & ~0x0007u) | (uint16_t)(state & 0x0007));
}

static int dm2_runtime_door_step(int current_state, int action) {
    if (current_state == 5) return 5;
    if (action == 1) {
        if (current_state >= 4) return 4;
        return current_state + 1;
    }
    if (current_state <= 0) return 0;
    return current_state - 1;
}

static void dm2_runtime_refresh_map_wall_gfx_list(DM2_V1_RuntimeState *rt) {
    DM2_V1_DungeonData *dd;
    int count;

    if (!rt) return;
    memset(rt->map_wall_gfx_list, 0, sizeof(rt->map_wall_gfx_list));
    rt->map_wall_gfx_count = 0;
    memset(rt->map_floor_gfx_list, 0, sizeof(rt->map_floor_gfx_list));
    rt->map_floor_gfx_count = 0;
    memset(rt->map_door_gfx_list, 0, sizeof(rt->map_door_gfx_list));
    if (!rt->boot || !rt->boot->dungeon_data) return;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    if (rt->dungeon_level >= 0 && rt->dungeon_level < dd->level_count) {
        rt->map_door_gfx_list[0] =
            (uint8_t)(dd->map_door_set0[rt->dungeon_level] & 0xff);
        rt->map_door_gfx_list[1] =
            (uint8_t)(dd->map_door_set1[rt->dungeon_level] & 0xff);
    }
    count = dm2_v1_dungeon_get_map_wall_gfx_list(
        dd,
        rt->dungeon_level,
        rt->map_wall_gfx_list,
        (int)sizeof(rt->map_wall_gfx_list));
    if (count > 0) {
        rt->map_wall_gfx_count = count;
    }
    count = dm2_v1_dungeon_get_map_floor_gfx_list(
        dd, rt->dungeon_level, rt->map_floor_gfx_list,
        (int)sizeof(rt->map_floor_gfx_list));
    if (count > 0) {
        rt->map_floor_gfx_count = count;
    }
}

static void dm2_runtime_refresh_gdat_scene_control(DM2_V1_RuntimeState *rt)
{
    DM2_V1_DungeonData *dd;
    DM2_V1_InterfacePalette palette;
    DM2_V1_WeatherGdatReceipt weather_receipt;
    DM2_V1_BootWeatherDestinationReceipt weather_destination;
    DM2_V1_DialogueGdatReceipt dialogue_shell;

    if (!rt) return;
    dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
    memset(&rt->gdat_scene_light_receipt, 0,
           sizeof(rt->gdat_scene_light_receipt));
    dm2_v1_gdat_wall_m11_command_plan_free(&rt->gdat_wall_material_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&rt->gdat_door_material_plan);
    rt->map_graphics_style = -1;
    rt->gdat_scene_control_ready = 0;
    rt->gdat_scene_control_hash = 0u;
    rt->gdat_scene_control_present_mask = 0u;
    rt->gdat_scene_control_query_count = 0u;
    rt->gdat_scene_colorkey = 0u;
    rt->gdat_scene_flags = 0u;
    rt->gdat_scene_ambient_light = 0u;
    rt->gdat_scene_highest_light_level = 0u;
    rt->gdat_scene_void_random_fall = 0u;
    rt->gdat_scene_animated_floor = 0u;
    rt->gdat_scene_rain = 0u;
    rt->gdat_misty_map = 0u;
    rt->gdat_thunder_position = 0u;
    rt->gdat_ambient_darkness = 0u;
    rt->gdat_weather_receipt_ready = 0;
    rt->gdat_weather_receipt_hash = 0u;
    rt->gdat_weather_material_mask = 0u;
    rt->gdat_weather_destination_ready = 0;
    rt->gdat_weather_destination_hash = 0u;
    rt->gdat_weather_destination_mask = 0u;
    rt->gdat_dialogue_shell_receipt_ready = 0;
    rt->gdat_dialogue_shell_receipt_hash = 0u;
    rt->gdat_interface_palette_ready = 0;
    rt->gdat_interface_palette_hash = 0u;
    memset(rt->gdat_interface_palette16, 0,
           sizeof(rt->gdat_interface_palette16));
    rt->gdat_interface_action_palette_ready = 0;
    rt->gdat_interface_action_palette_hash = 0u;
    rt->gdat_interface_action_palette_darkness = 0u;
    memset(rt->gdat_interface_action_palette16, 0,
           sizeof(rt->gdat_interface_action_palette16));
    if (!rt->boot || !rt->boot->dungeon_data) return;

    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    rt->map_graphics_style = dm2_v1_dungeon_get_map_graphics_style(
        dd, rt->dungeon_level);
    if (rt->map_graphics_style < 0) return;

    /* skproject UPDATE_GFXSET loads the selected GRAPHICSSET image pair and
     * CHECK_RECOMPUTE_LIGHT consumes its darkness word before dungeon draw.
     * Admit only this exact map's complete source family, never a nearby set. */
    if (!dm2_v1_boot_gdat_scene_m11_command_plan(
            rt->boot, rt->map_graphics_style, &rt->gdat_scene_material_plan) ||
        !rt->gdat_scene_material_plan.valid ||
        rt->gdat_scene_material_plan.graphicsset !=
            (uint8_t)rt->map_graphics_style) {
        return;
    }
    if (!dm2_v1_gdat_scene_light_m11_receipt(
            &rt->gdat_scene_material_plan, &rt->gdat_scene_light_receipt)) {
        dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
        return;
    }
    if (!dm2_v1_boot_gdat_wall_m11_command_plan(
            rt->boot, rt->map_graphics_style, &rt->gdat_wall_material_plan) ||
        !rt->gdat_wall_material_plan.valid ||
        rt->gdat_wall_material_plan.graphicsset != (uint8_t)rt->map_graphics_style) {
        dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
        return;
    }

    /* The old control receipt could fall through to another graphics set.
     * These four source fields are the complete G1 family and must remain
     * paired with the decoded floor/ceiling pixels from the selected map. */
    rt->gdat_scene_control_ready = 1;
    rt->gdat_scene_control_hash = rt->gdat_scene_material_plan.command_hash;
    rt->gdat_scene_control_present_mask = 0x0fu;
    rt->gdat_scene_control_query_count = 4u;
    rt->gdat_scene_flags = rt->gdat_scene_material_plan.scene_flags;
    rt->gdat_scene_colorkey = rt->gdat_scene_material_plan.scene_colorkey;
    rt->gdat_scene_highest_light_level =
        rt->gdat_scene_material_plan.highest_light_level;
    rt->gdat_ambient_darkness = rt->gdat_scene_material_plan.ambient_darkness;
    /* c_weather.cpp consumes the active MapGraphicsStyle after GRAPHICSSET
     * control resolution. Preserve the verified environment image/palette
     * receipt in the live frame boundary, but do not turn it into pixels: the
     * source destination clip is still intentionally unproven. */
    memset(&weather_receipt, 0, sizeof(weather_receipt));
    if (dm2_v1_boot_weather_gdat_receipt(rt->boot,
                                         rt->map_graphics_style,
                                         &weather_receipt) &&
        weather_receipt.valid && weather_receipt.receipt_hash != 0u) {
        rt->gdat_weather_receipt_ready = 1;
        rt->gdat_weather_receipt_hash = weather_receipt.receipt_hash;
        rt->gdat_weather_material_mask = weather_receipt.material_mask;
    }
    /* QUERY_TEMP_PICST resolves CD through the source dt04 rect table. Keep
     * that live destination proof separate from the material receipt; the
     * renderer remains no-draw until the complete weather execution route is
     * available. */
    memset(&weather_destination, 0, sizeof(weather_destination));
    if (dm2_v1_boot_weather_gdat_destination_receipt(
            rt->boot, rt->map_graphics_style, &weather_destination) &&
        weather_destination.valid && weather_destination.receipt_hash != 0u) {
        rt->gdat_weather_destination_ready = 1;
        rt->gdat_weather_destination_hash = weather_destination.receipt_hash;
        rt->gdat_weather_destination_mask = weather_destination.destination_mask;
    }
    memset(&dialogue_shell, 0, sizeof(dialogue_shell));
    if (dm2_v1_boot_dialogue_gdat_receipt(rt->boot, rt->map_graphics_style,
                                          0xfdu, &dialogue_shell) &&
        dialogue_shell.valid && dialogue_shell.receipt_hash != 0u) {
        rt->gdat_dialogue_shell_receipt_ready = 1;
        rt->gdat_dialogue_shell_receipt_hash = dialogue_shell.receipt_hash;
    }
    if (dm2_v1_boot_interface_palette(rt->boot, &palette)) {
        DM2_V1_InterfaceActionTable action_table;

        rt->gdat_interface_palette_ready = 1;
        rt->gdat_interface_palette_hash = palette.hash;
        memcpy(rt->gdat_interface_palette16, palette.palette16,
               sizeof(rt->gdat_interface_palette16));
        memcpy(rt->gdat_interface_action_palette16, palette.palette16,
               sizeof(rt->gdat_interface_action_palette16));
        memset(&action_table, 0, sizeof(action_table));
        /* skproject DISPLAY_VIEWPORT (32CB:5D13) derives the palette
         * darkness from glbLightLevel * 10.  Until dynamic light sources
         * are source-owned, the active GRAPHICSSET lower bound is the only
         * verified light level available to this renderer. */
        rt->gdat_interface_action_palette_darkness =
            (uint8_t)((rt->gdat_scene_highest_light_level > 5u ? 5u :
                       rt->gdat_scene_highest_light_level) * 10u);
        if (dm2_v1_boot_interface_action_table(rt->boot, &action_table) &&
            dm2_v1_interface_action_table_remap_palette(
                &action_table, rt->gdat_interface_action_palette16, 16u,
                rt->gdat_interface_action_palette_darkness, -1, -1)) {
            rt->gdat_interface_action_palette_ready = 1;
            rt->gdat_interface_action_palette_hash = action_table.hash;
        }
    }
}

static void dm2_runtime_populate_visible_terrain(DM2_V1_RuntimeState *rt,
                                                 DM2_V1_ViewportState *viewport,
                                                 int party_dir,
                                                 int party_x,
                                                 int party_y) {
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct {
        int square;
        int forward;
        int lateral;
    } visible_cells[] = {
        /* SKProject c_gui_vp.cpp consumes these D0..D3 center/side cells
         * through the existing wall panel plan. D0 sides are adjacent to the
         * party. D3L/D3R are the deep projections: SKProject's view-cell
         * table reaches five cells ahead and two cells out from the center
         * ray. D3C has no source GRAPHICSSET wall field, so it is deliberately
         * not promoted to a drawable terrain surface here. */
        { DM2_SQ_D0C, 1,  0 }, { DM2_SQ_D1C, 2,  0 },
        { DM2_SQ_D2C, 3,  0 },
        { DM2_SQ_D0L, 0, -1 }, { DM2_SQ_D0R, 0,  1 },
        { DM2_SQ_D1L, 1, -1 }, { DM2_SQ_D1R, 1,  1 },
        { DM2_SQ_D2L, 2, -1 }, { DM2_SQ_D2R, 2,  1 },
        { DM2_SQ_D3L, 5, -2 }, { DM2_SQ_D3R, 5,  2 },
    };
    DM2_V1_DungeonData *dd;
    int dir;

    if (!rt || !viewport || rt->outdoor || !rt->boot ||
        !rt->boot->dungeon_data) {
        return;
    }
    dir = party_dir & 3;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    for (size_t i = 0; i < sizeof(visible_cells) / sizeof(visible_cells[0]); ++i) {
        int map_x = party_x + dx[dir] * visible_cells[i].forward -
            dy[dir] * visible_cells[i].lateral;
        int map_y = party_y + dy[dir] * visible_cells[i].forward +
            dx[dir] * visible_cells[i].lateral;
        int raw = dm2_v1_dungeon_get_tile_raw(
            dd,
            rt->dungeon_level,
            map_x, map_y);
        int square_type = dm2_v1_dungeon_get_square_type(
            dd, rt->dungeon_level, map_x, map_y);
        int type;
        if (raw < 0) continue;
        type = raw & DM2_SQUARE_TYPE_MASK;
        if (dd->square_bytes == 1) {
            /* G1's tileTypeIndex values are not the renderer enum.  Convert
             * only the three source-defined classes; every other byte class
             * remains unavailable instead of becoming a fallback surface. */
            square_type = dm2_v1_viewport_g1_tile_class_to_square_type(
                (uint8_t)((unsigned int)raw >> 5));
            if (square_type < 0) continue;
        }
        {
            DM2_ViewSquare *surface = &viewport->squares[visible_cells[i].square];
            if (square_type == DM2_SQUARE_WALL) {
                surface->square_type = DM2_SQUARE_WALL;
                surface->flags |= DM2_SQF_HAS_WALL;
            } else if (square_type == DM2_SQUARE_FLOOR) {
                surface->square_type = DM2_SQUARE_FLOOR;
                surface->flags &= (uint8_t)~(DM2_SQF_HAS_WALL | DM2_SQF_HAS_DOOR);
            } else if (square_type == DM2_SQUARE_TELEPORTER) {
                surface->square_type = DM2_SQUARE_TELEPORTER;
                surface->flags &= (uint8_t)~(DM2_SQF_HAS_WALL | DM2_SQF_HAS_DOOR);
            }
        }
        if (dm2_runtime_is_door_at(dd, rt->dungeon_level, map_x, map_y, raw)) {
            DM2_ViewSquare *door = &viewport->squares[visible_cells[i].square];
            door->square_type =
                (uint8_t)(square_type >= 0 ? square_type : type);
            door->flags |= DM2_SQF_HAS_DOOR | DM2_SQF_HAS_WALL;
            {
                int door_state = dm2_runtime_door_state((uint16_t)raw);
                if (door_state < 0) {
                    door_state = 0;
                } else if (door_state > 5) {
                    door_state = 4;
                }
                door->door_open_pct =
                    (uint8_t)dm2_v1_creature_door_open_pct_from_state(
                        door_state);
                door->door_state = (uint8_t)door_state;
            }
            dm2_runtime_apply_door_record_metadata(
                dd, rt->dungeon_level, map_x, map_y, dir,
                rt->map_wall_gfx_list, rt->map_wall_gfx_count,
                rt->map_door_gfx_list, door);
        }
    }
}

static void dm2_runtime_capture_door_render_receipt(
    const DM2_V1_ViewportState *viewport)
{
    DM2_V1_DoorRenderPlan plan;
    const DM2_V1_DoorRender *door;

    memset(&g_dm2_last_door_render, 0, sizeof(g_dm2_last_door_render));
    if (!viewport ||
        !dm2_v1_viewport_build_door_render_plan(viewport, &plan) ||
        plan.door_count <= 0) {
        return;
    }
    door = &plan.doors[0];

    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_TILE routes the populated
     * center-cell DB0 door facts into panel, frame, button, ornate, and
     * destroyed-mask GDAT lookups; DRAW_DOOR_FRAMES consumes the same cell
     * geometry. This receipt captures the first bounded render-row chosen by
     * Firestaff immediately before the draw pass. */
    g_dm2_last_door_render.valid = 1;
    g_dm2_last_door_render.view_square = door->view_square;
    g_dm2_last_door_render.skproject_cell = door->skproject_cell;
    g_dm2_last_door_render.door_record_type = door->door_record_type;
    g_dm2_last_door_render.door_gfx_index = door->door_gfx_index;
    g_dm2_last_door_render.door_opening_dir = door->door_opening_dir;
    g_dm2_last_door_render.ornament_index = door->ornament_index;
    g_dm2_last_door_render.door_button = door->door_button;
    g_dm2_last_door_render.door_button_state = door->door_button_state;
    g_dm2_last_door_render.door_state = door->door_state;
    g_dm2_last_door_render.door_open_pct = door->door_open_pct;
    g_dm2_last_door_render.panel_gdat_index =
        door->panel_gdat_index;
    g_dm2_last_door_render.ornate_gdat_index =
        door->ornate_gdat_index;
    g_dm2_last_door_render.destroyed_mask_gdat_index =
        door->destroyed_mask_gdat_index;
    g_dm2_last_door_render.frame_gdat_index =
        door->frame_gdat_index;
    g_dm2_last_door_render.button_gdat_index =
        door->button_gdat_index;
    g_dm2_last_door_render.button_source_kind =
        door->button_source_kind;
    /* skproject MAKE_BUTTON_CLICKABLE is called only for rectnos 3/4 in
     * DRAW_DEFAULT_DOOR_BUTTON; retain that source gate in the runtime
     * receipt instead of treating every drawn button as interactive. */
    g_dm2_last_door_render.button_rectno =
        dm2_v1_viewport_door_button_rectno_for_square(door->view_square);
    g_dm2_last_door_render.button_clickable =
        door->button_source_kind == 1 &&
        dm2_v1_viewport_door_button_clickable_for_square(door->view_square);
    g_dm2_last_door_render.wall_button_index =
        door->wall_button_index;
    g_dm2_last_door_render.wall_button_field =
        door->wall_button_field;
    g_dm2_last_door_render.panel_blit_ready =
        door->panel_gdat_index != 0 &&
        door->panel_visible_rect.w > 0 &&
        door->panel_visible_rect.h > 0;
    g_dm2_last_door_render.ornate_blit_ready =
        door->ornate_gdat_index != 0 &&
        door->panel_rect.w > 0 &&
        door->panel_rect.h > 0;
    g_dm2_last_door_render.destroyed_mask_blit_ready =
        door->destroyed_mask_gdat_index != 0 &&
        door->panel_rect.w > 0 &&
        door->panel_rect.h > 0;
    g_dm2_last_door_render.frame_blit_ready =
        door->frame_gdat_index != 0 &&
        door->frame_rect.w > 0 &&
        door->frame_rect.h > 0;
    g_dm2_last_door_render.button_blit_ready =
        door->button_gdat_index != 0 &&
        door->button_rect.w > 0 &&
        door->button_rect.h > 0;
    /* skproject SKWINSPX/src/v0/dme.h Door::DoorType, Button,
     * ButtonState, OpeningDir and OrnateIndex feed
     * SKWINSPX/src/v4/skguidrw.cpp DRAW_DOOR/DRAW_DOOR_FRAMES as one door
     * material chain. Count the same required GDAT slots here before draw. */
    g_dm2_last_door_render.skproject_material_expected_count = 1;
    g_dm2_last_door_render.skproject_material_ready_count =
        g_dm2_last_door_render.panel_blit_ready ? 1 : 0;
    if (door->ornament_index > 0) {
        ++g_dm2_last_door_render.skproject_material_expected_count;
        if (g_dm2_last_door_render.ornate_blit_ready) {
            ++g_dm2_last_door_render.skproject_material_ready_count;
        }
    }
    if (door->door_state == 5) {
        ++g_dm2_last_door_render.skproject_material_expected_count;
        if (g_dm2_last_door_render.destroyed_mask_blit_ready) {
            ++g_dm2_last_door_render.skproject_material_ready_count;
        }
    }
    ++g_dm2_last_door_render.skproject_material_expected_count;
    if (g_dm2_last_door_render.frame_blit_ready) {
        ++g_dm2_last_door_render.skproject_material_ready_count;
    }
    if (door->button_gdat_index != 0) {
        ++g_dm2_last_door_render.skproject_material_expected_count;
        if (g_dm2_last_door_render.button_blit_ready) {
            ++g_dm2_last_door_render.skproject_material_ready_count;
        }
    }
    g_dm2_last_door_render.skproject_material_chain_ready =
        g_dm2_last_door_render.skproject_material_expected_count ==
        g_dm2_last_door_render.skproject_material_ready_count;
    g_dm2_last_door_render.skproject_material_chain_hash =
        2166136261u;
#define DM2_MIX_DOOR_RECEIPT(v) \
    do { \
        g_dm2_last_door_render.skproject_material_chain_hash ^= \
            (uint32_t)(v); \
        g_dm2_last_door_render.skproject_material_chain_hash *= 16777619u; \
    } while (0)
    DM2_MIX_DOOR_RECEIPT(door->view_square);
    DM2_MIX_DOOR_RECEIPT(door->skproject_cell);
    DM2_MIX_DOOR_RECEIPT(door->door_record_type);
    DM2_MIX_DOOR_RECEIPT(door->door_gfx_index);
    DM2_MIX_DOOR_RECEIPT(door->door_opening_dir);
    DM2_MIX_DOOR_RECEIPT(door->ornament_index);
    DM2_MIX_DOOR_RECEIPT(door->door_button);
    DM2_MIX_DOOR_RECEIPT(door->door_button_state);
    DM2_MIX_DOOR_RECEIPT(door->door_state);
    DM2_MIX_DOOR_RECEIPT(door->panel_gdat_index);
    DM2_MIX_DOOR_RECEIPT(door->ornate_gdat_index);
    DM2_MIX_DOOR_RECEIPT(door->destroyed_mask_gdat_index);
    DM2_MIX_DOOR_RECEIPT(door->frame_gdat_index);
    DM2_MIX_DOOR_RECEIPT(door->button_gdat_index);
#undef DM2_MIX_DOOR_RECEIPT
    g_dm2_last_door_render.panel_rect = door->panel_rect;
    g_dm2_last_door_render.panel_visible_rect = door->panel_visible_rect;
    g_dm2_last_door_render.overlay_rect = door->panel_rect;
    g_dm2_last_door_render.frame_rect = door->frame_rect;
    g_dm2_last_door_render.button_rect = door->button_rect;
}

static void dm2_runtime_finish_door_render_receipt(
    const DM2_V1_ViewportState *viewport)
{
    if (!viewport || !g_dm2_last_door_render.valid) {
        return;
    }

    /* skproject SKWIN/SkWinCore.cpp DRAW_DOOR_TILE and DRAW_DOOR_FRAMES
     * consume the selected DB0 row by fetching GDAT images before blitting.
     * Keep that actual asset-consumption receipt in DM2 runtime so callers do
     * not infer success from aggregate draw counters. */
    g_dm2_last_door_render.panel_asset_drawn =
        viewport->last_door_panel_asset_blit_valid;
    g_dm2_last_door_render.ornate_asset_drawn =
        viewport->last_door_ornate_asset_blit_valid;
    g_dm2_last_door_render.destroyed_mask_asset_drawn =
        viewport->last_door_destroyed_mask_asset_blit_valid;
    g_dm2_last_door_render.frame_asset_drawn =
        viewport->last_door_frame_asset_blit_valid;
    g_dm2_last_door_render.button_asset_drawn =
        viewport->last_door_button_asset_blit_valid;
    g_dm2_last_door_render.skproject_material_drawn_count = 0;
    if (g_dm2_last_door_render.panel_asset_drawn) {
        ++g_dm2_last_door_render.skproject_material_drawn_count;
    }
    if (g_dm2_last_door_render.ornament_index > 0 &&
        g_dm2_last_door_render.ornate_asset_drawn) {
        ++g_dm2_last_door_render.skproject_material_drawn_count;
    }
    if (g_dm2_last_door_render.door_state == 5 &&
        g_dm2_last_door_render.destroyed_mask_asset_drawn) {
        ++g_dm2_last_door_render.skproject_material_drawn_count;
    }
    if (g_dm2_last_door_render.frame_asset_drawn) {
        ++g_dm2_last_door_render.skproject_material_drawn_count;
    }
    if (g_dm2_last_door_render.button_gdat_index != 0 &&
        g_dm2_last_door_render.button_asset_drawn) {
        ++g_dm2_last_door_render.skproject_material_drawn_count;
    }
    g_dm2_last_door_render.skproject_material_chain_drawn =
        g_dm2_last_door_render.skproject_material_expected_count ==
        g_dm2_last_door_render.skproject_material_drawn_count;

    g_dm2_last_door_render.panel_asset_src_w =
        viewport->last_door_panel_asset_src_w;
    g_dm2_last_door_render.panel_asset_src_h =
        viewport->last_door_panel_asset_src_h;
    g_dm2_last_door_render.panel_asset_src_stride =
        viewport->last_door_panel_asset_src_stride;
    g_dm2_last_door_render.ornate_asset_src_w =
        viewport->last_door_ornate_asset_src_w;
    g_dm2_last_door_render.ornate_asset_src_h =
        viewport->last_door_ornate_asset_src_h;
    g_dm2_last_door_render.ornate_asset_src_stride =
        viewport->last_door_ornate_asset_src_stride;
    g_dm2_last_door_render.destroyed_mask_asset_src_w =
        viewport->last_door_destroyed_mask_asset_src_w;
    g_dm2_last_door_render.destroyed_mask_asset_src_h =
        viewport->last_door_destroyed_mask_asset_src_h;
    g_dm2_last_door_render.destroyed_mask_asset_src_stride =
        viewport->last_door_destroyed_mask_asset_src_stride;
    g_dm2_last_door_render.frame_asset_src_w =
        viewport->last_door_frame_asset_src_w;
    g_dm2_last_door_render.frame_asset_src_h =
        viewport->last_door_frame_asset_src_h;
    g_dm2_last_door_render.frame_asset_src_stride =
        viewport->last_door_frame_asset_src_stride;
    g_dm2_last_door_render.button_asset_src_w =
        viewport->last_door_button_asset_src_w;
    g_dm2_last_door_render.button_asset_src_h =
        viewport->last_door_button_asset_src_h;
    g_dm2_last_door_render.button_asset_src_stride =
        viewport->last_door_button_asset_src_stride;

    g_dm2_last_door_render.panel_asset_dst_rect =
        viewport->last_door_panel_asset_blit.dst_rect;
    g_dm2_last_door_render.ornate_asset_dst_rect =
        viewport->last_door_ornate_asset_blit.dst_rect;
    g_dm2_last_door_render.destroyed_mask_asset_dst_rect =
        viewport->last_door_destroyed_mask_asset_blit.dst_rect;
    g_dm2_last_door_render.frame_asset_dst_rect =
        viewport->last_door_frame_asset_blit.dst_rect;
    g_dm2_last_door_render.button_asset_dst_rect =
        viewport->last_door_button_asset_blit.dst_rect;
}

static int dm2_runtime_set_target_door_state(DM2_V1_RuntimeState *rt,
                                             int level,
                                             int x,
                                             int y,
                                             int state) {
    DM2_V1_DungeonData *dd;
    int raw;

    if (!rt || !rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return -1;
    return dm2_v1_dungeon_set_tile_raw(
        dd, level, x, y,
        dm2_runtime_door_set_state((uint16_t)raw, state));
}

static void dm2_runtime_record_message(DM2_V1_RuntimeState *rt,
                                       const char *message) {
    if (!rt || !message) return;
    snprintf(rt->last_target_message, sizeof(rt->last_target_message),
             "%s", message);
}

static void dm2_runtime_record_spawn(DM2_V1_RuntimeState *rt,
                                     int ai_index,
                                     int level,
                                     int x,
                                     int y) {
    int instance_id;

    if (!rt) return;
    instance_id = dm2_v1_creature_spawn(ai_index, x, y, level, 0, 8);
    rt->last_spawn_instance_id = instance_id;
    rt->last_spawn_ai = ai_index;
    rt->last_spawn_x = x;
    rt->last_spawn_y = y;
    rt->last_spawn_level = level;
    if (instance_id >= 0) rt->spawn_count++;
}

static int dm2_runtime_event_from_trigger(
    const DM2_V1_Trigger *trigger,
    const DM2_V1_TriggerState *state,
    DM2_V1_TriggerEvent *event) {
    if (!trigger || !state || !event) return 0;
    memset(event, 0, sizeof(*event));
    event->valid = 1;
    event->trigger_id = trigger->trigger_id;
    event->kind = trigger->kind;
    event->target = trigger->target;
    event->target_x = trigger->target_x;
    event->target_y = trigger->target_y;
    event->target_level = trigger->target_level;
    event->arg_creature_id = trigger->arg_creature_id;
    event->now_ms = state->last_fire_ms;
    event->fire_count = state->fired_count;
    event->message = trigger->message;
    return 1;
}

static void dm2_runtime_apply_trigger_event(DM2_V1_RuntimeState *rt,
                                            const DM2_V1_TriggerEvent *event) {
    DM2_V1_GameState *gs;
    int raw;
    int state;
    int next_state;

    if (!rt || !event || !event->valid) return;
    if (!rt->boot || !rt->boot->dm2_state) return;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    switch (event->target) {
        case DM2_TRIGGER_TARGET_DOOR_OPEN:
            dm2_runtime_set_target_door_state(rt, event->target_level,
                                              event->target_x,
                                              event->target_y, 0);
            break;
        case DM2_TRIGGER_TARGET_DOOR_CLOSE:
            dm2_runtime_set_target_door_state(rt, event->target_level,
                                              event->target_x,
                                              event->target_y, 4);
            break;
        case DM2_TRIGGER_TARGET_DOOR_TOGGLE:
            if (rt->boot->dungeon_data) {
                DM2_V1_DungeonData *dd =
                    (DM2_V1_DungeonData *)rt->boot->dungeon_data;
                raw = dm2_v1_dungeon_get_tile_raw(dd, event->target_level,
                                                  event->target_x,
                                                  event->target_y);
                if (raw >= 0) {
                    state = dm2_runtime_door_state((uint16_t)raw);
                    next_state = state == 0 ? 4 : 0;
                    dm2_runtime_set_target_door_state(rt,
                                                      event->target_level,
                                                      event->target_x,
                                                      event->target_y,
                                                      next_state);
                }
            }
            break;
        case DM2_TRIGGER_TARGET_TELEPORT_PARTY:
            gs->current_level = event->target_level;
            gs->party_x = event->target_x;
            gs->party_y = event->target_y;
            rt->dungeon_level = event->target_level;
            break;
        case DM2_TRIGGER_TARGET_SPAWN_CREATURE:
            dm2_runtime_record_spawn(rt, event->arg_creature_id,
                                     event->target_level,
                                     event->target_x,
                                     event->target_y);
            break;
        case DM2_TRIGGER_TARGET_DISPLAY_MSG:
            dm2_runtime_record_message(rt, event->message);
            break;
        default:
            break;
    }
}

static void dm2_runtime_apply_plate_event(DM2_V1_RuntimeState *rt,
                                          const DM2_V1_PlateEvent *event) {
    int raw;
    int state;
    int next_state;

    if (!rt || !event || !event->valid) return;
    switch (event->target_kind) {
        case DM2_PLATE_TARGET_DOOR_OPEN:
            dm2_runtime_set_target_door_state(rt, event->target_level,
                                              event->target_x,
                                              event->target_y, 0);
            break;
        case DM2_PLATE_TARGET_DOOR_CLOSE:
            dm2_runtime_set_target_door_state(rt, event->target_level,
                                              event->target_x,
                                              event->target_y, 4);
            break;
        case DM2_PLATE_TARGET_DOOR_TOGGLE:
            if (rt->boot && rt->boot->dungeon_data) {
                DM2_V1_DungeonData *dd =
                    (DM2_V1_DungeonData *)rt->boot->dungeon_data;
                raw = dm2_v1_dungeon_get_tile_raw(dd, event->target_level,
                                                  event->target_x,
                                                  event->target_y);
                if (raw >= 0) {
                    state = dm2_runtime_door_state((uint16_t)raw);
                    next_state = state == 0 ? 4 : 0;
                    dm2_runtime_set_target_door_state(rt,
                                                      event->target_level,
                                                      event->target_x,
                                                      event->target_y,
                                                      next_state);
                }
            }
            break;
        case DM2_PLATE_TARGET_PIT_TOGGLE:
            dm2_runtime_set_target_door_state(rt, event->target_level,
                                              event->target_x,
                                              event->target_y, 0);
            break;
        case DM2_PLATE_TARGET_MESSAGE:
            dm2_runtime_record_message(rt, event->message);
            break;
        case DM2_PLATE_TARGET_CREATURE_SPAWN:
            dm2_runtime_record_spawn(rt, DM2_AI_DRAGOTH_MINION,
                                     event->target_level,
                                     event->target_x,
                                     event->target_y);
            break;
        default:
            break;
    }
}

static void dm2_runtime_apply_timeline_event(DM2_V1_RuntimeState *rt,
                                             const DM2_V1_TimelineEvent *event) {
    if (!rt || !event) return;
    switch (event->kind) {
        case DM2_TIMELINE_EVENT_CREATURE_SPAWN:
            dm2_runtime_record_spawn(rt, event->arg_creature_id,
                                     event->arg_level,
                                     event->arg_x,
                                     event->arg_y);
            break;
        case DM2_TIMELINE_EVENT_DOOR_LOCK:
            dm2_runtime_set_target_door_state(rt, event->arg_level,
                                              event->arg_x,
                                              event->arg_y, 4);
            break;
        case DM2_TIMELINE_EVENT_DOOR_UNLOCK:
            dm2_runtime_set_target_door_state(rt, event->arg_level,
                                              event->arg_x,
                                              event->arg_y, 0);
            break;
        case DM2_TIMELINE_EVENT_MESSAGE_DISPLAY:
            dm2_runtime_record_message(rt, event->message);
            break;
        default:
            break;
    }
}

static void dm2_runtime_process_time_triggers(DM2_V1_RuntimeState *rt,
                                              int now_ms) {
    if (!rt) return;
    dm2_v1_trigger_set_now_ms(now_ms);
    for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
        DM2_V1_TriggerEvent event;
        const DM2_V1_TriggerState *state;
        const DM2_V1_Trigger *trigger = dm2_v1_trigger_get_builtin(i);
        int last;
        int delta;
        if (!trigger || !trigger->enabled) continue;
        if (trigger->kind != DM2_TRIGGER_KIND_TIME_ELAPSED) continue;
        state = dm2_v1_trigger_get_state(trigger->trigger_id);
        if (!state) continue;
        if (trigger->fire_once && state->fired_count > 0) continue;
        last = state->last_fire_ms;
        delta = (last == 0) ? now_ms : (now_ms - last);
        if (delta >= trigger->arg_time_ms &&
            dm2_v1_trigger_fire(trigger->trigger_id) ==
                (int)DM2_TRIGGER_RESULT_OK &&
            dm2_v1_trigger_copy_last_event(&event)) {
            dm2_runtime_apply_trigger_event(rt, &event);
        }
    }
}

static void dm2_runtime_process_timeline(DM2_V1_RuntimeState *rt, int now_ms) {
    int before[DM2_TIMELINE_NUM_BUILTIN];
    int fired;

    if (!rt) return;
    for (int i = 1; i <= DM2_TIMELINE_NUM_BUILTIN; ++i) {
        before[i - 1] = dm2_v1_timeline_get_fire_count(i);
    }
    fired = dm2_v1_timeline_tick(now_ms);
    if (fired <= 0) return;
    for (int i = 1; i <= DM2_TIMELINE_NUM_BUILTIN; ++i) {
        int after = dm2_v1_timeline_get_fire_count(i);
        if (after > before[i - 1]) {
            dm2_runtime_apply_timeline_event(rt,
                                             dm2_v1_timeline_get_builtin(i));
        }
    }
}

static void dm2_runtime_refresh_g1_map0_teleporter_transition(
    DM2_V1_RuntimeState *rt, int level, int x, int y)
{
    DM2_V1_G1TeleporterTransitionReceipt candidate;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *state;
    int raw;

    if (!rt) return;
    memset(&rt->g1_map0_teleporter_transition, 0,
           sizeof(rt->g1_map0_teleporter_transition));
    if (!rt->g1_first_map_runtime.committed || level != 0) {
        return;
    }

    if (!rt->boot || !rt->boot->dm2_state || !rt->boot->dungeon_data) return;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    state = (DM2_V1_GameState *)rt->boot->dm2_state;

    /* skproject/SKWIN/DME.h Teleporter lines 367-382 defines w2/w4.
     * SkWinCore.cpp _2fcf_0434 lines 51052-51090 dispatches DB1 only after
     * the source tile is ttTeleporter (5) and bit 0x08 is enabled; a DB1
     * root on an ordinary map cell is not a transition. It then checks party
     * scope bit 2 before applying destination, sound, and rotation.
     * c_map.cpp CHANGE_CURRENT_MAP_TO lines 328-370 has no
     * 0xff destination-map sentinel branch, so reject that byte explicitly. */
    for (int i = 0; i < rt->g1_first_map_runtime.teleporter_root_count; ++i) {
        const DM2_V1_G1TeleporterRoot *teleporter =
            &rt->g1_first_map_runtime.teleporters[i];
        if (teleporter->x != x || teleporter->y != y) continue;
        memset(&candidate, 0, sizeof(candidate));
        candidate.committed = 1;
        candidate.incomplete_world = rt->g1_first_map_runtime.incomplete_world;
        candidate.source_map = 0;
        candidate.source_x = x;
        candidate.source_y = y;
        candidate.source_object_id = teleporter->object_id;
        candidate.source_index = teleporter->index;
        candidate.destination_x = teleporter->destination_x;
        candidate.destination_y = teleporter->destination_y;
        candidate.destination_map = teleporter->destination_map;
        candidate.scope = teleporter->scope;
        candidate.sound = teleporter->sound;
        candidate.rotation = teleporter->rotation;
        candidate.rotation_type = teleporter->rotation_type;
        candidate.resolved_destination_map = -1;
        candidate.source_tile_active = 0;
        candidate.party_scope_allowed =
            (teleporter->scope & 2u) != 0u;
        candidate.destination_map_valid =
            teleporter->destination_map != 0xffu &&
            teleporter->destination_map < dungeon->level_count;
        if (candidate.destination_map_valid) {
            candidate.resolved_destination_map = teleporter->destination_map;
            candidate.destination_coordinates_valid =
                teleporter->destination_x <
                    dungeon->level_widths[teleporter->destination_map] &&
                teleporter->destination_y <
                    dungeon->level_heights[teleporter->destination_map];
        }
        raw = dm2_v1_dungeon_get_tile_raw(dungeon, 0, x, y);
        if (raw >= 0 &&
            dm2_v1_dungeon_get_square_type(dungeon, 0, x, y) == 5 &&
            (raw & 0x08) != 0) {
            candidate.source_tile_active = 1;
        }
        if (!dungeon->record_graph_complete || candidate.incomplete_world) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_INCOMPLETE_WORLD;
        } else if (raw < 0 ||
                   dm2_v1_dungeon_get_square_type(dungeon, 0, x, y) != 5) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_SOURCE_TILE;
        } else if ((raw & 0x08) == 0) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_SOURCE_DISABLED;
        } else if (!candidate.party_scope_allowed) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_SCOPE;
        } else if (teleporter->destination_map == 0xffu) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_MAP_SENTINEL;
        } else if (!candidate.destination_map_valid) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_MAP_RANGE;
        } else if (!candidate.destination_coordinates_valid) {
            candidate.no_transition_reason =
                DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_COORDINATES;
        } else {
            candidate.transition_applied = 1;
            candidate.sound_requested = teleporter->sound != 0;
            state->current_level = candidate.resolved_destination_map;
            state->party_x = teleporter->destination_x;
            state->party_y = teleporter->destination_y;
            state->party_dir = teleporter->rotation_type
                ? teleporter->rotation
                : ((state->party_dir + teleporter->rotation) & 3);
            state->outdoor = dm2_v1_dungeon_is_outdoor(
                dungeon, candidate.resolved_destination_map);
            rt->dungeon_level = state->current_level;
            rt->view_dir = state->party_dir;
            rt->outdoor = state->outdoor;
        }
        rt->g1_map0_teleporter_transition = candidate;
        return;
    }
}

/* ── Runtime init ──────────────────────────────────────────────────── */

void dm2_v1_runtime_init(DM2_V1_BootProfile *boot_profile) {
    if (!boot_profile) return;
    dm2_v1_gdat_scene_m11_command_plan_free(
        &g_dm2_runtime.gdat_scene_material_plan);
    dm2_v1_gdat_wall_m11_command_plan_free(
        &g_dm2_runtime.gdat_wall_material_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(
        &g_dm2_runtime.gdat_door_material_plan);
    memset(&g_dm2_runtime, 0, sizeof(g_dm2_runtime));
    memset(&g_dm2_frame_ownership, 0, sizeof(g_dm2_frame_ownership));
    g_dm2_runtime.boot = boot_profile;
    g_dm2_runtime.outdoor = 0;
    g_dm2_runtime.tick_count = 0;
    g_dm2_runtime.move_cooldown_ticks = 0;
    dm2_v1_weather_init(&g_dm2_runtime.weather);
    g_dm2_runtime.time_of_day_minutes = 720;  /* noon */
    g_dm2_runtime.dungeon_level = 0;
    g_dm2_runtime.view_dir = 0;  /* North */
    g_dm2_runtime.leader_hand_object = 0u;
    memset(g_dm2_runtime.champion_inventory_objects, 0,
           sizeof(g_dm2_runtime.champion_inventory_objects));
    dm2_v1_session_new(&g_dm2_runtime.session_snapshot);
    g_dm2_runtime.session_snapshot_valid = 1;
    memset(&g_dm2_runtime.minions, 0, sizeof(g_dm2_runtime.minions));
    g_dm2_runtime.last_npc_level = -1;
    g_dm2_runtime.last_npc_x = -1;
    g_dm2_runtime.last_npc_y = -1;
    g_dm2_runtime.last_npc_id = DM2_NPC_MERCHANT_FRIENDLY;
    g_dm2_runtime.last_npc_dialog_line = -1;
    g_dm2_runtime.last_target_message[0] = '\0';
    g_dm2_runtime.last_spawn_instance_id = -1;
    g_dm2_runtime.last_spawn_ai = -1;
    g_dm2_runtime.last_spawn_x = -1;
    g_dm2_runtime.last_spawn_y = -1;
    g_dm2_runtime.last_spawn_level = -1;
    g_dm2_runtime.spawn_count = 0;
    g_dm2_runtime.last_actuator_type = -1;
    g_dm2_runtime.last_actuator_x = -1;
    g_dm2_runtime.last_actuator_y = -1;
    g_dm2_runtime.last_actuator_level = -1;
    g_dm2_runtime.actuator_count = 0;
    g_dm2_runtime.last_generated_object = 0u;
    g_dm2_runtime.last_projectile_slot = -1;
    g_dm2_runtime.projectile_actuator_count = 0;
    dm2_v1_trigger_reset_state();
    dm2_v1_plate_reset_state();
    dm2_v1_timeline_reset_state();
    dm2_v1_timeline_init();
    g_dm2_runtime.move_callback  = NULL;
    g_dm2_runtime.turn_callback  = NULL;
    g_dm2_runtime.stairs_callback = NULL;
    g_dm2_runtime.viewport_asset_fetch = NULL;
    g_dm2_runtime.viewport_asset_user = NULL;
    g_dm2_runtime.viewport_asset_palette_fetch = NULL;
    g_dm2_runtime.viewport_asset_palette_user = NULL;
    memset(g_dm2_runtime.map_wall_gfx_list, 0,
           sizeof(g_dm2_runtime.map_wall_gfx_list));
    g_dm2_runtime.map_wall_gfx_count = 0;
    g_dm2_runtime.map_graphics_style = -1;
    if (boot_profile->dungeon_data) {
        (void)dm2_v1_dungeon_materialize_g1_first_map_runtime(
            (const DM2_V1_DungeonData *)boot_profile->dungeon_data,
            &g_dm2_runtime.g1_first_map_runtime);
        (void)dm2_v1_dungeon_materialize_g1_map5_text_runtime(
            (const DM2_V1_DungeonData *)boot_profile->dungeon_data,
            &g_dm2_runtime.g1_map5_text_runtime);
        if (g_dm2_runtime.g1_map5_text_runtime.committed) {
            (void)dm2_v1_dungeon_materialize_g1_map5_text_messages(
                (const DM2_V1_DungeonData *)boot_profile->dungeon_data,
                &g_dm2_runtime.g1_map5_text_runtime,
                &g_dm2_runtime.g1_map5_text_messages_runtime);
            (void)dm2_v1_boot_g1_gdat_text_materials(
                boot_profile, &g_dm2_runtime.g1_map5_text_runtime,
                &g_dm2_runtime.g1_map5_gdat_text_messages_runtime);
            (void)dm2_v1_boot_g1_text_wall_gfx_materials(
                boot_profile, &g_dm2_runtime.g1_map5_text_runtime,
                &g_dm2_runtime.g1_map5_text_wall_gfx_runtime);
        }
        (void)dm2_v1_boot_g1_actuator_wall_gfx_materials(
            boot_profile, g_dm2_runtime.dungeon_level,
            &g_dm2_runtime.g1_actuator_wall_gfx_runtime);
        (void)dm2_v1_boot_g1_creature_map_chip_materials(
            boot_profile, g_dm2_runtime.dungeon_level,
            &g_dm2_runtime.g1_creature_map_chip_runtime);
    }
    if (boot_profile->dm2_state) {
        DM2_V1_GameState *gs = (DM2_V1_GameState *)boot_profile->dm2_state;
        dm2_runtime_refresh_g1_map0_teleporter_transition(
            &g_dm2_runtime, gs->current_level, gs->party_x, gs->party_y);
    }
    dm2_runtime_refresh_map_wall_gfx_list(&g_dm2_runtime);
    dm2_runtime_refresh_gdat_scene_control(&g_dm2_runtime);
}

int dm2_v1_runtime_g1_first_map_receipt(
    DM2_V1_G1FirstMapRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_first_map_runtime.committed) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_first_map_runtime;
    return 1;
}

int dm2_v1_runtime_g1_map0_teleporter_transition_receipt(
    DM2_V1_G1TeleporterTransitionReceipt *out_receipt)
{
    if (!out_receipt ||
        !g_dm2_runtime.g1_map0_teleporter_transition.committed) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_map0_teleporter_transition;
    return 1;
}

int dm2_v1_runtime_g1_map5_text_receipt(
    DM2_V1_G1Map5TextRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_map5_text_runtime.committed) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_map5_text_runtime;
    return 1;
}

int dm2_v1_runtime_g1_map5_text_message_receipt(
    DM2_V1_G1TextMessageRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_map5_text_messages_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_map5_text_messages_runtime;
    return 1;
}

int dm2_v1_runtime_g1_map5_gdat_text_message_receipt(
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out_receipt)
{
    if (!out_receipt ||
        !g_dm2_runtime.g1_map5_gdat_text_messages_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_map5_gdat_text_messages_runtime;
    return 1;
}

int dm2_v1_runtime_g1_map5_text_wall_gfx_receipt(
    DM2_V1_G1TextWallGfxRuntimeReceipt *out_receipt)
{
    if (!out_receipt ||
        !g_dm2_runtime.g1_map5_text_wall_gfx_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_map5_text_wall_gfx_runtime;
    return 1;
}

int dm2_v1_runtime_g1_actuator_wall_gfx_receipt(
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_actuator_wall_gfx_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_actuator_wall_gfx_runtime;
    return 1;
}

int dm2_v1_runtime_g1_creature_map_chip_receipt(
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_creature_map_chip_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_creature_map_chip_runtime;
    return 1;
}

int dm2_v1_runtime_g1_scene_handoff_receipt(
    DM2_V1_G1SceneRuntimeHandoffReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.g1_scene_runtime_handoff;
    return out_receipt->valid;
}

int dm2_v1_runtime_bind_boot_profile(DM2_V1_BootProfile *boot_profile) {
    if (!boot_profile || !boot_profile->dm2_state) return 0;
    dm2_v1_runtime_init(boot_profile);
    if (boot_profile->graphics_dat) {
        dm2_v1_runtime_set_viewport_asset_provider(
            dm2_v1_boot_viewport_asset_fetch, boot_profile);
        g_dm2_runtime.viewport_asset_palette_fetch =
            dm2_v1_boot_viewport_asset_palette_fetch;
        g_dm2_runtime.viewport_asset_palette_user = boot_profile;
        dm2_runtime_refresh_gdat_scene_control(&g_dm2_runtime);
    }
    return 1;
}

int dm2_v1_runtime_bind_boot_profile_with_receipt(
    DM2_V1_BootProfile *boot_profile,
    DM2_V1_StartupHostReceipt *out_receipt)
{
    if (out_receipt) {
        dm2_v1_startup_host_receipt_clear(out_receipt);
        out_receipt->status_scope = "BOOT";
        out_receipt->status = "DM2 RUNTIME BIND FAILED";
    }
    if (!dm2_v1_runtime_bind_boot_profile(boot_profile)) {
        return 0;
    }
    if (out_receipt) {
        dm2_v1_startup_host_receipt_clear(out_receipt);
        out_receipt->status_scope = "BOOT";
        out_receipt->status = "DM2 RUNTIME READY";
    }
    return 1;
}

int dm2_v1_runtime_apply_session(const DM2_V1_SessionState *session) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_post_load;

    if (!session || !rt->boot || !rt->boot->dm2_state) {
        return -1;
    }
    if (!dm2_v1_session_validate(session)) {
        return -1;
    }
    if (!dm2_runtime_rebuild_original_timer_owners(session,
                                                   &timer_post_load)) {
        return -1;
    }
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    rt->session_snapshot = *session;
    rt->session_snapshot_valid = 1;
    rt->timer_post_load = timer_post_load;

    /* skproject SKWINSPX/src/v4/skgame.cpp SELECT_LOAD_GAME and
     * skfileop.cpp READ_SAVEGAMES_FILENAMES route startup resume through a
     * chosen SKSAVE digit after validating the 0xBEEF/0xDEAD slot header.
     * Firestaff's bounded session importer applies the startup-owned fields
     * already modeled by DM2_V1_GameState/RuntimeState; broader dungeon DB
     * pools stay owned by the later full SKSave importer. */
    gs->party_x = (int)session->party_x;
    gs->party_y = (int)session->party_y;
    gs->party_dir = (int)(session->party_dir & 3u);
    gs->current_level = (int)session->party_level;
    gs->outdoor = session->outdoor_mode ? 1 : 0;
    gs->gold = (int)session->gold;
    gs->reputation = (int)session->reputation;
    gs->time_of_day = (int)session->time_of_day_minutes;

    rt->tick_count = (int)session->game_tick;
    rt->outdoor = gs->outdoor;
    rt->time_of_day_minutes = gs->time_of_day;
    rt->dungeon_level = gs->current_level;
    rt->view_dir = gs->party_dir;
    dm2_runtime_refresh_map_wall_gfx_list(rt);
    dm2_runtime_refresh_gdat_scene_control(rt);
    rt->leader_hand_object = session->original_leader_hand_object;
    memset(rt->champion_inventory_objects, 0,
           sizeof(rt->champion_inventory_objects));
    for (uint8_t c = 0; c < session->champion_count && c < 4u; ++c) {
        const DM2_ChampionRecord *champ =
            (const DM2_ChampionRecord *)session->champion_data[c];
        for (uint8_t slot = 0; slot < 30u; ++slot) {
            rt->champion_inventory_objects[c][slot] = champ->inventory[slot];
        }
    }
    rt->minions = session->original_minions;
    if (rt->minions.count > DM2_MAX_MINIONS) {
        rt->minions.count = DM2_MAX_MINIONS;
    }
    dm2_v1_weather_set(&rt->weather, session->rain_intensity > 0
                                      ? DM2_WEATHER_RAIN
                                      : DM2_WEATHER_CLEAR);
    rt->weather.weather_intensity = (int)session->rain_intensity;
    if (!g_dm2_runtime_restore_in_progress && rt->boot->save_root[0]) {
        dm2_runtime_restore_live_sidecar(rt->boot->save_root, session);
    }
    return 0;
}

/* ── V1 Game Tick ──────────────────────────────────────────────────── */

/* Module-static projectile drain cache (refreshed each tick).
 * M11 game view can read this to draw fireballs/lightning/arrows. */
static DM2_V1_DrainedProjectile g_dm2_projectile_drain[DM2_DRAIN_MAX_PROJECTILES];
static int g_dm2_projectile_drain_count = 0;

static void dm2_runtime_populate_projectiles(DM2_V1_ViewportState *viewport,
                                             int party_dir,
                                             int party_x,
                                             int party_y)
{
    int count;
    if (!viewport) return;
    count = g_dm2_projectile_drain_count;
    if (count > DM2_MAX_PROJECTILES) count = DM2_MAX_PROJECTILES;
    viewport->projectile_count = count;
    for (int i = 0; i < count; ++i) {
        const DM2_V1_DrainedProjectile *src = &g_dm2_projectile_drain[i];
        DM2_Projectile *dst = &viewport->projectiles[i];
        memset(dst, 0, sizeof(*dst));
        dst->projectile_category =
            (uint8_t)(src->category == PROJECTILE_CATEGORY_MAGICAL ? 0x0d : 0x10);
        dst->projectile_type = (uint8_t)(src->subtype & 0xff);
        dst->frame_index = (uint8_t)(src->frame & 0xff);
        dst->depth = 0;
        dst->screen_x = (int16_t)src->pixel_x;
        dst->screen_y = (int16_t)src->pixel_y;
        {
            DM2_V1_ViewportSpritePlacement placement;
            if (dm2_v1_viewport_project_map_to_sprite(
                    src->map_x, src->map_y, party_dir, party_x, party_y,
                    &placement)) {
                dst->depth = (int16_t)placement.depth;
                dst->screen_x = (int16_t)placement.screen_x;
                dst->screen_y = (int16_t)placement.screen_y;
            }
        }
        dst->render_kind =
            (src->subtype == DM2_PROJ_SUBTYPE_MAGICAL_POISON_CLOUD)
                ? DM2_V1_PROJECTILE_RENDER_CLOUD
                : DM2_V1_PROJECTILE_RENDER_MISSILE;
        dst->direction = (uint8_t)(src->direction & 3);
        dst->object_direction = dst->direction;
        dst->frame_class =
            (dst->render_kind == DM2_V1_PROJECTILE_RENDER_CLOUD)
                ? DM2_V1_PROJECTILE_FRAME_CLASS_FRONT_ONLY
                : DM2_V1_PROJECTILE_FRAME_CLASS_DIRECTIONAL;
        switch (src->direction & 3) {
        case 0:
            dst->velocity_y = -3;
            break;
        case 1:
            dst->velocity_x = 3;
            break;
        case 2:
            dst->velocity_y = 3;
            break;
        default:
            dst->velocity_x = -3;
            break;
        }
        dst->palette_shift = (uint8_t)(src->frame & 7);
    }
}

static uint32_t dm2_runtime_creature_material_plan_step(uint32_t hash,
                                                        uint32_t value)
{
    int shift;

    for (shift = 0; shift < 32; shift += 8) {
        hash ^= (value >> shift) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

/* c_map.cpp's DB4 route selects CREATURES/type/F9 before DRAW_MAP_CHIP.
 * QUERY_CREATURE_PICST's live route instead selects a concrete dtImage field
 * through the V5 FB/FC/FD chain. Preserve either original owner in M11, but
 * never let one route borrow material from the other. */
static int dm2_runtime_creature_material_plan_identity(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash,
    int *out_count)
{
    uint32_t hash = 2166136261u;
    int i;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0;
    if (!receipt || !receipt->valid || !viewport || !out_hash ||
        !out_count || viewport->creature_count <= 0) {
        return 0;
    }
    for (i = 0; i < viewport->creature_count; ++i) {
        const DM2_CreatureSprite *sprite = &viewport->creatures[i];
        const DM2_V1_G1CreatureMapChipMaterial *material = NULL;
        uint32_t material_hash = 0u;
        int material_index;

        if (sprite->source_kind == 1) {
            if (!sprite->source_material_proven ||
                sprite->source_material_hash == 0u ||
                sprite->gdat_image_field == DM2_GDAT_IMG_MAP_CHIP) {
                return 0;
            }
            hash = dm2_runtime_creature_material_plan_step(
                hash, sprite->source_material_hash);
            hash = dm2_runtime_creature_material_plan_step(
                hash, (uint32_t)dm2_v1_viewport_creature_field_graphic_index(
                    sprite->creature_type, sprite->gdat_image_field));
            hash = dm2_runtime_creature_material_plan_step(
                hash, sprite->gdat_image_field);
            hash = dm2_runtime_creature_material_plan_step(
                hash, sprite->frame_index);
            hash = dm2_runtime_creature_material_plan_step(
                hash, (uint32_t)sprite->screen_x);
            hash = dm2_runtime_creature_material_plan_step(
                hash, (uint32_t)sprite->screen_y);
            hash = dm2_runtime_creature_material_plan_step(
                hash, (uint32_t)sprite->depth);
            ++*out_count;
            continue;
        }
        if (sprite->source_kind != 2) return 0;
        for (material_index = 0;
             material_index < receipt->material_count;
             ++material_index) {
            const DM2_V1_G1CreatureMapChipMaterial *candidate =
                &receipt->materials[material_index];
            if (candidate->object_id == sprite->object_id &&
                candidate->x == sprite->map_x && candidate->y == sprite->map_y &&
                candidate->direction == sprite->direction &&
                candidate->creature_type == sprite->creature_type) {
                material = candidate;
                break;
            }
        }
        if (!material ||
            !dm2_v1_g1_creature_map_chip_material_identity(
                material, &material_hash)) {
            return 0;
        }
        hash = dm2_runtime_creature_material_plan_step(hash, material_hash);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)dm2_v1_viewport_creature_graphic_index(
                sprite->creature_type, sprite->frame_index));
        hash = dm2_runtime_creature_material_plan_step(
            hash, sprite->frame_index);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)sprite->screen_x);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)sprite->screen_y);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)sprite->depth);
        ++*out_count;
    }
    *out_hash = hash ? hash : 1u;
    return 1;
}

static int dm2_runtime_teleporter_material_plan_identity(
    DM2_V1_BootProfile *boot,
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash)
{
    DM2_V1_BootViewportAssetEvidence evidence;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint32_t hash = 2166136261u;
    int gdat_index;

    if (out_hash) *out_hash = 0u;
    if (!boot || !viewport || !out_hash ||
        viewport->asset_teleporter_drawn_count <= 0) return 0;
    gdat_index = dm2_v1_viewport_teleporter_map_chip_graphic_index();
    if (!dm2_v1_boot_viewport_asset_evidence(boot, gdat_index, &evidence) ||
        evidence.category != DM2_GDAT_CATEGORY_TELEPORTERS ||
        evidence.entry_index != 0 || evidence.field != DM2_GDAT_IMG_MAP_CHIP ||
        dm2_v1_boot_viewport_asset_palette_fetch(
            boot, gdat_index, palette16, &palette_hash) != 0 ||
        palette_hash == 0u) {
        return 0;
    }
    hash = dm2_runtime_creature_material_plan_step(hash, evidence.raw_hash);
    hash = dm2_runtime_creature_material_plan_step(hash, evidence.decoded_hash);
    hash = dm2_runtime_creature_material_plan_step(hash, palette_hash);
    hash = dm2_runtime_creature_material_plan_step(
        hash, (uint32_t)viewport->tick_count);
    hash = dm2_runtime_creature_material_plan_step(
        hash, (uint32_t)viewport->asset_teleporter_drawn_count);
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* SKProject LOAD_LOCALLEVEL_DYN walks Map_definitions::FloorGraphics(), then
 * marks each map-local FLOOR_GFX/index/F9 map chip before DRAW_MAP_CHIP can
 * consume it.  This is a material admission, not a guessed 3D placement:
 * every listed chip must have its own decoded pixels and local palette. */
static int dm2_runtime_floor_gfx_map_chip_material_plan_identity(
    DM2_V1_BootProfile *boot,
    const uint8_t *floor_gfx_list,
    int floor_gfx_count,
    uint32_t *out_hash)
{
    uint32_t hash = 2166136261u;

    if (out_hash) *out_hash = 0u;
    if (!boot || !floor_gfx_list || !out_hash || floor_gfx_count <= 0 ||
        floor_gfx_count > 16) {
        return 0;
    }
    for (int i = 0; i < floor_gfx_count; ++i) {
        DM2_V1_BootViewportAssetEvidence evidence;
        uint8_t palette16[16];
        uint32_t palette_hash = 0u;
        int gdat_index = dm2_v1_viewport_floor_gfx_map_chip_graphic_index(
            floor_gfx_list[i]);

        if (gdat_index == 0 ||
            !dm2_v1_boot_viewport_asset_evidence(boot, gdat_index, &evidence) ||
            evidence.category != DM2_GDAT_CATEGORY_FLOOR_GFX ||
            evidence.entry_index != floor_gfx_list[i] ||
            evidence.field != DM2_GDAT_IMG_MAP_CHIP ||
            dm2_v1_boot_viewport_asset_palette_fetch(
                boot, gdat_index, palette16, &palette_hash) != 0 ||
            palette_hash == 0u) {
            return 0;
        }
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)floor_gfx_list[i]);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.raw_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.decoded_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, palette_hash);
    }
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* skproject LOAD_LOCALLEVEL_DYN makes every Map_definitions::WallGraphics()
 * entry loadable before DRAW_MAP_CHIP reaches a wall ornament.  Bind the
 * exact local list to WALL_GFX/index/F9; no generic wall image can stand in. */
static int dm2_runtime_wall_gfx_map_chip_material_plan_identity(
    DM2_V1_BootProfile *boot,
    const uint8_t *wall_gfx_list,
    int wall_gfx_count,
    uint32_t *out_hash)
{
    uint32_t hash = 2166136261u;

    if (out_hash) *out_hash = 0u;
    if (!boot || !wall_gfx_list || !out_hash || wall_gfx_count <= 0 ||
        wall_gfx_count > 16) {
        return 0;
    }
    for (int i = 0; i < wall_gfx_count; ++i) {
        DM2_V1_BootViewportAssetEvidence evidence;
        uint8_t palette16[16];
        uint32_t palette_hash = 0u;
        int gdat_index = dm2_v1_viewport_wall_gfx_map_chip_graphic_index(
            wall_gfx_list[i]);

        if (gdat_index == 0 ||
            !dm2_v1_boot_viewport_asset_evidence(boot, gdat_index, &evidence) ||
            evidence.category != DM2_GDAT_CATEGORY_WALL_GFX ||
            evidence.entry_index != wall_gfx_list[i] ||
            evidence.field != DM2_GDAT_IMG_MAP_CHIP ||
            dm2_v1_boot_viewport_asset_palette_fetch(
                boot, gdat_index, palette16, &palette_hash) != 0 ||
            palette_hash == 0u) {
            return 0;
        }
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)wall_gfx_list[i]);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.raw_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.decoded_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, palette_hash);
    }
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* LOAD_LOCALLEVEL_DYN only marks DoorType0/1 when the matching UseDoor bit
 * is set.  Admit exactly those map-local DOORS/F9 chips and their palettes. */
static int dm2_runtime_door_map_chip_material_plan_identity(
    DM2_V1_BootProfile *boot,
    const DM2_V1_DungeonData *dungeon,
    int level,
    int *out_required,
    uint32_t *out_hash)
{
    uint32_t hash = 2166136261u;
    int material_count = 0;

    if (out_required) *out_required = 0;
    if (out_hash) *out_hash = 0u;
    if (!boot || !dungeon || !out_required || !out_hash || level < 0 ||
        level >= dungeon->level_count) return 0;
    for (int slot = 0; slot < 2; ++slot) {
        int enabled = slot == 0 ? dungeon->map_use_door0[level]
                                : dungeon->map_use_door1[level];
        int door_type = slot == 0 ? dungeon->map_door_set0[level]
                                  : dungeon->map_door_set1[level];
        DM2_V1_BootViewportAssetEvidence evidence;
        uint8_t palette16[16];
        uint32_t palette_hash = 0u;
        int gdat_index;

        if (!enabled) continue;
        if (door_type < 0 || door_type > 0xff) return 0;
        gdat_index = dm2_v1_viewport_door_map_chip_graphic_index(door_type);
        if (gdat_index == 0) return 0;
        /* The global F9 dynamic-load mark includes categories whose selected
         * entry has no F9 image.  That is not a drawable door substitute:
         * leave it absent and let DRAW_DOOR's existing panel/frame plan own
         * visible doors.  Once an F9 image exists it becomes strict. */
        if (!dm2_v1_boot_viewport_asset_evidence(boot, gdat_index, &evidence)) {
            continue;
        }
        if (evidence.category != DM2_GDAT_CATEGORY_DOORS ||
            evidence.entry_index != door_type ||
            evidence.field != DM2_GDAT_IMG_MAP_CHIP ||
            dm2_v1_boot_viewport_asset_palette_fetch(
                boot, gdat_index, palette16, &palette_hash) != 0 ||
            palette_hash == 0u) return 0;
        ++material_count;
        hash = dm2_runtime_creature_material_plan_step(hash, (uint32_t)slot);
        hash = dm2_runtime_creature_material_plan_step(hash, (uint32_t)door_type);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.raw_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, evidence.decoded_hash);
        hash = dm2_runtime_creature_material_plan_step(hash, palette_hash);
    }
    if (material_count == 0) return 1;
    *out_required = 1;
    *out_hash = hash ? hash : 1u;
    return 1;
}

static void dm2_runtime_append_creature_sprite(
    DM2_V1_ViewportState *viewport,
    const DM2_V1_G1CreatureMapChipMaterial *material,
    int screen_x,
    int screen_y,
    int depth)
{
    DM2_CreatureSprite *dst;

    if (!viewport || !material) return;
    if (viewport->creature_count >= DM2_MAX_CREATURES_PER_SQ) return;

    dst = &viewport->creatures[viewport->creature_count++];
    memset(dst, 0, sizeof(*dst));
    /* The material receipt has already read only the direct DB4 b4 type and
     * matched its ObjectID/tile to CREATURES/type/F9. Do not re-traverse a
     * generic record chain to discover a different viewport candidate. */
    dst->creature_type = material->creature_type;
    dst->source_kind = 2;
    dst->object_id = material->object_id;
    dst->map_x = (int16_t)material->x;
    dst->map_y = (int16_t)material->y;
    dst->frame_index = 0;
    dst->depth = (int16_t)depth;
    dst->screen_x = (int16_t)screen_x;
    dst->screen_y = (int16_t)screen_y;
    dst->health_pct = 100;
    dst->direction = material->direction;

    memset(&g_dm2_last_creature_render, 0, sizeof(g_dm2_last_creature_render));
    /* skproject SKWIN/DME.h Creature::CreatureType() plus
     * SkWinCore.cpp DRAW_TEMP_PICST/QUERY_DUNGEON_MAP_CHIP_PICT is the DB4
     * record-to-GDAT sprite route. The render pass appends the atlas blit
     * fields once asset dimensions are known. */
    g_dm2_last_creature_render.valid = 1;
    g_dm2_last_creature_render.instance_id = -1;
    g_dm2_last_creature_render.thing_handle = material->object_id;
    g_dm2_last_creature_render.source_kind = 2;
    g_dm2_last_creature_render.creature_type = dst->creature_type;
    g_dm2_last_creature_render.frame_index = dst->frame_index;
    g_dm2_last_creature_render.direction = dst->direction;
    g_dm2_last_creature_render.hp_pct = 100;
    g_dm2_last_creature_render.map_x = material->x;
    g_dm2_last_creature_render.map_y = material->y;
    g_dm2_last_creature_render.screen_x = screen_x;
    g_dm2_last_creature_render.screen_y = screen_y;
    g_dm2_last_creature_render.depth = depth;
    g_dm2_last_creature_render.gdat_index =
        dm2_v1_viewport_creature_graphic_index(dst->creature_type,
                                               dst->frame_index);
}

static int dm2_runtime_creature_frame_source_from_instance(
    const DM2_V1_CreatureInstance *inst)
{
    if (!inst) return 0;
    if (inst->b_1a == DM2_CCM_CREATURE_ATTACKS_PARTY) return 2;
    if (inst->attack_cooldown > 0) return 1;
    return 0;
}

static void dm2_runtime_append_creature_instance_sprite(
    DM2_V1_ViewportState *viewport,
    const DM2_V1_CreatureInstance *inst,
    const DM2_V1_ViewportSpritePlacement *placement,
    const DM2_V1_BootDynamicCreatureMaterialReceipt *material)
{
    DM2_CreatureSprite *dst;
    int hp_pct = 100;

    if (!viewport || !inst || !placement || !material || !material->valid ||
        !placement->visible) return;
    if (!inst->alive || !inst->is_visible) return;
    if (viewport->creature_count >= DM2_MAX_CREATURES_PER_SQ) return;

    if (inst->hp_max > 0) {
        hp_pct = (inst->hp_current * 100) / inst->hp_max;
        if (hp_pct < 0) hp_pct = 0;
        if (hp_pct > 100) hp_pct = 100;
    }

    dst = &viewport->creatures[viewport->creature_count++];
    memset(dst, 0, sizeof(*dst));
    dst->creature_type = (uint8_t)(inst->ai_index & 0xff);
    dst->source_kind = 1;
    dst->source_material_proven = 1;
    dst->gdat_image_field = material->image_field;
    dst->source_material_hash = material->material_hash;
    dst->frame_index = (uint8_t)material->selected_frame;
    dst->depth = (int16_t)placement->depth;
    dst->screen_x = (int16_t)placement->screen_x;
    dst->screen_y = (int16_t)placement->screen_y;
    dst->health_pct = (uint8_t)hp_pct;
    dst->direction = (uint8_t)(inst->direction & 3);

    memset(&g_dm2_last_creature_render, 0, sizeof(g_dm2_last_creature_render));
    g_dm2_last_creature_render.valid = 1;
    g_dm2_last_creature_render.instance_id = inst->instance_id;
    g_dm2_last_creature_render.thing_handle = -1;
    g_dm2_last_creature_render.source_kind = 1;
    g_dm2_last_creature_render.creature_type = dst->creature_type;
    g_dm2_last_creature_render.frame_index = dst->frame_index;
    g_dm2_last_creature_render.direction = dst->direction;
    g_dm2_last_creature_render.hp_pct = hp_pct;
    g_dm2_last_creature_render.ccm_primary_state = inst->b_1a;
    g_dm2_last_creature_render.ccm_secondary_state = inst->b_17;
    g_dm2_last_creature_render.attack_cooldown = inst->attack_cooldown;
    g_dm2_last_creature_render.animation_tick = inst->animation_tick;
    g_dm2_last_creature_render.render_revision = inst->render_revision;
    g_dm2_last_creature_render.frame_source =
        dm2_runtime_creature_frame_source_from_instance(inst);
    g_dm2_last_creature_render.map_x = inst->world_x;
    g_dm2_last_creature_render.map_y = inst->world_y;
    g_dm2_last_creature_render.screen_x = placement->screen_x;
    g_dm2_last_creature_render.screen_y = placement->screen_y;
    g_dm2_last_creature_render.depth = placement->depth;
    g_dm2_last_creature_render.gdat_index =
        dm2_v1_viewport_creature_field_graphic_index(
            dst->creature_type, material->image_field);
}

static void dm2_runtime_finish_creature_render_receipt(
    const DM2_V1_ViewportState *viewport)
{
    const DM2_V1_CreatureAssetBlit *blit;
    const DM2_V1_CreatureRender *render;
    int frame_count;

    if (!viewport || !g_dm2_last_creature_render.valid ||
        !viewport->last_creature_render_valid) {
        return;
    }
    render = &viewport->last_creature_render;
    g_dm2_last_creature_render.draw_order =
        viewport->last_creature_draw_order;
    g_dm2_last_creature_render.fallback_rect = render->fallback_rect;
    if (!viewport->last_creature_asset_blit_valid ||
        viewport->last_creature_asset_blit.draw_order !=
            viewport->last_creature_draw_order) {
        /* skproject SKWIN/SkWinCore.cpp DRAW_TEMP_PICST falls back only when
         * QUERY_DUNGEON_MAP_CHIP_PICT cannot supply a drawable map-chip. Keep
         * that renderer-owned decision in the runtime receipt instead of
         * making host code infer it from aggregate counters. */
        g_dm2_last_creature_render.fallback_drawn = 1;
        g_dm2_last_creature_render.asset_blit_ready = 0;
        g_dm2_last_creature_render.requested_frame_index =
            render->frame_index;
        g_dm2_last_creature_render.party_direction = viewport->party_dir & 3;
        g_dm2_last_creature_render.relative_direction =
            ((viewport->party_dir & 3) - (render->direction & 3)) & 3;
        return;
    }
    blit = &viewport->last_creature_asset_blit;
    render = &viewport->last_creature_asset_render;
    frame_count = dm2_v1_viewport_map_chip_frame_count(
        viewport->last_creature_asset_src_w,
        viewport->last_creature_asset_src_h);
    /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP uses the requested
     * animation frame plus view-relative creature direction to choose the
     * atlas cell. Carry that resolved table row back to runtime receipt. */
    g_dm2_last_creature_render.gdat_index = blit->gdat_index;
    g_dm2_last_creature_render.draw_order = blit->draw_order;
    g_dm2_last_creature_render.asset_blit_ready = 1;
    g_dm2_last_creature_render.fallback_drawn = 0;
    g_dm2_last_creature_render.asset_src_w =
        viewport->last_creature_asset_src_w;
    g_dm2_last_creature_render.asset_src_h =
        viewport->last_creature_asset_src_h;
    g_dm2_last_creature_render.asset_src_stride =
        viewport->last_creature_asset_src_stride;
    g_dm2_last_creature_render.asset_frame_count = frame_count;
    g_dm2_last_creature_render.requested_frame_index =
        render->frame_index;
    g_dm2_last_creature_render.party_direction = viewport->party_dir & 3;
    g_dm2_last_creature_render.relative_direction =
        ((viewport->party_dir & 3) - (render->direction & 3)) & 3;
    g_dm2_last_creature_render.atlas_frame_index = blit->render_frame;
    g_dm2_last_creature_render.atlas_frame_x = blit->frame_x;
    g_dm2_last_creature_render.atlas_frame_y = blit->frame_y;
    g_dm2_last_creature_render.atlas_frame_w = blit->frame_w;
    g_dm2_last_creature_render.atlas_frame_h = blit->frame_h;
    g_dm2_last_creature_render.render_frame = blit->render_frame;
    g_dm2_last_creature_render.asset_dst_rect = blit->dst_rect;
}

static void dm2_runtime_finish_item_render_receipt(
    const DM2_V1_ViewportState *viewport)
{
    const DM2_V1_ItemRender *render;
    const DM2_V1_ItemAssetBlit *blit;

    memset(&g_dm2_last_item_render, 0, sizeof(g_dm2_last_item_render));
    if (!viewport || !viewport->last_item_render_valid) {
        return;
    }

    render = &viewport->last_item_render;
    g_dm2_last_item_render.valid = 1;
    g_dm2_last_item_render.source_kind = viewport->last_item_source_kind;
    g_dm2_last_item_render.item_index = render->item_index;
    g_dm2_last_item_render.item_category = render->item_category;
    g_dm2_last_item_render.item_type = render->item_type;
    g_dm2_last_item_render.frame_index = render->frame_index;
    g_dm2_last_item_render.direction = render->direction;
    g_dm2_last_item_render.depth = render->depth;
    g_dm2_last_item_render.center_x = render->center_x;
    g_dm2_last_item_render.center_y = render->center_y;
    g_dm2_last_item_render.gdat_index = render->gdat_index;
    g_dm2_last_item_render.draw_order = viewport->last_item_draw_order;
    g_dm2_last_item_render.flip_mirror = render->flip_mirror;
    g_dm2_last_item_render.fallback_radius = render->fallback_radius;

    if (!viewport->last_item_asset_blit_valid ||
        viewport->last_item_asset_blit.draw_order !=
            viewport->last_item_draw_order) {
        /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP consumes item,
         * carried-item and creature-possession map chips through the same
         * QUERY_DUNGEON_MAP_CHIP_PICT path. This receipt keeps that final
         * renderer decision DM2-owned, including fallback. */
        g_dm2_last_item_render.fallback_drawn = 1;
        return;
    }

    blit = &viewport->last_item_asset_blit;
    g_dm2_last_item_render.asset_blit_ready = 1;
    g_dm2_last_item_render.asset_src_w = viewport->last_item_asset_src_w;
    g_dm2_last_item_render.asset_src_h = viewport->last_item_asset_src_h;
    g_dm2_last_item_render.asset_src_stride =
        viewport->last_item_asset_src_stride;
    g_dm2_last_item_render.asset_frame_count =
        dm2_v1_viewport_map_chip_frame_count(
            viewport->last_item_asset_src_w,
            viewport->last_item_asset_src_h);
    g_dm2_last_item_render.render_frame = blit->render_frame;
    g_dm2_last_item_render.atlas_frame_x = blit->frame_x;
    g_dm2_last_item_render.atlas_frame_y = blit->frame_y;
    g_dm2_last_item_render.atlas_frame_w = blit->frame_w;
    g_dm2_last_item_render.atlas_frame_h = blit->frame_h;
    g_dm2_last_item_render.asset_dst_rect = blit->dst_rect;
}

static void dm2_runtime_finish_projectile_render_receipt(
    const DM2_V1_ViewportState *viewport)
{
    const DM2_V1_ProjectileRender *render;
    const DM2_V1_ProjectileAssetBlit *blit;

    memset(&g_dm2_last_projectile_render, 0,
           sizeof(g_dm2_last_projectile_render));
    if (!viewport || !viewport->last_projectile_render_valid) {
        return;
    }

    render = &viewport->last_projectile_render;
    g_dm2_last_projectile_render.valid = 1;
    g_dm2_last_projectile_render.projectile_index =
        render->projectile_index;
    g_dm2_last_projectile_render.projectile_category =
        render->projectile_category;
    g_dm2_last_projectile_render.projectile_type =
        render->projectile_type;
    g_dm2_last_projectile_render.frame_index = render->frame_index;
    g_dm2_last_projectile_render.direction = render->direction;
    g_dm2_last_projectile_render.object_direction =
        render->object_direction;
    g_dm2_last_projectile_render.frame_class = render->frame_class;
    g_dm2_last_projectile_render.render_kind = render->render_kind;
    g_dm2_last_projectile_render.depth = render->depth;
    g_dm2_last_projectile_render.center_x = render->center_x;
    g_dm2_last_projectile_render.center_y = render->center_y;
    g_dm2_last_projectile_render.gdat_index = render->gdat_index;
    g_dm2_last_projectile_render.draw_order =
        viewport->last_projectile_draw_order;
    g_dm2_last_projectile_render.flip_mirror = render->flip_mirror;
    g_dm2_last_projectile_render.cloud_flip_from_seed =
        render->cloud_flip_from_seed;
    g_dm2_last_projectile_render.fallback_dx = render->fallback_dx;
    g_dm2_last_projectile_render.fallback_dy = render->fallback_dy;
    g_dm2_last_projectile_render.fallback_len = render->fallback_len;

    if (!viewport->last_projectile_asset_blit_valid ||
        viewport->last_projectile_asset_blit.draw_order !=
            viewport->last_projectile_draw_order) {
        /* skproject SKWIN/SkWinCore.cpp lines 10672-10750 route missiles
         * and clouds through QUERY_DUNGEON_MAP_CHIP_PICT then
         * DRAW_CHIP_OF_MAGIC_MAP. Keep the final asset/fallback decision
         * in this DM2-owned receipt instead of host-side counter inference. */
        g_dm2_last_projectile_render.fallback_drawn = 1;
        return;
    }

    blit = &viewport->last_projectile_asset_blit;
    g_dm2_last_projectile_render.asset_blit_ready = 1;
    g_dm2_last_projectile_render.asset_src_w =
        viewport->last_projectile_asset_src_w;
    g_dm2_last_projectile_render.asset_src_h =
        viewport->last_projectile_asset_src_h;
    g_dm2_last_projectile_render.asset_src_stride =
        viewport->last_projectile_asset_src_stride;
    g_dm2_last_projectile_render.asset_frame_count =
        dm2_v1_viewport_map_chip_frame_count(
            viewport->last_projectile_asset_src_w,
            viewport->last_projectile_asset_src_h);
    g_dm2_last_projectile_render.render_frame = blit->render_frame;
    g_dm2_last_projectile_render.flip_mirror = blit->flip_mirror;
    g_dm2_last_projectile_render.atlas_frame_x = blit->frame_x;
    g_dm2_last_projectile_render.atlas_frame_y = blit->frame_y;
    g_dm2_last_projectile_render.atlas_frame_w = blit->frame_w;
    g_dm2_last_projectile_render.atlas_frame_h = blit->frame_h;
    g_dm2_last_projectile_render.asset_dst_rect = blit->dst_rect;
    g_dm2_last_projectile_render.random_seed_before =
        blit->random_seed_before;
    g_dm2_last_projectile_render.random_seed_after =
        blit->random_seed_after;
}

static void dm2_runtime_populate_active_creature_instances(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    if (!rt || !viewport || rt->outdoor) return;

    for (int slot = 0; slot < DM2_MAX_CREATURE_INSTANCES &&
                       viewport->creature_count < DM2_MAX_CREATURES_PER_SQ;
         ++slot) {
        const DM2_V1_CreatureInstance *inst =
            dm2_v1_creature_get_instance(slot);
        DM2_V1_ViewportSpritePlacement placement;

        if (!inst || !inst->alive || inst->map_index != rt->dungeon_level) {
            continue;
        }
        if (!dm2_v1_viewport_project_map_to_sprite(
                inst->world_x, inst->world_y, party_dir, party_x, party_y,
                &placement)) {
            continue;
        }
        DM2_V1_BootDynamicCreatureMaterialReceipt material;

        /* skproject QUERY_CREATURE_PICST receives the live command and V5
         * mutable animation state, then draws FD's exact CREATURES dtImage.
         * A missing table/image receipt omits the sprite entirely. */
        if (!rt->boot || !rt->boot->graphics_dat ||
            !dm2_v1_creature_ai_spec(inst->ai_index) ||
            !dm2_v1_boot_dynamic_creature_material_receipt(
                rt->boot, inst->ai_index, inst->b_1a,
                inst->gdat_animation_info, inst->direction, &material)) {
            continue;
        }
        if (dm2_v1_creature_set_gdat_animation_state(
                slot, material.sequence_offset, material.selected_frame) != 0) {
            continue;
        }
        dm2_runtime_append_creature_instance_sprite(
            viewport, inst, &placement, &material);
    }
}

static void dm2_runtime_populate_creatures(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt;

    if (!rt || !viewport || rt->outdoor) {
        return;
    }
    receipt = &rt->g1_creature_map_chip_runtime;
    if (!receipt->valid || receipt->map != rt->dungeon_level) return;

    for (int i = 0; i < receipt->material_count &&
                    viewport->creature_count < DM2_MAX_CREATURES_PER_SQ; ++i) {
        const DM2_V1_G1CreatureMapChipMaterial *material =
            &receipt->materials[i];
        DM2_V1_ViewportSpritePlacement placement;

        if (!dm2_v1_viewport_project_map_to_sprite(
                material->x, material->y, party_dir, party_x, party_y,
                &placement)) {
            continue;
        }
        dm2_runtime_append_creature_sprite(
            viewport, material, placement.screen_x, placement.screen_y,
            placement.depth);
    }
}

static void dm2_runtime_append_creature_possession_item(
    DM2_V1_ViewportState *viewport,
    uint16_t thing,
    int screen_x,
    int screen_y,
    int depth,
    const uint8_t *record,
    int record_size)
{
    int type;
    uint16_t w2;
    DM2_ItemSprite *dst;

    if (!viewport || !record || record_size < 4) return;
    if (viewport->creature_possession_item_count >=
        DM2_MAX_CREATURE_POSSESSION_ITEMS) {
        return;
    }
    type = (int)((thing >> 10) & 0x0fu);
    if (type < 5 || type > 10) return;

    dst = &viewport->creature_possession_items[
        viewport->creature_possession_item_count++];
    memset(dst, 0, sizeof(*dst));
    w2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    dst->item_category =
        (uint8_t)dm2_v1_viewport_item_category_for_db_pool(type);
    dst->item_type = (uint8_t)(w2 & 0x7fu);
    dst->frame_index = 0;
    dst->depth = (int16_t)depth;
    dst->screen_x = (int16_t)screen_x;
    dst->screen_y = (int16_t)screen_y;
    dst->direction = (uint8_t)((thing >> 14) & 3u);
}

static void dm2_runtime_populate_creature_possession_items(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    DM2_V1_DungeonData *dd;
    int dir;
    int right;

    if (!rt || !viewport || rt->outdoor || !rt->boot ||
        !rt->boot->dungeon_data) {
        return;
    }
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    dir = party_dir & 3;
    right = (dir + 1) & 3;

    for (int forward = 1; forward <= 4; ++forward) {
        for (int lateral = -2; lateral <= 2; ++lateral) {
            int map_x = party_x + dx[dir] * forward +
                        dx[right] * lateral;
            int map_y = party_y + dy[dir] * forward +
                        dy[right] * lateral;
            int thing = dm2_v1_dungeon_get_first_thing(
                dd, rt->dungeon_level, map_x, map_y);
            int guard = 0;
            DM2_V1_ViewportSpritePlacement base_placement;

            if (thing < 0 || thing == 0xfffe) continue;
            if (!dm2_v1_viewport_project_map_to_sprite(
                    map_x, map_y, party_dir, party_x, party_y,
                    &base_placement)) {
                continue;
            }
            while (thing >= 0 && thing != 0xfffe && guard++ < 64) {
                int type = -1;
                int size = 0;
                int next;
                const uint8_t *record = dm2_v1_dungeon_get_thing_record(
                    dd, (uint16_t)thing, &type, NULL, &size);
                if (!record || size < 2) break;
                if (type == 4 && size >= 4) {
                    uint16_t possession =
                        (uint16_t)record[2] | ((uint16_t)record[3] << 8);
                    int possession_guard = 0;
                    int possession_slot = 0;

                    /* skproject SKWIN/DME.h Creature::possession and
                     * SkWinCore.cpp lines 10644-10666 walk the creature
                     * possession chain with GET_NEXT_RECORD_LINK, then draw
                     * dbWeapon..dbMiscellaneous_item through DRAW_MAP_CHIP
                     * using ObjectID::Dir() for the map-chip flip table. */
                    while (possession != 0xfffe &&
                           possession_guard++ < 32 &&
                           viewport->creature_possession_item_count <
                               DM2_MAX_CREATURE_POSSESSION_ITEMS) {
                        int item_size = 0;
                        DM2_V1_ViewportSpritePlacement slot_placement;
                        const uint8_t *item_record =
                            dm2_v1_dungeon_get_thing_record(
                                dd, possession, NULL, NULL, &item_size);
                        if (!item_record || item_size < 2) break;
                        if (!dm2_v1_viewport_possession_slot_placement(
                                &base_placement, possession_slot,
                                &slot_placement)) {
                            break;
                        }
                        dm2_runtime_append_creature_possession_item(
                            viewport, possession,
                            slot_placement.screen_x,
                            slot_placement.screen_y,
                            slot_placement.depth, item_record, item_size);
                        ++possession_slot;
                        next = dm2_v1_dungeon_get_next_thing(dd, possession);
                        if (next < 0 || next == (int)possession) break;
                        possession = (uint16_t)next;
                    }
                }
                next = dm2_v1_dungeon_get_next_thing(dd, (uint16_t)thing);
                if (next < 0 || next == thing) break;
                thing = next;
            }
        }
    }
}

/*
 * dm2_v1_runtime_tick — advance DM2 game state by one V1 tick.
 *
 * Called at 18.2 Hz (every ~55ms) from the Firestaff game loop.
 * Advances: time-of-day, movement cooldown, weather, timers,
 * and refreshes the projectile drain cache for runtime viewport rendering.
 *
 * Movement is gated by move_cooldown_ticks — each successful move
 * consumes 1 tick; failing a move (wall) may incur penalty.
 *
 * Source: SKULL.ASM T048 — input dispatch / tick update
 *         SKULL.ASM T560 — dungeon tick
 *         SKULL.ASM T600 — outdoor tick
 *         skproject/SKULLWIN/c_render.cpp — projectile draw dispatch
 */
void dm2_v1_runtime_tick(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_CreatureFieldRuntime creature_field;
    rt->tick_count++;

    /* Advance time-of-day (1440 min per day) */
    if (rt->tick_count % 1092 == 0) {  /* ~1092 ticks = 1 minute */
        rt->time_of_day_minutes = (rt->time_of_day_minutes + 1) % 1440;
    }

    /* Movement cooldown counts down */
    if (rt->move_cooldown_ticks > 0) {
        rt->move_cooldown_ticks--;
    }

    /* Outdoor weather tick */
    if (rt->outdoor && rt->tick_count % 182 == 0) {  /* ~10 sec */
        dm2_v1_weather_next_state(&rt->weather);
    }

    dm2_runtime_process_time_triggers(rt, rt->tick_count * 55);
    dm2_runtime_process_timeline(rt, rt->tick_count * 55);

    memset(&creature_field, 0, sizeof(creature_field));
    creature_field.read_door = dm2_runtime_creature_read_door;
    creature_field.user = rt;
    dm2_v1_creature_set_field_runtime(&creature_field);
    /* skproject/SKULLWIN/c_ai.cpp DM2_THINK_CREATURE and
     * c_creature.cpp DM2_PROCEED_CCM read the live dungeon field while
     * advancing b_1a/b_17 creature state.  Firestaff now gives the creature
     * tick the runtime's dungeon-backed door reader, then clears the bridge so
     * standalone creature tests and later sessions cannot retain stale boot
     * pointers. */
    dm2_v1_creature_tick();
    dm2_v1_creature_reset_field_runtime();

    /* Phase 5+ extension: step then drain DM2 projectile list into
     * M11-ready cache.  The step path applies the STEP_MISSILE
     * energy-decay + despawn boundary (skproject/SKULLWIN/c_tim_proc.cpp
     * m_7CE0/m_7D2A), so the drain reflects only post-step survivors.
     * Without this step the cache would grow without bound and the
     * runtime viewport would draw stale projectiles forever.
     * Source: skproject/SKULLWIN/c_tim_proc.cpp:442-563   (DM2_STEP_MISSILE)
     *         skproject/SKULLWIN/c_render.cpp              (projectile draw)
     *         ReDMCSB DUNGEON.C:2362-2387                  (F0209 visible)
     */
    g_dm2_projectile_drain_count = dm2_v1_projectile_step_and_drain(
        g_dm2_projectile_drain, DM2_DRAIN_MAX_PROJECTILES, NULL);
}

/*
 * dm2_v1_runtime_get_tick_count — narrow probe accessor for the V1 tick
 * boundary.  This exposes only the deterministic tick counter advanced by
 * dm2_v1_runtime_tick(); it does not claim broader DM2 runtime parity.
 *
 * Source: SKULL.ASM T048/T560 tick/update boundary; ReDMCSB GAMELOOP.C
 * lines 55-70 show the V1 loop advancing timeline work once per loop.
 */
int dm2_v1_runtime_get_tick_count(void) {
    return g_dm2_runtime.tick_count;
}

int dm2_v1_runtime_last_timer_post_load_receipt(
    DM2_V1_RuntimeTimerPostLoadReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.timer_post_load;
    return out_receipt->valid;
}

/*
 * dm2_v1_runtime_get_projectile_drain — read-only access to the
 * per-tick projectile drain cache.  M11 game view calls this each
 * render frame to draw DM2 projectiles in the V1 viewport.
 *
 * Returns the count (0..DM2_DRAIN_MAX_PROJECTILES).
 * *out_list is set to the module-static array (do not free).
 */
int dm2_v1_runtime_get_projectile_drain(DM2_V1_DrainedProjectile **out_list) {
    if (out_list) *out_list = g_dm2_projectile_drain;
    return g_dm2_projectile_drain_count;
}

static void dm2_runtime_populate_carried_item(const DM2_V1_RuntimeState *rt,
                                              DM2_V1_ViewportState *viewport)
{
    uint8_t pool = 0;
    uint32_t index = 0u;
    DM2_ItemSprite *dst;

    if (!rt || !viewport) return;
    if (!dm2_db_decode_handle(rt->leader_hand_object, &pool, &index)) return;

    dst = &viewport->carried_item;
    memset(dst, 0, sizeof(*dst));
    dst->item_category =
        (uint8_t)dm2_v1_viewport_item_category_for_db_pool((int)pool);
    dst->item_type = (uint8_t)(index & 0xffu);
    dst->frame_index = 0;
    dst->depth = 0;
    dst->screen_x = 300;
    dst->screen_y = 184;
    viewport->carried_item_present = 1;
}

static uint8_t dm2_runtime_hud_pct_from_current_max(uint16_t current,
                                                    uint16_t max)
{
    if (max == 0u) return 0u;
    if (current >= max) return 100u;
    return (uint8_t)(((uint32_t)current * 100u) / (uint32_t)max);
}

static uint8_t dm2_runtime_hud_pct_from_current(uint16_t current)
{
    if (current >= 100u) return 100u;
    return (uint8_t)current;
}

static void dm2_runtime_populate_hud_party(const DM2_V1_RuntimeState *rt,
                                           DM2_V1_ViewportState *viewport)
{
    DM2_V1_HudPartyState hud;

    if (!rt || !viewport || !rt->session_snapshot_valid) return;

    /* skproject/SKWIN keeps HP, stamina, mana, leader, and champion names in
     * glbChampionSquad plus the selected leader field before the T560 HUD
     * draw path consumes them.  This bridge gives the viewport renderer the
     * same bounded party-state inputs without letting M11 rebuild HUD data. */
    memset(&hud, 0, sizeof(hud));
    hud.champion_count = rt->session_snapshot.champion_count;
    if (hud.champion_count > DM2_V1_HUD_CHAMPION_SLOT_COUNT) {
        hud.champion_count = DM2_V1_HUD_CHAMPION_SLOT_COUNT;
    }
    hud.leader_index = rt->session_snapshot.leader_index;
    if (hud.leader_index < 0 || hud.leader_index >= hud.champion_count) {
        hud.leader_index = 0;
    }

    for (int slot = 0; slot < hud.champion_count; ++slot) {
        const DM2_ChampionRecord *champ =
            (const DM2_ChampionRecord *)
                rt->session_snapshot.champion_data[slot];
        DM2_V1_HudChampionState *dst = &hud.champions[slot];
        const uint8_t *source_champion =
            rt->session_snapshot.champion_data[slot];
        char source_first_name[8];

        dst->occupied = champ->first_name[0] != '\0' ||
                        champ->cur_hp != 0u || champ->max_hp != 0u;
        dst->leader = slot == hud.leader_index;
        dst->hp_pct =
            dm2_runtime_hud_pct_from_current_max(champ->cur_hp,
                                                 champ->max_hp);
        dst->stamina_pct =
            dm2_runtime_hud_pct_from_current(champ->stamina);
        dst->mana_pct = dm2_runtime_hud_pct_from_current(champ->mana);
        dst->portrait_index = 0u;
        /* skproject/SKWIN/DME.h::Champion::heroType is byte 255 of the
         * 261-byte save record.  REVIVE_PLAYER writes it from the source
         * mirror actuator and DRAW_CHAMPION_PICTURE uses that exact GDAT
         * index.  The local portrait_index tail is not a substitute. */
        dst->portrait_type_source_bound = 0;
        memset(source_first_name, 0, sizeof(source_first_name));
        if (dm2_v1_boot_champion_hero_type_source_ready(
                rt->boot, source_champion[255], source_first_name) &&
            strncmp(source_first_name, champ->first_name,
                    sizeof(source_first_name)) == 0) {
            dst->portrait_index = source_champion[255];
            dst->portrait_type_source_bound = 1;
        }
        memcpy(dst->name, champ->first_name, DM2_V1_HUD_CHAMPION_NAME_MAX);
        dst->name[DM2_V1_HUD_CHAMPION_NAME_MAX] = '\0';
    }

    dm2_v1_viewport_set_hud_party(viewport, &hud);
}

/* ── Viewport rendering ────────────────────────────────────────────── */

/*
 * dm2_v1_runtime_render_frame — render one DM2 viewport frame.
 *
 * party_dir:  facing direction (0=N, 1=E, 2=S, 3=W)
 * party_x, party_y: position on dungeon grid or outdoor map
 *
 * DM2 viewport renders differently depending on outdoor/dungeon mode:
 *   - Dungeon: first-person 3D view using DM2 wall/floor graphics
 *   - Outdoor: overhead or 3D sky view with weather overlay
 *
 * For Phase 1, this renders a placeholder frame that distinguishes
 * DM2 from DM1/CSB: a blue-ish sky for outdoor, dark dungeon for indoor.
 * Real DM2 rendering is Phase 4.
 *
 * Returns 0 on success.
 *
 * Source: SKULL.ASM T560 — viewport frame rendering
 *         SKULL.ASM T600 — outdoor rendering
 */
int dm2_v1_runtime_render_frame(int party_dir, int party_x, int party_y,
                                  uint8_t *framebuffer, int fb_stride,
                                  int view_w, int view_h) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_ViewportState viewport;
    const uint8_t *rect14_rows = NULL;
    uint32_t rect14_row_count = 0u;
    uint32_t rect14_hash = 0u;
    const uint8_t *font_rows = NULL;
    uint32_t font_hash = 0u;
    DM2_V1_InterfaceHudLayout hud_layout;
    DM2_V1_GdatHudM11CommandPlan hud_material_plan;
    uint32_t hud_material_plan_hash = 0u;
    int hud_material_plan_required = 0;
    int hud_material_plan_consumed = 0;
    uint32_t creature_material_plan_hash = 0u;
    int creature_material_plan_required = 0;
    int creature_material_plan_consumed = 0;
    int creature_material_plan_count = 0;
    uint32_t teleporter_material_plan_hash = 0u;
    int teleporter_material_plan_required = 0;
    int teleporter_material_plan_consumed = 0;
    uint32_t floor_gfx_map_chip_material_plan_hash = 0u;
    int floor_gfx_map_chip_material_plan_required = 0;
    int floor_gfx_map_chip_material_plan_consumed = 0;
    uint32_t wall_gfx_map_chip_material_plan_hash = 0u;
    int wall_gfx_map_chip_material_plan_required = 0;
    int wall_gfx_map_chip_material_plan_consumed = 0;
    uint32_t door_map_chip_material_plan_hash = 0u;
    int door_map_chip_material_plan_required = 0;
    int door_map_chip_material_plan_consumed = 0;
    DM2_V1_DoorRenderPlan door_render_plan;
    DM2_V1_InterfaceRect14HostReceipt rect14_host;
    DM2_V1_DialogueBoxHostCommand save_dialogue_command;
    DM2_V1_DialogueOpenPanelHostCommand save_dialogue_open_panel;
    static const int forward_dx[4] = { 0, 1, 0, -1 };
    static const int forward_dy[4] = { -1, 0, 1, 0 };

    if (!framebuffer || fb_stride <= 0 ||
        view_w < DM2_VP_WIDTH || view_h < DM2_VP_HEIGHT) {
        return -1;
    }

    memset(&g_dm2_last_creature_render, 0, sizeof(g_dm2_last_creature_render));
    memset(&g_dm2_last_item_render, 0, sizeof(g_dm2_last_item_render));
    memset(&g_dm2_last_projectile_render, 0,
           sizeof(g_dm2_last_projectile_render));
    memset(&g_dm2_last_door_render, 0, sizeof(g_dm2_last_door_render));
    memset(&save_dialogue_command, 0, sizeof(save_dialogue_command));
    memset(&save_dialogue_open_panel, 0, sizeof(save_dialogue_open_panel));
    dm2_v1_viewport_init(&viewport, framebuffer, fb_stride);
    dm2_v1_viewport_set_party(&viewport, party_dir, party_x, party_y);
    dm2_v1_viewport_set_level(&viewport, rt->dungeon_level);
    if (rt->boot && rt->boot->dungeon_data) {
        const DM2_V1_DungeonData *dungeon =
            (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
        if (rt->dungeon_level >= 0 && rt->dungeon_level < dungeon->level_count) {
            dm2_v1_viewport_set_gdat_scene_map_origin(
                &viewport, dungeon->map_offset_x[rt->dungeon_level],
                dungeon->map_offset_y[rt->dungeon_level]);
        }
    }
    dm2_v1_viewport_set_outdoor(&viewport, rt->outdoor);
    if (rt->g1_first_map_runtime.committed) {
        dm2_v1_viewport_set_g1_first_map_runtime(
            &viewport, &rt->g1_first_map_runtime);
        rt->g1_first_map_viewport_consumed = 1;
    }
    dm2_runtime_refresh_g1_map0_teleporter_transition(
        rt, rt->dungeon_level, party_x, party_y);
    if (rt->g1_map0_teleporter_transition.committed) {
        dm2_v1_viewport_set_g1_map0_teleporter_transition(
            &viewport, &rt->g1_map0_teleporter_transition);
        rt->g1_map0_teleporter_transition_viewport_consumed = 1;
    }
    /* skproject/SKULLWIN/c_weather.cpp DM2_UPDATE_WEATHER owns the live
     * weather state.  Outdoor rendering must consume that state, rather than
     * treating every outdoor map as rain.  Indoor maps receive no weather
     * command.  This does not create an overlay: the viewport stays
     * fail-closed until a source-backed weather material is proven. */
    dm2_v1_viewport_set_weather(&viewport,
                                rt->outdoor ? rt->weather.weather
                                            : DM2_WEATHER_CLEAR,
                                rt->outdoor ? rt->weather.weather_intensity
                                            : 0);
    dm2_v1_viewport_set_time(
        &viewport,
        (float)(rt->time_of_day_minutes % 1440) / 1440.0f);
    viewport.random_seed = rt->weather.weather_seed;
    dm2_runtime_populate_visible_terrain(rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_projectiles(&viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_active_creature_instances(
        rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_creatures(rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_creature_possession_items(
        rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_carried_item(rt, &viewport);
    dm2_runtime_populate_hud_party(rt, &viewport);
    dm2_v1_viewport_set_asset_provider(&viewport,
                                       rt->viewport_asset_fetch,
                                       rt->viewport_asset_user);
    dm2_v1_viewport_set_asset_palette_provider(
        &viewport, rt->viewport_asset_palette_fetch,
        rt->viewport_asset_palette_user);
    dm2_v1_viewport_set_source_materials_required(
        &viewport,
        rt->viewport_asset_fetch == dm2_v1_boot_viewport_asset_fetch &&
        rt->viewport_asset_user != NULL);
    dm2_v1_viewport_set_g1_creature_map_chip_materials(
        &viewport, &rt->g1_creature_map_chip_runtime);
    dm2_v1_viewport_set_g1_wall_gfx_materials(
        &viewport,
        &rt->g1_map5_text_wall_gfx_runtime,
        &rt->g1_actuator_wall_gfx_runtime);
    dm2_v1_viewport_set_gdat_scene_control(
        &viewport,
        rt->gdat_scene_control_ready,
        rt->map_graphics_style,
        rt->gdat_scene_control_hash,
        rt->gdat_scene_colorkey,
        rt->gdat_scene_flags,
        rt->gdat_scene_ambient_light,
        rt->gdat_scene_highest_light_level,
        rt->gdat_scene_void_random_fall,
        rt->gdat_scene_animated_floor,
        rt->gdat_scene_rain,
        rt->gdat_misty_map,
        rt->gdat_thunder_position,
        rt->gdat_ambient_darkness);
    dm2_v1_viewport_set_gdat_scene_material_plan(
        &viewport, &rt->gdat_scene_material_plan);
    dm2_v1_viewport_set_gdat_scene_movement_active(
        &viewport, rt->scene_movement_pending);
    dm2_v1_viewport_set_gdat_wall_material_plan(
        &viewport, &rt->gdat_wall_material_plan);
    memset(&door_render_plan, 0, sizeof(door_render_plan));
    dm2_v1_gdat_door_overlay_m11_command_plan_free(&rt->gdat_door_material_plan);
    if (dm2_v1_viewport_build_door_render_plan(&viewport, &door_render_plan) &&
        door_render_plan.door_count > 0 &&
        dm2_v1_boot_gdat_door_overlay_m11_command_plan(
            rt->boot, &door_render_plan, &rt->gdat_door_material_plan)) {
        dm2_v1_viewport_set_gdat_door_overlay_material_plan(
            &viewport, &rt->gdat_door_material_plan);
    }
    dm2_runtime_refresh_g1_scene_handoff(
        rt, rt->dungeon_level,
        party_x + forward_dx[party_dir & 3],
        party_y + forward_dy[party_dir & 3]);
    if (rt->g1_scene_runtime_handoff.blocked) {
        /* A classified source tile has no verified GDAT material. Never let
         * the renderer turn this into a fallback dungeon frame. */
        return -1;
    }
    if (rt->g1_scene_runtime_handoff.valid &&
        rt->g1_scene_runtime_handoff.scene.root_class ==
            DM2_V1_G1_SCENE_ROOT_CREATURE) {
        const DM2_V1_G1DungeonSceneClassificationReceipt *scene =
            &rt->g1_scene_runtime_handoff.scene;
        dm2_v1_viewport_set_g1_scene_creature_material(
            &viewport, 1, scene->x, scene->y,
            rt->g1_scene_runtime_handoff.creature_type,
            rt->g1_scene_runtime_handoff.gdat_index,
            rt->g1_scene_runtime_handoff.material_width,
            rt->g1_scene_runtime_handoff.material_height,
            rt->g1_scene_runtime_handoff.material_stride,
            rt->g1_scene_runtime_handoff.material_palette_hash);
    } else {
        dm2_v1_viewport_set_g1_scene_creature_material(
            &viewport, 0, 0, 0, 0, 0, 0, 0, 0, 0u);
    }
    dm2_v1_viewport_set_gdat_interface_palette(
        &viewport,
        rt->gdat_interface_palette_ready,
        rt->gdat_interface_palette_hash,
        rt->gdat_interface_palette16);
    dm2_v1_viewport_set_gdat_interface_text_palette(
        &viewport,
        rt->gdat_interface_action_palette_ready,
        rt->gdat_interface_action_palette_hash,
        rt->gdat_interface_action_palette_ready
            ? rt->gdat_interface_action_palette16 : NULL);
    /* skproject LOAD_GDAT_INTERFACE_00_02 loads dt07/0 before
     * DRAW_PLAYER_3STAT_HEALTH_BAR and DRAW_STRING consume it.  Pass only
     * the exact boot-owned six-row table; a missing table leaves text absent
     * and the source-material gate rejects the frame. */
    if (dm2_v1_boot_interface_font_table(
            rt->boot, &font_rows, &font_hash)) {
        dm2_v1_viewport_set_gdat_interface_font(
            &viewport, font_rows, font_hash);
    }
    memset(&hud_layout, 0, sizeof(hud_layout));
    memset(&hud_material_plan, 0, sizeof(hud_material_plan));
    if (viewport.hud_party_valid) {
        (void)dm2_v1_boot_gdat_hud_m11_command_plan(
            rt->boot, &viewport.hud_party, &hud_material_plan);
        dm2_v1_viewport_set_gdat_hud_material_plan(
            &viewport, &hud_material_plan);
    }
    if (dm2_v1_boot_interface_hud_layout(rt->boot, &hud_layout)) {
        dm2_v1_viewport_set_gdat_interface_hud_layout(&viewport, &hud_layout);
    }
    memset(&rect14_host, 0, sizeof(rect14_host));
    if (dm2_v1_boot_interface_rect14_host_receipt(rt->boot, &rect14_host) &&
        rect14_host.valid &&
        dm2_v1_boot_interface_rect14_table(
            rt->boot, &rect14_rows, &rect14_row_count, &rect14_hash) &&
        rect14_hash == rect14_host.table_hash &&
        rect14_row_count == rect14_host.row_count) {
        /* The renderer receives Rect14 only after the host has consumed the
         * source table's bounded placement receipt. */
        dm2_v1_viewport_set_gdat_interface_rect14(
            &viewport, rect14_rows, rect14_row_count, rect14_hash);
    }
    /* c_dialog.cpp expands RECT_453 before it blits the save/load panel.
     * Keep that full source-owned command with the live frame; M11 must not
     * substitute an inferred rectangle or a launcher panel. */
    (void)dm2_v1_boot_dialogue_box_host_command(
        rt->boot, &save_dialogue_command);
    /* Keep c_dialog.cpp::DM2_dialog_OPEN_DIALOG_PANEL as one original GDAT
     * command: it carries the panel, GDAT button labels and raw4 locations.
     * M11 decides when a save/load session is active; this frame path never
     * invents a panel, labels, colours, or coordinates. */
    (void)dm2_v1_boot_dialogue_open_panel_host_command(
        rt->boot, &save_dialogue_open_panel);
    dm2_runtime_capture_door_render_receipt(&viewport);
    viewport.tick_count = rt->tick_count;
    dm2_v1_viewport_render(&viewport);
    /* TODO(source-bound): retain SKProject's full glbIsPlayerMoving cadence
     * only after its original timing state is decoded. This V1 runtime has
     * one accepted-move presentation frame, so consume the proven 700/701
     * offsets once instead of inventing additional animation samples. */
    rt->scene_movement_pending = 0;
    /* LOAD_GDAT_INTERFACE_00_02 must hand the full original command family
     * to M11. The count proves all chrome and four party portraits consumed
     * their plan-owned pixels; a partial plan cannot fall through to a
     * generic HUD image lookup. */
    hud_material_plan_required =
        viewport.asset_hud_core_drawn_count +
        viewport.asset_hud_portrait_drawn_count > 0;
    hud_material_plan_consumed =
        hud_material_plan_required && hud_material_plan.valid &&
        hud_material_plan.command_count > 0 &&
        hud_material_plan.command_hash != 0u &&
        viewport.gdat_hud_material_plan_consumed_count ==
            hud_material_plan.command_count;
    hud_material_plan_hash = hud_material_plan_consumed
        ? hud_material_plan.command_hash : 0u;
    dm2_v1_gdat_hud_m11_command_plan_free(&hud_material_plan);
    dm2_runtime_finish_door_render_receipt(&viewport);
    dm2_runtime_finish_creature_render_receipt(&viewport);
    dm2_runtime_finish_item_render_receipt(&viewport);
    dm2_runtime_finish_projectile_render_receipt(&viewport);
    creature_material_plan_required = viewport.asset_creature_drawn_count > 0;
    creature_material_plan_consumed =
        creature_material_plan_required &&
        viewport.fallback_creature_drawn_count == 0 &&
        dm2_runtime_creature_material_plan_identity(
            &rt->g1_creature_map_chip_runtime, &viewport,
            &creature_material_plan_hash, &creature_material_plan_count) &&
        creature_material_plan_count == viewport.asset_creature_drawn_count;
    if (!creature_material_plan_consumed) {
        creature_material_plan_hash = 0u;
    }
    teleporter_material_plan_required =
        viewport.asset_teleporter_drawn_count > 0;
    teleporter_material_plan_consumed =
        teleporter_material_plan_required &&
        dm2_runtime_teleporter_material_plan_identity(
            rt->boot, &viewport, &teleporter_material_plan_hash);
    if (!teleporter_material_plan_consumed) {
        teleporter_material_plan_hash = 0u;
    }
    floor_gfx_map_chip_material_plan_required =
        !rt->outdoor && rt->map_floor_gfx_count > 0;
    floor_gfx_map_chip_material_plan_consumed =
        !floor_gfx_map_chip_material_plan_required ||
        dm2_runtime_floor_gfx_map_chip_material_plan_identity(
            rt->boot, rt->map_floor_gfx_list, rt->map_floor_gfx_count,
            &floor_gfx_map_chip_material_plan_hash);
    if (!floor_gfx_map_chip_material_plan_consumed) {
        floor_gfx_map_chip_material_plan_hash = 0u;
    }
    wall_gfx_map_chip_material_plan_required =
        !rt->outdoor && rt->map_wall_gfx_count > 0;
    wall_gfx_map_chip_material_plan_consumed =
        !wall_gfx_map_chip_material_plan_required ||
        dm2_runtime_wall_gfx_map_chip_material_plan_identity(
            rt->boot, rt->map_wall_gfx_list, rt->map_wall_gfx_count,
            &wall_gfx_map_chip_material_plan_hash);
    if (!wall_gfx_map_chip_material_plan_consumed) {
        wall_gfx_map_chip_material_plan_hash = 0u;
    }
    if (rt->boot && rt->boot->dungeon_data) {
        const DM2_V1_DungeonData *dungeon =
            (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
        if (!rt->outdoor && rt->dungeon_level >= 0 &&
            rt->dungeon_level < dungeon->level_count) {
            door_map_chip_material_plan_consumed =
                dm2_runtime_door_map_chip_material_plan_identity(
                    rt->boot, dungeon, rt->dungeon_level,
                    &door_map_chip_material_plan_required,
                    &door_map_chip_material_plan_hash);
        }
    }
    if (door_map_chip_material_plan_required &&
        !door_map_chip_material_plan_consumed) {
        door_map_chip_material_plan_hash = 0u;
    }
    g_dm2_last_asset_floor_ceiling_count =
        viewport.asset_floor_ceiling_drawn_count;
    g_dm2_last_fallback_floor_ceiling_count =
        viewport.fallback_floor_ceiling_drawn_count;
    g_dm2_last_asset_wall_count = viewport.asset_wall_drawn_count;
    g_dm2_last_fallback_wall_count = viewport.fallback_wall_drawn_count;
    g_dm2_last_asset_door_panel_count =
        viewport.asset_door_panel_drawn_count;
    g_dm2_last_asset_door_overlay_count =
        viewport.asset_door_overlay_drawn_count;
    g_dm2_last_asset_door_frame_count =
        viewport.asset_door_frame_drawn_count;
    g_dm2_last_asset_door_button_count =
        viewport.asset_door_button_drawn_count;
    g_dm2_last_fallback_door_count = viewport.fallback_door_drawn_count;
    g_dm2_last_asset_creature_count =
        viewport.asset_creature_drawn_count;
    g_dm2_last_fallback_creature_count =
        viewport.fallback_creature_drawn_count;
    g_dm2_last_asset_item_count =
        viewport.asset_item_drawn_count;
    g_dm2_last_fallback_item_count =
        viewport.fallback_item_drawn_count;
    g_dm2_last_asset_creature_possession_item_count =
        viewport.asset_creature_possession_item_drawn_count;
    g_dm2_last_fallback_creature_possession_item_count =
        viewport.fallback_creature_possession_item_drawn_count;
    g_dm2_last_asset_carried_item_count =
        viewport.asset_carried_item_drawn_count;
    g_dm2_last_fallback_carried_item_count =
        viewport.fallback_carried_item_drawn_count;
    g_dm2_last_asset_projectile_count =
        viewport.asset_projectile_drawn_count;
    g_dm2_last_fallback_projectile_count =
        viewport.fallback_projectile_drawn_count;
    g_dm2_last_asset_hud_portrait_count =
        viewport.asset_hud_portrait_drawn_count;
    g_dm2_last_fallback_hud_portrait_count =
        viewport.fallback_hud_portrait_drawn_count;
    ++g_dm2_frame_ownership.generation;
    g_dm2_frame_ownership.runtime_frame_owned = 1;
    g_dm2_frame_ownership.is_outdoor = viewport.is_outdoor;
    g_dm2_frame_ownership.runtime_weather = viewport.weather;
    g_dm2_frame_ownership.runtime_weather_intensity =
        viewport.rain_intensity;
    g_dm2_frame_ownership.gdat_provider_bound =
        rt->viewport_asset_fetch != NULL;
    g_dm2_frame_ownership.floor_ceiling_gdat_blits =
        viewport.asset_floor_ceiling_drawn_count;
    g_dm2_frame_ownership.floor_ceiling_material_required_mask =
        viewport.last_floor_ceiling_material_required_mask;
    g_dm2_frame_ownership.floor_ceiling_material_consumed_mask =
        viewport.last_floor_ceiling_material_consumed_mask;
    /* skproject DM2_DRAW_DUNGEON resolves both GRAPHICSSET planes before
     * handing the indoor frame to the host. A count of two is insufficient:
     * retain the renderer's exact local-palette transaction as ownership. */
    g_dm2_frame_ownership.floor_ceiling_materials_complete =
        !viewport.source_materials_required ||
        (g_dm2_frame_ownership.is_outdoor
            ? g_dm2_frame_ownership.floor_ceiling_gdat_blits >= 2
            : g_dm2_frame_ownership.floor_ceiling_material_required_mask == 3u &&
              g_dm2_frame_ownership.floor_ceiling_material_consumed_mask == 3u);
    g_dm2_frame_ownership.outdoor_sky_gdat_blits =
        viewport.asset_outdoor_sky_drawn_count;
    g_dm2_frame_ownership.outdoor_ground_gdat_blits =
        viewport.asset_outdoor_ground_drawn_count;
    g_dm2_frame_ownership.wall_gdat_blits =
        viewport.asset_wall_drawn_count;
    g_dm2_frame_ownership.gdat_wall_material_plan_consumed =
        viewport.gdat_wall_material_plan_consumed_count;
    g_dm2_frame_ownership.hud_core_gdat_blits =
        viewport.asset_hud_core_drawn_count;
    g_dm2_frame_ownership.hud_gdat_blits =
        viewport.asset_hud_core_drawn_count +
        viewport.asset_hud_portrait_drawn_count;
    g_dm2_frame_ownership.door_gdat_blits =
        viewport.asset_door_panel_drawn_count +
        viewport.asset_door_overlay_drawn_count +
        viewport.asset_door_frame_drawn_count +
        viewport.asset_door_button_drawn_count;
    g_dm2_frame_ownership.creature_gdat_blits =
        viewport.asset_creature_drawn_count;
    g_dm2_frame_ownership.item_gdat_blits =
        viewport.asset_item_drawn_count +
        viewport.asset_creature_possession_item_drawn_count +
        viewport.asset_carried_item_drawn_count;
    g_dm2_frame_ownership.projectile_gdat_blits =
        viewport.asset_projectile_drawn_count;
    g_dm2_frame_ownership.total_runtime_gdat_blits =
        g_dm2_frame_ownership.floor_ceiling_gdat_blits +
        g_dm2_frame_ownership.wall_gdat_blits +
        g_dm2_frame_ownership.hud_gdat_blits +
        g_dm2_frame_ownership.door_gdat_blits +
        g_dm2_frame_ownership.creature_gdat_blits +
        g_dm2_frame_ownership.item_gdat_blits +
        g_dm2_frame_ownership.projectile_gdat_blits;
    g_dm2_frame_ownership.total_runtime_fallback_draws =
        viewport.fallback_floor_ceiling_drawn_count +
        viewport.fallback_wall_drawn_count +
        viewport.fallback_hud_core_drawn_count +
        viewport.fallback_hud_portrait_drawn_count +
        viewport.fallback_door_drawn_count +
        viewport.fallback_creature_drawn_count +
        viewport.fallback_item_drawn_count +
        viewport.fallback_creature_possession_item_drawn_count +
        viewport.fallback_carried_item_drawn_count +
        viewport.fallback_projectile_drawn_count;
    g_dm2_frame_ownership.blocked_material_draws =
        viewport.blocked_material_draw_count;
    g_dm2_frame_ownership.blocked_material_mask =
        viewport.blocked_material_mask;
    g_dm2_frame_ownership.viewport_raw_gdat_asset_count = 0;
    g_dm2_frame_ownership.viewport_decoded_gdat_asset_count = 0;
    g_dm2_frame_ownership.viewport_raw_gdat_byte_count = 0u;
    g_dm2_frame_ownership.viewport_decoded_gdat_pixel_count = 0u;
    g_dm2_frame_ownership.viewport_raw_gdat_hash = 0x32445652u;
    g_dm2_frame_ownership.viewport_decoded_gdat_hash = 0x32445644u;
    if (viewport.asset_floor_ceiling_drawn_count > 0) {
        dm2_runtime_add_viewport_asset_evidence(&g_dm2_frame_ownership,
            dm2_v1_viewport_scene_material_graphic_index(
                viewport.gdat_scene_material_index,
                DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_FLOOR));
        dm2_runtime_add_viewport_asset_evidence(&g_dm2_frame_ownership,
            dm2_v1_viewport_scene_material_graphic_index(
                viewport.gdat_scene_material_index,
                DM2_V1_VIEWPORT_GFX_SCENE_MATERIAL_CEILING));
    }
    if (viewport.asset_wall_drawn_count > 0) {
        int wall_graphicsset_index = viewport.gdat_scene_control_ready
            ? viewport.gdat_scene_material_index
            : DM2_V1_VIEWPORT_GFX_WALL_DEFAULT_GRAPHICSSET;
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_wall_graphic_index_for_graphicsset(
                wall_graphicsset_index, DM2_SQ_D0L));
    }
    if (viewport.asset_teleporter_drawn_count > 0) {
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_teleporter_map_chip_graphic_index());
    }
    if (viewport.asset_hud_core_drawn_count > 0) {
        /* skproject loads interface GDAT through
         * DM2_LOAD_GDAT_INTERFACE_00_02 before the runtime HUD draw. */
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_TOP_BAR));
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_ACTION_STRIP));
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_GOLD_BOX));
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            dm2_v1_viewport_hud_core_graphic_index(
                DM2_V1_VIEWPORT_GFX_HUD_CORE_PORTRAIT_PANEL));
    }
    if (viewport.asset_hud_portrait_drawn_count > 0) {
        int portrait_count = viewport.asset_hud_portrait_drawn_count;
        if (portrait_count > 4) portrait_count = 4;
        for (int i = 0; i < portrait_count; ++i) {
            dm2_runtime_add_viewport_asset_evidence(
                &g_dm2_frame_ownership,
                dm2_v1_viewport_hud_portrait_graphic_index(i));
        }
    }
    if (viewport.asset_door_panel_drawn_count > 0 &&
        g_dm2_last_door_render.panel_gdat_index != 0) {
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_door_render.panel_gdat_index);
    }
    if (g_dm2_last_door_render.ornate_asset_drawn &&
        g_dm2_last_door_render.ornate_gdat_index != 0) {
        /* skproject DRAW_DOOR renders the Door::OrnateIndex() overlay from
         * GDAT_CATEGORY_DOOR_GFX after the decoded base panel. */
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_door_render.ornate_gdat_index);
    }
    if (g_dm2_last_door_render.destroyed_mask_asset_drawn &&
        g_dm2_last_door_render.destroyed_mask_gdat_index != 0) {
        /* skproject DRAW_DOOR overlays the destroyed-door mask only after
         * the matching GDAT image has reached the viewport blitter. */
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_door_render.destroyed_mask_gdat_index);
    }
    if (viewport.asset_creature_drawn_count > 0 &&
        g_dm2_last_creature_render.gdat_index != 0) {
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_creature_render.gdat_index);
    }
    if ((viewport.asset_item_drawn_count > 0 ||
         viewport.asset_creature_possession_item_drawn_count > 0 ||
         viewport.asset_carried_item_drawn_count > 0) &&
        g_dm2_last_item_render.valid &&
        g_dm2_last_item_render.asset_blit_ready &&
        g_dm2_last_item_render.gdat_index != 0) {
        /* skproject DRAW_MAP_CHIP and DRAW_ITEM_IN_HAND both fetch visible
         * object graphics from the object's GDAT category/type before
         * drawing; count that raw/decoded object evidence in the frame
         * ownership receipt instead of only counting the blit. */
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_item_render.gdat_index);
    }
    if (viewport.asset_projectile_drawn_count > 0 &&
        g_dm2_last_projectile_render.valid &&
        g_dm2_last_projectile_render.asset_blit_ready &&
        g_dm2_last_projectile_render.gdat_index != 0) {
        dm2_runtime_add_viewport_asset_evidence(
            &g_dm2_frame_ownership,
            g_dm2_last_projectile_render.gdat_index);
    }
    g_dm2_frame_ownership.real_gdat_evidence_valid =
        rt->viewport_asset_fetch == dm2_v1_boot_viewport_asset_fetch &&
        g_dm2_frame_ownership.viewport_raw_gdat_asset_count >= 5 &&
        g_dm2_frame_ownership.viewport_decoded_gdat_asset_count >= 5 &&
        g_dm2_frame_ownership.viewport_raw_gdat_byte_count > 0u &&
        g_dm2_frame_ownership.viewport_decoded_gdat_pixel_count > 0u;
    g_dm2_frame_ownership.gdat_scene_control_ready =
        rt->gdat_scene_control_ready;
    g_dm2_frame_ownership.gdat_scene_control_consumed =
        viewport.gdat_scene_control_consumed_count;
    g_dm2_frame_ownership.gdat_scene_control_hash =
        rt->gdat_scene_control_hash;
    g_dm2_frame_ownership.gdat_scene_control_present_mask =
        rt->gdat_scene_control_present_mask;
    g_dm2_frame_ownership.gdat_scene_colorkey = rt->gdat_scene_colorkey;
    g_dm2_frame_ownership.gdat_scene_flags = rt->gdat_scene_flags;
    g_dm2_frame_ownership.gdat_scene_material_index =
        viewport.gdat_scene_material_index;
    g_dm2_frame_ownership.gdat_scene_material_consumed =
        viewport.gdat_scene_material_consumed_count;
    g_dm2_frame_ownership.gdat_scene_ambient_light =
        rt->gdat_scene_ambient_light;
    g_dm2_frame_ownership.gdat_scene_highest_light_level =
        rt->gdat_scene_highest_light_level;
    g_dm2_frame_ownership.gdat_scene_void_random_fall =
        rt->gdat_scene_void_random_fall;
    g_dm2_frame_ownership.gdat_scene_animated_floor =
        rt->gdat_scene_animated_floor;
    g_dm2_frame_ownership.gdat_scene_rain = rt->gdat_scene_rain;
    g_dm2_frame_ownership.gdat_misty_map = rt->gdat_misty_map;
    g_dm2_frame_ownership.gdat_thunder_position =
        rt->gdat_thunder_position;
    g_dm2_frame_ownership.gdat_ambient_darkness =
        rt->gdat_ambient_darkness;
    g_dm2_frame_ownership.gdat_weather_receipt_ready =
        rt->gdat_weather_receipt_ready;
    g_dm2_frame_ownership.gdat_weather_receipt_hash =
        rt->gdat_weather_receipt_hash;
    g_dm2_frame_ownership.gdat_weather_material_mask =
        rt->gdat_weather_material_mask;
    g_dm2_frame_ownership.gdat_weather_destination_ready =
        rt->gdat_weather_destination_ready;
    g_dm2_frame_ownership.gdat_weather_destination_hash =
        rt->gdat_weather_destination_hash;
    g_dm2_frame_ownership.gdat_weather_destination_mask =
        rt->gdat_weather_destination_mask;
    g_dm2_frame_ownership.gdat_dialogue_shell_receipt_ready =
        rt->gdat_dialogue_shell_receipt_ready;
    g_dm2_frame_ownership.gdat_dialogue_shell_receipt_hash =
        rt->gdat_dialogue_shell_receipt_hash;
    g_dm2_frame_ownership.gdat_scene_light_consumed =
        viewport.gdat_scene_light_consumed_count;
    g_dm2_frame_ownership.gdat_scene_weather_consumed =
        viewport.gdat_scene_weather_consumed_count;
    g_dm2_frame_ownership.gdat_sprite_palette_consumed =
        viewport.gdat_sprite_palette_consumed_count;
    g_dm2_frame_ownership.gdat_local_palette_consumed =
        viewport.gdat_local_palette_consumed_count;
    g_dm2_frame_ownership.gdat_interface_palette_ready =
        rt->gdat_interface_palette_ready;
    g_dm2_frame_ownership.gdat_interface_palette_consumed =
        viewport.gdat_interface_palette_consumed_count;
    g_dm2_frame_ownership.gdat_interface_action_palette_ready =
        rt->gdat_interface_action_palette_ready;
    g_dm2_frame_ownership.gdat_interface_action_palette_consumed =
        viewport.gdat_interface_action_palette_consumed_count;
    g_dm2_frame_ownership.gdat_interface_action_palette_hash =
        rt->gdat_interface_action_palette_hash;
    g_dm2_frame_ownership.gdat_interface_action_palette_darkness =
        rt->gdat_interface_action_palette_darkness;
    g_dm2_frame_ownership.gdat_interface_font_host_ready =
        font_rows != NULL && font_hash != 0u;
    g_dm2_frame_ownership.gdat_interface_font_consumed =
        viewport.gdat_interface_font_consumed_count;
    g_dm2_frame_ownership.gdat_interface_font_hash = font_hash;
    g_dm2_frame_ownership.gdat_interface_hud_layout_ready = hud_layout.valid;
    g_dm2_frame_ownership.gdat_interface_hud_layout_hash = hud_layout.table_hash;
    g_dm2_frame_ownership.gdat_interface_rect14_host_ready =
        rect14_host.valid;
    g_dm2_frame_ownership.gdat_interface_rect14_consumed =
        viewport.gdat_interface_rect14_consumed_count;
    g_dm2_frame_ownership.gdat_interface_rect14_table_hash =
        rect14_host.table_hash;
    g_dm2_frame_ownership.gdat_interface_rect14_placement_hash =
        rect14_host.placement_hash;
    g_dm2_frame_ownership.gdat_interface_rect14_placement_count =
        rect14_host.placement_count;
    g_dm2_frame_ownership.gdat_save_dialogue_material_bound =
        save_dialogue_command.draw.valid;
    g_dm2_frame_ownership.gdat_save_dialogue_host_command_ready =
        save_dialogue_command.valid;
    g_dm2_frame_ownership.gdat_save_dialogue_open_panel_ready =
        save_dialogue_open_panel.valid;
    g_dm2_frame_ownership.gdat_save_dialogue_material_hash =
        save_dialogue_command.draw.valid
            ? save_dialogue_command.draw.plan_hash : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_host_command_hash =
        save_dialogue_command.valid ? save_dialogue_command.command_hash : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_open_panel_hash =
        save_dialogue_open_panel.valid
            ? save_dialogue_open_panel.command_hash : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_rect_index =
        save_dialogue_command.valid
            ? save_dialogue_command.draw.expanded_rect_index : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_open_panel_rect_index =
        save_dialogue_open_panel.valid
            ? save_dialogue_open_panel.draw.panel_rect_index : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_open_panel_save_list_rect_index =
        save_dialogue_open_panel.valid
            ? save_dialogue_open_panel.draw.save_list_rect_index : 0u;
    g_dm2_frame_ownership.gdat_save_dialogue_x =
        save_dialogue_command.valid ? save_dialogue_command.rect.x : 0;
    g_dm2_frame_ownership.gdat_save_dialogue_y =
        save_dialogue_command.valid ? save_dialogue_command.rect.y : 0;
    g_dm2_frame_ownership.gdat_save_dialogue_w =
        save_dialogue_command.valid ? save_dialogue_command.rect.w : 0;
    g_dm2_frame_ownership.gdat_save_dialogue_h =
        save_dialogue_command.valid ? save_dialogue_command.rect.h : 0;
    g_dm2_frame_ownership.gdat_material_palette_floor_ceiling_consumed =
        viewport.gdat_material_palette_floor_ceiling_consumed_count;
    g_dm2_frame_ownership.gdat_material_palette_wall_consumed =
        viewport.gdat_material_palette_wall_consumed_count;
    g_dm2_frame_ownership.gdat_material_palette_door_frame_consumed =
        viewport.gdat_material_palette_door_frame_consumed_count;
    g_dm2_frame_ownership.gdat_interface_palette_hash =
        rt->gdat_interface_palette_hash;
    memcpy(g_dm2_frame_ownership.gdat_interface_palette16,
           rt->gdat_interface_palette16,
           sizeof(g_dm2_frame_ownership.gdat_interface_palette16));
    /* skproject SKWIN/SkWinCore.cpp routes the runtime HUD, floor/ceiling,
     * walls and overlays through GDAT-backed surface fetches before blitting.
     * A "full" DM2 runtime frame is only accepted when the mandatory HUD and
     * dungeon base layers are GDAT-backed and no visible runtime element fell
     * back to Firestaff's bounded placeholder paths. */
    g_dm2_frame_ownership.outdoor_gdat_frame_valid =
        g_dm2_frame_ownership.is_outdoor &&
        g_dm2_frame_ownership.gdat_provider_bound &&
        g_dm2_frame_ownership.outdoor_sky_gdat_blits > 0 &&
        g_dm2_frame_ownership.outdoor_ground_gdat_blits > 0 &&
        g_dm2_frame_ownership.hud_gdat_blits > 0 &&
        g_dm2_frame_ownership.total_runtime_fallback_draws == 0;
    g_dm2_frame_ownership.full_gdat_frame_valid =
        g_dm2_frame_ownership.gdat_provider_bound &&
        g_dm2_frame_ownership.floor_ceiling_gdat_blits >= 2 &&
        g_dm2_frame_ownership.floor_ceiling_materials_complete &&
        (g_dm2_frame_ownership.is_outdoor ||
         g_dm2_frame_ownership.wall_gdat_blits > 0) &&
        (!g_dm2_frame_ownership.real_gdat_evidence_valid ||
         (g_dm2_frame_ownership.gdat_scene_control_ready &&
          g_dm2_frame_ownership.gdat_scene_control_consumed > 0 &&
          g_dm2_frame_ownership.gdat_scene_control_hash != 0u &&
          g_dm2_frame_ownership.gdat_interface_palette_ready &&
          g_dm2_frame_ownership.gdat_interface_palette_consumed > 0 &&
          g_dm2_frame_ownership.gdat_local_palette_consumed > 0 &&
          g_dm2_frame_ownership.gdat_material_palette_floor_ceiling_consumed > 0 &&
          g_dm2_frame_ownership.gdat_material_palette_wall_consumed > 0 &&
          (viewport.asset_door_frame_drawn_count == 0 ||
           g_dm2_frame_ownership.gdat_material_palette_door_frame_consumed > 0) &&
          g_dm2_frame_ownership.gdat_interface_palette_hash != 0u)) &&
        /* skproject SKWIN uses raw INTERFACE_GENERAL tables for the HUD
         * chrome/layout and CHAMPIONS images for the visible portrait panel.
         * Do not require Firestaff's primitive rect fills to be separate
         * GDAT image blits; the frame is owned when the visible HUD imagery
         * and dungeon layers carry real GDAT evidence and no world element
         * falls back. */
        g_dm2_frame_ownership.hud_gdat_blits > 0 &&
        g_dm2_frame_ownership.total_runtime_gdat_blits > 0 &&
        g_dm2_frame_ownership.total_runtime_fallback_draws == 0 &&
        g_dm2_frame_ownership.blocked_material_draws == 0 &&
        (!g_dm2_frame_ownership.is_outdoor ||
         g_dm2_frame_ownership.outdoor_gdat_frame_valid);
    g_dm2_frame_ownership.valid =
        g_dm2_frame_ownership.runtime_frame_owned &&
        g_dm2_frame_ownership.full_gdat_frame_valid;
    memset(&g_dm2_last_m11_frame, 0, sizeof(g_dm2_last_m11_frame));
    g_dm2_last_m11_frame.source_materials_required =
        viewport.source_materials_required ? 1 : 0;
    g_dm2_last_m11_frame.map_load_token =
        (uint32_t)(rt->dungeon_level + 1) |
        (rt->outdoor ? UINT32_C(0x80000000) : 0u);
    g_dm2_last_m11_frame.scene_control_hash =
        g_dm2_frame_ownership.gdat_scene_control_hash;
    g_dm2_last_m11_frame.scene_light_hash =
        rt->gdat_scene_light_receipt.valid
            ? rt->gdat_scene_light_receipt.receipt_hash : 0u;
    /* UPDATE_GFXSET owns these exact GRAPHICSSET IMG3 records; retain their
     * individual identities so M11 cannot combine a current control receipt
     * with floor, ceiling, or WALL_GFX pixels from another plan. */
    g_dm2_last_m11_frame.floor_material_hash =
        rt->gdat_scene_material_plan.valid
            ? rt->gdat_scene_material_plan.commands[0].raw_hash : 0u;
    g_dm2_last_m11_frame.ceiling_material_hash =
        rt->gdat_scene_material_plan.valid
            ? rt->gdat_scene_material_plan.commands[1].raw_hash : 0u;
    g_dm2_last_m11_frame.wall_material_plan_hash =
        rt->gdat_wall_material_plan.valid
            ? rt->gdat_wall_material_plan.command_hash : 0u;
    /* skproject DM2_DRAW_DOOR/DRAW_DOOR_FRAMES resolve a multi-category
     * material plan before the viewport blits it.  Carry that exact plan to
     * M11 only when the presented frame actually used a door material; a
     * doorless dungeon frame has no original door identity to invent. */
    g_dm2_last_m11_frame.door_material_plan_required =
        g_dm2_frame_ownership.door_gdat_blits > 0;
    g_dm2_last_m11_frame.door_material_plan_consumed =
        g_dm2_last_m11_frame.door_material_plan_required &&
        rt->gdat_door_material_plan.valid &&
        rt->gdat_door_material_plan.command_hash != 0u &&
        viewport.gdat_door_overlay_material_plan_consumed_count > 0;
    g_dm2_last_m11_frame.door_material_plan_hash =
        g_dm2_last_m11_frame.door_material_plan_consumed
            ? rt->gdat_door_material_plan.command_hash : 0u;
    g_dm2_last_m11_frame.hud_material_plan_required =
        hud_material_plan_required;
    g_dm2_last_m11_frame.hud_material_plan_hash = hud_material_plan_hash;
    g_dm2_last_m11_frame.hud_material_plan_consumed =
        hud_material_plan_consumed;
    g_dm2_last_m11_frame.creature_material_plan_required =
        creature_material_plan_required;
    g_dm2_last_m11_frame.creature_material_plan_hash =
        creature_material_plan_hash;
    g_dm2_last_m11_frame.creature_material_plan_consumed =
        creature_material_plan_consumed;
    g_dm2_last_m11_frame.teleporter_material_plan_required =
        teleporter_material_plan_required;
    g_dm2_last_m11_frame.teleporter_material_plan_hash =
        teleporter_material_plan_hash;
    g_dm2_last_m11_frame.teleporter_material_plan_consumed =
        teleporter_material_plan_consumed;
    g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_required =
        floor_gfx_map_chip_material_plan_required;
    g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_hash =
        floor_gfx_map_chip_material_plan_hash;
    g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_consumed =
        floor_gfx_map_chip_material_plan_consumed;
    g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_required =
        wall_gfx_map_chip_material_plan_required;
    g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_hash =
        wall_gfx_map_chip_material_plan_hash;
    g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_consumed =
        wall_gfx_map_chip_material_plan_consumed;
    g_dm2_last_m11_frame.door_map_chip_material_plan_required =
        door_map_chip_material_plan_required;
    g_dm2_last_m11_frame.door_map_chip_material_plan_hash =
        door_map_chip_material_plan_hash;
    g_dm2_last_m11_frame.door_map_chip_material_plan_consumed =
        door_map_chip_material_plan_consumed;
    g_dm2_last_m11_frame.palette_hash =
        g_dm2_frame_ownership.gdat_interface_palette_hash;
    g_dm2_last_m11_frame.interface_action_palette_hash =
        g_dm2_frame_ownership.gdat_interface_action_palette_hash;
    g_dm2_last_m11_frame.interface_action_palette_consumed =
        g_dm2_frame_ownership.gdat_interface_action_palette_consumed > 0;
    g_dm2_last_m11_frame.floor_ceiling_material_required_mask =
        g_dm2_frame_ownership.floor_ceiling_material_required_mask;
    g_dm2_last_m11_frame.floor_ceiling_material_consumed_mask =
        g_dm2_frame_ownership.floor_ceiling_material_consumed_mask;
    g_dm2_last_m11_frame.floor_ceiling_materials_complete =
        g_dm2_frame_ownership.floor_ceiling_materials_complete;
    g_dm2_last_m11_frame.gdat_wall_material_plan_consumed =
        g_dm2_frame_ownership.gdat_wall_material_plan_consumed;
    g_dm2_last_m11_frame.valid =
        g_dm2_frame_ownership.valid &&
        g_dm2_last_m11_frame.source_materials_required &&
        g_dm2_last_m11_frame.floor_ceiling_materials_complete &&
        g_dm2_last_m11_frame.map_load_token != 0u &&
        g_dm2_last_m11_frame.scene_control_hash != 0u &&
        g_dm2_last_m11_frame.scene_light_hash != 0u &&
        g_dm2_last_m11_frame.floor_material_hash != 0u &&
        g_dm2_last_m11_frame.ceiling_material_hash != 0u &&
        (rt->outdoor || g_dm2_last_m11_frame.wall_material_plan_hash != 0u) &&
        (!g_dm2_last_m11_frame.door_material_plan_required ||
         (g_dm2_last_m11_frame.door_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.door_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.hud_material_plan_required ||
         (g_dm2_last_m11_frame.hud_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.hud_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.creature_material_plan_required ||
         (g_dm2_last_m11_frame.creature_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.creature_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.teleporter_material_plan_required ||
         (g_dm2_last_m11_frame.teleporter_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.teleporter_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_required ||
         (g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.floor_gfx_map_chip_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_required ||
         (g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.wall_gfx_map_chip_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.door_map_chip_material_plan_required ||
         (g_dm2_last_m11_frame.door_map_chip_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.door_map_chip_material_plan_consumed)) &&
        g_dm2_last_m11_frame.palette_hash != 0u &&
        (!g_dm2_frame_ownership.real_gdat_evidence_valid ||
         (g_dm2_last_m11_frame.interface_action_palette_hash != 0u &&
          g_dm2_last_m11_frame.interface_action_palette_consumed));
    g_dm2_last_m11_frame.m11_consume_frame =
        g_dm2_last_m11_frame.valid;
    rt->weather.weather_seed = viewport.random_seed;

    return 0;
}

void dm2_v1_runtime_note_startup_frame_consumption(
    int title_gdat_blits, int menu_gdat_blits)
{
    g_dm2_frame_ownership.startup_title_gdat_blits =
        title_gdat_blits > 0 ? title_gdat_blits : 0;
    g_dm2_frame_ownership.startup_menu_gdat_blits =
        menu_gdat_blits > 0 ? menu_gdat_blits : 0;
}

int dm2_v1_runtime_last_frame_ownership(
    DM2_V1_RuntimeFrameOwnershipReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    *out_receipt = g_dm2_frame_ownership;
    return out_receipt->valid;
}

int dm2_v1_runtime_last_m11_frame_receipt(
    DM2_V1_ViewportM11FrameReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_last_m11_frame;
    return out_receipt->valid;
}

int dm2_v1_runtime_graphicsset_scene_receipt(
    DM2_V1_RuntimeGraphicsSetSceneReceipt *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->ready = g_dm2_runtime.gdat_scene_control_ready;
    out_receipt->map_graphics_style = g_dm2_runtime.map_graphics_style;
    out_receipt->scene_material_index = g_dm2_runtime.map_graphics_style;
    out_receipt->hash = g_dm2_runtime.gdat_scene_control_hash;
    out_receipt->present_mask = g_dm2_runtime.gdat_scene_control_present_mask;
    out_receipt->query_count = g_dm2_runtime.gdat_scene_control_query_count;
    out_receipt->scene_colorkey = g_dm2_runtime.gdat_scene_colorkey;
    out_receipt->scene_flags = g_dm2_runtime.gdat_scene_flags;
    out_receipt->ambient_light = g_dm2_runtime.gdat_scene_ambient_light;
    out_receipt->highest_light_level =
        g_dm2_runtime.gdat_scene_highest_light_level;
    out_receipt->void_random_fall = g_dm2_runtime.gdat_scene_void_random_fall;
    out_receipt->animated_floor = g_dm2_runtime.gdat_scene_animated_floor;
    out_receipt->scene_rain = g_dm2_runtime.gdat_scene_rain;
    out_receipt->misty_map = g_dm2_runtime.gdat_misty_map;
    out_receipt->thunder_position = g_dm2_runtime.gdat_thunder_position;
    out_receipt->ambient_darkness = g_dm2_runtime.gdat_ambient_darkness;
    out_receipt->interface_palette_ready =
        g_dm2_runtime.gdat_interface_palette_ready;
    out_receipt->interface_palette_hash =
        g_dm2_runtime.gdat_interface_palette_hash;
    memcpy(out_receipt->interface_palette16,
           g_dm2_runtime.gdat_interface_palette16,
           sizeof(out_receipt->interface_palette16));
    return out_receipt->ready;
}

void dm2_v1_runtime_set_viewport_asset_provider(
    DM2_V1_ViewportAssetFetch fetch,
    void *user) {
    g_dm2_runtime.viewport_asset_fetch = fetch;
    g_dm2_runtime.viewport_asset_user = user;
    if (fetch != dm2_v1_boot_viewport_asset_fetch) {
        g_dm2_runtime.viewport_asset_palette_fetch = NULL;
        g_dm2_runtime.viewport_asset_palette_user = NULL;
    }
    dm2_runtime_refresh_gdat_scene_control(&g_dm2_runtime);
}

int dm2_v1_runtime_set_map_wall_gfx_list(const uint8_t *wall_gfx_list,
                                         int wall_gfx_count) {
    if (!wall_gfx_list || wall_gfx_count < 0 ||
        wall_gfx_count > (int)sizeof(g_dm2_runtime.map_wall_gfx_list)) {
        memset(g_dm2_runtime.map_wall_gfx_list, 0,
               sizeof(g_dm2_runtime.map_wall_gfx_list));
        g_dm2_runtime.map_wall_gfx_count = 0;
        return wall_gfx_count == 0 ? 0 : -1;
    }
    memcpy(g_dm2_runtime.map_wall_gfx_list, wall_gfx_list,
           (size_t)wall_gfx_count);
    g_dm2_runtime.map_wall_gfx_count = wall_gfx_count;
    return 0;
}

int dm2_v1_runtime_last_asset_floor_ceiling_count(void) {
    return g_dm2_last_asset_floor_ceiling_count;
}

int dm2_v1_runtime_last_fallback_floor_ceiling_count(void) {
    return g_dm2_last_fallback_floor_ceiling_count;
}

int dm2_v1_runtime_last_asset_wall_count(void) {
    return g_dm2_last_asset_wall_count;
}

int dm2_v1_runtime_last_fallback_wall_count(void) {
    return g_dm2_last_fallback_wall_count;
}

int dm2_v1_runtime_last_asset_door_panel_count(void) {
    return g_dm2_last_asset_door_panel_count;
}

int dm2_v1_runtime_last_asset_door_overlay_count(void) {
    return g_dm2_last_asset_door_overlay_count;
}

int dm2_v1_runtime_last_asset_door_frame_count(void) {
    return g_dm2_last_asset_door_frame_count;
}

int dm2_v1_runtime_last_asset_door_button_count(void) {
    return g_dm2_last_asset_door_button_count;
}

int dm2_v1_runtime_last_fallback_door_count(void) {
    return g_dm2_last_fallback_door_count;
}

int dm2_v1_runtime_last_door_render_receipt(
    DM2_V1_RuntimeDoorRenderReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_door_render.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_door_render;
    return 1;
}

int dm2_v1_runtime_last_asset_carried_item_count(void) {
    return g_dm2_last_asset_carried_item_count;
}

int dm2_v1_runtime_last_fallback_carried_item_count(void) {
    return g_dm2_last_fallback_carried_item_count;
}

int dm2_v1_runtime_last_asset_item_count(void) {
    return g_dm2_last_asset_item_count;
}

int dm2_v1_runtime_last_fallback_item_count(void) {
    return g_dm2_last_fallback_item_count;
}

int dm2_v1_runtime_last_item_render_receipt(
    DM2_V1_RuntimeItemRenderReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_item_render.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_item_render;
    return 1;
}

int dm2_v1_runtime_last_creature_render_receipt(
    DM2_V1_RuntimeCreatureRenderReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_creature_render.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_creature_render;
    return 1;
}

int dm2_v1_runtime_last_asset_creature_count(void) {
    return g_dm2_last_asset_creature_count;
}

int dm2_v1_runtime_last_fallback_creature_count(void) {
    return g_dm2_last_fallback_creature_count;
}

int dm2_v1_runtime_last_asset_creature_possession_item_count(void) {
    return g_dm2_last_asset_creature_possession_item_count;
}

int dm2_v1_runtime_last_fallback_creature_possession_item_count(void) {
    return g_dm2_last_fallback_creature_possession_item_count;
}

int dm2_v1_runtime_last_asset_projectile_count(void) {
    return g_dm2_last_asset_projectile_count;
}

int dm2_v1_runtime_last_fallback_projectile_count(void) {
    return g_dm2_last_fallback_projectile_count;
}

int dm2_v1_runtime_last_projectile_render_receipt(
    DM2_V1_RuntimeProjectileRenderReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_projectile_render.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_projectile_render;
    return 1;
}

int dm2_v1_runtime_last_asset_hud_portrait_count(void) {
    return g_dm2_last_asset_hud_portrait_count;
}

int dm2_v1_runtime_last_fallback_hud_portrait_count(void) {
    return g_dm2_last_fallback_hud_portrait_count;
}

/* ── Movement ──────────────────────────────────────────────────────── */

/*
 * dm2_v1_runtime_can_move — check if party can move this tick.
 *
 * DM2 movement: outdoor is 2x dungeon speed.
 * move_cooldown_ticks gates movement so you can't move every tick.
 *
 * Source: SKULL.ASM T520 — movement speed
 */
int dm2_v1_runtime_can_move(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    return (rt->move_cooldown_ticks == 0);
}

/*
 * dm2_v1_runtime_move — attempt party movement in direction dir.
 * Returns 0 on success, -1 if blocked.
 *
 * Source: SKULL.ASM T520
 */
int dm2_v1_runtime_move(int dir) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int dx[] = {0, 1, 0, -1};  /* N E S W */
    int dy[] = {-1, 0, 1, 0};
    int nx, ny;
    int blocked = 0;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    if (!dm2_v1_runtime_can_move()) return -1;

    /* Save pre-move position for smooth animation trigger */
    int old_x = gs->party_x;
    int old_y = gs->party_y;
    int old_dir = gs->party_dir;

    /* Detect turn-only (facing change, no movement) */
    int is_turn_only = (dir != old_dir);
    (void)is_turn_only;  /* turn-only detection reserved for future smooth-move path */

    nx = gs->party_x + dx[dir & 3];
    ny = gs->party_y + dy[dir & 3];

    /* Check dungeon collision if in dungeon mode.
     * Tile type is in lower 5 bits (0x1F) of raw tile.
     * Door cells are accepted through dm2_runtime_raw_is_door_square();
     * door state is in lower 3 bits (0x07):
     *   state 0 = open (passable), state 4 = closed (impassable).
     * Other non-walkable: type 0 (wall), type 5 (pit), 11 (lava), 13 (inaccessible).
     * Source: SKULL.ASM T520 — movement collision and door state check.
     *         dm2_special_squares.md — door tile type and state encoding. */
    if (!rt->outdoor && rt->boot->dungeon_data) {
        DM2_V1_DungeonData *dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
        int raw = dm2_v1_dungeon_get_tile_raw(dd, rt->dungeon_level, nx, ny);
        if (raw < 0) {
            blocked = 1;
        } else {
            int tile_type = dm2_runtime_square_type_at(
                dd, rt->dungeon_level, nx, ny, raw);
            /* Impassable tile types: wall (0), pit (5), lava (11), inaccessible (13) */
            if (tile_type == 0 || tile_type == 5 || tile_type == 11 || tile_type == 13) {
                blocked = 1;
            } else if (dm2_runtime_is_door_at(
                           dd, rt->dungeon_level, nx, ny, raw)) {
                /* Door tile: door state in lower 3 bits.
                 * DM2_DOOR_STATE_OPEN=0 (passable), DM2_DOOR_STATE_CLOSED=4 (impassable).
                 * Source: dm2_v1_object_model.h DM2_DoorState enum.
                 *         SKULL.ASM T520 movement tile access. */
                int door_state = raw & 0x0007;
                if (door_state != 0) {  /* not open */
                    blocked = 1;
                }
            }
            /* All other tile types (1=floor, 3=floor_ornate,
             * 8=teleporter, 10=water, etc.) are passable. */
        }
    }

    if (!blocked) {
        /* Fire smooth movement callback before updating state.
         * This gives the V2 layer the from/to positions for interpolation.
         * Source: Phase 5 runtime binding */
        if (rt->move_callback) {
            rt->move_callback(old_x, old_y, nx, ny);
        }
        /* SKProject DRAW_DUNGEON_GRAPHIC moves only the floor/ceiling planes
         * while glbIsPlayerMoving is active. Arm the next renderer-owned V1
         * frame after an accepted step; blocked moves do not enter it. */
        rt->scene_movement_pending = 1;
        gs->party_x = nx;
        gs->party_y = ny;
        for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
            DM2_V1_TriggerEvent event;
            const DM2_V1_Trigger *trigger =
                dm2_v1_trigger_get_builtin(i);
            if (trigger &&
                trigger->kind == DM2_TRIGGER_KIND_SQUARE_ENTERED &&
                trigger->arg_map_x == nx &&
                trigger->arg_map_y == ny &&
                trigger->arg_map_level == rt->dungeon_level &&
                dm2_v1_trigger_fire(trigger->trigger_id) ==
                    (int)DM2_TRIGGER_RESULT_OK &&
                dm2_v1_trigger_copy_last_event(&event)) {
                dm2_runtime_apply_trigger_event(rt, &event);
            }
        }
        dm2_v1_plate_set_party_position(nx, ny, rt->dungeon_level);
        for (int i = 1; i <= dm2_v1_plate_get_builtin_count(); ++i) {
            DM2_V1_PlateEvent event;
            const DM2_V1_PressurePlate *plate =
                dm2_v1_plate_get_builtin(i);
            if (plate && plate->map_x == nx && plate->map_y == ny &&
                plate->map_level == rt->dungeon_level &&
                dm2_v1_plate_check(i, rt->tick_count) ==
                    (int)DM2_PLATE_RESULT_OK &&
                dm2_v1_plate_copy_last_event(&event)) {
                dm2_runtime_apply_plate_event(rt, &event);
            }
        }
        (void)dm2_v1_runtime_invoke_square_actuators(
            rt->dungeon_level, nx, ny);
        dm2_runtime_refresh_g1_map0_teleporter_transition(
            rt, rt->dungeon_level, gs->party_x, gs->party_y);
    }

    /* Fire smooth turn callback when facing changes.
     * Turn triggers even on blocked moves (party still turns).
     * Source: Phase 5 runtime binding */
    if (dir != old_dir && rt->turn_callback) {
        rt->turn_callback(old_dir, dir);
    }

    if (!rt->g1_map0_teleporter_transition.transition_applied) {
        gs->party_dir = dir;
        rt->view_dir = dir;
    }

    /* Set movement cooldown: dungeon=1 tick, outdoor=0.5 tick */
    rt->move_cooldown_ticks = rt->outdoor ? 0 : 1;

    return blocked ? -1 : 0;
}

/*
 * dm2_v1_runtime_turn — rotate the party in place without attempting a
 * movement step. M11 keyboard input maps Q/E/Home/End to this boundary so
 * DM2 does not treat turning as a sidestep through dm2_v1_runtime_move().
 *
 * Source: SKULL.ASM T048 dispatches separate turn commands before the
 * T520 party-position update. The existing V2 turn callback is kept as the
 * single animation hand-off, matching the callback used by move().
 */
int dm2_v1_runtime_turn(int delta) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int old_dir;
    int new_dir;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    old_dir = gs->party_dir & 3;
    new_dir = (old_dir + (delta < 0 ? 3 : 1)) & 3;

    if (new_dir != old_dir && rt->turn_callback) {
        rt->turn_callback(old_dir, new_dir);
    }
    gs->party_dir = new_dir;
    rt->view_dir = new_dir;
    return 0;
}

/* ── Outdoor/Dungeon mode ─────────────────────────────────────────── */

/*
 * dm2_v1_runtime_set_outdoor — switch between outdoor and dungeon view.
 *
 * Source: SKULL.ASM T600 — outdoor mode entry
 *         SKULL.ASM T560 — dungeon mode entry
 */
void dm2_v1_runtime_set_outdoor(int is_outdoor) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    g_dm2_runtime.outdoor = is_outdoor ? 1 : 0;
    if (rt->boot && rt->boot->dm2_state) {
        DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
        gs->outdoor = g_dm2_runtime.outdoor;
    }
}

void dm2_v1_runtime_set_position(int level, int x, int y, int dir) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;

    if (!rt->boot || !rt->boot->dm2_state) return;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    gs->current_level = level;
    gs->party_x = x;
    gs->party_y = y;
    gs->party_dir = dir & 3;
    rt->dungeon_level = level;
    rt->view_dir = gs->party_dir;
    dm2_runtime_refresh_g1_map0_teleporter_transition(rt, level, x, y);
    (void)dm2_v1_boot_g1_actuator_wall_gfx_materials(
        rt->boot, level, &rt->g1_actuator_wall_gfx_runtime);
    (void)dm2_v1_boot_g1_creature_map_chip_materials(
        rt->boot, level, &rt->g1_creature_map_chip_runtime);
    dm2_runtime_refresh_map_wall_gfx_list(rt);
    dm2_runtime_refresh_gdat_scene_control(rt);
}

/* ── Party position accessors ─────────────────────────────────────── */

/* dm2_v1_runtime_get_party_x / _y / _dir — read V1-snapped party state.
 * Returns the instant (non-interpolated) V1 game state.
 *
 * Source: SKULL.ASM T520 — party position fields
 */
int dm2_v1_runtime_get_party_x(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_x;
}

int dm2_v1_runtime_get_party_y(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_y;
}

int dm2_v1_runtime_get_party_dir(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->boot || !rt->boot->dm2_state) return 0;
    DM2_V1_GameState *gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    return gs->party_dir;
}

int dm2_v1_runtime_get_weather(void) {
    return g_dm2_runtime.weather.weather;
}

int dm2_v1_runtime_get_weather_intensity(void) {
    return g_dm2_runtime.weather.weather_intensity;
}

int dm2_v1_runtime_import_sksave_receipted_candidate(
    const DM2_SKSaveCandidateReceipt *selected,
    DM2_V1_RuntimeCorpusImportReceipt *out)
{
    uint8_t payload[DM2_SESSION_MAX_SIZE];
    size_t size = 0u;
    DM2_V1_SaveCandidate candidate;
    DM2_V1_SessionState session;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!selected || !selected->path[0] || !selected->source_file_hash) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE;
        return 0;
    }
    /* ReDMCSB's load UI chooses a row before DM2_GAME_LOAD opens its stream.
     * skproject c_savegame.cpp::DM2_SELECT_LOAD_GAME and DM2_GAME_LOAD
     * follow that order. Do not scan for or replace this selected receipt. */
    out->candidate_kind = selected->kind;
    out->selected_payload_size = selected->payload_size;
    out->selected_payload_hash = selected->payload_hash;
    out->selected_source_file_hash = selected->source_file_hash;
    snprintf(out->selected_path, sizeof(out->selected_path), "%s", selected->path);
    if (!dm2_v1_sksave_corpus_load_receipted_candidate(selected, payload,
            sizeof(payload), &size) ||
        dm2_v1_session_parse_save_candidate(&candidate, payload, size) != 0) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    out->candidate_kind = candidate.kind;
    if (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE ||
        candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        /* skproject/SKULLWIN/c_savegame.cpp: original saves restore only
         * through the active dungeon/session binding.  The corpus receipt
         * authenticates the file bytes; restore_save_candidate owns the
         * fallible dungeon compatibility check and atomic state handoff. */
        if (dm2_v1_runtime_restore_save_candidate(payload, size) != 0) {
            out->rejected_original_candidate = 1;
            out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
            return 0;
        }
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_OK;
        out->restored = 1;
        return 1;
    }
    if (candidate.kind != DM2_V1_SAVE_CANDIDATE_FIRESTAFF_SESSION ||
        dm2_v1_session_deserialize(&session, payload, size) != 0 ||
        dm2_v1_runtime_apply_session(&session) != 0) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }
    out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_OK;
    out->restored = 1;
    /* The verified Firestaff session carries the persisted DM2 RNG seed.
     * Runtime weather owns the live copy; do not invent a weather transition. */
    g_dm2_runtime.weather.weather_seed = session.rng_seed;
    return 1;
}

int dm2_v1_runtime_import_sksave_corpus(
    const char *save_root, DM2_V1_RuntimeCorpusImportReceipt *out)
{
    DM2_SKSaveCorpusReceipt corpus;
    const DM2_SKSaveCandidateReceipt *selected = NULL;
    uint8_t i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!dm2_v1_sksave_corpus_scan(save_root, &corpus) ||
        corpus.importable_candidate_count == 0u ||
        corpus.first_importable_path[0] == '\0') {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE;
        return 0;
    }
    for (i = 0u; i < corpus.candidate_receipt_count; ++i) {
        if (strcmp(corpus.candidate_receipts[i].path,
                   corpus.first_importable_path) == 0) {
            selected = &corpus.candidate_receipts[i];
            break;
        }
    }
    if (!selected) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }
    return dm2_v1_runtime_import_sksave_receipted_candidate(selected, out);
}

uint32_t dm2_v1_runtime_get_leader_hand_object(void) {
    return g_dm2_runtime.leader_hand_object;
}

void dm2_v1_runtime_set_leader_hand_object(uint32_t object) {
    g_dm2_runtime.leader_hand_object = object;
}

uint32_t dm2_v1_runtime_get_champion_inventory_object(uint8_t champion,
                                                      uint8_t slot) {
    if (champion >= 4u || slot >= 30u) {
        return 0u;
    }
    return g_dm2_runtime.champion_inventory_objects[champion][slot];
}

int dm2_v1_runtime_set_champion_inventory_object(uint8_t champion,
                                                 uint8_t slot,
                                                 uint32_t object) {
    if (champion >= 4u || slot >= 30u) {
        return -1;
    }
    g_dm2_runtime.champion_inventory_objects[champion][slot] = object;
    return 0;
}

int dm2_v1_runtime_export_inventory_to_session(DM2_V1_SessionState *session) {
    if (!session || !dm2_v1_session_validate(session)) {
        return -1;
    }
    session->original_leader_hand_object = g_dm2_runtime.leader_hand_object;
    for (uint8_t c = 0; c < session->champion_count && c < 4u; ++c) {
        DM2_ChampionRecord *champ =
            (DM2_ChampionRecord *)session->champion_data[c];
        for (uint8_t slot = 0; slot < 30u; ++slot) {
            champ->inventory[slot] =
                g_dm2_runtime.champion_inventory_objects[c][slot];
        }
    }
    return 0;
}

int dm2_v1_runtime_export_session(DM2_V1_SessionState *session) {
    DM2_V1_GameState *gs;
    if (!session || !g_dm2_runtime.session_snapshot_valid) {
        return -1;
    }
    *session = g_dm2_runtime.session_snapshot;
    if (g_dm2_runtime.boot && g_dm2_runtime.boot->dm2_state) {
        gs = (DM2_V1_GameState *)g_dm2_runtime.boot->dm2_state;
        session->party_x = (uint16_t)gs->party_x;
        session->party_y = (uint16_t)gs->party_y;
        session->party_dir = (uint8_t)(gs->party_dir & 3);
        session->party_level = (uint8_t)gs->current_level;
        session->outdoor_mode = (uint8_t)(gs->outdoor ? 1 : 0);
        session->gold = (uint32_t)gs->gold;
        session->reputation = (int16_t)gs->reputation;
        session->time_of_day_minutes = (uint16_t)gs->time_of_day;
    }
    session->game_tick = (uint32_t)g_dm2_runtime.tick_count;
    session->rain_intensity =
        (uint8_t)g_dm2_runtime.weather.weather_intensity;
    session->original_minions = g_dm2_runtime.minions;
    if (dm2_v1_runtime_export_inventory_to_session(session) != 0) {
        return -1;
    }
    if (!dm2_v1_session_validate(session)) {
        return -1;
    }
    g_dm2_runtime.session_snapshot = *session;
    return 0;
}

size_t dm2_v1_runtime_live_save_size(void) {
    DM2_V1_DungeonData *dungeon;
    uint8_t session[sizeof(DM2_V1_SessionState)];
    DM2_V1_SessionState state;
    int session_size;

    if (!g_dm2_runtime.boot || !g_dm2_runtime.boot->dungeon_data) return 0;
    dungeon = (DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data;
    if (!dungeon->raw_data || dungeon->raw_size <= 0) return 0;
    /* Session serialization has a bounded fixed size. Keep the actual size
     * authoritative, since the compatible session envelope may grow. */
    if (dm2_v1_runtime_export_session(&state) != 0) return 0;
    session_size = dm2_v1_session_serialize(&state, session, sizeof(session));
    if (session_size < 0) return 0;
    return sizeof(DM2_V1_RuntimeSaveHeader) + (size_t)session_size +
           sizeof(DM2_V1_CreatureLiveState) + (size_t)dungeon->raw_size;
}

int dm2_v1_runtime_serialize_live_save(uint8_t *out, size_t out_size) {
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RuntimeSaveHeader header;
    DM2_V1_CreatureLiveState creatures;
    DM2_V1_SessionState session;
    int session_size;
    size_t total;
    uint8_t *cursor;

    if (!out || out_size < sizeof(header) || !g_dm2_runtime.boot ||
        !g_dm2_runtime.boot->dungeon_data ||
        dm2_v1_runtime_export_session(&session) != 0) return -1;
    dungeon = (DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data;
    if (!dungeon->raw_data || dungeon->raw_size <= 0 ||
        dm2_v1_creature_export_live_state(&creatures) != 0) return -1;
    session_size = dm2_v1_session_serialize(&session, out + sizeof(header),
                                             out_size > sizeof(header)
                                                 ? out_size - sizeof(header) : 0);
    if (session_size < 0) return -1;
    total = sizeof(header) + (size_t)session_size + sizeof(creatures) +
            (size_t)dungeon->raw_size;
    if (total > out_size) return -1;
    memset(&header, 0, sizeof(header));
    memcpy(header.magic, DM2_RUNTIME_SAVE_MAGIC, 8);
    header.version = DM2_RUNTIME_SAVE_VERSION;
    header.session_size = (uint32_t)session_size;
    header.creature_size = (uint32_t)sizeof(creatures);
    header.dungeon_size = (uint32_t)dungeon->raw_size;
    header.graphics_size = (uint32_t)g_dm2_runtime.boot->graphics_size;
    snprintf(header.graphics_md5, sizeof(header.graphics_md5), "%s",
             g_dm2_runtime.boot->graphics_md5);
    memcpy(header.map_wall_gfx_list, g_dm2_runtime.map_wall_gfx_list,
           sizeof(header.map_wall_gfx_list));
    memcpy(header.map_floor_gfx_list, g_dm2_runtime.map_floor_gfx_list,
           sizeof(header.map_floor_gfx_list));
    memcpy(header.map_door_gfx_list, g_dm2_runtime.map_door_gfx_list,
           sizeof(header.map_door_gfx_list));
    header.map_wall_gfx_count = g_dm2_runtime.map_wall_gfx_count;
    header.map_floor_gfx_count = g_dm2_runtime.map_floor_gfx_count;
    header.move_cooldown_ticks = g_dm2_runtime.move_cooldown_ticks;
    header.paused = g_dm2_runtime.paused;
    header.map_graphics_style = g_dm2_runtime.map_graphics_style;
    header.gdat_scene_control_ready =
        g_dm2_runtime.gdat_scene_control_ready;
    header.gdat_scene_control_hash =
        g_dm2_runtime.gdat_scene_control_hash;
    header.gdat_scene_control_present_mask =
        g_dm2_runtime.gdat_scene_control_present_mask;
    header.gdat_scene_control_query_count =
        g_dm2_runtime.gdat_scene_control_query_count;
    header.gdat_scene_colorkey = g_dm2_runtime.gdat_scene_colorkey;
    header.gdat_scene_flags = g_dm2_runtime.gdat_scene_flags;
    header.gdat_scene_ambient_light =
        g_dm2_runtime.gdat_scene_ambient_light;
    header.gdat_scene_highest_light_level =
        g_dm2_runtime.gdat_scene_highest_light_level;
    header.gdat_scene_void_random_fall =
        g_dm2_runtime.gdat_scene_void_random_fall;
    header.gdat_scene_animated_floor =
        g_dm2_runtime.gdat_scene_animated_floor;
    header.gdat_scene_rain = g_dm2_runtime.gdat_scene_rain;
    header.gdat_misty_map = g_dm2_runtime.gdat_misty_map;
    header.gdat_thunder_position = g_dm2_runtime.gdat_thunder_position;
    header.gdat_ambient_darkness = g_dm2_runtime.gdat_ambient_darkness;
    memcpy(out, &header, sizeof(header));
    cursor = out + sizeof(header) + (size_t)session_size;
    memcpy(cursor, &creatures, sizeof(creatures));
    cursor += sizeof(creatures);
    memcpy(cursor, dungeon->raw_data, (size_t)dungeon->raw_size);
    return (int)total;
}

int dm2_v1_runtime_restore_live_save(const uint8_t *data, size_t data_size) {
    const DM2_V1_RuntimeSaveHeader *header;
    const uint8_t *cursor;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SessionState session;
    DM2_V1_CreatureLiveState creatures;
    size_t total;

    if (!data || !g_dm2_runtime.boot || !g_dm2_runtime.boot->dm2_state ||
        !g_dm2_runtime.boot->dungeon_data ||
        data_size < sizeof(*header)) return -1;
    header = (const DM2_V1_RuntimeSaveHeader *)data;
    dungeon = (DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data;
    if (!dm2_runtime_live_header_valid(header, dungeon)) return -1;
    total = sizeof(*header) + (size_t)header->session_size +
            (size_t)header->creature_size + (size_t)header->dungeon_size;
    if (total != data_size ||
        dm2_v1_session_deserialize(&session, data + sizeof(*header),
                                   header->session_size) != 0) return -1;
    cursor = data + sizeof(*header) + header->session_size;
    memcpy(&creatures, cursor, sizeof(creatures));
    if (dm2_v1_creature_restore_live_state(&creatures) != 0) return -1;
    cursor += sizeof(creatures);
    memcpy(dungeon->raw_data, cursor, (size_t)dungeon->raw_size);
    g_dm2_runtime_restore_in_progress = 1;
    if (dm2_v1_runtime_apply_session(&session) != 0) {
        g_dm2_runtime_restore_in_progress = 0;
        return -1;
    }
    g_dm2_runtime_restore_in_progress = 0;
    memcpy(g_dm2_runtime.map_wall_gfx_list, header->map_wall_gfx_list,
           sizeof(g_dm2_runtime.map_wall_gfx_list));
    memcpy(g_dm2_runtime.map_floor_gfx_list, header->map_floor_gfx_list,
           sizeof(g_dm2_runtime.map_floor_gfx_list));
    memcpy(g_dm2_runtime.map_door_gfx_list, header->map_door_gfx_list,
           sizeof(g_dm2_runtime.map_door_gfx_list));
    g_dm2_runtime.map_wall_gfx_count = header->map_wall_gfx_count;
    g_dm2_runtime.map_floor_gfx_count = header->map_floor_gfx_count;
    g_dm2_runtime.move_cooldown_ticks = header->move_cooldown_ticks;
    g_dm2_runtime.paused = header->paused;
    if (header->gdat_scene_control_ready &&
        header->map_graphics_style == g_dm2_runtime.map_graphics_style &&
        header->gdat_scene_control_hash != 0u &&
        header->gdat_scene_control_present_mask != 0u) {
        /* skproject persists the current map and dungeon DB state; the
         * Firestaff sidecar keeps the derived GRAPHICSSET control receipt
         * aligned with that restored map so the first post-load frame need
         * not re-prove the handoff through fallback scene constants. */
        g_dm2_runtime.gdat_scene_control_ready = 1;
        g_dm2_runtime.gdat_scene_control_hash =
            header->gdat_scene_control_hash;
        g_dm2_runtime.gdat_scene_control_present_mask =
            header->gdat_scene_control_present_mask;
        g_dm2_runtime.gdat_scene_control_query_count =
            header->gdat_scene_control_query_count;
        g_dm2_runtime.gdat_scene_colorkey = header->gdat_scene_colorkey;
        g_dm2_runtime.gdat_scene_flags = header->gdat_scene_flags;
        g_dm2_runtime.gdat_scene_ambient_light =
            header->gdat_scene_ambient_light;
        g_dm2_runtime.gdat_scene_highest_light_level =
            header->gdat_scene_highest_light_level;
        g_dm2_runtime.gdat_scene_void_random_fall =
            header->gdat_scene_void_random_fall;
        g_dm2_runtime.gdat_scene_animated_floor =
            header->gdat_scene_animated_floor;
        g_dm2_runtime.gdat_scene_rain = header->gdat_scene_rain;
        g_dm2_runtime.gdat_misty_map = header->gdat_misty_map;
        g_dm2_runtime.gdat_thunder_position =
            header->gdat_thunder_position;
        g_dm2_runtime.gdat_ambient_darkness =
            header->gdat_ambient_darkness;
    }
    return 0;
}

int dm2_v1_runtime_restore_save_candidate(const uint8_t *data,
                                          size_t data_size)
{
    DM2_V1_SaveCandidate candidate;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_CreatureLiveState cleared_creatures;
    DM2_V1_DungeonData parsed_dungeon;
    DM2_V1_DungeonData saved_dungeon;
    int parsed_original_dungeon = 0;

    if (!data || !g_dm2_runtime.boot || !g_dm2_runtime.boot->dm2_state ||
        !g_dm2_runtime.boot->dungeon_data ||
        dm2_v1_session_parse_save_candidate(&candidate, data, data_size) != 0) {
        return -1;
    }
    dungeon = (DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data;
    if (!dungeon->raw_data || dungeon->raw_size <= 0 ||
        (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
         candidate.dungeon_size != (size_t)dungeon->raw_size)) {
        return -1;
    }

    memset(&parsed_dungeon, 0, sizeof(parsed_dungeon));
    if (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        /* skproject/SKWINSPX/src/v4/skcore.cpp::GAME_LOAD calls
         * READ_DUNGEON_STRUCTURE before it consumes skload_table_60.  The
         * raw prefix therefore owns both the bytes and the live G1 layout;
         * copying it under old DUNGEON.DAT metadata can select the wrong map
         * dimensions, GRAPHICSSET, or record-pool spans.  Parse the complete
         * saved prefix first, then publish it in one swap below. */
        if (dm2_v1_dungeon_load(&parsed_dungeon, candidate.dungeon_bytes,
                                (int)candidate.dungeon_size) != 0 ||
            candidate.session.party_level >= parsed_dungeon.level_count ||
            candidate.session.party_x >=
                (uint16_t)parsed_dungeon.level_widths[
                    candidate.session.party_level] ||
            candidate.session.party_y >=
                (uint16_t)parsed_dungeon.level_heights[
                    candidate.session.party_level]) {
            dm2_v1_dungeon_free(&parsed_dungeon);
            return -1;
        }
        saved_dungeon = *dungeon;
        *dungeon = parsed_dungeon;
        memset(&parsed_dungeon, 0, sizeof(parsed_dungeon));
        parsed_original_dungeon = 1;
    }

    /* Original SKSave has dungeon DB records but no Firestaff-only CCM cache.
     * Clear that cache before apply_session; a matching quicksave sidecar is
     * allowed to replace it with the exact saved CCM/animation/GDAT state. */
    memset(&cleared_creatures, 0, sizeof(cleared_creatures));
    if (dm2_v1_creature_restore_live_state(&cleared_creatures) != 0 ||
        dm2_v1_runtime_apply_session(&candidate.session) != 0) {
        if (parsed_original_dungeon) {
            dm2_v1_dungeon_free(dungeon);
            *dungeon = saved_dungeon;
        }
        return -1;
    }
    if (parsed_original_dungeon) {
        dm2_v1_dungeon_free(&saved_dungeon);
    }
    return 0;
}

int dm2_v1_runtime_load_save_slot(const char *save_base, uint8_t slot)
{
    uint8_t data[DM2_SESSION_MAX_SIZE];
    size_t data_size = 0u;

    if (dm2_sl_load(save_base, slot, data, sizeof(data), &data_size) != 0) {
        return -1;
    }
    return dm2_v1_runtime_restore_save_candidate(data, data_size);
}

int dm2_v1_runtime_load_last_session(const char *save_base)
{
    uint8_t data[DM2_SESSION_MAX_SIZE];
    size_t data_size = 0u;

    if (dm2_sl_load_last_session(save_base, data, sizeof(data), &data_size) != 0) {
        return -1;
    }
    return dm2_v1_runtime_restore_save_candidate(data, data_size);
}

static int dm2_runtime_write_live_sidecar(const char *save_root)
{
    char path[512];
    uint8_t *data;
    size_t size;
    FILE *file;

    if (!save_root || !FSP_JoinPath(path, sizeof(path), save_root,
                                    "SKSave.runtime")) return -1;
    size = dm2_v1_runtime_live_save_size();
    if (size == 0) return -1;
    data = (uint8_t *)malloc(size);
    if (!data) return -1;
    if (dm2_v1_runtime_serialize_live_save(data, size) < 0) {
        free(data);
        return -1;
    }
    file = fopen(path, "wb");
    if (!file || fwrite(data, 1, size, file) != size) {
        if (file) fclose(file);
        free(data);
        return -1;
    }
    fclose(file);
    free(data);
    return 0;
}

static void dm2_runtime_restore_live_sidecar(const char *save_root,
                                             const DM2_V1_SessionState *session)
{
    char path[512];
    FILE *file;
    long length;
    uint8_t *data;
    DM2_V1_SessionState saved;

    if (!save_root || !session || !FSP_JoinPath(path, sizeof(path), save_root,
                                    "SKSave.runtime")) return;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return;
    }
    data = (uint8_t *)malloc((size_t)length);
    if (!data || fread(data, 1, (size_t)length, file) != (size_t)length) {
        fclose(file);
        free(data);
        return;
    }
    fclose(file);
    if ((size_t)length <= sizeof(DM2_V1_RuntimeSaveHeader)) {
        free(data);
        return;
    }
    if (dm2_v1_session_deserialize(&saved, data + sizeof(DM2_V1_RuntimeSaveHeader),
                                   (size_t)length - sizeof(DM2_V1_RuntimeSaveHeader)) != 0 ||
        saved.game_tick != session->game_tick || saved.rng_seed != session->rng_seed ||
        saved.party_x != session->party_x || saved.party_y != session->party_y ||
        saved.party_level != session->party_level || saved.party_dir != session->party_dir) {
        free(data);
        return;
    }
    (void)dm2_v1_runtime_restore_live_save(data, (size_t)length);
    free(data);
}

static void dm2_v1_quicksave_receipt_init(
    DM2_V1_QuicksaveReceipt *receipt,
    DM2_V1_QuicksaveResult result,
    const char *status)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = result;
    receipt->status_scope = "SAVE";
    receipt->status = status;
}

int dm2_v1_runtime_quicksave_boot_profile_with_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_QuicksaveReceipt *out_receipt)
{
    DM2_V1_QuicksaveReceipt local;
    DM2_V1_QuicksaveReceipt *receipt = out_receipt ? out_receipt : &local;

    dm2_v1_quicksave_receipt_init(receipt,
                                  DM2_V1_QUICKSAVE_PROFILE_MISSING,
                                  "DM2 PROFILE MISSING");
    if (!profile) {
        return 0;
    }
    if (!profile->save_root[0]) {
        dm2_v1_boot_set_save_root(profile, NULL);
    }
    snprintf(receipt->save_root, sizeof(receipt->save_root),
             "%s", profile->save_root);
    if (!FSP_CreateDirectoryRecursive(profile->save_root)) {
        dm2_v1_quicksave_receipt_init(receipt,
                                      DM2_V1_QUICKSAVE_SAVE_DIR_FAILED,
                                      "DM2 SAVE DIR FAILED");
        snprintf(receipt->save_root, sizeof(receipt->save_root),
                 "%s", profile->save_root);
        return 0;
    }
    memset(&receipt->session, 0, sizeof(receipt->session));
    (void)dm2_v1_runtime_graphicsset_scene_receipt(
        &receipt->graphicsset_scene);
    if (dm2_v1_runtime_export_session(&receipt->session) != 0) {
        dm2_v1_quicksave_receipt_init(receipt,
                                      DM2_V1_QUICKSAVE_EXPORT_FAILED,
                                      "DM2 EXPORT FAILED");
        snprintf(receipt->save_root, sizeof(receipt->save_root),
                 "%s", profile->save_root);
        return 0;
    }
    /* skproject/SkWin save flow writes the current live runtime session to
     * SKSave.dat as the direct resume target. Firestaff keeps M11 outside
     * the SKSave write path; runtime owns export + last-session rotation. */
    if (dm2_v1_session_save_last_session(profile->save_root,
                                         "Firestaff DM2",
                                         &receipt->session) != 0) {
        dm2_v1_quicksave_receipt_init(receipt,
                                      DM2_V1_QUICKSAVE_WRITE_FAILED,
                                      "DM2 WRITE FAILED");
        snprintf(receipt->save_root, sizeof(receipt->save_root),
                 "%s", profile->save_root);
        return 0;
    }
    if (dm2_runtime_write_live_sidecar(profile->save_root) != 0) {
        dm2_v1_quicksave_receipt_init(receipt,
                                      DM2_V1_QUICKSAVE_WRITE_FAILED,
                                      "DM2 RUNTIME WRITE FAILED");
        snprintf(receipt->save_root, sizeof(receipt->save_root), "%s",
                 profile->save_root);
        return 0;
    }
    if (!FSP_JoinPath(receipt->save_path, sizeof(receipt->save_path),
                      profile->save_root, "SKSave.dat")) {
        receipt->save_path[0] = '\0';
    }
    receipt->result = DM2_V1_QUICKSAVE_OK;
    receipt->status_scope = "SAVE";
    receipt->status = "DM2 SKSAVE WRITTEN";
    receipt->session_valid = 1;
    return 1;
}

uint8_t dm2_v1_runtime_get_minion_count(void) {
    return g_dm2_runtime.minions.count;
}

int dm2_v1_runtime_get_minion_assoc(uint8_t index, DM2_MinionAssoc *out_assoc) {
    if (!out_assoc || index >= g_dm2_runtime.minions.count ||
        index >= DM2_MAX_MINIONS) {
        return -1;
    }
    *out_assoc = g_dm2_runtime.minions.entries[index];
    return 0;
}

uint32_t dm2_v1_runtime_get_weather_seed(void) {
    return g_dm2_runtime.weather.weather_seed;
}

void dm2_v1_runtime_set_weather_seed(uint32_t seed) {
    dm2_v1_weather_set_seed(&g_dm2_runtime.weather, seed);
}

/* dm2_v1_runtime_has_dungeon_data — returns 1 if dungeon state is available.
 * Used by dm2_v2_runtime_render_frame to detect headless (no dungeon) mode.
 * Source: Phase 5 runtime binding */
int dm2_v1_runtime_has_dungeon_data(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    return (rt->boot && rt->boot->dm2_state) ? 1 : 0;
}

/* ── V2 Smooth Movement Callbacks ───────────────────────────────── */

void dm2_v1_runtime_set_move_callback(DM2_V2_MoveCallback cb) {
    g_dm2_runtime.move_callback = cb;
}

void dm2_v1_runtime_set_turn_callback(DM2_V2_TurnCallback cb) {
    g_dm2_runtime.turn_callback = cb;
}

void dm2_v1_runtime_set_stairs_callback(DM2_V2_StairsCallback cb) {
    g_dm2_runtime.stairs_callback = cb;
}

/* ── Interaction / square helpers ─────────────────────────────────── */

int dm2_v1_runtime_get_square_type(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    return dm2_v1_dungeon_get_square_type(dd, level, x, y);
}

int dm2_v1_runtime_is_passable(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;
    int tile_type;

    if (!rt->boot || !rt->boot->dungeon_data) return 0;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return 0;

    tile_type = dm2_runtime_square_type_at(dd, level, x, y, raw);
    if (tile_type == 0 || tile_type == 5 ||
        tile_type == 11 || tile_type == 13) {
        return 0;
    }
    if (dm2_runtime_is_door_at(dd, level, x, y, raw)) {
        return dm2_door_state_blocks_movement(
                   dm2_runtime_door_state((uint16_t)raw), 1) ? 0 : 1;
    }
    return 1;
}

int dm2_v1_runtime_door_action(int level,
                               int x,
                               int y,
                               int facing_dir,
                               int action) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;
    int state;
    int next_state;

    (void)facing_dir;
    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0) return -1;
    if (!dm2_runtime_is_door_at(dd, level, x, y, raw)) return -1;

    state = dm2_runtime_door_state((uint16_t)raw);
    next_state = dm2_runtime_door_step(state, action);
    if (next_state == state) return 0;

    return dm2_v1_dungeon_set_tile_raw(
        dd, level, x, y,
        dm2_runtime_door_set_state((uint16_t)raw, next_state));
}

int dm2_v1_runtime_get_door_state(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int raw;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0 || !dm2_runtime_is_door_at(dd, level, x, y, raw)) return -1;
    return dm2_runtime_door_state((uint16_t)raw);
}

int dm2_v1_runtime_enter_shop(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int shop_id = DM2_SHOP_ID_NONE;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (!rt->outdoor && !gs->outdoor) return -1;
    for (int i = 1; i <= DM2_NUM_BUILTIN_SHOPS; i++) {
        const DM2_V1_ShopDescriptor *shop = dm2_v1_shop_get_builtin(i);
        if (shop && shop->map_level == level &&
            shop->map_x == x && shop->map_y == y) {
            shop_id = shop->shop_id;
            break;
        }
    }
    if (shop_id == DM2_SHOP_ID_NONE) return -1;
    dm2_v1_shop_set_party_gold((uint32_t)(gs->gold < 0 ? 0 : gs->gold));
    if (!dm2_v1_shop_enter(shop_id)) return -1;
    gs->time_of_day = rt->time_of_day_minutes;
    return 0;
}

int dm2_v1_runtime_npc_interact(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    int same_npc_square;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    if (dm2_v1_runtime_get_square_type(level, x, y) < 0) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (!rt->outdoor && !gs->outdoor) return -1;

    same_npc_square = rt->last_npc_level == level &&
                      rt->last_npc_x == x &&
                      rt->last_npc_y == y;
    rt->last_npc_id = DM2_NPC_MERCHANT_FRIENDLY;
    rt->last_npc_level = level;
    rt->last_npc_x = x;
    rt->last_npc_y = y;
    if (same_npc_square && rt->last_npc_dialog_line >= 0) {
        rt->last_npc_dialog_line =
            (rt->last_npc_dialog_line + 1) % DM2_NPC_DIALOG_LINES;
    } else {
        rt->last_npc_dialog_line = 0;
    }
    if (gs->reputation < 9999) {
        gs->reputation++;
    }
    return 0;
}

int dm2_v1_runtime_get_last_npc_id(void) {
    return g_dm2_runtime.last_npc_id;
}

int dm2_v1_runtime_get_last_npc_dialog_line(void) {
    return g_dm2_runtime.last_npc_dialog_line;
}

int dm2_v1_runtime_signal_item_used(int item_id) {
    DM2_V1_TriggerEvent event;
    if (dm2_v1_trigger_signal_item_used(item_id) <= 0) return 0;
    if (!dm2_v1_trigger_copy_last_event(&event)) return 0;
    dm2_runtime_apply_trigger_event(&g_dm2_runtime, &event);
    return 1;
}

int dm2_v1_runtime_signal_combat_ended(int victory) {
    int fired;
    int before[DM2_TRIGGER_NUM_BUILTIN];

    for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
        const DM2_V1_Trigger *trigger = dm2_v1_trigger_get_builtin(i);
        before[i - 1] = trigger
            ? dm2_v1_trigger_get_fire_count(trigger->trigger_id)
            : -1;
    }
    fired = dm2_v1_trigger_signal_combat_ended(victory);
    if (fired <= 0) return 0;
    for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
        DM2_V1_TriggerEvent event;
        const DM2_V1_Trigger *trigger = dm2_v1_trigger_get_builtin(i);
        const DM2_V1_TriggerState *state =
            trigger ? dm2_v1_trigger_get_state(trigger->trigger_id) : NULL;
        if (!trigger || !state) continue;
        if (trigger->kind != DM2_TRIGGER_KIND_COMBAT_ENDED) continue;
        if (state->fired_count <= before[i - 1]) continue;
        if (dm2_runtime_event_from_trigger(trigger, state, &event)) {
            dm2_runtime_apply_trigger_event(&g_dm2_runtime, &event);
        }
    }
    return fired;
}

const char *dm2_v1_runtime_get_last_target_message(void) {
    return g_dm2_runtime.last_target_message[0] != '\0'
        ? g_dm2_runtime.last_target_message
        : NULL;
}

int dm2_v1_runtime_get_last_spawn_instance_id(void) {
    return g_dm2_runtime.last_spawn_instance_id;
}

int dm2_v1_runtime_get_last_spawn_ai(void) {
    return g_dm2_runtime.last_spawn_ai;
}

int dm2_v1_runtime_get_last_spawn_x(void) {
    return g_dm2_runtime.last_spawn_x;
}

int dm2_v1_runtime_get_last_spawn_y(void) {
    return g_dm2_runtime.last_spawn_y;
}

int dm2_v1_runtime_get_last_spawn_level(void) {
    return g_dm2_runtime.last_spawn_level;
}

int dm2_v1_runtime_get_spawn_count(void) {
    return g_dm2_runtime.spawn_count;
}

int dm2_v1_runtime_get_last_actuator_type(void) {
    return g_dm2_runtime.last_actuator_type;
}

int dm2_v1_runtime_get_last_actuator_x(void) {
    return g_dm2_runtime.last_actuator_x;
}

int dm2_v1_runtime_get_last_actuator_y(void) {
    return g_dm2_runtime.last_actuator_y;
}

int dm2_v1_runtime_get_last_actuator_level(void) {
    return g_dm2_runtime.last_actuator_level;
}

int dm2_v1_runtime_get_actuator_count(void) {
    return g_dm2_runtime.actuator_count;
}

uint32_t dm2_v1_runtime_get_last_generated_object(void) {
    return g_dm2_runtime.last_generated_object;
}

int dm2_v1_runtime_get_last_projectile_slot(void) {
    return g_dm2_runtime.last_projectile_slot;
}

int dm2_v1_runtime_get_projectile_actuator_count(void) {
    return g_dm2_runtime.projectile_actuator_count;
}

static void dm2_runtime_record_actuator(DM2_V1_RuntimeState *rt,
                                        int level,
                                        int x,
                                        int y,
                                        DM2_ActuatorType type) {
    if (!rt) return;
    rt->last_actuator_type = (int)type;
    rt->last_actuator_x = x;
    rt->last_actuator_y = y;
    rt->last_actuator_level = level;
    rt->actuator_count++;
}

typedef struct {
    int actuator_type;
    uint16_t flag;
    int target_level;
    int target_x;
    int target_y;
} DM2_RuntimeSquareActuator;

static int dm2_runtime_decode_square_actuator(
    const uint8_t *record,
    int size,
    int current_level,
    int current_x,
    int current_y,
    DM2_RuntimeSquareActuator *out) {
    if (!record || size < 8 || !out) return 0;
    memset(out, 0, sizeof(*out));
    /* Bounded DB3 handoff:
     *   w0      next thing link, owned by GET_NEXT_RECORD_LINK
     *   byte 2  actuator/effect type
     *   byte 3  target level, or 0xff for current level
     *   w4      target flag/object id/payload
     *   byte 6  target x, or 0xff for current x
     *   byte 7  target y, or 0xff for current y
     * This preserves coordinate zero as a real target coordinate. */
    out->actuator_type = (int)record[2];
    out->target_level = (record[3] == 0xffu) ? current_level : (int)record[3];
    out->flag = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    out->target_x = (record[6] == 0xffu) ? current_x : (int)record[6];
    out->target_y = (record[7] == 0xffu) ? current_y : (int)record[7];
    return out->actuator_type != 0;
}

int dm2_v1_runtime_invoke_actuator(int level, int x, int y,
                                   DM2_ActuatorType type, uint16_t flag) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    int projectile_slot;
    int projectile_category;
    int projectile_subtype;

    if (dm2_v1_runtime_get_square_type(level, x, y) < 0) return -1;
    dm2_runtime_record_actuator(rt, level, x, y, type);
    if (type == DM2_ACTUATOR_SHOP_PANEL) {
        return dm2_v1_runtime_enter_shop(level, x, y);
    }
    if (type == DM2_ACTUATOR_PUSH_BUTTON_WALL_SWITCH ||
        type == DM2_ACTUATOR_WALL_SWITCH ||
        type == DM2_ACTUATOR_2_STATE_WALL_SWITCH ||
        type == DM2_ACTUATOR_DM1_WALL_SWITCH) {
        return 0;
    }
    if (type == DM2_ACTUATOR_CREATURE_GENERATOR) {
        int ai = flag ? (int)flag : DM2_AI_DRAGOTH_MINION;
        dm2_runtime_record_spawn(rt, ai, level, x, y);
        return rt->last_spawn_instance_id >= 0 ? 0 : -1;
    }
    if (type == DM2_ACTUATOR_ITEM_GENERATOR ||
        type == DM2_ACTUATOR_ITEM_CAPTURE ||
        type == DM2_ACTUATOR_ITEM_RECYCLER ||
        type == DM2_ACTUATOR_FLYING_ITEM_CATCHER ||
        type == DM2_ACTUATOR_FLYING_ITEM_TELEPORTER) {
        rt->last_generated_object = flag ? (uint32_t)flag : 0x0A000001u;
        return 0;
    }
    if (type == DM2_ACTUATOR_MISSILE_SHOOTER ||
        type == DM2_ACTUATOR_WEAPON_SHOOTER ||
        type == DM2_ACTUATOR_ITEM_SHOOTER) {
        projectile_category = PROJECTILE_CATEGORY_KINETIC;
        projectile_subtype = DM2_PROJ_SUBTYPE_KINETIC_ARROW;
        if (type == DM2_ACTUATOR_MISSILE_SHOOTER) {
            projectile_category = PROJECTILE_CATEGORY_MAGICAL;
            projectile_subtype = flag ? (int)flag : DM2_PROJ_SUBTYPE_MAGICAL_FIREBALL;
        } else if (type == DM2_ACTUATOR_ITEM_SHOOTER) {
            projectile_subtype = flag ? (int)flag : DM2_PROJ_SUBTYPE_BOMB;
        }
        projectile_slot = dm2_v1_projectile_dispatch_synthetic(
            projectile_category, projectile_subtype, x, y, level,
            g_dm2_runtime.view_dir);
        rt->last_projectile_slot = projectile_slot;
        if (projectile_slot >= 0) rt->projectile_actuator_count++;
        return projectile_slot >= 0 ? 0 : -1;
    }
    if (type == DM2_ACTUATOR_CROSS_MAP ||
        type == DM2_ACTUATOR_RELAY_1 ||
        type == DM2_ACTUATOR_RELAY_2 ||
        type == DM2_ACTUATOR_WORK_TIMER ||
        type == DM2_ACTUATOR_TICK_GENERATOR ||
        type == DM2_ACTUATOR_COUNTER ||
        type == DM2_ACTUATOR_ARRIVAL_DEPARTURE ||
        type == DM2_ACTUATOR_SWITCH_SIGN_FOR_CREATURE) {
        return 0;
    }
    return 0;
}

int dm2_v1_runtime_invoke_square_actuators(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_DungeonData *dd;
    int thing;
    int invoked = 0;
    int guard = 0;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    thing = dm2_v1_dungeon_get_first_thing(dd, level, x, y);
    if (thing < 0 || thing == 0xfffe) return 0;

    while (thing >= 0 && thing != 0xfffe && guard++ < 64) {
        int type = -1;
        int size = 0;
        int next;
        const uint8_t *record = dm2_v1_dungeon_get_thing_record(
            dd, (uint16_t)thing, &type, NULL, &size);
        if (!record || size < 2) break;
        if (type == 3 && size >= 8) {
            DM2_RuntimeSquareActuator decoded;
            if (dm2_runtime_decode_square_actuator(
                    record, size, level, x, y, &decoded) &&
                dm2_v1_runtime_invoke_actuator(
                    decoded.target_level, decoded.target_x, decoded.target_y,
                    (DM2_ActuatorType)decoded.actuator_type,
                    decoded.flag) == 0) {
                invoked++;
            }
        }
        next = dm2_v1_dungeon_get_next_thing(dd, (uint16_t)thing);
        if (next < 0 || next == thing) break;
        thing = next;
    }
    return invoked;
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *dm2_v1_runtime_source_evidence(void) {
    return
        "DM2 V1 Runtime Stub — Phase 1\n"
        "Source: SKULL.ASM T048  — input dispatch / tick update\n"
        "Source: SKULL.ASM T520  — movement speed and party placement\n"
        "Source: SKULL.ASM T560  — dungeon tick and viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor tick and weather rendering\n"
        "Weather transition seed: ReDMCSB BASE.C F0027/F0029 (LCG 0xBB40E62D, +11)\n"
        "Reference: CSB path in firestaff_game_loop.c (FS_GAME_CSB → csb_v1_viewport_render_frame)\n";
}
