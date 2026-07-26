#include "dm1_v1_explosion_bitmap_viewport_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_derived_index_fire(void)
{
    int idx = dm1_v1_explosion_bitmap_derived_index_pc34(
        DM1_V1_EXPLOSION_ASPECT_FIRE_PC34,
        DM1_V1_EXPLOSION_SCALE_D1_PC34);
    (void)idx;
    assert(idx >= 0);
}

static void test_derived_index_all_aspects(void)
{
    for (int a = 0; a < DM1_V1_EXPLOSION_ASPECT_COUNT_PC34; a++) {
        int idx = dm1_v1_explosion_bitmap_derived_index_pc34(a, DM1_V1_EXPLOSION_SCALE_D1_PC34);
        (void)idx;
        assert(idx >= 0);
    }
}

static void test_lookup_returns_data(void)
{
    int bw = 0, h = 0;
    const unsigned char *bmp = dm1_v1_explosion_bitmap_lookup_pc34(
        DM1_V1_EXPLOSION_ASPECT_FIRE_PC34,
        DM1_V1_EXPLOSION_SCALE_D1_PC34,
        &bw, &h, 0);
    (void)bmp;
    assert(bw > 0);
    assert(h > 0);
}

static void test_lookup_flipped(void)
{
    int bw1 = 0, h1 = 0, bw2 = 0, h2 = 0;
    dm1_v1_explosion_bitmap_lookup_pc34(
        DM1_V1_EXPLOSION_ASPECT_SPELL_PC34,
        DM1_V1_EXPLOSION_SCALE_D1_PC34,
        &bw1, &h1, 0);
    dm1_v1_explosion_bitmap_lookup_pc34(
        DM1_V1_EXPLOSION_ASPECT_SPELL_PC34,
        DM1_V1_EXPLOSION_SCALE_D1_PC34,
        &bw2, &h2, 1);
    (void)bw1;
    assert(bw1 == bw2);
    assert(h1 == h2);
}

int main(void)
{
    test_derived_index_fire();
    test_derived_index_all_aspects();
    test_lookup_returns_data();
    test_lookup_flipped();

    puts("ok: DM1 explosion bitmap viewport (Q-DM1-03) 4 tests passed");
    return 0;
}
