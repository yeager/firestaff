#include "firestaff/dm1/v1/champion_panel/inventory_tail_pc34_compat.h"

#include <string.h>

enum {
    kInventoryOwnerOrdinal = 3,
    kInventoryOwnerIndex = 2,
    kMutableIconLower = 0,
    kMutableIconUpper = 31,
    kMutablePotionLower = 148,
    kMutablePotionUpper = 163,
    kMutableEmptyFlask = 195
};

/*
 * ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1247:
 * after the top-row status-hand walk, a non-zero
 * G0423_i_InventoryChampionOrdinal selects the inventory owner, scans
 * slot indices C00_SLOT_READY_HAND..C30_SLOT_CHEST_1-1 as slotboxes
 * C08..C37, optionally scans open chest slotboxes C38..C45 when the
 * panel content is M569_PANEL_CHEST, ORs every F0295 change into
 * AL0884_B_DrawViewport, then sets MASK0x4000_VIEWPORT and calls F0292
 * exactly once when any tail slot changed.
 */
static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT or "
    "DUNGEON.DAT load. ReDMCSB CHAMDRAW.C F0296_CHAMPION_"
    "DrawChangedObjectIcons:1233-1247 inventory-owner tail: when "
    "G0423_i_InventoryChampionOrdinal is non-zero, M001_ORDINAL_TO_INDEX "
    "selects the inventory owner; F0296 scans inventory slots C00..C29 as "
    "slotboxes C08..C37 via F0295, calls F0386 only when the changed "
    "inventory slot index is C01_SLOT_ACTION_HAND, scans chest slotboxes "
    "C38..C45 only when panel content is M569_PANEL_CHEST, then sets "
    "MASK0x4000_VIEWPORT and calls F0292 once if any inventory or chest "
    "slot changed. CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:"
    "1153-1182 owns mutable icon comparison and F0038 redraw. DEFS.H:731 "
    "MASK0x4000_VIEWPORT, 781 C01_SLOT_ACTION_HAND, 810 C30_SLOT_CHEST_1, "
    "1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, 1876 "
    "C38_SLOT_BOX_CHEST_FIRST_SLOT, 3001/3007 M569_PANEL_CHEST.";

static const Dm1V1ChampionPanelInventoryTailEvidencePc34 s_evidence = {
    "ReDMCSB CHAMDRAW.C F0296_CHAMPION_DrawChangedObjectIcons:1233-1247",
    "ReDMCSB CHAMDRAW.C F0295_CHAMPION_HasObjectIconInSlotBoxChanged:1153-1182",
    "ReDMCSB DEFS.H:731 MASK0x4000_VIEWPORT, :781 C01_SLOT_ACTION_HAND, :810 C30_SLOT_CHEST_1, :1874 C08_SLOT_BOX_INVENTORY_FIRST_SLOT, :1876 C38_SLOT_BOX_CHEST_FIRST_SLOT, :3001/:3007 M569_PANEL_CHEST",
    "contract-only F0296 inventory-owner tail; no M11 renderer, no real bitmaps, no asset load, no pixel parity claim",
    "Disjoint from champion_panel_hand_slot_refresh (top-row status-hand walk-order + candidate owner gate), champion_panel_dead_member_hand_refresh (dead-member top-row branch), champion_panel_portrait_box_redraw_states (broader F0291/F0292 event matrix), inventory_panel_status_hand_open_chest_runtime (M11 runtime hit-zone path), and chest hand-swap lanes; this slice only pins F0296 inventory/chest tail scan and viewport cascade."
};

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= UINT32_C(16777619);
    return hash;
}

static int is_mutable_icon(int icon)
{
    return (icon >= kMutableIconLower && icon <= kMutableIconUpper) ||
           (icon >= kMutablePotionLower && icon <= kMutablePotionUpper) ||
           icon == kMutableEmptyFlask;
}

static int source_anchors_present(void)
{
    return strstr(s_source_evidence, "CHAMDRAW.C F0296_CHAMPION_"
                                     "DrawChangedObjectIcons:1233-1247") !=
               NULL &&
           strstr(s_source_evidence, "CHAMDRAW.C F0295_CHAMPION_"
                                     "HasObjectIconInSlotBoxChanged:1153-1182") !=
               NULL &&
           strstr(s_source_evidence, "MASK0x4000_VIEWPORT") != NULL &&
           strstr(s_source_evidence, "C08_SLOT_BOX_INVENTORY_FIRST_SLOT") !=
               NULL &&
           strstr(s_source_evidence, "C38_SLOT_BOX_CHEST_FIRST_SLOT") != NULL &&
           strstr(s_source_evidence, "M569_PANEL_CHEST") != NULL;
}

static int icon_changed(int current_icon, int object_icon)
{
    return is_mutable_icon(current_icon) && current_icon != object_icon;
}

void dm1_v1_champion_panel_inventory_tail_init_pc34(
    Dm1V1ChampionPanelInventoryTailStatePc34 *state)
{
    int i;

    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->assetFree = 1;
    state->inventoryChampionOrdinal = kInventoryOwnerOrdinal;
    state->inventoryChampionIndex = kInventoryOwnerIndex;
    state->panelContentIsChest = 1;

    for (i = 0; i < DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34; ++i) {
        state->inventoryCurrentIcon[i] = 20;
        state->inventoryObjectIcon[i] = 20;
    }
    state->inventoryObjectIcon[DM1_V1_CPIT_C01_SLOT_ACTION_HAND_PC34] = 21;
    state->inventoryObjectIcon[4] = 22;

    for (i = 0; i < DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34; ++i) {
        state->chestCurrentIcon[i] = 24;
        state->chestObjectIcon[i] = 24;
    }
    state->chestObjectIcon[0] = 25;
}

static int state_valid(const Dm1V1ChampionPanelInventoryTailStatePc34 *state)
{
    if (!state || !state->contractOnly || !state->assetFree) {
        return 0;
    }
    if (state->inventoryChampionOrdinal <= 0) {
        return 0;
    }
    return state->inventoryChampionIndex ==
           state->inventoryChampionOrdinal - 1;
}

int dm1_v1_champion_panel_inventory_tail_run_pc34(
    const Dm1V1ChampionPanelInventoryTailStatePc34 *state,
    Dm1V1ChampionPanelInventoryTailResultPc34 *result)
{
    int i;
    int inventory_changed;
    int chest_changed;
    int f0038_count;
    int f0386_count;
    uint32_t hash;

    if (!result) {
        return 0;
    }
    memset(result, 0, sizeof(*result));
    if (!state_valid(state)) {
        return 0;
    }

    inventory_changed = 0;
    chest_changed = 0;
    f0038_count = 0;
    f0386_count = 0;
    hash = UINT32_C(2166136261);

    result->accepted = 1;
    result->sourceAnchorsPresent = source_anchors_present();
    result->inventoryOwnerRequired = 1;
    result->inventoryOwnerIndexMatchesOrdinal = 1;
    result->inventoryFirstSlotBox =
        DM1_V1_CPIT_C08_SLOT_BOX_INVENTORY_FIRST_SLOT_PC34;
    result->inventoryLastSlotBox =
        DM1_V1_CPIT_C08_SLOT_BOX_INVENTORY_FIRST_SLOT_PC34 +
        DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34 - 1;
    result->chestFirstSlotBox =
        state->panelContentIsChest
            ? DM1_V1_CPIT_C38_SLOT_BOX_CHEST_FIRST_SLOT_PC34
            : -1;
    result->chestLastSlotBox =
        state->panelContentIsChest
            ? DM1_V1_CPIT_C38_SLOT_BOX_CHEST_FIRST_SLOT_PC34 +
                  DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34 - 1
            : -1;

    for (i = 0; i < DM1_V1_CPIT_INVENTORY_SLOT_COUNT_PC34; ++i) {
        int changed = icon_changed(state->inventoryCurrentIcon[i],
                                   state->inventoryObjectIcon[i]);
        ++result->inventorySlotScanCount;
        hash = mix_u32(hash,
                       (uint32_t)(DM1_V1_CPIT_C08_SLOT_BOX_INVENTORY_FIRST_SLOT_PC34 +
                                  i));
        hash = mix_u32(hash, (uint32_t)changed);
        if (changed) {
            ++inventory_changed;
            ++f0038_count;
            if (i == DM1_V1_CPIT_C01_SLOT_ACTION_HAND_PC34) {
                ++f0386_count;
            }
        }
    }

    if (state->panelContentIsChest) {
        for (i = 0; i < DM1_V1_CPIT_CHEST_SLOT_COUNT_PC34; ++i) {
            int changed = icon_changed(state->chestCurrentIcon[i],
                                       state->chestObjectIcon[i]);
            ++result->chestSlotScanCount;
            hash = mix_u32(hash,
                           (uint32_t)(DM1_V1_CPIT_C38_SLOT_BOX_CHEST_FIRST_SLOT_PC34 +
                                      i));
            hash = mix_u32(hash, (uint32_t)changed);
            if (changed) {
                ++chest_changed;
                ++f0038_count;
            }
        }
    }

    result->inventoryChangedCount = inventory_changed;
    result->chestChangedCount = chest_changed;
    result->f0038DrawIconInSlotBoxCount = f0038_count;
    result->f0386DrawActionIconCount = f0386_count;
    result->actionHandChangeDispatchesF0386 =
        icon_changed(state->inventoryCurrentIcon[DM1_V1_CPIT_C01_SLOT_ACTION_HAND_PC34],
                     state->inventoryObjectIcon[DM1_V1_CPIT_C01_SLOT_ACTION_HAND_PC34]) &&
        f0386_count == 1;
    result->nonActionInventoryChangeSkipsF0386 =
        inventory_changed > f0386_count;
    result->chestChangeSkipsF0386 =
        chest_changed == 0 || f0386_count == 1;
    if (inventory_changed || chest_changed) {
        result->viewportMaskSet = DM1_V1_CPIT_MASK0X4000_VIEWPORT_PC34;
        result->f0292DrawStateCount = 1;
    }
    result->f0292CalledOnceForAnyTailChange =
        (inventory_changed || chest_changed)
            ? (result->viewportMaskSet == DM1_V1_CPIT_MASK0X4000_VIEWPORT_PC34 &&
               result->f0292DrawStateCount == 1)
            : 1;
    result->noChangeSkipsViewportCascade =
        (inventory_changed || chest_changed)
            ? 1
            : (result->viewportMaskSet == 0 && result->f0292DrawStateCount == 0);
    result->hash = hash;
    return 1;
}

const Dm1V1ChampionPanelInventoryTailEvidencePc34 *
dm1_v1_champion_panel_inventory_tail_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_inventory_tail_source_evidence_pc34(void)
{
    return s_source_evidence;
}
