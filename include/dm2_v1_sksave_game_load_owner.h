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
    /* Must remain zero until global-effect timers, timer dispatch and the
     * M11 runtime have one source-complete owner. */
    int source_game_load_session_ready;
    uint16_t savegamew7;
    DM2_V1_OriginalRawSaveStateReceipt state;
    uint8_t savegame_buffer[60];
    uint8_t v1e0104[8];
    uint8_t globalb[64];
    uint8_t globalw[128];
    uint8_t savegames1[DM2_V1_ORIGINAL_SAVEGAMES1_SIZE];
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
        uint16_t current_map;
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
    DM2_V1_SksaveSpecialTimerReceipt receipt;
} DM2_V1_SksaveGameLoadOwner;

/* Read-only result from the DB0-only selection portion of
 * DM2_RECYCLE_A_RECORD_FROM_THE_WORLD.  `found` names a record which the
 * original allocator would subsequently clear and return; this receipt does
 * neither.  The cursor is prospective until a complete ALLOC_NEW_RECORD
 * transaction can atomically commit its c_map/record/CAII effects.
 *
 * Source: SKProject SKULLWIN/c_record.cpp:544-1073. */
typedef struct {
    int valid;
    int found;
    uint16_t selected_link;
    uint8_t selected_map;
    uint8_t selected_x;
    uint8_t selected_y;
    uint8_t cursor_before;
    uint8_t cursor_after;
    uint16_t maps_scanned;
    uint32_t records_examined;
    uint32_t static_possession_descents;
} DM2_V1_SksaveDb0RecyclerCandidate;

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

/* Apply DM2_PROCEED_GLOBAL_EFFECT_TIMERS to an already retained private
 * owner.  It is atomic: a timer type 0x0e or invalid actor restores the
 * source bytes and returns zero.  Weight recalculation intentionally remains
 * blocked because c_party's active hand/container owner is not retained. */
int dm2_v1_sksave_game_load_owner_apply_post_load_global_effects(
    DM2_V1_SksaveGameLoadOwner *owner);

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

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner);

#ifdef __cplusplus
}
#endif

#endif
