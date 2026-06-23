/*
 * DM1 V1 resurrect survival/load runtime probe.
 *
 * Source evidence:
 *   ReDMCSB REVIVE.C F0280 lines 227-242 materializes a mirror candidate
 *   with CurrentHealth/Stamina/Mana equal to their decoded maxima before
 *   the Resurrect/Reincarnate panel opens.  REVIVE.C F0282 lines 785-837
 *   confirms C160 resurrect without the C161 reincarnate stat-halving
 *   branch.  CHAMPION.C F0309 lines 1157-1177 computes maximum load from
 *   strength/stamina; F0310 lines 1180-1215 uses that value for movement
 *   cadence, so a resurrected champion must not be left with maxLoad == 0.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_champion_lifecycle_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int file_exists(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize)
{
    char graphicsPath[512];
    char dungeonPath[512];
    if (!dataDir || !out || outSize == 0U) return dataDir;
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/dm1/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/dm1/DUNGEON.DAT", dataDir);
    if (file_exists(graphicsPath) && file_exists(dungeonPath)) {
        snprintf(out, outSize, "%s/dm1", dataDir);
        return out;
    }
    return dataDir;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s=%d\n", label, got);
    return 1;
}

static int expect_true(const char* label, int ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static void set_halk_pose(M11_GameViewState* game)
{
    game->world.party.mapIndex = 0;
    game->world.party.mapX = 1;
    game->world.party.mapY = 2;
    game->world.party.direction = 0; /* DIR_NORTH */
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    char narrowedDataDir[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct ChampionState_Compat* champ;
    int ok = 1;
    int i;
    int hpAfterConfirm;
    int movementTicks;

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

    printf("probe=firestaff_dm1_v1_resurrect_survival_load_runtime_probe\n");
    printf("dataDir=%s\n", dataDir);

    set_halk_pose(&game);
    ok &= expect_int("front mirror ordinal", M11_GameView_GetFrontMirrorOrdinal(&game), 1);
    ok &= expect_int("select front candidate",
                     M11_GameView_SelectFrontMirrorCandidate(&game), 1);
    ok &= expect_int("candidate panel active", game.candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate party index", game.candidateMirrorPartyIndex, 0);

    champ = &game.world.party.champions[0];
    ok &= expect_int("candidate hp current=max",
                     champ->hp.current == champ->hp.maximum, 1);
    ok &= expect_true("candidate hp positive", champ->hp.current > 1);
    ok &= expect_int("candidate stamina current=max",
                     champ->stamina.current == champ->stamina.maximum, 1);
    ok &= expect_int("candidate mana current=max",
                     champ->mana.current == champ->mana.maximum, 1);
    ok &= expect_true("candidate maxLoad positive", champ->maxLoad > 0);

    ok &= expect_int("confirm resurrect", M11_GameView_ConfirmMirrorCandidate(&game, 0), 1);
    ok &= expect_int("post-confirm panel inactive", game.candidateMirrorPanelActive, 0);
    ok &= expect_int("post-confirm party count", game.world.party.championCount, 1);
    ok &= expect_int("post-confirm party alive", game.partyDead, 0);

    champ = &game.world.party.champions[0];
    hpAfterConfirm = (int)champ->hp.current;
    movementTicks = (int)F0841_LIFECYCLE_ComputeMoveTicks_Compat(
        champ->load, champ->maxLoad, champ->wounds, LIFECYCLE_ICON_NONE);

    ok &= expect_int("resurrect hp current=max",
                     champ->hp.current == champ->hp.maximum, 1);
    ok &= expect_true("resurrect hp positive", champ->hp.current > 1);
    ok &= expect_int("resurrect wounds clear", champ->wounds, 0);
    ok &= expect_int("resurrect poison clear", champ->poisonDose, 0);
    ok &= expect_true("resurrect food seeded", champ->food >= 1000);
    ok &= expect_true("resurrect water seeded", champ->water >= 1000);
    ok &= expect_true("resurrect maxLoad positive", champ->maxLoad > 0);
    ok &= expect_int("resurrect empty movement ticks", movementTicks, 2);

    for (i = 0; i < 600; ++i) {
        (void)M11_GameView_AdvanceIdleTick(&game);
    }

    champ = &game.world.party.champions[0];
    ok &= expect_int("idle party still alive", game.partyDead, 0);
    ok &= expect_true("idle hp still positive", champ->hp.current > 0);
    ok &= expect_int("idle hp not drained", champ->hp.current, hpAfterConfirm);

    printf("summary hp=%u/%u stamina=%u/%u mana=%u/%u food=%d water=%d load=%u maxLoad=%u ticks=%d partyDead=%d\n",
           (unsigned int)champ->hp.current,
           (unsigned int)champ->hp.maximum,
           (unsigned int)champ->stamina.current,
           (unsigned int)champ->stamina.maximum,
           (unsigned int)champ->mana.current,
           (unsigned int)champ->mana.maximum,
           (int)champ->food,
           (int)champ->water,
           (unsigned int)champ->load,
           (unsigned int)champ->maxLoad,
           movementTicks,
           game.partyDead);

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
