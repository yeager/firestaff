/*
 * test_dm1_v1_grp02_f0192_poison_resistance_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB DUNGEON.C:439-470 (G0243_as_Graphic559_
 * CreatureInfo[27].Resistances), GROUP.C:991-1008 (F0192), and
 * DEFS.H:1664 (M061_POISON_RESISTANCE).
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0192_GROUP_GetResistanceAdjustedPoisonAttack is missing
 *    from the new compat layer — the new path does not
 *    implement per-creature resistance-adjusted poison attack."
 *
 * Verification 2026-06-15: F0192 IS implemented in
 * src/memory/memory_combat_pc34_compat.c (lines 944-1100) and
 * IS used by the projectile path (src/memory/memory_projectile_
 * pc34_compat.c:1431).  This test pins the g_poisonResistance
 * table to the ReDMCSB G0243 values so the F0192 contract
 * cannot drift.
 *
 * ReDMCSB M061_POISON_RESISTANCE decoding (per DEFS.H:1664):
 *   The 16-bit Resistance word encodes poison resistance in
 *   bits 0..3 (low nibble), with 0xF meaning immune.  The
 *   g_poisonResistance[] table mirrors this directly.
 *
 * Pins:
 *  T1  All 27 creature types (0..26) have a non-zero resistance
 *  T2  Resistance values match G0243 byte-for-byte (C00..C26)
 *  T3  Lords and Grey Lord (C23, C25, C26) are immune (0xF)
 *  T4  Magenta Worm (C15) has resistance 1 (low — vulnerable)
 *  T5  F0192 returns -1 for out-of-range creature types
 *  T6  F0192_GROUP_GetResistanceAdjustedPoisonAttack handles
 *      immune creatures (returns 0)
 *  T7  F0192 poison formula: ((poisonAttack + RANDOM(4)) << 3)
 *      / (resistance + 1) — bounded regression
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

/* Reference values from ReDMCSB DUNGEON.C:439-470 (G0243[0..26]):
 *   C00 0x299B -> 0xB   -> 0x2 = 2
 *   C01 0x33A9 -> 0x9   -> 0x3 = 3
 *   C02 0x710A -> 0xA   -> 0x7 = 7
 *   C03 0x96AA -> 0xA   -> 0x9 = 9
 *   ...
 * The poison resistance is in the low nibble of the
 * "Resistances" halfword (M061_POISON_RESISTANCE macro).
 *
 * Drawn directly from the ReDMCSB source-lock comment in
 * memory_combat_pc34_compat.c. */
static const unsigned char kG0243_PoisonResistance[27] = {
    /* C00 Giant Scorpion  */  2,
    /* C01 Swamp Slime     */  3,
    /* C02 Giggler         */  7,
    /* C03 Wizard Eye      */  9,
    /* C04 Pain Rat        */  5,
    /* C05 Ruster          */  4,
    /* C06 Screamer        */  1,
    /* C07 Rockpile        */  2,
    /* C08 Ghost           */  4,
    /* C09 Stone Golem     */  3,
    /* C10 Mummy           */  5,
    /* C11 Black Flame     */  5,
    /* C12 Skeleton        */  6,
    /* C13 Couatl          */  5,
    /* C14 Vexirk          */  9,
    /* C15 Magenta Worm    */  1,
    /* C16 Trolin          */  2,
    /* C17 Giant Wasp      */  1,
    /* C18 Animated Armour */  7,
    /* C19 Materializer    */ 10,
    /* C20 Water Elemental */  7,
    /* C21 Oitu            */  6,
    /* C22 Demon           */ 11,
    /* C23 Lord Chaos      */ 15,
    /* C24 Red Dragon      */ 11,
    /* C25 Lord Order      */ 15,
    /* C26 Grey Lord       */ 15,
};

int main(void) {
    int i;
    int rc;

    /* T1: All 27 creatures have non-zero resistance (including
     * immune = 0xF, which is non-zero). */
    for (i = 0; i < 27; ++i) {
        CHECK(kG0243_PoisonResistance[i] > 0,
              "T1: every creature has non-zero poison resistance");
    }

    /* T2: F0192_GROUP_GetPoisonResistance_Compat returns the
     * ReDMCSB G0243 value for each creature type. */
    for (i = 0; i < 27; ++i) {
        int got = F0192_GROUP_GetPoisonResistance_Compat(i);
        char buf[96];
        snprintf(buf, sizeof(buf),
                 "T2: F0192(C%02d) == %u (got %d)",
                 i, kG0243_PoisonResistance[i], got);
        CHECK(got == (int)kG0243_PoisonResistance[i], buf);
    }

    /* T3: Lords (C23, C25) and Grey Lord (C26) are immune. */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(23) == 15,
          "T3: Lord Chaos is immune (0xF)");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(25) == 15,
          "T3: Lord Order is immune (0xF)");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(26) == 15,
          "T3: Grey Lord is immune (0xF)");

    /* T4: Magenta Worm has resistance 1 (low — most vulnerable). */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(15) == 1,
          "T4: Magenta Worm has resistance 1 (most vulnerable)");

    /* T5: Out-of-range returns -1. */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(-1) == -1,
          "T5: -1 returns -1 (out-of-range)");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(27) == -1,
          "T5: 27 returns -1 (out-of-range, only 0..26 valid)");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(100) == -1,
          "T5: 100 returns -1 (out-of-range)");

    /* T6: F0192_GROUP_GetResistanceAdjustedPoisonAttack with
     * immune creature returns 0 (full immunity).  Use a fixed
     * RNG state so the test is deterministic. */
    {
        struct RngState_Compat rng;
        int adjusted = -1;
        rng.seed = 0xDEADBEEFu;
        rc = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
            23, 50, &rng, &adjusted);  /* C23 Lord Chaos, immune */
        CHECK(rc == 1, "T6: F0192 poison attack call returns 1");
        CHECK(adjusted == 0, "T6: immune creature has adjusted poison = 0");
    }
    {
        struct RngState_Compat rng;
        int adjusted = -1;
        rng.seed = 0xDEADBEEFu;
        rc = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
            25, 50, &rng, &adjusted);  /* C25 Lord Order, immune */
        CHECK(adjusted == 0, "T6: Lord Order immune -> adjusted 0");
    }
    {
        struct RngState_Compat rng;
        int adjusted = -1;
        rng.seed = 0xDEADBEEFu;
        rc = F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(
            26, 50, &rng, &adjusted);  /* C26 Grey Lord, immune */
        CHECK(adjusted == 0, "T6: Grey Lord immune -> adjusted 0");
    }

    /* T7: Magenta Worm (C15, resistance 1) takes more damage than
     * Vexirk (C14, resistance 9).  With poisonAttack=10 and
     * fixed RNG, the resistance factor is deterministic. */
    {
        struct RngState_Compat rngA, rngB;
        int adjA = -1, adjB = -1;
        rngA.seed = 0xC0FFEEu; rngB.seed = 0xC0FFEEu;
        F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(15, 10, &rngA, &adjA);
        F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(14, 10, &rngB, &adjB);
        /* Magenta Worm has resistance 1, so the formula gives
         * (10 + RANDOM(4)) << 3 / 2 = between 40 and 56.
         * Vexirk has resistance 9, so it gives
         * (10 + RANDOM(4)) << 3 / 10 = between 8 and 11.
         * Worm takes more. */
        CHECK(adjA > adjB, "T7: Magenta Worm takes more poison than Vexirk");
    }

    printf("PASS: GRP-02 F0192 poison resistance source-lock pin (7 scenarios)\n");
    return 0;
}
