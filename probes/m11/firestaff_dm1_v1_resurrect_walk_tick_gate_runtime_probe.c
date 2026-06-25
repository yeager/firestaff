/*
 * DM1 V1 Hall of Champions resurrect-walk source-tick gate.
 *
 * Regresses the live-loop class where OS key-repeat can feed
 * M11_GameView_HandleInput faster than ReDMCSB's VBlank-gated game loop.
 * The source command core still owns stamina costs once a movement command is
 * consumed; this gate proves the M11 runtime filter does not let held walking
 * keys mint extra source ticks between 200 ms game-loop boundaries.
 *
 * Source evidence:
 *   ReDMCSB GAMELOOP.C F0002 lines ~122-155 resets the VBlank wait and
 *     decrements movement cooldowns once per game loop iteration.
 *   ReDMCSB COMMAND.C F0380 lines ~2075-2099 gates movement commands before
 *     they are dequeued.
 *   ReDMCSB CLIKMENU.C F0366 lines ~237-255 applies stamina only after a
 *     step command has actually been dequeued.
 *   ReDMCSB REVIVE.C F0280/F0282 materializes and confirms the resurrected
 *     Hall champion before normal movement resumes.
 */

#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass;
static int g_fail;

#define CHECK(label, cond) do { \
    if (cond) { \
        ++g_pass; \
        printf("  PASS: %s\n", label); \
    } else { \
        ++g_fail; \
        printf("  FAIL: %s\n", label); \
    } \
} while (0)

#define CHECK_INT(label, got, want) do { \
    int _got = (int)(got); \
    int _want = (int)(want); \
    if (_got == _want) { \
        ++g_pass; \
        printf("  PASS: %s == %d\n", label, _want); \
    } else { \
        ++g_fail; \
        printf("  FAIL: %s got=%d want=%d\n", label, _got, _want); \
    } \
} while (0)

static int file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) {
        return dataDir;
    }
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static void set_halk_pose(M11_GameViewState* game) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 1;
    game->world.party.mapY = 2;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static int gated_handle_input(M11_GameViewState* game, M12_MenuInput input) {
    if (M11_GameView_InputConsumesDm1V1SourceTick(game, input) &&
        !M11_GameView_Dm1V1SourceTickReadyForInput(game)) {
        return 0;
    }
    return M11_GameView_HandleInput(game, input) == M11_GAME_INPUT_REDRAW;
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowedDataDir[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct ChampionState_Compat* champ;
    unsigned short hp0;
    unsigned short stamina0;
    unsigned int tick0;
    int i;
    int rejected = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    dataDir = narrow_dm1_data_dir(argv[1], narrowedDataDir, sizeof(narrowedDataDir));
    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("probe=firestaff_dm1_v1_resurrect_walk_tick_gate_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);

    set_halk_pose(&game);
    CHECK_INT("front mirror ordinal HALK", M11_GameView_GetFrontMirrorOrdinal(&game), 1);
    CHECK_INT("select front candidate", M11_GameView_SelectFrontMirrorCandidate(&game), 1);
    CHECK_INT("confirm resurrect", M11_GameView_ConfirmMirrorCandidate(&game, 0), 1);
    CHECK_INT("party count after resurrect", game.world.party.championCount, 1);
    CHECK_INT("party alive after resurrect", game.partyDead, 0);

    champ = &game.world.party.champions[0];
    hp0 = champ->hp.current;
    stamina0 = champ->stamina.current;
    tick0 = game.world.gameTick;
    CHECK("resurrected HALK starts with positive HP", hp0 > 0);
    CHECK("resurrected HALK starts with positive stamina", stamina0 > 0);

    game.vblankTiming.stopWaitingForInput = 0;
    for (i = 0; i < 240; ++i) {
        rejected += gated_handle_input(&game, M12_MENU_INPUT_UP) ? 0 : 1;
    }

    champ = &game.world.party.champions[0];
    CHECK_INT("pre-vblank repeats rejected", rejected, 240);
    CHECK_INT("pre-vblank world tick unchanged", game.world.gameTick, tick0);
    CHECK_INT("pre-vblank HP unchanged", champ->hp.current, hp0);
    CHECK_INT("pre-vblank stamina unchanged", champ->stamina.current, stamina0);
    CHECK_INT("pre-vblank party still alive", game.partyDead, 0);

    game.vblankTiming.stopWaitingForInput = 1;
    CHECK_INT("single source-tick walk input accepted",
              gated_handle_input(&game, M12_MENU_INPUT_UP), 1);
    champ = &game.world.party.champions[0];
    CHECK("single source-tick walk leaves champion alive", champ->hp.current > 0);
    CHECK("single source-tick walk costs at most one stamina unit",
          (int)stamina0 - (int)champ->stamina.current <= 1);

    for (i = 0; i < 120; ++i) {
        game.vblankTiming.stopWaitingForInput = 1;
        (void)gated_handle_input(&game, M12_MENU_INPUT_UP);
        if (game.partyDead || game.world.party.champions[0].hp.current == 0) {
            break;
        }
    }
    champ = &game.world.party.champions[0];
    CHECK_INT("held walk source-ticked party still alive", game.partyDead, 0);
    CHECK("held walk source-ticked HP positive", champ->hp.current > 0);

    printf("summary passed=%d failed=%d hp=%u/%u stamina=%u/%u tick=%u\n",
           g_pass, g_fail,
           (unsigned int)champ->hp.current,
           (unsigned int)champ->hp.maximum,
           (unsigned int)champ->stamina.current,
           (unsigned int)champ->stamina.maximum,
           (unsigned int)game.world.gameTick);

    M11_GameView_Shutdown(&game);
    return g_fail == 0 ? 0 : 1;
}
