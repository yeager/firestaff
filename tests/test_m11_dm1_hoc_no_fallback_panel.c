/* ReDMCSB REVIVE.C F0281/F0282: C027/C040 are original panel graphics.
 * Missing PC34 art must not produce an M11 substitute panel. */

#include "m11_game_view.h"
#include "dm1_v1_graphic_ids_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    kFramebufferWidth = 320,
    kFramebufferHeight = 200,
    kViewportX = 0,
    kViewportY = 33,
    kColorDarkestGray = 12,
    kColorLightestGray = 13,
    kColorGold = 9
};

static const char* pc34_graphics_path(void)
{
    static char path[2048];
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home;

    if (configured && configured[0]) {
        return configured;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    return path;
}

static void seed_rename_state(M11_GameViewState* state)
{
    M11_GameView_Init(state);
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->inventoryPanelActive = 1;
    state->candidateMirrorOrdinal = 1;
    state->candidateMirrorPartyIndex = 0;
    state->candidateMirrorPanelActive = 1;
    state->candidateMirrorRenameActive = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

static void seed_c040_state(M11_GameViewState* state)
{
    seed_rename_state(state);
    state->candidateMirrorRenameActive = 0;
    memset(&state->candidateMirrorRename, 0,
           sizeof(state->candidateMirrorRename));
}

typedef struct HocFrontMirrorFixture {
    struct DungeonDatState_Compat dungeon;
    struct DungeonMapDesc_Compat map;
    struct DungeonMapTiles_Compat tiles;
    struct DungeonThings_Compat things;
    struct DungeonSensor_Compat sensors[2];
    unsigned char squareData[2];
    unsigned short squareFirstThings[2];
    unsigned short columnsCumulativeSquareFirstThingCount[1];
    unsigned char sensorRaw[16];
} HocFrontMirrorFixture;

static unsigned short hoc_sensor_thing(int index, int cell)
{
    return (unsigned short)(((cell & 0x03) << 14) |
                            ((THING_TYPE_SENSOR & 0x0f) << 10) |
                            (index & 0x03ff));
}

static int seed_front_mirror_state(M11_GameViewState* state,
                                   HocFrontMirrorFixture* fixture)
{
    struct ChampionState_Compat champion;

    if (!state || !fixture) {
        return 0;
    }
    M11_GameView_Init(state);
    memset(fixture, 0, sizeof(*fixture));
    state->active = 1;
    state->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 0;
    state->world.party.mapY = 1;
    state->world.party.direction = DIR_NORTH;
    state->world.party.activeChampionIndex = -1;

    fixture->squareData[0] = (unsigned char)(DUNGEON_ELEMENT_WALL << 5);
    fixture->squareData[1] = (unsigned char)(DUNGEON_ELEMENT_CORRIDOR << 5);
    fixture->squareFirstThings[0] = hoc_sensor_thing(0, 2);
    fixture->squareFirstThings[1] = THING_ENDOFLIST;
    fixture->sensorRaw[0] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    fixture->sensorRaw[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);
    fixture->sensors[0].sensorType = 127;
    fixture->sensors[0].sensorData = 13;
    fixture->sensors[0].ornamentOrdinal = 4;
    fixture->map.width = 1;
    fixture->map.height = 2;
    fixture->tiles.squareData = fixture->squareData;
    fixture->tiles.squareCount = 2;
    fixture->dungeon.header.mapCount = 1;
    fixture->dungeon.maps = &fixture->map;
    fixture->dungeon.columnsCumulativeSquareFirstThingCount =
        fixture->columnsCumulativeSquareFirstThingCount;
    fixture->dungeon.dungeonColumnCount = 1;
    fixture->dungeon.tiles = &fixture->tiles;
    fixture->dungeon.loaded = 1;
    fixture->dungeon.tilesLoaded = 1;
    fixture->things.squareFirstThings = fixture->squareFirstThings;
    fixture->things.squareFirstThingCount = 2;
    fixture->things.rawThingData[THING_TYPE_SENSOR] = fixture->sensorRaw;
    fixture->things.thingCounts[THING_TYPE_SENSOR] = 1;
    fixture->things.sensors = fixture->sensors;
    fixture->things.sensorCount = 1;
    fixture->things.loaded = 1;
    state->world.dungeon = &fixture->dungeon;
    state->world.things = &fixture->things;

    F0600_CHAMPION_InitEmpty_Compat(&champion);
    if (!F0606_CHAMPION_ParseMirrorTextIdentity_Compat(
            "HALK|THE BRAVE||M|AAGEAAHIAABJAAAA|AABOCACCCECGCIAAAA|AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
            &champion)) {
        return 0;
    }
    state->mirrorCatalogAvailable = 1;
    state->mirrorCatalog.count = 1;
    state->mirrorCatalog.records[0].textStringIndex = 0;
    state->mirrorCatalog.records[0].mirrorOrdinal = 13;
    state->mirrorCatalog.records[0].champion = champion;
    (void)F0628_CHAMPION_UnpackName_Compat(
        &champion, state->mirrorCatalog.records[0].nameText,
        sizeof(state->mirrorCatalog.records[0].nameText));
    (void)F0629_CHAMPION_UnpackTitle_Compat(
        &champion, state->mirrorCatalog.records[0].titleText,
        sizeof(state->mirrorCatalog.records[0].titleText));
    return 1;
}

static void seed_first_sensor_before_mirror(HocFrontMirrorFixture* fixture)
{
    if (!fixture) {
        return;
    }
    /* ReDMCSB REVIVE.C F0282 BUG0_87: the source disables the first sensor
     * on the square, even when a custom sensor precedes the C127 mirror. */
    fixture->sensors[0].sensorType = 1;
    fixture->sensors[0].sensorData = 0;
    fixture->sensors[0].ornamentOrdinal = 0;
    fixture->sensors[1].sensorType = 127;
    fixture->sensors[1].sensorData = 13;
    fixture->sensors[1].ornamentOrdinal = 4;
    fixture->sensorRaw[0] = (unsigned char)(hoc_sensor_thing(1, 2) & 0xffu);
    fixture->sensorRaw[1] = (unsigned char)((hoc_sensor_thing(1, 2) >> 8) & 0xffu);
    fixture->sensorRaw[8] = (unsigned char)(THING_ENDOFLIST & 0xffu);
    fixture->sensorRaw[9] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xffu);
    fixture->squareData[0] |= DUNGEON_SQUARE_MASK_THING_LIST;
    fixture->things.thingCounts[THING_TYPE_SENSOR] = 2;
    fixture->things.sensorCount = 2;
}

static int expect_original_font_foreground(
    const M11_FontState* font,
    const unsigned char* framebuffer,
    int x,
    int y,
    const char* text,
    unsigned char color)
{
    unsigned char expected[kFramebufferWidth * kFramebufferHeight];
    int i;
    int foregroundCount = 0;

    memset(expected, 0, sizeof(expected));
    y -= 4; /* ReDMCSB TEXT2.C F0644 MEDIA508 baseline conversion. */
    while (*text) {
        const int fontX = ((unsigned char)*text * 8) + 3;
        int row;
        for (row = 0; row < 6; ++row) {
            int col;
            for (col = 0; col < 6; ++col) {
                int dstX = x + col;
                int dstY = y + row;
                if (dstX >= 0 && dstX < kFramebufferWidth &&
                    dstY >= 0 && dstY < kFramebufferHeight) {
                    expected[dstY * kFramebufferWidth + dstX] =
                        M11_Font_GetPixel(font, fontX + col, row)
                            ? color : kColorDarkestGray;
                }
            }
        }
        x += 6;
        text++;
    }
    for (i = 0; i < (int)sizeof(expected); ++i) {
        if (expected[i] == color) {
            foregroundCount++;
            if (framebuffer[i] != color) {
                return 0;
            }
        }
    }
    return foregroundCount > 0;
}

static int test_real_pc34_rename_font_handoff(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    M11_GameViewState noFontState;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    unsigned char noFontFramebuffer[kFramebufferWidth * kFramebufferHeight];

    if (!graphicsPath) {
        return 1;
    }
    seed_rename_state(&state);
    if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 GRAPHICS.DAT did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.assetsAvailable = 1;
    M11_Font_Init(&state.originalFont);
    if (!M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                      state.assetLoader.fileState,
                                      state.assetLoader.runtimeState)) {
        fprintf(stderr, "FAIL PC34 M653 font did not load\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.originalFontAvailable = 1;
    snprintf(state.candidateMirrorRename.name,
             sizeof(state.candidateMirrorRename.name), "AB");
    snprintf(state.candidateMirrorRename.title,
             sizeof(state.candidateMirrorRename.title), "MAGE");
    state.candidateMirrorRename.fieldMode =
        DM1_V1_RESURRECTION_RENAME_UI_FIELD_NAME_PC34_COMPAT;
    state.candidateMirrorRename.characterIndex = 2;
    state.candidateMirrorRename.cursorX = 177 + (2 * 6);
    state.candidateMirrorRename.cursorY = 91;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);

    /* ReDMCSB REVIVE.C F0281:408-409 uses F0052 (viewport-relative)
     * for guides, while the current entry/cursor starts at 177,91. */
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 177, kViewportY + 58,
                                         "_______", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 name guide\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 105, kViewportY + 76,
                                         "___________________", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 title guide\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 177, kViewportY + 91,
                                         "AB", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 name entry\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 105, kViewportY + 109,
                                         "MAGE", kColorLightestGray)) {
        fprintf(stderr, "FAIL PC34 C027 title entry\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    if (!expect_original_font_foreground(&state.originalFont, framebuffer,
                                         kViewportX + 189, kViewportY + 91,
                                         "_", kColorGold)) {
        fprintf(stderr, "FAIL PC34 C027 cursor\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* The same real C027 graphic must not acquire host text when M653 cannot
     * be loaded.  The two inputs differ only in rename bytes. */
    seed_rename_state(&noFontState);
    if (!M11_AssetLoader_Init(&noFontState.assetLoader, graphicsPath)) {
        fprintf(stderr, "FAIL PC34 C027 reload failed\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    noFontState.assetsAvailable = 1;
    snprintf(noFontState.candidateMirrorRename.name,
             sizeof(noFontState.candidateMirrorRename.name), "DIFFER");
    snprintf(noFontState.candidateMirrorRename.title,
             sizeof(noFontState.candidateMirrorRename.title), "HOST TEXT");
    noFontState.candidateMirrorRename.cursorX = 177 + (6 * 6);
    noFontState.candidateMirrorRename.cursorY = 91;
    memset(noFontFramebuffer, 0, sizeof(noFontFramebuffer));
    M11_GameView_Draw(&noFontState, noFontFramebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    noFontState.candidateMirrorRename.name[0] = '\0';
    noFontState.candidateMirrorRename.title[0] = '\0';
    noFontState.candidateMirrorRename.cursorX = 177;
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&noFontState, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    if (memcmp(framebuffer, noFontFramebuffer, sizeof(framebuffer)) != 0) {
        fprintf(stderr, "FAIL C027 emitted host text without PC34 M653\n");
        M11_GameView_Shutdown(&noFontState);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    M11_GameView_Shutdown(&noFontState);
    M11_GameView_Shutdown(&state);
    return 1;
}

static int test_c040_host_input_requires_real_panel(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState noAssetState;
    M11_GameViewState realAssetState;
    const M11_AssetSlot* backdrop;

    /* ReDMCSB PANEL.C F0346 must have installed C040 before COMMAND.C
     * dispatches C160/C161/C162.  An invisible panel may not accept host
     * keyboard or pointer commands. */
    seed_c040_state(&noAssetState);
    if (M11_GameView_HandlePointer(&noAssetState, 110, 100, 1) !=
            M11_GAME_INPUT_IGNORED ||
        M11_GameView_HandleInput(&noAssetState, M12_MENU_INPUT_ACCEPT) !=
            M11_GAME_INPUT_IGNORED ||
        !noAssetState.candidateMirrorPanelActive ||
        noAssetState.world.party.championCount != 1) {
        fprintf(stderr, "FAIL C040 input accepted without original panel\n");
        M11_GameView_Shutdown(&noAssetState);
        return 0;
    }
    M11_GameView_Shutdown(&noAssetState);

    if (!graphicsPath) {
        return 1;
    }
    seed_c040_state(&realAssetState);
    if (!M11_AssetLoader_Init(&realAssetState.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 C040 did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&realAssetState);
        return 1;
    }
    /* The live loader has the exact C017/C040 pixels, while the broad
     * launcher asset-completeness latch is intentionally still clear.
     * A visible original mirror must remain clickable in that state. */
    realAssetState.assetsAvailable = 0;
    backdrop = M11_AssetLoader_Load(
        &realAssetState.assetLoader,
        (unsigned int)dm1_v1_graphic_inventory_backdrop_pc34());
    if (!backdrop || !backdrop->pixels ||
        backdrop->width != 224 || backdrop->height != 136) {
        fprintf(stderr, "FAIL PC34 C017 backdrop missing for C040\n");
        M11_GameView_Shutdown(&realAssetState);
        return 0;
    }
    /* C160's source F0282 tail owns a real first sensor on the front mirror
     * square.  A model-only C040 panel has no source mutation target. */
    if (M11_GameView_HandlePointer(&realAssetState, 130, 115, 1) !=
            M11_GAME_INPUT_IGNORED ||
        !realAssetState.candidateMirrorPanelActive ||
        realAssetState.world.party.championCount != 1) {
        fprintf(stderr, "FAIL C160 accepted without front source sensor\n");
        M11_GameView_Shutdown(&realAssetState);
        return 0;
    }
    /* C162 at the source cancel row must now consume the C040-backed panel
     * and its C017 base, then remove only the appended candidate champion. */
    if (M11_GameView_HandlePointer(&realAssetState, 110, 150, 1) !=
            M11_GAME_INPUT_REDRAW ||
        realAssetState.candidateMirrorPanelActive ||
        realAssetState.world.party.championCount != 0 ||
        realAssetState.world.party.activeChampionIndex != -1) {
        fprintf(stderr, "FAIL real PC34 C040 cancel route\n");
        M11_GameView_Shutdown(&realAssetState);
        return 0;
    }
    M11_GameView_Shutdown(&realAssetState);
    return 1;
}

static int test_c161_host_input_requires_real_c027_and_m653(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    HocFrontMirrorFixture fixture;

    if (!graphicsPath) {
        return 1;
    }
    if (!seed_front_mirror_state(&state, &fixture) ||
        !M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 C027 did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.assetsAvailable = 1;
    if (M11_GameView_HandlePointer(&state, 110, 83, 1) !=
            M11_GAME_INPUT_REDRAW ||
        !state.candidateMirrorPanelActive) {
        fprintf(stderr, "FAIL PC34 C127 did not open C040 for C161\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* ReDMCSB REVIVE.C F0282 invokes F0281 only for C161.  C027 without
     * M653 cannot show typed input, so the host must not enter that modal
     * route merely because C040 remains loaded. */
    if (M11_GameView_HandlePointer(&state, 186, 115, 1) !=
            M11_GAME_INPUT_IGNORED ||
        state.candidateMirrorRenameActive) {
        fprintf(stderr, "FAIL C161 accepted without PC34 M653\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }

    M11_Font_Init(&state.originalFont);
    if (!M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                      state.assetLoader.fileState,
                                      state.assetLoader.runtimeState)) {
        fprintf(stderr, "FAIL PC34 M653 did not load for C161\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    state.originalFontAvailable = 1;
    if (M11_GameView_HandlePointer(&state, 186, 115, 1) !=
            M11_GAME_INPUT_REDRAW ||
        !state.candidateMirrorRenameActive ||
        !state.candidateMirrorPanelActive) {
        fprintf(stderr, "FAIL real PC34 C161/C027 rename route\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    M11_GameView_Shutdown(&state);
    return 1;
}

static int test_front_mirror_host_input_requires_real_pc34_material(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState noAssetState;
    M11_GameViewState realAssetState;
    HocFrontMirrorFixture noAssetFixture;
    HocFrontMirrorFixture realAssetFixture;

    /* ReDMCSB DUNGEON.C F0172 publishes C127 only for the visible face;
     * DUNVIEW.C:3913-3928 presents C346 then its C026 atlas cell.  M11 host
     * input must not invoke REVIVE.C F0280 from an invisible substitute. */
    if (!seed_front_mirror_state(&noAssetState, &noAssetFixture) ||
        M11_GameView_HandlePointer(&noAssetState, 110, 83, 1) !=
            M11_GAME_INPUT_IGNORED ||
        noAssetState.candidateMirrorPanelActive ||
        noAssetState.world.party.championCount != 0) {
        fprintf(stderr, "FAIL C127 accepted without C346/C026 PC34 material\n");
        M11_GameView_Shutdown(&noAssetState);
        return 0;
    }
    M11_GameView_Shutdown(&noAssetState);

    if (!graphicsPath) {
        return 1;
    }
    if (!seed_front_mirror_state(&realAssetState, &realAssetFixture) ||
        !M11_AssetLoader_Init(&realAssetState.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 C346/C026 did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&realAssetState);
        return 1;
    }
    /* C346/C026 are resident even when the launcher did not mark every
     * optional asset ready.  The visible source mirror must still accept
     * its C127 click. */
    realAssetState.assetsAvailable = 0;
    if (M11_GameView_HandlePointer(&realAssetState, 110, 83, 1) !=
            M11_GAME_INPUT_REDRAW ||
        !realAssetState.candidateMirrorPanelActive ||
        realAssetState.candidateMirrorOrdinal != 13 ||
        realAssetState.world.party.championCount != 1) {
        fprintf(stderr, "FAIL real C346/C026 C127 mirror route\n");
        M11_GameView_Shutdown(&realAssetState);
        return 0;
    }
    M11_GameView_Shutdown(&realAssetState);
    return 1;
}

static int test_c160_disables_source_first_sensor(void)
{
    const char* graphicsPath = pc34_graphics_path();
    M11_GameViewState state;
    HocFrontMirrorFixture fixture;
    int confirmResult;

    if (!graphicsPath) {
        return 1;
    }
    if (!seed_front_mirror_state(&state, &fixture) ||
        !M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
        if (getenv("FIRESTAFF_DM1_GRAPHICS_DAT")) {
            fprintf(stderr, "FAIL configured PC34 F0282 source chain did not load\n");
            return 0;
        }
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.assetsAvailable = 1;
    if (M11_GameView_HandlePointer(&state, 110, 83, 1) !=
            M11_GAME_INPUT_REDRAW) {
        fprintf(stderr, "FAIL C127 did not open F0282 panel\n");
        M11_GameView_Shutdown(&state);
        return 0;
    }
    /* F0280 has already chosen the visible C127.  Model the F0282 source
     * chain exactly as BUG0_87 describes: a custom sensor now precedes it. */
    seed_first_sensor_before_mirror(&fixture);
    confirmResult = M11_GameView_HandlePointer(&state, 130, 115, 1);
    if (confirmResult != M11_GAME_INPUT_REDRAW ||
        fixture.sensors[0].sensorType != 0 ||
        fixture.sensors[1].sensorType != 127 ||
        state.candidateMirrorPanelActive ||
        state.world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL C160 first-sensor owner result=%d sensor0=%d sensor1=%d panel=%d party=%d\n",
                confirmResult, fixture.sensors[0].sensorType,
                fixture.sensors[1].sensorType,
                state.candidateMirrorPanelActive,
                state.world.party.championCount);
        M11_GameView_Shutdown(&state);
        return 0;
    }
    M11_GameView_Shutdown(&state);
    return 1;
}

int main(void) {
    M11_GameViewState state;
    unsigned char framebuffer[kFramebufferWidth * kFramebufferHeight];
    int x;
    int y;
    int substitutePixels = 0;

    seed_rename_state(&state);
    /* No GRAPHICS.DAT session: C027/C040 must fail closed. */
    state.assetsAvailable = 0;
    memset(framebuffer, 0, sizeof(framebuffer));

    M11_GameView_Draw(&state, framebuffer,
                      kFramebufferWidth, kFramebufferHeight);
    /* C101 occupies the 224x136 M11 viewport at (0,33). */
    for (y = 33; y < 33 + 136; ++y) {
        for (x = 0; x < 224; ++x) {
            unsigned char pixel = framebuffer[y * kFramebufferWidth + x];
            /* The retired substitute used a green fill and orange border.
             * Other M11 chrome may legitimately overlap C101. */
            if (pixel == 6 || pixel == 9) {
                substitutePixels++;
            }
        }
    }
    if (substitutePixels != 0) {
        fprintf(stderr,
                "FAIL test_m11_dm1_hoc_no_fallback_panel substitute_pixels=%d\n",
                substitutePixels);
        return 1;
    }
    if (!test_real_pc34_rename_font_handoff()) {
        return 1;
    }
    if (!test_c040_host_input_requires_real_panel()) {
        return 1;
    }
    if (!test_c161_host_input_requires_real_c027_and_m653()) {
        return 1;
    }
    if (!test_front_mirror_host_input_requires_real_pc34_material()) {
        return 1;
    }
    if (!test_c160_disables_source_first_sensor()) {
        return 1;
    }
    printf("PASS test_m11_dm1_hoc_no_fallback_panel substitute_pixels=0\n");
    return 0;
}
