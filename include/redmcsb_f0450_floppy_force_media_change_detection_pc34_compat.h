#ifndef FIRESTAFF_REDMCSB_F0450_FLOPPY_FORCE_MEDIA_CHANGE_DETECTION_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0450_FLOPPY_FORCE_MEDIA_CHANGE_DETECTION_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * PC 3.4 calls F0450 from F0452 immediately before its sector read. ReDMCSB
 * contains no PC 3.4 F0450 body: the available implementations are gated to
 * Atari ST media. This bridge therefore deliberately owns no media state and
 * invokes no host callback.
 */
void redmcsb_f0450_floppy_force_media_change_detection_pc34_compat(
    uint16_t drive_type);

void F0450_FLOPPY_ForceMediaChangeDetection(uint16_t drive_type);

const char *redmcsb_f0450_floppy_force_media_change_detection_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
