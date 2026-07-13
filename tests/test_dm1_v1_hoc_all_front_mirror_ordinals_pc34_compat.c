#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short next_thing(const struct DungeonThings_Compat* things,
                                 unsigned short thing)
{
    int type;
    int index;
    const unsigned char* raw;

    if (!things || thing == THING_NONE || thing == THING_ENDOFLIST) {
        return THING_ENDOFLIST;
    }
    type = (int)THING_GET_TYPE(thing);
    index = (int)THING_GET_INDEX(thing);
    if (type < 0 || type >= DUNGEON_THING_TYPE_COUNT ||
        index < 0 || index >= things->thingCounts[type] ||
        !things->rawThingData[type] ||
        s_thingDataByteCount[type] < 2) {
        return THING_ENDOFLIST;
    }
    raw = things->rawThingData[type] + index * (int)s_thingDataByteCount[type];
    return (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
}

static int open_game(const char* dataDir,
                     M12_StartupMenuState* menu,
                     M11_GameViewState* game)
{
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    return M11_GameView_OpenSelectedMenuEntry(game, menu);
}

static int step_x(int direction)
{
    static const int kDx[4] = { 0, 1, 0, -1 };
    return kDx[direction & 3];
}

static int step_y(int direction)
{
    static const int kDy[4] = { -1, 0, 1, 0 };
    return kDy[direction & 3];
}

static int portrait_cutout_matches(const unsigned char* left,
                                   const unsigned char* right)
{
    int x;
    int y;

    /* ReDMCSB DUNVIEW.C:3913-3928 draws C026 in the fixed D1C box. */
    for (y = 35; y < 64; ++y) {
        for (x = 96; x < 128; ++x) {
            if (left[y * 320 + x] != right[y * 320 + x]) {
                return 0;
            }
        }
    }
    return 1;
}

static int champion_name_matches(const struct ChampionState_Compat* champion,
                                 const char* expectedName)
{
    char unpacked[CHAMPION_NAME_TEXT_CAPACITY];

    if (!champion || !expectedName) {
        return 0;
    }
    F0628_CHAMPION_UnpackName_Compat(champion, unpacked, sizeof(unpacked));
    return strcmp(unpacked, expectedName) == 0;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const struct DungeonMapDesc_Compat* map;
    int seen[32];
    int expectedCount = 0;
    int firstPartyX = -1;
    int firstPartyY = -1;
    int firstDirection = -1;
    int firstOrdinal = -1;
    int ok = 1;
    int x;
    int y;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    memset(seen, 0, sizeof(seen));

    if (!open_game(dataDir, &menu, &game)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game from %s\n", dataDir);
        return 1;
    }
    if (!game.world.dungeon || !game.world.things ||
        game.world.dungeon->header.mapCount < 1 ||
        !game.world.things->sensors) {
        fprintf(stderr, "FAIL DM1 dungeon/things unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    map = &game.world.dungeon->maps[0];
    for (x = 0; x < (int)map->width; ++x) {
        for (y = 0; y < (int)map->height; ++y) {
            unsigned short thing =
                F0511_DUNGEON_GetSquareFirstThing_Compat(
                    game.world.dungeon, game.world.things, 0, x, y);
            int safety = 0;

            while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                   safety < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sensorIndex = (int)THING_GET_INDEX(thing);
                    if (sensorIndex >= 0 &&
                        sensorIndex < game.world.things->sensorCount &&
                        game.world.things->sensors[sensorIndex].sensorType == 127) {
                        int cell = (int)THING_GET_CELL(thing);
                        int expectedOrdinal =
                            (int)game.world.things->sensors[sensorIndex].sensorData;
                        int direction = (cell + 2) & 3;
                        int partyX = x - step_x(direction);
                        int partyY = y - step_y(direction);
                        int gotOrdinal;

                        if (expectedOrdinal >= 0 &&
                            expectedOrdinal < game.mirrorCatalog.count &&
                            partyX >= 0 && partyX < (int)map->width &&
                            partyY >= 0 && partyY < (int)map->height) {
                            game.world.party.mapIndex = 0;
                            game.world.party.mapX = partyX;
                            game.world.party.mapY = partyY;
                            game.world.party.direction = direction;
                            game.candidateMirrorPanelActive = 0;
                            game.candidateMirrorOrdinal = -1;
                            game.candidateMirrorPartyIndex = -1;

                            gotOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
                            if (gotOrdinal != expectedOrdinal) {
                                fprintf(stderr,
                                        "FAIL HoC mirror at sensor=(%d,%d) cell=%d party=(%d,%d,%d) got=%d want=%d\n",
                                        x, y, cell, partyX, partyY, direction,
                                        gotOrdinal, expectedOrdinal);
                                ok = 0;
                            } else {
                                if (!seen[expectedOrdinal]) {
                                    seen[expectedOrdinal] = 1;
                                    expectedCount++;
                                }
                                if (firstOrdinal < 0) {
                                    firstPartyX = partyX;
                                    firstPartyY = partyY;
                                    firstDirection = direction;
                                    firstOrdinal = expectedOrdinal;
                                }
                            }
                        }
                    }
                }
                thing = next_thing(game.world.things, thing);
                ++safety;
            }
        }
    }

    if (expectedCount != 24) {
        fprintf(stderr, "FAIL HoC visible mirror ordinal count got=%d want=24\n",
                expectedCount);
        ok = 0;
    }
    for (x = 0; x < 24; ++x) {
        if (!seen[x]) {
            fprintf(stderr, "FAIL HoC mirror ordinal %d was not reachable\n", x);
            ok = 0;
        }
    }

    /* REVIVE.C F0281 clears the temporary candidate name/title for C161,
     * whereas F0282 C162 removes that candidate and leaves the C127 mirror
     * available. Reopening the same real PC34 source must rematerialize its
     * original C026 portrait/name before C160 consumes the source sensor. */
    if (firstOrdinal < 0) {
        fprintf(stderr, "FAIL no real HoC C127 mirror available for C160 audit\n");
        ok = 0;
    } else {
        unsigned char before[320 * 200];
        unsigned char afterCancel[320 * 200];
        unsigned char afterConfirm[320 * 200];
        unsigned char portrait[CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT];
        char expectedName[16];
        int candidateIndex;
        int partyCountBefore;

        expectedName[0] = '\0';
        game.world.party.mapIndex = 0;
        game.world.party.mapX = firstPartyX;
        game.world.party.mapY = firstPartyY;
        game.world.party.direction = firstDirection;
        game.candidateMirrorPanelActive = 0;
        game.candidateMirrorOrdinal = -1;
        game.candidateMirrorPartyIndex = -1;
        M11_GameView_Draw(&game, before, 320, 200);
        partyCountBefore = game.world.party.championCount;

        if (!M11_GameView_GetMirrorNameByOrdinal(&game, firstOrdinal,
                                                  expectedName,
                                                  (int)sizeof(expectedName)) ||
            M11_GameView_GetFrontMirrorOrdinal(&game) != firstOrdinal ||
            !M11_GameView_SelectFrontMirrorCandidate(&game)) {
            fprintf(stderr, "FAIL HoC C127 candidate could not open at ordinal=%d\n",
                    firstOrdinal);
            ok = 0;
        } else {
            candidateIndex = game.candidateMirrorPartyIndex;
            if (candidateIndex < 0 || candidateIndex >= CHAMPION_MAX_PARTY ||
                !game.world.party.champions[candidateIndex].present ||
                !game.world.party.champions[candidateIndex].portraitBitmapValid ||
                !champion_name_matches(&game.world.party.champions[candidateIndex],
                                       expectedName)) {
                fprintf(stderr, "FAIL HoC C127 candidate did not materialize original name/portrait\n");
                ok = 0;
            } else {
                memcpy(portrait,
                       game.world.party.champions[candidateIndex].portraitBitmap,
                       sizeof(portrait));
                if (!M11_GameView_BeginMirrorCandidateReincarnateRename(&game) ||
                    game.world.party.champions[candidateIndex].name[0] != '\0' ||
                    !M11_GameView_CancelMirrorCandidate(&game)) {
                    fprintf(stderr, "FAIL HoC C161/C162 candidate cancel sequence\n");
                    ok = 0;
                } else {
                    M11_GameView_Draw(&game, afterCancel, 320, 200);
                    if (game.candidateMirrorPanelActive ||
                        game.candidateMirrorOrdinal != -1 ||
                        game.candidateMirrorPartyIndex != -1 ||
                        game.world.party.championCount != partyCountBefore ||
                        game.world.party.champions[candidateIndex].present ||
                        M11_GameView_GetFrontMirrorOrdinal(&game) != firstOrdinal ||
                        strcmp(game.inspectTitle, "CHAMPION MIRROR") != 0 ||
                        strstr(game.inspectDetail, "SELECTION CANCELLED") == NULL ||
                        !portrait_cutout_matches(before, afterCancel)) {
                        fprintf(stderr,
                                "FAIL HoC C162 left stale candidate state or altered source portrait\n");
                        ok = 0;
                    } else if (!M11_GameView_SelectFrontMirrorCandidate(&game) ||
                               game.candidateMirrorPartyIndex != candidateIndex ||
                               !champion_name_matches(
                                   &game.world.party.champions[candidateIndex],
                                   expectedName) ||
                               !game.world.party.champions[candidateIndex].portraitBitmapValid ||
                               memcmp(game.world.party.champions[candidateIndex].portraitBitmap,
                                      portrait, sizeof(portrait)) != 0 ||
                               M11_GameView_ConfirmMirrorCandidate(&game, 0) != 1) {
                        fprintf(stderr,
                                "FAIL HoC C160 did not rematerialize original name/portrait after C162\n");
                        ok = 0;
                    } else {
                        M11_GameView_Draw(&game, afterConfirm, 320, 200);
                        if (game.candidateMirrorPanelActive ||
                            game.candidateMirrorOrdinal != -1 ||
                            game.candidateMirrorPartyIndex != -1 ||
                            !game.world.party.champions[candidateIndex].present ||
                            !champion_name_matches(
                                &game.world.party.champions[candidateIndex],
                                expectedName) ||
                            memcmp(game.world.party.champions[candidateIndex].portraitBitmap,
                                   portrait, sizeof(portrait)) != 0 ||
                            M11_GameView_GetFrontMirrorOrdinal(&game) != -1 ||
                            strstr(game.inspectTitle, "MIRROR:") != NULL ||
                            strstr(game.inspectDetail, "CHOOSE") != NULL ||
                            portrait_cutout_matches(before, afterConfirm)) {
                            fprintf(stderr,
                                    "FAIL HoC C160 left stale mirror name/portrait after confirm\n");
                            ok = 0;
                        }
                    }
                }
            }
        }
    }

    printf("probe=dm1_v1_hoc_all_front_mirror_ordinals_pc34_compat\n");
    printf("sourceEvidence=ReDMCSB DUNGEON.C:2573,2608-2612 C127 front-wall portrait; MOVESENS.C:1501-1503; REVIVE.C F0280,F0281,F0282:744-805\n");
    printf("visibleMirrorOrdinals=%d\n", expectedCount);

    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
