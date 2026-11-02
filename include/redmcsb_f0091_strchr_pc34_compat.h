#ifndef REDMCSB_F0091_STRCHR_PC34_COMPAT_H
#define REDMCSB_F0091_STRCHR_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB STRING.C:63-73 semantics: searching for NUL returns NULL rather
 * than the address of the string terminator, as ISO C strchr does. */
char *redmcsb_f0091_strchr_pc34_compat(char *string, char character);

#ifdef __cplusplus
}
#endif

#endif
