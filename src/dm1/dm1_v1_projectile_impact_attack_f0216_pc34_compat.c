#include "dm1_v1_projectile_impact_attack_f0216_pc34_compat.h"
#include "dm1_v1_throw_shoot_pc34_compat.h"
#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

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

int dm1_v1_projectile_get_impact_attack_f0216_from_source_pc34(
    const struct DungeonThings_Compat* things,
    const struct DungeonProjectile_Compat* sourceProjectile,
    struct RngState_Compat* rng,
    DM1_F0216ImpactReceiptPc34* outReceipt)
{
    unsigned short slot;
    const unsigned char* raw;
    int type;
    int weight;
    int attack;
    int projectileIndex = -1;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!things || !things->loaded || !things->projectiles ||
        !sourceProjectile || !rng || !things->rawThingData[THING_TYPE_PROJECTILE] ||
        things->thingCounts[THING_TYPE_PROJECTILE] < things->projectileCount) {
        return 0;
    }
    for (i = 0; i < things->projectileCount; ++i) {
        if (sourceProjectile == &things->projectiles[i]) {
            projectileIndex = i;
            break;
        }
    }
    if (projectileIndex < 0) return 0;
    {
        const unsigned char* rawProjectile =
            things->rawThingData[THING_TYPE_PROJECTILE] + projectileIndex * 8;
        unsigned short rawSlot = (unsigned short)(rawProjectile[2] |
            ((unsigned short)rawProjectile[3] << 8));
        if (rawSlot != sourceProjectile->slot ||
            rawProjectile[4] != sourceProjectile->kineticEnergy ||
            rawProjectile[5] != sourceProjectile->attack) {
            return 0;
        }
    }
    slot = sourceProjectile->slot;
    if (slot == THING_NONE || slot == THING_ENDOFLIST) return 0;
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, slot);
    if (!raw) return 0;
    type = THING_GET_TYPE(slot);
    if (type == THING_TYPE_EXPLOSION) return 0; /* C15 needs its own source form. */
    if (!dm1_v1_dungeon_get_object_weight_f0140_pc34(things, slot, &weight)) {
        return 0;
    }
    if (type == THING_TYPE_WEAPON) {
        DM1_WeaponInfo info;
        if (!dm1_weapon_info_pc34(raw[2] & 0x7f, &info)) return 0;
        attack = info.kineticEnergy;
    } else {
        attack = F0732_COMBAT_RngRandom_Compat(rng, 4);
    }
    attack += weight;
    attack = ((attack + (int)sourceProjectile->kineticEnergy) >> 4) + 1;
    attack += F0732_COMBAT_RngRandom_Compat(rng, (attack >> 1) + 1);
    attack += F0732_COMBAT_RngRandom_Compat(rng, 4);
    {
        int reduced = attack - (32 - ((int)sourceProjectile->attack >> 3));
        attack = (attack >> 1) > reduced ? (attack >> 1) : reduced;
    }
    outReceipt->impactAttack = attack;
    outReceipt->poisonAttack = 0;
    outReceipt->attackTypeCode = COMBAT_ATTACK_BLUNT;
    return 1;
}

const char* dm1_v1_projectile_get_impact_attack_f0216_source_pc34(void) {
    return "ReDMCSB PROJEXPL.C F0216_PROJECTILE_GetImpactAttack: "
           "Projectile.Attack when nonzero, otherwise Projectile.KineticEnergy.";
}
