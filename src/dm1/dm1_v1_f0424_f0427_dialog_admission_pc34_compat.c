#include "dm1_v1_f0424_f0427_dialog_admission_pc34_compat.h"

#include "csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB Toolchains/Common/Source/DIALOG.C F0424:331-542 and F0427:"
    "680-1076; F0425:546-557; F0426:561-589; COORD.C C450-C471; "
    "GRAPHICS.DAT C000 Graphic17 dialog box and M653 source font. "
    "F0424 input selection and F0427 draw planning are admitted here only; "
    "no host font, synthetic bitmap, or fallback draw is permitted.";

static int material_is_source_bound(
    const DM1_V1_F0424F0427DialogMaterialPc34 *material)
{
    size_t minimumPixelCount;

    if (!material || !material->indexedPixels ||
        material->sourceGraphicId != DM1_V1_F0424_F0427_DIALOG_GRAPHIC_PC34 ||
        material->width != DM1_V1_F0424_F0427_DIALOG_WIDTH_PC34 ||
        material->height != DM1_V1_F0424_F0427_DIALOG_HEIGHT_PC34 ||
        material->stride < DM1_V1_F0424_F0427_DIALOG_WIDTH_PC34 ||
        !material->sourceGraphicsDatVerified || !material->sourcePaletteVerified ||
        !material->sourceFontVerified ||
        (material->fontGraphicId != DM1_V1_F0424_F0427_FONT_M653_PC34 &&
         material->fontGraphicId != DM1_V1_F0424_F0427_FONT_M653_LEGACY_PC34)) {
        return 0;
    }
    minimumPixelCount = (size_t)material->stride * (size_t)material->height;
    return material->indexedPixelCount >= minimumPixelCount;
}

static int text_is_source_bound(const DM1_V1_F0424F0427DialogTextPc34 *text,
                                size_t *outLength)
{
    const char *terminator;

    if (outLength) *outLength = 0;
    if (!text) return 1;
    if (!text->text || !text->sourceTextVerified || text->textCapacity == 0 ||
        text->textCapacity > 60) {
        return 0;
    }
    terminator = (const char *)memchr(text->text, '\0', text->textCapacity);
    if (!terminator) return 0;
    if (outLength) *outLength = (size_t)(terminator - text->text);
    return 1;
}

static int message_line_count(const DM1_V1_F0424F0427DialogTextPc34 *message,
                              int *outCount)
{
    char source[60];
    char part1[60];
    char part2[60];
    size_t length;

    if (outCount) *outCount = 0;
    if (!message) return 1;
    if (!text_is_source_bound(message, &length) || length >= sizeof(source)) {
        return 0;
    }
    memcpy(source, message->text, length + 1u);
    if (csb_v1_f0426_dialog_is_message_on_two_lines_pc34_compat(
            source, part1, part2)) {
        if (outCount) *outCount = 2;
    } else if (outCount) {
        *outCount = 1;
    }
    return 1;
}

static int patch_graphic_for_choice_count(int choiceCount)
{
    switch (choiceCount) {
        case 1: return 621;
        case 2: return 622;
        case 3: return 0;
        case 4: return 623;
        default: return -1;
    }
}

int dm1_v1_f0424_f0427_dialog_admit_pc34(
    const DM1_V1_F0424F0427DialogRequestPc34 *request,
    DM1_V1_F0424F0427DialogReceiptPc34 *outReceipt)
{
    int choiceIndex;
    int message1Lines;
    int message2Lines;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!request || !material_is_source_bound(request->material) ||
        request->choiceCount < 1 || request->choiceCount > 4 ||
        request->selectedChoice < 1 ||
        request->selectedChoice > request->choiceCount ||
        !request->selectedChoiceFromSourceInput ||
        !message_line_count(request->message1, &message1Lines) ||
        !message_line_count(request->message2, &message2Lines)) {
        return 0;
    }

    for (choiceIndex = 0; choiceIndex < request->choiceCount; ++choiceIndex) {
        DM1_V1_DialogRectPc34 hitRect;
        size_t textLength;
        if (!request->choices[choiceIndex] ||
            !text_is_source_bound(request->choices[choiceIndex], &textLength) ||
            textLength == 0 ||
            !dm1_v1_dialog_choice_hit_rect_pc34(
                request->choiceCount, choiceIndex, &hitRect) ||
            csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
                request->choices[choiceIndex]->text, textLength + 1u,
                (hitRect.x + hitRect.w / 2),
                dm1_v1_dialog_choice_text_rect_pc34(
                    request->choiceCount, choiceIndex, &hitRect)
                    ? hitRect.y + 4 : -1,
                &outReceipt->choicePlans[choiceIndex]) != 1) {
            memset(outReceipt, 0, sizeof(*outReceipt));
            return 0;
        }
        if (!dm1_v1_dialog_choice_hit_rect_pc34(
                request->choiceCount, choiceIndex,
                &outReceipt->choiceHitRects[choiceIndex])) {
            memset(outReceipt, 0, sizeof(*outReceipt));
            return 0;
        }
    }

    outReceipt->accepted = 1;
    outReceipt->choiceCount = request->choiceCount;
    outReceipt->selectedChoice = request->selectedChoice;
    outReceipt->backdropGraphicId = DM1_V1_F0424_F0427_DIALOG_GRAPHIC_PC34;
    outReceipt->choicePatchGraphicId =
        patch_graphic_for_choice_count(request->choiceCount);
    outReceipt->messageZoneId = request->choiceCount == 1 ? 469 : 471;
    outReceipt->messageLineCount = message1Lines + message2Lines;
    outReceipt->choiceFeedbackDelayTicks = 5;
    outReceipt->choiceFeedbackVblankCount = 2;
    outReceipt->suppressSyntheticFallback = 1;
    return 1;
}

const char *dm1_v1_f0424_f0427_dialog_source_evidence_pc34(void)
{
    return s_source_evidence;
}
