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
#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_sksave_game_load_owner.h"
#include "dm2_v1_creature.h"
#include "dm2_v1_door_mechanics.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_perform_move.h"
#include "dm2_v1_pressure_plate.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_gdat_hud_m11_command.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_gdat_door_overlay_m11_command.h"
#include "dm2_v1_skproject_core.h"
#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_think_creature_pc34_compat.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_delete_creature_full_pc34_compat.h"
#include "dm2_v1_creature_schedule_pc34_compat.h"
#include "dm2_v1_champion_stat_bridge.h"
#include "dm2_v1_update_weather_pc34_compat.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_trigger.h"
#include "dm2_v1_world_model.h"
#include "dm2_v1_game_load_world_owner.h"
#include "dm2_v1_i18n.h"
#include "dm2_v1_move_record_to_pc34_compat.h"
#include "dm2_v1_projectile_impact_attack.h"
#include "dm2_v1_move_2fcf_0434.h"
#include "dm2_v1_record_ops_pc34_compat.h"
#include "dm2_v1_save_load.h"
#include "dm2_v1_sound_queue_pc34_compat.h"
#include "dm2_v1_timer_ops_pc34_compat.h"
#include "dm2_v1_cloud_pc34_compat.h"
#include "dm2_v1_hero_ops_pc34_compat.h"
#include "dm2_v1_engage_command_pc34_compat.h"
#include "dm2_v1_light_ops_pc34_compat.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_creature_ops_pc34_compat.h"
#include "dm2_v1_creature_attacks_party_pc34_compat.h"
#include "dm2_v1_creature_attacks_player_pc34_compat.h"
#include "dm2_v1_combat_damage_pc34_compat.h"
#include "dm2_v1_ccm_loop_pc34_compat.h"
#include "dm2_v1_creature_ai_loop_pc34_compat.h"
#include "fs_portable_compat.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static int dm2_runtime_projectile_query_word(
    int record_link, int word_index, void *userdata);
static int dm2_runtime_projectile_query_weight(
    int record_link, void *userdata);
static int dm2_runtime_projectile_rand_mask(int mask, void *userdata);

typedef struct {
    int (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int (*get_player_at_position)(void *ctx, int position);
    int (*creature_ai_throw_only)(void *ctx, int creature_idx);
    int16_t (*calc_attack_damage)(void *ctx, int hero_idx, int creature_idx,
                                  int16_t action_strength, int32_t skill_id);
    void (*set_pending_combat_damage)(void *ctx, int16_t damage);
} DM2_RuntimeWieldCallbacks;
extern int dm2_v1_wield_weapon(
    int hero_idx, int16_t x, int16_t y, int hero_partypos, int hero_absdir,
    int16_t action_strength, int require_melee,
    const DM2_RuntimeWieldCallbacks *cb, void *ctx);
#include <string.h>

static void dm2_v1_runtime_append_mac_wall_targets(
    const DM2_V1_DungeonData *dungeon, const DM2_V1_GameState *game);
typedef struct DM2_V1_RuntimeState DM2_V1_RuntimeState;
static void dm2_runtime_refresh_map_transition_context(
    DM2_V1_RuntimeState *rt);
static int dm2_runtime_attack_creature_at(
    DM2_V1_RuntimeState *rt, DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int target_x, int target_y,
    int16_t creature_record, int allow_wield_action);

static void dm2_runtime_apply_entered_db1_teleporter(
    DM2_V1_RuntimeState *rt, DM2_V1_GameState *gs, int level, int x, int y);

/* ── DM2 V1 Runtime State ─────────────────────────────────────────── */

struct DM2_V1_RuntimeState {
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
    /* GAME_LOAD transfers this party from the source-complete candidate.  It
     * is the mutable c_party owner for input actions; session_snapshot is
     * retained separately for read-only receipts and must not be used for
     * mutations. */
    DM2_V1_Party source_party;
    int source_party_valid;
    /* c_startend.cpp/ddat.v1e0288: the one-based hero number excluded by
     * PROCESS_POISON.  This is transferred with the GAME_LOAD party owner. */
    int16_t source_next_champion_number;
    /* c_tim_proc.cpp 0x47 source globals: savegames1.b_02 and v1e0976. */
    uint8_t source_hero_ench_countdown;
    int16_t source_hero_ench_target;
    uint8_t source_savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];
    /* c_events.cpp/startend.cpp savegames1.b_04: Aura of Speed. */
    uint8_t source_aura_of_speed;
    int source_aura_of_speed_valid;
    /* c_tim 0x46 owner: source light value used by PROCESS_TIMER_LIGHT. */
    int16_t source_light_level;
    /* Source-owned selection state used by
     * SkWinCore::DISPLAY_RIGHT_PANEL_SQUAD_HANDS.  Zero means that the
     * original runtime has not selected a champion; do not promote it to a
     * leader or fabricate a hand icon. */
    int16_t source_curacthero;
    int16_t source_curactmode;
    int16_t source_event_hero_index;
    /* c_events.cpp:1846 v1e0976: the champion whose right panel is
     * selected by the source eye event.  This is distinct from
     * party.curacthero; the source does not silently change the active hand
     * champion when the eye is clicked. */
    int16_t source_v1e0976;
    int source_sleeping;
    uint8_t source_attack_counter;
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
    /* Last renderer-owned c_rwbb click targets.  These are copied only from
     * the authenticated frame that is handed to M11; host rectangles are not
     * promoted into gameplay targets. */
    DM2_V1_ViewportClickTarget source_click_targets[
        DM2_V1_VIEWPORT_CLICK_TARGET_COUNT];
    uint8_t source_click_target_count;
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
     * candidate evidence validates; movement and adjacent melee are bounded
     * CCM owners, while unsupported stream branches stay receipted
     * fail-closed. */
    DM2_V1_RecordPoolSet record_pools;
    int record_pools_valid;
    DM2_V1_ThinkCreatureBinding think_binding;
    int think_binding_ready;
    DM2_V1_CcmLoopReceipt last_ccm_receipt;
    int ccm_receipt_valid;
    int dynamic_path_attempts;
    int dynamic_path_admissions;
    int dynamic_move_queue_admissions;
    int dynamic_move_timer_consumptions;
    int dynamic_move_successes;
    int dynamic_move_last_failure;
    int dynamic_move_pending_source_map;
    int dynamic_move_pending_source_x;
    int dynamic_move_pending_source_y;
    int16_t dynamic_move_pending_record;
    int dynamic_path_last_failure;
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
    DM2_V1_RuntimeSpellFailureGdatReceipt last_spell_failure_gdat;
    DM2_V1_I18nContext i18n;
    int i18n_ready;
    /* Reserved only for a future complete GAME_LOAD sound handoff.  It must
     * not be initialised as a stand-in for the source-sized xsndptr2 table. */
    DM2_V1_SoundQueueState sound_queue;
    DM2_V1_SoundQueueEnv sound_env;
    int sound_queue_ready;
    /* Ownership transferred from the committed GAME_LOAD candidate. */
    DM2_V1_SoundSsoundEntry *source_sound_entries;
    DM2_V1_GameLoadSoundSampleBinding *source_sound_bindings;
    uint16_t source_sound_binding_count;
    /* Private GAME_LOAD candidate transferred from BootProfile.  This is
     * ownership only: the candidate remains non-playable until the complete
     * source session publication transaction is implemented. */
    DM2_V1_GameLoadRuntimeSessionCandidate *game_load_candidate;
    uint32_t game_load_candidate_hash;
    uint32_t game_load_candidate_source_hash;
    /* CDDA playback callback — set by M11 host to push PCM to SDL3 */
    void (*cdda_play_cb)(void *ctx, const uint8_t *pcm, size_t size, int loop);
    void (*cdda_stop_cb)(void *ctx);
    void *cdda_cb_ctx;
};

static int32_t __attribute__((unused)) dm2_runtime_mac_key_can_handle(
    uint16_t actuator, int16_t expected_item_type, void *user)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    uint16_t held;
    uint8_t cls2 = 0xffu;
    uint16_t distinctive = 0xffffu;
    DM2_V1_SkprojectQueryCls2Receipt cls_receipt;
    DM2_V1_SkprojectDistinctiveItemtypeReceipt type_receipt;

    (void)actuator;
    if (!rt || !rt->record_pools_valid || expected_item_type < 0) return 0;
    held = (uint16_t)rt->leader_hand_object;
    if (held == (uint16_t)DM2_V1_RECORD_HANDLE_NULL ||
        held == (uint16_t)DM2_V1_RECORD_HANDLE_END) return 0;
    memset(&cls_receipt, 0, sizeof(cls_receipt));
    if (!dm2_v1_skproject_query_cls2_from_record(
            held, &rt->record_pools, &cls2, &cls_receipt) ||
        cls2 == 0xffu) return 0;
    memset(&type_receipt, 0, sizeof(type_receipt));
    if (!dm2_v1_skproject_get_distinctive_itemtype(
            held, cls2, &distinctive, &type_receipt) ||
        !type_receipt.valid) return 0;
    return distinctive == (uint16_t)expected_item_type;
}

static DM2_V1_RuntimeState g_dm2_runtime;

static void dm2_runtime_apply_entered_db1_teleporter(
    DM2_V1_RuntimeState *rt, DM2_V1_GameState *gs, int level, int x, int y)
{
    DM2_V1_DungeonData *dungeon;
    int raw;
    int square_type;
    int first;
    int record_type = -1;
    const uint8_t *record;
    DM2_V1_Move2fcf0434Receipt gate;
    int destination_map;
    int destination_x;
    int destination_y;
    int scope;
    int rotation;
    int rotation_type;

    if (!rt || !gs || !rt->boot || !rt->boot->dungeon_data)
        return;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, level, x, y);
    square_type = dm2_v1_dungeon_get_square_type(dungeon, level, x, y);
    if (raw < 0 || square_type != 5 || (raw & 0x08) == 0)
        return;
    first = dm2_v1_dungeon_get_first_thing(dungeon, level, x, y);
    if (first < 0 || (((unsigned)first >> 10) & 0x0fu) != 1u)
        return;
    record = dm2_v1_dungeon_get_thing_record(
        dungeon, (uint16_t)first, &record_type, NULL, NULL);
    if (!record || record_type != 1)
        return;
    {
        uint16_t w2 = dm2_v1_dungeon_read_record_u16(dungeon, record + 2);
        uint16_t w4 = dm2_v1_dungeon_read_record_u16(dungeon, record + 4);
        destination_x = (int)(w2 & 0x1fu);
        destination_y = (int)((w2 >> 5) & 0x1fu);
        scope = (int)((w2 >> 13) & 3u);
        rotation = (int)((w2 >> 10) & 3u);
        rotation_type = (int)((w2 >> 12) & 1u);
        destination_map = (int)(w4 >> 8);
    }
    if (!dm2_v1_DM2_move_2fcf_0434_teleporter_gate(
            dungeon, level, x, y, record_type, first,
            dungeon->record_graph_complete, scope, destination_map,
            destination_x, destination_y, &gate) || !gate.admitted)
        return;
    gs->current_level = destination_map;
    gs->party_x = destination_x;
    gs->party_y = destination_y;
    gs->party_dir = rotation_type ? rotation : ((gs->party_dir + rotation) & 3);
    gs->outdoor = dm2_v1_dungeon_is_outdoor(dungeon, destination_map);
    rt->dungeon_level = destination_map;
    rt->view_dir = gs->party_dir;
    rt->outdoor = gs->outdoor;
    dm2_runtime_refresh_map_transition_context(rt);
}

/* SKProject's stair query resolves a map by shared world coordinates rather
 * than by a made-up level +/- 1 rule.  Keep the descriptor/cursor ownership
 * local to the move so every edition uses the authenticated DUNGEON.DAT
 * offsets and dimensions. */
static int dm2_runtime_resolve_entered_stairs(
    DM2_V1_DungeonData *dungeon, int source_map, int source_x, int source_y,
    int *out_map, int *out_x, int *out_y)
{
    DM2_V1_SkprojectMapDescriptor maps[DM2_V1_MAX_LEVELS];
    uint8_t cursor[DM2_V1_MAX_LEVELS];
    int raw;
    int16_t x;
    int16_t y;
    int delta;
    int target;

    if (out_map) *out_map = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (!dungeon || source_map < 0 || source_map >= dungeon->level_count ||
        dungeon->level_count > DM2_V1_MAX_LEVELS)
        return 0;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, source_map, source_x, source_y);
    if (raw < 0 || dm2_v1_dungeon_get_square_type(
            dungeon, source_map, source_x, source_y) != 3)
        return 0;

    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].world_x = (int16_t)dungeon->map_offset_x[map];
        maps[map].world_y = (int16_t)dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
        /* The locator only needs this field to reject a known teleporter
         * target. The actual destination tile is revalidated below. */
        maps[map].tile_type_at_local = 0;
        cursor[map] = (uint8_t)map;
    }
    /* A stair must leave its source map. Keep the source out of the scan; the
     * source routine otherwise legitimately returns the same map when the
     * world-coordinate rectangle overlaps itself. */
    cursor[source_map] = 0xffu;
    delta = (raw & 0x04) ? -1 : 1;
    x = (int16_t)source_x;
    y = (int16_t)source_y;
    target = dm2_v1_skproject_locate_other_level(
        maps, (uint16_t)dungeon->level_count, (int16_t)source_map,
        (int16_t)delta, &x, &y, cursor, (uint16_t)dungeon->level_count,
        0, NULL, NULL);
    if (target < 0 || target == source_map || x < 0 || y < 0 ||
        x >= dungeon->level_widths[target] ||
        y >= dungeon->level_heights[target])
        return -1;
    if (out_map) *out_map = target;
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    return 1;
}

static int dm2_runtime_resolve_entered_pit(
    DM2_V1_DungeonData *dungeon, int source_map, int source_x, int source_y,
    int *out_map, int *out_x, int *out_y)
{
    DM2_V1_SkprojectMapDescriptor maps[DM2_V1_MAX_LEVELS];
    uint8_t cursor[DM2_V1_MAX_LEVELS];
    int raw;
    int16_t x;
    int16_t y;
    int target;

    if (out_map) *out_map = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (!dungeon || source_map < 0 || source_map >= dungeon->level_count ||
        dungeon->level_count > DM2_V1_MAX_LEVELS)
        return 0;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, source_map, source_x, source_y);
    if (raw < 0 || dm2_v1_dungeon_get_square_type(
            dungeon, source_map, source_x, source_y) != 2 ||
        (raw & 0x08) == 0 || (raw & 0x01) != 0)
        return 0;
    memset(maps, 0, sizeof(maps));
    for (int map = 0; map < dungeon->level_count; ++map) {
        maps[map].map_id = (uint8_t)map;
        maps[map].world_x = (int16_t)dungeon->map_offset_x[map];
        maps[map].world_y = (int16_t)dungeon->map_offset_y[map];
        maps[map].width = (int16_t)dungeon->level_widths[map];
        maps[map].height = (int16_t)dungeon->level_heights[map];
        maps[map].tile_type_at_local = 0;
        cursor[map] = (uint8_t)map;
    }
    cursor[source_map] = 0xffu;
    x = (int16_t)source_x;
    y = (int16_t)source_y;
    /* DM2_query_19f0_124b's open-pit branch uses direction=1 and
     * flags=0x8; the locator delta is the same source level step. */
    target = dm2_v1_skproject_locate_other_level(
        maps, (uint16_t)dungeon->level_count, (int16_t)source_map, 1,
        &x, &y, cursor, (uint16_t)dungeon->level_count, 0, NULL, NULL);
    if (target < 0 || target == source_map || x < 0 || y < 0 ||
        x >= dungeon->level_widths[target] ||
        y >= dungeon->level_heights[target])
        return -1;
    /* c_move's destination owner still rejects a wall/inaccessible landing;
     * a pit route is not allowed to manufacture a safe landing tile. */
    {
        int target_type = dm2_v1_dungeon_get_square_type(dungeon, target, x, y);
        if (target_type == 0 || target_type == 13)
            return -1;
    }
    if (out_map) *out_map = target;
    if (out_x) *out_x = x;
    if (out_y) *out_y = y;
    return 1;
}
/* M11 input is serialized.  This transient selector lets the pointer path
 * preserve the exact authenticated c_rwbb target while reusing the existing
 * column-oriented keyboard owner. */
static int g_dm2_mac_wall_requested_target = -1;
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
static int g_dm2_last_wield_death_drop_count = 0;
static int g_dm2_last_wield_death_drop_iterations = 0;
static int g_dm2_last_wield_death_drop_alloc_failures = 0;
static int g_dm2_last_wield_death_drop_first_itemspec = 0;
static int g_dm2_last_wield_death_drop_first_db = -1;
static int g_dm2_last_wield_death_drop_alloc_free_records = 0;
static int g_dm2_last_wield_death_deallocated = 0;
static int g_dm2_last_asset_carried_item_count = 0;
static int g_dm2_last_fallback_carried_item_count = 0;
static int g_dm2_last_asset_projectile_count = 0;
static int g_dm2_last_fallback_projectile_count = 0;
static DM2_V1_RuntimeProjectileRenderReceipt g_dm2_last_projectile_render;
static DM2_V1_RuntimeMissileImpactReceipt g_dm2_last_missile_impact;
static DM2_V1_RuntimeCreatureDamageReceipt g_dm2_last_creature_damage;
static DM2_V1_RuntimeWieldAttackReceipt g_dm2_last_wield_attack;
static int g_dm2_last_asset_hud_portrait_count = 0;
static int g_dm2_last_fallback_hud_portrait_count = 0;
static DM2_V1_PerformMoveReceipt g_dm2_last_perform_move;
static DM2_V1_RuntimeFrameOwnershipReceipt g_dm2_frame_ownership;
static DM2_V1_ViewportM11FrameReceipt g_dm2_last_m11_frame;

typedef struct {
    DM2_V1_RuntimeState *runtime;
    uint32_t timer_ticket;
    int failed;
} DM2_V1_RuntimeSpellLightContext;

static void dm2_runtime_spell_queue_light_timer(
    void *context, int16_t value, uint32_t fire_tick)
{
    DM2_V1_RuntimeSpellLightContext *ctx =
        (DM2_V1_RuntimeSpellLightContext *)context;
    DM2_V1_SourceTimer timer;
    DM2_V1_SourceTimerResult result;

    if (!ctx || !ctx->runtime || ctx->runtime->dungeon_level < 0 ||
        ctx->runtime->dungeon_level > 0xff) {
        if (ctx) ctx->failed = 1;
        return;
    }
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)ctx->runtime->dungeon_level << 24) |
                          (fire_tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = DM2_V1_TIMER_LIGHT;
    timer.value_a = value;
    ctx->timer_ticket = dm2_v1_source_timer_enqueue_ticketed(
        &ctx->runtime->timer_queue, &timer, 0u, &result);
    if (result != DM2_V1_SOURCE_TIMER_OK || ctx->timer_ticket == 0u)
        ctx->failed = 1;
}

static void dm2_runtime_spell_recalc_light(void *context)
{
    /* DM2_PROCEED_LIGHT's source callback reaches the c_light recalculation
     * owner. Runtime light-map recalculation is still a separate viewport
     * consumer; the source light scalar and 0x46 timer are the complete
     * gameplay mutation owned by this cast transaction. */
    (void)context;
}

static void dm2_runtime_remove_enchant_timer_mask(
    DM2_V1_SourceTimerQueue *queue, uint8_t mask)
{
    size_t i = 0;

    if (!queue || mask == 0u) return;
    while (i < queue->count) {
        DM2_V1_SourceTimer *timer = &queue->timers[i];
        if (timer->type != 0x48 || (timer->actor & mask) == 0u) {
            ++i;
            continue;
        }
        timer->actor = (uint8_t)(timer->actor & (uint8_t)~mask);
        if (timer->actor != 0u) {
            ++i;
            continue;
        }
        if (i + 1u < queue->count) {
            memmove(&queue->timers[i], &queue->timers[i + 1u],
                    (queue->count - i - 1u) * sizeof(queue->timers[0]));
            memmove(&queue->source_indices[i], &queue->source_indices[i + 1u],
                    (queue->count - i - 1u) * sizeof(queue->source_indices[0]));
            memmove(&queue->tickets[i], &queue->tickets[i + 1u],
                    (queue->count - i - 1u) * sizeof(queue->tickets[0]));
        }
        --queue->count;
    }
}

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
    const DM2_V1_G1DirectDoorRoot *door = NULL;
    const DM2_V1_AssetLoader *loader;
    uint16_t creature_passes_closed_door;
    int door_gdat_index;
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
    /* SKProject c_map.cpp selects the DB0 root, Door::DoorType() selects
     * map-header slot 0/1, and skdoor.cpp::GET_DOOR_STAT_0D resolves that
     * slot's real DOORS/dtWordValue/0x0d entry.  A terrain tag or the
     * historical four-door compatibility table cannot supply this gameplay
     * value: it belongs to the selected G1 map and its GDAT corpus. */
    if (level != rt->dungeon_level || dd->square_bytes != 1 ||
        !dm2_v1_g1_runtime_map_door_at(&rt->g1_runtime_map_doors,
                                       x, y, &door) ||
        !door || door->door_type >= 2u ||
        !rt->map_door_gfx_active[door->door_type] ||
        !(loader = dm2_v1_boot_asset_loader(rt->boot))) {
        return 0;
    }
    door_gdat_index = (int)rt->map_door_gfx_list[door->door_type];
    if (!dm2_v1_asset_load_word_value(loader, DM2_GDAT_CATEGORY_DOORS,
                                      door_gdat_index, 0x0d,
                                      &creature_passes_closed_door)) {
        return 0;
    }
    state = dm2_runtime_door_state((uint16_t)raw);
    *out_state = state;
    /* The compact CCM boundary retains the existing bit-0 contract.  The
     * value is nevertheless sourced only from SKProject's exact GDAT 0x0d
     * query: nonzero lets a creature pass a fully closed door. */
    *out_attributes = creature_passes_closed_door != 0u
        ? DM2_DOOR_ATTR_CREATURES_CAN_SEE_THROUGH : 0u;
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

/* Convert the loader's source tile class into the common movement enum.
 * Byte-square maps use c_tim_proc's classes (0 wall, 1 floor, 2 pit, 3
 * stairs, 4 door, 5 teleporter, 6 trick-wall). Two-byte maps already store
 * the DM2_SquareType values in their low five bits. */
static int dm2_runtime_normalize_square_type_for_dungeon(
    const DM2_V1_DungeonData *dungeon, int raw_type, int raw_tile) {
    if (dungeon && dungeon->square_bytes == 1) {
        switch (raw_type & 0x1f) {
            case 0: return DM2_SQUARE_WALL;
            case 1: return DM2_SQUARE_FLOOR;
            case 2: return DM2_SQUARE_PIT;
            case 3: return (raw_tile & 0x04) != 0
                        ? DM2_SQUARE_STAIRS_DOWN : DM2_SQUARE_STAIRS_UP;
            case 4: return DM2_SQUARE_DOOR;
            case 5: return DM2_SQUARE_TELEPORTER;
            case 6: return DM2_SQUARE_SECRET_DOOR;
            default: return raw_type & 0x1f;
        }
    }
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
    square_type = dm2_runtime_normalize_square_type_for_dungeon(dd,
                                                                square_type,
                                                                raw);
    return square_type == DM2_SQUARE_DOOR ||
           dm2_runtime_has_door_record_at(dd, level, x, y) ||
           (dd && dd->square_bytes != 1 &&
            dm2_runtime_raw_is_door_square((uint16_t)raw));
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
    /* UPDATE_GFXSET has no WALL_GFX transaction for an outdoor map.  Keep
     * that source absence explicit: requiring an indoor plan here prevented
     * a verified Amiga outdoor scene from publishing its own sky/ground
     * materials before the renderer could make the same distinction. */
    if (!rt->outdoor &&
        (!dm2_v1_boot_gdat_wall_m11_command_plan(
             rt->boot, rt->map_graphics_style, &rt->gdat_wall_material_plan) ||
         !rt->gdat_wall_material_plan.valid ||
         rt->gdat_wall_material_plan.graphicsset !=
             (uint8_t)rt->map_graphics_style)) {
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
    DM2_MusicSystem music_system;
    int party_x = 0;
    int party_y = 0;
    int track = -1;

    memset(&receipt, 0, sizeof(receipt));
    receipt.map_index = rt ? rt->dungeon_level : -1;
    receipt.selected_track = -1;
    if (!rt || !rt->boot) return;
    music_system = dm2_v1_platform_music_system(rt->boot->platform);
    receipt.source_songlist_verified = rt->boot->songlist_verified ? 1 : 0;
    /* SKProject's DM2_GAME_LOAD restores party pose, c_hero, timers and the
     * dungeon graph before its post-load movement/map path can select a
     * level cue (sksvgame.cpp::DM2_GAME_LOAD lines 1415-1574;
     * c_sound.cpp::DM2_SOUND2 lines 465-499).  A boot-mounted File_header
     * supplies neither party ownership nor a live map transition.  Do not
     * let its default level zero queue HMP/CDDA merely because real media is
     * available; SHOW_MENU_SCREEN owns its distinct source menu cue.
     *
     * This is deliberately before the FM Towns coordinate lookup: probing
     * its default (0,0) cell would turn an unowned host pose into CDDA. */
    if (!rt->boot->source_game_load_session_ready) {
        receipt.valid = 1;
        receipt.blocked_no_session = 1;
        rt->music_map_receipt = receipt;
        return;
    }
    /* DMWeb's 40-byte CD.DAT format is a level/X/Y trigger table, not a
     * map-default playlist.  A missing live party owner must therefore
     * produce no CDDA request instead of probing the synthetic (0,0) cell.
     * Source: DMWeb "Dungeon Master II Music Triggers", format 1; SKProject
     * SKULLWIN/c_sound.cpp::DM2_GET_MUSIC_INDEX_FROM_MODLIST call boundary. */
    if (music_system == DM2_MUSIC_SYSTEM_CDDA_COORD) {
        const DM2_V1_GameState *game_state =
            (const DM2_V1_GameState *)rt->boot->dm2_state;
        if (!game_state) {
            rt->music_map_receipt = receipt;
            return;
        }
        party_x = game_state->party_x;
        party_y = game_state->party_y;
    }
    if (!dm2_v1_boot_music_track_for_level(rt->boot, rt->dungeon_level,
                                            party_x, party_y, &track)) {
        rt->music_map_receipt = receipt;
        return;
    }
    receipt.valid = 1;
    receipt.selected_track = track;
    memset(&queue, 0, sizeof(queue));

    if (music_system == DM2_MUSIC_SYSTEM_CDDA_COORD) {
        uint8_t *pcm = NULL;
        size_t pcm_size;
        int media_verified = 0;
        if (rt->cdda_stop_cb)
            rt->cdda_stop_cb(rt->cdda_cb_ctx);
        pcm_size = dm2_v1_boot_load_cdda_track(rt->boot, track, &pcm,
                                                &media_verified);
        if (pcm && pcm_size > 0) {
            receipt.queue_result = dm2_v1_sound_queue_cdda(
                pcm, pcm_size, track, 1, media_verified, &queue);
            receipt.source_stream_resolved = queue.asset_resolved ? 1 : 0;
            free(pcm);
        }
    } else if ((rt->boot->platform == DM2_PLATFORM_MAC_EN ||
                rt->boot->platform == DM2_PLATFORM_MAC_FR) &&
               rt->boot->mac_application_resource) {
        receipt.queue_result = dm2_v1_sound_queue_mac_midi(
            rt->boot->mac_application_resource,
            rt->boot->mac_application_resource_size, 1000 + track, 1, &queue);
        receipt.source_stream_resolved = queue.asset_resolved ? 1 : 0;
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
    DM2_V1_DungeonData *dungeon = NULL;
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
    dm2_v1_gdat_scene_m11_command_plan_free(
        &g_dm2_runtime.gdat_scene_material_plan);
    dm2_v1_gdat_wall_m11_command_plan_free(
        &g_dm2_runtime.gdat_wall_material_plan);
    dm2_v1_gdat_door_overlay_m11_command_plan_free(
        &g_dm2_runtime.gdat_door_material_plan);
    if (g_dm2_runtime.game_load_candidate) {
        dm2_v1_game_load_runtime_session_candidate_free(
            g_dm2_runtime.game_load_candidate);
        free(g_dm2_runtime.game_load_candidate);
    }
    if (g_dm2_runtime.record_pools_valid) {
        dm2_v1_record_pool_set_free(&g_dm2_runtime.record_pools);
    }
    if (g_dm2_runtime.caii_ready) {
        dm2_v1_caii_array_free(&g_dm2_runtime.caii);
    }
    free(g_dm2_runtime.source_sound_entries);
    free(g_dm2_runtime.source_sound_bindings);
    if (g_dm2_runtime.i18n_ready) {
        dm2_v1_i18n_destroy(&g_dm2_runtime.i18n);
    }
    memset(&g_dm2_runtime, 0, sizeof(g_dm2_runtime));
    memset(&g_dm2_frame_ownership, 0, sizeof(g_dm2_frame_ownership));
    memset(&g_dm2_last_missile_impact, 0,
           sizeof(g_dm2_last_missile_impact));
    memset(&g_dm2_last_creature_damage, 0,
           sizeof(g_dm2_last_creature_damage));
    /* A NULL profile is the explicit runtime-release path.  In particular,
     * startup may reject a selected FM Towns companion after the runtime has
     * borrowed the native GDAT loader; do not retain pointers into the
     * profile that boot cleanup is about to free. */
    if (!boot_profile) {
        dm2_v1_sound_bind_gdat_loader(NULL, 0);
        dm2_v1_sound_bind_runtime_queue(NULL);
        return;
    }
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
    g_dm2_runtime.source_curacthero = 0;
    g_dm2_runtime.source_curactmode = 0;
    g_dm2_runtime.source_event_hero_index = 0;
    g_dm2_runtime.source_v1e0976 = 0;
    g_dm2_runtime.source_sleeping = 0;
    memset(&g_dm2_runtime.minions, 0, sizeof(g_dm2_runtime.minions));
    g_dm2_runtime.last_npc_level = -1;
    g_dm2_runtime.last_npc_x = -1;
    g_dm2_runtime.last_npc_y = -1;
    /* No merchant/NPC identity exists until an admitted AI-33 DB creature
     * and its source-owned CCM/UI chain have supplied one.  Do not expose a
     * friendly-merchant fixture through the runtime accessor. */
    /* No merchant/NPC identity can exist until the source DB4/CCM owner is
     * decoded; keep the runtime sentinel independent of the test-only shop
     * catalog module. */
    g_dm2_runtime.last_npc_id = -1;
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
    memset(&g_dm2_runtime.last_spell_failure_gdat, 0,
           sizeof(g_dm2_runtime.last_spell_failure_gdat));
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
    /* Do not manufacture c_sound's xsndptr2 here.  DM2_GAME_LOAD sizes it
     * from the admitted GDAT/DYN4 SOUND9 population (c_dballoc then
     * c_gdatfile.cpp::DM2_482b_0684); its DOS corpus capacity is not the
     * legacy fixed host queue.  Until the private GAME_LOAD sound owner,
     * c_tim and c_map/party context commit together, binding a zeroed
     * DM2_V1_SoundQueueState would let timer or creature code resolve
     * caller-authored entries against a non-source queue.  Keep the global
     * query seam explicitly unbound instead. */
    dm2_v1_sound_bind_runtime_queue(NULL);
    memset(&g_dm2_runtime.sound_env, 0, sizeof(g_dm2_runtime.sound_env));
    g_dm2_runtime.sound_queue_ready = 0;
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

void dm2_v1_runtime_unbind_boot_profile(DM2_V1_BootProfile *boot_profile)
{
    if (!boot_profile || g_dm2_runtime.boot != boot_profile) {
        return;
    }
    dm2_v1_runtime_init(NULL);
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

/* Source-owned DM2_CREATE_CLOUD admission for the player Poison Cloud spell.
 * The ordinary DB15/0x19 lifecycle is present in the runtime. Reflector
 * subtype 0x0e is deliberately not admitted here: its timer lifecycle and
 * incoming spell-bounce consumer are separate source owners. */
static int dm2_runtime_spell_create_cloud(
    DM2_V1_RuntimeState *rt, int spell_index, int cast_power,
    int *out_record, uint32_t *out_ticket)
{
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RecordPoolSet pool_before;
    DM2_V1_SourceTimerQueue queue_before;
    DM2_V1_SoundQueueState sound_before;
    uint8_t *raw_before = NULL;
    DM2_V1_SourceTimer timer;
    DM2_V1_SourceTimerResult result;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;
    int map, x, y, strength, cloud_spell;
    int16_t head, record_handle;
    uint8_t *record = NULL;

    if (out_record) *out_record = -1;
    if (out_ticket) *out_ticket = 0u;
    memset(&pool_before, 0, sizeof(pool_before));
    if (rt) queue_before = rt->timer_queue;
    if (!rt || !rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid || !rt->sound_queue_ready ||
        rt->timer_queue.count >= DM2_V1_SOURCE_TIMER_MAX) {
        return 0;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = rt->dungeon_level;
    x = dm2_v1_runtime_get_party_x();
    y = dm2_v1_runtime_get_party_y();
    if (spell_index == 14) {
        cloud_spell = (int)(int16_t)0xff87;
        strength = cast_power;
        if (strength < 1) strength = 1;
        if (strength > 255) strength = 255;
    } else {
        return 0;
    }
    if (map < 0 || map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map] ||
        !dungeon->raw_data || dungeon->raw_size <= 0) {
        return 0;
    }
    if (!dm2_v1_record_pool_set_clone(&pool_before, &rt->record_pools) ||
        !(raw_before = (uint8_t *)malloc((size_t)dungeon->raw_size))) {
        dm2_v1_record_pool_set_free(&pool_before);
        free(raw_before);
        return 0;
    }
    memcpy(raw_before, dungeon->raw_data, (size_t)dungeon->raw_size);
    queue_before = rt->timer_queue;
    sound_before = rt->sound_queue;

    record_handle = dm2_v1_record_pool_alloc_new_record(
        &rt->record_pools, 15u);
    record = dm2_v1_record_pool_address_mut(&rt->record_pools, record_handle);
    if (record_handle < 0 || !record ||
        rt->record_pools.pools[15].record_size < 4) goto cloud_cast_rollback;
    record[0] = 0xfeu; record[1] = 0xffu;
    record[2] = (uint8_t)(((cloud_spell - (int)(int16_t)0xff80) & 0x7f) | 0x80u);
    record[3] = (uint8_t)strength;
    head = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (!dm2_v1_record_pool_append_to_list(&rt->record_pools, &head,
                                           record_handle) ||
        dm2_v1_dungeon_set_first_thing(dungeon, map, x, y,
                                       (uint16_t)head) != 0)
        goto cloud_cast_rollback;

    memset(&sound_env, 0, sizeof(sound_env));
    sound_env = rt->sound_env;
    sound_env.current_map = (int16_t)map;
    sound_env.party_x = (int16_t)x;
    sound_env.party_y = (int16_t)y;
    sound_env.facing = (uint16_t)rt->view_dir;
    sound_env.gametick = (int32_t)rt->tick_count;
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    if (!dm2_v1_sound_queue_noise_gen2(
        &rt->sound_queue, 0x0d, (uint8_t)(record[2] & 0x7f),
        (int8_t)0x81, (int8_t)0xfe, (int16_t)x, (int16_t)y,
        1, 0x6c, 0xff, &sound_env, &sound_receipt) ||
        !sound_receipt.valid) {
        /* c_sfx.cpp::DM2_QUEUE_NOISE_GEN2 is part of the cloud creation
         * transaction. A rejected/missing source sound owner must not leave
         * a DB15 record and PROCESS_CLOUD timer published without its paired
         * source event. */
        goto cloud_cast_rollback;
    }

    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        ((uint32_t)(rt->tick_count + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = DM2_V1_TIMER_PROCESS_CLOUD;
    timer.value_a = (int16_t)((x & 0xff) | ((y & 0xff) << 8));
    timer.value_b = record_handle;
    *out_ticket = dm2_v1_source_timer_enqueue_ticketed(
        &rt->timer_queue, &timer, 0u, &result);
    if (result != DM2_V1_SOURCE_TIMER_OK || *out_ticket == 0u)
        goto cloud_cast_rollback;
    if (out_record) *out_record = record_handle;
    dm2_v1_record_pool_set_free(&pool_before);
    free(raw_before);
    return 1;

cloud_cast_rollback:
    memcpy(dungeon->raw_data, raw_before, (size_t)dungeon->raw_size);
    dm2_v1_record_pool_set_free(&rt->record_pools);
    rt->record_pools = pool_before;
    memset(&pool_before, 0, sizeof(pool_before));
    rt->timer_queue = queue_before;
    rt->sound_queue = sound_before;
    free(raw_before);
    if (out_ticket) *out_ticket = 0u;
    return 0;
}

static void dm2_runtime_record_wr16(const DM2_V1_RuntimeState *rt,
                                    uint8_t *p, uint16_t value)
{
    if (rt && rt->record_pools.source_words_big_endian) {
        p[0] = (uint8_t)(value >> 8);
        p[1] = (uint8_t)value;
    } else {
        p[0] = (uint8_t)value;
        p[1] = (uint8_t)(value >> 8);
    }
}

/* Source: SkWinCore.cpp:17301-17325 CAST/SHOOT_CHAMPION_MISSILE and
 * SKULLWIN/c_item.cpp:1043-1138 DM2_SHOOT_ITEM.  A spell missile is not a
 * timer-only effect: the source first owns a special missile object id and a
 * DB14 record, then publishes the matching step timer. */
static int dm2_runtime_spell_create_missile(
    DM2_V1_RuntimeState *rt, const DM2_V1_SpellCastPlayerReceipt *cast,
    const DM2_V1_Hero *hero, int *out_record, uint16_t *out_object,
    uint8_t *out_damage, uint8_t *out_energy, uint32_t *out_ticket,
    int *out_failure_stage)
{
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RecordPoolSet pool_before;
    DM2_V1_SourceTimerQueue queue_before;
    uint8_t *raw_before = NULL;
    DM2_V1_SourceTimer timer;
    DM2_V1_SourceTimerResult result;
    int16_t head, record_handle;
    int tile_raw;
    uint8_t *record;
    int map, x, y, dir, timer_step, damage, kinetic_energy, effect;
    int mana_after, mp_bonus, accuracy;
    uint16_t object;

    if (out_record) *out_record = -1;
    if (out_object) *out_object = 0u;
    if (out_damage) *out_damage = 0u;
    if (out_energy) *out_energy = 0u;
    if (out_ticket) *out_ticket = 0u;
    if (out_failure_stage) *out_failure_stage = 0;
    memset(&pool_before, 0, sizeof(pool_before));
    if (rt) queue_before = rt->timer_queue;
    if (!rt || !cast || !hero || !rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid || rt->timer_queue.count >=
            DM2_V1_SOURCE_TIMER_MAX) return 0;
    effect = cast->object_effect;
    if (effect < 0 || effect > 0x3f) return 0;
    object = (uint16_t)(0xff80u + (uint16_t)effect);
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = rt->dungeon_level;
    x = dm2_v1_runtime_get_party_x();
    y = dm2_v1_runtime_get_party_y();
    dir = rt->view_dir & 3;
    if (map < 0 || map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map] ||
        !dungeon->raw_data || dungeon->raw_size <= 0) return 0;

    /* Source c_hero.cpp:3840-3874 first spends mana, then passes the
     * command power as argb1, 0x5a as argb2, and the derived accuracy as
     * argb3 to SHOOT_CHAMPION_MISSILE.  bitem.cpp:1043-1138 stores those
     * bytes verbatim in the DB14 record and timer word. */
    mana_after = hero->curMP - cast->mana_cost;
    if (mana_after < 0) mana_after = 0;
    mp_bonus = mana_after >> 5;
    if (mp_bonus > 6) mp_bonus = 6;
    accuracy = 10 - mp_bonus;
    damage = cast->cast_power;
    if (damage < (accuracy << 2)) {
        damage += 4;
        accuracy = damage / 4;
    }
    if (damage < 0) damage = 0;
    if (damage > 255) damage = 255;
    timer_step = accuracy & 0xf;
    kinetic_energy = 0x5a;

    if (!dm2_v1_record_pool_set_clone(&pool_before, &rt->record_pools)) {
        if (out_failure_stage) *out_failure_stage = 1;
        goto missile_cast_rollback;
    }
    if (!(raw_before = (uint8_t *)malloc((size_t)dungeon->raw_size))) {
        if (out_failure_stage) *out_failure_stage = 2;
        goto missile_cast_rollback;
    }
    memcpy(raw_before, dungeon->raw_data, (size_t)dungeon->raw_size);
    queue_before = rt->timer_queue;
    record_handle = dm2_v1_record_pool_alloc_new_record(&rt->record_pools, 14u);
    record = dm2_v1_record_pool_address_mut(&rt->record_pools, record_handle);
    if (record_handle < 0) {
        if (out_failure_stage) *out_failure_stage = 3;
        goto missile_cast_rollback;
    }
    if (!record || rt->record_pools.pools[14].record_size < 8) {
        if (out_failure_stage) *out_failure_stage = 4;
        goto missile_cast_rollback;
    }
    dm2_runtime_record_wr16(rt, record, 0xfffeu);
    dm2_runtime_record_wr16(rt, record + 2, object);
    record[4] = (uint8_t)damage;
    record[5] = (uint8_t)kinetic_energy;
    dm2_runtime_record_wr16(rt, record + 6, 0u);
    tile_raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    head = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    /* Mac/Amiga byte-square maps can retain the source object flag while
     * exposing an empty ground-stack head as the sign-extended 0xFEFF
     * sentinel.  That is an existing stack root, not a request to insert a
     * new object-index entry.  Preserve the source distinction: only a
     * genuinely unmarked cell takes APPEND_RECORD_TO's empty-tile route. */
    if (head < 0 && (tile_raw < 0 || (tile_raw & 0x10) == 0)) {
        if (dm2_v1_dungeon_insert_first_thing_empty_tile(
                dungeon, map, x, y, (uint16_t)record_handle) != 0) {
            if (out_failure_stage) *out_failure_stage = 6;
            goto missile_cast_rollback;
        }
    } else if (head < 0) {
        if (dm2_v1_dungeon_set_first_thing(
                dungeon, map, x, y, (uint16_t)record_handle) != 0) {
            if (out_failure_stage) *out_failure_stage = 6;
            goto missile_cast_rollback;
        }
    } else {
        if (!dm2_v1_record_pool_append_to_list(&rt->record_pools, &head,
                                               record_handle)) {
            if (out_failure_stage) *out_failure_stage = 5;
            goto missile_cast_rollback;
        }
        if (dm2_v1_dungeon_set_first_thing(dungeon, map, x, y,
                                           (uint16_t)head) != 0) {
            if (out_failure_stage) *out_failure_stage = 6;
            goto missile_cast_rollback;
        }
    }
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        ((rt->tick_count + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = 0x1eu;
    timer.actor = 0u;
    timer.value_a = record_handle;
    timer.value_b = (int16_t)((x & 0x1f) | ((y & 0x1f) << 5) |
                              ((dir & 3) << 10) | ((timer_step & 0xf) << 12));
    *out_ticket = dm2_v1_source_timer_enqueue_ticketed(
        &rt->timer_queue, &timer, 0u, &result);
    if (result != DM2_V1_SOURCE_TIMER_OK || *out_ticket == 0u) {
        if (out_failure_stage) *out_failure_stage = 7;
        goto missile_cast_rollback;
    }
    /* Newly-created runtime timers use source index zero, matching the
     * queue's source-index contract and the STEP_MISSILE owner check. */
    dm2_runtime_record_wr16(rt, record + 6, 0u);
    if (out_record) *out_record = record_handle;
    if (out_object) *out_object = object;
    if (out_damage) *out_damage = (uint8_t)damage;
    if (out_energy) *out_energy = (uint8_t)kinetic_energy;
    dm2_v1_record_pool_set_free(&pool_before);
    free(raw_before);
    return 1;

missile_cast_rollback:
    if (raw_before) memcpy(dungeon->raw_data, raw_before,
                           (size_t)dungeon->raw_size);
    if (pool_before.valid) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_before;
        memset(&pool_before, 0, sizeof(pool_before));
    }
    rt->timer_queue = queue_before;
    free(raw_before);
    if (out_ticket) *out_ticket = 0u;
    return 0;
}

int dm2_v1_runtime_set_spell_runes(int hero_index,
                                   const uint8_t *runes, int rune_count)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_Hero *hero;
    if (!runes || rune_count < 1 || rune_count > 4 ||
        !rt->source_party_valid ||
        hero_index < 0 || hero_index >= rt->source_party.heros_in_party ||
        hero_index >= DM2_MAX_HEROES) return 0;
    hero = &rt->source_party.hero[hero_index];
    if (hero->curHP <= 0) return 0;
    memset(hero->rune, 0, sizeof(hero->rune));
    memcpy(hero->rune, runes, (size_t)rune_count);
    hero->nrunes = (int8_t)rune_count;
    return 1;
}

int dm2_v1_runtime_cast_spell_player(
    int hero_index, int hand_index,
    DM2_V1_RuntimeSpellCastReceipt *out_receipt)
{
    static const int16_t light_table[16] = {
        0, 5, 12, 24, 33, 40, 46, 51,
        59, 68, 76, 82, 89, 94, 97, 100
    };
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_Hero *hero;
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt extended;
    DM2_V1_RuntimeSpellCastReceipt receipt;
    DM2_V1_RuntimeSpellLightContext light_ctx;
    DM2_V1_ProceedLightCallbacks light_callbacks;
    DM2_V1_Hero hero_before;
    DM2_V1_Party party_before;
    DM2_V1_SourceTimerQueue queue_before;
    int16_t light_before;
    int light_type;
    int enchant_type;
    uint8_t enchant_mask;

    memset(&receipt, 0, sizeof(receipt));
    receipt.hand_index = hand_index;
    if (out_receipt) *out_receipt = receipt;

    if (!rt->boot || !rt->source_party_valid ||
        !rt->boot->source_game_load_session_ready ||
        rt->source_party.heros_in_party <= 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES ||
        hero_index < 0 || hero_index >= rt->source_party.heros_in_party ||
        hand_index < 0 || hand_index > 1 || rt->dungeon_level < 0 ||
        rt->dungeon_level > 0xff) {
        return 0;
    }
    hero = &rt->source_party.hero[hero_index];
    if (hero->curHP <= 0 || hero->nrunes <= 0 ||
        hero->nrunes > 4) {
        return 0;
    }
    memset(&extended, 0, sizeof(extended));
    /* The boot receipt intentionally exposes only the authenticated custom
     * family hash/count, not the raw SPELL_DEF records. Fixed records remain
     * the only admissible runtime table until those records are transferred
     * as a source-owned table rather than reconstructed from a hash. */
    dm2_v1_spell_cast_player_build_table(&extended, &table);
    receipt.cast = dm2_v1_spell_cast_player(
        &table, (const uint8_t *)hero->rune,
        hero->ability[DM2_ABILITY_WIZARDRY][0], hero->curMP, 0);
    receipt.valid = receipt.cast.valid;
    receipt.source_owner_available = 0;
    receipt.mana_before = hero->curMP;
    receipt.mana_after = hero->curMP;
    receipt.light_before = rt->source_light_level;
    receipt.light_after = rt->source_light_level;
    if (!receipt.cast.valid || !receipt.cast.cast_success) {
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (receipt.cast.timer_kind == DM2_V1_SPELL_TIMER_CLOUD &&
        receipt.cast.spell_index == 14) {
        uint32_t cloud_ticket = 0u;
        int cloud_record = -1;
        if (!dm2_runtime_spell_create_cloud(
                rt, receipt.cast.spell_index, receipt.cast.cast_power,
                &cloud_record, &cloud_ticket)) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        hero->curMP = (int16_t)(hero->curMP - receipt.cast.mana_cost);
        hero->handcooldown[hand_index] = (int8_t)receipt.cast.cooldown_ticks;
        memset(hero->rune, 0, sizeof(hero->rune));
        hero->nrunes = 0;
        receipt.source_owner_available = 1;
        receipt.applied = 1;
        receipt.timer_enqueued = 1;
        receipt.timer_ticket = cloud_ticket;
        receipt.mana_after = hero->curMP;
        (void)cloud_record;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (receipt.cast.timer_kind == DM2_V1_SPELL_TIMER_PROJECTILE) {
        uint32_t missile_ticket = 0u;
        int missile_record = -1;
        uint16_t missile_object = 0u;
        uint8_t missile_damage = 0u, missile_energy = 0u;
        if (!dm2_runtime_spell_create_missile(
                rt, &receipt.cast, hero, &missile_record, &missile_object,
                &missile_damage, &missile_energy, &missile_ticket,
                &receipt.missile_failure_stage)) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        hero->curMP = (int16_t)(hero->curMP - receipt.cast.mana_cost);
        hero->handcooldown[hand_index] = (int8_t)receipt.cast.cooldown_ticks;
        memset(hero->rune, 0, sizeof(hero->rune));
        hero->nrunes = 0;
        receipt.source_owner_available = 1;
        receipt.applied = 1;
        receipt.timer_enqueued = 1;
        receipt.timer_ticket = missile_ticket;
        receipt.missile_record = missile_record;
        receipt.missile_object = missile_object;
        receipt.missile_damage = missile_damage;
        receipt.missile_energy = missile_energy;
        receipt.mana_after = hero->curMP;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    if (receipt.cast.timer_kind != DM2_V1_SPELL_TIMER_LIGHT ||
        (receipt.cast.spell_index != 0 && receipt.cast.spell_index != 1 &&
         receipt.cast.spell_index != 5)) {
        /* Fixed shields and the four attribute auras have a source mapping
         * in c_hero::get_adj_ability1 / SkWinCore.cpp:42426/42544. They are
         * handled below; all other successful branches stay fail-closed. */
        if (receipt.cast.spell_index == 11) {
            int32_t next;
            if (!rt->source_aura_of_speed_valid) {
                if (out_receipt) *out_receipt = receipt;
                return 1;
            }
            receipt.aura_of_speed_before = rt->source_aura_of_speed;
            next = (int32_t)rt->source_aura_of_speed +
                ((int32_t)receipt.cast.cast_power << 3);
            if (next > 0xff) next = 0xff;
            rt->source_aura_of_speed = (uint8_t)next;
            hero->curMP = (int16_t)(hero->curMP - receipt.cast.mana_cost);
            hero->handcooldown[hand_index] = (int8_t)receipt.cast.cooldown_ticks;
            memset(hero->rune, 0, sizeof(hero->rune));
            hero->nrunes = 0;
            receipt.aura_of_speed_after = rt->source_aura_of_speed;
            receipt.source_owner_available = 1;
            receipt.applied = 1;
            receipt.timer_enqueued = 0;
            receipt.mana_after = hero->curMP;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if ((receipt.cast.timer_kind != DM2_V1_SPELL_TIMER_ENCHANTMENT &&
             receipt.cast.timer_kind != DM2_V1_SPELL_TIMER_AURA) ||
            (receipt.cast.spell_index != 2 &&
             receipt.cast.spell_index != 4 &&
             receipt.cast.spell_index != 6 &&
             receipt.cast.spell_index != 7 &&
             receipt.cast.spell_index != 8 &&
             receipt.cast.spell_index != 9 &&
             receipt.cast.spell_index != 10)) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (receipt.cast.spell_index == 2) {
            enchant_type = 2; /* ENCHANTMENT_PARTY_SHIELD */
            enchant_mask = (uint8_t)((1u << rt->source_party.heros_in_party) - 1u);
        } else if (receipt.cast.spell_index == 4) {
            enchant_type = 1; /* ENCHANTMENT_SPELL_SHIELD */
            enchant_mask = (uint8_t)(1u << hero_index);
        } else {
            /* c_hero::get_adj_ability1 uses (ench_aura - 2) as the
             * source ability index. DM2 ability 1/2/3/4 are respectively
             * Strength/Dexterity/Wizardry/Vitality. */
            static const int aura_by_spell[11] = {
                0, 0, 0, 0, 0, 0, 5, 4, 0, 6, 3
            };
            enchant_type = aura_by_spell[receipt.cast.spell_index];
            if (receipt.cast.spell_index == 8)
                enchant_type = 0; /* ENCHANTMENT_FIRE_SHIELD */
            enchant_mask = (uint8_t)((1u << rt->source_party.heros_in_party) - 1u);
        }
        for (int i = 0; i < rt->source_party.heros_in_party; ++i) {
            if (rt->source_party.hero[i].curHP <= 0)
                enchant_mask = (uint8_t)(enchant_mask &
                                         (uint8_t)~(1u << i));
        }
        if (enchant_mask == 0u || receipt.cast.timer_value_a <= 0) {
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        party_before = rt->source_party;
        queue_before = rt->timer_queue;
        {
            int replace = 0;
            for (int i = 0; i < rt->source_party.heros_in_party; ++i) {
                if ((enchant_mask & (uint8_t)(1u << i)) != 0u &&
                    rt->source_party.hero[i].ench_aura != enchant_type) {
                    replace = 1;
                    break;
                }
            }
            if (replace)
                dm2_runtime_remove_enchant_timer_mask(
                    &rt->timer_queue, enchant_mask);
            for (int i = 0; i < rt->source_party.heros_in_party; ++i) {
                DM2_V1_Hero *target = &rt->source_party.hero[i];
                int32_t next;
                if ((enchant_mask & (uint8_t)(1u << i)) == 0u ||
                    target->curHP <= 0)
                    continue;
                target->ench_aura = (int8_t)enchant_type;
                next = (int32_t)target->ench_power +
                       receipt.cast.timer_value_a;
                if (next > INT16_MAX) next = INT16_MAX;
                target->ench_power = (int16_t)next;
            }
        }
        {
            DM2_V1_SourceTimer timer;
            DM2_V1_SourceTimerResult result;
            memset(&timer, 0, sizeof(timer));
            timer.ticks_and_map = ((uint32_t)rt->dungeon_level << 24) |
                ((uint32_t)(rt->tick_count +
                    (receipt.cast.timer_duration > 0 ?
                        receipt.cast.timer_duration : 1)) &
                 DM2_V1_SOURCE_TIMER_TICK_MASK);
            timer.type = 0x48;
            timer.actor = enchant_mask;
            timer.value_a = (int16_t)receipt.cast.timer_value_a;
            receipt.timer_ticket = dm2_v1_source_timer_enqueue_ticketed(
                &rt->timer_queue, &timer, 0u, &result);
            if (result != DM2_V1_SOURCE_TIMER_OK ||
                receipt.timer_ticket == 0u) {
                rt->source_party = party_before;
                rt->timer_queue = queue_before;
                if (out_receipt) *out_receipt = receipt;
                return 1;
            }
        }
        hero->curMP = (int16_t)(hero->curMP - receipt.cast.mana_cost);
        hero->handcooldown[hand_index] = (int8_t)receipt.cast.cooldown_ticks;
        memset(hero->rune, 0, sizeof(hero->rune));
        hero->nrunes = 0;
        receipt.source_owner_available = 1;
        receipt.applied = 1;
        receipt.timer_enqueued = 1;
        receipt.mana_after = hero->curMP;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }

    if (receipt.cast.spell_index == 1)
        light_type = 0x06;       /* Darkness */
    else if (receipt.cast.spell_index == 0)
        light_type = 0x27;       /* Long Light */
    else
        light_type = 0x26;       /* Light */

    hero_before = *hero;
    queue_before = rt->timer_queue;
    light_before = rt->source_light_level;
    memset(&light_ctx, 0, sizeof(light_ctx));
    light_ctx.runtime = rt;
    memset(&light_callbacks, 0, sizeof(light_callbacks));
    light_callbacks.global_light = &rt->source_light_level;
    light_callbacks.light_table = light_table;
    light_callbacks.light_table_size = (int)(sizeof(light_table) /
                                              sizeof(light_table[0]));
    light_callbacks.game_tick = (uint32_t)rt->tick_count;
    light_callbacks.queue_light_timer = dm2_runtime_spell_queue_light_timer;
    light_callbacks.recalc_light = dm2_runtime_spell_recalc_light;
    dm2_v1_proceed_light((uint16_t)light_type, receipt.cast.cast_power,
                         &light_callbacks, &light_ctx);
    if (light_ctx.failed) {
        *hero = hero_before;
        rt->timer_queue = queue_before;
        rt->source_light_level = light_before;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }

    hero->curMP = (int16_t)(hero->curMP - receipt.cast.mana_cost);
    hero->handcooldown[hand_index] = (int8_t)receipt.cast.cooldown_ticks;
    memset(hero->rune, 0, sizeof(hero->rune));
    hero->nrunes = 0;
    receipt.source_owner_available = 1;
    receipt.applied = 1;
    receipt.timer_enqueued = 1;
    receipt.timer_ticket = light_ctx.timer_ticket;
    receipt.light_before = light_before;
    receipt.light_after = rt->source_light_level;
    receipt.mana_after = hero->curMP;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

static void dm2_runtime_copy_source_hero_to_snapshot(
    DM2_V1_SessionState *session, int slot, const DM2_V1_Hero *hero)
{
    DM2_ChampionRecord *legacy;
    uint8_t *raw;

    if (!session || !hero || slot < 0 || slot >= DM2_MAX_HEROES) return;
    legacy = (DM2_ChampionRecord *)session->champion_data[slot];
    raw = session->original_champion_records[slot];
    memset(legacy, 0, sizeof(*legacy));
    memset(raw, 0, DM2_V1_ORIGINAL_CHAMPION_RECORD_SIZE);
    memcpy(raw, hero, DM2_V1_ORIGINAL_CHAMPION_RECORD_SIZE);
    memcpy(legacy->first_name, hero->name1, sizeof(legacy->first_name));
    memcpy(legacy->last_name, hero->name2, sizeof(legacy->last_name));
    legacy->absolute_direction = (uint16_t)(uint8_t)hero->absdir;
    legacy->squad_position = (uint8_t)hero->partypos;
    legacy->cur_hp = (uint16_t)hero->curHP;
    legacy->max_hp = (uint16_t)hero->maxHP;
    legacy->stamina = (uint16_t)hero->curStamina;
    legacy->mana = (uint16_t)hero->curMP;
    legacy->poison_value = (uint8_t)hero->poison;
    legacy->runes_count = (uint8_t)hero->nrunes;
    memcpy(legacy->spelled_runes, hero->rune,
           sizeof(legacy->spelled_runes));
    legacy->food = hero->food;
    legacy->water = hero->water;
    legacy->portrait_index = (uint8_t)hero->herotype;
    for (int slot = 0; slot < DM2_NUM_ITEMS; ++slot) {
        legacy->inventory[slot] = hero->item[slot] < 0
            ? 0u : (uint32_t)(uint16_t)hero->item[slot];
    }
}

int dm2_v1_runtime_commit_source_game_load(DM2_V1_BootProfile *boot_profile)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate;
    const DM2_V1_GameLoadWorldOwner *world_owner;
    const DM2_V1_SksaveGameLoadOwner *sksave_source;
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_GameState *game;
    DM2_V1_TimerQueue *source_timers;
    DM2_V1_SourceTimerQueue timer_preflight;
    DM2_V1_SoundQueueState sound_preflight;
    int active_timers = 0;
    int i;

    if (!boot_profile || rt->boot != boot_profile ||
        boot_profile->source_game_load_session_ready ||
        !boot_profile->dm2_state ||
        !boot_profile->game_load_runtime_session_candidate ||
        (!boot_profile->game_load_world_owner &&
         !boot_profile->sksave_game_load_source)) {
        return 0;
    }
    candidate = (DM2_V1_GameLoadRuntimeSessionCandidate *)
        boot_profile->game_load_runtime_session_candidate;
    world_owner = (const DM2_V1_GameLoadWorldOwner *)
        boot_profile->game_load_world_owner;
    sksave_source = (const DM2_V1_SksaveGameLoadOwner *)
        boot_profile->sksave_game_load_source;
    /* GAME_LOAD stages the candidate from this exact private world owner.
     * Do not publish a candidate merely because its local fields look
     * complete: a stale or cross-transaction clone must fail before any
     * runtime allocation is released or transferred. */
    if ((!world_owner && !sksave_source) ||
        (world_owner && (!world_owner->prepared || world_owner->committed ||
                         world_owner->source_transaction_hash == 0u)) ||
        (sksave_source && (!sksave_source->valid ||
                           sksave_source->source_game_load_session_ready ||
                           !sksave_source->asset_loader ||
                           sksave_source->asset_loader != candidate->asset_loader ||
                           !sksave_source->source_savegames1_valid ||
                           !candidate->source_savegames1_valid ||
                           memcmp(sksave_source->source_savegames1,
                                  candidate->source_savegames1,
                                  sizeof(candidate->source_savegames1)) != 0 ||
                           (sksave_source->state.fixed_sections_hash ^
                            sksave_source->state.timers_hash ^
                            sksave_source->state.dungeon.prefix_hash) !=
                               candidate->source_transaction_hash)) ||
        (world_owner && candidate->source_transaction_hash !=
            world_owner->source_transaction_hash) ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate) ||
        !candidate->valid || candidate->source_party_map < 0 ||
        candidate->source_party_x > 63u || candidate->source_party_y > 63u ||
        candidate->source_party_direction > 3u ||
        candidate->party.heros_in_party <= 0 ||
        candidate->party.heros_in_party > DM2_MAX_HEROES ||
        !candidate->record_pools.valid ||
        !candidate->record_pools.record_graph_complete ||
        !candidate->caii_slots.valid || !candidate->caii_slots.slots ||
        !candidate->sound_owner.valid ||
        !candidate->sound_owner.runtime_queue_initialized ||
        !candidate->sound_owner.queue_entries ||
        candidate->sound_owner.queue_capacity == 0u ||
        !candidate->timer_queue.entries || !candidate->timer_queue.indices ||
        candidate->timer_queue.max_timers <= 0) {
        return 0;
    }

    /* The current runtime dispatcher has a bounded source queue.  Count the
     * candidate's actual live entries before moving any ownership; no timer
     * may be silently dropped at the handoff boundary. */
    source_timers = &candidate->timer_queue;
    for (i = 0; i < source_timers->max_timers; ++i) {
        if (source_timers->entries[i].ttype != 0u) ++active_timers;
    }
    if (active_timers > (int)DM2_V1_SOURCE_TIMER_MAX) {
        return 0;
    }

    /* Prove every fallible conversion while the boot profile still owns all
     * candidate state.  The commit below only copies these already-admitted
     * queues, so a later capacity/binding failure cannot strand a half-moved
     * record pool or CAII owner. */
    dm2_v1_source_timer_queue_init(&timer_preflight);
    for (i = 0; i < source_timers->max_timers; ++i) {
        const DM2_V1_TimerEntry *src = &source_timers->entries[i];
        DM2_V1_SourceTimer timer;
        if (src->ttype == 0u) continue;
        memset(&timer, 0, sizeof(timer));
        timer.ticks_and_map = (uint32_t)src->l_00;
        timer.type = src->ttype;
        timer.actor = src->actor;
        timer.value_a = (int16_t)((uint8_t)src->xA |
                                  ((uint16_t)(uint8_t)src->yA << 8));
        timer.value_b = src->wvalueB;
        timer.reserved = src->dummya;
        if (dm2_v1_source_timer_enqueue(&timer_preflight, &timer,
                                        (uint16_t)i) != DM2_V1_SOURCE_TIMER_OK) {
            return 0;
        }
    }
    dm2_v1_sound_queue_state_init(&sound_preflight,
                                  candidate->sound_owner.queue_capacity);
    if (!dm2_v1_sound_queue_bind_entries(
        &sound_preflight, candidate->sound_owner.queue_entries,
        candidate->sound_owner.queue_entry_count,
        candidate->sound_owner.queue_capacity)) {
        return 0;
    }

    /* All validation above is read-only.  From this point the candidate's
     * owned allocations are transferred into the singleton and zeroed in the
     * staging object so boot cleanup cannot free them twice. */
    if (rt->record_pools_valid) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools_valid = 0;
    }
    if (rt->caii_ready) {
        dm2_v1_caii_array_free(&rt->caii);
        rt->caii_ready = 0;
    }
    free(rt->source_sound_entries);
    free(rt->source_sound_bindings);
    rt->source_sound_entries = NULL;
    rt->source_sound_bindings = NULL;
    rt->source_sound_binding_count = 0u;

    rt->record_pools = candidate->record_pools;
    memset(&candidate->record_pools, 0, sizeof(candidate->record_pools));
    rt->record_pools_valid = 1;
    rt->caii = candidate->caii_slots;
    memset(&candidate->caii_slots, 0, sizeof(candidate->caii_slots));
    rt->caii_ready = 1;

    rt->timer_queue = timer_preflight;

    rt->source_sound_entries = candidate->sound_owner.queue_entries;
    candidate->sound_owner.queue_entries = NULL;
    rt->source_sound_bindings = candidate->sound_owner.sample_bindings;
    candidate->sound_owner.sample_bindings = NULL;
    rt->source_sound_binding_count =
        candidate->sound_owner.sample_binding_count;
    rt->sound_queue = sound_preflight;
    rt->sound_queue.ssound_count = candidate->sound_owner.queue_entry_count;
    rt->sound_queue.sample_binding_count =
        candidate->sound_owner.sample_binding_count;
    rt->sound_queue_ready = 1;
    dm2_v1_sound_bind_runtime_queue(&rt->sound_queue);
    memset(&rt->sound_env, 0, sizeof(rt->sound_env));
    rt->sound_env.current_map = candidate->sound_owner.spatial_current_map;
    rt->sound_env.gate_map_a = candidate->sound_owner.spatial_audible_map;
    rt->sound_env.gate_map_b = candidate->sound_owner.spatial_alternate_map;
    rt->sound_env.facing = candidate->source_party_direction;
    rt->sound_env.party_x = candidate->source_party_x;
    rt->sound_env.party_y = candidate->source_party_y;

    game = (DM2_V1_GameState *)boot_profile->dm2_state;
    game->party_x = candidate->source_party_x;
    game->party_y = candidate->source_party_y;
    game->party_dir = candidate->source_party_direction;
    game->current_level = candidate->source_party_map;
    game->outdoor = candidate->source_party_map >= 0 &&
        dm2_v1_dungeon_is_outdoor((const DM2_V1_DungeonData *)
                                  boot_profile->dungeon_data,
                                  candidate->source_party_map);
    rt->dungeon_level = game->current_level;
    rt->view_dir = game->party_dir;
    rt->outdoor = game->outdoor;
    /* GAME_LOAD changes the active map after the boot-time runtime bind.
     * Rebuild the source-owned map/GDAT/light/material context now; otherwise
     * the first M11 frame keeps the pre-party context and is correctly
     * rejected with no floor, wall or frame receipt. */
    dm2_runtime_refresh_map_transition_context(rt);

    memset(&rt->session_snapshot, 0, sizeof(rt->session_snapshot));
    rt->source_party = candidate->party;
    rt->source_party_valid = 1;
    rt->source_next_champion_number = candidate->source_next_champion_number;
    rt->source_hero_ench_countdown = candidate->source_savegames1_valid &&
        candidate->source_savegames1[2] != 0u
        ? candidate->source_savegames1[2]
        : candidate->source_hero_ench_countdown;
    rt->source_hero_ench_target = candidate->source_hero_ench_target;
    memcpy(rt->source_savegames1, candidate->source_savegames1,
           sizeof(rt->source_savegames1));
    /* For a fresh GAME_LOAD this block was initialized from source-zero.
     * For SKSAVE Resume the preflight above proved byte identity with the
     * immutable source snapshot before any ownership moved; this copy is the
     * single writable runtime global-state owner. */
    rt->source_aura_of_speed = candidate->source_savegames1_valid
        ? candidate->source_savegames1[4] : 0u;
    rt->source_aura_of_speed_valid = candidate->source_savegames1_valid;
    rt->source_light_level = candidate->source_light_level;
    rt->source_attack_counter = rt->source_hero_ench_countdown;
    rt->source_savegames1[2] = rt->source_attack_counter;
    rt->session_snapshot.champion_count =
        (uint8_t)candidate->party.heros_in_party;
    rt->session_snapshot.leader_index = candidate->party.curactevhero >= 0 &&
        candidate->party.curactevhero < candidate->party.heros_in_party
        ? (uint8_t)candidate->party.curactevhero : 0u;
    rt->session_snapshot.party_x = candidate->source_party_x;
    rt->session_snapshot.party_y = candidate->source_party_y;
    rt->session_snapshot.party_dir = candidate->source_party_direction;
    rt->session_snapshot.party_level = (uint8_t)candidate->source_party_map;
    for (i = 0; i < candidate->party.heros_in_party; ++i) {
        dm2_runtime_copy_source_hero_to_snapshot(
            &rt->session_snapshot, i, &candidate->party.hero[i]);
    }
    rt->session_snapshot.original_champion_records_valid = 1u;
    rt->source_curacthero = candidate->party.curacthero;
    rt->source_curactmode = candidate->party.curactmode;
    rt->source_event_hero_index = candidate->source_event_hero_index;
    rt->source_v1e0976 = 0;
    rt->session_snapshot_valid = 1;
    rt->leader_hand_object = (uint32_t)(uint16_t)
        candidate->leader_hand_record;

    boot_profile->source_game_load_session_ready = 1;
    return 1;
}

int dm2_v1_runtime_handoff_game_load_candidate(
    DM2_V1_BootProfile *boot_profile,
    DM2_V1_RuntimeGameLoadCandidateHandoffReceipt *out_receipt)
{
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate;
    const DM2_V1_GameLoadWorldOwner *world_owner;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!boot_profile || g_dm2_runtime.boot != boot_profile ||
        boot_profile->source_game_load_session_ready ||
        g_dm2_runtime.game_load_candidate ||
        !boot_profile->game_load_runtime_session_candidate) {
        return 0;
    }
    candidate = (DM2_V1_GameLoadRuntimeSessionCandidate *)
        boot_profile->game_load_runtime_session_candidate;
    world_owner = (const DM2_V1_GameLoadWorldOwner *)
        boot_profile->game_load_world_owner;
    if (!world_owner || !world_owner->prepared || world_owner->committed ||
        world_owner->source_transaction_hash == 0u ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate) ||
        candidate->source_transaction_hash !=
            world_owner->source_transaction_hash) {
        return 0;
    }
    g_dm2_runtime.game_load_candidate = candidate;
    g_dm2_runtime.game_load_candidate_hash = candidate->candidate_hash;
    g_dm2_runtime.game_load_candidate_source_hash =
        candidate->source_transaction_hash;
    boot_profile->game_load_runtime_session_candidate = NULL;
    if (out_receipt) {
        out_receipt->valid = 1;
        out_receipt->source_game_load_session_ready =
            boot_profile->source_game_load_session_ready;
        out_receipt->candidate_hash = candidate->candidate_hash;
        out_receipt->source_transaction_hash =
            candidate->source_transaction_hash;
    }
    return 1;
}

int dm2_v1_runtime_game_load_candidate_view(
    DM2_V1_RuntimeGameLoadCandidateViewReceipt *out_receipt)
{
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate =
        g_dm2_runtime.game_load_candidate;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt || !candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(candidate)) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->source_game_load_session_ready =
        g_dm2_runtime.boot ? g_dm2_runtime.boot->source_game_load_session_ready : 0;
    out_receipt->candidate_hash = candidate->candidate_hash;
    out_receipt->source_transaction_hash = candidate->source_transaction_hash;
    out_receipt->current_map = candidate->current_map;
    out_receipt->party_count = candidate->party.heros_in_party;
    out_receipt->active_hero = candidate->party.curactevhero;
    out_receipt->record_graph_complete =
        candidate->record_pools.record_graph_complete;
    out_receipt->caii_capacity = candidate->caii_slots.capacity;
    out_receipt->caii_alloc_count = candidate->caii_slots.alloc_count;
    out_receipt->timer_capacity = candidate->timer_capacity;
    out_receipt->timer_count = candidate->timer_queue.num_timers > 0 ?
        (uint16_t)candidate->timer_queue.num_timers : 0u;
    if (candidate->timer_queue.num_timers > 0 &&
        candidate->timer_queue.indices && candidate->timer_queue.entries) {
        int16_t slot = candidate->timer_queue.indices[0];
        if (slot >= 0 && slot < candidate->timer_capacity) {
            const DM2_V1_TimerEntry *timer = &candidate->timer_entries[slot];
            out_receipt->first_timer_valid = timer->ttype != 0u;
            out_receipt->first_timer_type = timer->ttype;
            out_receipt->first_timer_actor = timer->actor;
            out_receipt->first_timer_value_a = (int16_t)(
                (uint8_t)timer->xA |
                ((uint16_t)(uint8_t)timer->yA << 8));
            out_receipt->first_timer_value_b = timer->wvalueB;
            out_receipt->first_timer_reserved = timer->dummya;
        }
    }
    out_receipt->sound_queue_entry_count = candidate->sound_owner.queue_entry_count;
    out_receipt->sound_sample_binding_count =
        candidate->sound_owner.sample_binding_count;
    return 1;
}

int dm2_v1_runtime_game_load_candidate_query_nearest_creature(
    int16_t *io_x, int16_t *io_y, uint16_t direction,
    uint32_t *out_handle, DM2_V1_GameLoadSpatialQueryReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_query_nearest_creature(
        g_dm2_runtime.game_load_candidate, io_x, io_y, direction,
        out_handle, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_classify_move(
    uint8_t move_command, int16_t source_x, int16_t source_y,
    int16_t target_x, int16_t target_y,
    DM2_V1_GameLoadMoveClassificationReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_classify_move(
        g_dm2_runtime.game_load_candidate, move_command, source_x, source_y,
        target_x, target_y, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_census_moverec_square(
    int16_t x, int16_t y, DM2_V1_GameLoadMoverecSquareReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_census_moverec_square(
        g_dm2_runtime.game_load_candidate, x, y, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_move_record_to(
    int16_t record, int16_t source_x, int16_t source_y,
    int16_t destination_x, int16_t destination_y,
    DM2_V1_GameLoadRecordMoveReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_move_record_to(
        g_dm2_runtime.game_load_candidate, record, source_x, source_y,
        destination_x, destination_y, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_dispatch_moverec(
    int16_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    DM2_V1_GameLoadMoverecDispatchReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_dispatch_moverec(
        g_dm2_runtime.game_load_candidate, record, x, y, kind, flags,
        out_receipt);
}

int dm2_v1_runtime_game_load_candidate_activate_moverec_caii(
    int16_t record, int16_t x, int16_t y,
    DM2_V1_GameLoadMoverecCaiiReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
        g_dm2_runtime.game_load_candidate, record, x, y, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_proceed_think_timer(
    uint32_t game_tick, DM2_V1_GameLoadThinkTimerReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_proceed_think_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_proceed_actuate_timer(
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateActuateReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_proceed_actuate_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_process_next_due_timer(
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTimerProcessReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_process_next_due_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_proceed_door_step_timer(
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateDoorStepReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_proceed_door_step_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_proceed_tick_generator_timer(
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTickGeneratorReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_proceed_tick_generator_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_proceed_record_flag_timer(
    uint32_t game_tick,
    DM2_V1_GameLoadRecordFlagTimerReceipt *out_receipt)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_proceed_record_flag_timer(
        g_dm2_runtime.game_load_candidate, game_tick, out_receipt);
}

int dm2_v1_runtime_game_load_candidate_change_current_map(int new_map)
{
    if (!g_dm2_runtime.game_load_candidate ||
        !dm2_v1_game_load_runtime_session_candidate_is_valid(
            g_dm2_runtime.game_load_candidate)) {
        return 0;
    }
    return dm2_v1_game_load_runtime_session_candidate_change_current_map_to(
        g_dm2_runtime.game_load_candidate, new_map);
}

int dm2_v1_runtime_bind_boot_profile_with_receipt(
    DM2_V1_BootProfile *boot_profile,
    DM2_V1_StartupHostReceipt *out_receipt)
{
    if (out_receipt) {
        dm2_v1_startup_host_receipt_clear(out_receipt);
        out_receipt->status_scope = "BOOT";
        /* Binding has no identified original GUI/status producer.  Keep the
         * structural return value for the caller but never manufacture an
         * English M11 receipt. */
        out_receipt->status = NULL;
    }
    if (!dm2_v1_runtime_bind_boot_profile(boot_profile)) {
        return 0;
    }
    if (out_receipt) {
        dm2_v1_startup_host_receipt_clear(out_receipt);
        out_receipt->status_scope = "BOOT";
        out_receipt->status = NULL;
    }
    return 1;
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
    dst->source_info_slot = material->info_slot;
    dst->source_animation_sequence = material->animation_sequence;
    dst->source_animation_info = material->animation_info;
    dst->source_animation_0958_valid = material->animation_0958_valid;
    dst->source_animation_0958_frame_bit14 =
        material->animation_0958_frame_bit14;
    dst->source_animation_0958_blocked_caii =
        material->animation_0958_blocked_caii;
    dst->source_animation_0958_query_index =
        material->animation_0958_query_index;
    dst->source_animation_0958_blended_value =
        material->animation_0958_blended_value;
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
    g_dm2_last_creature_render.source_info_slot = dst->source_info_slot;
    g_dm2_last_creature_render.source_animation_sequence =
        dst->source_animation_sequence;
    g_dm2_last_creature_render.source_animation_info =
        dst->source_animation_info;
    g_dm2_last_creature_render.source_animation_0958_valid =
        dst->source_animation_0958_valid;
    g_dm2_last_creature_render.source_animation_0958_frame_bit14 =
        dst->source_animation_0958_frame_bit14;
    g_dm2_last_creature_render.source_animation_0958_blocked_caii =
        dst->source_animation_0958_blocked_caii;
    g_dm2_last_creature_render.source_animation_0958_query_index =
        dst->source_animation_0958_query_index;
    g_dm2_last_creature_render.source_animation_0958_blended_value =
        dst->source_animation_0958_blended_value;
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
        /* SKWINSPX/src/v5/skgdtqdb.cpp:2978-2990 obtains iAnimSeq and
         * iAnimInfo through query_1c9a_02c3, while the V5 FB/FC/FD route is
         * advanced by the live CAII command in c_ai.cpp:5606.  This map
         * receipt owns only the DB4 record and its cursor words; it has no
         * CAII command owner yet.  Do not replay a V5 image with command 0
         * and frame 0xffff, which would be a fabricated animation state. */
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

static const DM2_V1_StaticObjectSourcePlan *
dm2_runtime_g1_static_object_source_plan_for_object(
    const DM2_V1_RuntimeState *rt, uint16_t object_id)
{
    int k;

    if (!rt || !object_id) return NULL;
    for (k = 0; k < rt->g1_static_object_material_count && k < 48; ++k) {
        const DM2_V1_G1StaticObjectMaterialReceipt *material =
            &rt->g1_static_object_materials[k];
        const DM2_V1_StaticObjectSourcePlan *plan =
            &rt->g1_static_object_source_plans[k];
        if (material->selector.valid &&
            material->selector.object_id == object_id &&
            plan->source_cell >= 0 && plan->source_pass >= 0) {
            return plan;
        }
    }
    return NULL;
}

static void dm2_runtime_admit_static_object_draw_item_material(
    const DM2_V1_RuntimeState *rt, DM2_ItemSprite *dst)
{
    const DM2_V1_G1StaticObjectMaterialReceipt *material;
    const DM2_V1_StaticObjectSourcePlan *plan;

    if (!rt || !dst) return;
    material = dm2_runtime_g1_static_object_material_for_object(
        rt, dst->object_id);
    plan = dm2_runtime_g1_static_object_source_plan_for_object(
        rt, dst->object_id);
    if (!material || !plan) return;
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
    /* DRAW_ITEM's optional Rect14 lookup is part of the same source plan as
     * this cell/pass/clip proof. Do not reinterpret a host frame index here.
     * If the GDAT field selectors disagree, retain the independently proven
     * non-Rect14 route rather than joining unrelated original records. */
    if (plan->rect14_applied && plan->rect14_scale64 > 0 &&
        plan->rect14_row_hash != 0u && plan->rect14_placement_hash != 0u &&
        plan->rect14_image_field == material->selector.image_field) {
        dst->source_static_object_rect14_applied = 1u;
        dst->source_static_object_rect14_scale64 =
            (int16_t)plan->rect14_scale64;
        dst->source_static_object_rect14_lateral_offset =
            (int16_t)plan->rect14_lateral_offset;
        dst->source_static_object_rect14_flip_mirror =
            (uint8_t)plan->rect14_flip_mirror;
        dst->source_static_object_rect14_row_hash = plan->rect14_row_hash;
        dst->source_static_object_rect14_placement_hash =
            plan->rect14_placement_hash;
    }
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
typedef struct {
    DM2_V1_RuntimeState *runtime;
    int16_t record_handle;
    int map;
    int x;
    int y;
    unsigned long game_tick;
} DM2_RuntimeCcmGoalContext;

static int32_t dm2_runtime_ccm_proceed_walking(
    void *context, uint8_t command, uint16_t creature_type,
    uint8_t *slot, uint16_t adj0, uint16_t adj1,
    unsigned long game_tick);

/* c_1c9a.cpp FIND_WALK_PATH adapters.  These callbacks deliberately expose
 * only the currently committed GAME_LOAD map and party owners.  Tile
 * admission is delegated to the dungeon-loader's editions-aware
 * source-symbol adapters rather than interpreting raw map words here. */
static DM2_V1_DungeonData *dm2_runtime_goal_dungeon(
    DM2_RuntimeCcmGoalContext *goal)
{
    if (!goal || !goal->runtime || !goal->runtime->boot)
        return NULL;
    return (DM2_V1_DungeonData *)goal->runtime->boot->dungeon_data;
}

static int16_t dm2_runtime_1c9a_map_width(void *user)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)user;
    DM2_V1_DungeonData *dungeon = dm2_runtime_goal_dungeon(goal);
    return dungeon && goal && goal->map >= 0 &&
        goal->map < dungeon->level_count ?
        (int16_t)dungeon->level_widths[goal->map] : 0;
}

static int16_t dm2_runtime_1c9a_map_height(void *user)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)user;
    DM2_V1_DungeonData *dungeon = dm2_runtime_goal_dungeon(goal);
    return dungeon && goal && goal->map >= 0 &&
        goal->map < dungeon->level_count ?
        (int16_t)dungeon->level_heights[goal->map] : 0;
}

static int16_t dm2_runtime_1c9a_creature_at(void *user, int16_t x, int16_t y)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)user;
    DM2_V1_DungeonData *dungeon = dm2_runtime_goal_dungeon(goal);
    if (!goal || !goal->runtime || !dungeon ||
        !goal->runtime->record_pools_valid)
        return -1;
    return dm2_v1_get_creature_at(&goal->runtime->record_pools, dungeon,
                                  goal->map, x, y);
}

static int dm2_runtime_source_path_passable(
    DM2_RuntimeCcmGoalContext *goal, int x, int y)
{
    DM2_V1_DungeonData *dungeon = dm2_runtime_goal_dungeon(goal);
    DM2_V1_SkprojectTilePassageReceipt passage;
    DM2_V1_SkprojectTileSolidReceipt solid;
    int raw;
    int square;

    if (!dungeon || !goal ||
        (raw = dm2_v1_dungeon_get_tile_raw(dungeon, goal->map, x, y)) < 0)
        return 0;
    /* The byte-map owner has an authenticated square-type normalizer which
     * preserves DOS/Amiga floor and ornate-floor encodings.  The loader's
     * passage/solid receipts are the corresponding owner for 16-bit maps. */
    if (dungeon->square_bytes == 1) {
        square = dm2_runtime_normalize_square_type_for_dungeon(
            dungeon, dm2_runtime_square_type_at(dungeon, goal->map, x, y,
                                                raw), raw);
        return square == DM2_SQUARE_FLOOR ||
               square == DM2_SQUARE_FLOOR_ORNATE;
    }
    if (
        !dm2_v1_skproject_is_tile_passage(dungeon, goal->map, x, y,
                                          &passage) ||
        !dm2_v1_skproject_is_tile_solid(dungeon, goal->map, x, y, &solid))
        return 0;
    return passage.is_passage != 0 && solid.is_solid == 0;
}

static int dm2_runtime_source_find_walk_path(
    DM2_RuntimeCcmGoalContext *goal, int start_x, int start_y,
    uint8_t *path, int path_capacity)
{
    DM2_V1_GameState *game;
    int width;
    int height;
    int target = -1;
    uint8_t target_cells[32u * 32u];
    uint16_t queue[32u * 32u];
    int16_t parent[32u * 32u];
    uint8_t direction[32u * 32u];
    uint8_t seen[32u * 32u];
    int head = 0;
    int tail = 0;
    int current;
    int i;
    static const int dx[4] = {0, 1, 0, -1};
    static const int dy[4] = {-1, 0, 1, 0};

    if (!goal || !goal->runtime || !goal->runtime->boot || !path ||
        path_capacity <= 0) {
        if (goal && goal->runtime)
            goal->runtime->dynamic_path_last_failure = 1;
        return -1;
    }
    game = (DM2_V1_GameState *)goal->runtime->boot->dm2_state;
    width = dm2_runtime_1c9a_map_width(goal);
    height = dm2_runtime_1c9a_map_height(goal);
    if (!game || width <= 0 || width > 32 || height <= 0 || height > 32 ||
        start_x < 0 || start_y < 0 ||
        start_x >= width || start_y >= height || game->party_x < 0 ||
        game->party_y < 0 || game->party_x >= width ||
        game->party_y >= height) {
        goal->runtime->dynamic_path_last_failure = 2;
        return -1;
    }
    memset(target_cells, 0, sizeof(target_cells));
    /* CREATURE_GO_THERE approaches the party; it does not path onto the
     * party's occupied square.  The source attack transition is selected
     * once the creature is on an adjacent square. */
    for (i = 0; i < 4; ++i) {
        int x = game->party_x + dx[i];
        int y = game->party_y + dy[i];
        int candidate;
        if (x < 0 || y < 0 || x >= width || y >= height ||
            !dm2_runtime_source_path_passable(goal, x, y))
            continue;
        candidate = y * width + x;
        if (candidate != start_y * width + start_x &&
            dm2_runtime_1c9a_creature_at(
                goal, (int16_t)x, (int16_t)y) >= 0)
            continue;
        target_cells[candidate] = 1u;
        if (target < 0)
            target = candidate;
    }
    if (target < 0) {
        goal->runtime->dynamic_path_last_failure = 3;
        return -1;
    }
    current = start_y * width + start_x;
    if (target_cells[current])
        return 0;
    memset(parent, 0xff, sizeof(parent));
    memset(direction, 0, sizeof(direction));
    memset(seen, 0, sizeof(seen));
    seen[current] = 1u;
    queue[tail++] = (uint16_t)current;
    while (head < tail) {
        current = queue[head++];
        for (i = 0; i < 4; ++i) {
            int x = (current % width) + dx[i];
            int y = (current / width) + dy[i];
            int next;
            if (x < 0 || y < 0 || x >= width || y >= height)
                continue;
            next = y * width + x;
            if (seen[next] || !dm2_runtime_source_path_passable(goal, x, y))
                continue;
            if (!target_cells[next] && dm2_runtime_1c9a_creature_at(
                    goal, (int16_t)x, (int16_t)y) >= 0)
                continue;
            seen[next] = 1u;
            parent[next] = (int16_t)current;
            direction[next] = (uint8_t)i;
            if (target_cells[next]) {
                target = next;
                head = tail;
                break;
            }
            if (tail < (int)(sizeof(queue) / sizeof(queue[0])))
                queue[tail++] = (uint16_t)next;
        }
    }
    if (target < 0 || !seen[target]) {
        goal->runtime->dynamic_path_last_failure = 3;
        return -1;
    }
    current = target;
    i = 0;
    while (current != start_y * width + start_x) {
        if (i >= path_capacity || parent[current] < 0) {
            goal->runtime->dynamic_path_last_failure = 4;
            return -1;
        }
        path[i++] = direction[current];
        current = parent[current];
    }
    for (int left = 0; left < i / 2; ++left) {
        uint8_t swap = path[left];
        path[left] = path[i - 1 - left];
        path[i - 1 - left] = swap;
    }
    goal->runtime->dynamic_path_last_failure = 0;
    return i;
}

static int16_t dm2_runtime_ccm_abs16(int16_t value)
{
    return value < 0 ? (int16_t)-value : value;
}

static int16_t dm2_runtime_ccm_vector_dir(void *context,
    int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    (void)context;
    if (x2 > x1) return 1;
    if (x2 < x1) return 3;
    if (y2 > y1) return 2;
    return 0;
}

static int16_t dm2_runtime_ccm_rand16(void *context, int16_t max)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    if (!goal || !goal->runtime || max <= 0) return 0;
    return (int16_t)dm2_v1_drops_rand16(&goal->runtime->drop_rng,
                                        (uint16_t)max);
}

static int16_t dm2_runtime_ccm_randdir(void *context)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    return goal && goal->runtime
        ? (int16_t)dm2_v1_drops_randdir(&goal->runtime->drop_rng) : 0;
}

static int dm2_runtime_ccm_randbit(void *context)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    return goal && goal->runtime
        ? (int)dm2_v1_drops_randbit(&goal->runtime->drop_rng) : 0;
}

static int16_t dm2_runtime_ccm_rand_full(void *context)
{
    return dm2_runtime_ccm_rand16(context, 0x7fffu);
}

static int16_t dm2_runtime_ccm_min16(int16_t a, int16_t b)
{
    return a < b ? a : b;
}

static int16_t dm2_runtime_ccm_get_player_at_position(void *context,
    uint8_t party_position)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_Party *party;
    if (!goal || !goal->runtime || !goal->runtime->source_party_valid)
        return -1;
    party = &goal->runtime->source_party;
    for (int i = 0; i < party->heros_in_party && i < DM2_MAX_HEROES; ++i) {
        if (party->hero[i].partypos == (int8_t)party_position &&
            party->hero[i].curHP > 0)
            return (int16_t)i;
    }
    return -1;
}

static int16_t dm2_runtime_ccm_find_hero_at(void *context,
    uint16_t x, uint16_t y, int16_t filter)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_GameState *game;
    if (!goal || !goal->runtime || !goal->runtime->source_party_valid ||
        filter != 0xff)
        return -1;
    game = goal->runtime->boot && goal->runtime->boot->dm2_state
        ? (DM2_V1_GameState *)goal->runtime->boot->dm2_state : NULL;
    if (!game || x != (uint16_t)game->party_x || y != (uint16_t)game->party_y)
        return -1;
    for (int i = 0; i < goal->runtime->source_party.heros_in_party &&
                    i < DM2_MAX_HEROES; ++i) {
        if (goal->runtime->source_party.hero[i].curHP > 0)
            return (int16_t)i;
    }
    return -1;
}

static int16_t dm2_runtime_ccm_wound_player(void *context, int16_t hero_idx,
    int16_t damage, int16_t wound_type, int16_t attack_type)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_Hero *hero;
    int32_t pending;
    (void)wound_type;
    (void)attack_type;
    if (!goal || !goal->runtime || !goal->runtime->source_party_valid ||
        hero_idx < 0 || hero_idx >= goal->runtime->source_party.heros_in_party ||
        hero_idx >= DM2_MAX_HEROES || damage <= 0)
        return 0;
    hero = &goal->runtime->source_party.hero[hero_idx];
    if (hero->curHP <= 0) return 0;
    pending = (int32_t)hero->damagesuffered + damage;
    if (pending > INT16_MAX) pending = INT16_MAX;
    hero->damagesuffered = (int16_t)pending;
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag |
                               DM2_V1_HERO_FLAG_0800);
    return damage;
}

static const uint8_t *dm2_runtime_ccm_ai_spec(void *context,
    uint8_t creature_type)
{
    const DM2_AIDefinition *spec = NULL;
    (void)context;
    return dm2_v1_creature_ai_spec_def((int)creature_type, &spec) && spec
        ? (const uint8_t *)spec : NULL;
}

static int16_t dm2_runtime_ccm_creature_attacks_player(void *context,
    uint16_t creature_record, int16_t hero_idx)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *game;
    DM2_V1_CreatureAttacksPlayerState state;
    DM2_V1_CreatureAttacksPlayerReceiptCallbacks callbacks;
    DM2_V1_CreatureAttacksPlayerReceipt receipt;
    const uint8_t *record;
    if (!goal || !goal->runtime || !goal->runtime->source_party_valid ||
        !goal->runtime->boot || !goal->runtime->boot->dm2_state ||
        hero_idx < 0 || hero_idx >= goal->runtime->source_party.heros_in_party)
        return 0;
    dungeon = (DM2_V1_DungeonData *)goal->runtime->boot->dungeon_data;
    game = (DM2_V1_GameState *)goal->runtime->boot->dm2_state;
    record = dm2_v1_record_pool_address(&goal->runtime->record_pools,
                                        (int16_t)creature_record);
    if (!dungeon || !record) return 0;
    memset(&state, 0, sizeof(state));
    state.hero_idx = hero_idx;
    state.heros_in_party = goal->runtime->source_party.heros_in_party;
    state.hero_cur_hp = goal->runtime->source_party.hero[hero_idx].curHP;
    state.hero_type = (uint8_t)goal->runtime->source_party.hero[hero_idx].herotype;
    state.creature_record = record;
    state.creature_data = record;
    state.party_x = (int16_t)game->party_x;
    state.party_y = (int16_t)game->party_y;
    state.v1e0238 = (int16_t)goal->runtime->source_sleeping;
    state.savegames1_b02 = goal->runtime->source_savegames1[2];
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.rand_fn = dm2_runtime_ccm_rand_full;
    callbacks.rand16 = dm2_runtime_ccm_rand16;
    callbacks.randdir = dm2_runtime_ccm_randdir;
    callbacks.randbit = dm2_runtime_ccm_randbit;
    callbacks.min_fn = dm2_runtime_ccm_min16;
    callbacks.query_ai_spec = dm2_runtime_ccm_ai_spec;
    callbacks.wound_player = dm2_runtime_ccm_wound_player;
    memset(&receipt, 0, sizeof(receipt));
    (void)dm2_v1_creature_attacks_player_receipt(
        &state, &callbacks, goal, &receipt);
    return receipt.valid && !receipt.fail_closed
        ? receipt.damage_dealt : 0;
}

static int32_t dm2_runtime_ccm_attack_party(
    DM2_RuntimeCcmGoalContext *goal, uint8_t command, uint16_t creature_type)
{
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *game;
    const uint8_t *record;
    DM2_V1_CreatureAttacksPartyState state;
    DM2_V1_CreatureAttacksPartyCallbacks callbacks;
    DM2_V1_CreatureAttacksPartyReceipt receipt;
    int distance;
    if (!goal || !goal->runtime || (command != 0x08u && command != 0x26u) ||
        !goal->runtime->source_party_valid || !goal->runtime->boot ||
        !goal->runtime->boot->dm2_state)
        return 0;
    dungeon = (DM2_V1_DungeonData *)goal->runtime->boot->dungeon_data;
    game = (DM2_V1_GameState *)goal->runtime->boot->dm2_state;
    record = dm2_v1_record_pool_address(&goal->runtime->record_pools,
                                        goal->record_handle);
    if (!dungeon || !record || goal->map != game->current_level) return 0;
    distance = abs(goal->x - game->party_x) + abs(goal->y - game->party_y);
    if (!dm2_v1_creature_attacks_party((int)creature_type, distance)) return 0;
    memset(&state, 0, sizeof(state));
    state.spx_word_0e = (uint16_t)record[0x0e] |
        ((uint16_t)record[0x0f] << 8);
    state.creature_x = (int16_t)goal->x;
    state.creature_y = (int16_t)goal->y;
    state.creature_b_20 = record[0x20];
    state.creature_b_1a = command;
    state.creature_b_1c = record[0x1c];
    state.creature_record = (uint16_t)goal->record_handle;
    state.ai_spec = dm2_runtime_ccm_ai_spec(NULL, (uint8_t)creature_type);
    state.party_x = (int16_t)game->party_x;
    state.party_y = (int16_t)game->party_y;
    state.party_map = (int16_t)game->current_level;
    state.current_map = (int16_t)goal->map;
    state.heros_in_party = goal->runtime->source_party.heros_in_party;
    for (int i = 0; i < DM2_MAX_HEROES; ++i) {
        state.hero_hp[i] = goal->runtime->source_party.hero[i].curHP;
        state.hero_partypos[i] = (uint8_t)goal->runtime->source_party.hero[i].partypos;
    }
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.abs_fn = dm2_runtime_ccm_abs16;
    callbacks.calc_vector_dir = dm2_runtime_ccm_vector_dir;
    callbacks.rand16 = dm2_runtime_ccm_rand16;
    callbacks.randdir = dm2_runtime_ccm_randdir;
    callbacks.randbit = dm2_runtime_ccm_randbit;
    callbacks.rand_full = dm2_runtime_ccm_rand_full;
    callbacks.min_fn = dm2_runtime_ccm_min16;
    callbacks.get_player_at_position = dm2_runtime_ccm_get_player_at_position;
    callbacks.find_hero_at = dm2_runtime_ccm_find_hero_at;
    callbacks.creature_attacks_player = dm2_runtime_ccm_creature_attacks_player;
    memset(&receipt, 0, sizeof(receipt));
    if (!dm2_v1_creature_attacks_party_full(&state, &callbacks, goal,
                                            &receipt) || receipt.fail_closed)
        return 0;
    return 1;
}

static int dm2_runtime_ccm_go_there(void *context, uint8_t *slot,
    uint16_t creature_type, int mode, int x, int y, int dir,
    int16_t *parw00, int16_t *parw01)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    uint8_t path[32u * 32u];
    int32_t path_length;

    (void)creature_type;
    (void)dir;
    if (goal && goal->runtime)
        ++goal->runtime->dynamic_path_attempts;
    if (!goal || !slot || !parw00 || !parw01 || (mode != 4 && mode != 5))
        return 0;
    path_length = dm2_runtime_source_find_walk_path(
        goal, x, y, path, (int)sizeof(path));
    if (path_length <= 0)
        return 0;
    ++goal->runtime->dynamic_path_admissions;
    slot[0x1d] = (uint8_t)(path[0] & 3u);
    slot[0x1a] = 0x01u; /* source WALK_NOW command */
    *parw00 = (int16_t)path[0];
    *parw01 = (int16_t)(path_length - 1);
    /* CREATURE_GO_THERE owns the source WALK_NOW handoff.  The CCM message
     * loop may later stop at an unbound animation row, so queue the normal
     * source MOVE_RECORD_TO transaction at this exact source boundary. */
    if (dm2_runtime_ccm_proceed_walking(
            goal, 0x01u, creature_type, slot, 0u, 0u,
            goal->game_tick) != -2)
        return 0;
    return 1;
}

/* c_creature.cpp WALK_NOW owner.  The source handler schedules the
 * MOVE_RECORD_TO path; it does not authorize a host-side position write.
 * Keep the same boundary here: validate the adjacent source-owned floor and
 * enqueue the normal 0x3c moverec timer, which is later consumed by the
 * authenticated runtime moverec owner. */
static int32_t dm2_runtime_ccm_proceed_walking(
    void *context, uint8_t command, uint16_t creature_type,
    uint8_t *slot, uint16_t adj0, uint16_t adj1,
    unsigned long game_tick)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_RuntimeState *rt;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimer timer;
    int dir;
    int nx;
    int ny;
    int raw;
    int square;
    DM2_V1_SourceTimerResult enqueue_result;

    (void)adj0;
    (void)adj1;
    if (!goal || !goal->runtime)
        return 0;
    if (command == 0x08u || command == 0x26u)
        return dm2_runtime_ccm_attack_party(goal, command, creature_type);
    (void)slot;
    if (command != 0x01u && command != 0x02u && command != 0x09u)
        return 0;
    rt = goal->runtime;
    dungeon = rt->boot ? (DM2_V1_DungeonData *)rt->boot->dungeon_data : NULL;
    if (!dungeon || !dungeon->record_graph_complete ||
        goal->map < 0 || goal->map >= dungeon->level_count ||
        goal->x < 0 || goal->y < 0 ||
        goal->x >= dungeon->level_widths[goal->map] ||
        goal->y >= dungeon->level_heights[goal->map])
        return 0;

    /* byte@0x1d is the source-facing field used by WALK_NOW. */
    dir = slot ? (slot[0x1d] & 3) : -1;
    if (dir < 0) return 0;
    nx = goal->x + dm2_v1_dir_dx[dir];
    ny = goal->y + dm2_v1_dir_dy[dir];
    if (nx < 0 || ny < 0 || nx >= dungeon->level_widths[goal->map] ||
        ny >= dungeon->level_heights[goal->map])
        return 0;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, goal->map, nx, ny);
    square = raw < 0 ? -1 : dm2_runtime_normalize_square_type_for_dungeon(
        dungeon, dm2_runtime_square_type_at(dungeon, goal->map, nx, ny, raw), raw);
    if (square != DM2_SQUARE_FLOOR && square != DM2_SQUARE_FLOOR_ORNATE)
        return 0;
    if (dm2_v1_get_creature_at(&rt->record_pools, dungeon, goal->map, nx, ny) !=
        DM2_V1_RECORD_HANDLE_NULL)
        return 0;

    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)goal->map << 24) |
        (((uint32_t)game_tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = 0x3c;
    timer.actor = (uint8_t)dir;
    timer.value_a = (int16_t)((nx & 0xff) | ((ny & 0xff) << 8));
    timer.value_b = goal->record_handle;
    enqueue_result = dm2_v1_source_timer_enqueue(&rt->timer_queue, &timer, 0);
    if (enqueue_result == DM2_V1_SOURCE_TIMER_OK) {
        ++rt->dynamic_move_queue_admissions;
        rt->dynamic_move_pending_source_map = goal->map;
        rt->dynamic_move_pending_source_x = goal->x;
        rt->dynamic_move_pending_source_y = goal->y;
        rt->dynamic_move_pending_record = goal->record_handle;
    }
    return enqueue_result == DM2_V1_SOURCE_TIMER_OK ? -2 : 0;
}

/* Bind the authenticated goal branches of DM2_14cd_09e2.  Melee range uses
 * the source ATTACKS_PARTY command; otherwise the strategy selector owns its
 * GDAT word@1/table1d607e admission and the bounded dynamic path callback
 * writes the source WALK_NOW command. */
static int dm2_runtime_ccm_ai_goal(
    void *context, uint8_t *slot, uint16_t creature_type,
    unsigned long game_tick)
{
    DM2_RuntimeCcmGoalContext *goal = (DM2_RuntimeCcmGoalContext *)context;
    DM2_V1_CreatureStrategyReceipt strategy;

    if (!goal || !goal->runtime || !slot) return 0;
    {
        DM2_V1_GameState *game = goal->runtime->boot &&
            goal->runtime->boot->dm2_state
            ? (DM2_V1_GameState *)goal->runtime->boot->dm2_state : NULL;
        int distance = game ? abs(goal->x - game->party_x) +
            abs(goal->y - game->party_y) : INT_MAX;
        /* c_creature.cpp's ATTACKS_PARTY fallback is selected at melee
         * distance only when the authenticated AI row owns melee. */
        if (game && goal->map == game->current_level &&
            dm2_v1_creature_attacks_party((int)creature_type, distance)) {
            slot[0x1a] = DM2_CCM_CREATURE_ATTACKS_PARTY;
            return 1;
        }
    }
    memset(&strategy, 0, sizeof(strategy));
    if (!dm2_v1_creature_strategy_select(
            &goal->runtime->record_pools, &goal->runtime->caii,
            goal->record_handle, creature_type, goal->x, goal->y,
            game_tick, dm2_runtime_ccm_go_there, goal, NULL, NULL,
            &strategy) ||
        !strategy.valid || strategy.is_static != 1 ||
        strategy.final_command < 0) {
        return 0;
    }
    slot[0x1a] = (uint8_t)strategy.final_command;
    return 1;
}

/* Defined with the DB14 owner below; this endian-aware record reader is also
 * the shared source-word reader for the think/death boundary above. */
static uint16_t dm2_runtime_missile_rd16(
    const DM2_V1_RecordPoolSet *pools, const uint8_t *p);

/* c_ai.cpp:5679-5747 — the source transfers the CAII accumulated attack
 * amount (creature-local word@0x14) to the DB4 HP word at offset 6 at the
 * beginning of DM2_THINK_CREATURE, clears the accumulator, and enters
 * WOUND_CREATURE.  ATTACK_CREATURE itself is only the accumulator owner;
 * treating its positive argument as an HP write makes a hit heal or bypass
 * the source death/drop transaction. */
static int dm2_runtime_apply_pending_creature_damage(
    DM2_V1_RuntimeState *rt, DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int16_t creature_record)
{
    uint8_t *record;
    uint8_t *slot;
    const DM2_AIDefinition *ai = NULL;
    DM2_V1_GameState *game;
    uint16_t pending;
    uint16_t hp;

    if (!rt || !dungeon || !rt->record_pools_valid || !rt->caii_ready ||
        !rt->caii.valid || dm2_v1_record_handle_pool(creature_record) != 4)
        return 0;
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             creature_record);
    if (!record || record[5] == 0xffu || record[5] >= rt->caii.capacity)
        return 0;
    if (!dm2_v1_creature_ai_spec_def((int)record[4], &ai) || !ai)
        return 0;
    /* c_creature.cpp:176-177 — byte@2 == 0xff has no woundable defense. */
    if (ai->ArmorClass == 0xffu)
        return 1;
    slot = rt->caii.slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
    pending = (uint16_t)(slot[0x14] | ((uint16_t)slot[0x15] << 8));
    if (pending == 0u)
        return 1;
    /* Preserve the last meaningful source damage receipt across the many
     * think timers that have no pending ATTACK_CREATURE amount. */
    memset(&g_dm2_last_creature_damage, 0,
           sizeof(g_dm2_last_creature_damage));
    g_dm2_last_creature_damage.valid = 1;
    g_dm2_last_creature_damage.creature_record = creature_record;
    g_dm2_last_creature_damage.creature_type = (int)record[4];
    g_dm2_last_creature_damage.pending_damage = pending;

    /* c_ai.cpp:5681-5687 — a zero HP record is initialized from the
     * accumulator before the accumulator is consumed.  Real loaded DB4
     * records normally already carry their source HP. */
    hp = dm2_runtime_missile_rd16(&rt->record_pools, record + 6);
    g_dm2_last_creature_damage.hp_before = hp;
    if (hp == 0u) {
        hp = 1u;
        dm2_runtime_record_wr16(rt, record + 6, hp);
    }
    slot[0x14] = 0u;
    slot[0x15] = 0u;

    if (pending < hp) {
        dm2_runtime_record_wr16(rt, record + 6,
                                (uint16_t)(hp - pending));
        g_dm2_last_creature_damage.hp_after = (int)(hp - pending);
        g_dm2_last_creature_damage.wound_applied = 1;
        return 1;
    }

    /* c_creature.cpp:207-227 — lethal WOUND_CREATURE first writes HP=1;
     * kill-flag records then enter DELETE_CREATURE_RECORD, while the
     * non-deleting branch enters the source dying mode. */
    dm2_runtime_record_wr16(rt, record + 6, 1u);
    g_dm2_last_creature_damage.hp_after = 1;
    g_dm2_last_creature_damage.wound_applied = 1;
    g_dm2_last_creature_damage.lethal = 1;
    if ((dm2_runtime_missile_rd16(&rt->record_pools, record) & 1u) == 0u) {
        slot[0x1a] = 0x13u;
        return 1;
    }
    if (!dm2_v1_creature_drop_slots_loaded((int)record[4])) {
        /* The source delete owner was not admitted: roll back the handoff so
         * a later authenticated think can retry it without losing damage. */
        dm2_runtime_record_wr16(rt, record + 6, g_dm2_last_creature_damage.hp_before);
        slot[0x14] = (uint8_t)(pending & 0xffu);
        slot[0x15] = (uint8_t)(pending >> 8);
        return 0;
    }
    game = rt->boot && rt->boot->dm2_state
        ? (DM2_V1_GameState *)rt->boot->dm2_state : NULL;
    if (!game) {
        dm2_runtime_record_wr16(rt, record + 6, g_dm2_last_creature_damage.hp_before);
        slot[0x14] = (uint8_t)(pending & 0xffu);
        slot[0x15] = (uint8_t)(pending >> 8);
        return 0;
    }
    {
        uint16_t drop_slots[DM2_DROP_SLOT_COUNT];
        DM2_V1_DeleteCreatureFullReceipt death;
        memset(drop_slots, 0, sizeof(drop_slots));
        for (int i = 0; i < DM2_DROP_SLOT_COUNT; ++i)
            drop_slots[i] = dm2_v1_creature_drop_slot_word(
                (int)record[4], i);
        memset(&death, 0, sizeof(death));
        if (!dm2_v1_delete_creature_record_full(
            &rt->record_pools, dungeon, &rt->caii, &rt->timer_queue,
            &rt->drop_rng, map, (unsigned long)rt->tick_count,
            x, y, 0, 0, game->party_x, game->party_y,
            game->party_dir, drop_slots, &death))
        {
            dm2_runtime_record_wr16(rt, record + 6,
                                    g_dm2_last_creature_damage.hp_before);
            slot[0x14] = (uint8_t)(pending & 0xffu);
            slot[0x15] = (uint8_t)(pending >> 8);
            return 0;
        }
        g_dm2_last_wield_death_drop_count = death.drop.drops_placed;
        g_dm2_last_wield_death_drop_iterations = death.drop.drops_iterations;
        g_dm2_last_wield_death_drop_alloc_failures =
            death.drop.generated_drop_alloc_failures;
        g_dm2_last_wield_death_drop_first_itemspec =
            (int)death.drop.generated_drop_first_itemspec;
        g_dm2_last_wield_death_drop_first_db =
            death.drop.generated_drop_first_db;
        g_dm2_last_wield_death_drop_alloc_free_records =
            death.drop.generated_drop_alloc_free_records;
        g_dm2_last_wield_death_deallocated = death.dealloc_performed;
        g_dm2_last_creature_damage.deallocated = death.dealloc_performed;
        g_dm2_last_creature_damage.drops_placed = death.drop.drops_placed;
    }
    return 2;
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
    int damage_result;

    if (!rt || !timer || !rt->boot || !rt->boot->dungeon_data) return 0;
    (void)think_type;

    damage_result = dm2_runtime_apply_pending_creature_damage(
        rt, (DM2_V1_DungeonData *)rt->boot->dungeon_data,
        map, x, y, creature_record);
    if (damage_result == 0)
        return 0;
    if (damage_result == 2)
        return 1;

    loader = rt->boot ? dm2_v1_boot_asset_loader(rt->boot) : NULL;

    /* Try the CCM message loop when all prerequisites are available. */
    if (loader && rt->caii_ready && rt->record_pools_valid) {
        DM2_V1_CcmLoopReceipt ccm_receipt;
        DM2_RuntimeCcmGoalContext goal_context;
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
            memset(&goal_context, 0, sizeof(goal_context));
            goal_context.runtime = rt;
            goal_context.record_handle = creature_record;
            goal_context.map = map;
            goal_context.x = x;
            goal_context.y = y;
            goal_context.game_tick = rt->tick_count;
            slot = rt->caii.slots +
                   (size_t)rec[5] * DM2_V1_CAII_SLOT_SIZE;
            /* Extract adj pair from CAII slot at offset 0x0e-0x11 */
            adj[0] = (int16_t)((uint16_t)slot[0x0e] |
                               ((uint16_t)slot[0x0f] << 8));
            adj[1] = (int16_t)((uint16_t)slot[0x10] |
                               ((uint16_t)slot[0x11] << 8));

            memset(&ccm_receipt, 0, sizeof(ccm_receipt));
            {
                int ccm_result = dm2_v1_ccm_message_loop(
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
                    /* c_ai.cpp's v1e0238 is the global sleep/wake lock,
                     * not the party-facing direction.  Passing view_dir
                     * here made the CCM pre-check depend on orientation and
                     * could admit a creature body while the source party
                     * was asleep. */
                    (int32_t)rt->source_sleeping,
                    (unsigned long)rt->tick_count,
                    dm2_runtime_ccm_ai_goal, &goal_context,
                    NULL, NULL,
                    dm2_runtime_ccm_proceed_walking, &goal_context,
                    &ccm_receipt);
                rt->last_ccm_receipt = ccm_receipt;
                rt->ccm_receipt_valid = 1;
                if (ccm_result == 1 && ccm_receipt.valid) {
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
    /* Keep the drop RNG source-shaped for the bounded CCM path.  The 0fcb
     * deletion branch remains unavailable here: it needs one transaction
     * owner for c_map, 3CE7D, DB allocation cleanup, CAII and the real
     * timer queue before it may mutate a live record graph. */
    dm2_v1_drops_rng_init(&rt->drop_rng);
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
 * runs against the session-owned record pools. The CCM stream has bounded
 * source owners for movement and adjacent melee; unsupported animation and
 * action branches remain receipted fail-closed. The timer is consumed exactly
 * like the source's early return
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
 * Bounded slice: reads the source timer's encoded map and tile byte, applies
 * the bit mutation, and writes back only when the authenticated map/cell
 * exists. A missing map/cell is acknowledged fail-closed rather than
 * mutating the current party map by accident.
 */
static int dm2_runtime_destroy_door_timer(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    int map, x, y, raw;
    (void)source_index;
    (void)receipt;

    if (!timer || !rt || !rt->boot || !rt->boot->dungeon_data)
        return 1;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;

    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    if (map < 0 || map >= dungeon->level_count || raw < 0)
        return 0;
    raw = (raw & 0xfff8) | 0x0005;
    if (dm2_v1_dungeon_set_tile_raw(dungeon, map, x, y,
                                    (uint16_t)raw) < 0)
        return 0;
    return 1;
}

static uint8_t *dm2_runtime_source_record_for_timer(
    DM2_V1_RuntimeState *rt,
    const DM2_V1_SourceTimer *timer,
    uint16_t record_id,
    int *out_type,
    int *out_size)
{
    const DM2_V1_DungeonData *dungeon;
    int map;

    if (!rt || !timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data) {
        return NULL;
    }
    dungeon = (const DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    if (!dungeon->record_graph_complete || map < 0 ||
        map >= dungeon->level_count) {
        return NULL;
    }
    return (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        dungeon, record_id, out_type, NULL, out_size);
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

    if (!timer) return 0;
    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = dm2_runtime_source_record_for_timer(rt, timer, record_id,
                                                 &type, &size);
    if (!record || size < 4) return 0;

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

    if (!timer) return 0;
    record_id = (uint16_t)(timer->value_b & 0xffffu);
    record = dm2_runtime_source_record_for_timer(rt, timer, record_id,
                                                 &type, &size);
    if (!record || size < 5) return 0;

    if (record[4] & 0x04u)
        return 1;

    record[4] &= (uint8_t)~0x01u;
    return 1;
}

/* Runtime cross-map moverec admission.  The pool and raw dungeon mirrors
 * share ObjectIDs but are separate owners; both must be complete and agree
 * before a DB4 is cut from one map and appended to another. */
static int dm2_runtime_record_chain_mirrors_complete(
    const DM2_V1_RuntimeState *rt, const DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int16_t needle, int *out_contains,
    int *out_has_actuator)
{
    int budget = 1;
    int16_t pool_cursor;
    int16_t raw_cursor;
    if (out_contains) *out_contains = 0;
    if (out_has_actuator) *out_has_actuator = 0;
    if (!rt || !dungeon || !dungeon->raw_data ||
        map < 0 || map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map])
        return 0;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        if (rt->record_pools.pools[db].record_count < 0 ||
            rt->record_pools.pools[db].extension_count < 0 ||
            budget > INT_MAX - rt->record_pools.pools[db].record_count -
                rt->record_pools.pools[db].extension_count)
            return 0;
        budget += rt->record_pools.pools[db].record_count +
                  rt->record_pools.pools[db].extension_count;
    }
    pool_cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, map, x, y);
    raw_cursor = pool_cursor;
    for (int step = 0; step < budget; ++step) {
        const uint8_t *pool_record;
        const uint8_t *raw_record;
        int raw_size = 0;
        int raw_type = -1;
        int16_t pool_next;
        int16_t raw_next;
        if (pool_cursor != raw_cursor ||
            pool_cursor == DM2_V1_RECORD_HANDLE_NULL ||
            raw_cursor == DM2_V1_RECORD_HANDLE_NULL)
            return 0;
        if (pool_cursor == DM2_V1_RECORD_HANDLE_END)
            return 1;
        pool_record = dm2_v1_record_pool_address(&rt->record_pools,
                                                  pool_cursor);
        raw_record = (const uint8_t *)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)raw_cursor, &raw_type, NULL, &raw_size);
        if (!pool_record || !raw_record || raw_size < 2 ||
            !dm2_v1_record_pool_next_link(&rt->record_pools, pool_cursor,
                                          &pool_next))
            return 0;
        raw_next = (int16_t)((uint16_t)raw_record[0] |
                             ((uint16_t)raw_record[1] << 8));
        if (pool_next != raw_next)
            return 0;
        if (pool_cursor == needle && out_contains) *out_contains = 1;
        if (dm2_v1_record_handle_pool(pool_cursor) == 3 && out_has_actuator)
            *out_has_actuator = 1;
        if (pool_next == DM2_V1_RECORD_HANDLE_NULL ||
            raw_next == DM2_V1_RECORD_HANDLE_NULL)
            return 0;
        pool_cursor = pool_next;
        raw_cursor = raw_next;
    }
    return 0;
}

static int dm2_runtime_moverec_move_between_maps(
    DM2_V1_RuntimeState *rt, DM2_V1_DungeonData *dungeon,
    int source_map, int source_x, int source_y,
    int destination_map, int destination_x, int destination_y,
    int16_t record_handle)
{
    int16_t source_pool_head;
    int16_t source_raw_head;
    int16_t destination_pool_head;
    int16_t destination_raw_head;
    DM2_V1_SkprojectCutRecordReceipt cut_receipt;
    DM2_V1_SkprojectAppendRecordReceipt append_receipt;
    int contains = 0;
    int actuator = 0;
    memset(&cut_receipt, 0, sizeof(cut_receipt));
    memset(&append_receipt, 0, sizeof(append_receipt));
    if (!rt || !dungeon || record_handle == DM2_V1_RECORD_HANDLE_NULL ||
        record_handle == DM2_V1_RECORD_HANDLE_END ||
        !dm2_runtime_record_chain_mirrors_complete(
            rt, dungeon, source_map, source_x, source_y, record_handle,
            &contains, &actuator) || !contains || actuator ||
        !dm2_runtime_record_chain_mirrors_complete(
            rt, dungeon, destination_map, destination_x, destination_y,
            record_handle, &contains, &actuator) || contains || actuator)
        return 0;
    source_pool_head = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, source_map, source_x, source_y);
    source_raw_head = source_pool_head;
    destination_pool_head = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, destination_map, destination_x, destination_y);
    destination_raw_head = destination_pool_head;
    if (!dm2_v1_record_pool_cut_from_list(
            &rt->record_pools, &source_pool_head, record_handle) ||
        !dm2_v1_skproject_cut_record_from(
            dungeon, (uint16_t)record_handle,
            (uint16_t *)&source_raw_head, -1, -1, -1, &cut_receipt) ||
        !cut_receipt.valid ||
        dm2_v1_dungeon_set_first_thing(
            dungeon, source_map, source_x, source_y,
            (uint16_t)source_pool_head) != 0 ||
        dm2_v1_dungeon_set_first_thing(
            dungeon, source_map, source_x, source_y,
            (uint16_t)source_raw_head) != 0 ||
        !dm2_v1_record_pool_append_to_list(
            &rt->record_pools, &destination_pool_head, record_handle) ||
        !dm2_v1_skproject_append_record_to(
            dungeon, (uint16_t)record_handle,
            (uint16_t *)&destination_raw_head, -1, -1, -1,
            &append_receipt) || !append_receipt.valid ||
        dm2_v1_dungeon_set_first_thing(
            dungeon, destination_map, destination_x, destination_y,
            (uint16_t)destination_pool_head) != 0 ||
        dm2_v1_dungeon_set_first_thing(
            dungeon, destination_map, destination_x, destination_y,
            (uint16_t)destination_raw_head) != 0)
        return 0;
    return 1;
}

/*
 * dm2_runtime_process_moverec_timer — runtime owner for c_tim 0x3C/0x3D.
 *
 * c_tim_proc.cpp decodes A as the destination, B as the DB4 handle, deletes
 * the due timer, then calls MOVE_RECORD_TO(record, -3, 0, x, y).  The
 * source helper discovers the record's actual ground-chain source; the
 * runtime therefore performs the same discovery instead of trusting timer
 * coordinates as a source.  The party-sentinel is admitted only on an
 * authenticated same-map plain floor without a creature; the cross-map
 * shape has a bounded mirror/map owner but remains behind its positive
 * verification gate, while wake/sleep and actuator tails remain fail-closed.
 */
static int dm2_runtime_process_moverec_timer(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SoundQueueState sound_backup;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;
    DM2_V1_MoveRecordToReceipt move_receipt;
    DM2_V1_CaiiMoverecActivationReceipt caii_receipt;
    uint8_t *dungeon_backup = NULL;
    uint8_t *caii_backup = NULL;
    uint8_t *record;
    int16_t record_handle;
    int map;
    int x;
    int y;
    int source_x = -1;
    int source_y = -1;
    int found = 0;
    int cross_map_move = 0;
    size_t caii_bytes;
    int queue_backup_ready = 0;
    int sound_backup_ready = 0;
    int party_move = 0;
    int party_backup_ready = 0;
    int moverec_failure_stage = 0;
    int old_party_x = -1;
    int old_party_y = -1;
    int old_party_dir = -1;
    int old_runtime_map = -1;
    int runtime_map_changed = 0;
    DM2_V1_Party party_backup;

    (void)source_index;
    if (receipt) receipt->handler_rejected_count++;
    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&sound_backup, 0, sizeof(sound_backup));
    memset(&move_receipt, 0, sizeof(move_receipt));
    memset(&caii_receipt, 0, sizeof(caii_receipt));
    memset(&sound_receipt, 0, sizeof(sound_receipt));

    if (!rt || !timer) {
        return 0;
    }
    if (timer->type == 0x3cu || timer->type == 0x3du)
        ++rt->dynamic_move_timer_consumptions;
    if (!rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid || !rt->caii_ready ||
        !rt->sound_queue_ready ||
        (timer->type != 0x3cu && timer->type != 0x3du)) {
        moverec_failure_stage = 1;
        goto moverec_rollback;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    record_handle = timer->value_b;

    if (map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map] ||
        !dungeon->record_graph_complete ||
        (record_handle != (int16_t)0xffff &&
         dm2_v1_record_handle_pool(record_handle) != 4) ||
        (record_handle == (int16_t)0xffff &&
         (map != rt->dungeon_level || !rt->source_party_valid ||
          !rt->boot->dm2_state ||
          dm2_runtime_normalize_square_type_for_dungeon(
              dungeon,
              dm2_runtime_square_type_at(
                  dungeon, map, x, y,
                  dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y)),
              dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y)) !=
              DM2_SQUARE_FLOOR ||
          dm2_v1_get_creature_at(&rt->record_pools, dungeon, map, x, y) !=
              DM2_V1_RECORD_HANDLE_NULL)) ||
        (record_handle != (int16_t)0xffff &&
         (!(record = dm2_v1_record_pool_address_mut(
                &rt->record_pools, record_handle)) || record[5] == 0xffu))) {
        goto moverec_rollback;
    }

    /* MOVE_RECORD_TO also mutates the party-sentinel path.  Establish the
     * complete rollback boundary before entering either party or creature
     * movement; otherwise a later sound/owner rejection could leave the
     * party pose and tile graph half-committed. */
    old_runtime_map = rt->dungeon_level;
    queue_backup = rt->timer_queue;
    queue_backup_ready = 1;
    caii_bytes = (size_t)rt->caii.capacity * DM2_V1_CAII_SLOT_SIZE;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools) ||
        dungeon->raw_size <= 0 || !dungeon->raw_data ||
        !(dungeon_backup = (uint8_t *)malloc((size_t)dungeon->raw_size)) ||
        (caii_bytes != 0u &&
         !(caii_backup = (uint8_t *)malloc(caii_bytes)))) {
        moverec_failure_stage = 2;
        goto moverec_rollback;
    }
    memcpy(dungeon_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
    if (caii_bytes != 0u) memcpy(caii_backup, rt->caii.slots, caii_bytes);
    sound_backup = rt->sound_queue;
    sound_backup_ready = 1;

    if (record_handle != (int16_t)0xffff) {
        int candidate_source_map = -1;
        int candidate_source_x = -1;
        int candidate_source_y = -1;
        for (int scan_map = 0; scan_map < dungeon->level_count &&
             candidate_source_map < 0; ++scan_map) {
            if (scan_map == map) continue;
            for (int scan_x = 0; scan_x < dungeon->level_widths[scan_map] &&
                 candidate_source_map < 0; ++scan_x) {
                for (int scan_y = 0; scan_y < dungeon->level_heights[scan_map];
                     ++scan_y) {
                    int contains = 0;
                    int actuator = 0;
                    if (dm2_runtime_record_chain_mirrors_complete(
                            rt, dungeon, scan_map, scan_x, scan_y,
                            record_handle, &contains, &actuator) &&
                        contains && !actuator) {
                        candidate_source_map = scan_map;
                        candidate_source_x = scan_x;
                        candidate_source_y = scan_y;
                        break;
                    }
                }
            }
        }
        if (candidate_source_map >= 0) {
            int source_raw = dm2_v1_dungeon_get_tile_raw(
                dungeon, candidate_source_map, candidate_source_x,
                candidate_source_y);
            int destination_raw = dm2_v1_dungeon_get_tile_raw(
                dungeon, map, x, y);
            int source_square = source_raw < 0 ? -1 :
                dm2_runtime_normalize_square_type_for_dungeon(
                    dungeon,
                    dm2_runtime_square_type_at(
                        dungeon, candidate_source_map, candidate_source_x,
                        candidate_source_y, source_raw), source_raw);
            int destination_square = destination_raw < 0 ? -1 :
                dm2_runtime_normalize_square_type_for_dungeon(
                    dungeon,
                    dm2_runtime_square_type_at(
                        dungeon, map, x, y, destination_raw), destination_raw);
            int destination_contains = 0;
            int destination_actuator = 0;
            if ((source_square != DM2_SQUARE_FLOOR &&
                 source_square != DM2_SQUARE_FLOOR_ORNATE) ||
                (destination_square != DM2_SQUARE_FLOOR &&
                 destination_square != DM2_SQUARE_FLOOR_ORNATE) ||
                dm2_v1_get_creature_at(&rt->record_pools, dungeon, map, x, y) !=
                    DM2_V1_RECORD_HANDLE_NULL ||
                !dm2_runtime_record_chain_mirrors_complete(
                    rt, dungeon, map, x, y, record_handle,
                    &destination_contains, &destination_actuator) ||
                destination_contains || destination_actuator ||
                !dm2_runtime_moverec_move_between_maps(
                    rt, dungeon, candidate_source_map, candidate_source_x,
                    candidate_source_y, map, x, y, record_handle)) {
                moverec_failure_stage = 3;
                goto moverec_rollback;
            }
            source_x = candidate_source_x;
            source_y = candidate_source_y;
            found = 1;
            cross_map_move = 1;
            if (map != rt->dungeon_level) {
                DM2_V1_GameState *game =
                    (DM2_V1_GameState *)rt->boot->dm2_state;
                if (!game) goto moverec_rollback;
                game->current_level = map;
                game->outdoor = dm2_v1_dungeon_is_outdoor(dungeon, map);
                rt->dungeon_level = map;
                rt->outdoor = game->outdoor;
                dm2_runtime_refresh_map_transition_context(rt);
                runtime_map_changed = 1;
            }
        }
    }

    if (record_handle == (int16_t)0xffff) {
        DM2_V1_GameState *game = (DM2_V1_GameState *)rt->boot->dm2_state;
        old_party_x = game->party_x;
        old_party_y = game->party_y;
        old_party_dir = game->party_dir;
        if (old_party_x < 0 || old_party_y < 0 ||
            old_party_x >= dungeon->level_widths[map] ||
            old_party_y >= dungeon->level_heights[map])
            goto moverec_rollback;
        party_backup = rt->source_party;
        party_backup_ready = 1;
        party_move = 1;
        if (!dm2_v1_move_record_to(
                &rt->record_pools, dungeon, &rt->timer_queue,
                (int16_t)0xffff, (int16_t)old_party_x,
                (int16_t)old_party_y, (int16_t)x, (int16_t)y, 0, map,
                (uint32_t)rt->tick_count, &move_receipt) ||
            !move_receipt.valid || !move_receipt.is_party_move ||
            move_receipt.fail_closed)
        {
            moverec_failure_stage = 4;
            goto moverec_rollback;
        }
        game->party_x = x;
        game->party_y = y;
        rt->view_dir = game->party_dir & 3;
        dm2_runtime_refresh_map_transition_context(rt);
    }

    /* WALK_NOW already supplied an authenticated source cell.  Preserve that
     * source across the timer boundary; 0x3c itself intentionally carries
     * only destination A and DB4 handle B in SKProject. */
    if (!party_move && !cross_map_move &&
        rt->dynamic_move_pending_record == record_handle &&
        rt->dynamic_move_pending_source_map == map &&
        rt->dynamic_move_pending_source_x >= 0 &&
        rt->dynamic_move_pending_source_y >= 0) {
        source_x = rt->dynamic_move_pending_source_x;
        source_y = rt->dynamic_move_pending_source_y;
        found = 1;
    }

    /* Locate the source by the dungeon's raw ground-chain first.  The
     * MOVE_RECORD_TO primitive cuts that chain; the session pool mirror is a
     * fallback only because some editions expose a differently masked pool
     * link during boot. */
    for (int sy = 0; !party_move && !cross_map_move &&
         sy < dungeon->level_heights[map] && !found; ++sy) {
        for (int sx = 0; sx < dungeon->level_widths[map]; ++sx) {
            uint16_t cursor = (uint16_t)dm2_v1_dungeon_get_first_thing(
                dungeon, map, sx, sy);
            int steps = 0;
            while (cursor != DM2_THING_END_MARKER &&
                   cursor != DM2_THING_NULL_MARKER &&
                   steps++ < DM2_V1_SKSAVE_RECYCLE_MAX_STEPS) {
                int record_type = -1;
                const uint8_t *raw_record =
                    dm2_v1_dungeon_get_thing_record(
                        dungeon, cursor, &record_type, NULL, NULL);
                if (!raw_record) break;
                if ((cursor & 0x3fffu) ==
                    ((uint16_t)record_handle & 0x3fffu)) {
                    source_x = sx;
                    source_y = sy;
                    found = 1;
                    break;
                }
                cursor = dm2_v1_dungeon_read_record_u16(dungeon, raw_record);
            }
        }
    }

    /* Locate the source by bounded authenticated pool-chain walks when the
     * raw graph did not expose the edition's masked handle. */
    for (int sy = 0; !party_move && !cross_map_move &&
         sy < dungeon->level_heights[map] && !found; ++sy) {
        for (int sx = 0; sx < dungeon->level_widths[map]; ++sx) {
            int16_t cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
                dungeon, map, sx, sy);
            int steps = 0;
            int contains = 0;
            while (cursor != DM2_V1_RECORD_HANDLE_END &&
                   cursor != DM2_V1_RECORD_HANDLE_NULL &&
                   steps++ < DM2_V1_SKSAVE_RECYCLE_MAX_STEPS) {
                int16_t next;
                if (!dm2_v1_record_pool_address(&rt->record_pools, cursor) ||
                    !dm2_v1_record_pool_next_link(&rt->record_pools, cursor,
                                                   &next)) {
                    break;
                }
                if (cursor == record_handle) {
                    contains = 1;
                    break;
                }
                cursor = next;
            }
            if (contains) {
                source_x = sx;
                source_y = sy;
                found = 1;
            }
        }
    }
    if (!party_move && (!found || (source_x == x && source_y == y)))
    {
        moverec_failure_stage = 5;
        goto moverec_rollback;
    }

    /* A creature may move onto an empty floor.  MOVE_RECORD_TO owns creating
     * the destination ground-chain root; only the authenticated no-creature
     * occupancy proof above is required here. */

    if (!party_move && !cross_map_move &&
        (!dm2_v1_move_record_to(&rt->record_pools, dungeon,
                               &rt->timer_queue, record_handle,
                               (int16_t)source_x, (int16_t)source_y,
                               (int16_t)x, (int16_t)y, 0, map,
                               (uint32_t)rt->tick_count, &move_receipt) ||
        !move_receipt.valid || move_receipt.fail_closed ||
        !move_receipt.record_cut || !move_receipt.record_appended)) {
        moverec_failure_stage = !move_receipt.record_cut ? 71 : 72;
        goto moverec_rollback;
    }
    if (!party_move) {
        /* The source commits the record relocation before the optional CAII
         * activation tail.  Unknown edition AI flags therefore suppress only
         * that tail; they must not roll back an already-authenticated move. */
        (void)dm2_v1_caii_moverec_activation(
            &rt->record_pools, dungeon, &rt->caii, &rt->timer_queue,
            map, (unsigned long)rt->tick_count, record_handle, x, y,
            &caii_receipt);
    }

    memset(&sound_env, 0, sizeof(sound_env));
    sound_env.current_map = (int16_t)map;
    sound_env.gate_map_a = rt->sound_env.gate_map_a;
    sound_env.gate_map_b = rt->sound_env.gate_map_b;
    sound_env.facing = (uint16_t)rt->view_dir;
    sound_env.party_x = (int16_t)dm2_v1_runtime_get_party_x();
    sound_env.party_y = (int16_t)dm2_v1_runtime_get_party_y();
    sound_env.gametick = rt->tick_count;
    if (!dm2_v1_sound_queue_noise_gen1(
            &rt->sound_queue, 3, 0, (int8_t)0x89, 0x61, 0x80,
            (int16_t)x, (int16_t)y, 1, &sound_env, &sound_receipt) &&
        !sound_receipt.rejected_map_gate) {
        moverec_failure_stage = 9;
        goto moverec_rollback;
    }

    dm2_v1_record_pool_set_free(&pool_backup);
    free(dungeon_backup);
    free(caii_backup);
    ++rt->dynamic_move_successes;
    rt->dynamic_move_last_failure = 0;
    if (rt->dynamic_move_pending_record == record_handle)
        rt->dynamic_move_pending_record = DM2_V1_RECORD_HANDLE_NULL;
    if (receipt && receipt->handler_rejected_count > 0)
        receipt->handler_rejected_count--;
    return 1;

moverec_rollback:
    if (rt && moverec_failure_stage > 0)
        rt->dynamic_move_last_failure = moverec_failure_stage;
    if (queue_backup_ready) rt->timer_queue = queue_backup;
    if (caii_backup && rt->caii.slots) memcpy(rt->caii.slots, caii_backup,
                                              caii_bytes);
    if (dungeon_backup && dungeon->raw_data)
        memcpy(dungeon->raw_data, dungeon_backup, (size_t)dungeon->raw_size);
    if (pool_backup.valid) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backup_ready) rt->sound_queue = sound_backup;
    if (runtime_map_changed && rt->boot && rt->boot->dm2_state &&
        old_runtime_map >= 0) {
        DM2_V1_GameState *game = (DM2_V1_GameState *)rt->boot->dm2_state;
        game->current_level = old_runtime_map;
        game->outdoor = dm2_v1_dungeon_is_outdoor(dungeon, old_runtime_map);
        rt->dungeon_level = old_runtime_map;
        rt->outdoor = game->outdoor;
        dm2_runtime_refresh_map_transition_context(rt);
    }
    if (party_backup_ready && rt->boot && rt->boot->dm2_state) {
        DM2_V1_GameState *game = (DM2_V1_GameState *)rt->boot->dm2_state;
        rt->source_party = party_backup;
        game->party_x = old_party_x;
        game->party_y = old_party_y;
        game->party_dir = old_party_dir;
        rt->view_dir = old_party_dir & 3;
        dm2_runtime_refresh_map_transition_context(rt);
    }
    free(dungeon_backup);
    free(caii_backup);
    return 0;
}

/*
 * dm2_runtime_move_record_rotate_timer — bounded party-sentinel owner for
 * c_tim 0x5D (c_tim_proc.cpp:4230-4248).
 *
 * The source passes 0xFFFF through DM2_MOVE_RECORD_TO, then rotates the
 * party only after the move returns.  The shared moverec primitive already
 * owns the party-sentinel no-chain case; this handler supplies the runtime
 * party/map owner and keeps cross-map and actuator tails closed.
 */
static int dm2_runtime_move_record_rotate_timer(
    void *user, const DM2_V1_SourceTimer *timer,
    uint16_t source_index, DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *game;
    DM2_V1_MoveRecordToReceipt move_receipt;
    uint16_t packed;
    int map, x, y, dir, raw, tile_class, old_dir, turns;
    int old_x, old_y, old_view_dir;
    DM2_V1_Party party_backup;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_RecordPoolSet pool_backup;
    uint8_t *dungeon_backup = NULL;
    int backup_ready = 0;

    (void)source_index;
    memset(&pool_backup, 0, sizeof(pool_backup));
    if (receipt) receipt->handler_rejected_count++;
    if (!rt || !timer || !rt->boot || !rt->boot->dungeon_data ||
        !rt->boot->dm2_state || !rt->source_party_valid ||
        !rt->record_pools_valid || timer->type != 0x5du)
        return 0;

    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    game = (DM2_V1_GameState *)rt->boot->dm2_state;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    packed = (uint16_t)timer->value_a;
    x = (int)(packed & 0x1fu);
    y = (int)((packed >> 5) & 0x1fu);
    dir = (int)((packed >> 10) & 0x3u);
    if (map != rt->dungeon_level || map != game->current_level ||
        map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map])
        return 0;

    raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    tile_class = raw < 0 ? -1 :
        dm2_runtime_normalize_square_type_for_dungeon(
            dungeon, dm2_runtime_square_type_at(dungeon, map, x, y, raw), raw);
    /* Special squares have their own MOVE_RECORD_TO/actuator owners.  Do not
     * move the party onto one and silently skip its source consequence. */
    if (raw < 0 || (tile_class != DM2_SQUARE_FLOOR &&
                    tile_class != DM2_SQUARE_FLOOR_ORNATE))
        return 0;
    /* c_moverec's party sentinel is still a movement onto the decoded
     * destination.  A live DB4 there is a source collision, not an empty
     * party cell; reject it before cloning or mutating either mirror. */
    if (dm2_v1_get_creature_at(&rt->record_pools, dungeon, map, x, y) !=
        DM2_V1_RECORD_HANDLE_NULL)
        return 0;

    old_x = game->party_x;
    old_y = game->party_y;
    old_dir = game->party_dir & 3;
    old_view_dir = rt->view_dir;
    if (old_x < 0 || old_y < 0 ||
        old_x >= dungeon->level_widths[map] ||
        old_y >= dungeon->level_heights[map] ||
        rt->source_party.heros_in_party < 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES)
        return 0;
    party_backup = rt->source_party;
    queue_backup = rt->timer_queue;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools) ||
        dungeon->raw_size <= 0 || !dungeon->raw_data ||
        !(dungeon_backup = (uint8_t *)malloc((size_t)dungeon->raw_size)))
        goto rotate_rollback;
    memcpy(dungeon_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
    backup_ready = 1;
    memset(&move_receipt, 0, sizeof(move_receipt));
    if (!dm2_v1_move_record_to(
            &rt->record_pools, dungeon, &rt->timer_queue,
            (int16_t)0xffff, (int16_t)old_x, (int16_t)old_y,
            (int16_t)x, (int16_t)y, (int16_t)dir, map,
            (uint32_t)rt->tick_count, &move_receipt) ||
        !move_receipt.valid || !move_receipt.is_party_move ||
        move_receipt.fail_closed) {
        goto rotate_rollback;
    }

    turns = (dir - old_dir) & 3;
    for (int i = 0; i < rt->source_party.heros_in_party; ++i) {
        rt->source_party.hero[i].partypos = (int8_t)(
            (rt->source_party.hero[i].partypos + turns) & 3);
        rt->source_party.hero[i].absdir = (int8_t)(
            (rt->source_party.hero[i].absdir + turns) & 3);
    }
    rt->source_party.absdir = (int16_t)dir;
    game->party_x = x;
    game->party_y = y;
    game->party_dir = dir;
    rt->view_dir = dir;
    dm2_runtime_refresh_map_transition_context(rt);
    dm2_v1_record_pool_set_free(&pool_backup);
    free(dungeon_backup);
    if (receipt && receipt->handler_rejected_count > 0)
        receipt->handler_rejected_count--;
    return 1;

rotate_rollback:
    if (backup_ready) {
        rt->timer_queue = queue_backup;
        if (dungeon_backup && dungeon->raw_data)
            memcpy(dungeon->raw_data, dungeon_backup,
                   (size_t)dungeon->raw_size);
        if (pool_backup.valid) {
            dm2_v1_record_pool_set_free(&rt->record_pools);
            rt->record_pools = pool_backup;
            memset(&pool_backup, 0, sizeof(pool_backup));
        }
        rt->source_party = party_backup;
        game->party_x = old_x;
        game->party_y = old_y;
        game->party_dir = old_dir;
        rt->view_dir = old_view_dir;
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(dungeon_backup);
    return 0;
}

/*
 * dm2_runtime_alloc_new_creature_timer — runtime owner for c_tim 0x5E.
 *
 * This is the source-shaped direct free-slot path from c_tim_proc.cpp:
 * xA/yA are the spawn tile, B is the creature type, health uses multiplier
 * seven, direction follows RANDDIR/CALC_VECTOR_DIR, and the new DB4 is
 * appended before CAII/0a48 initialisation.  The runtime deliberately does
 * not open the source recycler, cross-map placement or a missing AI/GDAT
 * owner.
 */
static int dm2_runtime_alloc_new_creature_timer(
    void *user,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    const DM2_V1_GameLoadRuntimeSessionCandidate *source;
    const DM2_AIDefinition *ai = NULL;
    const DM2_V1_AssetLoader *asset_loader = NULL;
    DM2_V1_DungeonData *dungeon = NULL;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SoundQueueState sound_backup;
    DM2_V1_SoundQueueState sound_state;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;
    DM2_V1_CaiiMoverecActivationReceipt caii_receipt;
    DM2_V1_CreatureSomethingReceipt something;
    DM2_V1_DropRng rng_backup;
    uint8_t *dungeon_backup = NULL;
    uint8_t *caii_backup = NULL;
    uint8_t *record;
    const uint8_t *animation = NULL;
    int16_t adj[2] = {0, 0};
    int16_t record_handle;
    int16_t head;
    int map;
    int x;
    int y;
    int type;
    int dir;
    int dx;
    int dy;
    int adx;
    int ady;
    int base_hp;
    int hp;
    int queue_backup_ready = 0;
    int sound_backup_ready = 0;
    size_t caii_bytes;

    (void)source_index;
    if (receipt) receipt->handler_rejected_count++;
    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&sound_backup, 0, sizeof(sound_backup));
    memset(&sound_state, 0, sizeof(sound_state));
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    memset(&caii_receipt, 0, sizeof(caii_receipt));
    memset(&something, 0, sizeof(something));

    if (!rt || !timer) return 0;
    rng_backup = rt->drop_rng;

    source = rt->game_load_candidate;
    dungeon = rt->boot ? (DM2_V1_DungeonData *)rt->boot->dungeon_data : NULL;
    asset_loader = source && source->asset_loader
        ? source->asset_loader
        : (rt->boot ? dm2_v1_boot_asset_loader(rt->boot) : NULL);
    if (!rt->boot || !dungeon || !rt->record_pools_valid ||
        !rt->caii_ready || !rt->sound_queue_ready || timer->type != 0x5eu ||
        !asset_loader ||
        (source && source->asset_loader &&
         !dm2_v1_caii_source_owner_ai_spec_def(
             &source->caii_source, (int)(uint8_t)timer->value_b, &ai)) ||
        (!ai && !dm2_v1_creature_ai_spec_def(
            (int)(uint8_t)timer->value_b, &ai)) ||
        !ai || ai->BaseHP == 0u) {
        goto alloc_creature_rollback;
    }
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    type = (int)(uint8_t)timer->value_b;
    if (map != rt->dungeon_level || map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map]) {
        goto alloc_creature_rollback;
    }
    /* ALLOC_NEW_CREATURE places a new DB4 on the requested cell.  A live
     * creature already rooted there is a source collision; reject before the
     * RNG, pool clone or any CAII/timer mutation can become observable. */
    if (dm2_v1_get_creature_at(&rt->record_pools, dungeon, map, x, y) !=
        DM2_V1_RECORD_HANDLE_NULL)
        goto alloc_creature_rollback;
    if (map == rt->dungeon_level &&
        x == dm2_v1_runtime_get_party_x() &&
        y == dm2_v1_runtime_get_party_y())
        goto alloc_creature_rollback;

    queue_backup = rt->timer_queue;
    queue_backup_ready = 1;
    caii_bytes = (size_t)rt->caii.capacity * DM2_V1_CAII_SLOT_SIZE;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools) ||
        dungeon->raw_size <= 0 || !dungeon->raw_data ||
        !(dungeon_backup = (uint8_t *)malloc((size_t)dungeon->raw_size)) ||
        (caii_bytes != 0u &&
         !(caii_backup = (uint8_t *)malloc(caii_bytes)))) {
        goto alloc_creature_rollback;
    }
    memcpy(dungeon_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
    if (caii_bytes != 0u) memcpy(caii_backup, rt->caii.slots, caii_bytes);
    sound_backup = rt->sound_queue;
    sound_backup_ready = 1;

    /* c_tim_proc.cpp:4253-4268 — RANDDIR, then vector direction. */
    dir = (int)dm2_v1_drops_randdir(&rt->drop_rng);
    if (dir == 0) {
        dir = (int)dm2_v1_drops_randdir(&rt->drop_rng);
    } else {
        dx = x - dm2_v1_runtime_get_party_x();
        dy = y - dm2_v1_runtime_get_party_y();
        adx = dx < 0 ? -dx : dx;
        ady = dy < 0 ? -dy : dy;
        if (adx == ady) {
            if (dm2_v1_drops_randbit(&rt->drop_rng) == 0u) ++ady;
            else ++adx;
        }
        dir = adx >= ady ? (dx <= 0 ? 1 : 3) : (dy <= 0 ? 2 : 0);
    }
    dir &= 3;
    base_hp = (7 * (int)ai->BaseHP) >> 3;
    hp = base_hp + (int)dm2_v1_drops_rand16(
        &rt->drop_rng, (uint16_t)((base_hp >> 3) + 1));
    if (hp <= 0 || hp > 0xffff) goto alloc_creature_rollback;

    record_handle = dm2_v1_record_pool_alloc_new_record(
        &rt->record_pools, 4u);
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             record_handle);
    if (record_handle < 0 || !record ||
        rt->record_pools.pools[4].record_size < 16) {
        /* Source recycler is deliberately not inferred from a failed slot. */
        goto alloc_creature_rollback;
    }
    record[2] = 0xfeu; record[3] = 0xffu;
    record[4] = (uint8_t)type;
    record[5] = 0xffu;
    record[6] = (uint8_t)hp; record[7] = (uint8_t)(hp >> 8);
    record[8] = 0xffu; record[9] = 0xffu;
    record[10] = 0u; record[11] = 0u;
    record[12] = 0u; record[13] = 0u;
    record[14] = 0u;
    record[15] = (uint8_t)(0xf8u | (uint8_t)dir);
    head = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (!dm2_v1_record_pool_append_to_list(&rt->record_pools, &head,
                                           record_handle) ||
        dm2_v1_dungeon_set_first_thing(dungeon, map, x, y,
                                       (uint16_t)head) != 0) {
        goto alloc_creature_rollback;
    }
    if (!dm2_v1_caii_moverec_activation(
            &rt->record_pools, dungeon, &rt->caii, &rt->timer_queue,
            map, (unsigned long)rt->tick_count, record_handle, x, y,
            &caii_receipt) || !caii_receipt.valid) {
        goto alloc_creature_rollback;
    }
    if (record[5] != 0xffu && (int)record[5] < rt->caii.capacity) {
        uint8_t *slot = rt->caii.slots +
            (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
        adj[0] = (int16_t)((uint16_t)slot[8] | ((uint16_t)slot[9] << 8));
        adj[1] = (int16_t)((uint16_t)slot[10] | ((uint16_t)slot[11] << 8));
    }
    if (dm2_v1_creature_something_1c9a_0a48_with_ai_spec(
            &rt->record_pools, &rt->caii, asset_loader, ai,
            &rt->drop_rng, record_handle, adj, &animation, map,
            source ? source->source_party_map : map, 0, 0, 0, x, y,
            (unsigned long)rt->tick_count, &something) < 0 ||
        !something.valid) {
        goto alloc_creature_rollback;
    }
    if (something.noise_would_queue) {
        dm2_v1_sound_queue_state_init(&sound_state, 0u);
        if (!dm2_v1_sound_queue_bind_entries(
                &sound_state, rt->source_sound_entries,
                rt->sound_queue.ssound_count,
                rt->sound_queue.ssound_capacity)) {
            goto alloc_creature_rollback;
        }
        sound_state.positional_count = rt->sound_queue.positional_count;
        sound_state.immediate_count = rt->sound_queue.immediate_count;
        sound_state.sound_enabled = rt->sound_queue.sound_enabled;
        sound_state.master_sfx_volume = rt->sound_queue.master_sfx_volume;
        memcpy(sound_state.positional, rt->sound_queue.positional,
               sizeof(sound_state.positional));
        memcpy(sound_state.immediate, rt->sound_queue.immediate,
               sizeof(sound_state.immediate));
        memcpy(sound_state.delayed, rt->sound_queue.delayed,
               sizeof(sound_state.delayed));
        memcpy(sound_state.sample_slots, rt->sound_queue.sample_slots,
               sizeof(sound_state.sample_slots));
        sound_env = rt->sound_env;
        sound_env.current_map = (int16_t)map;
        sound_env.gametick = rt->tick_count;
        if (!dm2_v1_sound_queue_noise_gen1(
                &sound_state, 0x0f, (int8_t)type,
                (int8_t)something.noise_index, 0x46, 0x80,
                (int16_t)x, (int16_t)y, 1, &sound_env, &sound_receipt))
            goto alloc_creature_rollback;
        rt->sound_queue = sound_state;
    }

    dm2_v1_record_pool_set_free(&pool_backup);
    free(dungeon_backup);
    free(caii_backup);
    if (receipt && receipt->handler_rejected_count > 0)
        receipt->handler_rejected_count--;
    return 1;

alloc_creature_rollback:
    if (queue_backup_ready) rt->timer_queue = queue_backup;
    if (caii_backup && rt->caii.slots)
        memcpy(rt->caii.slots, caii_backup, caii_bytes);
    if (dungeon_backup && dungeon && dungeon->raw_data)
        memcpy(dungeon->raw_data, dungeon_backup, (size_t)dungeon->raw_size);
    if (pool_backup.valid) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backup_ready) rt->sound_queue = sound_backup;
    rt->drop_rng = rng_backup;
    free(dungeon_backup);
    free(caii_backup);
    return 0;
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

    if (!timer) return 0;
    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = dm2_runtime_source_record_for_timer(rt, timer, record_id,
                                                 &type, &size);
    if (!record || size < 5) return 0;

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

    if (!timer) return 0;
    record_id = (uint16_t)(timer->value_a & 0xffffu);
    record = dm2_runtime_source_record_for_timer(rt, timer, record_id,
                                                 &type, &size);
    if (!record || size < 3) return 0;

    record[2] |= 0x01u;
    return 1;
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
    int link_type = -1;
    int link_size = 0;
    uint8_t *door_record;
    uint16_t door_attributes;

    rt->door_step_timers++;

    /* The source loop consumes the timer even when the map state is
     * unavailable. */
    if (timer == NULL || rt == NULL || rt->boot == NULL ||
        rt->boot->dungeon_data == NULL || !rt->record_pools_valid) {
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
    square_class = dm2_v1_dungeon_get_square_type(dungeon, map, x, y);
    if (square_class != DM2_V1_SQUARE_CLASS_DOOR) {
        if (receipt != NULL) {
            receipt->handler_rejected_count++;
        }
        return 1;
    }

    raw = (uint16_t)dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    door_record = dm2_runtime_source_record_for_timer(
        rt, timer, (uint16_t)timer->value_b, &link_type, &link_size);
    if (!dungeon->record_graph_complete || link_type != 0 || link_size < 4 ||
        dm2_v1_record_handle_pool((int16_t)(uint16_t)timer->value_b) != 0 ||
        dm2_v1_dungeon_get_first_thing(dungeon, map, x, y) !=
            (int)(uint16_t)timer->value_b ||
        dm2_v1_dungeon_find_thing_of_type(
            dungeon, (uint16_t)timer->value_b, 4, 64) >= 0 ||
        (map == rt->dungeon_level &&
         x == dm2_v1_runtime_get_party_x() &&
         y == dm2_v1_runtime_get_party_y())) {
        if (receipt != NULL) receipt->handler_rejected_count++;
        return 1;
    }
    current_state = dm2_door_get_state(raw);

    /* TIMELINE.C:750 — DESTROYED is sticky; the source returns immediately. */
    if (current_state == DM2_DOOR_STATE_DESTROYED) {
        return 1;
    }

    /* c_tim_proc::DM2_STEP_DOOR receives the original direction in actor;
     * valueB is the direct DB0 door handle.  Keep both source fields
     * separate: treating the handle's low bit as direction was a host-side
     * shortcut. */
    direction = (int)timer->actor;
    if (direction > 1) {
        if (receipt != NULL) receipt->handler_rejected_count++;
        return 1;
    }

    new_state = dm2_door_apply_toggle_step(current_state, direction);

    /* Write the mutated square back.  Preserve all upper bits and only
     * change the lower 3-bit state. */
    raw = dm2_door_set_state(raw, new_state);
    if (dm2_v1_dungeon_set_tile_raw(
            dungeon, map, x, y, raw) != 0) {
        if (receipt != NULL) {
            receipt->handler_rejected_count++;
        }
        return 1;
    }
    reached_target = (direction == DM2_DOOR_TOGGLE_DIR_OPEN &&
                      new_state == DM2_DOOR_STATE_OPEN) ||
                     (direction == DM2_DOOR_TOGGLE_DIR_CLOSE &&
                      new_state == DM2_DOOR_STATE_CLOSED);
    door_attributes = (uint16_t)door_record[2] |
        ((uint16_t)door_record[3] << 8);
    door_attributes = (uint16_t)(door_attributes & ~(uint16_t)0x1e00u);
    door_attributes = (uint16_t)(door_attributes |
        ((uint16_t)direction << 9));
    if (!reached_target) door_attributes = (uint16_t)(door_attributes | 0x1400u);
    door_record[2] = (uint8_t)door_attributes;
    door_record[3] = (uint8_t)(door_attributes >> 8);
    rt->door_step_mutations++;

    /* Re-queue the next step timer if we have not reached the target state.
     * The source DM2_STEP_DOOR does this via DM2_QUEUE_TIMER after
     * incrementing its data word; we schedule one tick later using
     * receipt->game_tick. */
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
    /* The source c_tim_proc actuator index is the byte-map class
     * (mapdat.map[x][y] >> 5).  Towns/2-byte dungeon words use a different
     * normalized tile encoding; treating their low five bits as an actuator
     * class can dispatch the wrong DB3/DB14 handler.  Keep this boundary
     * fail-closed until a source-owned 2-byte map-class adapter exists. */
    if (((const DM2_V1_DungeonData *)rt->boot->dungeon_data)->square_bytes != 1) {
        return -1;
    }
    return dm2_v1_dungeon_get_square_type(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data, map, x, y);
}

/* Source-owned 0x04 wall/floor mecha boundaries.
 * c_tim_proc.cpp dispatches by target square class and passes Value2 and
 * ActionType into ACTUATE_*_MECHA. The complete source walkers own the
 * record-chain admission and mutations; this adapter only supplies the live
 * session owners and decodes the timer fields. */
static int dm2_runtime_actuate_wall_mecha_timer(
    void *user, const DM2_V1_SourceTimer *timer, uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_ActuatorEventReceipt event;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SourceTimerQueue queue_backup;
    uint8_t *raw_backup = NULL;
    int backup_ready = 0;
    int map, x, y, action, direction;

    (void)source_index;
    (void)receipt;
    if (!rt || !timer || !rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid) {
        return 1;
    }
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    direction = (int)(uint8_t)(timer->value_b & 0xff);
    action = (int)(uint8_t)((timer->value_b >> 8) & 0xff);
    memset(&pool_backup, 0, sizeof(pool_backup));
    queue_backup = rt->timer_queue;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools) ||
        !rt->boot->dungeon_data ||
        ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size <= 0 ||
        !((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data ||
        !(raw_backup = (uint8_t *)malloc(
            (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size)))
        goto wall_mecha_reject;
    memcpy(raw_backup, ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data,
           (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size);
    backup_ready = 1;
    memset(&event, 0, sizeof(event));
    (void)dm2_v1_actuate_wall_mecha(
        &rt->record_pools, (DM2_V1_DungeonData *)rt->boot->dungeon_data,
        rt->caii_ready ? &rt->caii : NULL, &rt->timer_queue,
        map, x, y, action, direction, rt->tick_count,
        NULL, 0, NULL, NULL, &event);
    if (!event.valid || event.fail_closed != 0) {
        goto wall_mecha_reject;
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    rt->actuator_tile_wall_mecha++;
    return 1;

wall_mecha_reject:
    if (backup_ready) {
        rt->timer_queue = queue_backup;
        memcpy(((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data,
               raw_backup, (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size);
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    return 0;
}

static int dm2_runtime_actuate_floor_mecha_timer(
    void *user, const DM2_V1_SourceTimer *timer, uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_ActuatorEventReceipt event;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SourceTimerQueue queue_backup;
    uint8_t *raw_backup = NULL;
    int backup_ready = 0;
    int map, x, y, action, direction;

    (void)source_index;
    (void)receipt;
    if (!rt || !timer || !rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid) {
        return 1;
    }
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    direction = (int)(uint8_t)(timer->value_b & 0xff);
    action = (int)(uint8_t)((timer->value_b >> 8) & 0xff);
    memset(&pool_backup, 0, sizeof(pool_backup));
    queue_backup = rt->timer_queue;
    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools) ||
        !rt->boot->dungeon_data ||
        ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size <= 0 ||
        !((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data ||
        !(raw_backup = (uint8_t *)malloc(
            (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size)))
        goto floor_mecha_reject;
    memcpy(raw_backup, ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data,
           (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size);
    backup_ready = 1;
    memset(&event, 0, sizeof(event));
    (void)dm2_v1_actuate_floor_mecha(
        &rt->record_pools, (DM2_V1_DungeonData *)rt->boot->dungeon_data,
        rt->caii_ready ? &rt->caii : NULL, &rt->timer_queue,
        map, x, y, action, direction, rt->tick_count,
        NULL, 0, NULL, NULL, &event);
    if (!event.valid || event.fail_closed != 0) {
        goto floor_mecha_reject;
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    rt->floor_mecha_timers++;
    return 1;

floor_mecha_reject:
    if (backup_ready) {
        rt->timer_queue = queue_backup;
        memcpy(((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_data,
               raw_backup, (size_t)((DM2_V1_DungeonData *)rt->boot->dungeon_data)->raw_size);
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    rt->actuator_tile_pitfall_rejected++;
    return 0;
}

/* Source-owned class-4 door actuator.  This is the post-GAME_LOAD analogue
 * of the candidate owner in dm2_v1_game_load_world_owner.c: admission is by
 * the direct DB0 root and the complete ground-stack chain, then the source
 * 0x01 animation step is queued.  A coordinate-only door toggle is not a
 * valid substitute for this transaction. */
static int dm2_runtime_actuate_door_mecha_timer(
    void *user, const DM2_V1_SourceTimer *timer, uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimer next;
    DM2_V1_SourceTimerQueue queue_before;
    uint8_t *door_record;
    int map, x, y, raw_tile, action, direction;
    int16_t link, door_link, next_link;
    int chain_limit = 0, chain_count = 0;
    uint16_t attributes_before, attributes_after;

    (void)receipt;
    if (!rt) {
        return 1;
    }
    if (!timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->record_pools.valid ||
        !rt->record_pools.record_graph_complete) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    action = (int)(uint8_t)((timer->value_b >> 8) & 0xff);
    if (map < 0 || map >= dungeon->level_count || action > 2 ||
        (raw_tile = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y)) < 0 ||
        ((unsigned int)raw_tile >> 5) != 4u) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    link = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (link == DM2_V1_RECORD_HANDLE_NULL || link == DM2_V1_RECORD_HANDLE_END ||
        dm2_v1_record_handle_pool(link) != DM2_DB_DOOR ||
        !dm2_v1_record_pool_address(&rt->record_pools, link)) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    door_link = link;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        const DM2_V1_RecordPool *pool = &rt->record_pools.pools[db];
        if (pool->record_count < 0 || pool->extension_count < 0 ||
            chain_limit > INT_MAX - pool->record_count - pool->extension_count) {
            rt->actuator_tile_door_rejected++;
            return 1;
        }
        chain_limit += pool->record_count + pool->extension_count;
    }
    if (chain_limit <= 0) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    while (link != DM2_V1_RECORD_HANDLE_END) {
        if (link == DM2_V1_RECORD_HANDLE_NULL || chain_count++ >= chain_limit ||
            !dm2_v1_record_pool_address(&rt->record_pools, link) ||
            !dm2_v1_record_pool_next_link(&rt->record_pools, link, &next_link) ||
            dm2_v1_record_handle_pool(link) == DM2_DB_CREATURE) {
            rt->actuator_tile_door_rejected++;
            return 1;
        }
        link = next_link;
    }
    door_record = dm2_v1_record_pool_address_mut(&rt->record_pools, door_link);
    if (!door_record) {
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    attributes_before = (uint16_t)door_record[2] |
                        ((uint16_t)door_record[3] << 8);
    if (dm2_door_get_state((uint16_t)raw_tile) == DM2_DOOR_STATE_DESTROYED) {
        rt->actuator_tile_door++;
        return 1; /* source-consumed no-op */
    }
    direction = action == 0 ? 0 : action == 1 ? 1 :
        (dm2_door_get_state((uint16_t)raw_tile) == DM2_DOOR_STATE_OPEN ? 1 : 0);
    memset(&next, 0, sizeof(next));
    next.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                         (dm2_v1_source_timer_tick(timer) &
                          DM2_V1_SOURCE_TIMER_TICK_MASK);
    next.type = 0x01u;
    next.actor = (uint8_t)direction;
    next.value_a = (int16_t)((uint8_t)x | ((uint16_t)(uint8_t)y << 8));
    next.value_b = door_link;
    queue_before = rt->timer_queue;
    if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &next, source_index) !=
        DM2_V1_SOURCE_TIMER_OK) {
        rt->timer_queue = queue_before;
        rt->actuator_tile_door_rejected++;
        return 1;
    }
    attributes_after = (uint16_t)((attributes_before & ~(uint16_t)0x1e00u) |
                                   ((uint16_t)direction << 9) | 0x1400u);
    door_record[2] = (uint8_t)attributes_after;
    door_record[3] = (uint8_t)(attributes_after >> 8);
    rt->actuator_tile_door++;
    if (attributes_after != attributes_before) {
        rt->actuator_tile_door_mutations++;
    }
    return 1;
}

/*
 * dm2_runtime_process_0c_timer — 0x0C timer handler.
 * Source: SKProject/SKULLWIN/c_tim_proc.cpp:25-31 DM2_PROCESS_TIMER_0C.
 *
 * The GAME_LOAD commit now owns the source-sized c_party/c_hero copy in
 * rt->source_party.  This narrow operation can therefore use the original
 * 16-bit fields without touching the byte-sized presentation snapshot.
 */
static int dm2_runtime_process_0c_timer(void *user,
                                        const DM2_V1_SourceTimer *timer,
                                        uint16_t source_index,
                                        DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *game;
    DM2_V1_Hero *hero;
    int map;
    int hero_index;

    (void)source_index;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->boot->dm2_state) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    hero_index = (int)timer->actor;
    if (map < 0 || map >= dungeon->level_count ||
        hero_index < 0 || hero_index >= rt->source_party.heros_in_party ||
        hero_index >= DM2_MAX_HEROES) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }

    /* DM2_PROCEED_TIMERS performs DM2_CHANGE_CURRENT_MAP_TO before the
     * type dispatch. Keep the runtime's active map and its source material
     * context aligned with that source transition. */
    game = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (rt->dungeon_level != map) {
        game->current_level = map;
        game->outdoor = dm2_v1_dungeon_is_outdoor(dungeon, map);
        rt->dungeon_level = map;
        rt->outdoor = game->outdoor;
        dm2_runtime_refresh_map_transition_context(rt);
    }

    hero = &rt->source_party.hero[hero_index];
    hero->timeridx = -1;
    if (hero->curHP != 0)
        hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x0800u);

    return 1;
}

/*
 * dm2_runtime_resurrection_timer — 0x0D timer handler.
 * Source: c_tim_proc.cpp:39-124 DM2_PROCESS_TIMER_RESURRECTION.
 * Phase zero is bound to the transferred source-sized c_hero. Phases one
 * and two remain rejected until their altar-record cut and DB15 cloud
 * transaction can be committed atomically with the runtime owners.
 *
 * Source: SKProject/SKULLWIN/c_tim_proc.cpp:39-124
 *         SKProject/SKULLWIN/c_hero.h:40-130, 916-953
 */
static int dm2_runtime_resurrection_timer(void *user,
                                          const DM2_V1_SourceTimer *timer,
                                          uint16_t source_index,
                                          DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_GameState *game;
    DM2_V1_Hero *hero;
    int map;
    int hero_index;
    uint8_t phase;
    int16_t new_max;
    int x;
    int y;
    int16_t cursor;
    int16_t next;
    int16_t found_record = DM2_V1_RECORD_HANDLE_NULL;
    int16_t dungeon_next = DM2_V1_RECORD_HANDLE_END;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SoundQueueState sound_backup;
    uint8_t *raw_backup = NULL;
    int pool_cloned = 0;
    int sound_backed_up = 0;
    DM2_V1_SourceTimer cloud_timer;

    memset(&pool_backup, 0, sizeof(pool_backup));
    memset(&sound_backup, 0, sizeof(sound_backup));

    (void)source_index;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->boot->dm2_state) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    hero_index = (int)timer->actor;
    phase = (uint8_t)((uint16_t)timer->value_b >> 8);
    if (map < 0 || map >= dungeon->level_count ||
        hero_index < 0 || hero_index >= rt->source_party.heros_in_party ||
        hero_index >= DM2_MAX_HEROES || phase > 2u) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }

    /* DM2_PROCEED_TIMERS changes to the timer's map before entering any
     * resurrection phase, including the altar-record phase. */
    game = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (rt->dungeon_level != map) {
        game->current_level = map;
        game->outdoor = dm2_v1_dungeon_is_outdoor(dungeon, map);
        rt->dungeon_level = map;
        rt->outdoor = game->outdoor;
        dm2_runtime_refresh_map_transition_context(rt);
    }

    if (phase == 2u) {
        int direction;
        int16_t head;
        int16_t chain_link;
        int16_t next_link;
        int16_t cloud_record;
        int16_t dungeon_last = DM2_V1_RECORD_HANDLE_NULL;
        int chain_limit = 0;
        int chain_count = 0;
        int x = (int)(int8_t)(timer->value_a & 0xff);
        int y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
        uint8_t *cloud;
        uint8_t *dungeon_cloud;
        DM2_V1_SoundQueueEnv sound_env;
        DM2_V1_SoundQueueReceipt sound_receipt;
        uint32_t phase_game_tick = receipt ? receipt->game_tick :
            (uint32_t)rt->tick_count;

        direction = (int)(int8_t)(uint8_t)((uint16_t)timer->value_b & 0xffu);
        if (!rt->record_pools_valid || !rt->sound_queue_ready ||
            !dungeon->record_graph_complete || !dungeon->raw_data ||
            dungeon->raw_size <= 0 || x < 0 || y < 0 ||
            x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map])
            goto resurrection_phase2_reject;
        head = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, x, y);
        for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
            const DM2_V1_RecordPool *pool = &rt->record_pools.pools[db];
            if (pool->record_count < 0 || pool->extension_count < 0 ||
                chain_limit > INT_MAX - pool->record_count -
                    pool->extension_count)
                goto resurrection_phase2_reject;
            chain_limit += pool->record_count + pool->extension_count;
        }
        if (chain_limit <= 0) goto resurrection_phase2_reject;
        chain_link = head;
        while (chain_link != DM2_V1_RECORD_HANDLE_END) {
            const uint8_t *record;
            if (chain_link == DM2_V1_RECORD_HANDLE_NULL ||
                chain_count++ >= chain_limit ||
                !(record = dm2_v1_record_pool_address(
                    &rt->record_pools, chain_link)) ||
                !dm2_v1_record_pool_next_link(
                    &rt->record_pools, chain_link, &next_link))
                goto resurrection_phase2_reject;
            if (dm2_v1_record_handle_pool(chain_link) == 3)
                goto resurrection_phase2_reject;
            (void)record;
            dungeon_last = chain_link;
            chain_link = next_link;
        }
        if (!dm2_v1_record_pool_set_clone(&pool_backup,
                &rt->record_pools))
            goto resurrection_phase2_reject;
        pool_cloned = 1;
        sound_backup = rt->sound_queue;
        sound_backed_up = 1;
        raw_backup = (uint8_t *)malloc((size_t)dungeon->raw_size);
        if (!raw_backup) goto resurrection_phase2_rollback;
        memcpy(raw_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
        cloud_record = dm2_v1_record_pool_alloc_new_record(
            &rt->record_pools, 15u);
        cloud = dm2_v1_record_pool_address_mut(
            &rt->record_pools, cloud_record);
        dungeon_cloud = (uint8_t *)(uintptr_t)
            dm2_v1_dungeon_get_thing_record(
                dungeon, (uint16_t)cloud_record, NULL, NULL, NULL);
        if (cloud_record < 0 || !cloud ||
            rt->record_pools.pools[15].record_size < 4)
            goto resurrection_phase2_rollback;
        cloud[0] = 0xfeu; cloud[1] = 0xffu;
        cloud[2] = (uint8_t)(0x64u | (direction == 0xff ? 0x80u : 0u));
        cloud[3] = 0u;
        if (dungeon_cloud) {
            dungeon_cloud[0] = cloud[0]; dungeon_cloud[1] = cloud[1];
            dungeon_cloud[2] = cloud[2]; dungeon_cloud[3] = cloud[3];
        }
        if (!dm2_v1_record_pool_append_to_list(
                &rt->record_pools, &head, cloud_record))
            goto resurrection_phase2_rollback;
        if (dungeon_last < 0) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, map, x, y, (uint16_t)cloud_record) != 0)
                goto resurrection_phase2_rollback;
        } else {
            uint8_t *last = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)dungeon_last, NULL, NULL, NULL);
            if (!last) goto resurrection_phase2_rollback;
            last[0] = (uint8_t)cloud_record;
            last[1] = (uint8_t)((uint16_t)cloud_record >> 8);
        }
        memset(&sound_env, 0, sizeof(sound_env));
        sound_env.current_map = (int16_t)map;
        sound_env.gate_map_a = rt->sound_env.gate_map_a;
        sound_env.gate_map_b = rt->sound_env.gate_map_b;
        sound_env.facing = (uint16_t)rt->view_dir;
        sound_env.party_x = (int16_t)dm2_v1_runtime_get_party_x();
        sound_env.party_y = (int16_t)dm2_v1_runtime_get_party_y();
        sound_env.gametick = (int32_t)phase_game_tick;
        memset(&sound_receipt, 0, sizeof(sound_receipt));
        (void)dm2_v1_sound_queue_noise_gen2(
            &rt->sound_queue, 0x0d, 0x64, (int8_t)0x81, (int8_t)0xfe,
            (int16_t)x, (int16_t)y, 1, 0x6c, 1, &sound_env,
            &sound_receipt);
        memset(&cloud_timer, 0, sizeof(cloud_timer));
        cloud_timer.ticks_and_map =
            ((uint32_t)(map & 0xff) << 24) |
            ((phase_game_tick + 5u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
        cloud_timer.type = DM2_V1_TIMER_PROCESS_CLOUD;
        cloud_timer.value_a = timer->value_a;
        cloud_timer.value_b = cloud_record;
        if (dm2_v1_source_timer_enqueue(
                &rt->timer_queue, &cloud_timer, source_index) !=
            DM2_V1_SOURCE_TIMER_OK)
            goto resurrection_phase2_rollback;
        dm2_v1_record_pool_set_free(&pool_backup);
        free(raw_backup);
        return 1;

resurrection_phase2_rollback:
        if (raw_backup)
            memcpy(dungeon->raw_data, raw_backup, (size_t)dungeon->raw_size);
        if (pool_cloned) {
            dm2_v1_record_pool_set_free(&rt->record_pools);
            rt->record_pools = pool_backup;
            memset(&pool_backup, 0, sizeof(pool_backup));
        }
        if (sound_backed_up) rt->sound_queue = sound_backup;
        free(raw_backup);
        if (receipt) receipt->handler_rejected_count++;
        return 1;

resurrection_phase2_reject:
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }

    if (phase == 1u) {
        const uint8_t *dungeon_record;
        int record_type;
        int record_size;
        int dcursor;
        int dprevious = -1;
        int dsteps;

        x = (int)(int8_t)(timer->value_a & 0xff);
        y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
        if (!rt->record_pools_valid || !dungeon->record_graph_complete ||
            !dungeon->raw_data || dungeon->raw_size <= 0 ||
            x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
            y >= dungeon->level_heights[map]) {
            if (receipt) receipt->handler_rejected_count++;
            return 1;
        }

        /* Validate the source pool chain before taking ownership of any
         * mutable bytes.  DB10 index zero is the hero-bones/altar record;
         * its word[2] high two bits select the actor. */
        cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, x, y);
        for (int step = 0; cursor >= 0 &&
             cursor != DM2_V1_RECORD_HANDLE_END && step < 65536; ++step) {
            const uint8_t *record = dm2_v1_record_pool_address(
                &rt->record_pools, cursor);
            if (!record || !dm2_v1_record_pool_next_link(
                    &rt->record_pools, cursor, &next)) {
                if (receipt) receipt->handler_rejected_count++;
                return 1;
            }
            if (dm2_v1_record_handle_pool(cursor) == 10 &&
                dm2_v1_record_handle_index(cursor) == 0 &&
                (((uint16_t)record[2] | ((uint16_t)record[3] << 8)) >> 14) ==
                    (uint16_t)hero_index) {
                found_record = cursor;
                break;
            }
            cursor = next;
        }
        if (found_record < 0) {
            if (receipt) receipt->handler_rejected_count++;
            return 1;
        }

        /* The boot dungeon retains its own authenticated record bytes for
         * rendering/queries. Verify the same chain there before mutating the
         * pool owner, otherwise the two source views could diverge. */
        dcursor = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
        for (dsteps = 0; dcursor >= 0 &&
             dcursor != DM2_V1_RECORD_HANDLE_END && dsteps < 65536;
             ++dsteps) {
            dungeon_record = dm2_v1_dungeon_get_thing_record(
                dungeon, (uint16_t)dcursor, &record_type, NULL, &record_size);
            if (!dungeon_record || record_size < 2) break;
            if (dcursor == found_record) {
                dungeon_next = (int16_t)((uint16_t)dungeon_record[0] |
                                         ((uint16_t)dungeon_record[1] << 8));
                break;
            }
            dprevious = dcursor;
            dcursor = (int16_t)((uint16_t)dungeon_record[0] |
                                ((uint16_t)dungeon_record[1] << 8));
        }
        if (dcursor != found_record) {
            if (receipt) receipt->handler_rejected_count++;
            return 1;
        }

        if (!dm2_v1_record_pool_set_clone(&pool_backup,
                &rt->record_pools)) {
            if (receipt) receipt->handler_rejected_count++;
            return 1;
        }
        pool_cloned = 1;
        raw_backup = (uint8_t *)malloc((size_t)dungeon->raw_size);
        if (!raw_backup) goto resurrection_phase1_rollback;
        memcpy(raw_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
        if (!dm2_v1_record_pool_cut_from_tile(
                &rt->record_pools, dungeon, map, x, y, found_record))
            goto resurrection_phase1_rollback;
        if (dprevious < 0) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, map, x, y, (uint16_t)dungeon_next) != 0)
                goto resurrection_phase1_rollback;
        } else {
            uint8_t *previous_record = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)dprevious, NULL, NULL, NULL);
            if (!previous_record) goto resurrection_phase1_rollback;
            previous_record[0] = (uint8_t)dungeon_next;
            previous_record[1] = (uint8_t)((uint16_t)dungeon_next >> 8);
        }
        dungeon_record = (const uint8_t *)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)found_record, NULL, NULL, NULL);
        if (!dungeon_record) goto resurrection_phase1_rollback;
        ((uint8_t *)(uintptr_t)dungeon_record)[0] = 0xffu;
        ((uint8_t *)(uintptr_t)dungeon_record)[1] = 0xffu;
        {
            uint8_t *pool_record = dm2_v1_record_pool_address_mut(
                &rt->record_pools, found_record);
            if (!pool_record) goto resurrection_phase1_rollback;
            pool_record[0] = 0xffu;
            pool_record[1] = 0xffu;
        }
        dm2_v1_record_pool_set_free(&pool_backup);
        free(raw_backup);
        return 1;

resurrection_phase1_rollback:
        if (raw_backup) {
            memcpy(dungeon->raw_data, raw_backup, (size_t)dungeon->raw_size);
        }
        if (pool_cloned) {
            dm2_v1_record_pool_set_free(&rt->record_pools);
            rt->record_pools = pool_backup;
            memset(&pool_backup, 0, sizeof(pool_backup));
        }
        free(raw_backup);
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }

    hero = &rt->source_party.hero[hero_index];
    if (hero->maxHP <= 0) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }
    new_max = (int16_t)(hero->maxHP - hero->maxHP / 64 - 1);
    if (new_max < 25) new_max = 25;
    hero->weight = 0;
    for (int i = 0; i < DM2_NUM_ITEMS; ++i)
        hero->item[i] = -1;
    hero->maxHP = new_max;
    hero->curHP = (int16_t)(new_max / 2);
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x4000u);
    hero->ench_aura = 0;
    hero->ench_power = 0;

    return 1;
}

static uint8_t *dm2_runtime_cloud_get_record(void *ctx, uint16_t record)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    if (!rt || !rt->record_pools_valid)
        return NULL;
    return dm2_v1_record_pool_address_mut(&rt->record_pools,
                                          (int16_t)record);
}

static int16_t dm2_runtime_cloud_rand16(void *ctx, int16_t n)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    if (!rt || n <= 0)
        return 0;
    return (int16_t)dm2_v1_drops_rand16(&rt->drop_rng, (uint16_t)n);
}

static bool dm2_runtime_cloud_randbit(void *ctx)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    return rt ? dm2_v1_drops_randbit(&rt->drop_rng) != 0u : false;
}

static uint16_t dm2_runtime_cloud_randdir(void *ctx)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    return rt ? dm2_v1_drops_randdir(&rt->drop_rng) : 0u;
}

static uint8_t *dm2_runtime_cloud_ai_spec_from_type(
    void *ctx, uint16_t creature_type)
{
    const DM2_AIDefinition *spec = NULL;
    (void)ctx;
    return dm2_v1_creature_ai_spec_def((int)creature_type, &spec) && spec
        ? (uint8_t *)(uintptr_t)spec : NULL;
}

static const uint8_t *dm2_runtime_cloud_ai_spec_from_type_const(
    void *ctx, uint16_t creature_type)
{
    return dm2_runtime_cloud_ai_spec_from_type(ctx, creature_type);
}

static int16_t dm2_runtime_cloud_ai_spec_flags(
    void *ctx, uint16_t creature_type)
{
    uint16_t flags = 0u;
    (void)ctx;
    return dm2_v1_creature_ai_spec_flags((int)creature_type, &flags)
        ? (int16_t)flags : 0;
}

static int16_t dm2_runtime_cloud_poison_resistance(
    void *ctx, uint16_t creature_type, uint16_t damage)
{
    DM2_V1_CreaturePoisonCallbacks poison_cb;
    poison_cb.rand_dir = dm2_runtime_cloud_randdir;
    poison_cb.query_ai_spec = dm2_runtime_cloud_ai_spec_from_type_const;
    return dm2_v1_apply_creature_poison_resistance(
        creature_type, (int16_t)damage, &poison_cb, ctx);
}

static int16_t dm2_runtime_cloud_min16(int16_t a, int16_t b)
{
    return a < b ? a : b;
}

static int16_t dm2_runtime_cloud_max16(int16_t a, int16_t b)
{
    return a > b ? a : b;
}

static int16_t dm2_runtime_cloud_max16_ctx(
    void *ctx, int16_t a, int16_t b)
{
    (void)ctx;
    return dm2_runtime_cloud_max16(a, b);
}

static int16_t dm2_runtime_cloud_wound_party(
    void *ctx, int hero_idx, int16_t damage, int body_parts,
    int damage_type)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    DM2_V1_Hero *hero;
    int32_t pending;

    (void)body_parts;
    (void)damage_type;
    if (!rt || !rt->source_party_valid ||
        rt->source_party.heros_in_party < 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES ||
        hero_idx < 0 || hero_idx >= rt->source_party.heros_in_party ||
        damage <= 0)
        return 0;
    hero = &rt->source_party.hero[hero_idx];
    if (hero->curHP == 0)
        return 0;
    pending = (int32_t)hero->damagesuffered + damage;
    if (pending > INT16_MAX)
        pending = INT16_MAX;
    hero->damagesuffered = (int16_t)pending;
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag |
                               DM2_V1_HERO_FLAG_0800);
    return damage;
}

/* DM2_PROCESS_CLOUD (0x19): advance an authenticated DB15 cloud.
 * The runtime keeps the source pool and dungeon bytes in lockstep; ordinary
 * types use their source lifecycle (including 0x07/0x28 decay), type 0x64
 * becomes 0x65 and requeues once, while terminal 0x65 is cut/deallocated.
 * Reflector subtype 0x0e is deliberately excluded: its incoming-spell/
 * bounce owner is not the ordinary PROCESS_CLOUD lifecycle. */
static int dm2_runtime_process_cloud_timer(
    void *user, const DM2_V1_SourceTimer *timer, uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SoundQueueState sound_backup;
    uint8_t *raw_backup = NULL;
    uint8_t *pool_record;
    uint8_t *dungeon_record;
    int map;
    int x;
    int y;
    int16_t cloud_record;
    int type_dungeon;
    int size_dungeon;
    uint16_t cloud_word;
    int16_t cursor;
    int16_t previous = DM2_V1_RECORD_HANDLE_NULL;
    int16_t next = DM2_V1_RECORD_HANDLE_END;
    int steps;
    int cloned = 0;
    int sound_backed_up = 0;
    int cloud_link_found = 0;
    uint8_t cloud_type;
    DM2_V1_DropRng rng_backup;
    int rng_backed_up = 0;
    DM2_V1_Party party_backup;
    int party_backed_up = 0;
    DM2_V1_SourceTimerQueue timer_queue_backup;
    uint8_t *caii_backup = NULL;
    size_t caii_backup_bytes = 0u;
    int caii_backed_up = 0;
    int timer_queue_backed_up = 0;

    (void)source_index;
    memset(&pool_backup, 0, sizeof(pool_backup));
    if (!rt || !timer || !rt->record_pools_valid || !rt->sound_queue_ready ||
        !rt->boot || !rt->boot->dungeon_data) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);
    cloud_record = timer->value_b;
    if (map < 0 || map >= dungeon->level_count || x < 0 || y < 0 ||
        x >= dungeon->level_widths[map] || y >= dungeon->level_heights[map] ||
        dm2_v1_record_handle_pool(cloud_record) != 15)
        goto cloud_runtime_reject;
    pool_record = dm2_v1_record_pool_address_mut(
        &rt->record_pools, cloud_record);
    dungeon_record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        dungeon, (uint16_t)cloud_record, &type_dungeon, NULL, &size_dungeon);
    /* Dynamic DB15 slots may have pool bytes without a separate raw-record
     * mirror.  The authoritative shared boundary is the tile-rooted chain;
     * require membership there before accepting the timer. */
    if (!pool_record || (dungeon_record && size_dungeon < 4))
        goto cloud_runtime_reject;
    cloud_word = (uint16_t)pool_record[2] |
        ((uint16_t)pool_record[3] << 8);
    cloud_type = (uint8_t)(cloud_word & 0x7fu);
    if ((cloud_type >= 8u && cloud_type != 0x28u &&
         cloud_type != 0x64u && cloud_type != 0x65u))
        goto cloud_runtime_reject;
    if (dungeon_record && ((((uint16_t)dungeon_record[2] |
          ((uint16_t)dungeon_record[3] << 8)) & 0x7fu) !=
        (cloud_word & 0x7fu)))
        goto cloud_runtime_reject;

    /* The source effect owns the complete tile-rooted chain, not merely the
     * prefix through the DB15 cloud.  A malformed tail must be rejected
     * before party/creature/door damage or a cloud requeue can mutate state;
     * the candidate GAME_LOAD owner already follows this same prewalk. */
    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    for (steps = 0; cursor >= 0 &&
         cursor != DM2_V1_RECORD_HANDLE_END &&
         cursor != DM2_V1_RECORD_HANDLE_NULL && steps < 65536; ++steps) {
        const uint8_t *record = dm2_v1_record_pool_address(
            &rt->record_pools, cursor);
        if (!record || !dm2_v1_record_pool_next_link(
                &rt->record_pools, cursor, &next))
            goto cloud_runtime_reject;
        if (cursor == cloud_record) {
            cloud_link_found = 1;
        }
        if (next == DM2_V1_RECORD_HANDLE_NULL)
            goto cloud_runtime_reject;
        cursor = next;
    }
    if (!cloud_link_found || cursor != DM2_V1_RECORD_HANDLE_END ||
        steps >= 65536)
        goto cloud_runtime_reject;

    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools))
        goto cloud_runtime_reject;
    cloned = 1;
    sound_backup = rt->sound_queue;
    sound_backed_up = 1;
    rng_backup = rt->drop_rng;
    rng_backed_up = 1;
    party_backup = rt->source_party;
    party_backed_up = rt->source_party_valid;
    if (!dungeon->raw_data || dungeon->raw_size <= 0) goto cloud_runtime_rollback;
    raw_backup = (uint8_t *)malloc((size_t)dungeon->raw_size);
    if (!raw_backup) goto cloud_runtime_rollback;
    memcpy(raw_backup, dungeon->raw_data, (size_t)dungeon->raw_size);

    /* Regular source clouds (types 0..7) are admitted only when every target
     * effect that can occur on the cell is owned by this transaction. Party
     * damage uses the source ATTACK_PARTY pending-damage owner; a creature
     * cell uses the same CAII attack owner as STEP_MISSILE, and a door cell
     * uses the DB0 fireball gate below. */
    if ((cloud_word & 0x7fu) < 8u) {
        uint8_t cloud_type = (uint8_t)(cloud_word & 0x7fu);
        int raw_tile = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
        int party_here = map == rt->dungeon_level &&
            x == dm2_v1_runtime_get_party_x() &&
            y == dm2_v1_runtime_get_party_y();
        int16_t creature_here = dm2_v1_get_creature_at(
            &rt->record_pools, dungeon, map, x, y);

        /* Source c_cloud.cpp gives types 0 and 2 an immediate no-effect
         * path after the door pass.  Other ordinary cloud types may also
         * expire on an empty floor; a floor is not an invalid target merely
         * because no party, creature or door is present. */
        if (cloud_type != 0u && cloud_type != 2u) {
            if ((party_here && creature_here != DM2_V1_RECORD_HANDLE_NULL) ||
                (party_here && (!rt->source_party_valid ||
                    rt->source_party.heros_in_party < 0 ||
                    rt->source_party.heros_in_party > DM2_MAX_HEROES)) ||
                (party_here && (dm2_cloud_type_table[cloud_type] & 0x04u) == 0u) ||
                (creature_here != DM2_V1_RECORD_HANDLE_NULL &&
                 (dm2_cloud_type_table[cloud_type] & 0x08u) == 0u))
                goto cloud_runtime_rollback;
        }

        if (creature_here != DM2_V1_RECORD_HANDLE_NULL) {
            if (!rt->caii_ready || !rt->caii.valid || !rt->caii.slots ||
                rt->caii.capacity <= 0 || rt->caii.capacity > INT_MAX /
                    DM2_V1_CAII_SLOT_SIZE)
                goto cloud_runtime_rollback;
            caii_backup_bytes = rt->caii.capacity * DM2_V1_CAII_SLOT_SIZE;
            caii_backup = (uint8_t *)malloc(caii_backup_bytes);
            if (!caii_backup) goto cloud_runtime_rollback;
            memcpy(caii_backup, rt->caii.slots, caii_backup_bytes);
            caii_backed_up = 1;
            timer_queue_backup = rt->timer_queue;
            timer_queue_backed_up = 1;
        }

        if (party_here) {
            DM2_V1_HeroAttackPartyCallbacks party_cb;
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;

            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = rt;
            damage_cb.get_address_of_record = dm2_runtime_cloud_get_record;
            damage_cb.rand16 = dm2_runtime_cloud_rand16;
            damage_cb.randbit = dm2_runtime_cloud_randbit;
            damage_cb.min16 = dm2_runtime_cloud_min16;
            damage_cb.max16 = dm2_runtime_cloud_max16;
            damage_cb.query_creature_ai_spec_from_type =
                dm2_runtime_cloud_ai_spec_from_type;
            damage_cb.query_creature_ai_spec_flags =
                dm2_runtime_cloud_ai_spec_flags;
            damage_cb.apply_creature_poison_resistance =
                dm2_runtime_cloud_poison_resistance;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, -1);
            if (damage_receipt.damage > 0) {
                memset(&party_cb, 0, sizeof(party_cb));
                party_cb.hero_count = rt->source_party.heros_in_party;
                party_cb.wound_player = dm2_runtime_cloud_wound_party;
                party_cb.rand16 = dm2_runtime_cloud_rand16;
                party_cb.max16 = dm2_runtime_cloud_max16_ctx;
                (void)dm2_v1_hero_attack_party(
                    damage_receipt.damage, 0, 0, &party_cb, rt);
            }
        }

        if ((dm2_cloud_type_table[cloud_type] & 0x02u) != 0u &&
            raw_tile >= 0 && (((unsigned int)raw_tile >> 5) & 0x7u) == 4u) {
            int16_t door_link = (int16_t)dm2_v1_dungeon_get_first_thing(
                dungeon, map, x, y);
            uint8_t *door_record;
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;
            uint16_t old_raw = (uint16_t)raw_tile;
            int door_state = dm2_door_get_state(old_raw);
            int door_type;

            if (door_link == DM2_V1_RECORD_HANDLE_NULL ||
                door_link == DM2_V1_RECORD_HANDLE_END ||
                dm2_v1_record_handle_pool(door_link) != 0 ||
                !(door_record = dm2_v1_record_pool_address_mut(
                    &rt->record_pools, door_link))) {
                goto cloud_runtime_rollback;
            }
            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = rt;
            damage_cb.get_address_of_record = dm2_runtime_cloud_get_record;
            damage_cb.rand16 = dm2_runtime_cloud_rand16;
            damage_cb.randbit = dm2_runtime_cloud_randbit;
            damage_cb.min16 = dm2_runtime_cloud_min16;
            damage_cb.max16 = dm2_runtime_cloud_max16;
            damage_cb.query_creature_ai_spec_from_type =
                dm2_runtime_cloud_ai_spec_from_type;
            damage_cb.query_creature_ai_spec_flags =
                dm2_runtime_cloud_ai_spec_flags;
            damage_cb.apply_creature_poison_resistance =
                dm2_runtime_cloud_poison_resistance;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, door_link);
            door_type = (int)((uint16_t)door_record[2] & 0x0001u);
            if (damage_receipt.damage > 0 &&
                dm2_door_check_destruction(
                    door_type, door_state, damage_receipt.damage, 1,
                    door_record[2]) == DM2_DOOR_DESTROYED_YES &&
                dm2_v1_dungeon_set_tile_raw(
                    dungeon, map, x, y,
                    dm2_door_set_state(old_raw, DM2_DOOR_STATE_DESTROYED)) != 0)
                goto cloud_runtime_rollback;
        }

        if (creature_here != DM2_V1_RECORD_HANDLE_NULL) {
            DM2_V1_CaiiAttackReceipt attack_receipt;
            DM2_V1_CloudCallbacks damage_cb;
            DM2_V1_CalcCloudDamageReceipt damage_receipt;
            memset(&damage_cb, 0, sizeof(damage_cb));
            damage_cb.ctx = rt;
            damage_cb.get_address_of_record = dm2_runtime_cloud_get_record;
            damage_cb.rand16 = dm2_runtime_cloud_rand16;
            damage_cb.randbit = dm2_runtime_cloud_randbit;
            damage_cb.min16 = dm2_runtime_cloud_min16;
            damage_cb.max16 = dm2_runtime_cloud_max16;
            damage_cb.query_creature_ai_spec_from_type =
                dm2_runtime_cloud_ai_spec_from_type;
            damage_cb.query_creature_ai_spec_flags =
                dm2_runtime_cloud_ai_spec_flags;
            damage_cb.apply_creature_poison_resistance =
                dm2_runtime_cloud_poison_resistance;
            damage_receipt = dm2_v1_calc_cloud_damage(
                &damage_cb, (uint16_t)cloud_record, creature_here);
            if (damage_receipt.damage > 0 &&
                !dm2_v1_caii_attack_creature(
                    &rt->record_pools, dungeon, &rt->caii,
                    &rt->timer_queue, &rt->drop_rng, map,
                    (unsigned long)rt->tick_count,
                    creature_here, x, y, x, y, 0x200du, 0x64,
                    damage_receipt.damage, &attack_receipt))
                goto cloud_runtime_rollback;
        }

        if (cloud_type == 7u && (cloud_word >> 8) >= 6u) {
            cloud_word = (uint16_t)((cloud_word & 0x00ffu) |
                (uint16_t)(((cloud_word >> 8) - 3u) << 8));
            pool_record[2] = (uint8_t)cloud_word;
            pool_record[3] = (uint8_t)(cloud_word >> 8);
            if (dungeon_record) {
                dungeon_record[2] = pool_record[2];
                dungeon_record[3] = pool_record[3];
            }
            {
                DM2_V1_SourceTimer next_timer = *timer;
                uint32_t tick = receipt ? receipt->game_tick :
                    (uint32_t)rt->tick_count;
                next_timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                    ((tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
                if (dm2_v1_source_timer_enqueue(
                        &rt->timer_queue, &next_timer, source_index) !=
                    DM2_V1_SOURCE_TIMER_OK)
                    goto cloud_runtime_rollback;
            }
            dm2_v1_record_pool_set_free(&pool_backup);
            free(caii_backup);
            free(raw_backup);
            return 1;
        }

        /* Types other than the decaying poison cloud expire after the
         * source door pass.  Reuse the authenticated type-0x65 cut path
         * below rather than creating a second unlink implementation. */
        cloud_word = (uint16_t)((cloud_word & 0xff80u) | 0x65u);
        pool_record[2] = (uint8_t)cloud_word;
        pool_record[3] = (uint8_t)(cloud_word >> 8);
        if (dungeon_record) {
            dungeon_record[2] = pool_record[2];
            dungeon_record[3] = pool_record[3];
        }
    }

    /* c_cloud.cpp::PROCESS_CLOUD also owns subtype 0x28.  It decays the
     * high byte by 0x28 while strength is above 0x37, without the 0x64
     * transition sound; a weak 0x28 cloud expires through the cut path. */
    if (cloud_type == 0x28u) {
        if ((cloud_word >> 8) > 0x37u) {
            cloud_word = (uint16_t)((cloud_word & 0x00ffu) |
                (uint16_t)(((cloud_word >> 8) - 0x28u) << 8));
            pool_record[2] = (uint8_t)cloud_word;
            pool_record[3] = (uint8_t)(cloud_word >> 8);
            if (dungeon_record) {
                dungeon_record[2] = pool_record[2];
                dungeon_record[3] = pool_record[3];
            }
            {
                DM2_V1_SourceTimer next_timer = *timer;
                uint32_t tick = receipt ? receipt->game_tick :
                    (uint32_t)rt->tick_count;
                next_timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                    ((tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
                if (dm2_v1_source_timer_enqueue(
                        &rt->timer_queue, &next_timer, source_index) !=
                    DM2_V1_SOURCE_TIMER_OK)
                    goto cloud_runtime_rollback;
            }
            dm2_v1_record_pool_set_free(&pool_backup);
            free(caii_backup);
            free(raw_backup);
            return 1;
        }
        cloud_word = (uint16_t)((cloud_word & 0xff80u) | 0x65u);
        pool_record[2] = (uint8_t)cloud_word;
        pool_record[3] = (uint8_t)(cloud_word >> 8);
        if (dungeon_record) {
            dungeon_record[2] = pool_record[2];
            dungeon_record[3] = pool_record[3];
        }
    }

    if ((cloud_word & 0x7fu) == 0x65u) {
        cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, x, y);
        for (steps = 0; cursor >= 0 &&
             cursor != DM2_V1_RECORD_HANDLE_END && steps < 65536; ++steps) {
            const uint8_t *record = dm2_v1_record_pool_address(
                &rt->record_pools, cursor);
            if (!record || !dm2_v1_record_pool_next_link(
                    &rt->record_pools, cursor, &next))
                goto cloud_runtime_rollback;
            if (cursor == cloud_record) break;
            previous = cursor;
            cursor = next;
        }
        if (cursor != cloud_record) goto cloud_runtime_rollback;
        if (!dm2_v1_record_pool_cut_from_tile(
                &rt->record_pools, dungeon, map, x, y, cloud_record))
            goto cloud_runtime_rollback;
        if (previous < 0) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, map, x, y, (uint16_t)next) != 0)
                goto cloud_runtime_rollback;
        } else {
            uint8_t *prior = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)previous, NULL, NULL, NULL);
            if (!prior) goto cloud_runtime_rollback;
            prior[0] = (uint8_t)next;
            prior[1] = (uint8_t)((uint16_t)next >> 8);
        }
        pool_record[0] = 0xffu; pool_record[1] = 0xffu;
        if (dungeon_record) {
            dungeon_record[0] = 0xffu; dungeon_record[1] = 0xffu;
        }
    } else {
        pool_record[2] = (uint8_t)((cloud_word & 0xff80u) | 0x65u);
        pool_record[3] = (uint8_t)(cloud_word >> 8);
        if (dungeon_record) {
            dungeon_record[2] = pool_record[2];
            dungeon_record[3] = pool_record[3];
        }
        {
            DM2_V1_SoundQueueEnv env = rt->sound_env;
            DM2_V1_SoundQueueReceipt sound_receipt;
            env.current_map = (int16_t)map;
            env.party_x = (int16_t)dm2_v1_runtime_get_party_x();
            env.party_y = (int16_t)dm2_v1_runtime_get_party_y();
            env.facing = (uint16_t)rt->view_dir;
            env.gametick = receipt ? (int32_t)receipt->game_tick : rt->tick_count;
            memset(&sound_receipt, 0, sizeof(sound_receipt));
            (void)dm2_v1_sound_queue_noise_gen2(
                &rt->sound_queue, 0x0d, 0x64, (int8_t)0x81, (int8_t)0xfe,
                (int16_t)x, (int16_t)y, 1, 0x6c, 0xc8, &env,
                &sound_receipt);
        }
        {
            DM2_V1_SourceTimer next_timer = *timer;
            uint32_t tick = receipt ? receipt->game_tick : (uint32_t)rt->tick_count;
            next_timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                ((tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
            if (dm2_v1_source_timer_enqueue(
                    &rt->timer_queue, &next_timer, source_index) !=
                DM2_V1_SOURCE_TIMER_OK)
                goto cloud_runtime_rollback;
        }
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(caii_backup);
    free(raw_backup);
    return 1;

cloud_runtime_rollback:
    if (raw_backup)
        memcpy(dungeon->raw_data, raw_backup, (size_t)dungeon->raw_size);
    if (cloned) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    if (sound_backed_up) rt->sound_queue = sound_backup;
    if (rng_backed_up) rt->drop_rng = rng_backup;
    if (party_backed_up) rt->source_party = party_backup;
    if (timer_queue_backed_up) rt->timer_queue = timer_queue_backup;
    if (caii_backed_up && caii_backup && rt->caii.slots)
        memcpy(rt->caii.slots, caii_backup, caii_backup_bytes);
    free(caii_backup);
    free(raw_backup);
cloud_runtime_reject:
    if (receipt) receipt->handler_rejected_count++;
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
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    DM2_V1_ProcessItemBonusReceipt receipt;
    (void)mode;
    if (!rt || !rt->source_party_valid || !rt->boot ||
        !rt->record_pools_valid || actor >= rt->source_party.heros_in_party)
        return;
    /* PROCESS_TIMER_0E passes the source actor and the temporary record
     * handle into c_item::PROCESS_ITEM_BONUS with mode -1.  The callback is
     * deliberately void in the source ABI; the wrapper itself is atomic and
     * leaves the hero untouched when any source owner is missing. */
    (void)dm2_v1_process_source_item_bonus_for_timer(
        &rt->source_party.hero[actor], (int16_t)actor, record, -1,
        (int16_t)(int16_t)value, &rt->record_pools,
        dm2_v1_boot_asset_loader(rt->boot), &receipt);
}

/*
 * DM2_CONTINUE_TICK_GENERATOR (c_tim_proc.cpp) runtime bridge.
 *
 * The source operation publishes an ACTUATE_TILE timer before its next
 * generator timer.  Keep both publications on the source timer queue and
 * roll back the complete pair, plus the generator active bit, on failure.
 * The generic coordinate-only actuator entry point must not be used here:
 * the generator record itself owns the target coordinates and direction.
 */
typedef struct {
    DM2_V1_RuntimeState *runtime;
    const DM2_V1_SourceTimer *source_timer;
    DM2_V1_TickGenTimerState *timer_state;
    int callback_failed;
} DM2_V1_RuntimeTickGeneratorContext;

static uint8_t *dm2_runtime_tick_generator_record(void *ctx,
                                                   uint16_t record_word) {
    DM2_V1_RuntimeTickGeneratorContext *generator =
        (DM2_V1_RuntimeTickGeneratorContext *)ctx;
    if (!generator || !generator->runtime ||
        !generator->runtime->record_pools_valid)
        return NULL;
    return dm2_v1_record_pool_address_mut(&generator->runtime->record_pools,
                                           (int16_t)record_word);
}

static void dm2_runtime_tick_generator_invoke(void *ctx, uint8_t *record,
                                               uint16_t action,
                                               uint16_t param) {
    DM2_V1_RuntimeTickGeneratorContext *generator =
        (DM2_V1_RuntimeTickGeneratorContext *)ctx;
    DM2_V1_RuntimeState *rt = generator ? generator->runtime : NULL;
    DM2_V1_SourceTimer message;
    int map;

    if (!generator || !rt || !record || !generator->source_timer) {
        if (generator) generator->callback_failed = 1;
        return;
    }
    map = (int)((generator->source_timer->ticks_and_map >> 24) & 0xffu);
    memset(&message, 0, sizeof(message));
    message.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        (((uint32_t)rt->tick_count +
          (((uint32_t)record[4] | ((uint32_t)record[5] << 8)) >> 7 & 0x7fu)) &
         DM2_V1_SOURCE_TIMER_TICK_MASK);
    message.type = DM2_V1_TIMER_ACTUATE_TILE;
    if (action == 0u) message.actor = 1u;
    else if (action == 1u) message.actor = 3u;
    else if (action == 2u) message.actor = 2u;
    message.value_a = (int16_t)((uint16_t)dm2_actu_xcoord(record) |
                                ((uint16_t)dm2_actu_ycoord(record) << 8));
    message.value_b = (int16_t)((uint16_t)dm2_actu_direction(record) |
                                (action << 8));
    if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &message, 0u) !=
        DM2_V1_SOURCE_TIMER_OK)
        generator->callback_failed = 1;
    (void)param;
}

static void dm2_runtime_tick_generator_requeue(void *ctx, uint16_t delay_base,
                                                uint8_t multiplier) {
    DM2_V1_RuntimeTickGeneratorContext *generator =
        (DM2_V1_RuntimeTickGeneratorContext *)ctx;
    DM2_V1_RuntimeState *rt = generator ? generator->runtime : NULL;
    DM2_V1_SourceTimer continuation;
    uint32_t tick;
    int map;

    if (!generator || !rt || !generator->source_timer ||
        !generator->timer_state) {
        if (generator) generator->callback_failed = 1;
        return;
    }
    continuation = *generator->source_timer;
    map = (int)((continuation.ticks_and_map >> 24) & 0xffu);
    tick = (uint32_t)rt->tick_count +
        (uint32_t)delay_base * (uint32_t)multiplier;
    continuation.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    continuation.value_b = (int16_t)((uint16_t)multiplier |
        ((uint16_t)generator->timer_state->timer_b_bit8 << 8));
    if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &continuation, 0u) !=
        DM2_V1_SOURCE_TIMER_OK)
        generator->callback_failed = 1;
}

static int dm2_runtime_tick_generator_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    DM2_V1_TickGenTimerState timer_state;
    DM2_V1_RuntimeTickGeneratorContext generator;
    DM2_V1_TickGenCallbacks callbacks;
    DM2_V1_SourceTimerQueue queue_backup;
    uint8_t *record;
    uint8_t record_byte4;
    int map;
    int result;

    if (!rt || !timer || !rt->boot || !rt->boot->dungeon_data ||
        !rt->record_pools_valid)
        return 0;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    if (map < 0 || map >= ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->level_count ||
        dm2_v1_record_handle_pool((int16_t)timer->value_a) != 3)
        return 0;
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             timer->value_a);
    if (!record) return 0;

    queue_backup = rt->timer_queue;
    record_byte4 = record[4];
    timer_state.timer_yb = (uint8_t)(timer->value_b & 0xff);
    timer_state.timer_b_bit8 = (uint8_t)((timer->value_b >> 8) & 0x01);
    memset(&generator, 0, sizeof(generator));
    generator.runtime = rt;
    generator.source_timer = timer;
    generator.timer_state = &timer_state;
    callbacks.get_record_address = dm2_runtime_tick_generator_record;
    callbacks.invoke_actuator = dm2_runtime_tick_generator_invoke;
    callbacks.requeue_timer = dm2_runtime_tick_generator_requeue;
    result = dm2_v1_continue_tick_generator((uint16_t)timer->value_a,
                                             &timer_state, &callbacks,
                                             &generator);
    if (generator.callback_failed) {
        rt->timer_queue = queue_backup;
        record[4] = record_byte4;
        return 0;
    }
    return result || (record[4] != record_byte4);
}

/*
 * DM2_PROCESS_TIMER_0x48 (c_tim_proc.cpp:4129-4163).
 *
 * The committed GAME_LOAD owner now contains the source-sized c_party and
 * c_hero records, so use those records directly.  The older spell-timer
 * compatibility context uses DM2_ChampionRecord and is intentionally not a
 * runtime substitute for this path.
 */
static int dm2_runtime_ench_power_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    uint8_t actor_mask;
    int16_t amount;
    int map;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data ||
        rt->source_party.heros_in_party < 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES)
        return 0;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    if (map < 0 || map >= ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->level_count ||
        map != rt->dungeon_level)
        return 0;

    actor_mask = timer->actor;
    amount = timer->value_a;
    for (int hero_index = 0;
         hero_index < rt->source_party.heros_in_party; ++hero_index) {
        DM2_V1_Hero *hero = &rt->source_party.hero[hero_index];
        int32_t next;
        if ((actor_mask & (uint8_t)(1u << hero_index)) == 0u ||
            hero->curHP == 0)
            continue;
        next = (int32_t)hero->ench_power - amount;
        if (next < 0) next = 0;
        if (next > INT16_MAX) next = INT16_MAX;
        hero->ench_power = (int16_t)next;
    }
    return 1;
}

/*
 * DM2_PROCESS_TIMER_0x4B / DM2_PROCESS_POISON
 * (c_tim_proc.cpp:4164-4178, c_hero.cpp:3397).
 *
 * The timer's signed counter is removed first, one wound is applied, and the
 * remaining counter is restored with the source 0x24-tick continuation.  The
 * source global v1e0288 is carried by source_next_champion_number; it is not
 * interchangeable with the party count or a host-selected active hero.
 */
static int dm2_runtime_poison_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    DM2_V1_Hero *hero;
    DM2_V1_Hero hero_backup;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_SourceTimer continuation;
    int map;
    int actor;
    int16_t counters;
    int32_t wound;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data ||
        rt->source_party.heros_in_party <= 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES)
        return 0;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    actor = (int)(int8_t)timer->actor;
    if (map < 0 ||
        map >= ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->level_count ||
        map != rt->dungeon_level || actor < 0 ||
        actor >= rt->source_party.heros_in_party ||
        rt->source_next_champion_number <= 0 ||
        rt->source_next_champion_number > DM2_MAX_HEROES ||
        actor + 1 == rt->source_next_champion_number)
        return 0;
    hero = &rt->source_party.hero[actor];
    if (hero->poisoned <= 0)
        return 0;

    hero_backup = *hero;
    queue_backup = rt->timer_queue;
    counters = timer->value_a;
    hero->poisoned = (int8_t)(hero->poisoned - 1);
    hero->poison = (int16_t)(hero->poison - counters);
    wound = ((int32_t)counters + 0x1e) >> 6;
    if (wound < 1) wound = 1;
    if (hero->curHP != 0) {
        int32_t pending = (int32_t)hero->damagesuffered + wound;
        if (pending > INT16_MAX) pending = INT16_MAX;
        hero->damagesuffered = (int16_t)pending;
    }
    hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x2800u);
    counters = (int16_t)(counters - 1);
    if (counters > 0) {
        int32_t next_poison = (int32_t)hero->poison + counters;
        if (next_poison > 0x0c00) {
            counters = (int16_t)(0x0c00 - hero->poison);
            if (counters < 0) counters = 0;
        }
        if (counters > 0) {
            hero->poison = (int16_t)(hero->poison + counters);
            hero->poisoned = (int8_t)(hero->poisoned + 1);
            memset(&continuation, 0, sizeof(continuation));
            continuation.ticks_and_map =
                ((uint32_t)(map & 0xff) << 24) |
                (((uint32_t)rt->tick_count + 0x24u) &
                 DM2_V1_SOURCE_TIMER_TICK_MASK);
            continuation.type = DM2_V1_TIMER_POISON;
            continuation.actor = (uint8_t)actor;
            continuation.value_a = counters;
            if (dm2_v1_source_timer_enqueue(&rt->timer_queue,
                                            &continuation, 0u) !=
                DM2_V1_SOURCE_TIMER_OK) {
                rt->timer_queue = queue_backup;
                *hero = hero_backup;
                return 0;
            }
        }
    }
    return 1;
}

/* DM2_PROCESS_TIMER_0x47 (c_tim_proc.cpp:4112-4123).  The countdown and
 * target are GAME_LOAD source globals, while the flag mutation belongs to
 * the transferred c_hero record. */
static int dm2_runtime_hero_ench_flag_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    int map;
    int target;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data ||
        rt->source_party.heros_in_party <= 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES)
        return 0;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    target = rt->source_hero_ench_target;
    if (map < 0 ||
        map >= ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->level_count ||
        map != rt->dungeon_level || target < 0 ||
        target > rt->source_party.heros_in_party)
        return 0;

    rt->source_hero_ench_countdown--;
    rt->source_attack_counter = rt->source_hero_ench_countdown;
    rt->source_savegames1[2] = rt->source_hero_ench_countdown;
    if (rt->source_hero_ench_countdown == 0u && target != 0) {
        DM2_V1_Hero *hero = &rt->source_party.hero[target - 1];
        if (hero->curHP != 0)
            hero->heroflag = (int16_t)((uint16_t)hero->heroflag | 0x4000u);
    }
    return 1;
}

/* DM2_PROCESS_TIMER_LIGHT (c_tim_proc.cpp:918-959), using the source
 * c_light/savegames owner transferred by GAME_LOAD. */
static int dm2_runtime_light_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    static const int16_t light_steps[16] = {
        0, 5, 12, 24, 33, 40, 46, 51,
        59, 68, 76, 82, 89, 94, 97, 100
    };
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_SourceTimer continuation;
    int map;
    int16_t amount;
    int abs_amount;
    int16_t remaining;
    int delta;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data)
        return 0;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    amount = timer->value_a;
    abs_amount = amount < 0 ? -(int)amount : (int)amount;
    if (map < 0 ||
        map >= ((DM2_V1_DungeonData *)rt->boot->dungeon_data)->level_count ||
        map != rt->dungeon_level || abs_amount >= 16)
        return 0;

    queue_backup = rt->timer_queue;
    if (amount == 0) {
        delta = 0;
        remaining = 0;
    } else {
        delta = light_steps[abs_amount] - light_steps[abs_amount - 1];
        if (amount > 0) delta *= 2;
        else delta = -delta;
        remaining = (int16_t)(amount < 0 ? -(abs_amount - 1) :
                                             abs_amount - 1);
    }
    rt->source_light_level = (int16_t)(rt->source_light_level + delta);
    if (remaining != 0) {
        memset(&continuation, 0, sizeof(continuation));
        continuation.ticks_and_map =
            ((uint32_t)(map & 0xff) << 24) |
            (((uint32_t)rt->tick_count + 8u) &
             DM2_V1_SOURCE_TIMER_TICK_MASK);
        continuation.type = DM2_V1_TIMER_LIGHT;
        continuation.value_a = remaining;
        if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &continuation,
                                        0u) != DM2_V1_SOURCE_TIMER_OK) {
            rt->timer_queue = queue_backup;
            return 0;
        }
    }
    return 1;
}

/* DM2_CONTINUE_ORNATE_ANIMATOR (c_tim_proc.cpp:~1080).  The timer payload
 * carries the DB3 actuator handle in value_a and the wall/floor mode in
 * value_b; the live map and GDAT owner determine the animation length. */
static int dm2_runtime_ornate_animator_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index __attribute__((unused)),
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_SourceTimer continuation;
    uint8_t record_backup[16];
    uint8_t *record;
    int map;
    int wall;
    int category;
    int gfx_count;
    uint8_t gfx_index;
    uint8_t decoration;
    uint16_t word2;
    uint16_t frame;
    DM2_V1_GetOrnateAnimLenReceipt anim;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->record_pools_valid)
        return 0;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    if (map < 0 || map >= dungeon->level_count || map != rt->dungeon_level)
        return 0;
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             timer->value_a);
    if (!record || dm2_v1_record_handle_pool(timer->value_a) != 3 ||
        rt->record_pools.pools[3].record_size > (int)sizeof(record_backup))
        return 0;
    /* c_tim_proc.cpp supplies the wall/floor selector in value_b for 0x55;
     * this handler has no x/y payload: getA() is the DB3 record handle and
     * getBlong() is passed directly to DM2_GET_ORNATE_ANIM_LEN. */
    wall = (timer->value_b & 1) != 0;
    category = wall ? 0x09 : 0x0a;
    word2 = (uint16_t)record[2] | ((uint16_t)record[3] << 8);
    gfx_index = (uint8_t)(((uint16_t)record[4] |
                           ((uint16_t)record[5] << 8)) >> 12);
    decoration = 0xffu;
    if (wall) {
        gfx_count = rt->map_wall_gfx_count;
        if (gfx_index > 0u && gfx_index <= (uint8_t)gfx_count)
            decoration = rt->map_wall_gfx_list[gfx_index - 1u];
    } else {
        gfx_count = rt->map_floor_gfx_count;
        if (gfx_index > 0u && gfx_index <= (uint8_t)gfx_count)
            decoration = rt->map_floor_gfx_list[gfx_index - 1u];
    }
    memset(&anim, 0, sizeof(anim));
    if (decoration == 0xffu || !dm2_v1_boot_asset_loader(rt->boot) ||
        !dm2_v1_get_ornate_anim_len_receipt(
            dm2_v1_boot_asset_loader(rt->boot), category,
                                             decoration, 0, &anim) ||
        !anim.accepted || anim.length == 0u)
        return 0;
    memcpy(record_backup, record,
           rt->record_pools.pools[3].record_size);
    frame = (uint16_t)((word2 >> 7) & 0x1ffu);
    frame = (uint16_t)((frame + 1u) & 0x1ffu);
    word2 = (uint16_t)((word2 & 0x007fu) | (frame << 7));
    record[2] = (uint8_t)word2;
    record[3] = (uint8_t)(word2 >> 8);
    queue_backup = rt->timer_queue;
    if ((frame % (uint16_t)anim.length) == 0u) {
        record[4] &= (uint8_t)~0x01u;
        return 1;
    }
    continuation = *timer;
    continuation.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        (((uint32_t)rt->tick_count + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
    if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &continuation, 0u) !=
        DM2_V1_SOURCE_TIMER_OK) {
        rt->timer_queue = queue_backup;
        memcpy(record, record_backup, rt->record_pools.pools[3].record_size);
        return 0;
    }
    return 1;
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

/* DM2_CONTINUE_ORNATE_NOISE (c_tim_proc.cpp:~1120).  0x5A carries the
 * actuator record in value_b and the source x/y pair in value_a.  The
 * inactive arm clears only the frame high byte; the active arm resolves the
 * wall/floor decoration from the mounted map and requeues at the source
 * animation cadence.  GEN2 is best-effort, but a failed timer enqueue must
 * roll back both the record and the sound queue. */
static int dm2_runtime_ornate_noise_timer(
    void *context, const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt __attribute__((unused))) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)context;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimerQueue queue_backup;
    DM2_V1_SoundQueueState sound_backup;
    uint8_t record_backup[16];
    uint8_t *record;
    DM2_V1_GetOrnateAnimLenReceipt anim;
    DM2_V1_SoundQueueEnv sound_env;
    DM2_V1_SoundQueueReceipt sound_receipt;
    int map;
    int x;
    int y;
    int raw_tile;
    int wall;
    int category;
    int gfx_count;
    uint8_t gfx_index;
    uint8_t decoration;
    int record_size;
    int active;

    if (!rt || !timer || !rt->source_party_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->record_pools_valid ||
        !rt->sound_queue_ready)
        return 0;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    if (map < 0 || map >= dungeon->level_count || map != rt->dungeon_level)
        return 0;
    if (dm2_v1_record_handle_pool(timer->value_b) != 3)
        return 0;
    record_size = rt->record_pools.pools[3].record_size;
    if (record_size <= 0 || record_size > (int)sizeof(record_backup))
        return 0;
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             timer->value_b);
    if (!record)
        return 0;
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)(((uint16_t)timer->value_a >> 8) & 0xff);
    if (x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map])
        return 0;

    memcpy(record_backup, record, (size_t)record_size);
    queue_backup = rt->timer_queue;
    sound_backup = rt->sound_queue;
    active = (record[4] & 0x01u) != 0;
    if (!active) {
        record[3] = 0u;
        return 1;
    }

    raw_tile = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    if (raw_tile < 0)
        goto ornate_noise_runtime_rollback;
    wall = ((raw_tile >> 5) & 7) == 0;
    category = wall ? 0x09 : 0x0a;
    gfx_count = wall ? rt->map_wall_gfx_count : rt->map_floor_gfx_count;
    gfx_index = (uint8_t)(((uint16_t)record[4] |
                           ((uint16_t)record[5] << 8)) >> 12);
    decoration = 0xffu;
    if (gfx_index > 0u && gfx_index <= (uint8_t)gfx_count) {
        decoration = wall ? rt->map_wall_gfx_list[gfx_index - 1u]
                          : rt->map_floor_gfx_list[gfx_index - 1u];
    }
    memset(&anim, 0, sizeof(anim));
    if (decoration == 0xffu || !dm2_v1_boot_asset_loader(rt->boot) ||
        !dm2_v1_get_ornate_anim_len_receipt(
            dm2_v1_boot_asset_loader(rt->boot), category, decoration, 0,
            &anim) || !anim.accepted || anim.length == 0u)
        goto ornate_noise_runtime_rollback;

    memset(&sound_env, 0, sizeof(sound_env));
    sound_env.current_map = (int16_t)map;
    sound_env.gate_map_a = rt->sound_env.gate_map_a;
    sound_env.gate_map_b = rt->sound_env.gate_map_b;
    sound_env.facing = (uint16_t)rt->view_dir;
    sound_env.party_x = (int16_t)dm2_v1_runtime_get_party_x();
    sound_env.party_y = (int16_t)dm2_v1_runtime_get_party_y();
    sound_env.gametick = (int32_t)rt->tick_count;
    memset(&sound_receipt, 0, sizeof(sound_receipt));
    (void)dm2_v1_sound_queue_noise_gen2(
        &rt->sound_queue, (int8_t)category, (int8_t)decoration,
        (int8_t)0x88, (int8_t)0xfe, (int16_t)x, (int16_t)y,
        1, (int16_t)0x8c, (int16_t)0x80, &sound_env, &sound_receipt);

    {
        DM2_V1_SourceTimer next_timer = *timer;
        next_timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
            (((uint32_t)rt->tick_count + (uint32_t)anim.length) &
             DM2_V1_SOURCE_TIMER_TICK_MASK);
        if (dm2_v1_source_timer_enqueue(&rt->timer_queue, &next_timer,
                                        source_index) !=
            DM2_V1_SOURCE_TIMER_OK)
            goto ornate_noise_runtime_rollback;
    }
    return 1;

ornate_noise_runtime_rollback:
    memcpy(record, record_backup, (size_t)record_size);
    rt->timer_queue = queue_backup;
    rt->sound_queue = sound_backup;
    return 0;
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

static int dm2_runtime_missile_is_plain_passage(
    const DM2_V1_DungeonData *dungeon, int map, int x, int y)
{
    int raw;
    int tile_type;

    if (!dungeon || !dm2_v1_dungeon_c_map_is_tile_passage(
            dungeon, map, x, y))
        return 0;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    if (raw < 0) return 0;
    /* SKWIN DME.h:877-885: only FLOOR/PIT/STAIRS are admitted here.
     * DOOR, TELEPORTER, TRICK_WALL and MAP_EXIT each enter a different
     * c_move/c_moverec owner and must not be treated as a plain step. */
    tile_type = dungeon->square_bytes == 1
        ? (((unsigned int)raw >> 5) & 0x7)
        : (raw & 0x1f);
    return tile_type >= 1 && tile_type <= 3;
}

/* Move one authenticated DB14 between two raw c_map ground chains.  This is
 * deliberately separate from the ordinary same-map step: the source
 * teleporter path changes c_map, performs MOVE_RECORD_TO, and only then
 * publishes the continuation timer.  The caller owns the outer pool/raw/
 * timer rollback transaction. */
static int dm2_runtime_missile_move_between_tiles(
    DM2_V1_RecordPoolSet *pools, DM2_V1_DungeonData *dungeon,
    int source_map, int source_x, int source_y,
    int destination_map, int destination_x, int destination_y,
    int16_t missile, int16_t destination_missile)
{
    int budget = 0;
    int16_t cursor;
    int16_t source_previous = DM2_V1_RECORD_HANDLE_NULL;
    int16_t source_next = DM2_V1_RECORD_HANDLE_END;
    int16_t destination_previous = DM2_V1_RECORD_HANDLE_NULL;
    int source_found = 0;
    int raw_type = -1;
    int raw_size = 0;
    uint8_t *raw_cursor;
    uint8_t *raw_previous;
    uint8_t *raw_missile;

    if (!pools || !dungeon || source_map < 0 || destination_map < 0 ||
        source_map >= dungeon->level_count ||
        destination_map >= dungeon->level_count ||
        source_x < 0 || source_y < 0 || destination_x < 0 || destination_y < 0 ||
        source_x >= dungeon->level_widths[source_map] ||
        source_y >= dungeon->level_heights[source_map] ||
        destination_x >= dungeon->level_widths[destination_map] ||
        destination_y >= dungeon->level_heights[destination_map] ||
        missile == DM2_V1_RECORD_HANDLE_NULL ||
        missile == DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db) {
        if (pools->pools[db].record_count < 0 ||
            pools->pools[db].extension_count < 0 ||
            budget > INT_MAX - pools->pools[db].record_count -
                pools->pools[db].extension_count) {
            return 0;
        }
        budget += pools->pools[db].record_count +
            pools->pools[db].extension_count;
    }
    if (budget <= 0) return 0;

    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, source_map, source_x, source_y);
    for (int steps = 0; cursor != DM2_V1_RECORD_HANDLE_END &&
                          cursor != DM2_V1_RECORD_HANDLE_NULL &&
                          steps < budget; ++steps) {
        int16_t next;
        raw_cursor = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)cursor, &raw_type, NULL, &raw_size);
        if (!raw_cursor || raw_size < 2 ||
            !dm2_v1_record_pool_next_link(pools, cursor, &next)) {
            return 0;
        }
        if (cursor == missile) {
            source_next = next;
            source_found = 1;
        } else if (!source_found) {
            source_previous = cursor;
        }
        cursor = next;
    }
    /* MOVE_RECORD_TO must own the complete source chain, not merely the
     * prefix through the missile.  A malformed tail after the DB14 would
     * otherwise be silently detached by the teleporter branch. */
    if (!source_found || cursor != DM2_V1_RECORD_HANDLE_END ||
        source_next == DM2_V1_RECORD_HANDLE_NULL)
        return 0;

    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, destination_map, destination_x, destination_y);
    for (int steps = 0; cursor != DM2_V1_RECORD_HANDLE_END &&
                          cursor != DM2_V1_RECORD_HANDLE_NULL &&
                          steps < budget; ++steps) {
        int16_t next;
        raw_cursor = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)cursor, &raw_type, NULL, &raw_size);
        if (!raw_cursor || raw_size < 2 ||
            !dm2_v1_record_pool_next_link(pools, cursor, &next)) {
            return 0;
        }
        destination_previous = cursor;
        cursor = next;
    }
    if (cursor != DM2_V1_RECORD_HANDLE_END) return 0;

    if (!dm2_v1_record_pool_cut_from_tile(
            pools, dungeon, source_map, source_x, source_y, missile))
        return 0;
    if (source_previous == DM2_V1_RECORD_HANDLE_NULL) {
        /* cut_from_tile already rewrote the source root. */
    } else {
        raw_previous = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)source_previous, &raw_type, NULL, &raw_size);
        if (!raw_previous || raw_size < 2) return 0;
        raw_previous[0] = (uint8_t)source_next;
        raw_previous[1] = (uint8_t)((uint16_t)source_next >> 8);
    }

    {
        int16_t destination_head = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, destination_map, destination_x, destination_y);
        if (!dm2_v1_record_pool_append_to_list(
                pools, &destination_head, destination_missile))
            return 0;
        if (destination_previous == DM2_V1_RECORD_HANDLE_NULL) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, destination_map, destination_x, destination_y,
                    (uint16_t)destination_missile) != 0)
                return 0;
        } else {
            raw_previous = (uint8_t *)(uintptr_t)
                dm2_v1_dungeon_get_thing_record(
                    dungeon, (uint16_t)destination_previous,
                    &raw_type, NULL, &raw_size);
            if (!raw_previous || raw_size < 2) return 0;
            raw_previous[0] = (uint8_t)destination_missile;
            raw_previous[1] = (uint8_t)((uint16_t)destination_missile >> 8);
        }
    }
    raw_missile = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        dungeon, (uint16_t)missile, NULL, NULL, NULL);
    if (!raw_missile) return 0;
    raw_missile[0] = 0xfeu;
    raw_missile[1] = 0xffu;
    return 1;
}

/* Resolve the direct teleporter subbranch.  The destination probe and
 * map_3BF83 bounds are source-owned; the DB14 handle and continuation
 * direction are rotated with the same source delta used by the party pose
 * owner. */
static int dm2_runtime_missile_resolve_teleporter(
    const DM2_V1_RecordPoolSet *pools, const DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int16_t missile,
    int *out_map, int *out_x, int *out_y, int16_t *out_missile)
{
    const uint8_t *source_tiles;
    const uint8_t *destination_tiles;
    DM2_V1_SkprojectTeleporterDetail detail;
    DM2_V1_SkprojectGetTeleporterDetailReceipt detail_receipt;
    DM2_V1_SkprojectMap3BF83Receipt map_receipt;
    int16_t source_width, source_height;
    int16_t destination_width, destination_height;
    int raw;

    if (out_map) *out_map = -1;
    if (out_x) *out_x = -1;
    if (out_y) *out_y = -1;
    if (out_missile) *out_missile = missile;
    if (!pools || !dungeon || map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map]) return 0;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
    if (raw < 0 || dm2_v1_dungeon_get_square_type(dungeon, map, x, y) != 5)
        return 0;
    source_tiles = dm2_v1_dungeon_level_tile_data(
        dungeon, map, &source_width, &source_height);
    if (!source_tiles) return -1;
    memset(&detail, 0, sizeof(detail));
    memset(&detail_receipt, 0, sizeof(detail_receipt));
    /* The destination map is encoded by the origin record; the helper below
     * needs its tile plane, so first read the origin record's word@4. */
    {
        const int first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
        const uint8_t *origin = first >= 0
            ? dm2_v1_record_pool_address(pools, (int16_t)first) : NULL;
        const uint16_t word4 = origin
            ? (uint16_t)(origin[4] | ((uint16_t)origin[5] << 8)) : 0u;
        const int destination_map = (int)(word4 >> 8);
        if (!origin || destination_map < 0 ||
            destination_map >= dungeon->level_count) return -1;
        destination_tiles = dm2_v1_dungeon_level_tile_data(
            dungeon, destination_map, &destination_width, &destination_height);
        if (!destination_tiles ||
            !dm2_v1_skproject_get_teleporter_detail(
                (int16_t)x, (int16_t)y, source_tiles, source_width,
                source_height, pools, (uint8_t)map, destination_tiles,
                destination_width, destination_height, &detail,
                &detail_receipt) || !detail_receipt.valid ||
            detail.b_04 != (uint8_t)destination_map ||
            dm2_v1_get_creature_at(pools, dungeon, destination_map,
                                   detail.b_02, detail.b_03) !=
                DM2_V1_RECORD_HANDLE_NULL) return -1;
        {
            const uint8_t stored_dir = (uint8_t)(((uint16_t)missile >> 14) & 3u);
            const uint8_t destination_dir = (uint8_t)(
                (detail.b_01 - detail.b_00 + stored_dir) & 3u);
            const int16_t rotated_missile = (int16_t)(
                ((uint16_t)missile & 0x3fffu) |
                ((uint16_t)destination_dir << 14));
            if (out_missile) *out_missile = rotated_missile;
        }
        if (!dm2_v1_skproject_map_3bf83(
                detail.b_02, detail.b_03, detail.b_04,
                (int16_t)(((detail.b_01 - detail.b_00 +
                            (((uint16_t)missile >> 14) & 3u)) & 3u)),
                map, x, y,
                destination_width, destination_height, &map_receipt) ||
            !map_receipt.valid || !map_receipt.in_bounds) return -1;
        if (out_map) *out_map = destination_map;
        if (out_x) *out_x = detail.b_02;
        if (out_y) *out_y = detail.b_03;
    }
    return 1;
}

static uint16_t dm2_runtime_missile_rd16(
    const DM2_V1_RecordPoolSet *pools, const uint8_t *p)
{
    return pools && pools->source_words_big_endian
        ? (uint16_t)(((uint16_t)p[0] << 8) | p[1])
        : (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static void dm2_runtime_missile_wr16(
    const DM2_V1_RecordPoolSet *pools, uint8_t *p, uint16_t value)
{
    if (pools && pools->source_words_big_endian) {
        p[0] = (uint8_t)(value >> 8);
        p[1] = (uint8_t)value;
    } else {
        p[0] = (uint8_t)value;
        p[1] = (uint8_t)(value >> 8);
    }
}

/* Dynamic DB14 records are source-owned pool records; many authentic worlds
 * have no raw dungeon record for a newly allocated projectile. Keep the pool
 * chain authoritative and mirror raw links only where a raw record exists. */
static int dm2_runtime_missile_move_pool_between_tiles(
    DM2_V1_RecordPoolSet *pools, DM2_V1_DungeonData *dungeon,
    int source_map, int source_x, int source_y,
    int destination_map, int destination_x, int destination_y,
    int16_t missile)
{
    int budget = 0;
    int16_t cursor, source_prev = DM2_V1_RECORD_HANDLE_NULL;
    int16_t source_next = DM2_V1_RECORD_HANDLE_END;
    int16_t destination_prev = DM2_V1_RECORD_HANDLE_NULL;
    int same_tile;
    uint8_t *raw;

    if (!pools || !dungeon || missile < 0) return 0;
    same_tile = source_map == destination_map && source_x == destination_x &&
        source_y == destination_y;
    for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db)
        budget += pools->pools[db].record_count +
            pools->pools[db].extension_count;
    if (budget <= 0) return 0;

    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, source_map, source_x, source_y);
    for (int steps = 0; cursor >= 0 &&
             cursor != DM2_V1_RECORD_HANDLE_END && steps++ < budget;) {
        int16_t next;
        if (!dm2_v1_record_pool_next_link(pools, cursor, &next)) return 0;
        if (cursor == missile) {
            source_next = next;
            break;
        }
        source_prev = cursor;
        cursor = next;
    }
    if (cursor != missile || source_next == DM2_V1_RECORD_HANDLE_NULL)
        return 0;

    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
        dungeon, destination_map, destination_x, destination_y);
    for (int steps = 0; cursor >= 0 &&
             cursor != DM2_V1_RECORD_HANDLE_END && steps++ < budget;) {
        int16_t next;
        if (!dm2_v1_record_pool_next_link(pools, cursor, &next)) return 0;
        destination_prev = cursor;
        cursor = next;
    }
    if (cursor != DM2_V1_RECORD_HANDLE_END) return 0;

    if (source_prev == DM2_V1_RECORD_HANDLE_NULL) {
        if (dm2_v1_dungeon_set_first_thing(dungeon, source_map, source_x,
                                           source_y, (uint16_t)source_next))
            return 0;
    } else {
        uint8_t *prev = dm2_v1_record_pool_address_mut(pools, source_prev);
        if (!prev) return 0;
        dm2_runtime_missile_wr16(pools, prev, (uint16_t)source_next);
        raw = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)source_prev, NULL, NULL, NULL);
        if (raw) dm2_runtime_missile_wr16(pools, raw, (uint16_t)source_next);
    }
    {
        uint8_t *projectile = dm2_v1_record_pool_address_mut(pools, missile);
        if (!projectile) return 0;
        dm2_runtime_missile_wr16(pools, projectile,
                                 DM2_V1_RECORD_HANDLE_END);
        raw = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
            dungeon, (uint16_t)missile, NULL, NULL, NULL);
        if (raw) dm2_runtime_missile_wr16(pools, raw,
                                          DM2_V1_RECORD_HANDLE_END);
    }
    if (same_tile) return 1;
    {
        int16_t destination_head = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, destination_map, destination_x, destination_y);
        if (!dm2_v1_record_pool_append_to_list(
                pools, &destination_head, missile)) return 0;
        if (destination_prev == DM2_V1_RECORD_HANDLE_NULL) {
            if (dm2_v1_dungeon_set_first_thing(
                    dungeon, destination_map, destination_x, destination_y,
                    (uint16_t)missile)) return 0;
        } else {
            raw = dm2_v1_record_pool_address_mut(pools, destination_prev);
            if (!raw) return 0;
            dm2_runtime_missile_wr16(pools, raw, (uint16_t)missile);
            raw = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
                dungeon, (uint16_t)destination_prev, NULL, NULL, NULL);
            if (raw) dm2_runtime_missile_wr16(pools, raw, (uint16_t)missile);
        }
    }
    return 1;
}

/* Resolve the live source position of a DB4 handle after an attack owner has
 * run.  THINK_CREATURE is intentionally coordinate-based in c_ai.cpp, so a
 * partial ATTACK_CREATURE tail must reissue the source timer at the record's
 * current tile rather than at the projectile's old destination. */
static int dm2_runtime_find_creature_tile(
    const DM2_V1_RecordPoolSet *pools, const DM2_V1_DungeonData *dungeon,
    int map, int16_t creature, int *out_x, int *out_y)
{
    if (!pools || !dungeon || creature < 0 || !out_x || !out_y ||
        map < 0 || map >= dungeon->level_count) return 0;
    for (int y = 0; y < dungeon->level_heights[map]; ++y) {
        for (int x = 0; x < dungeon->level_widths[map]; ++x) {
            if (dm2_v1_get_creature_at(pools, dungeon, map, x, y) == creature) {
                *out_x = x;
                *out_y = y;
                return 1;
            }
        }
    }
    return 0;
}

/*
 * dm2_runtime_step_missile_timer — source-shaped DB14 half of
 * DM2_STEP_MISSILE (c_tim_proc.cpp:432-877).
 *
 * c_tim::A is the DB14 record handle.  c_tim::B packs x in bits 0..4, y in
 * bits 5..9 and the source energy step in bits 12..15.  The old reduced
 * spell delegate treated A/B as two coordinates, which could never bind the
 * saved missile record.  This handler admits only a DB14 whose record bytes
 * 6..7 point back to the dispatching source timer slot and whose chain really
 * contains the record on the encoded tile.
 *
 * The complete source path still owns actuator and broader
 * DM2_MOVE_RECORD_TO side effects.  In particular, c_tim_proc.cpp calls
 * DM2_move_075f_0af9 first and then enters DM2_MOVE_RECORD_TO; this handler
 * binds the bounded ordinary passage and teleporter/map-transition branches
 * for creature-free cells and NONMATERIAL creature pass-through. Other
 * creature outcomes remain fail-closed; all mutations are rolled back if the
 * continuation timer cannot be queued.
 */
static int dm2_runtime_step_missile_timer(
    void *user, const DM2_V1_SourceTimer *timer, uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt) {
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)user;
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RecordPoolSet pool_backup;
    DM2_V1_SourceTimerQueue queue_backup;
    uint8_t *caii_backup = NULL;
    int caii_backup_bytes = 0;
    DM2_V1_DropRng rng_backup = {0};
    uint8_t *raw_backup = NULL;
    uint8_t *record;
    uint8_t *dungeon_record;
    int map, x, y, size_dungeon = 0;
    int16_t missile;
    int16_t creature;
    int impact_x;
    int impact_y;
    int nonmaterial_pass_through = 0;
    uint16_t packed_b, energy_step;
    uint8_t record_energy;
    int move_dir;
    int next_x, next_y;
    int cloned = 0;
    int caii_cloned = 0;
    int rng_cloned = 0;

    memset(&pool_backup, 0, sizeof(pool_backup));
    if (!rt || !timer || !rt->record_pools_valid || !rt->boot ||
        !rt->boot->dungeon_data || !rt->boot->dm2_state) {
        if (receipt) receipt->handler_rejected_count++;
        return 1;
    }
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    missile = (int16_t)(uint16_t)timer->value_a;
    packed_b = (uint16_t)timer->value_b;
    x = (int)(packed_b & 0x1fu);
    y = (int)((packed_b >> 5) & 0x1fu);
    impact_x = x;
    impact_y = y;
    energy_step = (uint16_t)(packed_b >> 12);
    if (map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x >= dungeon->level_widths[map] ||
        y >= dungeon->level_heights[map] ||
        dm2_v1_record_handle_pool(missile) != 14 ||
        !(record = dm2_v1_record_pool_address_mut(
              &rt->record_pools, missile)) ||
        rt->record_pools.pools[14].record_size < 8 ||
        (dm2_runtime_missile_rd16(&rt->record_pools, record + 6) !=
         source_index)) {
        goto missile_reject;
    }

    dungeon_record = (uint8_t *)(uintptr_t)dm2_v1_dungeon_get_thing_record(
        dungeon, (uint16_t)missile, NULL, NULL, &size_dungeon);
    if (dungeon_record && (size_dungeon < 8 ||
        dm2_runtime_missile_rd16(&rt->record_pools, dungeon_record + 6) !=
            source_index)) {
        goto missile_reject;
    }
    creature = dm2_v1_get_creature_at(&rt->record_pools, dungeon, map, x, y);
    /* c_tim_proc.cpp:753-813 checks the destination cell after the
     * source moverec has selected the next direction.  A spell fired from
     * the party cell therefore has no creature at (x,y), but must still
     * enter the same ATTACK_CREATURE owner when the next cell contains a
     * DB4 creature. */
    if (creature == DM2_V1_RECORD_HANDLE_NULL && timer->type != 0x1du) {
        static const int missile_dx[4] = { 0, 1, 0, -1 };
        static const int missile_dy[4] = { -1, 0, 1, 0 };
        int next_dir = (int)((packed_b >> 10) & 0x3u);
        int candidate_x = x + missile_dx[next_dir];
        int candidate_y = y + missile_dy[next_dir];
        if (candidate_x >= 0 && candidate_y >= 0 &&
            candidate_x < dungeon->level_widths[map] &&
            candidate_y < dungeon->level_heights[map]) {
            creature = dm2_v1_get_creature_at(
                &rt->record_pools, dungeon, map, candidate_x, candidate_y);
            if (creature != DM2_V1_RECORD_HANDLE_NULL) {
                impact_x = candidate_x;
                impact_y = candidate_y;
            }
        }
    }
    if (creature != DM2_V1_RECORD_HANDLE_NULL) {
        const uint8_t *creature_record = dm2_v1_record_pool_address(
            &rt->record_pools, creature);
        uint16_t ai_flags = 0u;
        DM2_V1_CaiiAiSpecFlagsFn flags_fn =
            dm2_v1_caii_get_ai_spec_flags_fn();
        /* The source deletes an absorbed/reflected projectile, but its HIT
         * and TURNS_MISSILE branches need the still-unbound damage and
         * target-rotation owners.  Unknown AI provenance therefore keeps the
         * timer live instead of guessing a collision outcome. */
        if (!creature_record || !flags_fn ||
            flags_fn((int)creature_record[4], &ai_flags) != 1)
            goto missile_requeue;
        if ((ai_flags & (DM2_AIFLAG_ABSORBS_MISSILE |
                         DM2_AIFLAG_REFLECTOR)) == 0u) {
            /* NONMATERIAL passes through the creature-side moverec path;
             * its map continuation is still unbound.  The w30
             * TURNS_MISSILE bit only suppresses the pre-hit direction flip
             * in c_move.cpp:52090-52114; the hit itself still uses the same
             * impact/ATTACK_CREATURE owner below. */
            if ((ai_flags & DM2_AIFLAG_NONMATERIAL) != 0u)
                nonmaterial_pass_through = 1;
            else {
            DM2_V1_CaiiAttackReceipt attack_receipt;
            DM2_V1_CaiiAttackReceipt impact_attack_receipt;
            DM2_V1_ImpactAttackRequest impact_request;
            DM2_V1_ImpactAttackReceipt impact_receipt;
            int attack_completed;
            int impact_attack_completed = 1;
            memset(&impact_attack_receipt, 0, sizeof(impact_attack_receipt));
            /* c_tim_proc.cpp:795-802: after DM2_MOVE_RECORD_TO has moved
             * the missile, the source probes the destination creature and
             * invokes DM2_ATTACK_CREATURE with attack word 0x2006 and a
             * zero damage argument.  This is the creature reaction path,
             * not the projectile's WOUND_CREATURE damage owner. */
            if (!rt->caii_ready || !rt->caii.valid ||
                rt->caii.capacity <= 0 || rt->caii.capacity > INT_MAX /
                    DM2_V1_CAII_SLOT_SIZE)
                goto missile_requeue;

            if (!dm2_v1_record_pool_set_clone(&pool_backup,
                                              &rt->record_pools)) {
                goto missile_reject;
            }
            cloned = 1;
            queue_backup = rt->timer_queue;
            rng_backup = rt->drop_rng;
            rng_cloned = 1;
            if (rt->caii.slots == NULL)
                goto missile_rollback;
            caii_backup_bytes = rt->caii.capacity * DM2_V1_CAII_SLOT_SIZE;
            caii_backup = malloc((size_t)caii_backup_bytes);
            if (!caii_backup) goto missile_rollback;
            memcpy(caii_backup, rt->caii.slots, (size_t)caii_backup_bytes);
            caii_cloned = 1;
            if (!dungeon->raw_data || dungeon->raw_size <= 0)
                goto missile_rollback;
            raw_backup = malloc((size_t)dungeon->raw_size);
            if (!raw_backup) goto missile_rollback;
            memcpy(raw_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
            memset(&impact_request, 0, sizeof(impact_request));
            impact_request.recordLink = dm2_runtime_missile_rd16(
                &rt->record_pools, record + 2);
            impact_request.dbType = (impact_request.recordLink & 0x3c00) >> 10;
            impact_request.missileEnergyRemaining = record[4];
            impact_request.missileEnergyRemaining2 = record[5];
            impact_request.missilePowerNibble = (int)energy_step;
            impact_request.queryWord = dm2_runtime_projectile_query_word;
            impact_request.queryWeight = dm2_runtime_projectile_query_weight;
            impact_request.randMask = dm2_runtime_projectile_rand_mask;
            impact_request.userdata = rt;
            memset(&impact_receipt, 0, sizeof(impact_receipt));
            if (!dm2_v1_DM2_move_075f_06bd_projectile_get_impact_attack(
                    &impact_request, &impact_receipt) || !impact_receipt.valid)
                goto missile_rollback;
            if (impact_receipt.impactAttack > 0) {
                /* c_move.cpp:1738-1770: MOVE_075F applies the projectile
                 * attack as 0x200D/100 before STEP_MISSILE's later 0x2006
                 * creature reaction.  Keep both calls in CAII so
                 * THINK_CREATURE remains the HP/death/drop owner. */
                impact_attack_completed = dm2_v1_caii_attack_creature(
                    &rt->record_pools, dungeon, &rt->caii,
                    &rt->timer_queue, &rt->drop_rng, map,
                    (unsigned long)rt->tick_count,
                    creature, impact_x, impact_y, impact_x, impact_y,
                    0x200du, 100, impact_receipt.impactAttack,
                    &impact_attack_receipt);
                if (!impact_attack_completed &&
                    impact_attack_receipt.ai_flags_unknown)
                    goto missile_rollback;
            }
            attack_completed = dm2_v1_caii_attack_creature(
                &rt->record_pools, dungeon, &rt->caii,
                    &rt->timer_queue, &rt->drop_rng, map,
                    receipt ? receipt->game_tick : (unsigned long)rt->tick_count,
                    creature, impact_x, impact_y, impact_x, impact_y,
                    0x2006u, 0, 0, &attack_receipt);
            if (impact_attack_receipt.hp_applied &&
                !impact_attack_receipt.rescheduled) {
                int creature_x = -1;
                int creature_y = -1;
                DM2_V1_CreatureScheduleReceipt schedule;
                memset(&schedule, 0, sizeof(schedule));
                if (dm2_runtime_find_creature_tile(
                        &rt->record_pools, dungeon, map, creature,
                        &creature_x, &creature_y) &&
                    dm2_v1_caii_schedule_creature_at(
                        &rt->record_pools, dungeon, &rt->caii,
                        &rt->timer_queue, map,
                        (unsigned long)rt->tick_count,
                        creature_x, creature_y, &schedule) &&
                    schedule.valid) {
                    impact_attack_receipt.rescheduled = 1;
                }
            }
            memset(&g_dm2_last_missile_impact, 0,
                   sizeof(g_dm2_last_missile_impact));
            g_dm2_last_missile_impact.valid = 1;
            g_dm2_last_missile_impact.destination_hit =
                impact_x != x || impact_y != y;
            g_dm2_last_missile_impact.attack_completed = attack_completed;
            g_dm2_last_missile_impact.hp_applied =
                impact_attack_receipt.hp_applied;
            g_dm2_last_missile_impact.hp_after =
                impact_attack_receipt.hp_word_after;
            g_dm2_last_missile_impact.damage_amount =
                impact_receipt.impactAttack;
            g_dm2_last_missile_impact.damage_attack_completed =
                impact_attack_completed;
            g_dm2_last_missile_impact.damage_hp_word_after =
                impact_attack_receipt.hp_word_after;
            g_dm2_last_missile_impact.damage_rescheduled =
                impact_attack_receipt.rescheduled;
            g_dm2_last_missile_impact.missile_record = missile;
            g_dm2_last_missile_impact.creature_record = creature;
            g_dm2_last_missile_impact.creature_type =
                attack_receipt.creature_type;
            /* The source continues into the moverec result after this
             * reaction call.  A fail-closed reaction tail must not invent a
             * creature wound, so only the bounded call's own state is
             * committed here. */
            if (!attack_completed && attack_receipt.ai_flags_unknown)
                goto missile_rollback;
            if (!dm2_v1_record_pool_cut_from_tile(
                    &rt->record_pools, dungeon, map, x, y, missile))
                goto missile_rollback;
            record = dm2_v1_record_pool_address_mut(&rt->record_pools, missile);
            if (!record) goto missile_rollback;
            record[0] = 0xffu; record[1] = 0xffu;
            if (dungeon_record) {
                dungeon_record[0] = 0xffu; dungeon_record[1] = 0xffu;
            }
            g_dm2_last_missile_impact.missile_consumed = 1;
            dm2_v1_record_pool_set_free(&pool_backup);
            free(caii_backup);
            free(raw_backup);
            return 1;
            }
        }
    }

    /* Validate that the source handle is on the encoded tile before any
     * record or map byte changes. */
    {
        int16_t cursor = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, map, x, y);
        int found = 0;
        int steps = 0;
        int budget = 0;
        for (int db = 0; db < DM2_V1_RECORD_POOL_COUNT; ++db)
            budget += rt->record_pools.pools[db].record_count +
                rt->record_pools.pools[db].extension_count;
        while (cursor != DM2_V1_RECORD_HANDLE_END && cursor >= 0 &&
               steps++ < budget) {
            int16_t next;
            if (cursor == missile) found = 1;
            if (!dm2_v1_record_pool_next_link(&rt->record_pools, cursor,
                                               &next)) {
                goto missile_reject;
            }
            cursor = next;
        }
        if (!found || cursor != DM2_V1_RECORD_HANDLE_END) {
            goto missile_reject;
        }
    }

    if (!dm2_v1_record_pool_set_clone(&pool_backup, &rt->record_pools)) {
        goto missile_reject;
    }
    cloned = 1;
    queue_backup = rt->timer_queue;
    if (!dungeon->raw_data || dungeon->raw_size <= 0)
        goto missile_rollback;
    raw_backup = (uint8_t *)malloc((size_t)dungeon->raw_size);
    if (!raw_backup) goto missile_rollback;
    memcpy(raw_backup, dungeon->raw_data, (size_t)dungeon->raw_size);

    record_energy = record[4];
    if ((uint16_t)record_energy <= energy_step) {
        if (!dm2_runtime_missile_move_pool_between_tiles(
                &rt->record_pools, dungeon, map, x, y,
                map, x, y, missile))
            goto missile_rollback;
        record = dm2_v1_record_pool_address_mut(&rt->record_pools, missile);
        if (!record) goto missile_rollback;
        record[0] = 0xffu; record[1] = 0xffu;
        if (dungeon_record) {
            dungeon_record[0] = 0xffu; dungeon_record[1] = 0xffu;
        }
    } else {
        record[4] = (uint8_t)(record[4] - (uint8_t)energy_step);
        record[5] = record[5] >= (uint8_t)energy_step
            ? (uint8_t)(record[5] - (uint8_t)energy_step) : 0u;
        if (dungeon_record) {
            dungeon_record[4] = record[4];
            dungeon_record[5] = record[5];
        }
        {
            /* c_tim_proc.cpp:568-753: after the energy write, a normal
             * passage advances the DB14 record to the direction-selected
             * tile before the continuation timer is queued.  The source
             * pool copy and c_map raw chain are separate owners, so admit
             * only a bounded, creature-free passage and update both views. */
            static const int move_dx[4] = {0, 1, 0, -1};
            static const int move_dy[4] = {-1, 0, 1, 0};
            move_dir = (int)((packed_b >> 10) & 0x3u);
            next_x = x + move_dx[move_dir];
            next_y = y + move_dy[move_dir];
            {
                int teleporter_map = -1;
                int teleporter_x = -1;
                int teleporter_y = -1;
                int16_t teleporter_missile = missile;
                int teleporter_route = 0;
                if (next_x >= 0 && next_y >= 0 &&
                    next_x < dungeon->level_widths[map] &&
                    next_y < dungeon->level_heights[map]) {
                    teleporter_route = dm2_runtime_missile_resolve_teleporter(
                        &rt->record_pools, dungeon, map, next_x, next_y,
                        missile, &teleporter_map, &teleporter_x,
                        &teleporter_y, &teleporter_missile);
                }
                if (teleporter_route < 0)
                    goto missile_rollback;
                if (teleporter_route > 0) {
                    /* This is the source's map_3BF83/MOVE_RECORD_TO handoff,
                     * not a coordinate-only teleport: both raw chains and
                     * the pool chain are moved before the timer payload is
                     * retargeted. */
                    if (!dm2_runtime_missile_move_between_tiles(
                            &rt->record_pools, dungeon, map, x, y,
                            teleporter_map, teleporter_x, teleporter_y,
                            missile, teleporter_missile))
                        goto missile_rollback;
                    map = teleporter_map;
                    next_x = teleporter_x;
                    next_y = teleporter_y;
                    missile = teleporter_missile;
                    move_dir = (int)(((uint16_t)missile >> 14) & 3u);
                    packed_b = (uint16_t)((packed_b & ~0x3ffu) |
                        (uint16_t)next_x | ((uint16_t)next_y << 5) |
                        (uint16_t)(move_dir << 10));
                } else if (next_x >= 0 && next_y >= 0 &&
                           next_x < dungeon->level_widths[map] &&
                           next_y < dungeon->level_heights[map] &&
                           dm2_runtime_missile_is_plain_passage(
                               dungeon, map, next_x, next_y) &&
                           (nonmaterial_pass_through ||
                            dm2_v1_get_creature_at(
                                &rt->record_pools, dungeon, map, next_x, next_y) ==
                                DM2_V1_RECORD_HANDLE_NULL)) {
                if (!dm2_runtime_missile_move_pool_between_tiles(
                        &rt->record_pools, dungeon, map, x, y,
                        map, next_x, next_y, missile))
                    goto missile_rollback;
                packed_b = (uint16_t)((packed_b & ~0x3ffu) |
                    (uint16_t)next_x | ((uint16_t)next_y << 5));
                }
            }
            DM2_V1_SourceTimer continuation = *timer;
            uint32_t tick = receipt ? receipt->game_tick :
                (uint32_t)rt->tick_count;
            continuation.value_b = (int16_t)packed_b;
            continuation.value_a = missile;
            continuation.type = 0x1eu;
            continuation.ticks_and_map =
                ((uint32_t)(map & 0xff) << 24) |
                ((tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
            if (dm2_v1_source_timer_enqueue(
                    &rt->timer_queue, &continuation, source_index) !=
                DM2_V1_SOURCE_TIMER_OK)
                goto missile_rollback;
        }
    }
    dm2_v1_record_pool_set_free(&pool_backup);
    free(raw_backup);
    return 1;

missile_requeue:
    /* Collision/reflection and moverec remain source-owned. Preserve the
     * timer rather than consuming it as if the projectile had disappeared. */
    {
        DM2_V1_SourceTimer continuation = *timer;
        uint32_t tick = receipt ? receipt->game_tick : (uint32_t)rt->tick_count;
        continuation.type = 0x1eu;
        continuation.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
            ((tick + 1u) & DM2_V1_SOURCE_TIMER_TICK_MASK);
        if (dm2_v1_source_timer_enqueue(
                &rt->timer_queue, &continuation, source_index) ==
            DM2_V1_SOURCE_TIMER_OK)
            return 1;
    }
missile_reject:
    if (receipt) receipt->handler_rejected_count++;
    return 1;

missile_rollback:
    if (raw_backup)
        memcpy(dungeon->raw_data, raw_backup, (size_t)dungeon->raw_size);
    rt->timer_queue = queue_backup;
    if (rng_cloned)
        rt->drop_rng = rng_backup;
    if (caii_cloned && caii_backup && rt->caii.slots)
        memcpy(rt->caii.slots, caii_backup, (size_t)caii_backup_bytes);
    if (cloned) {
        dm2_v1_record_pool_set_free(&rt->record_pools);
        rt->record_pools = pool_backup;
        memset(&pool_backup, 0, sizeof(pool_backup));
    }
    free(raw_backup);
    free(caii_backup);
    if (receipt) receipt->handler_rejected_count++;
    return 1;
}

/* 0x19 cloud remains delegated to the source-owned runtime cloud handler;
 * 0x1D/0x1E now use the authenticated DB14 handler above. */

/* 0x46 light, 0x47 hero ench flag and 0x4B poison are delegated to the
 * compatibility spell handlers.  Runtime 0x48 uses the source-sized c_hero
 * owner directly above. */

/* 0x5E alloc new creature is bound to the source-shaped direct free-slot
 * owner above when the transferred runtime still has authenticated AI/GDAT,
 * DB4, CAII, timer and sound state.  The source recycler, cross-map placement
 * and missing-owner branches remain fail-closed. */

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


    /* GAME_LOAD owns gametick, the timer heap and the v1e14xx weather
     * globals as one restored session (SKProject c_savegame.cpp::DM2_GAME_LOAD
     * followed by c_tim_proc.cpp::DM2_PROCEED_TIMERS). A boot-mounted
     * File_header world has none of those owners, so advancing even the local
     * tick counter would manufacture elapsed time for a non-session. */
    if (!rt->boot || !rt->boot->source_game_load_session_ready) {
        return;
    }
    rt->tick_count++;

    /* SKULLWIN/startend.cpp consumes one savegames1.b_04 unit per source
     * update.  Keep this separate from the timer heap: Aura of Speed is a
     * global source byte, not a hero enchantment timer. */
    if (rt->source_aura_of_speed_valid && rt->source_aura_of_speed != 0u)
        rt->source_aura_of_speed--;

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
        /* c_tim_proc's 0x04 dispatch is record-owned. Wall/floor classes now
         * enter the complete source actuator chain walkers. Other classes
         * remain fail-closed; no value_b-only terrain mutation is allowed. */
        dispatcher.tile_class_at = dm2_runtime_tile_class_at;
        dispatcher.actuator_tile[0] = dm2_runtime_actuate_wall_mecha_timer;
        dispatcher.actuator_tile[1] = dm2_runtime_actuate_floor_mecha_timer;
        dispatcher.actuator_tile[4] = dm2_runtime_actuate_door_mecha_timer;
        dispatcher.handlers[DM2_V1_TIMER_UPDATE_WEATHER] =
            dm2_runtime_update_weather_timer;
        /* STEP_DOOR now requires the direct DB0 root at the timer cell, a
         * complete record graph, source actor direction and no party/DB4
         * collision. Its sound and full moverec follow-up remain separate. */
        dispatcher.handlers[DM2_V1_TIMER_STEP_DOOR] =
            dm2_runtime_door_step_timer;
        dispatcher.handlers[DM2_V1_TIMER_DESTROY_DOOR] =
            dm2_runtime_destroy_door_timer;
        /* These four tails are source-global record-byte operations.  The
         * committed GAME_LOAD graph now supplies the authenticated map
         * owner and record lookup; invalid map/record admission returns a
         * rejected handler result without a host fallback mutation. */
        dispatcher.handlers[DM2_V1_TIMER_RELEASE_DOOR_BUTTON] =
            dm2_runtime_release_door_button_timer;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_59] =
            dm2_runtime_process_timer_59;
        dispatcher.handlers[DM2_V1_TIMER_5B_RECORD_CLEAR] =
            dm2_runtime_5b_record_clear;
        dispatcher.handlers[DM2_V1_TIMER_5C_RECORD_SET] =
            dm2_runtime_5c_record_set;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_0C] =
            dm2_runtime_process_0c_timer;
        dispatcher.handlers[DM2_V1_TIMER_RESURRECTION] =
            dm2_runtime_resurrection_timer;
        /* PROCESS_0E temporarily changes an item record and runs the
         * source-owned c_item::PROCESS_ITEM_BONUS against c_hero. */
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_0E] =
            dm2_runtime_process_0e_timer;
        /* TICK_GENERATOR publishes the source-owned 0x04 message and its
         * continuation from the live DB3 actuator record.  The 0x04
         * consumer remains fail-closed until its complete DB3/DB14 owner is
         * admitted separately. */
        dispatcher.handlers[DM2_V1_TIMER_TICK_GENERATOR] =
            dm2_runtime_tick_generator_timer;
        dispatcher.handlers[DM2_V1_TIMER_ENCH_POWER] =
            dm2_runtime_ench_power_timer;
        dispatcher.handlers[DM2_V1_TIMER_POISON] =
            dm2_runtime_poison_timer;
        dispatcher.handlers[DM2_V1_TIMER_HERO_ENCH_FLAG] =
            dm2_runtime_hero_ench_flag_timer;
        dispatcher.handlers[DM2_V1_TIMER_LIGHT] =
            dm2_runtime_light_timer;
        dispatcher.handlers[DM2_V1_TIMER_ORNATE_ANIMATOR] =
            dm2_runtime_ornate_animator_timer;
        dispatcher.handlers[DM2_V1_TIMER_ORNATE_NOISE] =
            dm2_runtime_ornate_noise_timer;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_SOUND] =
            dm2_runtime_process_sound_timer;
        /* PROCESS_3D owns authenticated DB4 moverec.  MOVE_RECORD_ROTATE
         * owns the same-map party-sentinel path; the bounded cross-map shape
         * remains behind positive verification, and actuator tails remain
         * fail-closed. */
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_3C] =
            dm2_runtime_process_moverec_timer;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_3D] =
            dm2_runtime_process_moverec_timer;
        dispatcher.handlers[DM2_V1_TIMER_MOVE_RECORD_ROTATE] =
            dm2_runtime_move_record_rotate_timer;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_CLOUD] =
            dm2_runtime_process_cloud_timer;
        dispatcher.handlers[DM2_V1_TIMER_STEP_MISSILE] =
            dm2_runtime_step_missile_timer;
        dispatcher.handlers[0x1d] = dm2_runtime_step_missile_timer;
        /* 0x5E direct free-slot spawn is bound only when the transferred
         * candidate still supplies the authenticated AI/GDAT owner.  The
         * source recycler and cross-map placement remain closed. */
        dispatcher.handlers[DM2_V1_TIMER_ALLOC_NEW_CREATURE] =
            dm2_runtime_alloc_new_creature_timer;
        (void)dm2_v1_proceed_timers(&rt->timer_queue,
                                    (uint32_t)rt->tick_count,
                                    &dispatcher,
                                    &rt->proceed_timers);
        /* Keep the runtime receipt tied to the dispatcher’s source-order
         * tally.  Actuator handlers own their family-specific counters, but
         * this aggregate must include the class-3 source no-op as well. */
        for (int tile_class = 0;
             tile_class < DM2_V1_TIMER_ACTUATOR_TILE_CLASSES; ++tile_class) {
            rt->actuator_tile_timers +=
                rt->proceed_timers.actuator_tile_tally[tile_class];
        }
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

    /* The former cache drained a module-private fixture list. No live G1
     * DB14 record, CCM payload and timer transaction currently own a DM2
     * projectile producer, so stepping it could only advance test-authored
     * missiles. Keep the viewport empty until the original STEP_MISSILE
     * owner is imported. Source: skproject/SKWIN/c_tim_proc.cpp
     * DM2_STEP_MISSILE; c_creature.cpp DM2_PROCEED_CCM. */
    g_dm2_projectile_drain_count = 0;
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

void dm2_v1_runtime_note_spell_cast_apply_receipt(
    const DM2_V1_SpellCastApplyReceipt *a)
{
    const DM2_V1_AssetLoader *loader;
    uint8_t *pixels;
    uint8_t palette16[16];
    uint32_t palette_hash = 0u;
    uint32_t pixel_hash = 2166136261u;
    DM2_V1_BootExpandedRectReceipt destination;
    size_t pixel_count;
    int width = 0;
    int height = 0;
    DM2_ImageFormat format = DM2_IMG_FMT_UNKNOWN;

    memset(&g_dm2_runtime.last_spell_failure_gdat, 0,
           sizeof(g_dm2_runtime.last_spell_failure_gdat));
    if (!a || !a->valid) {
        g_dm2_runtime.last_spell_status_scope = NULL;
        g_dm2_runtime.last_spell_status = NULL;
        g_dm2_runtime.last_spell_failure_class = 0;
        return;
    }
    if (a->failure_feedback && a->failure_class != 0) {
        /* SKProject skgame.cpp::PROCEED_SPELL_FAILURE does not format a
         * status string. It updates C068--C070/global panel state and, for
         * class 0x30, draws INTERFACE_GENERAL/SPELLING/NEED_FLASK into rect
         * 0x5c. Until that GDAT hint consumer is bound to the live session,
         * preserve the source failure class but publish no invented text. */
        g_dm2_runtime.last_spell_status_scope = NULL;
        g_dm2_runtime.last_spell_status = NULL;
        g_dm2_runtime.last_spell_failure_class = a->failure_class;
        /* SKProject SKWINSPX/src/v5/skevents.cpp::DM2_PROCEED_SPELL_FAILURE
         * calls DRAW_TRANSPARENT_STATIC_PIC(1, 5, 11, 92, NOALPHA) for
         * class 0x30. Resolve that exact source image from the admitted
         * GRAPHICS.DAT; never substitute a text label or another image. */
        if (a->failure_class == 0x30 && g_dm2_runtime.boot &&
            (loader = dm2_v1_boot_asset_loader(g_dm2_runtime.boot)) != NULL &&
            dm2_v1_boot_query_expanded_rect_receipt(
                g_dm2_runtime.boot, 0x5cu, &destination) && destination.valid &&
            dm2_v1_asset_load_image_local_palette(
                loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 5, 0x0b,
                palette16, &palette_hash)) {
            pixels = dm2_v1_asset_load_image_field(
                loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 5, 0x0b,
                &width, &height, &format);
            if (pixels && width > 0 && height > 0 && palette_hash != 0u) {
                pixel_count = (size_t)width * (size_t)height;
                for (size_t i = 0u; i < pixel_count; ++i) {
                    pixel_hash ^= pixels[i];
                    pixel_hash *= 16777619u;
                }
                if (pixel_hash != 0u) {
                    DM2_V1_RuntimeSpellFailureGdatReceipt *r =
                        &g_dm2_runtime.last_spell_failure_gdat;
                    r->valid = 1;
                    r->no_draw = 1;
                    r->source_bound = 1;
                    r->category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
                    r->entry_index = 5u;
                    r->image_field = 0x0bu;
                    r->destination_rect = 0x5cu;
                    r->width = (uint16_t)width;
                    r->height = (uint16_t)height;
                    r->destination_x = (int16_t)destination.rect.x;
                    r->destination_y = (int16_t)destination.rect.y;
                    r->destination_width = (uint16_t)destination.rect.w;
                    r->destination_height = (uint16_t)destination.rect.h;
                    r->format = format;
                    r->decoded_pixels_hash = pixel_hash;
                    r->palette_hash = palette_hash;
                    r->destination_table_hash = destination.raw4_hash;
                    r->identity_hash = pixel_hash ^ palette_hash ^
                        destination.receipt_hash ^
                        ((uint32_t)r->destination_rect << 16);
                    if (r->identity_hash == 0u) r->identity_hash = 1u;
                }
            }
            dm2_v1_asset_free_pixels(pixels);
        }
    } else {
        g_dm2_runtime.last_spell_status_scope = NULL;
        g_dm2_runtime.last_spell_status = NULL;
        g_dm2_runtime.last_spell_failure_class = 0;
    }
}

int dm2_v1_runtime_last_spell_failure_gdat_receipt(
    DM2_V1_RuntimeSpellFailureGdatReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    *out_receipt = g_dm2_runtime.last_spell_failure_gdat;
    return out_receipt->valid;
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

int dm2_v1_runtime_last_ccm_receipt(DM2_V1_CcmLoopReceipt *out)
{
    if (!out || !g_dm2_runtime.ccm_receipt_valid)
        return 0;
    *out = g_dm2_runtime.last_ccm_receipt;
    return 1;
}

int dm2_v1_runtime_dynamic_path_attempts(void)
{
    return g_dm2_runtime.dynamic_path_attempts;
}

int dm2_v1_runtime_dynamic_path_admissions(void)
{
    return g_dm2_runtime.dynamic_path_admissions;
}

int dm2_v1_runtime_dynamic_move_queue_admissions(void)
{
    return g_dm2_runtime.dynamic_move_queue_admissions;
}

int dm2_v1_runtime_dynamic_move_timer_consumptions(void)
{
    return g_dm2_runtime.dynamic_move_timer_consumptions;
}

int dm2_v1_runtime_dynamic_move_successes(void)
{
    return g_dm2_runtime.dynamic_move_successes;
}

int dm2_v1_runtime_dynamic_move_last_failure(void)
{
    return g_dm2_runtime.dynamic_move_last_failure;
}

int dm2_v1_runtime_dynamic_path_last_failure(void)
{
    return g_dm2_runtime.dynamic_path_last_failure;
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

static uint16_t dm2_runtime_read_u16_le(const uint8_t *src)
{
    return (uint16_t)((uint16_t)src[0] | ((uint16_t)src[1] << 8));
}

static int16_t dm2_runtime_read_i16_le(const uint8_t *src)
{
    return (int16_t)dm2_runtime_read_u16_le(src);
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
        dst->stamina_pct = 0u;
        dst->mana_pct = 0u;
        /* SKProject INIT sets glbChampionColor to 7,11,8,14 before its HUD
         * path. Keep the bootstrap value explicit until a source save/runtime
         * mutation of that global is independently admitted. */
        static const uint8_t source_default_stat_bar_color[
            DM2_V1_HUD_CHAMPION_SLOT_COUNT] = { 7u, 11u, 8u, 14u };
        dst->stat_bar_color = source_default_stat_bar_color[slot];
        dst->stat_bar_color_source_bound = 1;
        dst->state_source_bound = 0;
        dst->portrait_index = 0u;
        /* SKWINDOS/src/c_hero.h places herotype at byte 257 of the PC-DOS
         * 0x107-byte c_hero record.  REVIVE_PLAYER writes it from the source
         * mirror actuator and DRAW_CHAMPION_PICTURE uses that exact GDAT
         * index.  The local portrait_index tail is not a substitute. */
        dst->portrait_type_source_bound = 0;
        if (dst->occupied && rt->session_snapshot.original_champion_records_valid) {
            const uint8_t *raw = rt->session_snapshot
                .original_champion_records[slot];
            DM2_V1_ChampionStatInput stat_input;
            DM2_V1_ChampionStatBridgeReceipt stat_receipt;

            /* SKProject/SKWINSPX c_hero has the three current/max pairs at
             * 54/56, 58/60, and 62/64.  The old 261-byte convenience view
             * does not retain the latter two maxima, so it cannot own a
             * drawable stat bar.  DRAW_PLAYER_3STAT_HEALTH_BAR also expands
             * max MP to max(current MP, max MP); the shared source bridge
             * retains that rule. */
            memset(&stat_input, 0, sizeof(stat_input));
            stat_input.cur_hp = dm2_runtime_read_i16_le(raw + 54);
            stat_input.max_hp = dm2_runtime_read_u16_le(raw + 56);
            stat_input.cur_stamina = dm2_runtime_read_u16_le(raw + 58);
            stat_input.max_stamina = dm2_runtime_read_u16_le(raw + 60);
            stat_input.cur_mp = dm2_runtime_read_u16_le(raw + 62);
            stat_input.max_mp = dm2_runtime_read_u16_le(raw + 64);
            stat_input.is_leader = (uint8_t)dst->leader;
            if (dm2_v1_champion_stat_bridge_compute(
                    &stat_input, NULL, 1, dst->stat_bar_color,
                    &stat_receipt) && stat_receipt.valid) {
                dst->hp_pct = stat_receipt.champions[0].hp_pct;
                dst->stamina_pct = stat_receipt.champions[0].stamina_pct;
                dst->mana_pct = stat_receipt.champions[0].mana_pct;
                dst->state_source_bound = 1;
                dst->portrait_index = raw[257];
                dst->portrait_type_source_bound = 1;
            }
        }
        memcpy(dst->name, champ->first_name, DM2_V1_HUD_CHAMPION_NAME_MAX);
        dst->name[DM2_V1_HUD_CHAMPION_NAME_MAX] = '\0';
    }

    dm2_v1_viewport_set_hud_party(viewport, &hud);
}

/* ReDMCSB/skproject SkWinCore.cpp::DISPLAY_RIGHT_PANEL_SQUAD_HANDS keeps
 * the selected champion and selected hand in party.curacthero/curactmode,
 * then calls DRAW_HAND_ACTION_ICONS. Without a source-selected champion
 * there is no hand-action command to present. */
static void dm2_runtime_bind_source_hand_action(
    const DM2_V1_RuntimeState *rt, DM2_V1_ViewportState *viewport,
    int party_dir)
{
    const uint8_t *raw;
    const uint8_t *pixels = NULL;
    const uint8_t *raw4;
    size_t raw4_size = 0u;
    DM2_V1_GdatRaw4BlitPlacement placement;
    DM2_V1_HudHandActionSource source;
    int player_index;
    int gdat_index;
    int width = 0;
    int height = 0;
    int stride = 0;
    uint32_t raw4_hash = 2166136261u;

    if (!rt || !viewport || rt->outdoor || !rt->session_snapshot_valid ||
        !rt->boot || !rt->boot->graphics_dat ||
        !viewport->asset_fetch || !viewport->asset_loader ||
        !viewport->gdat_interface_palette_ready ||
        viewport->gdat_scene_map_load_token == 0u ||
        viewport->gdat_scene_control_hash == 0u ||
        viewport->gdat_interface_palette_hash == 0u ||
        rt->source_curacthero <= 0 ||
        rt->source_curacthero > rt->session_snapshot.champion_count ||
        rt->source_curactmode < 0 || rt->source_curactmode > 1) {
        return;
    }

    player_index = rt->source_curacthero - 1;
    raw = rt->session_snapshot.original_champion_records[player_index];
    if (raw[0x1d] > 3u) return;
    gdat_index = dm2_v1_viewport_hud_hand_action_graphic_index(
        rt->source_curactmode, 1);
    if (gdat_index == 0 ||
        viewport->asset_fetch(viewport->asset_user, gdat_index, &pixels,
                              &width, &height, &stride) != 0 ||
        !pixels || width <= 0 || height <= 0 || stride < width) {
        return;
    }
    raw4 = dm2_v1_asset_load_typed_sized(
        viewport->asset_loader, DM2_GDAT_CATEGORY_INTERFACE_GENERAL, 0,
        DM2_GDAT_ENTRY_TYPE_RAW4, 0, &raw4_size);
    if (!raw4 || raw4_size == 0u ||
        !dm2_v1_gdat_door_overlay_query_raw4_blit_placement(
            viewport->asset_loader,
            (uint16_t)((rt->source_curactmode == 1 ? 0x46 : 0x4a) +
                       (((int)raw[0x1d] + 4 - (party_dir & 3)) & 3)),
            width, height, &placement)) {
        return;
    }
    for (size_t i = 0u; i < raw4_size; ++i) {
        raw4_hash ^= (uint32_t)raw4[i] + 0x9e3779b9u +
            (raw4_hash << 6) + (raw4_hash >> 2);
        if (raw4_hash == 0u) raw4_hash = 1u;
    }
    if (raw4_hash == 0u) return;

    memset(&source, 0, sizeof(source));
    source.valid = 1;
    source.player_index = (uint8_t)player_index;
    source.possession_index = (uint8_t)rt->source_curactmode;
    source.left_or_right = 1u;
    source.player_position = (uint8_t)raw[0x1d];
    source.party_direction = (uint8_t)(party_dir & 3);
    source.gdat_category = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    source.gdat_subcategory = 4u;
    source.gdat_entry = (uint8_t)(2 + (rt->source_curactmode * 2) + 1);
    source.rectno = (uint8_t)((rt->source_curactmode == 1 ? 0x46 : 0x4a) +
        (((int)raw[0x1d] + 4 - (party_dir & 3)) & 3));
    /* c_hero::handcooldown is a signed byte at 0x2a. The source tests only
     * for non-zero here; retain the byte as a receipt and let the viewport
     * apply the original checker overlay. */
    source.hand_cooldown = raw[0x2a + rt->source_curactmode];
    source.gray_overlay_required = source.hand_cooldown != 0u ||
        rt->source_sleeping;
    source.map_load_token = viewport->gdat_scene_map_load_token;
    source.scene_control_hash = viewport->gdat_scene_control_hash;
    source.palette_hash = viewport->gdat_interface_palette_hash;
    source.raw4_hash = raw4_hash;
    source.source_rect.x = placement.source_x;
    source.source_rect.y = placement.source_y;
    source.source_rect.w = placement.destination.w;
    source.source_rect.h = placement.destination.h;
    source.destination_rect = placement.destination;

    /* DRAW_ITEM_ON_WOOD_PANEL follows the selected hero's real c_hero item
     * link and asks the mounted record pools for cls1/cls2.  Admit an item
     * image only when that link, its GDAT class, and a source command entry
     * all resolve. Charge-consuming commands remain no-draw until the live
     * charge owner is available; showing them would be an invented state. */
    if (rt->record_pools_valid) {
        const uint8_t *item_bytes = raw + 0xc3u +
            (size_t)rt->source_curactmode * sizeof(int16_t);
        uint16_t item_word = (uint16_t)item_bytes[0] |
            ((uint16_t)item_bytes[1] << 8);
        uint8_t cls1 = 0xffu;
        uint8_t cls2 = 0xffu;
        DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
        DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
        int action_found = 0;

        memset(&cls1_receipt, 0, sizeof(cls1_receipt));
        memset(&cls2_receipt, 0, sizeof(cls2_receipt));
        if (item_word != 0xffffu &&
            dm2_v1_skproject_query_cls1_from_record_ex(
                item_word, &rt->record_pools, &cls1, &cls1_receipt) &&
            dm2_v1_skproject_query_cls2_from_record(
                item_word, &rt->record_pools, &cls2, &cls2_receipt) &&
            cls1 >= DM2_GDAT_CATEGORY_WEAPONS &&
            cls1 <= DM2_GDAT_CATEGORY_MISCELLANEOUS) {
            /* SKWIN/SkWinCore.cpp::IS_ITEM_HAND_ACTIVABLE admits a DB9
             * container of ContainerType()==0 before scanning command
             * entries. IS_CONTAINER_MONEYBOX is the subset whose exact
             * CONTAINERS/cls2/dtText/0x40 entry exists; the remaining
             * type-0 containers are source-classified as chests. Preserve
             * that admission boundary from the authenticated record pool.
             * The GDAT query is deliberately made even for a chest so a
             * missing moneybox list cannot be replaced by a guessed item
             * classification. */
            if (cls1 == DM2_GDAT_CATEGORY_CONTAINERS &&
                dm2_v1_record_handle_pool((int16_t)item_word) == 9) {
                const uint8_t *container = dm2_v1_record_pool_address(
                    &rt->record_pools, (int16_t)item_word);
                int container_size = dm2_v1_record_pool_record_size(9);
                uint16_t moneybox_data_index = 0xffffu;
                int has_moneybox_item_list =
                    dm2_v1_query_gdat_entry_data_index(
                        viewport->asset_loader,
                        DM2_GDAT_CATEGORY_CONTAINERS, cls2,
                        DM2_GDAT_ENTRY_TYPE_TEXT, 0x40,
                        &moneybox_data_index) &&
                    moneybox_data_index != 0xffffu;

                /* Source IS_CONTAINER_MONEYBOX/IS_CONTAINER_CHEST both
                 * require DB9 and ContainerType()==0. Their distinction is
                 * only the presence of the authentic moneybox list; both
                 * are admitted by IS_ITEM_HAND_ACTIVABLE. */
                (void)has_moneybox_item_list;
                if (container && container_size >= 5 &&
                    ((container[4] >> 1) & 3u) == 0u) {
                    action_found = 1;
                }
            }
            if (!action_found) {
                for (int command_entry = 8; command_entry < 12;
                     ++command_entry) {
                DM2_V1_GdatEntryQueryReceipt loadable;
                DM2_V1_CmdstrEntryReceipt command;
                DM2_V1_CmdstrEntryReceipt where;
                DM2_V1_CmdstrEntryReceipt charges;

                memset(&loadable, 0, sizeof(loadable));
                memset(&command, 0, sizeof(command));
                memset(&where, 0, sizeof(where));
                memset(&charges, 0, sizeof(charges));
                if (!dm2_v1_query_gdat_entry_if_loadable(
                        viewport->asset_loader, cls1, cls2,
                        DM2_GDAT_ENTRY_TYPE_TEXT, command_entry, &loadable) ||
                    !loadable.loadable_raw ||
                    !dm2_v1_query_cmdstr_entry_receipt(
                        viewport->asset_loader, cls1, cls2, command_entry,
                        2, &command) || !command.found ||
                    command.value == 0 ||
                    !dm2_v1_query_cmdstr_entry_receipt(
                        viewport->asset_loader, cls1, cls2, command_entry,
                        17, &where)) {
                    continue;
                }
                if (where.found && where.value != 0 &&
                    where.value - 1 != rt->source_curactmode) {
                    continue;
                }
                if (!dm2_v1_query_cmdstr_entry_receipt(
                        viewport->asset_loader, cls1, cls2, command_entry,
                        8, &charges)) {
                    continue;
                }
                /* IS_ITEM_HAND_ACTIVABLE uses ADD_ITEM_CHARGE(si, 0) for
                 * NC 18, maps NC 16/17 to one charge, and compares every
                 * other positive NC directly with the current charge. Read
                 * w2 from the authenticated record pool and run the exact
                 * source helper with delta zero. The helper receives a local
                 * copy, so this probe cannot spend a charge while drawing. */
                if (charges.found && charges.value != 0) {
                    const uint8_t *record = dm2_v1_record_pool_address(
                        &rt->record_pools, (int16_t)item_word);
                    int pool = dm2_v1_record_handle_pool((int16_t)item_word);
                    int record_size = dm2_v1_record_pool_record_size(pool);
                    uint16_t record_w2;
                    DM2_V1_SkprojectItemChargeReceipt charge_receipt;
                    int required = charges.value == 16 ||
                        charges.value == 17 ? 1 : charges.value;

                    if (!record || record_size < 6 || required <= 0) {
                        continue;
                    }
                    record_w2 = (uint16_t)record[4] |
                        ((uint16_t)record[5] << 8);
                    memset(&charge_receipt, 0, sizeof(charge_receipt));
                    if (!dm2_v1_skproject_add_item_charge(
                            item_word, &record_w2, 0, &charge_receipt) ||
                        !charge_receipt.valid ||
                        (charges.value == 18
                             ? charge_receipt.new_charge == 0u
                             : charge_receipt.new_charge < (uint16_t)required)) {
                        continue;
                    }
                    source.item_charge_valid = 1u;
                    source.item_charge = (uint8_t)charge_receipt.new_charge;
                    source.item_charge_required = (uint8_t)required;
                }
                action_found = 1;
                break;
                }
            }
        }
        if (action_found) {
            int item_gdat_index = dm2_v1_viewport_item_graphic_index(
                cls1, cls2, 0x18);
            const uint8_t *item_pixels = NULL;
            int item_width = 0;
            int item_height = 0;
            int item_stride = 0;

            if (item_gdat_index != 0 &&
                viewport->asset_fetch(viewport->asset_user,
                    item_gdat_index, &item_pixels, &item_width,
                    &item_height, &item_stride) == 0 && item_pixels &&
                item_width > 0 && item_height > 0 &&
                item_stride >= item_width) {
                source.item_present = 1u;
                source.item_gdat_index = item_gdat_index;
            }
        }
    }
    dm2_v1_viewport_set_hud_hand_action_source(viewport, &source);
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
    uint32_t scene_map_token = 0u;
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
    if (rt->outdoor &&
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
    /* UPDATE_GFXSET binds the current map and its decoded GRAPHICSSET
     * records as one transaction.  The renderer's static-scene gates already
     * model that source contract, but a live DM2 frame previously supplied
     * the records without installing the matching map token.  That left the
     * otherwise authentic floor/ceiling/wall plans unable to prove their
     * ownership.  Bind every static record against this exact current map;
     * any failed binding rejects the frame instead of drawing a fallback.
     * Source: SKWINSPX/src/v5/c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_DYN,
     *         SKULLWIN/c_gui_vp.cpp::DM2_DISPLAY_VIEWPORT. */
    if (rt->gdat_scene_control_ready) {
        scene_map_token = dm2_v1_runtime_g1_scene_map_token(
            rt->dungeon_level, rt->map_graphics_style, rt->outdoor);
        dm2_v1_viewport_set_scene_map_load_token(&viewport, scene_map_token);
        if (scene_map_token == 0u ||
            !dm2_v1_viewport_bind_static_graphicsset_scene_record(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_light_control(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_ambient_light_control(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_ambient_darkness_control(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_flags_control(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_colorkey_control(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_floor_material(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_ceiling_material(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_all_wall_materials(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_door_frame_material(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_door_frame_d1c_material(
                &viewport, scene_map_token, rt->gdat_scene_control_hash) ||
            !dm2_v1_viewport_bind_static_scene_door_frame_d2c_material(
                &viewport, scene_map_token, rt->gdat_scene_control_hash)) {
            return -1;
        }
    }
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
            rt->boot, 0, c_light_parameter,
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
        &viewport, 0);
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
    dm2_runtime_bind_source_hand_action(rt, &viewport, party_dir);
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
    if (viewport.hud_party_valid && !rt->outdoor) {
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
    rt->source_click_target_count = viewport.source_click_target_count;
    if (rt->source_click_target_count > DM2_V1_VIEWPORT_CLICK_TARGET_COUNT) {
        rt->source_click_target_count = DM2_V1_VIEWPORT_CLICK_TARGET_COUNT;
    }
    memcpy(rt->source_click_targets, viewport.source_click_targets,
           (size_t)rt->source_click_target_count *
               sizeof(rt->source_click_targets[0]));
    if (rt->source_click_target_count < DM2_V1_VIEWPORT_CLICK_TARGET_COUNT) {
        memset(rt->source_click_targets + rt->source_click_target_count, 0,
               (size_t)(DM2_V1_VIEWPORT_CLICK_TARGET_COUNT -
                        rt->source_click_target_count) *
                   sizeof(rt->source_click_targets[0]));
    }
    dm2_v1_runtime_append_mac_wall_targets(
        (const DM2_V1_DungeonData *)rt->boot->dungeon_data,
        (const DM2_V1_GameState *)rt->boot->dm2_state);
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
    if (rt->boot && rt->boot->platform == DM2_PLATFORM_FMTOWNS_JA &&
        !hud_material_plan_consumed) {
        /* FM Towns GRAPHICS.DAT has a distinct native HUD layout; this frame
         * did not draw a host/PC HUD command, so do not mark a missing HUD
         * transaction as required for the viewport receipt. */
        hud_material_plan_required = 0;
    }
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
        (g_dm2_frame_ownership.hud_gdat_blits > 0 ||
         (rt->boot && rt->boot->platform == DM2_PLATFORM_FMTOWNS_JA)) &&
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
    /* Some FM Towns poses have no visible wall cell, so the renderer does not
     * consume the cached wall command plan during its tile loop.  Rebind the
     * already authenticated map plan before publishing the frame receipt; a
     * no-blit pose must not erase the source wall identity. */
    if (!viewport.is_outdoor && !rt->gdat_wall_material_plan.valid &&
        rt->map_graphics_style >= 0 && rt->boot) {
        (void)dm2_v1_boot_gdat_wall_m11_command_plan(
            rt->boot, rt->map_graphics_style, &rt->gdat_wall_material_plan);
    }
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
        rt->outdoor ? 0 :
        (rt->gdat_wall_material_plan.valid
             ? rt->gdat_wall_material_plan.command_count
             : wall_material_plan_command_count);
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
    /* PC GDAT carries dt07/2 for its action-palette transform.  The native
     * 16-colour Amiga and FM Towns variants instead draw with their selected
     * physical palette entries; neither contains that PC-only table. */
    g_dm2_last_m11_frame.interface_action_palette_required =
        !(rt->boot && (rt->boot->platform == DM2_PLATFORM_AMIGA_EN ||
                       rt->boot->platform == DM2_PLATFORM_FMTOWNS_JA));
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
         !g_dm2_last_m11_frame.interface_action_palette_required ||
         (g_dm2_last_m11_frame.interface_action_palette_hash != 0u &&
          g_dm2_last_m11_frame.interface_action_palette_consumed) ||
         (rt->boot && rt->boot->platform == DM2_PLATFORM_FMTOWNS_JA &&
          (!g_dm2_last_m11_frame.hud_material_plan_required ||
           g_dm2_last_m11_frame.hud_material_plan_consumed)));
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

int dm2_v1_runtime_route_viewport_click(
    int screen_x, int screen_y,
    DM2_V1_RuntimeViewportClickReceipt *out_receipt)
{
    DM2_V1_RuntimeViewportClickReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.target_index = -1;
    receipt.object_id = -1;
    receipt.view_slot = -1;
    if (out_receipt) *out_receipt = receipt;

    /* c_events.cpp::CLICK_VWPT is downstream of DRAW_VIEWPORT.  Do not
     * resolve stale rectangles from an incomplete or rejected frame. */
    if (!g_dm2_last_m11_frame.valid ||
        g_dm2_runtime.source_click_target_count == 0u) {
        return 0;
    }
    for (int i = 0; i < (int)g_dm2_runtime.source_click_target_count; ++i) {
        const DM2_V1_ViewportClickTarget *target =
            &g_dm2_runtime.source_click_targets[i];
        if (screen_x < target->x || screen_y < target->y ||
            screen_x >= target->x + target->w ||
            screen_y >= target->y + target->h) {
            continue;
        }
        receipt.valid = 1;
        receipt.accepted = 1;
        receipt.target_index = i;
        receipt.target_kind = target->target_kind;
        receipt.object_id = target->object_id;
        receipt.view_slot = target->view_slot;
        receipt.rect.x = target->x;
        receipt.rect.y = target->y;
        receipt.rect.w = target->w;
        receipt.rect.h = target->h;
        if (out_receipt) *out_receipt = receipt;
        return 1;
    }
    return 0;
}

static int dm2_v1_runtime_mac_wall_target_exists(int view_slot)
{
    for (int i = 0; i < (int)g_dm2_runtime.source_click_target_count; ++i) {
        if ((int)g_dm2_runtime.source_click_targets[i].view_slot == view_slot &&
            g_dm2_runtime.source_click_targets[i].target_kind == 4u) {
            return 1;
        }
    }
    return 0;
}

/* Some Macintosh wall controls are standalone DB3 mechanisms rather than
 * door overlays. Publish a clickable source target only after the live map
 * and record graph prove that the visible wall cell owns one. */
static void dm2_v1_runtime_append_mac_wall_targets(
    const DM2_V1_DungeonData *dungeon, const DM2_V1_GameState *game)
{
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct { int square; int forward; int lateral; } cells[] = {
        { DM2_SQ_D0L, 0, -1 }, { DM2_SQ_D0C, 1, 0 },
        { DM2_SQ_D0R, 0, 1 }, { DM2_SQ_D1L, 1, -1 },
        { DM2_SQ_D1C, 2, 0 }, { DM2_SQ_D1R, 1, 1 },
        { DM2_SQ_D2L, 2, -1 }, { DM2_SQ_D2C, 3, 0 },
        { DM2_SQ_D2R, 2, 1 }
    };

    if (!dungeon || !game || dungeon->level_count <= 0 ||
        !g_dm2_runtime.record_pools_valid ||
        g_dm2_runtime.source_click_target_count >=
            DM2_V1_VIEWPORT_CLICK_TARGET_COUNT) return;
    for (size_t i = 0; i < sizeof(cells) / sizeof(cells[0]); ++i) {
        const int square = cells[i].square;
        DM2_V1_ViewportRect rect;
        int x, y;
        int16_t walk;
        int16_t source = DM2_V1_RECORD_HANDLE_NULL;
        int steps = 0;

        if (dm2_v1_runtime_mac_wall_target_exists(square) ||
            !dm2_v1_viewport_wall_frame_rect_for_square(square, &rect)) continue;
        x = game->party_x + dx[game->party_dir & 3] * cells[i].forward -
            dy[game->party_dir & 3] * cells[i].lateral;
        y = game->party_y + dy[game->party_dir & 3] * cells[i].forward +
            dx[game->party_dir & 3] * cells[i].lateral;
        if (x < 0 || y < 0 || x >= dungeon->level_widths[g_dm2_runtime.dungeon_level] ||
            y >= dungeon->level_heights[g_dm2_runtime.dungeon_level] ||
            dm2_v1_dungeon_get_square_type(
                dungeon, g_dm2_runtime.dungeon_level, x, y) != 0) continue;
        walk = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, g_dm2_runtime.dungeon_level, x, y);
        while ((uint16_t)walk != (uint16_t)DM2_V1_RECORD_HANDLE_END &&
               (uint16_t)walk != (uint16_t)DM2_V1_RECORD_HANDLE_NULL &&
               steps++ < 256) {
            const uint8_t *record = dm2_v1_record_pool_address(
                &g_dm2_runtime.record_pools, walk);
            int16_t next;
            if (record && dm2_v1_record_handle_pool(walk) == DM2_DB_ACTUATOR) {
                uint8_t type = dm2_actu_type(record);
                if (type == DM2_ACTU_2_STATE_SWITCH ||
                    type == DM2_ACTU_WALL_SWITCH || type == DM2_ACTU_KEY_HOLE) {
                    source = walk;
                    break;
                }
            }
            if (!dm2_v1_record_pool_next_link(
                    &g_dm2_runtime.record_pools, walk, &next)) break;
            walk = next;
        }
        if ((uint16_t)source == (uint16_t)DM2_V1_RECORD_HANDLE_NULL) continue;
        if (g_dm2_runtime.source_click_target_count >=
            DM2_V1_VIEWPORT_CLICK_TARGET_COUNT) return;
        {
            DM2_V1_ViewportClickTarget *target =
                &g_dm2_runtime.source_click_targets[
                    g_dm2_runtime.source_click_target_count++];
            target->x = (int16_t)rect.x;
            target->y = (int16_t)rect.y;
            target->w = (int16_t)rect.w;
            target->h = (int16_t)rect.h;
            target->object_id = source;
            target->view_slot = (uint8_t)square;
            target->target_kind = 4u;
        }
    }
}

typedef struct {
    DM2_V1_DungeonData *dungeon;
    DM2_V1_RecordPoolSet *pools;
    int level;
} DM2_V1_MacWallRotateContext;

static int16_t dm2_v1_mac_wall_get_tile_link(void *ctx, int16_t x, int16_t y)
{
    DM2_V1_MacWallRotateContext *c = (DM2_V1_MacWallRotateContext *)ctx;
    if (!c || !c->dungeon) return DM2_V1_RECORD_HANDLE_END;
    return (int16_t)dm2_v1_dungeon_get_first_thing(c->dungeon, c->level, x, y);
}

static int16_t dm2_v1_mac_wall_get_next_link(void *ctx, uint16_t rw)
{
    DM2_V1_MacWallRotateContext *c = (DM2_V1_MacWallRotateContext *)ctx;
    int16_t next = DM2_V1_RECORD_HANDLE_END;
    if (!c || !c->pools || !dm2_v1_record_pool_next_link(
            c->pools, (int16_t)rw, &next)) return DM2_V1_RECORD_HANDLE_END;
    return next;
}

static void dm2_v1_mac_wall_set_next_link(void *ctx, uint16_t rw,
                                           int16_t next)
{
    DM2_V1_MacWallRotateContext *c = (DM2_V1_MacWallRotateContext *)ctx;
    uint8_t *record;
    if (!c || !c->pools) return;
    record = dm2_v1_record_pool_address_mut(c->pools, (int16_t)rw);
    if (!record) return;
    record[0] = (uint8_t)((uint16_t)next & 0xffu);
    record[1] = (uint8_t)(((uint16_t)next >> 8) & 0xffu);
}

static void dm2_v1_mac_wall_set_tile_link(void *ctx, int16_t x, int16_t y,
                                           int16_t rw)
{
    DM2_V1_MacWallRotateContext *c = (DM2_V1_MacWallRotateContext *)ctx;
    if (!c || !c->dungeon) return;
    (void)dm2_v1_dungeon_set_first_thing(c->dungeon, c->level, x, y,
                                         (uint16_t)rw);
}

int dm2_v1_runtime_activate_mac_wall_button(
    int column, DM2_V1_RuntimeMacWallButtonReceipt *out_receipt)
{
    DM2_V1_RuntimeMacWallButtonReceipt receipt;
    const DM2_V1_DungeonData *dungeon;
    const DM2_V1_GameState *game;
    int target_index = -1;
    int target_slot = -1;
    int target_x = -1;
    int target_y = -1;
    static const int dx[4] = { 0, 1, 0, -1 };
    static const int dy[4] = { -1, 0, 1, 0 };
    static const struct { int square; int forward; int lateral; } cells[] = {
        { DM2_SQ_D0L, 0, -1 }, { DM2_SQ_D0C, 1, 0 },
        { DM2_SQ_D0R, 0, 1 }, { DM2_SQ_D1L, 1, -1 },
        { DM2_SQ_D1C, 2, 0 }, { DM2_SQ_D1R, 1, 1 },
        { DM2_SQ_D2L, 2, -1 }, { DM2_SQ_D2C, 3, 0 },
        { DM2_SQ_D2R, 2, 1 }
    };

    memset(&receipt, 0, sizeof(receipt));
    receipt.column = column;
    receipt.source_target_index = -1;
    receipt.view_slot = -1;
    receipt.map = -1;
    receipt.x = -1;
    receipt.y = -1;
    if (column < 0 || column > 2 || !g_dm2_runtime.boot ||
        !g_dm2_runtime.boot->source_game_load_session_ready ||
        !g_dm2_last_m11_frame.valid || !g_dm2_runtime.source_party_valid ||
        !g_dm2_runtime.record_pools_valid ||
        !(dungeon = (const DM2_V1_DungeonData *)g_dm2_runtime.boot->dungeon_data) ||
        !(game = (const DM2_V1_GameState *)g_dm2_runtime.boot->dm2_state)) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    for (int i = 0; i < (int)g_dm2_runtime.source_click_target_count; ++i) {
        const DM2_V1_ViewportClickTarget *target =
            &g_dm2_runtime.source_click_targets[i];
        const int centre_x = target->x + target->w / 2;
        const int target_column = centre_x < DM2_VP_WIDTH / 3 ? 0 :
            (centre_x < (DM2_VP_WIDTH * 2) / 3 ? 1 : 2);
        if (target->target_kind == 4u &&
            ((g_dm2_mac_wall_requested_target >= 0 &&
              i == g_dm2_mac_wall_requested_target) ||
             (g_dm2_mac_wall_requested_target < 0 &&
              target_column == column))) {
            target_index = i;
            target_slot = target->view_slot;
            break;
        }
    }
    if (target_index < 0) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    for (size_t i = 0; i < sizeof(cells) / sizeof(cells[0]); ++i) {
        if (cells[i].square != target_slot) continue;
        target_x = game->party_x +
            dx[game->party_dir & 3] * cells[i].forward -
            dy[game->party_dir & 3] * cells[i].lateral;
        target_y = game->party_y +
            dy[game->party_dir & 3] * cells[i].forward +
            dx[game->party_dir & 3] * cells[i].lateral;
        break;
    }
    if (target_x < 0 || target_y < 0 ||
        target_x >= dungeon->level_widths[g_dm2_runtime.dungeon_level] ||
        target_y >= dungeon->level_heights[g_dm2_runtime.dungeon_level]) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }
    {
        DM2_V1_MacWallRotateContext rotate_ctx;
        DM2_V1_ActuatorRotateCallbacks rotate_cb;
        uint16_t source_object = (uint16_t)
            g_dm2_runtime.source_click_targets[target_index].object_id;
        uint16_t source_handle = (uint16_t)DM2_V1_RECORD_HANDLE_NULL;
        uint8_t source_type = 0xffu;
        uint8_t source_local = 0u;
        int16_t walk;
        int walk_steps = 0;
        DM2_V1_ActuatorEventReceipt event;
        uint32_t hash = 2166136261u;

        /* Resolve the source actuator in the live tile chain. A column alone
         * is insufficient: c_rwbb owns the mechanism's DB3 record. */
        walk = (int16_t)dm2_v1_dungeon_get_first_thing(
            dungeon, g_dm2_runtime.dungeon_level, target_x, target_y);
        while ((uint16_t)walk != (uint16_t)DM2_V1_RECORD_HANDLE_END &&
               (uint16_t)walk != (uint16_t)DM2_V1_RECORD_HANDLE_NULL &&
               walk_steps++ < 256) {
            const uint8_t *record = dm2_v1_record_pool_address(
                &g_dm2_runtime.record_pools, walk);
            int16_t next;
            if (record && dm2_v1_record_handle_pool(walk) == DM2_DB_ACTUATOR &&
                ((uint16_t)walk == source_object ||
                 (uint16_t)source_handle ==
                     (uint16_t)DM2_V1_RECORD_HANDLE_NULL)) {
                source_handle = (uint16_t)walk;
                source_type = dm2_actu_type(record);
                source_local = (uint8_t)((dm2_actu_w4(record) >> 11) & 1u);
                if ((uint16_t)walk == source_object) break;
            }
            if (!dm2_v1_record_pool_next_link(
                    &g_dm2_runtime.record_pools, walk, &next)) break;
            walk = next;
        }
        if ((uint16_t)source_handle ==
            (uint16_t)DM2_V1_RECORD_HANDLE_NULL) {
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        receipt.source_actuator_type = source_type;
        receipt.source_local_action = source_local;

        if (source_type != DM2_ACTU_PUSH_BUTTON_SWITCH && source_local &&
            (source_type == DM2_ACTU_2_STATE_SWITCH ||
             source_type == DM2_ACTU_WALL_SWITCH ||
             source_type == DM2_ACTU_KEY_HOLE)) {
            memset(&rotate_cb, 0, sizeof(rotate_cb));
            rotate_ctx.dungeon = (DM2_V1_DungeonData *)dungeon;
            rotate_ctx.pools = &g_dm2_runtime.record_pools;
            rotate_ctx.level = g_dm2_runtime.dungeon_level;
            rotate_cb.get_tile_record_link = dm2_v1_mac_wall_get_tile_link;
            rotate_cb.get_next_record_link = dm2_v1_mac_wall_get_next_link;
            rotate_cb.set_next_record_link = dm2_v1_mac_wall_set_next_link;
            rotate_cb.set_tile_record_link = dm2_v1_mac_wall_set_tile_link;
            receipt.local_actuator_list_rotated = dm2_v1_rotate_actuator_list(
                target_x, target_y,
                (uint8_t)(((uint16_t)source_handle >> 14) & 3u),
                &rotate_cb, &rotate_ctx);
            hash ^= (uint32_t)target_index; hash *= 16777619u;
            hash ^= (uint32_t)source_handle; hash *= 16777619u;
            hash ^= (uint32_t)source_type; hash *= 16777619u;
            hash ^= (uint32_t)receipt.local_actuator_list_rotated;
            receipt.valid = 1;
            receipt.accepted = 1;
            receipt.source_target_index = target_index;
            receipt.view_slot = target_slot;
            receipt.map = g_dm2_runtime.dungeon_level;
            receipt.x = target_x;
            receipt.y = target_y;
            receipt.actuators_seen = 1;
            receipt.source_receipt_hash = hash;
            if (out_receipt) *out_receipt = receipt;
            return 1;
        }
        if (source_type != DM2_ACTU_PUSH_BUTTON_SWITCH) {
            receipt.blocked_unsupported_source_type = 1;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        memset(&event, 0, sizeof(event));
        if (!dm2_v1_push_button_switch_chain(
                &g_dm2_runtime.record_pools,
                (DM2_V1_DungeonData *)dungeon,
                g_dm2_runtime.dungeon_level, target_x, target_y,
                DM2_ACTMSG_TOGGLE, &event)) {
            receipt.blocked_incomplete_chain = event.fail_closed;
            if (out_receipt) *out_receipt = receipt;
            return 0;
        }
        hash ^= (uint32_t)target_index; hash *= 16777619u;
        hash ^= (uint32_t)target_slot; hash *= 16777619u;
        hash ^= (uint32_t)event.door_bit13_toggled; hash *= 16777619u;
        receipt.valid = 1;
        receipt.accepted = 1;
        receipt.source_target_index = target_index;
        receipt.view_slot = target_slot;
        receipt.map = g_dm2_runtime.dungeon_level;
        receipt.x = target_x;
        receipt.y = target_y;
        receipt.actuators_seen = event.push_button_invoked;
        receipt.doors_mutated = event.door_bit13_toggled;
        receipt.source_receipt_hash = hash;
    }
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

int dm2_v1_runtime_activate_mac_wall_target(
    int target_index, DM2_V1_RuntimeMacWallButtonReceipt *out_receipt)
{
    const DM2_V1_ViewportClickTarget *target;
    int centre_x;
    int column;
    int result;

    if (target_index < 0 ||
        target_index >= (int)g_dm2_runtime.source_click_target_count) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    target = &g_dm2_runtime.source_click_targets[target_index];
    if (target->target_kind != 4u || target->w <= 0 || target->h <= 0) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    centre_x = target->x + target->w / 2;
    column = centre_x < DM2_VP_WIDTH / 3 ? 0 :
        (centre_x < (DM2_VP_WIDTH * 2) / 3 ? 1 : 2);
    g_dm2_mac_wall_requested_target = target_index;
    result = dm2_v1_runtime_activate_mac_wall_button(column, out_receipt);
    g_dm2_mac_wall_requested_target = -1;
    return result;
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

int dm2_v1_runtime_last_wield_death_drop_count(void) {
    return g_dm2_last_wield_death_drop_count;
}

int dm2_v1_runtime_last_wield_death_drop_iterations(void) {
    return g_dm2_last_wield_death_drop_iterations;
}

int dm2_v1_runtime_last_wield_death_drop_alloc_failures(void) {
    return g_dm2_last_wield_death_drop_alloc_failures;
}

int dm2_v1_runtime_last_wield_death_drop_first_itemspec(void) {
    return g_dm2_last_wield_death_drop_first_itemspec;
}

int dm2_v1_runtime_last_wield_death_drop_first_db(void) {
    return g_dm2_last_wield_death_drop_first_db;
}

int dm2_v1_runtime_last_wield_death_drop_alloc_free_records(void) {
    return g_dm2_last_wield_death_drop_alloc_free_records;
}

int dm2_v1_runtime_last_wield_death_deallocated(void) {
    return g_dm2_last_wield_death_deallocated;
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

int dm2_v1_runtime_last_missile_impact_receipt(
    DM2_V1_RuntimeMissileImpactReceipt *out_receipt) {
    if (!out_receipt || !g_dm2_last_missile_impact.valid) {
        if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
        return 0;
    }
    *out_receipt = g_dm2_last_missile_impact;
    return 1;
}

int dm2_v1_runtime_last_creature_damage_receipt(
    DM2_V1_RuntimeCreatureDamageReceipt *out) {
    if (!out || !g_dm2_last_creature_damage.valid) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    *out = g_dm2_last_creature_damage;
    return 1;
}

int dm2_v1_runtime_last_wield_attack_receipt(
    DM2_V1_RuntimeWieldAttackReceipt *out) {
    if (!out || !g_dm2_last_wield_attack.valid) {
        if (out) memset(out, 0, sizeof(*out));
        return 0;
    }
    *out = g_dm2_last_wield_attack;
    return 1;
}

int dm2_v1_runtime_creature_record_receipt(
    int16_t record_handle, DM2_V1_RuntimeCreatureRecordReceipt *out)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    const uint8_t *record;
    const DM2_AIDefinition *ai = NULL;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!rt->record_pools_valid ||
        dm2_v1_record_handle_pool(record_handle) != 4)
        return 0;
    record = dm2_v1_record_pool_address(&rt->record_pools, record_handle);
    if (!record || rt->record_pools.pools[4].record_size < 8)
        return 0;
    if (!dm2_v1_creature_ai_spec_def((int)record[4], &ai) || !ai)
        return 0;
    out->valid = 1;
    out->record_handle = record_handle;
    out->creature_type = record[4];
    out->record_word = dm2_runtime_missile_rd16(&rt->record_pools, record);
    out->possession_head = dm2_runtime_missile_rd16(
        &rt->record_pools, record + 2);
    out->hp = dm2_runtime_missile_rd16(&rt->record_pools, record + 6);
    out->base_hp = (uint16_t)ai->BaseHP;
    out->caii_slot = record[5];
    if (record[5] != 0xffu && record[5] < rt->caii.capacity &&
        rt->caii.slots) {
        const uint8_t *slot = rt->caii.slots +
            (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
        out->pending_damage = (uint16_t)(slot[0x14] |
            ((uint16_t)slot[0x15] << 8));
    }
    out->kill_flag = (out->record_word & 1u) != 0u;
    out->drop_slots_loaded = dm2_v1_creature_drop_slots_loaded(
        out->creature_type);
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

static int dm2_runtime_gameplay_source_ready(const DM2_V1_RuntimeState *rt);

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
    /* `can_move` is a public input predicate, not merely the local cooldown
     * test.  Before DM2_GAME_LOAD has installed the one source-owned party,
     * record-pool, possession and timer graph, reporting that a party can
     * move gives callers a false playable state even though move() rejects
     * the command later.  SKProject reaches c_move only after GAME_LOAD has
     * completed (sksvgame.cpp::DM2_GAME_LOAD lines 1415-1546); keep this
     * predicate at the same boundary as move() and turn(). */
    return dm2_runtime_gameplay_source_ready(rt) &&
        rt->move_cooldown_ticks == 0;
}

/* SKProject enters c_move only after DM2_GAME_LOAD has mounted both original
 * owners and installed the GDAT callbacks. Keep input from manufacturing a
 * traversable host grid when a caller has supplied only a fixture dungeon.
 * This is deliberately the same source boundary as runtime_render_frame(). */
static int dm2_runtime_gameplay_source_ready(const DM2_V1_RuntimeState *rt)
{
    return rt && rt->boot && rt->boot->assets_verified &&
        /* A mounted File_header/GDAT pair is presentation evidence, not a
         * playable world. c_savegame.cpp::DM2_GAME_LOAD must first own the
         * party, record pools, possessions and timer queue together. */
        rt->boot->source_game_load_session_ready &&
        rt->boot->graphics_dat && rt->boot->dungeon_data &&
        rt->viewport_asset_fetch == dm2_v1_boot_viewport_asset_fetch &&
        rt->viewport_asset_user == rt->boot &&
        rt->viewport_asset_palette_fetch ==
            dm2_v1_boot_viewport_asset_palette_fetch &&
        rt->viewport_asset_palette_user == rt->boot;
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
    int entered_stairs = 0;
    int stair_map = -1;
    int stair_x = -1;
    int stair_y = -1;
    int entered_pit = 0;
    int pit_map = -1;
    int pit_x = -1;
    int pit_y = -1;
    int pit_status = 0;

    if (!dm2_runtime_gameplay_source_ready(rt) || !rt->boot->dm2_state) {
        return -1;
    }
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
            int tile_type = dm2_runtime_normalize_square_type_for_dungeon(
                dd, raw_tile_type, raw);
            move_request.target_raw_valid = 1;
            move_request.target_raw = raw;
            move_request.target_square_type = tile_type;
            if (tile_type == DM2_SQUARE_PIT) {
                pit_status = dm2_runtime_resolve_entered_pit(
                    dd, rt->dungeon_level, nx, ny,
                    &pit_map, &pit_x, &pit_y);
                if (pit_status > 0)
                    move_request.target_pit_transition_admitted = 1;
            }
            /* Impassable tile types: wall, pit, lava, inaccessible.
             * Normalized to DM2_SquareType enum before comparison. */
            if (tile_type == DM2_SQUARE_WALL ||
                (tile_type == DM2_SQUARE_PIT && pit_status <= 0) ||
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
            /* skmove.cpp:477-543 classifies a creature target before the
             * normal MOVE_RECORD_TO commit. The party must never be written
             * onto a live DB4 cell while that push/attack owner is absent. */
            if (!blocked && rt->record_pools_valid) {
                int16_t creature = dm2_v1_get_creature_at(
                    &rt->record_pools, dd, rt->dungeon_level, nx, ny);
                if (creature != DM2_V1_RECORD_HANDLE_NULL) {
                    /* A source attack consumes the move attempt.  A miss is
                     * still a blocked step; only the creature-side owner is
                     * allowed to change DB4/CAII state. */
                    (void)dm2_runtime_attack_creature_at(
                        rt, dd, rt->dungeon_level, old_x, old_y,
                        nx, ny, creature, 0);
                    blocked = 1;
                }
            }
            /* All other tile types (1=floor, 3=floor_ornate,
             * 8=teleporter, 10=water, etc.) are passable. */
        }
    }
    if (rt->outdoor) {
        move_request.target_raw_valid = 1;
    }
    if (!dm2_v1_DM2_PERFORM_MOVE_plan(&move_request, &move_receipt)) {
        memset(&g_dm2_last_perform_move, 0, sizeof(g_dm2_last_perform_move));
        return -1;
    }
    blocked = move_receipt.blocked;
    if (!blocked && pit_status > 0)
        entered_pit = 1;

    if (!blocked && !entered_pit && !rt->outdoor && rt->boot->dungeon_data) {
        int stair_status = dm2_runtime_resolve_entered_stairs(
            (DM2_V1_DungeonData *)rt->boot->dungeon_data,
            rt->dungeon_level, nx, ny, &stair_map, &stair_x, &stair_y);
        if (stair_status < 0) {
            /* Source movement does not leave the party stranded on an
             * unresolved ladder. Reject the step until the source-owned
             * destination can be proven from the loaded map corpus. */
            blocked = 1;
            move_receipt.blocked = 1;
        } else if (stair_status > 0) {
            entered_stairs = 1;
        }
    }
    g_dm2_last_perform_move = move_receipt;

    if (!blocked) {
        /* Fire smooth movement callback before updating state.
         * This gives the V2 layer the from/to positions for interpolation.
         * Source: Phase 5 runtime binding */
        if (!entered_stairs && !entered_pit && rt->move_callback) {
            rt->move_callback(old_x, old_y, nx, ny);
        }
        /* SKProject keeps the old pose and its glbIsPlayerMoving countdown
         * until PERFORM_MOVE commits the step. This V1 state has neither the
         * source hero/inventory inputs needed for CALC_PLAYER_WALK_DELAY nor
         * that delayed pose owner, so it must not invent a one-frame 700/701
         * viewport offset after an immediate move. */
        gs->party_x = entered_stairs ? stair_x :
                      (entered_pit ? pit_x : nx);
        gs->party_y = entered_stairs ? stair_y :
                      (entered_pit ? pit_y : ny);
        if (entered_stairs || entered_pit) {
            int special_map = entered_stairs ? stair_map : pit_map;
            int special_x = entered_stairs ? stair_x : pit_x;
            int special_y = entered_stairs ? stair_y : pit_y;
            gs->current_level = special_map;
            gs->outdoor = dm2_v1_dungeon_is_outdoor(
                (DM2_V1_DungeonData *)rt->boot->dungeon_data, special_map);
            rt->dungeon_level = special_map;
            rt->outdoor = gs->outdoor;
            rt->view_dir = gs->party_dir;
            dm2_runtime_refresh_map_transition_context(rt);
            if (rt->stairs_callback) {
                rt->stairs_callback(old_x, old_y, special_x, special_y,
                                    (float)(special_map - move_request.current_level));
            }
        }
        /* c_moverec.cpp resolves an enabled DB1 teleporter after the party
         * enters its source tile. Keep the transition behind the complete
         * source-owned 2fcf gate; ordinary movement remains unchanged. */
        dm2_runtime_apply_entered_db1_teleporter(
            rt, gs, rt->dungeon_level,
            entered_stairs ? stair_x : (entered_pit ? pit_x : nx),
            entered_stairs ? stair_y : (entered_pit ? pit_y : ny));
        /* CD.DAT's trigger keys include the party's exact square. Re-evaluate
         * only CDDA after a committed step; PC HMP map music remains a
         * level-transition concern. */
        if (dm2_v1_platform_music_system(rt->boot->platform) ==
            DM2_MUSIC_SYSTEM_CDDA_COORD) {
            dm2_runtime_refresh_music_map_trigger(rt);
        }
        for (int i = 1; i <= dm2_v1_trigger_get_builtin_count(); ++i) {
            DM2_V1_TriggerEvent event;
            const DM2_V1_Trigger *trigger =
                dm2_v1_trigger_get_builtin(i);
            if (trigger &&
                trigger->kind == DM2_TRIGGER_KIND_SQUARE_ENTERED &&
                trigger->arg_map_x == (entered_stairs ? stair_x :
                                      (entered_pit ? pit_x : nx)) &&
                trigger->arg_map_y == (entered_stairs ? stair_y :
                                      (entered_pit ? pit_y : ny)) &&
                trigger->arg_map_level == rt->dungeon_level &&
                dm2_v1_trigger_fire(trigger->trigger_id) ==
                    (int)DM2_TRIGGER_RESULT_OK &&
                dm2_v1_trigger_copy_last_event(&event)) {
                dm2_runtime_apply_trigger_event(rt, &event);
            }
        }
        dm2_v1_plate_set_party_position(
            entered_stairs ? stair_x : (entered_pit ? pit_x : nx),
            entered_stairs ? stair_y : (entered_pit ? pit_y : ny),
            rt->dungeon_level);
        for (int i = 1; i <= dm2_v1_plate_get_builtin_count(); ++i) {
            DM2_V1_PlateEvent event;
            const DM2_V1_PressurePlate *plate =
                dm2_v1_plate_get_builtin(i);
            if (plate && plate->map_x == (entered_stairs ? stair_x :
                                          (entered_pit ? pit_x : nx)) &&
                plate->map_y == (entered_stairs ? stair_y :
                                 (entered_pit ? pit_y : ny)) &&
                plate->map_level == rt->dungeon_level &&
                dm2_v1_plate_check(i, rt->tick_count) ==
                    (int)DM2_PLATE_RESULT_OK &&
                dm2_v1_plate_copy_last_event(&event)) {
                dm2_runtime_apply_plate_event(rt, &event);
            }
        }
        (void)dm2_v1_runtime_invoke_square_actuators(
            rt->dungeon_level,
            entered_stairs ? stair_x : (entered_pit ? pit_x : nx),
            entered_stairs ? stair_y : (entered_pit ? pit_y : ny));
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

    if (!dm2_runtime_gameplay_source_ready(rt) || !rt->boot->dm2_state) {
        return -1;
    }
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
           left->words_big_endian == right->words_big_endian &&
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
    if (!dm2_v1_original_save_state_corpus_probe_ordered(
            save_root, selected_entry->candidate.words_big_endian,
            &state_corpus)) {
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
    if (g_dm2_runtime.source_party_valid &&
        g_dm2_runtime.session_snapshot_valid) {
        return g_dm2_runtime.leader_hand_object;
    }
    /* Keep the bounded-session hand visible to the existing session API.  A
     * runtime with no admitted source session still exposes no host cache. */
    if (g_dm2_runtime.session_snapshot_valid)
        return g_dm2_runtime.leader_hand_object;
    return 0u;
}

int dm2_v1_runtime_get_session_snapshot(DM2_V1_SessionState *out_session)
{
    if (!out_session || !g_dm2_runtime.source_party_valid ||
        !g_dm2_runtime.session_snapshot_valid) return 0;
    *out_session = g_dm2_runtime.session_snapshot;
    return 1;
}

int dm2_v1_runtime_set_leader_hand_object(uint32_t object) {
    if (!g_dm2_runtime.session_snapshot_valid) return -1;
    if (g_dm2_runtime.source_party_valid) {
        if (object > 0xffffu ||
            (object != 0u && object != 0xffffu &&
             !dm2_v1_record_pool_address(
                 &g_dm2_runtime.record_pools, (int16_t)object))) {
            return -1;
        }
    }
    g_dm2_runtime.leader_hand_object = object;
    if (g_dm2_runtime.source_party_valid &&
        g_dm2_runtime.source_party.curacthero > 0 &&
        g_dm2_runtime.source_party.curacthero <=
            g_dm2_runtime.source_party.heros_in_party &&
        g_dm2_runtime.source_party.curactmode >= 0 &&
        g_dm2_runtime.source_party.curactmode < 2) {
        DM2_V1_Hero *hero = &g_dm2_runtime.source_party.hero[
            g_dm2_runtime.source_party.curacthero - 1];
        hero->item[g_dm2_runtime.source_party.curactmode] =
            object == 0u || object == 0xffffu
                ? (int16_t)-1 : (int16_t)(uint16_t)object;
        dm2_runtime_copy_source_hero_to_snapshot(
            &g_dm2_runtime.session_snapshot,
            g_dm2_runtime.source_party.curacthero - 1, hero);
    }
    return 0;
}

int dm2_v1_runtime_is_sleeping(void) {
    return g_dm2_runtime.source_sleeping != 0;
}

void dm2_v1_runtime_set_sleeping(int sleeping) {
    /* UI event 142/143 owns this one-bit source global. No timer or party
     * state is invented here; the renderer only consumes the bit for the
     * source gray overlay and the next source tick remains authoritative. */
    g_dm2_runtime.source_sleeping = sleeping != 0;
}

int dm2_v1_runtime_cycle_action_champion(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    int count;
    int current;

    if (!rt->session_snapshot_valid ||
        !rt->session_snapshot.original_champion_records_valid) {
        return 0;
    }
    count = rt->session_snapshot.champion_count;
    if (count <= 0 || count > DM2_V1_CHAMPION_STAT_BRIDGE_MAX_HEROES) {
        return 0;
    }

    /* DISPLAY_RIGHT_PANEL_SQUAD_HANDS consumes party.curacthero. The M11
     * Tab bridge may advance that source-owned selection, but it must never
     * invent a hero when the saved formation is incomplete. */
    current = rt->source_curacthero > 0 && rt->source_curacthero <= count
        ? rt->source_curacthero - 1
        : -1;
    for (int offset = 1; offset <= count; ++offset) {
        int candidate = (current + offset + count) % count;
        const DM2_ChampionRecord *champ =
            (const DM2_ChampionRecord *)
                rt->session_snapshot.champion_data[candidate];
        int occupied = champ->first_name[0] != '\0' ||
            champ->cur_hp != 0u || champ->max_hp != 0u;
        if (occupied) {
            if (rt->source_curacthero == candidate + 1) {
                return 0;
            }
            rt->source_curacthero = (int16_t)(candidate + 1);
            if (rt->source_party_valid)
                rt->source_party.curacthero = rt->source_curacthero;
            return 1;
        }
    }
    return 0;
}

int dm2_v1_runtime_activate_action_hand(int hero_index, int hand) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_Hero *hero;

    if (hero_index < 0 || hero_index >= DM2_MAX_HEROES ||
        hand < 0 || hand > 1 || !rt->source_party_valid ||
        !rt->session_snapshot_valid || !rt->record_pools_valid ||
        hero_index >= rt->source_party.heros_in_party) {
        return 0;
    }
    hero = &rt->source_party.hero[hero_index];
    if (hero->curHP <= 0) return 0;
    rt->source_curacthero = (int16_t)(hero_index + 1);
    rt->source_curactmode = (int16_t)hand;
    rt->source_party.curacthero = rt->source_curacthero;
    rt->source_party.curactmode = rt->source_curactmode;
    return 1;
}

int dm2_v1_runtime_get_active_champion_index(void) {
    const DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->source_party_valid || !rt->session_snapshot_valid ||
        rt->source_party.curacthero <= 0 ||
        rt->source_party.curacthero > rt->source_party.heros_in_party ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES) {
        return -1;
    }
    return rt->source_party.curacthero - 1;
}

int dm2_v1_runtime_get_active_hand(void)
{
    const DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->source_party_valid || rt->source_party.curactmode < 0 ||
        rt->source_party.curactmode > 1) return -1;
    return rt->source_party.curactmode;
}

int dm2_v1_runtime_get_champion_count(void) {
    const DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->source_party_valid || !rt->session_snapshot_valid ||
        rt->source_party.heros_in_party < 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES) {
        return 0;
    }
    return rt->source_party.heros_in_party;
}

int dm2_v1_runtime_click_inventory_eye(void) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    int hero;

    if (!rt->source_party_valid || !rt->session_snapshot_valid ||
        rt->source_party.heros_in_party <= 0 ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES ||
        rt->source_event_hero_index < 0 ||
        rt->source_event_hero_index >= rt->source_party.heros_in_party) {
        return 0;
    }
    /* The authenticated GAME_LOAD candidate carries the source event
     * queue's event_heroidx; use that exact owner just as c_events.cpp does. */
    hero = rt->source_event_hero_index;
    if (hero < 0 || hero >= rt->source_party.heros_in_party ||
        rt->source_party.hero[hero].curHP <= 0 ||
        rt->source_party.hero[hero].heroflag == 0) {
        return 0;
    }
    rt->source_v1e0976 = (int16_t)(hero + 1);
    return 1;
}

int dm2_v1_runtime_get_inventory_eye_champion_index(void) {
    const DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    if (!rt->source_party_valid || !rt->session_snapshot_valid ||
        rt->source_v1e0976 <= 0 ||
        rt->source_v1e0976 > rt->source_party.heros_in_party ||
        rt->source_party.heros_in_party > DM2_MAX_HEROES) {
        return -1;
    }
    return rt->source_v1e0976 - 1;
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
    g_dm2_runtime.source_curacthero = 0;
    g_dm2_runtime.source_curactmode = 0;
    g_dm2_runtime.source_event_hero_index = 0;
    g_dm2_runtime.source_v1e0976 = 0;
    g_dm2_runtime.source_sleeping = 0;
    memset(&g_dm2_runtime.source_party, 0,
           sizeof(g_dm2_runtime.source_party));
    g_dm2_runtime.source_party_valid = 0;
    g_dm2_last_wield_death_drop_count = 0;
    g_dm2_last_wield_death_drop_iterations = 0;
    g_dm2_last_wield_death_drop_alloc_failures = 0;
    g_dm2_last_wield_death_drop_first_itemspec = 0;
    g_dm2_last_wield_death_drop_first_db = -1;
    g_dm2_last_wield_death_drop_alloc_free_records = 0;
    g_dm2_last_wield_death_deallocated = 0;
    g_dm2_runtime.source_attack_counter = 0u;
    g_dm2_runtime.source_hero_ench_countdown = 0u;
    memset(g_dm2_runtime.source_savegames1, 0,
           sizeof(g_dm2_runtime.source_savegames1));
    g_dm2_runtime.source_aura_of_speed = 0u;
    g_dm2_runtime.source_aura_of_speed_valid = 0;

    /* skproject c_savegame.cpp::DM2_GAME_LOAD reaches the new-dungeon path
     * with a fresh timer clock. A previous session must not leak timers,
     * weather events, or its tick into the source-owned G1 entrance. The
     * actuator tick-generator walk remains unavailable until its record
     * owner is bound, so this reset establishes the correct empty base. */
    dm2_v1_source_timer_queue_init(&g_dm2_runtime.timer_queue);
    g_dm2_runtime.tick_count = 0;
    g_dm2_runtime.move_cooldown_ticks = 0;
    g_dm2_runtime.weather_source_timer_pending = 0;
    g_dm2_runtime.weather_chain_started = 0;
    dm2_v1_weather_init(&g_dm2_runtime.weather);
    memset(&g_dm2_runtime.weather_chain, 0,
           sizeof(g_dm2_runtime.weather_chain));
    memset(&g_dm2_runtime.weather_rng, 0,
           sizeof(g_dm2_runtime.weather_rng));
    memset(&g_dm2_runtime.timer_post_load, 0,
           sizeof(g_dm2_runtime.timer_post_load));
    memset(&g_dm2_runtime.proceed_timers, 0,
           sizeof(g_dm2_runtime.proceed_timers));
}

int dm2_v1_runtime_new_game_party_state_is_clear(void) {
    size_t champion;
    size_t slot;

    /* SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON clears the old
     * party before it reads the selected G1 structure.  This is a narrow
     * postcondition for Firestaff's cached projection of that state: it does
     * not claim that the unported original hero/record owner is complete. */
    if (g_dm2_runtime.leader_hand_object != 0u ||
        g_dm2_runtime.session_snapshot_valid) {
        return 0;
    }
    for (champion = 0;
         champion < sizeof(g_dm2_runtime.champion_inventory_objects) /
                        sizeof(g_dm2_runtime.champion_inventory_objects[0]);
         ++champion) {
        for (slot = 0;
             slot < sizeof(g_dm2_runtime.champion_inventory_objects[champion]) /
                        sizeof(g_dm2_runtime.champion_inventory_objects[champion][0]);
             ++slot) {
            if (g_dm2_runtime.champion_inventory_objects[champion][slot] != 0u) {
                return 0;
            }
        }
    }
    return 1;
}

uint32_t dm2_v1_runtime_get_champion_inventory_object(uint8_t champion,
                                                      uint8_t slot) {
    if (champion >= 4u || slot >= 30u) {
        return 0u;
    }
    if (g_dm2_runtime.source_party_valid &&
        g_dm2_runtime.session_snapshot_valid) {
        int16_t item = g_dm2_runtime.source_party.hero[champion].item[slot];
        return item < 0 ? 0u : (uint32_t)(uint16_t)item;
    }
    if (g_dm2_runtime.session_snapshot_valid) {
        return ((const DM2_ChampionRecord *)
                    g_dm2_runtime.session_snapshot.champion_data[champion])
            ->inventory[slot];
    }
    return 0u;
}

int dm2_v1_runtime_get_source_hero_timer_state(uint8_t champion,
                                               int16_t *out_timeridx,
                                               uint16_t *out_heroflag,
                                               int16_t *out_cur_hp) {
    const DM2_V1_Hero *hero;

    if (champion >= DM2_MAX_HEROES || !g_dm2_runtime.source_party_valid ||
        champion >= (uint8_t)g_dm2_runtime.source_party.heros_in_party)
        return 0;
    hero = &g_dm2_runtime.source_party.hero[champion];
    if (out_timeridx) *out_timeridx = hero->timeridx;
    if (out_heroflag) *out_heroflag = (uint16_t)hero->heroflag;
    if (out_cur_hp) *out_cur_hp = hero->curHP;
    return 1;
}

int dm2_v1_runtime_get_source_hero_state(
    uint8_t champion, DM2_V1_RuntimeSourceHeroStateReceipt *out_state) {
    const DM2_V1_Hero *hero;

    if (!out_state || champion >= DM2_MAX_HEROES ||
        !g_dm2_runtime.source_party_valid ||
        champion >= (uint8_t)g_dm2_runtime.source_party.heros_in_party)
        return 0;
    hero = &g_dm2_runtime.source_party.hero[champion];
    out_state->max_hp = hero->maxHP;
    out_state->cur_hp = hero->curHP;
    out_state->cur_mp = hero->curMP;
    out_state->wizardry_skill = hero->ability[DM2_ABILITY_WIZARDRY][0];
    out_state->rune_count = hero->nrunes;
    out_state->weight = hero->weight;
    out_state->ench_power = hero->ench_power;
    out_state->ench_aura = hero->ench_aura;
    out_state->first_item = hero->item[0];
    out_state->heroflag = (uint16_t)hero->heroflag;
    return 1;
}

int dm2_v1_runtime_get_source_aura_of_speed(uint8_t *out_value) {
    if (!out_value || !g_dm2_runtime.source_aura_of_speed_valid)
        return 0;
    *out_value = g_dm2_runtime.source_aura_of_speed;
    return 1;
}

int dm2_v1_runtime_set_champion_inventory_object(uint8_t champion,
                                                 uint8_t slot,
                                                 uint32_t object) {
    if (champion >= 4u || slot >= 30u) {
        return -1;
    }
    if (!g_dm2_runtime.session_snapshot_valid) {
        return -1;
    }
    if (g_dm2_runtime.source_party_valid) {
        if (object > 0xffffu ||
            (object != 0u && object != 0xffffu &&
             !dm2_v1_record_pool_address(
                 &g_dm2_runtime.record_pools, (int16_t)object))) {
            return -1;
        }
        g_dm2_runtime.source_party.hero[champion].item[slot] =
            object == 0u || object == 0xffffu
                ? (int16_t)-1 : (int16_t)(uint16_t)object;
        g_dm2_runtime.champion_inventory_objects[champion][slot] = object;
        dm2_runtime_copy_source_hero_to_snapshot(
            &g_dm2_runtime.session_snapshot, champion,
            &g_dm2_runtime.source_party.hero[champion]);
        return 0;
    }
    ((DM2_ChampionRecord *)
         g_dm2_runtime.session_snapshot.champion_data[champion])
        ->inventory[slot] = object;
    g_dm2_runtime.champion_inventory_objects[champion][slot] = object;
    return 0;
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

int dm2_v1_runtime_restore_save_candidate(const uint8_t *data,
                                          size_t data_size)
{
    /* SKProject GAME_LOAD continues after the bounded c_hex2a/SUPPRESS
     * sections with the linked c_record, possession, c_hero, actuator and
     * timer graph. Firestaff can currently receipt the raw prefix but cannot
     * publish that partial graph as a session. Keep every public resume route
     * unavailable rather than turning authentic-but-incomplete bytes into a
     * playable state. SKProject source: SKWINSPX/src/v4/skcore.cpp::GAME_LOAD
     * and SKWINSPX/src/v5/sksvgame.cpp. */
    (void)data;
    (void)data_size;
    return -1;
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
    DM2_V1_QuicksaveResult result)
{
    if (!receipt) return;
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = result;
    receipt->status_scope = "SAVE";
}

int dm2_v1_runtime_quicksave_boot_profile_with_receipt(
    DM2_V1_BootProfile *profile,
    DM2_V1_QuicksaveReceipt *out_receipt)
{
    DM2_V1_QuicksaveReceipt local;
    DM2_V1_QuicksaveReceipt *receipt = out_receipt ? out_receipt : &local;

    dm2_v1_quicksave_receipt_init(receipt,
                                  DM2_V1_QUICKSAVE_PROFILE_MISSING);
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
        receipt, DM2_V1_QUICKSAVE_ORIGINAL_WRITER_REQUIRED);
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
    /* ReDMCSB BASE.C F0027/F0029 advances the environment seed only from
     * the source-owned weather/timer state.  This public compatibility
     * symbol used to accept a host number even though no live GAME_LOAD,
     * SKSAVE or ENVIRONMENT handoff supplied it.  Do not retain an invented
     * seed in the production runtime. */
    (void)seed;
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
    DM2_V1_DungeonData *dungeon;
    DM2_V1_SourceTimer timer;
    DM2_V1_SourceTimerResult result;
    int raw;

    if (!rt->boot || !rt->boot->source_game_load_session_ready ||
        !rt->boot->dungeon_data || !rt->record_pools_valid ||
        !rt->record_pools.record_graph_complete || level != rt->dungeon_level)
        return -1;
    dungeon = (DM2_V1_DungeonData *)rt->boot->dungeon_data;
    raw = dm2_v1_dungeon_get_tile_raw(dungeon, level, x, y);
    if (raw < 0 || ((unsigned)raw >> 5) != 4u ||
        dm2_v1_dungeon_get_first_thing(dungeon, level, x, y) < 0)
        return -1;
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)(level & 0xff) << 24) |
        ((dm2_v1_runtime_get_tick_count() + 1u) &
         DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = DM2_V1_TIMER_ACTUATE_TILE;
    timer.value_a = (int16_t)((uint8_t)x | ((uint16_t)(uint8_t)y << 8));
    timer.value_b = (int16_t)((uint8_t)(facing_dir & 3) |
                              ((uint16_t)(action == 0 ? 2 : action) << 8));
    result = dm2_v1_runtime_enqueue_source_timer(&timer, 0u);
    return result == DM2_V1_SOURCE_TIMER_OK ? 0 : -1;
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
    return -1;
}

int dm2_v1_runtime_sell_to_shop(int inv_idx) {
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;

    (void)inv_idx;
    if (!rt->boot || !rt->boot->dm2_state) return -1;
    return -1;
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
    return -1;
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

const uint8_t *dm2_v1_runtime_query_gdat_text_override(
    void *context, int32_t category, int32_t index, int32_t field,
    size_t *out_size)
{
    (void)context;
    if (out_size) *out_size = 0u;
    if (category < 0 || category > 0xff || index < 0 || index > 0xff ||
        field < 0 || field > 0xff) {
        return NULL;
    }
    return dm2_v1_runtime_i18n_text((int)category, (int)index, (int)field,
                                    out_size);
}

int dm2_v1_runtime_bind_fmtowns_english_text_companion(
    const uint8_t *graphics_data, size_t graphics_size)
{
    DM2_V1_AssetLoader probe;
    if (!g_dm2_runtime.boot ||
        g_dm2_runtime.boot->platform != DM2_PLATFORM_FMTOWNS_JA ||
        !graphics_data || graphics_size == 0u ||
        dm2_v1_asset_loader_init(&probe, graphics_data, graphics_size) != 0) {
        return 0;
    }
    /* PC English is GDAT v5 (0x8005), while the selected Towns CD stays on
     * v4.  Reject any other source before it can become a language owner. */
    if (probe.data_size < 2u ||
        ((uint16_t)probe.data[0] | ((uint16_t)probe.data[1] << 8)) != 0x8005u) {
        dm2_v1_asset_loader_free(&probe);
        return 0;
    }
    dm2_v1_asset_loader_free(&probe);
    if (g_dm2_runtime.i18n_ready) {
        dm2_v1_i18n_destroy(&g_dm2_runtime.i18n);
        g_dm2_runtime.i18n_ready = 0;
    }
    dm2_v1_i18n_init(&g_dm2_runtime.i18n);
    if (!dm2_v1_i18n_load_english_overlay(&g_dm2_runtime.i18n,
                                           graphics_data, graphics_size)) {
        dm2_v1_i18n_destroy(&g_dm2_runtime.i18n);
        return 0;
    }
    dm2_v1_i18n_set_locale(&g_dm2_runtime.i18n, DM2_LOCALE_EN);
    g_dm2_runtime.i18n_ready = 1;
    return 1;
}

/* ── Engage command (hand actions) ────────────────────────────────── */

typedef struct {
    DM2_V1_RuntimeState *runtime;
    int hero_index;
    int queued;
} DM2_RuntimeAttackQueueContext;

static int dm2_runtime_queue_attack_timer(
    void *context, uint8_t type, int16_t delay,
    uint32_t game_tick, int16_t map)
{
    DM2_RuntimeAttackQueueContext *queue =
        (DM2_RuntimeAttackQueueContext *)context;
    DM2_V1_SourceTimer timer;
    DM2_V1_SourceTimerResult result;

    if (!queue || !queue->runtime || type != 0x47u || delay < 0 ||
        map < 0 || map > 0xff) {
        return 0;
    }
    memset(&timer, 0, sizeof(timer));
    timer.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
        ((game_tick + (uint32_t)delay) & DM2_V1_SOURCE_TIMER_TICK_MASK);
    timer.type = type;
    result = dm2_v1_source_timer_enqueue(
        &queue->runtime->timer_queue, &timer, 0u);
    if (result != DM2_V1_SOURCE_TIMER_OK) return 0;

    /* skengage.cpp increments savegames1.b_02 before DM2_QUEUE_TIMER and
     * sets the selected hero's 0x4000 flag only on the zero->one edge. */
    if (queue->runtime->source_attack_counter == 0u &&
        queue->runtime->source_curacthero > 0 &&
        queue->runtime->source_curacthero <= DM2_MAX_HEROES) {
        queue->runtime->source_party.hero[
            queue->runtime->source_curacthero - 1].heroflag |= 0x4000;
    }
    queue->runtime->source_attack_counter++;
    queue->runtime->source_hero_ench_countdown =
        queue->runtime->source_attack_counter;
    queue->runtime->source_savegames1[2] =
        queue->runtime->source_attack_counter;
    queue->queued = 1;
    (void)queue->hero_index;
    return 1;
}

static int dm2_runtime_query_cmd_field(
    const DM2_V1_AssetLoader *loader, uint8_t cls1, uint8_t cls2,
    int entry, int field, int16_t *out)
{
    DM2_V1_CmdstrEntryReceipt value;
    if (!out) return 0;
    memset(&value, 0, sizeof(value));
    if (!dm2_v1_query_cmdstr_entry_receipt(
            loader, cls1, cls2, entry, field, &value) || !value.found ||
        value.value < INT16_MIN || value.value > INT16_MAX) {
        return 0;
    }
    *out = (int16_t)value.value;
    return 1;
}

static uint8_t dm2_runtime_attack_cls1(void *ctx, uint16_t record);
static uint8_t dm2_runtime_attack_cls2(void *ctx, uint16_t record);
static int16_t dm2_runtime_attack_dbspec_index(
    void *ctx, uint8_t cls1, uint8_t cls2, uint8_t entry_type,
    uint8_t field);

static int dm2_runtime_projectile_query_word(
    int record_link, int word_index, void *userdata)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)userdata;
    DM2_V1_GdatDbspecCallbacks callbacks;
    if (!rt || record_link < 0 || record_link > 0xffff ||
        word_index < 0 || word_index > 0xff) return 0;
    callbacks.query_cls1_from_record = dm2_runtime_attack_cls1;
    callbacks.query_cls2_from_record = dm2_runtime_attack_cls2;
    callbacks.query_gdat_entry_data_index = dm2_runtime_attack_dbspec_index;
    return dm2_v1_query_gdat_dbspec_word_value(
        (uint16_t)record_link, (uint8_t)word_index, &callbacks, rt);
}

static int dm2_runtime_projectile_query_weight(
    int record_link, void *userdata)
{
    return dm2_runtime_projectile_query_word(record_link, 1, userdata);
}

static int dm2_runtime_projectile_rand_mask(int mask, void *userdata)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)userdata;
    if (!rt || mask < 0) return 0;
    if (mask >= 0x7fff) mask = 0x7fff;
    return (int)dm2_v1_drops_rand16(&rt->drop_rng, (uint16_t)mask + 1u);
}

static uint8_t dm2_runtime_attack_cls1(void *ctx, uint16_t record)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    uint8_t cls1 = 0xffu;
    DM2_V1_SkprojectQueryCls1Receipt receipt;
    return rt && dm2_v1_skproject_query_cls1_from_record_ex(
        record, &rt->record_pools, &cls1, &receipt) ? cls1 : 0xffu;
}

static uint8_t dm2_runtime_attack_cls2(void *ctx, uint16_t record)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    uint8_t cls2 = 0xffu;
    DM2_V1_SkprojectQueryCls2Receipt receipt;
    return rt && dm2_v1_skproject_query_cls2_from_record(
        record, &rt->record_pools, &cls2, &receipt) ? cls2 : 0xffu;
}

static int16_t dm2_runtime_attack_dbspec_index(
    void *ctx, uint8_t cls1, uint8_t cls2, uint8_t entry_type,
    uint8_t field)
{
    DM2_V1_RuntimeState *rt = (DM2_V1_RuntimeState *)ctx;
    const DM2_V1_AssetLoader *loader;
    uint16_t value = 0;
    if (!rt || !rt->boot || entry_type != DM2_GDAT_ENTRY_TYPE_WORD_VALUE)
        return -1;
    loader = dm2_v1_boot_asset_loader(rt->boot);
    return loader && dm2_v1_query_gdat_entry_data_index(
        loader, cls1, cls2, entry_type, field, &value) ? (int16_t)value : -1;
}

/* skengage.cpp/c_hero.cpp: the party attacks the creature occupying the
 * facing square.  This is deliberately called before MOVE_RECORD_TO, so a
 * failed or unverifiable attack can never put the party on a DB4 cell. */
static int dm2_runtime_attack_creature_at(
    DM2_V1_RuntimeState *rt, DM2_V1_DungeonData *dungeon,
    int map, int x, int y, int target_x, int target_y,
    int16_t creature_record, int allow_wield_action)
{
    DM2_V1_GameState *game;
    DM2_V1_Hero *hero;
    const DM2_V1_AssetLoader *loader;
    const DM2_AIDefinition *ai = NULL;
    DM2_V1_GdatDbspecCallbacks dbspec;
    DM2_V1_CalcAttackDamageRequest request;
    DM2_V1_CaiiAttackReceipt attack;
    uint8_t *record;
    uint16_t item;
    int hand;
    int slot;
    int16_t field;
    uint16_t dbspec_word[10] = {0};
    int skill;
    int have_wield_power;
    int have_wield_flags;
    int have_wield_variant;

    memset(&g_dm2_last_wield_attack, 0, sizeof(g_dm2_last_wield_attack));

    if (!rt || !dungeon || !rt->source_party_valid ||
        !rt->record_pools_valid || !rt->boot || !rt->boot->dm2_state ||
        !rt->caii_ready || !rt->caii.valid ||
        map != rt->dungeon_level || !rt->source_party.curacthero ||
        rt->source_party.curacthero > rt->source_party.heros_in_party)
        return 0;
    game = (DM2_V1_GameState *)rt->boot->dm2_state;
    if (game->current_level != map) return 0;
    hand = rt->source_party.curactmode;
    if (hand < 0 || hand > 1) return 0;
    hero = &rt->source_party.hero[rt->source_party.curacthero - 1];
    if (hero->curHP <= 0) return 0;
    item = (uint16_t)hero->item[hand];
    if (item == 0xffffu) return 0;
    g_dm2_last_wield_attack.valid = 1;
    g_dm2_last_wield_attack.command_admitted = 1;
    g_dm2_last_wield_attack.item_handle = (int16_t)item;
    g_dm2_last_wield_attack.creature_record = creature_record;
    record = dm2_v1_record_pool_address_mut(&rt->record_pools,
                                             creature_record);
    if (!record || dm2_v1_record_handle_pool(creature_record) != 4)
        return 0;
    g_dm2_last_wield_attack.creature_resolved = 1;
    if (!dm2_v1_creature_ai_spec_def((int)record[4], &ai) || !ai)
        return 0;
    loader = dm2_v1_boot_asset_loader(rt->boot);
    if (!loader) return 0;
    slot = hero->handcmd[hand];
    if (slot < 0 || slot > 2) return 0;
    memset(&request, 0, sizeof(request));
    request.hero_index = rt->source_party.curacthero - 1;
    request.hero_hp = hero->curHP;
    request.hero_dexterity = hero->ability[DM2_ABILITY_DEXTERITY][DM2_CUR];
    request.hero_strength = hero->ability[DM2_ABILITY_STRENGTH][DM2_CUR];
    g_dm2_last_wield_attack.hero_dexterity = request.hero_dexterity;
    g_dm2_last_wield_attack.hero_strength = request.hero_strength;
    /* c_hero.cpp passes the adjusted strength ability separately to
     * COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH; leaving it zero makes every
     * otherwise valid weapon attack collapse to zero damage. */
    request.hero_ability = (uint8_t)request.hero_strength;
    request.hero_luck = hero->ability[DM2_ABILITY_LUCK][DM2_CUR];
    request.hand = (int16_t)hand;
    request.item_handle = (int16_t)item;
    request.creature_record = creature_record;
    request.creature_defense = ai->Defense;
    request.creature_armor = ai->ArmorClass;
    g_dm2_last_wield_attack.creature_defense = request.creature_defense;
    g_dm2_last_wield_attack.creature_armor = request.creature_armor;
    request.creature_poison_resist = ai->w24;
    request.target_x = (int16_t)target_x;
    request.target_y = (int16_t)target_y;
    memset(&dbspec, 0, sizeof(dbspec));
    dbspec.query_cls1_from_record = dm2_runtime_attack_cls1;
    dbspec.query_cls2_from_record = dm2_runtime_attack_cls2;
    dbspec.query_gdat_entry_data_index = dm2_runtime_attack_dbspec_index;
    /* The item command is the source CMDSTR entry selected by the hand. */
    if (!dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 2, &field) ||
        (field != 1 && (!allow_wield_action || field != 8)) ||
        !dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 0, &request.skill_type) ||
        !dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 3, &request.power_base) ||
        !dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 4, &request.power_random) ||
        (!allow_wield_action && !dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 0x10, &request.damage_type)))
        return 0;
    if (allow_wield_action &&
        !dm2_runtime_query_cmd_field(loader,
                                     dm2_runtime_attack_cls1(rt, item),
                                     dm2_runtime_attack_cls2(rt, item),
                                     8 + slot, 0x10, &request.damage_type))
        request.damage_type = 0;
    /* c_hero.cpp:2064-2158 DM2_WIELD_WEAPON does not pass CMDSTR fields
     * 3/4 to CALC_PLAYER_ATTACK_DAMAGE. It passes field 11 as argw2 and
     * derives argl1 from fields 10/15; field 3 belongs to the generic
     * command dispatcher (healing/other actions). */
    have_wield_power = dm2_runtime_query_cmd_field(
        loader, dm2_runtime_attack_cls1(rt, item),
        dm2_runtime_attack_cls2(rt, item), 8 + slot, 11,
        &request.power_base);
    have_wield_flags = dm2_runtime_query_cmd_field(
        loader, dm2_runtime_attack_cls1(rt, item),
        dm2_runtime_attack_cls2(rt, item), 8 + slot, 10,
        &request.power_random);
    have_wield_variant = dm2_runtime_query_cmd_field(
        loader, dm2_runtime_attack_cls1(rt, item),
        dm2_runtime_attack_cls2(rt, item), 8 + slot, 15, &field);
    if (!have_wield_variant)
        field = 0;
    if (!have_wield_power || !have_wield_flags) {
        return 0;
    }
    /* CMDSTR stores the full twenty-entry skill id.  The live hero record
     * stores the sixteen learned entries in groups 1..4; the aggregate row
     * is not the level source for attack-strength calculation. */
    if (request.skill_type < 0 || request.skill_type >= 20) return 0;
    skill = request.skill_type / 4 + 1;
    /* COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH consumes only the source
     * weapon words 1, 5, 8 and 9.  Other DB5 weapon words are optional
     * payloads for unrelated actions and must not reject a valid WIELD. */
    for (int i = 0; i < 4; ++i) {
        static const int required_words[4] = { 1, 5, 8, 9 };
        int word = required_words[i];
        int16_t value = dm2_v1_query_gdat_dbspec_word_value(
            item, (uint8_t)word, &dbspec, rt);
        if (value < 0) {
            if (word == 1) return 0;
            value = 0;
        }
        dbspec_word[word] = (uint16_t)value;
    }
    /* c_hero.cpp indexes the sixteen learned skills as group 1..4, then
     * stores each level as 0x40 << level.  CMDSTR field 0 is that flat
     * learned-skill index; the aggregate skill[0] row is only for UI and
     * must not be passed as a level. */
    {
        int32_t encoded = hero->skill[skill][request.skill_type % 4];
        int level = 0;
        while (encoded >= 0x40 && level < 31) {
            encoded >>= 1;
            ++level;
        }
        request.hero_skill_level = (int16_t)level;
    }
    g_dm2_last_wield_attack.hero_skill_level = request.hero_skill_level;
    request.bodyflag = (uint8_t)hero->bodyflag;
    request.item_weight = dbspec_word[1];
    request.dbspec_word5 = dbspec_word[5];
    request.dbspec_word8 = dbspec_word[8];
    request.dbspec_word9 = dbspec_word[9];
    request.hero_max_load = (uint16_t)dm2_v1_hero_get_max_load_raw(hero, 0);
    /* c_querydb.cpp:2237-2378 first builds RG3 (ability + load + skill and
     * DBSPEC bonuses), then passes that value to c_hero::get_stamina_adj.
     * Passing the raw ability here silently discarded every weapon/skill
     * contribution and made authentic WIELD damage collapse to zero. */
    {
        int32_t pre_strength = (int32_t)request.hero_ability +
                               (int32_t)request.item_weight - 12;
        uint16_t quarter_load = (uint16_t)(request.hero_max_load >> 4);
        if (request.item_weight > quarter_load) {
            uint16_t excess = (uint16_t)(request.item_weight - quarter_load);
            uint16_t threshold = (uint16_t)(((quarter_load - 12u) / 2u) +
                                            quarter_load);
            pre_strength -= (int32_t)(excess / 2u);
            if (request.item_weight > threshold)
                pre_strength -= 2 * (int32_t)(request.item_weight - threshold);
        }
        if (request.skill_type >= 0) {
            pre_strength += 2 * (int32_t)request.hero_skill_level;
            if (request.skill_type == 0 ||
                (request.skill_type >= 4 && request.skill_type <= 7) ||
                request.skill_type == 9)
                pre_strength += (int32_t)request.dbspec_word8;
            else if (request.skill_type == 1 ||
                     (request.skill_type >= 10 && request.skill_type <= 11)) {
                uint16_t bonus = request.dbspec_word9;
                if (bonus != 0) {
                    int word5_8000 = (request.dbspec_word5 & 0x8000u) != 0;
                    if ((!word5_8000 && request.skill_type == 11) ||
                        (word5_8000 && request.skill_type != 11))
                        bonus = 0;
                }
                pre_strength += (int32_t)bonus;
            }
        }
        if (pre_strength > INT16_MAX) pre_strength = INT16_MAX;
        if (pre_strength < INT16_MIN) pre_strength = INT16_MIN;
        request.stamina_adj = dm2_v1_hero_get_stamina_adj_raw(
            hero, (int16_t)pre_strength);
    }
    request.creature_armor_mult = ai->ArmorClass;
    request.party_level = (int16_t)map;
    request.rand_hit = dm2_v1_drops_rand16(&rt->drop_rng, 0x100u);
    request.rand_defense = dm2_v1_drops_rand16(&rt->drop_rng, 0x100u);
    request.rand_armor = dm2_v1_drops_rand16(&rt->drop_rng, 0x100u);
    request.rand_poison = dm2_v1_drops_rand16(&rt->drop_rng, 0x100u);
    g_dm2_last_wield_attack.command_power = request.power_base;
    memset(&attack, 0, sizeof(attack));
    /* The compatibility calculation receipt accepts caller-authored combat
     * words and is deliberately not a production damage owner.  The
     * authenticated ATTACK_CREATURE transaction below owns both calculation
     * and mutation from the selected retail runtime state. */
    if (!dm2_v1_combat_attack_creature_source(
        &request, &rt->record_pools, dungeon, &rt->caii, &rt->timer_queue,
        &rt->drop_rng, map, (unsigned long)rt->tick_count, x, y,
        target_x, target_y, &attack) && !attack.hp_applied)
        return 0;
    /* A valid source miss does not enter ATTACK_CREATURE and therefore does
     * not apply HP.  It must remain a blocked collision: callers use this
     * result to prevent the party from treating a failed WIELD as a hit. */
    if (!attack.hp_applied)
        return 0;
    g_dm2_last_wield_attack.hp_applied = 1;

    /* Keep an accumulator receipt even when the due creature timer has not
     * run yet.  This distinguishes a real source hit from a WIELD command
     * that merely reached the facing-cell collision owner. */
    memset(&g_dm2_last_creature_damage, 0,
           sizeof(g_dm2_last_creature_damage));
    g_dm2_last_creature_damage.valid = 1;
    g_dm2_last_creature_damage.creature_record = creature_record;
    g_dm2_last_creature_damage.creature_type = (int)record[4];
    g_dm2_last_creature_damage.pending_damage = attack.hp_word_after;

    /* ATTACK_CREATURE only accumulates damage in CAII word@0x14.  The
     * source think timer owns HP transfer, WOUND_CREATURE and death/drop. */
    return 1;
}

typedef struct {
    DM2_V1_RuntimeState *runtime;
    DM2_V1_DungeonData *dungeon;
    int map;
    int target_x;
    int target_y;
    int16_t creature;
    int attack_succeeded;
} DM2_RuntimeWieldContext;

static int dm2_runtime_wield_get_creature(void *ctx, int16_t x, int16_t y)
{
    DM2_RuntimeWieldContext *w = (DM2_RuntimeWieldContext *)ctx;
    if (!w || !w->runtime || !w->dungeon || x != w->target_x || y != w->target_y)
        return -1;
    return w->creature;
}

static int16_t dm2_runtime_wield_calc_damage(
    void *ctx, int hero_idx, int creature_idx,
    int16_t action_strength, int32_t skill_id)
{
    DM2_RuntimeWieldContext *w = (DM2_RuntimeWieldContext *)ctx;
    (void)hero_idx;
    (void)creature_idx;
    (void)action_strength;
    (void)skill_id;
    if (!w || !w->runtime || !w->dungeon) return 0;
    w->attack_succeeded = dm2_runtime_attack_creature_at(
        w->runtime, w->dungeon, w->map,
        w->runtime->boot->dm2_state ?
            ((DM2_V1_GameState *)w->runtime->boot->dm2_state)->party_x : 0,
        w->runtime->boot->dm2_state ?
            ((DM2_V1_GameState *)w->runtime->boot->dm2_state)->party_y : 0,
        w->target_x, w->target_y, w->creature, 1);
    return w->attack_succeeded ? 1 : 0;
}

static void dm2_runtime_wield_set_pending(void *ctx, int16_t damage)
{
    (void)ctx;
    (void)damage;
}

static const DM2_RuntimeWieldCallbacks dm2_runtime_wield_callbacks = {
    dm2_runtime_wield_get_creature,
    NULL,
    NULL,
    dm2_runtime_wield_calc_damage,
    dm2_runtime_wield_set_pending
};

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

int dm2_v1_runtime_proceed_hand_command(
    int slot_index, DM2_V1_EngageCommandReceipt *receipt)
{
    DM2_V1_RuntimeState *rt = &g_dm2_runtime;
    DM2_V1_Hero *hero;
    DM2_V1_GameState *game;
    const DM2_V1_AssetLoader *loader;
    DM2_V1_SkprojectQueryCls1Receipt cls1_receipt;
    DM2_V1_SkprojectQueryCls2Receipt cls2_receipt;
    DM2_V1_EngageCommandRequest request;
    DM2_RuntimeAttackQueueContext queue;
    DM2_RuntimeWieldContext wield;
    uint16_t item;
    uint8_t cls1 = 0xffu;
    uint8_t cls2 = 0xffu;
    int16_t where;
    int entry;
    int result;
    int is_wield_action;

    if (receipt) memset(receipt, 0, sizeof(*receipt));
    if (slot_index < 0 || slot_index > 2 || !rt->source_party_valid ||
        !rt->session_snapshot_valid || !rt->record_pools_valid ||
        !rt->boot || !rt->boot->assets_verified ||
        !rt->boot->graphics_dat || !rt->source_party.curacthero ||
        rt->source_party.curacthero > rt->source_party.heros_in_party ||
        rt->source_party.curactmode < 0 || rt->source_party.curactmode > 1) {
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }
    hero = &rt->source_party.hero[rt->source_party.curacthero - 1];
    if (hero->curHP <= 0) {
        if (receipt) {
            receipt->valid = 1;
            receipt->hero_dead = 1;
        }
        return 0;
    }
    item = (uint16_t)hero->item[rt->source_party.curactmode];
    if (item == 0xffffu ||
        !dm2_v1_skproject_query_cls1_from_record_ex(
            item, &rt->record_pools, &cls1, &cls1_receipt) ||
        !dm2_v1_skproject_query_cls2_from_record(
            item, &rt->record_pools, &cls2, &cls2_receipt)) {
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }

    /* c_events.cpp passes 0..2 to DM2_ENGAGE_COMMAND. The current item
     * command table is the four CMDSTR text entries beginning at entry 8;
     * resolve that table from the mounted real GDAT, never from labels or a
     * host-side action list. */
    entry = 8 + slot_index;
    loader = dm2_v1_boot_asset_loader(rt->boot);
    if (!loader) {
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }
    memset(&cls1_receipt, 0, sizeof(cls1_receipt));
    memset(&cls2_receipt, 0, sizeof(cls2_receipt));
    memset(&request, 0, sizeof(request));
    memset(&queue, 0, sizeof(queue));
    memset(&wield, 0, sizeof(wield));
    if (!dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 2,
                                     &request.cmd.action_id)) {
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }
    is_wield_action = request.cmd.action_id == (DM2_ENGAGE_WIELD_WEAPON + 1) ||
                      request.cmd.action_id == 8;
    if ((!is_wield_action &&
         (!dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 17, &where) ||
          (where != 0 && where != 1 && where - 1 != rt->source_party.curactmode))) ||
        (request.cmd.action_id != (DM2_ENGAGE_ATTACK + 1) &&
         request.cmd.action_id != (DM2_ENGAGE_CAST_MISSILE + 1) &&
         request.cmd.action_id != (DM2_ENGAGE_WIELD_WEAPON + 1) &&
         request.cmd.action_id != 8) ||
        !dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 0,
                                     &request.cmd.skill_type) ||
        !dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 3,
                                     &request.cmd.power_base) ||
        !dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 4,
                                     &request.cmd.power_random) ||
        ((!dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 5,
                                       &request.cmd.delay)) &&
         request.cmd.action_id != 8) ||
        !dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 7,
                                     &request.cmd.defense_class) ||
        !dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 9,
                                     &request.cmd.skill_exp) ||
        ((!dm2_runtime_query_cmd_field(loader, cls1, cls2, entry, 0x10,
                                       &request.cmd.damage_type)) &&
         request.cmd.action_id != 8)) {
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }
    /* c_engage.cpp writes the raw command selector to
     * hero->handcmd[curactmode] before resolving CMDSTR.  Keep that source
     * owner live so a subsequent party/creature collision can resolve the
     * same authenticated action rather than guessing from the UI slot. */
    hero->handcmd[rt->source_party.curactmode] = (int8_t)slot_index;
    game = (DM2_V1_GameState *)rt->boot->dm2_state;
    request.hero_index = rt->source_party.curacthero - 1;
    request.hand = rt->source_party.curactmode;
    request.hero_alive = 1;
    request.hero_hp = hero->curHP;
    request.hero_max_hp = hero->maxHP;
    request.hero_mp = hero->curMP;
    request.hero_abs_dir = hero->absdir;
    request.hero_party_pos = hero->partypos;
    request.item_handle = (int16_t)item;
    request.party_x = game->party_x;
    request.party_y = game->party_y;
    request.party_dir = game->party_dir;
    request.party_map = game->current_level;
    request.creature_at_target = -1;
    request.tile_value = 0;
    request.wield_strength = request.cmd.power_base;
    if ((request.cmd.action_id == (DM2_ENGAGE_WIELD_WEAPON + 1) ||
         request.cmd.action_id == 8) && rt->boot->dungeon_data) {
        DM2_V1_DungeonData *dungeon =
            (DM2_V1_DungeonData *)rt->boot->dungeon_data;
        static const int dx[4] = { 0, 1, 0, -1 };
        static const int dy[4] = { -1, 0, 1, 0 };
        int dir = game->party_dir & 3;
        int tx = game->party_x + dx[dir];
        int ty = game->party_y + dy[dir];
        int raw = dm2_v1_dungeon_get_tile_raw(dungeon, game->current_level,
                                               tx, ty);
        wield.runtime = rt;
        wield.dungeon = dungeon;
        wield.map = game->current_level;
        wield.target_x = tx;
        wield.target_y = ty;
        wield.creature = dm2_v1_get_creature_at(
            &rt->record_pools, dungeon, game->current_level, tx, ty);
        request.creature_at_target = wield.creature;
        request.tile_value = raw >= 0 ? (int16_t)raw : 0;
        request.wield_cb = &dm2_runtime_wield_callbacks;
        request.wield_ctx = &wield;
    }
    request.game_tick = (uint32_t)rt->tick_count;
    request.queue_timer_cb = dm2_runtime_queue_attack_timer;
    request.queue_timer_ctx = &queue;
    request.attack_counter = rt->source_attack_counter;
    request.attack_hero_flag = rt->source_party.curacthero;
    hero->heroflag |= 0x0800;
    result = dm2_v1_runtime_engage_command(&request, receipt);
    if (!result || (is_wield_action
                        ? (!receipt || !receipt->creature_attacked)
                        : !queue.queued)) {
        hero->heroflag &= (int16_t)~0x0800;
        if (receipt) receipt->fail_closed = 1;
        return 0;
    }
    return 1;
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
