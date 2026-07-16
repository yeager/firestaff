#include "dm1_v1_projectile_impact_attack_f0216_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    struct ProjectileInstance_Compat projectile;
    const char* source;

    memset(&projectile, 0, sizeof(projectile));
    projectile.kineticEnergy = 71;
    projectile.attack = 23;
    assert(F0216_PROJECTILE_GetImpactAttack(&projectile) == 23);
    assert(dm1_v1_projectile_get_impact_attack_f0216_pc34(
               &projectile) == 23);

    projectile.attack = 0;
    assert(F0216_PROJECTILE_GetImpactAttack(&projectile) == 71);

    projectile.kineticEnergy = 0;
    assert(F0216_PROJECTILE_GetImpactAttack(&projectile) == 0);

    projectile.attack = -1;
    projectile.kineticEnergy = 71;
    assert(F0216_PROJECTILE_GetImpactAttack(&projectile) == -1);

    projectile.attack = 0;
    projectile.kineticEnergy = -1;
    assert(F0216_PROJECTILE_GetImpactAttack(&projectile) == -1);
    assert(F0216_PROJECTILE_GetImpactAttack(0) == -1);

    source = dm1_v1_projectile_get_impact_attack_f0216_source_pc34();
    assert(source && strstr(source, "PROJEXPL.C F0216") != 0);
    assert(strstr(source, "KineticEnergy") != 0);
    return 0;
}
