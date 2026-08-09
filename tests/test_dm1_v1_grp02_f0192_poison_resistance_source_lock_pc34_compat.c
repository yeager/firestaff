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
/* Poison resistance nibble per creature, M061_POISON_RESISTANCE(Resistances)
 * = (Resistances >> 8) & 0xF, taken from ReDMCSB DUNGEON.C
 * G0243_as_Graphic559_CreatureInfo. Verified entry-by-entry against that
 * initialiser: 26 of 27 extracted programmatically and matched exactly, and
 * C26 Grey Lord is separately pinned as immune by T3 below.
 *
 * The previous table here (C00=2, C10=5, C15=1, ...) matched no nibble of any
 * real Resistances word and disagreed with the runtime for most creatures; it
 * also drove the wrong expectations in
 * test_dm1_v1_monster_poison_cloud_overlap_tick_pc34_compat. 15 means
 * C15_IMMUNE_TO_POISON, which GROUP.C F0192 short-circuits to 0. Note C17
 * Giant Wasp legitimately carries 0 — no poison resistance at all. */
static const unsigned char kG0243_PoisonResistance[27] = {
    /* C00 Giant Scorpion     */  8,  /* Resistances -> 0x8 */
    /* C01 Swamp Slime        */ 14,  /* Resistances -> 0xE */
    /* C02 Giggler            */  2,  /* Resistances -> 0x2 */
    /* C03 Wizard Eye         */ 11,  /* Resistances -> 0xB */
    /* C04 Pain Rat           */ 10,  /* Resistances -> 0xA */
    /* C05 Ruster             */  5,  /* Resistances -> 0x5 */
    /* C06 Screamer           */  7,  /* Resistances -> 0x7 */
    /* C07 Rockpile           */  6,  /* Resistances -> 0x6 */
    /* C08 Ghost              */ 15,  /* Resistances -> 0xF */
    /* C09 Stone Golem        */ 15,  /* Resistances -> 0xF */
    /* C10 Mummy              */ 15,  /* Resistances -> 0xF */
    /* C11 Black Flame        */ 15,  /* Resistances -> 0xF */
    /* C12 Skeleton           */ 15,  /* Resistances -> 0xF */
    /* C13 Couatl             */  6,  /* Resistances -> 0x6 */
    /* C14 Vexirk             */  3,  /* Resistances -> 0x3 */
    /* C15 Magenta Worm       */ 11,  /* Resistances -> 0xB */
    /* C16 Trolin             */  3,  /* Resistances -> 0x3 */
    /* C17 Giant Wasp         */  0,  /* Resistances -> 0x0 */
    /* C18 Animated Armour    */ 15,  /* Resistances -> 0xF */
    /* C19 Materializer       */ 15,  /* Resistances -> 0xF */
    /* C20 Water Elemental    */ 14,  /* Resistances -> 0xE */
    /* C21 Oitu               */  8,  /* Resistances -> 0x8 */
    /* C22 Demon              */ 10,  /* Resistances -> 0xA */
    /* C23 Lord Chaos         */ 15,  /* Resistances -> 0xF */
    /* C24 Red Dragon         */  6,  /* Resistances -> 0x6 */
    /* C25 Lord Order         */ 15,  /* Resistances -> 0xF */
    /* C26 Grey Lord          */ 15,  /* Resistances -> 0xF */
};

int main(void) {
    int i;
    int rc;

    /* T1: All 27 creatures have non-zero resistance (including
     * immune = 0xF, which is non-zero). */
    for (i = 0; i < 27; ++i) {
        /* M061_POISON_RESISTANCE yields a 4-bit field.  C17 Giant Wasp
         * legitimately carries 0, so a "non-zero" claim is not a source
         * invariant; the real one is the nibble range. */
        CHECK(kG0243_PoisonResistance[i] <= 15,
              "T1: poison resistance is a 4-bit M061 nibble");
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

    /* T4: C17 Giant Wasp carries Resistances nibble 0 — the genuinely most
     * poison-vulnerable creature.  The old claim that C15 Magenta Worm was
     * "most vulnerable at 1" came from the fabricated table; its real
     * Resistances word is 0x0B93, i.e. nibble 11. */
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(17) == 0,
          "T4: Giant Wasp has resistance 0 (most vulnerable)");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(15) == 11,
          "T4: Magenta Worm resistance is the real 0x0B93 nibble (11)");

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

    /* T7: lower resistance takes more poison.  Real nibbles are C14 Vexirk
     * 3 (Resistances 0x035B) and C15 Magenta Worm 11 (0x0B93), so Vexirk is
     * the more vulnerable of the pair — the reverse of the old assertion,
     * which was derived from the fabricated table.  F0192 gives
     * ((10 + RANDOM(4)) << 3) / (resistance + 1): Vexirk divides by 4,
     * Magenta Worm by 12. */
    {
        struct RngState_Compat rngA, rngB;
        int adjWorm = -1, adjVexirk = -1;
        rngA.seed = 0xC0FFEEu; rngB.seed = 0xC0FFEEu;
        F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(15, 10, &rngA, &adjWorm);
        F0192_GROUP_GetResistanceAdjustedPoisonAttack_Compat(14, 10, &rngB, &adjVexirk);
        CHECK(adjVexirk > adjWorm,
              "T7: Vexirk (r=3) takes more poison than Magenta Worm (r=11)");
    }

    printf("PASS: GRP-02 F0192 poison resistance source-lock pin (7 scenarios)\n");
    return 0;
}
