#ifndef FIRESTAFF_DM1_V1_F0424_F0427_DIALOG_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0424_F0427_DIALOG_ADMISSION_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#include "csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat.h"
#include "dm1_v1_dialog_layout_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_F0424_F0427_DIALOG_GRAPHIC_PC34 = 17,
    DM1_V1_F0424_F0427_DIALOG_WIDTH_PC34 = 224,
    DM1_V1_F0424_F0427_DIALOG_HEIGHT_PC34 = 136,
    DM1_V1_F0424_F0427_FONT_M653_PC34 = 695,
    DM1_V1_F0424_F0427_FONT_M653_LEGACY_PC34 = 557
};

typedef struct DM1_V1_F0424F0427DialogMaterialPc34 {
    int sourceGraphicId;
    const uint8_t *indexedPixels;
    size_t indexedPixelCount;
    int width;
    int height;
    int stride;
    int sourceGraphicsDatVerified;
    int sourcePaletteVerified;
    int fontGraphicId;
    int sourceFontVerified;
} DM1_V1_F0424F0427DialogMaterialPc34;

typedef struct DM1_V1_F0424F0427DialogTextPc34 {
    const char *text;
    size_t textCapacity;
    int sourceTextVerified;
} DM1_V1_F0424F0427DialogTextPc34;

typedef struct DM1_V1_F0424F0427DialogRequestPc34 {
    const DM1_V1_F0424F0427DialogMaterialPc34 *material;
    const DM1_V1_F0424F0427DialogTextPc34 *message1;
    const DM1_V1_F0424F0427DialogTextPc34 *message2;
    const DM1_V1_F0424F0427DialogTextPc34 *choices[4];
    int choiceCount;
    int selectedChoice;
    int selectedChoiceFromSourceInput;
} DM1_V1_F0424F0427DialogRequestPc34;

typedef struct DM1_V1_F0424F0427DialogReceiptPc34 {
    int accepted;
    int choiceCount;
    int selectedChoice;
    int backdropGraphicId;
    int choicePatchGraphicId;
    int messageZoneId;
    int messageLineCount;
    int choiceFeedbackDelayTicks;
    int choiceFeedbackVblankCount;
    DM1_V1_DialogRectPc34 choiceHitRects[4];
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat choicePlans[4];
    int suppressSyntheticFallback;
} DM1_V1_F0424F0427DialogReceiptPc34;

/*
 * F0424/F0427 admission only. It derives choice geometry through the existing
 * F0425/F0426 helpers but never draws, fabricates glyphs, or supplies a host
 * font. A complete original dialog graphic, palette, font, and source text
 * are required before the plan is accepted.
 */
int dm1_v1_f0424_f0427_dialog_admit_pc34(
    const DM1_V1_F0424F0427DialogRequestPc34 *request,
    DM1_V1_F0424F0427DialogReceiptPc34 *outReceipt);

const char *dm1_v1_f0424_f0427_dialog_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
