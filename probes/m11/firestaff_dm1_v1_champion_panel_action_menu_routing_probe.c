/*
 * DM1 V1 champion-panel action-menu routing runtime probe.
 *
 * This is a data-free Firestaff runtime gate. It seeds a deterministic
 * champion panel, drives the real M11 pointer bridge through the source
 * action-icon cells, and checks the visible DM1 action-area state machine:
 * action cell -> acting ordinal/menu, row click -> trigger/clear, same-cell
 * click -> clear, and dead/absent/no-action-set cells -> ignored.
 *
 * Source evidence:
 *   ReDMCSB MENUS.C F0389 stores G0506_ui_ActingChampionOrdinal when the
 *   clicked champion has a usable action-hand ActionSet, with the empty-hand
 *   fallback to PUNCH/KICK/WAR CRY.
 *   ReDMCSB MENUS.C F0388 clears G0506 and returns the action area to the
 *   icon-cell branch.
 *   ReDMCSB MENUS.C F0391 routes action-menu rows through the selected
 *   ActionList entry and clears the acting champion after a real action.
 *   ReDMCSB MENUS.C F0386 suppresses action icons for objects whose
 *   ObjectInfo.ActionSetIndex is zero (scrolls/containers/etc.).
 */
#include "m11_game_view.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_CHAMPION_COUNT = 4,
    PROBE_TARGET_SLOT = 2
};

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static unsigned short thing_ref(int type, int index) {
    return (unsigned short)(((type & 0x0F) << 10) | (index & 0x03FF));
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = 90;
    champ->hp.maximum = 100;
    champ->stamina.current = 70;
    champ->stamina.maximum = 80;
    champ->mana.current = 40;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->active = 1;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 0;
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST);
}

static int click_action_cell(M11_GameViewState* game, int slot) {
    int x, y, w, h;
    if (!M11_GameView_GetV1ActionIconCellZone(slot, &x, &y, &w, &h)) {
        return -1;
    }
    return (int)M11_GameView_HandlePointerButton(
        game, x + w / 2, y + h / 2, M11_DM1_MOUSE_MASK_LEFT);
}

static int click_action_row(M11_GameViewState* game, int row) {
    int x, y, w, h;
    if (!M11_GameView_GetV1ActionMenuRowZone(row, &x, &y, &w, &h)) {
        return -1;
    }
    return (int)M11_GameView_HandlePointerButton(
        game, x + w / 2, y + h / 2, M11_DM1_MOUSE_MASK_LEFT);
}

static int expect_empty_hand_actions(const M11_GameViewState* game,
                                     const char* label) {
    unsigned char actions[3] = {0, 0, 0};
    int got = M11_GameView_GetActingActionIndices(game, actions);
    char itemLabel[160];
    int ok = 1;

    snprintf(itemLabel, sizeof(itemLabel), "%s got action tuple", label);
    ok &= expect_true(itemLabel, got == 1);
    snprintf(itemLabel, sizeof(itemLabel), "%s action[0] PUNCH", label);
    ok &= expect_int(itemLabel, (int)actions[0], 6);
    snprintf(itemLabel, sizeof(itemLabel), "%s action[1] KICK", label);
    ok &= expect_int(itemLabel, (int)actions[1], 7);
    snprintf(itemLabel, sizeof(itemLabel), "%s action[2] WAR CRY", label);
    ok &= expect_int(itemLabel, (int)actions[2], 8);
    return ok;
}

int main(int argc, char** argv) {
    M11_GameViewState game;
    struct DungeonThings_Compat noActionThings;
    int logBefore;
    int tickBefore;
    int ok = 1;

    (void)argc;
    (void)argv;

    memset(&game, 0, sizeof(game));
    M11_GameView_Init(&game);
    seed_party(&game);

    ok &= expect_int("initial idle acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);

    ok &= expect_int("action-cell click opens target champion menu",
                     click_action_cell(&game, PROBE_TARGET_SLOT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("target acting ordinal stored as DM1 ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game),
                     PROBE_TARGET_SLOT + 1);
    ok &= expect_empty_hand_actions(&game, "target empty hand");

    logBefore = M11_GameView_GetMessageLogCount(&game);
    tickBefore = (int)game.world.gameTick;
    ok &= expect_int("row click through pointer closes action menu",
                     click_action_row(&game, 2),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("row click clears acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);
    ok &= expect_true("row click emits a visible action log",
                      M11_GameView_GetMessageLogCount(&game) > logBefore);
    ok &= expect_true("row click advances or preserves monotonic tick",
                      (int)game.world.gameTick >= tickBefore);
    ok &= expect_int("row click makes acting champion the leader",
                     game.world.party.activeChampionIndex, PROBE_TARGET_SLOT);

    ok &= expect_int("reactivate same action cell",
                     click_action_cell(&game, PROBE_TARGET_SLOT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("same cell active before toggle",
                     (int)M11_GameView_GetActingChampionOrdinal(&game),
                     PROBE_TARGET_SLOT + 1);
    ok &= expect_int("same action-cell click toggles menu off",
                     click_action_cell(&game, PROBE_TARGET_SLOT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("toggle clears acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);

    ok &= expect_int("different living empty-hand cell opens",
                     click_action_cell(&game, 0),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("different cell stores its ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 1);
    M11_GameView_ClearActingChampion(&game);

    game.world.party.champions[1].hp.current = 0;
    ok &= expect_int("dead champion action cell ignored",
                     click_action_cell(&game, 1),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("dead champion leaves action menu idle",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);
    game.world.party.champions[1].hp.current = 90;

    memset(&noActionThings, 0, sizeof(noActionThings));
    game.world.things = &noActionThings;
    game.world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] =
        thing_ref(THING_TYPE_SCROLL, 0);
    ok &= expect_int("ActionSetIndex zero object action cell ignored",
                     click_action_cell(&game, 1),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("ActionSetIndex zero object keeps menu idle",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);
    game.world.party.champions[1].inventory[CHAMPION_SLOT_ACTION_HAND] =
        THING_NONE;
    game.world.things = NULL;

    game.world.party.championCount = 3;
    ok &= expect_int("absent fourth champion action cell ignored",
                     click_action_cell(&game, 3),
                     M11_GAME_INPUT_IGNORED);
    ok &= expect_int("absent fourth champion keeps menu idle",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel action-menu routing probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
