#ifndef FIRESTAFF_REDMCSB_F0926_FLOPPY_FORCE_MEDIA_CHANGE_DETECTION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0926_FLOPPY_FORCE_MEDIA_CHANGE_DETECTION_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * F0926 temporarily replaces Atari ST low-memory GEMDOS/BIOS vectors while
 * in supervisor mode. The PC 3.4 host supplies neither that mode nor those
 * vectors, so forcing media-change detection requires a platform adapter.
 */
bool redmcsb_f0926_floppy_force_media_change_detection_pc34_compat(
    int device_number);

const char *
redmcsb_f0926_floppy_force_media_change_detection_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0926_FLOPPY_FORCE_MEDIA_CHANGE_DETECTION_PC34_COMPAT_H */
