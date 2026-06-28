#include "dm2_v1_inventory_panel.h"

#include <stdio.h>
#include <string.h>

static const char *const k_slot_labels[DM2_V1_INV_SLOT_COUNT] = {
    "ready_hand",
    "action_hand",
    "head",
    "torso",
    "legs",
    "feet",
    "pouch_2",
    "quiver_line2_1",
    "quiver_line1_2",
    "quiver_line2_2",
    "neck",
    "pouch_1",
    "quiver_line1_1",
    "backpack_line1_1",
    "backpack_line2_2",
    "backpack_line2_3",
    "backpack_line2_4",
    "backpack_line2_5",
    "backpack_line2_6",
    "backpack_line2_7",
    "backpack_line2_8",
    "backpack_line2_9",
    "backpack_line1_2",
    "backpack_line1_3",
    "backpack_line1_4",
    "backpack_line1_5",
    "backpack_line1_6",
    "backpack_line1_7",
    "backpack_line1_8",
    "backpack_line1_9"
};

static int slot_valid(int slot)
{
    return slot == DM2_V1_INV_SLOT_LEADER_HAND ||
           (slot >= 0 && slot < DM2_V1_INV_SLOT_COUNT);
}

const char *dm2_v1_inventory_slot_label(int slot)
{
    if (slot == DM2_V1_INV_SLOT_LEADER_HAND) return "leader_hand";
    if (slot < 0 || slot >= DM2_V1_INV_SLOT_COUNT) return "invalid";
    return k_slot_labels[slot];
}

int dm2_v1_inventory_slot_is_equipment(int slot)
{
    switch (slot) {
        case DM2_V1_INV_SLOT_LEADER_HAND:
        case DM2_V1_INV_SLOT_READY_HAND:
        case DM2_V1_INV_SLOT_ACTION_HAND:
        case DM2_V1_INV_SLOT_HEAD:
        case DM2_V1_INV_SLOT_TORSO:
        case DM2_V1_INV_SLOT_LEGS:
        case DM2_V1_INV_SLOT_FEET:
        case DM2_V1_INV_SLOT_NECK:
            return 1;
        default:
            return 0;
    }
}

static const char *lookup_description(
    uint32_t object_id,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count)
{
    if (!descriptions) return NULL;
    for (size_t i = 0; i < description_count; ++i) {
        if (descriptions[i].object_id == object_id &&
            descriptions[i].description &&
            descriptions[i].description[0] != '\0') {
            return descriptions[i].description;
        }
    }
    return NULL;
}

int dm2_v1_inventory_panel_select_item(
    const DM2_ChampionRecord *champion,
    const DM2_LeaderPossession *leader_hand,
    int selected_slot,
    const DM2_DB_State *db,
    const DM2_V1_InventoryPanelDescription *descriptions,
    size_t description_count,
    DM2_V1_InventoryPanelItemView *out)
{
    uint32_t object_id;
    const char *desc;

    if (!out || !slot_valid(selected_slot)) return 0;
    if (selected_slot != DM2_V1_INV_SLOT_LEADER_HAND && !champion) return 0;

    memset(out, 0, sizeof(*out));
    out->selected_slot = selected_slot;

    object_id = (selected_slot == DM2_V1_INV_SLOT_LEADER_HAND)
        ? (leader_hand ? leader_hand->object : 0u)
        : champion->inventory[selected_slot];
    out->object_id = object_id;

    if (object_id == 0u) {
        snprintf(out->description, sizeof(out->description), "EMPTY");
        return 1;
    }

    out->has_object = 1;
    out->db_resolved = dm2_db_resolve(
        object_id, db, &out->db_pool, &out->db_index) ? 1 : 0;

    desc = lookup_description(object_id, descriptions, description_count);
    if (desc) {
        snprintf(out->description, sizeof(out->description), "%s", desc);
    } else if (out->db_resolved) {
        snprintf(out->description, sizeof(out->description),
                 "POOL %u INDEX %lu",
                 (unsigned)out->db_pool,
                 (unsigned long)out->db_index);
    } else {
        snprintf(out->description, sizeof(out->description), "UNRESOLVED");
    }

    return 1;
}

const char *dm2_v1_inventory_panel_source_evidence(void)
{
    return
        "ReDMCSB DEFS.H:779-810 slot indices and leader-hand sentinel\n"
        "ReDMCSB PANEL.C:1127-1200 object-description panel route\n"
        "ReDMCSB PANEL.C:1658-1692 action-hand item panel route\n"
        "ReDMCSB PANEL.C:2421-2423 inventory slot redraw loop\n"
        "ReDMCSB CHAMPION.C:250-268/270-282 leader-hand put/remove state\n"
        "ReDMCSB LOADSAVE.C:1535-1537/2744 leader-hand object persistence";
}
