#ifndef FIRESTAFF_DM1_V1_DIALOG_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DIALOG_LAYOUT_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_DialogRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_DialogRectPc34;

typedef struct DM1_V1_DialogPatchPc34 {
    int srcX;
    int srcY;
    int w;
    int h;
    int dstX;
    int dstY;
} DM1_V1_DialogPatchPc34;

const char* dm1_v1_dialog_layout_source_evidence_pc34(void);

int dm1_v1_dialog_version_text_origin_pc34(int* outX, int* outY);
int dm1_v1_dialog_choice_patch_pc34(
    int choiceCount,
    DM1_V1_DialogPatchPc34* out);

int dm1_v1_dialog_single_choice_message_text_y_pc34(int lineCount);
int dm1_v1_dialog_multi_choice_message_text_y_pc34(int lineCount);
int dm1_v1_dialog_message_rect_pc34(
    int choiceCount,
    DM1_V1_DialogRectPc34* out);
int dm1_v1_dialog_message_width_pc34(int choiceCount);

int dm1_v1_dialog_choice_text_zone_id_pc34(
    int choiceCount,
    int choiceIndex);
int dm1_v1_dialog_choice_text_rect_pc34(
    int choiceCount,
    int choiceIndex,
    DM1_V1_DialogRectPc34* out);

int dm1_v1_dialog_choice_button_zone_id_pc34(
    int choiceCount,
    int choiceIndex);
int dm1_v1_dialog_choice_hit_rect_pc34(
    int choiceCount,
    int choiceIndex,
    DM1_V1_DialogRectPc34* out);

#ifdef __cplusplus
}
#endif

#endif
