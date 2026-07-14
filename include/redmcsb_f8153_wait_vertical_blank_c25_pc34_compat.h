/* ReDMCSB VIDEODRV.C F8153 C25 VGA vertical-blank synchronization. */
#ifndef FIRESTAFF_REDMCSB_F8153_WAIT_VERTICAL_BLANK_C25_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F8153_WAIT_VERTICAL_BLANK_C25_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t (*RedmcsbF8153VideoStatusReaderPc34Compat)(void *context);

/* Waits for status bit 3 to become inactive, then active again. */
bool redmcsb_f8153_wait_vertical_blank_c25_pc34_compat(
    RedmcsbF8153VideoStatusReaderPc34Compat read_status, void *context);

const char *redmcsb_f8153_wait_vertical_blank_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
