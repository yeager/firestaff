/* DM1 runtime spell-pointer regression.
 *
 * ReDMCSB COMMAND.C G0447/G0454 and CLIKMENU.C F0370 route C100 through
 * the C013 parent box, then C101..C106/C107/C108 through layout-696 child
 * boxes. SYMBOL.C F0399 appends a rune; F0400 removes only the latest.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK_EQ(actual, expected, label) do { \
    int actual_ = (int)(actual); \
    int expected_ = (int)(expected); \
    if (actual_ != expected_) { \
        fprintf(stderr, "FAIL: %s: got %d expected %d\n", \
                label, actual_, expected_); \
        ++failures; \
    } \
} while (0)

static void seed_state(M11_GameViewState* state)
{
    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

int main(void)
{
    M11_GameViewState state;

    seed_state(&state);

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 234, 43, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C100 parent opens source spell panel");
    CHECK_EQ(state.spellPanelOpen, 1,
             "C100 leaves C009/C011 panel visible");
    CHECK_EQ(state.spellBuffer.runeCount, 0,
             "C100 starts with no selected rune");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 236, 52, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C101 first exact 13x11 rune box enters a rune");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "F0399 appends first rune");
    CHECK_EQ(state.spellBuffer.runes[0], 0x60,
             "F0399 encodes Lo from symbol 0 at step 0");
    CHECK_EQ(state.spellRuneRow, 1,
             "F0399 advances to the element row");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 250, 52, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C102 second exact 13x11 rune box enters a rune");
    CHECK_EQ(state.spellBuffer.runeCount, 2,
             "second source rune appends");
    CHECK_EQ(state.spellBuffer.runes[1], 0x67,
             "step 1 symbol 1 encodes Vi");
    CHECK_EQ(state.spellRuneRow, 2,
             "second rune advances to form row");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 306, 64, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C107 recant is consumed by its exact 14x11 box");
    CHECK_EQ(state.spellPanelOpen, 1,
             "F0400 keeps spell panel open");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "F0400 deletes only latest rune");
    CHECK_EQ(state.spellBuffer.runes[0], 0x60,
             "F0400 preserves earlier rune");
    CHECK_EQ(state.spellRuneRow, 1,
             "F0400 restores previous symbol step");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 234, 51, 0x0002),
             M11_GAME_INPUT_IGNORED,
             "pixel left of C245 cannot enter a rune");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "out-of-zone click cannot mutate spell data");

    M11_GameView_Shutdown(&state);
    return failures ? 1 : 0;
}
