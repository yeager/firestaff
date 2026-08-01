#include "theron_v1_track02_dm1_item_names.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } } while (0)
#define TEST(name) printf("  %-60s", name)
#define PASS() do { printf("PASS\n"); return 1; } while (0)

static int test_count(void) {
    TEST("DM1-compatible item count is 63");
    ASSERT(theron_v1_track02_dm1_item_name_count() == 63, "wrong count");
    PASS();
}

static int test_boundaries(void) {
    TEST("First and last items correct");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(0), "COMPASS") == 0, "first");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(62), "EMPTY FLASK") == 0, "last");
    ASSERT(theron_v1_track02_dm1_item_name(63) == NULL, "out of range");
    PASS();
}

static int test_dm1_exclusive_items(void) {
    TEST("DM1-exclusive items (plate armor, keys, FURY)");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(1), "TORCH") == 0, "TORCH");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(3), "EYE OF TIME") == 0, "EYE OF TIME");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(4), "FURY") == 0, "FURY");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(22), "TORSO PLATE") == 0, "TORSO PLATE");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(30), "LEG PLATE") == 0, "LEG PLATE");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(33), "ARMET") == 0, "ARMET");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(39), "FOOT PLATE") == 0, "FOOT PLATE");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(40), "GOLD COIN") == 0, "GOLD COIN");
    ASSERT(strcmp(theron_v1_track02_dm1_item_name(61), "RUBY KEY") == 0, "RUBY KEY");
    PASS();
}

static int test_no_null_names(void) {
    TEST("No NULL names in valid range");
    for (unsigned int i = 0; i < 63; i++) {
        const char *name = theron_v1_track02_dm1_item_name(i);
        if (!name || strlen(name) == 0) {
            printf("FAIL: index %u\n", i);
            return 0;
        }
    }
    PASS();
}

int main(void) {
    printf("Theron V1 Track 02 DM1-Compatible Item Names\n");
    int pass = 0, total = 0;
    total++; pass += test_count();
    total++; pass += test_boundaries();
    total++; pass += test_dm1_exclusive_items();
    total++; pass += test_no_null_names();
    printf("\n%d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
