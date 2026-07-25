#ifndef FIRESTAFF_CSB_V1_F0425_DIALOG_PRINT_CENTERED_CHOICE_RENDER_PLAN_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0425_DIALOG_PRINT_CENTERED_CHOICE_RENDER_PLAN_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DIALOG.C F0425 uses a 6-pixel monospaced glyph width, centers by
 * subtracting half of that width from X, then calls TEXT_Print with these
 * fixed viewport and color parameters. This adapter bounds the source-string
 * scan and returns the resulting print call as data; it does not draw. */
enum {
    CSB_V1_F0425_DIALOG_GLYPH_WIDTH_PC34 = 6,
    CSB_V1_F0425_DIALOG_VIEWPORT_BYTE_WIDTH_PC34 = 112,
    CSB_V1_F0425_DIALOG_FOREGROUND_GOLD_PC34 = 9,
    CSB_V1_F0425_DIALOG_BACKGROUND_LIGHT_BROWN_PC34 = 5
};

typedef struct CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat {
    const char *text;
    size_t text_length;
    int text_x;
    int text_y;
    int bitmap_byte_width;
    int foreground_color;
    int background_color;
} CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat;

/* Returns 1 when a source-faithful text-print plan was built, 0 for the
 * original NULL-string no-op, and -1 when text is not NUL-terminated within
 * text_capacity bytes or output is NULL. */
int csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
    const char *text,
    size_t text_capacity,
    int center_x,
    int text_y,
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat *out_plan);

#define F0425_DIALOG_PrintCenteredChoice \
    csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat

const char *csb_v1_f0425_dialog_print_centered_choice_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
