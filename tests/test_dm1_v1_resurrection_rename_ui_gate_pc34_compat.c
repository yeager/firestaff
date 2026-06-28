#include "firestaff/dm1/v1/resurrection_rename_ui_gate_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_assertions = 0;

static void check(int cond, const char *expr, const char *source, int line)
{
    ++g_assertions;
    if (!cond) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s:%d %s (%s)\n", __FILE__, line, expr, source);
    }
}

#define CHECK_REDMCSB(cond, source) check((cond), #cond, (source), __LINE__)

static void test_panel_command_gate(void)
{
    DM1_V1_ResurrectionRenameUiGatePc34Compat state;

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_RESURRECT_PC34_COMPAT);
    CHECK_REDMCSB(state.panelCommand == 160, "DEFS.H:338 C160");
    CHECK_REDMCSB(state.f0281RenameCallCount == 0,
                  "REVIVE.C F0282:806-807 C161-only F0281");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT,
                  "REVIVE.C F0282:806-807");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT) == 0,
                  "REVIVE.C F0281 is not live for C160");

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_CANCEL_PC34_COMPAT);
    CHECK_REDMCSB(state.panelCommand == 162, "DEFS.H:340 C162");
    CHECK_REDMCSB(state.f0281RenameCallCount == 0,
                  "REVIVE.C F0282:744-783 cancel path");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_NONE_PC34_COMPAT,
                  "REVIVE.C F0282:744-783");

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    CHECK_REDMCSB(state.panelCommand == 161, "DEFS.H:339 C161");
    CHECK_REDMCSB(state.f0281RenameCallCount == 1,
                  "REVIVE.C F0282:806-807");
    CHECK_REDMCSB(state.panelGraphicIndex ==
                      DM1_V1_RESURRECTION_RENAME_UI_PANEL_GRAPHIC_PC34_COMPAT,
                  "DEFS.H:2187 + REVIVE.C F0281:407");
    CHECK_REDMCSB(state.panelBox[0] == 80 && state.panelBox[1] == 223 &&
                      state.panelBox[2] == 52 && state.panelBox[3] == 124,
                  "DATA.C G0032 + REVIVE.C F0281:407");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT,
                  "REVIVE.C F0281:414-419");
    CHECK_REDMCSB(state.characterIndex == 0 && state.cursorX == 177 &&
                      state.cursorY == 91,
                  "REVIVE.C F0281:414-419");
    CHECK_REDMCSB(state.name[0] == '\0' && state.title[0] == '\0',
                  "REVIVE.C F0281:414-415");
}

static void test_name_gate_and_ok(void)
{
    DM1_V1_ResurrectionRenameUiGatePc34Compat state;

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:448 OK needs title mode or non-empty name");
    CHECK_REDMCSB(state.okAccepted == 0 && state.returned == 0,
                  "REVIVE.C F0281:448-464");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SPACE_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:518-520 leading space ignored");
    CHECK_REDMCSB(state.rejectedLeadingSpaceCount == 1 && state.name[0] == '\0',
                  "REVIVE.C F0281:518-520");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(
                      &state, 'z') == 1,
                  "REVIVE.C F0281:515-524 lowercase is uppercased");
    CHECK_REDMCSB(strcmp(state.name, "Z") == 0,
                  "REVIVE.C F0281:515-527");
    CHECK_REDMCSB(state.cursorX == 183 && state.cursorY == 91,
                  "REVIVE.C F0281:528 text step is 6 px");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SPACE_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:518-529 non-leading space accepted");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:448-464");
    CHECK_REDMCSB(strcmp(state.name, "Z") == 0,
                  "REVIVE.C F0281:451-454 trims right spaces");
    CHECK_REDMCSB(state.okAccepted == 1 && state.returned == 1,
                  "REVIVE.C F0281:464");
}

static void test_name_to_title_and_backspace(void)
{
    DM1_V1_ResurrectionRenameUiGatePc34Compat state;
    int i;

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT) == 0,
                  "REVIVE.C F0281:549-551 empty-name backspace ignored");
    CHECK_REDMCSB(state.ignoredBackspaceAtEmptyNameCount == 1,
                  "REVIVE.C F0281:549-551");
    for (i = 0; i < DM1_V1_RESURRECTION_RENAME_UI_NAME_MAX_PC34_COMPAT; ++i) {
        CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                          &state,
                          DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT + i) == 1,
                      "REVIVE.C F0281:526-530");
    }
    CHECK_REDMCSB(strcmp(state.name, "ABCDEFG") == 0,
                  "REVIVE.C F0281:526-530");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT,
                  "REVIVE.C F0281:529-545");
    CHECK_REDMCSB(state.characterIndex == 0 && state.cursorX == 105 &&
                      state.cursorY == 109,
                  "REVIVE.C F0281:541-545");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_BACKSPACE_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:557-567");
    CHECK_REDMCSB(strcmp(state.name, "ABCDEF") == 0,
                  "REVIVE.C F0281:557-567");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT &&
                      state.characterIndex == 6 && state.cursorX == 213 &&
                      state.cursorY == 91,
                  "REVIVE.C F0281:557-562");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_TITLE_PC34_COMPAT) == 1,
                  "DEFS.H:343 + REVIVE.C F0281:535-545");
    CHECK_REDMCSB(state.fieldMode ==
                      DM1_V1_RESURRECTION_RENAME_UI_FIELD_TITLE_PC34_COMPAT,
                  "REVIVE.C F0281:541-545");
}

static void test_title_specials_and_full_limit(void)
{
    DM1_V1_ResurrectionRenameUiGatePc34Compat state;
    int i;

    dm1_v1_resurrection_rename_ui_gate_init_pc34(
        &state, DM1_V1_RESURRECTION_RENAME_UI_COMMAND_REINCARNATE_PC34_COMPAT);
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(
                      &state, 'A') == 1,
                  "REVIVE.C F0281:518-529");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_ascii_pc34(
                      &state, '\r') == 1,
                  "REVIVE.C F0281:535-545");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COMMA_PC34_COMPAT) == 1,
                  "DEFS.H:370 + REVIVE.C F0281:498-500");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_PERIOD_PC34_COMPAT) == 1,
                  "DEFS.H:371 + REVIVE.C F0281:498-500");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_SEMICOLON_PC34_COMPAT) == 1,
                  "DEFS.H:372 + REVIVE.C F0281:498-500");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_COLON_PC34_COMPAT) == 1,
                  "DEFS.H:373 + REVIVE.C F0281:498-500");
    CHECK_REDMCSB(strcmp(state.title, ",.;:") == 0,
                  "DATA.C:430 + REVIVE.C F0281:498-500");
    for (i = 4; i < DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT; ++i) {
        CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                          &state,
                          DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT + 1) == 1,
                      "REVIVE.C F0281:526-529 title accepts up to 19 chars");
    }
    CHECK_REDMCSB((int)strlen(state.title) ==
                      DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT,
                  "REVIVE.C F0281:426-431 / 607-612 full-title guard");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_A_PC34_COMPAT + 2) == 0,
                  "REVIVE.C F0281:426-431 full title blocks append");
    CHECK_REDMCSB(state.rejectedFullTitleCount == 1,
                  "REVIVE.C F0281:426-431");
    CHECK_REDMCSB((int)strlen(state.title) ==
                      DM1_V1_RESURRECTION_RENAME_UI_TITLE_MAX_PC34_COMPAT,
                  "REVIVE.C F0281:426-431");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_apply_command_pc34(
                      &state,
                      DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT) == 1,
                  "REVIVE.C F0281:448-464");
}

static void test_source_evidence_and_self_test(void)
{
    const char *evidence =
        dm1_v1_resurrection_rename_ui_gate_source_evidence_pc34();
    CHECK_REDMCSB(evidence != 0, "source evidence string exists");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0281") != 0,
                  "REVIVE.C F0281 cited");
    CHECK_REDMCSB(strstr(evidence, "REVIVE.C F0282") != 0,
                  "REVIVE.C F0282 cited");
    CHECK_REDMCSB(strstr(evidence, "DATA.C") != 0,
                  "DATA.C rename strings cited");
    CHECK_REDMCSB(strstr(evidence, "DEFS.H") != 0,
                  "DEFS.H command constants cited");
    CHECK_REDMCSB(dm1_v1_resurrection_rename_ui_gate_run_self_test_pc34() == 1,
                  "self-test covers C160/C161/C162 + rename UI gate");
}

int main(void)
{
    test_panel_command_gate();
    test_name_gate_and_ok();
    test_name_to_title_and_backspace();
    test_title_specials_and_full_limit();
    test_source_evidence_and_self_test();

    if (g_failures != 0) {
        printf("FAIL dm1_v1_resurrection_rename_ui_gate_pc34_compat "
               "assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS dm1_v1_resurrection_rename_ui_gate_pc34_compat "
           "assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
