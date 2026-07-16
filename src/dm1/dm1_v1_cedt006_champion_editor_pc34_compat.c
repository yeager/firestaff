#include "dm1_v1_cedt006_champion_editor_pc34_compat.h"

#include <limits.h>
#include <string.h>

enum {
    kAsciiBackspace = 0x08,
    kAsciiLineFeed = 0x0a,
    kAsciiCarriageReturn = 0x0d,
    kAsciiEscape = 0x1b,
    kAsciiDelete = 0x7f
};

static size_t bounded_strlen_pc34(const char *text, size_t capacity)
{
    size_t length = 0;

    if (!text) return 0;
    while (length < capacity && text[length] != '\0') {
        ++length;
    }
    return length;
}

static void copy_bounded_text_pc34(char *dst, size_t dstCapacity,
                                   const char *src)
{
    size_t index;

    if (!dst || dstCapacity == 0) return;
    if (!src) {
        dst[0] = '\0';
        return;
    }
    for (index = 0; index + 1 < dstCapacity && src[index] != '\0'; ++index) {
        dst[index] = src[index];
    }
    dst[index] = '\0';
}

static int valid_box_pc34(const DM1_V1_CEDT006_BoxPc34 *box)
{
    return box && box->sourceBoxProven &&
           box->width > 0 && box->height > 0;
}

static int valid_color_index_pc34(int colorIndex)
{
    return colorIndex >= 0 &&
           colorIndex < DM1_V1_CEDT006_COLOR_COUNT_PC34;
}

int F7034_DrawButton(
    const DM1_V1_CEDT006_BoxPc34 *box,
    const char *text,
    int fillColor,
    DM1_V1_CEDT006_ButtonReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->fillColor = fillColor;
    out->sourceLineStart = 398;
    out->sourceLineEnd = 421;
    if (!valid_box_pc34(box) || !text || !valid_color_index_pc34(fillColor)) {
        return 0;
    }
    out->box = *box;
    copy_bounded_text_pc34(out->text, sizeof(out->text), text);
    out->drawBoxRequested = 1;
    out->drawTextRequested = 1;
    out->valid = 1;
    return 1;
}

int F7035_SetSelectedColorBox(
    int colorIndex,
    const DM1_V1_CEDT006_BoxPc34 *colorBox,
    DM1_V1_CEDT006_SelectedColorBoxReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->colorIndex = colorIndex;
    out->sourceLineStart = 423;
    out->sourceLineEnd = 439;
    if (!valid_color_index_pc34(colorIndex) || !valid_box_pc34(colorBox)) {
        return 0;
    }
    out->colorBox = *colorBox;
    out->drawSelectionBoxRequested = 1;
    out->valid = 1;
    return 1;
}

int F7036_SetSelectedColorIndex(
    int previousColorIndex,
    int selectedColorIndex,
    const DM1_V1_CEDT006_BoxPc34 *previousColorBox,
    const DM1_V1_CEDT006_BoxPc34 *selectedColorBox,
    DM1_V1_CEDT006_SelectedColorIndexReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->previousColorIndex = previousColorIndex;
    out->selectedColorIndex = selectedColorIndex;
    out->sourceLineStart = 442;
    out->sourceLineEnd = 458;
    if (!valid_color_index_pc34(previousColorIndex) ||
        !valid_color_index_pc34(selectedColorIndex) ||
        !valid_box_pc34(previousColorBox) ||
        !valid_box_pc34(selectedColorBox)) {
        return 0;
    }
    out->previousColorBox = *previousColorBox;
    out->selectedColorBox = *selectedColorBox;
    out->clearPreviousRequested = previousColorIndex != selectedColorIndex;
    out->drawSelectedRequested = 1;
    out->valid = 1;
    return 1;
}

int F7037_UpdateUndoBitmap(
    const uint8_t *selectedPortraitBytes,
    size_t selectedPortraitByteCount,
    int selectedPortraitBytesProven,
    uint8_t *undoBitmapBytes,
    size_t undoBitmapByteCapacity,
    DM1_V1_CEDT006_UndoBitmapReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->byteCount = selectedPortraitByteCount;
    out->sourceBytesProven = selectedPortraitBytesProven ? 1 : 0;
    out->sourceLineStart = 460;
    out->sourceLineEnd = 470;
    if (!selectedPortraitBytes || !undoBitmapBytes ||
        !selectedPortraitBytesProven ||
        selectedPortraitByteCount == 0 ||
        selectedPortraitByteCount > undoBitmapByteCapacity ||
        selectedPortraitByteCount > (size_t)INT_MAX) {
        return 0;
    }
    memcpy(undoBitmapBytes, selectedPortraitBytes, selectedPortraitByteCount);
    out->copiedByteCount = (int)selectedPortraitByteCount;
    out->valid = 1;
    return 1;
}

int F7032_DrawChampionNameOnTopOfScreen(
    int championIndex,
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champion,
    DM1_V1_CEDT006_TopNameReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->championIndex = championIndex;
    out->sourceLineStart = 351;
    out->sourceLineEnd = 365;
    if (!champion || !champion->present ||
        championIndex < 0 ||
        championIndex >= DM1_V1_CEDT006_CHAMPION_COUNT_PC34) {
        return 0;
    }
    copy_bounded_text_pc34(out->name, sizeof(out->name), champion->name);
    out->topScreenY = 0;
    out->valid = 1;
    return 1;
}

int F7033_DrawPortraitsAndNamesOnTopOfScreen(
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champions,
    size_t championCount,
    DM1_V1_CEDT006_PortraitsAndNamesReceiptPc34 *out)
{
    size_t index;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->sourceLineStart = 372;
    out->sourceLineEnd = 382;
    if (!champions ||
        championCount > DM1_V1_CEDT006_CHAMPION_COUNT_PC34) {
        return 0;
    }
    out->valid = 1;
    out->championCount = (int)championCount;
    for (index = 0; index < championCount; ++index) {
        if (!champions[index].present) {
            ++out->rejectedChampionCount;
            continue;
        }
        ++out->nameReceiptCount;
        if (champions[index].portraitPixelsProven) {
            ++out->portraitReceiptCount;
        }
    }
    return 1;
}

int F7038_PrintChampionNameOrTitleForEdition(
    int championIndex,
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champion,
    int editTitle,
    DM1_V1_CEDT006_NameOrTitleEditionReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->championIndex = championIndex;
    out->editTitle = editTitle ? 1 : 0;
    out->sourceLineStart = 472;
    out->sourceLineEnd = 479;
    if (!champion || !champion->present ||
        championIndex < 0 ||
        championIndex >= DM1_V1_CEDT006_CHAMPION_COUNT_PC34) {
        return 0;
    }
    copy_bounded_text_pc34(out->text, sizeof(out->text),
                           editTitle ? champion->title : champion->name);
    out->valid = 1;
    return 1;
}

static void format_int10_pc34(char *dst, size_t dstCapacity, int value)
{
    char reversed[12];
    unsigned int magnitude;
    size_t count = 0;
    size_t out = 0;
    int negative = value < 0;

    if (!dst || dstCapacity == 0) return;
    if (value < 0) {
        magnitude = (unsigned int)(-(value + 1)) + 1u;
    } else {
        magnitude = (unsigned int)value;
    }
    do {
        reversed[count++] = (char)('0' + (magnitude % 10u));
        magnitude /= 10u;
    } while (magnitude != 0u && count < sizeof(reversed));

    if (negative && out + 1 < dstCapacity) {
        dst[out++] = '-';
    }
    while (count > 0 && out + 1 < dstCapacity) {
        dst[out++] = reversed[--count];
    }
    dst[out] = '\0';
}

int F7039_DrawHealthOrStaminaOrMana(
    const char *name,
    int value,
    int screenY,
    DM1_V1_CEDT006_StatusLineReceiptPc34 *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->sourceLineStart = 493;
    out->sourceLineEnd = 503;
    out->value = value;
    out->screenY = screenY;
    if (!name || screenY < 0) {
        return 0;
    }

    copy_bounded_text_pc34(out->name, sizeof(out->name), name);
    format_int10_pc34(out->valueText, sizeof(out->valueText), value);
    out->drawTextRequested = 1;
    out->valid = 1;
    return 1;
}

int F7041_ProcessKeyboardInput(
    char *text,
    size_t textCapacity,
    size_t maximumLength,
    size_t *cursor,
    const uint16_t *rawKeys,
    size_t rawKeyCount,
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 *out)
{
    DM1_V1_CEDT006_KeyboardInputReceiptPc34 receipt;
    size_t length;
    size_t cursorValue;
    size_t keyIndex;

    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceLineStart = 662;
    receipt.sourceLineEnd = 678;

    if (!text || textCapacity == 0 || !cursor ||
        (!rawKeys && rawKeyCount != 0)) {
        if (out) *out = receipt;
        return 0;
    }

    length = bounded_strlen_pc34(text, textCapacity);
    if (length >= textCapacity) {
        text[textCapacity - 1u] = '\0';
        length = textCapacity - 1u;
    }
    if (maximumLength + 1u > textCapacity) {
        maximumLength = textCapacity - 1u;
    }
    cursorValue = *cursor > length ? length : *cursor;

    receipt.valid = 1;
    receipt.lengthBefore = length;
    receipt.cursorBefore = cursorValue;
    receipt.maximumLength = maximumLength;

    for (keyIndex = 0; keyIndex < rawKeyCount; ++keyIndex) {
        uint16_t ascii = rawKeys[keyIndex] & 0x00ffu;

        if (ascii == kAsciiCarriageReturn || ascii == kAsciiLineFeed) {
            receipt.accepted = 1;
            break;
        }
        if (ascii == kAsciiEscape) {
            receipt.cancelled = 1;
            break;
        }
        if (ascii == kAsciiBackspace) {
            if (cursorValue == 0) {
                ++receipt.rejectedKeyCount;
            } else {
                memmove(text + cursorValue - 1u, text + cursorValue,
                        length - cursorValue + 1u);
                --cursorValue;
                --length;
                ++receipt.deletedCount;
            }
            continue;
        }
        if (ascii == kAsciiDelete) {
            if (cursorValue >= length) {
                ++receipt.rejectedKeyCount;
            } else {
                memmove(text + cursorValue, text + cursorValue + 1u,
                        length - cursorValue);
                --length;
                ++receipt.deletedCount;
            }
            continue;
        }
        if (ascii < 0x20u || ascii > 0x7eu || ascii != rawKeys[keyIndex]) {
            ++receipt.rejectedKeyCount;
            continue;
        }
        if (length >= maximumLength || length + 1u >= textCapacity) {
            ++receipt.rejectedKeyCount;
            continue;
        }
        memmove(text + cursorValue + 1u, text + cursorValue,
                length - cursorValue + 1u);
        text[cursorValue] = (char)ascii;
        ++cursorValue;
        ++length;
        ++receipt.insertedCount;
    }

    *cursor = cursorValue;
    receipt.cursorAfter = cursorValue;
    receipt.lengthAfter = length;
    if (out) *out = receipt;
    return 1;
}

int F7040_SelectChampion(
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champions,
    size_t championCount,
    int selectedChampionIndex,
    int previousChampionIndex,
    DM1_V1_CEDT006_SelectChampionReceiptPc34 *out)
{
    const DM1_V1_CEDT006_ChampionSummaryPc34 *champion;

    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    out->selectedChampionIndex = selectedChampionIndex;
    out->previousChampionIndex = previousChampionIndex;
    out->sourceLineStart = 512;
    out->sourceLineEnd = 531;
    if (!champions ||
        championCount > DM1_V1_CEDT006_CHAMPION_COUNT_PC34 ||
        selectedChampionIndex < 0 ||
        (size_t)selectedChampionIndex >= championCount) {
        return 0;
    }
    champion = &champions[selectedChampionIndex];
    if (!champion->present) {
        return 0;
    }
    copy_bounded_text_pc34(out->name, sizeof(out->name), champion->name);
    copy_bounded_text_pc34(out->title, sizeof(out->title), champion->title);
    out->portraitIndex = champion->portraitIndex;
    out->portraitPixelsProven = champion->portraitPixelsProven ? 1 : 0;
    out->drawNameRequested = 1;
    out->drawTitleRequested = 1;
    out->drawPortraitRequested = out->portraitPixelsProven;
    out->valid = 1;
    return 1;
}

const char *F7039_F7041_CEDT006_SourceEvidencePc34(void)
{
    return "CEDT006.C:493 F7039_DrawHealthOrStaminaOrMana consumes "
           "caller-owned name/value/screenY facts and prepares a bounded "
           "draw receipt; CEDT006.C:662 F7041_ProcessKeyboardInput consumes "
           "only an explicit caller-owned key sequence and edits the provided "
           "text buffer in place. PC34 does not synthesize screen pixels, "
           "keyboard events, champion data, or fallback editor resources.";
}

const char *F7032_F7033_F7038_F7040_CEDT006_SourceEvidencePc34(void)
{
    return "CEDT006.C:351 F7032_DrawChampionNameOnTopOfScreen, "
           "CEDT006.C:372 F7033_DrawPortraitsAndNamesOnTopOfScreen, "
           "CEDT006.C:472 F7038_PrintChampionNameOrTitleForEdition, and "
           "CEDT006.C:512 F7040_SelectChampion are modeled as bounded "
           "Hall-of-Champions editor receipts over caller-owned champion "
           "summaries. PC34 does not synthesize champion records, portrait "
           "pixels, screen pixels, input events, or fallback editor resources.";
}

const char *F7034_F7035_F7036_F7037_CEDT006_SourceEvidencePc34(void)
{
    return "CEDT006.C:398 F7034_DrawButton, CEDT006.C:423 "
           "F7035_SetSelectedColorBox, CEDT006.C:442 "
           "F7036_SetSelectedColorIndex, and CEDT006.C:460 "
           "F7037_UpdateUndoBitmap are modeled as bounded Hall-of-Champions "
           "editor receipts over caller-owned boxes, color indices, and "
           "proven selected portrait bytes. PC34 does not synthesize "
           "champion data, portrait bytes, screen pixels, color-palette "
           "state, or input events.";
}
