/* DM2 V1 CREATURE_ATTACKS_PLAYER — skproject c_creature.cpp:651-903.
 * Ported from register-machine code to clean callback-based C. */

#include "dm2_v1_creature_attacks_player_pc34_compat.h"
#include <string.h>

/* c_creature.cpp uses table1d26f8 for wound type selection */
const uint8_t dm2_v1_table1d26f8[3] = { 0x01, 0x02, 0x04 };

int32_t dm2_v1_creature_attacks_player_receipt(
    const DM2_V1_CreatureAttacksPlayerState *state,
    const DM2_V1_CreatureAttacksPlayerReceiptCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksPlayerReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!state || !cb) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_creature.cpp:671-676 — validate hero index and HP */
    if ((uint16_t)state->hero_idx >= (uint16_t)state->heros_in_party)
        return 0;
    if (state->hero_cur_hp == 0)
        return 0;

    /* c_creature.cpp:678-684 — extract creature data word 0x0c bits */
    int16_t damage_result = 0;
    int32_t vl_04 = 0;
    if (state->creature_data) {
        uint16_t w0c = (uint16_t)(state->creature_data[0x0c] |
                                   (state->creature_data[0x0d] << 8));
        uint16_t shifted = w0c >> 12;
        vl_04 = 2 * (int32_t)shifted;
    }

    /* c_creature.cpp:683-687 — get AI spec, extract attack type (byte 0x1c) */
    const uint8_t *ai_spec = NULL;
    if (cb->query_ai_spec && state->creature_record)
        ai_spec = cb->query_ai_spec(ctx, state->creature_record[4]);

    int16_t attack_type = 0;
    if (ai_spec)
        attack_type = (int16_t)(uint8_t)ai_spec[0x1c];

    /* c_creature.cpp:688-701 — compute base defense modifier */
    int16_t defense_mod = 0;
    if (state->savegames1_b02 == 0 || (ai_spec && (ai_spec[1] & 0x4) != 0)) {
        if (ai_spec && (ai_spec[1] & 0x8) == 0)
            defense_mod = (int16_t)(2 * state->v1e0286);
    } else {
        defense_mod = 0x10;
    }

    /* c_creature.cpp:703-713 — override hit chance for attack types 8/9 */
    int16_t hit_chance = 0;
    if (ai_spec)
        hit_chance = (int16_t)(uint8_t)ai_spec[8];

    if (attack_type == 9) {
        int16_t doubled = (int16_t)(2 * hit_chance);
        hit_chance = cb->min_fn ? cb->min_fn(0xFF, doubled) : (doubled > 0xFF ? 0xFF : doubled);
    } else if (attack_type == 8) {
        hit_chance = 0xFF;
    }

    /* c_creature.cpp:715-750 — hit/miss determination */
    int hit = 0;
    if (state->v1e0238 != 0) {
        hit = 1;
    } else if (hit_chance == 0xFF) {
        hit = 1;
    } else {
        if (!cb->rand_fn || !cb->use_dexterity_attribute) {
            receipt->fail_closed = 1;
            return 0;
        }
        int16_t r = cb->rand_fn(ctx);
        r &= 0x1F;
        r = (int16_t)(r + hit_chance);
        r = (int16_t)(r + (int16_t)vl_04);
        int32_t total = (int32_t)(uint16_t)defense_mod + (int32_t)(int16_t)r;
        int16_t dex = cb->use_dexterity_attribute(ctx, state->hero_idx);
        int32_t threshold = total - 16;
        if ((int32_t)(uint16_t)dex < threshold) {
            hit = 0;
        } else {
            int16_t rdir = cb->randdir ? cb->randdir(ctx) : 0;
            if (rdir == 0)
                hit = 0;
            else
                hit = 1;
        }

        if (!hit) {
            if (cb->use_luck) {
                if (!cb->use_luck(ctx, state->hero_idx, 0x3C))
                    hit = 1;
                else {
                    receipt->dodged = 1;
                    return 0;
                }
            }
        }
    }

    if (!hit) {
        receipt->dodged = 1;
        return 0;
    }

    receipt->hit = 1;

    /* c_creature.cpp:756-808 — determine wound type */
    int16_t wound_type;
    if (!cb->rand_fn) {
        receipt->fail_closed = 1;
        return 0;
    }

    {
        int16_t r1 = cb->rand_fn(ctx);
        int16_t r2 = r1;
        int16_t masked = (int16_t)((r1 ^ (r1 >> 8)) & 0x70);
        int use_default = 0;

        if (masked == 0) {
            use_default = 1;
        } else {
            int16_t spec_1a = ai_spec ? (int16_t)(ai_spec[0x1a] | (ai_spec[0x1b] << 8)) : 0;
            if (spec_1a == 0) {
                use_default = 1;
            } else {
                int16_t r2_masked = (int16_t)((r2 ^ (r2 >> 8)) & 0xF);
                if (r2_masked == 0) r2_masked = 1;
                int16_t idx = 0;
                for (;;) {
                    if (idx >= 3) break;
                    int16_t nibble = (int16_t)(spec_1a & 0xF);
                    if (r2_masked <= nibble) break;
                    spec_1a >>= 4;
                    idx++;
                }
                wound_type = (int16_t)dm2_v1_table1d26f8[idx < 3 ? idx : 2];
            }
        }

        if (use_default) {
            wound_type = (int16_t)((r2 ^ (r2 >> 8)) & 0x1) + 1;
        }
    }

    /* c_creature.cpp:811-860 — compute damage */
    int16_t strength = ai_spec ? (int16_t)(uint8_t)ai_spec[6] : 0;
    {
        int16_t r = cb->rand_fn(ctx);
        r &= 0xF;
        r = (int16_t)(r + (int16_t)vl_04);
        int16_t base = cb->min_fn ?
            cb->min_fn(strength, (int16_t)r) : (r < strength ? r : strength);
        damage_result = (int16_t)(strength + base);
    }

    int compute_damage = 0;
    if (attack_type == 8) {
        compute_damage = 1;
    } else {
        int16_t armor_skill = cb->query_player_skill_lv ?
            cb->query_player_skill_lv(ctx, state->hero_idx, 7, 1) : 0;
        damage_result = (int16_t)(damage_result - 2 * armor_skill);
        if (damage_result > 1) {
            compute_damage = 1;
        } else {
            int rb = cb->randbit ? cb->randbit(ctx) : 0;
            if (rb == 0) {
                int16_t bonus = cb->randdir ? cb->randdir(ctx) : 0;
                damage_result = (int16_t)(bonus + 2);
                compute_damage = 1;
            } else {
                damage_result = 0;
            }
        }
    }

    if (compute_damage) {
        /* c_creature.cpp:844-860 — final damage computation with randomness */
        damage_result >>= 1;
        int16_t r1 = cb->rand16 ? cb->rand16(ctx, damage_result) : 0;
        int16_t r2 = cb->randdir ? cb->randdir(ctx) : 0;
        damage_result = (int16_t)(damage_result + r1 + r2);
        r1 = cb->rand16 ? cb->rand16(ctx, damage_result) : 0;
        damage_result = (int16_t)(damage_result + r1);
        damage_result >>= 2;
        r2 = cb->randdir ? cb->randdir(ctx) : 0;
        damage_result = (int16_t)(damage_result + r2 + 1);
        int rb = cb->randbit ? cb->randbit(ctx) : 0;
        if (rb != 0) {
            int16_t half = cb->rand16 ?
                cb->rand16(ctx, (int16_t)(damage_result / 2 + 1)) : 0;
            damage_result = (int16_t)(damage_result - (half - 1));
        }

        /* c_creature.cpp:862-896 — apply wound */
        if (cb->wound_player) {
            int16_t wound_result = cb->wound_player(ctx,
                state->hero_idx, damage_result, wound_type, attack_type);
            damage_result = wound_result;

            if (wound_result != 0) {
                if (cb->queue_noise_gen2) {
                    cb->queue_noise_gen2(ctx, state->hero_idx,
                        state->hero_type, state->party_x, state->party_y);
                }

                /* c_creature.cpp:879-894 — poison check */
                uint8_t poison_str = ai_spec ? ai_spec[7] : 0;
                if (poison_str != 0) {
                    int rb2 = cb->randbit ? cb->randbit(ctx) : 0;
                    if (rb2 != 0) {
                        int16_t adj = cb->get_adj_ability2 ?
                            cb->get_adj_ability2(ctx, state->hero_idx,
                                                 4, (int16_t)poison_str) : 0;
                        if (adj != 0 && cb->process_poison) {
                            cb->process_poison(ctx, state->hero_idx, adj);
                            receipt->poisoned = 1;
                        }
                    }
                }
            }
        }
    }

    receipt->damage_dealt = damage_result;

    /* c_creature.cpp:900-902 — wake from sleep */
    if (state->v1e0238 != 0) {
        if (cb->resume_from_wake)
            cb->resume_from_wake(ctx);
        receipt->woke_up = 1;
    }

    return (int32_t)damage_result;
}
