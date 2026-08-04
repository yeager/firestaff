/* DM2 V1 creature-vs-creature combat and confusion.
 * skproject c_creature.cpp: CREATURE_ATTACKS_CREATURE, CONFUSE_CREATURE,
 * APPLY_CREATURE_POISON_RESISTANCE. */

#ifndef DM2_V1_CREATURE_COMBAT_PC34_COMPAT_H
#define DM2_V1_CREATURE_COMBAT_PC34_COMPAT_H

#include <stdint.h>

/* --- CREATURE_ATTACKS_CREATURE (c_creature.cpp:906-998) --- */

typedef struct {
    int16_t (*rand_fn)(void *ctx);
    int16_t (*rand16)(void *ctx, int16_t max);
    int16_t (*randdir)(void *ctx);
    int     (*randbit)(void *ctx);
    int16_t (*min_fn)(int16_t a, int16_t b);
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t dir);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record);
    const uint8_t *(*query_ai_spec)(void *ctx, uint8_t creature_type);
    void    (*attack_creature)(void *ctx, uint16_t target_record,
                               int16_t src_x, int16_t src_y,
                               int16_t mode, int16_t threshold, int32_t damage);
} DM2_V1_CreatureAttacksCreatureCallbacks;

typedef struct {
    int16_t src_x;
    int16_t src_y;
    const uint8_t *attacker_ai_spec;
} DM2_V1_CreatureAttacksCreatureState;

typedef struct {
    int valid;
    int fail_closed;
    int16_t damage_dealt;
    int16_t target_record;
    int attacked;
} DM2_V1_CreatureAttacksCreatureReceipt;

int32_t dm2_v1_creature_attacks_creature(
    const DM2_V1_CreatureAttacksCreatureState *state,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksCreatureReceipt *receipt);

/* --- CONFUSE_CREATURE (c_creature.cpp:1308-1341) --- */

typedef struct {
    int16_t (*rand16)(void *ctx, int16_t max);
    uint8_t *(*get_record_address)(void *ctx, uint16_t record);
    const uint8_t *(*query_ai_spec)(void *ctx, uint8_t creature_type);
    void    (*attack_creature)(void *ctx, uint16_t target_record,
                               int16_t src_x, int16_t src_y,
                               int16_t mode, int16_t threshold, int32_t damage);
} DM2_V1_ConfuseCreatureCallbacks;

typedef struct {
    int16_t confusion_power;
    int16_t src_x;
    int16_t src_y;
    uint16_t v1e0b4c;
} DM2_V1_ConfuseCreatureState;

typedef struct {
    int valid;
    int confused;
} DM2_V1_ConfuseCreatureReceipt;

int32_t dm2_v1_confuse_creature(
    const DM2_V1_ConfuseCreatureState *state,
    const DM2_V1_ConfuseCreatureCallbacks *cb,
    void *ctx,
    DM2_V1_ConfuseCreatureReceipt *receipt);

/* --- APPLY_CREATURE_POISON_RESISTANCE (c_creature.cpp:34-56) --- */

typedef struct {
    int16_t (*randdir)(void *ctx);
    const uint8_t *(*query_ai_spec_from_type)(void *ctx, uint16_t record);
} DM2_V1_PoisonResistCallbacks;

int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_record, int16_t poison_amount,
    const DM2_V1_PoisonResistCallbacks *cb, void *ctx);

#endif
