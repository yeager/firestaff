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
    int mapDifficulty;
    unsigned int currentTick;
    unsigned int lastCreatureAttackTime;
    int currentStamina;
    int maximumStamina;
    int currentHealth;
    int staminaRandomValue;
} DM1_MeleeF0231SideEffectInputPc34;

typedef struct {
    int valid;
    int shouldWriteChampionState;
    int championIndex;
    int shouldAwardXp;
    int xpChampionIndex;
    int skillIndex;
    int experienceGain;
    int xpMapDifficulty;
    unsigned int xpCurrentTick;
    unsigned int xpLastCreatureAttackTime;
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
    unsigned char commandArg2;
    unsigned char reserved;
    unsigned int reserved2;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
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
    int requestedAutoTarget;
    int requestedAutoCreature;
    int directGroupIndex;
    int directCreatureIndex;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
} DM1_MeleeF0402CommandDecodePlanPc34;

typedef struct {
    int groupCount;
    int groupCells;
    int groupDirection;
    int creatureSize;
    int creatureHealth[4];
    int championCell;
    int targetDirection;
} DM1_MeleeF0177TargetCreatureInputPc34;

typedef struct {
    int valid;
    int firstLivingCreatureIndex;
    int selectedCreatureIndex;
    int singleCenteredGroup;
} DM1_MeleeF0177TargetCreaturePlanPc34;

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
    int championIndex;
    int championCurrentHealth;
    int creatureType;
    int creatureDexterity;
    int creatureAttributes;
    int isCandidateInvulnerable;
    int actionHitProbability;
} DM1_MeleeF0231DamageGateInputPc34;

typedef struct {
    int valid;
    int shouldReturnResolved;
    int resolvedOutcome;
    int canEnterDamageBlock;
    int normalizedHitProbability;
    int creatureIsNonMaterial;
    int actionHitsNonMaterial;
} DM1_MeleeF0231DamageGatePlanPc34;

typedef struct {
    int championIndex;
    int combatOutcome;
    int damageApplied;
    int groupIndex;
    int creatureIndex;
    int groupCount;
} DM1_MeleeF0231RuntimeResultInputPc34;

typedef struct {
    int valid;
    int shouldReturnHandledNoAction;
    int shouldWriteBackLuck;
    int shouldApplySideEffects;
    int shouldApplyGroupDamage;
    int shouldEmitDamageDealt;
    int groupDamageGroupIndex;
    int groupDamageCreatureIndex;
    int groupDamageApplied;
    int groupDamageFallbackOutcome;
    int emitChampionIndex;
    int emitGroupIndex;
    int emitDamageApplied;
} DM1_MeleeF0231RuntimeResultPlanPc34;

typedef struct {
    int championIndex;
    int snapshotLuck;
    int championCount;
} DM1_MeleeF0231LuckWritebackInputPc34;

typedef struct {
    int valid;
    int shouldWriteBack;
    int championIndex;
    int clampedLuck;
} DM1_MeleeF0231LuckWritebackPlanPc34;

typedef struct {
    int championIndex;
    int groupIndex;
    int creatureIndex;
    int groupCount;
} DM1_MeleeF0231ResolveRuntimeInputPc34;

typedef struct {
    int valid;
    struct CombatResult_Compat combatResult;
    DM1_MeleeF0231RuntimeResultPlanPc34 runtimeResultPlan;
} DM1_MeleeF0231ResolveRuntimePlanPc34;

typedef struct {
    int groupIndex;
    int creatureIndex;
    int creatureType;
    int creatureAttributes;
    int creatureProperties;
    int groupBehavior;
    int originalGroupCount;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
    unsigned int currentTick;
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
    int killNotifyGroupIndex;
    int killNotifyCreatureIndex;
    int killNotifyOutcome;
    int killNotifyCreatureType;
    unsigned int reactionFireAtTick;
    int reactionMapIndex;
    int reactionMapX;
    int reactionMapY;
    int reactionGroupIndex;
    int reactionCreatureType;
    struct ExplosionCreateInput_Compat smokeCreateInput;
    int mutationOutcome;
    int mutationGroupIndex;
    int mutationGroupBehavior;
    int mutationKilledCreatureIndex;
    int mutationOriginalGroupCount;
    int mutationCreatureType;
    int mutationCreatureAttributes;
    int mutationCreatureProperties;
    int mutationKilledCell;
    int mutationMapIndex;
    int mutationMapX;
    int mutationMapY;
    int mutationPartyMapIndex;
    int mutationPartyMapX;
    int mutationPartyMapY;
} DM1_MeleeF0231AftermathPlanPc34;

typedef struct {
    int valid;
    int shouldWriteRawGroup;
    int groupIndex;
} DM1_MeleeF0231RawGroupWritebackPlanPc34;

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
    int valid;
    int shouldApplyDamage;
    int originalGroupCount;
    int killedCell;
    int outcome;
    int damageApplied;
} DM1_MeleeF0190GroupDamageApplyPlanPc34;

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
    int valid;
    int shouldDropGroupFixedPossessions;
    int shouldDropGroupSlotPossessions;
    int shouldDropCreatureFixedPossessions;
    int groupFixedCellCount;
    int groupFixedCells[4];
    int creatureType;
    int creatureCell;
    int mapIndex;
    int mapX;
    int mapY;
} DM1_MeleeF0190PossessionDropApplyPlanPc34;

typedef struct {
    int valid;
    int dropCellCount;
    int dropCells[4];
} DM1_MeleeF0190FixedDropCellsPlanPc34;

#define DM1_MELEE_F0188_GROUP_SLOT_DROP_MAX_PC34 64

typedef struct {
    unsigned short thing;
    unsigned short nextThing;
} DM1_MeleeF0188GroupSlotDropChainEntryPc34;

typedef struct {
    unsigned short slotHead;
    int chainEntryCount;
    DM1_MeleeF0188GroupSlotDropChainEntryPc34
        chain[DM1_MELEE_F0188_GROUP_SLOT_DROP_MAX_PC34];
    int randomCellCount;
    unsigned char randomCells[DM1_MELEE_F0188_GROUP_SLOT_DROP_MAX_PC34];
} DM1_MeleeF0188GroupSlotDropInputPc34;

typedef struct {
    unsigned short sourceThing;
    unsigned short nextThing;
    unsigned short droppedThing;
    int dropCell;
    int thingType;
} DM1_MeleeF0188GroupSlotDropStepPc34;

typedef struct {
    int valid;
    int shouldDrop;
    int shouldClearGroupSlot;
    int stepCount;
    int truncated;
    int weaponDropped;
    int soundId;
    DM1_MeleeF0188GroupSlotDropStepPc34
        steps[DM1_MELEE_F0188_GROUP_SLOT_DROP_MAX_PC34];
} DM1_MeleeF0188GroupSlotDropPlanPc34;

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
    int valid;
    int shouldCleanupCreatureEvents;
    int shouldApplyFear;
    int groupIndex;
    int killedCreatureIndex;
    int mapIndex;
    int mapX;
    int mapY;
    int newGroupBehavior;
    int newAiStateKind;
    int fearCounter;
} DM1_MeleeF0190KilledSomeStateApplyPlanPc34;

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
    int valid;
    int shouldUnlinkGroupFromSquare;
    int shouldClearGroupNext;
    int shouldRemoveActiveGroupState;
    int groupIndex;
    int mapIndex;
    int mapX;
    int mapY;
    unsigned short groupThing;
    unsigned short clearedNextThing;
} DM1_MeleeF0190KilledAllStateApplyPlanPc34;

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

typedef struct {
    int eventCount;
    struct TimelineEvent_Compat events[TIMELINE_QUEUE_CAPACITY];
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
    int killedCreatureIndex;
} DM1_MeleeF0190TimelineCleanupBatchInputPc34;

typedef struct {
    int valid;
    int oldEventCount;
    int newEventCount;
    int deletedEventCount;
    struct TimelineEvent_Compat events[TIMELINE_QUEUE_CAPACITY];
} DM1_MeleeF0190TimelineCleanupBatchPlanPc34;

typedef struct {
    int outcome;
    int groupIndex;
    int groupBehavior;
    int killedCreatureIndex;
    int originalGroupCount;
    int creatureType;
    int creatureAttributes;
    int creatureProperties;
    int killedCell;
    int mapIndex;
    int mapX;
    int mapY;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
} DM1_MeleeF0190MutationDispatchInputPc34;

typedef struct {
    int valid;
    int shouldDropPossessions;
    int shouldApplyKilledSomeState;
    int shouldApplyKilledAllSideEffects;
    DM1_MeleeF0190PossessionDropPlanPc34 possessionDropPlan;
    DM1_MeleeF0190KilledSomeStatePlanPc34 killedSomeStatePlan;
    DM1_MeleeF0190KilledAllStatePlanPc34 killedAllStatePlan;
} DM1_MeleeF0190MutationDispatchPlanPc34;

typedef struct {
    int valid;
    int shouldDropPossessions;
    int shouldApplyKilledSomeState;
    int shouldEvaluateFear;
    int shouldApplyKilledAllSideEffects;
    DM1_MeleeF0190PossessionDropPlanPc34 possessionDropPlan;
    DM1_MeleeF0190KilledSomeStatePlanPc34 killedSomeStatePlan;
    DM1_MeleeF0190KilledAllStatePlanPc34 killedAllStatePlan;
} DM1_MeleeF0190MutationDispatchApplyPlanPc34;

typedef struct {
    int valid;
    int shouldEvaluateFear;
    int originalGroupCount;
    struct DM1GroupBehaviorContext_Compat fearContext;
} DM1_MeleeF0190FearRollPlanPc34;

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
int dm1_v1_melee_target_creature_plan_f0177_pc34(
    const DM1_MeleeF0177TargetCreatureInputPc34* in,
    DM1_MeleeF0177TargetCreaturePlanPc34* out);
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
int dm1_v1_melee_damage_gate_plan_f0231_pc34(
    const DM1_MeleeF0231DamageGateInputPc34* in,
    DM1_MeleeF0231DamageGatePlanPc34* out);
int dm1_v1_melee_runtime_result_plan_f0231_pc34(
    const DM1_MeleeF0231RuntimeResultInputPc34* in,
    DM1_MeleeF0231RuntimeResultPlanPc34* out);
int dm1_v1_melee_luck_writeback_plan_f0231_pc34(
    const DM1_MeleeF0231LuckWritebackInputPc34* in,
    DM1_MeleeF0231LuckWritebackPlanPc34* out);
int dm1_v1_melee_resolve_runtime_f0231_pc34(
    struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    const DM1_MeleeF0231ResolveRuntimeInputPc34* in,
    DM1_MeleeF0231ResolveRuntimePlanPc34* out);
int dm1_v1_melee_aftermath_plan_f0231_pc34(
    const DM1_MeleeF0231AftermathInputPc34* in,
    DM1_MeleeF0231AftermathPlanPc34* out);
int dm1_v1_melee_aftermath_raw_group_writeback_plan_f0231_pc34(
    const DM1_MeleeF0231AftermathPlanPc34* aftermathPlan,
    DM1_MeleeF0231RawGroupWritebackPlanPc34* out);
int dm1_v1_melee_reaction_plan_f0231_pc34(
    const DM1_MeleeF0231ReactionInputPc34* in,
    DM1_MeleeF0231ReactionPlanPc34* out);
int dm1_v1_melee_death_smoke_plan_f0190_pc34(
    const DM1_MeleeF0190DeathSmokeInputPc34* in,
    DM1_MeleeF0190DeathSmokePlanPc34* out);
int dm1_v1_melee_death_smoke_attack_f0190_pc34(int creatureAttributes);
int dm1_v1_melee_possession_drop_plan_f0190_pc34(
    const DM1_MeleeF0190PossessionDropInputPc34* in,
    DM1_MeleeF0190PossessionDropPlanPc34* out);
int dm1_v1_melee_possession_drop_apply_plan_f0190_pc34(
    const DM1_MeleeF0190PossessionDropPlanPc34* dropPlan,
    const struct DungeonGroup_Compat* group,
    DM1_MeleeF0190PossessionDropApplyPlanPc34* out);
int dm1_v1_melee_group_fixed_drop_cells_plan_f0190_pc34(
    const struct DungeonGroup_Compat* group,
    DM1_MeleeF0190FixedDropCellsPlanPc34* out);
int dm1_v1_melee_moving_fixed_drop_cells_plan_f0187_pc34(
    const unsigned char* movingFixedDropCells,
    int movingFixedDropCellCount,
    DM1_MeleeF0190FixedDropCellsPlanPc34* out);
int dm1_v1_melee_group_slot_drop_plan_f0188_pc34(
    const DM1_MeleeF0188GroupSlotDropInputPc34* in,
    DM1_MeleeF0188GroupSlotDropPlanPc34* out);
int dm1_v1_melee_killed_some_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_killed_some_fear_apply_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStateInputPc34* in,
    int shouldFlee,
    int fleeDelay,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_killed_some_fear_apply_from_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStatePlanPc34* statePlan,
    int shouldFlee,
    int fleeDelay,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_killed_some_state_apply_plan_f0190_pc34(
    const DM1_MeleeF0190KilledSomeStatePlanPc34* statePlan,
    DM1_MeleeF0190KilledSomeStateApplyPlanPc34* out);
int dm1_v1_melee_killed_all_state_plan_f0190_pc34(
    const DM1_MeleeF0190KilledAllStateInputPc34* in,
    DM1_MeleeF0190KilledAllStatePlanPc34* out);
int dm1_v1_melee_killed_all_state_apply_plan_f0190_pc34(
    const DM1_MeleeF0190KilledAllStatePlanPc34* statePlan,
    DM1_MeleeF0190KilledAllStateApplyPlanPc34* out);
int dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(
    const DM1_MeleeF0190TimelineCleanupInputPc34* in,
    DM1_MeleeF0190TimelineCleanupPlanPc34* out);
int dm1_v1_melee_timeline_cleanup_batch_plan_f0190_pc34(
    const DM1_MeleeF0190TimelineCleanupBatchInputPc34* in,
    DM1_MeleeF0190TimelineCleanupBatchPlanPc34* out);
int dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(
    const DM1_MeleeF0190MutationDispatchInputPc34* in,
    DM1_MeleeF0190MutationDispatchPlanPc34* out);
int dm1_v1_melee_aftermath_mutation_dispatch_plan_f0231_pc34(
    const DM1_MeleeF0231AftermathPlanPc34* aftermathPlan,
    DM1_MeleeF0190MutationDispatchPlanPc34* out);
int dm1_v1_melee_mutation_dispatch_apply_plan_f0190_pc34(
    const DM1_MeleeF0190MutationDispatchPlanPc34* dispatchPlan,
    DM1_MeleeF0190MutationDispatchApplyPlanPc34* out);
int dm1_v1_melee_mutation_dispatch_fear_roll_plan_f0190_pc34(
    const DM1_MeleeF0190MutationDispatchPlanPc34* dispatchPlan,
    DM1_MeleeF0190FearRollPlanPc34* out);
int dm1_v1_melee_mutation_dispatch_fear_apply_plan_f0190_pc34(
    const DM1_MeleeF0190MutationDispatchPlanPc34* dispatchPlan,
    int shouldFlee,
    int fleeDelay,
    DM1_MeleeF0190KilledSomeStatePlanPc34* out);
int dm1_v1_melee_resolve_damage_f0231_pc34(
    struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out);
int dm1_v1_melee_apply_group_damage_plan_f0190_pc34(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    DM1_MeleeF0190GroupDamageApplyPlanPc34* out);
int dm1_v1_melee_apply_group_damage_f0190_pc34(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    int* outOutcome);

#ifdef __cplusplus
}
#endif

#endif
