/* Test DM2 V1 creature operations (c_creature.cpp). */

#include "dm2_v1_creature_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Mock AI spec: 36-byte entry ---- */
static uint8_t g_ai_spec[36];

static const uint8_t *mock_ai_spec(void *ctx, uint16_t ctype)
{
    (void)ctx; (void)ctype;
    return g_ai_spec;
}

static uint16_t g_rand_val;
static uint16_t mock_rand(void *ctx) { (void)ctx; return g_rand_val; }

static void test_poison_resistance_immune(void)
{
    memset(g_ai_spec, 0, sizeof(g_ai_spec));
    /* resistance w24 bits 8-11 = 0xF (immune) */
    g_ai_spec[25] = 0x0F;
    DM2_V1_CreaturePoisonCallbacks cb = { mock_ai_spec, mock_rand };
    assert(dm2_v1_apply_creature_poison_resistance(0, 10, &cb, NULL) == 0);
    printf("  PASS: poison_resistance_immune\n");
}

static void test_poison_resistance_reduces(void)
{
    memset(g_ai_spec, 0, sizeof(g_ai_spec));
    /* resistance = 4 (bits 8-11) */
    g_ai_spec[25] = 0x04;
    g_rand_val = 2;
    DM2_V1_CreaturePoisonCallbacks cb = { mock_ai_spec, mock_rand };
    /* (10 + 2) * 8 / (4 + 2) = 96 / 6 = 16 */
    int16_t result = dm2_v1_apply_creature_poison_resistance(0, 10, &cb, NULL);
    assert(result == 16);
    printf("  PASS: poison_resistance_reduces\n");
}

static void test_poison_zero_amount(void)
{
    DM2_V1_CreaturePoisonCallbacks cb = { mock_ai_spec, mock_rand };
    assert(dm2_v1_apply_creature_poison_resistance(0, 0, &cb, NULL) == 0);
    printf("  PASS: poison_zero_amount\n");
}

static int g_confuse_attacked = 0;
static uint8_t g_confuse_record[32];

static uint8_t *cc_get_record(void *ctx, uint16_t record)
{ (void)ctx; (void)record; return g_confuse_record; }
static uint8_t cc_get_ctype(void *ctx, uint16_t record)
{ (void)ctx; (void)record; return 0; }
static uint16_t cc_query_resist(void *ctx, uint8_t ct)
{ (void)ctx; (void)ct; return 2; }
static int16_t cc_rand16(void *ctx, int16_t max)
{ (void)ctx; (void)max; return 5; }
static void cc_attack(void *ctx, uint16_t t, int16_t x, int16_t y,
                       int16_t at, int16_t s, int32_t d)
{ (void)ctx; (void)t; (void)x; (void)y; (void)at; (void)s; (void)d; g_confuse_attacked = 1; }

static void test_confuse_creature(void)
{
    memset(g_confuse_record, 0, sizeof(g_confuse_record));
    DM2_V1_ConfuseCreatureSimpleCallbacks cb = {
        0x1234, cc_get_record, cc_get_ctype, cc_query_resist, cc_rand16, cc_attack
    };
    g_confuse_attacked = 0;
    int result = dm2_v1_confuse_creature_simple(10, 3, 4, 100, &cb, NULL);
    assert(result == 1);
    assert(g_confuse_attacked == 1);

    assert(dm2_v1_confuse_creature_simple(10, 0, 0, 0, NULL, NULL) == 0);
    printf("  PASS: confuse_creature\n");
}

/* ---- Mock for creature_can_handle_item_in ---- */
static int16_t mock_next_rec(void *ctx, uint16_t rw)
{
    (void)ctx;
    if (rw == 0x1400) return 0x1801; /* type 5 -> type 6 */
    return (int16_t)0xFFFE;
}

static int mock_can_handle(void *ctx, uint16_t rw, int16_t ctype)
{
    (void)ctx; (void)ctype;
    return ((rw >> 10) & 0xF) == 6 ? 1 : 0;
}

static void test_creature_can_handle_item_in(void)
{
    DM2_V1_CreatureHandleCallbacks cb = { mock_next_rec, mock_can_handle };
    /* Start with type 5, dir 0 — should skip to type 6 */
    int16_t r = dm2_v1_creature_can_handle_item_in(
        0, 0x1400, 0xFF, &cb, NULL);
    assert((uint16_t)r == 0x1801);
    printf("  PASS: creature_can_handle_item_in\n");
}

static void test_null_safety(void)
{
    assert(dm2_v1_apply_creature_poison_resistance(0, 5, NULL, NULL) == 0);
    assert(dm2_v1_confuse_creature_simple(10, 0, 0, 0, NULL, NULL) == 0);
    dm2_v1_rotate_creature(0, 0, 0, NULL, NULL);
    printf("  PASS: null_safety\n");
}

int main(void)
{
    printf("test_dm2_v1_creature_ops:\n");
    test_poison_resistance_immune();
    test_poison_resistance_reduces();
    test_poison_zero_amount();
    test_confuse_creature();
    test_creature_can_handle_item_in();
    test_null_safety();
    printf("All creature_ops tests passed.\n");
    return 0;
}
