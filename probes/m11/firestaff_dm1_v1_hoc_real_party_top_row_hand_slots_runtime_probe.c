/*
 * DM1 V1 real Hall-of-Champions party top-row and hand-slot probe.
 *
 * The probe builds its four-member party only by selecting and resurrecting
 * four distinct C127 mirrors from the hash-verified PC 3.4 DUNGEON.DAT.
 * It then captures the live M11 V1 top row and checks C151..C154 geometry,
 * the two-pixel inter-box gutters, real champion names, and C211..C218
 * hand-slot perimeters against their GRAPHICS.DAT source assets.
 *
 * ReDMCSB: DUNGEON.C F0172/F0174 and DUNVIEW.C F0128 expose C127 mirrors;
 * REVIVE.C F0280/F0282 selects and resurrects candidates; CHAMDRAW.C F0292
 * draws C151..C154; CHAMDRAW.C F0291 lines 632-651 draws C211..C218 using
 * C033/C034/C035. DEFS.H:2157 fixes the 69-pixel top-row stride.
 */
#include "asset_loader_m11.h"
#include "dm1_v1_champion_status_layout_pc34_compat.h"
#include "m11_game_view.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_PARTY_SIZE = 4,
    PROBE_STATUS_W = 67,
    PROBE_STATUS_H = 29,
    PROBE_STATUS_STRIDE = 69,
    PROBE_HAND_W = 18,
    PROBE_HAND_H = 18
};

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static int expect_true(const char* label, int value)
{
    if (!value) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static unsigned char pixel_index(const unsigned char* fb, int x, int y)
{
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * PROBE_FB_W + x]);
}

static int count_color(const unsigned char* fb,
                       int x, int y, int w, int h, int color)
{
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (pixel_index(fb, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int recruit_real_hoc_party(M11_GameViewState* game)
{
    int recruited[24] = {0};
    int map_index;
    int ok = 1;

    if (!game || !game->world.dungeon) {
        return 0;
    }
    for (map_index = 0;
         map_index < (int)game->world.dungeon->header.mapCount &&
         game->world.party.championCount < PROBE_PARTY_SIZE;
         ++map_index) {
        const struct DungeonMapDesc_Compat* map =
            &game->world.dungeon->maps[map_index];
        int y;
        for (y = 0;
             y < (int)map->height &&
             game->world.party.championCount < PROBE_PARTY_SIZE;
             ++y) {
            int x;
            for (x = 0;
                 x < (int)map->width &&
                 game->world.party.championCount < PROBE_PARTY_SIZE;
                 ++x) {
                int direction;
                for (direction = 0;
                     direction < 4 &&
                     game->world.party.championCount < PROBE_PARTY_SIZE;
                     ++direction) {
                    int ordinal;
                    int before;
                    game->world.party.mapIndex = map_index;
                    game->world.party.mapX = x;
                    game->world.party.mapY = y;
                    game->world.party.direction = direction;
                    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
                    if (ordinal < 0 || ordinal >= 24 || recruited[ordinal]) {
                        continue;
                    }
                    before = game->world.party.championCount;
                    if (M11_GameView_SelectFrontMirrorCandidate(game) != 1 ||
                        M11_GameView_ConfirmMirrorCandidate(game, 0) != 1 ||
                        game->world.party.championCount != before + 1) {
                        return 0;
                    }
                    recruited[ordinal] = 1;
                }
            }
        }
    }
    ok &= expect_int("four real HoC candidates resurrected",
                     game->world.party.championCount, PROBE_PARTY_SIZE);
    return ok;
}

static int check_hand_perimeter(const M11_GameViewState* game,
                                const unsigned char* fb,
                                int slot,
                                int hand)
{
    DM1_V1_ChampionStatusRectPc34 rect;
    const struct ChampionState_Compat* champion =
        &game->world.party.champions[slot];
    const M11_AssetSlot* asset;
    int graphic;
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[128];

    graphic = dm1_v1_champion_status_hand_slot_graphic_pc34(
        hand, champion->wounds,
        game->actingChampionOrdinal == (unsigned int)(slot + 1));
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphic);
    snprintf(label, sizeof(label), "slot%d hand%d source slot asset", slot, hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                      asset->width == PROBE_HAND_W && asset->height == PROBE_HAND_H);
    snprintf(label, sizeof(label), "slot%d hand%d top-row zone", slot, hand);
    ok &= expect_true(label,
                      dm1_v1_champion_status_hand_slot_box_rect_pc34(
                          slot, hand, &rect) &&
                      rect.w == PROBE_HAND_W && rect.h == PROBE_HAND_H);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }
    for (yy = 0; yy < rect.h; ++yy) {
        int xx;
        for (xx = 0; xx < rect.w; ++xx) {
            unsigned char source;
            if (xx > 0 && xx < rect.w - 1 && yy > 0 && yy < rect.h - 1) {
                continue;
            }
            source = (unsigned char)(asset->pixels[yy * rect.w + xx] & 0x0f);
            if (source == 0) {
                continue;
            }
            ++expected;
            if (pixel_index(fb, rect.x + xx, rect.y + yy) == source) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d hand%d GRAPHICS.DAT perimeter", slot, hand);
    ok &= expect_true(label, expected > 0 && matched == expected);
    return ok;
}

static int check_top_row(const M11_GameViewState* game, const unsigned char* fb)
{
    int slot;
    int ok = 1;
    const int fill_color = dm1_v1_champion_status_box_fill_color_pc34();

    for (slot = 0; slot < PROBE_PARTY_SIZE; ++slot) {
        DM1_V1_ChampionStatusRectPc34 status;
        DM1_V1_ChampionStatusRectPc34 name;
        char label[128];
        snprintf(label, sizeof(label), "slot%d C151-C154 top-row geometry", slot);
        ok &= expect_true(label,
                          dm1_v1_champion_status_box_rect_pc34(slot, &status) &&
                          status.x == slot * PROBE_STATUS_STRIDE && status.y == 0 &&
                          status.w == PROBE_STATUS_W && status.h == PROBE_STATUS_H);
        snprintf(label, sizeof(label), "slot%d live status-box fill", slot);
        ok &= expect_true(label,
                          count_color(fb, status.x, status.y,
                                      status.w, status.h, fill_color) > 300);
        snprintf(label, sizeof(label), "slot%d real champion name zone", slot);
        ok &= expect_true(label,
                          game->world.party.champions[slot].present &&
                          game->world.party.champions[slot].name[0] != ' ' &&
                          dm1_v1_champion_status_name_text_rect_pc34(slot, &name) &&
                          count_color(fb, name.x, name.y, name.w, name.h,
                                      dm1_v1_champion_status_name_color_pc34(
                                          1,
                                          game->world.party.champions[slot].hp.current,
                                          slot == game->world.party.activeChampionIndex)) > 0);
        ok &= check_hand_perimeter(game, fb, slot, 0);
        ok &= check_hand_perimeter(game, fb, slot, 1);
        if (slot + 1 < PROBE_PARTY_SIZE) {
            snprintf(label, sizeof(label), "slot%d top-row two-pixel gutter", slot);
            ok &= expect_int(label,
                             count_color(fb, status.x + status.w, status.y,
                                         PROBE_STATUS_STRIDE - status.w,
                                         status.h, 0),
                             (PROBE_STATUS_STRIDE - status.w) * status.h);
        }
    }
    return ok;
}

int main(int argc, char** argv)
{
    const char* data_dir;
    M11_GameViewState game;
    unsigned char framebuffer[PROBE_FB_W * PROBE_FB_H];
    int ok;

    if (argc != 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    data_dir = argv[1];
    M11_GameView_Init(&game);
    if (!M11_GameView_StartDm1(&game, data_dir)) {
        fprintf(stderr, "FAIL could not open hash-verified DM1 data from %s\n",
                data_dir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    game.showDebugHUD = 0;
    ok = recruit_real_hoc_party(&game);
    if (ok) {
        memset(framebuffer, 0, sizeof(framebuffer));
        M11_GameView_Draw(&game, framebuffer, PROBE_FB_W, PROBE_FB_H);
        ok = check_top_row(&game, framebuffer);
    }
    M11_GameView_Shutdown(&game);
    printf("%s DM1 V1 real HoC party top-row hand-slot probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
