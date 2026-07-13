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
#include "dm2_v1_asset_loader.h"
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

    {
        char name[32];

        CHECK(dm2_db_decode_handle(tech_key, NULL, NULL),
              "DM2 DB handle decodes without a loaded pool");
        CHECK(strcmp(dm2_db_pool_label(DM2_DB_MISC), "MISC") == 0,
              "DM2 DB pool label for misc is stable");
        CHECK(dm2_db_format_handle_name(tech_key, name, sizeof(name)) &&
              strcmp(name, "DM2 MISC 3") == 0,
              "DM2 DB handle name formatter preserves pool and index");
    }
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

static void test_source_selected_item_gdat_material(void)
{
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entries[2];
    uint8_t raw[27];
    uint32_t offsets[1] = {0u};
    uint32_t sizes[1] = {sizeof(raw)};
    DM2_ChampionRecord champ;
    DM2_DB_State db;
    DM2_V1_InventoryPanelItemView item;
    DM2_V1_InventoryPanelGdatMaterialReceipt material;
    DM2_V1_InventoryPanelHudReceipt hud;
    DM2_V1_InventoryPanelHudBlit blit;
    DM2_V1_InventoryPanelHudSurface surface;
    DM2_V1_InventoryPanelHudConsumptionReceipt consumption;
    uint8_t hud_pixels[16];
    uint32_t powerblade;
    int palette;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    memset(raw, 0, sizeof(raw));
    memset(&champ, 0, sizeof(champ));
    memset(&db, 0, sizeof(db));
    raw[0] = 2u;
    raw[2] = 1u;
    raw[3] = 0x80u;
    raw[4] = 4u;
    raw[10] = 0x1cu;
    for (palette = 0; palette < 16; ++palette) {
        raw[11 + palette] = (uint8_t)(0x20 + palette);
    }
    entries[0].cls1 = DM2_GDAT_CATEGORY_WEAPONS;
    entries[0].cls2 = 0x2au;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[0].cls4 = 0x18u;
    entries[0].data_index = 0u;
    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = 1u;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.raw_data_count = 1u;
    loader.data = raw;
    loader.data_size = sizeof(raw);
    powerblade = dm2_db_make_handle(DM2_DB_WEAPON, 1u);
    champ.inventory[DM2_V1_INV_SLOT_ACTION_HAND] = powerblade;

    CHECK(dm2_v1_inventory_panel_select_item(
              &champ, NULL, DM2_V1_INV_SLOT_ACTION_HAND,
              &db, NULL, 0u, &item),
          "source-selected action-hand item snapshot succeeds");
    CHECK(dm2_v1_inventory_panel_gdat_material_receipt(
              &loader, powerblade, DM2_GDAT_CATEGORY_WEAPONS, 0x2au, 0x18u,
              &material) && material.valid &&
              material.decoded_width == 2u && material.decoded_height == 1u &&
              material.decoded_format == DM2_IMG_FMT_U4 &&
              material.local_palette16[0] == 0x20u &&
              material.local_palette16[15] == 0x2fu &&
              material.material_hash != 0u,
          "item material receipt binds exact GDAT image, pixels, and palette");
    CHECK(dm2_v1_inventory_panel_hud_receipt(&item, &material, &hud) &&
              hud.valid && hud.object_id == powerblade &&
              hud.selected_slot == DM2_V1_INV_SLOT_ACTION_HAND &&
              hud.receipt_hash != 0u,
          "HUD receipt accepts only matching selected item material");
    memset(hud_pixels, 0x7eu, sizeof(hud_pixels));
    memset(&blit, 0, sizeof(blit));
    blit.rect_number = 125u;
    blit.destination_x = 1;
    blit.destination_y = 2;
    blit.width = 2u;
    blit.height = 1u;
    blit.transparent_index = 12u;
    surface.pixels = hud_pixels;
    surface.width = 4;
    surface.height = 4;
    surface.stride = 4;
    CHECK(dm2_v1_inventory_panel_consume_hud_material(
              &loader, &hud, &blit, &surface, &consumption) &&
              consumption.valid && consumption.rect_number == 125u &&
              consumption.drawn_pixel_count == 1u &&
              consumption.transparent_pixel_count == 1u &&
              hud_pixels[2 * 4 + 1] == 0x21u &&
              hud_pixels[2 * 4 + 2] == 0x7eu,
          "live HUD consumption blits exact item palette with source key 12");
    blit.transparent_index = 0u;
    CHECK(!dm2_v1_inventory_panel_consume_hud_material(
              &loader, &hud, &blit, &surface, &consumption),
          "non-source transparency key cannot produce an item fallback blit");
    blit.transparent_index = 12u;
    blit.destination_x = 3;
    CHECK(!dm2_v1_inventory_panel_consume_hud_material(
              &loader, &hud, &blit, &surface, &consumption),
          "out-of-bounds source rect does not synthesize clipped HUD pixels");
    blit.destination_x = 1;
    raw[10] = 0x11u;
    CHECK(!dm2_v1_inventory_panel_consume_hud_material(
              &loader, &hud, &blit, &surface, &consumption),
          "changed GDAT item pixels invalidate the live HUD receipt");
    CHECK(!dm2_v1_inventory_panel_gdat_material_receipt(
              &loader, powerblade, DM2_GDAT_CATEGORY_WEAPONS, 0x2au, 0x19u,
              &material),
          "missing source-selected item field fails closed");
    CHECK(!dm2_v1_inventory_panel_gdat_material_receipt(
              &loader, powerblade, DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0x02u, 0u, &material),
          "non-item GDAT category cannot become an inventory icon");
    CHECK(!dm2_v1_inventory_panel_hud_receipt(&item, &material, &hud),
          "failed material cannot produce a fallback HUD receipt");
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
    test_source_selected_item_gdat_material();
    test_source_evidence();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
