/* test_dm2_v1_shop_pc34_compat.c - DM2 V1 Shop + NPC Parity Tests
 *
 * Phase 4 mechanics parity coverage (40+ assertions):
 *  1-10. Catalog + built-in shop integrity
 *  11-15. NPC ownership coherence
 *  16-20. Enter / leave lifecycle + state preservation
 *  21-26. Buy logic (gold, inventory, failure paths)
 *  27-30. Sell logic (gold add, item remove, price = half base)
 *  31-36. Price formula + clamping
 *  37-39. Inventory helpers (stack, full, clear)
 *  40-44. NPC dialog (count, names, lines, invalid args)
 *  45-47. Observability counters + reset
 *  48. Source evidence
 *  49. Round-trip: buy/sell/buy
 */

#include "dm2_v1_shop.h"
#include "dm2_v1_tech_magic.h"

#include <stdio.h>
#include <string.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name_) do { \
    printf("  %-58s", #name_); \
    tests_run++; \
    if (test_##name_()) { \
        tests_passed++; \
        printf("  PASS\n"); \
    } else { \
        printf("  FAIL\n"); \
    } \
} while (0)

/* ── Helpers ─────────────────────────────────────────────────────── */

static void setup_clean(void) {
    dm2_v1_shop_reset_state();
    dm2_v1_shop_set_party_gold(1000);
    dm2_v1_shop_set_negotiator(50);
    dm2_v1_shop_clear_inventory();
}

/* Helper: translate idx (0..N-1) to shop_id. */
static int helper_idx_to_id(int idx) {
    static const int order[DM2_NUM_BUILTIN_SHOPS] = {
        DM2_SHOP_ID_GENERAL, DM2_SHOP_ID_WEAPONS, DM2_SHOP_ID_MAGIC,
        DM2_SHOP_ID_TAVERN, DM2_SHOP_ID_BLACKSMITH
    };
    if (idx < 0 || idx >= DM2_NUM_BUILTIN_SHOPS) return 0;
    return order[idx];
}

/* ── Catalog tests (1-10) ──────────────────────────────────────── */

static int test_builtin_count(void) {
    return dm2_v1_shop_get_builtin_count() == DM2_NUM_BUILTIN_SHOPS;
}

static int test_get_builtin_known(void) {
    return dm2_v1_shop_get_builtin(DM2_SHOP_ID_GENERAL) != NULL
        && dm2_v1_shop_get_builtin(DM2_SHOP_ID_WEAPONS) != NULL
        && dm2_v1_shop_get_builtin(DM2_SHOP_ID_MAGIC) != NULL
        && dm2_v1_shop_get_builtin(DM2_SHOP_ID_TAVERN) != NULL
        && dm2_v1_shop_get_builtin(DM2_SHOP_ID_BLACKSMITH) != NULL;
}

static int test_get_builtin_unknown_returns_null(void) {
    return dm2_v1_shop_get_builtin(99) == NULL
        && dm2_v1_shop_get_builtin(0) == NULL
        && dm2_v1_shop_get_builtin(-1) == NULL;
}

static int test_shop_has_npc_owner(void) {
    const DM2_V1_ShopDescriptor *s;
    s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_GENERAL);
    if (!s || s->npc_id < 1 || s->npc_id > DM2_NUM_NPCS) return 0;
    s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_WEAPONS);
    if (!s || s->npc_id < 1 || s->npc_id > DM2_NUM_NPCS) return 0;
    s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_MAGIC);
    if (!s || s->npc_id < 1 || s->npc_id > DM2_NUM_NPCS) return 0;
    s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_TAVERN);
    if (!s || s->npc_id < 1 || s->npc_id > DM2_NUM_NPCS) return 0;
    s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_BLACKSMITH);
    if (!s || s->npc_id < 1 || s->npc_id > DM2_NUM_NPCS) return 0;
    return 1;
}

static int test_shop_has_stock(void) {
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(sid);
        if (!s || s->stock_count < 1) return 0;
    }
    return 1;
}

static int test_stock_item_ids_valid(void) {
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(sid);
        if (!s) continue;
        int any = 0;
        for (int j = 0; j < s->stock_count; j++) {
            if (s->stock[j].item_id > 0) { any = 1; break; }
        }
        if (!any) return 0;
    }
    return 1;
}

static int test_stock_prices_positive(void) {
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(sid);
        if (!s) continue;
        for (int j = 0; j < s->stock_count; j++) {
            if (s->stock[j].base_price <= 0) return 0;
        }
    }
    return 1;
}

static int test_stock_remaining_valid(void) {
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(sid);
        if (!s) continue;
        for (int j = 0; j < s->stock_count; j++) {
            int r = s->stock[j].stock_remaining;
            if (r != -1 && r <= 0) return 0;
        }
    }
    return 1;
}

static int test_lookup_index_known(void) {
    return dm2_v1_shop_lookup_index(DM2_SHOP_ID_GENERAL) >= 0
        && dm2_v1_shop_lookup_index(DM2_SHOP_ID_BLACKSMITH) >= 0;
}

static int test_lookup_index_unknown(void) {
    return dm2_v1_shop_lookup_index(99) == -1
        && dm2_v1_shop_lookup_index(0) == -1;
}

static int test_builtin_shops_unique_ids(void) {
    int seen[DM2_NUM_BUILTIN_SHOPS+1] = {0};
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        if (sid < 1 || sid > DM2_NUM_BUILTIN_SHOPS) return 0;
        if (seen[sid]) return 0;
        seen[sid] = 1;
    }
    return 1;
}

/* ── NPC ownership coherence (11-15) ───────────────────────────── */

static int test_blacksmith_owns_blacksmith_shop(void) {
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_BLACKSMITH);
    return s && s->npc_id == DM2_NPC_BLACKSMITH;
}

static int test_wizard_owns_magic_shop(void) {
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_MAGIC);
    return s && s->npc_id == DM2_NPC_WIZARD;
}

static int test_friendly_owns_general_and_tavern(void) {
    const DM2_V1_ShopDescriptor *g = dm2_v1_shop_get_builtin(DM2_SHOP_ID_GENERAL);
    const DM2_V1_ShopDescriptor *t = dm2_v1_shop_get_builtin(DM2_SHOP_ID_TAVERN);
    return g && t
        && g->npc_id == DM2_NPC_MERCHANT_FRIENDLY
        && t->npc_id == DM2_NPC_MERCHANT_FRIENDLY;
}

static int test_greedy_owns_weapons(void) {
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_WEAPONS);
    return s && s->npc_id == DM2_NPC_MERCHANT_GREEDY;
}

static int test_shop_location_valid(void) {
    for (int i = 0; i < DM2_NUM_BUILTIN_SHOPS; i++) {
        int sid = helper_idx_to_id(i);
        const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(sid);
        if (!s) continue;
        /* map_x, map_y, map_level should be valid signed ranges. */
        if (s->map_x < 0 || s->map_y < 0) return 0;
        if (s->map_level < 0 || s->map_level > 15) return 0;
    }
    return 1;
}

/* ── Enter / leave lifecycle (16-20) ──────────────────────────── */

static int test_enter_activates_shop(void) {
    setup_clean();
    int rc = dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    return rc == 1 && dm2_v1_shop_is_active() == 1
        && dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_GENERAL;
}

static int test_enter_unknown_returns_zero(void) {
    setup_clean();
    return dm2_v1_shop_enter(99) == 0
        && dm2_v1_shop_enter(0) == 0
        && dm2_v1_shop_is_active() == 0;
}

static int test_leave_preserves_state(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    dm2_v1_shop_add_inventory(110, 1);  /* Lantern */
    uint32_t gold_before = dm2_v1_shop_get_party_gold();
    uint32_t hash_before = dm2_v1_shop_party_state_hash();
    dm2_v1_shop_leave(DM2_SHOP_ID_GENERAL);
    return dm2_v1_shop_is_active() == 0
        && dm2_v1_shop_get_party_gold() == gold_before
        && dm2_v1_shop_party_state_hash() == hash_before;
}

static int test_leave_wrong_shop_returns_zero(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    int rc = dm2_v1_shop_leave(DM2_SHOP_ID_WEAPONS);  /* not active */
    return rc == 0 && dm2_v1_shop_is_active() == 1;
}

static int test_active_shop_none_after_leave(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_BLACKSMITH);
    dm2_v1_shop_leave(DM2_SHOP_ID_BLACKSMITH);
    return dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_NONE;
}

/* ── Buy logic (21-26) ────────────────────────────────────────── */

static int test_buy_deducts_gold(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
    uint32_t gold_before = dm2_v1_shop_get_party_gold();
    int rc = dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, 0);
    if (rc != 1) return 0;
    int price = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    return dm2_v1_shop_get_party_gold() == gold_before - (uint32_t)price;
}

static int test_buy_adds_item_to_inventory(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(DM2_SHOP_ID_WEAPONS);
    int slot = -1;
    for (int i = 0; i < s->stock_count; i++) {
        if (s->stock[i].item_id == DM2_ITEM_CROSSBOW) { slot = i; break; }
    }
    if (slot < 0) return 0;
    dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, slot);
    const DM2_V1_ShopState *st = dm2_v1_shop_get_state();
    if (st->inventory_count < 1) return 0;
    int found = 0;
    for (int i = 0; i < st->inventory_count; i++) {
        if (st->inventory_item[i] == DM2_ITEM_CROSSBOW) { found = 1; break; }
    }
    return found;
}

static int test_buy_insufficient_gold_fails(void) {
    setup_clean();
    dm2_v1_shop_set_party_gold(5);  /* less than any effective price */
    dm2_v1_shop_enter(DM2_SHOP_ID_MAGIC);
    /* Stock 0 = Magic Battery, base 300 → effective at 50% skill = 150. */
    int rc = dm2_v1_shop_buy(DM2_SHOP_ID_MAGIC, 0);
    return rc == (int)DM2_SHOP_RESULT_INSUFFICIENT_GOLD;
}

static int test_buy_out_of_range_fails(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    int rc = dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 99);  /* oob */
    return rc == (int)DM2_SHOP_RESULT_ITEM_NOT_IN_STOCK;
}

static int test_buy_inactive_shop_fails(void) {
    setup_clean();
    int rc = dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
    return rc == (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP;
}

static int test_buy_unknown_shop_fails(void) {
    setup_clean();
    int rc = dm2_v1_shop_buy(99, 0);
    return rc == (int)DM2_SHOP_RESULT_NOT_FOUND;
}

/* ── Sell logic (27-30) ───────────────────────────────────────── */

static int test_sell_adds_gold(void) {
    setup_clean();
    dm2_v1_shop_set_party_gold(0);
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 1);  /* base 25 → sell 12 */
    uint32_t gold_before = dm2_v1_shop_get_party_gold();
    int rc = dm2_v1_shop_sell(DM2_SHOP_ID_GENERAL, 0);
    if (rc != 1) return 0;
    return dm2_v1_shop_get_party_gold() > gold_before;
}

static int test_sell_removes_item(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 1);
    dm2_v1_shop_add_inventory(DM2_ITEM_LANTERN, 1);
    int n_before = dm2_v1_shop_get_state()->inventory_count;
    dm2_v1_shop_sell(DM2_SHOP_ID_GENERAL, 0);
    return dm2_v1_shop_get_state()->inventory_count == n_before - 1;
}

static int test_sell_price_is_half_base(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
    dm2_v1_shop_add_inventory(DM2_ITEM_PISTOL, 1);  /* base 250 → sell 125 */
    int sell_price = dm2_v1_shop_get_sell_price(DM2_SHOP_ID_WEAPONS, 0);
    return sell_price == 125;
}

static int test_sell_unknown_shop_returns_not_found(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    int rc = dm2_v1_shop_sell(99, 0);
    return rc == (int)DM2_SHOP_RESULT_NOT_FOUND;
}

/* ── Price formula (31-36) ────────────────────────────────────── */

static int test_effective_price_skill_0(void) {
    setup_clean();
    dm2_v1_shop_set_negotiator(0);
    int p = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    /* Stock 0 = crossbow base 50 → 50 * 100/100 = 50 */
    return p == 50;
}

static int test_effective_price_skill_50(void) {
    setup_clean();
    dm2_v1_shop_set_negotiator(50);
    int p = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    /* 50 * 50/100 = 25 */
    return p == 25;
}

static int test_effective_price_skill_100_min(void) {
    setup_clean();
    dm2_v1_shop_set_negotiator(100);
    int p = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    /* 50 * 0/100 = 0 → clamp to min 1 */
    return p == 1;
}

static int test_effective_price_clamp_high(void) {
    setup_clean();
    dm2_v1_shop_set_negotiator(200);  /* out of range, should clamp to 100 */
    int p = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    return p == 1;
}

static int test_effective_price_clamp_low(void) {
    setup_clean();
    dm2_v1_shop_set_negotiator(-50);  /* out of range, should clamp to 0 */
    int p = dm2_v1_shop_get_effective_price(DM2_SHOP_ID_WEAPONS, 0);
    return p == 50;
}

static int test_base_price_negative_idx(void) {
    return dm2_v1_shop_get_base_price(DM2_SHOP_ID_GENERAL, -1) == -1
        && dm2_v1_shop_get_base_price(DM2_SHOP_ID_GENERAL, 99) == -1;
}

/* ── Inventory helpers (37-39) ────────────────────────────────── */

static int test_add_inventory_stacks(void) {
    setup_clean();
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 3);
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 2);
    const DM2_V1_ShopState *st = dm2_v1_shop_get_state();
    if (st->inventory_count != 1) return 0;
    return st->inventory_qty[0] == 5;
}

static int test_inventory_full(void) {
    setup_clean();
    for (int i = 0; i < 32; i++) {
        if (dm2_v1_shop_add_inventory(2000 + i, 1) != 1) return 0;
    }
    return dm2_v1_shop_add_inventory(9999, 1) == 0;
}

static int test_clear_inventory(void) {
    setup_clean();
    dm2_v1_shop_add_inventory(DM2_ITEM_LANTERN, 1);
    dm2_v1_shop_clear_inventory();
    return dm2_v1_shop_get_state()->inventory_count == 0;
}

/* ── NPC dialog (40-44) ───────────────────────────────────────── */

static int test_npc_count_is_4(void) {
    return dm2_v1_npc_get_count() == 4;
}

static int test_npc_names_valid(void) {
    for (int i = 1; i <= DM2_NUM_NPCS; i++) {
        const char *n = dm2_v1_npc_get_name(i);
        if (!n || n[0] == '\0') return 0;
    }
    return 1;
}

static int test_npc_dialog_lines_valid(void) {
    for (int npc = 1; npc <= DM2_NUM_NPCS; npc++) {
        for (int line = 0; line < DM2_NPC_DIALOG_LINES; line++) {
            const char *s = dm2_v1_npc_get_dialog(npc, line);
            if (!s || s[0] == '\0') return 0;
        }
    }
    return 1;
}

static int test_npc_invalid_npc_id(void) {
    return dm2_v1_npc_get_name(0) == NULL
        && dm2_v1_npc_get_name(99) == NULL
        && dm2_v1_npc_get_name(-1) == NULL;
}

static int test_npc_invalid_line_idx(void) {
    return dm2_v1_npc_get_dialog(1, -1) == NULL
        && dm2_v1_npc_get_dialog(1, DM2_NPC_DIALOG_LINES) == NULL
        && dm2_v1_npc_get_dialog(1, 99) == NULL;
}

/* ── Observability counters (45-47) ───────────────────────────── */

static int test_buy_counter_increments(void) {
    setup_clean();
    int before = dm2_v1_shop_buy_count();
    dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
    dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, 0);
    return dm2_v1_shop_buy_count() == before + 1;
}

static int test_sell_counter_increments(void) {
    setup_clean();
    int before = dm2_v1_shop_sell_count();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    dm2_v1_shop_add_inventory(DM2_ITEM_HEAL_POTION, 1);
    dm2_v1_shop_sell(DM2_SHOP_ID_GENERAL, 0);
    return dm2_v1_shop_sell_count() == before + 1;
}

static int test_enter_leave_counters(void) {
    setup_clean();
    int eb = dm2_v1_shop_enter_count();
    int lb = dm2_v1_shop_leave_count();
    dm2_v1_shop_enter(DM2_SHOP_ID_TAVERN);
    dm2_v1_shop_leave(DM2_SHOP_ID_TAVERN);
    return dm2_v1_shop_enter_count() == eb + 1
        && dm2_v1_shop_leave_count() == lb + 1;
}

/* ── Reset + source (48-49) ───────────────────────────────────── */

static int test_reset_state_clears_counters(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_GENERAL);
    dm2_v1_shop_buy(DM2_SHOP_ID_GENERAL, 0);
    dm2_v1_shop_reset_state();
    return dm2_v1_shop_buy_count() == 0
        && dm2_v1_shop_enter_count() == 0
        && dm2_v1_shop_is_active() == 0;
}

static int test_source_evidence_mentions_skproject(void) {
    const char *e = dm2_v1_shop_source_evidence();
    return e != NULL && e[0] != '\0'
        && strstr(e, "skproject/SKULLWIN/c_shop.cpp") != NULL
        && strstr(e, "skproject/SKULLWIN/c_npc.cpp") != NULL
        && strstr(e, "NUM_NPCS=4") != NULL
        && strstr(e, "SHOP sensor 0x30") != NULL;
}

/* ── Round-trip ──────────────────────────────────────────────── */

static int test_buy_sell_buy_roundtrip(void) {
    setup_clean();
    dm2_v1_shop_enter(DM2_SHOP_ID_WEAPONS);
    dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, 0);
    int n_before = dm2_v1_shop_get_state()->inventory_count;
    dm2_v1_shop_sell(DM2_SHOP_ID_WEAPONS, 0);
    dm2_v1_shop_buy(DM2_SHOP_ID_WEAPONS, 0);
    int n_after = dm2_v1_shop_get_state()->inventory_count;
    return n_before == 1 && n_after == 1;
}

/* ── Main ─────────────────────────────────────────────────────── */

int main(void) {
    printf("DM2 V1 Shop + NPC parity - Phase 4 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_shop.cpp (transactions)\n");
    printf("        skproject/SKULLWIN/c_npc.cpp  (NPC dialog)\n");
    printf("        skproject/SKULLWIN/SKWinGlobal.h:42 (NUM_NPCS=4)\n\n");

    /* Catalog (1-10) */
    TEST(builtin_count);
    TEST(get_builtin_known);
    TEST(get_builtin_unknown_returns_null);
    TEST(shop_has_npc_owner);
    TEST(shop_has_stock);
    TEST(stock_item_ids_valid);
    TEST(stock_prices_positive);
    TEST(stock_remaining_valid);
    TEST(lookup_index_known);
    TEST(lookup_index_unknown);
    TEST(builtin_shops_unique_ids);

    /* NPC ownership (11-15) */
    TEST(blacksmith_owns_blacksmith_shop);
    TEST(wizard_owns_magic_shop);
    TEST(friendly_owns_general_and_tavern);
    TEST(greedy_owns_weapons);
    TEST(shop_location_valid);

    /* Enter / leave (16-20) */
    TEST(enter_activates_shop);
    TEST(enter_unknown_returns_zero);
    TEST(leave_preserves_state);
    TEST(leave_wrong_shop_returns_zero);
    TEST(active_shop_none_after_leave);

    /* Buy (21-26) */
    TEST(buy_deducts_gold);
    TEST(buy_adds_item_to_inventory);
    TEST(buy_insufficient_gold_fails);
    TEST(buy_out_of_range_fails);
    TEST(buy_inactive_shop_fails);
    TEST(buy_unknown_shop_fails);

    /* Sell (27-30) */
    TEST(sell_adds_gold);
    TEST(sell_removes_item);
    TEST(sell_price_is_half_base);
    TEST(sell_unknown_shop_returns_not_found);

    /* Price formula (31-36) */
    TEST(effective_price_skill_0);
    TEST(effective_price_skill_50);
    TEST(effective_price_skill_100_min);
    TEST(effective_price_clamp_high);
    TEST(effective_price_clamp_low);
    TEST(base_price_negative_idx);

    /* Inventory (37-39) */
    TEST(add_inventory_stacks);
    TEST(inventory_full);
    TEST(clear_inventory);

    /* NPC dialog (40-44) */
    TEST(npc_count_is_4);
    TEST(npc_names_valid);
    TEST(npc_dialog_lines_valid);
    TEST(npc_invalid_npc_id);
    TEST(npc_invalid_line_idx);

    /* Observability (45-47) */
    TEST(buy_counter_increments);
    TEST(sell_counter_increments);
    TEST(enter_leave_counters);

    /* Reset + source (48-49) */
    TEST(reset_state_clears_counters);
    TEST(source_evidence_mentions_skproject);

    /* Round-trip */
    TEST(buy_sell_buy_roundtrip);

    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
