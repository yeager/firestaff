#include "m11_game_view.h"
#include "dm1_v1_original_save_classifier.h"
#include "dm1_v1_original_save_pc34_handoff.h"
#include "dm1_v1_viewport_runtime_materialization_pc34_compat.h"
#include "memory_savegame_pc34_native_export_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define ORIGINAL_PC34_CHAMPION_BYTES 319
#define ORIGINAL_PC34_PARTY_INFO_BYTES 128
#define ORIGINAL_PC34_PARTY_BYTES \
    ((ORIGINAL_PC34_CHAMPION_BYTES * CHAMPION_MAX_PARTY) + \
     ORIGINAL_PC34_PARTY_INFO_BYTES)

static void wr16le(unsigned char* p, uint16_t v) {
    p[0] = (unsigned char)(v & 0xffu);
    p[1] = (unsigned char)((v >> 8) & 0xffu);
}

static void wr32le(unsigned char* p, uint32_t v) {
    wr16le(p, (uint16_t)(v & 0xffffu));
    wr16le(p + 2, (uint16_t)((v >> 16) & 0xffffu));
}

static uint16_t rd16le(const unsigned char* p) {
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static uint16_t original_first_half_checksum(const unsigned char* header) {
    uint16_t acc = 0;
    size_t i;
    for (i = 0; i < 32u; ++i) {
        acc = (uint16_t)(acc + rd16le(header + (i * 8u) + 0u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 2u));
        acc = (uint16_t)(acc - rd16le(header + (i * 8u) + 4u));
        acc = (uint16_t)(acc ^ rd16le(header + (i * 8u) + 6u));
    }
    return acc;
}

static uint16_t original_second_half_plain_checksum(const unsigned char* header) {
    uint16_t sum = 0;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        sum = (uint16_t)(sum + rd16le(header + (i * 2u)));
    }
    return sum;
}

static void xor_original_second_half(unsigned char* header, uint16_t key) {
    uint16_t rollingKey = key;
    size_t i;
    for (i = 128u; i < 256u; ++i) {
        unsigned char* word = header + (i * 2u);
        wr16le(word, (uint16_t)(rd16le(word) ^ rollingKey));
        rollingKey = (uint16_t)(rollingKey + (uint16_t)(256u - i));
    }
}

static uint16_t checksum_and_xor_original_words(unsigned char* bytes,
                                                size_t wordCount,
                                                uint16_t key) {
    uint16_t rollingKey = key;
    uint16_t checksum = key;
    size_t i;
    for (i = 0u; i < wordCount; ++i) {
        unsigned char* word = bytes + i * 2u;
        uint16_t v = rd16le(word);
        checksum = (uint16_t)(checksum + v);
        v = (uint16_t)(v ^ rollingKey);
        wr16le(word, v);
        checksum = (uint16_t)(checksum + v);
        rollingKey = (uint16_t)(rollingKey +
                                (uint16_t)(wordCount - i));
    }
    return checksum;
}

static int write_original_part(unsigned char* dst,
                               int dstCap,
                               const unsigned char* plain,
                               int byteCount,
                               uint16_t key,
                               uint16_t* outChecksum) {
    if (dstCap < 2 + byteCount || (byteCount & 1) != 0) return -1;
    wr16le(dst, (uint16_t)byteCount);
    if (byteCount > 0 && plain) {
        memcpy(dst + 2, plain, (size_t)byteCount);
    }
    *outChecksum = checksum_and_xor_original_words(
        dst + 2, (size_t)byteCount / 2u, key);
    return 2 + byteCount;
}

static void write_original_champion(unsigned char* dst) {
    memset(dst, 0, ORIGINAL_PC34_CHAMPION_BYTES);
    memset(dst + 0, ' ', 8u);
    memset(dst + 8, ' ', 20u);
    memcpy(dst + 0, "TIGGY", 5u);
    memcpy(dst + 8, "APPRENTICE", 10u);
    dst[28] = DIR_EAST;
    wr16le(dst + 52, 44u);
    wr16le(dst + 54, 55u);
    wr16le(dst + 56, 66u);
    wr16le(dst + 58, 77u);
    wr16le(dst + 60, 8u);
    wr16le(dst + 62, 9u);
    wr16le(dst + 66, 1500u);
    wr16le(dst + 68, 1200u);
    wr16le(dst + 70 + 3u, 33u);
    wr32le(dst + 91 + 2u, 1000u);
    wr16le(dst + 211 + (size_t)CHAMPION_SLOT_HAND_RIGHT * 2u, 0x1555u);
    wr16le(dst + 271, 345u);
}

static int write_original_pc34_dm1_save_file(const char* path) {
    unsigned char buf[SAVEGAME_PC34_MAX_FILE_SIZE];
    unsigned char header[SAVEGAME_PC34_DM_SAVE_HEADER_SIZE];
    unsigned char global[SAVEGAME_PC34_GLOBAL_DATA_BYTE_COUNT];
    unsigned char party[ORIGINAL_PC34_PARTY_BYTES];
    uint16_t keys[SAVEGAME_PC34_DM_KEYS_COUNT];
    uint16_t checksums[SAVEGAME_PC34_DM_CHECKSUMS_COUNT];
    int cursor = SAVEGAME_PC34_DM_SAVE_HEADER_SIZE;
    int n;
    int i;
    FILE* file;

    memset(buf, 0, sizeof(buf));
    memset(header, 0, sizeof(header));
    memset(global, 0, sizeof(global));
    memset(party, 0, sizeof(party));
    memset(checksums, 0, sizeof(checksums));

    for (i = 0; i < 127; ++i) {
        wr16le(header + (size_t)i * 2u,
               (uint16_t)(0x5151u + (uint16_t)(i * 11u)));
    }
    wr16le(header + 10u * 2u, 0x2468u);
    header[298] = 1u;
    header[299] = SAVEGAME_PC34_FORMAT_DUNGEON_MASTER_PC;
    wr32le(header + 306u, 0x50433334u);
    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        keys[i] = (uint16_t)(0x3000u + (uint16_t)(i * 0x77u));
    }

    wr32le(global + 0u, 7777u);
    wr16le(global + 10u, 1u);
    wr16le(global + 12u, 9u);
    wr16le(global + 14u, 10u);
    wr16le(global + 16u, DIR_EAST);
    /* The canonical DM1 start backing accepts map 0 at this pose. The
     * tail-less source fixture must bind to that real F0435 dungeon rather
     * than relying on a made-up map backing. */
    wr16le(global + 18u, 0u);
    wr16le(global + 20u, 0u);
    wr16le(global + 24u, 0u);
    wr16le(global + 26u, 0u);
    wr16le(global + 28u, 0u);
    wr16le(global + 30u, 0u);
    wr16le(global + 46u, 0u);
    write_original_champion(party);

    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            global, (int)sizeof(global),
                            keys[SAVEGAME_PC34_PART_GLOBAL_DATA],
                            &checksums[SAVEGAME_PC34_PART_GLOBAL_DATA]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            NULL, 0,
                            keys[SAVEGAME_PC34_PART_ACTIVE_GROUP],
                            &checksums[SAVEGAME_PC34_PART_ACTIVE_GROUP]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            party, (int)sizeof(party),
                            keys[SAVEGAME_PC34_PART_PARTY],
                            &checksums[SAVEGAME_PC34_PART_PARTY]);
    if (n < 0) return 0;
    cursor += n;
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            NULL, 0,
                            keys[SAVEGAME_PC34_PART_EVENTS],
                            &checksums[SAVEGAME_PC34_PART_EVENTS]);
    if (n < 0) return 0;
    cursor += n;
    /* A C4 entry may only name a non-NONE C3 EVENT. This focused save has
     * no active timers, so F0433's valid C4 payload is empty. */
    n = write_original_part(buf + cursor, (int)sizeof(buf) - cursor,
                            NULL, 0,
                            keys[SAVEGAME_PC34_PART_TIMELINE],
                            &checksums[SAVEGAME_PC34_PART_TIMELINE]);
    if (n < 0) return 0;
    cursor += n;

    /* LOADSAVE.C F0435 consumes four fixed portrait bitmaps after C4, even
     * when the focused runtime fixture has no dungeon tail. The zero-filled
     * bytes are source-shaped portrait payloads, not a host fallback. */
    if (cursor > (int)sizeof(buf) -
        (int)SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT) return 0;
    cursor += (int)SAVEGAME_PC34_EXTERNAL_PORTRAIT_BYTE_COUNT;

    for (i = 0; i < SAVEGAME_PC34_DM_KEYS_COUNT; ++i) {
        wr16le(header + 310u + (size_t)i * 2u, keys[i]);
        wr16le(header + 342u + (size_t)i * 2u, checksums[i]);
    }
    wr16le(header + 374u, SAVEGAME_PC34_PLATFORM_PC);
    wr16le(header + 376u, SAVEGAME_PC34_DUNGEON_ID_DM);
    {
        uint16_t secondSum = original_second_half_plain_checksum(header);
        uint16_t firstBeforeLast = original_first_half_checksum(header);
        uint16_t last = (uint16_t)(rd16le(header + 254u) ^
                                   firstBeforeLast ^
                                   secondSum);
        wr16le(header + 254u, last);
    }
    xor_original_second_half(
        header,
        rd16le(header + SAVEGAME_PC34_DM_HEADER_DECRYPTION_KEY_INDEX * 2u));
    memcpy(buf, header, sizeof(header));

    file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(buf, 1u, (size_t)cursor, file) != (size_t)cursor) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static int m12_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int m12_test_unsetenv(const char* name) {
#ifdef _WIN32
    return _putenv_s(name, "");
#else
    return unsetenv(name);
#endif
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int explored_cell_is_set(const M11_GameViewState *view, unsigned int cell) {
    if (!view || cell >= 1024U) {
        return 0;
    }
    return (view->exploredBits[cell / 32U] & (1U << (cell % 32U))) != 0;
}

int main(void) {
    const char* dataDir = getenv("FIRESTAFF_DM1_CANONICAL_DIR");
    char saveTemplate[] = "/tmp/firestaff-m11-resume-XXXXXX";
    char savePath[512];
    M11_GameViewState view;
    M11_GameViewState resumed;
    M11_GameViewState originalResumed;
    M11_GameViewState roundtripLoaded;
    M11_GameLaunchSpec spec;
    DM1OriginalSaveClassifyResult exportedPc34;
    int mapIndex, mapX, mapY, direction;
    const unsigned short leaderHandThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1U);
    const unsigned short openChestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 2U);
    unsigned int revealedCell = (17U * 32U) + 9U;
    unsigned int currentCell;

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
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", saveTemplate);
    m12_test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);
    m12_test_setenv("HOME", saveTemplate);

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!expect(M11_GameView_Start(&view, &spec), "initial DM1 start should succeed")) return 1;
    if (!expect(M11_GameView_Dm1StartupIntroBypassed(&view) == 1,
                "direct generic DM1 start should report game-view intro bypass")) return 1;
    if (!expect(view.inspectTitle[0] == '\0' && view.inspectDetail[0] == '\0',
                "DM1 start must not render Firestaff-only HoC help text")) return 1;

    /* COMMAND.C C140 reaches the native DM1 F0433 save route, whereas F9
     * below owns Firestaff's quicksave envelope. Exercise the actual visible
     * inventory SAVE hotspot (C140) so the button cannot regress into inert
     * artwork while its keyboard command remains healthy. */
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-command-save.sav",
             saveTemplate);
    m12_test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);
    view.inventoryPanelActive = 1;
    view.dm1MusicOn = 1;
    if (!expect(M11_GameView_HandlePointerButton(
                    &view, 168, 36, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                    M11_GAME_INPUT_REDRAW && view.dm1MusicOn == 0,
                "C141 visible music control should toggle live DM1 music")) return 1;
    if (!expect(M11_GameView_HandlePointerButton(
                    &view, 179, 35, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                    M11_GAME_INPUT_REDRAW,
                "C140 visible save control should be consumed by live DM1")) return 1;
    if (!expect(view.inventoryPanelActive == 0,
                "C140 visible save control should leave inventory input route")) return 1;
    if (!expect(view.dialogOverlayActive,
                "C140 visible save control should open the DM1 save-disk menu")) return 1;
    if (!expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "save-disk menu default choice should write the DM1 save")) return 1;
    if (!expect(M11_GameView_LoadDm1SavePath(&view, savePath, NULL),
                "C140 visible save control should write a loadable DM1 save")) return 1;

    view.inventoryPanelActive = 1;
    if (!expect(M11_GameView_HandlePointerButton(
                    &view, 190, 35, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                    M11_GAME_INPUT_REDRAW && view.resting &&
                    view.world.partyIsResting && view.world.lifecycle.rest.isResting,
                "C145 visible Zz control should start the complete rest state")) return 1;
    view.inventoryPanelActive = 1;
    if (!expect(M11_GameView_HandlePointerButton(
                    &view, 209, 35, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                    M11_GAME_INPUT_REDRAW && !view.inventoryPanelActive,
                "C011 visible close control should dismiss inventory")) return 1;

    /* The visible C140 route must make its normal user-data directory before
     * opening the file.  A test-only explicit path above cannot cover the
     * macOS/Linux default path that had previously surfaced as FILE NOT FOUND
     * for a fresh profile. */
    if (!expect(m12_test_unsetenv("FIRESTAFF_QUICKSAVE_PATH") == 0,
                "clear test-only save path override")) return 1;
    if (!expect(M11_GameView_GetQuickSavePath(&view, savePath,
                                               sizeof(savePath)),
                "resolve default DM1 user save path")) return 1;
    view.resting = 0;
    view.world.partyIsResting = 0;
    view.world.lifecycle.rest.isResting = 0;
    view.inventoryPanelActive = 1;
    if (!expect(M11_GameView_HandlePointerButton(
                    &view, 179, 35, DM1_V1_MOUSE_MASK_LEFT_PC34) ==
                    M11_GAME_INPUT_REDRAW,
                "C140 creates the default DM1 save directory")) return 1;
    if (!expect(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                "default C140 save-disk choice writes the local save")) return 1;
    if (!expect(access(savePath, R_OK) == 0,
                "default C140 save path exists after click")) return 1;
    if (!expect(M11_GameView_LoadDm1SavePath(&view, savePath, NULL),
                "default C140 save reloads")) return 1;

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav",
             saveTemplate);
    m12_test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);

    view.world.party.mapIndex = 2;
    view.world.party.mapX = 11;
    view.world.party.mapY = 7;
    view.world.party.direction = 3;
    view.world.gameTick = 4242;
    currentCell = (unsigned int)(view.world.party.mapX * 32 + view.world.party.mapY);
    view.exploredBits[currentCell / 32U] |= (1U << (currentCell % 32U));
    view.exploredBits[revealedCell / 32U] |= (1U << (revealedCell % 32U));
    view.leaderHandObjectPresent = 1;
    view.leaderHandThing = leaderHandThing;
    view.leaderHandIconIndex = -1;
    view.v1OpenChestThing = openChestThing;
    view.v1OpenChestOpenedByEye = 1;
    if (!expect(F0891_ORCH_WorldHash_Compat(&view.world, &view.lastWorldHash),
                "world hash refresh should succeed before save")) return 1;
    if (!expect(M11_GameView_QuickSave(&view), "quick save should write valid state")) return 1;
    mapIndex = view.world.party.mapIndex;
    mapX = view.world.party.mapX;
    mapY = view.world.party.mapY;
    direction = view.world.party.direction;
    M11_GameView_Shutdown(&view);

    spec.savePath = savePath;
    M11_GameView_Init(&resumed);
    if (!expect(M11_GameView_Start(&resumed, &spec), "quick-resume DM1 start should load save")) return 1;
    if (!expect(M11_GameView_Dm1StartupIntroBypassed(&resumed) == 1,
                "direct generic DM1 resume should report game-view intro bypass")) return 1;
    if (!expect(resumed.world.party.mapIndex == mapIndex, "resumed mapIndex should match saved state")) return 1;
    if (!expect(resumed.world.party.mapX == mapX, "resumed mapX should match saved state")) return 1;
    if (!expect(resumed.world.party.mapY == mapY, "resumed mapY should match saved state")) return 1;
    if (!expect(resumed.world.party.direction == direction, "resumed direction should match saved state")) return 1;
    if (!expect(resumed.world.gameTick == 4242, "resumed gameTick should match saved state")) return 1;
    if (!expect(resumed.dm1ViewportRuntimeOrigin ==
                    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_QUICKSAVE_RESUME_PC34,
                "Firestaff-native resume must not claim original PC34 provenance")) return 1;
    if (!expect(explored_cell_is_set(&resumed, revealedCell),
                "resumed explored tile should match saved reveal state")) return 1;
    if (!expect(explored_cell_is_set(&resumed, currentCell),
                "resumed current tile should stay marked explored")) return 1;
    if (!expect(M11_GameView_GetV1LeaderHandThing(&resumed) == leaderHandThing,
                "resumed leader hand should match saved V1 hand state")) return 1;
    if (!expect(M11_GameView_GetV1OpenChestThing(&resumed) == openChestThing,
                "resumed open chest should match saved V1 chest state")) return 1;
    if (!expect(resumed.v1OpenChestOpenedByEye == 1,
                "resumed open chest should preserve pressing-eye route state")) return 1;
    M11_GameView_Shutdown(&resumed);

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-original-pc34.sav", saveTemplate);
    if (!expect(write_original_pc34_dm1_save_file(savePath),
                "should write original PC34 fixture for M11 resume")) return 1;
    spec.savePath = NULL;
    M11_GameView_Init(&originalResumed);
    if (!expect(M11_GameView_Start(&originalResumed, &spec),
                "direct DM1 start should succeed before original PC34 load")) return 1;
    if (!expect(M11_GameView_LoadDm1SavePath(&originalResumed, savePath, NULL),
                "M11 should load original PC34 save through the F0435 runtime consumer")) return 1;
    if (!expect(originalResumed.world.party.mapIndex == 0,
                "original PC34 resumed mapIndex should match GLOBAL_DATA")) return 1;
    if (!expect(originalResumed.world.party.mapX == 9,
                "original PC34 resumed mapX should match GLOBAL_DATA")) return 1;
    if (!expect(originalResumed.world.party.mapY == 10,
                "original PC34 resumed mapY should match GLOBAL_DATA")) return 1;
    if (!expect(originalResumed.world.party.direction == DIR_EAST,
                "original PC34 resumed direction should match GLOBAL_DATA")) return 1;
    if (!expect(originalResumed.world.gameTick == 7777,
                "original PC34 resumed tick should match GLOBAL_DATA")) return 1;
    if (!expect(originalResumed.world.party.championCount == 1,
                "original PC34 resumed champion count should match PARTY")) return 1;
    if (!expect(originalResumed.dm1ViewportRuntimeOrigin ==
                    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34,
                "F0435 original PC34 resume should retain source provenance")) return 1;
    if (!expect(memcmp(originalResumed.world.party.champions[0].name,
                       "TIGGY   ",
                       CHAMPION_NAME_LENGTH) == 0,
                "original PC34 resumed champion name should match PARTY")) return 1;
    if (!expect(originalResumed.world.dungeon != NULL &&
                originalResumed.world.things != NULL &&
                originalResumed.world.ownsDungeon == 1,
                "original PC34 M11 resume should retain live dungeon ownership")) return 1;

    /* C040/G0299 boundary: while the resurrect/reincarnate candidate panel
     * owns input, ReDMCSB COMMAND.C F0380 keeps the C140 save/load command
     * surface out.  Direct host load APIs must therefore refuse to replace
     * the world behind a live panel, and accept the same load again once
     * the panel is cleared. */
    originalResumed.candidateMirrorOrdinal = 0;
    originalResumed.candidateMirrorPartyIndex = 0;
    originalResumed.candidateMirrorPanelActive = 1;
    originalResumed.candidateMirrorRenameActive = 0;
    if (!expect(!M11_GameView_LoadDm1SavePath(&originalResumed, savePath, NULL),
                "live C040 panel must block direct LoadDm1SavePath")) return 1;
    if (!expect(originalResumed.world.party.mapX == 9 &&
                originalResumed.world.party.mapY == 10 &&
                originalResumed.world.gameTick == 7777 &&
                originalResumed.candidateMirrorPanelActive == 1,
                "blocked LoadDm1SavePath must preserve world and live panel")) return 1;
    {
        FILE* fixtureFile = fopen(savePath, "rb");
        unsigned char fixtureBytes[4096];
        size_t fixtureSize = 0;
        if (!expect(fixtureFile != NULL,
                    "original PC34 fixture should reopen for the byte-load guard")) return 1;
        fixtureSize = fread(fixtureBytes, 1, sizeof(fixtureBytes), fixtureFile);
        fclose(fixtureFile);
        if (!expect(fixtureSize > 0 && fixtureSize < sizeof(fixtureBytes),
                    "original PC34 fixture should fit the byte-load buffer")) return 1;
        if (!expect(!M11_GameView_LoadDm1OriginalPc34SaveBytes(
                        &originalResumed, fixtureBytes, fixtureSize, savePath),
                    "live C040 panel must block direct LoadDm1OriginalPc34SaveBytes")) return 1;
    }
    if (!expect(originalResumed.world.party.mapX == 9 &&
                originalResumed.world.party.mapY == 10 &&
                originalResumed.world.gameTick == 7777 &&
                originalResumed.candidateMirrorPanelActive == 1,
                "blocked byte load must preserve world and live panel")) return 1;
    if (!expect(!M11_GameView_QuickLoad(&originalResumed),
                "live C040 panel must keep blocking direct QuickLoad")) return 1;
    originalResumed.candidateMirrorPanelActive = 0;
    if (!expect(M11_GameView_LoadDm1SavePath(&originalResumed, savePath, NULL),
                "cleared C040 panel must allow direct LoadDm1SavePath again")) return 1;
    if (!expect(originalResumed.world.party.mapX == 9 &&
                originalResumed.world.party.mapY == 10 &&
                originalResumed.world.gameTick == 7777 &&
                originalResumed.dm1ViewportRuntimeOrigin ==
                    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34,
                "re-admitted load must restore the same original PC34 state")) return 1;

    /* F0435 -> live M11 state -> F0433 must stay on the PC34 envelope path.
     * The exported file is reclassified before a second M11 F0435 load so a
     * Firestaff-native quicksave cannot accidentally satisfy this interop leg. */
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-pc34-runtime.sav", saveTemplate);
    m12_test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);
    if (!expect(M11_GameView_QuickSave(&originalResumed),
                "M11 should quicksave a runtime resumed from original PC34")) return 1;
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-pc34-roundtrip.dat", saveTemplate);
    {
        char runtimeQuickSavePath[512];
        snprintf(runtimeQuickSavePath, sizeof(runtimeQuickSavePath),
                 "%s/firestaff-dm1-pc34-runtime.sav", saveTemplate);
        if (!expect(M11_GameView_ExportQuickSaveAsDM1PC34(runtimeQuickSavePath, savePath),
                    "M11 should export the resumed runtime as PC34")) return 1;
    }
    memset(&exportedPc34, 0, sizeof(exportedPc34));
    if (!expect(dm1_v1_original_save_classify_file(savePath, &exportedPc34) &&
                exportedPc34.shape == DM1_ORIGINAL_SAVE_SHAPE_ORIGINAL_DM1_PC34 &&
                exportedPc34.pc34_importer_candidate &&
                exportedPc34.pc34_loader_part_envelope_candidate,
                "M11 runtime export should be a checksum-qualified PC34 envelope")) return 1;
    M11_GameView_Shutdown(&originalResumed);

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    M11_GameView_Init(&roundtripLoaded);
    if (!expect(M11_GameView_Start(&roundtripLoaded, &spec),
                "direct DM1 start should succeed before PC34 runtime reload")) return 1;
    /* F0803 now emits a vanilla PC3.4 envelope, so a save created from a
     * successfully imported PC34 world must traverse F0435 again. It is not
     * corpus proof, but it is a valid local save/load round trip. */
    if (!expect(M11_GameView_LoadDm1SavePath(&roundtripLoaded, savePath, NULL),
                "M11 must reload its vanilla PC34 re-export through F0435")) return 1;
    if (!expect(roundtripLoaded.dm1ViewportRuntimeOrigin ==
                    DM1_V1_VIEWPORT_RUNTIME_ORIGIN_ORIGINAL_SAVE_PC34,
                "reloaded vanilla PC34 export must retain PC34 provenance")) return 1;
    M11_GameView_Shutdown(&roundtripLoaded);

    puts("ok: DM1 M11 quick-resume restores Firestaff-native and original PC34 saves");
    return 0;
}
