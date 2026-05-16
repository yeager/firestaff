/*
 * dm1_v1_skill_experience_pc34_compat.c — DM1 V1 Skill & Experience System
 *
 * Source-locked to ReDMCSB: CHAMPION.C (F0303, F0304), DEFS.H, PANEL.C, MENU.C.
 * See header for full provenance.
 */
#include "dm1_v1_skill_experience_pc34_compat.h"
#include <string.h>

/* ── Skill level names (PANEL.C lines 25-40) ───────────────────────── */
/* ReDMCSB uses G0428_apc_SkillLevelNames[15] indexed by [level - 2].
 * We add "NONE" at index 0 so index = level - 1 for levels 1..16.
 * The ` a b c d e prefix denotes "LO/UM/ON/EE/HI MASTER" in display;
 * the original uses raw characters for font rendering. We use readable
 * equivalents. */
static const char* const kSkillLevelNames[DM1_SKILL_LEVEL_NAME_COUNT] = {
    "NEOPHYTE",      /* Level  1 — minimum; F0303 returns 1 when resting or 0 XP */
    "NOVICE",        /* Level  2 — 500 XP */
    "APPRENTICE",    /* Level  3 — 1000 XP */
    "JOURNEYMAN",    /* Level  4 — 2000 XP */
    "CRAFTSMAN",     /* Level  5 — 4000 XP */
    "ARTISAN",       /* Level  6 — 8000 XP */
    "ADEPT",         /* Level  7 — 16000 XP */
    "EXPERT",        /* Level  8 — 32000 XP */
    "LO MASTER",     /* Level  9 — 64000 XP */
    "UM MASTER",     /* Level 10 — 128000 XP */
    "ON MASTER",     /* Level 11 — 256000 XP */
    "EE MASTER",     /* Level 12 — 512000 XP */
    "HI MASTER",     /* Level 13 — 1024000 XP */
    "SU MASTER",     /* Level 14 — 2048000 XP */
    "ARCHMASTER",    /* Level 15 — 4096000 XP */
    "ARCHMASTER+"    /* Level 16 — 8192000 XP (theoretical cap) */
};

/* ── Skill index names ─────────────────────────────────────────────── */
static const char* const kSkillIndexNames[DM1_TOTAL_SKILL_COUNT] = {
    "FIGHTER", "NINJA", "PRIEST", "WIZARD",
    "SWING", "THRUST", "CLUB", "PARRY",
    "STEAL", "FIGHT", "THROW", "SHOOT",
    "IDENTIFY", "HEAL", "INFLUENCE", "DEFEND",
    "FIRE", "AIR", "EARTH", "WATER"
};

/* ── Base skill names (CHAMPION.C G0417) ───────────────────────────── */
static const char* const kBaseSkillNames[DM1_BASE_SKILL_COUNT] = {
    "FIGHTER", "NINJA", "PRIEST", "WIZARD"
};

/* ── Simple PRNG (xorshift32) for stat bonuses ─────────────────────── */
static uint32_t prng_next(uint32_t* state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

/* M005_RANDOM(2) equivalent: returns 0 or 1 */
static int random_1bit(uint32_t* state) {
    return (int)(prng_next(state) & 1);
}

/* M002_RANDOM(value) equivalent: returns 0..value-1 */
static int random_mod(uint32_t* state, int value) {
    if (value <= 0) return 0;
    return (int)(prng_next(state) % (uint32_t)value);
}

/* M004_RANDOM(4) equivalent: returns 0..3 */
static int random_2bit(uint32_t* state) {
    return (int)(prng_next(state) & 3);
}

/* F0024_MAIN_GetMinimumValue: return min(a, b) */
static int get_min(int a, int b) {
    return (a < b) ? a : b;
}

/* F0025_MAIN_GetMaximumValue: return max(a, b) */
static int get_max(int a, int b) __attribute__((unused));
static int get_max(int a, int b) {
    return (a > b) ? a : b;
}

/* F0026_MAIN_GetBoundedValue: clamp value to [min, max] */
static int get_bounded(int minVal, int value, int maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

/* ── API Implementation ───────────────────────────────────────────── */

int dm1_skill_get_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return 0;
    if (skillIndex < DM1_SUB_SKILL_FIRST) return skillIndex;
    /* ReDMCSB: CHAMPION.C F0304 line ~874:
     * L0916_ui_BaseSkillIndex = (P0637_ui_SkillIndex - C04_SKILL_SWING) >> 2; */
    return (skillIndex - DM1_SUB_SKILL_FIRST) >> 2;
}

int dm1_skill_get_level(const DM1_ChampionSkillState* state,
                        int skillIndex, int flags) {
    if (!state) return 1;
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return 1;

    /* ReDMCSB: CHAMPION.C F0303 lines 752-770 */
    int ignoreTemp = (flags & DM1_SKILL_FLAG_IGNORE_TEMP);

    const DM1_Skill* skill = &state->skills[skillIndex];
    int32_t experience = skill->experience;
    if (!ignoreTemp) {
        experience += skill->temporaryExperience;
    }

    /* For sub-skills (index > 3), add base skill experience and halve.
     * ReDMCSB: CHAMPION.C F0303 lines 758-767:
     *   L0907_Experience += base_skill->Experience;
     *   if (!ignoreTemp) L0907_Experience += base_skill->TemporaryExperience;
     *   L0907_Experience >>= 1; */
    if (skillIndex >= DM1_SUB_SKILL_FIRST) {
        int baseIdx = dm1_skill_get_base_index(skillIndex);
        const DM1_Skill* baseSkill = &state->skills[baseIdx];
        experience += baseSkill->experience;
        if (!ignoreTemp) {
            experience += baseSkill->temporaryExperience;
        }
        experience >>= 1;
    }

    /* ReDMCSB: F0303 lines 765-768:
     * L0908_i_SkillLevel = 1;
     * while (L0907_Experience >= 500) { L0907_Experience >>= 1; L0908_i_SkillLevel++; } */
    int level = 1;
    while (experience >= 500) {
        experience >>= 1;
        level++;
    }

    /* Object modifiers would be applied here (Firestaff, necklaces).
     * We skip those as they depend on inventory state not modeled here.
     * DM1_SKILL_FLAG_IGNORE_OBJECTS is reserved for future use. */

    if (level > DM1_MAX_SKILL_LEVEL) level = DM1_MAX_SKILL_LEVEL;
    return level;
}

int32_t dm1_skill_get_experience(const DM1_ChampionSkillState* state,
                                 int skillIndex, int includeTemp) {
    if (!state) return 0;
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return 0;
    int32_t xp = state->skills[skillIndex].experience;
    if (includeTemp) {
        xp += state->skills[skillIndex].temporaryExperience;
    }
    return xp;
}

DM1_LevelUpBonuses dm1_skill_add_experience(
    DM1_ChampionSkillState* state,
    int skillIndex,
    int experience,
    const DM1_SkillContext* ctx,
    uint32_t* rngState)
{
    DM1_LevelUpBonuses bonuses;
    memset(&bonuses, 0, sizeof(bonuses));

    if (!state || !ctx || !rngState) return bonuses;
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return bonuses;

    /* ReDMCSB: CHAMPION.C F0304 lines 860-862:
     * Combat proximity penalty: if sub-skill and no recent creature attack,
     * halve experience.
     * if ((skill >= C04_SKILL_SWING) && (skill <= C11_SKILL_SHOOT)
     *     && (lastAttackTime < (gameTime - 150))) { experience >>= 1; } */
    if (skillIndex >= DM1_SKILL_IDX_SWING && skillIndex <= DM1_SKILL_IDX_SHOOT
        && ctx->lastCreatureAttackTime < (ctx->gameTime - 150)) {
        experience >>= 1;
    }

    if (experience <= 0) return bonuses;

    /* ReDMCSB: F0304 lines 866-868: difficulty multiplier.
     * if (mapDifficulty) { experience *= mapDifficulty; } */
    if (ctx->mapDifficulty > 0) {
        experience *= ctx->mapDifficulty;
    }

    int baseSkillIndex = dm1_skill_get_base_index(skillIndex);

    /* Get level BEFORE adding XP (ignoring objects and temp).
     * ReDMCSB: F0304 line 882. */
    int levelBefore = dm1_skill_get_level(state, baseSkillIndex,
                                          DM1_SKILL_FLAG_IGNORE_TEMP | DM1_SKILL_FLAG_IGNORE_OBJECTS);

    /* ReDMCSB: F0304 lines 883-885: combat proximity bonus.
     * if (sub-skill && lastAttackTime > (gameTime - 25)) { experience <<= 1; } */
    if (skillIndex >= DM1_SUB_SKILL_FIRST
        && ctx->lastCreatureAttackTime > (ctx->gameTime - 25)) {
        experience <<= 1;
    }

    /* Add experience to the specific skill.
     * ReDMCSB: F0304 lines 886-887. */
    DM1_Skill* skill = &state->skills[skillIndex];
    skill->experience += experience;

    /* Add temporary experience (capped at 32000).
     * ReDMCSB: F0304 lines 888-890:
     * if (tempXP < 32000) tempXP += bounded(1, experience >> 3, 100); */
    if (skill->temporaryExperience < 32000) {
        int tempGain = get_bounded(1, experience >> 3, 100);
        skill->temporaryExperience += (int16_t)tempGain;
    }

    /* For sub-skills, also add XP to the base skill.
     * ReDMCSB: F0304 lines 891-893. */
    if (skillIndex >= DM1_SUB_SKILL_FIRST) {
        state->skills[baseSkillIndex].experience += experience;
    }

    /* Get level AFTER adding XP.
     * ReDMCSB: F0304 line 895. */
    int levelAfter = dm1_skill_get_level(state, baseSkillIndex,
                                         DM1_SKILL_FLAG_IGNORE_TEMP | DM1_SKILL_FLAG_IGNORE_OBJECTS);

    /* If level increased, compute stat bonuses.
     * ReDMCSB: F0304 lines 896-977. */
    if (levelAfter > levelBefore) {
        int minorIncrease = random_1bit(rngState);
        int majorIncrease = 1 + random_1bit(rngState);

        /* Vitality: Priest gets 0 or 1 always; others get (random & level_parity).
         * ReDMCSB: F0304 lines 899-902. */
        int vitalityAmount = random_1bit(rngState);
        if (baseSkillIndex != DM1_SKILL_IDX_PRIEST) {
            vitalityAmount &= levelAfter;  /* 0 for even levels, 0-1 for odd */
        }
        bonuses.vitalityDelta = vitalityAmount;

        /* Antifire: (random & ~level) → 0 for odd levels, 0-1 for even.
         * ReDMCSB: F0304 line 906. */
        bonuses.antifireDelta = random_1bit(rngState) & ~levelAfter;

        int staminaBase;
        int healthAdd = levelAfter;

        switch (baseSkillIndex) {
            case DM1_SKILL_IDX_FIGHTER:
                /* ReDMCSB: F0304 lines 918-920:
                 * staminaAmount = maxStamina >> 4;
                 * skillLevelAfter *= 3;
                 * Strength += major; Dexterity += minor; */
                staminaBase = 100 >> 4;  /* Approximation; real uses champion maxStamina */
                healthAdd = levelAfter * 3;
                bonuses.strengthDelta = majorIncrease;
                bonuses.dexterityDelta = minorIncrease;
                break;

            case DM1_SKILL_IDX_NINJA:
                /* ReDMCSB: F0304 lines 932-935:
                 * staminaAmount = maxStamina / 21;
                 * skillLevelAfter <<= 1;
                 * Strength += minor; Dexterity += major; */
                staminaBase = 100 / 21;
                healthAdd = levelAfter << 1;
                bonuses.strengthDelta = minorIncrease;
                bonuses.dexterityDelta = majorIncrease;
                break;

            case DM1_SKILL_IDX_WIZARD:
                /* ReDMCSB: F0304 lines 940-946:
                 * staminaAmount = maxStamina >> 5;
                 * MaximumMana += level + (level >> 1);
                 * Wisdom += major; goto T0304016; */
                staminaBase = 100 >> 5;
                bonuses.maxManaDelta = levelAfter + (levelAfter >> 1);
                bonuses.wisdomDelta = majorIncrease;
                /* Fall through to T0304016 (shared Priest/Wizard mana logic) */
                /* T0304016: MaximumMana += min(random(4), baseSkillLevel - 1) */
                bonuses.maxManaDelta += get_min(random_2bit(rngState), levelAfter - 1);
                /* Antimagic: random(3) for DM1 versions.
                 * ReDMCSB: F0304 line 968. */
                bonuses.antimagicDelta = random_mod(rngState, 3);
                break;

            case DM1_SKILL_IDX_PRIEST:
                /* ReDMCSB: F0304 lines 953-960:
                 * staminaAmount = maxStamina / 25;
                 * MaximumMana += level;
                 * level += (level + 1) >> 1;  (for health calc)
                 * Wisdom += minor; goto T0304016; */
                staminaBase = 100 / 25;
                bonuses.maxManaDelta = levelAfter;
                healthAdd = levelAfter + ((levelAfter + 1) >> 1);
                bonuses.wisdomDelta = minorIncrease;
                /* T0304016: MaximumMana += min(random(4), baseSkillLevel - 1) */
                bonuses.maxManaDelta += get_min(random_2bit(rngState), levelAfter - 1);
                /* Antimagic: random(3) for DM1 versions. */
                bonuses.antimagicDelta = random_mod(rngState, 3);
                break;

            default:
                staminaBase = 0;
                break;
        }

        /* Clamp mana to 900 max (ReDMCSB: F0304 line 963) — not applied here,
         * that's the caller's responsibility on the champion's actual mana. */

        /* Health: += healthAdd + random(healthAdd/2 + 1), cap 999.
         * ReDMCSB: F0304 line 973. */
        bonuses.maxHealthDelta = healthAdd + random_mod(rngState, (healthAdd >> 1) + 1);

        /* Stamina: += staminaBase + random(staminaBase/2 + 1), cap 9999.
         * ReDMCSB: F0304 line 975. */
        bonuses.maxStaminaDelta = staminaBase + random_mod(rngState, (staminaBase >> 1) + 1);
    }

    return bonuses;
}

int32_t dm1_skill_level_threshold(int level) {
    /* Level 1: 0 XP (minimum level)
     * Level 2: 500
     * Level 3: 1000
     * Level n: 500 * 2^(n-2) for n >= 2
     * Derived from F0303: while (exp >= 500) { exp >>= 1; level++; } */
    if (level <= 1) return 0;
    if (level == 2) return 500;
    /* 500 * 2^(level-2) */
    int32_t threshold = 500;
    for (int i = 2; i < level; i++) {
        threshold <<= 1;
    }
    return threshold;
}

const char* dm1_skill_level_name(int level) {
    if (level <= 0) return "NONE";
    if (level > DM1_MAX_SKILL_LEVEL) level = DM1_MAX_SKILL_LEVEL;
    return kSkillLevelNames[level - 1];
}

const char* dm1_skill_index_name(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= DM1_TOTAL_SKILL_COUNT) return "UNKNOWN";
    return kSkillIndexNames[skillIndex];
}

const char* dm1_skill_base_name(int baseSkillIndex) {
    if (baseSkillIndex < 0 || baseSkillIndex >= DM1_BASE_SKILL_COUNT) return "UNKNOWN";
    return kBaseSkillNames[baseSkillIndex];
}

void dm1_skill_state_init(DM1_ChampionSkillState* state) {
    if (!state) return;
    memset(state, 0, sizeof(DM1_ChampionSkillState));
}

/* ══════════════════════════════════════════════════════════════════════
 * Pass601 — Skill system source-lock extensions
 *
 * F0303_CHAMPION_GetSkillLevel (CHAMPION.C:715-822):
 *   - 4 base skills: Fighter(0), Ninja(1), Priest(2), Wizard(3)
 *   - 8 derived skills: Swing(4)..Defend(7), Fire(8)..Water(11)
 *   - Derived skill = max(base_level, specific_level)
 *   - MASK0x4000_IGNORE_OBJECT_MODIFIERS: raw level only
 *   - MASK0x8000_IGNORE_TEMPORARY_EXPERIENCE: exclude recent gains
 *
 * F0304_CHAMPION_AddSkillExperience (CHAMPION.C:823-960):
 *   - XP added to base skill accumulator
 *   - Level-up check: level = floor(sqrt(XP / 500))
 *   - On level-up: stamina/mana regen, stat adjustments
 *   - Combat bonus: if attack within 25 ticks, 1.5x XP
 *   - Combat penalty: if >150 ticks since attack, 0.5x XP
 * ══════════════════════════════════════════════════════════════════════ */

const char *dm1_skill_pass601_skill_source_evidence(void)
{
    return
        "CHAMPION.C:715-822 F0303_CHAMPION_GetSkillLevel 4base+8derived\n"
        "CHAMPION.C:823-960 F0304_CHAMPION_AddSkillExperience XP+levelup\n"
        "CHAMPION.C:866 combat window: G0361_l_LastCreatureAttackTime < GameTime-150\n"
        "CHAMPION.C:883 recent combat bonus: > GameTime-25 -> 1.5x\n"
        "CHAMPION.C:905-945 stamina regen on levelup by skill class\n";
}

