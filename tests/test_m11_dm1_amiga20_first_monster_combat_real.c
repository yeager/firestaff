/* Authentic Atari ST / Amiga M12 launch through first HoC combat encounter. */
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

/* The Atari M12 startup regression uses this source-input route to C127.
 * Do not reuse the Amiga route: its lateral movement sequence is different. */
static const M12_MenuInput route_to_atari_mirror[] = {
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_TURN_LEFT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_TURN_LEFT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_TURN_RIGHT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_TURN_RIGHT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_TURN_LEFT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP,
    M12_MENU_INPUT_TURN_RIGHT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_TURN_LEFT,
    M12_MENU_INPUT_UP, M12_MENU_INPUT_UP, M12_MENU_INPUT_TURN_LEFT
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
        fprintf(stderr, "FAIL: authentic platform route input ignored (%d)\n", input);
        return 0;
    }
    for (tick = 0; tick < 5; ++tick)
        (void)M11_GameView_AdvanceIdleTick(game);
    return 1;
}

int main(void)
{
    const char *atariArchive = getenv("FIRESTAFF_DM1_ATARI_ST_ARCHIVE");
    const int isAtari = atariArchive && atariArchive[0];
    const char *archive = isAtari
        ? atariArchive : getenv("FIRESTAFF_DM1_AMIGA_V20_ARCHIVE");
    const char *versionId = isAtari ? "st12-en" : "amiga20-en";
    const char *graphicsMd5 = isAtari
        ? "9ce2eaf7a9e78620e3f17594437caffa"
        : "6a2f135b53c2220f0251fa103e2a6e7e";
    M12_StartupMenuInitOptions options;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int version, result = 1;
    int menuInitialized = 0, gameInitialized = 0;
    size_t i;
    uint32_t attackBefore;
    unsigned short initialChampionHp;
    unsigned int naturalDamage;

    if (!archive || !archive[0]) {
        puts("SKIP: authentic DM1 Atari/Amiga original archive is not staged");
        return 77;
    }
    {
        FILE *media = fopen(archive, "rb");
        if (!media) {
            puts("SKIP: authentic DM1 Atari/Amiga original archive is not staged");
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
    version = M12_AssetStatus_FindVersionIndex("dm1", versionId);
    REQUIRE(version >= 0 && M12_AssetStatus_GetVersion(
                &menu.assetStatus, "dm1", (size_t)version)->matched &&
                strcmp(M12_AssetStatus_GetVersion(&menu.assetStatus, "dm1",
                    (size_t)version)->matchedMd5, graphicsMd5) == 0,
            "M12 authentic Atari/Amiga catalogue admission");
    REQUIRE(asset_file_matches_md5(M12_AssetStatus_GetVersion(
                &menu.assetStatus, "dm1", (size_t)version)->matchedPath,
                graphicsMd5),
            "original GRAPHICS.DAT bytes match the Atari/Amiga receipt");
    menu.selectedIndex = menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.gameOptions[0].architectureIndex = isAtari
        ? M12_ARCH_ATARI_ST : M12_ARCH_AMIGA;
    menu.gameOptions[0].versionIndex = version;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    M11_GameView_Init(&game);
    gameInitialized = 1;
    REQUIRE(M11_GameView_OpenSelectedMenuEntry(&game, &menu) &&
                game.active &&
                (isAtari ? game.assetLoader.atariStDm1
                         : game.assetLoader.legacyDm1) && game.world.dungeon &&
                game.world.things && game.world.party.mapIndex == 0 &&
                game.world.party.mapX == 1 && game.world.party.mapY == 3,
            "normal M12 selection launches the authentic platform world");

    if (isAtari) {
        for (i = 0; i < sizeof(route_to_atari_mirror) /
                            sizeof(route_to_atari_mirror[0]); ++i)
            REQUIRE(advance_input(&game, route_to_atari_mirror[i]),
                    "authentic Atari Hall route input accepted");
    } else {
        for (i = 0; i < sizeof(route_to_mirror) / sizeof(route_to_mirror[0]); ++i)
            REQUIRE(advance_input(&game, route_to_mirror[i]),
                    "authentic Amiga Hall route input accepted");
    }
    REQUIRE(M11_GameView_HandlePointerButton(&game, 112, 83,
                M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED &&
                game.candidateMirrorPanelActive &&
                game.candidateMirrorOrdinal == (isAtari ? 14 : 5),
            "authentic platform Hall route opens its original C127");
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
        unsigned char framebuffer[320 * 200];
        M11_Dm1WallOrnamentHostPresentationReceipt receipt;
        const M11_AssetSlot *source;
        const unsigned char *frame;
        unsigned int coordinateSet;
        int checkedPixels = 0;
        int indexedMatches = 0;
        int x;
        int y;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&game, framebuffer, 320, 200);
        memset(&receipt, 0, sizeof(receipt));
        M11_GameView_GetDm1WallOrnamentHostPresentationReceipt(&receipt);
        REQUIRE(receipt.valid && receipt.globalOrnamentIndex >= 0 &&
                    receipt.globalOrnamentIndex < 60 &&
                    receipt.viewWallIndex == 10 && receipt.width > 0 &&
                    receipt.height > 0 && receipt.destinationX >= 0 &&
                    receipt.destinationY >= 0 &&
                    receipt.destinationX + receipt.width <= 320 &&
                    receipt.destinationY + receipt.height <= 200,
                "authentic HoC wall ornament is presented inside the viewport");
        REQUIRE(receipt.graphicIndex ==
                    121 + receipt.globalOrnamentIndex * 2,
                "MEDIA009 F0107 selects the authentic back-facing source bitmap");
        coordinateSet = game.dm1NativeWallCoordinateSets[
            receipt.globalOrnamentIndex];
        REQUIRE(coordinateSet < 8,
                "HoC ornament selects a valid native coordinate set");
        source = M11_AssetLoader_Load(&game.assetLoader,
                                      (unsigned int)receipt.graphicIndex);
        frame = game.dm1NativeWallFrames[coordinateSet][receipt.viewWallIndex];
        REQUIRE(source && source->loaded && source->pixels &&
                    source->width == (unsigned int)frame[4] * 2u &&
                    source->height == frame[5] &&
                    receipt.width == (int)frame[1] - (int)frame[0] + 1 &&
                    receipt.height == (int)frame[3] - (int)frame[2] + 1,
                "HoC ornament dimensions come from native MEDIA009 data");
        /* View 10 uses the native unscaled, unflipped bitmap. Compare every
         * opaque source pixel with the actual M11 framebuffer so a valid
         * receipt cannot hide a no-draw, wrong-index, or corrupt blit. */
        for (y = 0; y < receipt.height; ++y) {
            for (x = 0; x < receipt.width; ++x) {
                unsigned char pixel = source->pixels[
                    y * (int)source->width + x];
                if (pixel == 10) continue;
                {
                    unsigned char actual = framebuffer[
                        (receipt.destinationY + y) * 320 +
                        receipt.destinationX + x];
                    if (actual == (unsigned char)(16u | pixel)) ++indexedMatches;
                }
                ++checkedPixels;
            }
        }
        REQUIRE(checkedPixels > 0,
                "authentic HoC ornament has visible source pixels");
        REQUIRE(indexedMatches == checkedPixels,
                "HoC viewport pixels match the authentic platform bitmap");
    }
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
    /* ReDMCSB COMMAND.C F0366 dispatches the native click; NEWMAP.C F0003
     * admits the original map-1 group before the first movement is resolved. */
    REQUIRE(M11_GameView_HandlePointerButton(&game, 167, 81,
                M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_IGNORED,
            "production pointer input opens the authentic first door");
    for (i = 0; i < 30; ++i) (void)M11_GameView_AdvanceIdleTick(&game);
    initialChampionHp = game.world.party.champions[0].hp.current;
    attackBefore = game.world.lifecycle.lastCreatureAttackTime;
    REQUIRE(M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP) !=
                M11_GAME_INPUT_IGNORED,
            "production movement crosses the first-door passage");
    for (i = 0; i < 5; ++i) (void)M11_GameView_AdvanceIdleTick(&game);
    REQUIRE(game.world.party.mapIndex == 1 && game.world.party.mapX == 6 &&
                game.world.party.direction == 2 &&
                game.world.lifecycle.lastCreatureAttackTime != attackBefore,
            "authentic map-1 creature reaches an attack attempt");
    REQUIRE(game.world.party.champions[0].hp.current < initialChampionHp,
            "authentic first monster attack deals damage to the champion");
    naturalDamage = (unsigned int)(initialChampionHp -
        game.world.party.champions[0].hp.current);
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
    printf("PASS: authentic %s M12 startup, HoC route and first-monster combat naturalDamage=%u\n",
           isAtari ? "Atari ST v1.2" : "Amiga v2.0", naturalDamage);

cleanup:
    if (gameInitialized) M11_GameView_Shutdown(&game);
    if (menuInitialized) M12_StartupMenu_Destroy(&menu);
    SDL_Quit();
    return result;
}
