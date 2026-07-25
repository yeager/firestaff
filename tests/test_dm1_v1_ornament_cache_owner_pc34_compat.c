#include "dm1_v1_ornament_cache_owner_pc34_compat.h"

#include <assert.h>
#include <stdio.h>

static void test_not_loaded(void)
{
    int out = -1;
    int ok = dm1_v1_ornament_cache_global_index_pc34(0, NULL, 0, 1, &out);
    (void)ok;
    assert(ok == 0);
}

static void test_null_output(void)
{
    int ok = dm1_v1_ornament_cache_global_index_pc34(1, NULL, 0, 1, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_ordinal_zero(void)
{
    int table[] = {10, 20, 30};
    int out = -1;
    int ok = dm1_v1_ornament_cache_global_index_pc34(1, table, 3, 0, &out);
    (void)ok;
    assert(ok == 0);
}

static void test_ordinal_valid(void)
{
    int table[] = {100, 200, 300};
    int out = -1;
    int ok = dm1_v1_ornament_cache_global_index_pc34(1, table, 3, 2, &out);
    (void)ok; (void)out;
    assert(ok == 1);
    assert(out == 200);
}

static void test_ordinal_out_of_range(void)
{
    int table[] = {10, 20};
    int out = -1;
    int ok = dm1_v1_ornament_cache_global_index_pc34(1, table, 2, 5, &out);
    (void)ok;
    assert(ok == 0);
}

static void test_negative_global_index(void)
{
    int table[] = {10, -1, 30};
    int out = -1;
    int ok = dm1_v1_ornament_cache_global_index_pc34(1, table, 3, 2, &out);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_not_loaded();
    test_null_output();
    test_ordinal_zero();
    test_ordinal_valid();
    test_ordinal_out_of_range();
    test_negative_global_index();

    puts("ok: DM1 ornament cache owner (Q-DM1-04) 6 tests passed");
    return 0;
}
