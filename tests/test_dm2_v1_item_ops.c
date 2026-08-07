/* Test DM2 V1 item operations (c_item.cpp). */

#include "dm2_v1_item_ops_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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

/* ---- RETRIEVE_ITEM_BONUS ---- */
static int16_t g_bonus_value;

static int16_t mock_bonus_gdat(void *ctx, uint16_t rw, uint8_t idx)
{
    (void)ctx;
    (void)rw;
    (void)idx;
    return g_bonus_value;
}

static void test_retrieve_item_bonus_source_filter(void)
{
    DM2_V1_ItemBonusCallbacks cb = { mock_bonus_gdat };

    /* bitem.cpp:31-44 retains only the original high-byte sign bit for an
     * unequipped non-conditional item.  Bit 8 must not accidentally pass. */
    g_bonus_value = (int16_t)0x0101;
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 0, 0,
                                      &cb, NULL) == 0);
    g_bonus_value = (int16_t)0x8101;
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 0, 0,
                                      &cb, NULL) == 1);

    /* Equipped items bypass that non-equipped filter and return the signed
     * low byte, while a conditional word requires the source contexts. */
    g_bonus_value = (int16_t)0x0081;
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 1, 0,
                                      &cb, NULL) == -127);
    g_bonus_value = (int16_t)0x4081;
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 1, 0,
                                      &cb, NULL) == 0);
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 1, 2,
                                      &cb, NULL) == -127);
    assert(dm2_v1_retrieve_item_bonus(0x1400u, 0x14u, 1, (int16_t)0xfffe,
                                      &cb, NULL) == 127);
    printf("  PASS: retrieve_item_bonus_source_filter\n");
}

static int read_graphics_from_env(uint8_t **out_bytes, size_t *out_size)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *bytes;

    *out_bytes = NULL;
    *out_size = 0u;
    if (!root || !root[0]) return 0;
    snprintf(path, sizeof(path), "%s/graphics.dat", root);
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        return 0;
    }
    bytes = (uint8_t *)malloc((size_t)size);
    if (!bytes || fread(bytes, 1u, (size_t)size, file) != (size_t)size) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_bytes = bytes;
    *out_size = (size_t)size;
    return 1;
}

static void test_source_item_name_receipt_real_gdat(void)
{
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_RecordPoolSet pools;
    DM2_V1_SourceItemNameReceipt receipt;
    uint8_t weapon_record[4] = { 0u, 0u, 3u, 0u };

    if (!read_graphics_from_env(&graphics, &graphics_size)) {
        puts("  SKIP: source_item_name_receipt_real_gdat (no DM2 data)");
        return;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&pools, 0, sizeof(pools));
    assert(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0);
    assert(dm2_v1_asset_loader_verify(&loader));
    pools.valid = 1;
    pools.pools[5].bytes = weapon_record;
    pools.pools[5].record_count = 1;
    pools.pools[5].record_size = 4;

    /* A source DB5 record with item subtype 3 must resolve through
     * CLS1=WEAPONS (0x10), CLS2=3, then the real GDAT name stream. */
    assert(dm2_v1_query_source_item_name_receipt(
               0x1400u, &pools, &loader, &receipt) == 1);
    assert(receipt.accepted == 1u);
    assert(receipt.cls1 == DM2_GDAT_CATEGORY_WEAPONS);
    assert(receipt.cls2 == 3u);
    assert(strcmp(receipt.gdat.text, "KALAN GAUNTLET") == 0);

    dm2_v1_asset_loader_free(&loader);
    free(graphics);
    printf("  PASS: source_item_name_receipt_real_gdat\n");
}

static void test_source_item_name_receipt_real_record(void)
{
    const char *root = getenv("FIRESTAFF_DM2_DATA_DIR");
    char path[1024];
    FILE *file;
    long size;
    uint8_t *dungeon_bytes = NULL;
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet pools;
    uint8_t *graphics = NULL;
    size_t graphics_size = 0u;
    DM2_V1_AssetLoader loader;
    DM2_V1_SourceItemNameReceipt receipt;

    if (!root || !root[0]) {
        puts("  SKIP: source_item_name_receipt_real_record (no DM2 data)");
        return;
    }
    snprintf(path, sizeof(path), "%s/dungeon.dat", root);
    file = fopen(path, "rb");
    if (!file || fseek(file, 0, SEEK_END) != 0 ||
        (size = ftell(file)) <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        if (file) fclose(file);
        puts("  SKIP: source_item_name_receipt_real_record (no DUNGEON.DAT)");
        return;
    }
    dungeon_bytes = (uint8_t *)malloc((size_t)size);
    assert(dungeon_bytes != NULL);
    assert(fread(dungeon_bytes, 1u, (size_t)size, file) == (size_t)size);
    fclose(file);
    memset(&dungeon, 0, sizeof(dungeon));
    memset(&pools, 0, sizeof(pools));
    assert(dm2_v1_dungeon_load(&dungeon, dungeon_bytes, (int)size) == 0);
    assert(dm2_v1_record_pool_set_init_from_dungeon(&pools, &dungeon) == 1);
    free(dungeon_bytes);

    assert(read_graphics_from_env(&graphics, &graphics_size));
    memset(&loader, 0, sizeof(loader));
    assert(dm2_v1_asset_loader_init(&loader, graphics, graphics_size) == 0);
    assert(dm2_v1_asset_loader_verify(&loader));

    /* G1 direct DB5 weapon receipt: map 17, object 0xD407, source index 7.
     * The name must come from this decoded record and the real GDAT stream. */
    assert(dm2_v1_query_source_item_name_receipt(
               0xd407u, &pools, &loader, &receipt) == 0);
    assert(receipt.cls1 == DM2_GDAT_CATEGORY_WEAPONS);
    assert(receipt.cls2 == 126u);
    assert(receipt.accepted == 0u && receipt.gdat.accepted == 0u);
    printf("  PASS: source_item_name_receipt_real_record (WEAPONS/%u unnamed)\n",
           receipt.cls2);

    dm2_v1_asset_loader_free(&loader);
    dm2_v1_record_pool_set_free(&pools);
    dm2_v1_dungeon_free(&dungeon);
    free(graphics);
}

int main(void)
{
    printf("test_dm2_v1_item_ops:\n");
    test_f958();
    test_drink_water();
    test_take_object();
    test_retrieve_item_bonus_source_filter();
    test_source_item_name_receipt_real_gdat();
    test_source_item_name_receipt_real_record();
    printf("All item_ops tests passed.\n");
    return 0;
}
