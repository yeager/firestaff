
#ifndef NEXUS_V1_COMBAT_H
#define NEXUS_V1_COMBAT_H

#include "nexus_v1_champions.h"
#include "nexus_v1_creatures.h"

/* Nexus combat system — DM.BIN disassembly.
 * RNG: DM.BIN 0x06027FCE — LCG state * 0xBB40E62D + 11.
 * Combat RNG at 0x018006: returns (state >> 8) & 3 (body part only).
 * Hit/miss: deterministic stat comparison (0x029498), not RNG.
 * Damage: two-component scale formula (0x0136BA).
 * Wound penalties: multiplicative tables at 0x036AF0/0x036AF6.
 * Stamina cost: formula-driven, not constant. */

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
 * DM.BIN 0x0195B8 (setup), 0x0136BA (core formula).
 * Deterministic hit: effective_stat > creature_defense.
 * Damage: scale(str+1024, range) + scale(dex+1024, range). */
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
