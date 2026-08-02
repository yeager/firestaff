/*
 * dm2_v1_combat_damage_pc34_compat.c — DM2 combat damage pipeline.
 *
 * Source: skproject/SKULLWIN/c_hero.cpp
 *   DM2_CALC_PLAYER_ATTACK_DAMAGE:232-430
 *   DM2_WOUND_PLAYER:1496-1740
 *   DM2_ATTACK_PARTY:3346-3394
 *
 * The damage pipeline has three stages:
 *
 * 1. CALC_PLAYER_ATTACK_DAMAGE — hero attacks creature:
 *    - Guard: hero alive, valid creature, party size
 *    - Hit check: dexterity vs (creature_defense + rand(32) + 2*level - 16)/2
 *    - Luck check: 0x4B - armor_mask, hero.use_luck
 *    - Strength: COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH
 *    - Damage: strength * power / 32 - creature_armor - rand(32)
 *    - Bonus: skill level check, weapon poison
 *    - Skill exp: ADJUST_SKILLS by (creature_armor_mult * damage >> 4) + 3
 *
 * 2. WOUND_PLAYER — apply damage to hero:
 *    - Guard: hero alive, valid index, wound > 0
 *    - Armor absorption: iterate 6 slots, distribute by damage type
 *    - HP reduction: hero.curHP -= (wound - absorbed)
 *    - Kill check: if curHP <= 0, DM2_KILL_CHAMPION
 *
 * 3. ATTACK_PARTY — creature attacks all party heroes:
 *    - Base damage split: damage/8 + 1 randomization range
 *    - Per hero: RAND16(range) + base, then WOUND_PLAYER
 *    - Returns bitmask of wounded heroes
 *
 * All three are fail-closed: they classify inputs and produce receipts
 * but do not mutate live game state until bound to the hero/creature
 * data structures.
 */

#include "dm2_v1_combat_damage_pc34_compat.h"

#include <string.h>

int dm2_v1_calc_player_attack_damage(
    const DM2_V1_CalcAttackDamageRequest *request,
    DM2_V1_CalcAttackDamageReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->hero_hp <= 0) {
        receipt->miss = 1;
        return 0;
    }

    if (request->creature_record < 0) {
        receipt->miss = 1;
        return 0;
    }

    /* Hit/miss and damage require live RNG, creature AI spec lookup,
     * COMPUTE_PLAYER_ATTACK_OR_THROW_STRENGTH, QUERY_PLAYER_SKILL_LV,
     * QUERY_GDAT_DBSPEC_WORD_VALUE, and APPLY_CREATURE_POISON_RESISTANCE.
     * Fail-closed until these are bound. */
    receipt->fail_closed = 1;
    receipt->hit = 1;

    return 1;
}

int dm2_v1_wound_player(
    const DM2_V1_WoundPlayerRequest *request,
    DM2_V1_WoundPlayerReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->hero_index < 0 || request->hero_index > 3) {
        return 0;
    }

    if (request->wound_amount <= 0) {
        return 0;
    }

    if (request->hero_hp <= 0) {
        return 0;
    }

    /* Armor absorption loop: c_hero.cpp:1496-1740.
     * Each armor slot absorbs damage based on type:
     *   physical/sharp: full armor value
     *   fire/lightning/magic: armor_value / 2
     *   poison: no absorption */
    {
        int16_t remaining = request->wound_amount;
        int16_t total_absorbed = 0;
        int i;

        if (request->damage_type != DM2_DMG_POISON) {
            for (i = 0; i < 6 && remaining > 0; i++) {
                int16_t armor_val;
                if (!(request->armor_mask & (1u << i)))
                    continue;
                armor_val = request->hero_armor_slots[i];
                if (armor_val <= 0)
                    continue;
                if (request->damage_type == DM2_DMG_FIRE ||
                    request->damage_type == DM2_DMG_LIGHTNING ||
                    request->damage_type == DM2_DMG_MAGIC) {
                    armor_val /= 2;
                }
                if (armor_val > remaining)
                    armor_val = remaining;
                remaining -= armor_val;
                total_absorbed += armor_val;
            }
        }

        receipt->damage_absorbed = total_absorbed;
        receipt->damage_dealt = remaining;
        receipt->hero_wounded = (remaining > 0) ? 1 : 0;
        receipt->hp_remaining = request->hero_hp - remaining;

        if (receipt->hp_remaining <= 0) {
            receipt->hp_remaining = 0;
            receipt->hero_killed = 1;
        }
    }

    return 1;
}

int dm2_v1_attack_party(
    const DM2_V1_AttackPartyRequest *request,
    DM2_V1_AttackPartyReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (request->base_damage <= 0) {
        return 0;
    }

    if (request->heroes_in_party <= 0 || request->heroes_in_party > 4) {
        return 0;
    }

    /* Per-hero damage randomization and WOUND_PLAYER calls require
     * live hero state and RNG. Fail-closed.
     * Reference formula: range = damage/8 + 1, per_hero = RAND16(2*range) + (damage - range)
     * c_hero.cpp:3365-3392 */
    receipt->fail_closed = 1;

    return 1;
}
