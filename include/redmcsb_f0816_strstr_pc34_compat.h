/*
 * ReDMCSB STRING.C F0816_strstr, PC-98 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0816_STRSTR_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0816_STRSTR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Mirrors ReDMCSB rather than the ISO C strstr contract: an empty needle
 * returns null. Inputs must point to NUL-terminated strings as in STRING.C.
 */
char *redmcsb_f0816_strstr_pc34_compat(char *string_haystack, char *string_needle);

const char *redmcsb_f0816_strstr_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
