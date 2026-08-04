
#include "nexus_v1_combat.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* Nexus combat — DM.BIN disassembly.
 * Dispatcher at 0x0197DE, core formula at 0x0136BA.
 * Hit/miss: deterministic (0x029498). Damage: two-component scale.
 * Wound penalties: multiplicative tables at 0x036AF0/0x036AF6. */

static uint32_t g_combat_rng_state;

void nexus_v1_combat_seed(unsigned int seed) {
    g_combat_rng_state = (uint32_t)seed;
}

int nexus_v1_combat_random(int max) {
    if (max <= 0) return 0;
    g_combat_rng_state = g_combat_rng_state * 0xBB40E62Du + 11u;
    return (int)((g_combat_rng_state >> 8) % (unsigned int)max);
}

/* DM.BIN 0x018006: body part RNG returns (state >> 8) & 3. */
static int combat_body_part_random(void) {
    g_combat_rng_state = g_combat_rng_state * 0xBB40E62Du + 11u;
    return (int)((g_combat_rng_state >> 8) & 3);
}

static int combat_max(int a, int b) { return a > b ? a : b; }
static int combat_min(int a, int b) { return a < b ? a : b; }

/* DM.BIN 0x036AF0: wound damage multiplier table (body/strength).
 * Indexed by wound level 0-5. Divide by 8 for actual multiplier. */
static const int g_wound_table_body[6] = {8, 12, 16, 20, 24, 28};

/* DM.BIN 0x036AF6: wound damage multiplier table (limbs/dexterity).
 * Indexed by wound level 0-5. Divide by 8 for actual multiplier. */
static const int g_wound_table_limbs[6] = {4, 8, 12, 16, 20, 24};

/* DM.BIN 0x037DA8: strength attack/defense ranges. */
static const int g_str_attack_lo = 95, g_str_attack_hi = 148;
static const int g_str_defense_lo = 108, g_str_defense_hi = 202;

/* DM.BIN 0x037DB0: dexterity attack/defense ranges. */
static const int g_dex_attack_lo = 113, g_dex_attack_hi = 148;
static const int g_dex_defense_lo = 126, g_dex_defense_hi = 202;

/* DM.BIN 0x0136BA: scale(stat, range) = min(range * stat, 3072). */
static int combat_scale(int stat, int range) {
    int v = range * stat;
    return combat_min(v, 3072);
}

/* DM.BIN 0x019882: creature flee threshold. */
#define NEXUS_CREATURE_FLEE_HP 120

/* DM.BIN 0x018006: body part selection — returns NEXUS_WOUND_* bitmask. */
static int select_wound_zone(void) {
    int part = combat_body_part_random();
    switch (part) {
    case 0: return NEXUS_WOUND_HEAD;
    case 1: return NEXUS_WOUND_BODY;
    case 2: return NEXUS_WOUND_ARMS;
    default: return NEXUS_WOUND_LEGS;
    }
}

/* DM.BIN 0x029498: deterministic hit test.
 * effective_stat = (champion_stat + weapon_bonus) / 2, capped at 16. */
static int combat_hit_test(int effective_attack, int creature_defense) {
    int capped = combat_min(effective_attack / 2, 16);
    return capped > creature_defense;
}

/* DM.BIN 0x0136BA: two-component damage formula.
 * damage = scale(str + 1024, str_range) + scale(dex + 1024, dex_range). */
static int combat_damage(int strength, int dexterity, int weapon_power) {
    int str_component = combat_scale(strength + 1024, g_str_attack_hi - g_str_attack_lo);
    int dex_component = combat_scale(dexterity + 1024, g_dex_attack_hi - g_dex_attack_lo);
    int base = (str_component + dex_component) / 256 + weapon_power;
    return combat_max(1, base);
}

/* DM.BIN 0x0197E6: stamina cost per attack — formula-driven.
 * cost = (champion_stat + 1024) * attack_range / 3072. */
static int combat_stamina_cost(int strength) {
    int range = g_str_attack_hi - g_str_attack_lo;
    int cost = ((strength + 1024) * range) / 3072;
    return combat_max(1, cost);
}

/* DM.BIN 0x01D144: apply wound penalty as multiplicative damage.
 * penalty = wound_table[wound_level] * base_damage / 8. */
static int apply_wound_penalty(int damage, int wounds) {
    int wound_level = 0;
    if (wounds & NEXUS_WOUND_HEAD) wound_level++;
    if (wounds & NEXUS_WOUND_BODY) wound_level++;
    if (wounds & NEXUS_WOUND_ARMS) wound_level++;
    if (wounds & NEXUS_WOUND_LEGS) wound_level++;
    if (wound_level <= 0) return damage;
    if (wound_level > 5) wound_level = 5;
    return (damage * g_wound_table_body[wound_level]) / 8;
}

Nexus_CombatResult nexus_v1_champion_melee_attack(
    Nexus_V1_Champion *attacker, int weapon_power,
    const Nexus_CreatureType *target)
{
    Nexus_CombatResult r = {0};
    int cost, dmg;

    if (!attacker || !attacker->alive || !target) return r;

    cost = combat_stamina_cost(attacker->strength);
    if (attacker->stamina < cost) return r;
    attacker->stamina -= cost;

    if (!combat_hit_test(attacker->dexterity + attacker->fighter_level * 2,
                         target->defense)) {
        return r;
    }
    r.hit = 1;

    dmg = combat_damage(attacker->strength, attacker->dexterity, weapon_power);
    dmg = apply_wound_penalty(dmg, attacker->wounds);
    r.damage = dmg;
    r.experience_gained = dmg;
    return r;
}

Nexus_CombatResult nexus_v1_creature_melee_attack(
    const Nexus_CreatureType *attacker,
    Nexus_V1_Champion *target)
{
    Nexus_CombatResult r = {0};
    int atk;

    if (!attacker || !target || !target->alive) return r;

    if (!combat_hit_test(attacker->speed, target->dexterity / 2)) {
        return r;
    }
    r.hit = 1;

    r.wound_zone = select_wound_zone();

    atk = attacker->attack;
    atk = apply_wound_penalty(atk, target->wounds);
    r.damage = combat_max(1, atk);
    r.experience_gained = r.damage;
    return r;
}

int nexus_v1_champion_take_damage(Nexus_V1_Champion *target, int damage,
                                  int wound_zone)
{
    if (!target || !target->alive || damage <= 0) return 0;

    target->health -= damage;
    if (wound_zone && !(target->wounds & wound_zone)) {
        if (combat_body_part_random() == 0) {
            target->wounds |= wound_zone;
        }
    }

    if (target->health <= 0) {
        target->health = 0;
        target->alive = 0;
        return 1;
    }
    return 0;
}

int nexus_v1_creature_take_damage(Nexus_Creature *target, int damage) {
    if (!target || !target->alive || damage <= 0) return 0;
    target->health -= damage;
    if (target->health <= 0) {
        target->health = 0;
        target->alive = 0;
        return 1;
    }
    return 0;
}

void nexus_v1_gain_experience(Nexus_V1_Champion *champ,
                              Nexus_ChampionClass skill, int amount)
{
    if (!champ || amount <= 0) return;
    switch (skill) {
    case NEXUS_CLASS_FIGHTER: champ->fighter_level += amount / 10240; break;
    case NEXUS_CLASS_NINJA:   champ->ninja_level += amount / 10240; break;
    case NEXUS_CLASS_PRIEST:  champ->priest_level += amount / 10240; break;
    case NEXUS_CLASS_WIZARD:  champ->wizard_level += amount / 10240; break;
    default: break;
    }
}

/* Legacy API */
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker,
                                   int weapon_power, int defense)
{
    Nexus_CombatResult r = {0};
    int cost, dmg;

    if (!attacker || !attacker->alive) return r;

    cost = combat_stamina_cost(attacker->strength);
    if (attacker->stamina < cost) return r;
    attacker->stamina -= cost;

    if (!combat_hit_test(attacker->dexterity + attacker->fighter_level * 2,
                         defense)) {
        return r;
    }
    r.hit = 1;

    dmg = combat_damage(attacker->strength, attacker->dexterity, weapon_power);
    dmg = apply_wound_penalty(dmg, attacker->wounds);
    if (dmg < 1) dmg = 1;
    r.damage = dmg;
    r.experience_gained = dmg;
    return r;
}

int nexus_v1_take_damage(Nexus_V1_Champion *target, int damage) {
    return nexus_v1_champion_take_damage(target, damage, 0);
}
