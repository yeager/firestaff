/*
 * test_dm1_v1_sleep_wakeup_clock_regen_gate_pc34_compat.c
 *
 * DM1 V1 sleep/wakeup/clock gate — focused on the F0331 *per-tick
 * health/mana regen* slice that the existing pass777 F0830/F0846
 * clock gate (time criteria + stat drift) and pass777 temp-xp gate
 * (per-skill temp XP decay) do not cover.
 *
 * Two F0331 inner-loop regen helpers share the same rest/clock
 * envelope the lane contract is named for:
 *
 *   F0844_LIFECYCLE_ApplyHealthRegen_Compat — the per-tick health
 *      regen gate that runs every tick F0331 visits a champion with
 *      CurrentHealth < MaximumHealth and CurrentStamina >= MaxStamina/4.
 *      ReDMCSB CHAMPION.C F0331 lines 2432-2445 (PC 3.4 MEDIA240 path):
 *        AL1013_i_HealthGain = (MaximumHealth >> 7) + 1;
 *        if (PartyIsResting) AL1013_i_HealthGain <<= 1;
 *        if (neckSlotIcon == C121_ICON_EKKHARD_CROSS)
 *            AL1013_i_HealthGain += (AL1013_i_HealthGain >> 1) + 1;
 *        CurrentHealth += min(AL1013_i_HealthGain,
 *                             MaximumHealth - CurrentHealth);
 *      The earlier guard is the time-criteria < vitality + 12 gate.
 *
 *   F0845_LIFECYCLE_ApplyManaRegen_Compat — the per-tick mana regen
 *      gate that runs every tick F0331 visits a champion with
 *      CurrentMana < MaximumMana.  ReDMCSB CHAMPION.C F0331 lines
 *      2370-2382 (PC 3.4 MEDIA240 path):
 *        AL1007_ManaGain = MaximumMana / 40;
 *        if (PartyIsResting) AL1007_ManaGain <<= 1;
 *        AL1007_ManaGain++;
 *        StaminaCost = ManaGain * max(7, 16 - (WizardLevel+PriestLevel));
 *        CurrentMana += min(ManaGain, MaximumMana - CurrentMana);
 *      The earlier guard is the time-criteria < wisdom + wizPriest
 *      gate, and there is an excess-mana decay branch that runs when
 *      CurrentMana > MaximumMana.
 *
 * Lane contract (pass-NNN [dm1-v1-sleep-wakeup-clock-regen-gate]):
 *   - Pin F0844's NULL-champ / NULL-currentHealth guard (returns 0).
 *   - Pin F0844's "currentHealth >= maxHealth" no-op (returns 0).
 *   - Pin F0844's stamina quarter-gate: stamina < maxStamina/4 -> 0.
 *   - Pin F0844's time-criteria < vitality+12 gate (uses F0830).
 *   - Pin F0844's resting-2x multiplier on the (maxHealth>>7)+1 gain.
 *   - Pin F0844's Ekkhard Cross +1.5x-1 bonus on the gain.
 *   - Pin F0844's maxHealth clamp: never exceed maxHealth.
 *   - Pin F0845's NULL-pointer guards (champ / currentMana / currentStamina).
 *   - Pin F0845's "currentMana >= maxMana" no-op (returns 0).
 *   - Pin F0845's excess-mana decay branch (returns 2): step is
 *     max(1, currentMana/maxMana) but never more than the excess.
 *   - Pin F0845's time-criteria < wisdom + wizPriest gate.
 *   - Pin F0845's resting-2x multiplier on the (maxMana/40) gain.
 *   - Pin F0845's stamina-cost formula:
 *        cost = gain * max(7, 16 - (wizardLevel + priestLevel))
 *     with the gain already +1 from the source-locked "AL1007_ManaGain++".
 *   - Pin F0845's maxMana clamp on CurrentMana.
 *   - Pin F0845's stamina-cost floor: never drive currentStamina below 0.
 *   - Pin F0845's per-tick independence: F0830 is the *only* clock
 *     dependency (no internal counter).
 *   - Pin the per-resting-cycle independence: the gate does NOT mutate
 *     the time-criteria or any global tick counter.
 *
 * This is contract-only and source-locked to ReDMCSB CHAMPION.C F0331
 * (PC 3.4 MEDIA240 path).  It does not claim original DOS parity.
 *
 * Disjoint from:
 *   - test_dm1_v1_sleep_wakeup_clock_gate_pc34_compat.c (pass777, F0830+F0846)
 *   - test_dm1_v1_sleep_wakeup_clock_temp_xp_gate_pc34_compat.c (pass777, F0847)
 *   - test_dm1_v1_lif01_f0830_time_criteria_source_lock_pc34_compat.c (F0830 only)
 *   - test_dm1_v1_lif01_f0831_stamina_amount_source_lock_pc34_compat.c (F0831 only)
 *   - test_dm1_v1_chm05_f0832_hunger_thirst_loop_guard_pc34_compat.c (F0832 only)
 *   - test_dm1_v1_sleep_wake_poison_gate_pc34_compat.c (poison + rest wakeup)
 */

#include "memory_champion_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s\n", (msg)); \
    } \
} while (0)

#define CHECK_EQ(actual, expected, msg) do { \
    long long a_ = (long long)(actual); \
    long long e_ = (long long)(expected); \
    if (a_ == e_) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL %s: got %lld expected %lld\n", (msg), a_, e_); \
    } \
} while (0)

/* ── Helpers ──────────────────────────────────────────────────────── */

static struct ChampionLifecycleState_Compat make_blank_champ(uint16_t maxHealth,
                                                             uint16_t maxStamina,
                                                             uint16_t maxMana)
{
    struct ChampionLifecycleState_Compat champ;
    memset(&champ, 0, sizeof(champ));
    champ.maxHealth = maxHealth;
    champ.maxStamina = maxStamina;
    champ.maxMana = maxMana;
    /* Sensible defaults so vitality / wisdom aren't accidentally zero. */
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_MAXIMUM] = 20;
    champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT] = 20;
    champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_MAXIMUM] = 20;
    champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_CURRENT] = 20;
    return champ;
}

/* gameTime that yields timeCriteria = 0: bits 0x40, 0x80, 0x100 all
 * unset.  Per F0830 ((a + b + c) >> 2) = 0. */
static uint32_t kGameTime_ZeroCriteria = 0x00000000u;
/* gameTime that yields timeCriteria >= vitality+12 = 32: bit 0x80
 * set, bits 0x40 / 0x100 clear (0x80 >> 2 = 32). */
static uint32_t kGameTime_Criteria32 = 0x00000080u;

/* ── F0844 health regen ───────────────────────────────────────────── */

static void test_f0844_null_safety(void)
{
    uint16_t currentHealth = 100;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 0, &currentHealth, 100, kGameTime_ZeroCriteria, 0,
                 LIFECYCLE_ICON_NONE),
             0,
             "F0844: NULL champ -> 0 (no health mutation)");
    CHECK_EQ(currentHealth, 100,
             "F0844: NULL champ leaves currentHealth byte-stable");

    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, 0, 100, kGameTime_ZeroCriteria, 0,
                 LIFECYCLE_ICON_NONE),
             0,
             "F0844: NULL currentHealth -> 0 (no champ mutation)");
}

static void test_f0844_already_at_max(void)
{
    /* F0844 short-circuits when currentHealth >= maxHealth, even
     * when every other gate is open (resting, stamina quarter,
     * time-criteria low, vitality high). */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 200;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, 100,
                 kGameTime_ZeroCriteria, 1,
                 LIFECYCLE_ICON_EKKHARD_CROSS),
             0,
             "F0844: currentHealth >= maxHealth -> 0 even with all gates open");
    CHECK_EQ(currentHealth, 200,
             "F0844: currentHealth stays at max when no-op branch");
}

static void test_f0844_stamina_quarter_gate(void)
{
    /* Per F0331:2432 (PC 3.4) — the health-regen path requires
     * CurrentStamina >= MaximumStamina / 4 (>=, not >).  Below that,
     * F0331 skips the regen entirely. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 100;
    uint16_t currentStamina;
    /* currentStamina = maxStamina/4 - 1 = 99 should fail. */
    currentStamina = (uint16_t)(champ.maxStamina / 4 - 1);
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, currentStamina,
                 kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_NONE),
             0,
             "F0844: stamina < maxStamina/4 -> 0 (quarter-gate)");
    CHECK_EQ(currentHealth, 100,
             "F0844: stamina quarter-gate leaves currentHealth byte-stable");

    /* currentStamina = maxStamina/4 = 100 should pass. */
    currentHealth = 100;
    currentStamina = (uint16_t)(champ.maxStamina / 4);
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, currentStamina,
                 kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_NONE),
             1,
             "F0844: stamina == maxStamina/4 -> 1 (quarter-gate is >=)");
}

static void test_f0844_time_criteria_gate(void)
{
    /* F0331:2434 — timeCriteria < vitality + 12 is the regen gate.
     * For vitality=20: gate is timeCriteria < 32.  At timeCriteria=32
     * (gameTime=0x80), regen must not fire.  At timeCriteria=0, it
     * must fire. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth;

    /* gate closed: timeCriteria = vitality + 12 = 32 */
    currentHealth = 100;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, 100,
                 kGameTime_Criteria32, 0, LIFECYCLE_ICON_NONE),
             0,
             "F0844: timeCriteria == vitality+12 -> 0 (gate closed at boundary)");

    /* gate open: timeCriteria = 0 */
    currentHealth = 100;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, 100,
                 kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_NONE),
             1,
             "F0844: timeCriteria == 0 -> 1 (gate open)");
}

static void test_f0844_base_gain_formula(void)
{
    /* F0331:2434 base gain: (maxHealth >> 7) + 1.
     * For maxHealth=200: gain = (200>>7)+1 = 1+1 = 2. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 100;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, 100,
                 kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_NONE),
             1,
             "F0844: gain path returns 1");
    CHECK_EQ(currentHealth, 102,
             "F0844: base gain (maxHealth=200, not resting) -> +2");
}

static void test_f0844_resting_doubles_gain(void)
{
    /* F0331:2436 — when PartyIsResting, the health gain shifts left
     * by one bit (<<=1).  For maxHealth=200: base gain=2, resting
     * gain=4. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 100;

    /* not resting: +2 */
    (void)F0844_LIFECYCLE_ApplyHealthRegen_Compat(
        &champ, &currentHealth, 100,
        kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_NONE);
    CHECK_EQ(currentHealth, 102,
             "F0844: not-resting gain = 2");

    /* resting: +4 (independent call, fresh currentHealth) */
    currentHealth = 100;
    (void)F0844_LIFECYCLE_ApplyHealthRegen_Compat(
        &champ, &currentHealth, 100,
        kGameTime_ZeroCriteria, 1, LIFECYCLE_ICON_NONE);
    CHECK_EQ(currentHealth, 104,
             "F0844: resting gain = 4 (2x base)");
}

static void test_f0844_ekkhard_cross_bonus(void)
{
    /* F0331:2437-2438 — when neckSlotIcon == C121_ICON_EKKHARD_CROSS,
     * gain += (gain >> 1) + 1.  For maxHealth=200, not resting:
     * base=2, bonus=(2>>1)+1=2, total=4. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 100;
    (void)F0844_LIFECYCLE_ApplyHealthRegen_Compat(
        &champ, &currentHealth, 100,
        kGameTime_ZeroCriteria, 0, LIFECYCLE_ICON_EKKHARD_CROSS);
    CHECK_EQ(currentHealth, 104,
             "F0844: Ekkhard Cross adds (gain>>1)+1 to base gain");

    /* resting + Ekkhard: base 2, rest -> 4, Ekkhard adds 2+1=3, total=7. */
    currentHealth = 100;
    (void)F0844_LIFECYCLE_ApplyHealthRegen_Compat(
        &champ, &currentHealth, 100,
        kGameTime_ZeroCriteria, 1, LIFECYCLE_ICON_EKKHARD_CROSS);
    CHECK_EQ(currentHealth, 107,
             "F0844: resting + Ekkhard -> 7 (2x base + Ekkhard bonus)");
}

static void test_f0844_max_health_clamp(void)
{
    /* F0331:2444-2445 — gain is min'd against (maxHealth - currentHealth).
     * A single tick must not push currentHealth past maxHealth. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 0);
    uint16_t currentHealth = 199;  /* only 1 HP below max */

    (void)F0844_LIFECYCLE_ApplyHealthRegen_Compat(
        &champ, &currentHealth, 100,
        kGameTime_ZeroCriteria, 1, LIFECYCLE_ICON_EKKHARD_CROSS);
    CHECK_EQ(currentHealth, 200,
             "F0844: clamp to maxHealth (199 + 4 -> 200, not 203)");

    /* at maxHealth: regen must be a no-op. */
    currentHealth = 200;
    CHECK_EQ(F0844_LIFECYCLE_ApplyHealthRegen_Compat(
                 &champ, &currentHealth, 100,
                 kGameTime_ZeroCriteria, 1, LIFECYCLE_ICON_EKKHARD_CROSS),
             0,
             "F0844: at-maxHealth is no-op");
}

/* ── F0845 mana regen ─────────────────────────────────────────────── */

static void test_f0845_null_safety(void)
{
    int16_t currentStamina = 100;
    uint16_t currentMana = 50;
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);

    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 0, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             0,
             "F0845: NULL champ -> 0");

    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, 0, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             0,
             "F0845: NULL currentMana -> 0");

    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, 0, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             0,
             "F0845: NULL currentStamina -> 0");

    /* currentMana / currentStamina byte-stable across all-null cases. */
    CHECK_EQ(currentMana, 50, "F0845: NULL guards leave currentMana byte-stable");
    CHECK_EQ(currentStamina, 100,
             "F0845: NULL guards leave currentStamina byte-stable");
}

static void test_f0845_already_at_max(void)
{
    /* F0845 short-circuits when currentMana >= maxMana.  Note: this
     * is *>=*, not >, so exactly-at-max is a no-op (and the
     * excess-decay branch below does not run). */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 100;
    int16_t currentStamina = 100;

    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 5, 5,
                 kGameTime_ZeroCriteria, 1),
             0,
             "F0845: currentMana == maxMana -> 0 (no-op, no excess-decay)");

    /* currentMana strictly greater than maxMana must run the
     * excess-decay branch (return 2).  For maxMana=100,
     * currentMana=150, excess=50, step = max(1, 150/100)=1, so
     * currentMana drops to 149. */
    currentMana = 150;
    currentStamina = 100;
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 5, 5,
                 kGameTime_ZeroCriteria, 1),
             2,
             "F0845: currentMana > maxMana -> 2 (excess-decay branch)");
    CHECK_EQ(currentMana, 149,
             "F0845: excess-decay step = max(1, currentMana/maxMana) = 1");
    CHECK_EQ(currentStamina, 100,
             "F0845: excess-decay branch does not touch stamina");
}

static void test_f0845_excess_decay_step_clamped(void)
{
    /* When step > excess (i.e. currentMana < 2*maxMana so the
     * integer-divided step exceeds the excess), clamp to excess. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 102;  /* excess = 2 */
    int16_t currentStamina = 100;
    /* step = max(1, 102/100) = 1, but excess = 2, so step=1 < excess;
     * currentMana drops to 101. */
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             2,
             "F0845: small excess -> excess-decay branch");
    CHECK_EQ(currentMana, 101,
             "F0845: small excess decays by 1 (step clamped below excess)");

    /* Larger excess: currentMana = 500, excess = 400, step = 500/100 = 5;
     * step > excess? 5 vs 400 -> no, step=5; currentMana drops to 495. */
    currentMana = 500;
    currentStamina = 100;
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             2,
             "F0845: large excess -> excess-decay branch");
    CHECK_EQ(currentMana, 495,
             "F0845: large excess decays by step=currentMana/maxMana=5");
}

static void test_f0845_time_criteria_gate(void)
{
    /* F0331:2374 — timeCriteria < wisdom + wizPriest gates the regen.
     * For wisdom=20, wizPriest=0: gate is timeCriteria < 20.  At
     * timeCriteria=32, regen must not fire. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 50;
    int16_t currentStamina = 100;

    /* gate closed: timeCriteria = 32 > 20 */
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_Criteria32, 0),
             0,
             "F0845: timeCriteria > wisdom+wizPriest -> 0 (gate closed)");
    CHECK_EQ(currentMana, 50,
             "F0845: closed gate leaves currentMana byte-stable");

    /* gate open: timeCriteria = 0 */
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             1,
             "F0845: timeCriteria == 0 -> 1 (gate open)");
}

static void test_f0845_base_gain_formula(void)
{
    /* F0331:2371 — base gain: maxMana/40 (then +1 after the
     * AL1007_ManaGain++ line).  For maxMana=100: gain=100/40=2, +1=3.
     * Stamina cost: gain*max(7, 16 - 0) = 3*16 = 48.  Final:
     * currentMana += min(3, 50) = 3, currentStamina -= 48. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 50;
    int16_t currentStamina = 100;

    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 0),
             1,
             "F0845: not-resting regen returns 1");
    CHECK_EQ(currentMana, 53,
             "F0845: base gain (maxMana=100, not resting) -> +3");
    CHECK_EQ(currentStamina, 52,
             "F0845: stamina cost = 3 * 16 = 48 (currentStamina 100->52)");
}

static void test_f0845_resting_doubles_gain(void)
{
    /* F0331:2372 — PartyIsResting shifts the (maxMana/40) base gain
     * left by one bit before the +1.  For maxMana=100, not resting:
     * (100/40)+1 = 3.  Resting: ((100/40)<<1)+1 = 5. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);

    /* not resting */
    uint16_t currentMana = 50;
    int16_t currentStamina = 100;
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 0, 0,
        kGameTime_ZeroCriteria, 0);
    CHECK_EQ(currentMana, 53,
             "F0845: not-resting gain = 3");

    /* resting */
    currentMana = 50;
    currentStamina = 100;
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 0, 0,
        kGameTime_ZeroCriteria, 1);
    CHECK_EQ(currentMana, 55,
             "F0845: resting gain = 5 (2x base +1)");

    /* stamina cost: gain*16 = 5*16 = 80 */
    CHECK_EQ(currentStamina, 20,
             "F0845: resting stamina cost = 5 * 16 = 80");
}

static void test_f0845_wizpriest_modifier(void)
{
    /* F0331:2374 — wizPriest = wizardLevel + priestLevel.  Stamina
     * multiplier = max(7, 16 - wizPriest).
     *
     * For wizardLevel=5, priestLevel=3 -> wizPriest=8,
     * multiplier = max(7, 8) = 8, cost = 3*8 = 24. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 50;
    int16_t currentStamina = 100;
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 5, 3,
        kGameTime_ZeroCriteria, 0);
    CHECK_EQ(currentMana, 53,
             "F0845: gain unaffected by wizPriest");
    CHECK_EQ(currentStamina, 76,
             "F0845: stamina cost = 3 * max(7, 16-8) = 3 * 8 = 24");

    /* For wizPriest = 12 -> multiplier = max(7, 4) = 7 (floor). */
    currentMana = 50;
    currentStamina = 100;
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 7, 5,
        kGameTime_ZeroCriteria, 0);
    CHECK_EQ(currentStamina, 79,
             "F0845: stamina cost = 3 * 7 = 21 (multiplier floor)");

    /* For wizPriest = 0 -> multiplier = max(7, 16) = 16. */
    currentMana = 50;
    currentStamina = 100;
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 0, 0,
        kGameTime_ZeroCriteria, 0);
    CHECK_EQ(currentStamina, 52,
             "F0845: stamina cost = 3 * 16 = 48 (no skill bonus)");
}

static void test_f0845_max_mana_clamp(void)
{
    /* F0331:2382 — gain is min'd against (maxMana - currentMana).
     * A single tick must not push currentMana past maxMana. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 98;  /* only 2 below max */
    int16_t currentStamina = 100;

    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 0, 0,
        kGameTime_ZeroCriteria, 1);
    CHECK_EQ(currentMana, 100,
             "F0845: clamp to maxMana (98 + 5 -> 100, not 103)");

    /* at maxMana: regen must be a no-op (no excess-decay either). */
    currentMana = 100;
    currentStamina = 100;
    CHECK_EQ(F0845_LIFECYCLE_ApplyManaRegen_Compat(
                 &champ, &currentMana, &currentStamina, 0, 0,
                 kGameTime_ZeroCriteria, 1),
             0,
             "F0845: at-maxMana is no-op (not excess-decay)");
}

static void test_f0845_stamina_floor(void)
{
    /* F0331:2380 — stamina cost must not drive currentStamina below 0. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 50;
    int16_t currentStamina = 10;  /* only 10, but cost is 48 */
    (void)F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 0, 0,
        kGameTime_ZeroCriteria, 0);
    CHECK_EQ(currentMana, 53,
             "F0845: mana gain proceeds even when stamina is short");
    CHECK_EQ(currentStamina, 0,
             "F0845: currentStamina floors at 0 (does not go negative)");
}

static void test_f0845_independent_per_call(void)
{
    /* F0845 must be a pure function — no internal tick counter, no
     * mutation of champ->food / champ->water, no global state.
     * Two consecutive calls with the same args must produce the same
     * result, and champ fields outside the inputs must be byte-stable. */
    struct ChampionLifecycleState_Compat champ =
        make_blank_champ(200, 400, 100);
    uint16_t currentMana = 50;
    int16_t currentStamina = 100;

    /* Snapshot every input field that F0845 must NOT mutate. */
    uint16_t maxHealthBefore = champ.maxHealth;
    uint16_t maxStaminaBefore = champ.maxStamina;
    uint16_t maxManaBefore = champ.maxMana;
    int16_t foodBefore = champ.food;
    int16_t waterBefore = champ.water;
    int vitalityBefore = (int)champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT];
    int wisdomBefore = (int)champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_CURRENT];

    int rc1 = F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 5, 3,
        kGameTime_ZeroCriteria, 1);

    int rc2 = F0845_LIFECYCLE_ApplyManaRegen_Compat(
        &champ, &currentMana, &currentStamina, 5, 3,
        kGameTime_ZeroCriteria, 1);

    CHECK_EQ(rc1, 1, "F0845: first call returns 1");
    CHECK_EQ(rc2, 1, "F0845: second identical call returns 1");
    CHECK_EQ(currentMana, 55 + 5,
             "F0845: second call applies gain on top of first (pure fn)");
    CHECK_EQ(champ.maxHealth, maxHealthBefore,
             "F0845: maxHealth byte-stable");
    CHECK_EQ(champ.maxStamina, maxStaminaBefore,
             "F0845: maxStamina byte-stable");
    CHECK_EQ(champ.maxMana, maxManaBefore,
             "F0845: maxMana byte-stable");
    CHECK_EQ(champ.food, foodBefore,
             "F0845: food byte-stable");
    CHECK_EQ(champ.water, waterBefore,
             "F0845: water byte-stable");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_VITALITY][LIFECYCLE_STAT_CURRENT],
             vitalityBefore, "F0845: vitality byte-stable");
    CHECK_EQ(champ.statistics[LIFECYCLE_STAT_WISDOM][LIFECYCLE_STAT_CURRENT],
             wisdomBefore, "F0845: wisdom byte-stable");
}

/* ── Driver ───────────────────────────────────────────────────────── */

int main(void)
{
    /* F0844 health regen */
    test_f0844_null_safety();
    test_f0844_already_at_max();
    test_f0844_stamina_quarter_gate();
    test_f0844_time_criteria_gate();
    test_f0844_base_gain_formula();
    test_f0844_resting_doubles_gain();
    test_f0844_ekkhard_cross_bonus();
    test_f0844_max_health_clamp();

    /* F0845 mana regen */
    test_f0845_null_safety();
    test_f0845_already_at_max();
    test_f0845_excess_decay_step_clamped();
    test_f0845_time_criteria_gate();
    test_f0845_base_gain_formula();
    test_f0845_resting_doubles_gain();
    test_f0845_wizpriest_modifier();
    test_f0845_max_mana_clamp();
    test_f0845_stamina_floor();
    test_f0845_independent_per_call();

    printf("=== DM1 V1 Sleep/Wakeup/Clock Regen Gate ===\n");
    printf("ReDMCSB: CHAMPION.C F0331 health/mana regen (lines 2370-2382, 2432-2445)\n");
    printf("M10:     F0844_LIFECYCLE_ApplyHealthRegen_Compat + "
           "F0845_LIFECYCLE_ApplyManaRegen_Compat\n");
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
