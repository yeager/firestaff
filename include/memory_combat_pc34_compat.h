#ifndef REDMCSB_MEMORY_COMBAT_PC34_COMPAT_H
#define REDMCSB_MEMORY_COMBAT_PC34_COMPAT_H

/*
 * Combat / damage resolver data layer for ReDMCSB PC 3.4 — Phase 13 of M10.
 *
 * Pure, caller-driven resolvers for the two most common combat
 * interactions (champion<->creature melee) plus the shared damage
 * application + RNG primitives.
 *
 * Conventions inherited from earlier phases:
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - All serialisation is MEDIA016 / PC LSB-first, 4-byte int32.
 *   - NO globals, NO UI, NO IO, NO hidden RNG. Randomness comes via
 *     an explicit RngState_Compat parameter.
 *   - Function numbering continues after timeline (F0720–F0728) and
 *     claims F0730–F0747.
 *
 * In scope for v1 (see PHASE13_PLAN.md §1):
 *   - Champion -> creature melee resolver         (mirror of F0231)
 *   - Creature -> champion melee resolver         (mirror of F0230)
 *   - Apply damage to champion / group            (mirror of F0321 / F0190)
 *   - Deterministic RNG primitive                 (LCG)
 *   - Timeline-event builder for follow-ups      (bridges to phase 12)
 *
 * Out of scope (see §1 "Explicitly OUT"):
 *   - Projectile flight / explosion AoE           -> phase 14
 *   - Magic / spells / shields                    -> phase 14 magic
 *   - Skill experience, stamina tick, poison tick -> phase 15+
 *   - Fear / flee / aspect-update AI events       -> post-M10
 *   - Sound hooks, save-file integration          -> later phases
 */

#include <stdint.h>

#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"

/* -------- Serialised sizes (MEDIA016, 4-byte int32 fields) -------- */

#define RNG_STATE_SERIALIZED_SIZE              4   /*  1 uint32 */
#define COMBAT_ACTION_SERIALIZED_SIZE         48   /* 12 int32  */
#define COMBAT_RESULT_SERIALIZED_SIZE         56   /* 14 int32  */
#define COMBATANT_CHAMPION_SERIALIZED_SIZE    76   /* 19 int32  */
#define COMBATANT_CREATURE_SERIALIZED_SIZE    52   /* 13 int32  */
#define WEAPON_PROFILE_SERIALIZED_SIZE        32   /*  8 int32  */

/* -------- Outcome codes (mirror of DEFS.H combat outcomes) -------- */

#define COMBAT_OUTCOME_INVALID                (-1)
#define COMBAT_OUTCOME_MISS                    0
#define COMBAT_OUTCOME_HIT_NO_DAMAGE           1
#define COMBAT_OUTCOME_HIT_DAMAGE              2
#define COMBAT_OUTCOME_KILLED_NO_CREATURES     3  /* mirror C0_OUTCOME_KILLED_NO_CREATURES_IN_GROUP */
#define COMBAT_OUTCOME_KILLED_SOME_CREATURES   4  /* mirror C1_OUTCOME_KILLED_SOME_CREATURES_IN_GROUP */
#define COMBAT_OUTCOME_KILLED_ALL_CREATURES    5  /* mirror C2_OUTCOME_KILLED_ALL_CREATURES_IN_GROUP */
#define COMBAT_OUTCOME_CHAMPION_DOWN           6

/* -------- Combat action kinds -------- */

#define COMBAT_ACTION_CHAMPION_MELEE           1
#define COMBAT_ACTION_CREATURE_MELEE           2
#define COMBAT_ACTION_APPLY_DAMAGE_CHAMPION    3
#define COMBAT_ACTION_APPLY_DAMAGE_GROUP       4

/* -------- Combat action flag bits -------- */

#define COMBAT_FLAG_USE_SHARP_DEFENSE          0x0001
#define COMBAT_FLAG_NON_MATERIAL_HIT           0x0002

/* -------- Wound masks (mirror of DEFS.H wound bits) -------- */

#define COMBAT_WOUND_NONE                      0x0000
#define COMBAT_WOUND_READY_HAND                0x0001
#define COMBAT_WOUND_HEAD                      0x0002
#define COMBAT_WOUND_TORSO                     0x0004
#define COMBAT_WOUND_ACTION_HAND               0x0008
#define COMBAT_WOUND_LEGS                      0x0010
#define COMBAT_WOUND_FEET                      0x0020

/* -------- Attack types (mirror of DEFS.H C0..C7_ATTACK_* set) -------- */

#define COMBAT_ATTACK_NORMAL                   0
#define COMBAT_ATTACK_FIRE                     1
#define COMBAT_ATTACK_SELF                     2
#define COMBAT_ATTACK_BLUNT                    3
#define COMBAT_ATTACK_SHARP                    4
#define COMBAT_ATTACK_MAGIC                    5
#define COMBAT_ATTACK_PSYCHIC                  6
#define COMBAT_ATTACK_LIGHTNING                7

/* -------- Weapon icon indices used by armour-piercing branches -------- */

#define COMBAT_ICON_DIAMOND_EDGE              39   /* C039_ICON_WEAPON_DIAMOND_EDGE */
#define COMBAT_ICON_VORPAL_BLADE              40   /* C040_ICON_WEAPON_VORPAL_BLADE */
#define COMBAT_ICON_HARDCLEAVE_EXECUTIONER    43   /* C043_ICON_WEAPON_HARDCLEAVE_EXECUTIONER */

/* -------- RNG state (pure LCG, 32-bit seed) -------- */

struct RngState_Compat {
    uint32_t seed;
};

/*
 * Caller-owned snapshot of a champion. Combat never touches live
 * ChampionState_Compat — the caller pre-computes all derived values
 * (strength, dexterity, per-slot wound defence) and passes this blob.
 *
 * Field order matches serialisation offsets (each entry 4 bytes LE int):
 *   +00 championIndex
 *   +04 currentHealth
 *   +08 dexterity
 *   +12 strengthActionHand
 *   +16 skillLevelParry
 *   +20 skillLevelAction
 *   +24 statisticVitality
 *   +28 statisticAntifire
 *   +32 statisticAntimagic
 *   +36 actionHandIcon
 *   +40 wounds
 *   +44 woundDefense[0]
 *   +48 woundDefense[1]
 *   +52 woundDefense[2]
 *   +56 woundDefense[3]
 *   +60 woundDefense[4]
 *   +64 woundDefense[5]
 *   +68 isResting
 *   +72 partyShieldDefense
 *   = 76 bytes
 */
struct CombatantChampionSnapshot_Compat {
    int championIndex;
    int currentHealth;
    int dexterity;
    int strengthActionHand;
    int skillLevelParry;
    int skillLevelAction;
    int statisticVitality;
    int statisticAntifire;
    int statisticAntimagic;
    int actionHandIcon;
    int wounds;
    int woundDefense[6];
    int isResting;
    int partyShieldDefense;
};

/*
 * Caller-owned snapshot of a creature, mirror of selected CREATURE_INFO
 * fields plus the group-slot context needed to apply the damage later.
 */
struct CombatantCreatureSnapshot_Compat {
    int creatureType;
    int attack;
    int defense;
    int dexterity;
    int baseHealth;
    int poisonAttack;
    int attackType;
    int attributes;
    int woundProbabilities;
    int properties;
    int doubledMapDifficulty;
    int creatureIndex;
    int healthBefore;
};

/*
 * Caller-supplied weapon stats. weaponType == -1 and weaponClass == -1
 * denote an unarmed (fist) attack; in that case weaponStrength is the
 * champion's action-hand contribution and damageFactor / hitProbability
 * carry the action's own numbers.
 */
struct WeaponProfile_Compat {
    int weaponType;
    int weaponClass;
    int weaponStrength;
    int kineticEnergy;
    int hitProbability;
    int damageFactor;
    int skillIndex;
    int attributes;
};

/*
 * One pending combat invocation — the data handed to the resolvers.
 */
struct CombatAction_Compat {
    int kind;
    int allowedWounds;
    int attackTypeCode;
    int rawAttackValue;
    int targetMapIndex;
    int targetMapX;
    int targetMapY;
    int targetCell;
    int attackerSlotOrCreatureIndex;
    int defenderSlotOrCreatureIndex;
    int scheduleDelayTicks;
    int flags;
};

/*
 * Output of every resolver. Pure result; no pointers, no aliasing.
 */
struct CombatResult_Compat {
    int outcome;
    int damageApplied;
    int rawAttackRoll;
    int defenseRoll;
    int hitLanded;
    int wasCritical;
    int woundMaskAdded;
    int poisonAttackPending;
    int targetKilled;
    int creatureSlotRemoved;
    int followupEventKind;
    int followupEventAux0;
    int rngCallCount;
    int wakeFromRest;
};

/* ==========================================================
 * Group A — RNG primitives (F0730–F0732).
 * Side-effect is through out-params / the rng state only.
 * ========================================================== */

int F0730_COMBAT_RngInit_Compat(
    struct RngState_Compat* rng,
    uint32_t seed);

uint32_t F0731_COMBAT_RngNextRaw_Compat(
    struct RngState_Compat* rng);

int F0732_COMBAT_RngRandom_Compat(
    struct RngState_Compat* rng,
    int modulus);

/* ==========================================================
 * Group B — Defence helpers (F0733–F0734). No RNG.
 * ========================================================== */

int F0733_COMBAT_GetChampionWoundDefense_Compat(
    const struct CombatantChampionSnapshot_Compat* champ,
    int woundSlotIndex,
    int useSharpDefense,
    int* outDefense);

int F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
    int statisticCurrent,
    int statisticMaximum,
    int attack,
    int* outAdjusted);

/* ==========================================================
 * Group C — Resolvers (F0735–F0736).
 * Pure except for the RNG advance, recorded in out->rngCallCount.
 * ========================================================== */

int F0735_COMBAT_ResolveChampionMelee_Compat(
    const struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out);

int F0736_COMBAT_ResolveCreatureMelee_Compat(
    const struct CombatantCreatureSnapshot_Compat* attacker,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out);

/* ==========================================================
 * Group D — Application (F0737–F0738). Pure mutators.
 * ========================================================== */

int F0737_COMBAT_ApplyDamageToChampion_Compat(
    const struct CombatResult_Compat* result,
    struct ChampionState_Compat* champ,
    int* outWasKilled);

int F0738_COMBAT_ApplyDamageToGroup_Compat(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    int* outOutcome);

/* ==========================================================
 * Group E — Timeline bridge (F0739).
 * Returns 1 iff a follow-up event should be scheduled.
 * ========================================================== */

int F0739_COMBAT_BuildTimelineEvent_Compat(
    const struct CombatAction_Compat* action,
    const struct CombatResult_Compat* result,
    uint32_t nowTick,
    struct TimelineEvent_Compat* outEvent);

/* ==========================================================
 * Group F — Serialisation (F0740–F0747 + internal helpers).
 * MEDIA016 / PC LSB-first. Every struct round-trips bit-identical.
 * ========================================================== */

int F0740_COMBAT_ActionSerialize_Compat(
    const struct CombatAction_Compat* action,
    unsigned char* outBuf,
    int outBufSize);

int F0741_COMBAT_ActionDeserialize_Compat(
    struct CombatAction_Compat* action,
    const unsigned char* buf,
    int bufSize);

int F0742_COMBAT_ResultSerialize_Compat(
    const struct CombatResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize);

int F0743_COMBAT_ResultDeserialize_Compat(
    struct CombatResult_Compat* result,
    const unsigned char* buf,
    int bufSize);

int F0744_COMBAT_ChampionSnapshotSerialize_Compat(
    const struct CombatantChampionSnapshot_Compat* champ,
    unsigned char* outBuf,
    int outBufSize);

int F0745_COMBAT_ChampionSnapshotDeserialize_Compat(
    struct CombatantChampionSnapshot_Compat* champ,
    const unsigned char* buf,
    int bufSize);

int F0746_COMBAT_CreatureSnapshotSerialize_Compat(
    const struct CombatantCreatureSnapshot_Compat* creature,
    unsigned char* outBuf,
    int outBufSize);

int F0747_COMBAT_CreatureSnapshotDeserialize_Compat(
    struct CombatantCreatureSnapshot_Compat* creature,
    const unsigned char* buf,
    int bufSize);

/* Internal helpers for WeaponProfile (sharing F0747 numbering range,
 * per PHASE13_PLAN.md §3 F0747.1 / F0747.2 note). */
int F0747a_COMBAT_WeaponProfileSerialize_Compat(
    const struct WeaponProfile_Compat* weapon,
    unsigned char* outBuf,
    int outBufSize);

int F0747b_COMBAT_WeaponProfileDeserialize_Compat(
    struct WeaponProfile_Compat* weapon,
    const unsigned char* buf,
    int bufSize);

#endif /* REDMCSB_MEMORY_COMBAT_PC34_COMPAT_H */
