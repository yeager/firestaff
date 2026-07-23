#include "dm1_v1_throw_shoot_pc34_compat.h"

#include "dm1_v1_combat_pc34_compat.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
#include "dm1_v1_dungeon_thing_data_pc34_compat.h"
#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"
#include "dm1_v1_projectile_impact_attack_f0216_pc34_compat.h"
#include "memory_champion_lifecycle_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#include <string.h>

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
    DM1_ArmourInfoPc34 info;

    /* F0140 reads the same G0239 ARMOUR_INFO row as F0143. */
    return dm1_armour_info_pc34(armourType, &info) ? info.weight : -1;
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

static int dm1_v1_f0140_maximum_raw_things_pc34(
    const struct DungeonThings_Compat* things)
{
    int type;
    int maximum = 0;

    if (!things) return 0;
    for (type = 0; type < DUNGEON_THING_TYPE_COUNT; ++type) {
        if (things->thingCounts[type] > 0) maximum += things->thingCounts[type];
    }
    return maximum;
}

static int dm1_v1_dungeon_get_object_weight_f0140_impl_pc34(
    const struct DungeonThings_Compat* things,
    unsigned short thing,
    int* remaining,
    int* outWeight)
{
    const unsigned char* raw;
    int type;
    int weight;

    if (outWeight) *outWeight = 0;
    if (!things || !remaining || !outWeight) return 0;
    if (thing == THING_NONE) return 1;
    if (thing == THING_ENDOFLIST || *remaining <= 0) return 0;
    --*remaining;

    raw = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!raw) return 0;
    type = THING_GET_TYPE(thing);
    switch (type) {
    case THING_TYPE_WEAPON: {
        DM1_WeaponInfo info;
        if (!dm1_weapon_info_pc34(raw[2] & 0x7f, &info)) return 0;
        *outWeight = info.weight;
        return 1;
    }
    case THING_TYPE_ARMOUR: {
        DM1_ArmourInfoPc34 info;
        if (!dm1_armour_info_pc34(raw[2] & 0x7f, &info)) return 0;
        *outWeight = info.weight;
        return 1;
    }
    case THING_TYPE_JUNK:
        weight = dm1_v1_throw_junk_base_weight_f0140_pc34(raw[2] & 0x7f);
        if (weight < 0) return 0;
        if ((raw[2] & 0x7f) == DM1_JUNK_WATERSKIN_PC34) {
            weight += ((raw[3] >> 6) & 0x03) << 1;
        }
        *outWeight = weight;
        return 1;
    case THING_TYPE_CONTAINER: {
        unsigned short child = (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8));
        weight = 50;
        while (child != THING_ENDOFLIST) {
            int childWeight;
            if (!dm1_v1_dungeon_get_object_weight_f0140_impl_pc34(
                    things, child, remaining, &childWeight)) {
                return 0;
            }
            weight += childWeight;
            child = F0512_DUNGEON_GetThingNext_Compat(things, child);
        }
        *outWeight = weight;
        return 1;
    }
    case THING_TYPE_POTION:
        *outWeight = (raw[3] & 0x7f) == DM1_POTION_EMPTY_FLASK_PC34 ? 1 : 3;
        return 1;
    case THING_TYPE_SCROLL:
        *outWeight = 1;
        return 1;
    default:
        return 0;
    }
}

int dm1_v1_dungeon_get_object_weight_f0140_pc34(
    const struct DungeonThings_Compat* things,
    unsigned short thing,
    int* outWeight)
{
    int remaining;

    if (outWeight) *outWeight = 0;
    if (!outWeight || !things || !things->loaded) return 0;
    remaining = dm1_v1_f0140_maximum_raw_things_pc34(things);
    return dm1_v1_dungeon_get_object_weight_f0140_impl_pc34(
        things, thing, &remaining, outWeight);
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

static int dm1_v1_f0810_raw_thing_size_pc34(int thingType)
{
    switch (thingType) {
    case THING_TYPE_CONTAINER: return 8;
    case THING_TYPE_WEAPON:
    case THING_TYPE_ARMOUR:
    case THING_TYPE_SCROLL:
    case THING_TYPE_POTION:
    case THING_TYPE_JUNK: return 4;
    default: return 0;
    }
}

static uint32_t dm1_v1_f0810_fnv1a_pc34(const unsigned char *bytes, int count)
{
    uint32_t hash = 2166136261u;
    int i;
    if (!bytes || count <= 0) return 0u;
    for (i = 0; i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_build_projectile_create_input_source_bound_pc34(
    const DM1_ProjectileCreateRequestPc34* req,
    const struct DungeonThings_Compat* things,
    struct ProjectileCreateInput_Compat* outInput,
    DM1_ProjectileCreateSourceReceiptPc34* outReceipt)
{
    DM1_ProjectileCreateSourceReceiptPc34 receipt;
    const unsigned char *raw;
    int type;
    int index;
    int rawSize;
    uint32_t inputHash;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB CHAMPION.C F0328 -> PROJEXPL.C F0212/F0810; "
        "DUNGEON.C F0156 raw carried Thing";
    *outReceipt = receipt;
    if (!req || !things || !things->loaded || !outInput ||
        req->carriedThing == THING_NONE || req->carriedThing == THING_ENDOFLIST) {
        return 0;
    }
    type = (int)THING_GET_TYPE(req->carriedThing);
    index = (int)THING_GET_INDEX(req->carriedThing);
    rawSize = dm1_v1_f0810_raw_thing_size_pc34(type);
    raw = dm1_v1_dungeon_get_thing_data_pc34(things, req->carriedThing);
    if (!raw || rawSize == 0 || index < 0 || index >= things->thingCounts[type]) {
        return 0;
    }
    if (!dm1_v1_build_projectile_create_input_pc34(req, outInput) ||
        (unsigned short)outInput->associatedThing != req->carriedThing) {
        return 0;
    }
    /* Bind the F0810 input to both original object bytes and its complete
     * source-shaped launch fields. This is a receipt, not a fallback creator. */
    inputHash = dm1_v1_f0810_fnv1a_pc34((const unsigned char *)outInput,
                                         (int)sizeof(*outInput));
    receipt.rawThingFNV1a = dm1_v1_f0810_fnv1a_pc34(raw, rawSize);
    if (receipt.rawThingFNV1a == 0u || inputHash == 0u) return 0;
    receipt.valid = 1;
    receipt.associatedThing = req->carriedThing;
    receipt.thingType = type;
    receipt.thingIndex = index;
    receipt.createInputFNV1a = inputHash;
    *outReceipt = receipt;
    return 1;
}

int dm1_v1_projectile_subtype_from_thing_pc34(int projectileThing,
                                              int* outSubtype) {
    int subtype;
    switch (projectileThing) {
        case DM1_PROJECTILE_THING_FIREBALL:
            subtype = PROJECTILE_SUBTYPE_FIREBALL;
            break;
        case DM1_PROJECTILE_THING_SLIME:
            subtype = PROJECTILE_SUBTYPE_SLIME;
            break;
        case DM1_PROJECTILE_THING_LIGHTNING_BOLT:
            subtype = PROJECTILE_SUBTYPE_LIGHTNING_BOLT;
            break;
        case DM1_PROJECTILE_THING_HARM_NON_MATERIAL:
            subtype = PROJECTILE_SUBTYPE_HARM_NON_MATERIAL;
            break;
        case DM1_PROJECTILE_THING_OPEN_DOOR:
            subtype = PROJECTILE_SUBTYPE_OPEN_DOOR;
            break;
        case DM1_PROJECTILE_THING_POISON_CLOUD:
            subtype = PROJECTILE_SUBTYPE_POISON_CLOUD;
            break;
        default:
            if (outSubtype) *outSubtype = -1;
            return 0;
    }
    if (outSubtype) *outSubtype = subtype;
    return 1;
}

int dm1_v1_projectile_attack_type_for_subtype_pc34(int subtype) {
    switch (subtype) {
        case PROJECTILE_SUBTYPE_FIREBALL:
            return COMBAT_ATTACK_FIRE;
        case PROJECTILE_SUBTYPE_LIGHTNING_BOLT:
            return COMBAT_ATTACK_LIGHTNING;
        case PROJECTILE_SUBTYPE_HARM_NON_MATERIAL:
        case PROJECTILE_SUBTYPE_OPEN_DOOR:
            return COMBAT_ATTACK_MAGIC;
        case PROJECTILE_SUBTYPE_SLIME:
        case PROJECTILE_SUBTYPE_POISON_CLOUD:
        default:
            return COMBAT_ATTACK_NORMAL;
    }
}

int dm1_v1_spell_projectile_launch_plan_f0327_pc34(
    const DM1_SpellF0412RuntimeReceipt* receipt,
    const DM1_SpellF0327ProjectileContextPc34* context,
    DM1_SpellF0327ProjectileLaunchPlanPc34* outPlan) {
    int subtype;
    int kineticEnergy;
    int stepEnergy;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->projectileThing = THING_NONE;
    outPlan->projectileSubtype = -1;
    outPlan->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    outPlan->attackTypeCode = COMBAT_ATTACK_NORMAL;
    outPlan->attack = 90;
    outPlan->launchCell = -1;
    outPlan->launchDirection = -1;

    if (!receipt || !context) return 0;
    if (receipt->castResult != DM1_SPELL_CAST_SUCCESS ||
        !receipt->createsProjectile) {
        return 1;
    }
    if (context->championIndex < 0 ||
        !dm1_v1_projectile_subtype_from_thing_pc34(
            receipt->projectileThing, &subtype)) {
        return 0;
    }

    kineticEnergy = receipt->projectileKineticEnergy;
    stepEnergy = receipt->projectileStepEnergy > 0
                     ? receipt->projectileStepEnergy
                     : 1;
    if (kineticEnergy < (stepEnergy << 2)) {
        kineticEnergy += 3;
        --stepEnergy;
    }
    if (stepEnergy < 1) stepEnergy = 1;

    /* ReDMCSB CHAMPION.C F0327:2091-2102 consumes F0412's projectile thing
     * and kinetic receipt, subtracts required mana, adjusts weak projectiles
     * by (+3 kinetic, -1 step), then calls F0326.  F0326:2064-2066 uses the
     * champion direction/cell to create the F0212 projectile at attack 90. */
    outPlan->valid = 1;
    outPlan->shouldCreateProjectile = 1;
    outPlan->projectileThing = (unsigned short)receipt->projectileThing;
    outPlan->projectileSubtype = subtype;
    outPlan->projectileCategory = PROJECTILE_CATEGORY_MAGICAL;
    outPlan->attackTypeCode =
        dm1_v1_projectile_attack_type_for_subtype_pc34(subtype);
    outPlan->kineticEnergyBeforeF0327 = receipt->projectileKineticEnergy;
    outPlan->kineticEnergyAfterF0327 = kineticEnergy;
    outPlan->stepEnergyBeforeF0327 = receipt->projectileStepEnergy;
    outPlan->stepEnergyAfterF0327 = stepEnergy;
    outPlan->launchDirection = receipt->championDirectionAfter & 3;
    outPlan->launchCell = dm1_v1_projectile_launch_cell_pc34(
        context->championCell, outPlan->launchDirection);
    outPlan->movementDisabledTicks = 0;
    outPlan->lastProjectileDisabledMovementDirection = -1;
    return 1;
}

int dm1_v1_build_spell_projectile_create_input_f0327_pc34(
    const DM1_SpellF0412RuntimeReceipt* receipt,
    const DM1_SpellF0327ProjectileContextPc34* context,
    struct ProjectileCreateInput_Compat* outInput) {
    DM1_SpellF0327ProjectileLaunchPlanPc34 plan;

    if (!outInput) return 0;
    memset(outInput, 0, sizeof(*outInput));
    if (!dm1_v1_spell_projectile_launch_plan_f0327_pc34(
            receipt, context, &plan)) {
        return 0;
    }
    if (!plan.valid || !plan.shouldCreateProjectile) {
        return 1;
    }

    outInput->category = plan.projectileCategory;
    outInput->subtype = plan.projectileSubtype;
    outInput->ownerKind = PROJECTILE_OWNER_CHAMPION;
    outInput->ownerIndex = context->championIndex;
    outInput->mapIndex = context->partyMapIndex;
    outInput->mapX = context->partyMapX;
    outInput->mapY = context->partyMapY;
    outInput->cell = plan.launchCell;
    outInput->direction = plan.launchDirection;
    outInput->kineticEnergy = plan.kineticEnergyAfterF0327;
    outInput->attack = plan.attack;
    outInput->launcherStrength = plan.attack;
    outInput->stepEnergy = plan.stepEnergyAfterF0327;
    outInput->currentTick = context->gameTick;
    outInput->attackTypeCode = plan.attackTypeCode;
    outInput->associatedThing = THING_NONE;
    outInput->firstMoveGraceFlag = 1;
    return 1;
}

int dm1_v1_build_creature_projectile_create_input_pc34(
    const DM1_CreatureProjectileCreateRequestPc34* req,
    struct ProjectileCreateInput_Compat* outInput) {
    int subtype;
    if (!req || !outInput) return 0;
    if (req->creatureGroupIndex < 0) return 0;
    if (!dm1_v1_projectile_subtype_from_thing_pc34(
            req->projectileThing, &subtype)) {
        return 0;
    }

    /* ReDMCSB: GROUP.C F0207/F0209 choose the projectile EXPLOSION thing
     * and PROJEXPL.C F0212 consumes it as a creature-owned projectile.
     * DM1 owns the thing->subtype, attack type, poison attack, and F0810
     * create-input shape; M11 supplies only live position/tick facts. */
    memset(outInput, 0, sizeof(*outInput));
    outInput->category = PROJECTILE_CATEGORY_MAGICAL;
    outInput->subtype = subtype;
    outInput->ownerKind = PROJECTILE_OWNER_CREATURE;
    outInput->ownerIndex = req->creatureGroupIndex;
    outInput->mapIndex = req->partyMapIndex;
    outInput->mapX = req->groupMapX;
    outInput->mapY = req->groupMapY;
    outInput->cell = req->targetCell & 3;
    outInput->direction = req->direction & 3;
    outInput->kineticEnergy = req->kineticEnergy;
    outInput->attack = req->attack;
    outInput->launcherStrength = req->attack;
    outInput->stepEnergy = req->stepEnergy > 0 ? req->stepEnergy : 1;
    outInput->currentTick = req->gameTick;
    outInput->poisonAttack =
        (subtype == PROJECTILE_SUBTYPE_POISON_CLOUD) ? req->attack : 0;
    outInput->attackTypeCode =
        dm1_v1_projectile_attack_type_for_subtype_pc34(subtype);
    outInput->associatedThing = THING_NONE;
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

int dm1_v1_projectile_group_slot_attach_receipt_f0215_pc34(
    unsigned short associatedThing,
    unsigned short groupSlotHead,
    const unsigned short* chainThings,
    int chainCount,
    DM1_ProjectileGroupSlotAttachReceiptPc34* outReceipt) {
    DM1_ProjectileGroupSlotAttachPlanPc34 plan;
    unsigned short tailThing = THING_NONE;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->associatedThing = associatedThing;
    outReceipt->groupSlotHead = groupSlotHead;
    outReceipt->tailThing = THING_NONE;

    if (!dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
            associatedThing, groupSlotHead, THING_NONE, &plan) ||
        !plan.valid ||
        !plan.shouldSetAssociatedNextEnd) {
        return 0;
    }

    outReceipt->valid = 1;
    outReceipt->shouldSetAssociatedNextEnd = 1;
    if (plan.shouldSetGroupSlotHead) {
        outReceipt->shouldSetGroupSlotHead = 1;
        return 1;
    }

    if (!chainThings || chainCount <= 0) {
        return 1;
    }
    for (i = 0; i < chainCount; ++i) {
        unsigned short thing = chainThings[i];
        if (thing == THING_NONE || thing == THING_ENDOFLIST) {
            break;
        }
        tailThing = thing;
    }
    if (i >= chainCount) {
        outReceipt->chainOverflow = 1;
        return 1;
    }
    if (tailThing == THING_NONE || tailThing == THING_ENDOFLIST) {
        return 1;
    }

    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
            associatedThing, groupSlotHead, tailThing, &plan) ||
        !plan.valid ||
        !plan.shouldAppendAfterTail ||
        !plan.shouldSetAssociatedNextEnd) {
        return 1;
    }
    outReceipt->foundTail = 1;
    outReceipt->shouldAppendAfterTail = 1;
    outReceipt->tailThing = tailThing;

    /* ReDMCSB: PROJEXPL.C F0215 lines 248-256 links Projectile.Slot into
     * GROUP.Slot; DUNGEON.C F0163 lines 1798-1837 walks until end-of-list
     * before appending. M10 supplies the observed chain, while this DM1
     * receipt owns the empty-vs-tail append decision. */
    return 1;
}

int dm1_v1_projectile_square_attach_plan_f0215_pc34(
    unsigned short droppedThing,
    unsigned short squareFirstThing,
    unsigned short tailThing,
    DM1_ProjectileSquareAttachPlanPc34* outPlan) {
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->droppedThing = droppedThing;
    outPlan->baseThing = (unsigned short)(droppedThing & 0x3FFFu);
    outPlan->squareFirstThing = squareFirstThing;
    outPlan->tailThing = tailThing;

    if (droppedThing == THING_NONE ||
        droppedThing == THING_ENDOFLIST ||
        THING_GET_TYPE(droppedThing) == THING_TYPE_EXPLOSION) {
        return 0;
    }

    outPlan->valid = 1;
    outPlan->shouldSetDroppedNextEnd = 1;
    if (squareFirstThing == THING_NONE ||
        squareFirstThing == THING_ENDOFLIST) {
        outPlan->shouldSetSquareFirstThing = 1;
    } else if (tailThing != THING_NONE && tailThing != THING_ENDOFLIST) {
        outPlan->shouldAppendAfterTail = 1;
    }

    /* ReDMCSB: PROJEXPL.C F0215 lines 248-259 materializes Projectile.Slot
     * on the projectile square; DUNGEON.C F0163 lines 1798-1837 appends to
     * an existing square chain instead of replacing the first thing. */
    return 1;
}

int dm1_v1_projectile_square_attach_receipt_f0215_pc34(
    unsigned short droppedThing,
    unsigned short squareFirstThing,
    const unsigned short* chainThings,
    int chainCount,
    DM1_ProjectileSquareAttachReceiptPc34* outReceipt) {
    DM1_ProjectileSquareAttachPlanPc34 plan;
    unsigned short tailThing = THING_NONE;
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->droppedThing = droppedThing;
    outReceipt->baseThing = (unsigned short)(droppedThing & 0x3FFFu);
    outReceipt->squareFirstThing = squareFirstThing;
    outReceipt->tailThing = THING_NONE;

    if (!dm1_v1_projectile_square_attach_plan_f0215_pc34(
            droppedThing, squareFirstThing, THING_NONE, &plan) ||
        !plan.valid ||
        !plan.shouldSetDroppedNextEnd) {
        return 0;
    }

    outReceipt->valid = 1;
    outReceipt->shouldSetDroppedNextEnd = 1;
    if (plan.shouldSetSquareFirstThing) {
        outReceipt->shouldSetSquareFirstThing = 1;
        return 1;
    }

    if (!chainThings || chainCount <= 0) {
        return 1;
    }
    for (i = 0; i < chainCount; ++i) {
        unsigned short thing = chainThings[i];
        if (thing == THING_NONE || thing == THING_ENDOFLIST) {
            break;
        }
        tailThing = thing;
    }
    if (i >= chainCount) {
        outReceipt->chainOverflow = 1;
        return 1;
    }
    if (tailThing == THING_NONE || tailThing == THING_ENDOFLIST) {
        return 1;
    }

    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_projectile_square_attach_plan_f0215_pc34(
            droppedThing, squareFirstThing, tailThing, &plan) ||
        !plan.valid ||
        !plan.shouldAppendAfterTail ||
        !plan.shouldSetDroppedNextEnd) {
        return 1;
    }
    outReceipt->foundTail = 1;
    outReceipt->shouldAppendAfterTail = 1;
    outReceipt->tailThing = tailThing;

    /* ReDMCSB: DUNGEON.C F0163 lines 1798-1837 walks the existing list
     * until C0xFFFE_THING_ENDOFLIST, clears Next for the inserted thing,
     * and then links it after the tail.  M10 supplies the observed chain;
     * this DM1 receipt owns the empty-vs-append decision. */
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

    if (associatedType != THING_TYPE_WEAPON &&
        associatedType != THING_TYPE_ARMOUR &&
        associatedType != THING_TYPE_SCROLL &&
        associatedType != THING_TYPE_POTION &&
        associatedType != THING_TYPE_CONTAINER &&
        associatedType != THING_TYPE_JUNK &&
        associatedType != THING_TYPE_EXPLOSION) {
        /* ReDMCSB PROJEXPL.C F0215 lines 248-259 only materializes a
         * projectile Slot when it is a carried object.  Runtime C14
         * projectile things and structural dungeon things must not become
         * floor objects after an impact; otherwise stale HoC lists can show
         * fireballs or group/sensor refs as loose items. */
        return 1;
    }

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

int dm1_v1_projectile_materialization_receipt_f0215_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int associatedThingMovedToGroup,
    int potionCount,
    unsigned short squareFirstThing,
    const unsigned short* squareChainThings,
    int squareChainCount,
    DM1_ProjectileMaterializationReceiptPc34* outReceipt) {
    DM1_ProjectileMaterializationPlanPc34 plan;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!dm1_v1_projectile_materialization_plan_pc34(
            projectile, result, associatedThingMovedToGroup, potionCount,
            &plan)) {
        return 0;
    }

    outReceipt->valid = 1;
    outReceipt->handled = plan.handled;
    outReceipt->shouldDeleteProjectile = 1;
    outReceipt->shouldClearProjectileNext = 1;
    outReceipt->shouldConsumePotion = plan.shouldConsumePotion;
    outReceipt->shouldMaterialize = plan.shouldMaterialize;
    outReceipt->mapIndex = plan.mapIndex;
    outReceipt->mapX = plan.mapX;
    outReceipt->mapY = plan.mapY;
    outReceipt->cell = plan.cell;
    outReceipt->shouldUnlinkProjectileFromSquare = 1;
    /* F0219 may commit a champion impact to its landing square before
     * F0215 materializes Projectile.Slot there.  The C14 projectile thing
     * itself still belongs to its stored source square until F0215 deletes
     * it, so its unlink coordinates must not borrow the drop coordinates. */
    outReceipt->cleanupMapIndex = projectile->mapIndex;
    outReceipt->cleanupMapX = projectile->mapX;
    outReceipt->cleanupMapY = projectile->mapY;
    outReceipt->projectileThing =
        (unsigned short)(((THING_TYPE_PROJECTILE << 10) |
                          (projectile->slotIndex & 0x03ff)) |
                         (unsigned short)((projectile->cell & 0x03) << 14));
    outReceipt->projectileNextAfterDelete = THING_NONE;
    outReceipt->materialization = plan;

    if (!plan.shouldMaterialize) {
        /* ReDMCSB: PROJEXPL.C F0215 lines 248-260 always deletes the
         * Projectile thing after any optional Slot materialization. */
        return 1;
    }

    if (!dm1_v1_projectile_square_attach_receipt_f0215_pc34(
            plan.droppedThing, squareFirstThing, squareChainThings,
            squareChainCount, &outReceipt->squareAttach)) {
        outReceipt->valid = 0;
        return 0;
    }

    /* ReDMCSB: PROJEXPL.C F0215 lines 248-260 owns the final
     * Projectile.Slot materialization; DUNGEON.C F0163 lines 1798-1837
     * owns empty-square vs append-after-tail linking.  This receipt gives
     * M10 one DM1-owned decision packet instead of rebuilding both parts. */
    return 1;
}

int dm1_v1_projectile_flight_relink_receipt_f0219_pc34(
    const struct ProjectileInstance_Compat* before,
    const struct ProjectileInstance_Compat* after,
    const struct ProjectileTickResult_Compat* result,
    DM1_ProjectileFlightRelinkReceiptPc34* outReceipt) {
    unsigned short projectileBase;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!before || !after || !result) return 0;
    if (before->slotIndex < 0 || before->slotIndex > 0x03ff) return 0;
    if (result->despawn) return 0;
    if (result->resultKind != PROJECTILE_RESULT_FLEW) return 0;
    if (after->mapIndex != result->newMapIndex ||
        after->mapX != result->newMapX ||
        after->mapY != result->newMapY ||
        (after->cell & 3) != (result->newCell & 3) ||
        (after->direction & 3) != (result->newDirection & 3) ||
        after->kineticEnergy != result->newKineticEnergy ||
        after->attack != result->newAttack ||
        (after->firstMoveGraceFlag ? 1 : 0) !=
            (result->newFirstMoveGraceFlag ? 1 : 0)) {
        return 0;
    }

    projectileBase =
        (unsigned short)((THING_TYPE_PROJECTILE << 10) |
                         (before->slotIndex & 0x03ff));
    outReceipt->valid = 1;
    outReceipt->shouldApply = 1;
    outReceipt->shouldWriteProjectileState = 1;
    outReceipt->shouldScheduleNextMove = 1;
    outReceipt->sourceMapIndex = before->mapIndex;
    outReceipt->sourceMapX = before->mapX;
    outReceipt->sourceMapY = before->mapY;
    outReceipt->sourceCell = before->cell & 3;
    outReceipt->destinationMapIndex = result->newMapIndex;
    outReceipt->destinationMapX = result->newMapX;
    outReceipt->destinationMapY = result->newMapY;
    outReceipt->destinationCell = result->newCell & 3;
    outReceipt->destinationDirection = result->newDirection & 3;
    outReceipt->destinationKineticEnergy = result->newKineticEnergy;
    outReceipt->destinationAttack = result->newAttack;
    outReceipt->destinationFirstMoveGraceFlag =
        result->newFirstMoveGraceFlag ? 1 : 0;
    outReceipt->sourceProjectileThing =
        (unsigned short)(projectileBase |
                         (unsigned short)((before->cell & 3) << 14));
    outReceipt->destinationProjectileThing =
        (unsigned short)(projectileBase |
                         (unsigned short)((result->newCell & 3) << 14));

    if (before->mapIndex != result->newMapIndex ||
        before->mapX != result->newMapX ||
        before->mapY != result->newMapY ||
        (before->cell & 3) != (result->newCell & 3)) {
        outReceipt->shouldUnlinkSourceSquare = 1;
        outReceipt->shouldLinkDestinationSquare = 1;
    }

    /* ReDMCSB: PROJEXPL.C F0219 lines 735-762 moves a live projectile by
     * unlinking/linking the Thing when the square/cell changes, then writes
     * MapX/MapY/Direction/Cell and schedules the next C48/C49 event.  M10
     * consumes this receipt so those live mutation decisions stay DM1-owned
     * instead of being reconstructed beside the timeline dispatcher. */
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

int dm1_v1_projectile_creature_action_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct CombatAction_Compat* action,
    const struct DungeonGroup_Compat* group,
    int creatureAttributes,
    DM1_ProjectileCreatureActionPlanPc34* outPlan) {
    int slotIndex;
    int healed;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->slotIndex = -1;
    outPlan->killedCell = EXPLOSION_CELL_CENTERED;
    if (!projectile || !action || !group) return 0;
    if (action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP ||
        action->rawAttackValue <= 0) {
        return 0;
    }
    outPlan->handled = 1;

    slotIndex = dm1_v1_group_creature_index_for_cell_pc34(
        group, action->targetCell);
    if (slotIndex < 0) return 1;

    outPlan->slotIndex = slotIndex;
    outPlan->damageApplied = action->rawAttackValue;
    outPlan->originalCreatureType = (int)group->creatureType;
    outPlan->originalCells = (int)group->cells;
    outPlan->originalGroupCount = (int)group->count;
    outPlan->killedCell =
        (group->cells == DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34)
            ? EXPLOSION_CELL_CENTERED
            : (int)((group->cells >> (slotIndex * 2)) & 0x03u);

    if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL &&
        group->creatureType == DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34) {
        /* ReDMCSB: PROJEXPL.C F0217 lines 527-531 heals Black Flame on
         * Fireball impact up to 1000 HP and skips F0190 damage. */
        healed = (int)group->health[slotIndex] + action->rawAttackValue;
        if (healed > DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34) {
            healed = DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34;
        }
        outPlan->healsBlackFlame = 1;
        outPlan->newHealth = healed;
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 532-533 lets non-material
     * creatures ignore all projectiles except Harm Non Material after
     * the prior Black Flame fireball-heal branch has had priority. */
    if ((creatureAttributes & DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34) &&
        projectile->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL) {
        outPlan->blockedByNonMaterial = 1;
        return 1;
    }

    outPlan->shouldApplyDamage = 1;
    return 1;
}

int dm1_v1_projectile_creature_action_apply_pc34(
    const DM1_ProjectileCreatureActionPlanPc34* actionPlan,
    struct DungeonGroup_Compat* group,
    DM1_ProjectileCreatureActionApplyPlanPc34* outPlan) {
    int outcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->outcomeCode = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    outPlan->creatureIndex = -1;
    if (!actionPlan || !actionPlan->handled || !group) return 0;
    outPlan->valid = 1;
    if (!actionPlan->shouldApplyDamage || actionPlan->slotIndex < 0) {
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 515-539 consumes the bounded
     * projectile group-action payload and applies it through GROUP.C F0190.
     * Firestaff's F0738 is the live GROUP mutation entrypoint for F0190. */
    outPlan->handled = 1;
    outPlan->creatureIndex = actionPlan->slotIndex;
    outPlan->damageApplied = actionPlan->damageApplied;
    outPlan->damage.damageApplied = actionPlan->damageApplied;
    if (!F0738_COMBAT_ApplyDamageToGroup_Compat(
            &outPlan->damage, group, actionPlan->slotIndex, &outcome)) {
        return 0;
    }
    outPlan->outcomeCode = outcome;
    return 1;
}

int dm1_v1_projectile_creature_action_aftermath_pc34(
    const DM1_ProjectileCreatureActionPlanPc34* actionPlan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int groupBehaviorAfterDamage,
    int damageOutcome,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath) {
    if (!outAftermath) return 0;
    memset(outAftermath, 0, sizeof(*outAftermath));
    if (!actionPlan || !projectile ||
        !actionPlan->handled || !actionPlan->shouldApplyDamage) {
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
    if (actionPlan->damageApplied > 0 &&
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

int dm1_v1_projectile_creature_precheck_damage_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct DungeonGroup_Compat* group,
    int creatureIndex,
    int creatureDefense,
    int creatureAttributes,
    DM1_ProjectileCreaturePrecheckDamagePlanPc34* outPlan) {
    int i;
    int impactAttack;
    int defense;
    int damage;
    int killedCell;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->killedCell = EXPLOSION_CELL_CENTERED;
    if (!projectile || !group || creatureIndex < 0 || creatureIndex > 3) {
        return 0;
    }
    if (creatureIndex > (int)group->count) return 0;
    for (i = 0; i < 4; ++i) outPlan->newHealth[i] = group->health[i];
    outPlan->newCount = group->count;
    outPlan->newCells = group->cells;
    outPlan->valid = 1;
    outPlan->handled = 1;

    impactAttack = F0216_PROJECTILE_GetImpactAttack(projectile);
    if (impactAttack < 0) return 0;
    if (projectile->projectileSubtype == PROJECTILE_SUBTYPE_FIREBALL &&
        group->creatureType == DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34) {
        int healed = (int)group->health[creatureIndex] + impactAttack;
        if (healed > DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34) {
            healed = DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34;
        }
        /* ReDMCSB: PROJEXPL.C F0217 lines 527-531 heals Black Flame on
         * fireball impact and skips normal F0190 creature damage. */
        outPlan->shouldWriteGroup = 1;
        outPlan->newHealth[creatureIndex] = (unsigned short)healed;
        return 1;
    }

    if ((creatureAttributes & DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34) &&
        projectile->projectileSubtype != PROJECTILE_SUBTYPE_HARM_NON_MATERIAL) {
        return 1;
    }

    defense = creatureDefense > 0 ? creatureDefense : 64;
    damage = (impactAttack << 6) / defense;
    if (damage <= 0) return 1;

    outPlan->shouldWriteGroup = 1;
    outPlan->damageApplied = damage;
    killedCell =
        (group->cells == DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34)
            ? EXPLOSION_CELL_CENTERED
            : (int)((group->cells >> (creatureIndex * 2)) & 0x03u);
    outPlan->killedCell = killedCell;
    if (group->health[creatureIndex] > (unsigned int)damage) {
        outPlan->newHealth[creatureIndex] =
            (unsigned short)(group->health[creatureIndex] - damage);
        return 1;
    }

    outPlan->newHealth[creatureIndex] = 0;
    if (group->count == 0) {
        outPlan->outcomeCode = 2;
        return 1;
    }
    for (i = creatureIndex; i < (int)group->count && i < 3; ++i) {
        outPlan->newHealth[i] = group->health[i + 1];
        if (outPlan->newCells !=
            DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34) {
            int shift = i * 2;
            int nextCell = (group->cells >> ((i + 1) * 2)) & 0x03u;
            outPlan->newCells =
                (unsigned char)((outPlan->newCells & ~(0x03u << shift)) |
                                ((nextCell & 0x03u) << shift));
        }
    }
    outPlan->newHealth[group->count] = 0;
    if (outPlan->newCells != DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34) {
        outPlan->newCells = (unsigned char)(outPlan->newCells & 0x3Fu);
    }
    outPlan->newCount = (unsigned char)(group->count - 1u);
    outPlan->outcomeCode = 1;
    /* ReDMCSB: MOVESENS.C F0266 lines 292-301 invokes F0217 during group
     * movement projectile prechecks; F0217 then resolves Black Flame,
     * non-material, defense scaling, and F0190-style slot compaction. */
    return 1;
}

int dm1_v1_projectile_creature_precheck_aftermath_pc34(
    const DM1_ProjectileCreaturePrecheckDamagePlanPc34* precheckPlan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath) {
    if (!outAftermath) return 0;
    memset(outAftermath, 0, sizeof(*outAftermath));
    if (!precheckPlan || !projectile ||
        !precheckPlan->valid || !precheckPlan->handled ||
        !precheckPlan->shouldWriteGroup) {
        return 0;
    }

    if (precheckPlan->damageApplied > 0 &&
        precheckPlan->outcomeCode != 2 &&
        precheckPlan->outcomeCode != COMBAT_OUTCOME_KILLED_ALL_CREATURES) {
        outAftermath->scheduleReaction = 1;
    }
    if ((precheckPlan->outcomeCode == 0 ||
         precheckPlan->outcomeCode == COMBAT_OUTCOME_KILLED_NO_CREATURES) &&
        projectile->projectileCategory == PROJECTILE_CATEGORY_KINETIC &&
        (creatureAttributes &
         DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34) &&
        dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(
            associatedWeaponType)) {
        /* ReDMCSB: MOVESENS.C F0266 lines 292-301 reuses F0217 while a
         * group enters/leaves a projectile square; F0217 lines 540-553 keeps
         * selected sharp thrown weapons in GROUP.Slot after non-killing hits. */
        outAftermath->keepSharpWeaponInGroup = 1;
    }
    return 1;
}

int dm1_v1_projectile_champion_action_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct CombatAction_Compat* action,
    int championPresent,
    DM1_ProjectileChampionImpactPlanPc34* outPlan) {
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    outPlan->impactCell = -1;
    if (!action) return 0;
    if (action->kind != COMBAT_ACTION_APPLY_DAMAGE_CHAMPION) {
        return 0;
    }

    /* ReDMCSB: PROJEXPL.C F0217 computes F0216 before it publishes this
     * damage action.  The action is therefore the authenticated impact
     * receipt at this boundary; recomputing from a later projectile state
     * loses the original attack after a C48/C49 transition. */
    if (!projectile || action->rawAttackValue < 0) return 0;
    outPlan->handled = 1;
    outPlan->championIndex = action->defenderSlotOrCreatureIndex;
    outPlan->championPresent = championPresent ? 1 : 0;
    outPlan->impactMapIndex = action->targetMapIndex;
    outPlan->impactMapX = action->targetMapX;
    outPlan->impactMapY = action->targetMapY;
    outPlan->impactCell = action->targetCell;
    outPlan->attackTypeCode = action->attackTypeCode;
    outPlan->rawAttackValue = action->rawAttackValue;
    outPlan->allowedWounds = action->allowedWounds;
    return 1;
}

int dm1_v1_projectile_champion_impact_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int championPresent,
    DM1_ProjectileChampionImpactPlanPc34* outPlan) {
    struct CombatAction_Compat action;
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    outPlan->impactCell = -1;
    if (!result) return 0;
    if (result->resultKind != PROJECTILE_RESULT_HIT_CHAMPION ||
        !result->emittedCombatAction) {
        return 0;
    }
    action = result->outAction;
    action.kind = COMBAT_ACTION_APPLY_DAMAGE_CHAMPION;
    if (!dm1_v1_projectile_champion_action_plan_pc34(
            projectile, &action, championPresent, outPlan)) {
        return 0;
    }
    outPlan->impactMapIndex = result->newMapIndex;
    outPlan->impactMapX = result->newMapX;
    outPlan->impactMapY = result->newMapY;
    outPlan->impactCell = result->newCell;
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

int dm1_v1_projectile_champion_damage_apply_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct ChampionState_Compat* champion,
    DM1_ProjectileChampionDamageApplyPlanPc34* outPlan) {
    int selectedWounds = 0;
    int killed = 0;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    if (!impactPlan || !impactPlan->handled ||
        !impactPlan->championPresent) {
        outPlan->valid = 1;
        return 1;
    }
    outPlan->championIndex = impactPlan->championIndex;
    if (!defender || !rng || !champion) return 0;

    /* ReDMCSB: PROJEXPL.C F0217 lines 513-558 routes champion projectile
     * impact through CHAMPION.C F0321; F0321 applies attack-type defense,
     * armour/vitality scaling, pending wound selection, then HP mutation. */
    if (!F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
            impactPlan->attackTypeCode,
            impactPlan->rawAttackValue,
            impactPlan->allowedWounds,
            defender,
            rng,
            &outPlan->scaledAttack,
            NULL)) {
        return 0;
    }
    outPlan->valid = 1;
    if (outPlan->scaledAttack <= 0) {
        return 1;
    }
    if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
            outPlan->scaledAttack,
            impactPlan->allowedWounds,
            defender,
            rng,
            &selectedWounds,
            NULL)) {
        return 0;
    }
    outPlan->selectedWounds = selectedWounds;
    outPlan->damage.damageApplied = outPlan->scaledAttack;
    outPlan->damage.woundMaskAdded = selectedWounds;
    if (!F0737_COMBAT_ApplyDamageToChampion_Compat(
            &outPlan->damage, champion, &killed)) {
        return 0;
    }
    outPlan->killed = killed ? 1 : 0;
    return 1;
}

int dm1_v1_projectile_champion_poison_apply_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct ProjectileInstance_Compat* projectile,
    int appliedDamage,
    int rng2,
    uint32_t gameTick,
    int poisonEventMapIndex,
    int poisonEventMapX,
    int poisonEventMapY,
    struct ChampionState_Compat* champion,
    DM1_ProjectileChampionPoisonApplyPlanPc34* outPlan) {
    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    if (!impactPlan || !champion) return 0;
    outPlan->championIndex = impactPlan->championIndex;

    if (!dm1_v1_projectile_champion_poison_plan_pc34(
            impactPlan, projectile, appliedDamage, (int)champion->hp.current,
            (int)champion->poisonDose, rng2, &outPlan->poisonPlan)) {
        return 0;
    }
    outPlan->valid = 1;
    if (!outPlan->poisonPlan.shouldApply) {
        return 1;
    }

    /* ReDMCSB: PROJEXPL.C F0217 lines 557-558 enters CHAMPION.C F0322
     * only after F0321 applied damage. CHAMPION.C F0322 lines 1954-1960
     * schedules C75 using the current party map/time; the orchestrator passes
     * the already resolved impact coordinates so stale party mirrors cannot
     * place delayed poison status on another map. */
    outPlan->shouldApply = 1;
    champion->hp.current =
        (unsigned short)((int)champion->hp.current -
                         outPlan->poisonPlan.poisonDamage);
    champion->poisonDose = (unsigned short)outPlan->poisonPlan.newPoisonDose;
    outPlan->championDown = champion->hp.current == 0 ? 1 : 0;
    /* ReDMCSB CHAMPION.C F0319 lines 1651-1652 unpoisons killed champions;
     * Firestaff mutates poison HP immediately, so do not enqueue a dead
     * champion's next C75 chain event. */
    if (outPlan->poisonPlan.nextAttack > 0 && !outPlan->championDown) {
        outPlan->schedulePoisonEvent = 1;
        outPlan->incrementPoisonEventCount = 1;
        outPlan->poisonEvent.kind = TIMELINE_EVENT_STATUS_TIMEOUT;
        outPlan->poisonEvent.fireAtTick =
            gameTick + (uint32_t)outPlan->poisonPlan.scheduleDelayTicks;
        outPlan->poisonEvent.mapIndex = poisonEventMapIndex;
        outPlan->poisonEvent.mapX = poisonEventMapX;
        outPlan->poisonEvent.mapY = poisonEventMapY;
        outPlan->poisonEvent.aux0 = LIFECYCLE_STATUS_POISON;
        outPlan->poisonEvent.aux1 = outPlan->poisonPlan.nextAttack;
        outPlan->poisonEvent.aux4 = outPlan->championIndex;
    }
    return 1;
}

int dm1_v1_projectile_champion_poison_event_count_after_pc34(
    const DM1_ProjectileChampionPoisonApplyPlanPc34* applyPlan,
    int currentPoisonEventCount,
    int* outPoisonEventCount) {
    int nextCount;
    if (!outPoisonEventCount) return 0;
    nextCount = currentPoisonEventCount;
    if (nextCount < 0) nextCount = 0;
    if (nextCount > 255) nextCount = 255;
    if (applyPlan && applyPlan->incrementPoisonEventCount && nextCount < 255) {
        nextCount++;
    }
    *outPoisonEventCount = nextCount;

    /* ReDMCSB: CHAMPION.C F0322 lines 1954-1960 queues the next C75 poison
     * event after poison damage. Firestaff stores the active event counter in
     * its lifecycle sidecar; keep the saturation decision with the DM1 poison
     * receipt instead of rebuilding it in M10/M11 callers. */
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

int dm1_v1_explosion_party_champion_apply_pc34(
    const DM1_ExplosionPartyChampionDamagePlanPc34* championPlan,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct ChampionState_Compat* champion,
    DM1_ExplosionPartyChampionApplyPlanPc34* outPlan) {
    int selectedWounds = 0;
    int killed = 0;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->championIndex = -1;
    if (!championPlan || !championPlan->shouldAttemptDamage) {
        outPlan->valid = 1;
        return 1;
    }
    outPlan->championIndex = championPlan->championIndex;
    if (!defender || !rng || !champion) return 0;

    /* ReDMCSB: CHAMPION.C F0324 lines 1991-2022 randomizes party-square
     * explosion attack per champion, then calls F0321 for defense,
     * wound selection, HP mutation, and down state. */
    if (!F0739b_COMBAT_ScaleChampionDamageF0321Rng_Compat(
            championPlan->attackTypeCode,
            championPlan->randomizedAttack,
            championPlan->allowedWounds,
            defender,
            rng,
            &outPlan->scaledAttack,
            NULL)) {
        return 0;
    }
    outPlan->valid = 1;
    if (outPlan->scaledAttack <= 0) {
        return 1;
    }
    if (!F0739c_COMBAT_SelectChampionWoundsF0321Rng_Compat(
            outPlan->scaledAttack,
            championPlan->allowedWounds,
            defender,
            rng,
            &selectedWounds,
            NULL)) {
        return 0;
    }
    outPlan->selectedWounds = selectedWounds;
    outPlan->damage.damageApplied = outPlan->scaledAttack;
    outPlan->damage.woundMaskAdded = selectedWounds;
    if (!F0737_COMBAT_ApplyDamageToChampion_Compat(
            &outPlan->damage, champion, &killed)) {
        return 0;
    }
    outPlan->killed = killed ? 1 : 0;
    return 1;
}

int dm1_v1_explosion_group_apply_pc34(
    const struct CombatAction_Compat* action,
    struct DungeonGroup_Compat* group,
    DM1_ExplosionGroupApplyPlanPc34* outPlan) {
    int creatureIndex = 0;
    int outcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;

    if (!outPlan) return 0;
    memset(outPlan, 0, sizeof(*outPlan));
    outPlan->finalOutcomeCode = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    if (!action || !group) return 0;
    if (action->kind != COMBAT_ACTION_APPLY_DAMAGE_GROUP) return 0;
    outPlan->valid = 1;
    outPlan->damage.damageApplied = action->rawAttackValue;
    if (outPlan->damage.damageApplied <= 0) return 1;

    /* ReDMCSB: PROJEXPL.C F0220 lines 857-866 applies explosion damage
     * to a group square. F0190/F0738 then mutates the live GROUP slot and
     * compacts killed creature entries until the group is dead or no live
     * slots remain. */
    outPlan->handled = 1;
    while (creatureIndex <= (int)group->count && creatureIndex < 4) {
        if (group->health[creatureIndex] == 0) {
            ++creatureIndex;
            continue;
        }
        if (!F0738_COMBAT_ApplyDamageToGroup_Compat(
                &outPlan->damage, group, creatureIndex, &outcome)) {
            return 0;
        }
        outPlan->appliedCount++;
        outPlan->finalOutcomeCode = outcome;
        if (outcome == COMBAT_OUTCOME_KILLED_ALL_CREATURES) break;
        if (outcome == COMBAT_OUTCOME_KILLED_NO_CREATURES) ++creatureIndex;
    }
    outPlan->finalGroupCount = group->count;
    outPlan->finalGroupCells = group->cells;
    for (creatureIndex = 0; creatureIndex < 4; ++creatureIndex) {
        outPlan->finalHealth[creatureIndex] = group->health[creatureIndex];
    }
    return 1;
}
