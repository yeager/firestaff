#include "dm1_v1_object_slotbox_icon_f0039_pc34_compat.h"

#include "firestaff/dm1/v1/slot_boxes_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void) {
    int i;
    const char* source;
    (void)source;

    assert(dm1_v1_slot_boxes_size_pc34() ==
           DM1_V1_SLOT_BOX_PC34_COMPAT_COUNT);
    assert(dm1_v1_slot_boxes_partition_status_hand_offset_pc34() == 0);
    assert(dm1_v1_slot_boxes_partition_inventory_offset_pc34() == 8);
    assert(dm1_v1_slot_boxes_partition_chest_offset_pc34() == 38);

    for (i = 0; i < dm1_v1_slot_boxes_size_pc34(); ++i) {
        assert(F0039_OBJECT_GetIconIndexInSlotBox(i) ==
               dm1_v1_slot_boxes_get_icon_index_pc34(i));
        assert(dm1_v1_object_get_icon_index_in_slotbox_f0039_pc34(i) ==
               F0039_OBJECT_GetIconIndexInSlotBox(i));
    }

    assert(F0039_OBJECT_GetIconIndexInSlotBox(0) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(7) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(8) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(37) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(38) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(45) == 0);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(-1) == -1);
    assert(F0039_OBJECT_GetIconIndexInSlotBox(46) == -1);

    source = dm1_v1_object_get_icon_index_in_slotbox_f0039_source_pc34();
    assert(source != 0);
    assert(strstr(source, "OBJECT.C F0039") != 0);
    assert(strstr(source, "G0030_as_Graphic562_SlotBoxes") != 0);
    assert(strstr(source, "DATA.C:264-309") != 0);
    return 0;
}
