/* DM2 V1 creature-vs-creature combat and confusion.
 * skproject c_creature.cpp: lines 34-56, 906-998, 1308-1341. */

#include "dm2_v1_creature_combat_pc34_compat.h"
#include <string.h>

/* c_creature.cpp:906-998 — DM2_CREATURE_ATTACKS_CREATURE */
int32_t dm2_v1_creature_attacks_creature(
    const DM2_V1_CreatureAttacksCreatureState *state,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksCreatureReceipt *receipt)
{
    if (!receipt) return -1;
    memset(receipt, 0, sizeof(*receipt));

    if (!state || !cb) {
        receipt->fail_closed = 1;
        return -1;
    }

    receipt->valid = 1;

    /* c_creature.cpp:923-926 — find creature at target position */
    if (!cb->get_creature_at)
        return -1;
    int16_t target = cb->get_creature_at(ctx, state->src_x, state->src_y);
    receipt->target_record = target;
    if (target == -1)
        return -1;

    /* c_creature.cpp:928-935 — get target AI spec, check hit chance */
    if (!cb->get_record_address || !cb->query_ai_spec || !cb->rand_fn)
        return 0;

    uint8_t *target_rec = cb->get_record_address(ctx, (uint16_t)target);
    if (!target_rec) return 0;

    const uint8_t *target_spec = cb->query_ai_spec(ctx, target_rec[4]);
    if (!target_spec) return 0;

    int16_t target_hit = (int16_t)(uint8_t)target_spec[8];
    if (target_hit == 0xFF)
        return 0;

    /* c_creature.cpp:936-953 — hit determination */
    int16_t r1 = cb->rand_fn(ctx);
    r1 &= 0x1F;

    int16_t attacker_hit = state->attacker_ai_spec ?
        (int16_t)(uint8_t)state->attacker_ai_spec[8] : 0;
    int32_t attack_total = (int32_t)r1 + (int32_t)attacker_hit;

    int16_t r2 = cb->rand_fn(ctx);
    r2 &= 0x1F;
    int32_t defense_total = (int32_t)(uint16_t)target_hit + (int32_t)r2;

    if ((int16_t)defense_total > (int16_t)attack_total) {
        int16_t rdir = cb->randdir ? cb->randdir(ctx) : 0;
        if (rdir != 0)
            return 0;
    }

    /* c_creature.cpp:954-966 — compute base damage */
    int16_t r3 = cb->rand_fn(ctx);
    r3 &= 0x1F;
    int16_t target_armor = target_spec ? (int16_t)(uint8_t)target_spec[2] : 0;
    int32_t armor_val = ((int32_t)(uint16_t)target_armor + (int32_t)r3) / 8;

    int16_t attacker_str = state->attacker_ai_spec ?
        (int16_t)(uint8_t)state->attacker_ai_spec[6] : 0;
    int16_t r4 = cb->rand_fn(ctx);
    r4 &= 0xF;
    int16_t base_min = cb->min_fn ?
        cb->min_fn(attacker_str, r4) : (r4 < attacker_str ? r4 : attacker_str);
    int16_t damage = (int16_t)(attacker_str + base_min - (int16_t)armor_val);

    if (damage <= 1) {
        int rb = cb->randbit ? cb->randbit(ctx) : 0;
        if (rb != 0)
            return 0;
        int16_t bonus = cb->randdir ? cb->randdir(ctx) : 0;
        damage = (int16_t)(bonus + 2);
    }

    /* c_creature.cpp:977-991 — randomize final damage */
    int16_t rv1 = cb->rand16 ? cb->rand16(ctx, damage) : 0;
    int16_t rv2 = cb->randdir ? cb->randdir(ctx) : 0;
    damage = (int16_t)(damage + rv1 + rv2);
    rv1 = cb->rand16 ? cb->rand16(ctx, damage) : 0;
    damage = (int16_t)(damage + rv1);
    damage >>= 2;
    rv2 = cb->randdir ? cb->randdir(ctx) : 0;
    damage = (int16_t)(damage + rv2 + 1);
    int rb2 = cb->randbit ? cb->randbit(ctx) : 0;
    if (rb2 != 0) {
        rv1 = cb->rand16 ? cb->rand16(ctx, (int16_t)(damage / 4 + 1)) : 0;
        damage = (int16_t)(damage - rv1);
    }

    /* c_creature.cpp:996 — apply damage via ATTACK_CREATURE */
    if (cb->attack_creature) {
        cb->attack_creature(ctx, (uint16_t)target,
            state->src_x, state->src_y, 2, 0x3C, (int32_t)damage);
        receipt->attacked = 1;
    }

    receipt->damage_dealt = damage;
    return (int32_t)damage;
}

/* c_creature.cpp:1308-1341 — DM2_CONFUSE_CREATURE */
int32_t dm2_v1_confuse_creature(
    const DM2_V1_ConfuseCreatureState *state,
    const DM2_V1_ConfuseCreatureCallbacks *cb,
    void *ctx,
    DM2_V1_ConfuseCreatureReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!state || !cb) return 0;
    receipt->valid = 1;

    if (state->v1e0b4c == 0xFFFF)
        return 0;

    if (!cb->get_record_address || !cb->query_ai_spec || !cb->rand16)
        return 0;

    uint8_t *rec = cb->get_record_address(ctx, state->v1e0b4c);
    if (!rec) return 0;

    const uint8_t *spec = cb->query_ai_spec(ctx, rec[4]);
    if (!spec) return 0;

    /* c_creature.cpp:1325-1330 — resistance check */
    int16_t resist = (int16_t)((spec[0x16] | (spec[0x17] << 8)) >> 4) & 0xF;
    int16_t roll = cb->rand16(ctx, state->confusion_power);
    if (resist > (uint16_t)roll)
        return 0;
    if (resist == 0xF)
        return 0;

    /* c_creature.cpp:1332-1336 — attack the confused creature */
    if (cb->attack_creature) {
        cb->attack_creature(ctx, state->v1e0b4c,
            state->src_x, state->src_y, 0x2005, 0x64, 0);
        receipt->confused = 1;
    }

    return 1;
}

/* c_creature.cpp:34-56 — DM2_APPLY_CREATURE_POISON_RESISTANCE */
int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_record, int16_t poison_amount,
    const DM2_V1_PoisonResistCallbacks *cb, void *ctx)
{
    if (poison_amount == 0)
        return 0;

    if (!cb || !cb->query_ai_spec_from_type || !cb->randdir)
        return 0;

    const uint8_t *spec = cb->query_ai_spec_from_type(ctx, creature_record);
    if (!spec) return 0;

    /* c_creature.cpp:44-46 — extract resistance nibble from word at 0x18 */
    uint16_t w18 = (uint16_t)(spec[0x18] | (spec[0x19] << 8));
    int16_t resist = (int16_t)((w18 >> 8) & 0xF);

    if (resist == 0xF)
        return 0;

    /* c_creature.cpp:50-55 — compute reduced poison */
    int16_t rdir = cb->randdir(ctx);
    int32_t numerator = ((int32_t)(uint16_t)poison_amount + (int32_t)rdir) << 3;
    int32_t denominator = (int32_t)resist + 2;
    return (int16_t)(numerator / denominator);
}
