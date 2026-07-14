/*
 * ReDMCSB STRING.C F0817_SubstituteFirstCharactersByString, PC-98 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0817_SUBSTITUTE_FIRST_CHARACTERS_BY_STRING_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0817_SUBSTITUTE_FIRST_CHARACTERS_BY_STRING_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Replaces the first character_count characters in string in place. The
 * caller provides sufficient writable storage when replacement grows it.
 * The return value preserves the original remaining-count convention.
 */
int16_t redmcsb_f0817_substitute_first_characters_by_string_pc34_compat(
    char *string,
    int16_t character_count,
    char *replacement_string);

const char *redmcsb_f0817_substitute_first_characters_by_string_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
