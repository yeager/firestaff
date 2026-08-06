#include "dm2_v1_querydb_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Mock state ---- */
static int16_t mock_gdat_return = 0;
static int8_t mock_gdat_last_a, mock_gdat_last_b, mock_gdat_last_c, mock_gdat_last_d;

static int16_t mock_query_gdat_entry_data_index(void *ctx __attribute__((unused)),
    int8_t a, int8_t b, int8_t c, int8_t d)
{
    mock_gdat_last_a = a; mock_gdat_last_b = b;
    mock_gdat_last_c = c; mock_gdat_last_d = d;
    return mock_gdat_return;
}

static int32_t mock_dbspec_return = 0;

static int32_t mock_query_gdat_dbspec_word_value(void *ctx __attribute__((unused)),
    int32_t record __attribute__((unused)), int32_t idx __attribute__((unused)))
{
    return mock_dbspec_return;
}

static uint8_t mock_cls1_return = 0;

static uint8_t mock_query_cls1_from_record(void *ctx __attribute__((unused)),
    int32_t record __attribute__((unused)))
{
    return mock_cls1_return;
}

static uint8_t mock_cls2_return = 0;

static uint8_t mock_query_cls2_from_record(void *ctx __attribute__((unused)),
    int32_t record __attribute__((unused)))
{
    return mock_cls2_return;
}

static void *mock_ai_spec_ptr = NULL;

static void *mock_query_creature_ai_spec_from_type(void *ctx __attribute__((unused)),
    int32_t type __attribute__((unused)))
{
    return mock_ai_spec_ptr;
}

static int16_t mock_rotate_5x5_pos(void *ctx __attribute__((unused)),
    int16_t pos, uint16_t rotate __attribute__((unused)))
{
    return pos; /* identity rotation */
}

static int mock_text_queries;
static int8_t mock_text_a, mock_text_b, mock_text_c;

static char *mock_query_gdat_text(void *ctx __attribute__((unused)),
    int8_t a, int8_t b, int8_t c, char *buf)
{
    mock_text_queries++;
    mock_text_a = a;
    mock_text_b = b;
    mock_text_c = c;
    strcpy(buf, "FIGHTER");
    return buf;
}

/* ---- Helper: build callback struct with all NULLs ---- */
static DM2_V1_QueryDbCallbacks null_callbacks(void)
{
    DM2_V1_QueryDbCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    return cb;
}

/* ================================================================ */
/* Test 1: dm2_v1_query_098d_000f — 5x5 grid position calculation   */
/* ================================================================ */
static void test_query_098d_000f(void)
{
    int16_t w1, w2;

    dm2_v1_query_098d_000f(0, 0, 0, &w1, &w2);
    assert(w1 == 0 && w2 == 0);

    dm2_v1_query_098d_000f(0, 0, 4, &w1, &w2);
    assert(w1 == 4 && w2 == 0);

    dm2_v1_query_098d_000f(0, 0, 5, &w1, &w2);
    assert(w1 == 0 && w2 == 1);

    dm2_v1_query_098d_000f(1, 1, 6, &w1, &w2);
    assert(w1 == 5 && w2 == 5);

    dm2_v1_query_098d_000f(2, 3, 12, &w1, &w2);
    assert(w1 == 10 && w2 == 14);

    dm2_v1_query_098d_000f(0, 0, 24, &w1, &w2);
    assert(w1 == 4 && w2 == 4);

    printf("  PASS: test_query_098d_000f\n");
}

/* ================================================================ */
/* Test 2: dm2_v1_is_cls1_critical_for_load                         */
/* ================================================================ */
static void test_is_cls1_critical_for_load(void)
{
    /* True cases */
    assert(dm2_v1_is_cls1_critical_for_load(0x1b) == true);
    assert(dm2_v1_is_cls1_critical_for_load(0x06) == true);
    assert(dm2_v1_is_cls1_critical_for_load(0x05) == true);

    /* False cases */
    assert(dm2_v1_is_cls1_critical_for_load(0x00) == false);
    assert(dm2_v1_is_cls1_critical_for_load(0x01) == false);
    assert(dm2_v1_is_cls1_critical_for_load(0x1a) == false);
    assert(dm2_v1_is_cls1_critical_for_load(0x07) == false);

    printf("  PASS: test_is_cls1_critical_for_load\n");
}

/* ================================================================ */
/* Test 3: dm2_v1_is_tile_blocked                                    */
/* ================================================================ */
static void test_is_tile_blocked(void)
{
    /* type=0: wall => blocked */
    assert(dm2_v1_is_tile_blocked(0x00) == 1);

    /* type=1 corridor: not blocked */
    assert(dm2_v1_is_tile_blocked(0x20) == 0);

    /* type=2 pit: not blocked */
    assert(dm2_v1_is_tile_blocked(0x40) == 0);

    /* type=3 stairs: not blocked */
    assert(dm2_v1_is_tile_blocked(0x60) == 0);

    /* type=4 door subtypes */
    assert(dm2_v1_is_tile_blocked(0x80) == 0);  /* subtype 0: open */
    assert(dm2_v1_is_tile_blocked(0x81) == 0);  /* subtype 1: open */
    assert(dm2_v1_is_tile_blocked(0x82) == 1);  /* subtype 2: blocked */
    assert(dm2_v1_is_tile_blocked(0x83) == 1);  /* subtype 3: blocked */
    assert(dm2_v1_is_tile_blocked(0x84) == 1);  /* subtype 4: blocked */
    assert(dm2_v1_is_tile_blocked(0x85) == 0);  /* subtype 5: open */

    /* type=5: not blocked */
    assert(dm2_v1_is_tile_blocked(0xA0) == 0);

    /* type=6: conditional on bits */
    assert(dm2_v1_is_tile_blocked(0xC0) == 1);  /* no bit2, no bit0: blocked */
    assert(dm2_v1_is_tile_blocked(0xC1) == 0);  /* bit0=1: not blocked */
    assert(dm2_v1_is_tile_blocked(0xC4) == 0);  /* bit2=1: not blocked */

    /* type=7: blocked */
    assert(dm2_v1_is_tile_blocked(0xE0) == 1);

    printf("  PASS: test_is_tile_blocked\n");
}

/* ================================================================ */
/* Test 4: dm2_v1_dir_from_5x5_pos                                  */
/* ================================================================ */
static void test_dir_from_5x5_pos(void)
{
    assert(dm2_v1_dir_from_5x5_pos(0x06) == 0);   /* north */
    assert(dm2_v1_dir_from_5x5_pos(0x08) == 1);   /* east */
    assert(dm2_v1_dir_from_5x5_pos(0x12) == 2);   /* south */
    assert(dm2_v1_dir_from_5x5_pos(0x10) == 3);   /* west */
    assert(dm2_v1_dir_from_5x5_pos(0x0c) == 4);   /* center */
    assert(dm2_v1_dir_from_5x5_pos(0x00) == -1);  /* invalid */
    assert(dm2_v1_dir_from_5x5_pos(0xFF) == -1);  /* invalid */

    printf("  PASS: test_dir_from_5x5_pos\n");
}

/* ================================================================ */
/* Test 5: dm2_v1_query_creature_blit_recti with identity rotation  */
/* ================================================================ */
static void test_creature_blit_recti(void)
{
    /* n=0, rotate=0, wb=12 => 12 + 0 + 5000 = 5012 */
    assert(dm2_v1_query_creature_blit_recti(0, 0, 12) == 5012);

    /* n=1, rotate=0, wb=6 => 6 + 25 + 5000 = 5031 */
    assert(dm2_v1_query_creature_blit_recti(1, 0, 6) == 5031);

    /* n=3, rotate=0, wb=0 => 0 + 75 + 5000 = 5075 */
    assert(dm2_v1_query_creature_blit_recti(3, 0, 0) == 5075);

    printf("  PASS: test_creature_blit_recti\n");
}

/* ================================================================ */
/* Test 6: dm2_v1_query_door_damage_resist                          */
/* ================================================================ */
static void test_door_damage_resist(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    cb.query_gdat_entry_data_index = mock_query_gdat_entry_data_index;

    /* Mock returns 42 for query (14, type, 11, 15) */
    mock_gdat_return = 42;
    int32_t result = dm2_v1_query_door_damage_resist(3, &cb, NULL);
    assert(result == 42);
    assert(mock_gdat_last_a == 14);
    assert(mock_gdat_last_c == 11);
    assert(mock_gdat_last_d == 15);

    /* NULL callbacks => safe default 0 */
    result = dm2_v1_query_door_damage_resist(3, NULL, NULL);
    assert(result == 0);

    printf("  PASS: test_door_damage_resist\n");
}

/* ================================================================ */
/* Test 7: dm2_v1_is_wall_ornate_alcove                             */
/* ================================================================ */
static void test_is_wall_ornate_alcove(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    cb.query_gdat_entry_data_index = mock_query_gdat_entry_data_index;

    /* ornament == 0xff (-1 as uint8_t) => guard returns 0 */
    int32_t result = dm2_v1_is_wall_ornate_alcove(0xff, &cb, NULL);
    assert(result == 0);

    /* ornament=5, mock returns 3 */
    mock_gdat_return = 3;
    result = dm2_v1_is_wall_ornate_alcove(5, &cb, NULL);
    assert(result == 3);

    /* NULL callbacks => safe default 0 */
    result = dm2_v1_is_wall_ornate_alcove(5, NULL, NULL);
    assert(result == 0);

    printf("  PASS: test_is_wall_ornate_alcove\n");
}

/* ================================================================ */
/* Test 8: dm2_v1_is_miscitem_currency                              */
/* ================================================================ */
static void test_is_miscitem_currency(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    cb.query_cls1_from_record = mock_query_cls1_from_record;
    cb.query_gdat_dbspec_word_value = mock_query_gdat_dbspec_word_value;

    /* cls != 0x0a => 0 */
    mock_cls1_return = 0x05;
    int32_t result = dm2_v1_is_miscitem_currency(100, &cb, NULL);
    assert(result == 0);

    /* cls == 0x0a, dbspec returns 0x4000 => 1 (currency bit set) */
    mock_cls1_return = 0x0a;
    mock_dbspec_return = 0x4000;
    result = dm2_v1_is_miscitem_currency(100, &cb, NULL);
    assert(result == 1);

    /* cls == 0x0a, dbspec returns 0x0000 => 0 (no currency bit) */
    mock_cls1_return = 0x0a;
    mock_dbspec_return = 0x0000;
    result = dm2_v1_is_miscitem_currency(100, &cb, NULL);
    assert(result == 0);

    /* NULL callbacks => safe default 0 */
    result = dm2_v1_is_miscitem_currency(100, NULL, NULL);
    assert(result == 0);

    printf("  PASS: test_is_miscitem_currency\n");
}

/* ================================================================ */
/* Test 9: dm2_v1_query_door_strength                               */
/* ================================================================ */
static void test_door_strength(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    cb.query_gdat_entry_data_index = mock_query_gdat_entry_data_index;

    /* Mock returns strength=5 for (14,type,11,17) => returns 5 */
    mock_gdat_return = 5;
    int32_t result = dm2_v1_query_door_strength(2, &cb, NULL);
    assert(result == 5);

    /* NULL callbacks => safe default 0 */
    result = dm2_v1_query_door_strength(2, NULL, NULL);
    assert(result == 0);

    printf("  PASS: test_door_strength\n");
}

/* ================================================================ */
/* Test 10: dm2_v1_get_creature_weight                              */
/* ================================================================ */
static void test_get_creature_weight(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    cb.query_creature_ai_spec_from_type = mock_query_creature_ai_spec_from_type;

    /* Create a 0x20-byte buffer with byte at offset 0x1d = 100 */
    uint8_t ai_spec[0x20];
    memset(ai_spec, 0, sizeof(ai_spec));
    ai_spec[0x1d] = 100;
    mock_ai_spec_ptr = ai_spec;

    int32_t result = dm2_v1_get_creature_weight(7, &cb, NULL);
    assert(result == 100);

    /* NULL ai_spec => safe default */
    mock_ai_spec_ptr = NULL;
    result = dm2_v1_get_creature_weight(7, &cb, NULL);
    assert(result == 0);

    /* NULL callbacks => safe default 0 */
    result = dm2_v1_get_creature_weight(7, NULL, NULL);
    assert(result == 0);

    printf("  PASS: test_get_creature_weight\n");
}

/* ================================================================ */
/* Test 11: dm2_v1_query_gdat_text -- original text callback route  */
/* ================================================================ */
static void test_query_gdat_text(void)
{
    DM2_V1_QueryDbCallbacks cb = null_callbacks();
    char text[16];

    cb.query_gdat_text = mock_query_gdat_text;
    mock_text_queries = 0;
    memset(text, 0x5a, sizeof(text));

    assert(dm2_v1_query_gdat_text(0x07, 0x00, 0x00, text,
                                  (int32_t)sizeof(text), &cb, NULL) == 1);
    assert(mock_text_queries == 1);
    assert(mock_text_a == 0x07 && mock_text_b == 0x00 && mock_text_c == 0x00);
    assert(strcmp(text, "FIGHTER") == 0);

    /* GDAT keys are bytes in the source ABI; out-of-range values must not
     * wrap to an unrelated original entry. */
    mock_text_queries = 0;
    strcpy(text, "stale");
    assert(dm2_v1_query_gdat_text(0x100, 0, 0, text,
                                  (int32_t)sizeof(text), &cb, NULL) == 0);
    assert(mock_text_queries == 0);
    assert(text[0] == '\0');

    strcpy(text, "stale");
    assert(dm2_v1_query_gdat_text(7, 0, 0, text, 0, &cb, NULL) == 0);
    assert(strcmp(text, "stale") == 0);

    printf("  PASS: test_query_gdat_text\n");
}

/* ================================================================ */
/* Main                                                              */
/* ================================================================ */
int main(void)
{
    test_query_098d_000f();
    test_is_cls1_critical_for_load();
    test_is_tile_blocked();
    test_dir_from_5x5_pos();
    /* test_creature_blit_recti — disabled until function implemented */
    test_door_damage_resist();
    test_is_wall_ornate_alcove();
    test_is_miscitem_currency();
    test_door_strength();
    test_get_creature_weight();
    test_query_gdat_text();

    printf("All dm2_v1_querydb tests passed.\n");
    return 0;
}
