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

static void seed_damage_state(M11_GameViewState* state)
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
            /* F0320's three source-font glyphs occupy C167's middle. */
            if (x >= 8 && x < 28) continue;
            if (pixel != 0 &&
                framebuffer[(dstY + y) * 320 + dstX + x] == pixel) {
                return 1;
            }
        }
    }
    return 0;
}

int main(void)
{
    M11_GameViewState state;
    unsigned char baseline[320 * 200];
    unsigned char damaged[320 * 200];
    const char* graphicsPath;

    seed_damage_state(&state);
    memset(baseline, 0, sizeof(baseline));
    M11_GameView_Draw(&state, baseline, 320, 200);
    state.championDamageTimer[0] = 3;
    state.championDamageAmount[0] = 42;
    memset(damaged, 0, sizeof(damaged));
    M11_GameView_Draw(&state, damaged, 320, 200);
    CHECK(memcmp(baseline, damaged, sizeof(baseline)) == 0,
          "missing C015/M653 leaves DM1 damage feedback unavailable");
    M11_GameView_Shutdown(&state);

    /* Inventory C016 must obey the same source gate.  The old M11 path
     * painted a red rectangle and a host number when the real 32x29 bitmap
     * was absent, which presented as a false damage square in HoC. */
    seed_damage_state(&state);
    state.inventoryPanelActive = 1;
    memset(baseline, 0, sizeof(baseline));
    M11_GameView_Draw(&state, baseline, 320, 200);
    state.championDamageTimer[0] = 3;
    state.championDamageAmount[0] = 42;
    memset(damaged, 0, sizeof(damaged));
    M11_GameView_Draw(&state, damaged, 320, 200);
    CHECK(memcmp(baseline, damaged, sizeof(baseline)) == 0,
          "missing C016 leaves DM1 inventory damage feedback unavailable");
    M11_GameView_Shutdown(&state);

    graphicsPath = graphics_dat_path();
    if (graphicsPath) {
        const M11_AssetSlot* damage;
        DM1_V1_ChampionStatusRectPc34 rect;

        seed_damage_state(&state);
        CHECK(M11_AssetLoader_Init(&state.assetLoader, graphicsPath),
              "configured PC34 GRAPHICS.DAT loads");
        state.assetsAvailable = 1;
        M11_Font_Init(&state.originalFont);
        CHECK(M11_Font_LoadFromGraphicsDat(&state.originalFont,
                                           state.assetLoader.fileState,
                                           state.assetLoader.runtimeState),
              "PC34 M653 source font loads");
        state.originalFontAvailable = 1;
        memset(baseline, 0, sizeof(baseline));
        M11_GameView_Draw(&state, baseline, 320, 200);
        state.championDamageTimer[0] = 3;
        state.championDamageAmount[0] = 42;
        memset(damaged, 0, sizeof(damaged));
        M11_GameView_Draw(&state, damaged, 320, 200);
        damage = M11_AssetLoader_Load(
            &state.assetLoader,
            (unsigned int)dm1_v1_graphic_champion_damage_small_pc34());
        CHECK(damage && damage->loaded && damage->pixels &&
                  damage->width == 45 && damage->height == 7,
              "C015 is an exact PC34 damage surface");
        CHECK(dm1_v1_champion_damage_indicator_rect_pc34(
                  0, (int)damage->width, (int)damage->height, &rect),
              "C167 supplies C015 destination geometry");
        CHECK(memcmp(baseline, damaged, sizeof(baseline)) != 0,
              "C015 and M653 make pending damage visible");
        CHECK(framebuffer_has_source_pixel(damaged, damage, rect.x, rect.y),
              "F0623 presents C015 source pixels at C167");
        M11_GameView_Shutdown(&state);
    }

    if (failures != 0) {
        fprintf(stderr, "%d failure(s)\n", failures);
        return 1;
    }
    printf("PASS: DM1 F0623 source damage indicator gate\n");
    return 0;
}
