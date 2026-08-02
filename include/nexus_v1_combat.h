
#ifndef NEXUS_V1_COMBAT_H
#define NEXUS_V1_COMBAT_H

#include "nexus_v1_champions.h"
#include "nexus_v1_creatures.h"

/* DM1-compatible combat system for Nexus.
 * Source: ReDMCSB PROJEXPL.C F0230 (creature attack), MENUS.C F0402
 * (champion melee), CHAMPION.C F0321 (damage application). */

typedef struct {
    int attack_type;  /* 0=melee, 1=ranged, 2=spell */
    int damage;
    int hit;
    int critical;
    int experience_gained;
    int wound_zone;   /* NEXUS_WOUND_* bitmask of wound inflicted */
    int poisoned;     /* nonzero if poison was applied */
} Nexus_CombatResult;

/* Seed the combat RNG (default: time-based). */
void nexus_v1_combat_seed(unsigned int seed);

/* Combat random: returns [0, max-1], clamped to 0 for max <= 0. */
int nexus_v1_combat_random(int max);

/* Champion melee attack against a creature.
 * Source: ReDMCSB MENUS.C F0402, PROJEXPL.C F0190.
 * attack = (strength * 2) + random(16) - random(creature_defense + 1)
 * Dexterity duel gates the hit. */
Nexus_CombatResult nexus_v1_champion_melee_attack(
    Nexus_V1_Champion *attacker, int weapon_power,
    const Nexus_CreatureType *target);

/* Creature melee attack against a champion.
 * Source: ReDMCSB PROJEXPL.C F0230.
 * Staged random damage with dexterity duel, wound zone, and poison. */
Nexus_CombatResult nexus_v1_creature_melee_attack(
    const Nexus_CreatureType *attacker,
    Nexus_V1_Champion *target);

/* Apply damage to a champion with wound zone.
 * Returns 1 if the champion died.
 * Source: ReDMCSB CHAMPION.C F0321. */
int nexus_v1_champion_take_damage(Nexus_V1_Champion *target, int damage,
                                  int wound_zone);

/* Apply damage to a creature. Returns 1 if it died. */
int nexus_v1_creature_take_damage(Nexus_Creature *target, int damage);

/* Award experience to a champion.
 * Source: ReDMCSB CHAMPION.C F0318. */
void nexus_v1_gain_experience(Nexus_V1_Champion *champ,
                              Nexus_ChampionClass skill, int amount);

/* Legacy API — kept for existing callers. */
Nexus_CombatResult nexus_v1_attack(Nexus_V1_Champion *attacker,
                                   int weapon_power, int defense);
int nexus_v1_take_damage(Nexus_V1_Champion *target, int damage);

#endif
