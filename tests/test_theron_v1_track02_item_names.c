#include "theron_v1_track02_item_names.h"
#include <stdio.h>
#include <string.h>

#define ASSERT(cond, msg) do { if (!(cond)) { printf("FAIL: %s\n", msg); return 0; } } while (0)
#define TEST(name) printf("  %-60s", name)
#define PASS() do { printf("PASS\n"); return 1; } while (0)

static int test_item_count(void) {
    TEST("US item name count is 66");
    ASSERT(theron_v1_track02_us_item_name_count() == 66, "wrong count");
    PASS();
}

static int test_first_item(void) {
    TEST("First item is COMPASS");
    const char *name = theron_v1_track02_us_item_name(0);
    ASSERT(name != NULL, "NULL");
    ASSERT(strcmp(name, "COMPASS") == 0, "wrong name");
    PASS();
}

static int test_last_item(void) {
    TEST("Last item is EMPTY FLASK");
    const char *name = theron_v1_track02_us_item_name(65);
    ASSERT(name != NULL, "NULL");
    ASSERT(strcmp(name, "EMPTY FLASK") == 0, "wrong name");
    PASS();
}

static int test_theron_unique_items(void) {
    TEST("Theron-unique items present");
    ASSERT(strcmp(theron_v1_track02_us_item_name(6), "VORPAL BLADE") == 0, "VORPAL BLADE");
    ASSERT(strcmp(theron_v1_track02_us_item_name(7), "THE RETALIATOR") == 0, "THE RETALIATOR");
    ASSERT(strcmp(theron_v1_track02_us_item_name(12), "ROCK") == 0, "ROCK");
    ASSERT(strcmp(theron_v1_track02_us_item_name(14), "STAFF OF MANAR") == 0, "STAFF OF MANAR");
    ASSERT(strcmp(theron_v1_track02_us_item_name(23), "MITHRAL AKETON") == 0, "MITHRAL AKETON");
    ASSERT(strcmp(theron_v1_track02_us_item_name(31), "MITHRAL MAIL") == 0, "MITHRAL MAIL");
    ASSERT(strcmp(theron_v1_track02_us_item_name(39), "EKKHARD CROSS") == 0, "EKKHARD CROSS");
    ASSERT(strcmp(theron_v1_track02_us_item_name(42), "RABBIT'S FOOT") == 0, "RABBIT'S FOOT");
    ASSERT(strcmp(theron_v1_track02_us_item_name(62), "CORN") == 0, "CORN");
    ASSERT(strcmp(theron_v1_track02_us_item_name(63), "DRUMSTICK") == 0, "DRUMSTICK");
    PASS();
}

static int test_out_of_range(void) {
    TEST("Out-of-range index returns NULL");
    ASSERT(theron_v1_track02_us_item_name(66) == NULL, "66 not NULL");
    ASSERT(theron_v1_track02_us_item_name(1000) == NULL, "1000 not NULL");
    PASS();
}

static int test_no_null_names(void) {
    TEST("No NULL names in valid range");
    for (unsigned int i = 0; i < 66; i++) {
        const char *name = theron_v1_track02_us_item_name(i);
        if (!name) {
            printf("FAIL: index %u is NULL\n", i);
            return 0;
        }
        if (strlen(name) == 0) {
            printf("FAIL: index %u is empty\n", i);
            return 0;
        }
    }
    PASS();
}

int main(void) {
    printf("Theron V1 Track 02 US Item Names\n");
    int pass = 0, total = 0;
    total++; pass += test_item_count();
    total++; pass += test_first_item();
    total++; pass += test_last_item();
    total++; pass += test_theron_unique_items();
    total++; pass += test_out_of_range();
    total++; pass += test_no_null_names();
    printf("\n%d/%d passed\n", pass, total);
    return pass == total ? 0 : 1;
}
