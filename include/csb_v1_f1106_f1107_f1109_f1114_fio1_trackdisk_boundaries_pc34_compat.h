#ifndef FIRESTAFF_CSB_V1_F1106_F1107_F1109_F1114_FIO1_TRACKDISK_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1106_F1107_F1109_F1114_FIO1_TRACKDISK_BOUNDARIES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int16_t F1106_IsTrackdiskDeviceOpened(int16_t drive_index);
int32_t F1107_GetDiskChangeCounter(int16_t drive_index);
int16_t F1109_GetDiskState(int16_t floppy_drive_index);
void F1114_CloseTrackdiskDevice(void);

int16_t csb_v1_f1106_is_trackdisk_device_opened_pc34_compat(
    int16_t drive_index);
int32_t csb_v1_f1107_get_disk_change_counter_pc34_compat(
    int16_t drive_index);
int16_t csb_v1_f1109_get_disk_state_pc34_compat(
    int16_t floppy_drive_index);
void csb_v1_f1114_close_trackdisk_device_pc34_compat(void);

const char *csb_v1_f1106_is_trackdisk_device_opened_source_evidence_pc34(void);
const char *csb_v1_f1107_get_disk_change_counter_source_evidence_pc34(void);
const char *csb_v1_f1109_get_disk_state_source_evidence_pc34(void);
const char *csb_v1_f1114_close_trackdisk_device_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1106_F1107_F1109_F1114_FIO1_TRACKDISK_BOUNDARIES_PC34_COMPAT_H */
