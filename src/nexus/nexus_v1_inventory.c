#include "nexus_v1_inventory.h"
#include <string.h>
#include <stdio.h>

/* ═══════════════════════════════════════════════════════════════════
 * Global item catalog — Nexus uses DM1 item roster (source-locked
 * via firestaff_item_encyclopedia.c, cross-referenced with
 * nexus_v1_creatures.c weapon_power/defense in combat formula).
 * All stats match DM1. No Saturn-specific items confirmed.
 * ═══════════════════════════════════════════════════════════════════ */

/* clang-format off */
const Nexus_ItemDef g_nexus_items[] = {
    /* Weapons (attack > 0, defense = 0) */
    [0]  = {"Falchion",      NEXUS_ITEM_WEAPON,    18, 30, 0,  NEXUS_ITEMF_EQUIPPABLE},
    [1]  = {"Rapier",        NEXUS_ITEM_WEAPON,    14, 24, 4,  NEXUS_ITEMF_EQUIPPABLE},
    [2]  = {"Mace",          NEXUS_ITEM_WEAPON,    30, 32, 0,  NEXUS_ITEMF_EQUIPPABLE},
    [3]  = {"Club",          NEXUS_ITEM_WEAPON,    20, 16, 0,  NEXUS_ITEMF_EQUIPPABLE},
    [4]  = {"Staff",         NEXUS_ITEM_WEAPON,    12, 10, 2,  NEXUS_ITEMF_EQUIPPABLE},
    [5]  = {"Sword",         NEXUS_ITEM_WEAPON,    22, 34, 2,  NEXUS_ITEMF_EQUIPPABLE},
    [6]  = {"Axe",           NEXUS_ITEM_WEAPON,    26, 36, 0,  NEXUS_ITEMF_EQUIPPABLE},
    [7]  = {"Dagger",        NEXUS_ITEM_WEAPON,     6, 14, 0,  NEXUS_ITEMF_EQUIPPABLE},
    [8]  = {"Arrow",         NEXUS_ITEM_WEAPON,     1, 10, 0,  NEXUS_ITEMF_STACKABLE},
    [9]  = {"Slayer",        NEXUS_ITEM_WEAPON,    28, 50, 6,  NEXUS_ITEMF_EQUIPPABLE},
    [10] = {"Vorpal Blade",  NEXUS_ITEM_WEAPON,    20, 48, 4,  NEXUS_ITEMF_EQUIPPABLE},
    [11] = {"Firestaff",     NEXUS_ITEM_WEAPON,    16, 40, 10, NEXUS_ITEMF_EQUIPPABLE},

    /* Armor (attack = 0, defense > 0) */
    [20] = {"Leather Jerkin", NEXUS_ITEM_ARMOR,     8,  0, 8,  NEXUS_ITEMF_EQUIPPABLE},
    [21] = {"Mail Aketon",   NEXUS_ITEM_ARMOR,    24,  0, 14, NEXUS_ITEMF_EQUIPPABLE},
    [22] = {"Plate Armor",   NEXUS_ITEM_ARMOR,    40,  0, 22, NEXUS_ITEMF_EQUIPPABLE},
    [23] = {"Shield",         NEXUS_ITEM_ARMOR,    16,  0, 12, NEXUS_ITEMF_EQUIPPABLE},
    [24] = {"Helmet",         NEXUS_ITEM_ARMOR,    10,  0, 6,  NEXUS_ITEMF_EQUIPPABLE},
    [25] = {"Boots",          NEXUS_ITEM_ARMOR,     6,  0, 4,  NEXUS_ITEMF_EQUIPPABLE},
    [26] = {"Gauntlets",      NEXUS_ITEM_ARMOR,     8,  0, 5,  NEXUS_ITEMF_EQUIPPABLE},

    /* Potions (consumable) */
    [30] = {"Health Potion",  NEXUS_ITEM_POTION,    2,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},
    [31] = {"Mana Potion",    NEXUS_ITEM_POTION,    2,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},
    [32] = {"Stamina Potion", NEXUS_ITEM_POTION,    2,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},
    [33] = {"Antidote",       NEXUS_ITEM_POTION,    2,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},

    /* Scrolls (consumable spell container — from CSB) */
    [40] = {"Scroll",         NEXUS_ITEM_SCROLL,    1,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},

    /* Containers */
    [50] = {"Chest",          NEXUS_ITEM_CONTAINER, 10, 0, 0,  NEXUS_ITEMF_NO_DROP},
    [51] = {"Sack",           NEXUS_ITEM_CONTAINER,  2, 0, 0,  NEXUS_ITEMF_STACKABLE},

    /* Misc */
    [60] = {"Torch",          NEXUS_ITEM_MISC,      6,  8, 0,  NEXUS_ITEMF_STACKABLE},
    [61] = {"Compass",        NEXUS_ITEM_MISC,       2,  0, 0,  0},
    [62] = {"Rabbit Foot",    NEXUS_ITEM_MISC,       1,  0, 0,  0},
    [63] = {"Corn",           NEXUS_ITEM_MISC,       3,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},
    [64] = {"Water Flask",    NEXUS_ITEM_MISC,       4,  0, 0,  NEXUS_ITEMF_CONSUMABLE|NEXUS_ITEMF_STACKABLE},
    [65] = {"Rope",           NEXUS_ITEM_MISC,       8,  0, 0,  0},
    [66] = {"Key",            NEXUS_ITEM_MISC,       1,  0, 0,  NEXUS_ITEMF_KEY},

    /* Keys (specific types) */
    [70] = {"Gold Key",       NEXUS_ITEM_KEY,        1,  0, 0,  NEXUS_ITEMF_KEY},
    [71] = {"Silver Key",     NEXUS_ITEM_KEY,        1,  0, 0,  NEXUS_ITEMF_KEY},
    [72] = {"Skeleton Key",   NEXUS_ITEM_KEY,        1,  0, 0,  NEXUS_ITEMF_KEY},

    /* Spell Runes (alignment-coded, used to cast spells) */
    [80] = {"Rune of Fire",   NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},
    [81] = {"Rune of Ice",    NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},
    [82] = {"Rune of Light",  NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},
    [83] = {"Rune of Dark",   NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},
    [84] = {"Rune of Earth",  NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},
    [85] = {"Rune of Wind",   NEXUS_ITEM_SPELL_RUNE, 2, 0, 0, 0},

    /* Sentinel */
    [255] = {NULL, NEXUS_ITEM_COUNT, 0, 0, 0, 0}
};
/* clang-format on */

#define ITEM_COUNT ((int)(sizeof(g_nexus_items)/sizeof(g_nexus_items[0])))

int nexus_itemdef_count(void) {
    int count = 0;
    for (int i = 0; i < ITEM_COUNT; i++)
        if (g_nexus_items[i].name) count++;
    return count;
}

const Nexus_ItemDef *nexus_itemdef_get(int id) {
    if (id < 0 || id >= ITEM_COUNT) return NULL;
    return g_nexus_items[id].name ? &g_nexus_items[id] : NULL;
}

const char *nexus_itemdef_category_name(Nexus_ItemCategory cat) {
    static const char *names[] = {
        "Weapon", "Armor", "Potion", "Scroll", "Container", "Misc", "Key", "Rune"
    };
    if (cat < 0 || cat >= NEXUS_ITEM_COUNT) return "Unknown";
    return names[cat];
}

/* ═══════════════════════════════════════════════════════════════════
 * Inventory management
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_inventory_init(Nexus_InventorySlot *inv, int count) {
    int i;
    if (!inv) return;
    for (i = 0; i < count; i++) {
        inv[i].item_id = -1;
        inv[i].quantity = 0;
    }
}

const Nexus_ItemDef *nexus_inventory_get(const Nexus_InventorySlot *inv, int slot) {
    if (!inv || slot < 0) return NULL;
    if (inv[slot].item_id < 0) return NULL;
    return nexus_itemdef_get(inv[slot].item_id);
}

int nexus_inventory_add(Nexus_InventorySlot *inv, int count, int item_id, int qty) {
    int i;
    const Nexus_ItemDef *def;
    if (!inv || item_id < 0) return -1;

    def = nexus_itemdef_get(item_id);
    if (!def) return -1;

    /* Stackable: find existing slot with same item */
    if (def->flags & NEXUS_ITEMF_STACKABLE) {
        for (i = 0; i < count; i++) {
            if (inv[i].item_id == item_id) {
                inv[i].quantity += qty;
                return i;
            }
        }
    }

    /* Find first empty slot */
    for (i = 0; i < count; i++) {
        if (inv[i].item_id < 0) {
            inv[i].item_id = item_id;
            inv[i].quantity = qty;
            return i;
        }
    }

    return -1; /* no space */
}

void nexus_inventory_remove(Nexus_InventorySlot *inv, int slot) {
    if (!inv || slot < 0) return;
    inv[slot].item_id = -1;
    inv[slot].quantity = 0;
}

int nexus_inventory_move(Nexus_InventorySlot *inv, int from, int to) {
    Nexus_InventorySlot tmp;
    if (!inv || from < 0 || to < 0) return -1;
    if (from == to) return 0;
    if (inv[to].item_id < 0) {
        /* Target empty: simple move */
        inv[to] = inv[from];
        inv[from].item_id = -1;
        inv[from].quantity = 0;
        return 0;
    } else {
        /* Swap */
        tmp = inv[to];
        inv[to] = inv[from];
        inv[from] = tmp;
        return 0;
    }
}

int nexus_inventory_weight(const Nexus_InventorySlot *inv, int count) {
    int total = 0, i;
    if (!inv) return 0;
    for (i = 0; i < count; i++) {
        const Nexus_ItemDef *def = nexus_inventory_get(inv, i);
        if (def) total += def->weight * inv[i].quantity;
    }
    return total;
}

int nexus_inventory_count_category(const Nexus_InventorySlot *inv, int count, Nexus_ItemCategory cat) {
    int i, n = 0;
    if (!inv) return 0;
    for (i = 0; i < count; i++) {
        const Nexus_ItemDef *def = nexus_inventory_get(inv, i);
        if (def && def->category == cat) n++;
    }
    return n;
}

int nexus_inventory_find(const Nexus_InventorySlot *inv, int count, int item_id) {
    int i;
    if (!inv || item_id < 0) return -1;
    for (i = 0; i < count; i++) {
        if (inv[i].item_id == item_id) return i;
    }
    return -1;
}

/* Equip item from inventory slot to appropriate equipment slot.
 * Returns slot that was cleared (old item goes to inventory) or -1. */
int nexus_inventory_equip(Nexus_InventorySlot *inv, int slot,
                           int weapon_slot, int shield_slot,
                           int ring1_slot, int ring2_slot,
                           int head_slot, int torso_slot,
                           int legs_slot, int feet_slot, int hands_slot,
                           int amulet_slot) {
    const Nexus_ItemDef *def;
    int item_id;
    int target_slot = -1;
    Nexus_InventorySlot old_item = { .item_id = -1, .quantity = 0 };

    if (!inv || slot < 0) return -1;

    def = nexus_inventory_get(inv, slot);
    if (!def) return -1;
    item_id = (int)(def - g_nexus_items);

    if (!(def->flags & NEXUS_ITEMF_EQUIPPABLE)) return -1;

    /* Determine target slot based on item category */
    if (def->attack > 0 && def->defense == 0) {
        /* Weapon */
        target_slot = weapon_slot;
        old_item = inv[weapon_slot];
        inv[weapon_slot] = inv[slot];
    } else if (def->defense > 0 && def->attack == 0) {
        /* Armor — map to specific slot */
        /* Armor — map to specific slot by item ID.
         * DO NOT use pointer arithmetic (def - g_nexus_items) — use item_id.
         * Fix: item_id range mapping replaces broken pointer subtraction.
         * Source: fix. */
        if (item_id >= 20 && item_id <= 22) {
            /* Leather Jerkin, Mail Aketon, Plate Armor → torso */
            target_slot = torso_slot; old_item = inv[torso_slot];
        } else if (item_id == 23) {
            /* Shield → shield slot */
            target_slot = shield_slot; old_item = inv[shield_slot];
        } else if (item_id == 24) {
            /* Helmet → head slot */
            target_slot = head_slot; old_item = inv[head_slot];
        } else if (item_id == 25) {
            /* Boots → feet slot */
            target_slot = feet_slot; old_item = inv[feet_slot];
        } else if (item_id == 26) {
            /* Gauntlets → hands slot */
            target_slot = hands_slot; old_item = inv[hands_slot];
        } else {
            /* Unknown armor → torso */
            target_slot = torso_slot; old_item = inv[torso_slot];
        }
        inv[target_slot] = inv[slot];
    } else {
        return -1; /* not equippable as armor */
    }

    inv[slot].item_id = -1;
    inv[slot].quantity = 0;

    /* Put old equipped item back into inventory (slot we came from) */
    if (old_item.item_id >= 0) {
        inv[slot] = old_item;
    }

    return target_slot;
}

int nexus_inventory_unequip(Nexus_InventorySlot *inv, int count,
                             int equip_slot,
                             int weapon_slot, int shield_slot,
                             int ring1_slot, int ring2_slot,
                             int head_slot, int torso_slot,
                             int legs_slot, int feet_slot, int hands_slot,
                             int amulet_slot) {
    int i;
    Nexus_InventorySlot item;

    if (!inv || equip_slot < 0) return -1;

    item = inv[equip_slot];
    if (item.item_id < 0) return -1; /* nothing equipped there */

    /* Find first open inventory slot */
    for (i = 0; i < count; i++) {
        if (inv[i].item_id < 0) {
            inv[i] = item;
            inv[equip_slot].item_id = -1;
            inv[equip_slot].quantity = 0;
            return i;
        }
    }
    return -1; /* inventory full */
}

/* ═══════════════════════════════════════════════════════════════════
 * Cursor management
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_cursor_clear(Nexus_CursorItem *cursor) {
    if (!cursor) return;
    cursor->item_id = -1;
    cursor->quantity = 0;
    cursor->source_type = 0;
    cursor->source_idx = -1;
}

int nexus_cursor_can_pickup(const Nexus_CursorItem *cursor) {
    return cursor && cursor->item_id < 0;
}

int nexus_cursor_pickup(Nexus_CursorItem *cursor, int item_id, int qty,
                         int source_type, int source_idx) {
    if (!cursor || item_id < 0) return -1;
    if (cursor->item_id >= 0) return -1; /* already holding something */
    cursor->item_id = item_id;
    cursor->quantity = qty;
    cursor->source_type = source_type;
    cursor->source_idx = source_idx;
    return 0;
}

int nexus_cursor_place(Nexus_CursorItem *cursor) {
    int item_id;
    if (!cursor || cursor->item_id < 0) return -1;
    item_id = cursor->item_id;
    cursor->item_id = -1;
    cursor->quantity = 0;
    cursor->source_type = 0;
    cursor->source_idx = -1;
    return item_id;
}

/* ═══════════════════════════════════════════════════════════════════
 * Floor items (items dropped on dungeon floor)
 * Simple zone-based storage — 32x32 grid with per-cell lists.
 * Source: nexus_squares.md (floor item management), DM1 floor items
 * ═══════════════════════════════════════════════════════════════════ */

#define FLOOR_GRID   32
#define FLOOR_PER_CELL 8

static Nexus_FloorItem g_floor_items[256];  /* 32x32 simplified grid */
static int g_floor_item_count = 0;

void nexus_floor_init(void) {
    g_floor_item_count = 0;
    memset(g_floor_items, 0, sizeof(g_floor_items));
}

int nexus_floor_drop(int x, int y, int item_id, int qty) {
    if (x < 0 || x >= FLOOR_GRID || y < 0 || y >= FLOOR_GRID) return -1;
    if (g_floor_item_count >= 256) return -1;
    g_floor_items[g_floor_item_count].item_id = item_id;
    g_floor_items[g_floor_item_count].quantity = qty;
    g_floor_items[g_floor_item_count].x = x;
    g_floor_items[g_floor_item_count].y = y;
    g_floor_items[g_floor_item_count].on_ground = 1;
    return g_floor_item_count++;
}

int nexus_floor_pickup(int floor_idx, int *out_item_id, int *out_qty) {
    if (!out_item_id || !out_qty) return -1;
    if (floor_idx < 0 || floor_idx >= g_floor_item_count) return -1;
    *out_item_id = g_floor_items[floor_idx].item_id;
    *out_qty = g_floor_items[floor_idx].quantity;
    /* Remove by swapping with last */
    g_floor_items[floor_idx] = g_floor_items[--g_floor_item_count];
    return 0;
}

int nexus_floor_count_at(int x, int y) {
    int i, c = 0;
    for (i = 0; i < g_floor_item_count; i++)
        if (g_floor_items[i].x == x && g_floor_items[i].y == y) c++;
    return c;
}

int nexus_floor_get_at(int x, int y, int idx, int *out_item_id, int *out_qty) {
    int i, c = 0;
    for (i = 0; i < g_floor_item_count; i++) {
        if (g_floor_items[i].x == x && g_floor_items[i].y == y) {
            if (c == idx) {
                if (out_item_id) *out_item_id = g_floor_items[i].item_id;
                if (out_qty) *out_qty = g_floor_items[i].quantity;
                return i;
            }
            c++;
        }
    }
    return -1;
}