#include "firestaff/dm1/v1/G0490_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static void test_returns_source_owned_names(void)
{
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(0), "N") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(6), "PUNCH") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(20), "FIREBALL") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(43), "FUSE") == 0);
}

static void test_preserves_duplicate_and_placeholder_rows(void)
{
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(3), "X") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(26), "X") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(9), "STAB") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(14), "STAB") == 0);
}

static void test_rejects_none_and_out_of_range_indices(void)
{
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(DM1_V1_ACTION_NONE_PC34), "") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(44), "") == 0);
    CHECK(strcmp(dm1_v1_action_name_f0384_pc34(254), "") == 0);
}

int main(void)
{
    test_returns_source_owned_names();
    test_preserves_duplicate_and_placeholder_rows();
    test_rejects_none_and_out_of_range_indices();
    printf("dm1 F0384 action names: %s\n", failures ? "FAIL" : "PASS");
    return failures != 0;
}
