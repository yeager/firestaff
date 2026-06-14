/*
 * test_dm1_v1_f0306_stamina_pc34_compat.c
 *
 * DM1 V1 BUG-115 (F0306 stamina compiler order hazard).
 * Source-locked per ReDMCSB CHAMPION.C F0306:1078-1103 + the
 * BUGX_XX comment.  Verifies both compiler-order variants:
 *   - First-operand (Atari ST Megamax C, FM-Towns High C,
 *     THINK C 4.0).
 *   - Second-operand (PC 3.4 Turbo C++ 1.01 — Firestaff's
 *     target).
 */
#include "memory_champion_stamina_adjusted_pc34_compat.h"

#include <stdio.h>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

int main(void) {
    int result;
    printf("=== DM1 V1 BUG-115 — F0306 stamina compiler order (v2.7.15) ===\n");

    /* Default: second-operand-first (PC 3.4 Turbo C++ 1.01). */
    csb_v1_stamina_compiler_order_set(0);
    CHECK(csb_v1_stamina_compiler_order_get() == 0,
          "default order is second-operand-first (PC 3.4)");

    /* When current stamina >= half-max, return value unchanged. */
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(50, 50, 40);
    CHECK(result == 40, "stamina == half: result == value (40)");

    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(75, 50, 40);
    CHECK(result == 40, "stamina > half: result == value (40)");

    /* Below half: second-operand-first formulation.  Use
     * pre-decrement value in the second operand.
     * For value=40, curStam=45, half=50:
     *   second-operand-first: (40*45/50) + 20 = 36 + 20 = 56
     *   first-operand-first:  (20*45/50) + 20 = 18 + 20 = 38
     * The BUGX_XX comment says second-operand-first
     * compilers give a HIGHER result; the example uses
     * "approximately 29 vs 38" which we read as
     * "first-operand ~29, second-operand ~38" in different
     * test conditions.  We pin our implementation to the
     * formula, not the example. */
    csb_v1_stamina_compiler_order_set(0);
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(45, 50, 40);
    CHECK(result == 56,
          "PC 3.4 order: value=40,cur=45,half=50 -> 36+20 = 56");

    /* First-operand formulation.  Halve value, then scale. */
    csb_v1_stamina_compiler_order_set(1);
    CHECK(csb_v1_stamina_compiler_order_get() == 1,
          "order flag can be set to first-operand-first");
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(45, 50, 40);
    CHECK(result == 38,
          "Atari ST order: value=40,cur=45,half=50 -> 18+20 = 38");

    /* Edge cases: stamina = 0 */
    csb_v1_stamina_compiler_order_set(0);
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(0, 50, 40);
    CHECK(result == 20,
          "stamina=0 second-order: 0+20 = 20 (half of value)");

    csb_v1_stamina_compiler_order_set(1);
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(0, 50, 40);
    CHECK(result == 20,
          "stamina=0 first-order: also 20 (no stam scaling, half)");

    /* Guard against divide-by-zero. */
    csb_v1_stamina_compiler_order_set(0);
    result = F0306_CHAMPION_GetStaminaAdjustedValue_Compat(0, 0, 40);
    CHECK(result == 40, "half=0 guard: return value unchanged");

    /* Reset to default for downstream code. */
    csb_v1_stamina_compiler_order_set(0);

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
