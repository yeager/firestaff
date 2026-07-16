#include "csb_v1_f1106_f1107_f1109_f1114_fio1_trackdisk_boundaries_pc34_compat.h"

int16_t csb_v1_f1106_is_trackdisk_device_opened_pc34_compat(
    int16_t drive_index)
{
    (void)drive_index;
    return 0;
}

int16_t F1106_IsTrackdiskDeviceOpened(int16_t drive_index)
{
    return csb_v1_f1106_is_trackdisk_device_opened_pc34_compat(drive_index);
}

const char *csb_v1_f1106_is_trackdisk_device_opened_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYAM.C:753-763 F1106_IsTrackdiskDeviceOpened opens "
           "trackdisk.device and immediately closes it on success; FIO1.C:737 "
           "counts opened drives. PC34 does not probe host drives and returns "
           "not-opened without synthesizing trackdisk state";
}

int32_t csb_v1_f1107_get_disk_change_counter_pc34_compat(
    int16_t drive_index)
{
    (void)drive_index;
    return -1;
}

int32_t F1107_GetDiskChangeCounter(int16_t drive_index)
{
    return csb_v1_f1107_get_disk_change_counter_pc34_compat(drive_index);
}

const char *csb_v1_f1107_get_disk_change_counter_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYAM.C:765-778 F1107_GetDiskChangeCounter returns -1 "
           "when trackdisk.device cannot be opened, otherwise reads "
           "TD_CHANGENUM; FIO1.C:156-157 consumes that counter. PC34 exposes "
           "only the no-host-open failure boundary";
}

int16_t csb_v1_f1109_get_disk_state_pc34_compat(
    int16_t floppy_drive_index)
{
    (void)floppy_drive_index;
    return 0;
}

int16_t F1109_GetDiskState(int16_t floppy_drive_index)
{
    return csb_v1_f1109_get_disk_state_pc34_compat(floppy_drive_index);
}

const char *csb_v1_f1109_get_disk_state_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYAM.C:830-849 F1109_GetDiskState initializes disk "
           "state to 0 (no disk), upgrades to 1 for write-protected disk and "
           "2 for writable disk only after successful trackdisk IO; FIO1.C:970 "
           "polls it. PC34 keeps the source default and performs no host disk "
           "probe";
}

void csb_v1_f1114_close_trackdisk_device_pc34_compat(void)
{
}

void F1114_CloseTrackdiskDevice(void)
{
    csb_v1_f1114_close_trackdisk_device_pc34_compat();
}

const char *csb_v1_f1114_close_trackdisk_device_source_evidence_pc34(void)
{
    return "ReDMCSB FLOPPYAM.C:335-342 F1114_CloseTrackdiskDevice closes "
           "trackdisk.device only when G3182_B_TrackdiskDeviceOpened is true "
           "and then clears it; CNFG.C:173 also calls it after motor off. PC34 "
           "has no opened trackdisk device to close";
}
