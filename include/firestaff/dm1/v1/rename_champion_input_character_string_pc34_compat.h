#ifndef FIRESTAFF_DM1_V1_RENAMECHAMPIONINPUTCHARACTERSTRING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_RENAMECHAMPIONINPUTCHARACTERSTRING_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0052_ac_Graphic562_RenameChampionInputCharacterString[2].
 *
 * G0052 is the 2-char null-terminated string " " (single space) used
 * by the rename-champion input UI as the default initial character.
 * PC 3.4 init = " " (space + NUL terminator at index 1). Read sites:
 * same REVIVE.C:429 family as G0051.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_RENAME_CHAMPION_INPUT_CHARACTER_STRING_PC34_COMPAT_SIZE 2

typedef struct DM1_V1_RenameChampionInputCharacterStringResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_RENAME_CHAMPION_INPUT_CHARACTER_STRING_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstCharSpace;
    int lastCharNulTerminator;
    int nulTerminated;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_RenameChampionInputCharacterStringResultPc34;

const char *
dm1_v1_rename_champion_input_character_string_table_pc34(void);

int
dm1_v1_rename_champion_input_character_string_size_pc34(void);

int
dm1_v1_rename_champion_input_character_string_get_pc34(int char_index);

int
dm1_v1_rename_champion_input_character_string_run_pc34(
    DM1_V1_RenameChampionInputCharacterStringResultPc34 *out);

#endif