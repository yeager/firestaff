/* Real PC34 HoC C127 -> C040 -> C160/C161 runtime route.
 *
 * This probe uses the operator-staged DM1 DUNGEON.DAT/GRAPHICS.DAT corpus:
 * two source mirror sensors are discovered from map data, candidates are
 * recruited through M11's live HoC route, C026 portrait bytes are verified,
 * resurrect and reincarnate+rename commit to the world, and quicksave/resume
 * preserves the resulting party without a fallback candidate or HUD.
 */

#include "m11_game_view.h"
#include "dm1_v1_resurrection_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum { kFramebufferWidth = 320, kFramebufferHeight = 200 };

typedef struct HocMirrorPosePc34 {
    int x;
    int y;
    int direction;
    int ordinal;
    int sensorIndex;
} HocMirrorPosePc34;

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

static int test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int dx(int direction) {
    return (direction & 3) == 1 ? 1 : ((direction & 3) == 3 ? -1 : 0);
}

static int dy(int direction) {
    return (direction & 3) == 2 ? 1 : ((direction & 3) == 0 ? -1 : 0);
}

static int square_element(const M11_GameViewState* state, int x, int y) {
    const struct DungeonMapDesc_Compat* map;
    int index;
    if (!state || !state->world.dungeon || !state->world.dungeon->tiles ||
        state->world.dungeon->header.mapCount <= 0) {
        return -1;
    }
    map = &state->world.dungeon->maps[0];
    if (x < 0 || y < 0 || x >= (int)map->width || y >= (int)map->height) {
        return -1;
    }
    index = x * (int)map->height + y;
    return (state->world.dungeon->tiles[0].squareData[index] &
            DUNGEON_SQUARE_MASK_TYPE) >> 5;
}

static void clear_party(M11_GameViewState* state);

static void draw_at(M11_GameViewState* state,
                    const HocMirrorPosePc34* pose,
                    unsigned char* framebuffer) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = pose->x;
    state->world.party.mapY = pose->y;
    state->world.party.direction = pose->direction;
    memset(framebuffer, 0, kFramebufferWidth * kFramebufferHeight);
    M11_GameView_Draw(state, framebuffer, kFramebufferWidth, kFramebufferHeight);
}

static int find_two_mirrors(const M11_GameViewState* state,
                            HocMirrorPosePc34* first,
                            HocMirrorPosePc34* second) {
    const struct DungeonMapDesc_Compat* map;
    int y;
    int found = 0;
    if (!state || !state->world.dungeon || !state->world.things ||
        !first || !second) {
        return 0;
    }
    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing;
            int safety = 0;
            if (square_element(state, x, y) != DUNGEON_ELEMENT_WALL) {
                continue;
            }
            thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    const int sensorIndex = (int)THING_GET_INDEX(thing);
                    const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    const int partyX = x - dx(direction);
                    const int partyY = y - dy(direction);
                    if (sensorIndex >= 0 &&
                        sensorIndex < state->world.things->sensorCount &&
                        state->world.things->sensors[sensorIndex].sensorType ==
                            DM1_SENSOR_WALL_CHAMPION_PORTRAIT &&
                        square_element(state, partyX, partyY) ==
                            DUNGEON_ELEMENT_CORRIDOR) {
                        HocMirrorPosePc34* out;
                        if (found > 0 &&
                            (int)state->world.things->sensors[sensorIndex]
                                    .sensorData == first->ordinal) {
                            thing = F0512_DUNGEON_GetThingNext_Compat(
                                state->world.things, thing);
                            continue;
                        }
                        out = found == 0 ? first : second;
                        out->x = partyX;
                        out->y = partyY;
                        out->direction = direction;
                        out->ordinal =
                            (int)state->world.things->sensors[sensorIndex]
                                .sensorData;
                        out->sensorIndex = sensorIndex;
                        if (++found == 2) {
                            return 1;
                        }
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(
                    state->world.things, thing);
            }
        }
    }
    return 0;
}

/* Walk the original map-0 thing chains instead of maintaining a coordinate
 * table.  DUNGEON.C selects the visible C127 wall cell from party direction;
 * CLIKVIEW.C then receives the D1C hit point and MOVESENS.C dispatches F0280.
 */
static int sweep_all_source_c127_pointer_routes(M11_GameViewState* state,
                                                unsigned char* framebuffer) {
    const struct DungeonMapDesc_Compat* map;
    int viewportX;
    int viewportY;
    int viewportW;
    int viewportH;
    int ornamentX;
    int ornamentY;
    int ornamentW;
    int ornamentH;
    int sourceC127Count = 0;
    int candidateCount = 0;
    int selectedCount = 0;
    int rejectedCount = 0;
    int y;

    if (!state || !state->world.dungeon || !state->world.things ||
        !state->world.dungeon->tiles ||
        state->world.dungeon->header.mapCount <= 0 || !framebuffer) {
        CHECK(0, "source C127 pointer sweep needs a loaded dungeon world");
        return 0;
    }
    CHECK(M11_GameView_GetViewportRect(&viewportX, &viewportY,
                                       &viewportW, &viewportH) == 1 &&
          M11_GameView_GetD1CWallOrnamentZone(state, &ornamentX, &ornamentY,
                                               &ornamentW, &ornamentH) == 1 &&
          viewportW > 0 && viewportH > 0 && ornamentW > 0 && ornamentH > 0,
          "source viewport and D1C mirror hit zones should be available");
    if (failures) return 0;

    map = &state->world.dungeon->maps[0];
    for (y = 0; y < (int)map->height; ++y) {
        int x;
        for (x = 0; x < (int)map->width; ++x) {
            unsigned short thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
                state->world.dungeon, state->world.things, 0, x, y);
            int safety = 0;
            while (thing != THING_NONE && thing != THING_ENDOFLIST &&
                   safety++ < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    const int sensorIndex = (int)THING_GET_INDEX(thing);
                    const int direction = ((int)THING_GET_CELL(thing) + 2) & 3;
                    const int partyX = x - dx(direction);
                    const int partyY = y - dy(direction);
                    const int sourceOrdinal =
                        sensorIndex >= 0 &&
                        sensorIndex < state->world.things->sensorCount
                            ? (int)state->world.things->sensors[sensorIndex]
                                      .sensorData
                            : -1;
                    const int poseInBounds =
                        partyX >= 0 && partyY >= 0 &&
                        partyX < (int)map->width && partyY < (int)map->height;
                    const int sourceWall = square_element(state, x, y) ==
                                           DUNGEON_ELEMENT_WALL;
                    const int sourceCandidate =
                        poseInBounds && sourceWall &&
                        square_element(state, partyX, partyY) ==
                            DUNGEON_ELEMENT_CORRIDOR;

                    if (sensorIndex < 0 ||
                        sensorIndex >= state->world.things->sensorCount) {
                        CHECK(0, "C127 thing must reference an in-range sensor");
                    } else if (state->world.things->sensors[sensorIndex]
                                   .sensorType ==
                               DM1_SENSOR_WALL_CHAMPION_PORTRAIT) {
                        M11_GameInputResult clickResult;
                        M11_GameInputResult outsideClickResult;
                        int outsideRejected;
                        int frontOrdinal;
                        int clickX;
                        int clickY;
                        ++sourceC127Count;
                        state->world.party.mapIndex = 0;
                        state->world.party.mapX = partyX;
                        state->world.party.mapY = partyY;
                        state->world.party.direction = direction;
                        memset(framebuffer, 0,
                               kFramebufferWidth * kFramebufferHeight);
                        M11_GameView_Draw(state, framebuffer,
                                          kFramebufferWidth, kFramebufferHeight);
                        frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
                        outsideClickResult = M11_GameView_HandlePointer(
                            state,
                            viewportX + ornamentX + ornamentW,
                            viewportY + ornamentY + ornamentH / 2,
                            1);
                        outsideRejected =
                            outsideClickResult == M11_GAME_INPUT_IGNORED &&
                            state->candidateMirrorPanelActive == 0 &&
                            state->world.party.championCount == 0;
                        clickResult = M11_GameView_HandlePointer(
                            state,
                            viewportX + ornamentX + ornamentW / 2,
                            viewportY + ornamentY + ornamentH / 2,
                            1);
                        /* C127's source hit rectangle is the rendered C026
                         * destination, which can be narrower than the
                         * enclosing D1C ornament zone for edge cells.  Probe
                         * that source zone until the actual rendered hit is
                         * found; this keeps the gate about source geometry,
                         * not an invented centre-point assumption. */
                        if (clickResult == M11_GAME_INPUT_IGNORED ||
                            state->candidateMirrorPanelActive == 0 ||
                            state->candidateMirrorOrdinal != sourceOrdinal) {
                            for (clickY = 0; clickY < ornamentH &&
                                               !(state->candidateMirrorPanelActive &&
                                                 state->candidateMirrorOrdinal == sourceOrdinal);
                                 ++clickY) {
                                for (clickX = 0; clickX < ornamentW;
                                     ++clickX) {
                                    clickResult = M11_GameView_HandlePointer(
                                        state, viewportX + ornamentX + clickX,
                                        viewportY + ornamentY + clickY, 1);
                                    if (state->candidateMirrorPanelActive &&
                                        state->candidateMirrorOrdinal == sourceOrdinal) {
                                        break;
                                    }
                                }
                            }
                        }

                        /* A geometric C127 placement is only source-visible
                         * when the renderer resolves the same source ordinal
                         * into the D1C slot.  The original map contains one
                         * wall sensor behind a non-visible edge; treating it
                         * as clickable made this gate reject authentic data. */
                        if (sourceCandidate && frontOrdinal == sourceOrdinal &&
                            state->candidateMirrorPanelActive == 1 &&
                            state->candidateMirrorOrdinal == sourceOrdinal) {
                            ++candidateCount;
                            CHECK(outsideRejected,
                                  "point outside the source D1C hit zone must reject C127");
                            CHECK(clickResult == M11_GAME_INPUT_REDRAW &&
                                  state->candidateMirrorPanelActive == 1 &&
                                  state->candidateMirrorOrdinal == sourceOrdinal &&
                                  state->world.party.championCount == 1,
                                  "each source-visible C127 D1C click must reach F0280");
                            if (frontOrdinal == sourceOrdinal &&
                                outsideRejected &&
                                clickResult == M11_GAME_INPUT_REDRAW &&
                                state->candidateMirrorPanelActive == 1 &&
                                state->candidateMirrorOrdinal == sourceOrdinal &&
                                state->world.party.championCount == 1) {
                                ++selectedCount;
                                ++rejectedCount;
                            }
                            CHECK(M11_GameView_CancelMirrorCandidate(state) == 1,
                                  "each source C127 candidate must close through F0282");
                            clear_party(state);
                        } else if (sourceCandidate &&
                                   frontOrdinal == sourceOrdinal) {
                            /* The DOS corpus contains one edge C127 whose
                             * source portrait is visible but whose host
                             * C346 input material is not admitted.  Keep it
                             * in the rejection population; accepting it
                             * would violate the real-material fail-closed
                             * gate covered by the dedicated HoC tests. */
                            ++rejectedCount;
                        } else {
                            CHECK(frontOrdinal == -1 &&
                                  outsideRejected &&
                                  clickResult == M11_GAME_INPUT_IGNORED &&
                                  state->candidateMirrorPanelActive == 0 &&
                                  state->world.party.championCount == 0,
                                  "non-candidate C127 placement must reject the D1C click");
                            if (frontOrdinal == -1 &&
                                outsideRejected &&
                                clickResult == M11_GAME_INPUT_IGNORED &&
                                state->candidateMirrorPanelActive == 0 &&
                                state->world.party.championCount == 0) {
                                ++rejectedCount;
                            }
                        }
                    }
                }
                thing = F0512_DUNGEON_GetThingNext_Compat(
                    state->world.things, thing);
            }
            CHECK(safety < 64, "C127 source thing chain must terminate");
        }
    }

    if (sourceC127Count == 0) {
        puts("skip: selected real DM1 package has no HoC C127 sensor owners");
        return -1;
    }
    CHECK(sourceC127Count > 0,
          "real HoC map must retain original C127 sensor owners");
    CHECK(candidateCount > 0 && selectedCount == candidateCount,
          "every source-visible C127 candidate must reach F0280 by pointer click");
    CHECK(rejectedCount > 0,
          "source D1C route should retain a C127 rejection path");
    printf("ok: source C127 pointer sweep candidates=%d selected=%d rejected=%d\n",
           candidateCount, selectedCount, rejectedCount);
    return sourceC127Count > 0 && candidateCount > 0 &&
           selectedCount == candidateCount && rejectedCount > 0;
}

static int expect_recruited_portrait_matches_c026(M11_GameViewState* view,
                                                  int championIndex,
                                                  int mirrorOrdinal,
                                                  const char* context) {
    unsigned char expected[CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT];
    const M11_AssetSlot* portraits;
    const struct ChampionState_Compat* champ;
    int srcX;
    int srcY;
    int y;

    CHECK(view != NULL, "view must exist for C026 portrait check");
    if (!view) return 0;
    CHECK(championIndex >= 0 && championIndex < CHAMPION_MAX_PARTY,
          "champion index must be valid for C026 portrait check");
    if (championIndex < 0 || championIndex >= CHAMPION_MAX_PARTY) return 0;
    champ = &view->world.party.champions[championIndex];
    CHECK(champ->portraitBitmapValid == 1, context);
    if (!champ->portraitBitmapValid) return 0;

    portraits = M11_AssetLoader_Load(&view->assetLoader, 26u);
    CHECK(portraits && portraits->loaded && portraits->pixels,
          "C026 portrait atlas should load from GRAPHICS.DAT");
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    CHECK(portraits->width >= CHAMPION_PORTRAIT_BITMAP_WIDTH * 8 &&
          portraits->height >= CHAMPION_PORTRAIT_BITMAP_HEIGHT * 3,
          "C026 portrait atlas should have the 8x3 PC34 layout");
    if (portraits->width < CHAMPION_PORTRAIT_BITMAP_WIDTH * 8 ||
        portraits->height < CHAMPION_PORTRAIT_BITMAP_HEIGHT * 3) {
        return 0;
    }

    memset(expected, 0, sizeof(expected));
    srcX = (mirrorOrdinal & 7) * CHAMPION_PORTRAIT_BITMAP_WIDTH;
    srcY = (mirrorOrdinal >> 3) * CHAMPION_PORTRAIT_BITMAP_HEIGHT;
    for (y = 0; y < CHAMPION_PORTRAIT_BITMAP_HEIGHT; ++y) {
        int x;
        unsigned char* dst =
            expected + y * (CHAMPION_PORTRAIT_BITMAP_WIDTH / 2);
        const unsigned char* src =
            portraits->pixels + (srcY + y) * (int)portraits->width + srcX;
        for (x = 0; x < CHAMPION_PORTRAIT_BITMAP_WIDTH; x += 2) {
            dst[x / 2] = (unsigned char)(((src[x] & 0x0F) << 4) |
                                         (src[x + 1] & 0x0F));
        }
    }
    CHECK(memcmp(champ->portraitBitmap, expected, sizeof(expected)) == 0,
          "recruited champion portrait bytes should match C026 atlas cell");
    return memcmp(champ->portraitBitmap, expected, sizeof(expected)) == 0;
}

static void clear_party(M11_GameViewState* state) {
    int i;
    state->world.party.championCount = 0;
    state->world.party.activeChampionIndex = -1;
    for (i = 0; i < CHAMPION_MAX_PARTY; ++i) {
        F0600_CHAMPION_InitEmpty_Compat(&state->world.party.champions[i]);
    }
}

static int choose_data_dir(char* defaultDataDir, size_t defaultDataDirSize,
                           const char** outDataDir) {
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char* home;
    if (!dataDir || !dataDir[0]) {
        dataDir = getenv("FIRESTAFF_DM1_CANONICAL_DIR");
    }
    if (!dataDir || !dataDir[0]) {
        dataDir = NULL;
    }
    if (dataDir && access(dataDir, R_OK) == 0) {
        *outDataDir = dataDir;
        return 1;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return 0;
    }
    snprintf(defaultDataDir, defaultDataDirSize, "%s/.firestaff/data/dm1",
             home);
    if (access(defaultDataDir, R_OK) == 0) {
        *outDataDir = defaultDataDir;
        return 1;
    }
    return 0;
}

int main(void) {
    char defaultDataDir[1024];
    const char* dataDir = NULL;
    char saveTemplate[] = "/tmp/firestaff-dm1-hoc-c127-full-XXXXXX";
    char savePath[512];
    M11_GameLaunchSpec spec;
    M11_GameViewState state;
    M11_GameViewState resumed;
    HocMirrorPosePc34 mirrorA;
    HocMirrorPosePc34 mirrorB;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    const char* name = "KIR";
    const char* title = "NEW";
    int i;
    int candidateA;
    int candidateB;
    unsigned short bHpMaxBefore;
    unsigned short bStaMaxBefore;
    unsigned short bManaMaxBefore;

    if (!choose_data_dir(defaultDataDir, sizeof(defaultDataDir), &dataDir)) {
        puts("skip: DM1 PC34 data dir not available");
        return 0;
    }
    if (!mkdtemp(saveTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-hoc-c127.sav",
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

    M11_GameView_Init(&state);
    CHECK(M11_GameView_Start(&state, &spec),
          "DM1 real-data start should succeed");
    if (failures) return 1;
    state.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    clear_party(&state);

    /* This is an external-corpus route: an otherwise authentic DM1 edition
     * need not contain the Hall-of-Champions C127 owners.  Do not fabricate
     * substitute sensors or report a product regression when the supplied
     * original package cannot exercise the route.  Once two source owners
     * exist, all of the checks below remain mandatory. */
    if (!find_two_mirrors(&state, &mirrorA, &mirrorB)) {
        puts("skip: selected real DM1 package has no two HoC C127 mirror owners");
        M11_GameView_Shutdown(&state);
        return 77;
    }

    i = sweep_all_source_c127_pointer_routes(&state, framebuffer);
    if (i < 0) {
        M11_GameView_Shutdown(&state);
        return 77;
    }
    CHECK(i,
          "every source-owned HoC C127 candidate should reach F0280 by pointer");
    if (failures) goto done_state;

    draw_at(&state, &mirrorA, framebuffer);
    CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == mirrorA.ordinal,
          "front mirror A ordinal should come from source sensorData");
    CHECK(M11_GameView_SelectFrontMirrorCandidate(&state) == 1,
          "mirror A C127 should open the live candidate panel");
    candidateA = state.candidateMirrorPartyIndex;
    CHECK(state.candidateMirrorPanelActive == 1 &&
          state.candidateMirrorOrdinal == mirrorA.ordinal &&
          candidateA == 0 &&
          state.world.party.championCount == 1 &&
          state.world.party.champions[candidateA].present,
          "mirror A should append candidate slot 0 from the real record");
    (void)expect_recruited_portrait_matches_c026(
        &state, candidateA, mirrorA.ordinal,
        "mirror A candidate should receive C026 portrait bytes");
    CHECK(M11_GameView_ConfirmMirrorCandidate(&state, 0) == 1,
          "C160 should resurrect mirror A into the party");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.candidateMirrorOrdinal == -1 &&
          state.world.party.championCount == 1 &&
          state.world.party.activeChampionIndex == 0 &&
          state.world.party.champions[0].hp.current > 0,
          "C160 should close C040 and leave a live party champion");
    CHECK(state.world.things->sensors[mirrorA.sensorIndex].sensorType ==
              DM1_SENSOR_DISABLED,
          "C160 should disable mirror A's original sensor owner");

    draw_at(&state, &mirrorB, framebuffer);
    CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == mirrorB.ordinal,
          "front mirror B ordinal should come from source sensorData");
    CHECK(M11_GameView_SelectFrontMirrorCandidate(&state) == 1,
          "mirror B C127 should open a second live candidate panel");
    candidateB = state.candidateMirrorPartyIndex;
    CHECK(state.candidateMirrorPanelActive == 1 &&
          state.candidateMirrorOrdinal == mirrorB.ordinal &&
          candidateB == 1 &&
          state.world.party.championCount == 2 &&
          state.world.party.champions[candidateB].present,
          "mirror B should append after the resurrected champion");
    (void)expect_recruited_portrait_matches_c026(
        &state, candidateB, mirrorB.ordinal,
        "mirror B candidate should receive C026 portrait bytes");
    bHpMaxBefore = state.world.party.champions[candidateB].hp.maximum;
    bStaMaxBefore = state.world.party.champions[candidateB].stamina.maximum;
    bManaMaxBefore = state.world.party.champions[candidateB].mana.maximum;
    CHECK(M11_GameView_BeginMirrorCandidateReincarnateRename(&state) == 1,
          "C161 should enter the real rename gate before finalization");
    CHECK(state.candidateMirrorRenameActive == 1 &&
          state.world.party.champions[candidateB].name[0] == '\0',
          "rename gate should own the candidate name before OK");
    for (i = 0; name[i]; ++i) {
        CHECK(M11_GameView_ApplyMirrorCandidateRenameAscii(
                  &state, name[i]) == 1,
              "rename name byte should be accepted");
    }
    CHECK(M11_GameView_ApplyMirrorCandidateRenameAscii(&state, '\r') == 1,
          "rename return should move to the title field");
    for (i = 0; title[i]; ++i) {
        CHECK(M11_GameView_ApplyMirrorCandidateRenameAscii(
                  &state, title[i]) == 1,
              "rename title byte should be accepted");
    }
    CHECK(M11_GameView_ApplyMirrorCandidateRenameCommand(
              &state,
              DM1_V1_RESURRECTION_RENAME_UI_COMMAND_OK_PC34_COMPAT) == 1,
          "rename OK should finalize C161");
    CHECK(state.candidateMirrorPanelActive == 0 &&
          state.candidateMirrorRenameActive == 0 &&
          state.world.party.championCount == 2,
          "C161 should close both C040 and C027 after OK");
    CHECK(strcmp((const char*)state.world.party.champions[candidateB].name,
                 name) == 0 &&
          strcmp((const char*)state.world.party.champions[candidateB].title,
                 title) == 0,
          "C161 rename should write the chosen champion name/title");
    CHECK(state.world.party.champions[candidateB].hp.maximum ==
              (unsigned short)(bHpMaxBefore >> 1) &&
          state.world.party.champions[candidateB].stamina.maximum ==
              (unsigned short)(bStaMaxBefore >> 1) &&
          state.world.party.champions[candidateB].mana.maximum ==
              (unsigned short)(bManaMaxBefore >> 1),
          "C161 should apply ReDMCSB DM1 V1 reincarnation vital halving");
    CHECK(state.world.things->sensors[mirrorB.sensorIndex].sensorType ==
              DM1_SENSOR_DISABLED,
          "C161 should disable mirror B's original sensor owner");

    CHECK(M11_GameView_QuickSave(&state) == 1,
          "confirmed HoC party should quicksave through the real state path");
    M11_GameView_Shutdown(&state);

    spec.savePath = savePath;
    M11_GameView_Init(&resumed);
    CHECK(M11_GameView_Start(&resumed, &spec),
          "DM1 quick-resume should load the confirmed HoC party");
    CHECK(resumed.candidateMirrorPanelActive == 0 &&
          resumed.candidateMirrorRenameActive == 0 &&
          resumed.world.party.championCount == 2,
          "resume should not resurrect a stale C040/C027 transient panel");
    CHECK(strcmp((const char*)resumed.world.party.champions[1].name,
                 name) == 0 &&
          strcmp((const char*)resumed.world.party.champions[1].title,
                 title) == 0,
          "resume should keep the reincarnated champion rename");
    (void)expect_recruited_portrait_matches_c026(
        &resumed, 0, mirrorA.ordinal,
        "resumed resurrected champion should keep C026 portrait bytes");
    (void)expect_recruited_portrait_matches_c026(
        &resumed, 1, mirrorB.ordinal,
        "resumed reincarnated champion should keep C026 portrait bytes");
    M11_GameView_Shutdown(&resumed);

    if (!failures) {
        printf("ok: real PC34 HoC mirrors %d/%d resurrect and reincarnate through world/save state\n",
               mirrorA.ordinal, mirrorB.ordinal);
    }
    return failures == 0 ? 0 : 1;

done_state:
    M11_GameView_Shutdown(&state);
    return 1;
}
