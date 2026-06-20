#ifndef FIRESTAFF_DM1_V1_REINCARNATESPECIALCHARACTERS_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_REINCARNATESPECIALCHARACTERS_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0053_ac_Graphic562_ReincarnateSpecialCharacters[6].
 *
 * G0053 is the 6-char special-character array used by the
 * reincarnate UI as the punctuation+space set: { ',', '.', ';',
 * ':', ' ', '\0' } (5 chars + NUL terminator). PC 3.4 init =
 * { ',', '.', ';', ':', ' ' } with a 1-byte NUL at index 5.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-852.
 */

#define DM1_V1_REINCARNATE_SPECIAL_CHARACTERS_PC34_COMPAT_SIZE 6

typedef struct DM1_V1_ReincarnateSpecialCharactersResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_REINCARNATE_SPECIAL_CHARACTERS_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstCharComma;
    int secondCharPeriod;
    int thirdCharSemicolon;
    int fourthCharColon;
    int fifthCharSpace;
    int sixthCharNulTerminator;
    int nulTerminated;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_ReincarnateSpecialCharactersResultPc34;

const char *
dm1_v1_reincarnate_special_characters_table_pc34(void);

int
dm1_v1_reincarnate_special_characters_size_pc34(void);

int
dm1_v1_reincarnate_special_characters_get_pc34(int char_index);

int
dm1_v1_reincarnate_special_characters_run_pc34(
    DM1_V1_ReincarnateSpecialCharactersResultPc34 *out);

#endif