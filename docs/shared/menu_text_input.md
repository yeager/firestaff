# DM1 V1 - Text Input (Name Entry)

## Source-Locked
ReDMCSB WIP20210206 - Toolchains/Common/Source/

---

## Champion Name Input State (CEDT001.C globals)

Champion name and title text entry uses these globals (CEDT001.C:20-35):

- G7104_TextCursorVisible         - blinking cursor flag
- G7105_EditChampionNameOrTitleCharacterIndex - cursor position in text field
- G7106_EditChampionNameOrTitle   - which field: 0=name, 1=title
- G7107_SelectedColorIndex        - selected portrait palette color
- G7108_BlinkingCursorTimer      - cursor blink countdown timer
- G7109_SelectedChampionIndex     - which champion slot (0-3)
- G2329_s_UndoPortrait            - undo buffer for portrait edits

---

## Text Rendering (TEXT.C)

F0054_TEXT_Initialize (STARTUP1.C:155) - initializes text system

F0645_GetStringPixelDimensions (TEXT.C:529) - calculates pixel width and
height of a rendered string. Returns FALSE if string is NULL.
Used to validate that entered text fits in the name field box.

F0646_GetLargestPrintableSubString (TEXT.C:566) - finds the largest
substring that fits within a given pixel width. Used for text truncation.

G0353_ac_StringBuildBuffer[128-150] - general-purpose string buffer for
building formatted text output (TEXT.C:9, TEXT.C:14).

G0354_i_MessageAreaScrollingLineCount (TEXT.C:173) - scrolling line counter.

G0355_B_ScrollMessageArea (BOOLEAN) - auto-scroll flag.

---

## Text Input for F31E/F31J (TEXT2.C)

MEDIA497 platforms include TEXT2.C for extended text input:
#ifdef MEDIA497_F20E_F20J_P20JA_P20JB_I34E_I34M_A36M_A31E_A31M_A33M_A35E_A35M_F31E_F31J_X31J_P31J
#include TEXT2.C
#endif

Japanese text detection:
F0818_IsJapaneseText (TEXT.C:571) - returns TRUE if string contains
0x1B (ESC sequence) or characters with high bit set (0x80+).

---

## Message Formatting with Name Substitution (MENU.C)

F0381_MENUS_PrintMessageAfterReplacements (MENU.C:555) - prints combat
messages to the message area. Token @p in message string is replaced
by the acting champion name followed by a space.

L1165_pc_ReplacementString = M516_CHAMPIONS[...].Name (MENU.C:589)

---

## Champion Name Storage

Champion names stored in the CHAMPION struct array:
M516_CHAMPIONS[M001_ORDINAL_TO_INDEX(G0506_ui_ActingChampionOrdinal)].Name

---

## Portrait Text Fields (CEDTINC*.C)

CEDTINC files contain the edit-in-place data for champion portraits
and text fields. Text is drawn directly into the portrait bitmap as
part of the champion creation dialog.

---

## Summary

Champion name entry flow:
1. User clicks champion slot -> G7109_SelectedChampionIndex set
2. User edits name -> G7105 tracks cursor position, G7106 = 0 or 1
3. G7104_TextCursorVisible toggles via G7108 blink timer
4. F0645_GetStringPixelDimensions validates fit in name field box
5. F0646_GetLargestPrintableSubString handles overflow truncation
6. Name stored in M516_CHAMPIONS[slot].Name

Text input also used for champion title editing (G7106 = 1) and
file picker filename display (G2285_as_FilePickerDialogButtons).
