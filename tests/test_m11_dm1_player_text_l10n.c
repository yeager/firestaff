#include "firestaff_po_loader.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_text(const char *actual, const char *expected,
                        const char *label)
{
    if (!actual || strcmp(actual, expected) != 0) {
        fprintf(stderr, "FAIL: %s: got '%s', expected '%s'\n", label,
                actual ? actual : "(null)", expected);
        ++failures;
    }
}

int main(int argc, char **argv)
{
    M11_GameViewState state;
    M11_ForcedPauseDialogLayout pause;
    M11_ReturnConfirmDialogLayout confirm;

    if (argc != 2 || !fs_po_load(argv[1])) {
        fprintf(stderr, "FAIL: Swedish DM1 catalog was not loaded\n");
        return 1;
    }
    M11_GameView_Init(&state);
    state.sourceKind = M11_GAME_SOURCE_DIRECT_DUNGEON;

    if (!M11_GameView_ShowDialogOverlayChoices(
            &state, "RETURN TO START MENU?", "YES", "NO", NULL, NULL)) {
        fprintf(stderr, "FAIL: dialog was not admitted\n");
        return 1;
    }
    expect_text(state.dialogOverlayText, "ÅTERGÅ TILL STARTMENYN?",
                "dialog message final presentation");
    expect_text(state.dialogChoices[0], "JA", "first dialog choice");
    expect_text(state.dialogChoices[1], "NEJ", "second dialog choice");

    state.fontScale = 3;
    M11_GameView_GetReturnConfirmDialogLayout(&state, 320, 200, &confirm);
    expect_text(confirm.prompt, "AVSLUTA?", "compact return prompt");
    expect_text(confirm.choice0, "JA", "localized return choice");

    M11_GameView_GetForcedPauseDialogLayout(&state, 320, 200, &pause);
    expect_text(pause.title, "TIDEN UTE", "compact timer title");
    expect_text(pause.line1, "ENTER MENY", "compact timer menu action");
    expect_text(pause.line2, "ESC STÄNG", "compact timer dismiss action");

    if (failures != 0) return 1;
    puts("PASS: DM1 player-facing dialog transitions use Swedish at presentation");
    return 0;
}
