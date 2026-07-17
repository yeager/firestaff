#ifndef FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_ATTACK_F0216_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_PROJECTILE_IMPACT_ATTACK_F0216_PC34_COMPAT_H

#include "memory_projectile_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB PROJEXPL.C F0216_PROJECTILE_GetImpactAttack.
 * The live projectile's Attack field wins; a zero Attack falls back to
 * KineticEnergy. Negative host values are rejected instead of becoming
 * synthetic impact data. */
int F0216_PROJECTILE_GetImpactAttack(
    const struct ProjectileInstance_Compat* projectile);

typedef struct {
    int impactAttack;
    int poisonAttack;
    int attackTypeCode;
} DM1_F0216ImpactReceiptPc34;

/* PROJEXPL.C F0216's source-owned C14 path.  `sourceProjectile->slot` is
 * authenticated through the loaded Thing table; unknown Thing shapes and
 * absent raw data reject instead of borrowing a runtime subtype. */
int dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
    const struct DungeonThings_Compat* things,
    const struct DungeonProjectile_Compat* sourceProjectile,
    struct RngState_Compat* rng,
    DM1_F0216ImpactReceiptPc34* outReceipt);
int dm1_v1_projectile_get_impact_attack_f0216_pc34(
    const struct ProjectileInstance_Compat* projectile);
const char* dm1_v1_projectile_get_impact_attack_f0216_source_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
