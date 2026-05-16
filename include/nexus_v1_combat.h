
#ifndef NEXUS_V1_COMBAT_H
#define NEXUS_V1_COMBAT_H

#include "nexus_v1_champions.h"

/* DM1-compatible combat system for Nexus.
 * Attack, defend, damage calculation, experience. */

typedef struct {
    int attack_type;  /* 0=melee, 1=ranged, 2=spell */
    int damage;
    int hit;
    int critical;
    int experience_gained;
} Nexus_CombatResult;

Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker, int weapon_power, int defense);
int nexus_v1_take_damage(Nexus_V1_Champion *target, int damage);
void nexus_v1_gain_experience(Nexus_V1_Champion *champ, Nexus_ChampionClass skill, int amount);

#endif

