/* Authentic Amiga v2.0 M12 launch through the first HoC combat encounter. */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "menu_input_m12.h"
#include "asset_find_by_hash.h"
#include "dm1_v1_creature_ai_behavior_pc34_compat.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRE(c, m) do { if (!(c)) { fprintf(stderr, "FAIL: %s\n", m); goto cleanup; } } while (0)

static const M12_MenuInput route_to_mirror[] = {
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_TURN_RIGHT,
    M12_MENU_INPUT_TURN_RIGHT
};

static const M12_MenuInput route_to_scroll[] = {
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_STRAFE_LEFT,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN,
    M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN, M12_MENU_INPUT_DOWN
};

static int advance_input(M11_GameViewState *game, M12_MenuInput input)
{
    int tick;
    if (M11_GameView_HandleInput(game, input) == M11_GAME_INPUT_IGNORED) {
        fprintf(stderr, "FAIL: authentic Amiga route input ignored (%d)\n", input);
        return 0;
    }
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(game);
    return 1;
}

int main(void)
{
    const char *archive = getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    M12_StartupMenuInitOptions options;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int version, result = 1;
    int menuInitialized = 0, gameInitialized = 0;
    size_t i;
    unsigned short initialHp;
    uint32_t attackBefore;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Amiga v2.0 archive is not staged");
        return 77;
    }
    {
        FILE *media = fopen(archive, "rb");
        if (!media) {
            puts("SKIP: authentic DM1 Amiga v2.0 archive is not staged");
            return 77;
        }
        fclose(media);
    }
    memset(&menu, 0, sizeof(menu));
    memset(&game, 0, sizeof(game));
    REQUIRE(SDL_Init(0), "initialize SDL for the production M12 startup path");
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, archive, "dm1", &options);
    menuInitialized = 1;
    version = M12_AssetStatus_FindVersionIndex("dm1", "amiga20-en");
    REQUIRE(version >= 0 && M12_AssetStatus_GetVersion(
                &menu.assetStatus, "dm1", (size_t)version)->matched &&
                strcmp(M12_AssetStatus_GetVersion(&menu.assetStatus, "dm1",
                    (size_t)version)->matchedMd5, "6a2f135b53c2220f0251fa103e2a6e7e") == 0,
            "M12 authentic Amiga v2.0 catalogue admission");
    REQUIRE(asset_file_matches_md5(M12_AssetStatus_GetVersion(
                &menu.assetStatus, "dm1", (size_t)version)->matchedPath,
                "6a2f135b53c2220f0251fa103e2a6e7e"),
            "authentic Amiga v2.0 GRAPHICS.DAT bytes match the original receipt");
    menu.selectedIndex = menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.gameOptions[0].architectureIndex = M12_ARCH_AMIGA;
    menu.gameOptions[0].versionIndex = version;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    M11_GameView_Init(&game);
    gameInitialized = 1;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(&game, &menu) &&
                game.active && game.assetLoader.legacyDm1 && game.world.dungeon &&
                game.world.things && game.world.party.mapIndex == 0 &&
                game.world.party.mapX == 1 && game.world.party.mapY == 3,
            "normal M12 selection launches the authentic Amiga v2.0 world");

    for (i = 0; i < sizeof(route_to_mirror) / sizeof(route_to_mirror[0]); ++i)
        REQUIRE(advance_input(&game, route_to_mirror[i]), "Hall route input accepted");
    REQUIRE(M11_GameView_HandlePointerButton(&game, 112, 83,
                M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED &&
                game.candidateMirrorPanelActive && game.candidateMirrorOrdinal == 5,
            "authentic Hall route opens C127 at ordinal 5");
    REQUIRE(M11_GameView_HandlePointerButton(&game, 130, 115,
                M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED &&
                game.world.party.championCount == 1,
            "production pointer input recruits the authentic C127 champion");

    for (i = 0; i < sizeof(route_to_scroll) / sizeof(route_to_scroll[0]); ++i)
        REQUIRE(advance_input(&game, route_to_scroll[i]), "HoC route input accepted");
    REQUIRE(game.world.party.mapIndex == 0 && game.world.party.mapX == 4 &&
                game.world.party.mapY == 15 && game.world.party.direction == 0,
            "authentic production movement reaches the HoC route");
    {
        static const M12_MenuInput to_door[] = {
            M12_MENU_INPUT_STRAFE_LEFT, M12_MENU_INPUT_UP,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_STRAFE_RIGHT,
            M12_MENU_INPUT_STRAFE_RIGHT, M12_MENU_INPUT_TURN_RIGHT,
            M12_MENU_INPUT_TURN_RIGHT
        };
        for (i = 0; i < sizeof(to_door) / sizeof(to_door[0]); ++i)
            REQUIRE(advance_input(&game, to_door[i]), "first-door route input accepted");
    }
    initialHp = game.world.party.champions[0].hp.current;
    attackBefore = game.world.lifecycle.lastCreatureAttackTime;
    /* ReDMCSB COMMAND.C F0366 dispatches the native click; NEWMAP.C F0003
     * admits the original map-1 group before the first movement is resolved. */
    REQUIRE(M11_GameView_HandlePointerButton(&game, 167, 81,
                M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED,
            "production pointer input opens the authentic first door");
    for (i = 0; i < 30; ++i) (void)M11_GameView_AdvanceIdleTick(&game);
    REQUIRE(M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP) !=
                M11_GAME_INPUT_IGNORED,
            "production movement crosses the first-door passage");
    for (i = 0; i < 5; ++i) (void)M11_GameView_AdvanceIdleTick(&game);
    REQUIRE(game.world.party.mapIndex == 1 && game.world.party.mapX == 6 &&
                game.world.party.direction == 2 &&
                game.world.party.champions[0].hp.current < initialHp &&
                game.world.lifecycle.lastCreatureAttackTime != attackBefore,
            "authentic Amiga map-1 monster naturally attacks and damages the party");
    {
        int found = 0;
        for (i = 0; i < (size_t)game.world.creatureAICount; ++i) {
            const struct CreatureAIState_Compat *ai = &game.world.creatureAI[i];
            if (ai->reserved0 == 169 && ai->groupMapIndex == 1 &&
                ai->stateKind == AI_STATE_ATTACK) {
                found = 1;
                break;
            }
        }
        REQUIRE(found && game.world.things->groupCount > 169 &&
                    game.world.things->groups[169].health[0] != 0,
                "authentic group 169 enters live attack behavior");
    }
    {
        unsigned char actions[3];
        int x, y, w, h;
        const int32_t ninjaXp = game.world.lifecycle.champions[0].skills20[1].experience;
        const int32_t fightXp = game.world.lifecycle.champions[0].skills20[9].experience;
        /* ReDMCSB MENU.C F0391/F0407 dispatches the visible PUNCH action;
         * MENU.C and CHAMPION.C award matching Fight/Ninja experience. */
        REQUIRE(M11_GameView_GetV1ActionIconCellZone(0, &x, &y, &w, &h) &&
                    M11_GameView_HandlePointerButton(&game, x + w / 2, y + h / 2,
                        M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED &&
                    game.actingChampionOrdinal == 1 &&
                    M11_GameView_GetActingActionIndices(&game, actions) &&
                    actions[0] == 6,
                "production pointer input selects first champion PUNCH action");
        REQUIRE(M11_GameView_GetV1ActionMenuRowZone(0, &x, &y, &w, &h) &&
                    M11_GameView_HandlePointerButton(&game, x + w / 2, y + h / 2,
                        M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED,
                "production pointer input dispatches PUNCH against the live encounter");
        for (i = 0; i < 5; ++i) (void)M11_GameView_AdvanceIdleTick(&game);
        REQUIRE(game.world.lifecycle.champions[0].skills20[1].experience > ninjaXp &&
                    game.world.lifecycle.champions[0].skills20[9].experience > fightXp &&
                    game.world.lifecycle.champions[0].skills20[1].experience - ninjaXp ==
                    game.world.lifecycle.champions[0].skills20[9].experience - fightXp,
                "authentic encounter PUNCH awards matching Ninja/Fight experience");
    }
    result = 0;
    puts("PASS: authentic Amiga v2.0 M12 startup, HoC route and first-monster combat");

cleanup:
    if (gameInitialized) M11_GameView_Shutdown(&game);
    if (menuInitialized) M12_StartupMenu_Destroy(&menu);
    SDL_Quit();
    return result;
}
