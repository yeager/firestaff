#ifndef FIRESTAFF_DM1_V1_OBJECT_SLOTBOX_ICON_F0039_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_OBJECT_SLOTBOX_ICON_F0039_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB OBJECT.C F0039_OBJECT_GetIconIndexInSlotBox.
 * Reads SLOT_BOX.IconIndex from the DM1 PC34 G0030 slot-box table. */
short F0039_OBJECT_GetIconIndexInSlotBox(int slotBoxIndex);
short dm1_v1_object_get_icon_index_in_slotbox_f0039_pc34(int slotBoxIndex);
const char* dm1_v1_object_get_icon_index_in_slotbox_f0039_source_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
