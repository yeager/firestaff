/*
 * DM2 V1 inventory/item panel regression gate.
 *
 * Data-free coverage for a narrow item-panel snapshot: selected inventory
 * slot, action-hand/equipment slot labels, leader-hand object display, and
 * DB-backed object description resolution.
 *
 * Source anchors:
 *   ReDMCSB DEFS.H:779-810 slot indices.
 *   ReDMCSB PANEL.C:1127-1200 object-description panel route.
 *   ReDMCSB PANEL.C:1658-1692 action-hand item panel route.
 *   ReDMCSB CHAMPION.C:250-268/270-282 leader-hand state.
 *   ReDMCSB LOADSAVE.C:1535-1537/2744 leader-hand persistence.
 */

#include "dm2_v1_inventory_panel.h"
#include "dm2_v1_object_model.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { ++passed; printf("  PASS: %s\n", msg); } \
    else { ++failed; printf("  FAIL: %s\n", msg); } \
} while (0)

static void test_slot_names_and_equipment_flags(void)
{
    CHECK(strcmp(dm2_v1_inventory_slot_label(DM2_V1_INV_SLOT_LEADER_HAND),
                 "leader_hand") == 0,
          "leader hand label stable");
    CHECK(strcmp(dm2_v1_inventory_slot_label(DM2_V1_INV_SLOT_ACTION_HAND),
                 "action_hand") == 0,
          "action hand label stable");
    CHECK(strcmp(dm2_v1_inventory_slot_label(DM2_V1_INV_SLOT_HEAD),
                 "head") == 0,
          "head equipment label stable");
    CHECK(strcmp(dm2_v1_inventory_slot_label(DM2_V1_INV_SLOT_BACKPACK_FIRST),
                 "backpack_line1_1") == 0,
          "first backpack label stable");
    CHECK(strcmp(dm2_v1_inventory_slot_label(30), "invalid") == 0,
          "out-of-range slot label is invalid");

    CHECK(dm2_v1_inventory_slot_is_equipment(DM2_V1_INV_SLOT_LEADER_HAND),
          "leader hand is equipment display");
    CHECK(dm2_v1_inventory_slot_is_equipment(DM2_V1_INV_SLOT_ACTION_HAND),
          "action hand is equipment display");
    CHECK(dm2_v1_inventory_slot_is_equipment(DM2_V1_INV_SLOT_HEAD),
          "head slot is equipment display");
    CHECK(dm2_v1_inventory_slot_is_equipment(DM2_V1_INV_SLOT_NECK),
          "neck slot is equipment display");
    CHECK(!dm2_v1_inventory_slot_is_equipment(DM2_V1_INV_SLOT_BACKPACK_FIRST),
          "backpack slot is not equipment display");
}

static void test_inventory_selection_and_description(void)
{
    DM2_ChampionRecord champ;
    DM2_LeaderPossession leader_hand;
    DM2_DB_State db;
    DM2_V1_InventoryPanelItemView view;
    unsigned char weapon_records[8] = {0};
    unsigned char cloth_records[4] = {0};
    unsigned char misc_records[16] = {0};
    uint32_t powerblade;
    uint32_t helm;
    uint32_t tech_key;
    DM2_V1_InventoryPanelDescription descriptions[3];

    memset(&champ, 0, sizeof(champ));
    memset(&leader_hand, 0, sizeof(leader_hand));
    memset(&db, 0, sizeof(db));

    db.pools[DM2_DB_WEAPON].data = weapon_records;
    db.pools[DM2_DB_WEAPON].rec_count = 2;
    db.pools[DM2_DB_WEAPON].rec_size = 4;
    db.pools[DM2_DB_CLOTH].data = cloth_records;
    db.pools[DM2_DB_CLOTH].rec_count = 1;
    db.pools[DM2_DB_CLOTH].rec_size = 4;
    db.pools[DM2_DB_MISC].data = misc_records;
    db.pools[DM2_DB_MISC].rec_count = 4;
    db.pools[DM2_DB_MISC].rec_size = 4;

    powerblade = dm2_db_make_handle(DM2_DB_WEAPON, 1);
    helm = dm2_db_make_handle(DM2_DB_CLOTH, 0);
    tech_key = dm2_db_make_handle(DM2_DB_MISC, 3);

    champ.inventory[DM2_V1_INV_SLOT_ACTION_HAND] = powerblade;
    champ.inventory[DM2_V1_INV_SLOT_HEAD] = helm;
    leader_hand.object = tech_key;

    descriptions[0].object_id = powerblade;
    descriptions[0].description = "Ven powerblade";
    descriptions[1].object_id = helm;
    descriptions[1].description = "Clan helm";
    descriptions[2].object_id = tech_key;
    descriptions[2].description = "Tech key";

    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_ACTION_HAND,
              &db, descriptions, 3, &view),
          "action-hand selection succeeds");
    CHECK(view.selected_slot == DM2_V1_INV_SLOT_ACTION_HAND,
          "selected slot recorded");
    CHECK(view.object_id == powerblade,
          "action-hand object handle selected");
    CHECK(view.has_object == 1 && view.db_resolved == 1,
          "action-hand object resolved through DB helper");
    CHECK(view.db_pool == DM2_DB_WEAPON && view.db_index == 1,
          "action-hand DB pool/index preserved");
    CHECK(strcmp(view.description, "Ven powerblade") == 0,
          "action-hand description displayed from catalog");

    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_HEAD,
              &db, descriptions, 3, &view),
          "head-slot selection succeeds");
    CHECK(view.object_id == helm,
          "head-slot object handle selected");
    CHECK(view.db_pool == DM2_DB_CLOTH && view.db_index == 0,
          "head-slot equipment DB pool/index preserved");
    CHECK(strcmp(view.description, "Clan helm") == 0,
          "head-slot equipment description displayed");

    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_LEADER_HAND,
              &db, descriptions, 3, &view),
          "leader-hand selection succeeds");
    CHECK(view.object_id == tech_key,
          "leader-hand object handle selected");
    CHECK(view.db_pool == DM2_DB_MISC && view.db_index == 3,
          "leader-hand DB pool/index preserved");
    CHECK(strcmp(view.description, "Tech key") == 0,
          "leader-hand description displayed");
}

static void test_empty_invalid_and_unresolved_slots(void)
{
    DM2_ChampionRecord champ;
    DM2_LeaderPossession leader_hand;
    DM2_DB_State db;
    DM2_V1_InventoryPanelItemView view;

    memset(&champ, 0, sizeof(champ));
    memset(&leader_hand, 0, sizeof(leader_hand));
    memset(&db, 0, sizeof(db));

    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_BACKPACK_FIRST,
              &db, NULL, 0, &view),
          "empty backpack selection succeeds");
    CHECK(view.has_object == 0 && view.object_id == 0,
          "empty backpack reports no object");
    CHECK(strcmp(view.description, "EMPTY") == 0,
          "empty backpack description stable");

    CHECK(!dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, -2, &db, NULL, 0, &view),
          "slot below leader-hand sentinel rejected");
    CHECK(!dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_COUNT, &db, NULL, 0, &view),
          "slot at inventory count rejected");
    CHECK(!dm2_v1_inventory_panel_select_item(
              NULL, &leader_hand, DM2_V1_INV_SLOT_ACTION_HAND,
              &db, NULL, 0, &view),
          "non-leader slot rejects NULL champion");
    CHECK(dm2_v1_inventory_panel_select_item(
              NULL, &leader_hand, DM2_V1_INV_SLOT_LEADER_HAND,
              &db, NULL, 0, &view),
          "leader-hand slot accepts NULL champion");

    champ.inventory[DM2_V1_INV_SLOT_NECK] =
        dm2_db_make_handle(DM2_DB_SCROLL, 9);
    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, &leader_hand, DM2_V1_INV_SLOT_NECK,
              &db, NULL, 0, &view),
          "unresolved neck-slot selection succeeds");
    CHECK(view.has_object == 1 && view.db_resolved == 0,
          "unresolved neck-slot object does not fake DB resolution");
    CHECK(strcmp(view.description, "UNRESOLVED") == 0,
          "unresolved neck-slot description stable");
}

static void test_source_evidence(void)
{
    const char *e = dm2_v1_inventory_panel_source_evidence();
    CHECK(e != NULL && strstr(e, "DEFS.H:779-810") != NULL,
          "source evidence cites slot constants");
    CHECK(e != NULL && strstr(e, "PANEL.C:1127-1200") != NULL,
          "source evidence cites object description route");
    CHECK(e != NULL && strstr(e, "PANEL.C:1658-1692") != NULL,
          "source evidence cites action-hand panel route");
    CHECK(e != NULL && strstr(e, "CHAMPION.C:250-268") != NULL,
          "source evidence cites leader-hand set route");
    CHECK(e != NULL && strstr(e, "LOADSAVE.C:1535-1537") != NULL,
          "source evidence cites leader-hand persistence");
}

int main(void)
{
    printf("=== DM2 V1 inventory/item panel gate ===\n\n");

    test_slot_names_and_equipment_flags();
    test_inventory_selection_and_description();
    test_empty_invalid_and_unresolved_slots();
    test_source_evidence();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
