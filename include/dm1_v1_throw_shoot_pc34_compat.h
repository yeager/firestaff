#ifndef FIRESTAFF_DM1_V1_THROW_SHOOT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_THROW_SHOOT_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_THROW_WEAPON_CLASS_MAX_KINETIC_XP_PC34 = 12,
    DM1_THROW_MIN_ATTACK_PC34 = 40,
    DM1_THROW_MAX_ATTACK_PC34 = 200,
    DM1_THROW_MIN_STEP_ENERGY_PC34 = 5,
    DM1_THROW_DEFAULT_WEAPON_KINETIC_PC34 = 1,
    DM1_THROW_BASE_XP_PC34 = 8,
    DM1_THROW_WEAPON_XP_PC34 = 4,
    DM1_JUNK_WATERSKIN_PC34 = 1,
    DM1_POTION_VEN_PC34 = 3,
    DM1_POTION_FUL_BOMB_PC34 = 19,
    DM1_PROJECTILE_BLACK_FLAME_CREATURE_PC34 = 11,
    DM1_PROJECTILE_SINGLE_CENTERED_CREATURE_CELL_PC34 = 0xFF,
    DM1_PROJECTILE_BLACK_FLAME_MAX_HEALTH_PC34 = 1000,
    DM1_PROJECTILE_ATTR_NON_MATERIAL_PC34 = 0x0040,
    DM1_PROJECTILE_ATTR_DROP_FIXED_POSSESSION_PC34 = 0x0200,
    DM1_PROJECTILE_ATTR_KEEP_THROWN_SHARP_WEAPONS_PC34 = 0x0400
};

typedef struct {
    int championIndex;
    int championCell;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int partyDirection;
    int gameTick;
    int subtype;
    int category;
    int kineticEnergy;
    int impactAttack;
    int attackTypeCode;
    int launchCell;
    int launchDirection;
    int stepEnergy;
    int launcherStrength;
    unsigned short carriedThing;
    int potionPower;
} DM1_ProjectileCreateRequestPc34;

typedef struct {
    int creatureGroupIndex;
    int partyMapIndex;
    int groupMapX;
    int groupMapY;
    int projectileThing;
    int targetCell;
    int direction;
    int kineticEnergy;
    int attack;
    int stepEnergy;
    int gameTick;
} DM1_CreatureProjectileCreateRequestPc34;

typedef struct {
    int shouldConsumePotion;
    int shouldMaterialize;
    unsigned short associatedThing;
    unsigned short droppedThing;
} DM1_ProjectileAssociatedThingDispositionPc34;

typedef struct {
    int handled;
    int shouldConsumePotion;
    int shouldMaterialize;
    unsigned short associatedThing;
    unsigned short droppedThing;
    int mapIndex;
    int mapX;
    int mapY;
    int cell;
} DM1_ProjectileMaterializationPlanPc34;

typedef struct {
    int handled;
    int shouldApplyDamage;
    int slotIndex;
    int damageApplied;
    int originalCreatureType;
    int originalCells;
    int originalGroupCount;
    int killedCell;
    int blockedByNonMaterial;
} DM1_ProjectileCreatureImpactPlanPc34;

typedef struct {
    int handled;
    int shouldApplyDamage;
    int blockedByNonMaterial;
    int healsBlackFlame;
    int slotIndex;
    int newHealth;
    int damageApplied;
    int originalCreatureType;
    int originalCells;
    int originalGroupCount;
    int killedCell;
} DM1_ProjectileCreatureActionPlanPc34;

typedef struct {
    int valid;
    int handled;
    int outcomeCode;
    int damageApplied;
    int creatureIndex;
    struct CombatResult_Compat damage;
} DM1_ProjectileCreatureActionApplyPlanPc34;

typedef struct {
    int scheduleReaction;
    int cleanupEventsAndFear;
    int dropFixedPossessions;
    int spawnDeathSmoke;
    int keepSharpWeaponInGroup;
} DM1_ProjectileCreatureImpactAftermathPc34;

typedef struct {
    int valid;
    int handled;
    int shouldWriteGroup;
    int outcomeCode;
    int damageApplied;
    int killedCell;
    unsigned short newHealth[4];
    unsigned char newCount;
    unsigned char newCells;
} DM1_ProjectileCreaturePrecheckDamagePlanPc34;

typedef struct {
    int valid;
    int shouldAttachToGroupSlot;
    unsigned short associatedThing;
    int weaponType;
} DM1_ProjectileGroupSlotMaterializationPlanPc34;

typedef struct {
    int valid;
    int shouldSetAssociatedNextEnd;
    int shouldSetGroupSlotHead;
    int shouldAppendAfterTail;
    unsigned short associatedThing;
    unsigned short groupSlotHead;
    unsigned short tailThing;
} DM1_ProjectileGroupSlotAttachPlanPc34;

typedef struct {
    int handled;
    int championPresent;
    int championIndex;
    int impactMapIndex;
    int impactMapX;
    int impactMapY;
    int impactCell;
    int attackTypeCode;
    int rawAttackValue;
    int allowedWounds;
} DM1_ProjectileChampionImpactPlanPc34;

typedef struct {
    int shouldApply;
    int championIndex;
    int poisonDamage;
    int newPoisonDose;
    int nextAttack;
    int scheduleDelayTicks;
} DM1_ProjectileChampionPoisonPlanPc34;

typedef struct {
    int valid;
    int championIndex;
    int scaledAttack;
    int selectedWounds;
    int killed;
    struct CombatResult_Compat damage;
} DM1_ProjectileChampionDamageApplyPlanPc34;

typedef struct {
    int valid;
    int shouldApply;
    int championIndex;
    int championDown;
    int incrementPoisonEventCount;
    int schedulePoisonEvent;
    struct TimelineEvent_Compat poisonEvent;
    DM1_ProjectileChampionPoisonPlanPc34 poisonPlan;
} DM1_ProjectileChampionPoisonApplyPlanPc34;

typedef struct {
    int handled;
    int baseAttack;
    int rngModulus;
    int attackTypeCode;
    int allowedWounds;
} DM1_ExplosionPartyDamageFanoutPlanPc34;

typedef struct {
    int shouldAttemptDamage;
    int championIndex;
    int randomizedAttack;
    int attackTypeCode;
    int allowedWounds;
} DM1_ExplosionPartyChampionDamagePlanPc34;

typedef struct {
    int valid;
    int championIndex;
    int scaledAttack;
    int selectedWounds;
    int killed;
    struct CombatResult_Compat damage;
} DM1_ExplosionPartyChampionApplyPlanPc34;

typedef struct {
    int valid;
    int handled;
    int appliedCount;
    int finalOutcomeCode;
    struct CombatResult_Compat damage;
} DM1_ExplosionGroupApplyPlanPc34;

enum {
    DM1_PROJECTILE_IMPACT_LOG_NONE_PC34 = 0,
    DM1_PROJECTILE_IMPACT_LOG_HIT_WALL_PC34,
    DM1_PROJECTILE_IMPACT_LOG_HIT_DOOR_PC34,
    DM1_PROJECTILE_IMPACT_LOG_HIT_FLUXCAGE_PC34,
    DM1_PROJECTILE_IMPACT_LOG_HIT_OTHER_PROJECTILE_PC34,
    DM1_PROJECTILE_IMPACT_LOG_DESPAWN_ENERGY_PC34,
    DM1_PROJECTILE_IMPACT_LOG_DESPAWN_BOUNDS_PC34
};

typedef struct {
    int handled;
    int logKind;
} DM1_ProjectileImpactLogPlanPc34;

typedef struct {
    int objectWeight;
    int championStrength;
    int championMaxLoad;
    int championCurrentStamina;
    int championMaximumStamina;
    int actionHandWounded;
    int isWeapon;
    int hasWeaponInfo;
    int weaponClass;
    int weaponStrength;
    int weaponKineticEnergy;
    int f0312SkillBonus;
    int throwSkillLevel;
    int rngStrength16;
    int rngKinetic16;
    int rngAttack32;
    unsigned short thrownThing;
    int thingType;
    int potionType;
    int potionPower;
    int partyDirection;
    int throwSide;
} DM1_ThrowF0328ProjectileInputPc34;

typedef struct {
    int valid;
    int staminaCost;
    int throwExperience;
    int throwStrength;
    int kineticEnergy;
    int attack;
    int stepEnergy;
    int actionDisableTicks;
    int combatSoundIndex;
    int projectileSubtype;
    int projectilePotionPower;
    int launchDirection;
    int projectileDisabledMovementTicks;
    int lastProjectileDisabledMovementDirection;
} DM1_ThrowF0328ProjectilePlanPc34;

int dm1_v1_throwing_stamina_cost_from_weight_pc34(int objectWeight);
int dm1_v1_throw_armour_weight_f0140_pc34(int armourType);
int dm1_v1_throw_junk_base_weight_f0140_pc34(int junkType);
int dm1_v1_throw_junk_weight_f0140_pc34(int junkType, int chargeCount);
int dm1_v1_throw_xp_for_object_pc34(int isWeapon,
                                    int hasWeaponInfo,
                                    int weaponClass,
                                    int weaponKineticEnergy);
int dm1_v1_throw_side_pc34(int championCell, int partyDirection);
int dm1_v1_throw_kinetic_energy_pc34(int baseStrength,
                                     int throwSkillLevel,
                                     int hasWeaponInfo,
                                     int weaponClass,
                                     int weaponKineticEnergy,
                                     int rng16);
int dm1_v1_throw_attack_pc34(int throwSkillLevel, int rng32);
int dm1_v1_throw_step_energy_pc34(int throwSkillLevel);
int dm1_v1_throw_projectile_plan_f0328_pc34(
    const DM1_ThrowF0328ProjectileInputPc34* in,
    DM1_ThrowF0328ProjectilePlanPc34* out);
int dm1_v1_thrown_potion_projectile_subtype_pc34(int potionType,
                                                 int* outSubtype);
int dm1_v1_shoot_step_energy_pc34(int actionClass, int* outStepEnergy);
int dm1_v1_shoot_ammunition_matches_pc34(int actionWeaponClass,
                                         int readyWeaponClass);
int dm1_v1_projectile_launch_cell_pc34(int championCell, int direction);
int dm1_v1_shoot_attack_pc34(int weaponShootAttack, int shootSkillLevel);
int dm1_v1_legacy_throw_attack_probe_pc34(int baseAttack, int throwSkillLevel);
int dm1_v1_build_projectile_create_input_pc34(
    const DM1_ProjectileCreateRequestPc34* req,
    struct ProjectileCreateInput_Compat* outInput);
int dm1_v1_projectile_subtype_from_thing_pc34(int projectileThing,
                                              int* outSubtype);
int dm1_v1_projectile_attack_type_for_subtype_pc34(int subtype);
int dm1_v1_build_creature_projectile_create_input_pc34(
    const DM1_CreatureProjectileCreateRequestPc34* req,
    struct ProjectileCreateInput_Compat* outInput);
const char* dm1_v1_projectile_subtype_name_pc34(int subtype);
int dm1_v1_projectile_impact_source_sound_index_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result);
int dm1_v1_projectile_explosion_create_input_pc34(
    const struct ProjectileTickResult_Compat* result,
    int currentTick,
    struct ExplosionCreateInput_Compat* outInput);
int dm1_v1_projectile_impact_log_plan_pc34(
    const struct ProjectileTickResult_Compat* result,
    DM1_ProjectileImpactLogPlanPc34* outPlan);
int dm1_v1_thrown_sharp_weapon_type_kept_by_creature_pc34(int weaponType);
int dm1_v1_projectile_group_slot_materialization_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    int damageOutcome,
    int creatureAttributes,
    int associatedWeaponType,
    DM1_ProjectileGroupSlotMaterializationPlanPc34* outPlan);
int dm1_v1_projectile_group_slot_attach_plan_f0215_pc34(
    unsigned short associatedThing,
    unsigned short groupSlotHead,
    unsigned short tailThing,
    DM1_ProjectileGroupSlotAttachPlanPc34* outPlan);
int dm1_v1_projectile_associated_thing_disposition_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int associatedThingMovedToGroup,
    int potionCount,
    DM1_ProjectileAssociatedThingDispositionPc34* outDisposition);
int dm1_v1_projectile_materialization_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int associatedThingMovedToGroup,
    int potionCount,
    DM1_ProjectileMaterializationPlanPc34* outPlan);
int dm1_v1_group_creature_index_for_cell_pc34(
    const struct DungeonGroup_Compat* group,
    int targetCell);
int dm1_v1_black_flame_fireball_heal_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    const struct DungeonGroup_Compat* group,
    int* outSlotIndex,
    int* outNewHealth);
int dm1_v1_projectile_creature_impact_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    const struct DungeonGroup_Compat* group,
    int creatureAttributes,
    DM1_ProjectileCreatureImpactPlanPc34* outPlan);
int dm1_v1_projectile_creature_action_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct CombatAction_Compat* action,
    const struct DungeonGroup_Compat* group,
    int creatureAttributes,
    DM1_ProjectileCreatureActionPlanPc34* outPlan);
int dm1_v1_projectile_creature_action_apply_pc34(
    const DM1_ProjectileCreatureActionPlanPc34* actionPlan,
    struct DungeonGroup_Compat* group,
    DM1_ProjectileCreatureActionApplyPlanPc34* outPlan);
int dm1_v1_projectile_creature_action_aftermath_pc34(
    const DM1_ProjectileCreatureActionPlanPc34* actionPlan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int groupBehaviorAfterDamage,
    int damageOutcome,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath);
int dm1_v1_projectile_creature_impact_aftermath_pc34(
    const DM1_ProjectileCreatureImpactPlanPc34* plan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int groupBehaviorAfterDamage,
    int damageOutcome,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath);
int dm1_v1_projectile_creature_precheck_damage_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct DungeonGroup_Compat* group,
    int creatureIndex,
    int creatureDefense,
    int creatureAttributes,
    DM1_ProjectileCreaturePrecheckDamagePlanPc34* outPlan);
int dm1_v1_projectile_creature_precheck_aftermath_pc34(
    const DM1_ProjectileCreaturePrecheckDamagePlanPc34* precheckPlan,
    const struct ProjectileInstance_Compat* projectile,
    int creatureAttributes,
    int associatedWeaponType,
    DM1_ProjectileCreatureImpactAftermathPc34* outAftermath);
int dm1_v1_projectile_champion_action_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct CombatAction_Compat* action,
    int championPresent,
    DM1_ProjectileChampionImpactPlanPc34* outPlan);
int dm1_v1_projectile_champion_impact_plan_pc34(
    const struct ProjectileInstance_Compat* projectile,
    const struct ProjectileTickResult_Compat* result,
    int championPresent,
    DM1_ProjectileChampionImpactPlanPc34* outPlan);
int dm1_v1_projectile_champion_poison_plan_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct ProjectileInstance_Compat* projectile,
    int appliedDamage,
    int championCurrentHealth,
    int championPoisonDose,
    int rng2,
    DM1_ProjectileChampionPoisonPlanPc34* outPlan);
int dm1_v1_projectile_champion_damage_apply_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct ChampionState_Compat* champion,
    DM1_ProjectileChampionDamageApplyPlanPc34* outPlan);
int dm1_v1_projectile_champion_poison_apply_pc34(
    const DM1_ProjectileChampionImpactPlanPc34* impactPlan,
    const struct ProjectileInstance_Compat* projectile,
    int appliedDamage,
    int rng2,
    uint32_t gameTick,
    int partyMapIndex,
    int partyMapX,
    int partyMapY,
    struct ChampionState_Compat* champion,
    DM1_ProjectileChampionPoisonApplyPlanPc34* outPlan);
int dm1_v1_projectile_champion_party_death_check_pc34(
    int combatKilledFlag,
    int championCurrentHealth);
int dm1_v1_explosion_party_damage_fanout_plan_pc34(
    int rawAttackValue,
    int attackTypeCode,
    int allowedWounds,
    DM1_ExplosionPartyDamageFanoutPlanPc34* outPlan);
int dm1_v1_explosion_party_champion_damage_plan_pc34(
    const DM1_ExplosionPartyDamageFanoutPlanPc34* fanoutPlan,
    int championIndex,
    int championPresent,
    int championCurrentHealth,
    int rngWindowRoll,
    DM1_ExplosionPartyChampionDamagePlanPc34* outPlan);
int dm1_v1_explosion_party_champion_apply_pc34(
    const DM1_ExplosionPartyChampionDamagePlanPc34* championPlan,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct ChampionState_Compat* champion,
    DM1_ExplosionPartyChampionApplyPlanPc34* outPlan);
int dm1_v1_explosion_group_apply_pc34(
    const struct CombatAction_Compat* action,
    struct DungeonGroup_Compat* group,
    DM1_ExplosionGroupApplyPlanPc34* outPlan);

#ifdef __cplusplus
}
#endif

#endif
