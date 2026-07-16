#ifndef FIRESTAFF_DM1_V1_HINT_STRING_HELPERS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_HINT_STRING_HELPERS_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB HINTCASE.C F1984_ConvertCharacterToLowerCase.
 * Source behavior is the ASCII case fold used by hint text helpers; non A-Z
 * bytes pass through unchanged. */
int F1984_ConvertCharacterToLowerCase(int character);
int dm1_v1_hint_convert_character_to_lower_case_f1984_pc34(int character);
const char*
dm1_v1_hint_convert_character_to_lower_case_f1984_source_pc34(void);

/* ReDMCSB HINT001.C F2014_ConvertStringToLowerCase.
 * Converts a caller-owned NUL-terminated buffer in place using F1984. */
char* F2014_ConvertStringToLowerCase(char* text);
char* dm1_v1_hint_convert_string_to_lower_case_f2014_pc34(char* text);
const char*
dm1_v1_hint_convert_string_to_lower_case_f2014_source_pc34(void);

/* ReDMCSB HINTHINT.C F1909_CopyStringUntilCharacter.
 * PC34 keeps the copy bounded: bytes are copied until stopCharacter or NUL,
 * and dst is NUL-terminated when dstCapacity is nonzero. */
size_t F1909_CopyStringUntilCharacter(char* dst,
                                      size_t dstCapacity,
                                      const char* src,
                                      int stopCharacter);
size_t dm1_v1_hint_copy_string_until_character_f1909_pc34(
    char* dst,
    size_t dstCapacity,
    const char* src,
    int stopCharacter);
const char*
dm1_v1_hint_copy_string_until_character_f1909_source_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
