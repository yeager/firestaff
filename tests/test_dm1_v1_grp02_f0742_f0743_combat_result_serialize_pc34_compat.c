/*
 * test_dm1_v1_grp02_f0742_f0743_combat_result_serialize_pc34_compat.c
 *
 * Source-locked to ReDMCSB COMBAT.C F0742 (serialize CombatResult)
 * and F0743 (deserialize CombatResult).  LSB-first / PC little-
 * endian, 14 int32 fields = 56 bytes total.
 *
 * GRP-02 (DM1 V1 functional-divergence-report.md):
 *   "F0190/F0191/F0192 damage outcomes are amalgam-only."
 *
 * F0742 and F0743 are the round-trip serialization helpers used
 * for savegame / replay persistence.  Pins the contract:
 *  T1  F0742 NULL result returns 0
 *  T2  F0742 NULL outBuf returns 0
 *  T3  F0742 outBufSize < COMBAT_RESULT_SERIALIZED_SIZE returns 0
 *  T4  F0742 successful serialize returns 1
 *  T5  F0742 LSB-first: bytes 0..3 = outcome
 *  T6  F0742 LSB-first: bytes 4..7 = damageApplied
 *  T7  F0742 field offsets match struct layout
 *  T8  F0742: 56 bytes total
 *  T9  F0743 NULL result returns 0
 *  T10 F0743 NULL buf returns 0
 *  T11 F0743 bufSize < SERIALIZED_SIZE returns 0
 *  T12 F0743 successful deserialize returns 1
 *  T13 F0743 round-trip: serialize then deserialize = original
 *  T14 F0742/F0743 round-trip with all-zero result
 *  T15 F0742/F0743 round-trip with all-0xFF result
 *  T16 F0742/F0743 round-trip with mixed values
 *  T17 F0742 writes LSB byte first (PC convention)
 *  T18 F0743 reads LSB byte first (PC convention)
 *  T19 F0742 + F0743 stable for 100 random values
 *
 * Source-locked to ReDMCSB COMBAT.C F0742 + F0743.
 */

#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg); \
        return 1; \
    } \
} while (0)

int main(void) {
    struct CombatResult_Compat original;
    struct CombatResult_Compat restored;
    unsigned char buf[64];
    int rc;

    /* T1: F0742 NULL result. */
    CHECK(F0742_COMBAT_ResultSerialize_Compat(NULL, buf, sizeof(buf)) == 0,
          "T1: F0742 NULL result returns 0");

    /* T2: F0742 NULL outBuf. */
    memset(&original, 0, sizeof(original));
    CHECK(F0742_COMBAT_ResultSerialize_Compat(&original, NULL, sizeof(buf)) == 0,
          "T2: F0742 NULL outBuf returns 0");

    /* T3: F0742 too-small buffer. */
    CHECK(F0742_COMBAT_ResultSerialize_Compat(&original, buf,
                                              COMBAT_RESULT_SERIALIZED_SIZE - 1) == 0,
          "T3: F0742 too-small buffer returns 0");

    /* T4: F0742 successful serialize. */
    memset(buf, 0xCC, sizeof(buf));
    CHECK(F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf)) == 1,
          "T4: F0742 returns 1 on success");

    /* T5: Outcome at offset 0 (LSB-first). */
    original.outcome = 0x12345678;
    F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    CHECK(buf[0] == 0x78, "T5a: outcome LSB byte = 0x78");
    CHECK(buf[1] == 0x56, "T5b: outcome next byte = 0x56");
    CHECK(buf[2] == 0x34, "T5c: outcome next byte = 0x34");
    CHECK(buf[3] == 0x12, "T5d: outcome MSB byte = 0x12");

    /* T6: damageApplied at offset 4. */
    original.outcome = 0;
    original.damageApplied = 0xDEADBEEF;
    F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    CHECK(buf[4] == 0xEF, "T6a: damageApplied LSB = 0xEF");
    CHECK(buf[5] == 0xBE, "T6b: damageApplied next = 0xBE");
    CHECK(buf[6] == 0xAD, "T6c: damageApplied next = 0xAD");
    CHECK(buf[7] == 0xDE, "T6d: damageApplied MSB = 0xDE");

    /* T7: Field offsets match layout. */
    {
        /* Test all 14 fields: 0,4,8,...,52 */
        original.outcome = 0x11;
        original.damageApplied = 0x22;
        original.rawAttackRoll = 0x33;
        original.defenseRoll = 0x44;
        original.hitLanded = 0x55;
        original.wasCritical = 0x66;
        original.woundMaskAdded = 0x77;
        original.poisonAttackPending = 0x88;
        original.targetKilled = 0x99;
        original.creatureSlotRemoved = 0xAA;
        original.followupEventKind = 0xBB;
        original.followupEventAux0 = 0xCC;
        original.rngCallCount = 0xDD;
        original.wakeFromRest = 0xEE;
        F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
        /* Spot-check at each offset. */
        CHECK(buf[0] == 0x11 && buf[4] == 0x22 && buf[8] == 0x33 &&
              buf[12] == 0x44 && buf[16] == 0x55 && buf[20] == 0x66 &&
              buf[24] == 0x77 && buf[28] == 0x88 && buf[32] == 0x99 &&
              buf[36] == 0xAA && buf[40] == 0xBB && buf[44] == 0xCC &&
              buf[48] == 0xDD && buf[52] == 0xEE,
              "T7: field offsets match (every 4 bytes)");
    }

    /* T8: 56 bytes total. */
    CHECK(COMBAT_RESULT_SERIALIZED_SIZE == 56,
          "T8: COMBAT_RESULT_SERIALIZED_SIZE == 56");

    /* T9: F0743 NULL result. */
    CHECK(F0743_COMBAT_ResultDeserialize_Compat(NULL, buf, sizeof(buf)) == 0,
          "T9: F0743 NULL result returns 0");

    /* T10: F0743 NULL buf. */
    CHECK(F0743_COMBAT_ResultDeserialize_Compat(&restored, NULL, sizeof(buf)) == 0,
          "T10: F0743 NULL buf returns 0");

    /* T11: F0743 too-small buffer. */
    CHECK(F0743_COMBAT_ResultDeserialize_Compat(&restored, buf,
                                                COMBAT_RESULT_SERIALIZED_SIZE - 1) == 0,
          "T11: F0743 too-small buffer returns 0");

    /* T12: F0743 successful deserialize. */
    memset(&restored, 0, sizeof(restored));
    CHECK(F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf)) == 1,
          "T12: F0743 returns 1 on success");

    /* T13: Round-trip preserves all 14 serialized fields.  Note
     * that F0742/F0743 do not serialize 'luckyHit' (F0308 luck);
     * the rest round-trip exactly. */
    {
        struct CombatResult_Compat tmp;
        struct CombatResult_Compat expected;
        memset(&tmp, 0xFF, sizeof(tmp));
        F0742_COMBAT_ResultSerialize_Compat(&tmp, buf, sizeof(buf));
        memset(&restored, 0, sizeof(restored));
        F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
        expected = tmp;
        expected.luckyHit = 0; /* not serialized */
        CHECK(memcmp(&expected, &restored, sizeof(expected)) == 0,
              "T13: F0742+F0743 round-trip preserves 14 serialized fields");
    }

    /* T14: All-zero result. */
    memset(&original, 0, sizeof(original));
    F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    memset(&restored, 0xFF, sizeof(restored));
    F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
    {
        struct CombatResult_Compat expected = original;
        expected.luckyHit = restored.luckyHit; /* not serialized */
        CHECK(memcmp(&expected, &restored, sizeof(expected)) == 0,
              "T14: round-trip zero result");
    }

    /* T15: All-0xFF result. */
    memset(&original, 0xFF, sizeof(original));
    F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    memset(&restored, 0, sizeof(restored));
    F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
    {
        struct CombatResult_Compat expected = original;
        expected.luckyHit = restored.luckyHit; /* not serialized */
        CHECK(memcmp(&expected, &restored, sizeof(expected)) == 0,
              "T15: round-trip 0xFF result");
    }

    /* T16: Mixed values. */
    {
        struct CombatResult_Compat tmp;
        struct CombatResult_Compat expected;
        tmp.outcome = COMBAT_OUTCOME_HIT_DAMAGE;
        tmp.damageApplied = 42;
        tmp.rawAttackRoll = 100;
        tmp.defenseRoll = 50;
        tmp.hitLanded = 1;
        tmp.wasCritical = 0;
        tmp.woundMaskAdded = 0x04;
        tmp.poisonAttackPending = 0;
        tmp.targetKilled = 0;
        tmp.creatureSlotRemoved = -1;
        tmp.followupEventKind = TIMELINE_EVENT_STATUS_TIMEOUT;
        tmp.followupEventAux0 = 7;
        tmp.rngCallCount = 3;
        tmp.wakeFromRest = 0;
        F0742_COMBAT_ResultSerialize_Compat(&tmp, buf, sizeof(buf));
        memset(&restored, 0, sizeof(restored));
        F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
        expected = tmp;
        expected.luckyHit = restored.luckyHit; /* not serialized */
        CHECK(memcmp(&expected, &restored, sizeof(expected)) == 0,
              "T16: mixed values round-trip");
    }

    /* T17: LSB byte first (PC convention). */
    original.outcome = 0xAABBCCDD;
    F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    CHECK(buf[0] == 0xDD && buf[1] == 0xCC && buf[2] == 0xBB && buf[3] == 0xAA,
          "T17: LSB byte first (DD CC BB AA)");

    /* T18: LSB read on deserialize. */
    buf[0] = 0xDD; buf[1] = 0xCC; buf[2] = 0xBB; buf[3] = 0xAA;
    memset(&restored, 0, sizeof(restored));
    F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
    CHECK((uint32_t)restored.outcome == 0xAABBCCDDu, "T18: LSB read = 0xAABBCCDD");

    /* T19: 100 random-ish values round-trip. */
    {
        int k;
        for (k = 0; k < 100; ++k) {
            struct CombatResult_Compat tmp;
            struct CombatResult_Compat expected;
            tmp.outcome = (k * 7) % 16;
            tmp.damageApplied = (k * 13) % 256;
            tmp.rawAttackRoll = k * 3;
            tmp.defenseRoll = k * 5;
            tmp.hitLanded = k & 1;
            tmp.wasCritical = (k >> 1) & 1;
            tmp.woundMaskAdded = (k & 0xF) << 2;
            tmp.poisonAttackPending = (k >> 2) & 1;
            tmp.targetKilled = (k >> 3) & 1;
            tmp.creatureSlotRemoved = (k & 3) - 1;
            tmp.followupEventKind = (k & 0x7) + 1;
            tmp.followupEventAux0 = k * 11;
            tmp.rngCallCount = k;
            tmp.wakeFromRest = (k >> 4) & 1;
            F0742_COMBAT_ResultSerialize_Compat(&tmp, buf, sizeof(buf));
            memset(&restored, 0, sizeof(restored));
            F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
            expected = tmp;
            expected.luckyHit = restored.luckyHit; /* not serialized */
            if (memcmp(&expected, &restored, sizeof(expected)) != 0) {
                fprintf(stderr, "T19: round-trip fail at k=%d\n", k);
                return 1;
            }
        }
        CHECK(1, "T19: 100 round-trips stable");
    }

    /* T20: Returns 1. */
    rc = F0742_COMBAT_ResultSerialize_Compat(&original, buf, sizeof(buf));
    CHECK(rc == 1, "T20: F0742 returns 1");
    rc = F0743_COMBAT_ResultDeserialize_Compat(&restored, buf, sizeof(buf));
    CHECK(rc == 1, "T20: F0743 returns 1");

    printf("PASS: GRP-02 F0742/F0743 combat-result serialize pin (20 scenarios)\n");
    return 0;
}
