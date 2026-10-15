#include "dm1_v1_fmtowns_entrance_viewport.h"
#include "m11_game_view.h"
#include "asset_status_m12.h"
#include "render_sdl_m11.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t assets[15][248 * 136];
static int failures;
#define CHECK(x) do { if (!(x)) { \
    fprintf(stderr, "FAIL:%d: %s\n", __LINE__, #x); ++failures; \
} } while (0)

static int native_size(unsigned int record, int* width, int* height)
{
    static const unsigned short sizes[][3] = {
        {75,224,70},{76,224,29},{84,32,136},{85,32,136},
        {86,248,111},{87,136,71},{88,117,51},{89,16,49}
    };
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
        if (sizes[i][0] == record) {
            *width = sizes[i][1]; *height = sizes[i][2]; return 1;
        }
    }
    return 0;
}

typedef struct Calls { unsigned int records[16]; int count; } Calls;
static int source_graphic(void* user, unsigned int record,
    const uint8_t** pixels, int* width, int* height)
{
    Calls* calls = user;
    if (!native_size(record, width, height)) return 0;
    if (calls && calls->count < 16) calls->records[calls->count++] = record;
    for (int y = 0; y < *height; ++y) for (int x = 0; x < *width; ++x)
        assets[record - 75][y * *width + x] = (uint8_t)((record + 3 * x + 7 * y) & 15);
    *pixels = assets[record - 75];
    return 1;
}

static int missing_wall(void* user, unsigned int record,
    const uint8_t** pixels, int* width, int* height)
{
    return record != 86 && source_graphic(user, record, pixels, width, height);
}

/* Independent F0797/F0128 oracle: the literal C255 corridors at column-major
 * entries10..14 and7 leave exactly these nine wall transactions. COORD.C
 * G2002 aliases/G3019 anchors below are resolved without production crop APIs.
 * The D3 outer pair comes first, D0 sides last; F20 has no D2 outer calls. */
static int source_page(uint8_t* page, DM1_V1_FmtownsEntranceGraphicProvider provider, void* user)
{
    static const struct { unsigned int record; int sx, width, x, y, flip; } walls[] = {
        {89,0,16,0,25,0}, {89,0,16,208,25,1},
        {88,32,84,0,25,0}, {88,0,85,139,25,0},
        {87,61,75,0,20,0},
        {86,188,60,0,9,0}, {86,0,60,164,9,0},
        {85,0,32,0,0,0}, {84,0,32,192,0,0}
    };
    const uint8_t* pixels;
    int width, height;
    memset(page, 0, 224 * 136);
    if (!provider(user, 76, &pixels, &width, &height) || width != 224 || height != 29) return 0;
    /* F0128 MEDIA544: even (2+0+South2) parity mirrors the ceiling
     * copied by F0098; the floor stays in its original orientation. */
    for (int y = 0; y < 29; ++y) for (int x = 0; x < 224; ++x)
        page[y * 224 + x] = pixels[y * 224 + 223 - x];
    if (!provider(user, 75, &pixels, &width, &height) || width != 224 || height != 70) return 0;
    memcpy(page + 66 * 224, pixels, 224 * 70);
    for (size_t i = 0; i < sizeof(walls) / sizeof(walls[0]); ++i) {
        int expected_width, expected_height;
        if (!provider(user, walls[i].record, &pixels, &width, &height) ||
            !native_size(walls[i].record, &expected_width, &expected_height) ||
            width != expected_width || height != expected_height) return 0;
        for (int y = 0; y < height; ++y) for (int x = 0; x < walls[i].width; ++x) {
            const int alias_x = walls[i].sx + x;
            const int sx = walls[i].flip ? width - 1 - alias_x : alias_x;
            const uint8_t color = pixels[y * width + sx];
            if (color != 10) page[(walls[i].y + y) * 224 + walls[i].x + x] = color;
        }
    }
    return 1;
}

static int original_graphic(void* user, unsigned int record,
    const uint8_t** pixels, int* width, int* height)
{
    M11_AssetLoader* loader = user;
    const M11_AssetSlot* slot = M11_AssetLoader_Load(loader, record);
    if (!slot || !slot->loaded || !slot->pixels) return 0;
    *pixels = slot->pixels; *width = slot->width; *height = slot->height;
    return 1;
}

/* Full production Draw: exact original HoC pose and accepted forward step.
 * COORD.C G3016 C007 and G2068 place the complete native136-row page at
 * EN y33 / JP y31. This does not establish original-emulator frame timing. */
static int original_runtime_walls(M11_GameViewState* state)
{
    uint8_t frame[320 * 200], rgb8[256][3];
    const int view_y = state->dm1FmtownsStartupReceipt.language == DM1_FMTOWNS_LANG_JP ? 31 : 33;
    for (int pose = 0; pose < 2; ++pose) {
        M11_Dm1F0128PerSquareSchedulerReceipt receipt;
        const int parity = pose;
        const int height = state->world.dungeon->maps[0].height;
        if (state->world.party.mapIndex != 0 || state->world.party.mapX != 1 ||
            state->world.party.mapY != 3 + pose || state->world.party.direction != 2 ||
            state->world.dungeon->maps[0].wallSet != 0) return 0;
        M11_GameView_Draw(state, frame, 320, 200);
        M11_GameView_GetDm1F0128PerSquareSchedulerReceipt(&receipt);
        if (!receipt.valid || receipt.d0WallMaterialCallbackStepCount != 2 ||
            !state->dm1FmtownsStartupReceipt.game_dungeon_palettes_verified ||
            !M11_Render_CopyIndexedPaletteRgb8(rgb8)) return 0;
        {
            const int level = M11_GameView_GetDungeonPaletteIndex(state);
            if (level < 0 || level > 5) return 0;
            for (int color = 0; color < 16; ++color) for (int channel = 0; channel < 3; ++channel) {
                const unsigned int dac = state->dm1FmtownsStartupReceipt.game_dungeon_palettes_rgb6[level][color][channel];
                if (rgb8[color + 16][channel] != (uint8_t)((dac << 2) | (dac >> 4))) {
                    fprintf(stderr, "F20 palette pose%d level%d color%d channel%d expected%u actual%u\n", pose, level, color, channel, (dac << 2) | (dac >> 4), rgb8[color + 16][channel]); return 0;
                }
            }
        }
        for (int right = 0; right < 2; ++right) {
            const int map_x = right ? 0 : 2;
            const unsigned int record = parity ? (right ? 85 : 84) : (right ? 84 : 85);
            const M11_AssetSlot* slot = M11_AssetLoader_Load(&state->assetLoader, record);
            const int left = right ? 192 : 0;
            if ((state->world.dungeon->tiles[0].squareData[map_x * height + 3 + pose] >> 5) != DUNGEON_ELEMENT_WALL ||
                !slot || slot->width != 32 || slot->height != 136) return 0;
            for (int y = 0; y < 136; ++y) for (int x = 0; x < 32; ++x) {
                const uint8_t pixel = slot->pixels[y * 32 + (parity ? 31 - x : x)];
                if (pixel != 10 && frame[(view_y + y) * 320 + left + x] != (uint8_t)(pixel | 16)) {
                    fprintf(stderr, "F20 D0 pose%d side%d pixel%d,%d expected%u actual%u\n", pose, right, left + x, view_y + y, pixel | 16, frame[(view_y + y) * 320 + left + x]); return 0;
                }
            }
        }
        {
            const M11_AssetSlot* slot = M11_AssetLoader_Load(&state->assetLoader, 87);
            if ((state->world.dungeon->tiles[0].squareData[2 * height + 5 + pose] >> 5) != DUNGEON_ELEMENT_WALL ||
                !slot || slot->width != 136 || slot->height != 71) return 0;
            /* G2002 D2L alias=(61,0,75,71), C708 at(0,20).
             * D1L's60columns leave precisely x60..74 exposed. Mirror the
             * entire native136columns before applying that alias. */
            for (int y = 0; y < 71; ++y) for (int x = 60; x < 75; ++x) {
                const int sx = parity ? 135 - (61 + x) : 61 + x;
                const uint8_t pixel = slot->pixels[y * 136 + sx];
                if (pixel != 10 && frame[(view_y + 20 + y) * 320 + x] != (uint8_t)(pixel | 16)) {
                    fprintf(stderr, "F20 D2L pose%d pixel%d,%d expected%u actual%u\n", pose, x, view_y + 20 + y, pixel | 16, frame[(view_y + 20 + y) * 320 + x]); return 0;
                }
            }
        }
        printf("PASS: original F20%s full Draw HoC(1,%d,S) parity%d D0/D2L aliases and original program dungeon palette\n",
            state->dm1FmtownsStartupReceipt.language == DM1_FMTOWNS_LANG_JP ? "J" : "E", 3 + pose, parity);
        if (!pose && M11_GameView_HandleInput(state, M12_MENU_INPUT_UP) != M11_GAME_INPUT_REDRAW) return 0;
    }
    return 1;
}

static int original_japanese_pointer(M11_GameViewState* state, const M11_GameLaunchSpec* spec)
{
    static const int rows[] = {31,32,166,30,167,168};
    static const char mirror_route[] = "UUUUULLLDDDDDLLULLULLLLLDLRR";
    uint8_t framebuffer[320 * 200];
    /* Debug viewport commands make C080 admission observable even where an
     * empty original page has no clickable Thing. Each case reloads the
     * authentic starting world; no party position or map is assigned. */
    for (size_t i = 0; i < sizeof(rows) / sizeof(rows[0]); ++i) {
        unsigned int tick;
        M11_GameInputResult result;
        if (!M11_GameView_Start(state, spec)) return 0;
        state->showDebugHUD = 1;
        tick = state->world.gameTick;
        result = M11_GameView_HandlePointerButton(state, 0, rows[i], M11_DM1_MOUSE_MASK_LEFT);
        if (state->pointerY != rows[i]) return 0;
        if (i < 3) {
            if (result != M11_GAME_INPUT_REDRAW || state->world.gameTick <= tick ||
                (i < 2 && state->world.party.direction != 1)) {
                fprintf(stderr, "JDM native C007 admitted row%d result%d tick%u->%u direction%d\n",
                    rows[i], result, tick, state->world.gameTick, state->world.party.direction); return 0;
            }
        } else if (result != M11_GAME_INPUT_IGNORED || state->world.gameTick != tick ||
                   state->world.party.direction != 2) {
            fprintf(stderr, "JDM C007 rejected row%d result%d tick%u->%u\n", rows[i], result, tick, state->world.gameTick); return 0;
        }
    }
    if (!M11_GameView_Start(state, spec)) return 0;
    state->showDebugHUD = 1;
    {
        const unsigned int tick = state->world.gameTick;
        if (M11_GameView_HandlePointerButton(state, 40, 170, M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_REDRAW ||
            state->pointerY != 170 || state->world.gameTick <= tick ||
            state->world.party.mapX != 1 || state->world.party.mapY != 4 ||
            state->world.party.direction != 2) {
            fprintf(stderr, "JDM debug control-strip forward rejected at(40,170)\n");
            return 0;
        }
    }
    if (!M11_GameView_Start(state, spec)) return 0;
    state->showDebugHUD = 0;
    for (size_t i = 0; mirror_route[i]; ++i) {
        const M12_MenuInput input = mirror_route[i] == 'U' ? M12_MENU_INPUT_UP :
            mirror_route[i] == 'D' ? M12_MENU_INPUT_DOWN :
            mirror_route[i] == 'L' ? M12_MENU_INPUT_STRAFE_LEFT : M12_MENU_INPUT_TURN_RIGHT;
        if (M11_GameView_HandleInput(state, input) == M11_GAME_INPUT_IGNORED) return 0;
        for (int tick = 0; tick < 5; ++tick) M11_GameView_AdvanceIdleTick(state);
    }
    if (state->world.party.mapIndex != 0 || state->world.party.mapX != 14 ||
        state->world.party.mapY != 3 || state->world.party.direction != 0 ||
        state->world.party.championCount) return 0;
    M11_GameView_Draw(state, framebuffer, 320, 200);
    /* F0172's real C127 mirror owns C026's D1C portrait at local y35.
     * Physical JDM y66 is its top edge; the old unadjusted y33 input
     * origin would produce local33 and reject this exact source target. */
    if (M11_GameView_HandlePointerButton(state, 112, 66, M11_DM1_MOUSE_MASK_LEFT) != M11_GAME_INPUT_REDRAW ||
        state->pointerY != 66 || !state->candidateMirrorPanelActive || state->candidateMirrorOrdinal != 5) {
        fprintf(stderr, "JDM original C127 top-edge click rejected at(14,3,N): panel%d ordinal%d\n",
            state->candidateMirrorPanelActive, state->candidateMirrorOrdinal); return 0;
    }
    puts("PASS: original JDM C007 pointer rows31/32/166 accepted,30/167/168 rejected; real C127 top-edge click y66 opens ordinal5");
    return 1;
}

static void check_original_media(void)
{
    const char* archive = getenv("FIRESTAFF_DM1_FMTOWNS_ARCHIVE");
    char default_archive[2048];
    FILE* file;
    if (!archive || !archive[0]) {
        const char* home = getenv("HOME");
        int length = home ? snprintf(default_archive, sizeof(default_archive),
            "%s/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip", home) : -1;
        if (length < 0 || (size_t)length >= sizeof(default_archive)) return;
        archive = default_archive;
    }
    file = fopen(archive, "rb");
    if (!file) {
        if (errno == ENOENT) { puts("SKIP: original F20 archive unavailable; geometry unit checks only"); return; }
        CHECK(file != NULL); return;
    }
    fclose(file);
    for (int japanese = 0; japanese < 2; ++japanese) {
        M11_GameViewState* state = calloc(1, sizeof(*state));
        M11_GameLaunchSpec spec = {0};
        M12_AssetStatus* scan = calloc(1, sizeof(*scan));
        const M12_AssetVersionStatus* version;
        char runtime[M12_ASSET_DATA_DIR_CAPACITY];
        const char* version_id = japanese ? "fmtowns-ja" : "fmtowns-en";
        DM1_V1_FmtownsEntranceViewportReceipt receipt;
        char graphics_md5[33];
        uint8_t actual[224 * 136], expected[224 * 136];
        int ok = 0, started = 0, stage = 0;
        if (!state || !scan) { free(state); free(scan); CHECK(0); return; }
        M11_GameView_Init(state);
        M12_AssetStatus_ScanGame(scan, archive, "dm1");
        {
            const int index = M12_AssetStatus_FindVersionIndex("dm1", version_id);
            version = index < 0 ? NULL : M12_AssetStatus_GetVersion(scan, "dm1", (size_t)index);
        }
        if (!version || !version->matched || !M12_AssetStatus_PrepareDM1RuntimeVersion(scan,
                version_id, runtime, sizeof(runtime))) goto done;
        spec.title = "Dungeon Master"; spec.gameId = spec.sourceId = "dm1";
        spec.dataDir = runtime; spec.verifiedAssetPath = version->matchedPath;
        spec.verifiedAssetMd5 = version->matchedMd5;
        spec.dm1Fmtowns = 1; spec.dm1FmtownsJapanese = japanese;
        /* Production startup binds the original EDM/JDM program and selected
         * graphics owner. No decoded media or driver substitute is installed. */
        started = M11_GameView_Start(state, &spec);
        if (!started || !state->dm1FmtownsStartupReceiptValid ||
            state->dm1FmtownsStartupReceipt.language != (japanese ? DM1_FMTOWNS_LANG_JP : DM1_FMTOWNS_LANG_EN) ||
            strcmp(state->dm1FmtownsStartupReceipt.game_program_name, japanese ? "JDM.EXP" : "EDM.EXP") ||
            strcmp(state->dm1FmtownsStartupReceipt.game_program_md5, japanese
                ? "acfbcfa5d65032a4bcabc8d5ea062dcc" : "c27e7b984df9753912c3375dc121919f") ||
            strncmp(version->matchedPath, archive, strlen(archive)) ||
            strncmp(version->matchedPath + strlen(archive), "::", 2) ||
            !m12_bytes_md5_hex(state->assetLoader.legacyData,
                (size_t)state->assetLoader.legacyDataSize, graphics_md5) ||
            strcmp(graphics_md5, version->matchedMd5) || strcmp(graphics_md5, japanese
                ? "edf47d7da5de8184604d6d80477ef01f" : "c10c512f63461ebe79b5ac365115b61b")) goto done;
        stage = 1;
        if (!source_page(expected, original_graphic, &state->assetLoader)) goto done;
        stage = 2;
        if (!M11_GameView_RenderDm1FmtownsEntranceC255(state, actual, sizeof(actual), 224, &receipt)) goto done;
        stage = 3;
        if (!receipt.valid || receipt.aperture_y != (japanese ? 31 : 33) ||
            memcmp(actual, expected, sizeof(actual))) goto done;
        stage = 4;
        if (!original_runtime_walls(state)) goto done;
        stage = 5;
        if (japanese && !original_japanese_pointer(state, &spec)) goto done;
        printf("PASS: original F20%s C255 all30464 native pixels, nine source walls, D0/outer order and aliases\n", japanese ? "J" : "E");
        ok = 1;
    done:
        CHECK(ok);
        if (!ok) fprintf(stderr, "F20%s original rejection stage%d started%d graphics=%s startup=%d language%d program%s md5%s\n",
            japanese ? "J" : "E", stage, started, state->assetLoader.graphicsDatPath, state->dm1FmtownsStartupReceiptValid,
            state->dm1FmtownsStartupReceipt.language, state->dm1FmtownsStartupReceipt.game_program_name,
            state->dm1FmtownsStartupReceipt.game_program_md5);
        M11_GameView_Shutdown(state);
        free(state);
        free(scan);
    }
}

int main(void)
{
    static const unsigned int call_order[] = {76,75,89,89,88,88,87,86,86,85,84};
    uint8_t actual[224 * 136], expected[224 * 136], repeat[224 * 136];
    DM1_V1_FmtownsEntranceViewportReceipt receipt;
    Calls calls = {{0},0};
    CHECK(source_page(expected, source_graphic, NULL));
    CHECK(dm1_v1_fmtowns_entrance_render_c255(actual, sizeof(actual), 224,
        source_graphic, &calls, DM1_FMTOWNS_LANG_EN, &receipt));
    CHECK(receipt.valid && receipt.aperture_x == 0 && receipt.aperture_y == 33);
    CHECK(receipt.missing_source_graphics == 0 && receipt.source_graphic_record_mask == UINT32_C(0x7e03));
    CHECK(receipt.requested_source_graphics == UINT32_C(0xf43f) &&
          receipt.loaded_source_graphics == receipt.requested_source_graphics);
    CHECK(calls.count == 11 && memcmp(calls.records, call_order, sizeof(call_order)) == 0);
    CHECK(memcmp(actual, expected, sizeof(actual)) == 0);
    CHECK(dm1_v1_fmtowns_entrance_render_c255(repeat, sizeof(repeat), 224,
        source_graphic, NULL, DM1_FMTOWNS_LANG_JP, &receipt));
    CHECK(receipt.valid && receipt.aperture_y == 31 && memcmp(repeat, expected, sizeof(repeat)) == 0);
    memset(actual, 0xee, sizeof(actual));
    CHECK(!dm1_v1_fmtowns_entrance_render_c255(actual, sizeof(actual), 224,
        missing_wall, NULL, DM1_FMTOWNS_LANG_JP, &receipt));
    CHECK(!receipt.valid && receipt.aperture_y == 31 && receipt.missing_source_graphics != 0);
    for (size_t i = 0; i < sizeof(actual); ++i) CHECK(actual[i] == 0xee);
    check_original_media();
    if (failures) return 1;
    puts("PASS: F20 C255 source ordering and independent pixel composition");
    return 0;
}
