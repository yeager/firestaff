
#include "dm2_v1_combat.h"

/* DM2 combat resolver
 * Source: SKULL.ASM combat damage calculation, ranged attack path */

int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance)
{
    int damage, range_penalty;
    if (!weapon) return 0;
    damage = weapon->base_damage + attacker_strength / 4;
    if (distance > weapon->range) return 0; /* out of range */
    /* Range penalty: -10% per extra tile */
    range_penalty = (distance - 1) * damage / 10;
    damage -= range_penalty;
    damage -= target_defense;
    return damage > 0 ? damage : 0;
}

int dm2_v1_combat_is_ranged(DM2_WeaponType type) {
    return type == DM2_WEAPON_CROSSBOW || type == DM2_WEAPON_GUN ||
           type == DM2_WEAPON_THROWN || type == DM2_WEAPON_BOMB;
}

const char *dm2_v1_combat_source_evidence(void) {
    return "SKULL.ASM: combat damage calculation, ranged path\n"
           "DM2 additions: crossbow, guns, bombs, range penalty\n";
}

