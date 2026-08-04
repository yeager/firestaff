#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_creature_combat_assess_pc34_compat.h"

/* ------------------------------------------------------------------ */
/* Mock callbacks                                                      */
/* ------------------------------------------------------------------ */

static uint16_t mock_get_creature_at(void *ctx, int16_t x, int16_t y)
{
    (void)ctx;
    /* Return a creature at (5, 5) only */
    if (x == 5 && y == 5) return 0x1000u;
    return 0xFFFFu;
}

static uint8_t mock_creature_record[16];

static uint8_t *mock_get_address_of_record(void *ctx, uint16_t handle)
{
    (void)ctx;
    if (handle == 0x1000u) return mock_creature_record;
    return NULL;
}

static uint16_t mock_get_next_record_link(void *ctx, uint16_t handle)
{
    (void)ctx;
    (void)handle;
    return 0xFFFEu; /* end of chain */
}

static int mock_creature_can_handle_it(void *ctx, uint16_t item_handle,
                                        int32_t type)
{
    (void)ctx;
    (void)item_handle;
    (void)type;
    return 1;
}

static int16_t mock_get_distinctive_itemtype(void *ctx, uint16_t handle)
{
    (void)ctx;
    (void)handle;
    return 3;
}

static int16_t mock_add_item_charge(void *ctx, uint16_t handle,
                                     int32_t amount)
{
    (void)ctx;
    (void)handle;
    (void)amount;
    return 5;
}

static int16_t mock_query_combat_stat(void *ctx, int32_t distinctive_type,
                                       int32_t creature_byte4,
                                       int32_t creature_word8,
                                       int32_t stat_type,
                                       int32_t param4, int32_t charge)
{
    (void)ctx;
    (void)distinctive_type;
    (void)creature_byte4;
    (void)creature_word8;
    (void)stat_type;
    (void)param4;
    (void)charge;
    return 10;
}

static int mock_is_container_chest(void *ctx, uint16_t handle)
{
    (void)ctx;
    (void)handle;
    return 0;
}

/* ------------------------------------------------------------------ */
/* creature_has_usable_item_ahead tests                                */
/* ------------------------------------------------------------------ */

static void test_has_usable_item_ahead_null_safety(void)
{
    DM2_V1_CreatureHasUsableItemAheadReceipt receipt;
    int r = dm2_v1_creature_has_usable_item_ahead(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_creature_has_usable_item_ahead(NULL, NULL);
    assert(r == 0);
    printf("  PASS: has_usable_item_ahead_null_safety\n");
}

static void test_has_usable_item_ahead_no_callbacks(void)
{
    DM2_V1_CreatureHasUsableItemAheadRequest req;
    DM2_V1_CreatureHasUsableItemAheadReceipt receipt;
    memset(&req, 0, sizeof(req));
    int r = dm2_v1_creature_has_usable_item_ahead(&req, &receipt);
    assert(r == 0);
    assert(receipt.valid == 1);
    assert(receipt.fail_closed == 1);
    printf("  PASS: has_usable_item_ahead_no_callbacks\n");
}

static void test_has_usable_item_ahead_no_creature(void)
{
    DM2_V1_CreatureHasUsableItemAheadRequest req;
    DM2_V1_CreatureHasUsableItemAheadReceipt receipt;
    uint8_t crec[16];
    int16_t dx[4] = {0, 1, 0, -1};
    int16_t dy[4] = {-1, 0, 1, 0};

    memset(&req, 0, sizeof(req));
    memset(crec, 0, sizeof(crec));
    /* facing north (dir=0) */
    crec[0x0f] = 0;
    req.creature_x = 10;
    req.creature_y = 10;
    req.direction_byte = 0xFF;
    req.creature_record = crec;
    req.dx_table = dx;
    req.dy_table = dy;
    req.get_creature_at = mock_get_creature_at;
    req.get_address_of_record = mock_get_address_of_record;
    req.get_next_record_link = mock_get_next_record_link;
    req.creature_can_handle_it = mock_creature_can_handle_it;

    int r = dm2_v1_creature_has_usable_item_ahead(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.result == 0);
    printf("  PASS: has_usable_item_ahead_no_creature\n");
}

/* ------------------------------------------------------------------ */
/* creature_assess_combat_stat tests                                   */
/* ------------------------------------------------------------------ */

static void test_assess_combat_stat_null_safety(void)
{
    DM2_V1_CreatureAssessCombatStatReceipt receipt;
    int r = dm2_v1_creature_assess_combat_stat(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_creature_assess_combat_stat(NULL, NULL);
    assert(r == 0);
    printf("  PASS: assess_combat_stat_null_safety\n");
}

static void test_assess_combat_stat_no_callbacks(void)
{
    DM2_V1_CreatureAssessCombatStatRequest req;
    DM2_V1_CreatureAssessCombatStatReceipt receipt;
    memset(&req, 0, sizeof(req));
    int r = dm2_v1_creature_assess_combat_stat(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: assess_combat_stat_no_callbacks\n");
}

static void test_assess_combat_stat_empty_chain(void)
{
    DM2_V1_CreatureAssessCombatStatRequest req;
    DM2_V1_CreatureAssessCombatStatReceipt receipt;

    memset(&req, 0, sizeof(req));
    memset(mock_creature_record, 0, sizeof(mock_creature_record));
    /* Set possession chain to 0xFFFE (empty) */
    mock_creature_record[2] = 0xFE;
    mock_creature_record[3] = 0xFF;

    req.creature_handle = 0x1000;
    req.direction_filter = 0xFF;
    req.creature_record = mock_creature_record;
    req.creature_can_handle_it = mock_creature_can_handle_it;
    req.get_distinctive_itemtype = mock_get_distinctive_itemtype;
    req.add_item_charge = mock_add_item_charge;
    req.query_combat_stat = mock_query_combat_stat;
    req.get_address_of_record = mock_get_address_of_record;
    req.get_next_record_link = mock_get_next_record_link;

    int r = dm2_v1_creature_assess_combat_stat(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.accumulated_score == -1); /* no items -> stays -1 */
    printf("  PASS: assess_combat_stat_empty_chain\n");
}

/* ------------------------------------------------------------------ */
/* count_items_in_chain tests                                          */
/* ------------------------------------------------------------------ */

static void test_count_items_null_safety(void)
{
    DM2_V1_CountItemsInChainReceipt receipt;
    int r = dm2_v1_count_items_in_chain(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_count_items_in_chain(NULL, NULL);
    assert(r == 0);
    printf("  PASS: count_items_null_safety\n");
}

static void test_count_items_no_callbacks(void)
{
    DM2_V1_CountItemsInChainRequest req;
    DM2_V1_CountItemsInChainReceipt receipt;
    memset(&req, 0, sizeof(req));
    int r = dm2_v1_count_items_in_chain(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: count_items_no_callbacks\n");
}

static void test_count_items_empty_chain(void)
{
    DM2_V1_CountItemsInChainRequest req;
    DM2_V1_CountItemsInChainReceipt receipt;

    memset(&req, 0, sizeof(req));
    req.first_handle = 0xFFFEu;
    req.direction_filter = 0xFF;
    req.get_address_of_record = mock_get_address_of_record;
    req.get_next_record_link = mock_get_next_record_link;
    req.creature_can_handle_it = mock_creature_can_handle_it;
    req.is_container_chest = mock_is_container_chest;

    int r = dm2_v1_count_items_in_chain(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.count == 0);
    printf("  PASS: count_items_empty_chain\n");
}

/* ------------------------------------------------------------------ */
/* ai_action_slot_resolve tests                                        */
/* ------------------------------------------------------------------ */

static void test_slot_resolve_null_safety(void)
{
    DM2_V1_AiActionSlotResolveReceipt receipt;
    int r = dm2_v1_ai_action_slot_resolve(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    r = dm2_v1_ai_action_slot_resolve(NULL, NULL);
    assert(r == 0);
    printf("  PASS: slot_resolve_null_safety\n");
}

static void test_slot_resolve_no_table(void)
{
    DM2_V1_AiActionSlotResolveRequest req;
    DM2_V1_AiActionSlotResolveReceipt receipt;
    memset(&req, 0, sizeof(req));
    int r = dm2_v1_ai_action_slot_resolve(&req, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);
    printf("  PASS: slot_resolve_no_table\n");
}

static void test_slot_resolve_allocates(void)
{
    DM2_V1_AiActionSlotResolveRequest req;
    DM2_V1_AiActionSlotResolveReceipt receipt;
    DM2_V1_AiActionSlotTable table;
    uint8_t hexe_entry[16];

    memset(&table, 0, sizeof(table));
    memset(hexe_entry, 0, sizeof(hexe_entry));
    memset(&req, 0, sizeof(req));

    req.hexe_entry = hexe_entry;
    req.action_type = 5;
    req.slot_table = &table;

    int r = dm2_v1_ai_action_slot_resolve(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.slot_ptr != NULL);
    assert(receipt.allocated_new == 1);
    printf("  PASS: slot_resolve_allocates\n");
}

static void test_slot_resolve_finds_existing(void)
{
    DM2_V1_AiActionSlotResolveRequest req;
    DM2_V1_AiActionSlotResolveReceipt receipt;
    DM2_V1_AiActionSlotTable table;
    uint8_t hexe_entry[16];

    memset(&table, 0, sizeof(table));
    memset(hexe_entry, 0, sizeof(hexe_entry));

    /* Pre-populate slot 0 with hexe_entry and action_type=5 */
    {
        uint8_t *ptr = hexe_entry;
        memcpy(&table.ai_action_slots[0], &ptr, sizeof(uint8_t *));
    }
    table.ai_action_slots[DM2_V1_AI_ACTION_SLOT_TYPE_OFF] = 5;

    memset(&req, 0, sizeof(req));
    req.hexe_entry = hexe_entry;
    req.action_type = 5;
    req.slot_table = &table;

    int r = dm2_v1_ai_action_slot_resolve(&req, &receipt);
    assert(r == 1);
    assert(receipt.valid == 1);
    assert(receipt.slot_ptr == table.ai_action_slots);
    assert(receipt.allocated_new == 0);
    printf("  PASS: slot_resolve_finds_existing\n");
}

static void test_slot_resolve_clears_on_flag(void)
{
    DM2_V1_AiActionSlotResolveRequest req;
    DM2_V1_AiActionSlotResolveReceipt receipt;
    DM2_V1_AiActionSlotTable table;
    uint8_t hexe_entry[16];

    memset(&table, 0xFF, sizeof(table));
    table.needs_clear = 1;
    memset(hexe_entry, 0, sizeof(hexe_entry));

    memset(&req, 0, sizeof(req));
    req.hexe_entry = hexe_entry;
    req.action_type = 3;
    req.slot_table = &table;

    int r = dm2_v1_ai_action_slot_resolve(&req, &receipt);
    assert(r == 1);
    assert(table.needs_clear == 0);
    assert(receipt.allocated_new == 1);
    printf("  PASS: slot_resolve_clears_on_flag\n");
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(void)
{
    printf("test_dm2_v1_creature_combat_assess_pc34_compat:\n");
    test_has_usable_item_ahead_null_safety();
    test_has_usable_item_ahead_no_callbacks();
    test_has_usable_item_ahead_no_creature();
    test_assess_combat_stat_null_safety();
    test_assess_combat_stat_no_callbacks();
    test_assess_combat_stat_empty_chain();
    test_count_items_null_safety();
    test_count_items_no_callbacks();
    test_count_items_empty_chain();
    test_slot_resolve_null_safety();
    test_slot_resolve_no_table();
    test_slot_resolve_allocates();
    test_slot_resolve_finds_existing();
    test_slot_resolve_clears_on_flag();
    printf("All creature combat assess tests passed.\n");
    return 0;
}
