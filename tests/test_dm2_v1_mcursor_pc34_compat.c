/*
 * test_dm2_v1_mcursor_pc34_compat.c — unit tests for DM2 mouse cursor graphics.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_mcursor_pc34_compat.h"

static int passed = 0, failed = 0;
#define RUN(fn) do { fn(); passed++; } while(0)

static void test_generate_cursor_invalid_idx(void)
{
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX];
    uint8_t src[4] = {0};
    uint8_t pal[16] = {0};

    DM2_V1_GenerateCursorReceipt r;
    r = dm2_v1_generate_cursor(cursors, src, -1, 0, 0, 2, 2, false, pal, 0);
    assert(!r.generated);

    r = dm2_v1_generate_cursor(cursors, src, 4, 0, 0, 2, 2, false, pal, 0);
    assert(!r.generated);

    printf("  PASS: generate_cursor_invalid_idx\n");
}

static void test_generate_cursor_8bpp(void)
{
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX];
    memset(cursors, 0, sizeof(cursors));

    uint8_t src[4] = {0xAA, 0xBB, 0xCC, 0xDD};
    DM2_V1_GenerateCursorReceipt r = dm2_v1_generate_cursor(
        cursors, src, 2, 3, 4, 2, 2, false, NULL, 0x55);

    assert(r.generated);
    assert(r.cursor_idx == 2);
    assert(cursors[2].pixel[0] == 0xAA);
    assert(cursors[2].pixel[1] == 0xBB);
    assert(cursors[2].pixel[2] == 0xCC);
    assert(cursors[2].pixel[3] == 0xDD);
    assert(cursors[2].alphamask == 0x55);
    assert(cursors[2].hx == 3);
    assert(cursors[2].hy == 4);
    assert(cursors[2].w == 2);
    assert(cursors[2].h == 2);

    printf("  PASS: generate_cursor_8bpp\n");
}

static void test_generate_cursor_4bpp(void)
{
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX];
    memset(cursors, 0, sizeof(cursors));

    /* 4x1 image: nibbles are 0xA, 0xB, 0xC, 0xD
     * MK_EVEN(4)=4, so byte layout: [0xAB, 0xCD] */
    uint8_t src[2] = {0xAB, 0xCD};
    uint8_t pal[16];
    for (int i = 0; i < 16; i++) pal[i] = (uint8_t)(i * 10);

    DM2_V1_GenerateCursorReceipt r = dm2_v1_generate_cursor(
        cursors, src, 0, 0, 0, 4, 1, true, pal, 12);

    assert(r.generated);
    assert(cursors[0].pixel[0] == pal[0xA]); /* 100 */
    assert(cursors[0].pixel[1] == pal[0xB]); /* 110 */
    assert(cursors[0].pixel[2] == pal[0xC]); /* 120 */
    assert(cursors[0].pixel[3] == pal[0xD]); /* 130 */
    assert(cursors[0].alphamask == (int8_t)pal[12]); /* 120 */

    printf("  PASS: generate_cursor_4bpp\n");
}

static void test_generate_cursor_hotspot(void)
{
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX];
    memset(cursors, 0, sizeof(cursors));
    uint8_t src[1] = {0};

    DM2_V1_GenerateCursorReceipt r = dm2_v1_generate_cursor(
        cursors, src, 1, 5, 7, 1, 1, false, NULL, 0);

    assert(r.generated);
    assert(r.hotspot_x == 5);
    assert(r.hotspot_y == 7);
    assert(r.width == 1);
    assert(r.height == 1);

    printf("  PASS: generate_cursor_hotspot\n");
}

static void test_init_basic_cursors(void)
{
    DM2_V1_MCursor cursors[DM2_V1_MCURSOR_MAX];
    memset(cursors, 0, sizeof(cursors));

    uint8_t cursor1[DM2_V1_MOUSECURSOR_FILESIZE1];
    memset(cursor1, 0x12, sizeof(cursor1));

    DM2_V1_Cursor2 cursor2;
    memset(&cursor2, 0, sizeof(cursor2));
    memset(cursor2.pixel, 0x34, sizeof(cursor2.pixel));

    uint8_t palette[16];
    for (int i = 0; i < 16; i++) palette[i] = (uint8_t)(i * 5);

    DM2_V1_InitBasicCursorsReceipt r = dm2_v1_init_basic_cursors(
        cursors, cursor1, &cursor2, palette);

    assert(r.initialized);
    assert(r.cursor_count == 2);
    assert(cursors[0].w == 12);
    assert(cursors[0].h == 16);
    assert(cursors[1].w == 16);
    assert(cursors[1].h == 16);

    printf("  PASS: init_basic_cursors\n");
}

static void test_null_safety(void)
{
    DM2_V1_GenerateCursorReceipt r;
    r = dm2_v1_generate_cursor(NULL, NULL, 0, 0, 0, 1, 1, false, NULL, 0);
    assert(!r.generated);

    DM2_V1_InitBasicCursorsReceipt r2;
    r2 = dm2_v1_init_basic_cursors(NULL, NULL, NULL, NULL);
    assert(!r2.initialized);

    printf("  PASS: null_safety\n");
}

int main(void)
{
    printf("test_dm2_v1_mcursor_pc34_compat:\n");
    RUN(test_generate_cursor_invalid_idx);
    RUN(test_generate_cursor_8bpp);
    RUN(test_generate_cursor_4bpp);
    RUN(test_generate_cursor_hotspot);
    RUN(test_init_basic_cursors);
    RUN(test_null_safety);
    printf("  %d passed, %d failed\n", passed, failed);
    return failed;
}
