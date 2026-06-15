/*
 * DM1 V1 live C040 mirror-candidate save/load regression.
 *
 * ReDMCSB anchors:
 * - REVIVE.C F0280:124-132 appends the mirror candidate and makes G0299 live.
 * - REVIVE.C F0282:744-806 clears or confirms the live C040 candidate.
 * - COMMAND.C F0359:1985-1990 routes C040 panel clicks while M568 owns input.
 * - CHEST.C F0333/F0334 owns G0426_T_OpenChest while a chest panel is open.
 * - PANEL.C F0346/F0347:1619-1657 draws C040 while G0299 is non-zero.
 *
 * Firestaff saves the source-locked GameWorld_Compat blob through
 * F0897/F0898, then persists transient V1 runtime panel state in the
 * quicksave .v1runtime sidecar. This test does not claim original DOS parity.
 */

#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int expect(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    const char* dataDir = getenv("FIRESTAFF_DM1_CANONICAL_DIR");
    char saveTemplate[] = "/tmp/firestaff-dm1-c040-save-XXXXXX";
    char savePath[512];
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    M11_GameViewState resumed;
    int previousPartyCount;
    int candidateIndex;
    int mirrorOrdinal = 0;
    int mapIndex = 2;
    int mapX = 11;
    int mapY = 7;
    int direction = 3;
    const unsigned short openChestThing =
        (unsigned short)((THING_TYPE_CONTAINER << 10) | 2U);

    if (!dataDir || dataDir[0] == '\0') {
        dataDir = "/Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1";
    }
    if (access(dataDir, R_OK) != 0) {
        printf("skip: DM1 canonical dir not available: %s\n", dataDir);
        return 0;
    }
    if (!mkdtemp(saveTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav",
             saveTemplate);
    test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);
    test_setenv("HOME", saveTemplate);

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!expect(M11_GameView_Start(&view, &spec),
                "initial DM1 start should succeed")) return 1;
    if (!expect(M11_GameView_GetMirrorCatalogCount(&view) > 0,
                "mirror catalog should be available")) return 1;
    if (!expect(view.world.party.championCount < CHAMPION_MAX_PARTY,
                "fixture must have room for a mirror candidate")) return 1;

    previousPartyCount = view.world.party.championCount;
    if (!expect(M11_GameView_RecruitChampionByMirrorOrdinal(&view, mirrorOrdinal) == 1,
                "fixture should append the mirror candidate")) return 1;
    candidateIndex = previousPartyCount;

    /* This mirrors the post-F0280 live panel state without dismissing C040:
     * the candidate is appended, G0299-equivalent panel state is active,
     * and a G0426-equivalent open chest is still present in the V1 runtime. */
    view.candidateMirrorOrdinal = mirrorOrdinal;
    view.candidateMirrorPartyIndex = candidateIndex;
    view.candidateMirrorPanelActive = 1;
    view.inventoryPanelActive = 1;
    view.world.party.activeChampionIndex = candidateIndex;
    view.world.party.mapIndex = mapIndex;
    view.world.party.mapX = mapX;
    view.world.party.mapY = mapY;
    view.world.party.direction = direction;
    view.world.gameTick = 674040U;
    view.v1OpenChestThing = openChestThing;
    view.v1OpenChestOpenedByEye = 0;

    if (!expect(F0891_ORCH_WorldHash_Compat(&view.world, &view.lastWorldHash),
                "world hash refresh should succeed before save")) return 1;
    if (!expect(M11_GameView_QuickSave(&view),
                "quick save should persist live C040 and open chest state")) return 1;
    M11_GameView_Shutdown(&view);

    spec.savePath = savePath;
    M11_GameView_Init(&resumed);
    if (!expect(M11_GameView_Start(&resumed, &spec),
                "quick-resume DM1 start should load save")) return 1;

    if (!expect(resumed.candidateMirrorPanelActive == 1,
                "C040 candidate panel should remain live after load")) return 1;
    if (!expect(resumed.inventoryPanelActive == 1,
                "inventory panel should remain visible for C040 after load")) return 1;
    if (!expect(resumed.candidateMirrorOrdinal == mirrorOrdinal,
                "candidate mirror ordinal should survive load")) return 1;
    if (!expect(resumed.candidateMirrorPartyIndex == candidateIndex,
                "candidate party index should survive load")) return 1;
    if (!expect(resumed.world.party.championCount == previousPartyCount + 1,
                "candidate should remain appended to the party")) return 1;
    if (!expect(resumed.world.party.champions[candidateIndex].present,
                "candidate slot should still be populated")) return 1;
    if (!expect(resumed.world.party.activeChampionIndex == candidateIndex,
                "candidate panel active champion index should survive load")) return 1;
    if (!expect(resumed.world.party.mapIndex == mapIndex &&
                resumed.world.party.mapX == mapX &&
                resumed.world.party.mapY == mapY &&
                resumed.world.party.direction == direction,
                "party pose should survive load")) return 1;
    if (!expect(M11_GameView_GetV1OpenChestThing(&resumed) == openChestThing,
                "open chest thing should survive load with C040 live")) return 1;
    if (!expect(resumed.v1OpenChestOpenedByEye == 0,
                "open chest route flag should survive load")) return 1;

    M11_GameView_Shutdown(&resumed);

    puts("ok: DM1 V1 live C040 mirror candidate and open chest survive quicksave/load");
    return 0;
}
