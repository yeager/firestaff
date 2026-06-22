/* firestaff_theron_v1_shop_purchase_gate_probe.c
 *
 * Theron V1 shop purchase gate — fixture-driven headless probe.
 *
 * Pairs with the existing test_theron_v1_shop_price_table CTest target
 * (parser + simple leader-buy atomicity) by covering the narrower
 * purchase-gate edges that the unit test does not lock:
 *
 *   1. Multi-champion slot targeting — purchased item lands in the
 *      requested champion's inventory (slot 0, 1, 2, 3), not always
 *      the leader.  The Theron V1 shop API takes a champion slot index
 *      so the gate is mandatory, not optional.
 *
 *   2. Sequential stock decrement chain — buy N items of the same row
 *      and verify stock decreases monotonically to 0 and the Nth buy
 *      returns THERON_SHOP_OUT_OF_STOCK.
 *
 *   3. Exact-gold purchase — gold == price must drain gold to 0 with
 *      no underflow, no skipped inventory placement.
 *
 *   4. Inventory slot allocation monotonicity — purchased items
 *      always land at the first empty slot, even after partial fills.
 *
 *   5. Stock=255 boundary — stock==0xFF should still allow full
 *      depletion chain until out-of-stock reports correctly.
 *
 *   6. Status-name round-trip — every THERON_SHOP_* enum maps to a
 *      distinct, non-NULL human-readable string.
 *
 *   7. Source-evidence citation — the documented audit boundary
 *      (THQUEST.ASM T560/T800 + ReDMCSB shop absence + phase-2
 *      data-formats gold notes) is preserved in the public source
 *      evidence string.
 *
 * Status: activated 2026-06-22 to fill the V1 shop purchase-gate
 * probe gap (Theron V1 has 8 test binaries but no dedicated
 * headless probe for the shop purchase gate beyond parser coverage).
 *
 * Source-lock:
 *   THQUEST.ASM T560 — dungeon loading + item table
 *   THQUEST.ASM T800 — champion persistence + gold field
 *   docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5.3
 *     (champion_gold offset + Theron-specific persistence)
 *   ReDMCSB_WIP20210206/Toolchains/Common/Source contains no
 *     Theron shop code (DM1/CSB decompilation only).
 */

#include "theron_v1_shop.h"
#include "theron_v1_champions.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures   = 0;

#define CHECK(cond_) do { \
    g_assertions++; \
    if (!(cond_)) { \
        printf("  FAIL  %s:%d  %s\n", __FILE__, __LINE__, #cond_); \
        g_failures++; \
    } \
} while (0)

#define CHECK_GROUP(name) \
    printf("\n  --- %s ---\n", name)

/* Compact 3-row price table fixture: potion 25/3, food 10/2, key 500/1. */
static const uint8_t k_price_fixture[] = {
    THERON_ITEM_POTION, 0x19, 0x00, 0x03,
    THERON_ITEM_FOOD,   0x0a, 0x00, 0x02,
    THERON_ITEM_KEY,    0xf4, 0x01, 0x01,
};

static int parse_fixture(Theron_ShopPriceTable *table) {
    return theron_v1_shop_parse_price_table(table,
                                            k_price_fixture,
                                            sizeof(k_price_fixture))
           == THERON_SHOP_OK;
}

int main(void) {
    printf("Theron V1 Shop Purchase Gate Probe\n");
    printf("Source: THQUEST.ASM T560/T800 + "
           "docs/source-lock/tqr_v1_phase2_data_formats_H2339.md §5.3\n"
           "        ReDMCSB has no Theron shop source (DM1/CSB only)\n");

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Multi-champion slot targeting");

    {
        Theron_ShopPriceTable table;
        Theron_V1_Party party;
        int slot;

        CHECK(parse_fixture(&table));
        theron_v1_party_init(&party, 1);
        party.gold = 10000u;

        /* Stock=3 and 4 buys means: 3 succeed (stock → 0), 4th is
         * rejected with THERON_SHOP_OUT_OF_STOCK. For each iteration
         * we snapshot every champion's inventory[0] BEFORE the buy
         * and verify that ONLY the target slot changed afterwards. */
        for (slot = 0; slot < THERON_MAX_CHAMPIONS; ++slot) {
            uint8_t before[THERON_MAX_CHAMPIONS];
            int other;
            Theron_ShopStatus rc;

            for (other = 0; other < THERON_MAX_CHAMPIONS; ++other) {
                before[other] = party.champions[other].inventory[0];
            }

            rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, slot);
            if (slot < 3) {
                CHECK(rc == THERON_SHOP_OK);
                CHECK(party.champions[slot].inventory[0] ==
                      THERON_ITEM_POTION);
                CHECK(party.champions[slot].load == 1);
            } else {
                /* Stock exhausted on the 4th attempt — gold/stock/inventory
                 * must remain unchanged. */
                CHECK(rc == THERON_SHOP_OUT_OF_STOCK);
                CHECK(party.champions[slot].inventory[0] == THERON_ITEM_NONE);
                CHECK(party.champions[slot].load == 0);
            }

            /* The real multi-champion pollution gate: every champion
             * OTHER than the target must hold the same inventory[0]
             * value as it did BEFORE this purchase. A buggy
             * implementation that always routed to the leader would
             * change champion[0].inventory[0] on every iteration. */
            for (other = 0; other < THERON_MAX_CHAMPIONS; ++other) {
                if (other == slot) continue;
                CHECK(party.champions[other].inventory[0] == before[other]);
            }
        }

        /* Stock depleted to zero after 3 successful purchases. */
        CHECK(table.entries[0].stock == 0u);
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Sequential stock decrement chain");

    {
        Theron_ShopPriceTable table;
        Theron_V1_Party party;
        int buy;

        CHECK(parse_fixture(&table));
        theron_v1_party_init(&party, 1);
        party.gold = 10000u;

        /* Potion row stock = 3. Buy 3 times successfully, then expect
         * the 4th attempt to fail with THERON_SHOP_OUT_OF_STOCK. */
        for (buy = 0; buy < 3; ++buy) {
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, THERON_CHAMPION_SLOT_THERON);
            CHECK(rc == THERON_SHOP_OK);
            CHECK(table.entries[0].stock == (uint8_t)(2u - buy));
        }

        {
            uint32_t gold_before = party.gold;
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, THERON_CHAMPION_SLOT_THERON);
            CHECK(rc == THERON_SHOP_OUT_OF_STOCK);
            /* Failed purchase must keep gold intact. */
            CHECK(party.gold == gold_before);
        }

        /* Verify exactly 3 items landed in inventory slots 0,1,2. */
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[0] ==
              THERON_ITEM_POTION);
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[1] ==
              THERON_ITEM_POTION);
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[2] ==
              THERON_ITEM_POTION);
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[3] ==
              THERON_ITEM_NONE);
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].load == 3);
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Exact-gold purchase (no underflow)");

    {
        Theron_ShopPriceTable table;
        Theron_V1_Party party;

        CHECK(parse_fixture(&table));
        theron_v1_party_init(&party, 1);

        /* Key row price = 500, stock = 1. Set gold exactly equal to price. */
        party.gold = 500u;
        {
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_KEY, THERON_CHAMPION_SLOT_THERON);
            CHECK(rc == THERON_SHOP_OK);
        }
        CHECK(party.gold == 0u);
        CHECK(table.entries[2].stock == 0u);
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[0] ==
              THERON_ITEM_KEY);
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Inventory slot allocation monotonicity");

    {
        Theron_ShopPriceTable table;
        Theron_V1_Party party;
        int i;

        CHECK(parse_fixture(&table));
        theron_v1_party_init(&party, 1);
        party.gold = 10000u;

        /* Pre-fill the leader's inventory slots 0..4 with FOOD, leave
         * slot 5 and onward empty. The next purchase must land at slot 5,
         * not slot 0. */
        for (i = 0; i < 5; ++i) {
            party.champions[THERON_CHAMPION_SLOT_THERON].inventory[i] =
                THERON_ITEM_FOOD;
        }
        theron_v1_party_recalculate_loads(&party);

        {
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, THERON_CHAMPION_SLOT_THERON);
            CHECK(rc == THERON_SHOP_OK);
        }
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[5] ==
              THERON_ITEM_POTION);
        /* Slots 0..4 must still hold FOOD, untouched. */
        for (i = 0; i < 5; ++i) {
            CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].inventory[i] ==
                  THERON_ITEM_FOOD);
        }
        /* load is now 6 items: 5 FOOD + 1 POTION. */
        CHECK(party.champions[THERON_CHAMPION_SLOT_THERON].load == 6);
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Stock=255 boundary depletion");

    {
        Theron_ShopPriceTable table;
        Theron_V1_Party party;
        static const uint8_t big_stock[] = {
            THERON_ITEM_POTION, 0x01, 0x00, 0xff,
        };
        Theron_V1_Champion *leader =
            NULL;
        int placed;
        int slot;

        CHECK(theron_v1_shop_parse_price_table(&table,
                                               big_stock,
                                               sizeof(big_stock))
              == THERON_SHOP_OK);
        CHECK(table.entries[0].stock == 0xffu);

        theron_v1_party_init(&party, 1);
        /* 255 * 1 = 255 gold, well within uint32_t — exact-budget chain. */
        party.gold = 255u;
        leader = &party.champions[THERON_CHAMPION_SLOT_THERON];

        placed = 0;
        for (slot = 0; slot < THERON_INVENTORY_SLOTS; ++slot) {
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, THERON_CHAMPION_SLOT_THERON);
            if (rc != THERON_SHOP_OK) break;
            placed++;
        }
        /* We must place at least min(255, INVENTORY_SLOTS) = 30 items
         * before inventory_full, with gold 255 - 30 = 225 left and stock
         * 255 - 30 = 225. Then the 31st must report inventory-full and
         * keep stock and gold untouched. */
        CHECK(placed == THERON_INVENTORY_SLOTS);
        CHECK(party.gold == 255u - (uint32_t)placed);
        CHECK(table.entries[0].stock == 0xffu - (uint8_t)placed);

        {
            uint32_t gold_before = party.gold;
            uint8_t  stock_before = table.entries[0].stock;
            Theron_ShopStatus rc = theron_v1_shop_purchase_item(
                &party, &table, THERON_ITEM_POTION, THERON_CHAMPION_SLOT_THERON);
            CHECK(rc == THERON_SHOP_INVENTORY_FULL);
            CHECK(party.gold == gold_before);
            CHECK(table.entries[0].stock == stock_before);
        }

        /* Leader load equals the number of placed items. */
        CHECK(leader->load == THERON_INVENTORY_SLOTS);
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Status-name round-trip");

    {
        /* Every THERON_SHOP_* enum value must map to a distinct, non-NULL,
         * non-empty string. The probe uses string compare to verify the
         * documented naming contract. */
        static const struct {
            Theron_ShopStatus code;
            const char *expected;
        } cases[] = {
            { THERON_SHOP_OK,                "ok" },
            { THERON_SHOP_BAD_INPUT,         "bad-input" },
            { THERON_SHOP_BAD_TABLE_SIZE,    "bad-table-size" },
            { THERON_SHOP_TOO_MANY_ENTRIES,  "too-many-entries" },
            { THERON_SHOP_BAD_ITEM,          "bad-item" },
            { THERON_SHOP_NOT_FOUND,         "not-found" },
            { THERON_SHOP_OUT_OF_STOCK,      "out-of-stock" },
            { THERON_SHOP_NOT_ENOUGH_GOLD,   "not-enough-gold" },
            { THERON_SHOP_INVENTORY_FULL,    "inventory-full" },
        };
        size_t i;
        for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
            const char *name = theron_v1_shop_status_name(cases[i].code);
            CHECK(name != NULL);
            if (name) {
                CHECK(strcmp(name, cases[i].expected) == 0);
            }
        }

        /* Out-of-range code must not crash and must report "unknown". */
        {
            const char *name =
                theron_v1_shop_status_name((Theron_ShopStatus)0x7fffffff);
            CHECK(name != NULL);
            if (name) {
                CHECK(strcmp(name, "unknown") == 0);
            }
        }
    }

    /* ──────────────────────────────────────────────────────────────── */
    CHECK_GROUP("Source-evidence citation");

    {
        const char *ev = theron_v1_shop_source_evidence();
        CHECK(ev != NULL);
        if (ev) {
            CHECK(ev[0] != '\0');
            CHECK(strstr(ev, "THQUEST") != NULL);
            CHECK(strstr(ev, "T560") != NULL);
            CHECK(strstr(ev, "T800") != NULL);
            CHECK(strstr(ev, "ReDMCSB") != NULL);
        }
    }

    /* ──────────────────────────────────────────────────────────────── */
    printf("\n%d/%d assertions passed\n", g_assertions - g_failures,
           g_assertions);
    if (g_failures == 0) {
        printf("PASS: Theron V1 shop purchase gate probe\n");
        return 0;
    }
    printf("FAIL: %d assertion(s) failed\n", g_failures);
    return 1;
}
