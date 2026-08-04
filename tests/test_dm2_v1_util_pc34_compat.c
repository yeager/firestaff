#include "dm2_v1_util_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

static void test_abs(void)
{
    assert(dm2_v1_abs_i16(5) == 5);
    assert(dm2_v1_abs_i16(-5) == 5);
    assert(dm2_v1_abs_i16(0) == 0);
    printf("test_abs OK\n");
}

static void test_min_max(void)
{
    assert(dm2_v1_min_i16(3, 7) == 3);
    assert(dm2_v1_min_i16(7, 3) == 3);
    assert(dm2_v1_max_i16(3, 7) == 7);
    assert(dm2_v1_max_i16(7, 3) == 7);
    printf("test_min_max OK\n");
}

static void test_clamp(void)
{
    assert(dm2_v1_clamp_i16(10, 220, 50) == 50);
    assert(dm2_v1_clamp_i16(10, 220, 5) == 10);
    assert(dm2_v1_clamp_i16(10, 220, 250) == 220);
    printf("test_clamp OK\n");
}

static void test_square_distance(void)
{
    assert(dm2_v1_calc_square_distance(0, 0, 3, 4) == 7);
    assert(dm2_v1_calc_square_distance(5, 5, 5, 5) == 0);
    assert(dm2_v1_calc_square_distance(10, 0, 0, 10) == 20);
    printf("test_square_distance OK\n");
}

static void test_vector_dir_north(void)
{
    /* from (5,5) to (5,0): dy > 0, direction 0 (north) */
    assert(dm2_v1_calc_vector_dir(5, 5, 5, 0, 0) == 0);
    printf("test_vector_dir_north OK\n");
}

static void test_vector_dir_east(void)
{
    /* from (0,5) to (5,5): dx < 0, direction 1 (east) */
    assert(dm2_v1_calc_vector_dir(0, 5, 5, 5, 0) == 1);
    printf("test_vector_dir_east OK\n");
}

static void test_vector_dir_south(void)
{
    /* from (5,0) to (5,5): dy < 0, direction 2 (south) */
    assert(dm2_v1_calc_vector_dir(5, 0, 5, 5, 0) == 2);
    printf("test_vector_dir_south OK\n");
}

static void test_vector_dir_west(void)
{
    /* from (5,5) to (0,5): dx > 0, direction 3 (west) */
    assert(dm2_v1_calc_vector_dir(5, 5, 0, 5, 0) == 3);
    printf("test_vector_dir_west OK\n");
}

static void test_vector_dir_tiebreak(void)
{
    /* dx == dy, rand_bit=0 -> ady++, so ady > adx -> use dy */
    int16_t r0 = dm2_v1_calc_vector_dir(0, 0, 3, 3, 0);
    /* dx == dy, rand_bit=1 -> adx++, so adx > ady -> use dx */
    int16_t r1 = dm2_v1_calc_vector_dir(0, 0, 3, 3, 1);
    assert(r0 != r1);
    printf("test_vector_dir_tiebreak OK\n");
}

static void test_vector_w_dir(void)
{
    /* table1d27fc = {0, 1, 0, -1}, table1d2804 = {-1, 0, 1, 0} */
    const int16_t dx[4] = {0, 1, 0, -1};
    const int16_t dy[4] = {-1, 0, 1, 0};
    int16_t x = 5, y = 5;
    /* dir=0, front=2, side=1: offset by 2*dx[0] + 1*dx[1], 2*dy[0] + 1*dy[1]
     * = 2*0 + 1*1 = 1, 2*(-1) + 1*0 = -2 */
    dm2_v1_calc_vector_w_dir(0, 2, 1, dx, dy, &x, &y);
    assert(x == 6 && y == 3);
    printf("test_vector_w_dir OK\n");
}

static void test_compute_power_4_within(void)
{
    /* a=0x15 (bits 0,2,4 set), d=2: 2nd set bit is bit 2, mask = 4 */
    assert(dm2_v1_compute_power_4_within(0x15, 2) == 4);
    /* d=1: first set bit is bit 0 */
    assert(dm2_v1_compute_power_4_within(0x15, 1) == 1);
    /* d=3: third set bit is bit 4 */
    assert(dm2_v1_compute_power_4_within(0x15, 3) == 16);
    printf("test_compute_power_4_within OK\n");
}

static void test_atimesb_rshiftc(void)
{
    /* (100 * 200) >> 3 = 20000 >> 3 = 2500 */
    assert(dm2_v1_atimesb_rshiftc(100, 3, 200) == 2500);
    /* (7 * 170) >> 7 = same as hero stat formula */
    assert(dm2_v1_atimesb_rshiftc(50, 7, 170) == 66);
    printf("test_atimesb_rshiftc OK\n");
}

static void test_fill_i16_table(void)
{
    int16_t table[8];
    dm2_v1_fill_i16_table(table, 0x1234, 8);
    for (int i = 0; i < 8; i++)
        assert(table[i] == 0x1234);
    printf("test_fill_i16_table OK\n");
}

int main(void)
{
    test_abs();
    test_min_max();
    test_clamp();
    test_square_distance();
    test_vector_dir_north();
    test_vector_dir_east();
    test_vector_dir_south();
    test_vector_dir_west();
    test_vector_dir_tiebreak();
    test_vector_w_dir();
    test_compute_power_4_within();
    test_atimesb_rshiftc();
    test_fill_i16_table();
    printf("All dm2_v1_util tests passed.\n");
    return 0;
}
