#ifndef FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H

/*
 * dm2_v1_creature_ops_pc34_compat.h — DM2 V1 creature operations from
 * skproject/SKULLWIN/c_creature.cpp.
 *
 * Callback-based implementations of:
 *   DM2_APPLY_CREATURE_POISON_RESISTANCE  c_creature.cpp:34
 *   DM2_ROTATE_CREATURE                   c_creature.cpp:58
 *   DM2_CREATURE_CAN_HANDLE_ITEM_IN      c_creature.cpp:104
 *   DM2_CONFUSE_CREATURE                  c_creature.cpp:1308
 *   DM2_CREATURE_KILL_ON_TIMER_POSITION   c_creature.cpp:2352
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- DM2_APPLY_CREATURE_POISON_RESISTANCE (c_creature.cpp:34) ----
 * Applies creature's poison resistance to incoming damage.
 * Returns reduced poison amount, 0 if immune. */
typedef struct {
    const uint8_t *(*query_ai_spec)(void *ctx, uint16_t creature_type);
    uint16_t (*rand_dir)(void *ctx);
} DM2_V1_CreaturePoisonCallbacks;

int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_type, int16_t poison_amount,
    const DM2_V1_CreaturePoisonCallbacks *cb, void *ctx);

/* ---- DM2_ROTATE_CREATURE (c_creature.cpp:58) ----
 * Rotates a creature record's facing direction.
 * mode 0: add delta to current facing.
 * mode != 0: set to absolute direction.
 * Also rotates all items in creature's possession chain if AI flag 0x01 set. */
typedef struct {
    uint8_t *(*get_record_address)(void *ctx, uint16_t record_word);
    uint16_t (*query_ai_spec_flags)(void *ctx, uint16_t record_word);
    uint8_t *(*get_next_record_address)(void *ctx, uint16_t record_word);
} DM2_V1_CreatureRotateCallbacks;

void dm2_v1_rotate_creature(
    uint16_t creature_record, int mode, int direction,
    const DM2_V1_CreatureRotateCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CAN_HANDLE_ITEM_IN (c_creature.cpp:104) ----
 * Walk a record chain, find the first item (types 5..13 or type 9)
 * matching a direction filter that the creature can handle.
 * direction_filter 0xFF = any direction.
 * Returns the matching record word, or 0xFFFE (END) if none. */
typedef struct {
    int16_t (*get_next_record_link)(void *ctx, uint16_t record_word);
    int (*creature_can_handle_it)(void *ctx, uint16_t record_word,
                                  int16_t creature_type);
} DM2_V1_CreatureHandleCallbacks;

int16_t dm2_v1_creature_can_handle_item_in(
    int16_t creature_type, int16_t first_record, uint8_t direction_filter,
    const DM2_V1_CreatureHandleCallbacks *cb, void *ctx);

/* ---- DM2_CONFUSE_CREATURE (c_creature.cpp:1308) ----
 * Confuse a creature: query AI spec confusion resistance (word@0x16
 * bits 4..7), roll rand16(power) against it. If roll > resistance and
 * resistance != 0xF, call ATTACK_CREATURE with type=0x2005 sound=0x64.
 * Returns 1 if the attack was dispatched, 0 otherwise. */
typedef struct {
    uint16_t confuser_record;   /* ddat.v1e0b4c */
    uint8_t *(*get_record_address)(void *ctx, uint16_t record);
    uint8_t (*get_creature_type)(void *ctx, uint16_t record);
    uint16_t (*query_ai_spec_resistance)(void *ctx, uint8_t creature_type);
    int16_t (*rand16)(void *ctx, int16_t max);
    void (*attack_creature)(void *ctx, uint16_t target, int16_t x, int16_t y,
                            int16_t attack_type, int16_t sound, int32_t damage);
} DM2_V1_ConfuseCreatureCallbacks;

int dm2_v1_confuse_creature(
    int16_t power, int16_t x, int16_t y, int32_t damage,
    const DM2_V1_ConfuseCreatureCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_KILL_ON_TIMER_POSITION (c_creature.cpp:2352) ----
 * Delete creature at timer position and set flag. */
typedef struct {
    void (*delete_creature_record)(void *ctx, uint16_t x, uint16_t y,
                                    int param1, int param2);
    int *v1e0570_flag;
} DM2_V1_CreatureKillCallbacks;

void dm2_v1_creature_kill_on_timer_position(
    uint16_t x, uint16_t y,
    const DM2_V1_CreatureKillCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_CCM06 (c_creature.cpp:1574) ----
 * Creature turns to face target. Randomizes if behind. */
typedef struct {
    uint8_t creature_facing;
    uint8_t target_dir;
    int (*rand_fn)(void *ctx);
} DM2_V1_CreatureCCM06Callbacks;

uint8_t dm2_v1_creature_ccm06(
    const DM2_V1_CreatureCCM06Callbacks *cb, void *ctx);

/* ---- DM2_1B7D5 (c_creature.cpp:2916) ----
 * Create cloud at creature position with random strength. */
typedef struct {
    int16_t (*rand16)(void *ctx, int16_t max);
    int (*create_cloud)(void *ctx, int spell_id, uint16_t strength,
                        uint16_t x, uint16_t y, int param);
} DM2_V1_CreateCloudCallbacks;

int dm2_v1_creature_create_cloud(
    uint16_t x, uint16_t y,
    const DM2_V1_CreateCloudCallbacks *cb, void *ctx);

/* ---- DM2_CREATURE_SHOOT_ITEM (c_creature.cpp:2359) ----
 * Creature fires a projectile from inventory. */
typedef struct {
    int16_t (*find_shootable_item)(void *ctx, int8_t capability, uint16_t inv_link,
                                    int filter);
    void (*cut_record)(void *ctx, uint16_t record, uint16_t *from_link,
                       int x, int y);
    int16_t (*rand16)(void *ctx, int16_t max);
    int16_t (*between_value)(int16_t lo, int16_t hi, int16_t val);
    void (*shoot_item)(void *ctx, uint16_t item, uint16_t x, uint16_t y,
                       int direction, int facing, uint8_t damage,
                       uint8_t speed, int type);
} DM2_V1_CreatureShootCallbacks;

typedef struct {
    int8_t capability;
    uint16_t inv_link;
    uint16_t x;
    uint16_t y;
    uint8_t creature_dir;
    uint8_t creature_facing;
    uint8_t attack_stat;
    uint8_t speed_stat;
} DM2_V1_CreatureShootState;

int dm2_v1_creature_shoot_item(
    const DM2_V1_CreatureShootState *state,
    const DM2_V1_CreatureShootCallbacks *cb, void *ctx);

/* ---- DM2_ATTACK_PARTY (c_hero.cpp:3346) ----
 * Distribute creature attack across all living party heroes. */
typedef struct {
    int hero_count;
    int16_t (*rand16)(void *ctx, int16_t max);
    int (*is_hero_alive)(void *ctx, int idx);
    void (*wound_player)(void *ctx, int hero_idx, int16_t damage,
                         int body_parts, int wound_type);
} DM2_V1_AttackPartyCallbacks;

uint8_t dm2_v1_attack_party(
    int16_t total_damage, int body_parts, int wound_type,
    const DM2_V1_AttackPartyCallbacks *cb, void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_CREATURE_OPS_PC34_COMPAT_H */
