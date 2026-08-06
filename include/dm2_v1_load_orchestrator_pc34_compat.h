#ifndef DM2_V1_LOAD_ORCHESTRATOR_PC34_COMPAT_H
#define DM2_V1_LOAD_ORCHESTRATOR_PC34_COMPAT_H

/* DM2 load-orchestrator compatibility seam.
 * Source: SKProject SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD
 * (1415-1530).
 *
 * The ABI describes the eventual source order, but it is deliberately not a
 * loader today.  A prior callback transcript skipped raw dungeon blocks and
 * could begin SUPPRESS at an invented offset.  Until the raw-SKSave receipt,
 * DM2_READ_SKSAVE_DUNGEON and live record allocation share one owner,
 * dm2_v1_load_orchestrate() rejects atomically without calling callbacks. */

#include "dm2_v1_save_load.h"
#include "dm2_v1_save_read_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_load_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include "dm2_v1_save_orchestrator_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Callbacks for load orchestrator — stores all loaded data. */
typedef struct {
    void *ctx;

    /* Read raw bytes from save file. Returns 0 on success. */
    int (*read_raw)(void *ctx, uint8_t *out, size_t size);

    /* Store save header (0x2a bytes). */
    int (*set_header)(void *ctx, const uint8_t *data);

    /* Store savegame words (0x2c bytes). */
    int (*set_sgwords)(void *ctx, const uint8_t *data);

    /* Store raw data block.
     * block_id: 0=v1e03c8, 1=v1e03d8, 2=dm2_v1e038c, 3=v1e03d0.
     * Returns 0 on success. */
    int (*set_raw_block)(void *ctx, int block_id,
                         const uint8_t *data, size_t size);

    /* Store record type array.
     * type: 0-15. Returns 0 on success. */
    int (*set_record_array)(void *ctx, int type,
                            const uint8_t *data, size_t count);

    /* Store map data block. */
    int (*set_map_data)(void *ctx, const uint8_t *data, size_t size);

    /* Receive the decoded savegame buffer. */
    int (*receive_savegame_buffer)(void *ctx, const DM2_SaveGameBuffer *buf);

    /* Store globalb (64 bytes). */
    int (*set_globalb)(void *ctx, const uint8_t *data);

    /* Store v1e0104 (8 bytes). */
    int (*set_v1e0104)(void *ctx, const uint8_t *data);

    /* Store globalw (128 bytes = 64 words). */
    int (*set_globalw)(void *ctx, const uint8_t *data);

    /* Store hero data from SUPPRESS. hero_idx: 0..N-1, data: 263 bytes. */
    int (*set_hero_data)(void *ctx, int hero_idx, const uint8_t *data);

    /* Store save state (6 bytes: savegames1). */
    int (*set_save_state)(void *ctx, const uint8_t *data);

    /* Store timer array entry. idx: 0..N-1, data: entry_size bytes. */
    int (*set_timer_entry)(void *ctx, int idx, const uint8_t *data,
                           size_t entry_size);

    /* Get sgwords field by index (warr_00[idx]).
     * Needed to know record counts and map sizes. */
    uint16_t (*get_sgwords_field)(void *ctx, int idx);

    /* Get timer entry size. */
    size_t (*get_timer_entry_size)(void *ctx);

    /* READ_RECORD_CHECKCODE callbacks for hero inventory + dungeon. */
    DM2_ReadRecordCallbacks read_record_cb;

    /* LOAD_EXTRA_DUNGEON_DATA callbacks. */
    DM2_LoadExtraDungeonCallbacks dungeon_cb;
} DM2_LoadOrchestratorCallbacks;

/* Result of load orchestrator. */
typedef struct {
    int valid;
    uint16_t hero_count;
    uint16_t num_timers;
    uint16_t current_map;
    int hero_items_loaded;
    int dungeon_loaded;
    int error;
} DM2_LoadOrchestratorResult;

enum {
    DM2_LOAD_ORCHESTRATOR_ERROR_ARGUMENT = 1,
    DM2_LOAD_ORCHESTRATOR_ERROR_RAW_LAYOUT_UNBOUND = 4
};

/* This currently returns -1.  It sets RAW_LAYOUT_UNBOUND for valid arguments
 * and leaves all caller state untouched. */
int dm2_v1_load_orchestrate(
    const DM2_LoadOrchestratorCallbacks *cb,
    const uint8_t *in_buf, size_t in_size,
    DM2_LoadOrchestratorResult *result);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_LOAD_ORCHESTRATOR_PC34_COMPAT_H */
