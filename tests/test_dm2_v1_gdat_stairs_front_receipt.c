#include "dm2_v1_gdat_stairs_front_receipt.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static int mock_loadable(void *ctx, uint8_t cls, uint8_t sub, uint8_t idx, uint8_t field)
{
    (void)ctx; (void)cls; (void)sub; (void)idx;
    return (field == 3) ? 1 : 0;
}

static int16_t g_rect_table[] = {10, 20, -1, 30, 40, 50};
static uint8_t g_field_table[] = {3, 5, 7, 9, 11, 13};
static uint8_t g_alt_table[] = {100, 101, 102, 103, 104, 105};
static uint8_t g_detail[64];

static void test_basic_loadable(void)
{
    memset(g_detail, 0, sizeof(g_detail));
    /* cell=0, no detail → table_idx = 0+0 = 0, rect=10, field=3 (loadable) */
    DM2_V1_StairsFrontCallbacks cb = {
        mock_loadable, g_rect_table, g_field_table, g_alt_table, g_detail
    };
    DM2_V1_GdatStairsFrontSourceReceipt r;
    assert(dm2_v1_gdat_stairs_front_source_receipt(0, 5, 100, &cb, NULL, &r) == 1);
    assert(r.valid == 1);
    assert(r.no_draw == 0);
    assert(r.rect_number == 10);
    assert(r.field == 3);
    assert(r.loadable == 1);
    assert(r.graphicsset == 5);
    printf("  PASS: basic_loadable\n");
}

static void test_not_loadable(void)
{
    memset(g_detail, 0, sizeof(g_detail));
    /* cell=0, detail set → table_idx = 1, rect=20, field=5 (not loadable) */
    g_detail[8] = 1; g_detail[9] = 0;
    DM2_V1_StairsFrontCallbacks cb = {
        mock_loadable, g_rect_table, g_field_table, g_alt_table, g_detail
    };
    DM2_V1_GdatStairsFrontSourceReceipt r;
    assert(dm2_v1_gdat_stairs_front_source_receipt(0, 5, 100, &cb, NULL, &r) == 1);
    assert(r.valid == 1);
    assert(r.no_draw == 0);
    assert(r.rect_number == 20);
    assert(r.field == 5);
    assert(r.loadable == 0);
    assert(r.alt_field == 101);
    printf("  PASS: not_loadable_uses_alt\n");
}

static void test_negative_rect(void)
{
    memset(g_detail, 0, sizeof(g_detail));
    /* cell=1, no detail → table_idx = 2, rect=-1 → no_draw */
    DM2_V1_StairsFrontCallbacks cb = {
        mock_loadable, g_rect_table, g_field_table, g_alt_table, g_detail
    };
    DM2_V1_GdatStairsFrontSourceReceipt r;
    assert(dm2_v1_gdat_stairs_front_source_receipt(1, 5, 100, &cb, NULL, &r) == 1);
    assert(r.valid == 1);
    assert(r.no_draw == 1);
    printf("  PASS: negative_rect_no_draw\n");
}

static void test_null_safety(void)
{
    assert(dm2_v1_gdat_stairs_front_source_receipt(0, 0, 0, NULL, NULL, NULL) == 0);
    DM2_V1_GdatStairsFrontSourceReceipt r;
    assert(dm2_v1_gdat_stairs_front_source_receipt(0, 0, 0, NULL, NULL, &r) == 1);
    assert(r.valid == 1);
    assert(r.rect_number < 0);
    assert(r.no_draw == 1);
    printf("  PASS: null_safety\n");
}

int main(void)
{
    printf("test_dm2_v1_gdat_stairs_front_receipt:\n");
    test_basic_loadable();
    test_not_loadable();
    test_negative_rect();
    test_null_safety();
    printf("All stairs_front receipt tests passed.\n");
    return 0;
}
