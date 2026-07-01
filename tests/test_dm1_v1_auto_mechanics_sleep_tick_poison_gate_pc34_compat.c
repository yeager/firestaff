/*
 * DM1 V1 auto-mechanics sleep/tick poison gate.
 *
 * Source-locked to the C75_EVENT_POISON_CHAMPION auto-mechanics chain
 * (ReDMCSB CHAMPION.C + TIMELINE.C). Pins the per-tick behaviour of the
 * timed poison chain, the sleep/wake entry point that creates it, and
 * the bookkeeping invariant on PoisonEventCount across the chain
 * boundary cases (chain exhaustion, stacking, dead-champion skip).
 *
 * The existing test_dm1_v1_combat_pc34_compat_integration.c covers ONE
 * chain step + one stack + one 50% gate (test_creature_poison_gate_and_
 * vitality_adjust + test_poison_start_immediate_and_followup). The
 * existing test_dm1_v1_sleep_wake_poison_gate_pc34_compat.c (currently
 * unwired in CTest) covers the F0759 wake + F0230/F0321 melee fixtures
 * but not the chain auto-mechanics. This gate is the bridge between
 * those two: it uses the F0759 poison-cloud wake-from-rest result as
 * the entry point, then drives the full timed poison chain via the v1
 * event_timer + combat layer and asserts the chain-end invariants.
 *
 * ReDMCSB source-lock references:
 *
 *   CHAMPION.C:1926-1962 (F0322_CHAMPION_Poison)
 *     - immediate damage = max(1, attack >> 6)
 *     - --attack; if --attack > 0 schedule next C75_EVENT_POISON_CHAMPION
 *       at Map_Time + 36 (CHAMPION.C:1956-1961)
 *     - else leave PoisonEventCount == 0 (CHAMPION.C:1961 early-out)
 *   CHAMPION.C:1965-1990 (F0323_CHAMPION_Unpoison)
 *     - delete every C75_EVENT_POISON_CHAMPION whose Priority == champIdx
 *     - PoisonEventCount = 0 (CHAMPION.C:1988)
 *   TIMELINE.C:1991-1993 (C75_EVENT_POISON_CHAMPION dispatch)
 *     - PoisonEventCount--, then F0322(priority, B.Attack)
 *   MAGIC.C (F0759 path) keeps wakeFromRest + poisonAttackPending on the
 *     same CombatResult so the M11/CHAMPION stack can route the chain
 *     through the auto-tick path without manual player interaction.
 *
 * Disjoint from:
 *   - test_dm1_v1_combat_pc34_compat_integration.c
 *     (one step / one stack / 50% gate only)
 *   - test_dm1_v1_sleep_wake_poison_gate_pc34_compat.c
 *     (F0759 wake + F0230/F0321 melee fixtures, no chain auto-mechanics)
 *   - test_dm1_v1_monster_poison_cloud_overlap_tick_pc34_compat.c
 *     (F0220 explosion cloud, F0191/F0192 creature damage)
 *   - test_dm1_v1_grp02_f0192_poison_resistance_source_lock_pc34_compat.c
 *     (F0192 creature poison resistance table, no timed chain)
 *   - test_dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat.c
 *     (C146 mirror sleep_repaint, not the poison chain)
 *   - test_dm1_v1_sleep_wakeup_clock_gate_pc34_compat.c +
 *     test_dm1_v1_sleep_wakeup_clock_temp_xp_gate_pc34_compat.c
 *     (clock temp/XP, not poison)
 *   - test_dm1_v1_champion_panel_box_poisoned_pc34_compat.c
 *     (G0037 PANEL.C poisoned-label rect)
 */

#include "dm1_v1_combat_pc34_compat.h"
#include "memory_combat_pc34_compat.h"
#include "memory_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s\n", msg); \
    } \
} while (0)

#define CHECK_EQ(actual, expected, msg) do { \
    int _a = (actual); \
    int _e = (expected); \
    if (_a == _e) { \
        ++g_pass; \
    } else { \
        ++g_fail; \
        fprintf(stderr, "FAIL: %s (got %d, expected %d)\n", msg, _a, _e); \
    } \
} while (0)

/*
 * ReDMCSB CHAMPION.C:1926-1962 (F0322_CHAMPION_Poison): the per-tick
 * poison auto-mechanics chain has a fixed shape:
 *
 *   immediate damage  = max(1, attack >> 6)
 *   next attack       = attack - 1
 *   reschedule at     = current tick + 36 (CHAMPION.C:1958)
 *   chain end         = when next attack == 0
 *
 * Across the chain, PoisonEventCount must remain 1 while a chain is
 * active (one scheduled C75_EVENT_POISON_CHAMPION in the timeline)
 * and must drop to 0 when the chain ends. This invariant is the
 * auto-mechanics contract that lets the M11 HUD poisoned-label
 * (PANEL.C:1601-1606 F0344) trust PoisonEventCount as a flag.
 */
static void test_chain_length_exhausts_at_attack_zero(void)
{
    DM1_CombatState s;
    int attack = 10;          /* small enough to drain in 10 ticks */
    int expectedImmediate = 1; /* max(1, attack>>6) for attack in [1..63] */
    int reschedules = 0;
    int maxTicks = 10 * 36 + 5; /* 10 cycles * 36 ticks + safety */

    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    /* F0759 entry point (poison cloud wake-from-rest): the immediate
     * effect lands on the awake champion; PoisonEventCount starts 0
     * because the resting party flag is the only thing the spell
     * result changed. The auto-mechanics chain begins here. */
    CHECK_EQ(dm1_combat_start_poison_pc34(&s, 0, attack), expectedImmediate,
             "initial F0322 immediate damage = max(1, attack>>6)");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "F0322 entry increments PoisonEventCount exactly once");
    CHECK_EQ(s.pendingPoison[0].active, 1,
             "F0322 entry leaves the chain active");
    CHECK_EQ(s.pendingPoison[0].attack, attack - 1,
             "F0322 entry schedules (attack-1) as the next poison value");
    CHECK_EQ(s.pendingPoison[0].ticksUntilNext, 36,
             "F0322 entry schedules the next event 36 ticks out (CHAMPION.C:1958)");

    /* Drive the auto-mechanics. Each F0322 call decrements pendingPoison
     * by 1 and reschedules at +36 ticks (CHAMPION.C:1958) UNLESS the
     * post-decrement attack is 0 (chain end). For attack=N the chain
     * runs exactly N F0322 calls (1 initial + N-1 from ticks); the final
     * F0322(1) lands dmg=1 with no reschedule and drops PoisonEventCount
     * to 0. Count F0322 invocations via the pendingDamage counter: every
     * F0322 call adds max(1, attack>>6) to pendingDamage. */
    for (int t = 0; t < maxTicks; ++t) {
        int before = s.pendingDamage[0];
        dm1_combat_tick_poison(&s);
        if (s.pendingDamage[0] > before) {
            reschedules++;
        }
        if (!s.pendingPoison[0].active &&
            s.champions[0].poisonEventCount == 0) {
            break;
        }
    }

    /* Apply any pending damage so the immediate damage rolls onto HP. */
    dm1_apply_pending_damage(&s);

    /* For attack=10: 1 initial F0322 + (attack-1) = 9 tick-driven F0322
     * calls total (the 9th being F0322(1) with no reschedule that ends
     * the chain). Each lands dmg=1 because attack in [1..63] gives
     * max(1, attack>>6) = 1. Total dmg = attack = 10. */
    CHECK_EQ(reschedules, attack - 1,
             "auto-mechanics chain drives exactly (attack-1) F0322 calls from ticks");
    CHECK_EQ(s.pendingDamage[0] + (100 - s.champions[0].currentHealth), attack,
             "auto-mechanics chain total immediate damage = attack");
    CHECK_EQ(s.pendingPoison[0].active, 0,
             "auto-mechanics chain ends with pendingPoison.active == 0");
    CHECK_EQ(s.pendingPoison[0].attack, 0,
             "auto-mechanics chain ends with pendingPoison.attack == 0");
    CHECK_EQ(s.pendingPoison[0].ticksUntilNext, 0,
             "auto-mechanics chain ends with pendingPoison.ticksUntilNext == 0");
    CHECK_EQ(s.champions[0].poisonEventCount, 0,
             "auto-mechanics chain ends with PoisonEventCount == 0 (CHAMPION.C:1961)");
    CHECK(s.champions[0].currentHealth < 100,
         "auto-mechanics chain actually deducts HP via dm1_apply_pending_damage");
}

/*
 * ReDMCSB CHAMPION.C:1926-1962 (F0322) stacking: a fresh poison call
 * while a chain is active must consume the existing scheduled event
 * (PoisonEventCount--), apply the new immediate damage, then either
 * reschedule with the new (attack-1) or end the chain. The bookkeeping
 * invariant: PoisonEventCount stays at 1 while a chain is active, and
 * drops to 0 only at chain end. This is the auto-mechanics contract
 * that lets F0323_CHAMPION_Unpoison and the C75 dispatch agree on
 * the count.
 */
static void test_stacking_keeps_event_count_at_one(void)
{
    DM1_CombatState s;
    int dmg;

    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    /* First poison (small attack so chain ends quickly). */
    dmg = dm1_combat_start_poison_pc34(&s, 0, 5);
    CHECK_EQ(dmg, 1, "first F0322 immediate damage");
    CHECK_EQ(s.champions[0].poisonEventCount, 1, "first F0322 increments PoisonEventCount");

    /* Second poison while the first chain is still active and a
     * rescheduled C75 is sitting in the timeline. */
    dmg = dm1_combat_start_poison_pc34(&s, 0, 8);
    CHECK_EQ(dmg, 1, "second F0322 immediate damage = max(1, 8>>6)");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "stacking keeps PoisonEventCount at 1 (consume + add net zero)");
    CHECK_EQ(s.pendingPoison[0].active, 1, "stacking leaves chain active");
    CHECK_EQ(s.pendingPoison[0].attack, 7,
             "stacking reschedules (8-1)=7 as the next poison value");

    /* The original first-chain schedule is gone: pendingPoison reflects
     * only the most recent F0322 call. This matches CHAMPION.C:1961
     * which sets p->attack = attack - 1 unconditionally on the
     * reschedule branch (F0322 always writes, it does not merge). */
    CHECK_EQ(s.pendingPoison[0].ticksUntilNext, 36,
             "stacking resets the 36-tick reschedule interval");

    /* Drive the new chain to exhaustion; the auto-mechanics must
     * produce exactly (new attack - 1) tick-driven F0322 calls. The
     * first F0322(8) immediate damage is already in pendingDamage from
     * the stacking call above, so the total pendingDamage at chain end
     * is 1 (initial F0322(5)) + 1 (stacked F0322(8)) + (new attack - 1)
     * tick-driven F0322 calls = new attack + 1. */
    {
        int reschedules = 0;
        int maxTicks = 8 * 36 + 5;
        for (int t = 0; t < maxTicks; ++t) {
            int dmgBefore = s.pendingDamage[0];
            dm1_combat_tick_poison(&s);
            if (s.pendingDamage[0] > dmgBefore) {
                reschedules++;
            }
            if (!s.pendingPoison[0].active &&
                s.champions[0].poisonEventCount == 0) {
                break;
            }
        }
        CHECK_EQ(reschedules, 7,
                 "stacked chain drives exactly (new attack - 1) tick F0322 calls");
        CHECK_EQ(s.champions[0].poisonEventCount, 0,
                 "stacked chain ends with PoisonEventCount == 0");
        /* total pendingDamage = 1 (initial F0322(5)) + 1 (stacked F0322(8))
         * + 7 (tick-driven) = 9. */
        CHECK_EQ(s.pendingDamage[0], 9,
                 "stacked chain pendingDamage = initial + stacked + tick-driven");
    }
}

/*
 * ReDMCSB CHAMPION.C:1965-1990 (F0323_CHAMPION_Unpoison) + the
 * C75_EVENT_POISON_CHAMPION dispatch at TIMELINE.C:1991-1993: when the
 * champion dies mid-chain, the auto-mechanics must drop the pending
 * poison slot without consuming a cycle or applying further damage.
 * This is the dead-champion skip that keeps the timeline from wasting
 * ticks on a corpse and matches the F0323 sweep semantics at chain
 * end.
 */
static void test_dead_champion_drops_pending_poison(void)
{
    DM1_CombatState s;
    int dmg;

    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    dmg = dm1_combat_start_poison_pc34(&s, 0, 10);
    CHECK_EQ(dmg, 1, "initial poison immediate damage");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "initial chain has one scheduled event");

    /* Kill the champion mid-chain (F0190-style: HP drops to zero). */
    s.champions[0].currentHealth = 1;
    s.pendingDamage[0] = 5;
    dm1_apply_pending_damage(&s);
    CHECK_EQ(s.champions[0].alive, 0,
             "champion killed via dm1_apply_pending_damage HP drop");
    CHECK_EQ(s.champions[0].currentHealth, 0,
             "champion HP clamped to 0 after kill");

    /* Tick the auto-mechanics: dm1_combat_tick_poison checks alive
     * before decrementing ticksUntilNext, so the slot is cleared
     * without consuming a 36-tick cycle. */
    for (int t = 0; t < 100; ++t) {
        dm1_combat_tick_poison(&s);
    }
    CHECK_EQ(s.pendingPoison[0].active, 0,
             "dead champion clears pendingPoison.active in auto-mechanics");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "dead champion skip preserves PoisonEventCount (no auto-decrement path)");
    CHECK_EQ(s.pendingPoison[0].ticksUntilNext, 36,
             "dead champion skip does not consume ticksUntilNext");
}

/*
 * F0759 entry point contract: the poison cloud spell impact must
 * set wakeFromRest + poisonAttackPending on the resting champion's
 * result, so the M11/CHAMPION layer can start the auto-mechanics
 * chain without waiting for player input. The existing
 * test_dm1_v1_sleep_wake_poison_gate_pc34_compat.c pins this exact
 * invariant; we re-state it here to lock the entry-point contract
 * for THIS lane, then run the auto-mechanics on top of the value
 * the spell result hands us.
 */
static void test_f0759_wake_then_f0322_auto_chain(void)
{
    struct SpellEffect_Compat effect;
    struct CombatantChampionSnapshot_Compat champ;
    struct MagicState_Compat magic;
    struct CombatResult_Compat spellResult;
    DM1_CombatState s;

    memset(&effect, 0, sizeof(effect));
    memset(&champ, 0, sizeof(champ));
    memset(&magic, 0, sizeof(magic));
    memset(&spellResult, 0, sizeof(spellResult));

    /* F0759 poison-cloud spell impact (spellType 7). The 64 raw
     * attack is preserved in the compat layer's damageApplied; the
     * spell also flags poisonAttackPending=3 so the caller's
     * auto-mechanics chain can pick up the chain. */
    effect.spellType = 7;
    effect.impactAttack = 64;
    effect.poisonAttackPending = 3;

    champ.championIndex = 0;
    champ.currentHealth = 120;
    champ.dexterity = 24;
    champ.isResting = 1;

    CHECK_EQ(F0759_MAGIC_ApplySpellImpactToChampion_Compat(
                 &effect, &champ, &magic, NULL, &spellResult), 1,
             "F0759 poison-cloud impact resolves on resting champion");
    CHECK_EQ(spellResult.wakeFromRest, 1,
             "F0759 poison-cloud wakeFromRest flag set on resting defender");
    CHECK_EQ(spellResult.poisonAttackPending, 3,
             "F0759 poison-cloud hands the auto-mechanics chain a pending value");
    CHECK_EQ(spellResult.hitLanded, 1,
             "F0759 poison-cloud lands as damage when unresisted");

    /* Hand the pending poison value into the auto-mechanics layer;
     * with poisonAttackPending=3 the chain length must be exactly 3. */
    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    CHECK_EQ(dm1_combat_start_poison_pc34(
                 &s, 0, spellResult.poisonAttackPending), 1,
             "F0759 -> F0322 first immediate damage");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "F0759 -> F0322 starts the auto-mechanics chain");
    CHECK_EQ(s.pendingPoison[0].attack, 2,
             "F0759 -> F0322 schedules (poisonAttackPending - 1) = 2");

    /* Drive the chain from the wake-up result: poisonAttackPending=3
     * means 1 initial F0322(3) + (3-1) tick-driven F0322 calls = 3
     * F0322 calls total, each landing dmg=1, for a chain-end
     * pendingDamage of 3. */
    {
        int reschedules = 0;
        int maxTicks = 3 * 36 + 5;
        for (int t = 0; t < maxTicks; ++t) {
            int dmgBefore = s.pendingDamage[0];
            dm1_combat_tick_poison(&s);
            if (s.pendingDamage[0] > dmgBefore) {
                reschedules++;
            }
            if (!s.pendingPoison[0].active &&
                s.champions[0].poisonEventCount == 0) {
                break;
            }
        }
        CHECK_EQ(reschedules, 2,
                 "F0759 -> F0322 chain runs exactly (poisonAttackPending-1) tick F0322 calls");
        CHECK_EQ(s.pendingDamage[0], 3,
                 "F0759 -> F0322 chain total damage = poisonAttackPending");
        CHECK_EQ(s.champions[0].poisonEventCount, 0,
                 "F0759 -> F0322 chain ends with PoisonEventCount == 0");
    }
}

/*
 * Negative input + safety boundary: dm1_combat_start_poison_pc34
 * must be a no-op for attack == 0 (CHAMPION.C:1945 T0321024 path).
 * This pins the early-out that keeps the auto-mechanics from
 * scheduling a 36-tick no-op event when the poison value has
 * already drained. It is the safety boundary between F0759's
 * poisonAttackPending==0 path and F0322's reschedule branch.
 */
static void test_zero_and_negative_attack_noop(void)
{
    DM1_CombatState s;

    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    CHECK_EQ(dm1_combat_start_poison_pc34(&s, 0, 0), 0,
             "F0322 attack=0 is a no-op (CHAMPION.C:1945 T0321024)");
    CHECK_EQ(s.champions[0].poisonEventCount, 0,
             "F0322 attack=0 leaves PoisonEventCount at 0");
    CHECK_EQ(s.pendingPoison[0].active, 0,
             "F0322 attack=0 leaves pendingPoison inactive");

    CHECK_EQ(dm1_combat_start_poison_pc34(&s, 0, -7), 0,
             "F0322 negative attack is a no-op");
    CHECK_EQ(s.champions[0].poisonEventCount, 0,
             "F0322 negative attack leaves PoisonEventCount at 0");

    /* Tick on an empty state must also be a no-op (no crash, no
     * spurious count changes). */
    dm1_combat_tick_poison(&s);
    CHECK_EQ(s.champions[0].poisonEventCount, 0,
             "tick on empty poison state is a no-op");
    CHECK_EQ(s.pendingPoison[0].active, 0,
             "tick on empty poison state leaves slot inactive");
}

/*
 * ReDMCSB CHAMPION.C:1961 boundary: when the chain is about to end,
 * the LAST dm1_combat_start_poison_pc34 call (with attack==1) must
 * still apply max(1, 1>>6) = 1 immediate damage, but the --attack
 * branch leaves attack==0 so no reschedule happens. PoisonEventCount
 * must therefore drop from 1 to 0 on that final cycle, not stay at
 * 1 forever. This pins the chain-end bookkeeping that F0323 then
 * trusts to mark the champion unpoisoned.
 */
static void test_chain_end_at_attack_one(void)
{
    DM1_CombatState s;
    int dmg;

    dm1_combat_init(&s);
    s.championCount = 1;
    dm1_combat_init_champion(&s.champions[0]);

    /* Drive the auto-mechanics so pendingPoison.attack is exactly 1. */
    dmg = dm1_combat_start_poison_pc34(&s, 0, 2);
    CHECK_EQ(dmg, 1, "F0322 attack=2 immediate damage");
    CHECK_EQ(s.pendingPoison[0].attack, 1, "F0322 attack=2 schedules attack=1");
    CHECK_EQ(s.champions[0].poisonEventCount, 1,
             "F0322 attack=2 leaves one scheduled event");

    /* Manually fire the next tick: ticksUntilNext -> 0 triggers the
     * chain-end branch which decrements PoisonEventCount and re-enters
     * F0322 with attack=1. The re-entry sees attack=1, applies
     * max(1, 1>>6) = 1 immediate damage, then --attack == 0 and no
     * reschedule. Net: PoisonEventCount returns to 0 and pendingDamage
     * holds the new 1 dmg without the previous 1 dmg being applied. */
    s.pendingPoison[0].ticksUntilNext = 1;
    dm1_combat_tick_poison(&s);

    CHECK_EQ(s.pendingPoison[0].active, 0,
             "chain-end branch clears pendingPoison.active");
    CHECK_EQ(s.champions[0].poisonEventCount, 0,
             "chain-end branch lands PoisonEventCount at 0");
    /* pendingDamage accumulates all 2 immediate damages (initial + cycle)
     * because dm1_apply_pending_damage is what flushes them to HP. */
    CHECK_EQ(s.pendingDamage[0], 2,
             "chain-end branch accumulates all immediate damages in pendingDamage");
    CHECK_EQ(s.champions[0].currentHealth, 100,
             "chain-end branch does NOT apply damage without dm1_apply_pending_damage");
    dm1_apply_pending_damage(&s);
    CHECK_EQ(s.champions[0].currentHealth, 98,
             "dm1_apply_pending_damage flushes both immediate damages");
}

int main(void)
{
    test_chain_length_exhausts_at_attack_zero();
    test_stacking_keeps_event_count_at_one();
    test_dead_champion_drops_pending_poison();
    test_f0759_wake_then_f0322_auto_chain();
    test_zero_and_negative_attack_noop();
    test_chain_end_at_attack_one();

    printf("%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
