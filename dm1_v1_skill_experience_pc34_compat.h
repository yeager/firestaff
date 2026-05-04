/*
 * dm1_v1_skill_experience_pc34_compat.h — DM1 V1 Skill & Experience System
 *
 * Source-locked to ReDMCSB: CHAMPION.C (F0303_CHAMPION_GetSkillLevel,
 * F0304_CHAMPION_AddSkillExperience), DEFS.H (skill indices, SKILL struct,
 * statistic indices, experience masks), PANEL.C (G0428_apc_SkillLevelNames),
 * MENU.C (G0497_auc_Graphic560_ActionExperienceGain).
 *
 * Implements: 4 base skills + 16 sub-skills with correct ReDMCSB indices,
 * experience-to-level conversion (F0303 algorithm), experience gain with
 * difficulty multiplier and combat proximity bonus (F0304), stat bonuses
 * on level-up, temporary experience, skill level name display.
 */
#ifndef FIRESTAFF_DM1_V1_SKILL_EXPERIENCE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SKILL_EXPERIENCE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Base skill indices (DEFS.H lines 757-760) ─────────────────────── */
enum {
    DM1_SKILL_IDX_FIGHTER  = 0,
    DM1_SKILL_IDX_NINJA    = 1,
    DM1_SKILL_IDX_PRIEST   = 2,
    DM1_SKILL_IDX_WIZARD   = 3
};
#define DM1_BASE_SKILL_COUNT 4

/* ── Sub-skill indices (DEFS.H lines 761-776) ──────────────────────── */
/* Fighter sub-skills */
enum {
    DM1_SKILL_IDX_SWING   = 4,
    DM1_SKILL_IDX_THRUST  = 5,
    DM1_SKILL_IDX_CLUB    = 6,
    DM1_SKILL_IDX_PARRY   = 7
};
/* Ninja sub-skills */
enum {
    DM1_SKILL_IDX_STEAL   = 8,
    DM1_SKILL_IDX_FIGHT   = 9,
    DM1_SKILL_IDX_THROW   = 10,
    DM1_SKILL_IDX_SHOOT   = 11
};
/* Priest sub-skills */
enum {
    DM1_SKILL_IDX_IDENTIFY  = 12,
    DM1_SKILL_IDX_HEAL      = 13,
    DM1_SKILL_IDX_INFLUENCE = 14,
    DM1_SKILL_IDX_DEFEND    = 15
};
/* Wizard sub-skills */
enum {
    DM1_SKILL_IDX_FIRE   = 16,
    DM1_SKILL_IDX_AIR    = 17,
    DM1_SKILL_IDX_EARTH  = 18,
    DM1_SKILL_IDX_WATER  = 19
};
#define DM1_TOTAL_SKILL_COUNT 20
#define DM1_SUB_SKILL_FIRST   4

/* ── Skill level names (PANEL.C lines 25-40) ───────────────────────── */
/* Level 1 = NEOPHYTE, level 16 = ARCHMASTER. Index = level - 1.
 * ReDMCSB uses index [level - 2] into a 15-entry array because level 1
 * is the unnamed minimum. We provide 16 entries for direct [level-1] access. */
#define DM1_SKILL_LEVEL_NAME_COUNT 16
#define DM1_MAX_SKILL_LEVEL       16

/* ── SKILL struct (DEFS.H lines 605-622) ───────────────────────────── */
typedef struct {
    int16_t  temporaryExperience;  /* Transient XP boost; capped at 32000 */
    int32_t  experience;           /* Permanent experience (long in original) */
} DM1_Skill;

/* ── Stat bonuses on level-up (F0304 output) ──────────────────────── */
typedef struct {
    int strengthDelta;
    int dexterityDelta;
    int wisdomDelta;
    int vitalityDelta;
    int antifireDelta;
    int antimagicDelta;
    int maxHealthDelta;
    int maxStaminaDelta;
    int maxManaDelta;
} DM1_LevelUpBonuses;

/* ── Champion skill state ─────────────────────────────────────────── */
typedef struct {
    DM1_Skill skills[DM1_TOTAL_SKILL_COUNT];
} DM1_ChampionSkillState;

/* ── Experience gain context (F0304 environment) ──────────────────── */
typedef struct {
    int      mapDifficulty;          /* G0269_ps_CurrentMap->C.Difficulty */
    int32_t  lastCreatureAttackTime; /* G0361_l_LastCreatureAttackTime */
    int32_t  gameTime;               /* G0313_ul_GameTime */
    int      partyIsResting;         /* G0300_B_PartyIsResting */
} DM1_SkillContext;

/* ── Core API ─────────────────────────────────────────────────────── */

/**
 * Get the base skill index for any skill (base or sub).
 * For base skills (0-3): returns the skill itself.
 * For sub-skills (4-19): returns (skillIndex - 4) >> 2.
 * ReDMCSB: CHAMPION.C F0304, line ~874.
 */
int dm1_skill_get_base_index(int skillIndex);

/**
 * Compute skill level from experience.
 * ReDMCSB: CHAMPION.C F0303 (lines 715-820).
 * Algorithm: level=1; while(exp>=500) { exp>>=1; level++; }
 * For sub-skills, uses average of (base XP + sub XP).
 *
 * flags: bit 0 = ignore temporary experience
 *        bit 1 = ignore object modifiers (unused in our impl; reserved)
 */
int dm1_skill_get_level(const DM1_ChampionSkillState* state,
                        int skillIndex, int flags);

/**
 * Get the raw experience for a skill (permanent + optional temporary).
 */
int32_t dm1_skill_get_experience(const DM1_ChampionSkillState* state,
                                 int skillIndex, int includeTemp);

/**
 * Add experience to a skill. Returns the level-up bonuses if a level
 * change occurred (all zeros otherwise).
 * ReDMCSB: CHAMPION.C F0304 (lines 823-977).
 * Implements: difficulty multiplier, combat proximity bonus/penalty,
 * temporary XP gain, base+sub XP propagation, stat bonuses on level-up.
 *
 * The random callbacks (for stat bonuses) use a simple PRNG seeded internally.
 */
DM1_LevelUpBonuses dm1_skill_add_experience(
    DM1_ChampionSkillState* state,
    int skillIndex,
    int experience,
    const DM1_SkillContext* ctx,
    uint32_t* rngState);

/**
 * Get minimum experience needed to reach a given level.
 * Level 1 = 0, Level 2 = 500, Level 3 = 1000, Level 4 = 2000, etc.
 */
int32_t dm1_skill_level_threshold(int level);

/**
 * Get the display name for a skill level (1-16).
 * ReDMCSB: PANEL.C G0428_apc_SkillLevelNames.
 * Returns "NONE" for level <= 0.
 */
const char* dm1_skill_level_name(int level);

/**
 * Get the display name for a skill index (0-19).
 */
const char* dm1_skill_index_name(int skillIndex);

/**
 * Get the base skill class name (FIGHTER/NINJA/PRIEST/WIZARD).
 * ReDMCSB: CHAMPION.C G0417_apc_BaseSkillNames.
 */
const char* dm1_skill_base_name(int baseSkillIndex);

/**
 * Initialize a champion's skill state to zero.
 */
void dm1_skill_state_init(DM1_ChampionSkillState* state);

/* ── Flags for dm1_skill_get_level ────────────────────────────────── */
#define DM1_SKILL_FLAG_IGNORE_TEMP    0x01  /* MASK0x8000 equivalent */
#define DM1_SKILL_FLAG_IGNORE_OBJECTS 0x02  /* MASK0x4000 equivalent */

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_SKILL_EXPERIENCE_PC34_COMPAT_H */
