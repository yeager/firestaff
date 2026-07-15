#include "m11_game_view.h"
#include "dm1_v1_layout_zones_pc34_compat.h"

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

static const char *graphics_dat_path(void)
{
    const char *configured = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char *home = getenv("HOME");
    static char home_path[1024];
    FILE *file;

    if (configured && configured[0] != '\0') return configured;
    if (!home || home[0] == '\0') return NULL;
    snprintf(home_path, sizeof(home_path),
             "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    file = fopen(home_path, "rb");
    if (!file) return NULL;
    fclose(file);
    return home_path;
}

static void seed_food_water_state(M11_GameViewState *state)
{
    M11_GameView_Init(state);
    state->active = 1;
    state->inventoryPanelActive = 1;
    state->v1FoodWaterPanelActive = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    state->world.party.champions[0].food = 512;
    state->world.party.champions[0].water = 256;
    state->world.party.champions[0].poisonDose = 1;
}

static int framebuffer_has_source_pixel(const unsigned char *framebuffer,
                                        const M11_AssetSlot *asset,
                                        int dst_x,
                                        int dst_y)
{
    int x;
    int y;

    if (!framebuffer || !asset || !asset->pixels) return 0;
    for (y = 0; y < (int)asset->height; ++y) {
        for (x = 0; x < (int)asset->width; ++x) {
            unsigned char pixel = asset->pixels[y * (int)asset->width + x];
            if (pixel != 12 &&
                framebuffer[(dst_y + y) * 320 + dst_x + x] == pixel) {
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    M11_GameViewState state;
    unsigned char frame[320 * 200];
    const char *graphics_path;

    /* No PC34 media: F0345 consumes the mouth-panel route but must not
     * fabricate C020/C030/C031/C032 with a host panel or text surface. */
    seed_food_water_state(&state);
    memset(frame, 0, sizeof(frame));
    M11_GameView_Draw(&state, frame, 320, 200);
    CHECK(state.v1FoodWaterPanelActive == 1,
          "F0345 no-media gate preserves the source runtime state");
    M11_GameView_Shutdown(&state);

    graphics_path = graphics_dat_path();
    if (graphics_path) {
        const M11_AssetSlot *panel;
        const M11_AssetSlot *food;
        const M11_AssetSlot *water;
        const M11_AssetSlot *poison;
        DM1_V1_LayoutZoneRectPc34 panel_rect;
        DM1_V1_LayoutZoneRectPc34 viewport_rect;

        seed_food_water_state(&state);
        CHECK(M11_AssetLoader_Init(&state.assetLoader, graphics_path),
              "configured PC34 GRAPHICS.DAT loads");
        state.assetsAvailable = 1;
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);

        panel_rect = dm1_v1_inventory_panel_rect_pc34();
        viewport_rect = dm1_v1_viewport_rect_pc34();
        panel = M11_AssetLoader_Load(&state.assetLoader, 20);
        food = M11_AssetLoader_Load(&state.assetLoader, 30);
        water = M11_AssetLoader_Load(&state.assetLoader, 31);
        poison = M11_AssetLoader_Load(&state.assetLoader, 32);
        CHECK(panel && panel->loaded && panel->pixels &&
                  panel->width == panel_rect.w && panel->height == panel_rect.h,
              "C020 is the exact PC34 F0345 panel surface");
        CHECK(food && food->loaded && food->pixels &&
                  food->width == 34 && food->height == 9,
              "C030 is the exact PC34 food label");
        CHECK(water && water->loaded && water->pixels &&
                  water->width == 46 && water->height == 9,
              "C031 is the exact PC34 water label");
        CHECK(poison && poison->loaded && poison->pixels &&
                  poison->width == 96 && poison->height == 15,
              "C032 is the exact PC34 poison label");
        if (panel && food && water && poison) {
            CHECK(framebuffer_has_source_pixel(
                      frame, panel, viewport_rect.x + panel_rect.x,
                      viewport_rect.y + panel_rect.y),
                  "F0345 presents C020 source pixels at C101");
            CHECK(framebuffer_has_source_pixel(
                      frame, food, viewport_rect.x + panel_rect.x + 32,
                      viewport_rect.y + panel_rect.y + 8),
                  "F0658 presents C030 source pixels at C500");
            CHECK(framebuffer_has_source_pixel(
                      frame, water, viewport_rect.x + panel_rect.x + 32,
                      viewport_rect.y + panel_rect.y + 31),
                  "F0658 presents C031 source pixels at C501");
            CHECK(framebuffer_has_source_pixel(
                      frame, poison, viewport_rect.x + panel_rect.x + 32,
                      viewport_rect.y + panel_rect.y + 50),
                  "F0658 presents C032 source pixels at C502");
        }
        M11_GameView_Shutdown(&state);
    }

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS: DM1 F0345 food/water source-material gate\n");
    return 0;
}
