#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat.h"

int main(void)
{
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat plan;
    CsbV1F0425DialogPrintCenteredChoiceRenderPlanPc34Compat unchanged;
    (void)unchanged;
    static const char choice[] = "CHOICE";
    (void)choice;
    static const char odd_choice[] = "YES";
    (void)odd_choice;
    char unterminated[3] = {'N', 'O', '!'};
    (void)unterminated;

    memset(&plan, 0, sizeof(plan));
    unchanged = plan;
    assert(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
               NULL, 0, 112, 114, &plan) == 0);
    assert(memcmp(&plan, &unchanged, sizeof(plan)) == 0);

    assert(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
               choice, sizeof(choice), 112, 77, &plan) == 1);
    assert(plan.text == choice);
    assert(plan.text_length == 6U);
    assert(plan.text_x == 94);
    assert(plan.text_y == 77);
    assert(plan.bitmap_byte_width == 112);
    assert(plan.foreground_color == 9);
    assert(plan.background_color == 5);

    assert(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
               odd_choice, sizeof(odd_choice), 59, 114, &plan) == 1);
    assert(plan.text_length == 3U);
    assert(plan.text_x == 50);

    memset(&plan, 0x5a, sizeof(plan));
    unchanged = plan;
    assert(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
               unterminated, sizeof(unterminated), 112, 114, &plan) == -1);
    assert(memcmp(&plan, &unchanged, sizeof(plan)) == 0);
    assert(csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat(
               choice, sizeof(choice), 112, 114, NULL) == -1);
    assert(strstr(csb_v1_f0425_dialog_print_centered_choice_source_evidence_pc34(),
                  "DIALOG.C:246-256 F0425") != NULL);

    puts("PASS csb_v1_f0425_dialog_print_centered_choice_render_plan_pc34_compat");
    return 0;
}
