#include "m11_game_view.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"

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

static const char* graphics_dat_path(void)
{
    const char* root = getenv("FIRESTAFF_DM1_DATA_DIR");
    static char path[1024];

    if (!root || root[0] == '\0') return NULL;
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    return path;
}

static void seed_status_state(M11_GameViewState* state)
{
    M11_GameView_Init(state);
    state->active = 1;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
}

static int framebuffer_has_source_pixel(const unsigned char* framebuffer,
                                        const M11_AssetSlot* asset,
                                        int dstX, int dstY)
{
    int x;
    int y;

    if (!framebuffer || !asset || !asset->pixels) return 0;
    for (y = 0; y < (int)asset->height; ++y) {
        for (x = 0; x < (int)asset->width; ++x) {
            unsigned char pixel = asset->pixels[y * (int)asset->width + x];
            if (pixel != 0 &&
                framebuffer[(dstY + y) * 320 + dstX + x] == pixel) {
                return 1;
            }
        }
    }
    return 0;
}

static int rect_is_color(const unsigned char* framebuffer,
                         const DM1_V1_ChampionStatusRectPc34* rect,
                         unsigned char color)
{
    int x;
    int y;
    if (!framebuffer || !rect) return 0;
    for (y = 0; y < rect->h; ++y) {
        for (x = 0; x < rect->w; ++x) {
            if (framebuffer[(rect->y + y) * 320 + rect->x + x] != color) {
                return 0;
            }
        }
    }
    return 1;
}

int main(void)
{
    M11_GameViewState state;
    unsigned char baseline[320 * 200];
    unsigned char augmented[320 * 200];
    const char* graphicsPath;

    seed_status_state(&state);
    memset(baseline, 0, sizeof(baseline));
    M11_GameView_Draw(&state, baseline, 320, 200);
    state.world.magic.fireShieldDefense = 1;
    state.world.party.champions[0].poisonDose = 1;
    memset(augmented, 0, sizeof(augmented));
    M11_GameView_Draw(&state, augmented, 320, 200);
    CHECK(memcmp(baseline, augmented, sizeof(baseline)) == 0,
          "missing C032/C038 keeps F0292 auxiliary media unavailable");
    M11_GameView_Shutdown(&state);

    graphicsPath = graphics_dat_path();
    if (graphicsPath) {
        const M11_AssetSlot* shield;
        const M11_AssetSlot* poison;
        DM1_V1_ChampionStatusRectPc34 statusRect;
        DM1_V1_ChampionStatusRectPc34 poisonRect;
        DM1_V1_ChampionStatusRectPc34 nameRect;
        int shieldGraphics[3] = {0, 0, 0};
        unsigned char foreignFont[M11_FONT_BITMAP_BYTES];

        seed_status_state(&state);
        if (!M11_AssetLoader_Init(&state.assetLoader, graphicsPath)) {
            fprintf(stderr, "configured PC34 GRAPHICS.DAT failed to load\n");
            M11_GameView_Shutdown(&state);
            return 1;
        }
        state.assetsAvailable = 1;
        snprintf(state.world.party.champions[0].name,
                 sizeof(state.world.party.champions[0].name), "HISS");
        memset(baseline, 0, sizeof(baseline));
        M11_GameView_Draw(&state, baseline, 320, 200);
        state.world.magic.fireShieldDefense = 1;
        state.world.party.champions[0].poisonDose = 1;
        memset(augmented, 0, sizeof(augmented));
        M11_GameView_Draw(&state, augmented, 320, 200);
        CHECK(dm1_v1_champion_status_shield_border_graphics_pc34(
                  1, 0, 0, shieldGraphics) == 1,
              "F0292 resolves one fire-shield border");
        shield = M11_AssetLoader_Load(&state.assetLoader,
                                      (unsigned int)shieldGraphics[0]);
        poison = M11_AssetLoader_Load(
            &state.assetLoader,
            (unsigned int)dm1_v1_graphic_poisoned_label_pc34());
        CHECK(shield && shield->loaded && shield->pixels &&
                  shield->width == 67 && shield->height == 29,
              "C038 is an exact PC34 status-border surface");
        CHECK(poison && poison->loaded && poison->pixels &&
                  poison->width == 96 && poison->height == 15,
              "C032 is an exact PC34 poisoned-label surface");
        CHECK(dm1_v1_champion_status_shield_border_rect_pc34(0, &statusRect),
              "F0292 resolves the C038 destination");
        CHECK(dm1_v1_champion_poison_label_rect_pc34(
                  0, (int)poison->width, (int)poison->height, &poisonRect),
              "F0292 resolves the C032 destination");
        CHECK(memcmp(baseline, augmented, sizeof(baseline)) != 0,
              "C032/C038 alter the live F0292 frame");
        CHECK(framebuffer_has_source_pixel(augmented, shield,
                                           statusRect.x, statusRect.y),
              "F0292 presents C038 source pixels at the status rectangle");
        CHECK(framebuffer_has_source_pixel(augmented, poison,
                                           poisonRect.x, poisonRect.y),
              "F0292 presents C032 source pixels at C502");

        /* F0292's C159 name strip and its F0053 glyphs have one M653 owner.
         * A 768-byte payload with a foreign graphic identity must leave the
         * already-cleared strip intact instead of rendering substitute text. */
        M11_Font_Init(&state.originalFont);
        CHECK(M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                           state.assetLoader.fileState,
                                           state.assetLoader.runtimeState),
              "PC34 M653 source font loads for status name");
        state.originalFontAvailable = 1;
        memset(baseline, 0, sizeof(baseline));
        M11_GameView_Draw(&state, baseline, 320, 200);
        CHECK(dm1_v1_champion_status_name_rect_pc34(0, &nameRect),
              "F0292 resolves C159 status-name rectangle");
        CHECK(!rect_is_color(baseline, &nameRect,
                             (unsigned char)
                                 dm1_v1_champion_status_name_clear_color_pc34()),
              "M653 draws the source champion name");
        memcpy(foreignFont, state.originalFont.bitmap, sizeof(foreignFont));
        CHECK(M11_Font_LoadFromRawBitmap(&state.originalFont, 694,
                                         foreignFont, sizeof(foreignFont)),
              "foreign 768-byte status-font fixture loads");
        state.originalFontAvailable = 1;
        memset(augmented, 0, sizeof(augmented));
        M11_GameView_Draw(&state, augmented, 320, 200);
        CHECK(rect_is_color(augmented, &nameRect,
                            (unsigned char)
                                dm1_v1_champion_status_name_clear_color_pc34()),
              "foreign font cannot draw the F0292 source name strip");
        M11_GameView_Shutdown(&state);
    }

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS: DM1 F0292 auxiliary source-material gate\n");
    return 0;
}
