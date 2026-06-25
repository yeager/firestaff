/*
 * firestaff_nexus_v1_light_overflow_probe.c
 * ==========================================
 *
 * Nexus V1 light-overflow data-model + bug-classification probe.
 *
 * Source-locked against the ReDMCSB WIP20210206 decompilation:
 *   - DATA.C   : 359     — G0039_ai_Graphic562_LightPowerToLightAmount
 *   - MENU.C   : 1125-1143 — F0404_MENUS_CreateEvent70_Light
 *   - MENU.C   : 1926-1942 — Light / Torch / Darkness dispatch
 *   - TIMELINE.C: 487-555  — F0238_AddEvent (BUG0_18 silent drop)
 *   - TIMELINE.C: 1720-1767 — F0257_ProcessEvent70_Light
 *   - CHAMPION.C: 27       — G0407_s_Party.MagicalLightAmount
 *
 * DMWeb evidence:
 *   - http://dmweb.free.fr/games/dungeon-master-nexus/editions/
 *     sega-saturn/ (light-overflow symptom documentation)
 *   - http://dmweb.free.fr/games/dungeon-master-nexus/solutions/
 *     cheats-and-hacks/ (BUG0_18 permanent-spell-effect exploitation)
 *
 * What this probe verifies
 * ------------------------
 *  1.  The 16-entry LightPowerToLightAmount table matches ReDMCSB
 *      DATA.C:359 byte-for-byte.
 *  2.  SpellPower-from-ordinal and the three LightPower formulas
 *      (Light / Torch / Darkness) match MENU.C:1926-1942 exactly.
 *  3.  A single Light spell with timeline headroom raises
 *      MagicalLightAmount by the documented delta and decays back
 *      to 0 through the expected number of F0257 events.
 *  4.  A single Torch spell produces the expected immediate delta.
 *  5.  A Darkness spell subtracts the documented delta, then its
 *      auto-recall chain returns MagicalLightAmount to the pre-cast
 *      value (Darkness is a temporary darkening in the original).
 *  6.  Repeated cast-then-tick cycles converge MagicalLightAmount
 *      back to 0 (no permanent light when the timeline drains).
 *  7.  Back-to-back Light casts that fill the timeline trigger the
 *      BUG0_18 silent-drop counter, and nexus_v1_light_overflow_
 *      classify() returns TIMELINE_FULL_PERMANENT_LIGHT.
 *  8.  Wrap-through-zero into a negative MagicalLightAmount is
 *      classified as LIGHT_BLEED_NEGATIVE.
 *  9.  Guard-mode timeline rejects casts once the cap is hit.
 * 10.  Determinism: the timeline state hash is stable across
 *      repeated same-input runs.
 *
 * The classification hook (TEST 7/8) is what lets the future M11
 * runtime or a debug overlay pick between emulating the original
 * behavior and guarding it.
 *
 * Run:
 *   ./build/firestaff_nexus_v1_light_overflow_probe
 * CTest:
 *   ctest --test-dir build -R nexus_v1_light_overflow --output-on-failure
 *
 * Expected: 18/18 invariants PASS on a clean build.
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nexus_v1_light_overflow.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                 \
    if (cond) { printf("  PASS: %s\n", (msg)); ++g_pass; } \
    else      { printf("  FAIL: %s\n", (msg)); ++g_fail; } \
} while (0)

/* ReDMCSB DATA.C:359 — full 16-entry table. */
static const int16_t kExpectedTable[NEXUS_V1_LIGHT_POWER_TABLE_SIZE] = {
    0, 5, 12, 24, 33, 40, 46, 51, 59, 68, 76, 82, 89, 94, 97, 100
};

/* Compute a stable 32-bit hash of the (timeline, state) pair so we
 * can prove determinism across runs. We mix active_count,
 * cast/decay/dropped counters, current_tick, and a fold over
 * in-use slots. */
static uint32_t light_state_hash(const Nexus_V1_LightTimeline *tl,
                                 const Nexus_V1_LightState *state)
{
    uint32_t h = 0x811c9dc5u;
    h ^= (uint32_t)tl->active_count;     h *= 0x01000193u;
    h ^= tl->cast_counter;                h *= 0x01000193u;
    h ^= tl->decay_counter;               h *= 0x01000193u;
    h ^= tl->dropped_counter;             h *= 0x01000193u;
    h ^= tl->current_tick;                h *= 0x01000193u;
    h ^= (uint32_t)state->magical_light_amount; h *= 0x01000193u;
    for (size_t i = 0; i < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP; ++i) {
        if (tl->slots[i].in_use) {
            h ^= (uint32_t)tl->slots[i].light_power * 0x9e3779b1u;
            h ^= tl->slots[i].fire_at_tick;
            h *= 0x01000193u;
        }
    }
    return h;
}

int main(void) {
    printf("=== Nexus V1 light-overflow data-model probe ===\n\n");

    /* ── Test 1: LightPowerToLightAmount table identity ─────────────── */
    printf("[1] LightPowerToLightAmount table (DATA.C:359)\n");
    for (int i = 0; i < NEXUS_V1_LIGHT_POWER_TABLE_SIZE; ++i) {
        CHECK(nexus_v1_light_power_to_amount[i] == kExpectedTable[i],
              "table entry matches ReDMCSB DATA.C:359");
    }
    CHECK(nexus_v1_light_amount_for_power(0) == 0,
          "amount_for_power(0) returns 0");
    CHECK(nexus_v1_light_amount_for_power(15) == 100,
          "amount_for_power(15) returns 100 (full light)");
    CHECK(nexus_v1_light_amount_for_power(20) == 100,
          "amount_for_power(>15) saturates at 100");

    /* ── Test 2: SpellPower / LightPower formulas (MENU.C:1926-1942) ── */
    printf("\n[2] SpellPower / LightPower formulas (MENU.C:1922-1942)\n");
    CHECK(nexus_v1_light_spellpower_for_ordinal(0) == 4,
          "PowerSymbol 0 (Lo) -> SpellPower 4");
    CHECK(nexus_v1_light_spellpower_for_ordinal(4) == 20,
          "PowerSymbol 4 (Mon) -> SpellPower 20");
    CHECK(nexus_v1_light_spellpower_for_ordinal(-1) == -1,
          "invalid ordinal rejected");
    CHECK(nexus_v1_light_spellpower_for_ordinal(5) == -1,
          "ordinal 5+ rejected");

    /* MENU.C:1927 — Light: LightPower = (SpellPower >> 1) - 1
     * SpellPower 4 -> 1; SpellPower 8 -> 3; SpellPower 20 -> 9. */
    CHECK(nexus_v1_light_initial_power_for_light(4) == 1,
          "Light(Lo Oh Ir Ra) SpellPower 4 -> LightPower 1");
    CHECK(nexus_v1_light_initial_power_for_light(20) == 9,
          "Light(Mon Oh Ir Ra) SpellPower 20 -> LightPower 9");

    /* MENU.C:1932 — Torch: LightPower = (SpellPower >> 2) + 1
     * SpellPower 4 -> 2; SpellPower 20 -> 6. */
    CHECK(nexus_v1_light_initial_power_for_torch(4) == 2,
          "Torch(Lo Ful) SpellPower 4 -> LightPower 2");
    CHECK(nexus_v1_light_initial_power_for_torch(20) == 6,
          "Torch(Mon Ful) SpellPower 20 -> LightPower 6");

    /* MENU.C:1940 — Darkness: LightPower = SpellPower >> 2
     * SpellPower 4 -> 1; SpellPower 20 -> 5. */
    CHECK(nexus_v1_light_initial_power_for_darkness(4) == 1,
          "Darkness SpellPower 4 -> LightPower 1");
    CHECK(nexus_v1_light_initial_power_for_darkness(20) == 5,
          "Darkness SpellPower 20 -> LightPower 5");

    /* ── Test 3: single Light spell decay chain ─────────────────────── */
    printf("\n[3] Single Light spell decay (TIMELINE.C F0257)\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);

        /* Mon Oh Ir Ra (PowerSymbol 4, SpellPower 20) -> LightPower 9.
         * Immediate delta = Table[9] = 68.
         * Decay chain: 9 -> 8 -> 7 -> ... -> 1 (8 weaker events,
         * each at current_tick+4) plus the initial fire. */
        int lp = nexus_v1_light_timeline_cast(&tl, &st,
                                              NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(lp == 9, "cast returns LightPower=9");
        CHECK(st.magical_light_amount == 68,
              "MagicalLightAmount rises to 68 after cast");
        CHECK(tl.active_count == 1, "timeline holds 1 decay event");
        CHECK(tl.cast_counter == 1, "cast_counter=1");

        /* Advance enough ticks to drain the chain (initial fire at
         * ticks=Ticks, then +4 per weaker step). Ticks for Light is
         * 10000 + (20-8)*512 = 16144. We just step 17000 ticks. */
        size_t fired = nexus_v1_light_timeline_advance(&tl, &st, 17000);
        CHECK(st.magical_light_amount == 0,
              "MagicalLightAmount returns to 0 after full decay chain");
        CHECK(fired >= 9, "decay chain fired at least 9 events");
        CHECK(tl.decay_counter >= 9, "decay_counter >= 9 (chain length)");
        CHECK(tl.active_count == 0, "timeline drained after full decay");
    }

    /* ── Test 4: single Torch spell ─────────────────────────────────── */
    printf("\n[4] Single Torch spell (MENU.C:1931)\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);

        /* Mon Ful (PowerSymbol 4, SpellPower 20) -> LightPower 6.
         * Table[6] = 46. Torch Ticks = 2000 + (20-3)*128 = 4176. */
        int lp = nexus_v1_light_timeline_cast(&tl, &st,
                                              NEXUS_LIGHT_KIND_TORCH, 4);
        CHECK(lp == 6, "cast returns LightPower=6");
        CHECK(st.magical_light_amount == 46,
              "MagicalLightAmount rises to 46 after Torch cast");
        /* Step until drained (6 weaker events). */
        nexus_v1_light_timeline_advance(&tl, &st, 5000);
        CHECK(st.magical_light_amount == 0,
              "Torch decays to 0 after full chain");
    }

    /* ── Test 5: single Darkness spell ──────────────────────────────── */
    printf("\n[5] Single Darkness spell (MENU.C:1939)\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);
        st.magical_light_amount = 50; /* pre-existing party light */

        /* Mon Darkness (PowerSymbol 4, SpellPower 20) -> LightPower 5.
         * Immediate delta = -Table[5] = -40. Ticks = 98. */
        int lp = nexus_v1_light_timeline_cast(&tl, &st,
                                              NEXUS_LIGHT_KIND_DARKNESS, 4);
        CHECK(lp == 5, "cast returns LightPower=5");
        CHECK(st.magical_light_amount == 10,
              "MagicalLightAmount drops by 40 (50 -> 10)");
        /* Advance enough ticks to drain the Darkness chain. The chain
         * fires 5 positive-LightPower events across +4 tick intervals;
         * each adds back a delta of Table[N] - Table[N-1]. The full
         * chain returns MagicalLightAmount to its pre-cast value
         * (50), because Darkness is a temporary darkening in the
         * original game, not a permanent drain. */
        nexus_v1_light_timeline_advance(&tl, &st, 200);
        CHECK(st.magical_light_amount == 50,
              "Darkness temporarily darkens (returns to pre-cast 50)");
        CHECK(tl.decay_counter >= 5,
              "Darkness chain fires at least 5 events");
    }

    /* ── Test 6: cast-tick cycle converges ──────────────────────────── */
    printf("\n[6] Cast-tick cycle converges to 0\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);

        /* Cast Lo Ful (LightPower 2), step enough ticks for decay,
         * repeat. No overflow should occur. */
        for (int i = 0; i < 5; ++i) {
            nexus_v1_light_timeline_cast(&tl, &st,
                                         NEXUS_LIGHT_KIND_TORCH, 0);
            nexus_v1_light_timeline_advance(&tl, &st, 3000);
        }
        CHECK(st.magical_light_amount == 0,
              "5 cast-then-drain cycles converge to 0");
        CHECK(tl.dropped_counter == 0,
              "no silent drops when timeline has headroom");
        CHECK(tl.cast_counter == 5 && tl.decay_counter >= 5,
              "5 casts, at least 5 decays recorded");
    }

    /* ── Test 7: BUG0_18 silent drop + permanent-light classification ─ */
    printf("\n[7] BUG0_18 silent drop + permanent-light classification\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);

        /* Cast Light at PowerSymbol 4 (LightPower 9, ticks 16144) 110
         * times back-to-back without ticking. After 100 casts the
         * timeline hits the cap and the rest are silently dropped. */
        for (int i = 0; i < 110; ++i) {
            nexus_v1_light_timeline_cast(&tl, &st,
                                         NEXUS_LIGHT_KIND_LIGHT, 4);
        }
        CHECK(tl.dropped_counter >= 10,
              "at least 10 silent drops after timeline cap");
        CHECK(nexus_v1_light_overflow_classify(&tl, &st) ==
              NEXUS_LIGHT_OVERFLOW_TIMELINE_FULL_PERMANENT_LIGHT,
              "classifier reports TIMELINE_FULL_PERMANENT_LIGHT");
        CHECK(st.magical_light_amount > 200,
              "MagicalLightAmount is elevated (cumulative immediate deltas)");
    }

    /* ── Test 8: light-bleed-through-zero classification ────────────── */
    printf("\n[8] Light-bleed-through-zero classification\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/0);
        nexus_v1_light_state_init(&st);
        st.magical_light_amount = -1; /* pre-existing overflow state */

        CHECK(nexus_v1_light_overflow_classify(&tl, &st) ==
              NEXUS_LIGHT_OVERFLOW_LIGHT_BLEED_NEGATIVE,
              "negative MagicalLightAmount reports LIGHT_BLEED_NEGATIVE");
    }

    /* ── Test 9: guard-mode rejects casts at cap ────────────────────── */
    printf("\n[9] Guard-mode rejects casts at cap\n");
    {
        Nexus_V1_LightTimeline tl;
        Nexus_V1_LightState st;
        nexus_v1_light_timeline_init(&tl, /*guard=*/1);
        nexus_v1_light_state_init(&st);

        /* Fill the buffer with non-decaying high-tick Light events.
         * Each cast schedules one initial event; the recursive
         * weaker-event chain is only created when an event actually
         * fires (we don't tick here), so the fill stays bounded. */
        int first_fill = nexus_v1_light_timeline_cast(&tl, &st,
                                                      NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(first_fill > 0, "first cast returns positive LightPower");
        int cast_count = 1;
        while (tl.active_count < NEXUS_V1_LIGHT_TIMELINE_BASE_CAP && cast_count < 256) {
            nexus_v1_light_timeline_cast(&tl, &st,
                                         NEXUS_LIGHT_KIND_LIGHT, 4);
            cast_count++;
        }
        CHECK(tl.active_count == NEXUS_V1_LIGHT_TIMELINE_BASE_CAP,
              "timeline filled to documented cap");
        int32_t light_at_cap = st.magical_light_amount;
        int r = nexus_v1_light_timeline_cast(&tl, &st,
                                             NEXUS_LIGHT_KIND_LIGHT, 4);
        CHECK(r == 0, "guard-mode rejects cast at cap (returns 0)");
        CHECK(st.magical_light_amount == light_at_cap,
              "MagicalLightAmount unchanged when guard rejects");
        CHECK(nexus_v1_light_overflow_should_guard(&tl, &st) == 1,
              "should_guard() reports 1 at cap");
    }

    /* ── Test 10: determinism across repeated runs ──────────────────── */
    printf("\n[10] Determinism (cast-then-tick hash stable)\n");
    {
        uint32_t expected = 0;
        int mismatch = 0;
        for (int rep = 0; rep < 5; ++rep) {
            Nexus_V1_LightTimeline tl;
            Nexus_V1_LightState st;
            nexus_v1_light_timeline_init(&tl, /*guard=*/0);
            nexus_v1_light_state_init(&st);
            /* Same script each run. */
            for (int i = 0; i < 8; ++i) {
                nexus_v1_light_timeline_cast(&tl, &st,
                                             NEXUS_LIGHT_KIND_TORCH, 2);
            }
            nexus_v1_light_timeline_advance(&tl, &st, 2000);
            uint32_t h = light_state_hash(&tl, &st);
            if (rep == 0) expected = h;
            else if (h != expected) { ++mismatch; }
        }
        CHECK(mismatch == 0,
              "5 runs produce the same (timeline,state) hash");
    }

    /* ── Test 11: NULL safety ───────────────────────────────────────── */
    printf("\n[11] NULL safety\n");
    CHECK(nexus_v1_light_timeline_cast(NULL, NULL,
                                       NEXUS_LIGHT_KIND_LIGHT, 4) == 0,
          "cast(NULL, NULL, ...) returns 0");
    CHECK(nexus_v1_light_timeline_tick(NULL, NULL) == 0,
          "tick(NULL, NULL) returns 0");
    CHECK(nexus_v1_light_overflow_classify(NULL, NULL) ==
          NEXUS_LIGHT_OVERFLOW_NONE,
          "classify(NULL, NULL) returns NONE");
    CHECK(nexus_v1_light_overflow_should_guard(NULL, NULL) == 0,
          "should_guard(NULL, NULL) returns 0");

    /* ── Summary ────────────────────────────────────────────────────── */
    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
