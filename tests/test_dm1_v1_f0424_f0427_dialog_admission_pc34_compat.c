#include "dm1_v1_f0424_f0427_dialog_admission_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

int main(void)
{
    static uint8_t pixels[224 * 136];
    DM1_V1_F0424F0427DialogMaterialPc34 material;
    DM1_V1_F0424F0427DialogTextPc34 message1 = { "SAVE THIS GAME?", 16u, 1 };
    DM1_V1_F0424F0427DialogTextPc34 choice1 = { "SAVE AND QUIT", 14u, 1 };
    DM1_V1_F0424F0427DialogTextPc34 choice2 = { "CANCEL", 7u, 1 };
    DM1_V1_F0424F0427DialogRequestPc34 request;
    DM1_V1_F0424F0427DialogReceiptPc34 receipt;

    memset(&material, 0, sizeof(material));
    material.sourceGraphicId = 17;
    material.indexedPixels = pixels;
    material.indexedPixelCount = sizeof(pixels);
    material.width = 224;
    material.height = 136;
    material.stride = 224;
    material.sourceGraphicsDatVerified = 1;
    material.sourcePaletteVerified = 1;
    material.fontGraphicId = 695;
    material.sourceFontVerified = 1;
    memset(&request, 0, sizeof(request));
    request.material = &material;
    request.message1 = &message1;
    request.choices[0] = &choice1;
    request.choices[1] = &choice2;
    request.choiceCount = 2;
    request.selectedChoice = 2;
    request.selectedChoiceFromSourceInput = 1;

    CHECK(strstr(dm1_v1_f0424_f0427_dialog_source_evidence_pc34(),
                 "DIALOG.C F0424") != NULL);
    CHECK(dm1_v1_f0424_f0427_dialog_admit_pc34(&request, &receipt));
    CHECK(receipt.accepted && receipt.choiceCount == 2 &&
          receipt.selectedChoice == 2 && receipt.backdropGraphicId == 17 &&
          receipt.choicePatchGraphicId == 622 && receipt.messageZoneId == 471 &&
          receipt.messageLineCount == 1 && receipt.choiceFeedbackDelayTicks == 5 &&
          receipt.choiceFeedbackVblankCount == 2 &&
          receipt.choiceHitRects[0].x == 16 && receipt.choiceHitRects[0].y == 67 &&
          receipt.choiceHitRects[1].x == 16 && receipt.choiceHitRects[1].y == 104 &&
          receipt.choicePlans[0].text_x == 73 && receipt.choicePlans[0].text_y == 77 &&
          receipt.choicePlans[1].text_x == 94 && receipt.choicePlans[1].text_y == 114 &&
          receipt.suppressSyntheticFallback);

    request.selectedChoiceFromSourceInput = 0;
    CHECK(!dm1_v1_f0424_f0427_dialog_admit_pc34(&request, &receipt));
    CHECK(!receipt.accepted && !receipt.suppressSyntheticFallback);

    request.selectedChoiceFromSourceInput = 1;
    material.fontGraphicId = 1;
    CHECK(!dm1_v1_f0424_f0427_dialog_admit_pc34(&request, &receipt));
    CHECK(!receipt.accepted);

    material.fontGraphicId = 695;
    choice2.sourceTextVerified = 0;
    CHECK(!dm1_v1_f0424_f0427_dialog_admit_pc34(&request, &receipt));
    CHECK(!receipt.accepted);

    printf("test_dm1_v1_f0424_f0427_dialog_admission_pc34_compat: %d assertions, %d failures\\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
