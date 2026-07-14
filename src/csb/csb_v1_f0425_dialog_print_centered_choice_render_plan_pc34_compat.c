#include "csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat.h"

const char *csb_v1_f0425_dialog_print_centered_choice_source_evidence_pc34(void)
{
    return "ReDMCSB DIALOG.C:246-256 F0425: NULL text is a no-op; "
           "X -= (strlen(text) * 6) >> 1; TEXT_Print uses viewport width "
           "112, C09 gold foreground, and C05 light-brown background.";
}

int csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
    const char *text,
    size_t text_capacity,
    int center_x,
    int text_y,
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat *out_plan)
{
    size_t text_length;

    if (text == NULL) {
        return 0;
    }
    if (out_plan == NULL) {
        return -1;
    }
    for (text_length = 0; text_length < text_capacity; ++text_length) {
        if (text[text_length] == '\0') {
            out_plan->text = text;
            out_plan->text_length = text_length;
            out_plan->text_x = center_x -
                (int)((text_length * CSB_V1_F0425_DIALOG_GLYPH_WIDTH_PC34) >> 1);
            out_plan->text_y = text_y;
            out_plan->bitmap_byte_width =
                CSB_V1_F0425_DIALOG_VIEWPORT_BYTE_WIDTH_PC34;
            out_plan->foreground_color =
                CSB_V1_F0425_DIALOG_FOREGROUND_GOLD_PC34;
            out_plan->background_color =
                CSB_V1_F0425_DIALOG_BACKGROUND_LIGHT_BROWN_PC34;
            return 1;
        }
    }
    return -1;
}
