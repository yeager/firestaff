#include "nexus_v1_inventory.h"
#include "nexus_v1_dungeon.h"
#include <string.h>

static int g_ibs_count;
static Nexus_ItemDef g_ibs_defs[NEXUS_V1_ITEM_IBS_DECLARATION_COUNT];

static uint16_t ibs_read_be16(const uint8_t *p) {
    return (uint16_t)((p[0] << 8) | p[1]);
}

void nexus_itemdef_clear_ibs_declarations(void) {
    g_ibs_count = 0;
    memset(g_ibs_defs, 0, sizeof(g_ibs_defs));
}

void nexus_itemdef_bind_ibs_raw(const uint8_t *data, int count) {
    int i;
    const uint8_t *rec;
    g_ibs_count = (!data || count <= 0) ? 0 :
        (count > NEXUS_V1_ITEM_IBS_DECLARATION_COUNT ?
            NEXUS_V1_ITEM_IBS_DECLARATION_COUNT : count);
    for (i = 0; i < g_ibs_count; ++i) {
        rec = data + i * 40;
        g_ibs_defs[i].name = NULL;
        g_ibs_defs[i].category = (Nexus_ItemCategory)rec[1];
        g_ibs_defs[i].carry_locations = rec[2];
        g_ibs_defs[i].ibs_flags = rec[3];
        g_ibs_defs[i].weight = rec[8];
        g_ibs_defs[i].attack = 0;
        g_ibs_defs[i].defense = 0;
        g_ibs_defs[i].flags = (rec[2] & 0x01) ? NEXUS_ITEMF_CONSUMABLE : 0;
        g_ibs_defs[i].action_id[0] = rec[16];
        g_ibs_defs[i].action_id[1] = rec[17];
        g_ibs_defs[i].action_id[2] = rec[18];
        g_ibs_defs[i].inv_image = ibs_read_be16(rec + 20);
        g_ibs_defs[i].floor_image = ibs_read_be16(rec + 22);
        g_ibs_defs[i].name_string = ibs_read_be16(rec + 24);
        g_ibs_defs[i].desc_string = ibs_read_be16(rec + 26);
        g_ibs_defs[i].action_string[0] = ibs_read_be16(rec + 28);
        g_ibs_defs[i].action_string[1] = ibs_read_be16(rec + 30);
        g_ibs_defs[i].action_string[2] = ibs_read_be16(rec + 32);
        g_ibs_defs[i].attribute = ibs_read_be16(rec + 36);
    }
}

void nexus_itemdef_bind_ibs_declarations(const uint8_t *category,
                                         const uint8_t *weight,
                                         const uint16_t *name_string,
                                         const uint16_t *desc_string,
                                         const uint16_t *action1_string,
                                         const uint16_t *action2_string,
                                         const uint16_t *action3_string,
                                         int count) {
    int i;
    g_ibs_count = (category && weight && count > 0) ?
        (count > NEXUS_V1_ITEM_IBS_DECLARATION_COUNT ?
            NEXUS_V1_ITEM_IBS_DECLARATION_COUNT : count) : 0;
    for (i = 0; i < g_ibs_count; ++i) {
        memset(&g_ibs_defs[i], 0, sizeof(g_ibs_defs[i]));
        g_ibs_defs[i].category = category[i] < NEXUS_ITEM_COUNT ?
            (Nexus_ItemCategory)category[i] : NEXUS_ITEM_COUNT;
        g_ibs_defs[i].weight = weight[i];
        g_ibs_defs[i].name_string = name_string ? name_string[i] : 0xffffU;
        g_ibs_defs[i].desc_string = desc_string ? desc_string[i] : 0xffffU;
        g_ibs_defs[i].action_string[0] = action1_string ? action1_string[i] : 0xffffU;
        g_ibs_defs[i].action_string[1] = action2_string ? action2_string[i] : 0xffffU;
        g_ibs_defs[i].action_string[2] = action3_string ? action3_string[i] : 0xffffU;
        g_ibs_defs[i].inv_image = 0xffffU;
        g_ibs_defs[i].floor_image = 0xffffU;
    }
}

void nexus_itemdef_bind_ibs_bank(const void *bank_ptr, int count) {
    const Nexus_V1_ItemIbsBank *bank = (const Nexus_V1_ItemIbsBank *)bank_ptr;
    int i;
    if (!bank || count <= 0) return;
    g_ibs_count = count > NEXUS_V1_ITEM_IBS_DECLARATION_COUNT ?
        NEXUS_V1_ITEM_IBS_DECLARATION_COUNT : count;
    for (i = 0; i < g_ibs_count; ++i) {
        memset(&g_ibs_defs[i], 0, sizeof(g_ibs_defs[i]));
        g_ibs_defs[i].category = bank->item_category[i] < NEXUS_ITEM_CAT_MAX ?
            (Nexus_ItemCategory)bank->item_category[i] : NEXUS_ITEM_CAT_MAX;
        g_ibs_defs[i].carry_locations = bank->item_carry_locations[i];
        g_ibs_defs[i].ibs_flags = bank->item_ibs_flags[i];
        g_ibs_defs[i].weight = bank->item_weight[i];
        /* DMWeb defines byte 2 as carry-location bits. It does not prove the
         * live consumable/equipment effect ABI, so do not reinterpret one of
         * those bits as a gameplay flag. ITEM.IBS remains declaration data
         * until the Saturn action dispatcher is bound. */
        g_ibs_defs[i].flags = 0;
        g_ibs_defs[i].action_id[0] = bank->item_action_id[i][0];
        g_ibs_defs[i].action_id[1] = bank->item_action_id[i][1];
        g_ibs_defs[i].action_id[2] = bank->item_action_id[i][2];
        g_ibs_defs[i].inv_image = bank->inventory_association[i];
        g_ibs_defs[i].floor_image = bank->floor_image[i];
        g_ibs_defs[i].name_string = bank->item_name_string[i];
        g_ibs_defs[i].desc_string = bank->item_desc_string[i];
        g_ibs_defs[i].action_string[0] = bank->item_action1_string[i];
        g_ibs_defs[i].action_string[1] = bank->item_action2_string[i];
        g_ibs_defs[i].action_string[2] = bank->item_action3_string[i];
        g_ibs_defs[i].attribute = bank->item_attribute[i];
    }
}

int nexus_itemdef_count(void) {
    return g_ibs_count;
}

const Nexus_ItemDef *nexus_itemdef_get(int id) {
    if (id >= 0 && id < g_ibs_count) return &g_ibs_defs[id];
    return NULL;
}

const char *nexus_itemdef_category_name(Nexus_ItemCategory cat) {
    switch (cat) {
    case NEXUS_ITEM_WEAPON: return "Weapon";
    case NEXUS_ITEM_ARMOUR: return "Armour";
    case NEXUS_ITEM_FOOD:   return "Food";
    case NEXUS_ITEM_POTION: return "Potion";
    case NEXUS_ITEM_SCROLL: return "Scroll";
    case NEXUS_ITEM_MISC:   return "Misc";
    default: return "Unknown";
    }
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
    int target_slot = -1;
    Nexus_InventorySlot old_item = { .item_id = -1, .quantity = 0 };
    (void)ring1_slot;
    (void)ring2_slot;
    (void)legs_slot;
    (void)amulet_slot;

    if (!inv || slot < 0) return -1;

    def = nexus_inventory_get(inv, slot);
    if (!def) return -1;
    if (!(def->flags & NEXUS_ITEMF_EQUIPPABLE)) return -1;

    /* Determine target slot based on item category */
    if (def->attack > 0 && def->defense == 0) {
        /* Weapon */
        target_slot = weapon_slot;
        old_item = inv[weapon_slot];
        inv[weapon_slot] = inv[slot];
    } else if (def->defense > 0 && def->attack == 0) {
        /* ITEM.IBS does not prove the Saturn armor-slot dispatcher. The old
         * implementation guessed slots from inherited DM1 item IDs 20..26
         * and sent unknown armor to torso; keep this route fail-closed until
         * a source-bound action/slot capture exists. */
        (void)shield_slot;
        (void)head_slot;
        (void)torso_slot;
        (void)feet_slot;
        (void)hands_slot;
        return -1;
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
    (void)weapon_slot;
    (void)shield_slot;
    (void)ring1_slot;
    (void)ring2_slot;
    (void)head_slot;
    (void)torso_slot;
    (void)legs_slot;
    (void)feet_slot;
    (void)hands_slot;
    (void)amulet_slot;

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
 * Simple legacy storage, separate from the DMWeb 64x64 DGN Structure1B map.
 * Source: nexus_squares.md (floor item management), DM1 floor items
 * ═══════════════════════════════════════════════════════════════════ */

#define FLOOR_GRID   NEXUS_MAX_MAP_SIZE
#define FLOOR_PER_CELL 8
#define FLOOR_MAX_ITEMS (FLOOR_GRID * FLOOR_GRID * FLOOR_PER_CELL)

static Nexus_FloorItem g_floor_items[FLOOR_MAX_ITEMS];
static int g_floor_item_count = 0;

void nexus_floor_init(void) {
    g_floor_item_count = 0;
    memset(g_floor_items, 0, sizeof(g_floor_items));
}

int nexus_floor_drop(int x, int y, int item_id, int qty) {
    if (x < 0 || x >= FLOOR_GRID || y < 0 || y >= FLOOR_GRID) return -1;
    if (g_floor_item_count >= FLOOR_MAX_ITEMS) return -1;
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
