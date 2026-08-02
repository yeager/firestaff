#ifndef DM2_V1_SAVE_ORCHESTRATOR_PC34_COMPAT_H
#define DM2_V1_SAVE_ORCHESTRATOR_PC34_COMPAT_H

/* DM2 save orchestrator — top-level save pipeline.
 * Source: sksvgame.cpp:2086-2287 (DM2_GAME_SAVE_MENU).
 *
 * Sequences all save phases: raw data writes, SUPPRESS sections
 * (savegame buffer, globals, heroes, save state, timers),
 * hero inventory record chains, extra dungeon data, possession
 * indices, and SUPPRESS flush.
 *
 * Uses callbacks for all runtime data access. */

#include "dm2_v1_save_load.h"
#include "dm2_v1_save_write_record_checkcode_pc34_compat.h"
#include "dm2_v1_save_store_extra_dungeon_data_pc34_compat.h"
#include "dm2_v1_save_write_possession_indices_pc34_compat.h"
#include "dm2_v1_save_suppress_masks_pc34_compat.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Save header (s_hex30): 0x2a bytes written raw before SUPPRESS section. */
#define DM2_SAVE_HEADER_SIZE 0x2A

/* Savegame words (s_sgwords): 0x2c bytes, contains record counts and map info. */
#define DM2_SAVE_SGWORDS_SIZE 0x2C

/* Number of record types. */
#define DM2_SAVE_RECORD_TYPES 16

/* Maximum heroes. */
#define DM2_SAVE_MAX_HEROES 4

/* Hero inventory slots. */
#define DM2_SAVE_HERO_ITEM_SLOTS 30

/* Savegame buffer (s_savegamebuffer): 0x3c bytes SUPPRESS-encoded. */
typedef struct {
    uint32_t gametick;          /* l_00 */
    uint32_t random_seed;       /* ul_04 */
    uint16_t heros_in_party;    /* w_08 */
    uint16_t map_x;             /* w_0a (v1e0270) */
    uint16_t map_y;             /* w_0c (v1e0272) */
    uint16_t facing;            /* w_0e (v1e0258) */
    uint16_t current_map;       /* w_10 (v1e0266) */
    uint16_t event_hero_idx;    /* w_12 */
    uint16_t num_timers;        /* w_14 */
    uint32_t field_16;          /* l_16 (v1d26a4) */
    uint32_t field_1a;          /* l_1a (v1e01a0) */
    uint16_t field_1e;          /* w_1e (v1e026e) */
    uint16_t field_20;          /* w_20 (v1e025e) */
    uint16_t field_22;          /* w_22 (v1e0274) */
    uint8_t  pad_24[4];         /* padding to reach w_28 */
    uint16_t map_packed;        /* w_28 */
    uint32_t field_2a;          /* l_2a (v1e147f) */
    uint8_t  field_2e;          /* b_2e (v1e1480) */
    uint8_t  field_2f;          /* b_2f (v1e1483) */
    uint8_t  field_30;          /* b_30 (v1e1482) */
    uint8_t  field_31;          /* b_31 (v1e147e) */
    uint8_t  field_32;          /* b_32 (v1e147d) */
    uint8_t  field_33;          /* b_33 (v1e1484) */
    uint16_t field_34;          /* w_34 (v1e1474) */
    uint8_t  field_36;          /* b_36 (v1e147b) */
    uint8_t  field_37;          /* b_37 (v1e1478) */
    uint32_t field_38;          /* l_38 (v1e1434) */
} DM2_SaveGameBuffer;

/* Callbacks for save orchestrator — provides all runtime data. */
typedef struct {
    void *ctx;

    /* Write raw bytes to save file. Returns 0 on success. */
    int (*write_raw)(void *ctx, const uint8_t *data, size_t size);

    /* Get save header (0x2a bytes). */
    int (*get_header)(void *ctx, uint8_t *out);

    /* Get savegame words (0x2c bytes, contains record counts). */
    int (*get_sgwords)(void *ctx, uint8_t *out);

    /* Get raw data block for pre-SUPPRESS section.
     * block_id: 0=v1e03c8, 1=v1e03d8, 2=dm2_v1e038c, 3=v1e03d0
     * Returns pointer and sets *size. */
    const uint8_t *(*get_raw_block)(void *ctx, int block_id, size_t *size);

    /* Get record type array pointer and count.
     * type: 0-15. Returns pointer, sets *count. */
    const uint8_t *(*get_record_array)(void *ctx, int type, size_t *count);

    /* Get map data block (v1e03e0). */
    const uint8_t *(*get_map_data)(void *ctx, size_t *size);

    /* Fill the savegame buffer struct. */
    int (*fill_savegame_buffer)(void *ctx, DM2_SaveGameBuffer *buf);

    /* Get globalb array (64 bytes). */
    const uint8_t *(*get_globalb)(void *ctx);

    /* Get v1e0104 array (8 bytes). */
    const uint8_t *(*get_v1e0104)(void *ctx);

    /* Get globalw array (128 bytes = 64 words). */
    const uint8_t *(*get_globalw)(void *ctx);

    /* Get hero data for SUPPRESS. hero_idx: 0..N-1. Returns 263-byte hero. */
    const uint8_t *(*get_hero_data)(void *ctx, int hero_idx);

    /* Get hero count. */
    int (*get_hero_count)(void *ctx);

    /* Get save state (6 bytes: savegames1). */
    const uint8_t *(*get_save_state)(void *ctx);

    /* Get timer array. Returns pointer and count. */
    const uint8_t *(*get_timer_array)(void *ctx, int *count);

    /* Get timer entry size. */
    size_t (*get_timer_entry_size)(void *ctx);

    /* Get hero item link for WRITE_RECORD_CHECKCODE.
     * hero_idx: 0..N-1, slot: 0..29.
     * Returns record link word. */
    uint16_t (*get_hero_item_link)(void *ctx, int hero_idx, int slot);

    /* Get savegamewpc.w_00 link for final WRITE_RECORD_CHECKCODE. */
    uint16_t (*get_wpc_link)(void *ctx);

    /* Get sgwords field by index (warr_00[idx]). */
    uint16_t (*get_sgwords_field)(void *ctx, int idx);

    /* Write record checkcode callbacks. */
    DM2_WriteRecordCallbacks write_record_cb;

    /* Store extra dungeon data callbacks. */
    DM2_StoreExtraDungeonCallbacks dungeon_cb;

    /* Write possession indices callback. */
    DM2_WritePossessionCallbacks possession_cb;
} DM2_SaveOrchestratorCallbacks;

/* Result of orchestrator. */
typedef struct {
    size_t suppress_bits_written;
    int creatures_written;
    int containers_written;
    int records_written;
    int error;
} DM2_SaveOrchestratorResult;

/* Run the full DM2 save pipeline.
 * out_buf/out_cap: output buffer for SUPPRESS-encoded data.
 * The raw data sections are written via cb->write_raw.
 * Returns 0 on success. */
int dm2_v1_save_orchestrate(
    const DM2_SaveOrchestratorCallbacks *cb,
    uint8_t *out_buf, size_t out_cap,
    DM2_SaveOrchestratorResult *result);

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_SAVE_ORCHESTRATOR_PC34_COMPAT_H */
