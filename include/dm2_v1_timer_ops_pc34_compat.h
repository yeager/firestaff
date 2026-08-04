#ifndef FIRESTAFF_DM2_V1_TIMER_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TIMER_OPS_PC34_COMPAT_H

/*
 * dm2_v1_timer_ops_pc34_compat.h — DM2 V1 timer handler operations from
 * skproject/SKULLWIN/c_tim_proc.cpp.
 *
 * Callback-based implementations of:
 *   DM2_PROCESS_TIMER_LIGHT               c_tim_proc.cpp:918
 *   DM2_PROCESS_TIMER_RELEASE_DOOR_BUTTON c_tim_proc.cpp:1068
 *   DM2_PROCESS_TIMER_DESTROY_DOOR        c_tim_proc.cpp:422
 *   DM2_PROCESS_TIMER_3D                  c_tim_proc.cpp:902
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_PROCESS_TIMER_LIGHT (c_tim_proc.cpp:918) ----
 * Advance a light decay timer one tick. Updates global light level
 * and re-queues if not fully decayed.
 * intensity: signed timer value (negative = adding light, positive = darkness).
 * light_table: lookup table for light levels (table1d6702).
 * Returns new intensity (0 if fully decayed). */
typedef struct {
    int16_t *global_light;
    void (*queue_light_timer)(void *ctx, int16_t intensity, uint32_t delay);
    const int16_t *light_table;
    int light_table_size;
} DM2_V1_LightTimerCallbacks;

/* Named _tile to avoid an ABI-incompatible symbol collision with the
 * DM2_V1_TimerRecord*-based dm2_v1_process_timer_light() declared in
 * dm2_v1_tim_proc_pc34_compat.h. */
int16_t dm2_v1_process_timer_light_tile(
    int16_t intensity,
    const DM2_V1_LightTimerCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_TIMER_RELEASE_DOOR_BUTTON (c_tim_proc.cpp:1068) ----
 * Clear bit 3 of byte+3 of a record (release door button). */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_RecordAddressCallbacks;

void dm2_v1_process_timer_release_door_button(
    uint16_t record_word,
    const DM2_V1_RecordAddressCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_TIMER_DESTROY_DOOR (c_tim_proc.cpp:422) ----
 * Set tile to destroyed-door state (low 3 bits = 5).
 * Flags redraw if on current map. */
typedef struct {
    uint8_t *(*get_tile_byte)(void *ctx, uint8_t x, uint8_t y);
    int16_t current_map;
    int16_t party_map;
    int *redraw_flags;
} DM2_V1_DestroyDoorCallbacks;

/* Named _tile to avoid an ABI-incompatible symbol collision with the
 * DM2_V1_TimerRecord*-based dm2_v1_process_timer_destroy_door() declared
 * in dm2_v1_tim_proc_pc34_compat.h. */
void dm2_v1_process_timer_destroy_door_tile(
    uint8_t tile_x, uint8_t tile_y,
    const DM2_V1_DestroyDoorCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_TIMER_3D (c_tim_proc.cpp:902) ----
 * Move a record to position and optionally generate noise. */
typedef struct {
    int (*move_record_to)(void *ctx, uint16_t record, int16_t level,
                          int16_t unused, int16_t x, int16_t y);
    void (*queue_noise)(void *ctx, int16_t x, int16_t y);
} DM2_V1_Timer3DCallbacks;

/* Named _tile to avoid an ABI-incompatible symbol collision with the
 * DM2_V1_TimerRecord*-based dm2_v1_process_timer_3d() declared in
 * dm2_v1_tim_proc_pc34_compat.h. */
void dm2_v1_process_timer_3d_tile(
    uint16_t record_word, uint8_t x, uint8_t y, uint8_t timer_type,
    const DM2_V1_Timer3DCallbacks *cb, void *ctx);

/* ---- PROCESS_TIMER_0E (SkWinCore.cpp:2173) ----
 * Temporarily morph an item's type, process bonus, then restore. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    void *(*alloc_memory)(void *ctx, int32_t size);
    void (*dealloc_memory)(void *ctx, void *ptr, int32_t size);
    void (*set_itemtype)(void *ctx, uint16_t record, uint16_t new_type);
    void (*process_item_bonus)(void *ctx, uint8_t actor, uint16_t record,
                               int mode, uint16_t value);
    void (*copy_memory)(void *dst, const void *src, int32_t size);
    int32_t (*get_item_size)(uint16_t db_type);
} DM2_V1_Timer0ECallbacks;

void dm2_v1_process_timer_0e(
    uint16_t record_db_type, uint16_t value2,
    uint8_t actor, uint16_t bonus_value,
    const DM2_V1_Timer0ECallbacks *cb, void *ctx);

/* ---- DM2_CONTINUE_ORNATE_ANIMATOR (c_tim_proc.cpp:961) ----
 * Advance ornate animation frame. Clears active bit when cycle done. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    int16_t (*get_ornate_anim_len)(void *ctx, uint8_t *rec, int mode);
    void (*queue_timer)(void *ctx);
} DM2_V1_OrnateAnimCallbacks;

int dm2_v1_continue_ornate_animator(
    uint16_t record_word, int anim_mode,
    const DM2_V1_OrnateAnimCallbacks *cb, void *ctx);

/* ---- DM2_CONTINUE_TICK_GENERATOR (c_tim_proc.cpp:994) ----
 * Process one tick generator step. Invokes actuator, re-queues if ticks remain. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    void (*invoke_actuator)(void *ctx, uint8_t *rec, uint16_t action, uint16_t param);
    void (*requeue_timer)(void *ctx, uint16_t delay_base, uint8_t multiplier);
} DM2_V1_TickGenCallbacks;

typedef struct {
    uint8_t timer_yb;
    uint8_t timer_b_bit8;
} DM2_V1_TickGenTimerState;

int dm2_v1_continue_tick_generator(
    uint16_t record_word, DM2_V1_TickGenTimerState *timer_state,
    const DM2_V1_TickGenCallbacks *cb, void *ctx);

/* ---- DM2_ACTIVATE_TICK_GENERATOR (c_tim_proc.cpp:1817) ----
 * Start a tick generator timer. Maps subtype to multiplier. */
typedef struct {
    uint32_t game_tick;
    int16_t current_map;
    void (*queue_tick_timer)(void *ctx, uint16_t record_idx, uint8_t multiplier,
                             uint32_t fire_tick);
} DM2_V1_ActivateTickGenCallbacks;

int dm2_v1_activate_tick_generator_cb(
    uint8_t *actuator_record, uint16_t record_idx,
    const DM2_V1_ActivateTickGenCallbacks *cb, void *ctx);

/* ---- SKW_3a15_0d5c (c_tim_proc.cpp:1888) ----
 * Actuator that rotates a creature at target coordinates. */
typedef struct {
    uint16_t (*get_creature_at)(void *ctx, uint16_t x, uint16_t y);
    void (*rotate_creature)(void *ctx, uint16_t creature_rw, int mode, int dir);
} DM2_V1_RotateCreatureActCallbacks;

int dm2_v1_skw_3a15_0d5c(
    const uint8_t *actuator_record, uint8_t timer_yb,
    const DM2_V1_RotateCreatureActCallbacks *cb, void *ctx);

/* ---- DM2_PROCESS_TIMER_0C (c_tim_proc.cpp:30) ----
 * Clear hero timer index; set heroflag 0x800 if alive. */
typedef struct {
    int16_t *hero_timeridx;
    int16_t *hero_curHP;
    uint16_t *hero_heroflag;
} DM2_V1_Timer0CCallbacks;

void dm2_v1_process_timer_0c_cb(
    int16_t hero_index,
    const DM2_V1_Timer0CCallbacks *cb);

/* ---- DM2_PROCESS_SOUND (c_tim_proc.cpp:4066) ----
 * Trigger a sound effect by ID. */
typedef struct {
    void (*play_sound)(void *ctx, int16_t sound_id);
} DM2_V1_SoundTimerCallbacks;

void dm2_v1_process_timer_sound(
    int16_t sound_id,
    const DM2_V1_SoundTimerCallbacks *cb, void *ctx);

/* ---- DM2_UPDATE_WEATHER (c_tim_proc.cpp:4182) ----
 * Trigger weather update. */
typedef struct {
    void (*update_weather)(void *ctx, int16_t mode);
} DM2_V1_WeatherTimerCallbacks;

void dm2_v1_process_timer_update_weather(
    int16_t mode,
    const DM2_V1_WeatherTimerCallbacks *cb, void *ctx);

/* ---- 0x47 HERO_ENCH_FLAG (c_tim_proc.cpp:4112) ----
 * Decrement savegames1.b_02; if zero and hero present, set heroflag 0x4000. */
typedef struct {
    uint8_t *counter;         /* savegames1.b_02 */
    int16_t hero_slot;        /* v1e0976 — hero who cast */
    uint16_t *hero_heroflag;  /* hero[slot].heroflag */
    int16_t *hero_curHP;
} DM2_V1_HeroEnchFlagCallbacks;

void dm2_v1_process_timer_hero_ench_flag(
    const DM2_V1_HeroEnchFlagCallbacks *cb);

/* ---- 0x48 ENCH_POWER (c_tim_proc.cpp:4130) ----
 * Decrement ench_power for party heroes matching actor mask. */
typedef struct {
    int16_t heros_in_party;
    int16_t *hero_ench_power; /* array stride 1 per hero */
    int16_t *hero_curHP;      /* array stride 1 per hero */
} DM2_V1_EnchPowerCallbacks;

void dm2_v1_process_timer_ench_power(
    uint8_t actor_mask, int16_t amount,
    const DM2_V1_EnchPowerCallbacks *cb);

/* ---- 0x4B POISON (c_tim_proc.cpp:4164) ----
 * Decrement hero poison counters and apply poison damage. */
typedef struct {
    int16_t *hero_poisoned;
    int16_t *hero_poison;
    void (*process_poison)(void *ctx, int16_t hero_index, int16_t amount);
} DM2_V1_PoisonTimerCallbacks;

void dm2_v1_process_timer_poison(
    uint8_t actor, int16_t amount,
    const DM2_V1_PoisonTimerCallbacks *cb, void *ctx);

/* ---- 0x59 PROCESS_TIMER_59 (c_tim_proc.cpp:1077) ----
 * Clear record active bit; flag redraw if on current map. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    int16_t current_map;
    int16_t party_map;
    uint8_t *redraw_byte;
} DM2_V1_Timer59Callbacks;

void dm2_v1_process_timer_59(
    uint16_t record_word, int16_t timer_map,
    const DM2_V1_Timer59Callbacks *cb, void *ctx);

/* ---- 0x5D MOVE_RECORD_ROTATE (c_tim_proc.cpp:4230) ----
 * Move a record and rotate party direction. */
typedef struct {
    int (*move_record_to)(void *ctx, uint16_t record, int16_t level,
                          int16_t unused, int16_t x, int16_t y);
    void (*party_rotate)(void *ctx, int16_t direction);
    int16_t party_map;
} DM2_V1_MoveRecordRotateCallbacks;

void dm2_v1_process_timer_move_record_rotate(
    uint16_t value_a, int16_t timer_map,
    int16_t party_x, int16_t party_y,
    const DM2_V1_MoveRecordRotateCallbacks *cb, void *ctx);

/* ---- 0x5E ALLOC_NEW_CREATURE (c_tim_proc.cpp:4253) ----
 * Spawn a new creature at timer coordinates. */
typedef struct {
    int16_t (*alloc_new_creature)(void *ctx, int16_t type, int16_t x_param,
                                   int16_t dir, int16_t x, int16_t y);
    int16_t (*rand_dir)(void *ctx);
    int16_t (*calc_vector_dir)(void *ctx, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    int16_t party_x;
    int16_t party_y;
} DM2_V1_AllocNewCreatureCallbacks;

void dm2_v1_process_timer_alloc_new_creature(
    uint8_t x, uint8_t y, uint8_t creature_type,
    const DM2_V1_AllocNewCreatureCallbacks *cb, void *ctx);

/* ---- 0x0D RESURRECTION (c_tim_proc.cpp:39) ----
 * Three-phase champion resurrection: create cloud, consume the matching
 * altar-list item record, then bring the champion to life.  Driven by
 * the timer's yB phase counter; phases 1/2 decrement yB and re-queue,
 * phase 0 finishes without a re-queue. */
typedef struct {
    void (*bring_champion_to_life)(void *ctx, int16_t actor);
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_next_record_link)(void *ctx, uint16_t record);
    int16_t (*query_cls1_from_record)(void *ctx, int32_t record);
    int16_t (*query_cls2_from_record)(void *ctx, int32_t record);
    int16_t (*add_item_charge)(void *ctx, int32_t record, int mode);
    void (*cut_record_from)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*dealloc_record)(void *ctx, uint16_t record);
    int16_t (*create_cloud)(void *ctx, int16_t type, int16_t param,
                            int16_t x, int16_t y, int16_t cls);
    void (*queue_timer)(void *ctx);
} DM2_V1_ResurrectionCallbacks;

/* Named _tile to avoid an ABI-incompatible symbol collision with the
 * DM2_V1_TimerRecord*-based dm2_v1_process_timer_resurrection() declared
 * in dm2_v1_tim_proc_pc34_compat.h. */
int dm2_v1_process_timer_resurrection_tile(
    uint8_t xA, uint8_t yA, uint8_t xB, uint8_t yB, uint8_t actor,
    const DM2_V1_ResurrectionCallbacks *cb, void *ctx);

/* ---- 0x19 PROCESS_CLOUD (c_cloud.cpp:587) ----
 * Apply one cloud tick: attack doors/party/creatures at the cloud cell,
 * decay type-specific cloud life, re-queue or deallocate. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    uint8_t (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    int16_t (*calc_cloud_damage)(void *ctx, uint16_t cloud_rw, int16_t target);
    void (*attack_door)(void *ctx, int16_t x, int16_t y, int16_t damage,
                        int16_t mode, int16_t param);
    void (*attack_party)(void *ctx, int16_t damage, int16_t mode, int16_t param);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int (*is_creature_immune)(void *ctx, int16_t creature);
    void (*attack_creature)(void *ctx, int16_t creature, int16_t x, int16_t y,
                            int16_t type, int16_t power, int16_t damage);
    void (*cut_record_from)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*dealloc_record)(void *ctx, uint16_t record);
    void (*queue_timer)(void *ctx);
    void (*queue_noise_gen2)(void *ctx, uint8_t cat, uint8_t idx,
                             uint8_t vol, uint8_t pan, int16_t x, int16_t y,
                             int repeat, uint8_t p1, uint8_t p2);
    int16_t current_map;
    int16_t party_map;
    int16_t party_x;
    int16_t party_y;
} DM2_V1_ProcessCloudCallbacks;

int dm2_v1_process_cloud(
    uint16_t record_word, uint8_t xA, uint8_t yA,
    const DM2_V1_ProcessCloudCallbacks *cb, void *ctx);

/* ---- 0x1E STEP_MISSILE (c_tim_proc.cpp:442) ----
 * Advance a missile one tile: wall bounce/deletion, party/creature hit
 * detection (with AI-spec reflection), else move and re-queue.
 * Direction is carried in the record's low 2 bits of byte@4. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    uint8_t (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    int16_t (*query_creature_ai_spec_flags)(void *ctx, int16_t creature);
    int (*move_record)(void *ctx, int16_t level, int16_t x, int16_t y,
                       int16_t dir, uint16_t record);
    void (*cut_record_from)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*append_record_to)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*delete_missile_record)(void *ctx, uint16_t record, int16_t x, int16_t y);
    void (*attack_creature)(void *ctx, int16_t creature, int16_t x, int16_t y,
                            int16_t type, int16_t power, int16_t damage);
    void (*queue_timer)(void *ctx);
    int16_t current_map;
    int16_t party_x;
    int16_t party_y;
    const int16_t *dx_table; /* index by direction 0..3 */
    const int16_t *dy_table;
} DM2_V1_StepMissileCallbacks;

int dm2_v1_step_missile(
    uint16_t record_word, uint8_t x, uint8_t y,
    const DM2_V1_StepMissileCallbacks *cb, void *ctx);

/* ---- 0x5A ORNATE_NOISE (c_tim_proc.cpp:1092) ----
 * When the ornament record is still active, resolve the wall/floor
 * decoration at the timer cell, re-queue with the ornament's animation
 * length, and emit noise on the party's map.  When inactive, clear the
 * frame counter (high byte of word@2). */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t rw);
    uint8_t (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    uint8_t (*get_wall_decoration)(void *ctx, uint8_t *rec);
    uint8_t (*get_floor_decoration)(void *ctx, uint8_t *rec);
    int16_t (*get_ornate_anim_len)(void *ctx, uint8_t *rec, int mode);
    void (*queue_timer_with_data)(void *ctx, int16_t extra_data);
    void (*queue_noise_gen2)(void *ctx, uint8_t cat, uint8_t idx,
                             uint8_t vol, uint8_t pan, int16_t x, int16_t y,
                             int repeat, uint8_t p1, uint8_t p2);
    int16_t party_map;
} DM2_V1_OrnateNoiseCallbacks;

int dm2_v1_continue_ornate_noise(
    uint16_t record_word, uint8_t timer_xA, uint8_t timer_yA,
    int16_t timer_map,
    const DM2_V1_OrnateNoiseCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIMER_OPS_PC34_COMPAT_H */
