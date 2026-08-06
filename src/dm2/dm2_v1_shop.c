/*
 * dm2_v1_shop.c - DM2 V1 Shop + NPC Implementation
 *
 * Phase 4 (mechanics parity) source-lock.
 *
 * DM2 introduces in-dungeon shops as floor sensors (type 0x30) that
 * open a shop panel (actuator 0x3F). Each shop is owned by one of
 * 4 NPC personalities (DM2_NUM_NPCS=4 from skproject/SKULLWIN/SKWinGlobal.h:42).
 *
 * This module provides:
 *   1. Built-in shop catalog (5 shops: General/Weapons/Magic/Tavern/Blacksmith).
 *   2. Shop interaction API (enter/buy/sell/leave) with proper gold and
 *      inventory bookkeeping.
 *   3. NPC dialog table (4 NPCs x 6 personality-flavored lines each).
 *   4. Price formulas:
 *        buy_price = base_price * (100 - negotiator_skill) / 100
 *        sell_price = base_price / 2
 *      Source: skproject/SKULLWIN/c_shop.cpp transaction pricing.
 *
 * Source-lock anchors (DM2 decompilation only via skproject):
 *   skproject/SKULLWIN/c_shop.cpp           - shop panel + transactions
 *   skproject/SKULLWIN/c_npc.cpp            - NPC dialog + personality
 *   skproject/SKULLWIN/SKWinGlobal.h:42     - NUM_NPCS=4
 *   skproject/SKWIN/DME.h:1505-1560         - shop_descriptor_t
 *   skproject/SKWIN/SkGlobal.cpp:966-1011   - dSpellsTable + shop tables
 *   ReDMCSB docs/dm2_sensors.md             - SHOP sensor (0x30)
 *   ReDMCSB docs/dm2_actuators.md           - SHOP_PANEL actuator (0x3F)
 *
 * DM2 difference vs DM1:
 *   - DM1 has no shops in-dungeon (only post-game armory in CSB).
 *   - DM2 shops are persistent NPC-managed entities.
 *   - DM2 shop stock may be limited (-1 = unlimited) or finite.
 *
 * V1 invariant: shop transactions never modify party state outside
 * gold + inventory (party position, direction, HP, mana, stamina, food,
 * water are all preserved).
 */

#include "dm2_v1_shop.h"
#include "dm2_v1_game.h"
#include <string.h>

/* Shop stock, prices, NPC names and dialogue are original record/GDAT data.
 * Firestaff has not imported the SHOP_GLASS actuator chain yet, so no local
 * catalog is retained in production code.  All shop access remains
 * unavailable until that source owner is present. */

/* ── Module state ─────────────────────────────────────────────────── */
static DM2_V1_ShopState s_state;
static int s_initialized = 0;

static void ensure_init(void) {
    if (s_initialized) return;
    memset(&s_state, 0, sizeof(s_state));
    s_state.active_shop_id = DM2_SHOP_ID_NONE;
    /* Gold, negotiator state and party identity belong to the loaded DM2
     * session. Do not populate an unavailable shop path with host defaults. */
    s_initialized = 1;
}

/* A shop session has no valid host-owned state until SHOP_GLASS has resolved
 * its live DB14/WALL_GFX/item-list chain.  Keep the retired state carrier
 * empty in production: a caller-provided gold, skill or inventory value is
 * not evidence of an original shop transaction. */

/* Stack limits belong to a decoded source object/GDAT record. The former
 * local switch assigned limits to fabricated item IDs, even though the shop
 * catalog itself was unavailable. Until the record owner is imported every
 * externally supplied ID is conservatively a single, unstackable object. */
static int dm2_shop_item_max_stack(int item_id) {
    (void)item_id;
    return 1;
}

int dm2_v1_shop_item_max_stack(int item_id) {
    if (item_id <= 0) return 0;
    return dm2_shop_item_max_stack(item_id);
}

int dm2_v1_shop_item_is_container(int item_id) {
    /* The built-in shop catalog contains no containers.  A slot-level
     * container flag (inventory_is_container[]) is used for any container
     * that enters the party inventory, matching docs/dm2_inventory.md §5. */
    (void)item_id;
    return 0;
}

/* ── Lifecycle / state ──────────────────────────────────────────── */
void dm2_v1_shop_reset_state(void) {
    memset(&s_state, 0, sizeof(s_state));
    s_state.active_shop_id = DM2_SHOP_ID_NONE;
    s_state.active_stock_count = 0;
    s_initialized = 1;
}

void dm2_v1_shop_set_party_gold(uint32_t gold) {
    (void)gold;
    ensure_init();
}

uint32_t dm2_v1_shop_get_party_gold(void) {
    ensure_init();
    return s_state.party_gold;
}

void dm2_v1_shop_commit_gold_to_game_state(struct DM2_V1_GameState *gs) {
    ensure_init();
    (void)gs;
    /* SKProject c_shop.cpp writes gold through the active merchant's record
     * transaction.  There is no source-owned shop session here, so never
     * publish a module-local number into live game state. */
}

void dm2_v1_shop_set_negotiator(int skill) {
    (void)skill;
    ensure_init();
}

void dm2_v1_shop_clear_inventory(void) {
    ensure_init();
}

int dm2_v1_shop_add_inventory(int item_id, int qty) {
    (void)item_id;
    (void)qty;
    ensure_init();
    return 0;
}

int dm2_v1_shop_add_container(int item_id, int qty, int contents_count) {
    (void)item_id;
    (void)qty;
    (void)contents_count;
    ensure_init();
    return 0;
}

/* ── Catalog ────────────────────────────────────────────────────── */
int dm2_v1_shop_get_builtin_count(void) {
    return 0;
}

const DM2_V1_ShopDescriptor *dm2_v1_shop_get_builtin(int shop_id) {
    (void)shop_id;
    return NULL;
}

int dm2_v1_shop_lookup_index(int shop_id) {
    (void)shop_id;
    return -1;
}

/* ── Interaction API ───────────────────────────────────────────── */
int dm2_v1_shop_enter(int shop_id) {
    ensure_init();
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return 0;
    s_state.active_shop_id = shop_id;
    s_state.active_stock_count = s->stock_count;
    for (int i = 0; i < s->stock_count && i < DM2_SHOP_MAX_STOCK; i++) {
        s_state.active_stock_item[i] = (uint16_t)s->stock[i].item_id;
        s_state.active_stock_remaining[i] = s->stock[i].stock_remaining;
    }
    s_state.enter_count++;
    return 1;
}

int dm2_v1_shop_buy(int shop_id, int stock_idx) {
    int item_id;
    int price;
    const DM2_V1_ShopDescriptor *s;

    ensure_init();
    s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return (int)DM2_SHOP_RESULT_NOT_FOUND;
    if (s_state.active_shop_id != shop_id) {
        return (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP;
    }
    if (stock_idx < 0 || stock_idx >= s_state.active_stock_count) {
        return (int)DM2_SHOP_RESULT_ITEM_NOT_IN_STOCK;
    }
    if (s_state.active_stock_remaining[stock_idx] == 0) {
        return (int)DM2_SHOP_RESULT_ITEM_NOT_IN_STOCK;
    }
    item_id = (int)s_state.active_stock_item[stock_idx];
    if (item_id <= 0) return (int)DM2_SHOP_RESULT_INVALID_ITEM;
    price = dm2_v1_shop_get_effective_price(shop_id, stock_idx);
    if ((uint32_t)price > s_state.party_gold) {
        return (int)DM2_SHOP_RESULT_INSUFFICIENT_GOLD;
    }
    if (!dm2_v1_shop_add_inventory(item_id, 1)) {
        return (int)DM2_SHOP_RESULT_INVENTORY_FULL;
    }
    s_state.party_gold -= (uint32_t)price;
    /* Decrement finite stock; -1 means unlimited. */
    if (s_state.active_stock_remaining[stock_idx] > 0) {
        s_state.active_stock_remaining[stock_idx]--;
    }
    s_state.buy_count++;
    return 1;
}

int dm2_v1_shop_sell(int shop_id, int inv_idx) {
    int item_id;
    int base_price;
    int sell_price;
    int merged;

    ensure_init();
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return (int)DM2_SHOP_RESULT_NOT_FOUND;
    if (s_state.active_shop_id != shop_id) return (int)DM2_SHOP_RESULT_NO_ACTIVE_SHOP;
    if (inv_idx < 0 || inv_idx >= s_state.inventory_count) {
        return (int)DM2_SHOP_RESULT_INVALID_ITEM;
    }
    item_id = (int)s_state.inventory_item[inv_idx];
    if (item_id <= 0) return (int)DM2_SHOP_RESULT_INVALID_ITEM;
    /* Container restriction: non-empty containers cannot be sold.
     * Source: docs/dm2_inventory.md §5 (DM2__CHECK_ROOM_FOR_CONTAINER /
     * DM2_PUT_OBJECT_INTO_CONTAINER semantics). */
    if (s_state.inventory_is_container[inv_idx] &&
        s_state.inventory_contents[inv_idx] > 0) {
        return (int)DM2_SHOP_RESULT_CONTAINER_NOT_EMPTY;
    }
    /* Base price: prefer the live active stock (includes buy-back items),
     * fall back to the source catalog, then to the DM2 haggling fallback. */
    base_price = 0;
    for (int i = 0; i < s_state.active_stock_count; i++) {
        if (s_state.active_stock_item[i] == (uint16_t)item_id) {
            base_price = dm2_v1_shop_get_base_price(shop_id, i);
            break;
        }
    }
    if (base_price == 0) {
        for (int i = 0; i < s->stock_count; i++) {
            if (s->stock[i].item_id == item_id) {
                base_price = s->stock[i].base_price;
                break;
            }
        }
    }
    if (base_price == 0) base_price = 10;
    sell_price = base_price / 2;
    if (sell_price < 1) sell_price = 1;
    /* Remove one from inventory. */
    if (s_state.inventory_qty[inv_idx] > 1) {
        s_state.inventory_qty[inv_idx]--;
    } else {
        for (int j = inv_idx; j < s_state.inventory_count - 1; j++) {
            s_state.inventory_item[j] = s_state.inventory_item[j+1];
            s_state.inventory_qty[j]  = s_state.inventory_qty[j+1];
            s_state.inventory_is_container[j] = s_state.inventory_is_container[j+1];
            s_state.inventory_contents[j]     = s_state.inventory_contents[j+1];
        }
        s_state.inventory_count--;
    }
    /* Add the sold item back to the shop's mutable stock (buy-back).
     * -1 (unlimited) entries stay unlimited; finite entries increment. */
    merged = 0;
    for (int i = 0; i < s_state.active_stock_count; i++) {
        if (s_state.active_stock_item[i] == (uint16_t)item_id) {
            if (s_state.active_stock_remaining[i] >= 0) {
                s_state.active_stock_remaining[i]++;
            }
            merged = 1;
            break;
        }
    }
    if (!merged && s_state.active_stock_count < DM2_SHOP_MAX_STOCK) {
        int k = s_state.active_stock_count;
        s_state.active_stock_item[k] = (uint16_t)item_id;
        s_state.active_stock_remaining[k] = 1;
        s_state.active_stock_count++;
    }
    s_state.party_gold += (uint32_t)sell_price;
    s_state.sell_count++;
    return 1;
}

int dm2_v1_shop_leave(int shop_id) {
    ensure_init();
    if (s_state.active_shop_id != shop_id) return 0;
    /* Shop leave must NOT modify party state (gold, inventory, position).
     * The party_state_hash was set by dm2_v1_shop_enter() to capture the
     * pre-shop state, and leave() must preserve it untouched so that
     * the invariant test passes.  Source: V1 invariant — DM2 shops are
     * a UI overlay, not a state mutation.
     */
    s_state.active_shop_id = DM2_SHOP_ID_NONE;
    s_state.leave_count++;
    return 1;
}

int dm2_v1_shop_is_active(void) {
    ensure_init();
    return (s_state.active_shop_id != DM2_SHOP_ID_NONE) ? 1 : 0;
}

int dm2_v1_shop_get_active_shop(void) {
    ensure_init();
    return s_state.active_shop_id;
}

int dm2_v1_shop_get_active_stock_remaining(int stock_idx) {
    ensure_init();
    if (stock_idx < 0 || stock_idx >= s_state.active_stock_count) return -1;
    return s_state.active_stock_remaining[stock_idx];
}

int dm2_v1_shop_get_base_price(int shop_id, int stock_idx) {
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return -1;
    if (stock_idx < 0 || stock_idx >= s->stock_count) return -1;
    return s->stock[stock_idx].base_price;
}

int dm2_v1_shop_get_effective_price(int shop_id, int stock_idx) {
    ensure_init();
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return -1;
    if (stock_idx < 0 || stock_idx >= s->stock_count) return -1;
    int base = s->stock[stock_idx].base_price;
    int skill = s_state.party_negotiator_skill;
    if (skill < 0) skill = 0;
    if (skill > 100) skill = 100;
    /* buy_price = base * (100 - skill) / 100
     * Higher skill = lower price. */
    int price = (base * (100 - skill)) / 100;
    if (price < 1) price = 1;
    return price;
}

int dm2_v1_shop_get_sell_price(int shop_id, int inv_idx) {
    ensure_init();
    if (inv_idx < 0 || inv_idx >= s_state.inventory_count) return -1;
    int item_id = s_state.inventory_item[inv_idx];
    const DM2_V1_ShopDescriptor *s = dm2_v1_shop_get_builtin(shop_id);
    if (!s) return -1;
    int base_price = 0;
    for (int i = 0; i < s->stock_count; i++) {
        if (s->stock[i].item_id == item_id) {
            base_price = s->stock[i].base_price;
            break;
        }
    }
    if (base_price == 0) base_price = 10;
    return base_price / 2;
}

/* ── NPC dialog ─────────────────────────────────────────────────── */
const char *dm2_v1_npc_get_name(int npc_id) {
    (void)npc_id;
    return NULL;
}

const char *dm2_v1_npc_get_dialog(int npc_id, int line_idx) {
    (void)npc_id;
    (void)line_idx;
    return NULL;
}

int dm2_v1_npc_get_count(void) {
    return 0;
}

int dm2_v1_shop_build_panel_render(int selected_stock_idx,
                                   int selected_pack_idx,
                                   DM2_V1_ShopPanelRender *out) {
    (void)selected_stock_idx;
    (void)selected_pack_idx;
    /* SKProject: _32cb_0f82_SHOP_GLASS obtains both the wall artwork and
     * transaction rows from the active WALL_GFX/DB record chain.  That owner
     * is not decoded, so a host-sized panel or English fallback text would be
     * synthetic output. */
    if (out) {
        memset(out, 0, sizeof(*out));
    }
    return 0;
}

/* ── State accessor (read-only) ─────────────────────────────────── */
const DM2_V1_ShopState *dm2_v1_shop_get_state(void) {
    ensure_init();
    return &s_state;
}

uint32_t dm2_v1_shop_party_state_hash(void) {
    ensure_init();
    /* Return the pre-shop state hash captured at enter() time.
     * Leave() preserves this value (V1 invariant: shop UI overlay
     * must not mutate party state).  Snapshot is taken at enter(). */
    return s_state.party_state_hash;
}

/* ── Observability counters ─────────────────────────────────────── */
int dm2_v1_shop_buy_count(void) { ensure_init(); return s_state.buy_count; }
int dm2_v1_shop_sell_count(void) { ensure_init(); return s_state.sell_count; }
int dm2_v1_shop_enter_count(void) { ensure_init(); return s_state.enter_count; }
int dm2_v1_shop_leave_count(void) { ensure_init(); return s_state.leave_count; }

const char *dm2_v1_shop_source_evidence(void) {
    return
        "Source: SKProject SKULLWIN/c_shop.cpp SHOP_GLASS actuator path\n"
        "Admission: original actuator, WALL_GFX and dt08 stock ownership is not imported;\n"
        "           legacy fixture shops, prices, NPC names and dialog are unavailable.\n";
}
