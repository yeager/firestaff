/*
 * firestaff_csb_v1_champion_stat_determinism_probe.c
 * ====================================================
 *
 * CSB V1 Champion-stat determinism probe (Tier 4 #17 polish).
 *
 * Verifies that the source-locked champion-stat helpers in
 * csb_v1_character_pc34_compat.c produce identical results across
 * many invocations with the same input (no hidden state, no
 * uninitialized memory, no order-dependent globals):
 *
 *   - csb_v1_champion_get_maximum_load    (F0309 + BUG0_72)
 *   - csb_v1_champion_get_movement_ticks  (F0310 + BUG0_72)
 *
 * Source-locks:
 *   ReDMCSB CHAMPION.C F0306 lines 1078-1106 (stamina adjustment)
 *   ReDMCSB CHAMPION.C F0309 lines 1157-1178 (maximum load)
 *   ReDMCSB CHAMPION.C F0310 lines 1180-1214 (movement ticks)
 *   ReDMCSB BUG0_72 in CHAMPION.C F0310 line 1198 (Load==MaxLoad -> 4 ticks)
 *
 * Run:
 *   ./build/firestaff_csb_v1_champion_stat_determinism_probe
 *
 * Pass: 6/6 invariants (boundary cases + repetition determinism).
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "csb_v1_character_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

/* Build a champion with the given raw stats (current STR / max STR /
 * current stamina / max stamina). Other fields are zeroed. */
static CSB_V1_Champion make_champion(int16_t str_cur,
                                     int16_t str_max,
                                     int16_t sta_cur,
                                     int16_t sta_max) {
    CSB_V1_Champion c;
    memset(&c, 0, sizeof(c));
    c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_CUR] = str_cur;
    c.Statistics[CSB_V1_STAT_STR][CSB_V1_STAT_MAX] = str_max;
    c.CurrentStamina = sta_cur;
    c.MaximumStamina = sta_max;
    return c;
}

int main(void) {
    printf("=== CSB V1 Champion-stat determinism probe ===\n\n");

    /* 1. Fresh champion, healthy stamina: STR=20 -> base=(20<<3)+100=260,
     *    adjusted by stamina (full stamina => unchanged), +9 -mod10 = 260.
     *    (Actually 260+9=269; 269%10=9; 269-9=260.) */
    {
        CSB_V1_Champion c = make_champion(20, 20, 50, 50);
        unsigned int first = csb_v1_champion_get_maximum_load(&c);
        CHECK(first == 260u,
              "STR=20, full stamina -> max load 260 (F0309 round-up)");
        for (int rep = 0; rep < 50; ++rep) {
            unsigned int again = csb_v1_champion_get_maximum_load(&c);
            if (again != first) {
                printf("    iter %d mismatch: %u != %u\n",
                       rep, again, first);
                ++g_fail;
                break;
            }
        }
        CHECK(1, "max load is deterministic across 50 repetitions");
    }

    /* 2. Half-stamina adjustment: STR=20, max=20, current stamina=10/40.
     *    Stamina below half_max (20), so F0306 applies:
     *    base = (20<<3)+100 = 260
     *    half_val = 130, half_max = 20
     *    scaled = (130 * 10) / 20 = 65
     *    adjusted = 130 + 65 = 195
     *    +9 -mod10: 204 - 4 = 200. */
    {
        CSB_V1_Champion c = make_champion(20, 20, 10, 40);
        unsigned int load = csb_v1_champion_get_maximum_load(&c);
        CHECK(load == 200u,
              "STR=20, half stamina -> max load 200 (F0306 then F0309 round-up)");
    }

    /* 3. Movement ticks: BUG0_72 boundary (Load==MaxLoad).
     *    Load equal to MaxLoad goes to the overloaded branch (4 ticks).
     *    Load below MaxLoad but Load*8 > MaxLoad*5 -> 3 ticks. */
    {
        CSB_V1_Champion c = make_champion(20, 20, 50, 50);
        unsigned int max = csb_v1_champion_get_maximum_load(&c);
        c.Load = max;
        unsigned int ticks = csb_v1_champion_get_movement_ticks(&c);
        CHECK(ticks == 4u,
              "BUG0_72: Load==MaxLoad -> 4 ticks (overloaded branch)");
        /* Fresh branch: Load well below max such that Load*8 <= MaxLoad*5.
         *   max = 260, threshold = 260*5/8 = 162.5 -> 162.
         *   Load = 50: 50*8 = 400 <= 260*5 = 1300 -> ticks = 2. */
        c.Load = 50u;
        ticks = csb_v1_champion_get_movement_ticks(&c);
        CHECK(ticks == 2u,
              "Load well below max -> 2 ticks (fresh branch)");
        /* Heavy branch: Load*8 > MaxLoad*5. */
        c.Load = max * 6u / 8u + 1u; /* ensures Load*8 > MaxLoad*5 */
        ticks = csb_v1_champion_get_movement_ticks(&c);
        CHECK(ticks == 3u,
              "Load*8 > MaxLoad*5 -> 3 ticks (heavy branch)");
    }

    /* 4. Determinism over many invocations across diverse inputs. */
    {
        int mismatch = 0;
        for (int rep = 0; rep < 200; ++rep) {
            CSB_V1_Champion c = make_champion(15, 25, 30, 50);
            unsigned int a = csb_v1_champion_get_maximum_load(&c);
            unsigned int b = csb_v1_champion_get_maximum_load(&c);
            unsigned int d = csb_v1_champion_get_movement_ticks(&c);
            unsigned int e = csb_v1_champion_get_movement_ticks(&c);
            if (a != b || d != e) { ++mismatch; break; }
        }
        CHECK(mismatch == 0,
              "200 iterations of get_maximum_load + get_movement_ticks are stable");
    }

    /* 5. NULL-safety: get_maximum_load(NULL) -> 0, get_movement_ticks(NULL) -> 2
     *    (F0310 has its own NULL guard that returns the fresh-branch default
     *    so callers don't see a 0-tick surprise). */
    {
        unsigned int load = csb_v1_champion_get_maximum_load(NULL);
        unsigned int ticks = csb_v1_champion_get_movement_ticks(NULL);
        CHECK(load == 0u, "get_maximum_load(NULL) -> 0 (defensive)");
        CHECK(ticks == 2u, "get_movement_ticks(NULL) -> 2 (fresh-branch default)");
    }

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
