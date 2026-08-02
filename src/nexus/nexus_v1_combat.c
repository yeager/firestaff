
#include "nexus_v1_combat.h"
#include <stdlib.h>
#include <stdio.h>

/* DM1-parity combat formulas for Nexus.
 * Source: ReDMCSB PROJEXPL.C F0230, MENUS.C F0402, CHAMPION.C F0321. */

static unsigned int g_combat_rng_state;

void nexus_v1_combat_seed(unsigned int seed) {
    g_combat_rng_state = seed;
    srand(seed);
}

int nexus_v1_combat_random(int max) {
    if (max <= 0) return 0;
    return rand() % max;
}

static int combat_max(int a, int b) { return a > b ? a : b; }

/* F0230 wound zone selection.
 * Source: ReDMCSB PROJEXPL.C lines 1381-1393.
 * Wound probabilities encode 4 nibbles for head/torso/legs/feet. */
static int select_wound_zone(int wound_probs) {
    int roll = nexus_v1_combat_random(16);
    int head_prob  = (wound_probs >> 12) & 0x0F;
    int torso_prob = (wound_probs >> 8) & 0x0F;
    int legs_prob  = (wound_probs >> 4) & 0x0F;

    if (roll < head_prob) return NEXUS_WOUND_HEAD;
    if (roll < head_prob + torso_prob) return NEXUS_WOUND_BODY;
    if (roll < head_prob + torso_prob + legs_prob) return NEXUS_WOUND_LEGS;
    return NEXUS_WOUND_ARMS;
}

/* Champion dexterity for combat.
 * Source: ReDMCSB CHAMPION.C F0312. */
static int champion_dexterity(const Nexus_V1_Champion *ch) {
    int dex = ch->dexterity;
    if (ch->wounds & NEXUS_WOUND_LEGS) dex -= 12;
    if (ch->wounds & NEXUS_WOUND_HEAD) dex -= 8;
    return combat_max(1, dex);
}

/* Champion strength for combat.
 * Source: ReDMCSB CHAMPION.C F0312. */
static int champion_strength(const Nexus_V1_Champion *ch) {
    int str = ch->strength;
    if (ch->wounds & NEXUS_WOUND_ARMS) str -= 12;
    if (ch->wounds & NEXUS_WOUND_BODY) str -= 8;
    return combat_max(1, str);
}

Nexus_CombatResult nexus_v1_champion_melee_attack(
    Nexus_V1_Champion *attacker, int weapon_power,
    const Nexus_CreatureType *target)
{
    Nexus_CombatResult r = {0};
    int str, champ_dex, crea_dex, attack;

    if (!attacker || !attacker->alive || !target) return r;
    if (attacker->stamina < 3) return r;
    attacker->stamina -= 3;

    str = champion_strength(attacker);
    champ_dex = champion_dexterity(attacker);
    crea_dex = target->defense > 0 ? target->defense / 2 : 1;

    /* Dexterity duel: random(champDex) must beat random(creaDex/2).
     * Source: ReDMCSB MENUS.C F0402 -> PROJEXPL.C F0190. */
    if (nexus_v1_combat_random(combat_max(1, champ_dex)) <
        nexus_v1_combat_random(combat_max(1, crea_dex))) {
        return r;
    }
    r.hit = 1;

    /* Attack = (strength * 2) + weapon_power + random(16).
     * Source: ReDMCSB MENUS.C F0402 lines 1510-1515. */
    attack = (str << 1) + weapon_power + nexus_v1_combat_random(16);

    /* Subtract creature defense.
     * Source: ReDMCSB PROJEXPL.C F0190. */
    attack -= nexus_v1_combat_random(target->defense + 1);
    if (attack <= 0) {
        attack = nexus_v1_combat_random(4) + 1;
    }

    r.damage = attack;
    r.experience_gained = attack;
    return r;
}

Nexus_CombatResult nexus_v1_creature_melee_attack(
    const Nexus_CreatureType *attacker,
    Nexus_V1_Champion *target)
{
    Nexus_CombatResult r = {0};
    int champ_dex, dex_roll, atk;

    if (!attacker || !target || !target->alive) return r;

    champ_dex = champion_dexterity(target);

    /* Dexterity duel: creature hits when champion dex < threshold OR
     * 1-in-4 miss gate is zero.
     * Source: ReDMCSB PROJEXPL.C F0230 lines 1353-1361. */
    dex_roll = nexus_v1_combat_random(32);
    if (champ_dex >= (dex_roll + attacker->speed - 16) &&
        nexus_v1_combat_random(4) != 0) {
        return r;
    }
    r.hit = 1;

    /* Wound zone from creature wound probabilities.
     * Default equal probability if creature has no specific data. */
    r.wound_zone = select_wound_zone(0x4444);

    /* Staged random attack.
     * Source: ReDMCSB PROJEXPL.C F0230 lines 1397-1403. */
    atk = nexus_v1_combat_random(16) + attacker->attack;
    if (atk <= 1) {
        if (nexus_v1_combat_random(2) != 0) return r;
        atk = nexus_v1_combat_random(4) + 2;
    }
    atk >>= 1;
    atk += nexus_v1_combat_random(combat_max(1, atk)) + nexus_v1_combat_random(4);
    atk += nexus_v1_combat_random(combat_max(1, atk));
    atk >>= 2;
    atk += nexus_v1_combat_random(4) + 1;
    if (nexus_v1_combat_random(2) != 0) {
        atk -= nexus_v1_combat_random((atk >> 1) + 1) - 1;
    }

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
        if (nexus_v1_combat_random(8) == 0) {
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
    case NEXUS_CLASS_FIGHTER: champ->fighter_level += amount / 100; break;
    case NEXUS_CLASS_NINJA:   champ->ninja_level += amount / 100;   break;
    case NEXUS_CLASS_PRIEST:  champ->priest_level += amount / 100;  break;
    case NEXUS_CLASS_WIZARD:  champ->wizard_level += amount / 100;  break;
    default: break;
    }
}

/* Legacy API */
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker,
                                   int weapon_power, int defense)
{
    Nexus_CombatResult r = {0};
    int hit_chance, str_bonus, damage, def_reduce;

    if (!attacker || !attacker->alive) return r;
    if (attacker->stamina < 3) return r;
    attacker->stamina -= 3;

    hit_chance = attacker->dexterity + attacker->fighter_level * 2;
    if (hit_chance > 95) hit_chance = 95;
    r.hit = (nexus_v1_combat_random(100) < hit_chance);
    if (!r.hit) return r;

    str_bonus = attacker->strength / 5;
    damage = weapon_power + str_bonus + nexus_v1_combat_random(str_bonus > 0 ? str_bonus : 1);
    if (nexus_v1_combat_random(100) < 5) {
        damage *= 2;
        r.critical = 1;
    }
    def_reduce = defense / 2 + nexus_v1_combat_random(defense / 2 + 1);
    damage -= def_reduce;
    if (damage < 1) damage = 1;

    r.damage = damage;
    r.experience_gained = damage;
    return r;
}

int nexus_v1_take_damage(Nexus_V1_Champion *target, int damage) {
    return nexus_v1_champion_take_damage(target, damage, 0);
}
