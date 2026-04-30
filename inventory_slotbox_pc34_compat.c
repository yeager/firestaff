#include "inventory_slotbox_pc34_compat.h"
#include <string.h>

typedef struct SlotSpec { const char* name; const char* mask; } SlotSpec;
static const SlotSpec kInventorySlots[30] = {
    { "READY_HAND", "MASK0xFFFF_ANY_SLOT" },
    { "ACTION_HAND", "MASK0xFFFF_ANY_SLOT" },
    { "HEAD", "MASK0x0002_HEAD" },
    { "TORSO", "MASK0x0008_TORSO" },
    { "LEGS", "MASK0x0010_LEGS" },
    { "FEET", "MASK0x0020_FEET" },
    { "POUCH_2", "MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS" },
    { "QUIVER_LINE2_1", "MASK0x0080_QUIVER_LINE2" },
    { "QUIVER_LINE1_2", "MASK0x0080_QUIVER_LINE2" },
    { "QUIVER_LINE2_2", "MASK0x0080_QUIVER_LINE2" },
    { "NECK", "MASK0x0004_NECK" },
    { "POUCH_1", "MASK0x0100_POUCH_AND_PASS_THROUGH_DOORS" },
    { "QUIVER_LINE1_1", "MASK0x0040_QUIVER_LINE1" },
    { "BACKPACK_LINE1_1", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_2", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_3", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_4", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_5", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_6", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_7", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_8", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE2_9", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_2", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_3", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_4", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_5", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_6", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_7", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_8", "MASK0xFFFF_ANY_SLOT" },
    { "BACKPACK_LINE1_9", "MASK0xFFFF_ANY_SLOT" }
};

unsigned int INVENTORY_Compat_GetInventorySlotBoxCount(void) { return 30u; }
unsigned int INVENTORY_Compat_GetChestSlotBoxCount(void) { return 8u; }
unsigned int INVENTORY_Compat_GetTotalSlotBoxCount(void) { return 46u; }

static void fill_inventory_slot(unsigned int slotBoxIndex, InventoryCompatSlotBox* outSlotBox) {
    unsigned int slotIndex = slotBoxIndex - 8u;
    memset(outSlotBox, 0, sizeof(*outSlotBox));
    outSlotBox->slotBoxIndex = slotBoxIndex;
    outSlotBox->slotIndex = slotIndex;
    outSlotBox->commandId = 20u + slotBoxIndex;
    outSlotBox->zoneIndex = 499u + slotBoxIndex;
    outSlotBox->slotName = kInventorySlots[slotIndex].name;
    outSlotBox->allowedMaskName = kInventorySlots[slotIndex].mask;
    outSlotBox->sourceLineEvidence = "DATA.C:977-1023 maps slot boxes 8..37 to C507..C536; COMMAND.C:419-450 maps commands C028..C057 to those zones; DATA.C:1049-1087 maps slot masks";
}

int INVENTORY_Compat_GetInventorySlotBox(unsigned int slotBoxIndex, InventoryCompatSlotBox* outSlotBox) {
    if (!outSlotBox || slotBoxIndex < 8u || slotBoxIndex > 37u) return 0;
    fill_inventory_slot(slotBoxIndex, outSlotBox);
    return 1;
}

int INVENTORY_Compat_GetChestSlotBox(unsigned int slotBoxIndex, InventoryCompatSlotBox* outSlotBox) {
    unsigned int chestOrdinal;
    if (!outSlotBox || slotBoxIndex < 38u || slotBoxIndex > 45u) return 0;
    chestOrdinal = slotBoxIndex - 38u;
    memset(outSlotBox, 0, sizeof(*outSlotBox));
    outSlotBox->slotBoxIndex = slotBoxIndex;
    outSlotBox->slotIndex = 30u + chestOrdinal;
    outSlotBox->commandId = 20u + slotBoxIndex;
    outSlotBox->zoneIndex = 499u + slotBoxIndex;
    switch (chestOrdinal) {
    case 0u: outSlotBox->slotName = "CHEST_1"; break;
    case 1u: outSlotBox->slotName = "CHEST_2"; break;
    case 2u: outSlotBox->slotName = "CHEST_3"; break;
    case 3u: outSlotBox->slotName = "CHEST_4"; break;
    case 4u: outSlotBox->slotName = "CHEST_5"; break;
    case 5u: outSlotBox->slotName = "CHEST_6"; break;
    case 6u: outSlotBox->slotName = "CHEST_7"; break;
    default: outSlotBox->slotName = "CHEST_8"; break;
    }
    outSlotBox->allowedMaskName = "MASK0x0400_CONTAINER";
    outSlotBox->sourceLineEvidence = "DATA.C:1016-1023 maps chest slot boxes 38..45 to C537..C544; COMMAND.C:499-506 maps commands C058..C065; DATA.C:1080-1087 gives MASK0x0400_CONTAINER";
    return 1;
}

int INVENTORY_Compat_GetSlotRouteFromCommand(unsigned int commandId, InventoryCompatSlotRoute* outRoute) {
    unsigned int slotBoxIndex;
    if (!outRoute || commandId < 20u || commandId > 65u) return 0;
    slotBoxIndex = commandId - 20u;
    memset(outRoute, 0, sizeof(*outRoute));
    outRoute->commandId = commandId;
    outRoute->slotBoxIndex = slotBoxIndex;
    if (slotBoxIndex < 8u) {
        outRoute->championIndexFromStatusBox = slotBoxIndex >> 1;
        outRoute->slotIndex = slotBoxIndex & 1u;
        outRoute->sourceLineEvidence = "CHAMPION.C:677-683 maps status-box slotBox < 8 to championIndex=slotBox>>1 and hand slot=M070_HAND_SLOT_INDEX(slotBox)";
    } else {
        outRoute->usesInventoryChampion = 1u;
        outRoute->slotIndex = slotBoxIndex - 8u;
        outRoute->usesChestSlots = (outRoute->slotIndex >= 30u) ? 1u : 0u;
        outRoute->sourceLineEvidence = "CHAMPION.C:685-692 maps inventory/chest commands to inventory champion and slotIndex=slotBox-8, then C30+ uses G0425_aT_ChestSlots";
    }
    return 1;
}

#define DM1_SLOT_ACTION_HAND_PC34_COMPAT 1u
#define DM1_ICON_CONTAINER_CHEST_CLOSED_PC34_COMPAT 144u
#define DM1_ICON_CONTAINER_CHEST_OPEN_PC34_COMPAT 145u
#define DM1_THING_NONE_PC34_COMPAT 0xFFFFu

int INVENTORY_Compat_IsMutableObjectIconIndex(unsigned int iconIndex) {
    /* Mirrors F0295_CHAMPION_HasObjectIconInSlotBoxChanged: only icons
     * that represent carried objects/potions are updated by the periodic
     * changed-icon pass. Placeholder slot icons (empty box, body slots, etc.)
     * are intentionally ignored. */
    if (iconIndex < 32u) return 1;
    if (iconIndex >= 148u && iconIndex <= 163u) return 1;
    if (iconIndex == 195u) return 1;
    return 0;
}


unsigned int INVENTORY_Compat_GetActionHandIconForOpenChest(unsigned int isInventoryChampion,
                                                            unsigned int slotIndex,
                                                            unsigned int thing,
                                                            unsigned int openChestThing,
                                                            unsigned int baseIconIndex) {
    if (isInventoryChampion &&
        slotIndex == DM1_SLOT_ACTION_HAND_PC34_COMPAT &&
        baseIconIndex == DM1_ICON_CONTAINER_CHEST_CLOSED_PC34_COMPAT &&
        thing != DM1_THING_NONE_PC34_COMPAT &&
        thing == openChestThing) {
        return DM1_ICON_CONTAINER_CHEST_OPEN_PC34_COMPAT;
    }
    return baseIconIndex;
}

const char* INVENTORY_Compat_GetActionHandOpenChestIconEvidence(void) {
    return "ReDMCSB source lock: CHEST.C:43-46 sets G0426_T_OpenChest then draws C145_ICON_CONTAINER_CHEST_OPEN into C09_SLOT_BOX_INVENTORY_ACTION_HAND when opening from the action hand; CHAMDRAW.C:621-630 F0291_CHAMPION_DrawSlot maps inventory action-hand closed chest icon C144 to C145; PANEL.C:1651-1691 closes any prior chest, reads inventory champion C01_SLOT_ACTION_HAND, and routes containers through F0342/F0333; PANEL.C:1119-1133 F0342 closes chest for eye/mouth refresh and calls F0333 for containers; CHAMPION.C:557-575 closes/refreshes panel when the inventory action-hand chest is removed; CHAMPION.C:636-638 marks MASK0x0800_PANEL when a container is added to inventory action hand; DEFS.H:1875 and 1941-1942 define C09 action-hand slotbox plus C144/C145 closed/open chest icons.";
}

const char* INVENTORY_Compat_GetSlotBoxSourceEvidence(void) {
    return "ReDMCSB source lock: DEFS.H:778-817 defines slots 0..37 and C30 chest start; DEFS.H:1873-1878 defines C08 inventory first slot, C38 chest first slot, and M070 hand slot; DATA.C:977-1023 maps PC/F20+ G0030_as_Graphic562_SlotBoxes to zone indices; COMMAND.C:419-450 and 499-506 bind mouse commands to inventory/chest slot zones; COMMAND.C:2174-2177 dispatches C028..C065 via command-20 slotBox index; CHAMPION.C:677-692 resolves status/inventory/chest slot storage.";
}

const char* INVENTORY_Compat_GetCarriedObjectIconEvidence(void) {
    return "ReDMCSB source lock: CHAMDRAW.C:595-673 F0291_CHAMPION_DrawSlot draws placeholder icons for empty body/neck/backpack slots, otherwise F0033 object icons and then F0038_OBJECT_DrawIconInSlotBox; CHAMDRAW.C:1153-1182 F0295_CHAMPION_HasObjectIconInSlotBoxChanged refreshes only current slotbox icon ranges C000..C031, C148..C163, or C195 before redrawing via F0038; CHAMDRAW.C:1226-1252 applies that changed-icon pass to status, inventory, and chest slot boxes.";
}
