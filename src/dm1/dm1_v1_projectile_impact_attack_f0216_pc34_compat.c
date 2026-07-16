#include "dm1_v1_projectile_impact_attack_f0216_pc34_compat.h"

int F0216_PROJECTILE_GetImpactAttack(
    const struct ProjectileInstance_Compat* projectile) {
    if (!projectile) return -1;
    if (projectile->attack < 0 || projectile->kineticEnergy < 0) return -1;

    return projectile->attack != 0
               ? projectile->attack
               : projectile->kineticEnergy;
}

int dm1_v1_projectile_get_impact_attack_f0216_pc34(
    const struct ProjectileInstance_Compat* projectile) {
    return F0216_PROJECTILE_GetImpactAttack(projectile);
}

const char* dm1_v1_projectile_get_impact_attack_f0216_source_pc34(void) {
    return "ReDMCSB PROJEXPL.C F0216_PROJECTILE_GetImpactAttack: "
           "Projectile.Attack when nonzero, otherwise Projectile.KineticEnergy.";
}
