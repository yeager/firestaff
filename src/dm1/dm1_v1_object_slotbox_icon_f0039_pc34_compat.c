#include "dm1_v1_object_slotbox_icon_f0039_pc34_compat.h"

#include "firestaff/dm1/v1/slot_boxes_pc34_compat.h"

short F0039_OBJECT_GetIconIndexInSlotBox(int slotBoxIndex) {
    return dm1_v1_slot_boxes_get_icon_index_pc34(slotBoxIndex);
}

short dm1_v1_object_get_icon_index_in_slotbox_f0039_pc34(
    int slotBoxIndex) {
    return F0039_OBJECT_GetIconIndexInSlotBox(slotBoxIndex);
}

const char* dm1_v1_object_get_icon_index_in_slotbox_f0039_source_pc34(void) {
    return "ReDMCSB OBJECT.C F0039_OBJECT_GetIconIndexInSlotBox: "
           "returns G0030_as_Graphic562_SlotBoxes[slotBoxIndex].IconIndex; "
           "DM1 consumes the DATA.C:264-309 PC34 slot-box table.";
}
