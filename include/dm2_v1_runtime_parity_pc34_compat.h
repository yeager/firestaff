/* DM2 V1 runtime parity ops — callback-based implementations for remaining
 * unimplemented skproject functions. Targets full algorithmic parity with
 * skproject source. */

#ifndef DM2_V1_RUNTIME_PARITY_PC34_COMPAT_H
#define DM2_V1_RUNTIME_PARITY_PC34_COMPAT_H

#include <stdint.h>

/* =====================================================================
 * util.cpp — DM2_ROTATE_5x5_POS
 * ===================================================================== */

int16_t dm2_v1_rotate_5x5_pos(int16_t pos, int16_t rotation);

/* =====================================================================
 * dm2global.cpp — DM2_UPDATE_GLOB_VAR / DM2_GET_GLOB_VAR
 * Three-tier storage: bits (0x00-0x3F), bytes (0x40-0x7F), words (0x80-0xBF)
 * ===================================================================== */

typedef struct {
    uint8_t bit_vars[8];     /* ddat.v1e0104 — 64 bit flags */
    uint8_t byte_vars[64];   /* ddat.globalb — byte vars 0x40..0x7F */
    int16_t word_vars[192];  /* ddat.v1e000c — word vars 0x00..0xBF */
} DM2_V1_GlobVarState;

void dm2_v1_glob_var_init(DM2_V1_GlobVarState *state);

int32_t dm2_v1_get_glob_var(
    const DM2_V1_GlobVarState *state, uint16_t index);

int32_t dm2_v1_update_glob_var_direct(
    DM2_V1_GlobVarState *state,
    int16_t var_idx, int16_t mode, int16_t value);

/* Legacy callback interface (retained for existing callers) */
typedef struct {
    uint8_t *bit_vars;
    uint8_t *byte_vars;
    int16_t *word_vars;
    int16_t (*get_glob_var)(void *ctx, int16_t var_idx);
    int16_t (*between_value)(int16_t lo, int16_t hi, int16_t val);
} DM2_V1_UpdateGlobVarCallbacks;

int32_t dm2_v1_update_glob_var(
    int16_t var_idx, int16_t mode, int16_t value,
    const DM2_V1_UpdateGlobVarCallbacks *cb, void *ctx);

/* =====================================================================
 * c_timer.cpp — Timer heap management
 * ===================================================================== */

typedef struct {
    int16_t type;
    int16_t data_w;
    uint32_t ticks;
    int16_t x;
    int16_t y;
    uint8_t actor;
    int16_t value;
} DM2_V1_TimerEntry;

typedef struct {
    DM2_V1_TimerEntry *timers;   /* timdat.timerarray */
    int16_t *indices;             /* timdat.timer_indices */
    int16_t num_timers;
    int16_t max_timers;
    int16_t num_indices;
    int16_t available_idx;
    int16_t timer_unk;
    uint32_t game_tick;
    void (*raise_syserr)(void *ctx, int code);
} DM2_V1_TimerHeapState;

void dm2_v1_timer_heap_sift(DM2_V1_TimerHeapState *s, int16_t pos);
void dm2_v1_rearrange_timerlist(DM2_V1_TimerHeapState *s);
int16_t dm2_v1_get_timer_new_index(DM2_V1_TimerHeapState *s, int16_t timer_slot);
void dm2_v1_delete_timer(DM2_V1_TimerHeapState *s, int16_t timer_slot);
int16_t dm2_v1_queue_timer(DM2_V1_TimerHeapState *s, const DM2_V1_TimerEntry *entry);
void dm2_v1_get_and_delete_next_timer(DM2_V1_TimerHeapState *s, DM2_V1_TimerEntry *out);
int dm2_v1_is_timer_to_proceed(DM2_V1_TimerHeapState *s);
void dm2_v1_timer_reindex(DM2_V1_TimerHeapState *s, int16_t timer_slot);

/* =====================================================================
 * c_record.cpp — Record operations
 * ===================================================================== */

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    int16_t (*query_cls2_of_text)(void *ctx, int32_t record);
    uint8_t (*get_wall_decoration)(void *ctx, uint8_t *record);
    int16_t (*query_gdat_entry_data_index)(void *ctx, int cls1, int cls2, int cat, int idx);
    int16_t *table1d3278;   /* 16-entry item-type base table */
} DM2_V1_RecordQueryCallbacks;

int16_t dm2_v1_get_distinctive_itemtype(
    int16_t record_word,
    const DM2_V1_RecordQueryCallbacks *cb, void *ctx);

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_SetItemtypeCallbacks;

void dm2_v1_set_itemtype(
    int32_t record_word, int32_t new_type,
    const DM2_V1_SetItemtypeCallbacks *cb, void *ctx);

/* dm2_v1_get_wall_tile_anyitem_record is declared in
 * dm2_v1_record_ops_pc34_compat.h with its authoritative signature
 * (map_x, map_y, DM2_V1_TileRecordWalkCallbacks). */

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_SetImportanceCallbacks;

void dm2_v1_set_item_importance(
    int16_t record_word, int16_t importance,
    const DM2_V1_SetImportanceCallbacks *cb, void *ctx);

typedef struct {
    int16_t *db_counts;       /* per-DB-type allocated count */
    int16_t *db_max;          /* per-DB-type max count */
    int16_t *free_list;       /* per-DB-type free list head */
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void (*raise_syserr)(void *ctx, int code);
} DM2_V1_AllocRecordCallbacks;

int16_t dm2_v1_alloc_new_record(
    int16_t db_type,
    const DM2_V1_AllocRecordCallbacks *cb, void *ctx);

void dm2_v1_dealloc_record(
    int16_t record_word,
    const DM2_V1_AllocRecordCallbacks *cb, void *ctx);

typedef struct {
    int16_t (*alloc_new_record)(void *ctx, int16_t db_type);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void (*copy_record)(void *ctx, uint16_t dst, uint16_t src);
    void (*set_next_link)(void *ctx, uint16_t record, int16_t next);
} DM2_V1_AllocDbitemCallbacks;

int16_t dm2_v1_alloc_new_dbitem(
    int16_t db_type, int16_t template_record,
    const DM2_V1_AllocDbitemCallbacks *cb, void *ctx);

/* Decoration functions moved to dm2_v1_record_ops_pc34_compat.h */

typedef struct {
    void (*drop_possessions)(void *ctx, int16_t record);
    void (*dealloc_record)(void *ctx, int16_t record);
    void (*unlink_from_tile)(void *ctx, int16_t record, int16_t x, int16_t y);
} DM2_V1_DeleteCreatureRecordCallbacks;

void dm2_v1_delete_creature_record(
    int16_t record_word, int16_t x, int16_t y,
    const DM2_V1_DeleteCreatureRecordCallbacks *cb, void *ctx);

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void (*place_item_on_tile)(void *ctx, int16_t item, int16_t x, int16_t y);
} DM2_V1_DropPossessionCallbacks;

void dm2_v1_drop_creature_possession(
    int16_t creature_record, int16_t x, int16_t y,
    const DM2_V1_DropPossessionCallbacks *cb, void *ctx);

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_RotateRecordCallbacks;

void dm2_v1_rotate_record_by_teleporter(
    int16_t record_word, int16_t rotation,
    const DM2_V1_RotateRecordCallbacks *cb, void *ctx);

typedef struct {
    int (*dispatch)(void *ctx, int16_t record, int16_t x, int16_t y);
} DM2_V1_075f056cCallbacks;

int dm2_v1_075f_056c(
    int16_t record, int16_t x, int16_t y,
    const DM2_V1_075f056cCallbacks *cb, void *ctx);

/* =====================================================================
 * c_record.cpp — init_global_records
 * ===================================================================== */

typedef struct {
    int db_type_count;
    int16_t *db_sizes;
    void (*init_db_pool)(void *ctx, int db_type, int16_t size);
} DM2_V1_InitGlobalRecordsCallbacks;

void dm2_v1_init_global_records(
    const DM2_V1_InitGlobalRecordsCallbacks *cb, void *ctx);

/* =====================================================================
 * c_creature.cpp — ATTACK_CREATURE, 4FCC
 * ===================================================================== */

typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    void (*wound_creature)(void *ctx, int16_t creature, int16_t damage);
    void (*play_sound)(void *ctx, int16_t x, int16_t y, int sample);
    int16_t (*query_ai_spec_armor)(void *ctx, int16_t creature_type);
    int16_t (*rand)(void *ctx);
    int16_t (*rand16)(void *ctx, int16_t max);
} DM2_V1_AttackCreatureCallbacks;

int32_t dm2_v1_attack_creature(
    int16_t creature_record, int16_t x, int16_t y,
    int16_t attack_type, int16_t sound_id, int32_t damage,
    const DM2_V1_AttackCreatureCallbacks *cb, void *ctx);

typedef struct {
    int (*dispatch)(void *ctx, int16_t creature_idx, int16_t x, int16_t y);
} DM2_V1_4FCCCallbacks;

int32_t dm2_v1_4fcc(
    int16_t creature_idx, int16_t x, int16_t y,
    const DM2_V1_4FCCCallbacks *cb, void *ctx);

/* =====================================================================
 * c_move.cpp — move_075f_06bd
 * ===================================================================== */

typedef struct {
    int (*dispatch)(void *ctx, int16_t x, int16_t y, int16_t dir);
} DM2_V1_Move075f06bdCallbacks;

void dm2_v1_move_075f_06bd(
    int16_t x, int16_t y, int16_t dir,
    const DM2_V1_Move075f06bdCallbacks *cb, void *ctx);

/* =====================================================================
 * c_tim_proc.cpp — Timer actuator functions
 * ===================================================================== */

typedef struct {
    uint8_t *(*get_tile_byte)(void *ctx, int16_t x, int16_t y);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void (*queue_noise)(void *ctx, int16_t x, int16_t y);
    void (*requeue_timer)(void *ctx, uint32_t delay);
    void (*invoke_actuator)(void *ctx, int16_t x, int16_t y, int16_t action, int16_t param);
    int16_t current_map;
    int16_t party_map;
    int16_t *redraw_flags;
} DM2_V1_TimerActuatorCallbacks;

/* Renamed from dm2_v1_step_door to avoid an ABI-incompatible symbol
 * collision with the DM2_V1_TimerRecord*-based dm2_v1_step_door()
 * declared in dm2_v1_tim_proc_pc34_compat.h (same name, different
 * calling convention -> caused a segfault when the linker picked this
 * definition for calls made against the other signature). */
void dm2_v1_step_door_tile(
    int16_t x, int16_t y, int16_t record_word, int16_t direction,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

void dm2_v1_actuate_pitfall(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

void dm2_v1_actuate_door(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

void dm2_v1_actuate_teleporter(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

void dm2_v1_actuate_trickwall(
    int16_t x, int16_t y, int16_t param,
    const DM2_V1_TimerActuatorCallbacks *cb, void *ctx);

/* =====================================================================
 * c_savegame.cpp — READ_DUNGEON_STRUCTURE
 * ===================================================================== */

typedef struct {
    int (*dispatch)(void *ctx, const uint8_t *data, int32_t length);
} DM2_V1_ReadDungeonStructureCallbacks;

int dm2_v1_read_dungeon_structure(
    const uint8_t *data, int32_t length,
    const DM2_V1_ReadDungeonStructureCallbacks *cb, void *ctx);

/* =====================================================================
 * SkWinCore2.cpp — PROCESS_PLAGUE
 * ===================================================================== */

typedef struct {
    int16_t (*get_hero_plague)(void *ctx, int hero_idx);
    void (*wound_player)(void *ctx, int hero_idx, int16_t damage);
    void (*set_hero_plague)(void *ctx, int hero_idx, int16_t value);
    int hero_count;
} DM2_V1_ProcessPlagueCallbacks;

void dm2_v1_process_plague(
    const DM2_V1_ProcessPlagueCallbacks *cb, void *ctx);

#endif /* DM2_V1_RUNTIME_PARITY_PC34_COMPAT_H */
