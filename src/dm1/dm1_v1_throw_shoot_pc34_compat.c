#include "dm1_v1_throw_shoot_pc34_compat.h"

#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
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

int dm1_v1_throw_projectile_plan_f0328_pc34(
    const DM1_ThrowF0328ProjectileInputPc34* in,
    DM1_ThrowF0328ProjectilePlanPc34* out) {
    int maxLoad;
    int oneSixteenthMaximumLoad;
    int loadThreshold;
    int strength;
    int halfMaximumStamina;
    int halfStrength;
    if (!in || !out) return 0;
    memset(out, 0, sizeof(*out));
    if (in->objectWeight < 0 || in->championStrength < 0 ||
        in->championCurrentStamina < 0 || in->championMaximumStamina < 0) {
        return 0;
    }

    /* ReDMCSB CHAMPION.C F0312 lines 1237-1303 and F0328 lines 2166-2194:
     * accepted throws play combat sound 13, spend F0305 stamina, disable
     * action for 4 ticks, award Throw XP, then derive F0212 projectile
     * strength/kinetic/attack/step from F0312, F0303 and two RNG draws. */
    strength = (in->rngStrength16 & 15) + in->championStrength;
    maxLoad = in->championMaxLoad;
    if (maxLoad <= 0) {
        maxLoad = (in->championStrength << 3) + 100;
    }
    oneSixteenthMaximumLoad = maxLoad >> 4;
    if (in->objectWeight <= oneSixteenthMaximumLoad) {
        strength += in->objectWeight - 12;
    } else {
        loadThreshold = oneSixteenthMaximumLoad +
            ((oneSixteenthMaximumLoad - 12) >> 1);
        if (in->objectWeight <= loadThreshold) {
            strength += (in->objectWeight - oneSixteenthMaximumLoad) >> 1;
        } else {
            strength -= (in->objectWeight - loadThreshold) << 1;
        }
    }
    if (in->hasWeaponInfo) {
        strength += in->weaponStrength;
        strength += in->f0312SkillBonus << 1;
    }
    halfMaximumStamina = in->championMaximumStamina >> 1;
    if (halfMaximumStamina > 0 &&
        in->championCurrentStamina < halfMaximumStamina) {
        halfStrength = strength >> 1;
        strength = halfStrength +
            (int)(((long)halfStrength * (long)in->championCurrentStamina) /
                  (long)halfMaximumStamina);
    }
    if (in->actionHandWounded) {
        strength >>= 1;
    }
    strength >>= 1;
    if (strength < 0) strength = 0;
    if (strength > 100) strength = 100;

    out->valid = 1;
    out->staminaCost =
        dm1_v1_throwing_stamina_cost_from_weight_pc34(in->objectWeight);
    out->throwExperience = dm1_v1_throw_xp_for_object_pc34(
        in->isWeapon, in->hasWeaponInfo, in->weaponClass,
        in->weaponKineticEnergy);
    out->throwStrength = strength;
    out->kineticEnergy = dm1_v1_throw_kinetic_energy_pc34(
        strength, in->throwSkillLevel, in->hasWeaponInfo,
        in->weaponClass, in->weaponKineticEnergy, in->rngKinetic16);
    out->attack = dm1_v1_throw_attack_pc34(
        in->throwSkillLevel, in->rngAttack32);
    out->stepEnergy = dm1_v1_throw_step_energy_pc34(in->throwSkillLevel);
    out->actionDisableTicks = 4;
    out->combatSoundIndex = 13;
    out->projectileSubtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    if (in->thingType == THING_TYPE_POTION) {
        (void)dm1_v1_thrown_potion_projectile_subtype_pc34(
            in->potionType, &out->projectileSubtype);
        out->projectilePotionPower = in->potionPower;
    }
    out->launchDirection =
        ((in->partyDirection & 3) + (in->throwSide & 1)) & 3;
    out->projectileDisabledMovementTicks = 4;
    out->lastProjectileDisabledMovementDirection = in->partyDirection & 3;
    return 1;
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

int dm1_v1_projectile_explosion_create_input_pc34(
    const struct ProjectileTickResult_Compat* result,
    int currentTick,
    struct ExplosionCreateInput_Compat* outInput) {
    if (!result || !outInput || !result->emittedExplosion) return 0;

    /* ReDMCSB: PROJEXPL.C F0217 lines 574-602 decides whether projectile
     * impact creates an explosion; F0213 lines 107-188 consumes the same
     * map/cell/type/attack tuple when scheduling the explosion thing. */
    memset(outInput, 0, sizeof(*outInput));
    outInput->explosionType = result->outExplosion.explosionType;
    outInput->attack = result->outExplosion.attack;
    outInput->mapIndex = result->outExplosion.mapIndex;
    outInput->mapX = result->outExplosion.mapX;
    outInput->mapY = result->outExplosion.mapY;
    outInput->cell = result->outExplosion.cell;
    outInput->centered = result->outExplosion.centered;
    outInput->poisonAttack = result->outExplosion.poisonAttack;
    outInput->currentTick = currentTick;
    outInput->ownerKind = result->outExplosion.ownerKind;
    outInput->ownerIndex = result->outExplosion.ownerIndex;
    outInput->creatorProjectileSlot =
        result->outExplosion.creatorProjectileSlot;
    return 1;
}

int dm1_v1_projectile_impact_log_plan_pc34(
    const struct ProjectileTickResult_Compat* result,
    DM1_ProjectileImpactLogPlanPc34* outPlan) {
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_NONE_PC34;
    if (!result) return 0;

    /* ReDMCSB: PROJEXPL.C F0217 lines 470-610 is the source impact
     * classifier.  Firestaff keeps UI wording in M11, but the generic
     * non-creature/non-champion impact classes belong to DM1. */
    switch (result->resultKind) {
        case PROJECTILE_RESULT_HIT_WALL:
            outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_HIT_WALL_PC34;
            break;
        case PROJECTILE_RESULT_HIT_DOOR:
            outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_HIT_DOOR_PC34;
            break;
        case PROJECTILE_RESULT_HIT_FLUXCAGE:
            outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_HIT_FLUXCAGE_PC34;
            break;
        case PROJECTILE_RESULT_HIT_OTHER_PROJECTILE:
            outPlan->logKind =
                DM1_PROJECTILE_IMPACT_LOG_HIT_OTHER_PROJECTILE_PC34;
            break;
        case PROJECTILE_RESULT_DESPAWN_ENERGY:
            outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_DESPAWN_ENERGY_PC34;
            break;
        case PROJECTILE_RESULT_DESPAWN_BOUNDS:
            outPlan->logKind = DM1_PROJECTILE_IMPACT_LOG_DESPAWN_BOUNDS_PC34;
            break;
        default:
            return 1;
    }
    outPlan->handled = 1;
    return 1;
}

int dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(int weaponType) {
    /* ReDMCSB: PROJEXPL.C F0217 lines 540-553 sharp thrown weapon list. */
    return weaponType == 8   ||  /* C08_WEAPON_DAGGER */
           weaponType == 27  ||  /* C27_WEAPON_ARROW */
           weaponType == 28  ||  /* C28_WEAPON_SLAYER */
           weaponType == 31  ||  /* C31_WEAPON_POISON_DART */
           weaponType == 32;     /* C32_WEAPON_THROWING_STAR */
}

int dm1_v1_projectile_group_slot_materialization_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome,
    int creatureAttributes,
    int associatedWeaponType,
    DM1_ProjectileGroupSlotMaterializationPlanPc34* outPlan) {
    unsigned short associatedThing;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->associatedThing = THING_NONE;
    outPlan->weaponType = -1;
    if (!projectile) return 0;

    outPlan->valid = 1;
    associatedThing = (unsigned short)projectile->reserved1;
    outPlan->associatedThing = associatedThing;
    outPlan->weaponType = associatedWeaponType;

    if (damageOutcome != COMBAT_OUTCOME_KILLED_NO_CREATURES) return 1;
    if ((projectile->flags & PROJECTILE_FLAG_CREATES_EXPLOSION) != 0) return 1;
    if (associatedThing == THING_NONE || associatedThing == THING_ENDOFLIST) {
        return 1;
    }
    if (THING_GET_TYPE(associatedThing) != THING_TYPE_WEAPON) return 1;
    if ((creatureAttributes & DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34) == 0) {
        return 1;
    }
    if (!dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(
            associatedWeaponType)) {
        return 1;
    }

    outPlan->shouldAttachToGroupSlot = 1;

    /* ReDMCSB: PROJEXPL.C F0217 lines 540-553 passes GROUP.Slot to F0215
     * only for non-exploding sharp weapon projectiles that hit, but do not
     * kill, creatures with MASK0x0400_KEEP_THROWN_SHARP_WEAPONS. */
    return 1;
}

int dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
    unsigned short associatedThing,
    unsigned short groupSlotHead,
    unsigned short tailThing,
    DM1_ProjectileGroupSlotAttachPlanPc34* outPlan) {
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->associatedThing = associatedThing;
    outPlan->groupSlotHead = groupSlotHead;
    outPlan->tailThing = tailThing;

    if (associatedThing == THING_NONE ||
        associatedThing == THING_ENDOFLIST ||
        THING_GET_TYPE(associatedThing) == THING_TYPE_EXPLOSION) {
        return 0;
    }

    outPlan->valid = 1;
    outPlan->shouldSetAssociatedNextEnd = 1;
    if (groupSlotHead == THING_ENDOFLIST) {
        outPlan->shouldSetGroupSlotHead = 1;
    } else if (tailThing != THING_NONE && tailThing != THING_ENDOFLIST) {
        outPlan->shouldAppendAfterTail = 1;
    }

    /* ReDMCSB: PROJEXPL.C F0215 lines 248-256 stores Projectile.Slot as the
     * group Slot head when the possession list is empty; otherwise it appends
     * through DUNGEON.C F0163 after the existing tail. */
    return 1;
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

int dm1_v1_projectile_materialization_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int associatedThingMovedToGroup,
    int potionCount,
    DM1_ProjectileMaterializationPlanPc34* outPlan) {
    DM1_ProjectileAssociatedThingDispositionPc34 disposition;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->associatedThing = THING_NONE;
    outPlan->droppedThing = THING_NONE;
    outPlan->cell = -1;
    if (!projectile) return 0;
    if (!dm1_v1_projectile_associated_thing_disposition_pc34(
            projectile, result, associatedThingMovedToGroup, potionCount,
            &disposition)) {
        return 0;
    }

    mapIndex = projectile->mapIndex;
    mapX = projectile->mapX;
    mapY = projectile->mapY;
    cell = projectile->cell & 3;
    if (result &&
        result->resultKind == PROJECTILE_RESULT_HIT_CHAMPION &&
        result->emittedCombatAction) {
        /* ReDMCSB: PROJEXPL.C F0219 lines 687-697 commits same-square
         * champion impacts, and lines 717-743 commit cross-cell champion
         * impacts before F0217 reaches F0215 delete/materialization. */
        mapIndex = result->newMapIndex;
        mapX = result->newMapX;
        mapY = result->newMapY;
        cell = result->newCell & 3;
    }

    outPlan->handled = 1;
    outPlan->shouldConsumePotion = disposition.shouldConsumePotion;
    outPlan->shouldMaterialize = disposition.shouldMaterialize;
    outPlan->associatedThing = disposition.associatedThing;
    outPlan->mapIndex = mapIndex;
    outPlan->mapX = mapX;
    outPlan->mapY = mapY;
    outPlan->cell = cell;
    outPlan->droppedThing =
        disposition.shouldMaterialize
            ? (unsigned short)((disposition.associatedThing & 0x3FFFu) |
                               (unsigned short)((cell & 0x03) << 14))
            : disposition.droppedThing;
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

int dm1_v1_projectile_creature_impact_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    const struct DungeonGroup_Compat* group,
    int creatureAttributes,
    DM1_ProjectileCreatureImpactPlanPc34* outPlan) {
    int slotIndex;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->slotIndex = -1;
    outPlan->killedCell = EXPLOSION_CELL_CENTERED;
    if (!projectile || !result || !group) return 0;
    if (result->resultKind != PROJECTILE_RESULT_HIT_CREATURE ||
        !result->emittedCombatAction) {
        return 0;
    }
    outPlan->handled = 1;

    /* ReDMCSB: PROJEXPL.C F0217 lines 532-533 lets non-material
     * creatures ignore every projectile except Harm Non Material. */
    if ((creatureAttributes & DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34) &&
        projectile->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL) {
        outPlan->blockedByNonMaterial = 1;
        return 1;
    }

    slotIndex = dm1_v1_group_creature_index_for_cell_pc34(
        group, result->outAction.targetCell);
    if (slotIndex < 0) return 1;

    /* ReDMCSB: PROJEXPL.C F0217 lines 515-539 resolves the ordinal with
     * F0176, records the original cell, then applies F0190 damage. */
    outPlan->shouldApplyDamage = result->outAction.rawAttackValue > 0;
    outPlan->slotIndex = slotIndex;
    outPlan->damageApplied = result->outAction.rawAttackValue;
    outPlan->originalCreatureType = (int)group->creatureType;
    outPlan->originalCells = (int)group->cells;
    outPlan->originalGroupCount = (int)group->count;
    outPlan->killedCell =
        (group->cells == DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34)
            ? EXPLOSION_CELL_CENTERED
            : (int)((group->cells >> (slotIndex * 2)) & 0x03u);
    return 1;
}

int dm1_v1_projectile_creature_impact_aftermath_pc34(
    const DM1_ProjectileCreatureImpactPlanPc34* plan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int groupBehaviorAfterDamage,
    int damageOutcome,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath) {
    if (!outAftermath) return 0;
    memset(outAftermath, 0, sizeof(*outAftermath));
    if (!plan || !projectile || !plan->handled || !plan->shouldApplyDamage) {
        return 0;
    }

    if (damageOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
        outAftermath->cleanupEventsAndFear =
            groupBehaviorAfterDamage == DM1_BEHAVIOR_ATTACK;
        outAftermath->dropFixedPossessions =
            (creatureAttributes &
             DM1_PROJECTILE_ATTR_DROP_FIXED_POSSESSION_PC34) != 0;
    }
    if (damageOutcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES ||
        damageOutcome == COMBAT_OUTCOME_KILLED_SOME_CREATURES) {
        outAftermath->spawnDeathSmoke = 1;
    }
    if (plan->damageApplied > 0 &&
        damageOutcome != COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        /* ReDMCSB: PROJEXPL.C F0217 lines 535-537 schedules F0209 hit
         * reaction for damaged groups that were not fully destroyed. */
        outAftermath->scheduleReaction = 1;
    }
    if (damageOutcome == COMBAT_OUTCOME_KILLED_NO_CREATURES &&
        projectile->projectileCategory == PROJECTILE_CATEGORY_KINETIC &&
        (creatureAttributes &
         DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34) &&
        dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(
            associatedWeaponType)) {
        /* ReDMCSB: PROJEXPL.C F0217 lines 540-553 keeps selected sharp
         * thrown weapons in GROUP.Slot only when no creature died. */
        outAftermath->keepSharpWeaponInGroup = 1;
    }
    return 1;
}

int dm1_v1_projectile_champion_impact_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int championPresent,
    DM1_ProjectileChampionImpactPlanPc34* outPlan) {
    (void)projectile;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    outPlan->impactCell = -1;
    if (!result) return 0;
    if (result->resultKind != PROJECTILE_RESULT_HIT_CHAMPION ||
        !result->emittedCombatAction) {
        return 0;
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 510-558 resolves the champion in
     * P0456_i_Cell, computes F0216 impact attack, calls F0321, then
     * optionally calls F0322 poison. */
    outPlan->handled = 1;
    outPlan->championIndex = result->outAction.defenderSlotOrCreatureIndex;
    outPlan->championPresent = championPresent ? 1 : 0;
    outPlan->impactMapIndex = result->newMapIndex;
    outPlan->impactMapX = result->newMapX;
    outPlan->impactMapY = result->newMapY;
    outPlan->impactCell = result->newCell;
    outPlan->attackTypeCode = result->outAction.attackTypeCode;
    outPlan->rawAttackValue = result->outAction.rawAttackValue;
    outPlan->allowedWounds = result->outAction.allowedWounds;
    return 1;
}

int dm1_v1_projectile_champion_poison_plan_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct ProjectileInstance_Compat* projectile,
    int appliedDamage,
    int championCurrentHealth,
    int championPoisonDose,
    int rng2,
    DM1_ProjectileChampionPoisonPlanPc34* outPlan) {
    int poisonDamage;
    int dose;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    if (!impactPlan || !projectile || !impactPlan->handled ||
        !impactPlan->championPresent) {
        return 0;
    }
    outPlan->championIndex = impactPlan->championIndex;
    if (appliedDamage <= 0 || projectile->poisonAttack <= 0 ||
        championCurrentHealth <= 0 || (rng2 & 1) == 0) {
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 557-558 gates poison after applied
     * damage and RANDOM(2). CHAMPION.C F0322 lines 1949-1960 applies
     * max(1, attack >> 6) damage and schedules attack-1 after 36 ticks. */
    poisonDamage = projectile->poisonAttack >> 6;
    if (poisonDamage < 1) poisonDamage = 1;
    if (poisonDamage > championCurrentHealth) {
        poisonDamage = championCurrentHealth;
    }
    dose = championPoisonDose + projectile->poisonAttack;
    if (dose > 0xFFFF) dose = 0xFFFF;

    outPlan->shouldApply = 1;
    outPlan->poisonDamage = poisonDamage;
    outPlan->newPoisonDose = dose;
    outPlan->nextAttack = projectile->poisonAttack - 1;
    outPlan->scheduleDelayTicks = outPlan->nextAttack > 0 ? 36 : 0;
    return 1;
}

int dm1_v1_projectile_champion_party_death_check_pc34(
    int combatKilledFlag,
    int championCurrentHealth) {
    /* ReDMCSB: CHAMPION.C lines 1659-1667 marks the party dead when the
     * last live champion reaches zero HP. M11 owns the party-wide scan. */
    return combatKilledFlag || championCurrentHealth <= 0;
}

int dm1_v1_explosion_party_damage_fanout_plan_pc34(
    int rawAttackValue,
    int attackTypeCode,
    int allowedWounds,
    DM1_ExplosionPartyDamageFanoutPlanPc34* outPlan) {
    int randomWindow;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    if (rawAttackValue <= 0) return 1;

    /* ReDMCSB: CHAMPION.C F0324 lines 1991-2022 damages all living
     * champions with attack randomized by +/- attack/8 before F0321. */
    randomWindow = (rawAttackValue >> 3) + 1;
    outPlan->handled = 1;
    outPlan->baseAttack = rawAttackValue - randomWindow;
    outPlan->rngModulus = randomWindow << 1;
    outPlan->attackTypeCode = attackTypeCode;
    outPlan->allowedWounds = allowedWounds;
    return 1;
}

int dm1_v1_explosion_party_champion_damage_plan_pc34(
    const DM1_ExplosionPartyDamageFanoutPlanPc34* fanoutPlan,
    int championIndex,
    int championPresent,
    int championCurrentHealth,
    int rngWindowRoll,
    DM1_ExplosionPartyChampionDamagePlanPc34* outPlan) {
    int attack;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = championIndex;
    if (!fanoutPlan || !fanoutPlan->handled ||
        !championPresent || championCurrentHealth <= 0) {
        return 1;
    }

    /* ReDMCSB: CHAMPION.C F0324 clamps each randomized attack to at
     * least 1, then lets F0321 handle shield/defense/wound effects. */
    attack = fanoutPlan->baseAttack + rngWindowRoll;
    if (attack < 1) attack = 1;
    outPlan->shouldAttemptDamage = 1;
    outPlan->randomizedAttack = attack;
    outPlan->attackTypeCode = fanoutPlan->attackTypeCode;
    outPlan->allowedWounds = fanoutPlan->allowedWounds;
    return 1;
}
