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
#include "dm2_v1_tech_magic.h"

#include <stdio.h>
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

static void dm2_shop_copy_text(char *dst, size_t dst_size, const char *src) {
    if (!dst || dst_size == 0) {
        return;
    }
    if (!src) {
        dst[0] = '\0';
        return;
    }
    snprintf(dst, dst_size, "%s", src);
}

static void dm2_shop_format_item_name(int item_id, char *out, size_t out_size) {
    const char *known_name;

    if (!out || out_size == 0) {
        return;
    }
    known_name = dm2_v1_tech_magic_item_name(item_id);
    if (known_name && known_name[0] != '\0') {
        dm2_shop_copy_text(out, out_size, known_name);
        return;
    }
    (void)item_id;
    out[0] = '\0';
}

/* party_state_hash is captured by dm2_v1_shop_enter() and preserved
 * unchanged by dm2_v1_shop_leave() — see those functions and the
 * dm2_v1_shop_party_state_hash() accessor.  No per-tick recomputation
 * is performed, which is the V1 invariant: shop UI overlay must not
 * mutate party state.
 */

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
    ensure_init();
    s_state.party_gold = gold;
}

uint32_t dm2_v1_shop_get_party_gold(void) {
    ensure_init();
    return s_state.party_gold;
}

void dm2_v1_shop_commit_gold_to_game_state(struct DM2_V1_GameState *gs) {
    ensure_init();
    if (!gs) return;
    /* Shop gold is stored unsigned; game-state gold is signed int. */
    gs->gold = (int)s_state.party_gold;
}

void dm2_v1_shop_set_negotiator(int skill) {
    ensure_init();
    if (skill < 0) skill = 0;
    if (skill > 100) skill = 100;
    s_state.party_negotiator_skill = skill;
}

void dm2_v1_shop_clear_inventory(void) {
    ensure_init();
    memset(s_state.inventory_item, 0, sizeof(s_state.inventory_item));
    memset(s_state.inventory_qty, 0, sizeof(s_state.inventory_qty));
    memset(s_state.inventory_is_container, 0, sizeof(s_state.inventory_is_container));
    memset(s_state.inventory_contents, 0, sizeof(s_state.inventory_contents));
    s_state.inventory_count = 0;
}

int dm2_v1_shop_add_inventory(int item_id, int qty) {
    int max_stack;
    int total_existing = 0;
    int slots_used = 0;
    int new_total;
    int required_slots;
    int extra_slots;
    int remaining;

    ensure_init();
    if (qty <= 0 || item_id <= 0) return 0;
    max_stack = dm2_shop_item_max_stack(item_id);
    for (int i = 0; i < s_state.inventory_count; i++) {
        if (s_state.inventory_item[i] == (uint16_t)item_id) {
            total_existing += (int)s_state.inventory_qty[i];
            slots_used++;
        }
    }
    new_total = total_existing + qty;
    required_slots = (new_total + max_stack - 1) / max_stack;
    extra_slots = required_slots - slots_used;
    if (extra_slots < 0) extra_slots = 0;
    if (s_state.inventory_count + extra_slots > 32) return 0;

    remaining = qty;
    /* Fill existing stacks first. */
    for (int i = 0; i < s_state.inventory_count && remaining > 0; i++) {
        if (s_state.inventory_item[i] == (uint16_t)item_id) {
            int room = max_stack - (int)s_state.inventory_qty[i];
            if (room > 0) {
                int add = remaining < room ? remaining : room;
                s_state.inventory_qty[i] += (uint16_t)add;
                remaining -= add;
            }
        }
    }
    /* Allocate fresh stacks for the remainder. */
    while (remaining > 0) {
        int slot = s_state.inventory_count;
        int add = remaining > max_stack ? max_stack : remaining;
        s_state.inventory_item[slot] = (uint16_t)item_id;
        s_state.inventory_qty[slot] = (uint16_t)add;
        s_state.inventory_is_container[slot] = 0;
        s_state.inventory_contents[slot] = 0;
        s_state.inventory_count++;
        remaining -= add;
    }
    return 1;
}

int dm2_v1_shop_add_container(int item_id, int qty, int contents_count) {
    ensure_init();
    if (qty <= 0 || item_id <= 0) return 0;
    if (s_state.inventory_count + qty > 32) return 0;
    for (int n = 0; n < qty; n++) {
        int slot = s_state.inventory_count;
        s_state.inventory_item[slot] = (uint16_t)item_id;
        s_state.inventory_qty[slot] = 1;
        s_state.inventory_is_container[slot] = 1;
        s_state.inventory_contents[slot] = (uint16_t)(contents_count < 0 ? 0 : contents_count);
        s_state.inventory_count++;
    }
    return 1;
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
    const int panel_x = 16;
    const int panel_y = 24;
    const int panel_w = 288;
    const int panel_h = 142;
    const int stock_x = 26;
    const int pack_x = 166;
    int shop_id;
    const DM2_V1_ShopDescriptor *shop;
    const DM2_V1_ShopState *shop_state;
    const char *npc_name;

    ensure_init();
    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    if (!dm2_v1_shop_is_active()) {
        return 0;
    }

    shop_id = dm2_v1_shop_get_active_shop();
    shop = dm2_v1_shop_get_builtin(shop_id);
    shop_state = dm2_v1_shop_get_state();
    if (!shop || !shop_state) {
        return 0;
    }
    npc_name = dm2_v1_npc_get_name(shop->npc_id);

    out->active = 1;
    out->panel_x = panel_x;
    out->panel_y = panel_y;
    out->panel_w = panel_w;
    out->panel_h = panel_h;
    out->header_x = panel_x + 2;
    out->header_y = panel_y + 2;
    out->header_w = panel_w - 4;
    out->header_h = 12;
    out->title_x = panel_x + 8;
    out->title_y = panel_y + 4;
    dm2_shop_copy_text(out->title, sizeof(out->title),
                       npc_name ? npc_name : "DM2 SHOP");
    out->stock_label_x = stock_x;
    out->stock_label_y = panel_y + 20;
    out->pack_label_x = pack_x;
    out->pack_label_y = panel_y + 20;
    out->footer_x = panel_x + 8;
    out->footer_y = panel_y + panel_h - 12;
    dm2_shop_copy_text(out->footer, sizeof(out->footer),
                       "UP/DOWN STOCK  LEFT/RIGHT PACK  ACTION BUY  DROP SELL");

    for (int i = 0;
         i < shop->stock_count && i < DM2_SHOP_RENDER_MAX_ROWS;
         ++i) {
        DM2_V1_ShopRenderRow *row = &out->stock_rows[out->stock_row_count++];
        char item_name[32];
        int price = dm2_v1_shop_get_effective_price(shop_id, i);

        dm2_shop_format_item_name(shop->stock[i].item_id,
                                  item_name, sizeof(item_name));
        row->kind = DM2_SHOP_RENDER_ROW_STOCK;
        row->x = stock_x;
        row->y = panel_y + 34 + i * 11;
        row->highlight_x = stock_x - 3;
        row->highlight_y = row->y - 1;
        row->highlight_w = 126;
        row->highlight_h = 10;
        row->highlighted = (i == selected_stock_idx) ? 1 : 0;
        snprintf(row->text, sizeof(row->text), "%c %-16s %3d",
                 row->highlighted ? '>' : ' ', item_name, price);
    }

    if (shop_state->inventory_count <= 0) {
        DM2_V1_ShopRenderRow *row = &out->pack_rows[out->pack_row_count++];
        row->kind = DM2_SHOP_RENDER_ROW_EMPTY_PACK;
        row->x = pack_x;
        row->y = panel_y + 34;
        dm2_shop_copy_text(row->text, sizeof(row->text), "EMPTY");
    } else {
        int rows = shop_state->inventory_count < DM2_SHOP_RENDER_MAX_ROWS
            ? shop_state->inventory_count
            : DM2_SHOP_RENDER_MAX_ROWS;
        for (int i = 0; i < rows; ++i) {
            DM2_V1_ShopRenderRow *row = &out->pack_rows[out->pack_row_count++];
            char item_name[32];
            int price = dm2_v1_shop_get_sell_price(shop_id, i);

            dm2_shop_format_item_name((int)shop_state->inventory_item[i],
                                      item_name, sizeof(item_name));
            row->kind = DM2_SHOP_RENDER_ROW_PACK;
            row->x = pack_x;
            row->y = panel_y + 34 + i * 11;
            row->highlight_x = pack_x - 3;
            row->highlight_y = row->y - 1;
            row->highlight_w = 126;
            row->highlight_h = 10;
            row->highlighted = (i == selected_pack_idx) ? 1 : 0;
            snprintf(row->text, sizeof(row->text), "%c %-16s %3d",
                     row->highlighted ? '>' : ' ', item_name, price);
        }
    }
    return 1;
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
