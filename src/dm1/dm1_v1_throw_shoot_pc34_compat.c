#include "dm1_v1_throw_shoot_pc34_compat.h"

#include "dm1_v1_combat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <string.h>

static const unsigned char DM1_ARMOUR_WEIGHT_F0140_PC34[58] = {
      3,   4,   3,   6,  16,   4,   4,   3,   3,   4,
      2,   4,   5,   3,   3,   4,   6,   8,  14,   6,
      5,   5,   5,   4,   6,  11,  14,  15,  11,  10,
     14,  21,  65,  53,  52,  41,  16,  16,  19, 120,
     80,  28,  34,  17, 108,  72,  24,  30,  35, 141,
     90,  31,  40,  14,  57,  81,   3,   2
};

static const unsigned char DM1_JUNK_WEIGHT_F0140_PC34[53] = {
      1,   3,   2,   2,   4,  15,   1,   1,   1,   2,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,  81,   2,   3,   2,   4,
      4,   3,   8,   5,  11,   4,   6,   2,   3,   2,
      2,   2,   6,   9,   3,  10,   1,   0,   1,   1,
      2,   0,   8
};

int dm1_v1_throwing_stamina_cost_from_weight_pc34(int objectWeight) {
    int weight;
    int cost;

    /* ReDMCSB: CHAMPION.C F0305 lines 1061-1074. */
    weight = objectWeight >> 1;
    if (weight < 1) cost = 1;
    else if (weight > 10) cost = 10;
    else cost = weight;

    while ((weight -= 10) > 0) {
        cost += weight >> 1;
    }
    return cost;
}

int dm1_v1_throw_armour_weight_f0140_pc34(int armourType) {
    /* ReDMCSB: DUNGEON.C F0140 lines 1103-1130, armour table. */
    if (armourType < 0 ||
        armourType >= (int)(sizeof(DM1_ARMOUR_WEIGHT_F0140_PC34) /
                            sizeof(DM1_ARMOUR_WEIGHT_F0140_PC34[0]))) {
        return -1;
    }
    return (int)DM1_ARMOUR_WEIGHT_F0140_PC34[armourType];
}

int dm1_v1_throw_junk_base_weight_f0140_pc34(int junkType) {
    /* ReDMCSB: DUNGEON.C F0140 lines 1103-1130, junk table. */
    if (junkType < 0 ||
        junkType >= (int)(sizeof(DM1_JUNK_WEIGHT_F0140_PC34) /
                          sizeof(DM1_JUNK_WEIGHT_F0140_PC34[0]))) {
        return -1;
    }
    return (int)DM1_JUNK_WEIGHT_F0140_PC34[junkType];
}

int dm1_v1_throw_junk_weight_f0140_pc34(int junkType, int chargeCount) {
    int weight = dm1_v1_throw_junk_base_weight_f0140_pc34(junkType);
    if (weight < 0) return -1;
    if (junkType == DM1_JUNK_WATERSKIN_PC34) {
        if (chargeCount < 0) chargeCount = 0;
        weight += chargeCount << 1;
    }
    return weight;
}

int dm1_v1_throw_xp_for_object_pc34(int isWeapon,
                                    int hasWeaponInfo,
                                    int weaponClass,
                                    int weaponKineticEnergy) {
    int xp = DM1_THROW_BASE_XP_PC34;
    /* ReDMCSB: CHAMPION.C F0328 lines 2166-2173. */
    if (!isWeapon) return xp;
    xp += DM1_THROW_WEAPON_XP_PC34;
    if (hasWeaponInfo &&
        weaponClass <= DM1_THROW_WEAPON_CLASS_MAX_KINETIC_XP_PC34) {
        xp += weaponKineticEnergy >> 2;
    }
    return xp;
}

int dm1_v1_throw_side_pc34(int championCell, int partyDirection) {
    partyDirection &= 3;
    championCell &= 3;
    /* ReDMCSB: MENU.C F0407 lines 1613-1615. */
    return (championCell == ((partyDirection + 1) & 3) ||
            championCell == ((partyDirection + 2) & 3)) ? 1 : 0;
}

int dm1_v1_throw_kinetic_energy_pc34(int baseStrength,
                                     int throwSkillLevel,
                                     int hasWeaponInfo,
                                     int weaponClass,
                                     int weaponKineticEnergy,
                                     int rng16) {
    int kineticEnergy;
    int objectKineticEnergy = DM1_THROW_DEFAULT_WEAPON_KINETIC_PC34;
    /* ReDMCSB: CHAMPION.C F0328 lines 2176-2186. */
    if (hasWeaponInfo &&
        weaponClass <= DM1_THROW_WEAPON_CLASS_MAX_KINETIC_XP_PC34) {
        objectKineticEnergy = weaponKineticEnergy;
    }
    rng16 &= 15;
    kineticEnergy = baseStrength + objectKineticEnergy;
    return kineticEnergy + rng16 + (kineticEnergy >> 1) + throwSkillLevel;
}

int dm1_v1_throw_attack_pc34(int throwSkillLevel, int rng32) {
    int attack;
    /* ReDMCSB: CHAMPION.C F0328 lines 2185-2187. */
    rng32 &= 31;
    attack = (throwSkillLevel << 3) + rng32;
    if (attack < DM1_THROW_MIN_ATTACK_PC34) attack = DM1_THROW_MIN_ATTACK_PC34;
    if (attack > DM1_THROW_MAX_ATTACK_PC34) attack = DM1_THROW_MAX_ATTACK_PC34;
    return attack;
}

int dm1_v1_throw_step_energy_pc34(int throwSkillLevel) {
    int stepEnergy = 11 - throwSkillLevel;
    /* ReDMCSB: CHAMPION.C F0328 lines 2187-2189. */
    return stepEnergy < DM1_THROW_MIN_STEP_ENERGY_PC34
               ? DM1_THROW_MIN_STEP_ENERGY_PC34
               : stepEnergy;
}

int dm1_v1_thrown_potion_projectile_subtype_pc34(int potionType,
                                                 int* outSubtype) {
    if (outSubtype) *outSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    /* ReDMCSB: PROJEXPL.C F0212 special explosion subtype mapping plus
     * CHAMPION.C F0328 thrown-potion handoff. */
    if (potionType == DM1_POTION_VEN_PC34) {
        if (outSubtype) *outSubtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
        return 1;
    }
    if (potionType == DM1_POTION_FUL_BOMB_PC34) {
        if (outSubtype) *outSubtype = PROJECTILE_SUBTYPE_FIREBALL;
        return 1;
    }
    return 0;
}

int dm1_v1_shoot_step_energy_pc34(int actionClass, int* outStepEnergy) {
    if (!outStepEnergy) return 0;
    /* ReDMCSB: MENU.C F0407 SHOOT class ranges. */
    if (actionClass >= DM1_WEAPON_CLASS_FIRST_BOW &&
        actionClass <= DM1_WEAPON_CLASS_LAST_BOW) {
        *outStepEnergy = actionClass - DM1_WEAPON_CLASS_FIRST_BOW;
        return 1;
    }
    if (actionClass >= DM1_WEAPON_CLASS_FIRST_SLING &&
        actionClass <= DM1_WEAPON_CLASS_LAST_SLING) {
        *outStepEnergy = actionClass - DM1_WEAPON_CLASS_FIRST_SLING;
        return 1;
    }
    return 0;
}

int dm1_v1_shoot_ammunition_matches_pc34(int actionWeaponClass,
                                         int readyWeaponClass) {
    /* ReDMCSB: MENU.C F0407 lines 1364-1388. */
    if (actionWeaponClass >= DM1_WEAPON_CLASS_FIRST_BOW &&
        actionWeaponClass <= DM1_WEAPON_CLASS_LAST_BOW) {
        return readyWeaponClass == DM1_WEAPON_CLASS_BOW_AMMUNITION;
    }
    if (actionWeaponClass >= DM1_WEAPON_CLASS_FIRST_SLING &&
        actionWeaponClass <= DM1_WEAPON_CLASS_LAST_SLING) {
        return readyWeaponClass == DM1_WEAPON_CLASS_SLING_AMMUNITION;
    }
    return 0;
}

int dm1_v1_projectile_launch_cell_pc34(int championCell, int direction) {
    championCell &= 3;
    direction &= 3;
    /* ReDMCSB: PROJEXPL.C F0212 launch-cell normalization as consumed by
     * CHAMPION.C F0326/F0328. */
    return (((((championCell - direction + 1) & 2) >> 1) + direction) & 3);
}

int dm1_v1_shoot_attack_pc34(int weaponShootAttack, int shootSkillLevel) {
    int attack;
    /* ReDMCSB: MENU.C F0407 line 1395. */
    attack = (weaponShootAttack + shootSkillLevel) << 1;
    return attack > 255 ? 255 : attack;
}

int dm1_v1_legacy_throw_attack_probe_pc34(int baseAttack, int throwSkillLevel) {
    int attack = (baseAttack + throwSkillLevel) << 1;
    return attack > 255 ? 255 : attack;
}

int dm1_v1_build_projectile_create_input_pc34(
    const DM1_ProjectileCreateRequestPc34* req,
    struct ProjectileCreateInput_Compat* outInput) {
    int direction;
    int cell;
    if (!req || !outInput) return 0;
    if (req->championIndex < 0) return 0;

    /* ReDMCSB: CHAMPION.C F0326/F0328 and PROJEXPL.C F0212.  M11 supplies
     * host state; DM1 owns the source-shaped F0810 create input. */
    memset(outInput, 0, sizeof(*outInput));
    direction = req->launchDirection >= 0
                    ? (req->launchDirection & 3)
                    : (req->partyDirection & 3);
    cell = req->launchCell >= 0
               ? (req->launchCell & 3)
               : dm1_v1_projectile_launch_cell_pc34(
                     req->championCell, direction);

    outInput->category = req->category;
    outInput->subtype = req->subtype;
    outInput->ownerKind = PROJECTILE_OWNER_CHAMPION;
    outInput->ownerIndex = req->championIndex;
    outInput->mapIndex = req->partyMapIndex;
    outInput->mapX = req->partyMapX;
    outInput->mapY = req->partyMapY;
    outInput->cell = cell;
    outInput->direction = direction;
    outInput->kineticEnergy = req->kineticEnergy;
    outInput->attack = req->impactAttack;
    outInput->launcherStrength = req->launcherStrength;
    outInput->stepEnergy = req->stepEnergy > 0 ? req->stepEnergy : 1;
    outInput->currentTick = req->gameTick;
    if (req->subtype == PROJECTILE_SUBTYPE_POISON_BOLT ||
        req->subtype == PROJECTILE_SUBTYPE_POISON_CLOUD) {
        if (req->carriedThing != THING_NONE &&
            req->carriedThing != THING_ENDOFLIST &&
            THING_GET_TYPE(req->carriedThing) == THING_TYPE_POTION) {
            outInput->poisonAttack = req->potionPower;
        } else {
            outInput->poisonAttack = req->impactAttack;
        }
    }
    outInput->attackTypeCode = req->attackTypeCode;
    outInput->potionPower = req->potionPower;
    outInput->associatedThing =
        (req->carriedThing != THING_NONE &&
         req->carriedThing != THING_ENDOFLIST)
            ? (int)req->carriedThing
            : (int)THING_NONE;
    outInput->firstMoveGraceFlag = 1;
    return 1;
}

const char* dm1_v1_projectile_subtype_name_pc34(int subtype) {
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL:          return "FIREBALL";
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:    return "LIGHTNING BOLT";
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL: return "DISPELL";
        case PROJECTILE_SUBTYPE_POISON_BOLT:       return "POISON BOLT";
        case PROJECTILE_SUBTYPE_POISON_CLOUD:      return "POISON CLOUD";
        case PROJECTILE_SUBTYPE_OPEN_DOOR:         return "MAGIC";
        case PROJECTILE_SUBTYPE_SLIME:             return "SLIME";
        case PROJECTILE_SUBTYPE_KINETIC_ARROW:     return "MISSILE";
        default:                                   return "PROJECTILE";
    }
}

int dm1_v1_projectile_impact_source_sound_index_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result) {
    if (!projectile || !result) return -1;
    switch (result->resultKind) {
        case PROJECTILE_RESULT_HIT_WALL:
        case PROJECTILE_RESULT_HIT_DOOR:
        case PROJECTILE_RESULT_HIT_CHAMPION:
        case PROJECTILE_RESULT_HIT_CREATURE:
        case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
            break;
        default:
            return -1;
    }

    if (result->emittedExplosion) {
        switch (result->outExplosion.explosionType) {
            case C000_EXPLOSION_FIREBALL:
            case C001_EXPLOSION_SLIME:
            case C002_EXPLOSION_LIGHTNING_BOLT:
                return (result->outExplosion.attack > 80) ? 5 : 6;
            case C040_EXPLOSION_SMOKE:
                return -1;
            default:
                return 16;
        }
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 574-584 deletes spent lightning and
     * poison bolts without the non-explosion thud branch. */
    if (projectile->projectileCategory == PROJECTILE_CATEGORY_MAGICAL) {
        if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
            (projectile->kineticEnergy >> 1) == 0) {
            return -1;
        }
        if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_POISON_BOLT &&
            (projectile->kineticEnergy >> 2) == 0) {
            return -1;
        }
    }

    if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_POISON_BOLT) {
        return 16;
    }
    if (projectile->projectileCategory == PROJECTILE_CATEGORY_KINETIC &&
        projectile->projectileSubtype == PROJECTILE_SUBTYPE_KINETIC_ARROW) {
        return 0;
    }
    return 4;
}

int dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(int weaponType) {
    /* ReDMCSB: PROJEXPL.C F0217 lines 540-553 sharp thrown weapon list. */
    return weaponType == 8   ||  /* C08_WEAPON_DAGGER */
           weaponType == 27  ||  /* C27_WEAPON_ARROW */
           weaponType == 28  ||  /* C28_WEAPON_SLAYER */
           weaponType == 31  ||  /* C31_WEAPON_POISON_DART */
           weaponType == 32;     /* C32_WEAPON_THROWING_STAR */
}

int dm1_v1_projectile_associated_thing_disposition_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int associatedThingMovedToGroup,
    int potionCount,
    DM1_ProjectileAssociatedThingDispositionPc34* outDisposition) {
    unsigned short associatedThing;
    int associatedType;
    int associatedIndex;
    if (!outDisposition) return 0;
    memset(outDisposition, 0, sizeof(*outDisposition));
    outDisposition->associatedThing = THING_NONE;
    outDisposition->droppedThing = THING_NONE;
    if (!projectile) return 0;

    associatedThing = (unsigned short)projectile->reserved1;
    outDisposition->associatedThing = associatedThing;
    if (associatedThing == THING_NONE || associatedThing == THING_ENDOFLIST) {
        return 1;
    }
    associatedType = (int)THING_GET_TYPE(associatedThing);
    associatedIndex = (int)THING_GET_INDEX(associatedThing);

    if (result && result->despawn &&
        (projectile->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0 &&
        associatedType == THING_TYPE_POTION &&
        associatedIndex >= 0 && associatedIndex < potionCount) {
        /* ReDMCSB: PROJEXPL.C F0217 lines 444-455 RemovePotion path. */
        outDisposition->shouldConsumePotion = 1;
        return 1;
    }

    if (associatedThingMovedToGroup) return 1;
    if ((projectile->flags & PROJECTILE_FLAG_REMOVE_POTION_ON_IMPACT) != 0) {
        return 1;
    }
    if (associatedType == THING_TYPE_EXPLOSION) return 1;

    /* ReDMCSB: PROJEXPL.C F0215 lines 248-259 materializes Projectile.Slot
     * on the projectile's stored square if the delete tail did not consume
     * or transfer it. */
    outDisposition->shouldMaterialize = 1;
    outDisposition->droppedThing =
        (unsigned short)((associatedThing & 0x3FFFu) |
                         (unsigned short)((projectile->cell & 0x03) << 14));
    return 1;
}

int dm1_v1_group_creature_index_for_cell_pc34(
    const struct DungeonGroup_Compat* group,
    int targetCell) {
    int i;
    if (!group) return -1;
    if (group->count == 0 ||
        group->cells == DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34) {
        return group->health[0] ? 0 : -1;
    }
    for (i = 0; i <= (int)group->count && i < 4; ++i) {
        int cell;
        if (group->health[i] == 0) continue;
        cell = (int)((group->cells >> (i * 2)) & 0x03u);
        if (cell == (targetCell & 3)) return i;
    }
    return -1;
}

int dm1_v1_black_flame_fireball_heal_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    const struct DungeonGroup_Compat* group,
    int* outSlotIndex,
    int* outNewHealth) {
    int slotIndex;
    int healed;
    if (outSlotIndex) *outSlotIndex = -1;
    if (outNewHealth) *outNewHealth = 0;
    if (!projectile || !result || !group) return 0;
    if (result->resultKind != PROJECTILE_RESULT_HIT_CREATURE ||
        !result->emittedCombatAction) {
        return 0;
    }
    if (projectile->projectileSubtype != PROJECTILE_SUBTYPE_FIREBALL) return 0;
    if (group->creatureType != DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34) {
        return 0;
    }

    slotIndex = dm1_v1_group_creature_index_for_cell_pc34(
        group, result->outAction.targetCell);
    if (slotIndex < 0) return 0;

    /* ReDMCSB: PROJEXPL.C F0217 lines 529-531 heals Black Flame on
     * fireball impact up to 1000 HP and skips normal damage/explosion. */
    healed = (int)group->health[slotIndex] + result->outAction.rawAttackValue;
    if (healed > DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34) {
        healed = DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34;
    }
    if (outSlotIndex) *outSlotIndex = slotIndex;
    if (outNewHealth) *outNewHealth = healed;
    return 1;
}
