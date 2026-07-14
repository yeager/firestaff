#ifndef FIRESTAFF_REDMCSB_F0089_STRNCPY_H
#define FIRESTAFF_REDMCSB_F0089_STRNCPY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB F0089_strncpy (PC 3.4).
 *
 * Source evidence: Toolchains/Common/Source/DEFS.H:3085 declares the
 * signed 16-bit count; the recovered F0089 body copies until either that
 * count is exhausted or the source NUL is copied. Unlike ISO C strncpy,
 * it does not pad the remaining destination bytes after copying a NUL.
 *
 * The original loop has no bounded C11 meaning for a negative signed count.
 * PC 3.4 callers supply a non-negative count. This callable preserves the
 * proven domain and treats a negative count as a bounded no-op.
 */
char *redmcsb_f0089_strncpy(char *destination,
                             const char *source,
                             int16_t count);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0089_STRNCPY_H */
