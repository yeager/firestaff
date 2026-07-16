#ifndef FIRESTAFF_CSB_V1_F0089_STRNCPY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0089_STRNCPY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

char *F0089_strncpy(char *destination, const char *source, int16_t count);

char *csb_v1_f0089_strncpy_pc34_compat(
    char *destination,
    const char *source,
    int16_t count);

const char *csb_v1_f0089_strncpy_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
