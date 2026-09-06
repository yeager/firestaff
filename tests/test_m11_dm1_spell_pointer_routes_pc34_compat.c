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
    state->world.party.championCount = 2;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[1].present = 1;
    state->world.party.champions[1].hp.current = 100;
    state->world.party.champions[1].hp.maximum = 100;
    state->world.party.champions[0].mana.current = 100;
    state->world.party.champions[1].mana.current = 100;
}

int main(void)
{
    M11_GameViewState state;
    M11_GameViewState csbState;

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

    /* C109 maps only the outer spell strip.  F0393 owns the precise tab
     * rectangles; caster changes must keep each Champion.Symbols[] and
     * SymbolStep independent from the party leader. */
    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 281, 43, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C109 selects champion 1 through its F0393 tab rectangle");
    CHECK_EQ(state.world.party.activeChampionIndex, 0,
             "C109 does not change the party leader");
    CHECK_EQ(state.dm1SpellCasting.magicCasterIndex, 1,
             "F0394 owns G0514 as champion 1");
    CHECK_EQ(state.spellBuffer.runeCount, 0,
             "champion 1 starts with its own empty Symbols array");
    CHECK_EQ(state.spellRuneRow, 0,
             "champion 1 starts at its own SymbolStep");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 236, 52, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C101 appends to champion 1 only");
    CHECK_EQ(state.dm1SpellCasting.input[1].symbols[0], 0x60,
             "champion 1 stores its own Lo symbol");
    CHECK_EQ(state.dm1SpellCasting.input[0].symbols[0], 0x60,
             "champion 0 retains its earlier Lo symbol");
    CHECK_EQ(state.dm1SpellCasting.input[1].symbolStep, 1,
             "champion 1 advances its own SymbolStep");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 234, 43, 0x0002),
             M11_GAME_INPUT_REDRAW,
             "C109 restores champion 0's source tab state");
    CHECK_EQ(state.dm1SpellCasting.magicCasterIndex, 0,
             "F0394 returns G0514 to champion 0");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "champion 0's stored Symbols reload without leakage");
    CHECK_EQ(state.spellBuffer.runes[0], 0x60,
             "champion 0's stored rune is preserved");
    CHECK_EQ(state.spellRuneRow, 1,
             "champion 0's stored SymbolStep is preserved");

    state.world.party.champions[1].hp.current = 0;
    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 281, 43, 0x0002),
             M11_GAME_INPUT_IGNORED,
             "F0394 rejects a dead champion tab");
    CHECK_EQ(state.dm1SpellCasting.magicCasterIndex, 0,
             "dead C109 tab preserves current magic caster");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "dead C109 tab cannot mutate current Symbols");

    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 234, 51, 0x0002),
             M11_GAME_INPUT_IGNORED,
             "pixel left of C245 cannot enter a rune");
    CHECK_EQ(state.spellBuffer.runeCount, 1,
             "out-of-zone click cannot mutate spell data");

    /* SYMBOL.C F0399/F0400: charge on input, no refund on deletion,
     * reject an unaffordable rune atomically, and restart after four. */
    CHECK_EQ(state.world.party.champions[0].mana.current, 96,
             "Lo and Vi cost 1+3 even after Vi was recanted");
    CHECK_EQ(state.world.party.champions[1].mana.current, 99,
             "second caster pays only its own Lo");
    state.world.party.champions[0].mana.current = 2;
    CHECK_EQ(M11_GameView_EnterRune(&state, 1), 0,
             "Vi rejects with only two mana");
    CHECK_EQ(state.spellBuffer.runeCount, 1, "rejection preserves Lo");
    CHECK_EQ(state.world.party.champions[0].mana.current, 2,
             "rejection preserves mana");
    state.world.party.champions[0].mana.current = 40;
    CHECK_EQ(M11_GameView_EnterRune(&state, 1), 1, "Vi accepted");
    CHECK_EQ(M11_GameView_EnterRune(&state, 4), 1, "Bro accepted");
    CHECK_EQ(M11_GameView_EnterRune(&state, 4), 1, "Ra accepted");
    CHECK_EQ(state.spellRuneRow, 0, "fourth symbol wraps step");
    CHECK_EQ(state.spellBuffer.runeCount, 4, "four symbols remain castable");
    CHECK_EQ(M11_GameView_HandlePointerButton(&state, 306, 64, 0x0002),
             M11_GAME_INPUT_REDRAW, "recant removes the fourth symbol");
    CHECK_EQ(state.spellRuneRow, 3, "recant wraps back to class row");
    CHECK_EQ(state.spellBuffer.runeCount, 3, "recant retains three symbols");
    CHECK_EQ(state.world.party.champions[0].mana.current, 24,
             "recant does not refund the fourth rune");
    CHECK_EQ(M11_GameView_EnterRune(&state, 4), 1, "replacement Ra pays again");
    CHECK_EQ(M11_GameView_EnterRune(&state, 5), 1, "fifth symbol starts Mon");
    CHECK_EQ(state.spellBuffer.runeCount, 1, "fifth symbol truncates old chain");
    CHECK_EQ(state.spellBuffer.runes[0], 101, "new power symbol is Mon");
    CHECK_EQ(state.world.party.champions[0].mana.current, 12,
             "Vi Bro Ra Ra Mon consume 3+7+6+6+6 without refund");
    M11_GameView_Shutdown(&state);

    /* CSB shares the C100/C101 geometry, but the rune path also consumes
     * verified CSB graphics/runtime data.  A bare host fixture must not
     * fabricate that data just to make the shared boxes appear interactive. */
    seed_state(&csbState);
    csbState.sourceKind = M11_GAME_SOURCE_CSB_BOOT;
    CHECK_EQ(M11_GameView_HandlePointerButton(&csbState, 234, 43, 0x0002),
             M11_GAME_INPUT_IGNORED,
             "CSB C100 rejects an unbound runtime fixture");
    CHECK_EQ(csbState.spellPanelOpen, 0,
             "CSB C100 cannot expose an inert source spell panel");
    CHECK_EQ(M11_GameView_HandlePointerButton(&csbState, 236, 52, 0x0002),
             M11_GAME_INPUT_IGNORED,
             "CSB C101 rejects an unbound runtime fixture");
    CHECK_EQ(csbState.spellBuffer.runeCount, 0,
             "CSB C101 cannot mutate an unbound spell buffer");
    M11_GameView_Shutdown(&csbState);
    return failures ? 1 : 0;
}
