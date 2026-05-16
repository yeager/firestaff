
#include "nexus_v1_combat.h"
#include <stdlib.h>
#include <stdio.h>

/* DM1-style damage formula:
 * base_damage = weapon_power + strength_bonus
 * hit_chance = dexterity + skill_level (capped at 95%)
 * damage_reduction = defense * random(0.5-1.0) */

static int rng(int max) { return max > 0 ? (rand() % max) : 0; }

Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense) {
    Nexus_CombatResult r = {0};
    int hit_chance, str_bonus, damage, def_reduce;

    if (!attacker || !attacker->alive) return r;

    /* Consume stamina */
    if (attacker->stamina < 3) return r;
    attacker->stamina -= 3;

    /* Hit chance: dex + fighter level, max 95% */
    hit_chance = attacker->dexterity + attacker->fighter_level * 2;
    if (hit_chance > 95) hit_chance = 95;

    r.hit = (rng(100) < hit_chance);
    if (!r.hit) return r;

    /* Damage calculation */
    str_bonus = attacker->strength / 5;
    damage = weapon_power + str_bonus + rng(str_bonus);

    /* Critical: 5% chance, double damage */
    if (rng(100) < 5) {
        damage *= 2;
        r.critical = 1;
    }

    /* Defense reduction */
    def_reduce = defense / 2 + rng(defense / 2);
    damage -= def_reduce;
    if (damage < 1) damage = 1;

    r.damage = damage;
    r.experience_gained = damage;
    return r;
}

int nexus_v1_take_damage(Nexus_V1_Champion *target, int damage) {
    if (!target || !target->alive) return 0;
    target->health -= damage;
    if (target->health <= 0) {
        target->health = 0;
        target->alive = 0;
        printf("%s has died!\n", target->name_ascii);
        return 1; /* died */
    }
    return 0;
}

void nexus_v1_gain_experience(Nexus_V1_Champion *champ, Nexus_ChampionClass skill, int amount) {
    if (!champ || amount <= 0) return;
    switch (skill) {
    case NEXUS_CLASS_FIGHTER: champ->fighter_level += amount / 100; break;
    case NEXUS_CLASS_NINJA:   champ->ninja_level += amount / 100;   break;
    case NEXUS_CLASS_PRIEST:  champ->priest_level += amount / 100;  break;
    case NEXUS_CLASS_WIZARD:  champ->wizard_level += amount / 100;  break;
    default: break;
    }
}

