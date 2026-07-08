#ifndef FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MELEE_ACTION_F0402_PC34_COMPAT_H

#include "dm1_v1_creature_ai_behavior_pc34_compat.h"
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

typedef struct {
    int hasWeaponInfo;
    int hasLiveActionIndex;
    int actionHandEmpty;
} DM1_MeleeF0402WeaponAvailabilityInputPc34;

typedef struct {
    int valid;
    int useActionHandWeaponInfo;
    int useEmptyHandWeaponInfo;
    int hasUsableF0231WeaponInfo;
    int weaponClass;
} DM1_MeleeF0402WeaponAvailabilityPlanPc34;

typedef struct {
    unsigned int reserved2;
    int partyDirection;
} DM1_MeleeF0402CommandDecodeInputPc34;

typedef struct {
    int valid;
    int actionIndex;
    int actionSkillIndex;
    int hasLiveActionIndex;
    int hasLegacyMarker;
    int hasTargetDirection;
    int targetDirection;
} DM1_MeleeF0402CommandDecodePlanPc34;

typedef struct {
    int requestedAutoTarget;
    int hasLiveActionIndex;
    int hasLiveGroupTable;
    int targetResolved;
    int reachBlocked;
    int disruptBlocked;
    int candidateInvulnerable;
    int championSnapshotReady;
    int creatureSnapshotReady;
} DM1_MeleeF0402PreflightInputPc34;

typedef struct {
    int valid;
    int shouldReturnHandled;
    int shouldEmitDamageDealt;
    int emitOutcome;
    int canResolveDamage;
    int canUseLegacyMarker;
} DM1_MeleeF0402PreflightPlanPc34;

typedef struct {
    int championStrength;
    int currentStamina;
    int maximumStamina;
    int maximumLoad;
    int random16;
    int objectWeight;
    int hasActionHandWeapon;
    int weaponStrength;
    int weaponSkillBonus;
    int actionHandWounded;
} DM1_MeleeF0312StrengthInputPc34;

typedef struct {
    int valid;
    int oneSixteenthMaximumLoad;
    int loadThreshold;
    int strengthBeforeStamina;
    int strengthAfterStamina;
    int strengthAfterWound;
    int strengthActionHand;
} DM1_MeleeF0312StrengthPlanPc34;

typedef struct {
    int championIndex;
    int championPresent;
    int currentHealth;
    int dexterity;
    int strengthActionHand;
    int actionSkillIndex;
    int weaponClass;
    int skillLevelParry;
    int skillLevelAction;
    int statisticVitality;
    int statisticAntifire;
    int statisticAntimagic;
    int statisticWisdom;
    int statisticLuck;
    int statisticLuckMax;
    int statisticLuckMin;
    int actionHandIcon;
    int wounds;
    int isResting;
    int partyShieldDefense;
} DM1_MeleeF0231ChampionSnapshotInputPc34;

typedef struct {
    int valid;
    int normalizedActionSkillIndex;
    struct CombatantChampionSnapshot_Compat snapshot;
} DM1_MeleeF0231ChampionSnapshotPlanPc34;

typedef struct {
    int groupIndex;
    int groupCount;
    int groupCreatureType;
    int creatureIndex;
    int creatureHealth;
    int profileAttack;
    int profileDefense;
    int profileDexterity;
    int profileBaseHealth;
    int profilePoisonAttack;
    int profileAttackType;
    int profileAttributes;
    int profileWoundProbabilities;
    int profileProperties;
    int doubledMapDifficulty;
    int candidateInvulnerableEnabled;
    int candidateInvulnerableGroupIndex;
    int candidateInvulnerableCreatureIndex;
} DM1_MeleeF0231CreatureSnapshotInputPc34;

typedef struct {
    int valid;
    struct CombatantCreatureSnapshot_Compat snapshot;
} DM1_MeleeF0231CreatureSnapshotPlanPc34;

typedef struct {
    int groupIndex;
    int creatureType;
    int creatureAttributes;
    int killedCell;
    int damageOutcome;
    int fallbackCombatOutcome;
    int fearTriggered;
} DM1_MeleeF0231AftermathInputPc34;

typedef struct {
    int valid;
    int outcome;
    int shouldDropPossessions;
    int shouldCreateDeathSmoke;
    int smokeAttack;
    int smokeCell;
    int shouldApplyKilledSomeState;
    int shouldApplyKilledAllSideEffects;
    int shouldWriteRawGroup;
    int shouldEmitKillNotify;
    int shouldScheduleReaction;
    int reactionEventKind;
} DM1_MeleeF0231AftermathPlanPc34;

typedef struct {
    int groupIndex;
    int creatureType;
    int mapIndex;
    int mapX;
    int mapY;
    unsigned int currentTick;
    int outcome;
} DM1_MeleeF0231ReactionInputPc34;

typedef struct {
    int valid;
    int shouldSchedule;
    unsigned int fireAtTick;
    int mapIndex;
    int mapX;
    int mapY;
    int groupIndex;
    int creatureType;
    int eventKind;
} DM1_MeleeF0231ReactionPlanPc34;

typedef struct {
    int shouldCreate;
    int smokeAttack;
    int smokeCell;
    int mapIndex;
    int mapX;
    int mapY;
    int currentTick;
} DM1_MeleeF0190DeathSmokeInputPc34;

typedef struct {
    int valid;
    int shouldCreate;
    struct ExplosionCreateInput_Compat createInput;
} DM1_MeleeF0190DeathSmokePlanPc34;

typedef struct {
    int outcome;
    int creatureType;
    int creatureAttributes;
    int killedCell;
    int mapIndex;
    int mapX;
    int mapY;
} DM1_MeleeF0190PossessionDropInputPc34;

typedef struct {
    int valid;
    int shouldDropGroupFixedPossessions;
    int shouldDropGroupSlotPossessions;
    int shouldDropCreatureFixedPossessions;
    int creatureType;
    int creatureCell;
    int mapIndex;
    int mapX;
    int mapY;
} DM1_MeleeF0190PossessionDropPlanPc34;

typedef struct {
    int outcome;
    int groupBehavior;
    int groupIndex;
    int killedCreatureIndex;
    int originalGroupCount;
    int mapIndex;
    int mapX;
    int mapY;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int creatureType;
    int creatureProperties;
} DM1_MeleeF0190KilledSomeStateInputPc34;

typedef struct {
    int valid;
    int shouldCleanupCreatureEvents;
    int shouldEvaluateFear;
    int shouldApplyFear;
    int newGroupBehavior;
    int newAiStateKind;
    int fearCounter;
    int groupIndex;
    int killedCreatureIndex;
    int originalGroupCount;
    int mapIndex;
    int mapX;
    int mapY;
    struct DM1GroupBehaviorContext_Compat fearContext;
} DM1_MeleeF0190KilledSomeStatePlanPc34;

typedef struct {
    int outcome;
    int groupIndex;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
} DM1_MeleeF0190KilledAllStateInputPc34;

typedef struct {
    int valid;
    int shouldUnlinkGroupFromSquare;
    int shouldClearGroupNext;
    int shouldRemoveActiveGroupState;
    int shouldWriteRawGroup;
    int groupIndex;
    int mapIndex;
    int mapX;
    int mapY;
} DM1_MeleeF0190KilledAllStatePlanPc34;

typedef struct {
    int eventKind;
    int eventMapIndex;
    int eventMapX;
    int eventMapY;
    int eventType;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
    int killedCreatureIndex;
} DM1_MeleeF0190TimelineCleanupInputPc34;

typedef struct {
    int valid;
    int shouldKeepEvent;
    int newEventType;
    int eventCreatureIndex;
} DM1_MeleeF0190TimelineCleanupPlanPc34;

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
int dm1_v1_melee_weapon_availability_plan_f0402_pc34(
    const DM1_MeleeF0402WeaponAvailabilityInputPc34* in,
    DM1_MeleeF0402WeaponAvailabilityPlanPc34* out);
int dm1_v1_melee_command_decode_plan_f0402_pc34(
    const DM1_MeleeF0402CommandDecodeInputPc34* in,
    DM1_MeleeF0402CommandDecodePlanPc34* out);
int dm1_v1_melee_preflight_plan_f0402_pc34(
    const DM1_MeleeF0402PreflightInputPc34* in,
    DM1_MeleeF0402PreflightPlanPc34* out);
int dm1_v1_melee_strength_plan_f0312_pc34(
    const DM1_MeleeF0312StrengthInputPc34* in,
    DM1_MeleeF0312StrengthPlanPc34* out);
int dm1_v1_melee_champion_snapshot_plan_f0231_pc34(
    const DM1_MeleeF0231ChampionSnapshotInputPc34* in,
    DM1_MeleeF0231ChampionSnapshotPlanPc34* out);
int dm1_v1_melee_creature_snapshot_plan_f0231_pc34(
    const DM1_MeleeF0231CreatureSnapshotInputPc34* in,
    DM1_MeleeF0231CreatureSnapshotPlanPc34* out);
int dm1_v1_melee_aftermath_plan_f0231_pc34(
    const DM1_MeleeF0231AftermathInputPc34* in,
    DM1_MeleeF0231AftermathPlanPc34* out);
int dm1_v1_melee_reaction_plan_f0231_pc34(
    const DM1_MeleeF0231ReactionInputPc34* in,
    DM1_MeleeF0231ReactionPlanPc34* out);
int dm1_v1_melee_death_smoke_plan_f0190_pc34(
    const DM1_MeleeF0190DeathSmokeInputPc34* in,
    DM1_MeleeF0190DeathSmokePlanPc34* out);
int dm1_v1_melee_possession_drop_plan_f0190_pc34(
    const DM1_MeleeF0190PossessionDropInputPc34* in,
    DM1_MeleeF0190PossessionDropPlanPc34* out);
int dm1_v1_melee_killed_some_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_killed_some_fear_apply_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    int shouldFlee,
    int fleeDelay,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_killed_all_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledAllStateInputPc34* in,
    DM1_MeleeF0190KilledAllStatePlanPc34* out);
int dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
    const DM1_MeleeF0190TimelineCleanupInputPc34* in,
    DM1_MeleeF0190TimelineCleanupPlanPc34* out);
int dm1_v1_melee_resolve_damage_f0231_pc34(
    struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out);
int dm1_v1_melee_apply_group_damage_f0190_pc34(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    int* outOutcome);

#ifdef __cplusplus
}
#endif

#endif
