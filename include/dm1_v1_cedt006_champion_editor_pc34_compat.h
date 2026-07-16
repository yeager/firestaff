#ifndef FIRESTAFF_DM1_V1_CEDT006_CHAMPION_EDITOR_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CEDT006_CHAMPION_EDITOR_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CEDT006_STATUS_NAME_CAPACITY_PC34 = 32,
    DM1_V1_CEDT006_STATUS_VALUE_CAPACITY_PC34 = 16,
    DM1_V1_CEDT006_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CEDT006_CHAMPION_NAME_CAPACITY_PC34 = 32,
    DM1_V1_CEDT006_CHAMPION_TITLE_CAPACITY_PC34 = 32,
    DM1_V1_CEDT006_BUTTON_TEXT_CAPACITY_PC34 = 32,
    DM1_V1_CEDT006_COLOR_COUNT_PC34 = 16
};

typedef struct {
    int x;
    int y;
    int width;
    int height;
    int sourceBoxProven;
} DM1_V1_CEDT006_BoxPc34;

typedef struct {
    int valid;
    char name[DM1_V1_CEDT006_STATUS_NAME_CAPACITY_PC34];
    int value;
    int screenY;
    char valueText[DM1_V1_CEDT006_STATUS_VALUE_CAPACITY_PC34];
    int drawTextRequested;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_StatusLineReceiptPc34;

typedef struct {
    int valid;
    size_t lengthBefore;
    size_t lengthAfter;
    size_t cursorBefore;
    size_t cursorAfter;
    size_t maximumLength;
    int insertedCount;
    int deletedCount;
    int rejectedKeyCount;
    int accepted;
    int cancelled;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_KeyboardInputReceiptPc34;

typedef struct {
    int present;
    char name[DM1_V1_CEDT006_CHAMPION_NAME_CAPACITY_PC34];
    char title[DM1_V1_CEDT006_CHAMPION_TITLE_CAPACITY_PC34];
    int portraitIndex;
    int portraitPixelsProven;
} DM1_V1_CEDT006_ChampionSummaryPc34;

typedef struct {
    int valid;
    int championIndex;
    int topScreenY;
    char name[DM1_V1_CEDT006_CHAMPION_NAME_CAPACITY_PC34];
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_TopNameReceiptPc34;

typedef struct {
    int valid;
    int championCount;
    int nameReceiptCount;
    int portraitReceiptCount;
    int rejectedChampionCount;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_PortraitsAndNamesReceiptPc34;

typedef struct {
    int valid;
    int championIndex;
    int editTitle;
    char text[DM1_V1_CEDT006_CHAMPION_TITLE_CAPACITY_PC34];
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_NameOrTitleEditionReceiptPc34;

typedef struct {
    int valid;
    int selectedChampionIndex;
    int previousChampionIndex;
    char name[DM1_V1_CEDT006_CHAMPION_NAME_CAPACITY_PC34];
    char title[DM1_V1_CEDT006_CHAMPION_TITLE_CAPACITY_PC34];
    int portraitIndex;
    int portraitPixelsProven;
    int drawNameRequested;
    int drawTitleRequested;
    int drawPortraitRequested;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_SelectChampionReceiptPc34;

typedef struct {
    int valid;
    DM1_V1_CEDT006_BoxPc34 box;
    char text[DM1_V1_CEDT006_BUTTON_TEXT_CAPACITY_PC34];
    int fillColor;
    int drawBoxRequested;
    int drawTextRequested;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_ButtonReceiptPc34;

typedef struct {
    int valid;
    int colorIndex;
    DM1_V1_CEDT006_BoxPc34 colorBox;
    int drawSelectionBoxRequested;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_SelectedColorBoxReceiptPc34;

typedef struct {
    int valid;
    int previousColorIndex;
    int selectedColorIndex;
    DM1_V1_CEDT006_BoxPc34 previousColorBox;
    DM1_V1_CEDT006_BoxPc34 selectedColorBox;
    int clearPreviousRequested;
    int drawSelectedRequested;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_SelectedColorIndexReceiptPc34;

typedef struct {
    int valid;
    size_t byteCount;
    int sourceBytesProven;
    int copiedByteCount;
    int sourceLineStart;
    int sourceLineEnd;
} DM1_V1_CEDT006_UndoBitmapReceiptPc34;

int F7034_DrawButton(
    const DM1_V1_CEDT006_BoxPc34 *box,
    const char *text,
    int fillColor,
    DM1_V1_CEDT006_ButtonReceiptPc34 *out);

int F7035_SetSelectedColorBox(
    int colorIndex,
    const DM1_V1_CEDT006_BoxPc34 *colorBox,
    DM1_V1_CEDT006_SelectedColorBoxReceiptPc34 *out);

int F7036_SetSelectedColorIndex(
    int previousColorIndex,
    int selectedColorIndex,
    const DM1_V1_CEDT006_BoxPc34 *previousColorBox,
    const DM1_V1_CEDT006_BoxPc34 *selectedColorBox,
    DM1_V1_CEDT006_SelectedColorIndexReceiptPc34 *out);

int F7037_UpdateUndoBitmap(
    const uint8_t *selectedPortraitBytes,
    size_t selectedPortraitByteCount,
    int selectedPortraitBytesProven,
    uint8_t *undoBitmapBytes,
    size_t undoBitmapByteCapacity,
    DM1_V1_CEDT006_UndoBitmapReceiptPc34 *out);

int F7032_DrawChampionNameOnTopOfScreen(
    int championIndex,
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champion,
    DM1_V1_CEDT006_TopNameReceiptPc34 *out);

int F7033_DrawPortraitsAndNamesOnTopOfScreen(
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champions,
    size_t championCount,
    DM1_V1_CEDT006_PortraitsAndNamesReceiptPc34 *out);

int F7038_PrintChampionNameOrTitleForEdition(
    int championIndex,
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champion,
    int editTitle,
    DM1_V1_CEDT006_NameOrTitleEditionReceiptPc34 *out);

int F7039_DrawHealthOrStaminaOrMana(
    const char *name,
    int value,
    int screenY,
    DM1_V1_CEDT006_StatusLineReceiptPc34 *out);

int F7041_ProcessKeyboardInput(
    char *text,
    size_t textCapacity,
    size_t maximumLength,
    size_t *cursor,
    const uint16_t *rawKeys,
    size_t rawKeyCount,
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 *out);

int F7040_SelectChampion(
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champions,
    size_t championCount,
    int selectedChampionIndex,
    int previousChampionIndex,
    DM1_V1_CEDT006_SelectChampionReceiptPc34 *out);

const char *F7039_F7041_CEDT006_SourceEvidencePc34(void);
const char *F7032_F7033_F7038_F7040_CEDT006_SourceEvidencePc34(void);
const char *F7034_F7035_F7036_F7037_CEDT006_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CEDT006_CHAMPION_EDITOR_PC34_COMPAT_H */
