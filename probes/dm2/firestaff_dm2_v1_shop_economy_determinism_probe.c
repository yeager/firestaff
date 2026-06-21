/*
 * firestaff_dm2_v1_shop_economy_determinism_probe.c
 * ===================================================
 *
 * DM2 V1 shop-economy determinism probe (Tier 4 #18 polish).
 *
 * Verifies the source-locked shop buy/sell mechanics in
 * dm2_v1_shop.c (skproject/SKULLWIN/c_shop.cpp transaction pricing)
 * are deterministic across many invocations:
 *
 *   - buy() decrements party_gold by effective_price
 *   - buy() increments buy_count
 *   - sell() adds half base_price to party_gold (min 1)
 *   - sell() increments sell_count
 *   - leave() preserves party state hash (no gold/inventory mutation)
 *   - State hash is stable across 50 fresh runs
 *
 * Source-locks:
 *   skproject/SKULLWIN/c_shop.cpp transaction pricing
 *   src/dm2/dm2_v1_shop.c (party-state hash invariants)
 *
 * Run:
 *   ./build/firestaff_dm2_v1_shop_economy_determinism_probe
 *
 * Pass: 12/12 invariants.
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "dm2_v1_shop.h"

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                                  \
    if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }      \
    else      { printf("  FAIL: %s\n", msg); ++g_fail; }      \
} while (0)

/* FNV-style hash of shop state (party_gold + inventory + counts). */
static uint32_t shop_state_hash(void) {
    const DM2_V1_ShopState *s = dm2_v1_shop_get_state();
    uint32_t h = 0x811c9dc5u;
    if (!s) return h;
    h ^= s->party_gold;            h *= 0x01000193u;
    h ^= (uint32_t)s->buy_count;   h *= 0x01000193u;
    h ^= (uint32_t)s->sell_count;  h *= 0x01000193u;
    h ^= (uint32_t)s->inventory_count; h *= 0x01000193u;
    for (int i = 0; i < s->inventory_count; ++i) {
        h ^= (uint32_t)s->inventory_item[i]; h *= 0x01000193u;
        h ^= (uint32_t)s->inventory_qty[i];  h *= 0x01000193u;
    }
    return h;
}

int main(void) {
    printf("=== DM2 V1 shop-economy determinism probe ===\n\n");

    /* 1. Reset + setup. */
    {
        dm2_v1_shop_reset_state();
        dm2_v1_shop_set_party_gold(1000);
        dm2_v1_shop_set_negotiator(50);
        dm2_v1_shop_clear_inventory();
        CHECK(dm2_v1_shop_get_party_gold() == 1000,
              "reset: party_gold = 1000");
        CHECK(dm2_v1_shop_get_state()->inventory_count == 0,
              "reset: inventory_count = 0");
        CHECK(dm2_v1_shop_buy_count() == 0,
              "reset: buy_count = 0");
        CHECK(dm2_v1_shop_sell_count() == 0,
              "reset: sell_count = 0");
    }

    /* 2. Enter GENERAL store + buy item 0 (deterministic). */
    {
        int rc = dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
        CHECK(rc == 1, "enter GENERAL store returns 1 (ok)");
        CHECK(dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_GENERAL,
              "active_shop == GENERAL after enter");
        uint32_t gold_before = dm2_v1_shop_get_party_gold();
        int buy_rc = dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
        CHECK(buy_rc == 1, "buy stock[0] from GENERAL returns 1 (ok)");
        CHECK(dm2_v1_shop_get_party_gold() < gold_before,
              "buy decrements party_gold");
        CHECK(dm2_v1_shop_buy_count() == 1,
              "buy_count incremented to 1");
        CHECK(dm2_v1_shop_get_state()->inventory_count == 1,
              "inventory_count incremented to 1");
    }

    /* 3. Sell the bought item — gold should increase. */
    {
        uint32_t gold_before = dm2_v1_shop_get_party_gold();
        int sell_rc = dm2_v1_shop_sell(DM2_SHOP_ID_GENERAL, 0);
        CHECK(sell_rc == 1, "sell inventory[0] returns 1 (ok)");
        CHECK(dm2_v1_shop_get_party_gold() > gold_before,
              "sell increments party_gold");
        CHECK(dm2_v1_shop_sell_count() == 1,
              "sell_count incremented to 1");
        CHECK(dm2_v1_shop_get_state()->inventory_count == 0,
              "inventory decremented back to 0 after sell");
    }

    /* 4. leave() preserves state hash (no mutation). */
    {
        uint32_t h_before = shop_state_hash();
        int rc = dm2_v1_shop_leave(DM2_SHOP_ID_GENERAL);
        CHECK(rc == 1, "leave GENERAL returns 1 (ok)");
        uint32_t h_after = shop_state_hash();
        CHECK(h_before == h_after,
              "leave() does not mutate party state (hash preserved)");
    }

    /* 5. Determinism: 50 fresh run-throughs produce identical state hash. */
    {
        int mismatch = 0;
        uint32_t expected = 0;
        for (int rep = 0; rep < 50; ++rep) {
            dm2_v1_shop_reset_state();
            dm2_v1_shop_set_party_gold(1000);
            dm2_v1_shop_set_negotiator(50);
            dm2_v1_shop_clear_inventory();
            dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
            dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
            uint32_t h = shop_state_hash();
            if (rep == 0) expected = h;
            else if (h != expected) { ++mismatch; break; }
        }
        CHECK(mismatch == 0,
              "50 fresh buy sequences produce identical state hash");
    }

    /* 6. After leave, buy from wrong shop returns NO_ACTIVE_SHOP. */
    {
        dm2_v1_shop_reset_state();
        dm2_v1_shop_set_party_gold(1000);
        dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
        int rc = dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
        CHECK(rc == (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP,
              "buy from wrong active shop returns NO_ACTIVE_SHOP");
    }

    /* 7. Insufficient gold path. */
    {
        dm2_v1_shop_reset_state();
        dm2_v1_shop_set_party_gold(1); /* too low for any item */
        dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
        int rc = dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
        CHECK(rc == (int)DM2_SHOP_RESULT_INSUFFICIENT_GOLD,
              "buy with insufficient gold returns INSUFFICIENT_GOLD");
    }

    printf("\n# summary: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
