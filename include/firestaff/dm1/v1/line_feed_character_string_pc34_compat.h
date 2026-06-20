#ifndef FIRESTAFF_DM1_V1_LINE_FEED_CHARACTER_STRING_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_LINE_FEED_CHARACTER_STRING_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0066_ac_Graphic562_LineFeedCharacterString[2] = "\n".
 *
 * G0066 is the 2-char null-terminated newline character string used
 * by F0040_TEXT_Print for line-feed rendering. PC 3.4 init = "\n"
 * (newline + NUL). Read site: F0040_TEXT_Print.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates pass798/800-806/811-863.
 */

#define DM1_V1_LINE_FEED_CHARACTER_STRING_PC34_COMPAT_SIZE 2

typedef struct DM1_V1_LINE_FEED_CHARACTER_STRINGResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_LINE_FEED_CHARACTER_STRING_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int firstCharNewline;
    int lastCharNulTerminator;
    int nulTerminated;
    int lookupFunctionCorrect;
    int lookupOutOfRangeReturnsZero;
} DM1_V1_LINE_FEED_CHARACTER_STRINGResultPc34;

const char *
dm1_v1_line_feed_character_string_table_pc34(void);

int
dm1_v1_line_feed_character_string_size_pc34(void);

int
dm1_v1_line_feed_character_string_get_pc34(int char_index);

int
dm1_v1_line_feed_character_string_run_pc34(
    DM1_V1_LINE_FEED_CHARACTER_STRINGResultPc34 *out);

#endif
