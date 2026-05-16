
#ifndef FIRESTAFF_DM2_V1_COMBAT_H
#define FIRESTAFF_DM2_V1_COMBAT_H
#include <stdint.h>

/* DM2 Combat — extends DM1 with ranged combat, tech weapons
 * DM2 adds: crossbow, guns (tech weapons), thrown bombs,
 * companion NPCs, outdoor combat with different movement rules.
 * Source: SKULL.ASM combat routines */

typedef enum {
    DM2_WEAPON_MELEE = 0,
    DM2_WEAPON_THROWN,
    DM2_WEAPON_CROSSBOW,
    DM2_WEAPON_GUN,       /* tech weapon */
    DM2_WEAPON_BOMB,
    DM2_WEAPON_MAGIC,
} DM2_WeaponType;

typedef struct {
    DM2_WeaponType type;
    int base_damage;
    int range;        /* tiles, 1 = melee */
    int ammo_required;
    int tech_level;   /* 0 = magic era, 1+ = tech */
} DM2_V1_WeaponInfo;

int dm2_v1_combat_resolve_attack(const DM2_V1_WeaponInfo *weapon,
    int attacker_strength, int target_defense, int distance);
int dm2_v1_combat_is_ranged(DM2_WeaponType type);
const char *dm2_v1_combat_source_evidence(void);
#endif

