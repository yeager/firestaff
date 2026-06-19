#ifndef FIRESTAFF_DM1_V1_SLOT_BOXES_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_SLOT_BOXES_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0030_as_Graphic562_SlotBoxes[46].
 *
 * G0030 is the per-slot pixel-coordinate table for the 46 clickable
 * slot boxes in DM1's status/champion/chest UI: 8 status-box hands
 * (2 per champion * up to 4 champions), 30 inventory slots (ready
 * hand through backpack line-1/line-2 + quivers + pouches + neck),
 * and 8 chest slots.
 *
 * Each entry is a SLOT_BOX { X, Y, ZoneIndex, IconIndex }. The PC 3.4
 * init (DATA.C:264-309) leaves ZoneIndex = 0 in all 46 entries and
 * uses IconIndex for the slot's currently-displayed icon index.
 *
 * Read sites:
 * - OBJECT.C:435  F0486_OBJECT_DrawSlotBoxAtSlotIndex: reads X+Y+IconIndex
 * - OBJECT.C:521  F0488_OBJECT_GetSlotBoxIconIndex: returns .IconIndex
 * - CHAMDRAW.C:557  F0487_CHAMPION_DrawSlotBoxAtChampionSlotIndex: reads X+Y
 * - CHAMDRAW.C:562  F0619_GetSlotBoxBorderCoordinates(ZoneIndex): border
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-796 (champion-panel/leader/mirror), pass797 (icon-graphic-
 * first-icon-index). This gate is a non-mirror-candidate contract
 * for the slot-box pixel-coordinate table.
 */

#define DM1_V1_SLOT_BOX_PC34_COMPAT_COUNT 46
#define DM1_V1_SLOT_BOX_STATUS_HAND_COUNT 8
#define DM1_V1_SLOT_BOX_INVENTORY_COUNT   30
#define DM1_V1_SLOT_BOX_CHEST_COUNT       8

typedef struct DM1_V1_SlotBoxPc34Compat {
    short x;
    short y;
    short zoneIndex;
    short iconIndex;
} DM1_V1_SlotBoxPc34Compat;

typedef struct DM1_V1_SlotBoxesResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_SLOT_BOX_PC34_COMPAT_COUNT * 4];
    int tableSize;
    int tableMatchesDeclaration;
    int statusHandCount8;
    int inventoryCount30;
    int chestCount8;
    int allZoneIndexZero;
    int allXWithinViewport;
    int allYWithinPanel;
    int statusBoxYIs10;
    int chestBoxYIs16Plus;
    int chestBoxXPixelMonotonic;
    int inventoryXIsAtLeast6;
    int inventoryYWithinInventoryPanel;
    int statusBoxHandXEvenOffset;
    int chestBoxYPixelMonotonic;
    int iconIndexLookupFunctionCorrect;
    int statusBoxIconIndexRange;
    int inventoryIconIndexRange;
    int chestIconIndexRange;
    int partitionOrderingCorrect;
} DM1_V1_SlotBoxesResultPc34;

const DM1_V1_SlotBoxPc34Compat *
dm1_v1_slot_boxes_table_pc34(void);

int
dm1_v1_slot_boxes_size_pc34(void);

int
dm1_v1_slot_boxes_partition_status_hand_count_pc34(void);

int
dm1_v1_slot_boxes_partition_inventory_count_pc34(void);

int
dm1_v1_slot_boxes_partition_chest_count_pc34(void);

int
dm1_v1_slot_boxes_partition_status_hand_offset_pc34(void);

int
dm1_v1_slot_boxes_partition_inventory_offset_pc34(void);

int
dm1_v1_slot_boxes_partition_chest_offset_pc34(void);

const DM1_V1_SlotBoxPc34Compat *
dm1_v1_slot_boxes_get_pc34(int slot_box_index);

short
dm1_v1_slot_boxes_get_x_pc34(int slot_box_index);

short
dm1_v1_slot_boxes_get_y_pc34(int slot_box_index);

short
dm1_v1_slot_boxes_get_zone_index_pc34(int slot_box_index);

short
dm1_v1_slot_boxes_get_icon_index_pc34(int slot_box_index);

int
dm1_v1_slot_boxes_is_status_hand_pc34(int slot_box_index);

int
dm1_v1_slot_boxes_is_inventory_pc34(int slot_box_index);

int
dm1_v1_slot_boxes_is_chest_pc34(int slot_box_index);

int
dm1_v1_slot_boxes_run_pc34(
    DM1_V1_SlotBoxesResultPc34 *out);

#endif