#ifndef FIRESTAFF_DM1_V1_UNDERSCORECHARACTERSTRING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_UNDERSCORECHARACTERSTRING_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0051_ac_Graphic562_UnderscoreCharacterString[2].
 *
 * G0051 is the 2-char null-terminated string "_" used by the rename
 * champion input UI as the underscore character placeholder. PC 3.4
 * init = "_" (with NUL terminator at index 1). Read sites:
 * REVIVE.C:429 (M521_F0040_TEXT_Print underscore-character blit).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_UNDERSCORE_CHARACTER_STRING_PC34_COMPAT_SIZE 2

typedef struct DM1_V1_UnderscoreCharacterStringResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_UNDERSCORE_CHARACTER_STRING_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstCharUnderscore;
    int lastCharNulTerminator;
    int nulTerminated;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_UnderscoreCharacterStringResultPc34;

const char *
dm1_v1_underscore_character_string_table_pc34(void);

int
dm1_v1_underscore_character_string_size_pc34(void);

int
dm1_v1_underscore_character_string_get_pc34(int char_index);

int
dm1_v1_underscore_character_string_run_pc34(
    DM1_V1_UnderscoreCharacterStringResultPc34 *out);

#endif