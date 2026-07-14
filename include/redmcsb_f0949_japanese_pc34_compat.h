#ifndef FIRESTAFF_REDMCSB_F0949_JAPANESE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0949_JAPANESE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Converts the packed Shift-JIS character used by the Japanese PC ports to
 * the PC-98 character code consumed by F0950_JAPANESE_. */
int16_t redmcsb_f0949_japanese_pc34_compat(int16_t packed_character);

const char *redmcsb_f0949_japanese_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_REDMCSB_F0949_JAPANESE_PC34_COMPAT_H */
