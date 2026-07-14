#ifndef FIRESTAFF_REDMCSB_F0670_F0671_STRING_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0670_F0671_STRING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ReDMCSB STRING.C F0670/F0671, PC I34E/I34M route.
 * Destination storage is caller-owned and must hold the source contract's
 * complete result; no allocation, truncation, or localization fallback occurs.
 */
int redmcsb_f0670_replace_character_by_string_pc34_compat(
    const char *source_string,
    const char *replacement_string,
    char character,
    char *destination_string);

void redmcsb_f0671_convert_long_to_string_pc34_compat(
    int32_t value,
    char *destination_string);

const char *redmcsb_f0670_f0671_string_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
