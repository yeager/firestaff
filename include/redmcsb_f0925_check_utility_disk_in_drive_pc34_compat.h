#ifndef FIRESTAFF_REDMCSB_F0925_CHECK_UTILITY_DISK_IN_DRIVE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0925_CHECK_UTILITY_DISK_IN_DRIVE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Host boundaries for PRIM1.C F0925's F0926 and Floprd calls. */
typedef void (*redmcsb_f0925_force_media_change_detection_pc34_compat)(
    void *context,
    int device_number);

typedef int (*redmcsb_f0925_floprd_pc34_compat)(
    void *context,
    char *buffer,
    long reserved,
    int device_number,
    int sector,
    int track,
    int side,
    int count);

/*
 * ReDMCSB PRIM1.C F0925. Calls force_media_change_detection, then reads
 * sector 7 of track 0, side 0. A nonzero read result returns -1. Otherwise
 * it compares the strings at offsets 0 and 128 with the persistent strings
 * saved during the most recent nonzero first_loop_iteration call. A match
 * returns 1; a mismatch returns first_loop_iteration unchanged. A nonzero
 * first_loop_iteration saves the two strings after the comparison.
 *
 * As in the source, the callbacks and sector strings must be valid.
 */
int redmcsb_f0925_check_utility_disk_in_drive_pc34_compat(
    int device_number,
    int first_loop_iteration,
    redmcsb_f0925_force_media_change_detection_pc34_compat
        force_media_change_detection,
    redmcsb_f0925_floprd_pc34_compat floprd,
    void *context);

const char *
redmcsb_f0925_check_utility_disk_in_drive_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0925_CHECK_UTILITY_DISK_IN_DRIVE_PC34_COMPAT_H */
