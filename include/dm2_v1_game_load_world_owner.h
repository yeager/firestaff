#ifndef FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H
#define FIRESTAFF_DM2_V1_GAME_LOAD_WORLD_OWNER_H

/*
 * dm2_v1_game_load_world_owner.h — owned File_header world for New Game.
 *
 * SKProject loads the dungeon structure before DM2_GAME_LOAD mutates the
 * DYN, champion and timer state.  This owner is that first durable boundary:
 * it copies only the hash-admitted File_header image and its exact DB pools
 * to RAM.  It is intentionally not a playable session.
 *
 * Source: SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON (616-640),
 *         ::DM2_GAME_LOAD (1415-1546),
 *         SKULLWIN/c_record.cpp::DM2_GET_ADDRESS_OF_RECORD (28-57).
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_caii_source_owner.h"
#include "dm2_v1_eventqueue_pc34_compat.h"
#include "dm2_v1_init_game_ui_owner.h"
#include "dm2_v1_gdat_scene_m11_command.h"
#include "dm2_v1_game_load_sound_owner.h"
#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_loadlevel_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_sound.h"
#include "dm2_v1_skproject_core.h"
#include "dm2_v1_timer_queue_pc34_compat.h"
#include "dm2_v1_update_weather_pc34_compat.h"
#include "dm2_v1_viewport_renderer.h"
#include "dm2_v1_world_model.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Immutable transition receipt for the actual mirror clicks.  The final hero
 * count alone cannot prove that an eventual input owner preserved the click
 * order.  This remains private until complete GAME_LOAD ownership exists.
 * Source: SKProject c_hero.cpp::DM2_SELECT_CHAMPION (1119-1168),
 *         ::DM2_SELECT_CHAMPION_LEADER (2325-2354). */
typedef struct {
    int valid;
    uint8_t click_count;
    uint8_t leader_select_count;
    int16_t initial_event_hero_index;
    int16_t final_event_hero_index;
    int16_t next_champion_number_after_click[DM2_MAX_HEROES];
    uint16_t mirror_object_id[DM2_MAX_HEROES];
    int8_t party_position[DM2_MAX_HEROES];
    uint32_t transition_hash;
} DM2_V1_GameLoadChampionSelectionReceipt;

/* The original start path first sizes xsndptr2 from every type-2 GDAT row,
 * then DM2_LOAD_DYN4 adds only marked triples and DM2_482b_0684 binds their
 * already materialised raw samples.  This is a durable private equivalent.
 * It deliberately has no mixer, PCM conversion or process-global binding. */
/* GEN2 bridge over the dynamic GAME_LOAD sound owner.  This copies only the
 * fixed c_sfx queues into the compatibility algorithm; xsndptr2 remains the
 * authenticated dynamic SOUND9 span and is never replaced by the old array. */
typedef struct {
    int valid;
    int blocked_owner;
    int blocked_queue;
    int blocked_sample;
    int queued_positional;
    uint16_t queue_index;
    int16_t sample_id;
    uint32_t transaction_hash;
} DM2_V1_GameLoadSoundNoiseReceipt;

int dm2_v1_game_load_sound_owner_queue_noise_gen2(
    DM2_V1_GameLoadSoundOwner *owner,
    int8_t cls1, int8_t cls2, int8_t cls3, int8_t cls_alt,
    int16_t x, int16_t y, int16_t ecxw, int16_t volume, int16_t delay_mode,
    const DM2_V1_SoundQueueEnv *env,
    DM2_V1_GameLoadSoundNoiseReceipt *out_receipt);

/* Source-shaped DM2_QUERY_SND_ENTRY_INDEX over the private GAME_LOAD
 * xsndptr2 owner.  Unlike the legacy fixed queue, this may only return an
 * already DYN4/482b_0684-materialised entry with a proven raw/sample binding;
 * it never appends or resolves a new GDAT row.  The returned index is the
 * original 1-based SOUND9 index, or zero when absent/unadmitted.
 * Source: SKProject SKULLWIN/c_sound.cpp::DM2_QUERY_SND_ENTRY_INDEX
 * (650-673), c_gdatfile.cpp::DM2_482b_0684 (932-975). */
/* Private result of the three writes in DM2_LOAD_NEW_DUNGEON immediately
 * before DM2_READ_DUNGEON_STRUCTURE(1). The selected medium itself remains
 * owned by BootProfile; this is deliberately not a host file-open shim.
 * Source: SKProject SKWINSPX/src/v5/sksvgame.cpp::DM2_LOAD_NEW_DUNGEON
 * (616-640). */
typedef struct {
    int valid;
    int16_t party_count;
    int16_t leader_hand_record;
    uint32_t save_stream_bytes_consumed;
    uint32_t receipt_hash;
} DM2_V1_GameLoadNewDungeonResetReceipt;

/* Source-owned c_light inputs immediately after DM2_GAME_LOAD establishes
 * the initial map, but before the mirror event loop creates a c_party.
 * Every zero here is an original DM2 data-initialiser value, not a host
 * default: retain the complete branch inputs so later presentation may not
 * replace the dynamic-map calculation with a static scene value.
 * Source: SKWINSPX/src/v5/dm2data.cpp::c_dm2data (1130-1454),
 *         sksvgame.cpp::DM2_GAME_LOAD (1546-1565),
 *         sklodlvl.cpp::DM2_LOAD_LOCALLEVEL_DYN (714-730),
 *         sklight.cpp::DM2_RECALC_LIGHT_LEVEL (24-198). */
typedef struct {
    int valid;
    int map;
    uint8_t graphicsset;
    uint8_t party_count;
    uint16_t leader_hand_record;
    uint16_t savegame_light;
    uint16_t v1e0974;
    uint16_t v1e0978;
    uint8_t weather_active;
    uint8_t weather_index;
    uint8_t weather_delta;
    uint8_t weather_darkness_active;
    DM2_V1_CLightMapDescriptorReceipt map_descriptor;
    uint32_t source_state_hash;
} DM2_V1_GameLoadPreselectionLightReceipt;

/* RAM-owned portion of c_light::DM2_CHECK_RECOMPUTE_LIGHT.  LOAD_LOCALLEVEL_DYN
 * sets dirty bit 2, then CHECK_RECOMPUTE_LIGHT selects the party map and
 * zeroes one byte per source square at x*32+y.  FIND_WALK_PATH subsequently
 * fills those bytes.  Keep allocation and its pre-walk bytes separate: an
 * all-zero buffer is original initial state, never a completed visibility
 * result. Source: SKProject SKULLWIN/c_loadlevel.cpp:1363 and
 * c_light.cpp:490-529. */
typedef struct {
    int valid;
    int16_t primary_map;
    int16_t secondary_map;
    uint8_t primary_width;
    uint8_t primary_height;
    uint8_t secondary_width;
    uint8_t secondary_height;
    uint8_t dirty_flags_before_check;
    uint8_t walk_path_pending;
    uint8_t *primary_cells;
    uint8_t *secondary_cells;
    size_t primary_cell_bytes;
    size_t secondary_cell_bytes;
    uint32_t source_state_hash;
} DM2_V1_GameLoadLightVisibilityOwner;

/* Exact per-map lists consumed by LOAD_LOCALLEVEL_DYN before it resolves
 * wall, floor and door-decoration GDAT. The lists are copied from the
 * authenticated File_header map payload, never inferred from a graphics
 * set or re-used from another level. */
typedef struct {
    int valid;
    int map;
    uint8_t wall_count;
    uint8_t floor_count;
    uint8_t door_ornate_count;
    uint8_t wall_gfx[16];
    uint8_t floor_gfx[16];
    uint8_t door_ornate_gfx[16];
    uint32_t source_list_hash;
} DM2_V1_GameLoadLocalLevelGraphicsReceipt;

/* One DB4 Creature::possession outcome.  A null root is source data and is
 * kept as `has_possession == 0`; it is not an error and must not become an
 * empty synthetic item chain. */
typedef struct {
    int valid;
    uint16_t creature_object_id;
    uint8_t has_possession;
    DM2_V1_FileHeaderCreaturePossessionReceipt receipt;
} DM2_V1_GameLoadCreaturePossessionReceipt;

/* The source viewport queries this exact pre-mirror projection from the
 * current c_map.  It is deliberately a map receipt rather than a renderer
 * state: every coordinate, raw tile word and ground-stack root comes from
 * the authenticated File_header clone, so no default floor/wall can leak
 * into the entrance before a complete session exists.
 *
 * Source: SKProject SKULLWIN/c_gui_vp.cpp view-cell traversal; the same
 * D0..D3 projection is used by the bounded runtime bridge. */
#define DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT 11
typedef struct {
    uint8_t view_square;
    uint8_t source_available;
    int16_t map_x;
    int16_t map_y;
    uint16_t raw_tile; /* zero only when source_available is zero */
    uint16_t ground_stack_root;
    uint8_t square_type;
} DM2_V1_GameLoadPreselectionViewCell;

typedef struct {
    int valid;
    int map;
    uint8_t party_x;
    uint8_t party_y;
    uint8_t party_direction;
    uint8_t cell_count;
    DM2_V1_GameLoadPreselectionViewCell
        cells[DM2_V1_GAME_LOAD_PRESELECTION_VIEW_CELL_COUNT];
    uint32_t source_view_hash;
} DM2_V1_GameLoadPreselectionViewReceipt;

/* Renderer-shaped, but still private, entrance terrain state.  D3C has no
 * source GRAPHICSSET field and therefore remains explicitly absent; all
 * other cells are copied from the authenticated projection above. */
typedef struct {
    int valid;
    int map;
    uint32_t map_data_hash;
    uint32_t scene_control_hash;
    uint32_t c_light_hash;
    uint8_t source_cell_count;
    DM2_ViewSquare squares[DM2_SQ_COUNT];
    uint32_t source_viewport_hash;
} DM2_V1_GameLoadPreselectionViewportReceipt;

/* The CAII array is not stored in DUNGEON.DAT. DM2_INIT derives its exact
 * capacity from the authenticated DB4 records and AIDefinition flags before
 * it allocates any c_creature slots. Source: SKProject
 * SKULLWIN/startend.cpp::DM2_1c9a_3c30 (462-501). */
typedef struct {
    int valid;
    uint16_t db4_record_count;
    uint16_t nonstatic_creature_count;
    uint16_t source_capacity;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiCapacityReceipt;

/* One source DB4 encounter in FILL_CAII_CUR_MAP order. The record remains
 * unmodified; this is the all-map traversal prerequisite for RESET_CAII.
 * Source: SKProject SKULLWIN/c_1c9a.cpp::DM2_FILL_CAII_CUR_MAP
 * (9896-9994). */
typedef struct {
    int16_t map;
    uint8_t x;
    uint8_t y;
    int16_t record_handle;
    uint8_t creature_type;
    uint8_t static_ai;
    uint16_t record_word_a;
    uint16_t packed_position;
    /* DM2_GET_CREATURE_ANIMATION_FRAME(type, 0x11, ..., packed_position)
     * for static AI only. Dynamic AI requires the later c_random/CCM owner
     * and remains 0xffff here. */
    uint16_t static_animation_frame;
} DM2_V1_GameLoadCaiiMapCandidate;

typedef struct {
    int valid;
    uint16_t map_count;
    uint16_t candidate_count;
    uint16_t static_candidate_count;
    uint16_t dynamic_candidate_count;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiMapReceipt;

/* The deterministic static half of RESET_CAII.  It is deliberately a
 * private staging receipt: dynamic creatures still need the complete
 * c_creature/timer/CCM transaction before GAME_LOAD can be published.
 * Source: SKProject SKULLWIN/startend.cpp::DM2_RESET_CAII (1033-1070),
 * c_1c9a.cpp::DM2_FILL_CAII_CUR_MAP (9896-9994). */
typedef struct {
    int valid;
    uint16_t db4_slot_reset_count;
    uint16_t static_animation_count;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiStaticReceipt;

/* Private source-shaped result of DM2_1c9a_0cf7.  A timer handle here is a
 * c_tim slot, never a synthetic ticket: slot zero is valid in the original
 * timer array. Source: SKProject SKULLWIN/c_1c9a.cpp (5695-5732). */
typedef struct {
    int valid;
    int replaced_pending_timer;
    int16_t record_handle;
    int16_t caii_slot;
    int16_t timer_slot;
    uint8_t timer_type;
    uint8_t creature_type;
    int16_t map;
    uint8_t x;
    uint8_t y;
    uint32_t due_tick;
} DM2_V1_GameLoadCaiiThinkReceipt;

/* The dynamic half of RESET_CAII/FILL_ORPHAN_CAII. Every count is derived
 * from authentic DB4 traversal and its one shared c_tim/c_creature/SOUND9
 * transaction; no source candidate is silently skipped. A map-gated sound
 * no-op is distinct from a queued positional sound.
 * Source: SKProject SKWINSPX/src/v5/SK1C9A.cpp (5772-5894, 9896-10031). */
typedef struct {
    int valid;
    int blocked_unowned_0a48;
    int16_t failed_record_handle;
    uint8_t failed_creature_type;
    int16_t failed_map;
    uint8_t failed_x;
    uint8_t failed_y;
    int failed_0a48_result;
    uint16_t dynamic_candidate_count;
    uint16_t allocated_slot_count;
    uint16_t think_timer_count;
    uint16_t noise_queue_count;
    uint16_t noise_map_gate_count;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiDynamicReceipt;

/* Minimal, private PREPARE_LOCAL_CREATURE_VAR identity for a dynamic DB4
 * candidate. It deliberately retains no fabricated c_creature slot and no
 * mutable CCM state. `noise_request_pending` is not an event: 0a48 can only
 * form QUEUE_NOISE_GEN1 after a real GDAT animation row supplies its index.
 * Source:
 * SKProject SKULLWIN/c_ai.cpp (5817-5892), c_1c9a.cpp (5434-5561). */
typedef struct {
    int valid;
    int16_t record_handle;
    int16_t map;
    uint8_t x;
    uint8_t y;
    uint8_t creature_type;
    uint16_t ai_flags;
    uint16_t record_word_a;
    uint16_t packed_position;
    uint8_t initial_timer_type;
    int16_t home_map;
    uint16_t adj_owner_offset;
    uint16_t adj_base_before;
    uint16_t adj_frame_before;
    int noise_request_pending;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiLocalContext;

typedef struct {
    int valid;
    uint16_t context_count;
    uint16_t noise_request_pending_count;
    uint32_t source_hash;
} DM2_V1_GameLoadCaiiLocalContextReceipt;

/* Fresh c_moverec globals retained with the private GAME_LOAD world. These
 * are source register state, not inferred movement results. DM2_MOVE_RECORD_TO
 * resets v1e1020 at entry and writes the remaining fields only after its
 * c_map/record/timer work completes; no consumer may execute it until the
 * complete transaction owner exists.
 *
 * Source: SKULLWIN/dm2data.cpp (1128, 1231-1237), c_moverec.cpp::
 * DM2_MOVE_RECORD_TO (392-1142), c_map.cpp::DM2_CHANGE_CURRENT_MAP_TO. */
typedef struct {
    int valid;
    uint32_t v1e0390_l00;
    int32_t v1e1020;
    int16_t v1e1024;
    int16_t v1e1026;
    int16_t v1e1028;
    int16_t v1e102a;
    int16_t v1e102c;
    int16_t v1e102e;
    int16_t v1d3248;
    int16_t v1d3248_before_final_change;
} DM2_V1_GameLoadMoverecState;

typedef struct {
    /* Private host-lifecycle guard. This is not original game state: it only
     * makes replacement of a retained all-RAM owner safe without inspecting
     * arbitrary caller bytes as allocation pointers. */
    uint32_t lifecycle_tag;
    /* `prepared` means all source bytes have one RAM owner. `committed` is
     * deliberately zero until the later source-ordered DYN/hero/timer
     * transaction can publish every owner atomically. */
    int prepared;
    int committed;
    /* The original GAME_LOAD has completed its New Dungeon side before the
     * player clicks a mirror.  Keep that selection-free boundary explicit:
     * a nonzero value means this owner has authenticated map/DYN/timer
     * prerequisites but still has no c_party or inventory publication. */
    int source_preselection_ready;
    int current_map;
    uint32_t source_transaction_hash;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet record_pools;
    /* Read-only AIDefinition/DB4 source bytes plus DM2_1c9a_3c30's exact
     * capacity calculation. Candidate records remain unmodified until the
     * complete all-map RESET_CAII transaction owns dynamic c_tim, CCM and
     * animation branches. */
    DM2_V1_CaiiSourceOwner caii_source;
    DM2_V1_GameLoadCaiiCapacityReceipt caii_capacity;
    DM2_V1_GameLoadCaiiMapReceipt caii_map_receipt;
    DM2_V1_GameLoadCaiiMapCandidate *caii_map_candidates;
    /* DM2_RESET_CAII first allocates/clears c_creature slots and then
     * traverses maps. Keep that storage and the exact c_randomdata start
     * state privately, but do not assign a slot or rewrite DB4 byte@5 until
     * the complete 09db/0cf7/0a48 mutation can commit atomically.
     * Source: SKProject SKULLWIN/startend.cpp::DM2_RESET_CAII (1033-1070),
     * c_random.cpp::c_randomdata::init (7-13). */
    DM2_V1_CaiiArray caii_slots;
    DM2_V1_DropRng caii_rng;
    int caii_rng_initialized;
    DM2_V1_GameLoadCaiiStaticReceipt caii_static_animation;
    /* Published only after every dynamic DB4 candidate has completed the
     * shared slot/timer/0a48/SOUND9 transaction. */
    DM2_V1_GameLoadCaiiDynamicReceipt caii_dynamic;
    /* Read-only local-creature contexts for each real dynamic candidate.
     * They are the source data predecessor of 0a48, not an active s350 or a
     * permission to allocate CAII slots. */
    DM2_V1_GameLoadCaiiLocalContext *caii_local_contexts;
    DM2_V1_GameLoadCaiiLocalContextReceipt caii_local_context_receipt;
    /* c_savegame.cpp::DM2_READ_DUNGEON_STRUCTURE computes these capacities
     * before it allocates the original 12-byte c_tim array and index heap.
     * They are allocation limits, not invented queued timers. */
    uint16_t record_capacities[DM2_V1_RECORD_POOL_COUNT];
    DM2_V1_TimerEntry *timer_entries;
    int16_t *timer_indices;
    DM2_V1_TimerQueue timer_queue;
    uint16_t timer_capacity;
    int fresh_game_mode;
    /* The private LOAD_NEW_DUNGEON reset is an explicit predecessor of every
     * selected champion. It records source values only and cannot publish a
     * party or a save stream to M11. */
    DM2_V1_GameLoadNewDungeonResetReceipt load_new_dungeon_reset;
    int actuator_generators_processed;
    /* DM2_move_2fcf_0b8b / CHANGE_CURRENT_MAP_TO source state.  This stays
     * private until the later all-owner session commit. */
    int source_map_context_materialized;
    int source_party_map;
    uint8_t source_party_x;
    uint8_t source_party_y;
    uint8_t source_party_direction;
    /* Private result of skmove.cpp::DM2_move_2fcf_0b8b.  The original
     * probes the party square and its four neighbours for a File_header
     * teleporter after GAME_LOAD.  These are map-context bytes only; no
     * viewport, party placement or live map is published from them. */
    int source_staircase_flag;
    int16_t source_teleporter_map;
    int16_t source_display_x;
    int16_t source_display_y;
    uint8_t source_party_absdir;
    /* DM2_move_2fcf_0b8b derives absdir from the two bytes returned by
     * GET_TELEPORTER_DETAIL. Preserve them with the selected direct (-1) or
     * neighbouring (0..3) probe so a later presentation owner cannot use
     * the destination map byte as a direction. */
    int8_t source_teleporter_probe_direction;
    uint8_t source_teleporter_source_direction;
    uint8_t source_teleporter_destination_direction;
    int source_display_pose_valid;
    DM2_V1_GameLoadMoverecState source_moverec;
    /* DM2__INIT_GAME calls c_1031_0541(5) while LOAD_NEW_DUNGEON's party is
     * still empty, before INIT_CHAMPIONS/DM2_2f3f_0789.  This is the mutable
     * source UI-table state for that precise point, not an M11 menu or a
     * post-selection reconstruction. */
    DM2_V1_InitGameUiOwner preselection_init_game_ui;
    int preselection_init_game_ui_materialized;
    DM2_V1_GameLoadLocalLevelGraphicsReceipt preselection_local_graphics;
    /* c_light inputs for the real entrance map. This receipt owns the
     * source initialisation inputs only; it is not a fabricated light level
     * and does not make a viewport or a party visible. */
    DM2_V1_GameLoadPreselectionLightReceipt preselection_light;
    DM2_V1_GameLoadLightVisibilityOwner preselection_light_visibility;
    /* Material is decoded from the admitted GDAT only after the source map
     * has supplied v1d6c02. The c_light receipt remains private and is
     * available only for a branch whose complete inputs are owned here. */
    int preselection_scene_materialized;
    DM2_V1_GdatSceneM11CommandPlan preselection_scene_plan;
    DM2_V1_GdatSceneLightM11Receipt preselection_scene_light;
    DM2_V1_CLightM11Receipt preselection_c_light;
    DM2_V1_GameLoadPreselectionViewReceipt preselection_view;
    DM2_V1_GameLoadPreselectionViewportReceipt preselection_viewport;
    /* Borrowed only while this private owner exists; it is the same
     * hash-admitted profile that owns asset_loader. */
    const DM2_V1_BootProfile *boot_profile;
    const DM2_V1_AssetLoader *asset_loader;
    int dyn4_materialized;
    uint16_t dyn4_selector_count;
    uint32_t dyn4_selector_ids[
        DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES];
    uint32_t dyn4_materialized_hash;
    uint16_t validated_map_count;
    uint32_t validated_world_hash;
    DM2_V1_GdatDyn4MaterializedSelection dyn4_selections[
        DM2_V1_BOOT_MAX_CHAMPION_SELECTION_CANDIDATES];
    DM2_V1_GameLoadSoundOwner sound_owner;
    /* Source facts available before DM2_SELECT_CHAMPION.  They deliberately
     * live outside `transaction`, whose validity still means that one or
     * more real mirror clicks have been admitted. */
    DM2_V1_FileHeaderWorldInteractionReceipt preselection_world_interactions;
    DM2_V1_FileHeaderActuatorGeneratorReceipt preselection_actuator_generators;
    DM2_V1_FileHeaderRuntimeMapReceipt preselection_entrance_map;
    /* Direct DB0 Door payloads for the File_header map currently owned by
     * c_map.  This is an address-only receipt; it has no timer, collision or
     * presentation side effect. */
    DM2_V1_G1RuntimeMapDoorReceipt preselection_map_doors;
    /* Source-addressed DB5..DB15 entries reached from this map's complete
     * File_header chains.  It deliberately remains a locator receipt until
     * DRAW_STATIC_OBJECT/DRAW_ITEM have a complete placement owner. */
    DM2_V1_FileHeaderRuntimeObjectReceipt preselection_map_objects;
    /* DB2 Text payloads reached by the same validated map walk.  Visibility,
     * message lookup and special-marker effects stay uncommitted. */
    DM2_V1_FileHeaderRuntimeTextReceipt preselection_map_texts;
    /* Direct DB1 Teleporter payloads on the current File_header map.  They
     * are retained without applying a party transition or sound request. */
    DM2_V1_FileHeaderRuntimeTeleporterReceipt preselection_map_teleporters;
    /* Direct DB3 Actuator fields on the current File_header map.  The
     * generator pass owns only its separately proved mutations; this receipt
     * grants no generic actuator dispatch. */
    DM2_V1_G1RuntimeMapActuatorReceipt preselection_map_actuators;
    /* DB4 creature records reached through the current File_header map's
     * complete chains. CAII slots, movement and drops remain absent. */
    DM2_V1_FileHeaderRuntimeCreatureReceipt preselection_map_creatures;
    uint8_t preselection_creature_possessions_materialized;
    uint16_t preselection_creature_possession_count;
    DM2_V1_GameLoadCreaturePossessionReceipt
        preselection_creature_possessions[
            DM2_V1_FILE_HEADER_RUNTIME_MAX_CREATURE_RECORDS];
    DM2_V1_BootNewGameEntranceReceipt preselection_entrance;
    DM2_V1_BootChampionDyn4RosterReceipt preselection_dyn4_roster;
    /* Canonical DB3 mirror order and each source candidate's real hero/item
     * preconditions. This is the only roster a future M11 mirror panel may
     * expose; it is not a default party order. */
    DM2_V1_BootChampionSelectionCensus preselection_mirror_roster;
    uint32_t preselection_hash;
    DM2_V1_BootNewGameTransactionReceipt transaction;
    DM2_V1_BootNewGamePartySelection selected_mirrors[DM2_MAX_HEROES];
    uint8_t selected_mirror_count;
    /* This remains zeroed until the source-ordered champion step.  It is
     * never installed in M11 or the runtime party state. */
    int champion_selection_materialized;
    /* Private equivalents of ddat.v1e0288 and eventqueue.event_heroidx.
     * SELECT_CHAMPION writes v1e0288 after every real mirror click, and only
     * the first click calls SELECT_CHAMPION_LEADER(0).  They are kept here
     * because the UI/event queue is not yet a session owner. */
    int16_t source_next_champion_number;
    int16_t source_event_hero_index;
    /* c_eventqueue::init runs before startend's first mirror selection.
     * After SELECT_CHAMPION_LEADER(0), its only durable GAME_LOAD input is
     * the selected leader.  Retain the whole source-shaped empty queue so a
     * later session handoff does not fabricate queue state around that
     * scalar. Source: SKULLWIN/c_eventqueue.cpp::init; startend.cpp::
     * DM2_2f3f_0789. */
    DM2_V1_EventQueue source_event_queue;
    int source_event_queue_materialized;
    /* startend.cpp::DM2_2f3f_0789 calls events_2f3f_04ea with 0x92 directly
     * after the scripted first selection.  These source fields retain its
     * private record-bit/hero release transition; they are not UI state. */
    int source_startend_first_champion_released;
    uint32_t source_startend_first_champion_tick;
    uint16_t source_startend_first_champion_object_id;
    DM2_V1_GameLoadChampionSelectionReceipt champion_selection_receipt;
    DM2_V1_SksaveItemBonusReceipt champion_item_bonus;
    DM2_V1_Party selected_party;
} DM2_V1_GameLoadWorldOwner;

/* Fresh-game values owned by c_move/c_input before the first player movement.
 * These are not host defaults: dm2data.cpp initializes each field before
 * DM2_GAME_LOAD, and c_move.cpp reads and later mutates them while handling
 * delayed movement, creature interaction and input suppression.  Retaining
 * them in the private candidate gives a later c_moverec transaction the
 * source state it must consume; it does not make movement executable yet.
 *
 * Source: SKULLWIN/dm2data.cpp (1063, 1107-1109, 1254, 1326-1332, 1383),
 *         c_move.cpp::DM2_PERFORM_MOVE (197-612), c_input.cpp (289-312). */
typedef struct {
    int valid;
    int16_t delayed_move_ticks;       /* ddat.v1e025c */
    int16_t delayed_target_x;         /* ddat.v1e026a */
    int16_t delayed_target_y;         /* ddat.v1e0268 */
    int16_t delayed_direction;        /* ddat.v1e0278 */
    int16_t delayed_command;          /* ddat.v1e0256 */
    int16_t move_clock;               /* ddat.v1e026e */
    int16_t move_event;               /* ddat.v1e025e */
    int16_t move_event_direction;     /* ddat.v1e0274 */
    int16_t pending_creature;         /* ddat.v1d4000: OBJECT_END */
    int command_in_progress;          /* ddat.v1e0488 */
} DM2_V1_GameLoadMovementState;

/* The retained prefix of DM2_LOAD_LOCALLEVEL_DYN before its map-tile walk.
 * This is not a completed dynamic load: the exact record/actuator traversal,
 * DYN4 resolution, weather and light passes remain pending and therefore
 * cannot be consumed by a renderer, mixer or host runtime.  Keeping this
 * source-ordered RAM queue prevents a later world owner from reconstituting
 * base resources with caller-authored defaults.
 *
 * Source: SKULLWIN/c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_DYN (203-327). */
typedef struct {
    int valid;
    int record_traversal_pending;
    int dyn4_pending;
    uint8_t source_v1e13fe[2];
    int16_t source_map;
    uint8_t source_music_type;
    uint8_t source_party_count;
    uint16_t fixed_prefix_count;
    uint16_t map_selector_index;
    DM2_V1_DynLoadState queue;
    uint32_t source_resource_hash;
} DM2_V1_GameLoadLocalDynPrelude;

/* One ordered GenericRecord visit made by LOAD_LOCALLEVEL_DYN's map scan.
 * `object_id` remains an ObjectID into the candidate's mutable RecordPoolSet;
 * no record is copied, normalised or reallocated. Source: c_loadlevel.cpp
 * (518-626), c_map.cpp::GET_TILE_RECORD_LINK, c_record.cpp::
 * DM2_GET_NEXT_RECORD_LINK. */
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t object_id;
    uint16_t next_object_id;
    uint16_t word2;
    uint8_t type;
    uint8_t record_size;
    uint32_t record_hash;
} DM2_V1_GameLoadLocalDynRecordVisit;

/* Complete source-order map input after LOAD_LOCALLEVEL_DYN's fixed prefix
 * and before its record-specific selector/temporary-array branches. This
 * owns only facts already present in the candidate File_header/RecordPoolSet:
 * DYN4, GDAT, weather, light and any gameplay state remain pending. */
typedef struct {
    int valid;
    int16_t map;
    uint16_t width;
    uint16_t height;
    uint16_t marked_tile_count;
    uint16_t root_count;
    uint16_t record_count;
    uint16_t record_capacity;
    uint16_t type_count[DM2_V1_RECORD_POOL_COUNT];
    DM2_V1_GameLoadLocalDynRecordVisit *records;
    uint32_t source_trace_hash;
} DM2_V1_GameLoadLocalDynMapScan;

/* The record-specific, RAM-only part of LOAD_LOCALLEVEL_DYN's map walk.
 * RG51p, xp_0c and xp_10 are the three source-cleared 0xfa-byte buffers.
 * `selector_queue` starts with the retained fixed prefix and receives only
 * the source-proven DB3 subtype 0x7e selectors in scan order.  DB3 subtype
 * 0x27 on a teleporter square needs mapdat.tmpmap's cross-map byte list;
 * keeping it as an explicit blocker is intentional until that owner exists.
 * Source: SKULLWIN/c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_DYN (518-626). */
typedef struct {
    int valid;
    int16_t map;
    uint8_t creature_marks[DM2_V1_LOADLEVEL_CREATURE_TABLE_SIZE]; /* RG51p */
    uint8_t wall_text_marks[DM2_V1_LOADLEVEL_CREATURE_TABLE_SIZE]; /* xp_0c */
    uint8_t floor_text_marks[DM2_V1_LOADLEVEL_CREATURE_TABLE_SIZE]; /* xp_10 */
    uint16_t text_record_count;
    uint16_t text_temp_mark_count;
    uint16_t creature_mark_count;
    uint16_t mirror_selector_count;
    uint16_t cross_map_actuator_count;
    DM2_V1_DynLoadState selector_queue;
    uint32_t source_effect_hash;
} DM2_V1_GameLoadLocalDynRecordEffects;

/* A private, all-RAM candidate for a complete mutable DM2_GAME_LOAD state.
 * It is a transaction staging area, not a live game: no field is installed
 * in M11, the process-global runtime, audio backend or input dispatcher.
 *
 * A selected champion and allocated CAII storage are insufficient.  The
 * source first runs RESET_CAII and FILL_CAII_CUR_MAP, including each dynamic
 * 0a48/CCM/noise/timer transaction, before the state may advance as a game
 * session.  Until one owner can prove that full transaction, construction is
 * intentionally rejected rather than cloning a pre-reset DB4 pool beside an
 * unrelated c_creature/timer snapshot.
 *
 * Source order: sksvgame.cpp::DM2_GAME_LOAD (1415-1565), startend.cpp::
 * DM2_RESET_CAII (1111-1145), SK1C9A.cpp::DM2_FILL_CAII_CUR_MAP
 * (9896-10012), skhero.cpp::DM2_SELECT_CHAMPION (1119-1168). */
typedef struct {
    /* See DM2_V1_GameLoadWorldOwner::lifecycle_tag. */
    uint32_t lifecycle_tag;
    int valid;
    uint32_t source_transaction_hash;
    /* Provenance only: this binds the source transaction and retained
     * capacities. It is not a content hash of mutable records, timers, CAII
     * or party state and must never be used for save/replay/cache identity. */
    uint32_t candidate_hash;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet record_pools;
    /* c_dballoc capacities retained by GAME_LOAD for pools that are empty in
     * the static dungeon but allocated by later source effects (DB14/DB15). */
    uint16_t record_capacities[DM2_V1_RECORD_POOL_COUNT];
    DM2_V1_Party party;
    int16_t leader_hand_record;
    DM2_V1_GameLoadMovementState movement;
    DM2_V1_GameLoadMoverecState moverec;
    /* c_eventqueue::init followed by the real first mirror/leader selection.
     * This remains private and receives no host input before a complete
     * session owner exists. */
    DM2_V1_EventQueue event_queue;
    int16_t source_event_hero_index;
    /* Private source owner for ddat.v1e0288.  PROCESS_POISON excludes the
     * hero whose one-based index equals this value. */
    int16_t source_next_champion_number;
    /* Deep copy of the source's pre-champion DM2_1031_0541(5) state. It
     * owns copies of the mutable UI tables and remains unpublished until the
     * following LOAD_NEWMAP/CAII transaction is source-complete. */
    DM2_V1_InitGameUiOwner init_game_ui;
    /* c_loadlevel.cpp::DM2_LOAD_LOCALLEVEL_GRAPHICS_TABLE receives these
     * direct File_header lists immediately after selecting the current map.
     * This copy remains private until the complete local-level transaction
     * owns DYN4 and the post-load consumers. */
    DM2_V1_GameLoadLocalLevelGraphicsReceipt local_level_graphics;
    DM2_V1_GameLoadLocalDynPrelude local_dyn_prelude;
    DM2_V1_GameLoadLocalDynMapScan local_dyn_map_scan;
    DM2_V1_GameLoadLocalDynRecordEffects local_dyn_record_effects;
    DM2_V1_TimerEntry *timer_entries;
    int16_t *timer_indices;
    DM2_V1_TimerQueue timer_queue;
    uint16_t timer_capacity;
    /* ddat.savegames1.b_02 and v1e0976, retained only for the private
     * source-ordered 0x47 timer owner. */
    uint8_t source_hero_ench_countdown;
    int16_t source_hero_ench_target;
    /* Exact c_wbbb/ddat.savegames1 source block. Fresh GAME_LOAD starts
     * zeroed; SKSAVE Resume must transfer the authenticated six bytes before
     * this candidate can become playable. */
    uint8_t source_savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];
    int source_savegames1_valid;
    /* Exact s_savegamebuffer fields restored by DM2_GAME_LOAD before the
     * saved timer heap is sorted.  These are deliberately separate from the
     * host presentation weather state: they are the resumable c_random /
     * c_weather owner and are populated only by an authenticated SKSAVE. */
    uint32_t source_game_tick;
    uint32_t source_random_seed;
    DM2_V1_UpdateWeatherState source_weather_chain;
    int source_weather_chain_valid;
    int16_t source_light_level;
    DM2_V1_CaiiArray caii_slots;
    /* Deep clone of the exact AI-definition table that admitted this
     * candidate.  It is deliberately separate from the mutable DB4 pool:
     * the pool is cloned from the post-RESET/FILL owner while these rows are
     * immutable GDAT/executable source data. */
    DM2_V1_CaiiSourceOwner caii_source;
    DM2_V1_DropRng caii_rng;
    int caii_rng_initialized;
    /* Borrowed immutable, hash-admitted GDAT. The candidate never writes it;
     * it is required by c_querydb's real dtRaw7/0xfd creature-position path. */
    const DM2_V1_AssetLoader *asset_loader;
    DM2_V1_GameLoadSoundOwner sound_owner;
    /* Private immutable descriptor backing the cloned SKSAVE c_map view. */
    DM2_V1_OriginalRawDungeonReceipt sksave_dungeon_receipt;
    /* SKSAVE Resume retains the mutable c_map ground-stack owner separately
     * from the authenticated DUNGEON.DAT descriptor. LOAD_LOCALLEVEL_DYN
     * must walk this owner after READ_SKSAVE_DUNGEON; using the pristine
     * File_header roots would bind records from a different transaction. */
    DM2_V1_SksaveMapOwner sksave_map_owner;
    int sksave_map_owner_valid;
    /* The candidate owns c_map's selected descriptor/tile/column view plus
     * the global first-thing record-root table, separately from party
     * coordinates.  It is initialized through the
     * forced v1d3248=-1 path in DM2_move_2fcf_0b8b and may be changed only by
     * the private CHANGE_CURRENT_MAP_TO adapter below. */
    DM2_V1_SkprojectChangeCurrentMapReceipt map_context;
    int16_t source_display_map;
    int16_t source_runtime_display_x;
    int16_t source_runtime_display_y;
    uint8_t source_runtime_display_direction;
    int source_runtime_display_uses_alternate;
    int current_map;
    int source_party_map;
    uint8_t source_party_x;
    uint8_t source_party_y;
    uint8_t source_party_direction;
    uint8_t source_party_absdir;
    int source_staircase_flag;
    int16_t source_teleporter_map;
    int16_t source_display_x;
    int16_t source_display_y;
    int8_t source_teleporter_probe_direction;
    uint8_t source_teleporter_source_direction;
    uint8_t source_teleporter_destination_direction;
    int source_display_pose_valid;
} DM2_V1_GameLoadRuntimeSessionCandidate;

/* Private, source-complete c_querydb spatial scan over a cloned GAME_LOAD
 * candidate. It resolves map-chain DB4 records, the cloned CAII position
 * cursor and hash-admitted GDAT dtRaw7/0xfd. Any cursor mutation remains in
 * the candidate; the prepared source world, M11 and live runtime are never
 * touched. Source: SKULLWIN/c_querydb.cpp::DM2_query_1c9a_03cf (3769-3844),
 * ::DM2_QUERY_CREATURE_5x5_POS (3012-3034). */
typedef struct {
    int valid;
    int blocked_invalid_candidate;
    int blocked_out_of_bounds;
    int blocked_missing_owner;
    int16_t input_x;
    int16_t input_y;
    uint16_t direction;
    uint32_t result_handle;
    DM2_V1_SkprojectQuery1c9a03cfReceipt source;
} DM2_V1_GameLoadSpatialQueryReceipt;

/* Read-only source classification for the first branch of
 * c_move.cpp::DM2_PERFORM_MOVE.  The six result values deliberately retain
 * DM2_12b4_0881's one-based return convention: 1 stair-back, 2 stair,
 * 3 blocked tile, 4 direct creature, 5 creature path, 6 clear path.  It is
 * only a classifier: it does not adjust stamina, move the party sentinel,
 * dispatch c_moverec, advance timers or publish M11 state. */
typedef struct {
    int valid;
    int blocked_invalid_candidate;
    int blocked_out_of_bounds;
    int blocked_missing_owner;
    uint8_t move_command;
    int16_t source_x;
    int16_t source_y;
    int16_t target_x;
    int16_t target_y;
    uint8_t source_tile;
    uint8_t target_tile;
    int16_t direct_creature;
    int16_t spatial_creature;
    uint8_t direct_creature_offset;
    uint8_t classification;
    DM2_V1_SkprojectTileBlockedReceipt tile_blocked;
    DM2_V1_GameLoadSpatialQueryReceipt spatial;
} DM2_V1_GameLoadMoveClassificationReceipt;

/* Read-only source-chain census for one c_moverec square.  It follows the
 * cloned RecordPoolSet rather than rereading File_header record bytes, so
 * post-GAME_LOAD mutations remain visible to the next transaction.  This is
 * deliberately only the c_moverec input boundary: dispatching DB0..DB15,
 * cutting/appending records and emitting timers require the same rollback
 * owner and are not implied by a valid census.
 *
 * Source: SKULLWIN/c_moverec.cpp::DM2_moverec_3CE7D (1147-1430),
 *         c_record.cpp::DM2_GET_NEXT_RECORD_LINK (53-57). */
typedef struct {
    int valid;
    int blocked_invalid_candidate;
    int blocked_out_of_bounds;
    int blocked_missing_record;
    int blocked_cycle;
    int16_t map;
    int16_t x;
    int16_t y;
    uint8_t tile;
    int16_t first_record;
    uint16_t record_count;
    uint16_t record_type_count[DM2_V1_RECORD_POOL_COUNT];
    uint32_t chain_hash;
} DM2_V1_GameLoadMoverecSquareReceipt;

/* Private tile-rooted DM2_MOVE_RECORD_TO result.  This is the bounded
 * cut-then-append branch for authenticated source/destination squares,
 * including the mirror-verified plain-floor cross-map DB4 form.  The
 * candidate timer owner separately admits a no-chain same-tile party-
 * sentinel case; creature-level, wake/sleep and actuator tails remain gated. */
typedef struct {
    int valid;
    int mutated;
    int rolled_back;
    int blocked_invalid_candidate;
    int blocked_map_or_square;
    int blocked_source_chain;
    int blocked_destination_chain;
    int map;
    int16_t record;
    int16_t source_x;
    int16_t source_y;
    int16_t destination_x;
    int16_t destination_y;
    uint16_t source_head_before;
    uint16_t source_head_after;
    uint16_t destination_head_before;
    uint16_t destination_head_after;
    uint32_t transaction_hash;
} DM2_V1_GameLoadRecordMoveReceipt;

/* Read-only post-move audit for DM2_moverec_3CE7D.  The generic dispatcher
 * remains callback-owned; this receipt admits only the source call shape and
 * never publishes a timer, actuator or CAII mutation. */
typedef struct {
    int valid;
    int dispatched;
    int blocked_invalid_candidate;
    int blocked_record;
    int map;
    int16_t record;
    int16_t x;
    int16_t y;
    int32_t kind;
    int32_t flags;
    uint8_t db_type;
    uint32_t transaction_hash;
} DM2_V1_GameLoadMoverecDispatchReceipt;

/* Source c_moverec.cpp:966-978: update an already-owned creature think
 * timer in place when a record reaches a new square.  The candidate uses its
 * cloned c_tim slot heap; no new CAII slot or timer is fabricated here. */
typedef struct {
    int valid;
    int timer_updated;
    int blocked_invalid_candidate;
    int blocked_record;
    int blocked_no_slot;
    int blocked_no_pending_timer;
    int blocked_unbound_allocation;
    int blocked_ai_flags;
    int ai_flags_known;
    int ai_bit0_clear;
    int allocation_performed;
    int timer_scheduled;
    uint8_t timer_type;
    uint32_t due_tick;
    int map;
    int16_t record;
    int16_t x;
    int16_t y;
    int16_t caii_slot;
    int16_t timer_slot;
    uint32_t transaction_hash;
} DM2_V1_GameLoadMoverecCaiiReceipt;

/* Candidate-side adapter for the c_creature CAII timer word.  The candidate
 * stores a fixed c_tim slot index at CAII slot+2 (the source session stores a
 * stable ticket), so cancellation must use the candidate heap owner and may
 * not call the SourceTimerQueue compatibility helper.  This operation only
 * detaches the pending timer; record/CAII ownership remains untouched until
 * the complete DELETE_CREATURE_RECORD tail is bound. */
typedef struct {
    int valid;
    int detached;
    int blocked_invalid_candidate;
    int blocked_record;
    int blocked_caii;
    int blocked_timer;
    int16_t record;
    int16_t caii_slot;
    int16_t timer_slot;
    uint8_t timer_type;
    uint32_t transaction_hash;
} DM2_V1_GameLoadCandidateCaiiTimerDetachReceipt;

/* Private source-shaped c_tim 0x3C/0x3D owner.  This admits an existing DB4
 * creature with an already-owned CAII think timer, a mirror-verified
 * plain-floor cross-map form, and a same-map plain-floor party-sentinel
 * no-chain case.  The source MOVE_RECORD_TO cut/append, CAII timer relocation
 * and GEN1 noise are one candidate transaction; creature-level, wake/sleep,
 * static allocation and post-move actuator/collision routes remain outside
 * this owner. */
typedef struct {
    int valid;
    int consumed;
    int moved;
    int noise_queued;
    int noise_map_gate_noop;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_source_chain;
    int blocked_destination_chain;
    int blocked_caii;
    int blocked_sound;
    int map;
    int source_map;
    int16_t record;
    int16_t source_x;
    int16_t source_y;
    int16_t x;
    int16_t y;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadMoverecTimerReceipt;

/* Bounded c_tim dispatch for source THINK_CREATURE (0x21/0x22).  The timer
 * is consumed in source order and the per-cell creature lookup is performed;
 * the CCM/AI body remains explicitly unbound. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int body_unbound;
    int no_creature_at_cell;
    int map_switch;
    int map;
    int16_t x;
    int16_t y;
    int16_t creature_record;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadThinkTimerReceipt;

/* Bounded c_tim 0x0E PROCESS_TIMER_0E dispatch.  The source temporarily
 * changes the authenticated root item's type, applies PROCESS_ITEM_BONUS to
 * the source hero, and restores the item record before consuming the timer. */
typedef struct {
    int valid;
    int consumed;
    int bonus_applied;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_actor;
    int blocked_record;
    int blocked_item_bonus;
    int map_switch;
    int map;
    uint8_t timer_type;
    uint8_t actor;
    uint16_t record_db_type;
    uint16_t record;
    uint16_t value2;
    int16_t bonus_value;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadProcess0eTimerReceipt;

/* Bounded c_tim 0x48 ENCH_POWER dispatch.  The actor byte is the source
 * hero-mask and wvalueA (timer bytes 6..7) is the signed decrement. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_party;
    int blocked_map;
    int map;
    int map_switch;
    uint8_t timer_type;
    uint8_t actor_mask;
    int16_t amount;
    int heroes_seen;
    int heroes_mutated;
    int heroes_skipped_dead;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadEnchPowerTimerReceipt;

/* Bounded c_tim 0x4B POISON dispatch.  The timer's signed wvalueA is first
 * removed from c_hero::poison; PROCESS_POISON then applies one pending wound
 * and requeues the remaining poison counters at source delay 0x24. */
typedef struct {
    int valid;
    int consumed;
    int requeued;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_party;
    int blocked_map;
    int blocked_actor;
    int blocked_last_hero_owner;
    uint8_t timer_type;
    int map;
    int map_switch;
    int16_t actor;
    int16_t amount;
    int16_t poison_before;
    int16_t poison_after;
    int16_t poisoned_before;
    int16_t poisoned_after;
    int16_t wound_amount;
    int16_t cur_hp_before;
    int16_t cur_hp_after;
    uint16_t hero_flags_before;
    uint16_t hero_flags_after;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadPoisonTimerReceipt;

/* Bounded c_tim 0x47 HERO_ENCH_FLAG dispatch.  The source-owned countdown
 * is private session state; the hero target is the 1-based v1e0976 slot. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_party;
    int blocked_map;
    int countdown_decremented;
    int countdown_expired;
    int hero_flag_set;
    int hero_skipped_dead;
    int map;
    int map_switch;
    int16_t target_slot;
    uint8_t countdown_before;
    uint8_t countdown_after;
    uint16_t hero_flags_before;
    uint16_t hero_flags_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadHeroEnchFlagTimerReceipt;

/* Bounded c_tim 0x46 LIGHT dispatch. */
typedef struct {
    int valid;
    int consumed;
    int requeued;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_value;
    int map;
    int map_switch;
    int16_t amount;
    int16_t light_before;
    int16_t light_after;
    int16_t remaining;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadLightTimerReceipt;

/* Source-bound c_tim 0x5A ORNATE_NOISE.  The inactive arm clears only the
 * frame high byte; the active arm resolves decoration/GDAT and requeues the
 * timer while treating GEN2 sound as the source's best-effort side effect. */
typedef struct {
    int valid;
    int consumed;
    int requeued;
    int sound_queued;
    int frame_cleared;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_active_ornament;
    int map;
    int map_switch;
    int16_t record;
    uint16_t animation_length;
    uint8_t decoration;
    uint8_t category;
    uint16_t word2_before;
    uint16_t word2_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadOrnateNoiseTimerReceipt;

/* Source-bound c_tim 0x55 CONTINUE_ORNATE_ANIMATOR.  getA()/xA+yA carry the
 * DB3 actuator handle, wvalueB/getBlong() carries the animation mode, and
 * the original timer is requeued at +1 tick after the frame step. */
typedef struct {
    int valid;
    int consumed;
    int requeued;
    int frame_advanced;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_asset;
    int map;
    int16_t record;
    uint16_t word2_before;
    uint16_t word2_after;
    uint16_t frame_before;
    uint16_t frame_after;
    uint16_t animation_length;
    uint8_t animation_mode;
    uint8_t decoration;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadOrnateAnimatorTimerReceipt;

/* Bounded c_tim 0x02 DESTROY_DOOR dispatch. */
typedef struct {
    int valid;
    int consumed;
    int tile_mutated;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int map;
    int map_switch;
    int16_t x;
    int16_t y;
    uint16_t tile_before;
    uint16_t tile_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadDestroyDoorTimerReceipt;

/* Bounded c_tim 0x0C PROCESS_TIMER_0C dispatch. */
typedef struct {
    int valid;
    int consumed;
    int hero_mutated;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_actor;
    int map;
    int map_switch;
    int16_t hero_index;
    int16_t timer_index_before;
    int16_t timer_index_after;
    uint16_t hero_flags_before;
    uint16_t hero_flags_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadProcess0cTimerReceipt;

/* Bounded final phase of c_tim 0x0D RESURRECTION (yB == 0). */
typedef struct {
    int valid;
    int consumed;
    int champion_revived;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_actor;
    int blocked_phase;
    int blocked_chain;
    int blocked_cloud_owner;
    int blocked_cloud_actuator;
    int cloud_created;
    int cloud_timer_queued;
    int cloud_sound_queued;
    int record_removed;
    int map;
    int map_switch;
    int16_t hero_index;
    int16_t altar_record;
    int16_t cloud_record;
    uint8_t phase;
    int16_t max_hp_before;
    int16_t max_hp_after;
    int16_t cur_hp_after;
    uint16_t hero_flags_before;
    uint16_t hero_flags_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadResurrectionTimerReceipt;

/* Bounded c_tim 0x19 PROCESS_CLOUD arm.  The source's type-0x64 step
 * advances the cloud to 0x65, emits GEN2 noise and requeues one tick later.
 * Ordinary party, direct DB0-door damage and candidate-context creature
 * damage are owned against the candidate's source-sized state; spread and
 * actuator tails remain outside this owner. */
typedef struct {
    int valid;
    int consumed;
    int requeued;
    int deallocated;
    int sound_queued;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int map;
    int16_t cloud_record;
    int16_t party_damage;
    int party_wounded_mask;
    int16_t creature_damage;
    int creature_attacked;
    int16_t door_damage;
    int door_destroyed;
    uint8_t cloud_type_before;
    uint8_t cloud_type_after;
    /* DB15 word@2: low 7 bits are the subtype; the high byte is the
     * source cloud strength/lifetime parameter. */
    uint8_t cloud_strength_before;
    uint8_t cloud_strength_after;
    uint8_t timer_type;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadCloudTimerReceipt;

/* Private c_tim 0x04 actuator boundary.  Source class 3 has an intentional
 * empty branch in DM2_PROCEED_TIMERS; consuming that message needs no wall,
 * floor, party or CAII owner.  All other tile classes remain fail-closed. */
typedef struct {
    int valid;
    int consumed;
    int source_noop;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_unowned_class;
    int map_switch;
    int map;
    int16_t x;
    int16_t y;
    uint8_t timer_type;
    uint8_t tile_class;
    uint8_t raw_tile;
    uint8_t direction;
    uint8_t action;
    int push_button_actuators_seen;
    int push_button_doors_mutated;
    uint32_t private_push_button_hash;
    int counter_actuators_seen;
    int counter_records_mutated;
    int counter_messages_queued;
    uint32_t private_counter_hash;
    int relay_actuators_seen;
    int relay_messages_queued;
    uint32_t private_relay_hash;
    int cross_map_actuators_seen;
    int cross_map_messages_queued;
    uint32_t private_cross_map_hash;
    int finite_relay_actuators_seen;
    int finite_relay_records_mutated;
    int finite_relay_messages_queued;
    uint32_t private_finite_relay_hash;
    int text_records_seen;
    int text_records_toggled;
    int blocked_non_text_chain;
    int blocked_hint_delivery;
    int blocked_unowned_tile_advance;
    int pit_tele_tile_mutated;
    uint8_t tile_state_before;
    uint8_t tile_state_after;
    uint32_t private_text_visibility_hash;
    int door_actuator_timer_queued;
    int door_actuator_source_noop_destroyed;
    int16_t door_actuator_record;
    uint8_t door_actuator_direction;
    uint16_t door_record_attributes_before;
    uint16_t door_record_attributes_after;
    int door_record_state_mutated;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadCandidateActuateReceipt;

/* Private source 0x1E STEP_MISSILE atom.  This owner covers the authenticated
 * creature-free ordinary passage and terminal energy/dealloc branch. Impact,
 * reflection, moverec and cross-map teleporter effects remain separate
 * source owners and therefore fail closed here. */
typedef struct {
    int valid;
    int consumed;
    int deallocated;
    int moved;
    int requeued;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_timer_owner;
    int blocked_incomplete_chain;
    int blocked_creature_collision;
    int blocked_teleporter;
    int blocked_unowned_impact;
    int map_switch;
    int map;
    int16_t x;
    int16_t y;
    int16_t next_x;
    int16_t next_y;
    int16_t missile_record;
    uint8_t timer_type;
    uint8_t energy_before;
    uint8_t energy_after;
    uint16_t energy_step;
    uint8_t direction;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadCandidateMissileReceipt;

/* Private source 0x01 DM2_STEP_DOOR atom.  It admits only a direct DB0 door
 * whose tile chain has no party or creature collision; the next animation
 * step stays in the candidate timer heap. */
typedef struct {
    int valid;
    int consumed;
    int source_noop_destroyed;
    int door_state_mutated;
    int requeued;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_party_collision;
    int blocked_creature_collision;
    int blocked_incomplete_chain;
    int map_switch;
    int map;
    int16_t x;
    int16_t y;
    int16_t door_record_link;
    uint8_t timer_type;
    uint8_t direction;
    uint8_t state_before;
    uint8_t state_after;
    uint16_t door_record_attributes_before;
    uint16_t door_record_attributes_after;
    int door_record_state_mutated;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadCandidateDoorStepReceipt;

typedef struct {
    int valid;
    int actuator_invoked;
    int message_queued;
    int requeued;
    int active_flag_cleared;
    int16_t record_link;
    uint8_t action;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t target_direction;
} DM2_V1_GameLoadCandidateTickGeneratorReceipt;

/* Source timer 0x58/0x59/0x5B/0x5C record-flag tails.  Payload A is the
 * record handle except for 0x59, which uses payload B; the timer is consumed
 * only after the private record address is admitted. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int guard_skipped;
    int redraw_unbound;
    int map_switch;
    int map;
    int16_t record;
    uint8_t timer_type;
    uint8_t byte_offset;
    uint8_t value_before;
    uint8_t value_after;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadRecordFlagTimerReceipt;

/* Private PROCESS_SOUND (0x15) owner. The timer A payload selects one of
 * c_sfx's eight delayed slots; the slot is cleared after source-shaped GEN1
 * replay, including the source map-gate no-op. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_slot;
    int blocked_sound;
    int map_gate_noop;
    int sound_queued;
    uint8_t timer_type;
    uint8_t slot;
    int map;
    int16_t x;
    int16_t y;
    int16_t ecxw;
    int16_t volume;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadProcessSoundTimerReceipt;

/* Private ALLOC_NEW_CREATURE (0x5E) owner. This receipt is only valid after
 * the new DB4 root, tile chain, CAII slot and first think timer share one
 * transaction. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    int blocked_map;
    int blocked_record;
    int blocked_tile;
    int blocked_ai;
    int blocked_caii;
    int blocked_sound;
    uint16_t recycler_candidates_examined;
    int recycler_candidate_found;
    int16_t recycler_candidate_record;
    int16_t recycler_candidate_map;
    int16_t recycler_candidate_x;
    int16_t recycler_candidate_y;
    int16_t recycler_candidate_caii_slot;
    int recycler_candidate_caii_match;
    uint16_t recycler_candidate_timer_slot;
    int recycler_candidate_pending_timer;
    int16_t recycler_candidate_possession_root;
    uint16_t recycler_candidate_ai_flags;
    int recycler_candidate_ai_flags_known;
    uint16_t recycler_candidate_gdat_word1;
    int recycler_candidate_gdat_word1_known;
    int recycler_candidate_drop_slots_loaded;
    int recycler_candidate_delete_inputs_ready;
    int sound_queued;
    uint8_t sound_index;
    uint8_t timer_type;
    uint8_t creature_type;
    uint8_t direction;
    int16_t record;
    int16_t x;
    int16_t y;
    uint16_t health_multiplier;
    uint16_t hit_points;
    int16_t caii_slot;
    int16_t think_timer_slot;
    uint32_t game_tick;
    uint32_t transaction_hash;
} DM2_V1_GameLoadAllocCreatureTimerReceipt;

/* Source-ordered private candidate timer boundary.  The dispatcher peeks the
 * due heap head and delegates only to timer families whose complete private
 * adapter already exists. */
typedef struct {
    int valid;
    int consumed;
    int blocked_invalid_candidate;
    int blocked_no_due_timer;
    int blocked_unsupported_type;
    uint8_t timer_type;
    uint32_t game_tick;
    DM2_V1_GameLoadThinkTimerReceipt think;
    DM2_V1_GameLoadProcess0eTimerReceipt process_0e;
    DM2_V1_GameLoadProcessSoundTimerReceipt process_sound;
    DM2_V1_GameLoadAllocCreatureTimerReceipt alloc_creature;
    DM2_V1_GameLoadOrnateAnimatorTimerReceipt ornate_animator;
    DM2_V1_GameLoadLightTimerReceipt light;
    DM2_V1_GameLoadOrnateNoiseTimerReceipt ornate_noise;
    DM2_V1_GameLoadDestroyDoorTimerReceipt destroy_door;
    DM2_V1_GameLoadProcess0cTimerReceipt process_0c;
    DM2_V1_GameLoadResurrectionTimerReceipt resurrection;
    DM2_V1_GameLoadCloudTimerReceipt cloud;
    DM2_V1_GameLoadHeroEnchFlagTimerReceipt hero_ench_flag;
    DM2_V1_GameLoadEnchPowerTimerReceipt ench_power;
    DM2_V1_GameLoadPoisonTimerReceipt poison;
    DM2_V1_GameLoadCandidateTickGeneratorReceipt tick_generator;
    DM2_V1_GameLoadCandidateActuateReceipt actuate;
    DM2_V1_GameLoadCandidateMissileReceipt missile;
    DM2_V1_GameLoadCandidateDoorStepReceipt door_step;
    DM2_V1_GameLoadRecordFlagTimerReceipt record_flag;
    DM2_V1_GameLoadMoverecTimerReceipt moverec;
} DM2_V1_GameLoadCandidateTimerProcessReceipt;

/* Clone every already-owned mutable GAME_LOAD predecessor atomically, but
 * only after the complete CAII transaction has been materialized. An already
 * initialized `out` is replaced only after the new clone succeeds;
 * arbitrary/unknown storage is zeroed without dereferencing it. On failure a
 * prior initialized candidate is unchanged, while unknown storage is zeroed;
 * the source owner and source media are unchanged. The candidate is
 * deliberately not publishable. */
int dm2_v1_game_load_runtime_session_candidate_init(
    DM2_V1_GameLoadRuntimeSessionCandidate *out,
    const DM2_V1_GameLoadWorldOwner *source);
struct DM2_V1_SksaveGameLoadOwner;
int dm2_v1_game_load_runtime_session_candidate_init_from_sksave(
    DM2_V1_GameLoadRuntimeSessionCandidate *out,
    const struct DM2_V1_SksaveGameLoadOwner *source);
void dm2_v1_game_load_runtime_session_candidate_free(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate);
int dm2_v1_game_load_runtime_session_candidate_is_valid(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate);

/* Private c_map.cpp::DM2_CHANGE_CURRENT_MAP_TO transition over the cloned
 * File_header world.  This only retargets the candidate's map descriptor,
 * tile/column view and global first-thing table reference plus source display
 * pose; it never moves a party,
 * dispatches timers or exposes M11 state. */
int dm2_v1_game_load_runtime_session_candidate_change_current_map_to(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, int new_map);

int dm2_v1_game_load_runtime_session_candidate_query_nearest_creature(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t *io_x, int16_t *io_y, uint16_t direction,
    uint32_t *out_handle, DM2_V1_GameLoadSpatialQueryReceipt *out_receipt);

/* Private equivalent of DM2_12b4_0881 over the cloned GAME_LOAD candidate.
 * `move_command` is the source `vw_28` value, so only literal value 2 takes
 * the source-tile-class-3 stair-back branch. Both coordinates are local to
 * candidate->current_map. */
int dm2_v1_game_load_runtime_session_candidate_classify_move(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate, uint8_t move_command,
    int16_t source_x, int16_t source_y, int16_t target_x, int16_t target_y,
    DM2_V1_GameLoadMoveClassificationReceipt *out_receipt);

/* Census the current-map source/destination square before a future
 * DM2_MOVE_RECORD_TO call.  This function is strictly read-only and never
 * promotes a successful result to movement or input availability. */
int dm2_v1_game_load_runtime_session_candidate_census_moverec_square(
    const DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t x, int16_t y, DM2_V1_GameLoadMoverecSquareReceipt *out_receipt);

/* Execute only the source tile-rooted cut/append branch of
 * DM2_MOVE_RECORD_TO. The candidate and its mutable File_header image are
 * restored byte-for-byte if either half fails. */
int dm2_v1_game_load_runtime_session_candidate_move_record_to(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t source_x, int16_t source_y,
    int16_t destination_x, int16_t destination_y,
    DM2_V1_GameLoadRecordMoveReceipt *out_receipt);

/* Audit the source post-move dispatcher over a retained record.  This is a
 * private no-side-effect boundary until the candidate owns the timer/CAII
 * transaction required by c_moverec.cpp:960-985. */
int dm2_v1_game_load_runtime_session_candidate_dispatch_moverec(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    DM2_V1_GameLoadMoverecDispatchReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_activate_moverec_caii(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record, int16_t x, int16_t y,
    DM2_V1_GameLoadMoverecCaiiReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_detach_caii_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    int16_t record,
    DM2_V1_GameLoadCandidateCaiiTimerDetachReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_moverec_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadMoverecTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_think_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadThinkTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_process_0e_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadProcess0eTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_process_sound_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadProcessSoundTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_alloc_creature_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadAllocCreatureTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_ench_power_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadEnchPowerTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_poison_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadPoisonTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_hero_ench_flag_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadHeroEnchFlagTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_light_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadLightTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_ornate_noise_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadOrnateNoiseTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_ornate_animator_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadOrnateAnimatorTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_destroy_door_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadDestroyDoorTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_process_0c_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadProcess0cTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_resurrection_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadResurrectionTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_cloud_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCloudTimerReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_actuate_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateActuateReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_missile_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateMissileReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_door_step_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateDoorStepReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_tick_generator_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTickGeneratorReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_process_next_due_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick,
    DM2_V1_GameLoadCandidateTimerProcessReceipt *out_receipt);
int dm2_v1_game_load_runtime_session_candidate_proceed_record_flag_timer(
    DM2_V1_GameLoadRuntimeSessionCandidate *candidate,
    uint32_t game_tick, DM2_V1_GameLoadRecordFlagTimerReceipt *out_receipt);

/* Build the source-owned GAME_LOAD predecessor without selecting a champion.
 * It replaces an already initialized owner only after the new owner is
 * complete; arbitrary caller storage is instead safely zeroed. It clones only
 * the authenticated File_header, DB pools, DYN4 and timer capacity. Call the
 * generator and map-context functions in source order before delivering any
 * mirror click. */
int dm2_v1_game_load_world_owner_prepare_new_game(
    DM2_V1_GameLoadWorldOwner *owner, const DM2_V1_BootProfile *profile);

/* Add an already source-resolved mirror click to a prepared owner.  The
 * function rebuilds the whole click-ordered receipt from the original data
 * and publishes nothing to M11 or the runtime. */
int dm2_v1_game_load_world_owner_select_champion(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootNewGamePartySelection *selection);

/* Retain the source c_light inputs for the established fresh-game map before
 * any mirror click.  This has no renderer/session side effect. */
int dm2_v1_game_load_world_owner_materialize_preselection_light(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain c_light's source-sized, zeroed visibility buffers before its
 * FIND_WALK_PATH calls. This does not make visibility, positional sound or
 * a viewport live. */
int dm2_v1_game_load_world_owner_materialize_preselection_light_visibility(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain c_sfx's source map/origin facts after move_2fcf_0b8b has established
 * the private display map. No noise is queued by this function. */
int dm2_v1_game_load_world_owner_materialize_preselection_sound_spatial(
    DM2_V1_GameLoadWorldOwner *owner);

/* Copy the current map's authentic LOAD_LOCALLEVEL graphics lists. This is
 * intentionally before GDAT resource consumption and has no renderer/UI
 * side effect. */
int dm2_v1_game_load_world_owner_materialize_preselection_local_graphics(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain the current File_header map's direct DB0 Door payloads before the
 * entrance projection consumes a visible door.  Source: SKWIN/DME.h::Door;
 * c_map.cpp::GET_ADDRESS_OF_TILE_RECORD. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_doors(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain the current File_header map's DB5..DB15 object record addresses.
 * The result is not an item inventory or a rendered sprite list. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_objects(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain current-map DB2 Text fields for later source-owned UI/sensor
 * consumers.  This does not decode a host string or change visibility. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_texts(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain current-map direct DB1 Teleporter fields for the later c_moverec
 * transition owner. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_teleporters(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain current-map direct DB3 Actuator payloads for the later source timer
 * and sensor owner. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_actuators(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain current-map DB4 creature placement, HP and possession-root fields
 * before any CAII/timer/runtime consumer is published. */
int dm2_v1_game_load_world_owner_materialize_preselection_map_creatures(
    DM2_V1_GameLoadWorldOwner *owner);

/* Retain each current-map Creature::possession chain (or its authentic null
 * root) without moving, equipping or dropping any record. */
int dm2_v1_game_load_world_owner_materialize_preselection_creature_possessions(
    DM2_V1_GameLoadWorldOwner *owner);

/* Decode the real entrance floor/ceiling material and build its c_light
 * result only when the preceding private light inputs cover the source
 * branch. It deliberately has no renderer or M11 publication. */
int dm2_v1_game_load_world_owner_materialize_preselection_scene(
    DM2_V1_GameLoadWorldOwner *owner);

/* Materialize only the real source cells visible from the entry pose.  This
 * has no framebuffer, input, HUD or runtime-session side effect. */
int dm2_v1_game_load_world_owner_materialize_preselection_view(
    DM2_V1_GameLoadWorldOwner *owner);

int dm2_v1_game_load_world_owner_materialize_preselection_viewport(
    DM2_V1_GameLoadWorldOwner *owner);

/* Apply source input event 1 (left turn) or 2 (right turn) before the first
 * mirror selection.  It rotates only the private empty-party owner, then
 * recomputes the source map's teleporter/display context and view receipts.
 * It never publishes M11 input, a tick, framebuffer or playable session.
 * Source: SKProject uiinput.cpp events 1/2 and
 * skhero.cpp::DM2_PERFORM_TURN_SQUAD. */
int dm2_v1_game_load_world_owner_turn_preselection(
    DM2_V1_GameLoadWorldOwner *owner, int source_event);

/* Advance the private, empty source party one square before the first mirror
 * selection.  This is only DM2_PERFORM_MOVE's complete no-record floor
 * branch: both departure and destination must be real G1 floors with no
 * ground-stack flag, record chain or direct teleporter.  Doors, pits,
 * creatures, records and map transitions remain blocked until their full
 * mutable session owners are present. */
int dm2_v1_game_load_world_owner_advance_preselection(
    DM2_V1_GameLoadWorldOwner *owner);

/* Execute only the original source movement events 3..6 against the private
 * empty-party owner (forward, right, back, left).  It retains exactly the
 * same no-record-floor restriction as advance_preselection; every other
 * c_move branch remains unavailable until a complete session owner exists.
 * Source: SKProject uiinput.cpp::DM2_HANDLE_UI_EVENT events 3..6 and
 * c_move.cpp::DM2_PERFORM_MOVE. */
int dm2_v1_game_load_world_owner_move_preselection(
    DM2_V1_GameLoadWorldOwner *owner, int source_event);

/* Apply only RESET_CAII's deterministic static-AI branch to the private
 * File_header DB4 pool.  The operation resets every Creature byte@5, then
 * reproduces FILL_CAII_CUR_MAP's 09db word@0xA merge for source-static
 * creatures in x/y/map order.  It is transactional and never assigns a
 * dynamic CAII slot, queues a timer, consumes RNG or publishes a session. */
int dm2_v1_game_load_world_owner_materialize_static_caii(
    DM2_V1_GameLoadWorldOwner *owner);

/* Exact private c_tim producer used by DM2_ALLOC_CAII_TO_CREATURE after the
 * source slot has been initialized. This does not allocate a slot or run
 * 0a48; it is intentionally available so the later all-map transaction can
 * use the same dynamic GAME_LOAD heap. */
int dm2_v1_game_load_world_owner_schedule_caii_think(
    DM2_V1_GameLoadWorldOwner *owner, int16_t record_handle, int map,
    int x, int y, DM2_V1_GameLoadCaiiThinkReceipt *out_receipt);

/* Materialize RESET_CAII/FILL_ORPHAN_CAII's dynamic DB4 branch privately.
 * It allocates source-shaped c_creature slots, queues each 0cf7 think timer,
 * executes 0a48 with the real GDAT/AI/RNG inputs, and carries a noise only
 * through the admitted dynamic SOUND9 owner. Any unresolved local state,
 * class triple, sample binding or occlusion restores all DB4, CAII, timer,
 * RNG and SFX state before returning zero. No session is published. */
int dm2_v1_game_load_world_owner_materialize_dynamic_caii(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadCaiiDynamicReceipt *out_receipt);

/* Preserve every real dynamic candidate's local-creature identity and mark
 * its 0a48 noise dependency pending. No QUEUE_NOISE_GEN1 request exists
 * until the real animation row supplies its index; slot allocation, CCM and
 * sound queue insertion remain unavailable until they share one rollback
 * transaction. */
int dm2_v1_game_load_world_owner_materialize_caii_local_context(
    DM2_V1_GameLoadWorldOwner *owner);

typedef struct {
    int valid;
    uint16_t candidate_count;
    uint16_t control_bit2_clear_count;
    uint16_t activation_count;
    uint16_t queued_timer_count;
    uint32_t receipt_hash;
} DM2_V1_GameLoadActuatorGeneratorReceipt;

typedef struct {
    int valid;
    int actuator_invoked;
    int message_queued;
    int requeued;
    int active_flag_cleared;
    int16_t record_link;
    uint8_t action;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t target_direction;
} DM2_V1_GameLoadTickGeneratorReceipt;

/* Result of consuming a private 0x04 message produced by the tick-generator
 * continuation above.  Tile class 3 is the only complete source case that
 * needs no further owner: c_tim_proc.cpp falls through without a handler.
 * PIT/TELEPORTER may enter the owned DB2-only FLOOR subset only for a close
 * transition. Opening invokes DM2_ADVANCE_TILES_TIME, whose party/creature
 * moves remain outside this owner. Every other mutating class remains blocked
 * until its complete source chain and follow-up timers share one owner. */
typedef struct {
    int valid;
    int source_noop;
    int blocked_incomplete_chain;
    int map;
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    uint8_t action;
    uint8_t tile_class;
    uint8_t raw_tile;
    /* Only the DB2-only FLOOR atom below can commit.  The count and hash
     * describe mutable bytes in this private owner, never a hint payload or
     * a live HUD side effect. */
    int text_records_seen;
    int text_records_toggled;
    int blocked_non_text_chain;
    int blocked_hint_delivery;
    int blocked_unowned_tile_advance;
    int pit_tele_tile_mutated;
    /* A complete all-DB3 PUSH_BUTTON_SWITCH wall/floor chain can change only the
     * bit-13 state of source-addressed direct DB0 target doors.  It is kept
     * separate from the generic FLOOR counters so callers cannot mistake an
     * unowned actuator family for this narrowly complete mutation. */
    int push_button_actuators_seen;
    int push_button_doors_mutated;
    uint32_t private_push_button_hash;
    /* CROSS_MAP is the one WALL_MECHA route that only creates a source 0x04
     * message for another authenticated map.  The queued record remains
     * private; no target tile is dispatched here. */
    int cross_map_actuators_seen;
    int cross_map_messages_queued;
    uint32_t private_cross_map_hash;
    /* COUNTER is a self-contained DB3 mutation when the complete active
     * tile chain contains only DB3 records.  It updates its authentic Data
     * word and queues only the source-addressed 0x04 continuation. */
    int counter_actuators_seen;
    int counter_records_mutated;
    int counter_messages_queued;
    uint32_t private_counter_hash;
    /* RELAY_1/RELAY_3 have no independent game-state mutation: their source
     * effect is a delayed 0x04 message.  The message stays in this owner's
     * dynamic c_tim heap until a fully-owned target family can consume it. */
    int relay_actuators_seen;
    int relay_messages_queued;
    uint32_t private_relay_hash;
    int finite_relay_actuators_seen;
    int finite_relay_records_mutated;
    int finite_relay_messages_queued;
    uint32_t private_finite_relay_hash;
    uint8_t tile_state_before;
    uint8_t tile_state_after;
    uint32_t private_text_visibility_hash;
} DM2_V1_GameLoadActuateReceipt;

/* Private result of one real 0x01 DM2_STEP_DOOR timer.  A step remains
 * private until the GAME_LOAD transaction owns party damage, creature
 * collision and the audible GDAT/SND delivery path.  Therefore this atom
 * accepts only an authenticated DB0-first door square without the party or a
 * DB4 creature in its local chain.  It still preserves the original timer's
 * DB0 handle and source direction, and requeues the next real 0x01 record
 * only while an animation frame remains. */
typedef struct {
    int valid;
    int source_noop_destroyed;
    int door_state_mutated;
    int requeued;
    int blocked_party_collision;
    int blocked_creature_collision;
    int blocked_incomplete_chain;
    int map;
    uint8_t x;
    uint8_t y;
    uint8_t direction;
    int16_t door_record_link;
    uint8_t state_before;
    uint8_t state_after;
    uint16_t door_record_attributes_before;
    uint16_t door_record_attributes_after;
    int door_record_state_mutated;
    uint32_t private_animation_hash;
} DM2_V1_GameLoadDoorStepReceipt;

/* Private, source-ordered result for one due c_tim record.  The owner only
 * consumes timer families whose entire immediate mutation chain is already
 * retained here.  Any other heap head remains in place and blocks the
 * pre-session transaction rather than being silently dropped. */
typedef struct {
    int valid;
    int timer_dequeued;
    int blocked_unowned_timer;
    uint8_t timer_type;
    int16_t timer_map;
    DM2_V1_GameLoadTickGeneratorReceipt tick_generator;
    DM2_V1_GameLoadActuateReceipt actuate;
    DM2_V1_GameLoadDoorStepReceipt door_step;
} DM2_V1_GameLoadTimerProcessReceipt;

/* Build a source-owned, pre-selection world from one currently mounted,
 * hash-verified PC File_header/GDAT pair and exact mirror clicks. It rejects
 * partial record graphs and never modifies profile-owned raw media. A prior
 * initialized owner is replaced only after the new transaction is complete;
 * unknown caller storage is safely zeroed instead. */
int dm2_v1_game_load_world_owner_init_new_game(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_BootProfile *profile,
    const DM2_V1_BootNewGamePartySelection *selections,
    int selection_count);

void dm2_v1_game_load_world_owner_free(DM2_V1_GameLoadWorldOwner *owner);
int dm2_v1_game_load_world_owner_is_prepared(
    const DM2_V1_GameLoadWorldOwner *owner);

/* Source port of c_tim_proc.cpp::DM2_PROCESS_ACTUATOR_TICK_GENERATOR for
 * the prepared fresh-game owner. It changes only owned DB3 bytes and its
 * owned c_tim heap; an enqueue failure restores both before returning 0. */
int dm2_v1_game_load_world_owner_process_actuator_tick_generators(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadActuatorGeneratorReceipt *out_receipt);

/* Source-ordered New Game equivalent of the DM2_move_2fcf_0b8b call at the
 * tail of DM2_GAME_LOAD.  It is legal only after the full generator scan and
 * derives map, position and direction from authenticated DUNGEON.DAT header
 * fields.  It does not publish a party, viewport or M11 map. */
int dm2_v1_game_load_world_owner_materialize_source_map_context(
    DM2_V1_GameLoadWorldOwner *owner);

/* Materialize DM2__INIT_GAME's first c_1031_0541(5) call at its original
 * temporal point: after c_eventqueue::init but before the first automatic
 * mirror champion. The resulting mutable source tables stay private. */
int dm2_v1_game_load_world_owner_materialize_preselection_init_game_ui(
    DM2_V1_GameLoadWorldOwner *owner);

/* Materialize the click-ordered c_hero and possession result that was
 * authenticated while the owner was built.  This is deliberately after the
 * fresh-game actuator-generator phase, as in c_savegame.cpp::DM2_GAME_LOAD.
 * It only transfers real source records into the private c_party image; it
 * neither removes map-chain records nor publishes a live session. */
int dm2_v1_game_load_world_owner_materialize_champion_selection(
    DM2_V1_GameLoadWorldOwner *owner);

/* Source port of c_tim_proc.cpp::DM2_CONTINUE_TICK_GENERATOR and its
 * DM2_INVOKE_ACTUATOR/DM2_INVOKE_MESSAGE tail. `timer` must be a popped 0x56
 * entry from this owner's c_tim heap. This schedules only private 0x04/0x56
 * entries and never dispatches their side effects into M11. */
int dm2_v1_game_load_world_owner_continue_tick_generator(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadTickGeneratorReceipt *out_receipt);

/* Consume one private 0x04 message after it was popped from this owner's
 * queue.  This ports DM2_PROCEED_TIMERS' tile-class selection exactly, but
 * performs a successful action only for source class 3 (the original no-op)
 * and the fully-owned DB2-only FLOOR subset.  The latter implements the
 * exact Text::TextVisibility update from ACTUATE_FLOOR_MECHA, but rejects a
 * mixed record chain and a newly-visible party-square hint until the message
 * delivery owner exists. Classes 2 and 5 can also close their source tile
 * before that same DB2 atom; their opening path stays blocked because it
 * requires DM2_ADVANCE_TILES_TIME. A complete direction-matching DB3 COUNTER
 * chain on class 0 or 1 can also update its source Data word and queue its
 * exact private 0x04 continuation. A complete RELAY_1/RELAY_3 chain can
 * likewise retain its source-gated delayed 0x04 continuation. Other class
 * 0, 4 and 6 families remain
 * rejected without mutation rather than applying an incomplete
 * wall/door/trick-wall fragment.
 */
int dm2_v1_game_load_world_owner_dispatch_actuate_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadActuateReceipt *out_receipt);

/* Consume one private type-0x01 DM2_STEP_DOOR timer.  The timer must carry
 * the direct DB0 door handle in wvalueB, an original direction in actor, and
 * its actual File_header coordinate/map.  No coordinate-only door is ever
 * accepted.  Source: SKProject SKULLWIN/c_tim_proc.cpp::DM2_STEP_DOOR
 * (line 127+) and ::DM2_ACTUATE_DOOR (line 3744). */
int dm2_v1_game_load_world_owner_process_door_step_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    const DM2_V1_TimerEntry *timer,
    DM2_V1_GameLoadDoorStepReceipt *out_receipt);

/* Source order for one due c_tim entry: pop first, change the private current
 * map, then dispatch.  Only the retained 0x56 generator, 0x04 actuator and
 * 0x01 door paths can commit.  A missing family restores the full private
 * map, DB pools and heap byte-for-byte, leaving its head pending. */
int dm2_v1_game_load_world_owner_process_next_due_timer(
    DM2_V1_GameLoadWorldOwner *owner,
    DM2_V1_GameLoadTimerProcessReceipt *out_receipt);

#ifdef __cplusplus
}
#endif

#endif
