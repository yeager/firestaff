#ifndef FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H

#include "memory_tick_orchestrator_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int championIndex;
    int actionIndex;
    int championPresent;
    int championDirection;
} DM1_MeleeActionTickInputPc34;

typedef struct {
    int valid;
    unsigned char command;
    unsigned char commandArg1;
    unsigned char commandArg2;
    unsigned char reserved;
    unsigned int reserved2;
    int hasTargetDirection;
    int targetDirection;
} DM1_MeleeActionTickPlanPc34;

typedef struct {
    int damage;
    int combatOutcome;
} DM1_MeleeDamageEmissionInputPc34;

typedef struct {
    int valid;
    int performed;
    int showDamageFeedback;
    int damage;
} DM1_MeleeDamageEmissionPlanPc34;

typedef struct {
    int actionIndex;
    int defaultDisabledTicks;
    int observedAttackDamage;
    int combatOutcome;
    int closedDoorBranchPerformed;
    int closedDoorDisabledTicks;
    int directParryEmptyFront;
} DM1_MeleeRuntimeOutcomeInputPc34;

typedef struct {
    int valid;
    int performed;
    int meleeFailureTail;
    int disabledTicks;
    int showDamageFeedback;
    int damage;
} DM1_MeleeRuntimeOutcomePlanPc34;

typedef struct {
    int creatureType;
    int creatureBaseHealth;
    int activeChampionIndex;
    int activeChampionPresent;
} DM1_MeleeKillNotifyInputPc34;

typedef struct {
    int valid;
    int shouldLogDefeated;
    int shouldAwardKillXp;
    int championIndex;
    int creatureType;
    int xpBonus;
} DM1_MeleeKillNotifyPlanPc34;

typedef struct {
    int championIndex;
    int championPresent;
    int championCurrentHealth;
    int championCell;
    int targetDirection;
    int partyChampionCount;
    int otherChampionPresent[4];
    int otherChampionCurrentHealth[4];
    int otherChampionCell[4];
} DM1_MeleeReachGateInputPc34;

typedef struct {
    int valid;
    int blocked;
    int relativeCell;
    int blockingCell;
    int blockingChampionIndex;
    int damage;
    int combatOutcome;
} DM1_MeleeReachGatePlanPc34;

typedef struct {
    int actionIndex;
    int targetCreatureAttributes;
} DM1_MeleeDisruptMaterialGateInputPc34;

typedef struct {
    int valid;
    int blocked;
    int damage;
    int combatOutcome;
} DM1_MeleeDisruptMaterialGatePlanPc34;

typedef struct {
    int weaponType;
    int weaponClass;
    int weaponStrength;
    int kineticEnergy;
    int weaponAttributes;
    int actionIndex;
    int actionSkillIndex;
} DM1_MeleeWeaponProfileInputPc34;

typedef struct {
    int valid;
    int normalizedActionIndex;
    int hitProbability;
    int damageFactor;
    int hitNonMaterialFlagSet;
    struct WeaponProfile_Compat weaponProfile;
} DM1_MeleeWeaponProfilePlanPc34;

typedef struct {
    int championIndex;
    int actionSkillIndex;
    int damageApplied;
    int creatureProperties;
    int currentStamina;
    int maximumStamina;
    int currentHealth;
    int staminaRandomValue;
} DM1_MeleeF0231SideEffectInputPc34;

typedef struct {
    int valid;
    int shouldAwardXp;
    int skillIndex;
    int experienceGain;
    int staminaRandomModulus;
    int staminaBaseCost;
    int staminaCost;
    int currentStaminaAfter;
    int currentHealthAfter;
    int pendingHealthDamage;
} DM1_MeleeF0231SideEffectPlanPc34;

int dm1_v1_melee_action_tick_plan_f0402_pc34(
    const DM1_MeleeActionTickInputPc34* in,
    DM1_MeleeActionTickPlanPc34* out);
int dm1_v1_melee_damage_emission_plan_f0231_pc34(
    const DM1_MeleeDamageEmissionInputPc34* in,
    DM1_MeleeDamageEmissionPlanPc34* out);
int dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(
    const DM1_MeleeRuntimeOutcomeInputPc34* in,
    DM1_MeleeRuntimeOutcomePlanPc34* out);
int dm1_v1_melee_kill_notify_plan_f0231_pc34(
    const DM1_MeleeKillNotifyInputPc34* in,
    DM1_MeleeKillNotifyPlanPc34* out);
int dm1_v1_melee_reach_gate_plan_f0402_pc34(
    const DM1_MeleeReachGateInputPc34* in,
    DM1_MeleeReachGatePlanPc34* out);
int dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(
    const DM1_MeleeDisruptMaterialGateInputPc34* in,
    DM1_MeleeDisruptMaterialGatePlanPc34* out);
int dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(
    const DM1_MeleeWeaponProfileInputPc34* in,
    DM1_MeleeWeaponProfilePlanPc34* out);
int dm1_v1_melee_side_effect_plan_f0231_pc34(
    const DM1_MeleeF0231SideEffectInputPc34* in,
    DM1_MeleeF0231SideEffectPlanPc34* out);

#ifdef __cplusplus
}
#endif

#endif
