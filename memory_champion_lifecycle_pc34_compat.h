#ifndef REDMCSB_MEMORY_CHAMPION_LIFECYCLE_PC34_COMPAT_H
#define REDMCSB_MEMORY_CHAMPION_LIFECYCLE_PC34_COMPAT_H

/*
 * Champion lifecycle data layer for ReDMCSB PC 3.4 — Phase 18 of M10.
 *
 * Pure, caller-driven port of:
 *   - CHAMPION.C:F0331_CHAMPION_ApplyTimeEffects_CPSF (hunger/thirst,
 *     stamina/health/mana regen, stat drift, temp XP decay).
 *   - CHAMPION.C:F0310_CHAMPION_GetMovementTicks     (move cooldown).
 *   - CHAMPION.C:F0303_CHAMPION_GetSkillLevel        (XP -> level).
 *   - CHAMPION.C:F0304_CHAMPION_AddSkillExperience   (XP award + level up).
 *   - CHAMPION.C:F0322_CHAMPION_Poison               (poison self-reschedule).
 *   - CHAMPION.C:F0325_CHAMPION_DecrementStamina     (starvation damage).
 *   - TIMELINE.C:C70..C83 event dispatch             (status expiry).
 *
 * Conventions (inherited from Phases 10-17):
 *   - All symbols suffixed _pc34_compat / _Compat.
 *   - MEDIA016 / PC LSB-first serialisation. Every struct round-trips
 *     bit-identical.
 *   - NO globals, NO UI, NO IO. Randomness flows through Phase 13's
 *     explicit RngState_Compat.
 *   - Function numbering claims F0830..F0859 exclusively.
 *   - ADDITIVE ONLY: consumes Phase 10..17 interfaces; never edits them.
 *
 * CRITICAL drift note (plan §2.9): Phase 10 ChampionState_Compat has
 * `food` / `water` as unsigned char (0..255). Fontanel's semantic range
 * is int16 with [-1024, +max]. Phase 18 carries its OWN int16 food/water
 * in ChampionLifecycleState_Compat; Phase 10 fields are never modified
 * by this phase.
 */

#include <stdint.h>
#include <stddef.h>

#include "memory_champion_state_pc34_compat.h"
#include "memory_timeline_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

/* -------- Skill indices (DEFS.H:C0_SKILL_FIGHTER..C19_SKILL_WATER) -- */

#define LIFECYCLE_SKILL_FIGHTER    0
#define LIFECYCLE_SKILL_NINJA      1
#define LIFECYCLE_SKILL_PRIEST     2
#define LIFECYCLE_SKILL_WIZARD     3
#define LIFECYCLE_SKILL_SWING      4
#define LIFECYCLE_SKILL_THRUST     5
#define LIFECYCLE_SKILL_CLUB       6
#define LIFECYCLE_SKILL_PARRY      7
#define LIFECYCLE_SKILL_STEAL      8
#define LIFECYCLE_SKILL_FIGHT      9
#define LIFECYCLE_SKILL_THROW     10
#define LIFECYCLE_SKILL_SHOOT     11
#define LIFECYCLE_SKILL_IDENTIFY  12
#define LIFECYCLE_SKILL_HEAL      13
#define LIFECYCLE_SKILL_INFLUENCE 14
#define LIFECYCLE_SKILL_DEFEND    15
#define LIFECYCLE_SKILL_FIRE      16
#define LIFECYCLE_SKILL_AIR       17
#define LIFECYCLE_SKILL_EARTH     18
#define LIFECYCLE_SKILL_WATER     19
#define LIFECYCLE_SKILL_COUNT     20

/* First hidden skill (base 0..3 map to hidden 4..19 via
 * hidden = C04 + (base << 2) + subIdx). */
#define LIFECYCLE_HIDDEN_SKILL_FIRST  LIFECYCLE_SKILL_SWING

/* -------- Statistic indices (DEFS.H:C0..C6_STATISTIC_*) ------------- */

#define LIFECYCLE_STAT_LUCK       0
#define LIFECYCLE_STAT_STRENGTH   1
#define LIFECYCLE_STAT_DEXTERITY  2
#define LIFECYCLE_STAT_WISDOM     3
#define LIFECYCLE_STAT_VITALITY   4
#define LIFECYCLE_STAT_ANTIMAGIC  5
#define LIFECYCLE_STAT_ANTIFIRE   6
#define LIFECYCLE_STAT_COUNT      7

/* Each statistic row = [MAX, CURRENT, MIN] as unsigned char, mirroring
 * Fontanel's `Statistics[7][3]` layout. */
#define LIFECYCLE_STAT_MAXIMUM    0
#define LIFECYCLE_STAT_CURRENT    1
#define LIFECYCLE_STAT_MINIMUM    2

/* -------- Wound bit flags (mirror combat wound masks) --------------- */

#define LIFECYCLE_WOUND_READY_HAND  0x0001
#define LIFECYCLE_WOUND_HEAD        0x0002
#define LIFECYCLE_WOUND_TORSO       0x0004
#define LIFECYCLE_WOUND_ACTION_HAND 0x0008
#define LIFECYCLE_WOUND_LEGS        0x0010
#define LIFECYCLE_WOUND_FEET        0x0020

/* -------- Footwear / neck icon constants used by regen rules -------- */

#define LIFECYCLE_ICON_NONE              (-1)
#define LIFECYCLE_ICON_EKKHARD_CROSS     9    /* C009_ICON_NECK_EKKHARD_CROSS */
#define LIFECYCLE_ICON_BOOT_OF_SPEED    66    /* C066_ICON_ARMOUR_BOOT_OF_SPEED */

/* -------- Rest interrupt reasons ----------------------------------- */

#define LIFECYCLE_REST_INTERRUPT_NONE       0
#define LIFECYCLE_REST_INTERRUPT_MONSTER    1
#define LIFECYCLE_REST_INTERRUPT_PROJECTILE 2
#define LIFECYCLE_REST_INTERRUPT_DEATH      3

/* -------- Status-effect event kinds (mirror of TIMELINE.C C70..C83) - */

#define LIFECYCLE_STATUS_LIGHT            70  /* C70 */
#define LIFECYCLE_STATUS_INVISIBILITY     71  /* C71 */
#define LIFECYCLE_STATUS_CHAMPION_SHIELD  72  /* C72 */
#define LIFECYCLE_STATUS_THIEVES_EYE      73  /* C73 */
#define LIFECYCLE_STATUS_PARTY_SHIELD     74  /* C74 */
#define LIFECYCLE_STATUS_POISON           75  /* C75 */
#define LIFECYCLE_STATUS_SPELL_SHIELD     77  /* C77 */
#define LIFECYCLE_STATUS_FIRE_SHIELD      78  /* C78 */
#define LIFECYCLE_STATUS_FOOTPRINTS       79  /* C79 */
#define LIFECYCLE_STATUS_MAGIC_MAP_LO     80  /* C80..C83 magic map stubs */
#define LIFECYCLE_STATUS_MAGIC_MAP_HI     83

/* -------- Tunable constants (F0331 mirrors) ------------------------ */

#define LIFECYCLE_FOOD_FLOOR       (-1024)
#define LIFECYCLE_WATER_FLOOR      (-1024)
#define LIFECYCLE_FOOD_WATER_STARVE_THRESHOLD (-512)
#define LIFECYCLE_STAT_DRIFT_PERIOD_NORMAL  256
#define LIFECYCLE_STAT_DRIFT_PERIOD_REST     64
#define LIFECYCLE_POISON_RESCHEDULE_TICKS    36
#define LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY  80
#define LIFECYCLE_IDLE_STAMINA_BONUS_2_DELAY 250
#define LIFECYCLE_MAX_HEALTH_CAP   999
#define LIFECYCLE_MAX_STAMINA_CAP 9999
#define LIFECYCLE_MAX_MANA_CAP     900
#define LIFECYCLE_TEMP_XP_CAP    32000
#define LIFECYCLE_RECENT_COMBAT_WINDOW  25
#define LIFECYCLE_STALE_COMBAT_WINDOW  150
#define LIFECYCLE_XP_LEVEL_THRESHOLD   500

/* -------- Serialised sizes (per plan §2) --------------------------- */

#define SKILL_STATE_SERIALIZED_SIZE          8   /* 4 + 2 + 2 pad */
#define MOVE_TIMER_STATE_SERIALIZED_SIZE    12   /* 4 + 4 + 2 + 2 pad */
#define STATUS_EFFECT_STATE_SERIALIZED_SIZE 16   /* 7 × 2 + 2 pad */
#define REST_STATE_SERIALIZED_SIZE          12   /* 1 + 1 + 2 pad + 4 + 4 */
#define CHAMPION_LIFECYCLE_STATE_SERIALIZED_SIZE 208
#define LIFECYCLE_STATE_SERIALIZED_SIZE    872

/* -------- Data structures ----------------------------------------- */

struct SkillState_Compat {
    int32_t  experience;          /* 4 */
    int16_t  temporaryExperience; /* 2 */
    uint8_t  padding[2];          /* 2 */
};

struct MoveTimerState_Compat {
    uint32_t lastMoveTick;        /* 4 */
    uint32_t nextAllowedTick;     /* 4 */
    uint16_t cachedMoveTicks;     /* 2 */
    uint8_t  padding[2];          /* 2 */
};

struct StatusEffectState_Compat {
    int16_t  partyShieldDefense;       /* 2 */
    int16_t  partySpellShieldDefense;  /* 2 */
    int16_t  partyFireShieldDefense;   /* 2 */
    uint16_t invisibilityCount;        /* 2 */
    uint16_t thievesEyeCount;          /* 2 */
    uint16_t footprintsCount;          /* 2 */
    uint16_t lightEventCount;          /* 2 — v1 stub counter */
    uint8_t  padding[2];               /* 2 */
};

struct RestState_Compat {
    uint8_t  isResting;          /* 1 */
    uint8_t  interruptReason;    /* 1 */
    uint8_t  padding[2];         /* 2 */
    uint32_t restStartTick;      /* 4 */
    uint32_t lastMovementTime;   /* 4 */
};

struct ChampionLifecycleState_Compat {
    struct SkillState_Compat    skills20[LIFECYCLE_SKILL_COUNT]; /* 160 bytes */
    struct MoveTimerState_Compat moveTimer;                      /*  12 bytes */
    int16_t  shieldDefense;       /*   2 — C72 champion-shield accumulator */
    uint8_t  poisonEventCount;    /*   1 — active poison event chains */
    uint8_t  padding;             /*   1 */
    int16_t  food;                /*   2 — Phase 18 authoritative food */
    int16_t  water;               /*   2 — Phase 18 authoritative water */
    uint8_t  statistics[LIFECYCLE_STAT_COUNT][3]; /* 21 — max/cur/min */
    uint8_t  statPadding;         /*   1 */
    uint16_t maxHealth;           /*   2 */
    uint16_t maxStamina;          /*   2 */
    uint16_t maxMana;             /*   2 */
};

struct LifecycleState_Compat {
    struct ChampionLifecycleState_Compat champions[CHAMPION_MAX_PARTY]; /* 832 */
    struct StatusEffectState_Compat      status;       /* 16 */
    struct RestState_Compat              rest;         /* 12 */
    uint32_t lastCreatureAttackTime; /*  4 */
    uint32_t gameTime;               /*  4 */
    uint8_t  padding[4];             /*  4 */
};

/* Per-tick input aggregate for hunger/thirst (not persisted). */
struct HungerThirstInput_Compat {
    int16_t  currentStamina;
    int16_t  maxStamina;
    uint8_t  isResting;
    uint8_t  padding[3];
    uint32_t gameTime;
    uint32_t lastMovementTime;
};

/* Per-tick output aggregate (net stamina/health change, food/water clamp). */
struct HungerThirstResult_Compat {
    int16_t newFood;
    int16_t newWater;
    int16_t netStaminaChange;    /* positive = loss, negative = gain */
    int16_t healthDamage;        /* from starvation stamina overflow */
};

/* Level-up marker emitted on base-skill level increases. */
struct LevelUpMarker_Compat {
    int championIndex;
    int baseSkillIndex;
    int newLevel;
    int previousLevel;
};

/* ==========================================================
 *  Group A — Hunger/thirst lifecycle (F0830-F0834)
 * ========================================================== */

uint16_t F0830_LIFECYCLE_ComputeTimeCriteria_Compat(uint32_t gameTime);

int16_t F0831_LIFECYCLE_ComputeStaminaAmount_Compat(
    uint16_t maxStamina,
    int isResting,
    uint32_t lastMovementTime,
    uint32_t gameTime);

int16_t F0832_LIFECYCLE_TickHungerThirst_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int16_t staminaAmount,
    int16_t currentStamina);

int F0833_LIFECYCLE_ApplyHungerThirstFull_Compat(
    struct ChampionLifecycleState_Compat* champ,
    const struct HungerThirstInput_Compat* in,
    struct HungerThirstResult_Compat* out);

int F0834_LIFECYCLE_ClampFoodWater_Compat(
    int16_t* food,
    int16_t* water);

/* ==========================================================
 *  Group B — Status expiry (F0835-F0840)
 * ========================================================== */

int F0835_LIFECYCLE_HandleStatusExpiry_Compat(
    struct LifecycleState_Compat* state,
    const struct TimelineEvent_Compat* expired,
    int championIndex,
    struct TimelineEvent_Compat* outRescheduled);

int F0836_LIFECYCLE_HandlePoisonTick_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int poisonAttackBefore,
    uint32_t currentTick,
    int* outDamage,
    int* outNewAttack,
    struct TimelineEvent_Compat* outReschedule);

int F0837_LIFECYCLE_HandleShieldExpiry_Compat(
    struct LifecycleState_Compat* state,
    int championIndex,
    int isChampionShield,
    int defense);

int F0838_LIFECYCLE_HandleMagicExpiry_Compat(
    struct LifecycleState_Compat* state,
    int isFireShield,
    int defense);

int F0839_LIFECYCLE_HandleCounterExpiry_Compat(
    struct LifecycleState_Compat* state,
    int statusKind);

int F0840_LIFECYCLE_HandleLightExpiry_Compat(
    struct LifecycleState_Compat* state);

/* ==========================================================
 *  Group C — Move-timer (F0841-F0843)
 * ========================================================== */

uint16_t F0841_LIFECYCLE_ComputeMoveTicks_Compat(
    uint16_t load,
    uint16_t maxLoad,
    unsigned short wounds,
    int footwearIcon);

int F0842_LIFECYCLE_UpdateMoveTimer_Compat(
    struct MoveTimerState_Compat* timer,
    uint32_t currentTick,
    uint16_t moveTicks);

int F0843_LIFECYCLE_CanChampionMove_Compat(
    const struct MoveTimerState_Compat* timer,
    uint32_t currentTick);

/* ==========================================================
 *  Group D — Regen (F0844-F0847)
 * ========================================================== */

int F0844_LIFECYCLE_ApplyHealthRegen_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint16_t* currentHealth,
    uint16_t currentStamina,
    uint32_t gameTime,
    int isResting,
    int neckSlotIcon);

int F0845_LIFECYCLE_ApplyManaRegen_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint16_t* currentMana,
    int16_t* currentStamina,
    int wizardLevel,
    int priestLevel,
    uint32_t gameTime,
    int isResting);

int F0846_LIFECYCLE_ApplyStatDrift_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint32_t gameTime,
    int isResting);

int F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat(
    struct ChampionLifecycleState_Compat* champ);

/* ==========================================================
 *  Group E — XP & level-up (F0848-F0853)
 * ========================================================== */

int F0848_LIFECYCLE_ComputeSkillLevel_Compat(
    const struct ChampionLifecycleState_Compat* champ,
    int skillIndex,
    int ignoreTemporary);

int F0849_LIFECYCLE_AddSkillExperience_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int skillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    int* outBaseLevelBefore,
    int* outBaseLevelAfter);

int F0850_LIFECYCLE_ApplyLevelUp_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int baseSkillIndex,
    int newLevel,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker);

int F0851_LIFECYCLE_AwardCombatXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int physicalSkillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker);

int F0852_LIFECYCLE_AwardMagicXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int magicSkillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker);

int F0853_LIFECYCLE_AwardKillXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int creatureType,
    int* outAwarded);

/* ==========================================================
 *  Group F — Emission helpers (F0854-F0856)
 * ========================================================== */

int F0854_LIFECYCLE_EmitTimelineEvent_Compat(
    int kind,
    uint32_t fireAtTick,
    int championIndex,
    int aux0,
    int aux1,
    struct TimelineEvent_Compat* outEvent);

int F0855_LIFECYCLE_ScheduleNextHungerThirst_Compat(
    uint32_t currentTick,
    int championIndex,
    struct TimelineEvent_Compat* outEvent);

int F0856_LIFECYCLE_BuildLevelUpMarker_Compat(
    int championIndex,
    int baseSkillIndex,
    int previousLevel,
    int newLevel,
    struct LevelUpMarker_Compat* outMarker);

/* ==========================================================
 *  Group G — Serialisation (F0857-F0859)
 * ========================================================== */

int F0857_LIFECYCLE_Serialize_Compat(
    const struct LifecycleState_Compat* state,
    unsigned char* outBuf,
    int outBufSize);

int F0858_LIFECYCLE_Deserialize_Compat(
    struct LifecycleState_Compat* state,
    const unsigned char* buf,
    int bufSize);

int F0859_LIFECYCLE_Init_Compat(
    struct LifecycleState_Compat* state,
    const struct PartyState_Compat* party);

#endif /* REDMCSB_MEMORY_CHAMPION_LIFECYCLE_PC34_COMPAT_H */
