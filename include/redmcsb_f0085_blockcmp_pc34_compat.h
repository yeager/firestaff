#ifndef FIRESTAFF_REDMCSB_F0085_BLOCKCMP_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0085_BLOCKCMP_PC34_COMPAT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int16_t F0085_blockcmp(const void *left, const void *right, int16_t byte_count);
int16_t redmcsb_f0085_blockcmp_pc34_compat(
    const void *left,
    const void *right,
    int16_t byte_count);
const char *redmcsb_f0085_blockcmp_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
