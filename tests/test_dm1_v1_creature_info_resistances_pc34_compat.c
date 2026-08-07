/*
 * Verify G0243 creature info: poison resistance high-nibble consistency
 * and defense/baseHealth field coverage from the profile table.
 */

#include <stdio.h>
#include <string.h>
#include "memory_combat_pc34_compat.h"
#include "memory_creature_ai_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { g_pass++; } else { g_fail++; fprintf(stderr, "FAIL: %s\n", msg); } \
} while (0)

static void test_poison_resistance_range(void) {
    int i;
    for (i = 0; i < 27; i++) {
        int r = F0192_GROUP_GetPoisonResistance_Compat(i);
        char msg[128];
        snprintf(msg, sizeof(msg), "C%02d: poison resistance in [0,15]", i);
        CHECK(r >= 0 && r <= 15, msg);
    }
}

static void test_poison_resistance_immune_archenemies(void) {
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(23) == 15,
          "C23 Lord Chaos: poison immune");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(25) == 15,
          "C25 Lord Order: poison immune");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(26) == 15,
          "C26 Grey Lord: poison immune");
}

static void test_profile_defense_health_nonzero(void) {
    int i;
    for (i = 0; i < 27; i++) {
        const struct CreatureBehaviorProfile_Compat* p =
            CREATURE_GetProfile_Compat(i);
        char msg[128];
        CHECK(p != NULL, "profile exists");
        if (!p) continue;
        snprintf(msg, sizeof(msg), "C%02d: baseDefense > 0", i);
        CHECK(p->baseDefense > 0, msg);
        snprintf(msg, sizeof(msg), "C%02d: baseHealth > 0", i);
        CHECK(p->baseHealth > 0, msg);
    }
}

static void test_out_of_range(void) {
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(-1) == -1,
          "negative type returns -1");
    CHECK(F0192_GROUP_GetPoisonResistance_Compat(27) == -1,
          "type 27 returns -1");
}

int main(void) {
    test_poison_resistance_range();
    test_poison_resistance_immune_archenemies();
    test_profile_defense_health_nonzero();
    test_out_of_range();
    printf("test_dm1_v1_creature_info_resistances: %d passed, %d failed\n",
           g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
