#ifndef FIRESTAFF_REDMCSB_F0811_CHECK_COPY_PROTECTION_SIDE0_TRACK0_UNREFERENCED_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0811_CHECK_COPY_PROTECTION_SIDE0_TRACK0_UNREFERENCED_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB IO.C:4122-4151 implements F0811 with two PC-98 DISK BIOS read-ID
 * interrupts and checks the returned hardware registers. The portable host
 * has no equivalent BIOS interface, so this source-locked bridge performs no
 * disk I/O and cannot report a successful copy-protection check.
 */
bool redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_pc34_compat(
    void);

const char *
redmcsb_f0811_check_copy_protection_side0_track0_unreferenced_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif
