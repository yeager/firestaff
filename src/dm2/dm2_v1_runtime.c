/*
 * dm2_v1_runtime.c — DM2 V1 source-bound runtime
 *
 * Provides the source-gated M11 tick and real-GDAT frame path for an
 * admitted boot profile. Gameplay families whose original runtime state is
 * not yet owned remain fail-closed rather than becoming a host substitute.
 * This runtime wires the DM2 viewport into the Firestaff game loop only
 * after the boot profile and the frame's GDAT material receipts validate.
 * It never uses a crash-avoidance or data-free viewport fallback.
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
#include "dm2_v1_boot.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_delete_creature_full_pc34_compat.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_perform_move.h"
#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_projectile_pc34_compat.h"
#include "dm2_v1_projectile_step_pc34_compat.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_spell_timer_handlers_pc34_compat.h"
#include "dm2_v1_think_creature_pc34_compat.h"
#include "dm2_v1_creature_schedule_pc34_compat.h"
#include "dm2_v1_update_weather_pc34_compat.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_shop.h"
#include "dm2_v1_trigger.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_i18n.h"
#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_record_ops_pc34_compat.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_sound_queue_pc34_compat.h"
#include "dm2_v1_timer_ops_pc34_compat.h"
#include "dm2_v1_engage_command_pc34_compat.h"
#include "dm2_v1_light_ops_pc34_compat.h"
#include "dm2_v1_creature_ops_pc34_compat.h"
#include "dm2_v1_ccm_loop_pc34_compat.h"
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
    DM2_V1_SetTimerWeatherReceipt set_timer_weather;
    DM2_V1_Weather3df70037Receipt weather_3df7_0037;
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
    uint8_t map_door_gfx_active[2];
    uint8_t map_door_ornate_list[16];
    int map_door_ornate_count;
    int map_graphics_style;
    DM2_V1_CLightMapDescriptorReceipt c_light_map_descriptor;
    DM2_V1_GdatSceneM11CommandPlan gdat_scene_material_plan;
    DM2_V1_GdatSceneLightM11Receipt gdat_scene_light_receipt;
    DM2_V1_CLightM11Receipt c_light_receipt;
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
    /* c_weather.cpp can append cloud, rain, then a lightning bolt to the
     * live DistantEnvironment chain. Keep all three source slots through
     * the M11 handoff; truncating the final bolt silently changed the
     * original weather transaction. */
    DM2_V1_DistantEnvironmentReceipt
        weather_distant_slots[DM2_V1_WEATHER_MAX_SLOTS];
    unsigned int weather_distant_slot_count;
    uint32_t weather_distant_slots_map_token;
    uint32_t weather_distant_slots_source_receipt_hash;
    uint8_t weather_distant_slots_graphicsset;
    int gdat_weather_renderer_ready;
    uint32_t gdat_weather_renderer_hash;
    uint32_t gdat_weather_renderer_command_count;
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
    DM2_V1_G1CreatureV5RuntimeReceipt g1_creature_v5_runtime;
    DM2_V1_G1WeaponMapChipRuntimeReceipt g1_weapon_map_chip_runtime;
    DM2_V1_G1RuntimeMapDoorReceipt g1_runtime_map_doors;
    DM2_V1_G1ContainerMapChipRuntimeReceipt g1_container_map_chip_runtime;
    DM2_V1_G1StaticObjectMaterialReceipt g1_static_object_materials[48];
    DM2_V1_StaticObjectSourcePlan g1_static_object_source_plans[48];
    DM2_V1_StaticObjectM11DeliveryPlan g1_static_object_delivery_plans[48];
    int g1_static_object_material_count;
    int g1_static_object_delivery_plan_count;
    DM2_V1_G1SceneRuntimeHandoffReceipt g1_scene_runtime_handoff;
    int g1_first_map_viewport_consumed;
    int g1_map0_teleporter_transition_viewport_consumed;
    DM2_V1_RuntimeMusicMapReceipt music_map_receipt;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_post_load;
    DM2_V1_RuntimeRawSaveHandoffReceipt raw_sksave_handoff;
    DM2_V1_WeatherTimerReceipt last_weather_timer_receipt;
    /* DM2-003: DM2-owned source-order timer queue + dispatcher receipt.
     * Every DM2 timer routes through dm2_v1_proceed_timers
     * (skproject/SKULLWIN/c_tim_proc.cpp DM2_PROCEED_TIMERS); the host no
     * longer runs an unconditional creature-tick simulation. */
    DM2_V1_SourceTimerQueue timer_queue;
    DM2_V1_ProceedTimersReceipt proceed_timers;
    /* DM2-003 follow-up (round 24): counters for the bounded DM2_STEP_DOOR
     * type-0x01 timer handler.  The handler performs one source-ordered
     * door-state mutation per tick and re-queues the next step until the
     * transition finishes. */
    int door_step_timers;        /* type-0x01 timers consumed */
    int door_step_mutations;     /* successful square state writes */
    int door_step_requeues;      /* next-step timers queued */
    /* DM2-003 follow-up: 1 while a producer-bound type-0x54 weather
     * timer (skproject/SKULLWIN/c_weather.cpp:20-30 DM2_SET_TIMER_WEATHER)
     * is pending in timer_queue. */
    int weather_source_timer_pending;
    /* DM2-003 follow-up: the source 0x54 weather chain
     * (skproject/SKULLWIN/c_weather.cpp).  Session-owned v1e14xx state,
     * the chain LCG (seeded from the session weather seed at chain
     * start), and the chain-started flag.  The chain is
     * self-perpetuating: DM2_weather_3df7_0037 queues the next 0x54
     * timer and DM2_UPDATE_WEATHER(1) re-queues RAND16(256)+50. */
    DM2_V1_UpdateWeatherState weather_chain;
    DM2_V1_DropRng weather_rng;
    int weather_chain_started;
    /* DM2-003/005 follow-up: session-owned DM2-002 record pools plus the
     * per-cell DM2_THINK_CREATURE binding (skproject c_querydb.cpp:1486-1507
     * DM2_GET_CREATURE_AT + c_tim_proc.cpp:4079-4088 payload decode).  The
     * pools are populated lazily from the boot dungeon data once its G1
     * candidate evidence validates; the think body stays unbound (receipted
     * fail-closed) until the CCM stream owner/grammar is proven. */
    DM2_V1_RecordPoolSet record_pools;
    int record_pools_valid;
    DM2_V1_ThinkCreatureBinding think_binding;
    int think_binding_ready;
    /* DM2-003/005 follow-up: session-owned CAII creature array for the
     * bounded DM2_ALLOC_CAII_TO_CREATURE slice (c_1c9a.cpp:5772-5894);
     * capacity is caller-owned until DM2_1c9a_3c30 (DM2_INIT) is proven. */
    DM2_V1_CaiiArray caii;
    int caii_ready;
    /* 2026-07-21 (round 22): production wiring of the 0fcb branch
     * (c_1c9a.cpp:5956-5957) to the COMPLETE DM2_DELETE_CREATURE_RECORD
     * composition.  The session LCG feeds the bound drop slice; the
     * last composition receipt is kept for the runtime accessor. */
    DM2_V1_DropRng drop_rng;
    DM2_V1_DeleteCreatureFullReceipt last_delete_full;
    int last_delete_full_valid;
    /* 2026-07-21 (round 23): session receipt for the floor-mecha CAII
     * activation wiring (0x04 timer, square class 1 ->
     * DM2_ACTUATE_FLOOR_MECHA chain walk -> DB3 record type 0x3a ->
     * DM2_ANIMATE_CREATURE, c_tim_proc.cpp:3009-3532 + 4297-4299). */
    int floor_mecha_timers;       /* class-1 0x04 timers consumed */
    int floor_mecha_0x3a_records; /* DB3 type-0x3a records visited */
    int floor_mecha_activations;  /* animate slices evaluated valid */
    int floor_mecha_allocs;       /* bound CAII allocations performed */
    int floor_mecha_db_break;     /* DB > 3 chain link: source return */
    int floor_mecha_walk_failed;  /* chain walk failed closed */
    /* 2026-07-23 (Lane B, cycle 8): counters for the expanded 0x04 actuator
     * tile subdispatch.  Each class gets a consumed counter; bounded mutations
     * (pitfall, door) also track rejections. */
    int actuator_tile_timers;        /* total 0x04 timers consumed */
    int actuator_tile_wall_mecha;    /* class 0 consumed */
    int actuator_tile_pitfall;       /* class 2 consumed */
    int actuator_tile_pitfall_toggles; /* successful floor<->pit writes */
    int actuator_tile_pitfall_rejected;
    int actuator_tile_door;          /* class 4 consumed */
    int actuator_tile_door_mutations; /* successful door state writes */
    int actuator_tile_door_rejected;
    int actuator_tile_teleporter;    /* class 5 consumed */
    int actuator_tile_trickwall;     /* class 6 consumed */
    /* DM2-007 cycle 12: last spell-cast failure feedback for M11 status scope.
     * Cleared on init; populated when a spell cast reports failure_feedback. */
    const char *last_spell_status_scope;
    const char *last_spell_status;
    int last_spell_failure_class;
    DM2_V1_SpellTimerHandlerContext spell_timer_ctx;
    int spell_timer_ctx_ready;
    DM2_V1_I18nContext i18n;
    int i18n_ready;
    /* DM2-008: source-ordered sound queue (c_sfx.cpp, c_sound.cpp).
     * QUEUE_NOISE_GEN1/GEN2 push entries; DM2_PLAY_SOUND drains them. */
    DM2_V1_SoundQueueState sound_queue;
    DM2_V1_SoundQueueEnv sound_env;
    int sound_queue_ready;
    /* CDDA playback callback — set by M11 host to push PCM to SDL3 */
    void (*cdda_play_cb)(void *ctx, const uint8_t *pcm, size_t size, int loop);
    void (*cdda_stop_cb)(void *ctx);
    void *cdda_cb_ctx;
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
static DM2_V1_RuntimeFlyingItemReceipt g_dm2_last_flying_item;
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
static DM2_V1_PerformMoveReceipt g_dm2_last_perform_move;
static DM2_V1_RuntimeFrameOwnershipReceipt g_dm2_frame_ownership;
static DM2_V1_ViewportM11FrameReceipt g_dm2_last_m11_frame;

enum {
    DM2_SKPROJECT_TTY_CHAMPION = 0x0c,
    DM2_SKPROJECT_TTY_MISSILE_0 = 0x1d,
    DM2_SKPROJECT_TTY_MISSILE_1 = 0x1e
};

static uint32_t dm2_v1_runtime_raw_sksave_hash(const uint8_t *data,
                                                size_t size);

static int dm2_runtime_door_state(uint16_t square_raw);
static int dm2_runtime_is_door_at(const DM2_V1_DungeonData *dd,
                                  int level,
                                  int x,
                                  int y,
                                  int raw);

/* skproject/SKULLWIN/c_creature.cpp DM2_PROCEED_CCM reads the door cell in
 * front of the creature through the field runtime before executing the CCM
 * step.  The runtime bridges the creature pool to the live dungeon tile state
 * so door-blocking and open-percent reporting match the visible square. */
static int dm2_runtime_creature_read_door(void *user,
                                          int level,
                                          int x,
                                          int y,
                                          int *out_state,
                                          uint16_t *out_attributes)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dd;
    int raw;
    int state;

    if (!rt || !rt->boot || !rt->boot->dungeon_data ||
        !out_state || !out_attributes) {
        return 0;
    }
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dd, level, x, y);
    if (raw < 0 || !dm2_runtime_is_door_at(dd, level, x, y, raw)) {
        return 0;
    }
    state = dm2_runtime_door_state((uint16_t)raw);
    *out_state = state;
    /* DM2 door attributes relevant to creatures: bit 0 = creatures can see
     * through (ReDMCSB TIMELINE.C/GROUP.C lineage).  The runtime supplies the
     * default (closed doors block nonmaterial creatures unless the attribute
     * says otherwise); full DB0 attribute decoding is future work. */
    *out_attributes = 0;
    (void)level;
    (void)x;
    (void)y;
    return 1;
}

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

/* skproject GAME_LOAD completes its bounded session/timer checks before it
 * publishes the new world. Keep the fallible part shared by direct session
 * apply and raw-SKSave restore, so a bad post-load timer cannot clear live
 * runtime state while an original candidate is still being staged. */
static int dm2_runtime_prepare_session_apply(
    const DM2_V1_SessionState *session,
    DM2_V1_RuntimeTimerPostLoadReceipt *out)
{
    return session && out && dm2_v1_session_validate(session) &&
           dm2_runtime_rebuild_original_timer_owners(session, out);
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

static int dm2_runtime_promote_direct_g1_front_door(
    DM2_V1_RuntimeState *rt)
{
    const DM2_V1_G1DirectDoorRoot *door = NULL;

    if (!rt || !rt->g1_scene_runtime_handoff.blocked ||
        rt->g1_scene_runtime_handoff.scene.root_class !=
            DM2_V1_G1_SCENE_ROOT_DOOR ||
        !dm2_v1_g1_runtime_map_door_at(
            &rt->g1_runtime_map_doors,
            rt->g1_scene_runtime_handoff.scene.x,
            rt->g1_scene_runtime_handoff.scene.y, &door) ||
        !door || !rt->gdat_door_material_plan.valid) {
        return 0;
    }
    /* c_gui_vp.cpp selects the direct DB0 Door before DRAW_DOOR builds the
     * D0C panel.  The already-admitted M11 plan is the only acceptable GDAT
     * material owner for this route; another visible door cannot satisfy it. */
    for (int i = 0; i < rt->gdat_door_material_plan.command_count; ++i) {
        const DM2_V1_GdatDoorOverlayM11Command *command =
            &rt->gdat_door_material_plan.commands[i];
        if (command->kind != DM2_V1_GDAT_DOOR_PANEL ||
            command->view_square != DM2_SQ_D0C ||
            command->gdat_index == 0 || command->width == 0u ||
            command->height == 0u || command->palette_hash == 0u) {
            continue;
        }
        rt->g1_scene_runtime_handoff.blocked = 0;
        rt->g1_scene_runtime_handoff.valid = 1;
        rt->g1_scene_runtime_handoff.gdat_index = command->gdat_index;
        rt->g1_scene_runtime_handoff.creature_type = -1;
        rt->g1_scene_runtime_handoff.material_width = command->width;
        rt->g1_scene_runtime_handoff.material_height = command->height;
        rt->g1_scene_runtime_handoff.material_stride = command->width;
        rt->g1_scene_runtime_handoff.material_palette_hash =
            command->palette_hash;
        return 1;
    }
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
    (void)dm2_runtime_promote_direct_g1_front_door(rt);
}

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

/* Convert a raw DM1/DM2 tile type (or G1 byte-square class) into the
 * DM2_SquareType enum used by movement/planning consumers.
 *
 * The original PC tile encoding places wall at raw 0 and floor at raw 1
 * (ReDMCSB DEFS.H:385-390, HASHBUCKET.C), while the DM2_SquareType enum
 * swaps those two values.  All other types (door, pit, teleporter, etc.)
 * are identical, so only wall and floor need remapping.
 *
 * Source: ReDMCSB DEFS.H:385-390; skproject/SKWIN/DME.h tileTypeIndex. */
static int dm2_runtime_normalize_square_type(int raw_type) {
    switch (raw_type & 0x1f) {
        case 0: return DM2_SQUARE_WALL;
        case 1: return DM2_SQUARE_FLOOR;
        case 2: return DM2_SQUARE_DOOR;
        case 3: return DM2_SQUARE_FLOOR_ORNATE;
        case 4: return DM2_SQUARE_SECRET_DOOR;
        case 5: return DM2_SQUARE_PIT;
        case 6: return DM2_SQUARE_STAIRS_UP;
        case 7: return DM2_SQUARE_STAIRS_DOWN;
        case 8: return DM2_SQUARE_TELEPORTER;
        case 9: return DM2_SQUARE_FAKE_WALL;
        case 10: return DM2_SQUARE_WATER;
        case 11: return DM2_SQUARE_LAVA;
        case 12: return DM2_SQUARE_ASH;
        case 13: return DM2_SQUARE_INACCESSIBLE;
        default: return raw_type & 0x1f;
    }
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

static void dm2_runtime_apply_door_record_metadata(
    DM2_V1_BootProfile *boot, DM2_V1_DungeonData *dd,
    int level,
    int x,
    int y,
    int view_dir,
    const uint8_t *wall_gfx_list,
    int wall_gfx_count,
    const uint8_t *door_gfx_list,
    const uint8_t *door_gfx_active,
    const uint8_t *door_ornate_list,
    int door_ornate_count, uint32_t tick,
    DM2_ViewSquare *door) {
    int thing;
    int door_thing;
    int type = -1;
    int index = -1;
    int size = 0;
    const uint8_t *record;
    uint16_t w2;
    int wall_gfx_index = -1;
    int wall_gfx_field = -1;
    uint8_t animated_field = 0u;
    uint32_t animated_receipt = 0u;

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
    if (door_gfx_list && door_gfx_active &&
        door_gfx_active[door->door_record_type & 1u]) {
        door->door_gfx_index = door_gfx_list[door->door_record_type & 1u];
        door->door_gfx_admitted = 1u;
    }
    /* skproject DRAW_DOOR uses Door::OrnateIndex()-1 as an index into the
     * current map's glbMapDoorOrnatesList before its DOOR_GFX GDAT query. */
    if (door->ornament_index > 0u && door_ornate_list &&
        door->ornament_index <= (uint8_t)door_ornate_count) {
        door->door_ornate_gfx_index =
            door_ornate_list[door->ornament_index - 1u];
    }
    if (!door->door_button &&
        dm2_v1_dungeon_find_text_wall_gfx(dd, (uint16_t)thing,
                                          view_dir, 2, 8,
                                          &wall_gfx_index,
                                          &wall_gfx_field) == 0) {
        door->door_wall_button = 1;
        door->door_wall_button_index = (uint8_t)wall_gfx_index;
        door->door_wall_button_field = (uint8_t)wall_gfx_field;
        door->door_wall_button_x = (int16_t)x;
        door->door_wall_button_y = (int16_t)y;
        door->door_wall_button_object_id = (uint16_t)thing;
    } else if (!door->door_button &&
               dm2_v1_dungeon_resolve_actuator_wall_gfx(
                   dd, (uint16_t)thing, view_dir, 2, 8,
                   wall_gfx_list, wall_gfx_count,
                   &wall_gfx_index, &wall_gfx_field) == 0) {
        door->door_wall_button = 1;
        door->door_wall_button_index = (uint8_t)wall_gfx_index;
        door->door_wall_button_field = (uint8_t)wall_gfx_field;
        door->door_wall_button_x = (int16_t)x;
        door->door_wall_button_y = (int16_t)y;
        door->door_wall_button_object_id = (uint16_t)thing;
    }
    if (door->door_wall_button && boot &&
        dm2_v1_boot_wall_gfx_ornate_animation_field(
            boot, door->door_wall_button_index, tick, 0u,
            &animated_field, &animated_receipt)) {
        /* QUERY_ORNATE_ANIM_FRAME is the sole source of a non-static field.
         * If its selected frame cannot later decode, M11 blocks the command. */
        door->door_wall_button_field = animated_field;
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
    memset(rt->map_door_gfx_active, 0, sizeof(rt->map_door_gfx_active));
    memset(rt->map_door_ornate_list, 0, sizeof(rt->map_door_ornate_list));
    rt->map_door_ornate_count = 0;
    memset(&rt->g1_runtime_map_doors, 0, sizeof(rt->g1_runtime_map_doors));
    if (!rt->boot || !rt->boot->dungeon_data) return;
    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    if (dd->square_bytes == 1) {
        (void)dm2_v1_dungeon_materialize_g1_runtime_map_doors(
            dd, rt->dungeon_level, &rt->g1_runtime_map_doors);
    }
    if (rt->dungeon_level >= 0 && rt->dungeon_level < dd->level_count) {
        if (dd->map_use_door0[rt->dungeon_level]) {
            rt->map_door_gfx_list[0] =
                (uint8_t)(dd->map_door_set0[rt->dungeon_level] & 0xff);
            rt->map_door_gfx_active[0] = 1u;
        }
        if (dd->map_use_door1[rt->dungeon_level]) {
            rt->map_door_gfx_list[1] =
                (uint8_t)(dd->map_door_set1[rt->dungeon_level] & 0xff);
            rt->map_door_gfx_active[1] = 1u;
        }
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
    count = dm2_v1_dungeon_get_map_door_ornate_list(
        dd, rt->dungeon_level, rt->map_door_ornate_list,
        (int)sizeof(rt->map_door_ornate_list));
    if (count > 0) rt->map_door_ornate_count = count;
}

/* The G1 receipts retain decoded pointers, coordinates and source hashes from
 * the currently loaded dungeon bytes.  GAME_LOAD/session restore can replace
 * those bytes, so rebuild every source-owned record-to-GDAT route together.
 * A missing route remains an empty receipt; no previous frame may lend it art. */
static void dm2_runtime_refresh_g1_runtime_materials(DM2_V1_RuntimeState *rt)
{
    const DM2_V1_DungeonData *dungeon;

    if (!rt) return;
    memset(&rt->g1_first_map_runtime, 0,
           sizeof(rt->g1_first_map_runtime));
    memset(&rt->g1_map5_text_runtime, 0,
           sizeof(rt->g1_map5_text_runtime));
    memset(&rt->g1_map5_text_messages_runtime, 0,
           sizeof(rt->g1_map5_text_messages_runtime));
    memset(&rt->g1_map5_gdat_text_messages_runtime, 0,
           sizeof(rt->g1_map5_gdat_text_messages_runtime));
    memset(&rt->g1_map5_text_wall_gfx_runtime, 0,
           sizeof(rt->g1_map5_text_wall_gfx_runtime));
    memset(&rt->g1_actuator_wall_gfx_runtime, 0,
           sizeof(rt->g1_actuator_wall_gfx_runtime));
    memset(&rt->g1_creature_map_chip_runtime, 0,
           sizeof(rt->g1_creature_map_chip_runtime));
    memset(&rt->g1_creature_v5_runtime, 0,
           sizeof(rt->g1_creature_v5_runtime));
    memset(&rt->g1_weapon_map_chip_runtime, 0,
           sizeof(rt->g1_weapon_map_chip_runtime));
    memset(&rt->g1_container_map_chip_runtime, 0,
           sizeof(rt->g1_container_map_chip_runtime));
    memset(rt->g1_static_object_materials, 0,
           sizeof(rt->g1_static_object_materials));
    memset(rt->g1_static_object_source_plans, 0,
           sizeof(rt->g1_static_object_source_plans));
    memset(rt->g1_static_object_delivery_plans, 0,
           sizeof(rt->g1_static_object_delivery_plans));
    rt->g1_static_object_material_count = 0;
    rt->g1_static_object_delivery_plan_count = 0;

    if (!rt->boot || !rt->boot->dungeon_data) return;
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    if (dungeon->square_bytes != 1) return;

    (void)dm2_v1_dungeon_materialize_g1_first_map_runtime(
        dungeon, &rt->g1_first_map_runtime);
    (void)dm2_v1_dungeon_materialize_g1_map5_text_runtime(
        dungeon, &rt->g1_map5_text_runtime);
    if (rt->g1_map5_text_runtime.committed) {
        (void)dm2_v1_dungeon_materialize_g1_map5_text_messages(
            dungeon, &rt->g1_map5_text_runtime,
            &rt->g1_map5_text_messages_runtime);
        (void)dm2_v1_boot_g1_gdat_text_materials(
            rt->boot, &rt->g1_map5_text_runtime,
            &rt->g1_map5_gdat_text_messages_runtime);
        (void)dm2_v1_boot_g1_text_wall_gfx_materials(
            rt->boot, &rt->g1_map5_text_runtime,
            &rt->g1_map5_text_wall_gfx_runtime);
    }
    (void)dm2_v1_boot_g1_actuator_wall_gfx_materials(
        rt->boot, rt->dungeon_level, &rt->g1_actuator_wall_gfx_runtime);
    (void)dm2_v1_boot_g1_creature_map_chip_materials(
        rt->boot, rt->dungeon_level, &rt->g1_creature_map_chip_runtime);
    (void)dm2_v1_boot_g1_weapon_map_chip_materials(
        rt->boot, rt->dungeon_level, &rt->g1_weapon_map_chip_runtime);
    (void)dm2_v1_boot_g1_container_map_chip_materials(
        rt->boot, rt->dungeon_level, &rt->g1_container_map_chip_runtime);
}

static int dm2_runtime_apply_direct_g1_door_metadata(
    const DM2_V1_RuntimeState *rt,
    int x,
    int y,
    DM2_ViewSquare *door)
{
    const DM2_V1_G1DirectDoorRoot *source = NULL;

    if (!rt || !door || !dm2_v1_g1_runtime_map_door_at(
            &rt->g1_runtime_map_doors, x, y, &source) || !source) {
        return 0;
    }
    /* DME.h::Door exposes only these w2 fields.  In particular, do not
     * attempt the old generic next-link walk to find a wall-button owner. */
    door->door_direct_g1_root = 1u;
    door->door_button = source->button;
    door->door_button_state = source->button_state;
    door->door_record_type = source->door_type;
    door->door_opening_dir = source->opening_dir;
    door->ornament_index = source->ornate_index;
    if (source->door_type < 2u &&
        rt->map_door_gfx_active[source->door_type]) {
        door->door_gfx_index = rt->map_door_gfx_list[source->door_type];
        door->door_gfx_admitted = 1u;
    }
    if (source->ornate_index > 0u &&
        source->ornate_index <= (uint8_t)rt->map_door_ornate_count) {
        door->door_ornate_gfx_index =
            rt->map_door_ornate_list[source->ornate_index - 1u];
    }
    return 1;
}

static int dm2_runtime_apply_direct_g1_wall_button_metadata(
    const DM2_V1_RuntimeState *rt,
    int x,
    int y,
    DM2_ViewSquare *door)
{
    int i;

    if (!rt || !door || door->door_button) {
        return 0;
    }
    if (rt->g1_map5_text_wall_gfx_runtime.valid &&
        rt->g1_map5_text_wall_gfx_runtime.map == rt->dungeon_level) {
        const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt =
            &rt->g1_map5_text_wall_gfx_runtime;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1TextWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == x && material->y == y &&
                material->front_image_ready && material->local_palette_hash) {
                door->door_wall_button = 1;
                door->door_wall_button_index = material->wall_gfx_index;
                door->door_wall_button_field = 1u;
                door->door_wall_button_x = (int16_t)x;
                door->door_wall_button_y = (int16_t)y;
                door->door_wall_button_object_id = material->object_id;
                return 1;
            }
        }
    }
    if (rt->g1_actuator_wall_gfx_runtime.valid &&
        rt->g1_actuator_wall_gfx_runtime.map == rt->dungeon_level) {
        const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt =
            &rt->g1_actuator_wall_gfx_runtime;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1ActuatorWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == x && material->y == y &&
                material->front_image_ready && material->local_palette_hash) {
                door->door_wall_button = 1;
                door->door_wall_button_index = material->wall_gfx_index;
                door->door_wall_button_field = 1u;
                door->door_wall_button_x = (int16_t)x;
                door->door_wall_button_y = (int16_t)y;
                door->door_wall_button_object_id = material->object_id;
                return 1;
            }
        }
    }
    return 0;
}

static void dm2_runtime_refresh_gdat_scene_control(DM2_V1_RuntimeState *rt)
{
    DM2_V1_DungeonData *dd;
    DM2_V1_InterfacePalette palette;
    DM2_V1_WeatherGdatReceipt weather_receipt;
    DM2_V1_BootWeatherDestinationReceipt weather_destination;
    DM2_V1_DialogueGdatReceipt dialogue_shell;
    uint32_t scene_flags = 0u;
    uint32_t scene_colorkey = 0u;
    uint32_t ambient_light = 0u;
    uint32_t highest_light_level = 0u;
    uint32_t void_random_fall = 0u;
    uint32_t animated_floor = 0u;
    uint32_t scene_rain = 0u;
    uint32_t misty_map = 0u;
    uint32_t thunder_position = 0u;
    uint32_t ambient_darkness = 0u;

    if (!rt) return;
    /* DistantEnvironment is timer-owned live state.  It is meaningful only
     * for the exact GRAPHICSSET receipt that admitted it; a new source
     * transaction must ask the original timer route to publish fresh slots. */
    memset(rt->weather_distant_slots, 0, sizeof(rt->weather_distant_slots));
    rt->weather_distant_slot_count = 0u;
    rt->weather_distant_slots_map_token = 0u;
    rt->weather_distant_slots_source_receipt_hash = 0u;
    rt->weather_distant_slots_graphicsset = 0u;
    dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
    memset(&rt->gdat_scene_light_receipt, 0,
           sizeof(rt->gdat_scene_light_receipt));
    memset(&rt->c_light_receipt, 0, sizeof(rt->c_light_receipt));
    memset(&rt->c_light_map_descriptor, 0,
           sizeof(rt->c_light_map_descriptor));
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
    rt->gdat_interface_palette_ready = 0;
    rt->gdat_interface_palette_hash = 0u;
    memset(rt->gdat_interface_palette16, 0,
           sizeof(rt->gdat_interface_palette16));
    if (!rt->boot || !rt->boot->dungeon_data) return;

    dd = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    rt->map_graphics_style = dm2_v1_dungeon_get_map_graphics_style(
        dd, rt->dungeon_level);
    if (rt->map_graphics_style < 0) return;
    if (!dm2_v1_dungeon_c_light_map_descriptor_receipt(
            dd, rt->dungeon_level, &rt->c_light_map_descriptor)) {
        return;
    }

    /* skproject/SKWIN/SkWinCore.cpp refreshes glbMapGraphicsSet from
     * Map_definitions::MapGraphicsStyle(), then reads GRAPHICSSET dtWordValue
     * 0x64/0x65/0x67/0x68/0x6A/0x6B for live dungeon rendering. */
    if (!dm2_v1_boot_graphicsset_scene_control(
            rt->boot,
            rt->map_graphics_style,
            &rt->gdat_scene_control_hash,
            &rt->gdat_scene_control_present_mask,
            &rt->gdat_scene_control_query_count,
            &scene_flags,
            &scene_colorkey,
            &ambient_light,
            &highest_light_level,
            &void_random_fall,
            &animated_floor,
            &scene_rain,
            &misty_map,
            &thunder_position,
            &ambient_darkness)) {
        return;
    }
    /* c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT resolves the selected
     * GRAPHICSSET's ceiling/floor IMG3 pair before its light and wall
     * passes.  The control words above authenticate the map identity, but
     * they do not materialize those source images.  Publish the matching
     * plan here so every later receipt and consumer shares that exact GDAT
     * transaction. */
    if (!dm2_v1_boot_gdat_scene_m11_command_plan(
            rt->boot,
            rt->map_graphics_style,
            &rt->gdat_scene_material_plan) ||
        !rt->gdat_scene_material_plan.valid ||
        rt->gdat_scene_material_plan.graphicsset !=
            (uint8_t)rt->map_graphics_style) {
        dm2_v1_gdat_scene_m11_command_plan_free(
            &rt->gdat_scene_material_plan);
        return;
    }
    if (!dm2_v1_gdat_scene_light_m11_receipt(
            &rt->gdat_scene_material_plan, &rt->gdat_scene_light_receipt)) {
        dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
        return;
    }
    if (!rt->c_light_map_descriptor.dynamic_light) {
        /* SKProject c_light.cpp::DM2_RECALC_LIGHT_LEVEL sets a difficulty-0
         * map to level one before the final modifier/clamp.  That fixed branch
         * is owned by the admitted map descriptor.  Dynamic maps instead
         * combine v1e0974, light-producing possessions, spell effects, rain
         * and the source modifiers; no map-only default may stand in for that
         * live state.  Leave such a viewport unpresentable until the complete
         * source state is handed off.
         * Source: SKWINSPX/src/v4/skgame.cpp::RECALC_LIGHT_LEVEL lines
         * 283-362; src/v5/sklight.cpp::DM2_RECALC_LIGHT_LEVEL. */
        DM2_V1_CLightSourceState source;
        memset(&source, 0, sizeof(source));
        source.valid = 1;
        source.dynamic_map = 0u;
        source.base_light = 1u;
        source.darkness_offset = 0u;
        source.source_state_hash = rt->c_light_map_descriptor.descriptor_hash;
        if (!dm2_v1_c_light_m11_receipt_build_for_map(
                &rt->gdat_scene_light_receipt,
                &rt->c_light_map_descriptor,
                &source,
                &rt->c_light_receipt)) {
            dm2_v1_gdat_scene_m11_command_plan_free(
                &rt->gdat_scene_material_plan);
            return;
        }
    }
    if (!dm2_v1_boot_gdat_wall_m11_command_plan(
            rt->boot, rt->map_graphics_style, &rt->gdat_wall_material_plan) ||
        !rt->gdat_wall_material_plan.valid ||
        rt->gdat_wall_material_plan.graphicsset != (uint8_t)rt->map_graphics_style) {
        dm2_v1_gdat_scene_m11_command_plan_free(&rt->gdat_scene_material_plan);
        return;
    }

    /* The old control receipt could fall through to another graphics set.
     * These five source fields are the complete G1 light/scene family and must remain
     * paired with the decoded floor/ceiling pixels from the selected map. */
    rt->gdat_scene_control_ready = 1;
    rt->gdat_scene_control_hash = rt->gdat_scene_material_plan.command_hash;
    rt->gdat_scene_control_present_mask = 0x0fu;
    rt->gdat_scene_control_query_count = 4u;
    rt->gdat_scene_flags = rt->gdat_scene_material_plan.scene_flags;
    rt->gdat_scene_colorkey = rt->gdat_scene_material_plan.scene_colorkey;
    rt->gdat_scene_ambient_light = rt->gdat_scene_material_plan.ambient_light;
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
        uint8_t c_light_palette_darkness = 0u;

        rt->gdat_interface_palette_ready = 1;
        rt->gdat_interface_palette_hash = palette.hash;
        memcpy(rt->gdat_interface_palette16, palette.palette16,
               sizeof(rt->gdat_interface_palette16));
        memcpy(rt->gdat_interface_action_palette16, palette.palette16,
               sizeof(rt->gdat_interface_action_palette16));
        memset(&action_table, 0, sizeof(action_table));
        /* DISPLAY_VIEWPORT uses glbLightLevel * 10, not GRAPHICSSET's
         * HIGHEST_LIGHT_LEVEL control word. Missing live c_light state keeps
         * the action palette unavailable rather than inventing darkness. */
        int c_light_ok = dm2_v1_c_light_m11_palette_darkness(
            &rt->gdat_scene_light_receipt, &rt->c_light_receipt,
            &c_light_palette_darkness);
        int action_table_ok = dm2_v1_boot_interface_action_table(
            rt->boot, &action_table);
        int remap_ok = action_table_ok && dm2_v1_interface_action_table_remap_palette(
            &action_table, rt->gdat_interface_action_palette16, 16u,
            c_light_palette_darkness, -1, -1);
        if (c_light_ok && action_table_ok && remap_ok) {
            rt->gdat_interface_action_palette_ready = 1;
            rt->gdat_interface_action_palette_darkness =
                c_light_palette_darkness;
            rt->gdat_interface_action_palette_hash = action_table.hash;
        }
    }
}

static void dm2_runtime_refresh_music_map_trigger(DM2_V1_RuntimeState *rt)
{
    DM2_V1_RuntimeMusicMapReceipt receipt;
    DM2_V1_MusicQueueReceipt queue;
    int track = -1;

    memset(&receipt, 0, sizeof(receipt));
    receipt.map_index = rt ? rt->dungeon_level : -1;
    receipt.selected_track = -1;
    if (!rt || !rt->boot) return;
    receipt.source_songlist_verified = rt->boot->songlist_verified ? 1 : 0;
    if (!dm2_v1_boot_music_track_for_level(rt->boot, rt->dungeon_level,
                                            0, 0, &track)) {
        rt->music_map_receipt = receipt;
        return;
    }
    receipt.valid = 1;
    receipt.selected_track = track;
    memset(&queue, 0, sizeof(queue));

    if (dm2_v1_platform_music_system(rt->boot->platform) ==
        DM2_MUSIC_SYSTEM_CDDA_COORD) {
        uint8_t *pcm = NULL;
        size_t pcm_size;
        if (rt->cdda_stop_cb)
            rt->cdda_stop_cb(rt->cdda_cb_ctx);
        pcm_size = dm2_v1_boot_load_cdda_track(rt->boot, track, &pcm);
        if (pcm && pcm_size > 0) {
            receipt.queue_result = dm2_v1_sound_queue_cdda(
                pcm, pcm_size, track, 1, &queue);
            receipt.source_stream_resolved = queue.asset_resolved ? 1 : 0;
            free(pcm);
        }
    } else {
        receipt.queue_result = dm2_v1_sound_queue_music(track, 1, &queue);
        receipt.source_stream_resolved = queue.asset_resolved ? 1 : 0;
    }
    /* A queue result establishes neither scheduling nor audible output. */
    receipt.playback_started = 0;
    rt->music_map_receipt = receipt;
}

/* UPDATE_GFXSET, CHECK_RECOMPUTE_LIGHT and c_weather all consume the active
 * map together.  A level handoff cannot retain any material from the prior
 * map: rebuild the map lists, bounded G1 record receipts and GDAT plans as
 * one fail-closed transaction. */
static void dm2_runtime_refresh_map_transition_context(DM2_V1_RuntimeState *rt)
{
    if (!rt) return;
    dm2_runtime_refresh_music_map_trigger(rt);
    dm2_runtime_refresh_map_wall_gfx_list(rt);
    dm2_runtime_refresh_g1_runtime_materials(rt);
    dm2_runtime_refresh_gdat_scene_control(rt);
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
            if (dd->square_bytes == 1) {
                /* The canonical G1 runtime permits only the cached direct
                 * DB0 root receipt first. Some bounded G1 fixtures expose
                 * the same original DB0 through the square first-thing list;
                 * use it only to complete the same real door record metadata,
                 * never to synthesize a generic panel. */
                int direct = dm2_runtime_apply_direct_g1_door_metadata(
                    rt, map_x, map_y, door);
                if (!direct || !door->door_gfx_admitted ||
                    (!door->door_button && !door->door_wall_button)) {
                    dm2_runtime_apply_door_record_metadata(
                        rt->boot, dd, rt->dungeon_level, map_x, map_y, dir,
                        rt->map_wall_gfx_list, rt->map_wall_gfx_count,
                        rt->map_door_gfx_list, rt->map_door_gfx_active,
                        rt->map_door_ornate_list,
                        rt->map_door_ornate_count,
                        (uint32_t)rt->tick_count, door);
                }
                (void)dm2_runtime_apply_direct_g1_wall_button_metadata(
                    rt, map_x, map_y, door);
            } else {
                dm2_runtime_apply_door_record_metadata(
                    rt->boot, dd, rt->dungeon_level, map_x, map_y, dir,
                    rt->map_wall_gfx_list, rt->map_wall_gfx_count,
                    rt->map_door_gfx_list, rt->map_door_gfx_active,
                    rt->map_door_ornate_list,
                    rt->map_door_ornate_count, (uint32_t)rt->tick_count,
                    door);
            }
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
    g_dm2_last_door_render.door_gfx_admitted = door->door_gfx_admitted;
    g_dm2_last_door_render.door_opening_dir = door->door_opening_dir;
    g_dm2_last_door_render.ornament_index = door->ornament_index;
    g_dm2_last_door_render.door_ornate_gfx_index =
        door->door_ornate_gfx_index;
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
    for (int side = 0; side < 2; ++side) {
        g_dm2_last_door_render.side_frame_gdat_index[side] =
            door->side_frame_gdat_index[side];
        g_dm2_last_door_render.side_frame_graphicsset_field[side] =
            door->side_frame_graphicsset_field[side];
        g_dm2_last_door_render.side_frame_rect_number[side] =
            door->side_frame_rect_number[side];
        g_dm2_last_door_render.side_frame_mirror_flip[side] =
            door->side_frame_mirror_flip[side];
        g_dm2_last_door_render.side_frame_offset_x[side] =
            door->side_frame_offset_x[side];
        g_dm2_last_door_render.side_frame_offset_y[side] =
            door->side_frame_offset_y[side];
    }
    g_dm2_last_door_render.button_gdat_index =
        door->button_gdat_index;
    g_dm2_last_door_render.button_source_kind =
        door->button_source_kind;
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
    for (int side = 0; side < 2; ++side) {
        DM2_MIX_DOOR_RECEIPT(door->side_frame_gdat_index[side]);
        DM2_MIX_DOOR_RECEIPT(door->side_frame_graphicsset_field[side]);
        DM2_MIX_DOOR_RECEIPT(door->side_frame_rect_number[side]);
        DM2_MIX_DOOR_RECEIPT(door->side_frame_mirror_flip[side]);
        DM2_MIX_DOOR_RECEIPT(door->side_frame_offset_x[side]);
        DM2_MIX_DOOR_RECEIPT(door->side_frame_offset_y[side]);
    }
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
            if (rt->boot && rt->boot->dungeon_data) {
                const DM2_V1_DungeonData *dungeon =
                    (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
                gs->outdoor = dm2_v1_dungeon_is_outdoor(
                    dungeon, event->target_level);
                rt->outdoor = gs->outdoor;
            }
            dm2_runtime_refresh_map_transition_context(rt);
            break;
        case DM2_TRIGGER_TARGET_SPAWN_CREATURE:
            /* A generic trigger target has no DB14/CCM/timer payload.  The
             * original allocator owns direction, multiplier and record links,
             * so this source-less event cannot create a creature. */
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
            /* Never substitute a fixed creature type for a decoded actuator
             * record.  See DM2_INVOKE_ACTUATOR / CREATE_MINION. */
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
        const DM2_V1_G1DirectTeleporterRoot *teleporter =
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
            dm2_runtime_refresh_map_transition_context(rt);
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
    if (g_dm2_runtime.record_pools_valid) {
        dm2_v1_record_pool_set_free(&g_dm2_runtime.record_pools);
    }
    if (g_dm2_runtime.caii_ready) {
        dm2_v1_caii_array_free(&g_dm2_runtime.caii);
    }
    if (g_dm2_runtime.i18n_ready) {
        dm2_v1_i18n_destroy(&g_dm2_runtime.i18n);
    }
    memset(&g_dm2_runtime, 0, sizeof(g_dm2_runtime));
    memset(&g_dm2_frame_ownership, 0, sizeof(g_dm2_frame_ownership));
    /* DM2-003: source-order timer queue (skproject c_timer.cpp heap). */
    dm2_v1_source_timer_queue_init(&g_dm2_runtime.timer_queue);
    g_dm2_runtime.boot = boot_profile;
    g_dm2_runtime.outdoor = 0;
    g_dm2_runtime.tick_count = 0;
    g_dm2_runtime.move_cooldown_ticks = 0;
    dm2_v1_weather_init(&g_dm2_runtime.weather);
    memset(&g_dm2_runtime.weather_chain, 0,
           sizeof(g_dm2_runtime.weather_chain));
    g_dm2_runtime.weather_rng.random = 0u;
    g_dm2_runtime.weather_chain_started = 0;
    /* skweathr.cpp::DM2_UPDATE_WEATHER derives this from source-owned clock
     * state.  Leave it unknown until an accepted session supplies a value. */
    g_dm2_runtime.time_of_day_minutes = DM2_TIME_UNKNOWN;
    g_dm2_runtime.dungeon_level = 0;
    g_dm2_runtime.view_dir = 0;  /* North */
    g_dm2_runtime.leader_hand_object = 0u;
    memset(g_dm2_runtime.champion_inventory_objects, 0,
           sizeof(g_dm2_runtime.champion_inventory_objects));
    /* GAME_LOAD supplies the squad state.  DUNGEON.DAT supplies the map
     * start, but not Champion::HeroType records; creating a starter party
     * here would make the first HUD frame select portraits by invention.
     * Keep the party absent until an original save or verified new-game
     * handoff publishes source-owned champion records. */
    memset(&g_dm2_runtime.session_snapshot, 0,
           sizeof(g_dm2_runtime.session_snapshot));
    g_dm2_runtime.session_snapshot_valid = 0;
    memset(&g_dm2_runtime.minions, 0, sizeof(g_dm2_runtime.minions));
    g_dm2_runtime.last_npc_level = -1;
    g_dm2_runtime.last_npc_x = -1;
    g_dm2_runtime.last_npc_y = -1;
    /* No merchant/NPC identity exists until an admitted AI-33 DB creature
     * and its source-owned CCM/UI chain have supplied one.  Do not expose a
     * friendly-merchant fixture through the runtime accessor. */
    g_dm2_runtime.last_npc_id = DM2_NPC_NONE;
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
    g_dm2_runtime.move_callback  = NULL;
    g_dm2_runtime.turn_callback  = NULL;
    g_dm2_runtime.stairs_callback = NULL;
    g_dm2_runtime.viewport_asset_fetch = NULL;
    g_dm2_runtime.viewport_asset_user = NULL;
    g_dm2_runtime.last_spell_status_scope = NULL;
    g_dm2_runtime.last_spell_status = NULL;
    g_dm2_runtime.last_spell_failure_class = 0;
    memset(g_dm2_runtime.map_wall_gfx_list, 0,
           sizeof(g_dm2_runtime.map_wall_gfx_list));
    g_dm2_runtime.map_wall_gfx_count = 0;
    g_dm2_runtime.map_graphics_style = -1;
    dm2_runtime_refresh_map_transition_context(&g_dm2_runtime);
    if (boot_profile->dm2_state) {
        DM2_V1_GameState *gs = (DM2_V1_GameState *)boot_profile->dm2_state;
        dm2_runtime_refresh_g1_map0_teleporter_transition(
            &g_dm2_runtime, gs->current_level, gs->party_x, gs->party_y);
    }
    dm2_runtime_refresh_map_wall_gfx_list(&g_dm2_runtime);
    dm2_runtime_refresh_gdat_scene_control(&g_dm2_runtime);
    {
        DM2_V1_CreatureFieldRuntime field_runtime;
        memset(&field_runtime, 0, sizeof(field_runtime));
        field_runtime.read_door = dm2_runtime_creature_read_door;
        field_runtime.user = &g_dm2_runtime;
        dm2_v1_creature_set_field_runtime(&field_runtime);
    }
    /* Bind the GDAT loader to the sound subsystem so QUEUE_NOISE_GEN1/GEN2
     * can resolve sample bindings from the asset database. */
    {
        const DM2_V1_AssetLoader *snd_loader =
            dm2_v1_boot_asset_loader(boot_profile);
        dm2_v1_sound_bind_gdat_loader(snd_loader, snd_loader ? 1 : 0);
    }
    /* Initialize sound queue for QUEUE_NOISE_GEN1/GEN2 runtime routing. */
    dm2_v1_sound_queue_state_init(&g_dm2_runtime.sound_queue,
                                  DM2_V1_SOUND_SSOUND_QUEUE_CAP);
    /* c_sound.cpp queries the current xsndptr2 queue.  Bind the queue just
     * initialised for this admitted runtime; a query must not fall back to a
     * process-global host fixture. */
    dm2_v1_sound_bind_runtime_queue(&g_dm2_runtime.sound_queue);
    memset(&g_dm2_runtime.sound_env, 0, sizeof(g_dm2_runtime.sound_env));
    g_dm2_runtime.sound_env.current_map = (int16_t)g_dm2_runtime.dungeon_level;
    g_dm2_runtime.sound_env.gate_map_a = -1;
    g_dm2_runtime.sound_env.gate_map_b = -1;
    g_dm2_runtime.sound_queue_ready = 1;
    /* A selected FM Towns session must remain bound to its authenticated
     * CD/G1 payload.  The former convenience path reopened a sibling PC
     * GRAPHICS.DAT beneath $HOME and silently overlaid its text.  That mixes
     * releases after M12 has selected a platform and gives an unrelated loose
     * file authority over the live game.  Keep the source FM Towns strings
     * until an explicit language selection carries a separately verified
     * companion corpus all the way through the boot receipt.  Do not infer
     * that companion from a host path.
     *
     * Source ownership: SKProject/SKWINSPX/src/v5/skfileop.cpp media
     * selection precedes GDAT access; M12's selected-media receipt is the
     * Firestaff equivalent. */
    dm2_v1_i18n_init(&g_dm2_runtime.i18n);
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

int dm2_v1_runtime_g1_weapon_map_chip_receipt(
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.g1_weapon_map_chip_runtime.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.g1_weapon_map_chip_runtime;
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

/* The session structure is a decoded fragment of SKProject's SUPPRESS stream,
 * not a public runtime-state format. Keep publication private to the original
 * save-candidate transaction below; callers cannot construct a session and
 * turn it into a playable DM2 state. */
static int dm2_runtime_apply_source_session(const DM2_V1_SessionState *session) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *gs;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_post_load;

    if (!session || !rt->boot || !rt->boot->dm2_state) {
        return -1;
    }
    if (!dm2_runtime_prepare_session_apply(session, &timer_post_load)) {
        return -1;
    }
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;
    rt->session_snapshot = *session;
    rt->session_snapshot_valid = 1;
    rt->timer_post_load = timer_post_load;

    /* skproject SKWINSPX/src/v4/skgame.cpp SELECT_LOAD_GAME and
     * skfileop.cpp READ_SAVEGAMES_FILENAMES route startup resume through a
     * chosen SKSAVE digit after validating the source c_hex2a header.
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
    rt->weather.time_of_day = gs->time_of_day;
    rt->weather.time_fraction =
        (float)gs->time_of_day / (float)DM2_TIME_MINUTES_MAX;
    rt->dungeon_level = gs->current_level;
    rt->view_dir = gs->party_dir;
    dm2_runtime_refresh_map_transition_context(rt);
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
    /* The bounded session model has no proven source owner for its
     * rain_intensity field.  Do not promote it into c_weather's selector or
     * synthesize a clear-weather state on zero; the recovered v1e14xx
     * environment block must bind this later. */
    rt->weather.weather = DM2_WEATHER_UNKNOWN;
    rt->weather.weather_intensity = 0;
    rt->weather.weather_seed = 0u;
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

/* DRAW_MAP_CHIP and QUERY_CREATURE_PICST may consume several creatures in a
 * frame. Keep the renderer's actual GDAT key list separate from the source
 * plan: a valid plan must not let an omitted or substituted later blit hide
 * behind the final creature receipt. */
static int dm2_runtime_creature_drawn_material_identity(
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash,
    int *out_count)
{
    uint32_t hash = 2166136261u;
    int i;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0;
    if (!viewport || !out_hash || !out_count ||
        viewport->asset_creature_drawn_count <= 0 ||
        viewport->creature_material_drawn_count !=
            viewport->asset_creature_drawn_count ||
        viewport->creature_material_drawn_count >
            DM2_MAX_CREATURES_PER_SQ) {
        return 0;
    }
    for (i = 0; i < viewport->creature_material_drawn_count; ++i) {
        int gdat_index = viewport->creature_material_gdat_indices[i];
        if (gdat_index == 0) return 0;
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)gdat_index);
    }
    *out_count = viewport->creature_material_drawn_count;
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* SKWIN routes every missile and cloud through QUERY_DUNGEON_MAP_CHIP_PICT
 * before DRAW_CHIP_OF_MAGIC_MAP. Preserve the renderer's actual GDAT-key
 * order: the final projectile receipt is useful for diagnostics, but cannot
 * stand in for an earlier material in the same presented frame. */
static int dm2_runtime_projectile_drawn_material_identity(
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash,
    int *out_count)
{
    uint32_t hash = 2166136261u;
    int i;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0;
    if (!viewport || !out_hash || !out_count ||
        viewport->asset_projectile_drawn_count <= 0 ||
        viewport->projectile_material_drawn_count !=
            viewport->asset_projectile_drawn_count ||
        viewport->projectile_material_drawn_count > DM2_MAX_PROJECTILES) {
        return 0;
    }
    for (i = 0; i < viewport->projectile_material_drawn_count; ++i) {
        int gdat_index = viewport->projectile_material_gdat_indices[i];
        if (gdat_index == 0) return 0;
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)gdat_index);
    }
    *out_count = viewport->projectile_material_drawn_count;
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* SKWIN's floor objects, creature possessions, and leader hand all reach
 * DRAW_MAP_CHIP or DRAW_ITEM_IN_HAND through a record-owned GDAT category.
 * Preserve both the key and source pass: a final-item diagnostic cannot prove
 * that an earlier floor or possession blit was actually presented. */
static int dm2_runtime_item_drawn_material_identity(
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash,
    int *out_count)
{
    uint32_t hash = 2166136261u;
    int expected_count;
    int i;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0;
    if (!viewport || !out_hash || !out_count) return 0;
    expected_count = viewport->asset_item_drawn_count +
        viewport->asset_creature_possession_item_drawn_count +
        viewport->asset_carried_item_drawn_count;
    if (expected_count <= 0 ||
        viewport->item_material_drawn_count != expected_count ||
        viewport->item_material_drawn_count >
            DM2_MAX_PRESENTED_ITEM_MATERIALS) {
        return 0;
    }
    for (i = 0; i < viewport->item_material_drawn_count; ++i) {
        int gdat_index = viewport->item_material_gdat_indices[i];
        unsigned int source_kind = viewport->item_material_source_kinds[i];
        if (gdat_index == 0 || source_kind < 1u || source_kind > 3u) {
            return 0;
        }
        hash = dm2_runtime_creature_material_plan_step(hash, source_kind);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)gdat_index);
    }
    *out_count = viewport->item_material_drawn_count;
    *out_hash = hash ? hash : 1u;
    return 1;
}

/* DRAW_WALL preflights the complete visible GRAPHICSSET set and records one
 * consumed bit per panel. Rebuild the same source plan here so M11 owns every
 * actual wall material, not just an arbitrary representative cell. */
static int dm2_runtime_wall_drawn_material_identity(
    const DM2_V1_ViewportState *viewport,
    uint32_t *out_hash,
    int *out_count)
{
    DM2_V1_WallPanelRenderPlan plan;
    uint32_t hash = 2166136261u;
    int count = 0;
    int i;

    if (out_hash) *out_hash = 0u;
    if (out_count) *out_count = 0;
    if (!viewport || !out_hash || !out_count ||
        !viewport->source_materials_required ||
        viewport->asset_wall_drawn_count <= 0 ||
        viewport->last_dungeon_wall_material_required_mask == 0u ||
        viewport->last_dungeon_wall_material_required_mask !=
            viewport->last_dungeon_wall_material_consumed_mask ||
        !dm2_v1_viewport_build_wall_panel_render_plan(viewport, &plan)) {
        return 0;
    }
    for (i = 0; i < plan.panel_count; ++i) {
        const DM2_V1_WallPanelRender *panel = &plan.panels[i];
        uint16_t bit;

        if (panel->view_square < 0 || panel->view_square >= 16 ||
            panel->gdat_index == 0) {
            return 0;
        }
        bit = (uint16_t)(1u << (unsigned)panel->view_square);
        if ((viewport->last_dungeon_wall_material_consumed_mask & bit) == 0u) {
            return 0;
        }
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)panel->view_square);
        hash = dm2_runtime_creature_material_plan_step(
            hash, (uint32_t)panel->gdat_index);
        ++count;
    }
    if (count != viewport->asset_wall_drawn_count) return 0;
    *out_count = count;
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
    if (!viewport->last_creature_asset_blit_valid ||
        viewport->last_creature_asset_blit.draw_order !=
            viewport->last_creature_draw_order) {
        /* QUERY_DUNGEON_MAP_CHIP_PICT has no source-independent substitute. */
        g_dm2_last_creature_render.fallback_drawn = 0;
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

    if (!viewport->last_item_asset_blit_valid ||
        viewport->last_item_asset_blit.draw_order !=
            viewport->last_item_draw_order) {
        /* skproject SKWIN/SkWinCore.cpp DRAW_MAP_CHIP consumes item,
         * carried-item and creature-possession map chips through the same
         * QUERY_DUNGEON_MAP_CHIP_PICT path. Missing material is blocked by
         * the viewport; it must not be reported as a rendered fallback. */
        g_dm2_last_item_render.fallback_drawn = 0;
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

    if (!viewport->last_projectile_asset_blit_valid ||
        viewport->last_projectile_asset_blit.draw_order !=
            viewport->last_projectile_draw_order) {
        /* skproject SKWIN/SkWinCore.cpp lines 10672-10750 route missiles
         * and clouds through QUERY_DUNGEON_MAP_CHIP_PICT then
         * DRAW_CHIP_OF_MAGIC_MAP. A missing source chip produces no draw,
         * never a fallback image or a false-positive render receipt. */
        g_dm2_last_projectile_render.fallback_drawn = 0;
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

static void dm2_runtime_populate_creatures(
    DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt;

    if (!rt || !viewport) return;
    memset(&rt->g1_creature_v5_runtime, 0,
           sizeof(rt->g1_creature_v5_runtime));
    if (rt->outdoor) {
        return;
    }
    receipt = &rt->g1_creature_map_chip_runtime;
    if (!receipt->valid || receipt->map != rt->dungeon_level) return;
    rt->g1_creature_v5_runtime.valid = 1;
    rt->g1_creature_v5_runtime.map = rt->dungeon_level;

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
        if (rt->boot &&
            rt->g1_creature_v5_runtime.count < DM2_V1_G1_CREATURE_V5_MAX) {
            /* skproject QUERY_CREATURE_PICST's live non-static route resolves
             * the record's dtImage field through the real FB/FC/FD animation
             * chain (base frame, view-relative direction).  When the exact
             * decoded image evidence exists, the sprite leaves the F9
             * map-chip route; otherwise it keeps the map-chip gate. */
            DM2_V1_BootDynamicCreatureMaterialReceipt v5;
            DM2_V1_G1CreatureV5Material *slot;
            DM2_CreatureSprite *sprite;

            memset(&v5, 0, sizeof(v5));
            if (!dm2_v1_boot_dynamic_creature_material_receipt(
                    rt->boot, material->creature_type, 0, 0xffffu,
                    (party_dir - material->direction) & 3, &v5) ||
                !v5.valid) {
                continue;
            }
            slot = &rt->g1_creature_v5_runtime.materials[
                rt->g1_creature_v5_runtime.count++];
            sprite = &viewport->creatures[viewport->creature_count - 1];
            sprite->source_v5_field = 1;
            sprite->source_material_proven = 1;
            sprite->gdat_image_field = v5.image_field;
            slot->object_id = material->object_id;
            slot->map_x = (int16_t)material->x;
            slot->map_y = (int16_t)material->y;
            slot->creature_type = (uint8_t)material->creature_type;
            slot->image_field = v5.image_field;
            slot->gdat_index = dm2_v1_viewport_creature_field_graphic_index(
                material->creature_type, v5.image_field);
            slot->width = v5.image.decoded_w;
            slot->height = v5.image.decoded_h;
            slot->stride = v5.image.decoded_stride;
            slot->palette_hash = v5.palette_hash;
            slot->decoded_hash = v5.image.decoded_hash;
            slot->raw_material_hash = v5.raw_material_hash;
            slot->raw_material_receipt_hash =
                v5.raw_material_receipt_hash;
        }
    }
}

static const DM2_V1_G1StaticObjectMaterialReceipt *
dm2_runtime_g1_static_object_material_for_object(
    const DM2_V1_RuntimeState *rt, uint16_t object_id)
{
    int k;

    if (!rt || !object_id) return NULL;
    for (k = 0; k < rt->g1_static_object_material_count && k < 48; ++k) {
        const DM2_V1_G1StaticObjectMaterialReceipt *material =
            &rt->g1_static_object_materials[k];
        if (material->selector.valid &&
            material->selector.object_id == object_id) {
            return material;
        }
    }
    return NULL;
}

static void dm2_runtime_admit_static_object_draw_item_material(
    const DM2_V1_RuntimeState *rt, DM2_ItemSprite *dst)
{
    const DM2_V1_G1StaticObjectMaterialReceipt *material;

    if (!rt || !dst) return;
    material = dm2_runtime_g1_static_object_material_for_object(
        rt, dst->object_id);
    if (!material) return;
    /* The record has an admitted static-object delivery plan, so the sprite
     * may carry the DRAW_ITEM image field (F0/F4, never the F9 automap chip),
     * the expanded-clip rect identity, the raw GDAT/clip receipt hashes and
     * the record-owned dtImageOffset.  Objects without that evidence keep the
     * blocked F9 field. */
    dst->source_gdat_field = material->selector.image_field;
    dst->source_static_object_clip_rect_id = material->clip_rect_id;
    dst->source_static_object_raw_gfx256_hash = material->raw_gfx256_hash;
    dst->source_static_object_raw_gfx256_receipt_hash =
        material->raw_gfx256_receipt_hash;
    dst->source_static_object_raw4_hash = material->raw4_hash;
    dst->source_static_object_raw4_receipt_hash =
        material->raw4_receipt_hash;
    dst->source_static_object_image_offset = material->selector.image_offset;
}

static void dm2_runtime_populate_g1_weapon_map_chip_items(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt;

    if (!rt || !viewport || rt->outdoor) return;
    receipt = &rt->g1_weapon_map_chip_runtime;
    if (!receipt->valid || receipt->map != rt->dungeon_level) return;

    /* skproject SkWinCore.cpp::DRAW_MAP_CHIP selects DB5 Weapon::ItemType
     * through QUERY_DUNGEON_MAP_CHIP_PICT(..., F9).  These receipt rows are
     * the only floor objects allowed to enter this direct G1 render path. */
    for (int i = 0; i < receipt->material_count &&
                    viewport->item_count < DM2_MAX_ITEMS_PER_SQ; ++i) {
        const DM2_V1_G1WeaponMapChipMaterial *material =
            &receipt->materials[i];
        DM2_V1_ViewportSpritePlacement placement;
        DM2_ItemSprite *dst;
        int source_cell;
        int source_pass;

        if (!dm2_v1_viewport_project_map_to_sprite(
                material->x, material->y, party_dir, party_x, party_y,
                &placement)) {
            continue;
        }
        if (!dm2_v1_viewport_static_object_cell_for_map(
                material->x, material->y, party_dir, party_x, party_y,
                &source_cell, &source_pass)) {
            /* The old bounded projection was not SKProject DRAW_ITEM
             * geometry.  Do not place a real DB5 bitmap until its physical
             * cell has an observed DM2_DRAW_STATIC_OBJECT table route. */
            continue;
        }
        dst = &viewport->items[viewport->item_count++];
        memset(dst, 0, sizeof(*dst));
        dst->item_category = 0x10; /* skproject dbWeapon -> WEAPONS */
        dst->item_type = material->item_type;
        dst->frame_index = 0;
        dst->depth = (int16_t)placement.depth;
        dst->screen_x = (int16_t)placement.screen_x;
        dst->screen_y = (int16_t)placement.screen_y;
        dst->direction = material->direction;
        dst->object_id = material->object_id;
        dst->map_x = (int16_t)material->x;
        dst->map_y = (int16_t)material->y;
        dst->source_gdat_field = 0xf9;
        dst->source_g1_weapon = 1;
        dst->source_static_object_admitted = 1;
        dst->source_static_object_cell = (uint8_t)source_cell;
        dst->source_static_object_pass = (int8_t)source_pass;
        dm2_runtime_admit_static_object_draw_item_material(rt, dst);
    }
}

static uint32_t dm2_runtime_static_object_visibility_mask_5x5(
    const DM2_V1_G1RuntimeMapWeaponReceipt *weapons,
    const DM2_V1_G1RuntimeMapContainerReceipt *containers,
    int x, int y, int party_dir)
{
    /* SKWIN/SkWinCore.cpp lines 45361-45370: the per-cell 5x5 visibility mask
     * ((*_4976_5be2)[cellPos]) ORs 1 << QUERY_OBJECT_5x5_POS(record, view_dir)
     * for every dbWeapon..dbMiscellaneous_item record on the square.  Only the
     * declared direct G1 DB5/DB9 roots contribute; their positions and
     * directions are record-owned real game data.  The source additionally
     * gates the bit on the tile state (xsrd.w0/w6[0]); tile-state ownership
     * stays with the dungeon materialization that admitted these roots. */
    uint32_t mask = 0u;
    int j;

    for (j = 0; j < weapons->weapon_root_count; ++j) {
        if (weapons->weapons[j].x == x && weapons->weapons[j].y == y) {
            mask |= dm2_v1_viewport_static_object_visibility_bit(
                weapons->weapons[j].direction, party_dir);
        }
    }
    for (j = 0; j < containers->container_root_count; ++j) {
        if (containers->containers[j].x == x && containers->containers[j].y == y) {
            mask |= dm2_v1_viewport_static_object_visibility_bit(
                containers->containers[j].direction, party_dir);
        }
    }
    return mask;
}

static void dm2_runtime_populate_g1_static_object_materials(
    DM2_V1_RuntimeState *rt, int party_dir, int party_x, int party_y)
{
    const DM2_V1_DungeonData *dungeon;
    DM2_V1_G1RuntimeMapWeaponReceipt weapons;
    DM2_V1_G1RuntimeMapContainerReceipt containers;
    uint32_t session_identity;
    const uint8_t *rect14_rows = NULL;
    uint32_t rect14_row_count = 0;
    uint32_t rect14_hash = 0;
    int rect14_table_ready;

    if (!rt || !rt->boot || rt->outdoor || !rt->boot->dungeon_data ||
        !rt->session_snapshot_valid) return;
    session_identity = dm2_v1_runtime_dm2_viewport_session_identity(
        &rt->session_snapshot);
    if (!session_identity) return;
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    rect14_table_ready =
        dm2_v1_boot_interface_rect14_table(rt->boot, &rect14_rows,
                                           &rect14_row_count, &rect14_hash) &&
        rect14_rows != NULL && rect14_row_count > 0u && rect14_hash != 0u;
    memset(&weapons, 0, sizeof(weapons));
    memset(&containers, 0, sizeof(containers));
    if (!dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
            dungeon, rt->dungeon_level, &weapons) || !weapons.committed ||
        !dm2_v1_dungeon_materialize_g1_runtime_map_containers(
            dungeon, rt->dungeon_level, &containers) || !containers.committed) return;
    for (int pass = 0; pass < 2; ++pass) {
        int count = pass == 0 ? weapons.weapon_root_count : containers.container_root_count;
        for (int i = 0; i < count && rt->g1_static_object_material_count < 48; ++i) {
            DM2_V1_G1StaticObjectMaterialSelector selector;
            DM2_V1_StaticObjectSourcePlan plan;
            DM2_V1_G1StaticObjectMaterialReceipt receipt;
            int x = pass == 0 ? weapons.weapons[i].x : containers.containers[i].x;
            int y = pass == 0 ? weapons.weapons[i].y : containers.containers[i].y;
            int cell, source_pass;
            if (!dm2_v1_viewport_static_object_cell_for_map(x, y, party_dir,
                    party_x, party_y, &cell, &source_pass) ||
                !(pass == 0 ? dm2_v1_boot_g1_static_weapon_selector(rt->boot,
                               &weapons.weapons[i], &selector) :
                              dm2_v1_boot_g1_static_container_selector(rt->boot,
                               &containers.containers[i], &selector)) ||
                !dm2_v1_viewport_static_object_source_plan(cell, source_pass,
                    selector.category, selector.direction, selector.container_open,
                    0, party_dir, 1u,
                    dm2_runtime_static_object_visibility_mask_5x5(
                        &weapons, &containers, x, y, party_dir),
                    &plan)) continue;
            /* draw_slot 0 and record_list_ordinal 1 are proven, not assumed:
             * the materializers only admit each tile's square-first-thing
             * chain head, and DRAW_PUT_DOWN_ITEM's chain walk draws the head
             * of a matching direction group first (si == 0). */
            /* When the real INTERFACE_GENERAL dt07/0x0A Rect14 table is present,
             * bind the matching row to this static-object plan.  A missing row
             * leaves the plan in its existing source-geometry state; it does
             * not synthesize placement data. */
            if (rect14_table_ready) {
                (void)dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
                    rect14_rows, rect14_row_count, rect14_hash,
                    selector.direction, party_dir, &plan);
            }
            if (!dm2_v1_boot_g1_static_object_material_receipt(rt->boot,
                    &selector, (uint16_t)(plan.clip_rect_id & 0x7fffu), &receipt)) continue;
            if (!dm2_v1_viewport_build_static_object_m11_delivery_plan(
                    &receipt, &plan, session_identity,
                    &rt->g1_static_object_delivery_plans[
                        rt->g1_static_object_delivery_plan_count])) continue;
            rt->g1_static_object_materials[rt->g1_static_object_material_count] = receipt;
            rt->g1_static_object_source_plans[rt->g1_static_object_material_count] = plan;
            ++rt->g1_static_object_material_count;
            ++rt->g1_static_object_delivery_plan_count;
        }
    }
}

static void dm2_runtime_populate_g1_container_map_chip_items(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport,
    int party_dir,
    int party_x,
    int party_y)
{
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt;

    if (!rt || !viewport || rt->outdoor) return;
    receipt = &rt->g1_container_map_chip_runtime;
    if (!receipt->valid || receipt->map != rt->dungeon_level) return;

    /* skproject DRAW_MAP_CHIP reaches DB9 Container through the direct G1
     * map root. ContainerType is the only admitted class-2 selector; its
     * contained-object link is deliberately not traversed. */
    for (int i = 0; i < receipt->material_count &&
                    viewport->item_count < DM2_MAX_ITEMS_PER_SQ; ++i) {
        const DM2_V1_G1ContainerMapChipMaterial *material =
            &receipt->materials[i];
        DM2_V1_ViewportSpritePlacement placement;
        DM2_ItemSprite *dst;
        int source_cell;
        int source_pass;

        if (!dm2_v1_viewport_project_map_to_sprite(
                material->x, material->y, party_dir, party_x, party_y,
                &placement)) {
            continue;
        }
        if (!dm2_v1_viewport_static_object_cell_for_map(
                material->x, material->y, party_dir, party_x, party_y,
                &source_cell, &source_pass)) {
            /* See the DB5 path above: side/deep DRAW_ITEM placement has no
             * recovered source table and must remain unavailable. */
            continue;
        }
        dst = &viewport->items[viewport->item_count++];
        memset(dst, 0, sizeof(*dst));
        dst->item_category = 0x14; /* skproject dbContainer -> CONTAINERS */
        dst->item_type = material->container_type;
        dst->depth = (int16_t)placement.depth;
        dst->screen_x = (int16_t)placement.screen_x;
        dst->screen_y = (int16_t)placement.screen_y;
        dst->direction = material->direction;
        dst->object_id = material->object_id;
        dst->map_x = (int16_t)material->x;
        dst->map_y = (int16_t)material->y;
        dst->source_gdat_field = 0xf9;
        dst->source_g1_container = 1;
        dst->source_static_object_admitted = 1;
        dst->source_static_object_cell = (uint8_t)source_cell;
        dst->source_static_object_pass = (int8_t)source_pass;
        dm2_runtime_admit_static_object_draw_item_material(rt, dst);
    }
}

static uint32_t dm2_runtime_indexed_pixel_hash(const uint8_t *pixels,
                                               int width,
                                               int height,
                                               int stride)
{
    uint32_t hash = 2166136261u;

    if (!pixels || width <= 0 || height <= 0 || stride < width) return 0u;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            hash ^= pixels[y * stride + x];
            hash *= 16777619u;
        }
    }
    return hash ? hash : 1u;
}

static void dm2_runtime_bind_g1_scene_static_item_materials(
    const DM2_V1_RuntimeState *rt,
    DM2_V1_ViewportState *viewport)
{
    DM2_V1_G1SceneStaticItemMaterial
        materials[DM2_V1_G1_SCENE_STATIC_ITEM_MATERIAL_MAX];
    int material_count = 0;

    if (!rt || !viewport || !rt->viewport_asset_fetch ||
        !rt->viewport_asset_palette_fetch) {
        dm2_v1_viewport_set_g1_scene_item_material_direct(
            viewport, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0u, 0u);
        dm2_v1_viewport_set_g1_scene_static_item_materials_direct(
            viewport, NULL, 0);
        return;
    }
    memset(materials, 0, sizeof(materials));
    for (int i = 0; i < viewport->item_count; ++i) {
        const DM2_ItemSprite *candidate = &viewport->items[i];
        if ((candidate->source_g1_weapon || candidate->source_g1_container) &&
            candidate->source_static_object_admitted) {
            const DM2_V1_G1StaticObjectMaterialReceipt *material;
            const uint8_t *pixels = NULL;
            uint8_t palette16[16];
            int width = 0;
            int height = 0;
            int stride = 0;
            uint32_t palette_hash = 0u;
            uint32_t pixel_hash;
            int gdat_index;

            if (material_count >= DM2_V1_G1_SCENE_STATIC_ITEM_MATERIAL_MAX ||
                candidate->source_gdat_field == 0xf9u) goto reject;
            gdat_index = dm2_v1_viewport_item_graphic_index(
                candidate->item_category, candidate->item_type,
                candidate->source_gdat_field);
            if (gdat_index == 0 ||
                rt->viewport_asset_fetch(rt->viewport_asset_user, gdat_index,
                                         &pixels, &width, &height, &stride) != 0 ||
                !pixels || width <= 0 || height <= 0 || stride < width ||
                rt->viewport_asset_palette_fetch(rt->viewport_asset_palette_user,
                                                 gdat_index, palette16,
                                                 &palette_hash) != 0 ||
                palette_hash == 0u) goto reject;
            pixel_hash = dm2_runtime_indexed_pixel_hash(
                pixels, width, height, stride);
        /* DRAW_ITEM route: the F0/F4 image is not the F9 map chip, so the
         * map-chip instance receipt does not apply.  The sprite must instead
         * carry the admitted static-object material receipt with matching
         * raw identities before its decoded GDAT image may be bound. */
            material = dm2_runtime_g1_static_object_material_for_object(
                rt, candidate->object_id);
            if (!material || pixel_hash == 0u ||
                material->selector.category != candidate->item_category ||
                material->selector.item_type != candidate->item_type ||
                material->selector.image_field != candidate->source_gdat_field ||
                material->clip_rect_id !=
                    candidate->source_static_object_clip_rect_id ||
                material->raw_gfx256_hash !=
                    candidate->source_static_object_raw_gfx256_hash ||
                material->raw_gfx256_receipt_hash !=
                    candidate->source_static_object_raw_gfx256_receipt_hash ||
                material->raw4_hash != candidate->source_static_object_raw4_hash ||
                material->raw4_receipt_hash !=
                    candidate->source_static_object_raw4_receipt_hash) goto reject;
            materials[material_count].ready = 1;
            materials[material_count].item_category = candidate->item_category;
            materials[material_count].item_type = candidate->item_type;
            materials[material_count].gdat_index = gdat_index;
            materials[material_count].object_id = candidate->object_id;
            materials[material_count].map_x = candidate->map_x;
            materials[material_count].map_y = candidate->map_y;
            materials[material_count].width = width;
            materials[material_count].height = height;
            materials[material_count].stride = stride;
            materials[material_count].pixels = pixels;
            materials[material_count].pixel_hash = pixel_hash;
            memcpy(materials[material_count].palette16, palette16,
                   sizeof(palette16));
            materials[material_count].palette_hash = palette_hash;
            materials[material_count].raw_gfx256_hash = material->raw_gfx256_hash;
            materials[material_count].raw_gfx256_receipt_hash =
                material->raw_gfx256_receipt_hash;
            materials[material_count].raw4_hash = material->raw4_hash;
            materials[material_count].raw4_receipt_hash =
                material->raw4_receipt_hash;
            ++material_count;
        }
    }
    dm2_v1_viewport_set_g1_scene_item_material_direct(
        viewport, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0u, 0u);
    dm2_v1_viewport_set_g1_scene_static_item_materials_direct(
        viewport, materials, material_count);
    return;

reject:
    dm2_v1_viewport_set_g1_scene_item_material_direct(
        viewport, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0u, 0u);
    dm2_v1_viewport_set_g1_scene_static_item_materials_direct(
        viewport, NULL, 0);
}

static int dm2_runtime_g1_wall_button_receipt_matches(
    const DM2_V1_RuntimeState *rt, const DM2_V1_DoorRender *door,
    int width, int height, uint32_t palette_hash, uint16_t *out_raw_index,
    const uint8_t **out_raw_bytes, size_t *out_raw_byte_count,
    uint32_t *out_raw_hash, uint32_t *out_raw_receipt_hash)
{
    int i;

    if (out_raw_index) *out_raw_index = 0u;
    if (out_raw_bytes) *out_raw_bytes = NULL;
    if (out_raw_byte_count) *out_raw_byte_count = 0u;
    if (out_raw_hash) *out_raw_hash = 0u;
    if (out_raw_receipt_hash) *out_raw_receipt_hash = 0u;
    if (!rt || !door || door->button_source_kind != 2 ||
        door->wall_button_field != 1 || width <= 0 || height <= 0 ||
        palette_hash == 0u) {
        return 0;
    }
    if (rt->g1_map5_text_wall_gfx_runtime.valid &&
        rt->g1_map5_text_wall_gfx_runtime.map == rt->dungeon_level) {
        const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt =
            &rt->g1_map5_text_wall_gfx_runtime;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1TextWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index &&
                material->front_image_ready &&
                material->front_image_width == (uint16_t)width &&
                material->front_image_height == (uint16_t)height &&
                material->local_palette_hash == palette_hash &&
                material->raw_material_bytes &&
                material->raw_material_byte_count != 0u &&
                material->raw_material_hash != 0u &&
                material->raw_material_receipt_hash != 0u) {
                if (out_raw_index) *out_raw_index = material->raw_material_index;
                if (out_raw_bytes) *out_raw_bytes = material->raw_material_bytes;
                if (out_raw_byte_count) *out_raw_byte_count = material->raw_material_byte_count;
                if (out_raw_hash) *out_raw_hash = material->raw_material_hash;
                if (out_raw_receipt_hash) *out_raw_receipt_hash = material->raw_material_receipt_hash;
                return 1;
            }
        }
    }
    if (rt->g1_actuator_wall_gfx_runtime.valid &&
        rt->g1_actuator_wall_gfx_runtime.map == rt->dungeon_level) {
        const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt =
            &rt->g1_actuator_wall_gfx_runtime;
        for (i = 0; i < receipt->material_count; ++i) {
            const DM2_V1_G1ActuatorWallGfxMaterial *material =
                &receipt->materials[i];
            if (material->x == door->wall_button_x &&
                material->y == door->wall_button_y &&
                material->object_id == door->wall_button_object_id &&
                material->wall_gfx_index == (uint8_t)door->wall_button_index &&
                material->front_image_ready &&
                material->front_image_width == (uint16_t)width &&
                material->front_image_height == (uint16_t)height &&
                material->local_palette_hash == palette_hash &&
                material->raw_material_bytes &&
                material->raw_material_byte_count != 0u &&
                material->raw_material_hash != 0u &&
                material->raw_material_receipt_hash != 0u) {
                if (out_raw_index) *out_raw_index = material->raw_material_index;
                if (out_raw_bytes) *out_raw_bytes = material->raw_material_bytes;
                if (out_raw_byte_count) *out_raw_byte_count = material->raw_material_byte_count;
                if (out_raw_hash) *out_raw_hash = material->raw_material_hash;
                if (out_raw_receipt_hash) *out_raw_receipt_hash = material->raw_material_receipt_hash;
                return 1;
            }
        }
    }
    return 0;
}

/* SKProject DRAW_DEFAULT_DOOR_BUTTON consumes a custom WALL_GFX field-1
 * surface after DB2/DB3 has selected the root. Carry that single verified
 * surface through M11; a missing receipt deliberately leaves the normal
 * source plan to no-draw rather than substituting another door image. */
static void dm2_runtime_bind_g1_scene_wall_button_material(
    const DM2_V1_RuntimeState *rt, DM2_V1_ViewportState *viewport,
    const DM2_V1_DoorRenderPlan *plan)
{
    const DM2_V1_DoorRender *door = NULL;
    const uint8_t *pixels = NULL;
    uint8_t palette16[16];
    int width = 0;
    int height = 0;
    int stride = 0;
    int gdat_index;
    uint32_t palette_hash = 0u;
    uint32_t pixel_hash;
    uint16_t raw_index = 0u;
    const uint8_t *raw_bytes = NULL;
    size_t raw_byte_count = 0u;
    uint32_t raw_hash = 0u;
    uint32_t raw_receipt_hash = 0u;

    if (!rt || !viewport || !plan ||
        rt->viewport_asset_fetch != dm2_v1_boot_viewport_asset_fetch ||
        !rt->viewport_asset_user || !rt->viewport_asset_palette_fetch) {
        goto clear;
    }
    for (int i = 0; i < plan->door_count; ++i) {
        if (plan->doors[i].button_source_kind == 2) {
            door = &plan->doors[i];
            break;
        }
    }
    if (!door || door->wall_button_field != 1 ||
        door->wall_button_index < 0 || door->wall_button_index > 0xff) {
        goto clear;
    }
    gdat_index = dm2_v1_viewport_wall_button_graphic_index(
        door->wall_button_index, door->wall_button_field);
    if (gdat_index == 0 || gdat_index != door->button_gdat_index ||
        rt->viewport_asset_fetch(rt->viewport_asset_user, gdat_index,
                                 &pixels, &width, &height, &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width ||
        rt->viewport_asset_palette_fetch(rt->viewport_asset_palette_user,
                                         gdat_index, palette16,
                                         &palette_hash) != 0 ||
        !dm2_runtime_g1_wall_button_receipt_matches(
            rt, door, width, height, palette_hash, &raw_index, &raw_bytes,
            &raw_byte_count, &raw_hash, &raw_receipt_hash)) {
        goto clear;
    }
    pixel_hash = dm2_runtime_indexed_pixel_hash(pixels, width, height, stride);
    dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
        viewport, 1, gdat_index, door->wall_button_index,
        door->wall_button_field, door->wall_button_x, door->wall_button_y,
        door->wall_button_object_id, pixels, width, height, stride,
        palette16, palette_hash, pixel_hash, raw_index, raw_bytes,
        raw_byte_count, raw_hash, raw_receipt_hash);
    return;

clear:
    dm2_v1_viewport_set_g1_scene_wall_button_material_direct(
        viewport, 0, 0, 0, 0, 0, 0, 0, NULL, 0, 0, 0, NULL, 0u, 0u,
        0u, NULL, 0u, 0u, 0u);
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
 * dm2_runtime_ensure_think_binding — lazily populate the session-owned
 * DM2-002 record pools from the boot dungeon data and bind the per-cell
 * DM2_THINK_CREATURE dispatch (DM2-003/005 follow-up).
 *
 * The pool set copies the exact G1 source spans once the loader's
 * candidate evidence validates (dm2_v1_record_pool_set_init_from_dungeon);
 * without validated evidence the binding stays unready and 0x21/0x22
 * timers are acknowledged fail-closed by the dispatcher, never simulated.
 */
/* dm2_runtime_delete_creature_full — production wiring of the 0fcb
 * branch (c_1c9a.cpp:5956-5957) to the COMPLETE
 * DM2_DELETE_CREATURE_RECORD composition.  Mirrors the source call
 * DM2_DELETE_CREATURE_RECORD(x, y, 0, 1): mode 0, noise arg 1.  The
 * dungeon cast matches the hook contract (the composition's
 * ground-stack writes land in the dungeon data exactly like the
 * source's map state; the runtime session owns the boot dungeon
 * mutably).  GDAT drop slots are passed only when the session loaded
 * them for the creature's type; the generated-drops part is otherwise
 * skipped (receipted). */
static int dm2_runtime_delete_creature_full(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int x, int y,
    void *context) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    uint16_t drop_slots[DM2_DROP_SLOT_COUNT];
    const uint16_t *drop_slots_arg = 0;
    int16_t handle;
    int creature_type;

    handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
    if (handle != DM2_V1_RECORD_HANDLE_NULL) {
        const uint8_t *record =
            dm2_v1_record_pool_address(pool_set, handle);
        if (record != 0) {
            creature_type = (int)record[4];
            if (dm2_v1_creature_drop_slots_loaded(creature_type) == 1) {
                int i;
                for (i = 0; i < DM2_DROP_SLOT_COUNT; ++i) {
                    drop_slots[i] =
                        dm2_v1_creature_drop_slot_word(creature_type, i);
                }
                drop_slots_arg = drop_slots;
            }
        }
    }

    rt->last_delete_full_valid = 0;
    memset(&rt->last_delete_full, 0, sizeof(rt->last_delete_full));
    {
        int result = dm2_v1_delete_creature_record_full(
            pool_set, dungeon, caii, queue, &rt->drop_rng,
            0, (unsigned long)rt->tick_count, x, y, 0, 1,
            dm2_v1_runtime_get_party_x(),
            dm2_v1_runtime_get_party_y(),
            dm2_v1_runtime_get_party_dir(),
            drop_slots_arg, &rt->last_delete_full);
        rt->last_delete_full_valid = 1;
        return result;
    }
}

/*
 * dm2_runtime_think_body — creature AI body with CCM loop invocation.
 * Source: c_ai.cpp:5649-5999 DM2_THINK_CREATURE.
 *
 * When the asset loader and CAII are available, invokes the CCM message
 * loop (DM2_13e4_0982) which runs the creature's AI behavior script.
 * The CCM loop owns animation, movement commands and timer re-queue.  An
 * incomplete CCM handoff consumes the dispatched timer without mutation;
 * re-queuing a creature from its coordinate alone would invent source state.
 */
static int dm2_runtime_think_body(
    void *context,
    int16_t creature_record,
    const DM2_V1_SourceTimer *timer,
    int map, int x, int y, int think_type)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    const DM2_V1_AssetLoader *loader;

    if (!rt || !timer) return 0;
    (void)x;
    (void)y;
    (void)think_type;

    loader = rt->boot ? dm2_v1_boot_asset_loader(rt->boot) : NULL;

    /* Try the CCM message loop when all prerequisites are available. */
    if (loader && rt->caii_ready && rt->record_pools_valid) {
        DM2_V1_CcmLoopReceipt ccm_receipt;
        DM2_V1_SourceTimer timer_copy = *timer;
        int16_t adj[2] = {0, 0};
        const uint8_t *anim_row = NULL;
        int v1e0584 = -1;
        uint8_t *rec;
        uint8_t *slot;

        rec = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                              creature_record);
        if (rec && rec[5] != 0xffu &&
            (int)rec[5] < rt->caii.capacity) {
            slot = rt->caii.slots +
                   (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;
            /* Extract adj pair from CAII slot at offset 0x0e-0x11 */
            adj[0] = (int16_t)((uint16_t)slot[0x0e] |
                               ((uint16_t)slot[0x0f] << 8));
            adj[1] = (int16_t)((uint16_t)slot[0x10] |
                               ((uint16_t)slot[0x11] << 8));

            memset(&ccm_receipt, 0, sizeof(ccm_receipt));
            if (dm2_v1_ccm_message_loop(
                    &rt->record_pools,
                    &rt->caii,
                    &rt->timer_queue,
                    loader,
                    &rt->drop_rng,
                    creature_record,
                    &timer_copy,
                    0,
                    adj,
                    &anim_row,
                    &v1e0584,
                    0,
                    (int16_t)map,
                    0,
                    rt->dungeon_level,
                    rt->dungeon_level,
                    (int32_t)rt->view_dir,
                    (unsigned long)rt->tick_count,
                    NULL, NULL, NULL, NULL, NULL, NULL,
                    &ccm_receipt) == 1 && ccm_receipt.valid) {
                /* CCM loop handled the creature — it re-queued the
                 * timer internally via ccm_requeue_tail. Write back
                 * the updated adj pair to the CAII slot. */
                slot[0x0e] = (uint8_t)(adj[0] & 0xff);
                slot[0x0f] = (uint8_t)((uint16_t)adj[0] >> 8);
                slot[0x10] = (uint8_t)(adj[1] & 0xff);
                slot[0x11] = (uint8_t)((uint16_t)adj[1] >> 8);
                return 1;
            }
        }
    }

    /* SKProject's DM2_THINK_CREATURE re-queue belongs to the completed CCM
     * execution.  There is no source-authorized coordinate-only retry path:
     * acknowledge this timer while preserving the record pools and queue. */
    return 0;
}

static void dm2_runtime_ensure_think_binding(DM2_V1_RuntimeState *rt) {
    const DM2_V1_DungeonData *dungeon;

    if (rt->think_binding_ready) {
        return;
    }
    if (!rt->boot || !rt->boot->dungeon_data) {
        return;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    if (!rt->record_pools_valid) {
        if (!dm2_v1_record_pool_set_init_from_dungeon(&rt->record_pools,
                                                      dungeon)) {
            return;
        }
        rt->record_pools_valid = 1;
    }
    dm2_v1_think_creature_binding_init(&rt->think_binding,
                                       &rt->record_pools, dungeon);
    /* The CAII module's AI-spec gates (0fcb record-delete flag,
     * ATTACK_CREATURE vl_18 gate) resolve through the proven GDAT
     * extended-mode provider owned by the creature module. */
    dm2_v1_caii_set_ai_spec_flags_fn(dm2_v1_creature_ai_spec_flags);
    /* ATTACK_CREATURE's aggro BaseHP probe (c_creature.cpp:420-423) and
     * the table1d607e GDAT word@1 index (c_creature.cpp:441 + 612,
     * c_record.cpp:1387) get the same proven provenance. */
    dm2_v1_caii_set_ai_base_hp_fn(dm2_v1_creature_ai_base_hp);
    dm2_v1_caii_set_gdat_word1_fn(dm2_v1_creature_gdat_word1);
    /* The 0fcb branch (c_1c9a.cpp:5956-5957) runs the COMPLETE
     * DM2_DELETE_CREATURE_RECORD composition through the session-owned
     * hook. */
    dm2_v1_drops_rng_init(&rt->drop_rng);
    dm2_v1_caii_set_delete_creature_full_fn(
        dm2_runtime_delete_creature_full, rt);
    rt->think_binding.think_body = dm2_runtime_think_body;
    rt->think_binding.think_body_context = rt;
    rt->think_binding_ready = 1;
}

/*
 * dm2_runtime_think_creature_timer — DM2-owned 0x21/0x22 handler for the
 * source-order timer dispatcher (DM2-003/005 follow-up).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4079-4088 dispatches timer types
 * 0x21/0x22 to c_ai.cpp DM2_THINK_CREATURE(xA, yA, type), which resolves
 * the creature record AT THE TIMER CELL via DM2_GET_CREATURE_AT
 * (c_querydb.cpp:1486-1507) over the DM2-002 record pool.  The former
 * unconditional CCM-instance step is retired: per-cell resolution now
 * runs against the session-owned record pools, and the think body stays
 * unbound (receipted fail-closed) until the CCM stream owner/grammar is
 * proven.  The timer is consumed exactly like the source's early return
 * when the cell holds no creature.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4079-4088 (0x21/0x22 dispatch)
 *         skproject/SKULLWIN/c_ai.cpp:5649-5677   (DM2_THINK_CREATURE)
 *         skproject/SKULLWIN/c_querydb.cpp:1486-1507 (DM2_GET_CREATURE_AT)
 */

/*
 * dm2_runtime_destroy_door_timer — DM2-owned 0x02 handler.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:422-440 DM2_PROCESS_TIMER_DESTROY_DOOR:
 * sets the tile square's lower 3 bits to 5 (DESTROYED state) by
 * masking byte with 0xf8 | 0x05.  Also checks if current map matches
 * ddat.v1e0266 to set ddat.v1e0390.l_00 = 3 (viewport redraw flag).
 *
 * Bounded slice: reads the tile byte, applies the bit mutation, writes
 * back.  The viewport redraw flag is tracked as a counter.
 */
static int dm2_runtime_destroy_door_timer(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int x, y;
    uint16_t raw;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt->boot || !rt->boot->dungeon_data)
        return 1;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    raw = (uint16_t)dm2_v1_dungeon_get_tile_raw(
        dungeon, rt->dungeon_level, x, y);
    raw = (raw & 0xfff8u) | 0x0005u;
    (void)dm2_v1_dungeon_set_tile_raw(
        dungeon, rt->dungeon_level, x, y, raw);
    return 1;
}

/*
 * dm2_runtime_release_door_button_timer — DM2-owned 0x58 handler.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:1068-1074
 * DM2_PROCESS_TIMER_RELEASE_DOOR_BUTTON:
 * GET_ADDRESS_OF_RECORD(timer->valueA), then clears bit 0x08 on
 * record byte@3 (and8(location(RG1P + 3), 0xf7)).
 *
 * Bounded slice: walks the record pool to find the record, clears the
 * bit.  Fails closed when record pools are not valid.
 */
static int dm2_runtime_release_door_button_timer(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t record_id;
    uint8_t *record;
    int type, size;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data)
        return 1;

    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        record_id, &type, NULL, &size);
    if (!record || size < 4)
        return 1;

    record[3] &= (uint8_t)~0x08u;
    return 1;
}

/*
 * dm2_runtime_process_timer_59 — DM2-owned 0x59 handler.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:1077-1090 DM2_PROCESS_TIMER_59:
 * GET_ADDRESS_OF_RECORD(timer->valueB).  If record byte@4 bit 0x04
 * is set, returns early (already processing).  Otherwise: if current
 * map matches party map, sets viewport redraw flag (ddat.v1e0390.b_00
 * |= 1).  Clears bit 0x01 on record byte@4.
 *
 * Bounded slice: the record byte mutation is the critical path.
 */
static int dm2_runtime_process_timer_59(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t record_id;
    uint8_t *record;
    int type, size;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data)
        return 1;

    record_id = (uint16_t)(timer->value_b & 0xffffu);
    record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        record_id, &type, NULL, &size);
    if (!record || size < 5)
        return 1;

    if (record[4] & 0x04u)
        return 1;

    record[4] &= (uint8_t)~0x01u;
    return 1;
}

/*
 * dm2_runtime_5b_record_clear — DM2-owned 0x5b handler.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4278-4280 (fall-through for 0x5b):
 * GET_ADDRESS_OF_RECORD(timer->valueA), then byte@4 &= ~0x01.
 */
static int dm2_runtime_5b_record_clear(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t record_id;
    uint8_t *record;
    int type, size;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data)
        return 1;

    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        record_id, &type, NULL, &size);
    if (!record || size < 5)
        return 1;

    record[4] &= (uint8_t)~0x01u;
    return 1;
}

/*
 * dm2_runtime_5c_record_set — DM2-owned 0x5c handler.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4223-4225 (0x5c inline):
 * GET_ADDRESS_OF_RECORD(timer->valueA), then byte@2 |= 0x01.
 */
static int dm2_runtime_5c_record_set(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t record_id;
    uint8_t *record;
    int type, size;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data)
        return 1;

    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        record_id, &type, NULL, &size);
    if (!record || size < 3)
        return 1;

    record[2] |= 0x01u;
    return 1;
}

/*
 * dm2_runtime_invoke_message — DM2_INVOKE_MESSAGE boundary.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4332-4364:
 * Creates a 0x04 actuator timer with the given parameters and queues it.
 * Parameters: x=target_x, y=target_y, action=action_type, delay_tick=absolute.
 * The actor byte encodes the action: 0->1, 1->3, 2->2.
 */
static void dm2_runtime_invoke_message(DM2_V1_RuntimeState *rt,
                                       int target_x, int target_y,
                                       int action, int action_type,
                                       uint32_t delay_tick) {
    DM2_V1_SourceTimer t;
    uint8_t actor;

    if (!rt) return;

    switch (action_type) {
        case 0: actor = 1; break;
        case 1: actor = 3; break;
        case 2: actor = 2; break;
        default: return;
    }

    memset(&t, 0, sizeof(t));
    t.ticks_and_map =
        ((uint32_t)(rt->dungeon_level & 0xff) << 24) |
        (delay_tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    t.type = DM2_V1_TIMER_ACTUATE_TILE;
    t.actor = actor;
    t.value_a = (uint16_t)((target_x & 0xff) |
                           ((target_y & 0xff) << 8));
    t.value_b = (uint16_t)((action & 0xff) |
                           ((action_type & 0xff) << 8));
    (void)dm2_v1_runtime_enqueue_source_timer(&t, 0);
}

/*
 * dm2_runtime_invoke_actuator — DM2_INVOKE_ACTUATOR boundary.
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4367-4392:
 * Reads the actuator record's timing and target fields, computes the
 * absolute tick for delivery, and calls DM2_INVOKE_MESSAGE.
 *
 * Register decode from c_tim_proc.cpp:4374-4391:
 *   w4 = word_at(record+4)
 *   base_delay = ((uint16_t)(w4 << 5)) >> 12  — bits 11..7 of w4
 *   absolute_tick = base_delay + gametick + ebxl (caller's delay_add)
 *
 *   w6 = word_at(record+6)
 *   action = ((uint16_t)(w6 << 10)) >> 14      — bits 5..4 of w6
 *   target_y = w6 >> 11                         — bits 15..11 of w6
 *   target_x = ((uint16_t)(w6 << 5)) >> 11     — bits 10..6 of w6
 */
static void __attribute__((unused)) dm2_runtime_invoke_actuator(DM2_V1_RuntimeState *rt,
                                        const uint8_t *record,
                                        int record_size,
                                        int action_type, int delay_add) {
    uint16_t w4, w6;
    int base_delay;
    uint32_t tick;
    int action, target_x, target_y;

    if (!rt || !record || record_size < 8) return;

    w4 = (uint16_t)record[4] | ((uint16_t)record[5] << 8);
    w6 = (uint16_t)record[6] | ((uint16_t)record[7] << 8);

    base_delay = (int)(((uint16_t)(w4 << 5)) >> 12);
    tick = (uint32_t)(base_delay + rt->tick_count + delay_add);

    action = (int)(((uint16_t)(w6 << 10)) >> 14);
    target_y = (int)(w6 >> 11);
    target_x = (int)(((uint16_t)(w6 << 5)) >> 11);

    dm2_runtime_invoke_message(rt, target_x, target_y, action,
                               action_type, tick);
}

static int dm2_runtime_think_creature_timer(void *user,
                                            const DM2_V1_SourceTimer *timer,
                                            uint16_t source_index,
                                            DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    return dm2_v1_think_creature_timer_handler(&rt->think_binding, timer,
                                               source_index, receipt);
}

/*
 * dm2_runtime_update_weather_timer — DM2-owned 0x54 handler for the
 * source-order timer dispatcher (DM2-003).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4179-4183 dispatches timer type
 * 0x54 to DM2_UPDATE_WEATHER(1) (c_weather.cpp:33-90).  The handler
 * steps the session-owned v1e14xx chain state via
 * dm2_v1_update_weather_1 and re-queues the next 0x54 timer with the
 * source delay (RAND16(256)+50).  When the handler forces a transition
 * (retry > 0x1f) the runtime runs the bound DM2_weather_3df7_0037
 * (c_weather.cpp:509-567), which owns the reseed and queues the next
 * timer itself — the chain is self-perpetuating exactly like the
 * source.  The presentation weather intensity is derived from the
 * source intensity v1e1474 (bounded 0..255 -> 0..100 mapping); the
 * weather enum stays a host presentation selector until the arg==0
 * DM2_UPDATE_WEATHER branch is bound.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4179-4183 (0x54 dispatch)
 *         skproject/SKULLWIN/c_weather.cpp:33-90   (DM2_UPDATE_WEATHER(1))
 *         skproject/SKULLWIN/c_weather.cpp:509-567 (DM2_weather_3df7_0037)
 */
static int dm2_runtime_update_weather_timer(void *user,
                                            const DM2_V1_SourceTimer *timer,
                                            uint16_t source_index,
                                            DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_UpdateWeatherReceipt weather_rc;
    int delay = -1;
    (void)timer;
    (void)source_index;
    (void)receipt;

    if (!rt->outdoor || !rt->weather_chain_started) {
        /* The chain is not running (indoor or not started); consume the
         * timer without simulating, mirroring the source's fail-closed
         * consumption of unowned timers. */
        return 1;
    }

    if (dm2_v1_update_weather_1(&rt->weather_chain, &rt->weather_rng,
                                &weather_rc)) {
        delay = weather_rc.reschedule_delay;
        if (weather_rc.transition_forced) {
            DM2_V1_WeatherTransitionReceipt transition_rc;
            memset(&transition_rc, 0, sizeof(transition_rc));
            /* The return value is days elapsed; gate on the receipt. */
            (void)dm2_v1_weather_transition(&rt->weather_chain,
                                            rt->tick_count, 0,
                                            &rt->weather_rng,
                                            &transition_rc);
            if (transition_rc.valid) {
                delay = transition_rc.queue_delay;
            }
        }
        if (delay >= 0) {
            DM2_V1_SourceTimer next;
            memset(&next, 0, sizeof(next));
            next.ticks_and_map =
                (uint32_t)(rt->tick_count + delay) &
                DM2_V1_SOURCE_TIMER_TICK_MASK; /* map 0: outdoor session */
            next.type = DM2_V1_TIMER_UPDATE_WEATHER; /* 0x54 */
            next.actor = 0; /* c_weather.cpp:28 tim.setactor(0) */
            (void)dm2_v1_runtime_enqueue_source_timer(&next, 0);
        }
        /* Bounded presentation mapping: the source intensity v1e1474
         * (0..255) drives the host weather intensity (0..100). */
        rt->weather.weather_intensity =
            (int)rt->weather_chain.intensity * 100 / 255;
    }
    return 1;
}

/*
 * DM2 square class for doors, from skproject c_tim_proc.cpp:4214-4230
 * subdispatch and c_map.cpp square-type encoding (mapdat.map[x][y] >> 5).
 * Class 4 is the door class.
 */
#define DM2_V1_SQUARE_CLASS_DOOR 4

/*
 * dm2_runtime_door_step_timer — DM2-owned 0x01 handler for the source-order
 * timer dispatcher (DM2-003 follow-up, round 24).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4041 dispatches timer type 0x01 to
 * DM2_STEP_DOOR.  The bounded Firestaff slice reads the door square from the
 * boot dungeon, applies one ReDMCSB TIMELINE.C state transition, writes the
 * new square back, and re-queues subsequent steps until OPEN/CLOSED.  Party
 * damage on close, door-record direction decoding, and sound dispatch remain
 * unbound (receipted fail-closed).
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4041 (0x01 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:127+   (DM2_STEP_DOOR)
 *         ReDMCSB TIMELINE.C:750-810               (door state transitions)
 */
static int dm2_runtime_door_step_timer(void *user,
                                       const DM2_V1_SourceTimer *timer,
                                       uint16_t source_index,
                                       DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int map;
    int x;
    int y;
    int square_class;
    uint16_t raw;
    int current_state;
    int direction;
    int new_state;
    int reached_target;

    rt->door_step_timers++;

    /* The source loop consumes the timer even when the map state is
     * unavailable. */
    if (timer == NULL || rt->boot == NULL || rt->boot->dungeon_data == NULL) {
        if (receipt != NULL) {
            receipt->handler_rejected_count++;
        }
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    /* c_timer.h:64-66 — map is the high byte of ticks_and_map; x/y are the
     * valueA lo/hi bytes (c_timer.h:80-81 getxA/getyA). */
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    /* The source reads mapdat.map[x][y] to get both the square class and
     * the door state (lower 3 bits).  Fail closed on missing data. */
    square_class = dm2_v1_dungeon_get_square_type(
        dungeon, rt->dungeon_level, x, y);
    if (square_class != DM2_V1_SQUARE_CLASS_DOOR) {
        if (receipt != NULL) {
            receipt->handler_rejected_count++;
        }
        return 1;
    }

    raw = (uint16_t)dm2_v1_dungeon_get_tile_raw(
        dungeon, rt->dungeon_level, x, y);
    current_state = dm2_door_get_state(raw);

    /* TIMELINE.C:750 — DESTROYED is sticky; the source returns immediately. */
    if (current_state == DM2_DOOR_STATE_DESTROYED) {
        return 1;
    }

    /* Bounded wiring slice: direction is carried in value_b.  The full
     * source DM2_ACTUATE_DOOR/DM2_STEP_DOOR encode direction in door-record
     * word[2] bits 9/10 and byte[3] bit 0x4; decoding those from the thing
     * record is left for a follow-up once the record-pool door grammar is
     * proven. */
    direction = (int)(timer->value_b & 0x1);

    new_state = dm2_door_apply_toggle_step(current_state, direction);

    /* Write the mutated square back.  Preserve all upper bits and only
     * change the lower 3-bit state. */
    raw = dm2_door_set_state(raw, new_state);
    if (dm2_v1_dungeon_set_tile_raw(
            dungeon, rt->dungeon_level, x, y, raw) != 0) {
        if (receipt != NULL) {
            receipt->handler_rejected_count++;
        }
        return 1;
    }
    rt->door_step_mutations++;

    /* Re-queue the next step timer if we have not reached the target state.
     * The source DM2_STEP_DOOR does this via DM2_QUEUE_TIMER after
     * incrementing its data word; we schedule one tick later using
     * receipt->game_tick. */
    reached_target = (direction == DM2_DOOR_TOGGLE_DIR_OPEN &&
                      new_state == DM2_DOOR_STATE_OPEN) ||
                     (direction == DM2_DOOR_TOGGLE_DIR_CLOSE &&
                      new_state == DM2_DOOR_STATE_CLOSED);
    if (!reached_target && receipt != NULL) {
        DM2_V1_SourceTimer next;
        next = *timer;
        next.ticks_and_map =
            ((uint32_t)(map & 0xff) << 24) |
            ((receipt->game_tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
        rt->door_step_requeues++;
        (void)dm2_v1_runtime_enqueue_source_timer(&next, source_index);
    }

    return 1;
}

/*
 * dm2_runtime_tile_class_at — square-class provider for the source
 * 0x04 actuator dispatch (c_tim_proc.cpp:4283-4287: mapdat.map[x][y]
 * byte >> 5).  Bound over the boot dungeon's raw square byte through
 * dm2_v1_dungeon_get_square_type; unavailable map state fails closed
 * (-1) exactly like the dispatcher's contract.
 */
static int dm2_runtime_tile_class_at(void *user, int map, int x, int y) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;

    if (!rt->boot || !rt->boot->dungeon_data) {
        return -1;
    }
    return dm2_v1_dungeon_get_square_type(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data, map, x, y);
}

/*
 * dm2_runtime_actuate_floor_mecha — DM2-owned class-1 handler for the
 * 0x04 actuator subdispatch (DM2-003 follow-up, round 23).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4297-4299 dispatches square class 1
 * to DM2_ACTUATE_FLOOR_MECHA (c_tim_proc.cpp:3009-3532): the source
 * walks the tile record chain at (getxA, getyA), a link whose DB index
 * exceeds 3 returns the whole function, and a DB3 record whose word@2
 * & 0x7f type byte is 0x3a runs DM2_ANIMATE_CREATURE(x, y, yB==0)
 * (c_tim_proc.cpp:3177-3184) — the CAII activation site bound in round
 * 23 as dm2_v1_caii_animate_activation.  The chain walk mirrors the
 * bounded contract of dm2_v1_tile_record_walk inline (visit count
 * bounded by the declared pool records, corrupt/unresolvable links fail
 * closed) so the runtime keeps its link boundary.  Every other record
 * type in the dispatch matrix (wall mecha 0x27, ornate animators
 * 0x2c/0x2e/0x32, relays 0x3d/0x45, teleporters, the DB2
 * creature-killer/generator branches and the DB0/1 item branches) stays
 * host-owned — acknowledged in source order, never simulated.  The
 * getyB()==0 flag only selects DM2_ai_13e4_0806 vs 071b inside the
 * unbound CCM tail, so it does not reach the bounded slice.  The timer
 * is always consumed, exactly like the source loop.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4297-4299 (class-1 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:3009-3532 (DM2_ACTUATE_FLOOR_MECHA)
 *         skproject/SKULLWIN/c_tim_proc.cpp:2859-2900 (DM2_ANIMATE_CREATURE)
 */
static int dm2_runtime_actuate_floor_mecha(void *user,
                                           const DM2_V1_SourceTimer *timer,
                                           uint16_t source_index,
                                           DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    const DM2_V1_DungeonData *dungeon;
    int x;
    int y;
    int total_records = 0;
    int visited = 0;
    int i;
    int16_t link;
    (void)source_index;
    (void)receipt;

    rt->floor_mecha_timers++;
    if (!rt->think_binding_ready || !rt->caii_ready ||
        !rt->boot || !rt->boot->dungeon_data) {
        /* Unready session state: consumed fail-closed, never simulated. */
        return 1;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;

    /* c_timer.h:80-81 — getxA/getyA are the valueA lo/hi bytes. */
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    /* DM2_GET_TILE_RECORD_LINK (c_map.cpp:61-69) over the session's
     * current map (map 0), like every other runtime accessor. */
    link = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, 0, x, y);
    for (i = 0; i < DM2_V1_RECORD_POOL_COUNT; i++) {
        total_records += rt->record_pools.pools[i].record_count;
    }
    while (link != DM2_V1_RECORD_HANDLE_END &&
           link != DM2_V1_RECORD_HANDLE_NULL) {
        const uint8_t *record;
        int16_t next = DM2_V1_RECORD_HANDLE_END;
        int pool;
        int type;

        if (++visited > total_records) {
            /* Corrupt chain (self-loop): bounded fail-closed. */
            rt->floor_mecha_walk_failed++;
            return 1;
        }
        record = dm2_v1_record_pool_address(&rt->record_pools, link);
        if (record == NULL) {
            rt->floor_mecha_walk_failed++;
            return 1;
        }
        pool = dm2_v1_record_handle_pool(link);
        if (pool > 3) {
            /* c_tim_proc.cpp:3054-3055: a DB index above 3 returns the
             * whole function. */
            rt->floor_mecha_db_break++;
            return 1;
        }
        type = (int)(((unsigned)record[2] | ((unsigned)record[3] << 8)) &
                     0x7fu);
        if (pool == 3 && type == 0x3a) {
            DM2_V1_CaiiAnimateActivationReceipt anim;

            rt->floor_mecha_0x3a_records++;
            memset(&anim, 0, sizeof(anim));
            if (dm2_v1_caii_animate_activation(
                    &rt->record_pools, dungeon, &rt->caii, &rt->timer_queue,
                    0, (unsigned long)rt->tick_count, x, y, &anim) == 1) {
                rt->floor_mecha_activations++;
                if (anim.alloc_performed) {
                    rt->floor_mecha_allocs++;
                }
            }
        }
        /* DM2_GET_NEXT_RECORD_LINK (c_record.cpp:54-57), m_47AFA. */
        if (dm2_v1_record_pool_next_link(&rt->record_pools, link, &next) != 1) {
            rt->floor_mecha_walk_failed++;
            return 1;
        }
        link = next;
    }
    return 1;
}

/*
 * dm2_runtime_process_0c_timer — 0x0C timer handler.
 * Source: SKProject/SKULLWIN/c_tim_proc.cpp:25-31 DM2_PROCESS_TIMER_0C.
 *
 * The original clears a 16-bit c_hero::timeridx and, for a living hero,
 * sets the distinct 16-bit c_hero::heroflag bit 0x0800. The bounded session
 * surrogate has byte-sized timer_index/hero_flag fields, so its former 0x08
 * write was not a truncation-safe implementation of the source operation.
 * Keep ordered dispatch but make no state change until c_hero is imported.
 */
static int dm2_runtime_process_0c_timer(void *user,
                                        const DM2_V1_SourceTimer *timer,
                                        uint16_t source_index,
                                        DM2_V1_ProceedTimersReceipt *receipt) {
    (void)user;
    (void)timer;
    (void)source_index;
    (void)receipt;

    return 1;
}

/*
 * dm2_runtime_resurrection_timer — 0x0D timer handler.
 * Source: c_tim_proc.cpp:39-124 DM2_PROCESS_TIMER_RESURRECTION.
 * Three phases (yB countdown):
 *   yB==2: create cloud effect at position (fail-closed: needs CREATE_CLOUD)
 *   yB==1: walk tile records, dealloc tombstone (fail-closed: needs record walk)
 *   yB==0: final phase — call BRING_CHAMPION_TO_LIFE(actor).
 *
 * The bounded session record is not SKProject's c_hero: its 261-byte
 * persistence surrogate has a byte-sized hero_flag and 32-bit inventory
 * handles, whereas c_hero is 263 bytes and owns 16-bit hero flags and item
 * records.  It also has no source-bound tombstone chain or CREATE_CLOUD
 * owner for phases 1 and 2.  Do not apply just the final formula to that
 * surrogate: that would create a champion without the preceding source
 * state transitions.  Consume the timer in source order until the complete
 * c_hero/tile-record implementation is available.
 *
 * Source: SKProject/SKULLWIN/c_tim_proc.cpp:39-124
 *         SKProject/SKULLWIN/c_hero.h:40-130, 916-953
 */
static int dm2_runtime_resurrection_timer(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt) {
    (void)user;
    (void)timer;
    (void)source_index;
    (void)receipt;
    return 1;
}

/*
 * dm2_runtime_process_0e_timer — 0x0E timer handler.
 * Source: skevent.cpp:27-49 PROCESS_TIMER_0E.
 * Temporarily morphs an item's type, processes bonus, then restores.
 * Timer fields: value_a bits 0-9 = record DB type,
 *               value_b = new type (Value2),
 *               actor = hero index.
 * Caller bonus_value = 0xFFFFFFFE (-2).
 */
static uint8_t *dm2_0e_get_record(void *ctx, uint16_t rw) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    return dm2_v1_record_pool_address_mut(&rt->record_pools, (int16_t)rw);
}
static void *dm2_0e_alloc(void *ctx, int32_t size) {
    (void)ctx;
    return malloc((size_t)size);
}
static void dm2_0e_dealloc(void *ctx, void *ptr, int32_t size) {
    (void)ctx; (void)size;
    free(ptr);
}
static void dm2_0e_set_itemtype(void *ctx, uint16_t record, uint16_t new_type) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    dm2_v1_record_pool_set_itemtype(&rt->record_pools, record, new_type);
}
static void dm2_0e_process_item_bonus(void *ctx, uint8_t actor,
                                       uint16_t record, int mode,
                                       uint16_t value) {
    (void)ctx; (void)actor; (void)record; (void)mode; (void)value;
    /* PROCESS_ITEM_BONUS requires the full champion stat system.
     * Fail-closed: the item type is still morphed and restored
     * correctly; only the bonus application is skipped. */
}
static void dm2_0e_copy_memory(void *dst, const void *src, int32_t size) {
    if (size > 0) memcpy(dst, src, (size_t)size);
}
static int32_t dm2_0e_get_item_size(uint16_t db_type) {
    return (int32_t)dm2_v1_record_pool_record_size((int)db_type);
}

static int dm2_runtime_process_0e_timer(void *user,
                                        const DM2_V1_SourceTimer *timer,
                                        uint16_t source_index,
                                        DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_Timer0ECallbacks cb;

    (void)source_index; (void)receipt;

    if (!rt || !rt->record_pools_valid)
        return 1;

    cb.get_record_address = dm2_0e_get_record;
    cb.alloc_memory = dm2_0e_alloc;
    cb.dealloc_memory = dm2_0e_dealloc;
    cb.set_itemtype = dm2_0e_set_itemtype;
    cb.process_item_bonus = dm2_0e_process_item_bonus;
    cb.copy_memory = dm2_0e_copy_memory;
    cb.get_item_size = dm2_0e_get_item_size;

    dm2_v1_process_timer_0e(
        (uint16_t)(timer->value_a & 0x3FF),
        (uint16_t)timer->value_b,
        timer->actor,
        0xFFFEu,
        &cb, rt);

    return 1;
}

/*
 * dm2_runtime_process_sound_timer — 0x15 timer handler.
 * Source: skevent.cpp:2590 / c_sfx.cpp:303 DM2_PROCESS_SOUND(timer.getA()).
 * Reads s_sizee delayed slot, if map matches then QUEUE_NOISE_GEN1,
 * clears the slot.
 */
static int dm2_runtime_process_sound_timer(void *user,
                                           const DM2_V1_SourceTimer *timer,
                                           uint16_t source_index,
                                           DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t slot_index;
    DM2_V1_SoundDelayedSlot *slot;

    (void)source_index; (void)receipt;

    if (!rt || !rt->sound_queue_ready)
        return 1;

    slot_index = (uint16_t)(timer->value_a & 0xFFFF);
    if (slot_index >= DM2_V1_SOUND_DELAYED_SLOT_COUNT)
        return 1;

    slot = &rt->sound_queue.delayed[slot_index];
    if (slot->l_00 == 0)
        return 1;

    /* c_sfx.cpp:303-327: if slot map matches current or gate maps,
     * call QUEUE_NOISE_GEN1 with the stored parameters. */
    {
        int8_t cls1 = (int8_t)slot->barr_04[0];
        int8_t cls2 = (int8_t)slot->barr_04[1];
        int8_t cls3 = (int8_t)slot->barr_04[2];
        int8_t slot_map = (int8_t)slot->barr_04[3];
        int16_t sx = (int16_t)slot->barr_04[4];
        int16_t sy = (int16_t)slot->barr_04[5];

        if (slot_map == rt->sound_env.current_map ||
            slot_map == rt->sound_env.gate_map_a ||
            slot_map == rt->sound_env.gate_map_b) {
            DM2_V1_SoundQueueReceipt snd_rc;
            dm2_v1_sound_queue_noise_gen1(
                &rt->sound_queue,
                cls1, cls2, cls3,
                slot->w_0a, slot->w_0c,
                sx, sy, 1,
                &rt->sound_env, &snd_rc);
        }
    }

    slot->l_00 = 0;
    return 1;
}

/*
 * dm2_runtime_spell_timer_delegate — shared handler for timer types that have
 * proven implementations in dm2_v1_spell_timer_handlers_pc34_compat.c.
 * Forwards to dm2_v1_spell_timer_dispatch() which routes by timer->type.
 * Types handled: 0x46 light, 0x47 hero ench flag, 0x48 ench power,
 * 0x4B poison, 0x19 cloud, 0x1E missile, 0x5E summon.
 */
static int dm2_runtime_spell_timer_delegate(void *user,
                                            const DM2_V1_SourceTimer *timer,
                                            uint16_t source_index,
                                            DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    if (!rt || !rt->spell_timer_ctx_ready)
        return 1;
    return dm2_v1_spell_timer_dispatch(&rt->spell_timer_ctx, timer,
                                       source_index, receipt);
}

/* 0x19 cloud and 0x1D/0x1E missile — delegated to spell timer handlers via
 * dm2_runtime_spell_timer_delegate. */

/*
 * dm2_runtime_process_3d_timer — 0x3C/0x3D timer handler.
 * Source: skevent.cpp:2570 PROCESS_TIMER_3D.
 * Moves a record from "nowhere" (-3,0) to the timer's (x,y) coordinates.
 * Timer fields: value_a = XcoordB|YcoordB, value_b = record handle (id8).
 * If type==0x3C and move succeeds, queues teleport noise (not yet wired).
 */
static int dm2_runtime_process_3d_timer(void *user,
                                        const DM2_V1_SourceTimer *timer,
                                        uint16_t source_index,
                                        DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_MoveRecordToReceipt move_rc;
    int16_t dest_x, dest_y, record_handle;
    int move_ok;

    (void)source_index; (void)receipt;
    rt->actuator_tile_timers++;

    if (!rt->record_pools_valid || !rt->boot || !rt->boot->dungeon_data)
        return 1;

    dest_x = (int16_t)(timer->value_a & 0xFF);
    dest_y = (int16_t)((timer->value_a >> 8) & 0xFF);
    record_handle = timer->value_b;

    move_ok = dm2_v1_move_record_to(
        &rt->record_pools,
        (DM2_V1_DungeonData *)rt->boot->dungeon_data,
        &rt->timer_queue,
        record_handle, -3, 0, dest_x, dest_y, 0,
        rt->dungeon_level, (uint32_t)rt->tick_count,
        &move_rc);

    (void)move_ok;
    return 1;
}

/* 0x46 light, 0x47 hero ench flag, 0x48 ench power, 0x4B poison —
 * delegated to spell timer handlers via dm2_runtime_spell_timer_delegate. */

/*
 * dm2_runtime_ornate_noise_timer — 0x5A timer handler.
 * Source: skevent.cpp:2818 / c_tim_proc.cpp:4216 DM2_CONTINUE_ORNATE_NOISE.
 * Reads actuator record, checks ActiveStatus, resolves wall/floor decoration,
 * requeues timer with anim_len delay, and queues activation sound.
 */
static int dm2_runtime_ornate_noise_timer(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint8_t *rec;
    uint16_t w4, graphic_number, active_status;
    uint8_t tile_x, tile_y;
    int tile_raw, is_wall;
    uint8_t category, decoration;
    const uint8_t *gfx_list;
    int gfx_count;
    int timer_map;

    (void)source_index; (void)receipt;

    if (!rt || !rt->record_pools_valid || !rt->boot || !rt->boot->dungeon_data)
        return 1;

    rec = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                          timer->value_b);
    if (!rec)
        return 1;

    w4 = (uint16_t)(rec[4] | (rec[5] << 8));
    active_status = w4 & 0x01u;
    timer_map = (int)((timer->ticks_and_map >> 24) & 0xFFu);

    if (active_status == 0 || timer_map != rt->dungeon_level)
        return 1;

    tile_x = (uint8_t)(timer->value_a & 0xFF);
    tile_y = (uint8_t)((timer->value_a >> 8) & 0xFF);

    tile_raw = dm2_v1_dungeon_get_tile_raw(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        rt->dungeon_level, tile_x, tile_y);
    if (tile_raw < 0)
        return 1;

    is_wall = ((tile_raw >> 5) & 7) == 0 ? 1 : 0;
    graphic_number = (w4 >> 12) & 0x0Fu;
    if (graphic_number == 0)
        return 1;

    if (is_wall) {
        category = 0x09;
        gfx_list = rt->map_wall_gfx_list;
        gfx_count = rt->map_wall_gfx_count;
    } else {
        category = 0x0A;
        gfx_list = rt->map_floor_gfx_list;
        gfx_count = rt->map_floor_gfx_count;
    }

    if ((int)graphic_number > gfx_count)
        return 1;
    decoration = gfx_list[graphic_number - 1];
    if (decoration == 0xFFu)
        return 1;

    /* Requeue timer: tick += GET_ORNATE_ANIM_LEN */
    {
        const DM2_V1_AssetLoader *loader =
            dm2_v1_boot_asset_loader(rt->boot);
        if (loader) {
            DM2_V1_GetOrnateAnimLenReceipt anim_rc;
            if (dm2_v1_get_ornate_anim_len_receipt(
                    loader, (int)category, (int)decoration, 0, &anim_rc) &&
                anim_rc.accepted && anim_rc.length > 0) {
                DM2_V1_SourceTimer requeue = *timer;
                requeue.ticks_and_map =
                    (timer->ticks_and_map & 0xFF000000u) |
                    ((dm2_v1_source_timer_tick(timer) +
                      (uint32_t)anim_rc.length) &
                     DM2_V1_SOURCE_TIMER_TICK_MASK);
                dm2_v1_runtime_enqueue_source_timer(&requeue, 0);
            }
        }
    }

    /* QUEUE_NOISE_GEN2: activation sound */
    if (rt->sound_queue_ready) {
        DM2_V1_SoundQueueReceipt snd_rc;
        dm2_v1_sound_queue_noise_gen2(
            &rt->sound_queue,
            (int8_t)category, (int8_t)decoration, (int8_t)0x88,
            (int8_t)0xFF, (int16_t)tile_x, (int16_t)tile_y,
            1, 0x8C, 0x80,
            &rt->sound_env, &snd_rc);
    }

    return 1;
}

/*
 * dm2_runtime_move_record_rotate_timer — 0x5D timer handler.
 * Source: skevent.cpp:3140-3145.
 * If timer map == player map: MOVE_RECORD_TO(NULL, playerX, playerY,
 * targetX, targetY) then ROTATE_SQUAD(direction).
 * Timer value_a: bits 0-4 = target X, bits 5-9 = target Y, bits 10-11 = dir.
 * Timer value_b: map index (compared against player map).
 */
static int dm2_runtime_move_record_rotate_timer(void *user,
                                                const DM2_V1_SourceTimer *timer,
                                                uint16_t source_index,
                                                DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    int16_t target_x, target_y, new_dir;

    (void)source_index; (void)receipt;

    if (!rt || !rt->record_pools_valid || !rt->boot || !rt->boot->dungeon_data)
        return 1;

    if ((int)timer->value_b != rt->dungeon_level)
        return 1;

    target_x = (int16_t)(timer->value_a & 0x1F);
    target_y = (int16_t)((timer->value_a >> 5) & 0x1F);
    new_dir = (int16_t)((timer->value_a >> 10) & 0x3);

    /* MOVE_RECORD_TO(OBJECT_NULL, playerX, playerY, targetX, targetY)
     * is party teleport — this modifies session state (party position).
     * For now, update party position directly without the full
     * MOVE_RECORD_TO chain (which handles creature wake/sleep). */
    if (rt->session_snapshot_valid) {
        rt->session_snapshot.party_x = (uint16_t)target_x;
        rt->session_snapshot.party_y = (uint16_t)target_y;
        rt->session_snapshot.party_dir = (uint8_t)new_dir;
    }

    return 1;
}

/* 0x5E alloc new creature — delegated to spell timer handlers via
 * dm2_runtime_spell_timer_delegate. */

/*
 * dm2_runtime_ornate_animator_timer — 0x55 timer handler.
 * Source: skevent.cpp:2742 CONTINUE_ORNATE_ANIMATOR.
 * Advances the ornate animation frame on the actuator record via
 * dm2_v1_continue_ornate_animator, querying GDAT for the animation
 * length through the asset loader's decoration table lookup.
 */

typedef struct {
    DM2_V1_RuntimeState *rt;
    DM2_V1_SourceTimer requeue;
} DM2_OrnateAnimCtx;

static uint8_t *dm2_ornate_get_record(void *ctx, uint16_t rw) {
    DM2_OrnateAnimCtx *oc = (DM2_OrnateAnimCtx *)ctx;
    return dm2_v1_record_pool_address_mut(&oc->rt->record_pools, (int16_t)rw);
}

static int16_t dm2_ornate_get_anim_len(void *ctx, uint8_t *rec, int mode) {
    DM2_OrnateAnimCtx *oc = (DM2_OrnateAnimCtx *)ctx;
    DM2_V1_RuntimeState *rt = oc->rt;
    const DM2_V1_AssetLoader *loader = dm2_v1_boot_asset_loader(rt->boot);
    uint16_t w4 = (uint16_t)(rec[4] | (rec[5] << 8));
    int16_t gfx_num = (int16_t)((w4 >> 12) & 0xf);
    int category;
    uint8_t decoration;
    DM2_V1_GetOrnateAnimLenReceipt receipt;

    if (gfx_num == 0) return 1;
    if (mode == 0) {
        category = 0x0a;
        if (gfx_num - 1 >= rt->map_floor_gfx_count) return 1;
        decoration = rt->map_floor_gfx_list[gfx_num - 1];
    } else {
        category = 0x09;
        if (gfx_num - 1 >= rt->map_wall_gfx_count) return 1;
        decoration = rt->map_wall_gfx_list[gfx_num - 1];
    }
    if (!loader) return 1;
    if (!dm2_v1_get_ornate_anim_len_receipt(
            loader, category, (int)decoration, 0, &receipt))
        return 1;
    return (int16_t)receipt.length;
}

static void dm2_ornate_queue_timer(void *ctx) {
    DM2_OrnateAnimCtx *oc = (DM2_OrnateAnimCtx *)ctx;
    oc->requeue.ticks_and_map =
        (oc->requeue.ticks_and_map & ~DM2_V1_SOURCE_TIMER_TICK_MASK) |
        ((dm2_v1_source_timer_tick(&oc->requeue) + 1) &
         DM2_V1_SOURCE_TIMER_TICK_MASK);
    dm2_v1_runtime_enqueue_source_timer(&oc->requeue, 0);
}

static int dm2_runtime_ornate_animator_timer(void *user,
                                             const DM2_V1_SourceTimer *timer,
                                             uint16_t source_index,
                                             DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_OrnateAnimCtx oc;
    DM2_V1_OrnateAnimCallbacks cb;

    (void)source_index; (void)receipt;
    rt->actuator_tile_timers++;

    if (!rt->record_pools_valid) return 1;
    if (dm2_v1_record_handle_pool(timer->value_a) != DM2_DB_ACTUATOR) return 1;

    oc.rt = rt;
    oc.requeue = *timer;
    cb.get_record_address = dm2_ornate_get_record;
    cb.get_ornate_anim_len = dm2_ornate_get_anim_len;
    cb.queue_timer = dm2_ornate_queue_timer;

    dm2_v1_continue_ornate_animator(
        (uint16_t)timer->value_a, (int)timer->value_b, &cb, &oc);
    return 1;
}

/*
 * dm2_runtime_tick_generator_timer — 0x56 timer handler.
 * Source: skevent.cpp CONTINUE_TICK_GENERATOR (v4 line 2764).
 * Resolves the actuator record from the timer's value_a (ObjectID),
 * fires INVOKE_ACTUATOR on the actuator's target, then re-queues
 * itself with the next tick offset.
 */
static int dm2_runtime_tick_generator_timer(void *user,
                                            const DM2_V1_SourceTimer *timer,
                                            uint16_t source_index,
                                            DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    int16_t record_link;
    uint8_t *actu;
    uint8_t action_type;
    uint8_t once_only;
    uint8_t si;
    int requeue;
    uint16_t value2;
    uint16_t actu_data;
    DM2_V1_SourceTimer requeue_timer;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;

    if (!rt->record_pools_valid) return 1;

    record_link = timer->value_a;
    if (dm2_v1_record_handle_pool(record_link) != DM2_DB_ACTUATOR) return 1;

    actu = dm2_v1_record_pool_address_mut(&rt->record_pools, record_link);
    if (!actu) return 1;

    action_type = dm2_actu_action_type(actu);
    once_only   = dm2_actu_once_only(actu);
    value2      = (uint16_t)(timer->value_b & 0xFF);
    actu_data   = dm2_actu_data(actu);

    if (action_type == 3) {
        /* Toggle b4_0_0 bit (active status), use toggled state as action.
         * Source: skevent.cpp:2771-2774 */
        uint8_t toggled = (uint8_t)(dm2_actu_active_status(actu) ^ 1u);
        dm2_actu_set_active_status(actu, toggled);
        si = toggled | once_only;
        dm2_v1_invoke_actuator(&rt->timer_queue, actu,
                               (toggled != 0) ? 0 : 1, 0,
                               (int)(timer->ticks_and_map >> 24),
                               (uint32_t)rt->tick_count);
    } else {
        si = once_only;
        if (si != 0) {
            dm2_v1_invoke_actuator(&rt->timer_queue, actu,
                                   action_type, 0,
                                   (int)(timer->ticks_and_map >> 24),
                                   (uint32_t)rt->tick_count);
        }
    }

    requeue = (si != 0);
    if (requeue && actu_data > 0 && value2 > 0) {
        uint32_t next_tick = (timer->ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK)
                           + (uint32_t)value2 * (uint32_t)actu_data;
        requeue_timer = *timer;
        requeue_timer.ticks_and_map =
            (next_tick & DM2_V1_SOURCE_TIMER_TICK_MASK)
            | (timer->ticks_and_map & ~DM2_V1_SOURCE_TIMER_TICK_MASK);
        dm2_v1_source_timer_enqueue(&rt->timer_queue, &requeue_timer, 0);
    } else if (!requeue) {
        dm2_actu_set_active_status(actu, 0);
    }

    return 1;
}

/*
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4214 (class-0 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:1923 (DM2_ACTUATE_WALL_MECHA)
 */
static int dm2_runtime_actuate_wall_mecha(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_ActuatorEventReceipt actu_receipt;
    int x, y, action_type, direction;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;
    rt->actuator_tile_wall_mecha++;

    if (!rt->record_pools_valid || !rt->boot || !rt->boot->dungeon_data) {
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xFF);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xFF);
    direction   = (int)(timer->value_b & 0xFF);
    action_type = (int)((timer->value_b >> 8) & 0xFF);

    memset(&actu_receipt, 0, sizeof(actu_receipt));
    dm2_v1_actuate_wall_mecha(&rt->record_pools, dungeon,
                              &rt->caii, &rt->timer_queue,
                              rt->dungeon_level, x, y,
                              action_type, direction,
                              (uint32_t)rt->tick_count,
                              NULL, 0,
                              NULL, NULL,
                              &actu_receipt);
    return 1;
}

/*
 * dm2_runtime_actuate_pitfall — DM2-owned class-2 handler for the 0x04
 * actuator tile subdispatch (Lane B, cycle 8).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4216 dispatches square class 2 to
 * DM2_ACTUATE_PITFALL (c_tim_proc.cpp:3707).  The source operates on the
 * byte-square at (getxA, getyA) and toggles the pit open/closed state.
 * Firestaff's bounded slice treats value_b bit 0 as the target direction:
 *   0 -> set square type to DM2_SQUARE_FLOOR (close the pit)
 *   1 -> set square type to DM2_SQUARE_PIT   (open the pit)
 * Only the lower 5 bits of the raw square byte are modified; the actuator
 * class in bits 5-7 is preserved.  Squares that are neither FLOOR nor PIT
 * are rejected (fail-closed).  Party damage, sound, and the full CCM tail
 * remain unbound.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4216 (class-2 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:3707 (DM2_ACTUATE_PITFALL)
 *         ReDMCSB DEFS.H:385-390 (DM2_SQUARE_* type constants)
 */
static int dm2_runtime_actuate_pitfall(void *user,
                                       const DM2_V1_SourceTimer *timer,
                                       uint16_t source_index,
                                       DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int x;
    int y;
    int raw;
    int direction;
    int target_type;
    uint16_t new_raw;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;
    rt->actuator_tile_pitfall++;

    if (timer == NULL || rt->boot == NULL || rt->boot->dungeon_data == NULL) {
        rt->actuator_tile_pitfall_rejected++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    raw = dm2_v1_dungeon_get_tile_raw(dungeon, rt->dungeon_level, x, y);
    if (raw < 0) {
        rt->actuator_tile_pitfall_rejected++;
        return 1;
    }

    /* Source direction inference: value_b bit 0 selects open vs close. */
    direction = (int)(timer->value_b & 0x1);
    target_type = direction ? DM2_SQUARE_PIT : DM2_SQUARE_FLOOR;

    /* Bounded mutation: only toggle between FLOOR and PIT. */
    if ((raw & DM2_SQUARE_TYPE_MASK) != DM2_SQUARE_FLOOR &&
        (raw & DM2_SQUARE_TYPE_MASK) != DM2_SQUARE_PIT) {
        rt->actuator_tile_pitfall_rejected++;
        return 1;
    }

    new_raw = (uint16_t)((raw & (uint16_t)~DM2_SQUARE_TYPE_MASK) |
                         (target_type & DM2_SQUARE_TYPE_MASK));
    if (dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level, x, y,
                                    new_raw) != 0) {
        rt->actuator_tile_pitfall_rejected++;
        return 1;
    }
    rt->actuator_tile_pitfall_toggles++;
    return 1;
}

/*
 * dm2_runtime_actuate_door — DM2-owned class-4 handler for the 0x04 actuator
 * tile subdispatch (Lane B, cycle 8).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4218 dispatches square class 4 to
 * DM2_ACTUATE_DOOR (c_tim_proc.cpp:3744).  The bounded Firestaff slice reads
 * the door square, applies one ReDMCSB TIMELINE.C toggle step in the
 * direction encoded by value_b bit 0, and writes the new state back.  The
 * full source derives direction from the door-record word[2] bits 9/10 and
 * byte[3] bit 0x4; decoding those from the thing record is left for a
 * follow-up.  Party damage on close, door-record direction decoding, and
 * sound dispatch remain unbound.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4218 (class-4 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:3744 (DM2_ACTUATE_DOOR)
 *         ReDMCSB TIMELINE.C:750-810 (door state transitions)
 */
static int dm2_runtime_actuate_door(void *user,
                                    const DM2_V1_SourceTimer *timer,
                                    uint16_t source_index,
                                    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int x;
    int y;
    int raw;
    int current_state;
    int direction;
    int new_state;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;
    rt->actuator_tile_door++;

    if (timer == NULL || rt->boot == NULL || rt->boot->dungeon_data == NULL) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    raw = dm2_v1_dungeon_get_tile_raw(dungeon, rt->dungeon_level, x, y);
    if (raw < 0) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }

    current_state = dm2_door_get_state((uint16_t)raw);
    if (current_state == DM2_DOOR_STATE_DESTROYED) {
        return 1;
    }

    direction = (int)(timer->value_b & 0x1);
    new_state = dm2_door_apply_toggle_step(current_state, direction);

    raw = (int)dm2_door_set_state((uint16_t)raw, new_state);
    if (dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level, x, y,
                                    (uint16_t)raw) != 0) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    rt->actuator_tile_door_mutations++;
    return 1;
}

/*
 * dm2_runtime_actuate_teleporter — DM2-owned class-5 handler for the 0x04
 * actuator tile subdispatch (Lane B, cycle 8).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4219 dispatches square class 5 to
 * DM2_ACTUATE_TELEPORTER (c_tim_proc.cpp:3832).  The source body resolves
 * teleporter target coordinates and may move the party, creatures, or items;
 * that CCM tail is not yet source-bound.  The bounded handler consumes the
 * timer in source order and increments a fail-closed counter.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4219 (class-5 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:3832 (DM2_ACTUATE_TELEPORTER)
 */
static int dm2_runtime_actuate_teleporter(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int x, y, raw, current_type;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;
    rt->actuator_tile_teleporter++;

    if (timer == NULL || rt->boot == NULL || rt->boot->dungeon_data == NULL)
        return 1;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    raw = dm2_v1_dungeon_get_tile_raw(dungeon, rt->dungeon_level, x, y);
    if (raw < 0) return 1;

    current_type = raw & DM2_SQUARE_TYPE_MASK;

    /* c_tim_proc.cpp:3849-3850 — word@4 bits 1-2 == 0x6 means
     * fall through to floor mecha instead. */
    if (current_type == DM2_SQUARE_TELEPORTER) {
        int16_t first = dm2_v1_dungeon_get_first_thing(
            (const DM2_V1_DungeonData *)dungeon, rt->dungeon_level, x, y);
        if (first != DM2_V1_RECORD_HANDLE_NULL &&
            first != DM2_V1_RECORD_HANDLE_END && rt->record_pools_valid) {
            const uint8_t *trec = dm2_v1_record_pool_address(
                &rt->record_pools, first);
            if (trec) {
                uint16_t w4 = (uint16_t)trec[4] | ((uint16_t)trec[5] << 8);
                if ((w4 & 0x6) == 0x6) {
                    /* Redirect to floor mecha. */
                    return dm2_runtime_actuate_floor_mecha(
                        user, timer, source_index, receipt);
                }
            }
        }

        /* c_tim_proc.cpp:3853-3872 — toggle teleporter open bit (byte bit 3).
         * yB==2: query current state; result 0 = open, else close.
         * yB!=2: use yB directly as the action value. */
        uint8_t yB = (uint8_t)((timer->value_b >> 8) & 0xff);
        int action;
        if (yB == 2) {
            action = (raw & 0x08) ? 1 : 0;
        } else {
            action = (int)yB;
        }
        if (action == 0) {
            uint16_t new_raw = (uint16_t)(raw | 0x08);
            dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level,
                                         x, y, new_raw);
        } else {
            uint16_t new_raw = (uint16_t)(raw & ~0x08);
            dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level,
                                         x, y, new_raw);
        }
        /* Fall through to floor mecha like the source. */
        return dm2_runtime_actuate_floor_mecha(
            user, timer, source_index, receipt);
    }
    return 1;
}

/*
 * dm2_runtime_actuate_trickwall — DM2-owned class-6 handler for the 0x04
 * actuator tile subdispatch (Lane B, cycle 8).
 *
 * skproject/SKULLWIN/c_tim_proc.cpp:4220 dispatches square class 6 to
 * DM2_ACTUATE_TRICKWALL (c_tim_proc.cpp:3875).  The source body toggles the
 * trickwall open/closed state and may update the visible wall set; the exact
 * byte layout is not yet source-bound in Firestaff.  The bounded handler
 * consumes the timer in source order and increments a fail-closed counter.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4220 (class-6 dispatch)
 *         skproject/SKULLWIN/c_tim_proc.cpp:3875 (DM2_ACTUATE_TRICKWALL)
 */
static int dm2_runtime_actuate_trickwall(void *user,
                                         const DM2_V1_SourceTimer *timer,
                                         uint16_t source_index,
                                         DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int x, y, raw;
    uint16_t new_raw;

    (void)source_index;
    (void)receipt;

    rt->actuator_tile_timers++;
    rt->actuator_tile_trickwall++;

    if (timer == NULL || rt->boot == NULL || rt->boot->dungeon_data == NULL)
        return 1;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    raw = dm2_v1_dungeon_get_tile_raw(dungeon, rt->dungeon_level, x, y);
    if (raw < 0) return 1;

    /* c_tim_proc.cpp:3896-3904 — yB==2: query tile bit 2 (0x4).
     * Result 0 = passable (open), 1 = solid (closed). */
    {
        uint8_t yB = (uint8_t)((timer->value_b >> 8) & 0xff);
        int action;
        if (yB == 2) {
            action = (raw & 0x04) ? 1 : 0;
        } else {
            action = (int)(yB & 0xff);
        }

        if (action != 1) {
            /* c_tim_proc.cpp:3907-3908 — set bit 2 (make solid/wall). */
            new_raw = (uint16_t)(raw | 0x04);
            dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level,
                                         x, y, new_raw);
        } else {
            /* c_tim_proc.cpp:3910-3941 — try to open (clear bit 2).
             * Blocked if party is here or creature with AI flag bit 5
             * clear is present — re-queue timer. */
            int blocked = 0;

            if (rt->session_snapshot_valid &&
                rt->dungeon_level == (int)timer->ticks_and_map >> 24 &&
                (int)rt->session_snapshot.party_x == x &&
                (int)rt->session_snapshot.party_y == y) {
                blocked = 1;
            }

            if (!blocked && rt->record_pools_valid) {
                int16_t cr = dm2_v1_get_creature_at(
                    &rt->record_pools, (const DM2_V1_DungeonData *)dungeon,
                    rt->dungeon_level, x, y);
                if (cr != DM2_V1_RECORD_HANDLE_NULL) {
                    const uint8_t *crec = dm2_v1_record_pool_address(
                        &rt->record_pools, cr);
                    if (crec) {
                        uint16_t flags = 0;
                        dm2_v1_creature_ai_spec_flags(
                            (int)crec[4], &flags);
                        if ((flags & 0x20) == 0)
                            blocked = 1;
                    }
                }
            }

            if (blocked) {
                /* Re-queue: increment data (retry counter) and re-enqueue. */
                DM2_V1_SourceTimer requeue;
                requeue = *timer;
                requeue.value_b = (int16_t)(timer->value_b + 1);
                dm2_v1_runtime_enqueue_source_timer(&requeue, 0);
            } else {
                new_raw = (uint16_t)(raw & ~0x04);
                dm2_v1_dungeon_set_tile_raw(dungeon, rt->dungeon_level,
                                             x, y, new_raw);
            }
        }
    }

    /* c_tim_proc.cpp:3944-3946 — viewport redraw flag. */
    return 1;
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
    rt->tick_count++;

    /* skweathr.cpp derives environmental time from its source globals and
     * tick schedule. The former fixed 1,092-tick minute was invented, so do
     * not advance a recovered value until that owner is imported. */

    /* Movement cooldown counts down */
    if (rt->move_cooldown_ticks > 0) {
        rt->move_cooldown_ticks--;
    }

    /* SKProject c_savegame.cpp starts c_weather only from the recovered
     * v1e14xx environment globals.  Firestaff has not yet imported those
     * globals, so merely entering an outdoor map must not invent a chain,
     * RNG seed, timers, clouds, rain or lightning.  A future source importer
     * may set weather_chain_started with its complete record receipt. */
    if (!rt->outdoor) {
        rt->weather_chain_started = 0;
        rt->weather_source_timer_pending = 0;
    }

    dm2_runtime_process_time_triggers(rt, rt->tick_count * 55);

    /* DM2-003: every DM2 timer routes through the DM2-owned source-order
     * dispatcher (skproject/SKULLWIN/c_tim_proc.cpp:3980-4230
     * DM2_PROCEED_TIMERS).  The former unconditional host-side creature
     * simulation is removed: creature state advances only when a
     * source-ordered 0x21/0x22 DM2_THINK_CREATURE timer is dispatched,
     * and known timer types without a bound DM2-owned handler are
     * acknowledged fail-closed, never simulated.  The 0x54 dispatch is
     * bound to DM2_UPDATE_WEATHER(1) via dm2_runtime_update_weather_timer
     * above; the handler owns the source re-queue. */
    {
        DM2_V1_TimerDispatcher dispatcher;
        /* Session-owned record pools + the per-cell think binding are
         * populated lazily from the boot dungeon data; without validated
         * G1 evidence the 0x21/0x22 handlers stay unbound and the
         * dispatcher acknowledges those timers fail-closed. */
        dm2_runtime_ensure_think_binding(rt);
        memset(&dispatcher, 0, sizeof(dispatcher));
        dispatcher.context = rt;
        if (rt->think_binding_ready) {
            dispatcher.handlers[DM2_V1_TIMER_THINK_CREATURE_A] =
                dm2_runtime_think_creature_timer;
            dispatcher.handlers[DM2_V1_TIMER_THINK_CREATURE_B] =
                dm2_runtime_think_creature_timer;
        }
        /* c_tim_proc's 0x04 dispatch is record-owned: its DB3/DB14 target,
         * direction and payload decide every observable square mutation.
         * Firestaff has the timer bytes but not that complete GAME_LOAD
         * transaction yet.  In particular, the old class-2/4/5/6 helpers
         * derived pit, door, teleporter and trick-wall state from value_b;
         * that was a host-side substitute, not an authenticated actuator.
         * Leave all actuator classes unbound so PROCEED_TIMERS consumes an
         * unsupported source timer without changing the real dungeon.
         *
         * Re-admit a class only with its live DB3/DB14 record link, payload
         * grammar, map owner and any required follow-up timer in one receipt.
         * Source: skproject/SKULLWIN/c_tim_proc.cpp:4214-4230,
         * DM2_INVOKE_ACTUATOR / DM2_INVOKE_MESSAGE. */
        dispatcher.tile_class_at = dm2_runtime_tile_class_at;
        /* No dispatcher.actuator_tile[] binding until the source transaction
         * above exists. */
        /* Keep the bounded transcriptions compiled as source studies for
         * their focused regressions, but make their non-registration explicit
         * to both readers and strict warning builds.  These expressions take
         * no action and do not install a callback. */
        (void)dm2_runtime_actuate_wall_mecha;
        (void)dm2_runtime_actuate_pitfall;
        (void)dm2_runtime_actuate_door;
        (void)dm2_runtime_actuate_teleporter;
        (void)dm2_runtime_actuate_trickwall;
        /* Ornament animation/noise both mutate or requeue through an
         * actuator record. A raw pool address is insufficient without the
         * original animator/timer ownership, so retain no live callback. */
        (void)dm2_runtime_ornate_animator_timer;
        /* CONTINUE_TICK_GENERATOR invokes the same incomplete actuator
         * transaction.  Do not let its convenience record decode enqueue a
         * guessed 0x04 mutation. */
        (void)dm2_runtime_tick_generator_timer;
        dispatcher.handlers[DM2_V1_TIMER_UPDATE_WEATHER] =
            dm2_runtime_update_weather_timer;
        /* STEP/DESTROY_DOOR likewise need the decoded DB0 direction,
         * collision, sound and queue state.  The old bit-only handlers are
         * intentionally not registered in a real-data runtime. */
        (void)dm2_runtime_door_step_timer;
        (void)dm2_runtime_destroy_door_timer;
        /* 0x58/0x59/0x5b/0x5c alter DB records. GAME_LOAD has not restored
         * their original timer queue and record transaction as one unit, so
         * do not treat a boot-time raw record pool as mutable live state. */
        (void)dm2_runtime_release_door_button_timer;
        (void)dm2_runtime_process_timer_59;
        (void)dm2_runtime_5b_record_clear;
        (void)dm2_runtime_5c_record_set;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_0C] =
            dm2_runtime_process_0c_timer;
        dispatcher.handlers[DM2_V1_TIMER_RESURRECTION] =
            dm2_runtime_resurrection_timer;
        /* PROCESS_0E temporarily changes an item record and must restore it
         * through the same c_hero/inventory owner.  The local pool address
         * alone is not that owner, so do not mutate it in production. */
        (void)dm2_runtime_process_0e_timer;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_SOUND] =
            dm2_runtime_process_sound_timer;
        /* PROCESS_3D and MOVE_RECORD_ROTATE require MOVE_RECORD_TO's full
         * link, wake/sleep and party transaction.  Their old direct writes
         * could relocate original records or the party from timer bytes
         * alone, so both stay unbound until that source owner is present. */
        (void)dm2_runtime_process_3d_timer;
        (void)dm2_runtime_ornate_noise_timer;
        (void)dm2_runtime_move_record_rotate_timer;
        /* Spell-effect timer delegation: 0x46 light, 0x47 hero ench flag,
         * 0x48 ench power, 0x4B poison, 0x19 cloud, 0x1E missile, 0x5E summon.
         * 0x47/0x48/0x4B consume in source order but are deliberately
         * non-mutating: session champion records are not SKProject c_hero.
         * Do not copy that surrogate merely to make an effect appear live. */
        if (rt->session_snapshot_valid &&
            rt->session_snapshot.champion_count > 0 &&
            rt->session_snapshot.champion_count <= 4) {
            int sc = rt->session_snapshot.champion_count;
            dm2_v1_spell_timer_handler_context_init_ex(
                &rt->spell_timer_ctx,
                NULL, sc,
                &rt->timer_queue,
                (uint32_t)rt->tick_count,
                rt->session_snapshot.party_level,
                rt->record_pools_valid ? &rt->record_pools : NULL,
                rt->boot ? (struct DM2_V1_DungeonData *)rt->boot->dungeon_data : NULL,
                rt->caii_ready ? &rt->caii : NULL);
            rt->spell_timer_ctx_ready = 1;
        } else {
            rt->spell_timer_ctx_ready = 0;
        }
        dispatcher.handlers[DM2_V1_TIMER_LIGHT] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_HERO_ENCH_FLAG] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_ENCH_POWER] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_POISON] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_CLOUD] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_STEP_MISSILE] =
            dm2_runtime_spell_timer_delegate;
        dispatcher.handlers[DM2_V1_TIMER_ALLOC_NEW_CREATURE] =
            dm2_runtime_spell_timer_delegate;
        (void)dm2_v1_proceed_timers(&rt->timer_queue,
                                    (uint32_t)rt->tick_count,
                                    &dispatcher,
                                    &rt->proceed_timers);
        rt->spell_timer_ctx_ready = 0;
    }

    /* Drain sound queue — DM2_SOUND8 (c_sound.cpp:633-647).
     * Timer handlers (0x15, 0x5A) and actuator events push entries;
     * the flush drains them to the audio backend each tick. */
    if (rt->sound_queue_ready) {
        DM2_V1_SoundPlayReceipt snd_receipt;
        memset(&snd_receipt, 0, sizeof(snd_receipt));
        dm2_v1_sound_queue_sound8_flush(&rt->sound_queue, 0, &snd_receipt);
    }

    /* CDDA flush: push queued FM Towns PCM to the SDL3 audio stream */
    {
        DM2_V1_CddaFlushReceipt cdda;
        if (dm2_v1_sound_flush_cdda(&cdda) && cdda.valid &&
            rt->cdda_play_cb) {
            rt->cdda_play_cb(rt->cdda_cb_ctx,
                             cdda.pcm_data, cdda.pcm_size,
                             cdda.loop);
        }
    }

    /* After any 0x54 weather timer has stepped the v1e14xx chain, run the
     * arg==0 DM2_UPDATE_WEATHER(0) frame update to produce the live
     * DistantEnvironment slots that drive real GDAT weather overlays this
     * frame.  This binds the timer state machine to the renderer receipt;
     * without it, real weather assets stay no-draw.
     *
     * Source: skproject/SKULLWIN/c_weather.cpp:91-506. */
    if (rt->outdoor && rt->weather_chain_started) {
        (void)dm2_v1_runtime_update_weather_frame(NULL, NULL);
    }

    /* A map being outdoors is not the source-owned c_weather timer chain.
     * DM2_SET_TIMER_WEATHER is reached only after GAME_LOAD restores the
     * v1e14xx state and its 0x54 timer owner.  Do not manufacture a valid
     * receipt from the outdoor flag/tick counter: it could otherwise combine
     * with independently supplied GDAT slots and authorize weather pixels.
     *
     * Source: SKProject/SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD and
     *         SKWINSPX/src/v5/c_weather.cpp::DM2_SET_TIMER_WEATHER. */
    memset(&rt->set_timer_weather, 0, sizeof(rt->set_timer_weather));
    if (rt->outdoor && rt->weather_chain_started) {
        (void)dm2_v1_weather_set_timer_weather_receipt(
            1, (uint32_t)rt->tick_count, &rt->set_timer_weather);
    }

    /* Do not advance the legacy standalone creature pool here.  Its only
     * producer is explicitly fixture-only: SKProject ALLOC_NEW_CREATURE
     * instead allocates and links a live DB4 record before its CCM command
     * stream can execute.  Production creature changes therefore belong
     * exclusively to the source-ordered 0x21/0x22 handler above, after that
     * handler has bound the DB4 record, CAII row and command stream.  Calling
     * dm2_v1_creature_tick() here would revive the discarded host-side pool
     * as a second, unowned simulation clock.
     *
     * Source: skproject/SKULLWIN/c_tim_proc.cpp:3980-4230
     *         (DM2_PROCEED_TIMERS), c_ai.cpp (DM2_THINK_CREATURE),
     *         skcrture.cpp:6380-6430 (ALLOC_NEW_CREATURE). */

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

DM2_V1_SourceTimerResult dm2_v1_runtime_enqueue_source_timer(
    const DM2_V1_SourceTimer *timer, uint16_t source_index)
{
    /* DM2-003: single DM2-owned entry point for runtime timers.  Ordering
     * is skproject c_timer.cpp DM2_cmp_timers via dm2_v1_source_timer_enqueue;
     * dispatch happens in dm2_v1_runtime_tick through dm2_v1_proceed_timers. */
    return dm2_v1_source_timer_enqueue(&g_dm2_runtime.timer_queue, timer,
                                       source_index);
}

int dm2_v1_runtime_last_proceed_timers_receipt(
    DM2_V1_ProceedTimersReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.proceed_timers;
    return out_receipt->valid;
}

int dm2_v1_runtime_weather_source_timer_pending(void)
{
    return g_dm2_runtime.weather_source_timer_pending;
}

int dm2_v1_runtime_weather_chain_started(void)
{
    return g_dm2_runtime.weather_chain_started;
}

int dm2_v1_runtime_weather_chain_snapshot(DM2_V1_UpdateWeatherState *out)
{
    if (!out || !g_dm2_runtime.weather_chain_started) {
        return 0;
    }
    *out = g_dm2_runtime.weather_chain;
    return 1;
}

/* DM2-007 cycle 12: capture spell-cast failure feedback for M11's DM2 status
 * scope.  The caller (future DM2 spell-cast UI) provides the apply receipt;
 * failure classes are mapped to source-named status strings. */
static const char *dm2_runtime_spell_failure_status(int failure_class)
{
    switch (failure_class) {
    case 0x10: return "DM2 SPELL FAILED";
    case 0x20: return "DM2 UNKNOWN RUNES";
    case 0x30: return "DM2 NEED FLASK";
    default:   return "DM2 SPELL FAILED";
    }
}

void dm2_v1_runtime_note_spell_cast_apply_receipt(
    const DM2_V1_SpellCastApplyReceipt *a)
{
    if (!a || !a->valid) {
        g_dm2_runtime.last_spell_status_scope = NULL;
        g_dm2_runtime.last_spell_status = NULL;
        g_dm2_runtime.last_spell_failure_class = 0;
        return;
    }
    if (a->failure_feedback && a->failure_class != 0) {
        g_dm2_runtime.last_spell_status_scope = "DM2 SPELL";
        g_dm2_runtime.last_spell_status =
            dm2_runtime_spell_failure_status(a->failure_class);
        g_dm2_runtime.last_spell_failure_class = a->failure_class;
    } else {
        g_dm2_runtime.last_spell_status_scope = NULL;
        g_dm2_runtime.last_spell_status = NULL;
        g_dm2_runtime.last_spell_failure_class = 0;
    }
}

const char *dm2_v1_runtime_status_scope(void)
{
    return g_dm2_runtime.last_spell_status_scope;
}

const char *dm2_v1_runtime_status_message(void)
{
    return g_dm2_runtime.last_spell_status;
}

int dm2_v1_runtime_last_spell_failure_class(void)
{
    return g_dm2_runtime.last_spell_failure_class;
}

/* Build one c_weather.cpp DistantEnvironment ten-byte register image from the
 * source frame receipt and the already-verified GDAT command receipt.  Cloud
 * and rain keep the GDAT FW key; bolts keep the c_weather.cpp RANDDIR byte.
 * Source: skproject/SKULLWIN/c_weather.cpp:221-266 (slot write) and
 *         c_bkgrnd.cpp ENVIRONMENT_DRAW_DISTANT_ELEMENT (slot read). */
static int dm2_runtime_build_weather_slot_raw(
    const DM2_V1_WeatherGdatReceipt *weather,
    const DM2_V1_UpdateWeatherFrameReceipt *frame,
    unsigned int slot_index,
    uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES])
{
    const DM2_V1_WeatherCommandReceipt *command;
    uint8_t cmd;
    uint8_t flip;

    if (!weather || !frame || !raw || slot_index >= 3u ||
        slot_index >= (unsigned int)frame->slots) {
        return 0;
    }
    cmd = frame->live_cmds[slot_index];
    if (cmd < DM2_V1_WEATHER_BOLT_CMD_BASE ||
        cmd > DM2_V1_WEATHER_RAIN_STORM_CMD) {
        return 0;
    }
    command = &weather->commands[(unsigned int)(cmd - DM2_V1_WEATHER_BOLT_CMD_BASE)];
    if (command->command != cmd || !command->material_valid) {
        return 0;
    }
    if (cmd >= DM2_V1_WEATHER_BOLT_CMD_BASE &&
        cmd <= DM2_V1_WEATHER_BOLT_CMD_LAST) {
        /* c_weather.cpp:471 writes RANDDIR (0..3) into cmFW after a
         * successful bolt retrieve; only value 2 evaluates the 0x20 mirror
         * in ENVIRONMENT_DRAW_DISTANT_ELEMENT. */
        flip = (uint8_t)((unsigned int)frame->bolt_dir & 3u);
    } else {
        flip = command->flip_mode;
    }
    memset(raw, 0, DM2_V1_DISTANT_ENVIRONMENT_BYTES);
    raw[0] = cmd;
    raw[1] = flip;
    raw[2] = (uint8_t)(command->rect_number & 0xffu);
    raw[3] = (uint8_t)(command->rect_number >> 8);
    /* RETRIEVE_ENVIRONMENT_CMD_CD_FW initializes w4/w6 to zero and b8/b9
     * to 0x40 (c_querydb.cpp DM2_RETRIEVE_ENVIRONMENT_CMD_CD_FW). */
    raw[8] = 0x40u;
    raw[9] = 0x40u;
    return 1;
}

/* Run one DM2_UPDATE_WEATHER(0) frame update against the session-owned weather
 * chain and bind the resulting live DistantEnvironment slots to the runtime.
 * This is the missing link between the 0x54 timer state machine and the
 * source-owned weather renderer receipt: without live slots the renderer stays
 * no-draw even when real GDAT weather assets exist.
 *
 * Source: skproject/SKULLWIN/c_weather.cpp:91-506 (arg == 0 frame update). */
/* Test-only helper: replace the session-owned weather chain state.  This
 * lives only in the fixture-specific compilation of this translation unit;
 * a production executable has no setter for c_weather's live v1e14xx chain.
 *
 * Source: skproject/SKULLWIN/c_weather.cpp v1e14xx globals. */
#if defined(FIRESTAFF_DM2_RUNTIME_TESTING)
int dm2_v1_runtime_set_weather_chain_state_for_test(
    const DM2_V1_UpdateWeatherState *state)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (!state) return 0;
    rt->weather_chain = *state;
    rt->weather_chain_started = 1;
    return 1;
}
#endif

int dm2_v1_runtime_update_weather_frame(
    DM2_V1_DistantEnvironmentReceipt *out_slots,
    unsigned int *out_slot_count)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_WeatherGdatReceipt weather;
    DM2_V1_UpdateWeatherFrameReceipt frame;
    DM2_V1_DistantEnvironmentReceipt slots[3];
    unsigned int slot_count = 0u;
    unsigned int retrieve_mask = 0u;
    uint16_t gdat_entry_6c = 0u;
    unsigned int i;

    if (out_slot_count) *out_slot_count = 0u;
    if (out_slots) memset(out_slots, 0,
                          sizeof(DM2_V1_DistantEnvironmentReceipt) * 3u);
    if (!rt->outdoor || !rt->weather_chain_started ||
        !rt->gdat_weather_receipt_ready || !rt->boot) {
        (void)dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u);
        return 0;
    }
    memset(&weather, 0, sizeof(weather));
    if (!dm2_v1_boot_weather_gdat_receipt(rt->boot, rt->map_graphics_style,
                                          &weather) ||
        !weather.valid ||
        weather.receipt_hash != rt->gdat_weather_receipt_hash) {
        (void)dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u);
        return 0;
    }
    /* RETRIEVE_ENVIRONMENT_CMD_CD_FW succeeds only when the selected command
     * has a verified GDAT material receipt. */
    if (weather.material_mask &
        (DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_CLOUD_LIGHT_CMD) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_CLOUD_HEAVY_CMD) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_CLOUD_STORM_CMD))) {
        retrieve_mask |= DM2_V1_UPDATE_WEATHER_RETRIEVE_CLOUD;
    }
    if (weather.material_mask &
        (DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_RAIN_LIGHT_CMD) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_RAIN_HEAVY_CMD) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_RAIN_STORM_CMD))) {
        retrieve_mask |= DM2_V1_UPDATE_WEATHER_RETRIEVE_RAIN;
    }
    if (weather.material_mask &
        (DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_BOLT_CMD_BASE) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_BOLT_CMD_BASE + 1u) |
         DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_BOLT_CMD_LAST))) {
        retrieve_mask |= DM2_V1_UPDATE_WEATHER_RETRIEVE_BOLT;
    }
    if (weather.material_mask &
        DM2_V1_WEATHER_COMMAND_MASK(DM2_V1_WEATHER_RAIN_STORM_CMD)) {
        gdat_entry_6c = 1u;
    }
    memset(&frame, 0, sizeof(frame));
    if (!dm2_v1_update_weather_0(&rt->weather_chain, (int32_t)rt->tick_count,
                                 retrieve_mask, gdat_entry_6c,
                                 &rt->weather_rng, &frame) ||
        !frame.valid) {
        (void)dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u);
        return 0;
    }
    memset(slots, 0, sizeof(slots));
    for (i = 0u; i < (unsigned int)frame.slots && i < 3u; ++i) {
        uint8_t raw[DM2_V1_DISTANT_ENVIRONMENT_BYTES];
        if (!dm2_runtime_build_weather_slot_raw(&weather, &frame, i, raw)) {
            (void)dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u);
            return 0;
        }
        if (!dm2_v1_weather_distant_environment_receipt(
                &weather, frame.live_cmds[i], (uint8_t)i, raw, &slots[i]) ||
            !slots[i].valid) {
            (void)dm2_v1_runtime_bind_weather_distant_environment(NULL, 0u);
            return 0;
        }
        ++slot_count;
    }
    if (!dm2_v1_runtime_bind_weather_distant_environment(slots, slot_count)) {
        return 0;
    }
    if (out_slot_count) *out_slot_count = slot_count;
    if (out_slots) {
        memcpy(out_slots, slots,
               sizeof(DM2_V1_DistantEnvironmentReceipt) * slot_count);
    }
    return 1;
}

int dm2_v1_runtime_record_pools_valid(void)
{
    return g_dm2_runtime.record_pools_valid;
}

int dm2_v1_runtime_think_creature_receipt(DM2_V1_ThinkCreatureReceipt *out)
{
    if (!out || !g_dm2_runtime.think_binding_ready) {
        return 0;
    }
    *out = g_dm2_runtime.think_binding.receipt;
    return 1;
}

int dm2_v1_runtime_floor_mecha_receipt(DM2_V1_RuntimeFloorMechaReceipt *out)
{
    if (!out || !g_dm2_runtime.think_binding_ready) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->timers = g_dm2_runtime.floor_mecha_timers;
    out->records_0x3a = g_dm2_runtime.floor_mecha_0x3a_records;
    out->activations = g_dm2_runtime.floor_mecha_activations;
    out->allocs = g_dm2_runtime.floor_mecha_allocs;
    out->db_break = g_dm2_runtime.floor_mecha_db_break;
    out->walk_failed = g_dm2_runtime.floor_mecha_walk_failed;
    out->valid = 1;
    return 1;
}

int dm2_v1_runtime_door_step_receipt(DM2_V1_RuntimeDoorStepReceipt *out)
{
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->timers = g_dm2_runtime.door_step_timers;
    out->mutations = g_dm2_runtime.door_step_mutations;
    out->requeues = g_dm2_runtime.door_step_requeues;
    out->valid = 1;
    return 1;
}

int dm2_v1_runtime_actuator_tile_receipt(
    DM2_V1_RuntimeActuatorTileReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->timers = rt->actuator_tile_timers;
    out->wall_mecha = rt->actuator_tile_wall_mecha;
    out->floor_mecha = rt->floor_mecha_timers;
    out->pitfall = rt->actuator_tile_pitfall;
    out->door = rt->actuator_tile_door;
    out->teleporter = rt->actuator_tile_teleporter;
    out->trickwall = rt->actuator_tile_trickwall;
    out->unbound_fail_closed =
        out->wall_mecha + out->teleporter + out->trickwall;
    out->pitfall_rejected = rt->actuator_tile_pitfall_rejected;
    out->door_rejected = rt->actuator_tile_door_rejected;
    out->valid = 1;
    return 1;
}

/*
 * dm2_v1_runtime_schedule_creature_at — DM2-owned boundary for the
 * creature-scheduling producer DM2_1c9a_0cf7 (c_1c9a.cpp:5695-5728).
 *
 * The source invokes the producer from spawn/activation sites
 * (DM2_ALLOC_CAII_TO_CREATURE map-load instantiation, c_creature.cpp:648,
 * c_move.cpp:700, c_ai.cpp:5958) that are not yet bound; this boundary
 * exposes the producer over the session-owned record pools, boot dungeon
 * data and source timer queue so those future bindings — and tests — can
 * drive it.  The CAII slot timer word and the DM2_1c9a_0db0 delete stay
 * host-owned until the CCM body is proven (receipted, never simulated).
 */
int dm2_v1_runtime_schedule_creature_at(int map_id, int x, int y,
                                        DM2_V1_CreatureScheduleReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    const DM2_V1_DungeonData *dungeon;

    dm2_runtime_ensure_think_binding(rt);
    if (!rt->think_binding_ready || !rt->boot || !rt->boot->dungeon_data) {
        return 0;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    return dm2_v1_creature_schedule_at(&rt->record_pools, dungeon,
                                       &rt->timer_queue, map_id,
                                       (unsigned long)rt->tick_count,
                                       x, y, out);
}

int dm2_v1_runtime_caii_init(int capacity)
{
#ifdef FIRESTAFF_DM2_CAII_TESTING
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (rt->caii_ready) {
        dm2_v1_caii_array_free(&rt->caii);
        rt->caii_ready = 0;
    }
    dm2_v1_caii_array_init(&rt->caii, capacity);
    rt->caii_ready = rt->caii.valid;
    return rt->caii_ready;
#else
    (void)capacity;
    /* DM2_INIT derives ddat.v1e08a0 from the original session/save owner.
     * A caller-supplied capacity is fixture data, never a valid live CAII
     * allocation contract. */
    return 0;
#endif
}

/*
 * dm2_v1_runtime_alloc_caii_at — DM2-owned lazy creature-activation
 * boundary: the bounded slice of DM2_ALLOC_CAII_TO_CREATURE
 * (c_1c9a.cpp:5772-5894) reached the way DM2_ATTACK_CREATURE reaches it
 * (record resolved via DM2_GET_CREATURE_AT at the activation cell,
 * c_creature.cpp:347-352).  The session is single-map, so the map id is
 * 0 and the gametick is the session tick count.
 */
int dm2_v1_runtime_alloc_caii_at(int x, int y,
                                 DM2_V1_CaiiAllocReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    const DM2_V1_DungeonData *dungeon;
    int16_t handle;

    dm2_runtime_ensure_think_binding(rt);
    if (!rt->think_binding_ready || !rt->caii_ready ||
        !rt->boot || !rt->boot->dungeon_data) {
        return 0;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    handle = dm2_v1_get_creature_at(&rt->record_pools, dungeon, 0, x, y);
    if (handle == DM2_V1_RECORD_HANDLE_NULL) {
        return 0;
    }
    return dm2_v1_caii_alloc_to_creature(&rt->record_pools, dungeon,
                                         &rt->caii, &rt->timer_queue, 0,
                                         (unsigned long)rt->tick_count,
                                         handle, x, y, out);
}

int dm2_v1_runtime_caii_ready(void)
{
    return g_dm2_runtime.caii_ready;
}

int dm2_v1_runtime_caii_alloc_count(void)
{
    return g_dm2_runtime.caii_ready ? g_dm2_runtime.caii.alloc_count : 0;
}

int dm2_v1_runtime_caii_set_slot_mode_byte(int slot_index, int value)
{
    (void)slot_index;
    (void)value;
    /* The source writes CAII byte@1a only from named CCM/record owners
     * (SKProject c_1c9a.cpp:5921-5929).  This old public setter accepted an
     * arbitrary slot and mode solely to manufacture the 0x13 delete branch
     * in a fixture.  A real session must never mutate CAII that way. */
    return 0;
}

int dm2_v1_runtime_last_delete_full_receipt(
    DM2_V1_DeleteCreatureFullReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (out == 0 || !rt->last_delete_full_valid) {
        return 0;
    }
    *out = rt->last_delete_full;
    return 1;
}

/*
 * dm2_v1_runtime_reschedule_creature_at — the complete DM2_1c9a_0cf7
 * replacement slice over the session CAII array (c_1c9a.cpp:5695-5728),
 * exposed for the source's direct producer callers (c_creature.cpp:648,
 * c_move.cpp:700) and for tests.  The session is single-map, so the map
 * id is 0 and the gametick is the session tick count.
 */
int dm2_v1_runtime_reschedule_creature_at(int x, int y,
                                          DM2_V1_CreatureScheduleReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    const DM2_V1_DungeonData *dungeon;

    dm2_runtime_ensure_think_binding(rt);
    if (!rt->think_binding_ready || !rt->caii_ready ||
        !rt->boot || !rt->boot->dungeon_data) {
        return 0;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    return dm2_v1_caii_schedule_creature_at(&rt->record_pools, dungeon,
                                            &rt->caii, &rt->timer_queue, 0,
                                            (unsigned long)rt->tick_count,
                                            x, y, out);
}

int dm2_v1_runtime_free_caii_slot(int slot_index,
                                  DM2_V1_CaiiFreeReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    const DM2_V1_DungeonData *dungeon;

    dm2_runtime_ensure_think_binding(rt);
    if (!rt->think_binding_ready || !rt->caii_ready) {
        return 0;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    return dm2_v1_caii_free_slot(&rt->record_pools, dungeon, &rt->caii,
                                 &rt->timer_queue, slot_index, out);
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
    uint8_t image_field = 0u;
    uint32_t index = 0u;
    int category;
    DM2_ItemSprite *dst;

    if (!rt || !viewport) return;
    if (!dm2_db_decode_handle(rt->leader_hand_object, &pool, &index)) return;
    category = dm2_v1_viewport_item_category_for_db_pool((int)pool);
    if (index > 0xffu || category == 0) {
        return;
    }
    if (!rt->boot || !rt->boot->graphics_dat ||
        !dm2_v1_boot_leader_hand_image_field(
            rt->boot, category, (int)index, index,
            (uint32_t)rt->tick_count, rt->view_dir, &image_field)) {
        /* SKProject DM2_DRAW_ITEM_IN_HAND (skguidr5.cpp:1517) first derives
         * the item frame through _2405_014a, then queries that exact GDAT
         * image and its local palette. A generic provider and its field zero
         * cannot establish either source value, so no carried-item pixel is
         * admitted until the boot-owned transaction resolves. */
        return;
    }

    dst = &viewport->carried_item;
    memset(dst, 0, sizeof(*dst));
    dst->item_category = (uint8_t)category;
    dst->item_type = (uint8_t)(index & 0xffu);
    dst->frame_index = image_field;
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

        dst->occupied = champ->first_name[0] != '\0' ||
                        champ->cur_hp != 0u || champ->max_hp != 0u;
        dst->leader = slot == hud.leader_index;
        dst->hp_pct =
            dm2_runtime_hud_pct_from_current_max(champ->cur_hp,
                                                 champ->max_hp);
        dst->stamina_pct =
            dm2_runtime_hud_pct_from_current(champ->stamina);
        dst->mana_pct = dm2_runtime_hud_pct_from_current(champ->mana);
        /* SKProject INIT sets glbChampionColor to 7,11,8,14 before its HUD
         * path. Keep the bootstrap value explicit until a source save/runtime
         * mutation of that global is independently admitted. */
        static const uint8_t source_default_stat_bar_color[
            DM2_V1_HUD_CHAMPION_SLOT_COUNT] = { 7u, 11u, 8u, 14u };
        dst->stat_bar_color = source_default_stat_bar_color[slot];
        dst->stat_bar_color_source_bound = 1;
        dst->portrait_index = 0u;
        /* SKWINDOS/src/c_hero.h places herotype at byte 257 of the PC-DOS
         * 0x107-byte c_hero record.  REVIVE_PLAYER writes it from the source
         * mirror actuator and DRAW_CHAMPION_PICTURE uses that exact GDAT
         * index.  The local portrait_index tail is not a substitute. */
        dst->portrait_type_source_bound = 0;
        char source_first_name[DM2_V1_HUD_CHAMPION_NAME_MAX + 1];
        memset(source_first_name, 0, sizeof(source_first_name));
        memcpy(source_first_name, champ->first_name,
               DM2_V1_HUD_CHAMPION_NAME_MAX);
        source_first_name[DM2_V1_HUD_CHAMPION_NAME_MAX] = '\0';
        if (dst->occupied && rt->session_snapshot.original_champion_records_valid) {
            dst->portrait_index = rt->session_snapshot
                .original_champion_records[slot][257];
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
 * The runtime admits a frame only through the source-backed GDAT material
 * routes below.  Missing material blocks the frame rather than substituting
 * a host-coloured placeholder.
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
    int hud_material_plan_command_count = 0;
    int hud_material_plan_required = 0;
    int hud_material_plan_consumed = 0;
    uint32_t creature_material_plan_hash = 0u;
    uint32_t creature_drawn_material_hash = 0u;
    int creature_material_plan_required = 0;
    int creature_material_plan_consumed = 0;
    int creature_material_plan_count = 0;
    int creature_drawn_material_count = 0;
    uint32_t projectile_drawn_material_hash = 0u;
    int projectile_material_plan_required = 0;
    int projectile_material_plan_consumed = 0;
    int projectile_drawn_material_count = 0;
    uint32_t item_drawn_material_hash = 0u;
    int item_material_plan_required = 0;
    int item_material_plan_consumed = 0;
    int item_drawn_material_count = 0;
    uint32_t wall_drawn_material_hash = 0u;
    int wall_drawn_material_count = 0;
    int wall_material_plan_command_count = 0;
    int wall_material_plan_consumed = 0;
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
    DM2_V1_GdatSceneM11CommandPlan scene_material_plan;
    const DM2_V1_GdatSceneM11CommandPlan *scene_material_plan_for_m11;
    DM2_V1_InterfaceRect14HostReceipt rect14_host;
    DM2_V1_WeatherRestoredStateReceipt weather_state;
    DM2_V1_WeatherDrawContext weather_context;
    DM2_V1_WeatherRendererReceipt weather_renderer;
    DM2_V1_OutdoorWeatherM11Receipt weather_m11;
    int map_offset_x = 0;
    int map_offset_y = 0;
    static const int forward_dx[4] = { 0, 1, 0, -1 };
    static const int forward_dy[4] = { -1, 0, 1, 0 };

    if (!framebuffer || fb_stride <= 0 ||
        view_w < DM2_VP_WIDTH || view_h < DM2_VP_HEIGHT) {
        return -1;
    }

    /* SKProject DM2_GAME_LOAD reaches DRAW_DUNGEON only after the original
     * dungeon and graphics owners have both mounted.  A direct caller with
     * an empty boot profile (or an injected fixture provider) used to obtain
     * a successful no-source frame here.  That is not a playable DM2 state:
     * require the same hash-verified boot-owned GDAT provider that M11 binds
     * through dm2_v1_runtime_bind_boot_profile(). */
    if (!rt->boot || !rt->boot->assets_verified ||
        !rt->boot->graphics_dat || !rt->boot->dungeon_data ||
        rt->viewport_asset_fetch != dm2_v1_boot_viewport_asset_fetch ||
        rt->viewport_asset_user != rt->boot ||
        rt->viewport_asset_palette_fetch !=
            dm2_v1_boot_viewport_asset_palette_fetch ||
        rt->viewport_asset_palette_user != rt->boot) {
        return -1;
    }

    memset(&g_dm2_last_creature_render, 0, sizeof(g_dm2_last_creature_render));
    memset(&g_dm2_last_item_render, 0, sizeof(g_dm2_last_item_render));
    memset(&g_dm2_last_projectile_render, 0,
           sizeof(g_dm2_last_projectile_render));
    memset(&g_dm2_last_door_render, 0, sizeof(g_dm2_last_door_render));
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
            map_offset_x = dungeon->map_offset_x[rt->dungeon_level];
            map_offset_y = dungeon->map_offset_y[rt->dungeon_level];
        }
    }
    dm2_v1_viewport_set_outdoor(&viewport, rt->outdoor);
    if (rt->time_of_day_minutes >= 0 &&
        rt->time_of_day_minutes < DM2_TIME_MINUTES_MAX) {
        dm2_v1_viewport_set_time(
            &viewport,
            (float)rt->time_of_day_minutes /
                (float)DM2_TIME_MINUTES_MAX);
    }
    viewport.random_seed = rt->weather.weather_seed;
    dm2_runtime_populate_visible_terrain(rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_projectiles(&viewport, party_dir, party_x, party_y);
    /* skproject DRAW_MAP_CHIP dereferences a DB4 Creature record before it
     * reads AI state or GDAT. The local CCM pool has no source-owned record
     * handle, so it may advance simulation but must not manufacture a
     * viewport sprite or fallback image. Direct G1 DB4 material below is the
     * only admitted creature route until that ownership bridge exists. */
    dm2_runtime_populate_creatures(rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_g1_static_object_materials(rt, party_dir, party_x, party_y);
    dm2_runtime_populate_g1_weapon_map_chip_items(
        rt, &viewport, party_dir, party_x, party_y);
    dm2_runtime_populate_g1_container_map_chip_items(
        rt, &viewport, party_dir, party_x, party_y);
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
    if (rt->boot && rt->boot->graphics_dat) {
        dm2_v1_viewport_set_asset_loader(
            &viewport, dm2_v1_boot_asset_loader(rt->boot));
    }
    dm2_v1_viewport_set_source_materials_required(
        &viewport,
        rt->viewport_asset_fetch == dm2_v1_boot_viewport_asset_fetch &&
        rt->viewport_asset_user != NULL);
    /* Keep the selected DB5/DB9 F9 bytes stable through this M11 frame.
     * The renderer consumes this direct receipt instead of resolving the
     * virtual GDAT address a second time. */
    dm2_runtime_bind_g1_scene_static_item_materials(rt, &viewport);
    /* c_weather.cpp emits DistantEnvironment records before its image draw.
     * Generic weather intensity is not an image selector. Consume weather
     * pixels only when a source-owned slot was explicitly admitted and the
     * party is stationary, where the source transform is fully known. */
    rt->gdat_weather_renderer_ready = 0;
    rt->gdat_weather_renderer_hash = 0u;
    rt->gdat_weather_renderer_command_count = 0u;
    memset(&weather_state, 0, sizeof(weather_state));
    memset(&weather_context, 0, sizeof(weather_context));
    memset(&weather_renderer, 0, sizeof(weather_renderer));
    memset(&weather_m11, 0, sizeof(weather_m11));
    if (rt->outdoor && !rt->scene_movement_pending &&
        rt->weather_distant_slot_count > 0u &&
        rt->weather_distant_slots_map_token ==
            dm2_v1_runtime_g1_scene_map_token(rt->dungeon_level,
                                               rt->map_graphics_style,
                                               rt->outdoor) &&
        rt->weather_distant_slots_source_receipt_hash ==
            rt->gdat_weather_receipt_hash &&
        rt->weather_distant_slots_graphicsset ==
            (uint8_t)rt->map_graphics_style) {
        if (dm2_v1_weather_restored_state_receipt(&rt->weather, &weather_state)) {
            weather_context.direction = (uint8_t)(party_dir & 3);
            weather_context.player_direction = (uint8_t)(party_dir & 3);
            weather_context.map_x = (int16_t)party_x;
            weather_context.map_y = (int16_t)party_y;
            weather_context.map_offset_x = (int16_t)map_offset_x;
            weather_context.map_offset_y = (int16_t)map_offset_y;
            weather_context.map_level = (int16_t)rt->dungeon_level;
            weather_context.scene_flags = rt->gdat_scene_flags;
            weather_context.game_tick = (uint16_t)rt->tick_count;
            if (dm2_v1_boot_weather_renderer_receipt(
                    rt->boot, rt->map_graphics_style, &weather_state,
                    rt->weather_distant_slots, rt->weather_distant_slot_count,
                    &weather_context, &weather_renderer) && weather_renderer.valid) {
                DM2_V1_WeatherGdatReceipt weather_gdat;

                /* M11 receives outdoor weather only after the same source timer
                 * owner that selected the live DistantEnvironment slot and every
                 * raw dtText/dtImage identity have passed the final receipt. */
                {
                    int gdat_rc = dm2_v1_boot_weather_gdat_receipt(
                        rt->boot, rt->map_graphics_style, &weather_gdat);
                    int m11_rc = gdat_rc ? dm2_v1_weather_gdat_outdoor_m11_receipt(
                        &weather_gdat, &weather_renderer,
                        &rt->set_timer_weather, &weather_m11) : 0;
                    if (gdat_rc && m11_rc) {
                        dm2_v1_viewport_set_gdat_weather_renderer_receipt(
                            &viewport, (uint8_t)rt->map_graphics_style,
                            &weather_renderer);
                        rt->gdat_weather_renderer_ready = 1;
                        rt->gdat_weather_renderer_hash = weather_renderer.renderer_hash;
                        rt->gdat_weather_renderer_command_count =
                            weather_renderer.command_count;
                    }
                }
            }
        }
    }
    dm2_v1_viewport_set_g1_creature_map_chip_materials(
        &viewport, &rt->g1_creature_map_chip_runtime);
    dm2_v1_viewport_set_g1_creature_v5_materials(
        &viewport, &rt->g1_creature_v5_runtime);
    dm2_v1_viewport_set_g1_weapon_map_chip_materials(
        &viewport, &rt->g1_weapon_map_chip_runtime);
    dm2_v1_viewport_set_g1_container_map_chip_materials(
        &viewport, &rt->g1_container_map_chip_runtime);
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
    /* A raw PC runtime/save bridge may publish this only after it has
     * authenticated the `c_light.cpp` inputs. Passing the zero receipt here
     * explicitly clears any stale viewport result across a frame handoff. */
    dm2_v1_viewport_set_c_light_receipt(&viewport, &rt->c_light_receipt);
    scene_material_plan_for_m11 = &rt->gdat_scene_material_plan;
    if (rt->c_light_receipt.valid) {
        uint8_t c_light_parameter = 0u;

        scene_material_plan = rt->gdat_scene_material_plan;
        int darkness_ok = dm2_v1_c_light_m11_palette_darkness(
            &rt->gdat_scene_light_receipt, &rt->c_light_receipt,
            &c_light_parameter);
        int apply_ok = darkness_ok && dm2_v1_boot_gdat_scene_m11_apply_light_palette(
            rt->boot, rt->scene_movement_pending, c_light_parameter,
            rt->c_light_receipt.receipt_hash, &scene_material_plan);
        if (!apply_ok) {
            /* An observed c_light state selects _32cb_0804.  Do not present
             * the base local palettes when its exact dt07 branch is present
             * but undecoded. */
            scene_material_plan_for_m11 = NULL;
        } else {
            scene_material_plan_for_m11 = &scene_material_plan;
        }
    }
    dm2_v1_viewport_set_gdat_scene_material_plan(
        &viewport, scene_material_plan_for_m11);
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
        int light_palette_required = 0;
        uint8_t c_light_parameter = 0u;

        for (uint8_t i = 0u;
             i < rt->gdat_door_material_plan.command_count; ++i) {
            const DM2_V1_GdatDoorOverlayM11Command *command =
                &rt->gdat_door_material_plan.commands[i];
            if (command->kind == DM2_V1_GDAT_DOOR_PANEL &&
                command->light_palette != 0u) {
                light_palette_required = 1;
                break;
            }
        }
        if (!light_palette_required ||
            (dm2_v1_c_light_m11_palette_darkness(
                 &rt->gdat_scene_light_receipt, &rt->c_light_receipt,
                 &c_light_parameter) &&
             dm2_v1_boot_gdat_door_overlay_apply_light_palette(
                 rt->boot, c_light_parameter,
                 rt->c_light_receipt.receipt_hash,
                 &rt->gdat_door_material_plan))) {
            dm2_v1_viewport_set_gdat_door_overlay_material_plan(
                &viewport, &rt->gdat_door_material_plan);
        } else {
            /* The D3 field-zero retry has no base-palette route. Its
             * QUERY_TEMP_PICST light transform needs live c_light evidence. */
            dm2_v1_gdat_door_overlay_m11_command_plan_free(
                &rt->gdat_door_material_plan);
        }
    }
    dm2_runtime_bind_g1_scene_wall_button_material(
        rt, &viewport, &door_render_plan);
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
            DM2_V1_G1_SCENE_ROOT_CREATURE &&
        rt->g1_scene_runtime_handoff.material_pixel_hash != 0u) {
        const DM2_V1_G1DungeonSceneClassificationReceipt *scene =
            &rt->g1_scene_runtime_handoff.scene;
        dm2_v1_viewport_set_g1_scene_creature_material_direct(
            &viewport, 1, scene->x, scene->y,
            rt->g1_scene_runtime_handoff.creature_type,
            rt->g1_scene_runtime_handoff.gdat_index,
            rt->g1_scene_runtime_handoff.material_pixels,
            rt->g1_scene_runtime_handoff.material_width,
            rt->g1_scene_runtime_handoff.material_height,
            rt->g1_scene_runtime_handoff.material_stride,
            rt->g1_scene_runtime_handoff.material_palette16,
            rt->g1_scene_runtime_handoff.material_palette_hash,
            rt->g1_scene_runtime_handoff.material_pixel_hash);
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
        if (!dm2_v1_boot_gdat_hud_m11_command_plan(
                rt->boot, &viewport.hud_party, &hud_material_plan)) {
            /* A missing per-champion GDAT record must not replace the
             * complete source HUD chrome. Keep the static plan and let the
             * individual portrait remain no-draw in the viewport. */
            (void)dm2_v1_boot_gdat_hud_static_m11_command_plan(
                rt->boot, rt->outdoor, &hud_material_plan);
        }
    } else if (rt->boot && rt->boot->graphics_dat) {
        /* The static chrome omits the right-side portrait panel in outdoor
         * mode because DM2 outdoor viewports do not draw it. */
        (void)dm2_v1_boot_gdat_hud_static_m11_command_plan(
            rt->boot, rt->outdoor, &hud_material_plan);
    }
    dm2_v1_viewport_set_gdat_hud_material_plan(
        &viewport, &hud_material_plan);
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
    hud_material_plan_command_count = hud_material_plan_consumed
        ? hud_material_plan.command_count : 0;
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
        dm2_runtime_creature_drawn_material_identity(
            &viewport, &creature_drawn_material_hash,
            &creature_drawn_material_count) &&
        creature_material_plan_count == viewport.asset_creature_drawn_count &&
        creature_drawn_material_count == viewport.asset_creature_drawn_count;
    if (!creature_material_plan_consumed) {
        creature_material_plan_hash = 0u;
        creature_drawn_material_hash = 0u;
        creature_drawn_material_count = 0;
    }
    projectile_material_plan_required =
        viewport.asset_projectile_drawn_count > 0;
    projectile_material_plan_consumed =
        projectile_material_plan_required &&
        viewport.fallback_projectile_drawn_count == 0 &&
        dm2_runtime_projectile_drawn_material_identity(
            &viewport, &projectile_drawn_material_hash,
            &projectile_drawn_material_count) &&
        projectile_drawn_material_count ==
            viewport.asset_projectile_drawn_count;
    if (!projectile_material_plan_consumed) {
        projectile_drawn_material_hash = 0u;
        projectile_drawn_material_count = 0;
    }
    item_material_plan_required =
        viewport.asset_item_drawn_count > 0 ||
        viewport.asset_creature_possession_item_drawn_count > 0 ||
        viewport.asset_carried_item_drawn_count > 0;
    item_material_plan_consumed =
        item_material_plan_required &&
        viewport.fallback_item_drawn_count == 0 &&
        viewport.fallback_creature_possession_item_drawn_count == 0 &&
        viewport.fallback_carried_item_drawn_count == 0 &&
        dm2_runtime_item_drawn_material_identity(
            &viewport, &item_drawn_material_hash,
            &item_drawn_material_count);
    if (!item_material_plan_consumed) {
        item_drawn_material_hash = 0u;
        item_drawn_material_count = 0;
    }
    if (viewport.asset_wall_drawn_count > 0 &&
        viewport.source_materials_required &&
        dm2_runtime_wall_drawn_material_identity(
            &viewport, &wall_drawn_material_hash,
            &wall_drawn_material_count) &&
        rt->gdat_wall_material_plan.valid &&
        rt->gdat_wall_material_plan.command_count > 0 &&
        viewport.gdat_wall_material_plan_consumed_count ==
            viewport.asset_wall_drawn_count &&
        wall_drawn_material_count == viewport.asset_wall_drawn_count &&
        wall_drawn_material_count > 0) {
        wall_material_plan_command_count =
            wall_drawn_material_count;
        wall_material_plan_consumed = 1;
    } else {
        wall_drawn_material_hash = 0u;
        wall_drawn_material_count = 0;
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
    g_dm2_frame_ownership.gdat_provider_bound =
        rt->viewport_asset_fetch != NULL;
    g_dm2_frame_ownership.floor_ceiling_gdat_blits =
        viewport.asset_floor_ceiling_drawn_count;
    g_dm2_frame_ownership.outdoor_sky_gdat_blits =
        viewport.asset_outdoor_sky_drawn_count;
    g_dm2_frame_ownership.outdoor_ground_gdat_blits =
        viewport.asset_outdoor_ground_drawn_count;
    g_dm2_frame_ownership.wall_gdat_blits =
        viewport.asset_wall_drawn_count;
    g_dm2_frame_ownership.wall_gdat_material_evidence_count =
        wall_drawn_material_count;
    g_dm2_frame_ownership.wall_gdat_material_evidence_hash =
        wall_drawn_material_hash;
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
    g_dm2_frame_ownership.creature_gdat_material_evidence_count =
        creature_drawn_material_count;
    g_dm2_frame_ownership.creature_gdat_material_evidence_hash =
        creature_drawn_material_hash;
    g_dm2_frame_ownership.item_gdat_blits =
        viewport.asset_item_drawn_count +
        viewport.asset_creature_possession_item_drawn_count +
        viewport.asset_carried_item_drawn_count;
    g_dm2_frame_ownership.item_gdat_material_evidence_count =
        item_drawn_material_count;
    g_dm2_frame_ownership.item_gdat_material_evidence_hash =
        item_drawn_material_hash;
    g_dm2_frame_ownership.projectile_gdat_blits =
        viewport.asset_projectile_drawn_count;
    g_dm2_frame_ownership.projectile_gdat_material_evidence_count =
        projectile_drawn_material_count;
    g_dm2_frame_ownership.projectile_gdat_material_evidence_hash =
        projectile_drawn_material_hash;
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
        viewport.fallback_hud_portrait_drawn_count +
        viewport.fallback_door_drawn_count +
        viewport.fallback_creature_drawn_count +
        viewport.fallback_item_drawn_count +
        viewport.fallback_creature_possession_item_drawn_count +
        viewport.fallback_carried_item_drawn_count +
        viewport.fallback_projectile_drawn_count;
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
    if (wall_material_plan_consumed) {
        DM2_V1_WallPanelRenderPlan wall_plan;
        if (dm2_v1_viewport_build_wall_panel_render_plan(&viewport,
                                                         &wall_plan)) {
            for (int i = 0; i < wall_plan.panel_count; ++i) {
                dm2_runtime_add_viewport_asset_evidence(
                    &g_dm2_frame_ownership,
                    wall_plan.panels[i].gdat_index);
            }
        }
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
    if (creature_material_plan_consumed) {
        for (int i = 0; i < viewport.creature_material_drawn_count; ++i) {
            dm2_runtime_add_viewport_asset_evidence(
                &g_dm2_frame_ownership,
                viewport.creature_material_gdat_indices[i]);
        }
    }
    if (item_material_plan_consumed) {
        for (int i = 0; i < viewport.item_material_drawn_count; ++i) {
            dm2_runtime_add_viewport_asset_evidence(
                &g_dm2_frame_ownership,
                viewport.item_material_gdat_indices[i]);
        }
    }
    if (projectile_material_plan_consumed) {
        for (int i = 0; i < viewport.projectile_material_drawn_count; ++i) {
            dm2_runtime_add_viewport_asset_evidence(
                &g_dm2_frame_ownership,
                viewport.projectile_material_gdat_indices[i]);
        }
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
    g_dm2_frame_ownership.gdat_c_light_receipt_ready =
        viewport.gdat_c_light_receipt_ready;
    g_dm2_frame_ownership.gdat_c_light_level =
        viewport.gdat_c_light_level;
    g_dm2_frame_ownership.gdat_c_light_receipt_hash =
        viewport.gdat_c_light_receipt_hash;
    g_dm2_frame_ownership.gdat_c_light_source_state_hash =
        viewport.gdat_c_light_source_state_hash;
    g_dm2_frame_ownership.gdat_c_light_consumed =
        viewport.gdat_c_light_consumed_count;
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
    g_dm2_frame_ownership.gdat_weather_renderer_ready =
        rt->gdat_weather_renderer_ready;
    g_dm2_frame_ownership.gdat_weather_renderer_hash =
        rt->gdat_weather_renderer_hash;
    g_dm2_frame_ownership.gdat_weather_renderer_command_count =
        rt->gdat_weather_renderer_command_count;
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
    g_dm2_frame_ownership.gdat_interface_palette_ready =
        rt->gdat_interface_palette_ready;
    g_dm2_frame_ownership.gdat_interface_palette_consumed =
        viewport.gdat_interface_palette_consumed_count;
    g_dm2_frame_ownership.gdat_interface_action_palette_hash =
        rt->gdat_interface_action_palette_hash;
    g_dm2_frame_ownership.gdat_interface_action_palette_consumed =
        viewport.gdat_interface_action_palette_consumed_count;
    /* skproject/SKWIN/SkWinCore.cpp LOAD_GDAT_INTERFACE_00_0A must reach the
     * runtime frame ownership receipt before M11 presents the viewport. */
    g_dm2_frame_ownership.gdat_interface_rect14_ready =
        rect14_host.valid &&
        rect14_host.table_hash != 0u &&
        rect14_host.row_count > 0u &&
        rect14_host.placement_hash != 0u;
    g_dm2_frame_ownership.gdat_interface_rect14_table_hash =
        g_dm2_frame_ownership.gdat_interface_rect14_ready
        ? rect14_host.table_hash : 0u;
    g_dm2_frame_ownership.gdat_interface_rect14_placement_hash =
        g_dm2_frame_ownership.gdat_interface_rect14_ready
        ? rect14_host.placement_hash : 0u;
    g_dm2_frame_ownership.gdat_interface_rect14_row_count =
        g_dm2_frame_ownership.gdat_interface_rect14_ready
        ? rect14_host.row_count : 0u;
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
    g_dm2_frame_ownership.floor_ceiling_material_required_mask =
        (uint8_t)viewport.last_floor_ceiling_material_required_mask;
    g_dm2_frame_ownership.floor_ceiling_material_consumed_mask =
        (uint8_t)viewport.last_floor_ceiling_material_consumed_mask;
    g_dm2_frame_ownership.floor_ceiling_materials_complete =
        viewport.last_floor_ceiling_material_required_mask ==
        viewport.last_floor_ceiling_material_consumed_mask;
    g_dm2_frame_ownership.blocked_material_draws =
        viewport.blocked_material_draw_count;
    /* skproject SKWIN/SkWinCore.cpp routes the runtime HUD, floor/ceiling,
     * walls and overlays through GDAT-backed surface fetches before blitting.
     * A "full" DM2 runtime frame is only accepted when the mandatory HUD and
     * dungeon base layers are GDAT-backed and no visible runtime element fell
     * back to Firestaff's bounded placeholder paths. */
    /* A callback can provide fixture pixels to the isolated viewport tests,
     * but only dm2_v1_boot_viewport_asset_fetch carries the raw+decoded GDAT
     * evidence from the mounted original GRAPHICS.DAT.  Do not let a clean
     * fixture blit receipt become a playable DM2 frame merely because it has
     * no fallback draws. */
    g_dm2_frame_ownership.outdoor_gdat_frame_valid =
        g_dm2_frame_ownership.is_outdoor &&
        g_dm2_frame_ownership.gdat_provider_bound &&
        g_dm2_frame_ownership.real_gdat_evidence_valid &&
        g_dm2_frame_ownership.outdoor_sky_gdat_blits > 0 &&
        g_dm2_frame_ownership.outdoor_ground_gdat_blits > 0 &&
        g_dm2_frame_ownership.hud_gdat_blits > 0 &&
        g_dm2_frame_ownership.total_runtime_fallback_draws == 0;
    g_dm2_frame_ownership.full_gdat_frame_valid =
        g_dm2_frame_ownership.gdat_provider_bound &&
        g_dm2_frame_ownership.real_gdat_evidence_valid &&
        g_dm2_frame_ownership.floor_ceiling_gdat_blits >= 2 &&
        (g_dm2_frame_ownership.is_outdoor ||
         g_dm2_frame_ownership.wall_gdat_blits > 0) &&
        g_dm2_frame_ownership.gdat_scene_control_ready &&
        g_dm2_frame_ownership.gdat_scene_control_consumed > 0 &&
        g_dm2_frame_ownership.gdat_scene_control_hash != 0u &&
        g_dm2_frame_ownership.gdat_interface_palette_ready &&
        g_dm2_frame_ownership.gdat_interface_palette_consumed > 0 &&
        g_dm2_frame_ownership.gdat_material_palette_floor_ceiling_consumed > 0 &&
        (g_dm2_frame_ownership.is_outdoor ||
         g_dm2_frame_ownership.gdat_material_palette_wall_consumed > 0) &&
        (viewport.asset_door_frame_drawn_count == 0 ||
         g_dm2_frame_ownership.gdat_material_palette_door_frame_consumed > 0) &&
        g_dm2_frame_ownership.gdat_interface_palette_hash != 0u &&
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
        (g_dm2_frame_ownership.creature_gdat_blits == 0 ||
         (g_dm2_frame_ownership.creature_gdat_material_evidence_count ==
              g_dm2_frame_ownership.creature_gdat_blits &&
          g_dm2_frame_ownership.creature_gdat_material_evidence_hash != 0u)) &&
        (!g_dm2_frame_ownership.is_outdoor ||
         g_dm2_frame_ownership.outdoor_gdat_frame_valid);
    g_dm2_frame_ownership.valid =
        g_dm2_frame_ownership.runtime_frame_owned &&
        g_dm2_frame_ownership.full_gdat_frame_valid;
    memset(&g_dm2_last_m11_frame, 0, sizeof(g_dm2_last_m11_frame));
    g_dm2_last_m11_frame.source_materials_required =
        viewport.source_materials_required ? 1 : 0;
    g_dm2_last_m11_frame.map_load_token =
        dm2_v1_runtime_g1_scene_map_token(rt->dungeon_level,
                                           rt->map_graphics_style,
                                           rt->outdoor);
    g_dm2_last_m11_frame.scene_control_hash =
        g_dm2_frame_ownership.gdat_scene_control_hash;
    g_dm2_last_m11_frame.scene_light_hash =
        rt->gdat_scene_light_receipt.valid
            ? rt->gdat_scene_light_receipt.receipt_hash : 0u;
    g_dm2_last_m11_frame.scene_ambient_light =
        rt->gdat_scene_light_receipt.valid
            ? rt->gdat_scene_light_receipt.ambient_light : 0u;
    g_dm2_last_m11_frame.c_light_receipt_hash =
        viewport.gdat_c_light_receipt_hash;
    g_dm2_last_m11_frame.c_light_source_state_hash =
        viewport.gdat_c_light_source_state_hash;
    g_dm2_last_m11_frame.c_light_level =
        viewport.gdat_c_light_level;
    /* UPDATE_GFXSET owns these exact GRAPHICSSET IMG3 records; retain their
     * individual identities so M11 cannot combine a current control receipt
     * with floor, ceiling, or WALL_GFX pixels from another plan. */
    g_dm2_last_m11_frame.floor_material_hash =
        rt->gdat_scene_material_plan.valid
            ? rt->gdat_scene_material_plan.commands[0].raw_hash : 0u;
    g_dm2_last_m11_frame.ceiling_material_hash =
        rt->gdat_scene_material_plan.valid
            ? rt->gdat_scene_material_plan.commands[1].raw_hash : 0u;
    /* Outdoor frames present sky/ground planes, not indoor wall materials.
     * Clear the wall plan identity so M11 does not compare a non-zero hash
     * against zero commands. */
    g_dm2_last_m11_frame.wall_material_plan_hash =
        rt->gdat_wall_material_plan.valid && !rt->outdoor
            ? rt->gdat_wall_material_plan.command_hash : 0u;
    g_dm2_last_m11_frame.wall_material_plan_command_count =
        rt->outdoor ? 0 : wall_material_plan_command_count;
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
        rt->gdat_door_material_plan.command_count > 0 &&
        viewport.gdat_door_overlay_material_plan_consumed_count ==
            rt->gdat_door_material_plan.command_count;
    g_dm2_last_m11_frame.door_material_plan_hash =
        g_dm2_last_m11_frame.door_material_plan_consumed
            ? rt->gdat_door_material_plan.command_hash : 0u;
    g_dm2_last_m11_frame.door_material_plan_command_count =
        g_dm2_last_m11_frame.door_material_plan_consumed
            ? rt->gdat_door_material_plan.command_count : 0;
    g_dm2_last_m11_frame.hud_material_plan_required =
        hud_material_plan_required;
    g_dm2_last_m11_frame.hud_material_plan_hash = hud_material_plan_hash;
    g_dm2_last_m11_frame.hud_scene_control_hash = hud_material_plan_consumed
        ? g_dm2_last_m11_frame.scene_control_hash : 0u;
    g_dm2_last_m11_frame.hud_material_plan_command_count =
        hud_material_plan_command_count;
    g_dm2_last_m11_frame.hud_material_plan_consumed =
        hud_material_plan_consumed;
    g_dm2_last_m11_frame.creature_material_plan_required =
        creature_material_plan_required;
    g_dm2_last_m11_frame.creature_material_plan_hash =
        creature_material_plan_hash;
    g_dm2_last_m11_frame.creature_material_plan_command_count =
        creature_material_plan_consumed ? creature_material_plan_count : 0;
    g_dm2_last_m11_frame.creature_material_plan_consumed =
        creature_material_plan_consumed;
    g_dm2_last_m11_frame.projectile_material_plan_required =
        projectile_material_plan_required;
    g_dm2_last_m11_frame.projectile_material_plan_hash =
        projectile_drawn_material_hash;
    g_dm2_last_m11_frame.projectile_material_plan_command_count =
        projectile_material_plan_consumed ? projectile_drawn_material_count : 0;
    g_dm2_last_m11_frame.projectile_material_plan_consumed =
        projectile_material_plan_consumed;
    g_dm2_last_m11_frame.item_material_plan_required =
        item_material_plan_required;
    g_dm2_last_m11_frame.item_material_plan_hash = item_drawn_material_hash;
    g_dm2_last_m11_frame.item_scene_control_hash =
        item_material_plan_consumed
            ? g_dm2_last_m11_frame.scene_control_hash : 0u;
    g_dm2_last_m11_frame.item_material_plan_command_count =
        item_material_plan_consumed ? item_drawn_material_count : 0;
    g_dm2_last_m11_frame.item_material_plan_consumed =
        item_material_plan_consumed;
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
    /* c_weather.cpp emits the DistantEnvironment transaction before its
     * QUERY_TEMP_PICST image calls. Carry the exact renderer receipt only
     * when this frame consumed every selected source layer. */
    g_dm2_last_m11_frame.weather_material_plan_required =
        rt->gdat_weather_renderer_ready &&
        rt->gdat_weather_renderer_command_count > 0u;
    g_dm2_last_m11_frame.weather_material_plan_consumed =
        g_dm2_last_m11_frame.weather_material_plan_required &&
        viewport.gdat_weather_renderer_consumed_hash ==
            rt->gdat_weather_renderer_hash &&
        viewport.gdat_weather_renderer_consumed_command_count ==
            rt->gdat_weather_renderer_command_count &&
        viewport.asset_weather_drawn_count ==
            (int)rt->gdat_weather_renderer_command_count;
    g_dm2_last_m11_frame.weather_material_plan_hash =
        g_dm2_last_m11_frame.weather_material_plan_consumed
            ? rt->gdat_weather_renderer_hash : 0u;
    g_dm2_last_m11_frame.weather_material_plan_command_count =
        g_dm2_last_m11_frame.weather_material_plan_consumed
            ? (int)rt->gdat_weather_renderer_command_count : 0;
    g_dm2_last_m11_frame.weather_graphicsset_bound =
        g_dm2_last_m11_frame.weather_material_plan_consumed &&
        rt->gdat_weather_receipt_ready && rt->gdat_weather_destination_ready;
    g_dm2_last_m11_frame.weather_graphicsset =
        g_dm2_last_m11_frame.weather_graphicsset_bound
            ? (uint8_t)rt->map_graphics_style : 0u;
    g_dm2_last_m11_frame.weather_source_receipt_hash =
        g_dm2_last_m11_frame.weather_graphicsset_bound
            ? rt->gdat_weather_receipt_hash : 0u;
    g_dm2_last_m11_frame.weather_destination_receipt_hash =
        g_dm2_last_m11_frame.weather_graphicsset_bound
            ? rt->gdat_weather_destination_hash : 0u;
    g_dm2_last_m11_frame.presentation_state_hash =
        dm2_v1_runtime_frame_presentation_state_hash(
            g_dm2_last_m11_frame.scene_light_hash,
            g_dm2_last_m11_frame.scene_ambient_light,
            g_dm2_last_m11_frame.c_light_receipt_hash,
            g_dm2_last_m11_frame.c_light_source_state_hash,
            g_dm2_last_m11_frame.c_light_level,
            g_dm2_frame_ownership.gdat_c_light_consumed > 0,
            g_dm2_last_m11_frame.weather_graphicsset_bound,
            g_dm2_last_m11_frame.weather_graphicsset,
            g_dm2_last_m11_frame.weather_source_receipt_hash,
            g_dm2_last_m11_frame.weather_destination_receipt_hash,
            g_dm2_last_m11_frame.weather_material_plan_hash);
    g_dm2_last_m11_frame.palette_hash =
        g_dm2_frame_ownership.gdat_interface_palette_hash;
    g_dm2_last_m11_frame.interface_action_palette_hash =
        g_dm2_frame_ownership.gdat_interface_action_palette_hash;
    g_dm2_last_m11_frame.interface_action_palette_consumed =
        g_dm2_frame_ownership.gdat_interface_action_palette_consumed > 0;
    g_dm2_last_m11_frame.interface_rect14_required =
        g_dm2_frame_ownership.gdat_interface_rect14_ready;
    g_dm2_last_m11_frame.interface_rect14_consumed =
        g_dm2_frame_ownership.gdat_interface_rect14_ready;
    g_dm2_last_m11_frame.interface_rect14_table_hash =
        g_dm2_frame_ownership.gdat_interface_rect14_table_hash;
    g_dm2_last_m11_frame.interface_rect14_placement_hash =
        g_dm2_frame_ownership.gdat_interface_rect14_placement_hash;
    g_dm2_last_m11_frame.interface_rect14_row_count =
        g_dm2_frame_ownership.gdat_interface_rect14_row_count;
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
        g_dm2_last_m11_frame.presentation_state_hash != 0u &&
        g_dm2_last_m11_frame.floor_material_hash != 0u &&
        g_dm2_last_m11_frame.ceiling_material_hash != 0u &&
        (rt->outdoor || g_dm2_last_m11_frame.wall_material_plan_hash != 0u) &&
        (!g_dm2_last_m11_frame.door_material_plan_required ||
         (g_dm2_last_m11_frame.door_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.door_material_plan_command_count > 0 &&
          g_dm2_last_m11_frame.door_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.hud_material_plan_required ||
         (g_dm2_last_m11_frame.hud_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.hud_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.creature_material_plan_required ||
         (g_dm2_last_m11_frame.creature_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.creature_material_plan_command_count > 0 &&
          g_dm2_last_m11_frame.creature_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.projectile_material_plan_required ||
         (g_dm2_last_m11_frame.projectile_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.projectile_material_plan_command_count > 0 &&
          g_dm2_last_m11_frame.projectile_material_plan_consumed)) &&
        (!g_dm2_last_m11_frame.item_material_plan_required ||
         (g_dm2_last_m11_frame.item_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.item_material_plan_command_count > 0 &&
          g_dm2_last_m11_frame.item_material_plan_consumed)) &&
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
        (!g_dm2_last_m11_frame.weather_material_plan_required ||
         (g_dm2_last_m11_frame.weather_material_plan_hash != 0u &&
          g_dm2_last_m11_frame.weather_material_plan_command_count > 0 &&
          g_dm2_last_m11_frame.weather_material_plan_consumed &&
          g_dm2_last_m11_frame.weather_graphicsset_bound)) &&
        g_dm2_last_m11_frame.palette_hash != 0u &&
        (!g_dm2_frame_ownership.real_gdat_evidence_valid ||
         (g_dm2_last_m11_frame.interface_action_palette_hash != 0u &&
          g_dm2_last_m11_frame.interface_action_palette_consumed));
    g_dm2_last_m11_frame.m11_consume_frame =
        g_dm2_last_m11_frame.valid;
    /* The M10/M11 frame route may consume a raw save only after the active
     * dungeon still has the exact GAME_LOAD prefix and party pose that were
     * atomically published. A later map replacement leaves this receipt
     * unconsumed instead of associating a frame with unrelated bytes. */
    if (rt->raw_sksave_handoff.valid && rt->boot &&
        rt->boot->dungeon_data) {
        const DM2_V1_DungeonData *dungeon =
            (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
        DM2_V1_RawSKSaveMapSceneReceipt live_map_scene;
        memset(&live_map_scene, 0, sizeof(live_map_scene));
        if (dungeon->raw_data && dungeon->raw_size > 0 &&
            (size_t)dungeon->raw_size ==
                rt->raw_sksave_handoff.dungeon_byte_count &&
            dm2_v1_runtime_raw_sksave_hash(
                dungeon->raw_data, (size_t)dungeon->raw_size) ==
                rt->raw_sksave_handoff.prefix_hash &&
            rt->dungeon_level == (int)rt->raw_sksave_handoff.party_level &&
            party_x == (int)rt->raw_sksave_handoff.party_x &&
            party_y == (int)rt->raw_sksave_handoff.party_y &&
            (party_dir & 3) == (int)rt->raw_sksave_handoff.party_dir &&
            rt->raw_sksave_handoff.map_scene_valid &&
            dm2_v1_dungeon_collect_raw_sksave_map_scene(
                dungeon, rt->dungeon_level, &live_map_scene) &&
            live_map_scene.valid &&
            live_map_scene.map_data_hash ==
                rt->raw_sksave_handoff.map_scene_map_data_hash &&
            live_map_scene.terrain_hash ==
                rt->raw_sksave_handoff.map_scene_terrain_hash &&
            live_map_scene.object_record_hash ==
                rt->raw_sksave_handoff.map_scene_object_record_hash &&
            live_map_scene.thing_bearing_tile_count ==
                rt->raw_sksave_handoff.map_scene_thing_bearing_tile_count &&
            live_map_scene.addressable_root_count ==
                rt->raw_sksave_handoff.map_scene_addressable_root_count &&
            memcmp(live_map_scene.root_count_by_type,
                   rt->raw_sksave_handoff.map_scene_root_count_by_type,
                   sizeof(live_map_scene.root_count_by_type)) == 0) {
            rt->raw_sksave_handoff.first_frame_consumed = 1;
        }
    }
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

int dm2_v1_runtime_last_raw_sksave_handoff_receipt(
    DM2_V1_RuntimeRawSaveHandoffReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.raw_sksave_handoff;
    return out_receipt->valid;
}

uint32_t dm2_v1_runtime_g1_scene_map_token(int level, int graphicsset,
                                            int outdoor)
{
    if (level < 0 || level >= DM2_V1_MAX_LEVELS ||
        graphicsset < 0 || graphicsset > 0x0f) return 0u;
    return (uint32_t)(level + 1) | ((uint32_t)(graphicsset + 1) << 8) |
        (outdoor ? UINT32_C(0x80000000) : 0u);
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
    g_dm2_runtime.viewport_asset_palette_fetch =
        fetch == dm2_v1_boot_viewport_asset_fetch
            ? dm2_v1_boot_viewport_asset_palette_fetch
            : NULL;
    g_dm2_runtime.viewport_asset_palette_user =
        g_dm2_runtime.viewport_asset_palette_fetch ? user : NULL;
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

int dm2_v1_runtime_last_flying_item_receipt(
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_flying_item.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_flying_item;
    return 1;
}

int dm2_v1_runtime_flying_item_viewport_evidence(
    const DM2_V1_RuntimeFlyingItemReceipt *material,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    uint32_t session_identity, uint32_t map_load_token,
    DM2_V1_RuntimeFlyingItemViewportEvidence *out_evidence)
{
    uint32_t hash = 2166136261u;
    if (!out_evidence) return 0;
    memset(out_evidence, 0, sizeof(*out_evidence));
    if (!material || !selector || !geometry || !material->valid ||
        !material->no_draw || !material->timer_receipt_hash ||
        !material->raw_gfx256_hash || !material->raw_gfx256_receipt_hash ||
        !material->palette_hash || !material->raw4_hash ||
        !material->raw4_receipt_hash || !material->identity_hash ||
        !selector->valid || !geometry->valid || !geometry->no_draw ||
        geometry->image_field_available || !selector->identity_hash ||
        !geometry->identity_hash || !session_identity || !map_load_token ||
        selector->missile_object_id != material->source.missile_object_id ||
        (selector->branch_temp_picst && !geometry->temp_picst_eligible) ||
        (!selector->branch_temp_picst && !geometry->draw_item_opaque)) return 0;
    hash ^= material->identity_hash; hash *= 16777619u;
    hash ^= selector->identity_hash; hash *= 16777619u;
    hash ^= geometry->identity_hash; hash *= 16777619u;
    hash ^= session_identity; hash *= 16777619u;
    hash ^= map_load_token; hash *= 16777619u;
    out_evidence->valid = 1; out_evidence->no_draw = 1;
    out_evidence->session_identity = session_identity;
    out_evidence->map_load_token = map_load_token;
    out_evidence->timer_receipt_hash = material->timer_receipt_hash;
    out_evidence->gdat_identity_hash = material->identity_hash;
    out_evidence->selector_identity_hash = selector->identity_hash;
    out_evidence->geometry_identity_hash = geometry->identity_hash;
    out_evidence->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_flying_item_decoded_material_plan(
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint32_t session_identity, uint32_t map_load_token,
    DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *out_plan)
{
    uint32_t hash = 2166136261u;
    if (!out_plan) return 0;
    memset(out_plan, 0, sizeof(*out_plan));
    if (!timer_receipt || !selector || !vb30 || !geometry || !material ||
        !timer_receipt->valid || !timer_receipt->no_draw ||
        !timer_receipt->timer_receipt_hash || !timer_receipt->source.valid ||
        !selector->valid || !vb30->valid || vb30->temp_picst_blocked ||
        !geometry->valid || !geometry->no_draw ||
        !geometry->temp_picst_eligible || geometry->draw_item_opaque ||
        !material->valid || !material->no_draw || !material->identity_hash ||
        !material->raw_gfx256_hash || !material->raw_gfx256_receipt_hash ||
        !material->decoded_pixels_hash || !material->palette_hash ||
        !material->offset_receipt_hash ||
        !session_identity || !map_load_token ||
        selector->missile_object_id != timer_receipt->source.missile_object_id ||
        material->selector_identity_hash != selector->identity_hash ||
        material->vb30_identity_hash != vb30->identity_hash ||
        material->geometry_identity_hash != geometry->identity_hash) return 0;
    hash ^= timer_receipt->timer_receipt_hash; hash *= 16777619u;
    hash ^= selector->identity_hash; hash *= 16777619u;
    hash ^= vb30->identity_hash; hash *= 16777619u;
    hash ^= geometry->identity_hash; hash *= 16777619u;
    hash ^= material->identity_hash; hash *= 16777619u;
    hash ^= session_identity; hash *= 16777619u;
    hash ^= map_load_token; hash *= 16777619u;
    out_plan->valid = 1; out_plan->no_draw = 1;
    out_plan->session_identity = session_identity;
    out_plan->map_load_token = map_load_token;
    out_plan->timer_receipt_hash = timer_receipt->timer_receipt_hash;
    out_plan->selector_identity_hash = selector->identity_hash;
    out_plan->vb30_identity_hash = vb30->identity_hash;
    out_plan->geometry_identity_hash = geometry->identity_hash;
    out_plan->material_identity_hash = material->identity_hash;
    out_plan->raw_gfx256_hash = material->raw_gfx256_hash;
    out_plan->raw_gfx256_receipt_hash = material->raw_gfx256_receipt_hash;
    out_plan->decoded_pixels_hash = material->decoded_pixels_hash;
    out_plan->palette_hash = material->palette_hash;
    out_plan->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_flying_item_decoded_material_plan_matches(
    const DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *plan,
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint32_t session_identity, uint32_t map_load_token)
{
    DM2_V1_RuntimeFlyingItemDecodedMaterialPlan expected;
    if (!plan || !plan->valid || !plan->no_draw ||
        !dm2_v1_runtime_flying_item_decoded_material_plan(timer_receipt,
            selector, vb30, geometry, material, session_identity,
            map_load_token, &expected)) return 0;
    return plan->identity_hash == expected.identity_hash &&
        plan->session_identity == expected.session_identity &&
        plan->map_load_token == expected.map_load_token &&
        plan->timer_receipt_hash == expected.timer_receipt_hash &&
        plan->selector_identity_hash == expected.selector_identity_hash &&
        plan->vb30_identity_hash == expected.vb30_identity_hash &&
        plan->geometry_identity_hash == expected.geometry_identity_hash &&
        plan->material_identity_hash == expected.material_identity_hash &&
        plan->raw_gfx256_hash == expected.raw_gfx256_hash &&
        plan->raw_gfx256_receipt_hash == expected.raw_gfx256_receipt_hash &&
        plan->decoded_pixels_hash == expected.decoded_pixels_hash &&
        plan->palette_hash == expected.palette_hash;
}

static uint32_t dm2_v1_runtime_hash_indexed_bytes(const uint8_t *bytes,
                                                   size_t count)
{
    uint32_t hash = 2166136261u;
    if (!bytes || !count) return 0u;
    for (size_t i = 0u; i < count; ++i) {
        hash ^= bytes[i]; hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

int dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_FlyingItemM11DeliveryPlan *delivery,
    const DM2_V1_RuntimeFlyingItemDecodedMaterialPlan *plan,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *out_receipt)
{
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    uint8_t *pixels;
    int width = 0, height = 0;
    DM2_ImageFormat format;
    size_t pixel_count;
    uint32_t pixel_hash;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!loader || !delivery || !plan || !material || !source ||
        !delivery->valid || !delivery->no_draw ||
        delivery->pixel_decoder_ready || !delivery->m11_delivery_ready ||
        !delivery->identity_hash || !plan->valid || !plan->no_draw ||
        !plan->identity_hash || !material->valid || !material->no_draw ||
        !material->identity_hash || !source->valid || !source->identity_hash ||
        source->missile_object_id != delivery->missile_object_id ||
        source->category != material->category ||
        source->image_field != material->field || source->clip_rect_id == 0u ||
        source->clip_rect_id != delivery->clip_rect_id || source->flip_flags > 3u ||
        delivery->raw_gfx256_hash != material->raw_gfx256_hash ||
        delivery->raw_gfx256_receipt_hash != material->raw_gfx256_receipt_hash ||
        delivery->palette_hash != material->palette_hash ||
        delivery->timer_receipt_hash != plan->timer_receipt_hash ||
        delivery->viewport_session_identity != plan->session_identity ||
        delivery->viewport_map_load_token != plan->map_load_token ||
        delivery->image_field != material->field ||
        plan->material_identity_hash != material->identity_hash ||
        !dm2_v1_query_gdat_summary_image_receipt(loader, material->category,
            material->index, material->field, &summary) || !summary.accepted ||
        summary.gdat_bypassed_for_ff ||
        summary.receipt_hash != material->summary_receipt_hash ||
        summary.palette_hash != material->palette_hash ||
        summary.offset_receipt_hash != material->offset_receipt_hash ||
        summary.metadata.query_offset_x != material->source_offset_x ||
        summary.metadata.query_offset_y != material->source_offset_y) return 0;
    pixels = dm2_v1_asset_load_image_field(loader, material->category,
        material->index, material->field, &width, &height, &format);
    if (!pixels || width != material->width || height != material->height ||
        width <= 0 || height <= 0 || format != material->format ||
        (size_t)width > SIZE_MAX / (size_t)height) {
        free(pixels); return 0;
    }
    pixel_count = (size_t)width * (size_t)height;
    pixel_hash = dm2_v1_runtime_hash_indexed_bytes(pixels, pixel_count);
    free(pixels);
    if (!pixel_hash || pixel_hash != material->decoded_pixels_hash) return 0;
    hash ^= delivery->identity_hash; hash *= 16777619u;
    hash ^= plan->identity_hash; hash *= 16777619u;
    hash ^= material->identity_hash; hash *= 16777619u;
    hash ^= source->identity_hash; hash *= 16777619u;
    hash ^= pixel_hash; hash *= 16777619u;
    hash ^= material->palette_hash; hash *= 16777619u;
    hash ^= source->clip_rect_id; hash *= 16777619u;
    hash ^= source->flip_flags; hash *= 16777619u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->source_order = 8u;
    out_receipt->follows_static_and_creature = 1u;
    out_receipt->indexed_bytes_consumed = 1u;
    out_receipt->orientation_unapplied = 1u;
    out_receipt->clip_rect_id = source->clip_rect_id;
    out_receipt->flip_flags = source->flip_flags;
    out_receipt->width = (uint16_t)width; out_receipt->height = (uint16_t)height;
    out_receipt->format = format;
    out_receipt->source_offset_x = material->source_offset_x;
    out_receipt->source_offset_y = material->source_offset_y;
    out_receipt->offset_receipt_hash = material->offset_receipt_hash;
    out_receipt->delivery_identity_hash = delivery->identity_hash;
    out_receipt->decoded_material_identity_hash = material->identity_hash;
    out_receipt->indexed_pixels_hash = pixel_hash;
    out_receipt->palette_hash = material->palette_hash;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_flying_item_destination_receipt(
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *consumer,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_BootExpandedRectReceipt *clip,
    DM2_V1_Dm2FlyingItemDestinationReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_gui_vp.cpp:3745-3785 passes vl24 to QUERY_TEMP_PICST, whose
     * QUERY_PICST_IT clip is the RAW4-expanded rect. The x/y offsets are
     * SUMMARY_IMAGE facts; flip has no proven raster transform here. */
    if (!consumer || !source || !clip || !consumer->valid ||
        !consumer->no_draw || !consumer->identity_hash ||
        !consumer->offset_receipt_hash || !source->valid ||
        !source->identity_hash || !source->clip_rect_id ||
        source->clip_rect_id != consumer->clip_rect_id ||
        source->flip_flags != consumer->flip_flags || !clip->valid ||
        clip->rect_id != source->clip_rect_id || !clip->raw4_hash ||
        !clip->receipt_hash || clip->rect.w <= 0 || clip->rect.h <= 0) return 0;
    hash ^= consumer->identity_hash; hash *= 16777619u;
    hash ^= source->identity_hash; hash *= 16777619u;
    hash ^= clip->raw4_hash; hash *= 16777619u;
    hash ^= clip->receipt_hash; hash *= 16777619u;
    hash ^= consumer->offset_receipt_hash; hash *= 16777619u;
    hash ^= source->flip_flags; hash *= 16777619u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->clip_rect_id = source->clip_rect_id;
    out_receipt->clip_rect = clip->rect;
    out_receipt->source_offset_x = consumer->source_offset_x;
    out_receipt->source_offset_y = consumer->source_offset_y;
    out_receipt->flip_flags = source->flip_flags;
    out_receipt->orientation_unapplied = 1u;
    out_receipt->raw4_hash = clip->raw4_hash;
    out_receipt->raw4_receipt_hash = clip->receipt_hash;
    out_receipt->offset_receipt_hash = consumer->offset_receipt_hash;
    out_receipt->consumer_identity_hash = consumer->identity_hash;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_flying_item_picst_transform_receipt(
    const DM2_V1_Dm2FlyingItemDestinationReceipt *destination,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_Dm2FlyingItemPicstTransformReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* c_gui_vp.cpp:3745-3785 passes vw18 to both PICST scales. For the
     * bounded normal-scale branch, c_image.cpp:98-115 leaves offsets intact.
     * DB14 has not yet proven the nonzero blitmode source-coordinate branch. */
    if (!destination || !material || !source || !destination->valid ||
        !destination->no_draw || !destination->identity_hash ||
        !material->valid || !material->no_draw || !material->identity_hash ||
        !source->valid || source->stretch_factor64 != 0x40u ||
        source->flip_flags != 0u || destination->flip_flags != 0u ||
        destination->clip_rect_id != source->clip_rect_id ||
        destination->source_offset_x != material->source_offset_x ||
        destination->source_offset_y != material->source_offset_y ||
        destination->offset_receipt_hash != material->offset_receipt_hash ||
        destination->clip_rect.w <= 0 || destination->clip_rect.h <= 0) return 0;
    hash ^= destination->identity_hash; hash *= 16777619u;
    hash ^= material->identity_hash; hash *= 16777619u;
    hash ^= source->identity_hash; hash *= 16777619u;
    hash ^= 0x40u; hash *= 16777619u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->scale_x = 0x40u; out_receipt->scale_y = 0x40u;
    out_receipt->blitmode = 0u;
    out_receipt->clip_rect = destination->clip_rect;
    out_receipt->source_offset_x = destination->source_offset_x;
    out_receipt->source_offset_y = destination->source_offset_y;
    out_receipt->destination_identity_hash = destination->identity_hash;
    out_receipt->material_identity_hash = material->identity_hash;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_blit_flying_item_normal_scale_indexed(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *consumer,
    const DM2_V1_Dm2FlyingItemDestinationReceipt *destination,
    const DM2_V1_Dm2FlyingItemPicstTransformReceipt *transform,
    const DM2_V1_G1FlyingItemDecodedMaterialReceipt *material,
    uint8_t *framebuffer, int framebuffer_width, int framebuffer_height,
    int framebuffer_stride)
{
    uint8_t *pixels;
    int width = 0, height = 0;
    DM2_ImageFormat format;
    size_t count;
    if (!loader || !consumer || !destination || !transform || !material ||
        !framebuffer || framebuffer_width <= 0 || framebuffer_height <= 0 ||
        framebuffer_stride < framebuffer_width || !consumer->valid ||
        !consumer->indexed_bytes_consumed || !destination->valid ||
        !transform->valid || !transform->no_draw || transform->scale_x != 0x40u ||
        transform->scale_y != 0x40u || transform->blitmode != 0u ||
        transform->source_offset_x != 0 || transform->source_offset_y != 0 ||
        !material->valid || material->source_offset_x != 0 ||
        material->source_offset_y != 0 || destination->clip_rect.x < 0 ||
        destination->clip_rect.y < 0 || destination->clip_rect.w != material->width ||
        destination->clip_rect.h != material->height ||
        destination->clip_rect.x > framebuffer_width - destination->clip_rect.w ||
        destination->clip_rect.y > framebuffer_height - destination->clip_rect.h ||
        consumer->palette_hash != material->palette_hash ||
        consumer->indexed_pixels_hash != material->decoded_pixels_hash ||
        transform->material_identity_hash != material->identity_hash ||
        transform->destination_identity_hash != destination->identity_hash) return 0;
    pixels = dm2_v1_asset_load_image_field(loader, material->category,
        material->index, material->field, &width, &height, &format);
    if (!pixels || width != material->width || height != material->height ||
        format != material->format || (size_t)width > SIZE_MAX / (size_t)height) {
        free(pixels); return 0;
    }
    count = (size_t)width * (size_t)height;
    if (dm2_v1_runtime_hash_indexed_bytes(pixels, count) !=
        material->decoded_pixels_hash) { free(pixels); return 0; }
    for (int y = 0; y < height; ++y)
        memcpy(framebuffer + (size_t)(destination->clip_rect.y + y) *
               (size_t)framebuffer_stride + destination->clip_rect.x,
               pixels + (size_t)y * (size_t)width, (size_t)width);
    free(pixels);
    return 1;
}

int dm2_v1_runtime_build_dm2_viewport_m11_material_composition(
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *flying_item,
    DM2_V1_Dm2ViewportM11MaterialCompositionReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!composition || !flying_item || !composition->valid ||
        !composition->no_draw || composition->pixel_decoder_ready ||
        !composition->m11_delivery_ready || !composition->identity_hash ||
        !composition->flying_item_identity_hash || !flying_item->valid ||
        !flying_item->no_draw || !flying_item->identity_hash ||
        !flying_item->indexed_bytes_consumed ||
        !flying_item->follows_static_and_creature ||
        flying_item->source_order != 8u ||
        flying_item->delivery_identity_hash != composition->flying_item_identity_hash)
        return 0;
    hash ^= composition->identity_hash; hash *= 16777619u;
    hash ^= flying_item->identity_hash; hash *= 16777619u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->composition = *composition;
    out_receipt->flying_item_material = *flying_item;
    out_receipt->identity_hash = hash ? hash : 1u;
    return 1;
}

int dm2_v1_runtime_dm2_viewport_m11_material_composition_matches(
    const DM2_V1_Dm2ViewportM11MaterialCompositionReceipt *receipt,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt *flying_item)
{
    DM2_V1_Dm2ViewportM11MaterialCompositionReceipt expected;
    if (!receipt || !receipt->valid || !receipt->no_draw ||
        !dm2_v1_runtime_build_dm2_viewport_m11_material_composition(
            composition, flying_item, &expected)) return 0;
    return receipt->identity_hash == expected.identity_hash &&
        receipt->composition.identity_hash == expected.composition.identity_hash &&
        receipt->flying_item_material.identity_hash ==
            expected.flying_item_material.identity_hash &&
        receipt->flying_item_material.delivery_identity_hash ==
            expected.flying_item_material.delivery_identity_hash &&
        receipt->flying_item_material.decoded_material_identity_hash ==
            expected.flying_item_material.decoded_material_identity_hash &&
        receipt->flying_item_material.indexed_pixels_hash ==
            expected.flying_item_material.indexed_pixels_hash &&
        receipt->flying_item_material.palette_hash ==
            expected.flying_item_material.palette_hash &&
        receipt->flying_item_material.clip_rect_id ==
            expected.flying_item_material.clip_rect_id &&
        receipt->flying_item_material.flip_flags ==
            expected.flying_item_material.flip_flags &&
        receipt->flying_item_material.source_order ==
            expected.flying_item_material.source_order;
}

int dm2_v1_runtime_last_static_object_m11_delivery_plans(
    DM2_V1_StaticObjectM11DeliveryPlan *out_plans, int max_plans,
    int *out_count)
{
    const DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (out_count) *out_count = 0;
    if (!out_plans || !out_count || max_plans < 0 ||
        rt->g1_static_object_delivery_plan_count < 0 ||
        rt->g1_static_object_delivery_plan_count > 48 ||
        max_plans < rt->g1_static_object_delivery_plan_count) return 0;
    if (rt->g1_static_object_delivery_plan_count > 0) {
        memcpy(out_plans, rt->g1_static_object_delivery_plans,
               (size_t)rt->g1_static_object_delivery_plan_count *
               sizeof(*out_plans));
    }
    *out_count = rt->g1_static_object_delivery_plan_count;
    return 1;
}

int dm2_v1_runtime_flying_item_timer_receipt(
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_G1MissileTimerReceipt *timer,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt)
{
    uint32_t hash = 2166136261u;
    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!missile || !source || !timer || !missile->valid || !source->valid ||
        !timer->valid || !timer->raw_timer_hash ||
        missile->timer_index != timer->timer_index || timer->direction > 3u ||
        missile->object_id != source->missile_object_id) return 0;
    hash ^= missile->record_hash; hash *= 16777619u;
    hash ^= source->identity_hash; hash *= 16777619u;
    hash ^= timer->raw_timer_hash; hash *= 16777619u;
    hash ^= timer->timer_index; hash *= 16777619u;
    hash ^= timer->direction; hash *= 16777619u;
    out_receipt->valid = 1; out_receipt->no_draw = 1;
    out_receipt->source = *source;
    out_receipt->timer_receipt_hash = hash ? hash : 1u;
    out_receipt->identity_hash = out_receipt->timer_receipt_hash;
    return 1;
}

int dm2_v1_runtime_flying_item_timer_from_session(
    const DM2_V1_SessionState *session,
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt)
{
    DM2_V1_G1MissileTimerReceipt timer;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* The original save reader retains SKWIN's SUPPRESS timer table as
     * contiguous ten-byte rows. The runtime heap is not an owner here. */
    if (!session || !missile || session->original_timer_count > DM2_MAX_TIMERS ||
        !dm2_v1_g1_direct_missile_timer_receipt(
            (const uint8_t *)session->original_timers,
            (size_t)session->original_timer_count * DM2_TIMER_ENTRY_SIZE,
            missile->timer_index, &timer)) return 0;
    return dm2_v1_runtime_flying_item_timer_receipt(
        missile, source, &timer, out_receipt);
}

int dm2_v1_runtime_admit_flying_item_material(
    const DM2_V1_SessionState *session,
    const DM2_V1_G1DirectMissileReceipt *missile,
    const DM2_V1_G1FlyingItemSourceReceipt *source,
    const DM2_V1_G1FlyingItemMaterialReceipt *material)
{
    DM2_V1_RuntimeFlyingItemReceipt timer_receipt;
    DM2_V1_RuntimeFlyingItemReceipt complete;

    memset(&g_dm2_last_flying_item, 0, sizeof(g_dm2_last_flying_item));
    if (!dm2_v1_runtime_flying_item_timer_from_session(
            session, missile, source, &timer_receipt) ||
        !dm2_v1_runtime_flying_item_material_receipt(
            &timer_receipt, material, &complete)) return 0;
    /* Runtime ownership is deliberately not renderer admission. */
    complete.no_draw = 1;
    g_dm2_last_flying_item = complete;
    return 1;
}

int dm2_v1_runtime_flying_item_material_receipt(
    const DM2_V1_RuntimeFlyingItemReceipt *timer_receipt,
    const DM2_V1_G1FlyingItemMaterialReceipt *material,
    DM2_V1_RuntimeFlyingItemReceipt *out_receipt)
{
    DM2_V1_RuntimeFlyingItemReceipt timer;
    uint32_t hash;
    if (!out_receipt) return 0;
    if (!timer_receipt || !material || !timer_receipt->valid ||
        !timer_receipt->no_draw || !material->valid ||
        timer_receipt->source.identity_hash != material->source.identity_hash ||
        timer_receipt->source.missile_object_id !=
            material->source.missile_object_id ||
        !material->raw_gfx256_bytes || !material->raw_gfx256_byte_count ||
        !material->raw_gfx256_hash || !material->raw_gfx256_receipt_hash ||
        !material->local_palette_hash || !material->raw4_hash ||
        !material->raw4_receipt_hash || !material->identity_hash) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    timer = *timer_receipt;
    *out_receipt = timer;
    out_receipt->raw_gfx256_hash = material->raw_gfx256_hash;
    out_receipt->raw_gfx256_receipt_hash = material->raw_gfx256_receipt_hash;
    out_receipt->palette_hash = material->local_palette_hash;
    out_receipt->raw4_hash = material->raw4_hash;
    out_receipt->raw4_receipt_hash = material->raw4_receipt_hash;
    hash = timer.timer_receipt_hash;
    hash ^= material->identity_hash; hash *= 16777619u;
    out_receipt->identity_hash = hash ? hash : 1u;
    out_receipt->no_draw = 1;
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

int dm2_v1_runtime_last_perform_move_receipt(
    DM2_V1_PerformMoveReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_perform_move.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_perform_move;
    return 1;
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
    DM2_V1_PerformMoveRequest move_request;
    DM2_V1_PerformMoveReceipt move_receipt;
    int dx[] = {0, 1, 0, -1};  /* N E S W */
    int dy[] = {-1, 0, 1, 0};
    int nx, ny;
    int blocked = 0;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    gs = (DM2_V1_GameState *)rt->boot->dm2_state;

    /* Save pre-move position for smooth animation trigger */
    int old_x = gs->party_x;
    int old_y = gs->party_y;
    int old_dir = gs->party_dir;

    if (!dm2_v1_runtime_can_move()) {
        memset(&move_request, 0, sizeof(move_request));
        move_request.runtime_ready = 1;
        move_request.can_move = 0;
        move_request.outdoor = rt->outdoor;
        move_request.current_level = rt->dungeon_level;
        move_request.from_x = old_x;
        move_request.from_y = old_y;
        move_request.from_dir = old_dir;
        move_request.direction = dir;
        (void)dm2_v1_DM2_PERFORM_MOVE_plan(&move_request,
                                           &g_dm2_last_perform_move);
        return -1;
    }

    /* Detect turn-only (facing change, no movement) */
    int is_turn_only = (dir != old_dir);
    (void)is_turn_only;  /* turn-only detection reserved for future smooth-move path */

    nx = gs->party_x + dx[dir & 3];
    ny = gs->party_y + dy[dir & 3];
    memset(&move_request, 0, sizeof(move_request));
    move_request.runtime_ready = 1;
    move_request.can_move = 1;
    move_request.outdoor = rt->outdoor;
    move_request.current_level = rt->dungeon_level;
    move_request.from_x = gs->party_x;
    move_request.from_y = gs->party_y;
    move_request.from_dir = old_dir;
    move_request.direction = dir;

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
            move_request.target_raw_valid = 0;
        } else {
            int raw_tile_type = dm2_runtime_square_type_at(
                dd, rt->dungeon_level, nx, ny, raw);
            int tile_type = dm2_runtime_normalize_square_type(raw_tile_type);
            move_request.target_raw_valid = 1;
            move_request.target_raw = raw;
            move_request.target_square_type = tile_type;
            /* Impassable tile types: wall, pit, lava, inaccessible.
             * Normalized to DM2_SquareType enum before comparison. */
            if (tile_type == DM2_SQUARE_WALL ||
                tile_type == DM2_SQUARE_PIT ||
                tile_type == DM2_SQUARE_LAVA ||
                tile_type == DM2_SQUARE_INACCESSIBLE) {
                blocked = 1;
            } else if (dm2_runtime_is_door_at(
                           dd, rt->dungeon_level, nx, ny, raw)) {
                /* Door tile: door state in lower 3 bits.
                 * DM2_DOOR_STATE_OPEN=0 (passable), DM2_DOOR_STATE_CLOSED=4 (impassable).
                 * Source: dm2_v1_object_model.h DM2_DoorState enum.
                 *         SKULL.ASM T520 movement tile access. */
                int door_state = raw & 0x0007;
                move_request.target_is_door = 1;
                move_request.target_door_state = door_state;
                if (door_state != 0) {  /* not open */
                    blocked = 1;
                }
            }
            /* All other tile types (1=floor, 3=floor_ornate,
             * 8=teleporter, 10=water, etc.) are passable. */
        }
    }
    if (!rt->outdoor && !rt->boot->dungeon_data) {
        /* Preserve the existing headless/no-data runtime path. Real dungeon
         * launches still provide target_raw through dm2_v1_dungeon_load. */
        move_request.target_raw_valid = 1;
        move_request.target_square_type = 1;
    }
    if (rt->outdoor) {
        move_request.target_raw_valid = 1;
    }
    if (!dm2_v1_DM2_PERFORM_MOVE_plan(&move_request, &move_receipt)) {
        memset(&g_dm2_last_perform_move, 0, sizeof(g_dm2_last_perform_move));
        return -1;
    }
    g_dm2_last_perform_move = move_receipt;
    blocked = move_receipt.blocked;

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
    }

    /* Fire smooth turn callback when facing changes.
     * Turn triggers even on blocked moves (party still turns).
     * Source: Phase 5 runtime binding */
    if (dir != old_dir && rt->turn_callback) {
        rt->turn_callback(old_dir, dir);
    }

    gs->party_dir = dir;
    rt->view_dir = dir;

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
    dm2_runtime_refresh_map_transition_context(rt);
}

int dm2_v1_runtime_last_music_map_receipt(
    DM2_V1_RuntimeMusicMapReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.boot) return 0;
    *out_receipt = g_dm2_runtime.music_map_receipt;
    return out_receipt->valid;
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

int dm2_v1_runtime_last_weather_timer_receipt(
    DM2_V1_WeatherTimerReceipt *out_receipt)
{
    if (!out_receipt || !g_dm2_runtime.last_weather_timer_receipt.valid) {
        return 0;
    }
    *out_receipt = g_dm2_runtime.last_weather_timer_receipt;
    return 1;
}

int dm2_v1_runtime_import_sksave_receipted_candidate(
    const DM2_SKSaveCandidateReceipt *candidate_receipt,
    DM2_V1_RuntimeCorpusImportReceipt *out)
{
    DM2_V1_SaveCandidate candidate;
    uint8_t *payload = NULL;
    size_t payload_size = 0u;
    int restored = 0;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE;
    if (!candidate_receipt || !candidate_receipt->path[0] ||
        candidate_receipt->payload_size == 0u ||
        candidate_receipt->payload_hash == 0u ||
        candidate_receipt->source_file_hash == 0u ||
        (candidate_receipt->kind !=
             DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
         candidate_receipt->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW)) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    /* The corpus receipt identifies one exact file and payload. Reload it
     * through the scanner-owned verifier before parsing or changing runtime
     * state, so GAME_LOAD cannot silently select another save or accept a
     * changed file. */
    payload = (uint8_t *)malloc(candidate_receipt->payload_size);
    if (!payload ||
        !dm2_v1_sksave_corpus_load_receipted_candidate(
            candidate_receipt, payload, candidate_receipt->payload_size,
            &payload_size) ||
        payload_size != candidate_receipt->payload_size ||
        dm2_v1_session_parse_save_candidate(&candidate, payload,
                                             payload_size) != 0 ||
        (int)candidate.kind != candidate_receipt->kind) {
        free(payload);
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    restored = dm2_v1_runtime_restore_save_candidate(payload, payload_size) == 0;
    free(payload);
    if (!restored) {
        out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    out->result = DM2_V1_RUNTIME_CORPUS_IMPORT_OK;
    out->restored = 1;
    out->candidate_kind = candidate_receipt->kind;
    out->selected_payload_size = candidate_receipt->payload_size;
    out->selected_payload_hash = candidate_receipt->payload_hash;
    out->selected_source_file_hash = candidate_receipt->source_file_hash;
    out->rejected_original_candidate = 0;
    snprintf(out->selected_path, sizeof(out->selected_path), "%s",
             candidate_receipt->path);
    return 1;
}

static int dm2_runtime_original_corpus_entry_matches(
    const DM2_OriginalSaveStateCorpusEntry *expected,
    const DM2_OriginalSaveStateCorpusEntry *actual)
{
    const DM2_SKSaveCandidateReceipt *left;
    const DM2_SKSaveCandidateReceipt *right;

    if (!expected || !actual) return 0;
    left = &expected->candidate;
    right = &actual->candidate;
    return left->kind == right->kind &&
           left->payload_size == right->payload_size &&
           left->payload_hash == right->payload_hash &&
           left->source_file_hash == right->source_file_hash &&
           strcmp(left->path, right->path) == 0 &&
           expected->state_hash == actual->state_hash;
}

int dm2_v1_runtime_import_original_sksave_state_entry(
    const char *save_root,
    const DM2_OriginalSaveStateCorpusEntry *selected_entry,
    DM2_V1_RuntimeOriginalCorpusImportReceipt *out)
{
    DM2_OriginalSaveStateCorpusReceipt state_corpus;
    const DM2_OriginalSaveStateCorpusEntry *admitted = NULL;
    uint8_t i;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_UNAVAILABLE;
    if (!save_root || !selected_entry ||
        (selected_entry->candidate.kind !=
             DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
         selected_entry->candidate.kind !=
             DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW)) {
        out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    /* GAME_LOAD consumes the selected save stream, not a directory default.
     * Rebuild the diagnostic census first so a stale row can never be used
     * to select a different valid SKSave (including a Firestaff session). */
    if (!dm2_v1_original_save_state_corpus_probe(save_root, &state_corpus)) {
        return 0;
    }
    out->original_candidate_count = state_corpus.original_candidate_count;
    out->parsed_candidate_count = state_corpus.parsed_candidate_count;
    out->corpus_hash = state_corpus.corpus_hash;
    out->corpus_complete = state_corpus.scan_complete &&
        state_corpus.original_candidate_list_complete &&
        state_corpus.original_candidate_count != 0u &&
        state_corpus.parsed_candidate_count ==
            state_corpus.original_candidate_count &&
        state_corpus.rejected_candidate_count == 0u &&
        state_corpus.entry_count == state_corpus.original_candidate_count;
    if (!out->corpus_complete) {
        out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }
    for (i = 0u; i < state_corpus.entry_count; ++i) {
        if (dm2_runtime_original_corpus_entry_matches(
                selected_entry, &state_corpus.entries[i])) {
            admitted = &state_corpus.entries[i];
            break;
        }
    }
    if (!admitted) {
        out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }

    out->selected_state_hash = admitted->state_hash;
    if (admitted->candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        if (!admitted->raw_dungeon_layout_valid ||
            admitted->raw_dungeon_map_count == 0u ||
            admitted->raw_dungeon_prefix_hash == 0u ||
            admitted->raw_map_data_hash == 0u) {
            out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
            return 0;
        }
        out->selected_raw_dungeon_layout_valid = 1;
        out->selected_raw_dungeon_map_count = admitted->raw_dungeon_map_count;
        out->selected_raw_dungeon_prefix_hash =
            admitted->raw_dungeon_prefix_hash;
        out->selected_raw_map_data_hash = admitted->raw_map_data_hash;
        memcpy(out->selected_raw_db_record_counts,
               admitted->raw_db_record_counts,
               sizeof(out->selected_raw_db_record_counts));
    }
    if (!dm2_v1_runtime_import_sksave_receipted_candidate(
            &admitted->candidate, &out->runtime_import)) {
        return 0;
    }
    if (out->runtime_import.candidate_kind != admitted->candidate.kind ||
        out->runtime_import.selected_payload_size !=
            admitted->candidate.payload_size ||
        out->runtime_import.selected_payload_hash !=
            admitted->candidate.payload_hash ||
        out->runtime_import.selected_source_file_hash !=
            admitted->candidate.source_file_hash ||
        strcmp(out->runtime_import.selected_path,
               admitted->candidate.path) != 0) {
        out->runtime_import.result = DM2_V1_RUNTIME_CORPUS_IMPORT_REJECTED;
        return 0;
    }
    out->selected_state_admitted = 1;
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

void dm2_v1_runtime_clear_new_game_party_state(void) {
    /* SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON clears
     * party.heros_in_party and ddat.savegamewpc.w_00 before
     * DM2_READ_DUNGEON_STRUCTURE(1). A stale decoded SKSave must therefore
     * not keep portraits, inventory, or a leader hand alive on the new-game
     * title boundary. */
    g_dm2_runtime.leader_hand_object = 0u;
    memset(g_dm2_runtime.champion_inventory_objects, 0,
           sizeof(g_dm2_runtime.champion_inventory_objects));
    memset(&g_dm2_runtime.session_snapshot, 0,
           sizeof(g_dm2_runtime.session_snapshot));
    g_dm2_runtime.session_snapshot_valid = 0;
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

static int dm2_v1_runtime_raw_sksave_dungeon_matches_receipt(
    const DM2_V1_DungeonData *dungeon,
    const DM2_V1_SaveCandidate *candidate)
{
    const DM2_V1_OriginalRawDungeonReceipt *receipt;
    size_t expected_column_base;
    size_t expected_sft_base;
    size_t expected_text_base;
    int type;

    if (!dungeon || !candidate || !candidate->dungeon_bytes ||
        candidate->dungeon_size == 0u) {
        return 0;
    }
    receipt = &candidate->dungeon_receipt;
    if (!receipt->valid || receipt->suppress_state_offset !=
                               candidate->dungeon_size ||
        dungeon->raw_size < 0 || (size_t)dungeon->raw_size !=
                                       candidate->dungeon_size ||
        !dungeon->raw_data ||
        memcmp(dungeon->raw_data, candidate->dungeon_bytes,
               candidate->dungeon_size) != 0) {
        return 0;
    }

    expected_column_base = 44u + (size_t)receipt->map_count * 16u;
    expected_sft_base = expected_column_base +
                        (size_t)receipt->column_index_count * 2u;
    expected_text_base = expected_sft_base +
                         (size_t)receipt->ground_stack_count * 2u;
    if (receipt->map_count == 0u ||
        dungeon->level_count != (int)receipt->map_count ||
        dungeon->column_index_base != (int)expected_column_base ||
        dungeon->square_first_thing_base != (int)expected_sft_base ||
        dungeon->text_data_base != (int)expected_text_base ||
        dungeon->square_first_thing_count !=
            (int)receipt->ground_stack_count ||
        dungeon->text_word_count != (int)receipt->text_word_count ||
        dungeon->raw_map_data_base != (int)receipt->map_data_offset ||
        (size_t)dungeon->raw_size - (size_t)dungeon->raw_map_data_base !=
            (size_t)receipt->map_data_byte_count) {
        return 0;
    }

    /* SKProject's READ_DUNGEON_STRUCTURE assigns recordptr[type] in source
     * DB order directly after text.  Check every admitted pool base/count;
     * this is an address gate only and never follows GenericRecord::w0. */
    for (type = 0; type < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++type) {
        size_t next_pool_offset = type + 1 < DM2_RAW_SKSAVE_DB_POOL_COUNT
            ? receipt->db_pool_offsets[type + 1]
            : receipt->map_data_offset;
        if (dungeon->thing_type_counts[type] !=
                (int)receipt->db_record_counts[type] ||
            dungeon->thing_data_bases[type] !=
                (int)receipt->db_pool_offsets[type]) {
            return 0;
        }
        if (receipt->db_record_counts[type] != 0u) {
            DM2_V1_OriginalRawDbRecordReceipt first_record;
            DM2_V1_OriginalRawDbRecordReceipt last_record;
            const int last_index =
                (int)receipt->db_record_counts[type] - 1;

            if (!dm2_v1_original_raw_sksave_db_record_receipt(
                    candidate->dungeon_bytes, candidate->dungeon_size,
                    type, 0, &first_record) ||
                !dm2_v1_original_raw_sksave_db_record_receipt(
                    candidate->dungeon_bytes, candidate->dungeon_size,
                    type, last_index, &last_record) ||
                first_record.record_offset != receipt->db_pool_offsets[type] ||
                last_record.record_offset + last_record.record_size !=
                    next_pool_offset) {
                return 0;
            }
        } else if (receipt->db_pool_offsets[type] != next_pool_offset) {
            return 0;
        }
    }
    return 1;
}

static int dm2_v1_runtime_raw_sksave_reachable_records_are_valid(
    DM2_V1_DungeonData *dungeon)
{
    int level;
    int has_map_record_root = 0;

    if (!dungeon || !dungeon->raw_data || dungeon->square_bytes != 1 ||
        dungeon->level_count <= 0) {
        return 0;
    }

    /* c_map.cpp reads a first ObjectID only for a thing-bearing square, then
     * c_record.cpp follows GenericRecord::w0. Unused pool slots are
     * allocation state and must not make an otherwise valid save fail. */
    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        if (dungeon->level_widths[level] <= 0 ||
            dungeon->level_heights[level] <= 0) {
            return 0;
        }
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                const int tile = dm2_v1_dungeon_get_tile_raw(
                    dungeon, level, x, y);
                if (tile < 0) return 0;
                if ((tile & 0x10) != 0) has_map_record_root = 1;
            }
        }
    }
    if (!has_map_record_root) return 1;

    /* The raw SKSave has no untyped G1 extension. A marked map square is
     * therefore admitted only with a complete, bounded source record graph. */
    dungeon->record_graph_complete = 1;
    return dm2_v1_dungeon_validate_record_graph(dungeon);
}

static uint32_t dm2_v1_runtime_raw_sksave_hash(const uint8_t *data,
                                                size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!data || size == 0u) return 0u;
    for (i = 0u; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash ? hash : 1u;
}

static void dm2_v1_runtime_raw_sksave_handoff_from_candidate(
    const DM2_V1_SaveCandidate *candidate,
    const DM2_V1_RawSKSaveMapSceneReceipt *map_scene,
    DM2_V1_RuntimeRawSaveHandoffReceipt *out_receipt)
{
    const DM2_V1_OriginalRawDungeonReceipt *dungeon;
    int type;

    if (!out_receipt) return;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!candidate || candidate->kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW ||
        !candidate->dungeon_bytes || candidate->dungeon_size == 0u) {
        return;
    }
    dungeon = &candidate->dungeon_receipt;
    if (!dungeon->valid || dungeon->suppress_state_offset !=
                               candidate->dungeon_size ||
        !map_scene || !map_scene->valid ||
        map_scene->map != (int)candidate->session.party_level ||
        map_scene->map_data_hash == 0u || map_scene->terrain_hash == 0u ||
        map_scene->object_record_hash == 0u) {
        return;
    }
    out_receipt->map_count = dungeon->map_count;
    out_receipt->dungeon_byte_count = candidate->dungeon_size;
    out_receipt->prefix_hash = dungeon->prefix_hash;
    out_receipt->map_data_hash = dungeon->map_data_hash;
    out_receipt->party_level = candidate->session.party_level;
    out_receipt->party_x = candidate->session.party_x;
    out_receipt->party_y = candidate->session.party_y;
    out_receipt->party_dir = candidate->session.party_dir & 3u;
    out_receipt->map_scene_valid = 1;
    out_receipt->map_scene_thing_bearing_tile_count =
        map_scene->thing_bearing_tile_count;
    out_receipt->map_scene_addressable_root_count =
        map_scene->addressable_root_count;
    memcpy(out_receipt->map_scene_root_count_by_type,
           map_scene->root_count_by_type,
           sizeof(out_receipt->map_scene_root_count_by_type));
    out_receipt->map_scene_map_data_hash = map_scene->map_data_hash;
    out_receipt->map_scene_terrain_hash = map_scene->terrain_hash;
    out_receipt->map_scene_object_record_hash =
        map_scene->object_record_hash;
    for (type = 0; type < DM2_RAW_SKSAVE_DB_POOL_COUNT; ++type) {
        out_receipt->db_record_counts[type] = dungeon->db_record_counts[type];
    }
    out_receipt->valid = out_receipt->map_count != 0u &&
                         out_receipt->prefix_hash != 0u &&
                         out_receipt->map_data_hash != 0u;
}

int dm2_v1_runtime_restore_save_candidate(const uint8_t *data,
                                          size_t data_size)
{
    DM2_V1_SaveCandidate candidate;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_CreatureLiveState cleared_creatures;
    DM2_V1_DungeonData parsed_dungeon;
    DM2_V1_DungeonData saved_dungeon;
    DM2_V1_RuntimeRawSaveHandoffReceipt raw_handoff;
    DM2_V1_RawSKSaveMapSceneReceipt raw_map_scene;
    DM2_V1_RuntimeTimerPostLoadReceipt timer_preflight;
    int parsed_original_dungeon = 0;

    if (!data || data_size == 0u || !g_dm2_runtime.boot ||
        !g_dm2_runtime.boot->dm2_state || !g_dm2_runtime.boot->dungeon_data) {
        return -1;
    }
    /* SKProject GAME_LOAD continues after the bounded c_hex2a/SUPPRESS
     * sections with the linked c_record, possession, c_hero, actuator and
     * timer graph. Firestaff can currently receipt the raw prefix but cannot
     * publish that partial graph as a session. Keep every public resume route
     * unavailable rather than turning authentic-but-incomplete bytes into a
     * playable state. The parser below remains a diagnostic/source-study path
     * until the full read order is imported. */
    return -1;

    /* Unreachable pending the complete GAME_LOAD owner handoff. */
    if (dm2_v1_session_parse_save_candidate(&candidate, data, data_size) != 0) {
        return -1;
    }
    if (candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_ENVELOPE &&
        candidate.kind != DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        return -1;
    }
    dungeon = (DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data;
    if (!dungeon->raw_data || dungeon->raw_size <= 0 ||
        (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW &&
         candidate.dungeon_size != (size_t)dungeon->raw_size)) {
        return -1;
    }

    /* GAME_LOAD's timer-owner reconstruction is fallible even after the
     * SUPPRESS stream has decoded. Run it before swapping G1 or clearing the
     * Firestaff-only CCM cache; apply_session repeats this exact check at the
     * publication boundary. */
    if (!dm2_runtime_prepare_session_apply(&candidate.session,
                                           &timer_preflight)) {
        return -1;
    }
    memset(&cleared_creatures, 0, sizeof(cleared_creatures));
    if (dm2_v1_creature_live_state_valid(&cleared_creatures) != 0) {
        return -1;
    }

    memset(&parsed_dungeon, 0, sizeof(parsed_dungeon));
    memset(&raw_handoff, 0, sizeof(raw_handoff));
    memset(&raw_map_scene, 0, sizeof(raw_map_scene));
    if (candidate.kind == DM2_V1_SAVE_CANDIDATE_ORIGINAL_RAW) {
        /* skproject/SKWINSPX/src/v4/skcore.cpp::GAME_LOAD calls
         * READ_DUNGEON_STRUCTURE before it consumes skload_table_60.  The
         * raw prefix therefore owns both the bytes and the live G1 layout;
         * copying it under old DUNGEON.DAT metadata can select the wrong map
         * dimensions, GRAPHICSSET, or record-pool spans.  Parse the complete
         * saved prefix first, then publish it in one swap below. */
        if (dm2_v1_dungeon_load(&parsed_dungeon, candidate.dungeon_bytes,
                                (int)candidate.dungeon_size) != 0 ||
            !dm2_v1_runtime_raw_sksave_dungeon_matches_receipt(
                &parsed_dungeon, &candidate) ||
            !dm2_v1_runtime_raw_sksave_reachable_records_are_valid(
                &parsed_dungeon) ||
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
        if (!dm2_v1_dungeon_collect_raw_sksave_map_scene(
                &parsed_dungeon, candidate.session.party_level,
                &raw_map_scene)) {
            dm2_v1_dungeon_free(&parsed_dungeon);
            return -1;
        }
        saved_dungeon = *dungeon;
        *dungeon = parsed_dungeon;
        memset(&parsed_dungeon, 0, sizeof(parsed_dungeon));
        parsed_original_dungeon = 1;
        dm2_v1_runtime_raw_sksave_handoff_from_candidate(&candidate,
                                                          &raw_map_scene,
                                                          &raw_handoff);
        if (!raw_handoff.valid) {
            dm2_v1_dungeon_free(dungeon);
            *dungeon = saved_dungeon;
            return -1;
        }
    }

    /* Original SKSave has dungeon DB records but no Firestaff-only CCM cache.
     * Clear that cache before apply_session; a matching quicksave sidecar is
     * allowed to replace it with the exact saved CCM/animation/GDAT state. */
    if (dm2_v1_creature_restore_live_state(&cleared_creatures) != 0 ||
        dm2_runtime_apply_source_session(&candidate.session) != 0) {
        if (parsed_original_dungeon) {
            dm2_v1_dungeon_free(dungeon);
            *dungeon = saved_dungeon;
        }
        return -1;
    }
    if (parsed_original_dungeon) {
        dm2_v1_dungeon_free(&saved_dungeon);
        g_dm2_runtime.raw_sksave_handoff = raw_handoff;
    } else {
        memset(&g_dm2_runtime.raw_sksave_handoff, 0,
               sizeof(g_dm2_runtime.raw_sksave_handoff));
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
    /* SKProject SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_SAVE writes the
     * complete original save graph (globals, heroes, timers and dungeon) in
     * source order.  Firestaff has only an import-side session envelope, so
     * exporting it as SKSave.dat would create a plausible-looking but
     * non-original save.  Refuse before creating a directory, exporting a
     * session or writing the former SKSave.runtime sidecar. */
    dm2_v1_quicksave_receipt_init(
        receipt, DM2_V1_QUICKSAVE_ORIGINAL_WRITER_REQUIRED,
        "DM2 ORIGINAL SAVE WRITER REQUIRED");
    if (profile->save_root[0]) {
        snprintf(receipt->save_root, sizeof(receipt->save_root), "%s",
                 profile->save_root);
    }
    return 0;
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

int dm2_v1_runtime_last_set_timer_weather_receipt(
    DM2_V1_SetTimerWeatherReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.set_timer_weather;
    return out_receipt->valid;
}

int dm2_v1_runtime_last_weather_3df7_0037_receipt(
    DM2_V1_Weather3df70037Receipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.weather_3df7_0037;
    return out_receipt->valid;
}

int dm2_v1_runtime_bind_weather_distant_environment(
    const DM2_V1_DistantEnvironmentReceipt *slots, unsigned int slot_count)
{
    DM2_V1_WeatherGdatReceipt weather;
    DM2_V1_DistantEnvironmentReceipt admitted[DM2_V1_WEATHER_MAX_SLOTS];
    uint32_t map_token;

    if (slot_count > DM2_V1_WEATHER_MAX_SLOTS ||
        (slot_count != 0u && !slots)) return 0;
    if (slot_count == 0u) {
        memset(g_dm2_runtime.weather_distant_slots, 0,
               sizeof(g_dm2_runtime.weather_distant_slots));
        g_dm2_runtime.weather_distant_slot_count = 0u;
        g_dm2_runtime.weather_distant_slots_map_token = 0u;
        g_dm2_runtime.weather_distant_slots_source_receipt_hash = 0u;
        g_dm2_runtime.weather_distant_slots_graphicsset = 0u;
        return 1;
    }
    /* c_weather's DistantEnvironment slots are selected through the active
     * MapGraphicsStyle.  A raw ten-byte slot alone is not portable across
     * levels or GRAPHICSSETs, even if its command byte happens to exist in
     * both sets. Rebuild its current GDAT owner before accepting it. */
    if (!g_dm2_runtime.boot || !g_dm2_runtime.outdoor ||
        g_dm2_runtime.map_graphics_style < 0 ||
        g_dm2_runtime.map_graphics_style > 0xff ||
        !g_dm2_runtime.gdat_weather_receipt_ready ||
        g_dm2_runtime.gdat_weather_receipt_hash == 0u ||
        !dm2_v1_boot_weather_gdat_receipt(
            g_dm2_runtime.boot, g_dm2_runtime.map_graphics_style, &weather) ||
        !weather.valid || weather.receipt_hash == 0u ||
        weather.receipt_hash != g_dm2_runtime.gdat_weather_receipt_hash ||
        (map_token = dm2_v1_runtime_g1_scene_map_token(
             g_dm2_runtime.dungeon_level, g_dm2_runtime.map_graphics_style,
             g_dm2_runtime.outdoor)) == 0u) {
        return 0;
    }
    memset(admitted, 0, sizeof(admitted));
    for (unsigned int i = 0u; i < slot_count; ++i) {
        if (!slots[i].valid || slots[i].slot_index != i ||
            slots[i].raw_hash == 0u || slots[i].raw[0] != slots[i].command ||
            !dm2_v1_weather_distant_environment_receipt(
                &weather, slots[i].command, (uint8_t)i, slots[i].raw,
                &admitted[i]) ||
            admitted[i].raw_hash != slots[i].raw_hash) {
            return 0;
        }
    }
    memset(g_dm2_runtime.weather_distant_slots, 0,
           sizeof(g_dm2_runtime.weather_distant_slots));
    memcpy(g_dm2_runtime.weather_distant_slots, admitted,
           (size_t)slot_count * sizeof(admitted[0]));
    g_dm2_runtime.weather_distant_slot_count = slot_count;
    g_dm2_runtime.weather_distant_slots_map_token = map_token;
    g_dm2_runtime.weather_distant_slots_source_receipt_hash =
        weather.receipt_hash;
    g_dm2_runtime.weather_distant_slots_graphicsset =
        (uint8_t)g_dm2_runtime.map_graphics_style;
    return 1;
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

void dm2_v1_runtime_set_cdda_callback(
    void (*play)(void *ctx, const uint8_t *pcm, size_t size, int loop),
    void (*stop)(void *ctx),
    void *ctx)
{
    g_dm2_runtime.cdda_play_cb = play;
    g_dm2_runtime.cdda_stop_cb = stop;
    g_dm2_runtime.cdda_cb_ctx = ctx;
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

    (void)level;
    (void)x;
    (void)y;
    /* SKProject's shop glass is not selected by a fixed map coordinate or
     * catalog. _32cb_0f82_SHOP_GLASS receives the live wall actuator while
     * DRAW_WALL_ORNATE resolves its WALL_GFX GDAT image/overlay chain
     * (SKWINSPX/src/v4/skguidrw.cpp:3551-3640, 4042-4049). Until Firestaff
     * carries that complete record and GDAT ownership, opening a fixed shop
     * would invent stock, price, NPC and transaction state. */
    if (!rt->boot || !rt->boot->dm2_state) return -1;
    return -1;
}

int dm2_v1_runtime_leave_shop(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (!rt->boot || !rt->boot->dm2_state) return -1;
    /* SHOP_GLASS has no decoded DB14/CCM/WALL_GFX ownership yet.  Do not
     * let an otherwise unreachable local shop state write arbitrary ObjectID
     * or gold values back into the live session. */
    return -1;
}

int dm2_v1_runtime_buy_from_shop(int stock_idx) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    (void)stock_idx;
    if (!rt->boot || !rt->boot->dm2_state) return -1;
    return (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP;
}

int dm2_v1_runtime_sell_to_shop(int inv_idx) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    (void)inv_idx;
    if (!rt->boot || !rt->boot->dm2_state) return -1;
    return (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP;
}

int dm2_v1_runtime_npc_interact(int level, int x, int y) {
    (void)level;
    (void)x;
    (void)y;
    /* A merchant is a live AI-33 creature with a DB record and its CCM
     * PLACE_MERCHANDISE/TAKE_MERCHANDISE state.  The original does not turn
     * an arbitrary outdoor square into a fixed "friendly merchant", nor
     * does it own Firestaff's local names, dialog strings, or reputation
     * counter.  Reject until the active DB creature, GDAT merchandise fields
     * and source-owned UI route are handed through together.
     * Source: SKProject SKWINSPX/src/v4/skcrture.cpp lines 5368-5444,
     * 5697-5700; src/v5/skai.cpp::DM2_THINK_CREATURE. */
    return -1;
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

int dm2_v1_runtime_invoke_actuator(int level, int x, int y,
                                   DM2_ActuatorType type, uint16_t flag) {
    (void)level;
    (void)x;
    (void)y;
    (void)type;
    (void)flag;
    /* DM2_INVOKE_ACTUATOR reads the live DB3/DB14 record and passes its
     * links, payload, direction and timer state to the particular effect.
     * This compatibility entry receives only a coordinate, taxonomy byte and
     * flag, so even a seemingly harmless wall switch or relay would be a
     * Firestaff-created state transition. Reject every generic call until a
     * record-specific source handoff reaches the runtime.
     * Source: SKWINSPX/src/v5/c_tim_proc.cpp::DM2_INVOKE_ACTUATOR;
     * src/v4/skcrture.cpp::PLACE_MERCHANDISE/TAKE_MERCHANDISE. */
    return -1;
}

int dm2_v1_runtime_invoke_square_actuators(int level, int x, int y) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    if (!rt->boot || !rt->boot->dungeon_data) return -1;
    (void)level;
    (void)x;
    (void)y;
    /* A DB3 link alone is not an actuator command. SKProject resolves the
     * complete live record graph, DB14 payload and timer context in
     * DM2_INVOKE_ACTUATOR. Do not infer a byte layout or transition from a
     * square-local fixture before that owner is ported.
     * Source: SKWINSPX/src/v5/c_tim_proc.cpp::DM2_INVOKE_ACTUATOR. */
    return 0;
}

/* ── i18n text overlay ────────────────────────────────────────────── */

const uint8_t *dm2_v1_runtime_i18n_text(int category, int index, int field,
                                        size_t *out_size) {
    if (!g_dm2_runtime.i18n_ready) return NULL;
    return dm2_v1_i18n_query_text(&g_dm2_runtime.i18n, category, index, field,
                                  out_size);
}

int dm2_v1_runtime_i18n_ready(void) {
    return g_dm2_runtime.i18n_ready;
}

/* ── Engage command (hand actions) ────────────────────────────────── */

int dm2_v1_runtime_engage_command(
    const DM2_V1_EngageCommandRequest *request,
    DM2_V1_EngageCommandReceipt *receipt)
{
    DM2_V1_EngageCommandRequest patched;
    DM2_V1_EngageCommandReceipt local;
    if (!receipt) receipt = &local;
    if (!request) {
        memset(receipt, 0, sizeof(*receipt));
        receipt->fail_closed = 1;
        return 0;
    }
    patched = *request;

    /* Bind PROCEED_LIGHT callbacks when the runtime has a light system.
     * The light_cb/ctx remain NULL for now (the runtime light table is
     * not yet populated), so cases 5/37/38 will still fall through to
     * the engage_command's own NULL guard until the light subsystem is
     * wired.  This is the correct layering: the runtime owns the
     * binding decision. */

    /* Bind CONFUSE_CREATURE callbacks when the runtime has creature data.
     * Same layering: the confuse_cb stays NULL until the runtime owns
     * the creature record resolver and AI spec query. */

    return dm2_v1_engage_command(&patched, receipt);
}

/* ── Source evidence ──────────────────────────────────────────────── */

const char *dm2_v1_runtime_source_evidence(void) {
    return
        "DM2 V1 Runtime — source-bound boot and frame gates\n"
        "Source: SKULL.ASM T048  — input dispatch / tick update\n"
        "Source: SKULL.ASM T520  — movement speed and party placement\n"
        "Source: SKULL.ASM T560  — dungeon tick and viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor tick and weather rendering\n"
        "Weather transition seed: ReDMCSB BASE.C F0027/F0029 (LCG 0xBB40E62D, +11)\n"
        "Reference: CSB path in firestaff_game_loop.c (FS_GAME_CSB → csb_v1_viewport_render_frame)\n";
}
