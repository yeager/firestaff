#include "dm1_v1_projectile_impact_attack_f0216_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    struct ProjectileInstance_Compat projectile;
    const char* source;
    (void)source;
    struct DungeonThings_Compat things;
    struct DungeonProjectile_Compat sourceProjectile;
    unsigned char rawWeapon[4];
    unsigned char rawProjectile[8];
    DM1_F0216ImpactReceiptPc34 receipt;
    struct RngState_Compat rng;
    struct RngState_Compat expectedRng;
    DM1_WeaponInfo weaponInfo;
    int expected;

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

    memset(&things, 0, sizeof(things));
    memset(&sourceProjectile, 0, sizeof(sourceProjectile));
    memset(rawWeapon, 0, sizeof(rawWeapon));
    memset(rawProjectile, 0, sizeof(rawProjectile));
    memset(&receipt, 0, sizeof(receipt));
    rawWeapon[2] = 0; /* source WEAPON.Type */
    things.loaded = 1;
    things.projectiles = &sourceProjectile;
    things.projectileCount = 1;
    things.rawThingData[THING_TYPE_PROJECTILE] = rawProjectile;
    things.thingCounts[THING_TYPE_PROJECTILE] = 1;
    things.rawThingData[THING_TYPE_WEAPON] = rawWeapon;
    things.thingCounts[THING_TYPE_WEAPON] = 1;
    sourceProjectile.slot = (unsigned short)(THING_TYPE_WEAPON << 10);
    sourceProjectile.kineticEnergy = 40;
    sourceProjectile.attack = 24;
    rawProjectile[2] = (unsigned char)(sourceProjectile.slot & 0xffu);
    rawProjectile[3] = (unsigned char)(sourceProjectile.slot >> 8);
    rawProjectile[4] = sourceProjectile.kineticEnergy;
    rawProjectile[5] = sourceProjectile.attack;
    F0730_COMBAT_RngInit_Compat(&rng, 0x216u);
    F0730_COMBAT_RngInit_Compat(&expectedRng, 0x216u);
    assert(dm1_weapon_info_pc34(0, &weaponInfo) == 1);
    expected = ((weaponInfo.kineticEnergy + weaponInfo.weight + 40) >> 4) + 1;
    expected += F0732_COMBAT_RngRandom_Compat(&expectedRng, (expected >> 1) + 1);
    expected += F0732_COMBAT_RngRandom_Compat(&expectedRng, 4);
    expected = (expected >> 1) > expected - (32 - (24 >> 3))
                   ? (expected >> 1) : expected - (32 - (24 >> 3));
    assert(dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
               &things, &sourceProjectile, &rng, &receipt) == 1);
    assert(receipt.impactAttack == expected);
    assert(receipt.attackTypeCode == COMBAT_ATTACK_BLUNT);
    assert(rng.seed == expectedRng.seed);

    rawProjectile[4] = 39;
    assert(dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
               &things, &sourceProjectile, &rng, &receipt) == 0);
    rawProjectile[4] = 40;

    sourceProjectile.slot = THING_NONE;
    assert(dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
               &things, &sourceProjectile, &rng, &receipt) == 0);
    sourceProjectile.slot = (unsigned short)(THING_TYPE_EXPLOSION << 10);
    assert(dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
               &things, &sourceProjectile, &rng, &receipt) == 0);
    return 0;
}
