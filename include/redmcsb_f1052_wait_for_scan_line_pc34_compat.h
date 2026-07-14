#ifndef FIRESTAFF_REDMCSB_F1052_WAIT_FOR_SCAN_LINE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F1052_WAIT_FOR_SCAN_LINE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FILLBOX.C F1052_WaitForScanLine polls the Amiga VPOSR/VHPOSR hardware
 * register. PC 3.4 supplies no corresponding timing primitive or host
 * adapter, so this compatibility boundary deliberately performs no wait.
 */
void redmcsb_f1052_wait_for_scan_line_pc34_compat(int16_t scan_line);

const char *redmcsb_f1052_wait_for_scan_line_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F1052_WAIT_FOR_SCAN_LINE_PC34_COMPAT_H */
