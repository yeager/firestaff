#ifndef FIRESTAFF_DM2_V1_SKSAVE_GAME_LOAD_OWNER_H
#define FIRESTAFF_DM2_V1_SKSAVE_GAME_LOAD_OWNER_H

/* Private, source-ordered RAM ownership for one original PC-DOS SKSAVE.
 *
 * This is deliberately below the M11/session boundary.  It preserves the
 * exact c_map, c_record, c_hero and c_tim objects produced by
 * DM2_GAME_LOAD/DM2_READ_SKSAVE_DUNGEON, but cannot make Resume playable.
 * Source: SKProject SKWINSPX/src/v5/sksvgame.cpp:1476-1528.
 */

#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_save_post_load_global_effects_pc34_compat.h"
#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_caii_source_owner.h"
#include "dm2_v1_game_load_sound_owner.h"
#include "dm2_v1_timer_queue_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* First-field lifetime marker.  Constructors must tolerate both zeroed and
 * previously completed owners without dropping their RAM-owned map and
 * record-pool allocations.  It is an ownership guard, never save data. */
#define DM2_V1_SKSAVE_GAME_LOAD_OWNER_LIFECYCLE_TAG 0x53474c4fu /* "SGLO" */

typedef struct DM2_V1_SksaveGameLoadOwner {
    uint32_t lifecycle_tag;
    int valid;
    uint8_t *owned_raw_file;
    size_t owned_raw_file_size;
    /* A source-ordered import can reach DM2_ALLOC_NEW_RECORD's DB0 or DB2
     * recycler boundary before every map stream is restored. Keep that exact
     * mutable c_map/c_record snapshot for read-only recycler analysis only.
     * It is deliberately distinct from `valid`: no caller may use an
     * inspection owner for Resume, timer dispatch or a game session.
     * Source: SKProject c_record.cpp:DB88.  Only DB0 takes the direct-return
     * branch there; DB2 is retained solely because its exhaustion is a useful
     * source boundary for later inspection. */
    int recycler_boundary_inspection_valid;
    /* Must remain zero until global-effect timers, timer dispatch and the
     * M11 runtime have one source-complete owner. */
    int source_game_load_session_ready;
    uint16_t savegamew7;
    DM2_V1_OriginalRawSaveStateReceipt state;
    /* Authenticated DUNGEON.DAT layout paired with this SKSAVE.  The save
     * body owns mutable map/record state; this separate copy owns the static
     * dungeon descriptor required by the later runtime candidate. */
    DM2_V1_DungeonData source_dungeon;
    int source_dungeon_valid;
    int source_dungeon_tile_layout_valid;
    uint32_t source_dungeon_tile_layout_hash;
    uint8_t savegame_buffer[60];
    uint8_t v1e0104[8];
    uint8_t globalb[64];
    uint8_t globalw[128];
    uint8_t savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];
    /* Exact source c_wbbb/ddat.savegames1 bytes. `savegames1` above is the
     * mutable post-load work area; this snapshot is never rewritten by
     * global-effect reconstruction. */
    uint8_t source_savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];
    int source_savegames1_valid;
    DM2_V1_Hero heroes[DM2_MAX_HEROES];
    DM2_V1_SaveTimerRecord timers[DM2_V1_SAVE_TIMER_MAX];
    int16_t timer_indices[DM2_V1_SAVE_TIMER_MAX];
    int16_t timer_queue_count;
    int16_t timer_free_head;
    uint16_t leader_hand_root;
    /* sksvgame.cpp::DM2_PROCEED_GLOBAL_EFFECT_TIMERS runs over the retained
     * c_tim/c_hero/savegames1 objects only.  It never promotes this owner to
     * M11; an unimplemented 0x0e leaves the transaction invalid. */
    DM2_V1_GlobalEffectReceipt global_effect_receipt;
    int global_effects_complete;
    int weight_recompute_blocked;
    /* Retained CREATURES[type] -> v1d296c flags for actual DB4 records in
     * this SKSAVE transaction.  These are copied during admission through
     * the same authenticated callback used by READ_RECORD_CHECKCODE; later
     * recycler work must not consult ambient/global GDAT state. */
    uint16_t retained_creature_ai_flags[256];
    uint8_t retained_creature_ai_valid[256];
    /* Source-owned executable/GDAT AI rows for this exact SKSAVE dungeon.
     * This is kept private so later recycler/runtime admission never falls
     * back to process-global CREATURES state. */
    DM2_V1_CaiiSourceOwner caii_source;
    /* Borrowed immutable GRAPHICS.DAT owner retained for the future runtime
     * candidate; the SKSAVE clone never writes through this pointer. */
    const DM2_V1_AssetLoader *asset_loader;
    /* DM2_1c9a_3c30 capacity derived from the authenticated DB4 pool and
     * AIDefinition flags. This is not yet the mutable 34-byte CAII array. */
    uint16_t caii_source_capacity;
    uint16_t caii_nonstatic_creature_count;
    int caii_capacity_valid;
    DM2_V1_CaiiArray caii_slots;
    int caii_slots_valid;
    DM2_V1_DropRng caii_rng;
    int caii_rng_initialized;
    int caii_static_animation_valid;
    uint32_t caii_static_animation_hash;
    int caii_dynamic_valid;
    uint16_t caii_dynamic_candidate_count;
    uint16_t caii_dynamic_allocated_slot_count;
    uint16_t caii_dynamic_think_timer_count;
    uint16_t caii_dynamic_noise_queue_count;
    uint32_t caii_dynamic_hash;
    /* The narrow, private context that immediately precedes
     * c_record.cpp::DM2_RECYCLE_A_RECORD_FROM_THE_WORLD.  It owns no
     * recycler operation: DB0 returns a selected source record to
     * DM2_ALLOC_NEW_RECORD, while DB4 and DB14 can delete creatures or
     * missiles and other DBs can relocate linked objects.  Even DB0 needs
     * the complete two-pass map/chain traversal (including static-creature
     * possessions) before ALLOC_NEW_RECORD may zero the selected record, so
     * callers must still reject allocation when a DB is exhausted.
     * `map_cursors` is ddat.v1e0426[0..17], which the
     * original runtime initializes to zero, not a value serialized in the
     * SKSave body.  The remaining values are authenticated from the exact
     * GAME_LOAD c_map and savegame-buffer spans retained by this owner.
     *
     * Source: SKProject SKWINSPX/src/v5/dm2data.cpp:1371,
     *         sksvgame.cpp:1485-1493,
     *         c_record.cpp::DM2_RECYCLE_A_RECORD_FROM_THE_WORLD.
     */
    struct {
        int valid;
        uint8_t map_cursors[18];
        uint8_t map_count;
        /* c_map's active map at the allocator boundary. It is saved and
         * restored by DM2_RECYCLE_A_RECORD_FROM_THE_WORLD. */
        uint16_t current_map;
        /* ddat.v1e0266: party map, which the recycler skips during its first
         * pass. It is not necessarily c_map's active map while a resident
         * chain is being restored. */
        uint16_t party_map;
        uint16_t party_x;
        uint16_t party_y;
        uint16_t party_direction;
        /* During DM2_GAME_LOAD, READ_SKSAVE_DUNGEON runs before
         * DM2_move_2fcf_0b8b discovers an alternate teleporter map.
         * dm2data::init supplies v1e0234=0; the recycler consequently derives
         * no protected map (-1). Retain that derived source phase explicitly
         * rather than treating a missing field as a host default. */
        int protected_map_active;
        int16_t protected_map;
        uint16_t column_index_count;
        uint16_t ground_stack_count;
        uint16_t map_data_byte_count;
        uint32_t column_index_hash;
        uint32_t ground_stack_hash;
        uint32_t map_data_hash;
        /* Always true until the source deletion/move callbacks and their
         * CAII, c_map and timer owners are retained in the same transaction. */
        int recycle_blocked;
    } recycler_context;
    DM2_V1_SksaveMapOwner map_owner;
    DM2_V1_RecordPoolSet record_pools;
    /* Writable c_tim-shaped clone prepared from the authenticated SKSAVE
     * timer array. It is not dispatched while this owner is pre-session. */
    DM2_V1_TimerEntry *runtime_timer_entries;
    int16_t *runtime_timer_indices;
    DM2_V1_TimerQueue runtime_timer_queue;
    uint16_t runtime_timer_capacity;
    int source_timer_owner_ready;
    /* GAME_LOAD's SOUND9 owner is not serialized by SKSAVE. It is admitted
     * separately from the exact GRAPHICS.DAT DYN4 selector set and remains
     * private until a complete runtime-candidate transfer. */
    DM2_V1_GameLoadSoundOwner sound_owner;
    int source_sound_materialized;
    DM2_V1_SksaveSpecialTimerReceipt receipt;
} DM2_V1_SksaveGameLoadOwner;

/* Read-only RESET_CAII/FILL_ORPHAN_CAII admission census.  This proves the
 * source-owned DB4 positions and the 0x21/0x22 timer evidence without writing
 * a fabricated c_creature slot or changing DB4 byte@5. */
typedef struct {
    int valid;
    int map_owner_ready;
    int source_ai_ready;
    uint16_t live_db4_count;
    uint16_t static_candidate_count;
    uint16_t static_lazy_fill_candidate_count;
    uint16_t static_already_assigned_count;
    uint16_t dynamic_candidate_count;
    uint16_t dynamic_timer_match_count;
    uint16_t dynamic_timer_missing_count;
    uint16_t dynamic_timer_ambiguous_count;
    uint32_t source_hash;
} DM2_V1_SksaveCaiiAdmissionReceipt;

/* Read-only preview of the source startend.cpp::DM2_RESET_CAII followed by
 * c_1c9a.cpp::DM2_FILL_ORPHAN_CAII pass.  The preview deliberately exposes
 * counts and provenance only: it does not clear DB4 byte@5, allocate a slot,
 * or enqueue a think timer. */
typedef struct {
    int valid;
    uint16_t map_count;
    uint16_t creature_count;
    uint16_t static_fill_count;
    uint16_t dynamic_activation_count;
    uint16_t think_timer_count_required;
    uint16_t raw_slot_marker_count;
    uint32_t source_hash;
} DM2_V1_SksaveCaiiResetFillPreview;

typedef struct {
    int valid;
    uint16_t record_handle;
    uint16_t map;
    uint8_t x;
    uint8_t y;
    uint32_t source_hash;
} DM2_V1_SksaveRecordPositionReceipt;

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
    uint32_t source_hash;
} DM2_V1_SksaveDynamicCaiiReceipt;

int dm2_v1_sksave_game_load_owner_caii_admission(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveCaiiAdmissionReceipt *out_receipt);

int dm2_v1_sksave_game_load_owner_caii_reset_fill_preview(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveCaiiResetFillPreview *out_preview);

/* Apply the source static 09db half of FILL_CAII_CUR_MAP atomically. It
 * resets DB4 byte@5 and merges the authenticated static animation word; it
 * does not allocate a dynamic CAII slot or queue a think timer. */
int dm2_v1_sksave_game_load_owner_materialize_static_caii(
    DM2_V1_SksaveGameLoadOwner *owner);

/* Run the source GAME_LOAD dynamic CAII/0a48 transaction against this
 * authenticated SKSAVE owner. All mutable owners are published only after
 * the complete bridge succeeds; otherwise the owner is unchanged. */
int dm2_v1_sksave_game_load_owner_materialize_dynamic_caii(
    DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDynamicCaiiReceipt *out_receipt);

/* Complete the source-ordered RESET_CAII -> FILL_ORPHAN_CAII half of
 * GAME_LOAD after the saved world, current-map layout, timer owner and
 * SOUND9 owner have been bound.  This is deliberately separate from the
 * read-only CAII census: it mutates only the private Resume owner and rolls
 * back on any missing 0a48/animation/timer evidence. */
int dm2_v1_sksave_game_load_owner_materialize_runtime_caii(
    DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDynamicCaiiReceipt *out_receipt);

/* Resolve a direct source DB4 handle through the mutable SKSAVE c_map
 * ground-stack owner. This is the position half of the future owner-bound
 * CAII/think-timer scheduler; it never reads pristine DUNGEON.DAT roots. */
int dm2_v1_sksave_game_load_owner_record_position(
    const DM2_V1_SksaveGameLoadOwner *owner,
    uint16_t record_handle,
    DM2_V1_SksaveRecordPositionReceipt *out_receipt);

/* Queue the source 0x21/0x22 think timer directly against the authenticated
 * handle and its mutable c_map position. This mutates only the private timer
 * owner; CAII slot word@2 publication remains the caller's transaction. */
int dm2_v1_sksave_game_load_owner_schedule_think_timer(
    DM2_V1_SksaveGameLoadOwner *owner,
    uint16_t record_handle,
    uint16_t map,
    uint8_t x,
    uint8_t y,
    DM2_V1_CreatureScheduleReceipt *out_receipt);

/* Read-only result from the DB0 direct-return portion of
 * DM2_RECYCLE_A_RECORD_FROM_THE_WORLD.  `found` names a record which the
 * original allocator would subsequently clear and return; this receipt does
 * neither.  The cursor is prospective until a complete ALLOC_NEW_RECORD
 * transaction can atomically commit its c_map/record/CAII effects.
 *
 * Source: SKProject SKULLWIN/c_record.cpp:544-1073. */
typedef struct {
    int valid;
    int found;
    uint8_t requested_db;
    uint16_t selected_link;
    uint8_t selected_map;
    uint8_t selected_x;
    uint8_t selected_y;
    uint8_t cursor_before;
    uint8_t cursor_after;
    uint16_t maps_scanned;
    uint32_t records_examined;
    uint32_t static_possession_descents;
} DM2_V1_SksaveRecyclerCandidate;

typedef DM2_V1_SksaveRecyclerCandidate DM2_V1_SksaveDb0RecyclerCandidate;

typedef struct {
    int valid;
    int found;
    uint16_t missile_record;
    uint8_t map;
    uint8_t x;
    uint8_t y;
    int16_t timer_index;
} DM2_V1_SksaveDb14MissileDeleteCandidate;

/* Read-only 0x1e dispatch candidate over the private writable timer heap.
 * It proves the due timer, DB14 handle, timer backlink and tile coordinates,
 * but does not consume the timer or move/delete the missile. */
typedef struct {
    int valid;
    int due;
    int blocked_no_timer;
    int blocked_wrong_type;
    int blocked_record;
    int blocked_chain;
    int16_t timer_slot;
    int16_t missile_record;
    int16_t map;
    int16_t x;
    int16_t y;
    uint8_t direction;
    uint16_t energy_step;
} DM2_V1_SksaveMissileTimerCandidate;

/* Admission facts for the future SKSAVE -> GAME_LOAD runtime-candidate
 * transfer. This is deliberately not a playable-session receipt: the
 * missing CAII array and sound owner remain explicit blockers. */
typedef struct {
    int valid;
    int source_party_ready;
    int source_map_ready;
    int source_record_pool_ready;
    int source_record_graph_ready;
    int source_timer_ready;
    int source_savegames1_ready;
    int source_caii_ready;
    int source_caii_capacity_ready;
    int source_sound_ready;
    int runtime_candidate_ready;
    uint16_t party_count;
    uint16_t timer_count;
    uint16_t caii_capacity;
    uint32_t source_transaction_hash;
} DM2_V1_SksaveRuntimeCandidateAdmissionReceipt;

/* Materialize one fully source-walked private transaction.  `raw_body` is
 * the original SKSAVE payload after its 42-byte header; it is never modified
 * or retained.  A completed owner is atomically replaced only after the new
 * transaction succeeds; an unsuccessful replacement leaves that owner
 * unchanged.  Uninitialized output is cleared on failure. */
int dm2_v1_sksave_game_load_owner_init(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx);

/* Ordered variant for original platform saves.  `words_big_endian` is part
 * of the authenticated raw-dungeon receipt and is never inferred from the
 * host.  The legacy entry point above remains the little-endian DOS wrapper. */
int dm2_v1_sksave_game_load_owner_init_ordered(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    int words_big_endian, const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx);

/* Retain an otherwise rejected import only when the real source stream has
 * reached DB0 or DB2 exhaustion in DM2_READ_SKSAVE_DUNGEON. The retained
 * state is an inspection transaction through the exact allocator boundary:
 * it owns source-mutated maps, pools, heroes and c_tim decoded before that
 * point but remains `valid == 0`, `source_game_load_session_ready == 0`, and
 * cannot perform allocation, cursor writes or Resume. All other failures
 * leave the output unchanged (or zeroed when uninitialised).
 *
 * Source: SKProject SKULLWIN/c_savegame.cpp:1108-1350,
 *         c_record.cpp:DB88-DBA5. */
int dm2_v1_sksave_game_load_owner_init_to_recycler_boundary(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx);

int dm2_v1_sksave_game_load_owner_init_to_recycler_boundary_ordered(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_body, size_t raw_body_size, uint16_t savegamew7,
    int words_big_endian, const DM2_V1_AssetLoader *asset_loader,
    DM2_ReadRecordCreatureAiFlagsFn query_creature_ai_flags,
    void *query_creature_ai_flags_ctx);

/* Apply DM2_PROCEED_GLOBAL_EFFECT_TIMERS to an already retained private
 * owner.  It is atomic: a timer type 0x0e or invalid actor restores the
 * source bytes and returns zero.  Weight recalculation intentionally remains
 * blocked because c_party's active hand/container owner is not retained. */
int dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
    DM2_V1_SksaveGameLoadOwner *owner);

/* Copy the authenticated six-byte c_wbbb/savegames1 snapshot without
 * exposing mutable post-load work state. This is preparation for the future
 * atomic SKSAVE -> runtime candidate commit; it does not make the owner
 * playable or alter source_game_load_session_ready. */
int dm2_v1_sksave_game_load_owner_copy_source_savegames1(
    const DM2_V1_SksaveGameLoadOwner *owner,
    uint8_t out_savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE]);

/* Bind the authenticated PC-DOS DUNGEON.DAT layout to an already materialized
 * SKSAVE owner. The bytes are copied; the save's mutable c_map/record owner
 * remains separate. Map count and every map dimension must match exactly. */
int dm2_v1_sksave_game_load_owner_bind_dungeon_layout(
    DM2_V1_SksaveGameLoadOwner *owner,
    const uint8_t *raw_dungeon, size_t raw_dungeon_size);

/* Bind the source DYN4 selector/materialization set used to initialize the
 * SOUND9 queue. The selector IDs and selections are borrowed only during the
 * call; all resulting queue/sample state is copied into the owner. */
int dm2_v1_sksave_game_load_owner_bind_sound(
    DM2_V1_SksaveGameLoadOwner *owner,
    const DM2_V1_AssetLoader *loader, uint16_t selector_count,
    const uint32_t *selector_ids,
    const DM2_V1_GdatDyn4MaterializedSelection *selections);

/* Derive the source DYN4 selectors from the saved current-map DB3 chains and
 * bind the corresponding SOUND9 owner. This is the Resume-side equivalent
 * of LOAD_LOCALLEVEL_DYN's subtype-0x7e branch. */
int dm2_v1_sksave_game_load_owner_bind_current_map_sound(
    DM2_V1_SksaveGameLoadOwner *owner,
    const DM2_V1_AssetLoader *loader);

/* Convert the exact save c_tim records into a private writable GAME_LOAD
 * timer heap. No timer is consumed or dispatched by this call. */
int dm2_v1_sksave_game_load_owner_materialize_timer_owner(
    DM2_V1_SksaveGameLoadOwner *owner);

/* Source-ordered read-only gate for the eventual runtime-candidate bridge. */
int dm2_v1_sksave_game_load_owner_runtime_candidate_admission(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveRuntimeCandidateAdmissionReceipt *out_receipt);

/* Deep-copy the complete source-owned runtime underlay for a future Resume
 * candidate. Immutable asset/raw-body provenance may remain borrowed, but all
 * mutable map, pool, CAII, timer and SOUND9 storage is independently owned. */
int dm2_v1_sksave_game_load_owner_clone_runtime_underlay(
    DM2_V1_SksaveGameLoadOwner *out,
    const DM2_V1_SksaveGameLoadOwner *source);

/* Read only the CREATURES -> v1d296c flags retained for an actual DB4 type
 * during this import. Missing provenance is failure, never zero flags. */
int dm2_v1_sksave_game_load_owner_creature_ai_flags(
    const DM2_V1_SksaveGameLoadOwner *owner, uint8_t creature_type,
    uint16_t *out_flags);

/* Reproduce only the non-mutating DB0 candidate selection. It walks the
 * source two-pass map ring, respects DB3 and protected DB2 chain barriers,
 * and descends through source-static DB4 possessions using AI flags retained
 * during SKSAVE admission. It never writes a cursor, map, pool, timer or
 * record, and it never makes Resume playable. A missing retained AI row or
 * malformed chain invalidates the whole receipt rather than guessing. */
int dm2_v1_sksave_game_load_owner_db0_recycler_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDb0RecyclerCandidate *out_candidate);

/* The source's non-mutating recycler candidate audit. DB0, DB2, DB3, DB7 and
 * DB9 may produce direct-return candidates; DB5/DB6/DB8/DB10 may produce a
 * source cut-only candidate after their byte guards. DB2/Text protected
 * records are never returned or mutated. DB4, creature-bearing DB14 tiles,
 * DB13, DB15 and malformed pools remain fail-closed until their source owners
 * are complete. */
int dm2_v1_sksave_game_load_owner_recycler_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner, uint8_t requested_db,
    DM2_V1_SksaveRecyclerCandidate *out_candidate);

/* Read-only DB14 DELETE_MISSILE_RECORD admission. It requires a source
 * no-creature recycler candidate plus one active 0x1e missile timer whose
 * timer slot and A-handle both identify that exact DB14 record. */
int dm2_v1_sksave_game_load_owner_db14_missile_delete_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveDb14MissileDeleteCandidate *out_candidate);

int dm2_v1_sksave_game_load_owner_missile_timer_candidate(
    const DM2_V1_SksaveGameLoadOwner *owner,
    DM2_V1_SksaveMissileTimerCandidate *out_candidate);

/* Commit the admitted no-creature DB14 delete in the private pre-session
 * owner. Map/pool/timer changes publish together; source_game_load_session_ready
 * remains clear and the raw SKSAVE body is never modified. */
int dm2_v1_sksave_game_load_owner_commit_db14_missile_delete(
    DM2_V1_SksaveGameLoadOwner *owner, uint16_t *out_record);

/* Commit the source's DB0 direct-return path through a private owner.  This
 * clones the mutable record pools, applies ALLOC_NEW_RECORD's zero-and-mark
 * operation to the selected DB0 record, and advances ddat.v1e0426[0] only
 * after all steps succeed.  The owner remains inspection-only and the
 * operation never makes Resume or a runtime session playable.
 *
 * Source: SKULLWIN/c_record.cpp:DB88-DBA5 and :1076-1139. */
int dm2_v1_sksave_game_load_owner_commit_db0_recycler(
    DM2_V1_SksaveGameLoadOwner *owner, uint16_t *out_record);

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner);

#ifdef __cplusplus
}
#endif

#endif
