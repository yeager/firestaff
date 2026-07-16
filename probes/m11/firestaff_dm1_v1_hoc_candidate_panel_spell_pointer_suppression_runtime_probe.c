/*
 * DM1 V1 Hall-of-Champions candidate-panel spell pointer suppression probe.
 *
 * Opens hash-verified DM1 PC 3.4 data, uses HALK's shipped C127 mirror
 * sensor, then clicks two physical C013 spell-area points through the M11
 * pointer bridge.  C040 owns the input while its candidate panel is live.
 *
 * ReDMCSB: COMMAND.C F0380 lines 2303-2311 gates the C100 spell family on
 * !G0299; REVIVE.C F0280 lines 272-276 sets G0299 for the selected mirror
 * candidate; DUNGEON.C F0172/F0174 and DUNVIEW.C F0128 supply the real
 * front-wall C127 route used here.
 */
#include "m11_game_view.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <stdio.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static int expect_true(const char* label, int value)
{
    if (!value) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static void set_real_halk_mirror_pose(M11_GameViewState* game)
{
    /* Shipped PC 3.4 DUNGEON.DAT: HALK C127 sensorData=1 is front-visible
     * from (7,9) facing north after the compact SquareFirstThings lookup. */
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 7;
    game->world.party.mapY = 9;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static int open_real_dm1(const char* data_dir,
                         M11_GameViewState* game)
{
    M11_GameView_Init(game);
    return M11_GameView_StartDm1(game, data_dir);
}

static int prepare_live_candidate(M11_GameViewState* game)
{
    int ok = 1;

    set_real_halk_mirror_pose(game);
    ok &= expect_int("real front mirror is HALK",
                     M11_GameView_GetFrontMirrorOrdinal(game), 1);
    ok &= expect_int("open spell panel", M11_GameView_OpenSpellPanel(game), 1);
    ok &= expect_int("enter first rune", M11_GameView_EnterRune(game, 0), 1);
    ok &= expect_int("enter second rune", M11_GameView_EnterRune(game, 0), 1);
    ok &= expect_int("select real front mirror candidate",
                     M11_GameView_SelectFrontMirrorCandidate(game), 1);
    ok &= expect_int("candidate panel active", game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal retained", game->candidateMirrorOrdinal, 1);
    ok &= expect_int("candidate party index retained", game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("candidate appended", game->world.party.championCount, 1);
    return ok;
}

static int check_spell_area_click_ignored(M11_GameViewState* game,
                                          int x,
                                          int y,
                                          const char* label)
{
    const int runes_before = game->spellBuffer.runeCount;
    const int panel_before = game->spellPanelOpen;
    const int ordinal_before = game->candidateMirrorOrdinal;
    const int party_before = game->world.party.championCount;
    const unsigned int tick_before = game->world.gameTick;
    int ok = 1;

    ok &= expect_int(label,
                     M11_GameView_HandlePointerButton(
                         game, x, y, DM1_V1_MOUSE_MASK_LEFT_PC34),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("spell panel remains open", game->spellPanelOpen, panel_before);
    ok &= expect_int("spell runes remain intact", game->spellBuffer.runeCount, runes_before);
    ok &= expect_int("candidate panel remains active", game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal remains intact",
                     game->candidateMirrorOrdinal, ordinal_before);
    ok &= expect_int("candidate party size remains intact",
                     game->world.party.championCount, party_before);
    ok &= expect_true("candidate spell click does not advance tick",
                      game->world.gameTick == tick_before);
    return ok;
}

int main(int argc, char** argv)
{
    const char* data_dir;
    DM1_V1_SpellAreaRectPc34 spell_area;
    M11_GameViewState game;
    int ok = 1;

    if (argc != 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    data_dir = argv[1];
    if (!open_real_dm1(data_dir, &game)) {
        fprintf(stderr, "FAIL could not open hash-verified DM1 data from %s\n",
                data_dir);
        return 1;
    }

    spell_area = dm1_v1_spell_area_click_rect_pc34();
    ok &= expect_true("C013 spell area is physical DM1 right-column rectangle",
                      spell_area.x == 233 && spell_area.y == 42 &&
                      spell_area.w == 87 && spell_area.h == 33);
    if (ok && prepare_live_candidate(&game)) {
        ok &= check_spell_area_click_ignored(
            &game, spell_area.x + 2, spell_area.y + 2,
            "candidate panel owns C013 spell-area near click");
        ok &= check_spell_area_click_ignored(
            &game, spell_area.x + spell_area.w - 3,
            spell_area.y + spell_area.h - 3,
            "candidate panel owns C013 spell-area far click");
    } else {
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    printf("%s DM1 V1 HoC candidate-panel spell pointer suppression probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
