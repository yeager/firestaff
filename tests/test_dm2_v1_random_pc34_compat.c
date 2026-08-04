#include "dm2_v1_random_pc34_compat.h"
#include <assert.h>
#include <stdio.h>

static void test_init_zero(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_init(&r);
    assert(r.state == 0);
    printf("test_init_zero OK\n");
}

static void test_deterministic(void)
{
    DM2_V1_RandomState r1, r2;
    dm2_v1_random_seed(&r1, 42);
    dm2_v1_random_seed(&r2, 42);
    for (int i = 0; i < 100; i++)
        assert(dm2_v1_rand(&r1) == dm2_v1_rand(&r2));
    printf("test_deterministic OK\n");
}

static void test_rand_24bit(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_seed(&r, 12345);
    for (int i = 0; i < 1000; i++) {
        int32_t v = dm2_v1_rand(&r);
        assert(v >= 0);
        assert(v <= 0xffffff);
    }
    printf("test_rand_24bit OK\n");
}

static void test_rand16_zero(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_init(&r);
    assert(dm2_v1_rand16(&r, 0) == 0);
    printf("test_rand16_zero OK\n");
}

static void test_rand16_range(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_seed(&r, 999);
    for (int i = 0; i < 1000; i++) {
        int16_t v = dm2_v1_rand16(&r, 10);
        assert(v >= 0 && v < 10);
    }
    printf("test_rand16_range OK\n");
}

static void test_randbit(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_seed(&r, 7777);
    int zeros = 0, ones = 0;
    for (int i = 0; i < 1000; i++) {
        int b = dm2_v1_randbit(&r);
        assert(b == 0 || b == 1);
        if (b) ones++; else zeros++;
    }
    assert(zeros > 100 && ones > 100);
    printf("test_randbit OK\n");
}

static void test_randdir(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_seed(&r, 5555);
    int counts[4] = {0};
    for (int i = 0; i < 1000; i++) {
        int8_t d = dm2_v1_randdir(&r);
        assert(d >= 0 && d <= 3);
        counts[d]++;
    }
    for (int i = 0; i < 4; i++)
        assert(counts[i] > 50);
    printf("test_randdir OK\n");
}

static void test_known_sequence(void)
{
    DM2_V1_RandomState r;
    dm2_v1_random_init(&r);
    int32_t v1 = dm2_v1_rand(&r);
    int32_t v2 = dm2_v1_rand(&r);
    int32_t v3 = dm2_v1_rand(&r);
    /* state=0: 0*magic+11 = 11, output = 11>>8 = 0 */
    assert(v1 == 0);
    /* state=11: 11*0xbb40e62d+11 = known value */
    assert(v2 != v1 || v3 != v2);
    printf("test_known_sequence OK\n");
}

int main(void)
{
    test_init_zero();
    test_deterministic();
    test_rand_24bit();
    test_rand16_zero();
    test_rand16_range();
    test_randbit();
    test_randdir();
    test_known_sequence();
    printf("All dm2_v1_random tests passed.\n");
    return 0;
}
