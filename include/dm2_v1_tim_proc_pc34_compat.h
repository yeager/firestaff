#ifndef FIRESTAFF_DM2_V1_TIM_PROC_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TIM_PROC_PC34_COMPAT_H

/*
 * dm2_v1_tim_proc_pc34_compat.h — DM2 timer processing dispatcher.
 *
 * Ports c_tim_proc.cpp from skproject.  This module is the main timer
 * event loop (DM2_PROCEED_TIMERS) and all the individual timer-type
 * handlers: door stepping, missile stepping, resurrection, light
 * changes, ornate animators, tick generators, actuator mechanics,
 * pitfalls, teleporters, trickwalls, and the wall/floor mechanism
 * actuators.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Timer event types (settype values) ─────────────────────────── */
typedef enum {
    DM2_TIMER_TYPE_00           = 0x00,
    DM2_TIMER_TYPE_STEP_DOOR    = 0x01,
    DM2_TIMER_TYPE_DESTROY_DOOR = 0x02,
    DM2_TIMER_TYPE_ACTUATE      = 0x04,
    DM2_TIMER_TYPE_0C           = 0x0C,
    DM2_TIMER_TYPE_RESURRECTION = 0x0D,
    DM2_TIMER_TYPE_0E           = 0x0E,
    DM2_TIMER_TYPE_SOUND        = 0x15,
    DM2_TIMER_TYPE_CLOUD        = 0x19,
    DM2_TIMER_TYPE_MISSILE_1D   = 0x1D,
    DM2_TIMER_TYPE_MISSILE_1E   = 0x1E,
    DM2_TIMER_TYPE_THINK_21     = 0x21,
    DM2_TIMER_TYPE_THINK_22     = 0x22,
    DM2_TIMER_TYPE_3C           = 0x3C,
    DM2_TIMER_TYPE_3D           = 0x3D,
    DM2_TIMER_TYPE_LIGHT        = 0x46,
    DM2_TIMER_TYPE_47           = 0x47,
    DM2_TIMER_TYPE_ENCH_POWER   = 0x48,
    DM2_TIMER_TYPE_POISON       = 0x4B,
    DM2_TIMER_TYPE_WEATHER      = 0x54,
    DM2_TIMER_TYPE_ORNATE_ANIM  = 0x55,
    DM2_TIMER_TYPE_TICK_GEN     = 0x56,
    DM2_TIMER_TYPE_DOOR_BUTTON  = 0x58,
    DM2_TIMER_TYPE_59           = 0x59,
    DM2_TIMER_TYPE_ORNATE_NOISE = 0x5A,
    DM2_TIMER_TYPE_5B           = 0x5B,
    DM2_TIMER_TYPE_5C           = 0x5C,
    DM2_TIMER_TYPE_5D           = 0x5D,
    DM2_TIMER_TYPE_5E           = 0x5E
} DM2_V1_TimerType;

/* ── Tile types (upper 3 bits of tile byte) ─────────────────────── */
typedef enum {
    DM2_TILE_WALL       = 0,
    DM2_TILE_FLOOR      = 1,
    DM2_TILE_PIT        = 2,
    DM2_TILE_STAIRS     = 3,
    DM2_TILE_DOOR       = 4,
    DM2_TILE_TELEPORTER = 5,
    DM2_TILE_TRICKWALL  = 6
} DM2_V1_TileType;

/* ── Timer record (portable representation of c_tim) ────────────── */
typedef struct {
    uint8_t  type;
    uint8_t  actor;
    uint8_t  xA;
    uint8_t  yA;
    uint8_t  xB;
    uint8_t  yB;
    uint8_t  map;
    uint16_t valueA;      /* getA()/setA() */
    uint16_t valueB;      /* getB()/setB() */
    int32_t  data;        /* timer data (incdata/adddata) */
    uint32_t ticks;       /* game ticks target */
} DM2_V1_TimerRecord;

/* ── Callback struct — all external dependencies ────────────────── */
typedef struct DM2_V1_TimProcCallbacks {
    /* Timer queue operations */
    bool (*is_timer_to_proceed)(void *ctx);
    void (*get_and_delete_next_timer)(DM2_V1_TimerRecord *out, void *ctx);
    int16_t (*queue_timer)(const DM2_V1_TimerRecord *tim, void *ctx);

    /* Map operations */
    void (*change_current_map_to)(int32_t map_id, void *ctx);
    uint8_t (*get_tile_value)(int32_t x, int32_t y, void *ctx);
    uint8_t *(*get_address_of_tile_record)(int16_t x, int16_t y, void *ctx);
    int16_t (*get_tile_record_link)(int16_t x, int16_t y, void *ctx);
    int32_t (*get_next_record_link)(uint16_t record, void *ctx);
    uint8_t *(*get_address_of_record)(uint16_t record, void *ctx);
    int32_t (*get_wall_tile_anyitem_record)(int32_t x, int16_t y, void *ctx);

    /* Record operations */
    void (*cut_record_from)(int32_t record, void *unused, int16_t x,
                            int16_t y, void *ctx);
    void (*append_record_to)(int32_t record, void *unused, int16_t x,
                             int16_t y, void *ctx);
    void (*move_record_to)(int32_t record, int32_t x, int32_t y,
                           int32_t dx, int16_t dy, void *ctx);
    void (*dealloc_record)(int32_t record, void *ctx);
    int32_t (*alloc_new_record)(int32_t db, void *ctx);
    int32_t (*alloc_new_dbitem)(int32_t type, void *ctx);

    /* Door operations */
    int32_t (*query_door_damage_resist)(int32_t is_rebirth, void *ctx);
    int32_t (*is_rebirth_altar)(uint8_t *addr, void *ctx);

    /* Creature operations */
    int16_t (*get_creature_at)(int32_t x, int32_t y, void *ctx);
    int16_t (*query_creature_ai_spec_flags)(int32_t creature, void *ctx);
    uint8_t *(*query_creature_ai_spec_from_type)(int32_t creature, void *ctx);
    void (*attack_creature)(int32_t creature, int32_t x, int32_t y,
                            int32_t flags, int32_t param1, int32_t param2,
                            void *ctx);
    void (*rotate_creature)(int32_t creature, int32_t mode, int32_t dir,
                            void *ctx);
    int16_t (*alloc_new_creature)(int16_t type, int16_t param, int16_t dir,
                                 int16_t x, int16_t y, void *ctx);
    void (*think_creature)(int32_t x, int32_t y, int32_t type, void *ctx);

    /* Hero/party operations */
    int32_t (*attack_party)(int32_t resist, int32_t flags, int32_t mode,
                            void *ctx);
    void (*bring_champion_to_life)(int16_t index, void *ctx);
    int16_t (*get_heros_in_party)(void *ctx);
    int16_t (*get_hero_curHP)(int32_t index, void *ctx);

    /* Item operations */
    int16_t (*query_cls1_from_record)(int32_t record, void *ctx);
    int16_t (*query_cls2_from_record)(int32_t record, void *ctx);
    int16_t (*add_item_charge)(int32_t record, int32_t param, void *ctx);
    void (*delete_missile_record)(int32_t record, void *unused, int32_t x,
                                  int32_t y, void *ctx);
    void (*shoot_item)(int32_t item, int32_t x, int32_t y, int32_t dir,
                       int32_t facing, int8_t p1, int32_t p2, int8_t p3,
                       void *ctx);
    void (*set_itemtype)(int32_t record, int32_t type, void *ctx);
    void (*set_item_importance)(int32_t record, int32_t importance, void *ctx);
    int32_t (*get_itemdb_of_itemspec_actuator)(int32_t spec, void *ctx);
    int32_t (*get_itemtype_of_itemspec_actuator)(int32_t spec, void *ctx);

    /* Cloud operations */
    int32_t (*create_cloud)(int32_t type, int32_t param, int32_t x,
                            int32_t y, int32_t dir, void *ctx);
    void (*process_cloud)(DM2_V1_TimerRecord *tim, void *ctx);

    /* Light operations */
    void (*recalc_light_level)(void *ctx);

    /* Sound operations */
    void (*queue_noise_gen1)(int32_t a, int8_t b, int8_t c, int16_t d,
                             int16_t e, int16_t f, int16_t g, int32_t h,
                             void *ctx);
    void (*queue_noise_gen2)(int32_t a, int8_t b, int8_t c, int8_t d,
                             int16_t e, int16_t f, int32_t g, int32_t h,
                             int32_t i, void *ctx);
    void (*process_sound)(int16_t value, void *ctx);

    /* Actuator operations */
    void (*invoke_actuator)(uint8_t *addr, int32_t param, int32_t extra,
                            void *ctx);
    void (*invoke_message)(int32_t x, int32_t y, int32_t dir, int32_t action,
                           int32_t ticks, void *ctx);
    int32_t (*get_ornate_anim_len)(uint8_t *addr, int32_t param, void *ctx);
    int8_t (*get_floor_decoration_of_actuator)(uint8_t *addr, void *ctx);
    int8_t (*get_wall_decoration_of_actuator)(uint8_t *addr, void *ctx);
    int16_t (*query_gdat_entry_data_index)(int8_t a, int8_t b, int32_t c,
                                           int32_t d, void *ctx);

    /* Missile movement */
    int32_t (*move_075f_0af9)(int32_t type, int32_t x, int32_t y,
                              int32_t dir, int32_t record, void *ctx);

    /* Misc */
    void (*ibmio_user_input_check)(void *ctx);
    int32_t (*rand16)(int32_t limit, void *ctx);
    int32_t (*rand)(void *ctx);
    bool (*randbit)(void *ctx);
    int16_t (*randdir)(void *ctx);
    int32_t (*get_glob_var)(int32_t index, void *ctx);
    void (*update_glob_var)(int32_t index, int32_t mode, int32_t value,
                            void *ctx);
    void (*update_weather)(int32_t param, void *ctx);
    void (*process_item_bonus)(int32_t actor, int32_t record, int32_t p1,
                               int32_t p2, void *ctx);
    void (*process_poison)(int32_t hero, int32_t amount, void *ctx);
    void (*prepare_exit)(void *ctx);
    void (*select_palette_set)(int32_t set, void *ctx);
    void (*display_hint_text)(int32_t color, const char *text, void *ctx);
    void (*query_message_text)(char *buf, int32_t record, int32_t param,
                               void *ctx);
    int32_t (*locate_other_level)(int32_t map, int32_t dir, int16_t *x,
                                  int16_t *y, void *unused, void *ctx);
    int16_t (*calc_vector_dir)(int16_t x1, int16_t y1, int16_t x2,
                               int16_t y2, void *ctx);
    void (*activate_item_teleport)(DM2_V1_TimerRecord *tim, uint8_t *addr,
                                   int32_t p1, int32_t p2, int32_t p3,
                                   void *p4, int32_t p5, int32_t p6,
                                   void *ctx);
    void (*move_item_to)(int32_t record, uint8_t *addr, int32_t p1,
                         int32_t p2, int16_t x, int16_t y, int32_t dir,
                         int32_t p5, int32_t p6, void *ctx);
    int32_t (*dm2_1c9a_09b9)(int32_t a, int32_t b, void *ctx);
    void (*ai_13e4_0360)(int32_t a, int32_t b, int32_t c, int32_t d,
                          int32_t e, void *ctx);
    void (*ai_13e4_071b)(void *ctx);
    void (*ai_13e4_0806)(void *ctx);
    void *(*prepare_local_creature_var)(int32_t creature, int32_t x,
                                        int32_t y, int32_t mode, void *ctx);
    void (*unprepare_local_creature_var)(void *var, void *ctx);
    void (*alloc_caii_to_creature)(int32_t creature, int32_t x, int32_t y,
                                   void *ctx);
    bool (*creatures_exist)(void *ctx);
    void (*map_3bf83)(int32_t x, int32_t y, int32_t map, int32_t dir,
                      void *ctx);
    void (*copy_memory)(void *dst, void *src, int32_t size, void *ctx);
    void *(*alloc_lobigpool_memory)(int32_t size, void *ctx);
    void (*dealloc_lobigpool)(int32_t size, void *ctx);
    int32_t (*get_record_size)(int32_t type, void *ctx);

    /* Game state accessors */
    int16_t (*get_current_map)(void *ctx);
    int16_t (*get_party_map)(void *ctx);
    int16_t (*get_party_x)(void *ctx);
    int16_t (*get_party_y)(void *ctx);
    int16_t (*get_party_dir)(void *ctx);
    int32_t (*get_gametick)(void *ctx);
    int16_t (*get_map_width)(void *ctx);
    int16_t (*get_map_height)(void *ctx);
    int16_t (*get_light_table_entry)(int32_t index, void *ctx);

    /* Render invalidation */
    void (*set_render_flag)(int32_t value, void *ctx);

} DM2_V1_TimProcCallbacks;

/* ── Direction tables (matching skproject table1d27fc / table1d2804) ── */
extern const int16_t dm2_v1_tim_proc_dir_dx[4]; /* { 0, 1, 0, -1 } */
extern const int16_t dm2_v1_tim_proc_dir_dy[4]; /* { -1, 0, 1, 0 } */

/* ── Receipt structs ────────────────────────────────────────────── */

typedef struct {
    int valid;
    int timer_type_dispatched;
    int timers_processed;
    int fail_closed;
} DM2_V1_ProceedTimersReceipt;

typedef struct {
    int valid;
    int hero_flag_set;
    int hero_index;
    int fail_closed;
} DM2_V1_ProcessTimer0CReceipt;

typedef struct {
    int valid;
    int champion_brought_to_life;
    int cloud_created;
    int records_removed;
    int requeued;
    int fail_closed;
} DM2_V1_ProcessTimerResurrectionReceipt;

typedef struct {
    int valid;
    int door_destroyed;
    int fail_closed;
} DM2_V1_ProcessTimerDestroyDoorReceipt;

typedef struct {
    int valid;
    int door_stepped;
    int door_opened;
    int door_closed;
    int creature_attacked;
    int party_attacked;
    int requeued;
    int fail_closed;
} DM2_V1_StepDoorReceipt;

typedef struct {
    int valid;
    int missile_moved;
    int missile_destroyed;
    int missile_reflected;
    int requeued;
    int fail_closed;
} DM2_V1_StepMissileReceipt;

typedef struct {
    int valid;
    int record_moved;
    int noise_queued;
    int fail_closed;
} DM2_V1_ProcessTimer3DReceipt;

typedef struct {
    int valid;
    int light_changed;
    int requeued;
    int fail_closed;
} DM2_V1_ProcessTimerLightReceipt;

typedef struct {
    int valid;
    int value_computed;
    int32_t result;
} DM2_V1_TimProc1DA8Receipt;

typedef struct {
    int valid;
    int message_invoked;
    int fail_closed;
} DM2_V1_InvokeMessageReceipt;

typedef struct {
    int valid;
    int actuator_invoked;
    int fail_closed;
} DM2_V1_InvokeActuatorReceipt;

/* ── Public function declarations ───────────────────────────────── */

/* Main timer processing loop */
void dm2_v1_proceed_timers(
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProceedTimersReceipt *receipt);

/* Individual timer handlers */
void dm2_v1_process_timer_0c_receipt(
    int16_t hero_index,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimer0CReceipt *receipt);

void dm2_v1_process_timer_resurrection(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerResurrectionReceipt *receipt);

void dm2_v1_process_timer_destroy_door(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerDestroyDoorReceipt *receipt);

void dm2_v1_step_door(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_StepDoorReceipt *receipt);

void dm2_v1_step_missile(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_StepMissileReceipt *receipt);

void dm2_v1_process_timer_3d(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimer3DReceipt *receipt);

void dm2_v1_process_timer_light(
    DM2_V1_TimerRecord *tim,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_ProcessTimerLightReceipt *receipt);

/* Helper: compute timer flag value (SKW_3a15_1da8) */
int32_t dm2_v1_timproc_compute_flag(int32_t yB_value, int32_t current_flag,
    DM2_V1_TimProc1DA8Receipt *receipt);

/* Invoke message / actuator */
void dm2_v1_invoke_message(
    int32_t x, int32_t y, int32_t dir, int32_t action, int32_t ticks,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_InvokeMessageReceipt *receipt);

void dm2_v1_invoke_actuator_receipt(
    uint8_t *addr, int32_t param, int32_t extra,
    const DM2_V1_TimProcCallbacks *cb, void *ctx,
    DM2_V1_InvokeActuatorReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TIM_PROC_PC34_COMPAT_H */
