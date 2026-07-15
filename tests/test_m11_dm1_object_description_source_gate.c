#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

#define CHECK(condition, message) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL: %s\n", (message)); \
        ++failures; \
    } \
} while (0)

static const char* graphics_dat_path(void) {
    const char* configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home = getenv("HOME");
    static char homePath[1024];
    FILE* file;

    if (configured && configured[0] != '\0') return configured;
    if (!home || home[0] == '\0') return NULL;
    snprintf(homePath, sizeof(homePath),
             "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    file = fopen(homePath, "rb");
    if (!file) return NULL;
    fclose(file);
    return homePath;
}

static void seed_description_state(
    M11_GameViewState* state,
    struct DungeonThings_Compat* things,
    struct DungeonWeapon_Compat* weapon)
{
    const unsigned short dagger = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int i;

    memset(things, 0, sizeof(*things));
    memset(weapon, 0, sizeof(*weapon));
    weapon->type = 8;
    things->weapons = weapon;
    things->weaponCount = 1;
    M11_GameView_Init(state);
    state->active = 1;
    state->inventoryPanelActive = 1;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
    CHECK(M11_GameView_SetV1LeaderHandObject(state, dagger) == 1,
          "leader hand accepts source weapon");
    CHECK(M11_GameView_HandlePointer(state, 20, 54, 1) == M11_GAME_INPUT_REDRAW,
          "eye click opens source object description");
    CHECK(state->v1ObjectDescriptionPanelActive == 1,
          "object description route is active");
}

static int framebuffer_has_source_pixel(const unsigned char* framebuffer,
                                        const M11_AssetSlot* asset,
                                        int dstX,
                                        int dstY,
                                        unsigned char transparent,
                                        int skipX,
                                        int skipY,
                                        int skipW,
                                        int skipH)
{
    int x;
    int y;
    if (!framebuffer || !asset || !asset->pixels) return 0;
    for (y = 0; y < (int)asset->height; ++y) {
        for (x = 0; x < (int)asset->width; ++x) {
            unsigned char pixel = asset->pixels[y * (int)asset->width + x];
            if (x >= skipX && x < skipX + skipW &&
                y >= skipY && y < skipY + skipH) {
                continue;
            }
            if (pixel != transparent) {
                return framebuffer[(dstY + y) * 320 + dstX + x] == pixel;
            }
        }
    }
    return 0;
}

int main(void)
{
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned char framebuffer[320 * 200];
    const char* graphicsPath;

    seed_description_state(&state, &things, &weapon);
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    CHECK(framebuffer[(33 + 52) * 320 + 80] == 0,
          "missing C020 keeps C017 panel pixels untouched");
    CHECK(framebuffer[(33 + 53) * 320 + 103] == 0,
          "missing C029 emits no procedural circle");
    CHECK(framebuffer[(33 + 59) * 320 + 111] == 0,
          "missing icon media emits no placeholder");
    M11_GameView_Shutdown(&state);

    graphicsPath = graphics_dat_path();
    if (graphicsPath) {
        const M11_AssetSlot* panel;
        const M11_AssetSlot* circle;

        seed_description_state(&state, &things, &weapon);
        CHECK(M11_AssetLoader_Init(&state.assetLoader, graphicsPath),
              "configured PC34 GRAPHICS.DAT loads");
        state.assetsAvailable = 1;
        M11_Font_Init(&state.originalFont);
        CHECK(M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                           state.assetLoader.fileState,
                                           state.assetLoader.runtimeState),
              "PC34 M653 source font loads");
        state.originalFontAvailable = 1;
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        panel = M11_AssetLoader_Load(&state.assetLoader, 20u);
        circle = M11_AssetLoader_Load(&state.assetLoader, 29u);
        CHECK(framebuffer_has_source_pixel(framebuffer, panel, 80, 33 + 52, 8,
                                           -1, -1, 0, 0),
              "F0342 presents C020 source pixels at C101");
        CHECK(framebuffer_has_source_pixel(framebuffer, circle, 103, 33 + 53, 1,
                                           8, 6, 16, 16),
              "F0342 presents C029 source pixels at C504");
        M11_GameView_Shutdown(&state);
    }

    if (failures) return 1;
    printf("PASS: DM1 object-description source gate\n");
    return 0;
}
