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

void dm2_v1_sksave_game_load_owner_free(DM2_V1_SksaveGameLoadOwner *owner);

#ifdef __cplusplus
}
#endif

#endif
