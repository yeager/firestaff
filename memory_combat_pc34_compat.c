/*
 * Combat / damage resolver data layer for ReDMCSB PC 3.4 — Phase 13 of M10.
 *
 * Pure resolvers for champion<->creature melee, damage application
 * primitives, deterministic RNG, and a timeline-event builder bridging
 * into the phase 12 queue.
 *
 * See PHASE13_PLAN.md for the authoritative scope + algorithm spec.
 *
 * Design rules this file honours:
 *   - NO globals, NO hidden state — every call takes (inputs, out) only.
 *   - NO IO, NO UI, NO sound hooks.
 *   - Caller supplies snapshots and an RngState_Compat; nothing is
 *     mutated except out-params.
 *   - MEDIA016 / PC LSB-first serialisation for every struct.
 *
 * Fontanel branches with no reachable runtime state (luck rolls,
 * skill-experience awards, party shields beyond partyShieldDefense,
 * magical resistances for fire/magic/psychic) are intentionally
 * stubbed and flagged with "NEEDS DISASSEMBLY REVIEW" markers below.
 */

#include <string.h>

#include "memory_combat_pc34_compat.h"

/* ==========================================================
 *  Internal helpers: LE int32 serialisation (same pattern as
 *  memory_timeline_pc34_compat.c).
 * ========================================================== */

static void write_i32_le(unsigned char* p, int value) {
    unsigned int u = (unsigned int)value;
    p[0] = (unsigned char)(u & 0xFF);
    p[1] = (unsigned char)((u >> 8) & 0xFF);
    p[2] = (unsigned char)((u >> 16) & 0xFF);
    p[3] = (unsigned char)((u >> 24) & 0xFF);
}

static int read_i32_le(const unsigned char* p) {
    unsigned int u =
        ((unsigned int)p[0]) |
        ((unsigned int)p[1] << 8) |
        ((unsigned int)p[2] << 16) |
        ((unsigned int)p[3] << 24);
    return (int)u;
}

/* ==========================================================
 *  Static lookup tables (mirrors of Fontanel CHAMPION.C globals).
 * ========================================================== */

/* Mirror of G0050_auc_Graphic562_WoundDefenseFactor. */
static const unsigned char WoundDefenseFactor[6] = {
    0x15, 0x10, 0x1A, 0x1A, 0x12, 0x12
};

/* Mirror of G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask. */
static const unsigned short WoundProbabilityIndexToWoundMask[4] = {
    COMBAT_WOUND_HEAD,   /* 0x0002 */
    COMBAT_WOUND_LEGS,   /* 0x0010 */
    COMBAT_WOUND_TORSO,  /* 0x0004 */
    COMBAT_WOUND_FEET    /* 0x0020 */
};

/* AttackSize_ToExplosionAttack — referenced by the PLAN for the future
 * explosion/smoke follow-up event (we populate followupEventAux0 only). */
static const unsigned char AttackSize_ToExplosionAttack[3] = {
    110, 190, 255
};

/* ==========================================================
 *  Group A — RNG (F0730–F0732).
 *
 *  Pure 32-bit LCG. See PHASE13_PLAN.md §4.6 + Risk R7: we do not
 *  claim bit-for-bit agreement with Borland rand(); determinism
 *  relative to our own seed is the contract.
 * ========================================================== */

int F0730_COMBAT_RngInit_Compat(
    struct RngState_Compat* rng,
    uint32_t seed)
{
    if (rng == 0) return 0;
    rng->seed = seed;
    return 1;
}

uint32_t F0731_COMBAT_RngNextRaw_Compat(
    struct RngState_Compat* rng)
{
    if (rng == 0) return 0;
    rng->seed = rng->seed * 1103515245u + 12345u;
    return rng->seed;
}

int F0732_COMBAT_RngRandom_Compat(
    struct RngState_Compat* rng,
    int modulus)
{
    uint32_t raw;
    uint32_t shifted;
    if (rng == 0) return 0;
    if (modulus <= 0) return 0;          /* NOTE: does not advance state (Invariant 16). */
    raw = F0731_COMBAT_RngNextRaw_Compat(rng);
    shifted = (raw >> 16) & 0x7FFF;
    return (int)(shifted % (uint32_t)modulus);
}

/* ==========================================================
 *  Group B — Defence helpers (F0733–F0734).
 * ========================================================== */

int F0733_COMBAT_GetChampionWoundDefense_Compat(
    const struct CombatantChampionSnapshot_Compat* champ,
    int woundSlotIndex,
    int useSharpDefense,
    int* outDefense)
{
    int baseline;
    int adjusted;
    if (champ == 0 || outDefense == 0) return 0;
    if (woundSlotIndex < 0 || woundSlotIndex > 5) return 0;

    /* Baseline: caller-precomputed armour/shield contribution for that slot. */
    baseline = champ->woundDefense[woundSlotIndex];

    /* Vitality bonus (mirror of F0313 tail): ActionDefense + ShieldDefense +
     * Party.ShieldDefense are folded into partyShieldDefense at snapshot time.
     * The original also adds M002_RANDOM((vit>>3)+1); in the snapshot path we
     * bake the expected value (vit>>4) into a deterministic add so callers of
     * F0733 get a stable defence number without consuming the rng. The
     * resolver F0736 re-introduces stochasticity at the Attack roll instead. */
    adjusted = baseline + (champ->partyShieldDefense);
    adjusted = adjusted + ((WoundDefenseFactor[woundSlotIndex] * champ->statisticVitality) >> 8);

    if (useSharpDefense) {
        adjusted = adjusted + (WoundDefenseFactor[woundSlotIndex] >> 1);
    }

    *outDefense = adjusted;
    return 1;
}

int F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
    int statisticCurrent,
    int statisticMaximum,
    int attack,
    int* outAdjusted)
{
    int factor;
    int result;
    if (outAdjusted == 0) return 0;

    /* Mirror of F0307 (CHAMPION.C:1106) — minus the Megamax-compiler bug
     * (BUG0_41) so antifire/antimagic actually participate. */
    factor = 170 - statisticCurrent;
    if (factor < 16) {
        result = attack >> 3;
    } else {
        /* F0030_MAIN_GetScaledProduct(attack, 7, factor) expands to
         * (attack * factor) / (1 << 7). */
        result = (attack * factor) >> 7;
    }

    /* statisticMaximum is passed for future clamping; unused in v1 — silence
     * -Wunused-parameter without introducing warnings. */
    (void)statisticMaximum;

    *outAdjusted = result;
    return 1;
}

/* ==========================================================
 *  Internal helper for F0736 — defender-statistic adjustment
 *  per attack type. Mirror of the C0..C7 switch in CHAMPION.C:1824–1910.
 *  v1 supports C0 (normal), C2 (self), C3 (blunt), C4 (sharp),
 *  C7 (lightning) in full; C1 (fire), C5 (magic), C6 (psychic) fall
 *  through to "no adjustment" and are flagged for phase 14 review.
 * ========================================================== */

static int combat_apply_defender_statistic_adjustment(
    int attackType,
    const struct CombatantChampionSnapshot_Compat* defender,
    int attack)
{
    int adjusted = attack;
    int tmp;

    switch (attackType) {
        case COMBAT_ATTACK_NORMAL:
            /* No statistic adjustment. */
            break;

        case COMBAT_ATTACK_SELF:
            /* Self-inflicted (e.g. trap damage) — halved vs vitality. */
            if (F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
                    defender->statisticVitality, 255, adjusted, &tmp)) {
                adjusted = tmp;
            }
            break;

        case COMBAT_ATTACK_BLUNT:
        case COMBAT_ATTACK_SHARP:
            /* Physical — vitality tempers it. */
            if (F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
                    defender->statisticVitality, 255, adjusted, &tmp)) {
                adjusted = tmp;
            }
            break;

        case COMBAT_ATTACK_LIGHTNING:
            /* Antimagic scales lightning damage down, per CHAMPION.C:1878. */
            if (F0734_COMBAT_GetStatisticAdjustedAttack_Compat(
                    defender->statisticAntimagic, 255, adjusted, &tmp)) {
                adjusted = tmp;
            }
            break;

        case COMBAT_ATTACK_FIRE:
        case COMBAT_ATTACK_MAGIC:
        case COMBAT_ATTACK_PSYCHIC:
            /* NEEDS DISASSEMBLY REVIEW: fire/magic/psychic defence paths
             * depend on SpellShieldDefense / FireShieldDefense globals that
             * are not modelled in phase 13. Phase 14 magic will wire these
             * through. For v1 we leave `adjusted` unchanged and DO NOT
             * fabricate. Goldens do not exercise these attack types. */
            break;

        default:
            /* Unknown attack type: pass through. */
            break;
    }

    return adjusted;
}

/* ==========================================================
 *  Group C — Resolvers (F0735 champion→creature, F0736 creature→champion)
 * ========================================================== */

int F0735_COMBAT_ResolveChampionMelee_Compat(
    const struct CombatantChampionSnapshot_Compat* attacker,
    const struct WeaponProfile_Compat* weapon,
    const struct CombatantCreatureSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out)
{
    int doubledMapDifficulty;
    int nonMaterial;
    int actionHitsNonMat;
    int rand1;
    int rand2;
    int dexThreshold;
    int dexOk;
    int rand2IsZero;
    int baseDamage;
    int bonus;
    int defense;
    int damage0;
    int r;
    int delta;
    int d1;
    int d2;
    int skillRoll;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->outcome = COMBAT_OUTCOME_MISS;
    out->creatureSlotRemoved = -1;
    out->followupEventKind = TIMELINE_EVENT_CREATURE_TICK;

    if (attacker == 0 || weapon == 0 || defender == 0 || rng == 0) return 0;
    if (attacker->championIndex < 0 || attacker->championIndex >= CHAMPION_MAX_PARTY) return 1;
    if (attacker->currentHealth <= 0) return 1;
    if (defender->creatureType < 0 || defender->creatureType > DUNGEON_CREATURE_TYPE_MAX) return 1;

    doubledMapDifficulty = defender->doubledMapDifficulty;
    nonMaterial = (defender->attributes >> 6) & 1;        /* MASK0x0040_NON_MATERIAL */
    actionHitsNonMat = (weapon->hitProbability >> 15) & 1; /* MASK0x8000_HIT_NON_MATERIAL_CREATURES */

    /* Dexterity duel (PROJEXPL.C:1439–1445). */
    rand1 = F0732_COMBAT_RngRandom_Compat(rng, 32);
    out->rngCallCount++;
    dexThreshold = rand1 + defender->dexterity + doubledMapDifficulty - 16;
    dexOk = (attacker->dexterity > dexThreshold);

    rand2 = F0732_COMBAT_RngRandom_Compat(rng, 4);
    out->rngCallCount++;
    rand2IsZero = (rand2 == 0);

    /* NEEDS DISASSEMBLY REVIEW: F0308_CHAMPION_IsLucky is hidden state in the
     * original (Luck statistic + cursed-items exploit, CHAMPION.C:1130). v1
     * collapses luck to 0. Determinism relative to our own rng is preserved. */

    if ((!nonMaterial || actionHitsNonMat) && (dexOk || rand2IsZero)) {
        out->hitLanded = 1;
        out->rawAttackRoll = rand1;

        baseDamage = attacker->strengthActionHand;
        if (baseDamage == 0) {
            /* BUG0_81 weak-branch entry: uninitialised `damage` in original.
             * We deterministically set damage0 to 0 and walk the recovery arm. */
            damage0 = 0;
            goto weak_branch;
        }
        bonus = F0732_COMBAT_RngRandom_Compat(rng, (baseDamage >> 1) + 1);
        out->rngCallCount++;
        baseDamage += bonus;
        baseDamage = (baseDamage * weapon->damageFactor) >> 5;

        defense = F0732_COMBAT_RngRandom_Compat(rng, 32) + defender->defense + doubledMapDifficulty;
        out->rngCallCount++;
        if (weapon->kineticEnergy /* (unused in defence branch but documented) */ == -0x7FFFFFFF) {
            /* unreachable, silences -Wunused while preserving the field presence */
        }
        if (attacker->actionHandIcon == COMBAT_ICON_DIAMOND_EDGE) {
            defense -= defense >> 2;
        } else if (attacker->actionHandIcon == COMBAT_ICON_HARDCLEAVE_EXECUTIONER) {
            defense -= defense >> 3;
        }
        out->defenseRoll = defense;

        damage0 = F0732_COMBAT_RngRandom_Compat(rng, 32) + baseDamage - defense;
        out->rngCallCount++;

        if (damage0 <= 1) {
        weak_branch:
            r = F0732_COMBAT_RngRandom_Compat(rng, 4);
            out->rngCallCount++;
            if (r == 0) {
                out->damageApplied = 0;
                out->outcome = COMBAT_OUTCOME_HIT_NO_DAMAGE;
                goto done;
            }
            baseDamage = r + 1;
            delta = F0732_COMBAT_RngRandom_Compat(rng, 16);
            out->rngCallCount++;
            damage0 += delta;
            if (damage0 > 0 || F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
                if (damage0 <= 0) {
                    out->rngCallCount++;
                }
                baseDamage += F0732_COMBAT_RngRandom_Compat(rng, 4);
                out->rngCallCount++;
                if (F0732_COMBAT_RngRandom_Compat(rng, 4) == 0) {
                    out->rngCallCount++;
                    delta = F0732_COMBAT_RngRandom_Compat(rng, 16);
                    out->rngCallCount++;
                    if (damage0 + delta > 0) {
                        baseDamage += damage0 + delta;
                    }
                    out->wasCritical = 1;
                } else {
                    out->rngCallCount++;
                }
            }
        } else {
            baseDamage = damage0;
        }

        baseDamage >>= 1;
        d1 = F0732_COMBAT_RngRandom_Compat(rng, baseDamage > 0 ? baseDamage : 1);
        out->rngCallCount++;
        d2 = F0732_COMBAT_RngRandom_Compat(rng, 4);
        out->rngCallCount++;
        baseDamage += d1 + d2;

        d1 = F0732_COMBAT_RngRandom_Compat(rng, baseDamage > 0 ? baseDamage : 1);
        out->rngCallCount++;
        baseDamage += d1;

        baseDamage >>= 2;
        baseDamage += F0732_COMBAT_RngRandom_Compat(rng, 4) + 1;
        out->rngCallCount++;

        if (attacker->actionHandIcon == COMBAT_ICON_VORPAL_BLADE && !nonMaterial) {
            baseDamage >>= 1;
            if (baseDamage == 0) {
                out->damageApplied = 0;
                out->outcome = COMBAT_OUTCOME_HIT_NO_DAMAGE;
                goto done;
            }
        }

        skillRoll = F0732_COMBAT_RngRandom_Compat(rng, 64);
        out->rngCallCount++;
        if (skillRoll < attacker->skillLevelAction) {
            baseDamage = baseDamage + baseDamage + 10;
        }

        if (baseDamage < 0) baseDamage = 0;
        out->damageApplied = baseDamage;
        out->outcome = (baseDamage > 0) ? COMBAT_OUTCOME_HIT_DAMAGE
                                        : COMBAT_OUTCOME_HIT_NO_DAMAGE;
    }

done:
    /* Explosion/smoke follow-up populates aux0 for future phase 14 use. */
    out->followupEventAux0 = AttackSize_ToExplosionAttack[0];
    return 1;
}

int F0736_COMBAT_ResolveCreatureMelee_Compat(
    const struct CombatantCreatureSnapshot_Compat* attacker,
    const struct CombatantChampionSnapshot_Compat* defender,
    struct RngState_Compat* rng,
    struct CombatResult_Compat* out)
{
    int rand1;
    int rand2;
    int dexFails;
    uint32_t woundTest;
    int probs;
    int idx;
    int atk;
    int rnd16;
    int r2;
    int add1;
    int add2;

    if (out == 0) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->outcome = COMBAT_OUTCOME_MISS;
    out->creatureSlotRemoved = -1;
    out->followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;

    if (attacker == 0 || defender == 0 || rng == 0) return 0;
    if (defender->currentHealth <= 0) return 1;
    if (defender->championIndex < 0 || defender->championIndex >= CHAMPION_MAX_PARTY) return 1;

    if (defender->isResting) {
        out->wakeFromRest = 1;
        /* NEEDS DISASSEMBLY REVIEW: Fontanel calls F0314_CHAMPION_WakeUp
         * and *then* continues the attack. We flag and continue. */
    }

    /* Dexterity duel — mirror of PROJEXPL.C:1354 (MEDIA064 path). */
    rand1 = F0732_COMBAT_RngRandom_Compat(rng, 32);
    out->rngCallCount++;
    out->rawAttackRoll = rand1;

    rand2 = F0732_COMBAT_RngRandom_Compat(rng, 4);
    out->rngCallCount++;

    dexFails = (defender->dexterity < (rand1 + attacker->dexterity + attacker->doubledMapDifficulty - 16))
            || (rand2 == 0);

    if (defender->isResting || dexFails) {
        /* Wound mask roll (PROJEXPL.C:1372). */
        woundTest = (uint32_t)F0732_COMBAT_RngRandom_Compat(rng, 32768);
        out->rngCallCount++;
        woundTest = (woundTest << 1) | ((uint32_t)(F0732_COMBAT_RngRandom_Compat(rng, 2)) & 1u);
        out->rngCallCount++;

        if (woundTest & 0x0070u) {
            probs = attacker->woundProbabilities;
            idx = 0;
            woundTest &= 0x000Fu;
            while ((int)woundTest > (probs & 0x000F)) {
                probs >>= 4;
                idx++;
                if (idx > 3) break;
            }
            if (idx > 3) idx = 3;
            out->woundMaskAdded = WoundProbabilityIndexToWoundMask[idx];
        } else {
            out->woundMaskAdded = (int)(woundTest & 0x0001u);  /* READY_HAND */
        }

        /* Attack value (PROJEXPL.C:1386). */
        rnd16 = F0732_COMBAT_RngRandom_Compat(rng, 16);
        out->rngCallCount++;
        atk = (rnd16 + attacker->attack + attacker->doubledMapDifficulty)
            - (defender->skillLevelParry << 1);

        if (atk <= 1) {
            r2 = F0732_COMBAT_RngRandom_Compat(rng, 2);
            out->rngCallCount++;
            if (r2 != 0) {
                /* Miss branch — matches goto T0230014 path. */
                out->outcome = COMBAT_OUTCOME_MISS;
                out->woundMaskAdded = 0;
                return 1;
            }
            atk = F0732_COMBAT_RngRandom_Compat(rng, 4) + 2;
            out->rngCallCount++;
        }
        atk >>= 1;
        add1 = F0732_COMBAT_RngRandom_Compat(rng, atk > 0 ? atk : 1);
        out->rngCallCount++;
        add2 = F0732_COMBAT_RngRandom_Compat(rng, 4);
        out->rngCallCount++;
        atk += add1 + add2;
        add1 = F0732_COMBAT_RngRandom_Compat(rng, atk > 0 ? atk : 1);
        out->rngCallCount++;
        atk += add1;
        atk >>= 2;
        atk += F0732_COMBAT_RngRandom_Compat(rng, 4) + 1;
        out->rngCallCount++;

        /* Statistic adjustment by attack type. */
        atk = combat_apply_defender_statistic_adjustment(
            attacker->attackType, defender, atk);

        if (atk <= 0) {
            out->outcome = COMBAT_OUTCOME_MISS;
            out->woundMaskAdded = 0;
            return 1;
        }

        out->hitLanded = 1;
        out->damageApplied = atk;
        out->defenseRoll = defender->skillLevelParry << 1;
        out->outcome = COMBAT_OUTCOME_HIT_DAMAGE;

        /* Poison follow-up trigger — flag only, caller handles. */
        if (attacker->poisonAttack != 0) {
            if (F0732_COMBAT_RngRandom_Compat(rng, 2) != 0) {
                out->rngCallCount++;
                /* NEEDS DISASSEMBLY REVIEW: Fontanel runs the poison value
                 * through F0307 vs vitality *before* committing to it. We
                 * emit the raw poisonAttack + the vitality so the caller
                 * can apply F0734 itself. */
                out->poisonAttackPending = attacker->poisonAttack;
            } else {
                out->rngCallCount++;
            }
        }
    }
    return 1;
}

/* ==========================================================
 *  Group D — Application (F0737–F0738)
 * ========================================================== */

int F0737_COMBAT_ApplyDamageToChampion_Compat(
    const struct CombatResult_Compat* result,
    struct ChampionState_Compat* champ,
    int* outWasKilled)
{
    int newHp;
    if (result == 0 || champ == 0 || outWasKilled == 0) return 0;

    if (champ->hp.current == 0) {
        /* Already dead — combat ignores dead targets (CHAMPION.C:1814). */
        *outWasKilled = 1;
        return 1;
    }

    newHp = (int)champ->hp.current - result->damageApplied;
    if (newHp <= 0) {
        champ->hp.current = 0;
        *outWasKilled = 1;
    } else {
        champ->hp.current = (unsigned short)newHp;
        *outWasKilled = 0;
    }
    champ->wounds |= (unsigned short)result->woundMaskAdded;
    return 1;
}

int F0738_COMBAT_ApplyDamageToGroup_Compat(
    const struct CombatResult_Compat* result,
    struct DungeonGroup_Compat* group,
    int creatureIndex,
    int* outOutcome)
{
    int slotHp;
    int damage;
    if (result == 0 || group == 0 || outOutcome == 0) return 0;
    if (creatureIndex < 0 || creatureIndex > 3) return 0;

    slotHp = (int)group->health[creatureIndex];
    damage = result->damageApplied;

    if (slotHp > damage) {
        group->health[creatureIndex] = (unsigned short)(slotHp - damage);
        *outOutcome = COMBAT_OUTCOME_KILLED_NO_CREATURES;
    } else {
        group->health[creatureIndex] = 0;
        if (group->count == 0) {
            *outOutcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
        } else {
            group->count = (unsigned char)((int)group->count - 1);
            *outOutcome = COMBAT_OUTCOME_KILLED_SOME_CREATURES;
        }
        /* NEEDS DISASSEMBLY REVIEW: cell / direction packing reshuffle on
         * kill — deferred to phase 14 (tangled with ACTIVE_GROUP state,
         * not yet modelled). v1 leaves group->cells and group->direction
         * untouched; the visual consequence is acceptable since we are
         * not rendering. HP math matches Fontanel. */
    }
    return 1;
}

/* ==========================================================
 *  Group E — Timeline bridge (F0739)
 * ========================================================== */

int F0739_COMBAT_BuildTimelineEvent_Compat(
    const struct CombatAction_Compat* action,
    const struct CombatResult_Compat* result,
    uint32_t nowTick,
    struct TimelineEvent_Compat* outEvent)
{
    if (action == 0 || result == 0 || outEvent == 0) return 0;

    memset(outEvent, 0, sizeof(*outEvent));

    if (result->followupEventKind == TIMELINE_EVENT_INVALID) {
        outEvent->kind = TIMELINE_EVENT_INVALID;
        return 0;
    }

    outEvent->kind = result->followupEventKind;
    outEvent->fireAtTick = nowTick + (uint32_t)action->scheduleDelayTicks;
    outEvent->mapIndex = action->targetMapIndex;
    outEvent->mapX = action->targetMapX;
    outEvent->mapY = action->targetMapY;
    outEvent->cell = action->targetCell;
    outEvent->aux0 = result->followupEventAux0;
    outEvent->aux1 = action->attackerSlotOrCreatureIndex;
    outEvent->aux2 = action->defenderSlotOrCreatureIndex;
    outEvent->aux3 = result->damageApplied;
    outEvent->aux4 = result->outcome;
    return 1;
}

/* ==========================================================
 *  Group F — Serialisation (F0740–F0747)
 * ========================================================== */

int F0740_COMBAT_ActionSerialize_Compat(
    const struct CombatAction_Compat* action,
    unsigned char* outBuf,
    int outBufSize)
{
    if (action == 0 || outBuf == 0) return 0;
    if (outBufSize < COMBAT_ACTION_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, action->kind);
    write_i32_le(outBuf +  4, action->allowedWounds);
    write_i32_le(outBuf +  8, action->attackTypeCode);
    write_i32_le(outBuf + 12, action->rawAttackValue);
    write_i32_le(outBuf + 16, action->targetMapIndex);
    write_i32_le(outBuf + 20, action->targetMapX);
    write_i32_le(outBuf + 24, action->targetMapY);
    write_i32_le(outBuf + 28, action->targetCell);
    write_i32_le(outBuf + 32, action->attackerSlotOrCreatureIndex);
    write_i32_le(outBuf + 36, action->defenderSlotOrCreatureIndex);
    write_i32_le(outBuf + 40, action->scheduleDelayTicks);
    write_i32_le(outBuf + 44, action->flags);
    return 1;
}

int F0741_COMBAT_ActionDeserialize_Compat(
    struct CombatAction_Compat* action,
    const unsigned char* buf,
    int bufSize)
{
    if (action == 0 || buf == 0) return 0;
    if (bufSize < COMBAT_ACTION_SERIALIZED_SIZE) return 0;

    action->kind                        = read_i32_le(buf +  0);
    action->allowedWounds               = read_i32_le(buf +  4);
    action->attackTypeCode              = read_i32_le(buf +  8);
    action->rawAttackValue              = read_i32_le(buf + 12);
    action->targetMapIndex              = read_i32_le(buf + 16);
    action->targetMapX                  = read_i32_le(buf + 20);
    action->targetMapY                  = read_i32_le(buf + 24);
    action->targetCell                  = read_i32_le(buf + 28);
    action->attackerSlotOrCreatureIndex = read_i32_le(buf + 32);
    action->defenderSlotOrCreatureIndex = read_i32_le(buf + 36);
    action->scheduleDelayTicks          = read_i32_le(buf + 40);
    action->flags                       = read_i32_le(buf + 44);
    return 1;
}

int F0742_COMBAT_ResultSerialize_Compat(
    const struct CombatResult_Compat* result,
    unsigned char* outBuf,
    int outBufSize)
{
    if (result == 0 || outBuf == 0) return 0;
    if (outBufSize < COMBAT_RESULT_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, result->outcome);
    write_i32_le(outBuf +  4, result->damageApplied);
    write_i32_le(outBuf +  8, result->rawAttackRoll);
    write_i32_le(outBuf + 12, result->defenseRoll);
    write_i32_le(outBuf + 16, result->hitLanded);
    write_i32_le(outBuf + 20, result->wasCritical);
    write_i32_le(outBuf + 24, result->woundMaskAdded);
    write_i32_le(outBuf + 28, result->poisonAttackPending);
    write_i32_le(outBuf + 32, result->targetKilled);
    write_i32_le(outBuf + 36, result->creatureSlotRemoved);
    write_i32_le(outBuf + 40, result->followupEventKind);
    write_i32_le(outBuf + 44, result->followupEventAux0);
    write_i32_le(outBuf + 48, result->rngCallCount);
    write_i32_le(outBuf + 52, result->wakeFromRest);
    return 1;
}

int F0743_COMBAT_ResultDeserialize_Compat(
    struct CombatResult_Compat* result,
    const unsigned char* buf,
    int bufSize)
{
    if (result == 0 || buf == 0) return 0;
    if (bufSize < COMBAT_RESULT_SERIALIZED_SIZE) return 0;

    result->outcome               = read_i32_le(buf +  0);
    result->damageApplied         = read_i32_le(buf +  4);
    result->rawAttackRoll         = read_i32_le(buf +  8);
    result->defenseRoll           = read_i32_le(buf + 12);
    result->hitLanded             = read_i32_le(buf + 16);
    result->wasCritical           = read_i32_le(buf + 20);
    result->woundMaskAdded        = read_i32_le(buf + 24);
    result->poisonAttackPending   = read_i32_le(buf + 28);
    result->targetKilled          = read_i32_le(buf + 32);
    result->creatureSlotRemoved   = read_i32_le(buf + 36);
    result->followupEventKind     = read_i32_le(buf + 40);
    result->followupEventAux0     = read_i32_le(buf + 44);
    result->rngCallCount          = read_i32_le(buf + 48);
    result->wakeFromRest          = read_i32_le(buf + 52);
    return 1;
}

int F0744_COMBAT_ChampionSnapshotSerialize_Compat(
    const struct CombatantChampionSnapshot_Compat* champ,
    unsigned char* outBuf,
    int outBufSize)
{
    int i;
    if (champ == 0 || outBuf == 0) return 0;
    if (outBufSize < COMBATANT_CHAMPION_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, champ->championIndex);
    write_i32_le(outBuf +  4, champ->currentHealth);
    write_i32_le(outBuf +  8, champ->dexterity);
    write_i32_le(outBuf + 12, champ->strengthActionHand);
    write_i32_le(outBuf + 16, champ->skillLevelParry);
    write_i32_le(outBuf + 20, champ->skillLevelAction);
    write_i32_le(outBuf + 24, champ->statisticVitality);
    write_i32_le(outBuf + 28, champ->statisticAntifire);
    write_i32_le(outBuf + 32, champ->statisticAntimagic);
    write_i32_le(outBuf + 36, champ->actionHandIcon);
    write_i32_le(outBuf + 40, champ->wounds);
    for (i = 0; i < 6; i++) {
        write_i32_le(outBuf + 44 + (i * 4), champ->woundDefense[i]);
    }
    write_i32_le(outBuf + 68, champ->isResting);
    write_i32_le(outBuf + 72, champ->partyShieldDefense);
    return 1;
}

int F0745_COMBAT_ChampionSnapshotDeserialize_Compat(
    struct CombatantChampionSnapshot_Compat* champ,
    const unsigned char* buf,
    int bufSize)
{
    int i;
    if (champ == 0 || buf == 0) return 0;
    if (bufSize < COMBATANT_CHAMPION_SERIALIZED_SIZE) return 0;

    champ->championIndex      = read_i32_le(buf +  0);
    champ->currentHealth      = read_i32_le(buf +  4);
    champ->dexterity          = read_i32_le(buf +  8);
    champ->strengthActionHand = read_i32_le(buf + 12);
    champ->skillLevelParry    = read_i32_le(buf + 16);
    champ->skillLevelAction   = read_i32_le(buf + 20);
    champ->statisticVitality  = read_i32_le(buf + 24);
    champ->statisticAntifire  = read_i32_le(buf + 28);
    champ->statisticAntimagic = read_i32_le(buf + 32);
    champ->actionHandIcon     = read_i32_le(buf + 36);
    champ->wounds             = read_i32_le(buf + 40);
    for (i = 0; i < 6; i++) {
        champ->woundDefense[i] = read_i32_le(buf + 44 + (i * 4));
    }
    champ->isResting          = read_i32_le(buf + 68);
    champ->partyShieldDefense = read_i32_le(buf + 72);
    return 1;
}

int F0746_COMBAT_CreatureSnapshotSerialize_Compat(
    const struct CombatantCreatureSnapshot_Compat* creature,
    unsigned char* outBuf,
    int outBufSize)
{
    if (creature == 0 || outBuf == 0) return 0;
    if (outBufSize < COMBATANT_CREATURE_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, creature->creatureType);
    write_i32_le(outBuf +  4, creature->attack);
    write_i32_le(outBuf +  8, creature->defense);
    write_i32_le(outBuf + 12, creature->dexterity);
    write_i32_le(outBuf + 16, creature->baseHealth);
    write_i32_le(outBuf + 20, creature->poisonAttack);
    write_i32_le(outBuf + 24, creature->attackType);
    write_i32_le(outBuf + 28, creature->attributes);
    write_i32_le(outBuf + 32, creature->woundProbabilities);
    write_i32_le(outBuf + 36, creature->properties);
    write_i32_le(outBuf + 40, creature->doubledMapDifficulty);
    write_i32_le(outBuf + 44, creature->creatureIndex);
    write_i32_le(outBuf + 48, creature->healthBefore);
    return 1;
}

int F0747_COMBAT_CreatureSnapshotDeserialize_Compat(
    struct CombatantCreatureSnapshot_Compat* creature,
    const unsigned char* buf,
    int bufSize)
{
    if (creature == 0 || buf == 0) return 0;
    if (bufSize < COMBATANT_CREATURE_SERIALIZED_SIZE) return 0;

    creature->creatureType         = read_i32_le(buf +  0);
    creature->attack               = read_i32_le(buf +  4);
    creature->defense              = read_i32_le(buf +  8);
    creature->dexterity            = read_i32_le(buf + 12);
    creature->baseHealth           = read_i32_le(buf + 16);
    creature->poisonAttack         = read_i32_le(buf + 20);
    creature->attackType           = read_i32_le(buf + 24);
    creature->attributes           = read_i32_le(buf + 28);
    creature->woundProbabilities   = read_i32_le(buf + 32);
    creature->properties           = read_i32_le(buf + 36);
    creature->doubledMapDifficulty = read_i32_le(buf + 40);
    creature->creatureIndex        = read_i32_le(buf + 44);
    creature->healthBefore         = read_i32_le(buf + 48);
    return 1;
}

int F0747a_COMBAT_WeaponProfileSerialize_Compat(
    const struct WeaponProfile_Compat* weapon,
    unsigned char* outBuf,
    int outBufSize)
{
    if (weapon == 0 || outBuf == 0) return 0;
    if (outBufSize < WEAPON_PROFILE_SERIALIZED_SIZE) return 0;

    write_i32_le(outBuf +  0, weapon->weaponType);
    write_i32_le(outBuf +  4, weapon->weaponClass);
    write_i32_le(outBuf +  8, weapon->weaponStrength);
    write_i32_le(outBuf + 12, weapon->kineticEnergy);
    write_i32_le(outBuf + 16, weapon->hitProbability);
    write_i32_le(outBuf + 20, weapon->damageFactor);
    write_i32_le(outBuf + 24, weapon->skillIndex);
    write_i32_le(outBuf + 28, weapon->attributes);
    return 1;
}

int F0747b_COMBAT_WeaponProfileDeserialize_Compat(
    struct WeaponProfile_Compat* weapon,
    const unsigned char* buf,
    int bufSize)
{
    if (weapon == 0 || buf == 0) return 0;
    if (bufSize < WEAPON_PROFILE_SERIALIZED_SIZE) return 0;

    weapon->weaponType     = read_i32_le(buf +  0);
    weapon->weaponClass    = read_i32_le(buf +  4);
    weapon->weaponStrength = read_i32_le(buf +  8);
    weapon->kineticEnergy  = read_i32_le(buf + 12);
    weapon->hitProbability = read_i32_le(buf + 16);
    weapon->damageFactor   = read_i32_le(buf + 20);
    weapon->skillIndex     = read_i32_le(buf + 24);
    weapon->attributes     = read_i32_le(buf + 28);
    return 1;
}
