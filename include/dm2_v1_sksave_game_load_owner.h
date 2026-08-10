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

typedef struct DM2_V1_SksaveGameLoadOwner {
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

/* Materialize one fully source-walked private transaction.  `raw_body` is
 * the original SKSAVE payload after its 42-byte header; it is never modified
 * or retained.  On failure `owner` is cleared and no partial owner survives. */
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

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner);

#ifdef __cplusplus
}
#endif

#endif
