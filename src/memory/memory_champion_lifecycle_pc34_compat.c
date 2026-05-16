/*
 * Phase 18 of M10 — Champion lifecycle data layer (pure C).
 *
 * See memory_champion_lifecycle_pc34_compat.h + PHASE18_PLAN.md.
 */

#include "memory_champion_lifecycle_pc34_compat.h"

#include <string.h>
#include <limits.h>

/* ---------------- LSB-first helpers (MEDIA016 / PC) ---------------- */

static void write_u8(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v & 0xFFu);
}

static unsigned int read_u8(const unsigned char* p) {
    return (unsigned int)p[0];
}

static void write_i16_le(unsigned char* p, int16_t v) {
    uint16_t u = (uint16_t)v;
    p[0] = (unsigned char)(u & 0xFFu);
    p[1] = (unsigned char)((u >> 8) & 0xFFu);
}

static int16_t read_i16_le(const unsigned char* p) {
    uint16_t u = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
    return (int16_t)u;
}

static void write_u16_le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
}

static uint16_t read_u16_le(const unsigned char* p) {
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void write_i32_le(unsigned char* p, int32_t v) {
    uint32_t u = (uint32_t)v;
    p[0] = (unsigned char)(u & 0xFFu);
    p[1] = (unsigned char)((u >> 8) & 0xFFu);
    p[2] = (unsigned char)((u >> 16) & 0xFFu);
    p[3] = (unsigned char)((u >> 24) & 0xFFu);
}

static int32_t read_i32_le(const unsigned char* p) {
    uint32_t u = (uint32_t)p[0]
               | ((uint32_t)p[1] << 8)
               | ((uint32_t)p[2] << 16)
               | ((uint32_t)p[3] << 24);
    return (int32_t)u;
}

static void write_u32_le(unsigned char* p, uint32_t v) {
    p[0] = (unsigned char)(v & 0xFFu);
    p[1] = (unsigned char)((v >> 8) & 0xFFu);
    p[2] = (unsigned char)((v >> 16) & 0xFFu);
    p[3] = (unsigned char)((v >> 24) & 0xFFu);
}

static uint32_t read_u32_le(const unsigned char* p) {
    return (uint32_t)p[0]
         | ((uint32_t)p[1] << 8)
         | ((uint32_t)p[2] << 16)
         | ((uint32_t)p[3] << 24);
}

/* ---------------- Small math helpers ---------------- */

static int clamp_int(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

/* Bounded RNG call; delegates to Phase 13's F0732 which tolerates mod<=0. */
static int rng_next_mod(struct RngState_Compat* rng, int modulus) {
    if (rng == 0 || modulus <= 0) return 0;
    return F0732_COMBAT_RngRandom_Compat(rng, modulus);
}

/* ================================================================
 *  Group A — Hunger / thirst / stamina
 * ================================================================ */

uint16_t F0830_LIFECYCLE_ComputeTimeCriteria_Compat(uint32_t gameTime) {
    /* NEEDS DISASSEMBLY REVIEW: F0331 uses
     *   timeCriteria = ((GT & 0x80) + ((GT & 0x100) >> 2)
     *                   + ((GT & 0x40) << 2)) >> 2
     * — verbatim port; verify PC 3.4 bits vs other revisions. */
    uint32_t a = gameTime & 0x0080u;
    uint32_t b = (gameTime & 0x0100u) >> 2;
    uint32_t c = (gameTime & 0x0040u) << 2;
    return (uint16_t)(((a + b + c) >> 2) & 0xFFFFu);
}

int16_t F0831_LIFECYCLE_ComputeStaminaAmount_Compat(
    uint16_t maxStamina,
    int isResting,
    uint32_t lastMovementTime,
    uint32_t gameTime)
{
    int base = ((int)maxStamina >> 8) - 1;
    uint32_t delay;
    if (base < 1) base = 1;
    if (base > 6) base = 6;
    if (isResting) base <<= 1;

    /* Idle bonus: only meaningful when gameTime >= lastMovementTime. */
    if (gameTime >= lastMovementTime) {
        delay = gameTime - lastMovementTime;
        if (delay > (uint32_t)LIFECYCLE_IDLE_STAMINA_BONUS_1_DELAY) {
            base += 1;
            if (delay > (uint32_t)LIFECYCLE_IDLE_STAMINA_BONUS_2_DELAY) {
                base += 1;
            }
        }
    }

    if (base > INT16_MAX) base = INT16_MAX;
    return (int16_t)base;
}

int F0834_LIFECYCLE_ClampFoodWater_Compat(int16_t* food, int16_t* water) {
    if (food == 0 || water == 0) return 0;
    if (*food < LIFECYCLE_FOOD_FLOOR) *food = (int16_t)LIFECYCLE_FOOD_FLOOR;
    if (*water < LIFECYCLE_WATER_FLOOR) *water = (int16_t)LIFECYCLE_WATER_FLOOR;
    return 1;
}

int16_t F0832_LIFECYCLE_TickHungerThirst_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int16_t staminaAmount,
    int16_t currentStamina)
{
    /* Port of F0331's inner do-while, CHAMPION.C:2360..2415. */
    int staminaGainCycleCount = 4;
    int staminaMagnitude;
    int staminaLoss = 0;
    int cycles = 0;

    if (champ == 0) return 0;

    staminaMagnitude = (int)champ->maxStamina;
    /* NEEDS DISASSEMBLY REVIEW: F0331 expands gain cycles via repeated
     * halving of maxStamina vs currentStamina. Verbatim port; verify
     * termination under signed currentStamina near 0. */
    while (1) {
        staminaMagnitude >>= 1;
        if (staminaMagnitude <= 0) break;
        if ((int)currentStamina >= staminaMagnitude) break;
        staminaGainCycleCount += 2;
        if (staminaGainCycleCount >= 64) break; /* hard loop-guard */
    }

    do {
        int aboveHalf = (staminaGainCycleCount <= 4);

        /* FOOD -------------------------------------------------- */
        if (champ->food < LIFECYCLE_FOOD_WATER_STARVE_THRESHOLD) {
            if (aboveHalf) {
                staminaLoss += staminaAmount;
                champ->food -= 2;
            }
        } else {
            if (champ->food >= 0) {
                staminaLoss -= staminaAmount;
            }
            if (aboveHalf) {
                champ->food -= 2;
            } else {
                champ->food -= (int16_t)(staminaGainCycleCount >> 1);
            }
        }

        /* WATER -------------------------------------------------- */
        if (champ->water < LIFECYCLE_FOOD_WATER_STARVE_THRESHOLD) {
            if (aboveHalf) {
                staminaLoss += staminaAmount;
                champ->water -= 1;
            }
        } else {
            if (champ->water >= 0) {
                staminaLoss -= staminaAmount;
            }
            if (aboveHalf) {
                champ->water -= 1;
            } else {
                champ->water -= (int16_t)(staminaGainCycleCount >> 2);
            }
        }

        cycles++;
        staminaGainCycleCount--;
        if (staminaGainCycleCount <= 0) break;
        if ((int)currentStamina - staminaLoss >= (int)champ->maxStamina) break;
        if (cycles > 64) break; /* loop guard */
    } while (1);

    F0834_LIFECYCLE_ClampFoodWater_Compat(&champ->food, &champ->water);

    if (staminaLoss > INT16_MAX) staminaLoss = INT16_MAX;
    if (staminaLoss < INT16_MIN) staminaLoss = INT16_MIN;
    return (int16_t)staminaLoss;
}

int F0833_LIFECYCLE_ApplyHungerThirstFull_Compat(
    struct ChampionLifecycleState_Compat* champ,
    const struct HungerThirstInput_Compat* in,
    struct HungerThirstResult_Compat* out)
{
    int16_t staminaAmount;
    int16_t staminaLoss;
    int stamina;
    int damage = 0;

    if (champ == 0 || in == 0 || out == 0) return 0;

    staminaAmount = F0831_LIFECYCLE_ComputeStaminaAmount_Compat(
        (uint16_t)in->maxStamina, in->isResting,
        in->lastMovementTime, in->gameTime);
    staminaLoss = F0832_LIFECYCLE_TickHungerThirst_Compat(
        champ, staminaAmount, in->currentStamina);

    stamina = (int)in->currentStamina - (int)staminaLoss;

    /* Stamina overflow rules (F0325 mirror): */
    if (stamina < 0) {
        damage = (-stamina) >> 1;
        stamina = 0;
    } else if (stamina > (int)in->maxStamina) {
        stamina = (int)in->maxStamina;
    }

    out->newFood = champ->food;
    out->newWater = champ->water;
    out->netStaminaChange = (int16_t)(clamp_int(
        (int)in->currentStamina - stamina, INT16_MIN, INT16_MAX));
    out->healthDamage = (int16_t)clamp_int(damage, 0, INT16_MAX);
    return 1;
}

/* ================================================================
 *  Group B — Status expiry (F0835-F0840)
 * ================================================================ */

int F0836_LIFECYCLE_HandlePoisonTick_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int poisonAttackBefore,
    uint32_t currentTick,
    int* outDamage,
    int* outNewAttack,
    struct TimelineEvent_Compat* outReschedule)
{
    int damage;
    int newAttack;
    if (champ == 0 || outDamage == 0 || outNewAttack == 0
        || outReschedule == 0) return 0;

    /* F0322: damage = max(1, attack >> 6); attack -= 1. */
    damage = poisonAttackBefore >> 6;
    if (damage < 1) damage = 1;
    newAttack = poisonAttackBefore - 1;

    *outDamage = damage;
    *outNewAttack = newAttack;

    memset(outReschedule, 0, sizeof(*outReschedule));
    outReschedule->kind = TIMELINE_EVENT_INVALID;

    if (newAttack > 0) {
        outReschedule->kind = TIMELINE_EVENT_STATUS_TIMEOUT;
        outReschedule->fireAtTick = currentTick
            + (uint32_t)LIFECYCLE_POISON_RESCHEDULE_TICKS;
        outReschedule->aux0 = LIFECYCLE_STATUS_POISON;
        outReschedule->aux1 = newAttack; /* carry attack through chain */
        return 1; /* reschedule */
    }

    /* Chain terminated — decrement the per-champion counter. */
    if (champ->poisonEventCount > 0) {
        champ->poisonEventCount--;
    }
    return 2; /* terminal tick */
}

int F0837_LIFECYCLE_HandleShieldExpiry_Compat(
    struct LifecycleState_Compat* state,
    int championIndex,
    int isChampionShield,
    int defense)
{
    if (state == 0) return 0;
    if (isChampionShield) {
        if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
        state->champions[championIndex].shieldDefense -= (int16_t)defense;
    } else {
        state->status.partyShieldDefense -= (int16_t)defense;
    }
    return 1;
}

int F0838_LIFECYCLE_HandleMagicExpiry_Compat(
    struct LifecycleState_Compat* state,
    int isFireShield,
    int defense)
{
    if (state == 0) return 0;
    if (isFireShield) {
        state->status.partyFireShieldDefense -= (int16_t)defense;
    } else {
        state->status.partySpellShieldDefense -= (int16_t)defense;
    }
    return 1;
}

int F0839_LIFECYCLE_HandleCounterExpiry_Compat(
    struct LifecycleState_Compat* state,
    int statusKind)
{
    if (state == 0) return 0;
    switch (statusKind) {
    case LIFECYCLE_STATUS_INVISIBILITY:
        if (state->status.invisibilityCount > 0)
            state->status.invisibilityCount--;
        return 1;
    case LIFECYCLE_STATUS_THIEVES_EYE:
        if (state->status.thievesEyeCount > 0)
            state->status.thievesEyeCount--;
        return 1;
    case LIFECYCLE_STATUS_FOOTPRINTS:
        if (state->status.footprintsCount > 0)
            state->status.footprintsCount--;
        return 1;
    default:
        return 0;
    }
}

int F0840_LIFECYCLE_HandleLightExpiry_Compat(
    struct LifecycleState_Compat* state)
{
    if (state == 0) return 0;
    /* v1 stub — light system is separate. We track a counter so the
     * timeline can decrement it when light events expire. */
    if (state->status.lightEventCount > 0) state->status.lightEventCount--;
    return 1;
}

int F0835_LIFECYCLE_HandleStatusExpiry_Compat(
    struct LifecycleState_Compat* state,
    const struct TimelineEvent_Compat* expired,
    int championIndex,
    struct TimelineEvent_Compat* outRescheduled)
{
    int statusKind;
    if (state == 0 || expired == 0 || outRescheduled == 0) return 0;

    memset(outRescheduled, 0, sizeof(*outRescheduled));
    outRescheduled->kind = TIMELINE_EVENT_INVALID;

    statusKind = expired->aux0;

    switch (statusKind) {
    case LIFECYCLE_STATUS_LIGHT:
        F0840_LIFECYCLE_HandleLightExpiry_Compat(state);
        return 1;
    case LIFECYCLE_STATUS_INVISIBILITY:
    case LIFECYCLE_STATUS_THIEVES_EYE:
    case LIFECYCLE_STATUS_FOOTPRINTS:
        F0839_LIFECYCLE_HandleCounterExpiry_Compat(state, statusKind);
        return 1;
    case LIFECYCLE_STATUS_CHAMPION_SHIELD:
        /* aux1 carries the Defense value assigned at cast time. */
        F0837_LIFECYCLE_HandleShieldExpiry_Compat(
            state, championIndex, /*isChampionShield=*/1, expired->aux1);
        return 1;
    case LIFECYCLE_STATUS_PARTY_SHIELD:
        F0837_LIFECYCLE_HandleShieldExpiry_Compat(
            state, championIndex, /*isChampionShield=*/0, expired->aux1);
        return 1;
    case LIFECYCLE_STATUS_SPELL_SHIELD:
        F0838_LIFECYCLE_HandleMagicExpiry_Compat(
            state, /*isFireShield=*/0, expired->aux1);
        return 1;
    case LIFECYCLE_STATUS_FIRE_SHIELD:
        F0838_LIFECYCLE_HandleMagicExpiry_Compat(
            state, /*isFireShield=*/1, expired->aux1);
        return 1;
    case LIFECYCLE_STATUS_POISON: {
        int damage = 0;
        int newAttack = 0;
        int rc;
        struct ChampionLifecycleState_Compat* champ = 0;
        if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
        champ = &state->champions[championIndex];
        rc = F0836_LIFECYCLE_HandlePoisonTick_Compat(
            champ, expired->aux1, expired->fireAtTick,
            &damage, &newAttack, outRescheduled);
        if (rc == 1) {
            outRescheduled->mapIndex = expired->mapIndex;
            outRescheduled->mapX = expired->mapX;
            outRescheduled->mapY = expired->mapY;
            outRescheduled->cell = expired->cell;
            outRescheduled->aux2 = championIndex;
            outRescheduled->aux3 = damage;
        }
        return rc > 0 ? 1 : 0;
    }
    default:
        if (statusKind >= LIFECYCLE_STATUS_MAGIC_MAP_LO
            && statusKind <= LIFECYCLE_STATUS_MAGIC_MAP_HI) {
            /* NEEDS DISASSEMBLY REVIEW: C80..C83 magic-map per-champion
             * counters (CSB). v1 stub. */
            return 1;
        }
        return 0;
    }
}

/* ================================================================
 *  Group C — Move-timer (F0841-F0843)
 * ================================================================ */

uint16_t F0841_LIFECYCLE_ComputeMoveTicks_Compat(
    uint16_t load,
    uint16_t maxLoad,
    unsigned short wounds,
    int footwearIcon)
{
    int ticks;
    int woundTicks;

    /* Port of F0310, CHAMPION.C:1180-1215. BUG0_72: comparison is `>`
     * so load==maxLoad is treated as overloaded; preserved faithfully. */
    if ((int)maxLoad > (int)load) {
        ticks = 2;
        if (((int)load << 3) > ((int)maxLoad * 5)) {
            ticks = 3;
        }
        woundTicks = 1;
    } else {
        int overload = (int)load - (int)maxLoad;
        if (maxLoad == 0) {
            ticks = 4; /* defensive: avoid div-by-zero */
        } else {
            ticks = 4 + ((overload << 2) / (int)maxLoad);
        }
        woundTicks = 2;
    }

    if (wounds & LIFECYCLE_WOUND_FEET) ticks += woundTicks;

    if (footwearIcon == LIFECYCLE_ICON_BOOT_OF_SPEED) {
        ticks -= 1;
        if (ticks < 1) ticks = 1; /* defensive floor */
    }

    if (ticks < 0) ticks = 0;
    if (ticks > 65535) ticks = 65535;
    return (uint16_t)ticks;
}

int F0842_LIFECYCLE_UpdateMoveTimer_Compat(
    struct MoveTimerState_Compat* timer,
    uint32_t currentTick,
    uint16_t moveTicks)
{
    if (timer == 0) return 0;
    timer->lastMoveTick = currentTick;
    timer->cachedMoveTicks = moveTicks;
    timer->nextAllowedTick = currentTick + (uint32_t)moveTicks;
    return 1;
}

int F0843_LIFECYCLE_CanChampionMove_Compat(
    const struct MoveTimerState_Compat* timer,
    uint32_t currentTick)
{
    if (timer == 0) return 0;
    return (currentTick >= timer->nextAllowedTick) ? 1 : 0;
}

/* ================================================================
 *  Group D — Regen (F0844-F0847)
 * ================================================================ */

int F0844_LIFECYCLE_ApplyHealthRegen_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint16_t* currentHealth,
    uint16_t currentStamina,
    uint32_t gameTime,
    int isResting,
    int neckSlotIcon)
{
    uint16_t timeCriteria;
    int vitality;
    int gain;
    int newHealth;

    if (champ == 0 || currentHealth == 0) return 0;

    if (*currentHealth >= champ->maxHealth) return 0;
    if ((int)currentStamina < ((int)champ->maxStamina >> 2)) return 0;

    timeCriteria = F0830_LIFECYCLE_ComputeTimeCriteria_Compat(gameTime);
    vitality = (int)champ->statistics[LIFECYCLE_STAT_VITALITY]
                                     [LIFECYCLE_STAT_CURRENT];
    if ((int)timeCriteria >= vitality + 12) return 0;

    gain = ((int)champ->maxHealth >> 7) + 1;
    if (isResting) gain <<= 1;
    if (neckSlotIcon == LIFECYCLE_ICON_EKKHARD_CROSS) {
        gain += (gain >> 1) + 1;
    }

    newHealth = (int)(*currentHealth) + gain;
    if (newHealth > (int)champ->maxHealth) newHealth = (int)champ->maxHealth;
    *currentHealth = (uint16_t)newHealth;
    return 1;
}

int F0845_LIFECYCLE_ApplyManaRegen_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint16_t* currentMana,
    int16_t* currentStamina,
    int wizardLevel,
    int priestLevel,
    uint32_t gameTime,
    int isResting)
{
    uint16_t timeCriteria;
    int wisdom;
    int wizPriest;
    int gain;
    int staminaCost;
    int multiplier;
    int newMana;
    int newStamina;

    if (champ == 0 || currentMana == 0 || currentStamina == 0) return 0;

    if (*currentMana > champ->maxMana) {
        int excess = (int)(*currentMana) - (int)champ->maxMana;
        int step;
        /* PC 3.4: faster decay for excess. */
        if (champ->maxMana > 0) {
            step = (int)(*currentMana) / (int)champ->maxMana;
            if (step < 1) step = 1;
        } else {
            step = 1;
        }
        if (step > excess) step = excess;
        *currentMana = (uint16_t)((int)(*currentMana) - step);
        return 2;
    }

    if (*currentMana >= champ->maxMana) return 0;

    wizPriest = wizardLevel + priestLevel;
    timeCriteria = F0830_LIFECYCLE_ComputeTimeCriteria_Compat(gameTime);
    wisdom = (int)champ->statistics[LIFECYCLE_STAT_WISDOM]
                                  [LIFECYCLE_STAT_CURRENT];
    if ((int)timeCriteria >= wisdom + wizPriest) return 0;

    gain = ((int)champ->maxMana) / 40;
    if (isResting) gain <<= 1;
    gain += 1;

    multiplier = 16 - wizPriest;
    if (multiplier < 7) multiplier = 7;
    staminaCost = gain * multiplier;

    newStamina = (int)(*currentStamina) - staminaCost;
    if (newStamina < 0) newStamina = 0;
    *currentStamina = (int16_t)newStamina;

    newMana = (int)(*currentMana) + gain;
    if (newMana > (int)champ->maxMana) newMana = (int)champ->maxMana;
    *currentMana = (uint16_t)newMana;
    return 1;
}

int F0846_LIFECYCLE_ApplyStatDrift_Compat(
    struct ChampionLifecycleState_Compat* champ,
    uint32_t gameTime,
    int isResting)
{
    int i;
    uint32_t period;
    if (champ == 0) return 0;
    period = isResting ? (uint32_t)LIFECYCLE_STAT_DRIFT_PERIOD_REST
                       : (uint32_t)LIFECYCLE_STAT_DRIFT_PERIOD_NORMAL;
    if (period == 0) return 0;
    if ((gameTime % period) != 0) return 0;

    for (i = 0; i < LIFECYCLE_STAT_COUNT; i++) {
        int maxv = (int)champ->statistics[i][LIFECYCLE_STAT_MAXIMUM];
        int curv = (int)champ->statistics[i][LIFECYCLE_STAT_CURRENT];
        if (curv < maxv) {
            curv += 1;
        } else if (curv > maxv) {
            int step;
            if (maxv > 0) {
                step = curv / maxv;
                if (step < 1) step = 1;
            } else {
                step = 1;
            }
            curv -= step;
        }
        if (curv < 0) curv = 0;
        if (curv > 255) curv = 255;
        champ->statistics[i][LIFECYCLE_STAT_CURRENT] = (uint8_t)curv;
    }
    return 1;
}

int F0847_LIFECYCLE_ApplyTemporaryXPDecay_Compat(
    struct ChampionLifecycleState_Compat* champ)
{
    int i;
    if (champ == 0) return 0;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; i++) {
        if (champ->skills20[i].temporaryExperience > 0) {
            champ->skills20[i].temporaryExperience--;
        }
    }
    return 1;
}

/* ================================================================
 *  Group E — XP & level-up (F0848-F0853)
 * ================================================================ */

int F0848_LIFECYCLE_ComputeSkillLevel_Compat(
    const struct ChampionLifecycleState_Compat* champ,
    int skillIndex,
    int ignoreTemporary)
{
    int32_t exp;
    int level = 1;

    if (champ == 0) return 1;
    if (skillIndex < 0 || skillIndex >= LIFECYCLE_SKILL_COUNT) return 1;

    exp = champ->skills20[skillIndex].experience;
    if (!ignoreTemporary) {
        exp += (int32_t)champ->skills20[skillIndex].temporaryExperience;
    }

    /* Hidden skills average with their base skill (F0303). */
    if (skillIndex >= LIFECYCLE_HIDDEN_SKILL_FIRST) {
        int baseIdx = (skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        if (baseIdx >= 0 && baseIdx < LIFECYCLE_HIDDEN_SKILL_FIRST) {
            int32_t baseExp = champ->skills20[baseIdx].experience;
            if (!ignoreTemporary) {
                baseExp += (int32_t)champ->skills20[baseIdx].temporaryExperience;
            }
            /* Average of hidden+base, to avoid double-count. */
            exp = (exp + baseExp) >> 1;
        }
    }

    if (exp < 0) exp = 0;
    while (exp >= (int32_t)LIFECYCLE_XP_LEVEL_THRESHOLD) {
        exp >>= 1;
        level += 1;
        if (level > 16) break; /* hard guard */
    }
    return level;
}

int F0849_LIFECYCLE_AddSkillExperience_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int skillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    int* outBaseLevelBefore,
    int* outBaseLevelAfter)
{
    int baseIdx;
    int tempAdd;
    int64_t expLong;
    int levelBefore;
    int levelAfter;

    if (champ == 0) return 0;
    if (skillIndex < 0 || skillIndex >= LIFECYCLE_SKILL_COUNT) return 0;

    baseIdx = (skillIndex >= LIFECYCLE_HIDDEN_SKILL_FIRST)
        ? ((skillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2)
        : skillIndex;
    if (baseIdx < 0) baseIdx = 0;
    if (baseIdx >= LIFECYCLE_HIDDEN_SKILL_FIRST) baseIdx = 0;

    /* Map difficulty multiplier (F0304 line ~835 in Fontanel). */
    expLong = (int64_t)experience;
    if (mapDifficulty > 0) expLong *= (int64_t)mapDifficulty;

    /* Staleness / freshness modifiers only for hidden combat skills. */
    if (skillIndex >= LIFECYCLE_SKILL_SWING
        && skillIndex <= LIFECYCLE_SKILL_SHOOT) {
        if (gameTime >= lastCreatureAttackTime) {
            uint32_t delay = gameTime - lastCreatureAttackTime;
            if (delay <= (uint32_t)LIFECYCLE_RECENT_COMBAT_WINDOW) {
                expLong <<= 1;
            } else if (delay >= (uint32_t)LIFECYCLE_STALE_COMBAT_WINDOW) {
                expLong >>= 1;
            }
        }
    }

    if (expLong < 0) expLong = 0;
    if (expLong > (int64_t)INT32_MAX) expLong = (int64_t)INT32_MAX;

    levelBefore = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
        champ, baseIdx, /*ignoreTemporary=*/0);

    /* Award experience to the hidden skill AND to the base skill.
     * F0304: both get the raw (post-difficulty / post-staleness) value. */
    {
        int32_t add = (int32_t)expLong;
        int32_t prevHid = champ->skills20[skillIndex].experience;
        int32_t prevBase = champ->skills20[baseIdx].experience;
        int64_t sumHid = (int64_t)prevHid + (int64_t)add;
        int64_t sumBase = (int64_t)prevBase + (int64_t)add;
        if (sumHid > (int64_t)INT32_MAX) sumHid = (int64_t)INT32_MAX;
        if (sumBase > (int64_t)INT32_MAX) sumBase = (int64_t)INT32_MAX;
        champ->skills20[skillIndex].experience = (int32_t)sumHid;
        if (skillIndex != baseIdx) {
            champ->skills20[baseIdx].experience = (int32_t)sumBase;
        }
    }

    /* Temporary XP: bounded(1, experience >> 3, 100), capped at 32000
     * on the hidden skill (where most combat XP lands). */
    tempAdd = (int)expLong >> 3;
    if (tempAdd < 1) tempAdd = 1;
    if (tempAdd > 100) tempAdd = 100;
    {
        int newTemp = (int)champ->skills20[skillIndex].temporaryExperience + tempAdd;
        if (newTemp > LIFECYCLE_TEMP_XP_CAP) newTemp = LIFECYCLE_TEMP_XP_CAP;
        if (newTemp < 0) newTemp = 0;
        if (newTemp > INT16_MAX) newTemp = INT16_MAX;
        champ->skills20[skillIndex].temporaryExperience = (int16_t)newTemp;
    }
    if (skillIndex != baseIdx) {
        int newTemp = (int)champ->skills20[baseIdx].temporaryExperience + tempAdd;
        if (newTemp > LIFECYCLE_TEMP_XP_CAP) newTemp = LIFECYCLE_TEMP_XP_CAP;
        if (newTemp < 0) newTemp = 0;
        if (newTemp > INT16_MAX) newTemp = INT16_MAX;
        champ->skills20[baseIdx].temporaryExperience = (int16_t)newTemp;
    }

    levelAfter = F0848_LIFECYCLE_ComputeSkillLevel_Compat(
        champ, baseIdx, /*ignoreTemporary=*/0);

    if (outBaseLevelBefore) *outBaseLevelBefore = levelBefore;
    if (outBaseLevelAfter)  *outBaseLevelAfter = levelAfter;
    return (levelAfter > levelBefore) ? 1 : 0;
}

int F0850_LIFECYCLE_ApplyLevelUp_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int baseSkillIndex,
    int newLevel,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker)
{
    int level = newLevel;
    int halfPlus1;
    int maxH, maxS, maxM;
    int strengthBump = 0;
    int dexterityBump = 0;
    int wisdomBump = 0;
    int vitalityBump = 0;
    int antifireBump = 0;
    int antimagicBump = 0;
    int healthBump = 0;
    int staminaBump = 0;
    int manaBump = 0;
    int oddLevel;

    if (champ == 0) return 0;
    if (baseSkillIndex < 0 || baseSkillIndex >= LIFECYCLE_HIDDEN_SKILL_FIRST)
        return 0;
    if (level < 2) return 0;

    halfPlus1 = (level >> 1) + 1;
    oddLevel = level & 1;

    maxH = (int)champ->maxHealth;
    maxS = (int)champ->maxStamina;
    maxM = (int)champ->maxMana;

    /* Class-specific bonuses (F0304). */
    switch (baseSkillIndex) {
    case LIFECYCLE_SKILL_FIGHTER:
        strengthBump = 1 + rng_next_mod(rng, 2);
        dexterityBump = rng_next_mod(rng, 2);
        staminaBump = maxS >> 4;
        healthBump = level + rng_next_mod(rng, halfPlus1);
        break;
    case LIFECYCLE_SKILL_NINJA:
        strengthBump = rng_next_mod(rng, 2);
        dexterityBump = 1 + rng_next_mod(rng, 2);
        staminaBump = maxS / 21;
        healthBump = level + rng_next_mod(rng, halfPlus1);
        break;
    case LIFECYCLE_SKILL_WIZARD:
        manaBump = level + (level >> 1)
            + ((rng_next_mod(rng, 4) < level - 1)
               ? rng_next_mod(rng, 4) : (level - 1));
        wisdomBump = 1 + rng_next_mod(rng, 2);
        staminaBump = maxS >> 5;
        healthBump = level + rng_next_mod(rng, halfPlus1);
        antimagicBump = rng_next_mod(rng, 4);
        break;
    case LIFECYCLE_SKILL_PRIEST:
        manaBump = level + ((rng_next_mod(rng, 4) < level - 1)
                            ? rng_next_mod(rng, 4) : (level - 1));
        wisdomBump = rng_next_mod(rng, 2);
        vitalityBump = rng_next_mod(rng, 2);
        staminaBump = maxS / 25;
        healthBump = level + rng_next_mod(rng, halfPlus1);
        antimagicBump = rng_next_mod(rng, 3);
        break;
    default:
        return 0;
    }

    /* Universal per-level bonuses. */
    if (baseSkillIndex != LIFECYCLE_SKILL_PRIEST && oddLevel) {
        vitalityBump += rng_next_mod(rng, 2);
    } else if (baseSkillIndex == LIFECYCLE_SKILL_PRIEST) {
        /* priest already set above */
    }
    if (!oddLevel) {
        antifireBump += rng_next_mod(rng, 2);
    }

    /* Apply to stats (maxima). */
    {
        int s;
        s = (int)champ->statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM]
             + strengthBump;
        champ->statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
        s = (int)champ->statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM]
             + dexterityBump;
        champ->statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
        s = (int)champ->statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM]
             + wisdomBump;
        champ->statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
        s = (int)champ->statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM]
             + vitalityBump;
        champ->statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
        s = (int)champ->statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_MAXIMUM]
             + antifireBump;
        champ->statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
        s = (int)champ->statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_MAXIMUM]
             + antimagicBump;
        champ->statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int(s, 0, 255);
    }

    /* Apply HP / Stamina / Mana bumps with caps. */
    maxH += healthBump;
    if (maxH > LIFECYCLE_MAX_HEALTH_CAP) maxH = LIFECYCLE_MAX_HEALTH_CAP;
    if (maxH > 65535) maxH = 65535;
    champ->maxHealth = (uint16_t)maxH;

    maxS += staminaBump;
    if (maxS > LIFECYCLE_MAX_STAMINA_CAP) maxS = LIFECYCLE_MAX_STAMINA_CAP;
    if (maxS > 65535) maxS = 65535;
    champ->maxStamina = (uint16_t)maxS;

    maxM += manaBump;
    if (maxM > LIFECYCLE_MAX_MANA_CAP) maxM = LIFECYCLE_MAX_MANA_CAP;
    if (maxM > 65535) maxM = 65535;
    champ->maxMana = (uint16_t)maxM;

    if (outMarker) {
        outMarker->championIndex = -1; /* caller fills */
        outMarker->baseSkillIndex = baseSkillIndex;
        outMarker->previousLevel = level - 1;
        outMarker->newLevel = level;
    }
    return 1;
}

int F0851_LIFECYCLE_AwardCombatXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int physicalSkillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker)
{
    int levelBefore = 0;
    int levelAfter = 0;
    int rc;

    if (champ == 0) return 0;
    if (physicalSkillIndex < LIFECYCLE_SKILL_SWING
        || physicalSkillIndex > LIFECYCLE_SKILL_SHOOT) return 0;

    rc = F0849_LIFECYCLE_AddSkillExperience_Compat(
        champ, physicalSkillIndex, experience, mapDifficulty,
        gameTime, lastCreatureAttackTime, &levelBefore, &levelAfter);

    if (rc == 1 && levelAfter > levelBefore) {
        int baseIdx = (physicalSkillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        F0850_LIFECYCLE_ApplyLevelUp_Compat(
            champ, baseIdx, levelAfter, rng, outMarker);
        if (outMarker) outMarker->championIndex = championIndex;
        return 1;
    }
    if (outMarker) {
        outMarker->championIndex = championIndex;
        outMarker->baseSkillIndex = (physicalSkillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        outMarker->previousLevel = levelBefore;
        outMarker->newLevel = levelAfter;
    }
    return 0;
}

int F0852_LIFECYCLE_AwardMagicXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int magicSkillIndex,
    int experience,
    int mapDifficulty,
    uint32_t gameTime,
    uint32_t lastCreatureAttackTime,
    struct RngState_Compat* rng,
    struct LevelUpMarker_Compat* outMarker)
{
    int levelBefore = 0;
    int levelAfter = 0;
    int rc;

    if (champ == 0) return 0;
    if (magicSkillIndex < LIFECYCLE_SKILL_IDENTIFY
        || magicSkillIndex > LIFECYCLE_SKILL_WATER) return 0;

    rc = F0849_LIFECYCLE_AddSkillExperience_Compat(
        champ, magicSkillIndex, experience, mapDifficulty,
        gameTime, lastCreatureAttackTime, &levelBefore, &levelAfter);

    if (rc == 1 && levelAfter > levelBefore) {
        int baseIdx = (magicSkillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        F0850_LIFECYCLE_ApplyLevelUp_Compat(
            champ, baseIdx, levelAfter, rng, outMarker);
        if (outMarker) outMarker->championIndex = championIndex;
        return 1;
    }
    if (outMarker) {
        outMarker->championIndex = championIndex;
        outMarker->baseSkillIndex = (magicSkillIndex - LIFECYCLE_HIDDEN_SKILL_FIRST) >> 2;
        outMarker->previousLevel = levelBefore;
        outMarker->newLevel = levelAfter;
    }
    return 0;
}

int F0853_LIFECYCLE_AwardKillXP_Compat(
    struct ChampionLifecycleState_Compat* champ,
    int championIndex,
    int creatureType,
    int* outAwarded)
{
    /* NEEDS DISASSEMBLY REVIEW: Fontanel does NOT have a separate
     * kill-XP function — XP is awarded per-hit via F0304 (F0849 here).
     * v1 returns 0 (no-op) and reports 0 awarded to prevent
     * double-counting with Phase 13 unclaimed-kill markers. */
    (void)champ;
    (void)championIndex;
    (void)creatureType;
    if (outAwarded) *outAwarded = 0;
    return 0;
}

/* ================================================================
 *  Group F — Emission helpers (F0854-F0856)
 * ================================================================ */

int F0854_LIFECYCLE_EmitTimelineEvent_Compat(
    int kind,
    uint32_t fireAtTick,
    int championIndex,
    int aux0,
    int aux1,
    struct TimelineEvent_Compat* outEvent)
{
    if (outEvent == 0) return 0;
    memset(outEvent, 0, sizeof(*outEvent));
    outEvent->kind = kind;
    outEvent->fireAtTick = fireAtTick;
    outEvent->mapIndex = -1;
    outEvent->mapX = -1;
    outEvent->mapY = -1;
    outEvent->cell = -1;
    outEvent->aux0 = aux0;
    outEvent->aux1 = aux1;
    outEvent->aux2 = championIndex;
    outEvent->aux3 = 0;
    outEvent->aux4 = 0;
    return 1;
}

int F0855_LIFECYCLE_ScheduleNextHungerThirst_Compat(
    uint32_t currentTick,
    int championIndex,
    struct TimelineEvent_Compat* outEvent)
{
    return F0854_LIFECYCLE_EmitTimelineEvent_Compat(
        TIMELINE_EVENT_HUNGER_THIRST,
        currentTick + 1u,
        championIndex,
        /*aux0=*/championIndex,
        /*aux1=*/0,
        outEvent);
}

int F0856_LIFECYCLE_BuildLevelUpMarker_Compat(
    int championIndex,
    int baseSkillIndex,
    int previousLevel,
    int newLevel,
    struct LevelUpMarker_Compat* outMarker)
{
    if (outMarker == 0) return 0;
    outMarker->championIndex = championIndex;
    outMarker->baseSkillIndex = baseSkillIndex;
    outMarker->previousLevel = previousLevel;
    outMarker->newLevel = newLevel;
    return 1;
}

/* ================================================================
 *  Group G — Serialisation (F0857-F0859)
 * ================================================================ */

static void skill_state_ser(const struct SkillState_Compat* s, unsigned char* p) {
    write_i32_le(p + 0, s->experience);
    write_i16_le(p + 4, s->temporaryExperience);
    write_u8(p + 6, s->padding[0]);
    write_u8(p + 7, s->padding[1]);
}

static void skill_state_de(struct SkillState_Compat* s, const unsigned char* p) {
    s->experience = read_i32_le(p + 0);
    s->temporaryExperience = read_i16_le(p + 4);
    s->padding[0] = (uint8_t)read_u8(p + 6);
    s->padding[1] = (uint8_t)read_u8(p + 7);
}

static void move_timer_ser(const struct MoveTimerState_Compat* m, unsigned char* p) {
    write_u32_le(p + 0, m->lastMoveTick);
    write_u32_le(p + 4, m->nextAllowedTick);
    write_u16_le(p + 8, m->cachedMoveTicks);
    write_u8(p + 10, m->padding[0]);
    write_u8(p + 11, m->padding[1]);
}

static void move_timer_de(struct MoveTimerState_Compat* m, const unsigned char* p) {
    m->lastMoveTick = read_u32_le(p + 0);
    m->nextAllowedTick = read_u32_le(p + 4);
    m->cachedMoveTicks = read_u16_le(p + 8);
    m->padding[0] = (uint8_t)read_u8(p + 10);
    m->padding[1] = (uint8_t)read_u8(p + 11);
}

static void status_ser(const struct StatusEffectState_Compat* s, unsigned char* p) {
    write_i16_le(p + 0, s->partyShieldDefense);
    write_i16_le(p + 2, s->partySpellShieldDefense);
    write_i16_le(p + 4, s->partyFireShieldDefense);
    write_u16_le(p + 6, s->invisibilityCount);
    write_u16_le(p + 8, s->thievesEyeCount);
    write_u16_le(p + 10, s->footprintsCount);
    write_u16_le(p + 12, s->lightEventCount);
    write_u8(p + 14, s->padding[0]);
    write_u8(p + 15, s->padding[1]);
}

static void status_de(struct StatusEffectState_Compat* s, const unsigned char* p) {
    s->partyShieldDefense = read_i16_le(p + 0);
    s->partySpellShieldDefense = read_i16_le(p + 2);
    s->partyFireShieldDefense = read_i16_le(p + 4);
    s->invisibilityCount = read_u16_le(p + 6);
    s->thievesEyeCount = read_u16_le(p + 8);
    s->footprintsCount = read_u16_le(p + 10);
    s->lightEventCount = read_u16_le(p + 12);
    s->padding[0] = (uint8_t)read_u8(p + 14);
    s->padding[1] = (uint8_t)read_u8(p + 15);
}

static void rest_ser(const struct RestState_Compat* r, unsigned char* p) {
    write_u8(p + 0, r->isResting);
    write_u8(p + 1, r->interruptReason);
    write_u8(p + 2, r->padding[0]);
    write_u8(p + 3, r->padding[1]);
    write_u32_le(p + 4, r->restStartTick);
    write_u32_le(p + 8, r->lastMovementTime);
}

static void rest_de(struct RestState_Compat* r, const unsigned char* p) {
    r->isResting = (uint8_t)read_u8(p + 0);
    r->interruptReason = (uint8_t)read_u8(p + 1);
    r->padding[0] = (uint8_t)read_u8(p + 2);
    r->padding[1] = (uint8_t)read_u8(p + 3);
    r->restStartTick = read_u32_le(p + 4);
    r->lastMovementTime = read_u32_le(p + 8);
}

static void champ_life_ser(
    const struct ChampionLifecycleState_Compat* c,
    unsigned char* p)
{
    int i, j;
    int off = 0;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; i++) {
        skill_state_ser(&c->skills20[i], p + off);
        off += SKILL_STATE_SERIALIZED_SIZE;
    }
    move_timer_ser(&c->moveTimer, p + off);
    off += MOVE_TIMER_STATE_SERIALIZED_SIZE;
    /* off = 172 */
    write_i16_le(p + off, c->shieldDefense); off += 2;      /* 174 */
    write_u8(p + off, c->poisonEventCount);  off += 1;      /* 175 */
    write_u8(p + off, c->padding);           off += 1;      /* 176 */
    write_i16_le(p + off, c->food);          off += 2;      /* 178 */
    write_i16_le(p + off, c->water);         off += 2;      /* 180 */
    for (i = 0; i < LIFECYCLE_STAT_COUNT; i++) {
        for (j = 0; j < 3; j++) {
            write_u8(p + off, c->statistics[i][j]);
            off += 1;
        }
    }
    /* off = 201 */
    write_u8(p + off, c->statPadding);       off += 1;      /* 202 */
    write_u16_le(p + off, c->maxHealth);     off += 2;      /* 204 */
    write_u16_le(p + off, c->maxStamina);    off += 2;      /* 206 */
    write_u16_le(p + off, c->maxMana);       off += 2;      /* 208 */
}

static void champ_life_de(
    struct ChampionLifecycleState_Compat* c,
    const unsigned char* p)
{
    int i, j;
    int off = 0;
    for (i = 0; i < LIFECYCLE_SKILL_COUNT; i++) {
        skill_state_de(&c->skills20[i], p + off);
        off += SKILL_STATE_SERIALIZED_SIZE;
    }
    move_timer_de(&c->moveTimer, p + off);
    off += MOVE_TIMER_STATE_SERIALIZED_SIZE;
    c->shieldDefense = read_i16_le(p + off); off += 2;
    c->poisonEventCount = (uint8_t)read_u8(p + off); off += 1;
    c->padding = (uint8_t)read_u8(p + off); off += 1;
    c->food = read_i16_le(p + off); off += 2;
    c->water = read_i16_le(p + off); off += 2;
    for (i = 0; i < LIFECYCLE_STAT_COUNT; i++) {
        for (j = 0; j < 3; j++) {
            c->statistics[i][j] = (uint8_t)read_u8(p + off);
            off += 1;
        }
    }
    c->statPadding = (uint8_t)read_u8(p + off); off += 1;
    c->maxHealth = read_u16_le(p + off); off += 2;
    c->maxStamina = read_u16_le(p + off); off += 2;
    c->maxMana = read_u16_le(p + off); off += 2;
}

int F0857_LIFECYCLE_Serialize_Compat(
    const struct LifecycleState_Compat* state,
    unsigned char* outBuf,
    int outBufSize)
{
    int i, off;
    if (state == 0 || outBuf == 0) return 0;
    if (outBufSize < LIFECYCLE_STATE_SERIALIZED_SIZE) return 0;

    memset(outBuf, 0, LIFECYCLE_STATE_SERIALIZED_SIZE);

    off = 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        champ_life_ser(&state->champions[i], outBuf + off);
        off += CHAMPION_LIFECYCLE_STATE_SERIALIZED_SIZE;
    }
    status_ser(&state->status, outBuf + off);
    off += STATUS_EFFECT_STATE_SERIALIZED_SIZE;
    rest_ser(&state->rest, outBuf + off);
    off += REST_STATE_SERIALIZED_SIZE;
    write_u32_le(outBuf + off, state->lastCreatureAttackTime); off += 4;
    write_u32_le(outBuf + off, state->gameTime);               off += 4;
    write_u8(outBuf + off, state->padding[0]); off += 1;
    write_u8(outBuf + off, state->padding[1]); off += 1;
    write_u8(outBuf + off, state->padding[2]); off += 1;
    write_u8(outBuf + off, state->padding[3]); off += 1;
    return 1;
}

int F0858_LIFECYCLE_Deserialize_Compat(
    struct LifecycleState_Compat* state,
    const unsigned char* buf,
    int bufSize)
{
    int i, off;
    if (state == 0 || buf == 0) return 0;
    if (bufSize < LIFECYCLE_STATE_SERIALIZED_SIZE) return 0;

    memset(state, 0, sizeof(*state));

    off = 0;
    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        champ_life_de(&state->champions[i], buf + off);
        off += CHAMPION_LIFECYCLE_STATE_SERIALIZED_SIZE;
    }
    status_de(&state->status, buf + off);
    off += STATUS_EFFECT_STATE_SERIALIZED_SIZE;
    rest_de(&state->rest, buf + off);
    off += REST_STATE_SERIALIZED_SIZE;
    state->lastCreatureAttackTime = read_u32_le(buf + off); off += 4;
    state->gameTime = read_u32_le(buf + off);               off += 4;
    state->padding[0] = (uint8_t)read_u8(buf + off); off += 1;
    state->padding[1] = (uint8_t)read_u8(buf + off); off += 1;
    state->padding[2] = (uint8_t)read_u8(buf + off); off += 1;
    state->padding[3] = (uint8_t)read_u8(buf + off); off += 1;
    return 1;
}

int F0859_LIFECYCLE_Init_Compat(
    struct LifecycleState_Compat* state,
    const struct PartyState_Compat* party)
{
    int i, j;
    if (state == 0) return 0;

    memset(state, 0, sizeof(*state));

    if (party == 0) return 1; /* zero-init is valid */

    for (i = 0; i < CHAMPION_MAX_PARTY; i++) {
        const struct ChampionState_Compat* src = &party->champions[i];
        struct ChampionLifecycleState_Compat* dst = &state->champions[i];

        /* Sign-extend Phase 10 uint8 food/water into Phase 18 int16.
         * Phase 10 range is [0..255] so direct widening preserves
         * semantics. When Phase 10 inherits a save from Fontanel, the
         * load-path responsible for sign-extension lives here. */
        dst->food  = (int16_t)src->food;
        dst->water = (int16_t)src->water;

        /* Seed 20-skill array from Phase 10's 4 base skills. */
        for (j = 0; j < CHAMPION_SKILL_COUNT; j++) {
            int32_t exp = (int32_t)src->skillExperience[j];
            if (exp < 0) exp = 0;
            dst->skills20[j].experience = exp;
        }

        /* Seed max H/S/M from Phase 10 triples. */
        dst->maxHealth = src->hp.maximum;
        dst->maxStamina = src->stamina.maximum;
        dst->maxMana = src->mana.maximum;

        /* Seed statistics rows from Phase 10 attributes (max). Current
         * equals maximum at init. */
        /* Order mapping: Phase 10 CHAMPION_ATTR_* uses STR=0,DEX=1,
         * WIS=2,VIT=3,ANTIMAGIC=4,ANTIFIRE=5. Phase 18 stat[1..6]
         * mirrors this; stat[0] (Luck) has no Phase 10 backing and
         * stays zero. */
        dst->statistics[LIFECYCLE_STAT_STRENGTH][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_STRENGTH], 0, 255);
        dst->statistics[LIFECYCLE_STAT_DEXTERITY][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_DEXTERITY], 0, 255);
        dst->statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_WISDOM], 0, 255);
        dst->statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_VITALITY], 0, 255);
        dst->statistics[LIFECYCLE_STAT_ANTIMAGIC][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_ANTIMAGIC], 0, 255);
        dst->statistics[LIFECYCLE_STAT_ANTIFIRE][LIFECYCLE_STAT_MAXIMUM]
            = (uint8_t)clamp_int((int)src->attributes[CHAMPION_ATTR_ANTIFIRE], 0, 255);
        /* Current = maximum on init. */
        for (j = 0; j < LIFECYCLE_STAT_COUNT; j++) {
            dst->statistics[j][LIFECYCLE_STAT_CURRENT]
                = dst->statistics[j][LIFECYCLE_STAT_MAXIMUM];
            dst->statistics[j][LIFECYCLE_STAT_MINIMUM] = 0;
        }
    }
    return 1;
}
