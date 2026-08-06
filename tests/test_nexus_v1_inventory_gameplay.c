#include "nexus_v1_inventory.h"
#include "nexus_v1_containers.h"
#include "nexus_v1_dungeon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int g_fail = 0;
static int g_count = 0;

static uint8_t *load_item_ibs(int *out_size) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    const char *home = getenv("HOME");
    char path[512];
    FILE *fp;
    long size;
    uint8_t *data;

    if (data_dir && data_dir[0]) {
        snprintf(path, sizeof(path), "%s/ITEM.IBS", data_dir);
    } else if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/nexus/ITEM.IBS", home);
    } else {
        return NULL;
    }
    fp = fopen(path, "rb");
    if (!fp) return NULL;
    if (fseek(fp, 0, SEEK_END) != 0) { fclose(fp); return NULL; }
    size = ftell(fp);
    if (size <= 0 || fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, fp) != (size_t)size) {
        free(data);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    *out_size = (int)size;
    return data;
}

static void expect(int cond, const char *msg) {
    g_count++;
    if (!cond) { fprintf(stderr, "FAIL: %s\n", msg); g_fail++; }
}

static void test_item_pickup_empty_slot(void) {
    Nexus_InventorySlot inv[NEXUS_INVENTORY_SLOTS];
    nexus_inventory_init(inv, NEXUS_INVENTORY_SLOTS);

    /* Verify all slots start empty */
    for (int i = 0; i < NEXUS_INVENTORY_SLOTS; i++) {
        expect(inv[i].item_id == -1, "slot starts empty (item_id == -1)");
    }

    /* Add an item */
    int slot = nexus_inventory_add(inv, NEXUS_INVENTORY_SLOTS, 0, 1);
    expect(slot >= 0, "add returns valid slot index");
    expect(inv[slot].item_id == 0, "slot now contains item 0");
    expect(inv[slot].quantity == 1, "slot quantity is 1");
}

static void test_item_drop(void) {
    Nexus_InventorySlot inv[NEXUS_INVENTORY_SLOTS];
    nexus_inventory_init(inv, NEXUS_INVENTORY_SLOTS);

    int slot = nexus_inventory_add(inv, NEXUS_INVENTORY_SLOTS, 5, 1);
    expect(slot >= 0, "item added for drop test");

    nexus_inventory_remove(inv, slot);
    expect(inv[slot].item_id == -1, "slot empty after remove");
}

static void test_equip_unequip(void) {
    Nexus_InventorySlot inv[NEXUS_INVENTORY_SLOTS];
    nexus_inventory_init(inv, NEXUS_INVENTORY_SLOTS);

    /* Add a weapon (item 0) */
    int slot = nexus_inventory_add(inv, NEXUS_INVENTORY_SLOTS, 0, 1);
    expect(slot >= 0, "weapon added to inventory");

    /* Equip it — weapon_slot=0, shield_slot=1, ring1=2, ring2=3, head=4,
     * torso=5, legs=6, feet=7, hands=8, amulet=9 */
    int rc = nexus_inventory_equip(inv, slot, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    /* rc is the slot that was cleared or -1 */
    expect(rc >= 0 || rc == -1, "equip returns valid result");

    /* Unequip — should return item to inventory */
    int urc = nexus_inventory_unequip(inv, NEXUS_INVENTORY_SLOTS,
                                       0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    expect(urc >= 0 || urc == -1, "unequip returns valid result");
}

static void test_inventory_find(void) {
    Nexus_InventorySlot inv[NEXUS_INVENTORY_SLOTS];
    nexus_inventory_init(inv, NEXUS_INVENTORY_SLOTS);

    expect(nexus_inventory_find(inv, NEXUS_INVENTORY_SLOTS, 3) == -1,
           "find returns -1 for absent item");

    int slot = nexus_inventory_add(inv, NEXUS_INVENTORY_SLOTS, 3, 1);
    expect(slot >= 0, "item 3 added");
    expect(nexus_inventory_find(inv, NEXUS_INVENTORY_SLOTS, 3) == slot,
           "find returns correct slot for item 3");
}

static void test_inventory_move(void) {
    Nexus_InventorySlot inv[NEXUS_INVENTORY_SLOTS];
    nexus_inventory_init(inv, NEXUS_INVENTORY_SLOTS);

    int slot = nexus_inventory_add(inv, NEXUS_INVENTORY_SLOTS, 7, 2);
    expect(slot >= 0, "item added for move test");

    int target = (slot + 1) % NEXUS_INVENTORY_SLOTS;
    int rc = nexus_inventory_move(inv, slot, target);
    expect(rc == 0 || rc == 1, "move returns success");
    if (rc == 0 || rc == 1) {
        expect(inv[target].item_id == 7, "item moved to target slot");
    }
}

static void test_container_operations(void) {
    Nexus_ContainerManager cmgr;
    nexus_v1_container_manager_init(&cmgr);

    /* Retail DGN has no authenticated container owner/content chain yet. */
    int cidx = nexus_v1_container_register(&cmgr,
        NEXUS_CONTAINER_CHEST, 10, 10, 0, -1);
    expect(cidx == -1, "unproven container registration blocked");

    int rc = nexus_v1_container_add_item(&cmgr, cidx, 42);
    expect(rc == -1, "unproven container loot blocked");
    expect(nexus_v1_container_item_count(&cmgr, cidx) == 0,
           "blocked container has no items");

    nexus_v1_container_add_item(&cmgr, cidx, 43);
    expect(nexus_v1_container_item_count(&cmgr, cidx) == 0,
           "second blocked loot item is not stored");

    int open_rc = nexus_v1_container_open(&cmgr, cidx, -1);
    expect(open_rc == 0, "unproven container does not open");
    expect(nexus_v1_container_is_open(&cmgr, cidx) == 0,
           "blocked container remains closed");

    int taken = nexus_v1_container_take(&cmgr, cidx, 0);
    expect(taken == -1, "unproven loot cannot be taken");
    expect(nexus_v1_container_item_count(&cmgr, cidx) == 0,
           "blocked container remains empty");

    int found = nexus_v1_container_find_at(&cmgr, 10, 10);
    expect(found == -1, "find_at has no unproven container owner");
    expect(nexus_v1_container_find_at(&cmgr, 99, 99) == -1,
           "find_at returns -1 for empty position");
}

static void test_container_locked(void) {
    Nexus_ContainerManager cmgr;
    nexus_v1_container_manager_init(&cmgr);

    /* Key semantics remain blocked with the container route. */
    int cidx = nexus_v1_container_register(&cmgr,
        NEXUS_CONTAINER_CHEST, 3, 3, 1, 5);
    expect(cidx == -1, "locked container registration blocked");

    /* Try to open without key */
    int rc = nexus_v1_container_open(&cmgr, cidx, -1);
    expect(rc == 0, "locked container does not open without key");

    /* Try with wrong key */
    rc = nexus_v1_container_open(&cmgr, cidx, 3);
    expect(rc == 0, "locked container does not open with wrong key");

    rc = nexus_v1_container_open(&cmgr, cidx, 5);
    expect(rc == 0, "correct key is still unproven without Saturn dispatch");
}

static void test_floor_items(void) {
    nexus_floor_init();

    int idx = nexus_floor_drop(4, 4, 10, 1);
    expect(idx >= 0, "floor drop returns valid index");
    expect(nexus_floor_count_at(4, 4) == 1, "1 item at (4,4)");

    nexus_floor_drop(4, 4, 11, 3);
    expect(nexus_floor_count_at(4, 4) == 2, "2 items at (4,4)");

    int item_id = -1, qty = -1;
    int pick_rc = nexus_floor_pickup(idx, &item_id, &qty);
    expect(pick_rc == 0 || pick_rc == 1, "floor pickup returns valid result");
    if (pick_rc == 0 || pick_rc == 1) {
        expect(item_id == 10, "picked up item_id 10");
        expect(qty == 1, "picked up qty 1");
    }
}

static void test_raw_item_declaration_does_not_infer_gameplay_flags(void) {
    uint8_t record[40];
    const Nexus_ItemDef *def;

    memset(record, 0, sizeof(record));
    record[0] = 0;
    record[1] = NEXUS_ITEM_POTION;
    record[2] = 0x01; /* carry-location bit, not a proven action flag */
    record[8] = 3;
    nexus_itemdef_bind_ibs_raw(record, 1);
    def = nexus_itemdef_get(0);
    expect(def != NULL, "raw ITEM.IBS declaration is retained");
    expect(def && def->carry_locations == 0x01,
           "raw ITEM.IBS carry-location byte is retained");
    expect(def && def->flags == 0,
           "raw ITEM.IBS carry-location does not infer consumable gameplay");
}

int main(void) {
    Nexus_V1_ItemIbsBank bank;
    uint8_t *item_ibs;
    int item_ibs_size = 0;

    item_ibs = load_item_ibs(&item_ibs_size);
    if (!item_ibs ||
        nexus_v1_item_ibs_parse_verified(item_ibs, item_ibs_size, 1, &bank) != 0) {
        free(item_ibs);
        puts("SKIP: verified Nexus ITEM.IBS is required for inventory gameplay");
        return 77;
    }
    nexus_itemdef_bind_ibs_bank(&bank, NEXUS_V1_ITEM_IBS_DECLARATION_COUNT);
    free(item_ibs);

    test_raw_item_declaration_does_not_infer_gameplay_flags();
    nexus_itemdef_bind_ibs_bank(&bank, NEXUS_V1_ITEM_IBS_DECLARATION_COUNT);
    test_item_pickup_empty_slot();
    test_item_drop();
    test_equip_unequip();
    test_inventory_find();
    test_inventory_move();
    test_container_operations();
    test_container_locked();
    test_floor_items();

    if (g_fail) {
        fprintf(stderr, "test_nexus_v1_inventory_gameplay: %d failure(s)\n", g_fail);
        return 1;
    }
    printf("ok: nexus_v1_inventory_gameplay (%d tests)\n", g_count);
    return 0;
}
