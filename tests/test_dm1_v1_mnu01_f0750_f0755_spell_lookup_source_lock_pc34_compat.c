/*
 * test_dm1_v1_mnu01_f0750_f0755_spell_lookup_source_lock_pc34_compat.c
 *
 * Source-locked to ReDMCSB MENU.C F0409 (spell symbol packing) and
 * F0755 (skill requirement check).
 *
 * MNU-01 (DM1 V1 functional-divergence-report.md):
 *   "F0409 / F0412 spell lookup and casting are amalgam-only" —
 *   intentionally split.  The compat layer has F0750 (rune
 *   sequence encoder) and F0755 (skill check), and F0756/F0757
 *   for cast effect generation.
 *
 * Pins the F0750_MAGIC_EncodeRuneSequence_Compat and
 * F0755_MAGIC_CheckSkillRequired_Compat contracts.
 *
 * F0750 invariants (ReDMCSB MENU.C:1666..1707, F0409 packing loop):
 *  T1  NULL seq returns 0
 *  T2  NULL outPacked returns 0
 *  T3  runeCount=0 returns 0 (must have at least 1 rune)
 *  T4  runeCount=5 returns 0 (max 4)
 *  T5  runeCount=1..4: pack runes[i] in MSB-first order (24, 16, 8, 0)
 *  T6  rune > 0x77 returns 0 (out of valid range)
 *  T7  rune=0x00 is valid (no rune)
 *  T8  rune=0x77 is valid
 *  T9  runes with high bits set (any bits above byte 0) return 0
 *  T10 Encoded 4-rune sequence: 0x11223344 packs to 0x11223344
 *  T11 Encoded 1-rune sequence: 0x00000042 packs to 0x42000000
 *
 * F0755 invariants (ReDMCSB MENU.C:1709..1727):
 *  T12 NULL outMissing returns 0
 *  T13 baseRequired=5, powerOrdinal=0, skillLevel=5 -> success, missing=0
 *  T14 baseRequired=5, powerOrdinal=0, skillLevel=4 -> fail, missing=1
 *  T15 baseRequired=5, powerOrdinal=2, skillLevel=7 -> success, missing=0
 *      (5+2=7, skill=7)
 *  T16 baseRequired=5, powerOrdinal=2, skillLevel=6 -> fail, missing=1
 *      (5+2=7, skill=6)
 *  T17 baseRequired=0, powerOrdinal=0, skillLevel=0 -> success
 *  T18 Skill way above requirement -> success, missing=0
 *  T19 Skill way below requirement -> fail, missing=large
 *
 * Source-locked to ReDMCSB MENU.C:1666..1727.
 */

#include "memory_magic_pc34_compat.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct RuneSequence_Compat seq;
    uint32_t packed;
    int missing;

    /* T1: NULL seq returns 0. */
    packed = 0xDEADBEEF;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(NULL, &packed) == 0,
          "T1: NULL seq returns 0");
    CHECK(packed == 0xDEADBEEF, "T1: outPacked untouched on NULL seq");

    /* T2: NULL outPacked returns 0. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x42;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, NULL) == 0,
          "T2: NULL outPacked returns 0");

    /* T3: runeCount=0 returns 0. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 0;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 0,
          "T3: runeCount=0 returns 0");

    /* T4: runeCount=5 returns 0 (max 4). */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 5;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 0,
          "T4: runeCount=5 returns 0 (max 4)");

    /* T5: runeCount=1..4 packed MSB-first. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x42;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 1,
          "T5: runeCount=1 returns 1");
    CHECK(packed == 0x42000000, "T5: 1-rune pack is MSB-first");

    /* T6: rune > 0x77 returns 0. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x78; /* out of range */
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 0,
          "T6: rune > 0x77 returns 0");

    /* T7: rune=0x00 is valid. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x00;
    packed = 0xDEADBEEF;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 1,
          "T7: rune=0x00 is valid");
    CHECK(packed == 0x00000000, "T7: rune=0x00 packs to 0x00000000");

    /* T8: rune=0x77 is valid. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x77;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 1,
          "T8: rune=0x77 is valid");

    /* T9: runes with high bits set return 0. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x100; /* bit 8 set */
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 0,
          "T9: rune with high bits set returns 0");

    /* T10: 4-rune sequence packs to literal byte sequence. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 4;
    seq.runes[0] = 0x11;
    seq.runes[1] = 0x22;
    seq.runes[2] = 0x33;
    seq.runes[3] = 0x44;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 1,
          "T10: 4-rune returns 1");
    CHECK(packed == 0x11223344,
          "T10: 4-rune pack is 0x11223344 (literal byte sequence MSB-first)");

    /* T11: 1-rune sequence is rune in MSB. */
    memset(&seq, 0, sizeof(seq));
    seq.runeCount = 1;
    seq.runes[0] = 0x42;
    packed = 0;
    CHECK(F0750_MAGIC_EncodeRuneSequence_Compat(&seq, &packed) == 1,
          "T11: 1-rune returns 1");
    CHECK(packed == 0x42000000,
          "T11: 1-rune 0x42 packs to 0x42000000 (MSB byte)");

    /* T12: F0755 NULL outMissing returns 0. */
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 0, 5, NULL) == 0,
          "T12: F0755 NULL outMissing returns 0");

    /* T13: Skill meets requirement (base+power=5+0=5, skill=5). */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 0, 5, &missing) == 1,
          "T13: 5/0/5 returns 1 (success)");
    CHECK(missing == 0, "T13: missing=0 on success");

    /* T14: Skill below requirement (5+0=5, skill=4). */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 0, 4, &missing) == 0,
          "T14: 5/0/4 returns 0 (fail)");
    CHECK(missing == 1, "T14: missing=1 (one short)");

    /* T15: Skill meets requirement with powerOrdinal (5+2=7, skill=7). */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 2, 7, &missing) == 1,
          "T15: 5/2/7 returns 1 (success)");
    CHECK(missing == 0, "T15: missing=0 on success");

    /* T16: Skill below requirement with powerOrdinal (5+2=7, skill=6). */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 2, 6, &missing) == 0,
          "T16: 5/2/6 returns 0 (fail)");
    CHECK(missing == 1, "T16: missing=1 (one short)");

    /* T17: Zero base, zero power, zero skill -> success. */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(0, 0, 0, &missing) == 1,
          "T17: 0/0/0 returns 1");
    CHECK(missing == 0, "T17: missing=0");

    /* T18: Skill way above requirement. */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(5, 2, 100, &missing) == 1,
          "T18: 5/2/100 returns 1");
    CHECK(missing == 0, "T18: missing=0 (skill >> requirement)");

    /* T19: Skill way below requirement. */
    missing = -1;
    CHECK(F0755_MAGIC_CheckSkillRequired_Compat(20, 4, 1, &missing) == 0,
          "T19: 20/4/1 returns 0");
    CHECK(missing == 23, "T19: missing=23 (need 24, have 1)");

    printf("PASS: MNU-01 F0750/F0755 spell-lookup source-lock (19 scenarios)\n");
    return 0;
}
