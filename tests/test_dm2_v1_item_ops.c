/* Test DM2 V1 item operations (c_item.cpp). */

#include "dm2_v1_item_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- F958 ---- */
static int16_t mock_query_item_value(void *ctx, uint16_t rw, int cat)
{
    (void)ctx; (void)rw;
    if (cat == 2) return -5;
    return 0;
}

static int16_t mock_query_item_value_positive(void *ctx, uint16_t rw, int cat)
{
    (void)ctx; (void)rw; (void)cat;
    return 3;
}

static void test_f958(void)
{
    DM2_V1_ItemValueCallbacks cb = { mock_query_item_value };
    assert(dm2_v1_f958(0x1400, &cb, NULL) == -5);

    /* Positive value clamped to -1 */
    DM2_V1_ItemValueCallbacks cb2 = { mock_query_item_value_positive };
    assert(dm2_v1_f958(0x1400, &cb2, NULL) == -1);

    assert(dm2_v1_f958(0, NULL, NULL) == -1);
    printf("  PASS: f958\n");
}

/* ---- IS_MISCITEM_DRINK_WATER ---- */
static int16_t g_drink_gdat;
static int16_t g_drink_charges;
static int g_drink_charge_delta;
static int g_drink_retake;

static int16_t mock_drink_gdat(void *ctx, uint16_t rw, uint8_t idx)
{
    (void)ctx; (void)rw; (void)idx;
    return g_drink_gdat;
}

static int16_t mock_drink_charge(void *ctx, uint16_t rw, int16_t delta)
{
    (void)ctx; (void)rw;
    g_drink_charge_delta = delta;
    if (delta == 0) return g_drink_charges;
    g_drink_charges += delta;
    return g_drink_charges;
}

static void mock_drink_retake(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    g_drink_retake = 1;
}

static void test_drink_water(void)
{
    DM2_V1_DrinkWaterCallbacks cb = {
        mock_drink_gdat, mock_drink_charge, 0xFFFF, mock_drink_retake
    };

    /* Not drinkable (bit 0 clear) */
    g_drink_gdat = 0x00;
    assert(dm2_v1_is_miscitem_drink_water(0x2800, &cb, NULL) == 0);

    /* Drinkable with charges */
    g_drink_gdat = 0x01;
    g_drink_charges = 3;
    g_drink_retake = 0;
    assert(dm2_v1_is_miscitem_drink_water(0x2800, &cb, NULL) == 1);
    assert(g_drink_charges == 2);

    /* Drinkable but no charges */
    g_drink_charges = 0;
    assert(dm2_v1_is_miscitem_drink_water(0x2800, &cb, NULL) == 0);

    /* Item in hand triggers retake */
    g_drink_charges = 5;
    cb.item_in_hand = 0x2800;
    g_drink_retake = 0;
    assert(dm2_v1_is_miscitem_drink_water(0x2800, &cb, NULL) == 1);
    assert(g_drink_retake == 1);

    printf("  PASS: is_miscitem_drink_water\n");
}

/* ---- TAKE_OBJECT ---- */
static int g_take_drawn, g_take_named, g_take_events, g_take_deferred;
static int g_take_bonus, g_take_moverec;
static uint16_t g_take_hand_rw;

static int16_t mock_take_gdat(void *ctx, uint16_t rw, uint8_t idx)
{
    (void)ctx; (void)rw; (void)idx;
    return 42;
}
static int16_t mock_take_weight(void *ctx, uint16_t rw)
{
    (void)ctx; (void)rw;
    return 10;
}
static void mock_take_set_hand(void *ctx, uint16_t rw, int16_t gw, int16_t wt)
{
    (void)ctx; (void)gw; (void)wt;
    g_take_hand_rw = rw;
}
static void mock_take_draw(void *ctx) { (void)ctx; g_take_drawn = 1; }
static void mock_take_name(void *ctx, uint16_t rw) { (void)ctx; (void)rw; g_take_named = 1; }
static void mock_take_events(void *ctx) { (void)ctx; g_take_events = 1; }
static void mock_take_defer(void *ctx) { (void)ctx; g_take_deferred = 1; }
static void mock_take_bonus(void *ctx, uint16_t rw) { (void)ctx; (void)rw; g_take_bonus = 1; }
static void mock_take_moverec(void *ctx) { (void)ctx; g_take_moverec = 1; }

static void test_take_object(void)
{
    DM2_V1_TakeObjectCallbacks cb = {
        mock_take_gdat, mock_take_weight, mock_take_set_hand,
        mock_take_draw, mock_take_name, mock_take_events,
        mock_take_defer, mock_take_bonus, mock_take_moverec
    };
    g_take_drawn = g_take_named = g_take_events = g_take_deferred = 0;
    g_take_bonus = g_take_moverec = 0;
    g_take_hand_rw = 0xFFFF;

    dm2_v1_take_object(0x1400, 0, &cb, NULL);
    assert(g_take_hand_rw == 0x1400);
    assert(g_take_drawn == 1);
    assert(g_take_named == 1);
    assert(g_take_events == 1);
    assert(g_take_deferred == 0);
    assert(g_take_bonus == 1);
    assert(g_take_moverec == 1);

    /* Deferred mode */
    g_take_events = 0;
    g_take_deferred = 0;
    dm2_v1_take_object(0x1800, 1, &cb, NULL);
    assert(g_take_events == 0);
    assert(g_take_deferred == 1);

    /* NULL item */
    g_take_drawn = 0;
    dm2_v1_take_object(0xFFFF, 0, &cb, NULL);
    assert(g_take_drawn == 0);

    printf("  PASS: take_object\n");
}

int main(void)
{
    printf("test_dm2_v1_item_ops:\n");
    test_f958();
    test_drink_water();
    test_take_object();
    printf("All item_ops tests passed.\n");
    return 0;
}
