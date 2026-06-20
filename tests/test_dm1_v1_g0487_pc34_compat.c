#include "firestaff/dm1/v1/G0487_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *file, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s\n", file, line, expr);
    }
}

#define CHECK(c) check((c), #c, __FILE__, __LINE__)

static void test_table_values(void)
{
    const unsigned char *t = dm1_v1_g0487_table_pc34();
    int n = dm1_v1_g0487_size_pc34();
    int i;
    CHECK(t != 0);
    CHECK(n == 200);
    /* Spell 0 (Shield): bytes 0-3 = 0x00 0x66 0x6F 0x00 (Symbols=0x00666F00). */
    CHECK(dm1_v1_g0487_get_pc34(0, 0) == 0x00);
    CHECK(dm1_v1_g0487_get_pc34(0, 1) == 0x66);
    CHECK(dm1_v1_g0487_get_pc34(0, 2) == 0x6F);
    CHECK(dm1_v1_g0487_get_pc34(0, 3) == 0x00);
    /* Spell 0 byte 4 (BaseRequiredSkillLevel=2), byte 5 (SkillIndex=15=DEFEND). */
    CHECK(dm1_v1_g0487_get_pc34(0, 4) == 2);
    CHECK(dm1_v1_g0487_get_pc34(0, 5) == 15);
    /* Spell 24 (Zokathra) bytes 0-3 = 0x00 0x6B 0x6E 0x76 (0x006B6E76). */
    CHECK(dm1_v1_g0487_get_pc34(24, 0) == 0x00);
    CHECK(dm1_v1_g0487_get_pc34(24, 1) == 0x6B);
    CHECK(dm1_v1_g0487_get_pc34(24, 2) == 0x6E);
    CHECK(dm1_v1_g0487_get_pc34(24, 3) == 0x76);
    /* Spell 24 byte 4 (BaseReq=0), byte 5 (SkillIndex=3=WIZARD). */
    CHECK(dm1_v1_g0487_get_pc34(24, 4) == 0);
    CHECK(dm1_v1_g0487_get_pc34(24, 5) == 3);
    /* All values fit in uint8_t. */
    for (i = 0; i < 200; ++i) {
        CHECK(t[i] >= 0);
        CHECK(t[i] <= 255);
    }
}

static void test_lookup_function(void)
{
    int spell, off;
    for (spell = 0; spell < 25; ++spell) {
        for (off = 0; off < 8; ++off) {
            CHECK(dm1_v1_g0487_get_pc34(spell, off) >= 0);
            CHECK(dm1_v1_g0487_get_pc34(spell, off) <= 255);
        }
    }
    CHECK(dm1_v1_g0487_get_pc34(-1, 0) == -1);
    CHECK(dm1_v1_g0487_get_pc34(0, -1) == -1);
    CHECK(dm1_v1_g0487_get_pc34(25, 0) == -1);
    CHECK(dm1_v1_g0487_get_pc34(0, 8) == -1);
    CHECK(dm1_v1_g0487_get_pc34(999, 999) == -1);
}

static void test_first_last_specific(void)
{
    /* Shield: Symbols=0x00666F00 (first 4 bytes). */
    CHECK(dm1_v1_g0487_get_pc34(0, 0) == 0x00);
    /* Zokathra: SkillsIndex=3 (WIZARD). */
    CHECK(dm1_v1_g0487_get_pc34(24, 5) == 3);
}

static void test_run_accepted(void)
{
    DM1_V1_G0487ResultPc34 r;
    int ok = dm1_v1_g0487_run_pc34(&r);
    int i;
    CHECK(ok == 1);
    CHECK(r.accepted == 1);
    CHECK(r.assertionCount == 8);
    CHECK(r.tableSize == 200);
    CHECK(r.tableMatchesDeclaration == 1);
    CHECK(r.spell0ShieldSymbols == 1);
    CHECK(r.allSkillsInValidRange == 1);
    CHECK(r.allBaseReqInByteRange == 1);
    CHECK(r.allAttrsInUint16Range == 1);
    CHECK(r.lookupFunctionCorrect == 1);
    CHECK(r.lookupOutOfRangeReturnsMinusOne == 1);
    for (i = 0; i < 200; ++i) {
        int spell = i / 8;
        int off = i % 8;
        CHECK(r.tableEntries[i] == dm1_v1_g0487_get_pc34(spell, off));
    }
}

int main(void)
{
    test_table_values();
    test_lookup_function();
    test_first_last_specific();
    test_run_accepted();
    printf("dm1_v1_g0487: %d/%d assertions passed\n",
           g_assertions - g_failures, g_assertions);
    return g_failures == 0 ? 0 : 1;
}
