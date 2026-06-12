/*
 * DM1 V1 champion panel runtime pixel probe.
 *
 * This is Firestaff-side evidence only.  It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs a deterministic four
 * champion party, renders one V1 frame, and checks that the party HUD/status
 * boxes are populated through the real M11 draw stack and GRAPHICS.DAT-backed
 * slot-box assets.  It does not claim original DOS screenshot parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 draws the C151..C154 status boxes;
 *   ReDMCSB CHAMDRAW.C F0287 draws bottom-anchored HP/stamina/mana bars;
 *   ReDMCSB CHAMDRAW.C F0291 draws C033/C034/C035 hand slot boxes;
 *   ReDMCSB CHAMDRAW.C F0622 lines 41-58 prepares the C113..C116
 *   19x14 champion icon composite from the C028 icon strip.
 *   ReDMCSB COORD.C/layout-696 anchors C113..C116 champion icon zones.
 *   ReDMCSB COORD.C/layout-696 keeps C151..C154 on a 69px stride with
 *   67x29 status boxes, leaving two black pixels between adjacent boxes.
 *   ReDMCSB CHAMDRAW.C F0292 lines 879-884 clears the live status-box
 *   name strip after the full 67x29 box has already been refreshed.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_CHAMPION_COUNT = 4,
    PROBE_STALE_PIXEL = M11_FB_ENCODE(0, 15),
    PROBE_ACTION_EMPTY_HAND_ICON = 201,
    PROBE_ACTION_CELL_CYAN = 4
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int count_color(const unsigned char* fb,
                       int width,
                       int x,
                       int y,
                       int w,
                       int h,
                       int color) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if ((int)px_index(fb, width, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int count_raw_pixel(const unsigned char* fb,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char rawPixel) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] == rawPixel) {
                ++count;
            }
        }
    }
    return count;
}

static int expected_fill_height(int current, int maximum, int fullHeight) {
    long scaled;
    if (current <= 0 || maximum <= 0 || fullHeight <= 0) {
        return 0;
    }
    scaled = (long)fullHeight * (long)current / (long)maximum;
    if (scaled < 1) {
        scaled = 1;
    }
    if (scaled > fullHeight) {
        scaled = fullHeight;
    }
    return (int)scaled;
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction,
                          int hp,
                          int hpMax,
                          int stamina,
                          int staminaMax,
                          int mana,
                          int manaMax,
                          unsigned short wounds) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = (unsigned short)hpMax;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = (unsigned short)staminaMax;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = (unsigned short)manaMax;
    champ->wounds = wounds;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 2; /* slot 1 action hand uses C035 */
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 100, 80, 80, 60, 60, 0);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 50, 100, 40, 80, 30, 60, 0x0001u);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 25, 100, 20, 80, 15, 60, 0x0002u);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 1, 100, 1, 80, 1, 60, 0);
}

static int check_status_box_pixels(const M11_GameViewState* game,
                                   const unsigned char* fb,
                                   int slot) {
    int ok = 1;
    int x, y, w, h;
    int nx, ny, nw, nh;
    int nameTextX, nameTextY, nameTextW, nameTextH;
    int fillColor = M11_GameView_GetV1StatusBoxFillColor();
    int nameClearColor = M11_GameView_GetV1StatusNameClearColor();
    int nameColor = M11_GameView_GetV1StatusNameColor(game, slot);
    char label[128];

    snprintf(label, sizeof(label), "slot%d status box zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                             w == 67 && h == 29);

    snprintf(label, sizeof(label), "slot%d status box fill visible", slot);
    ok &= expect_true(label, count_color(fb, PROBE_FB_W, x, y, w, h, fillColor) > 300);

    /* ReDMCSB: CHAMDRAW.C F0292 lines 879-905 refreshes live status
     * boxes before drawing name text and bar graphs.  Render over a
     * nonzero framebuffer-level sentinel and prove the full 67x29
     * party-HUD/status-box rectangle was touched by the M11 V1 draw
     * stack.  This is Firestaff runtime evidence, not DOS parity. */
    snprintf(label, sizeof(label), "slot%d status box overwrites stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);

    snprintf(label, sizeof(label), "slot%d name clear zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusNameZone(slot, &nx, &ny, &nw, &nh) &&
                             count_color(fb, PROBE_FB_W, nx, ny, nw, nh, nameClearColor) > 90);

    snprintf(label, sizeof(label), "slot%d name text color", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusNameTextZone(slot,
                                                                  &nameTextX,
                                                                  &nameTextY,
                                                                  &nameTextW,
                                                                  &nameTextH) &&
                             count_color(fb, PROBE_FB_W,
                                         nameTextX, nameTextY,
                                         nameTextW, nameTextH,
                                         nameColor) > 0);
    return ok;
}

static int check_status_box_gutter_pixels(const unsigned char* fb) {
    int ok = 1;
    int slot;
    char label[128];

    for (slot = 0; slot < PROBE_CHAMPION_COUNT - 1; ++slot) {
        int x, y, w, h;
        int nextX, nextY, nextW, nextH;
        int gutterX;
        int gutterW;
        snprintf(label, sizeof(label), "slot%d status box zone for gutter", slot);
        ok &= expect_true(label, M11_GameView_GetV1StatusBoxZone(slot,
                                                                 &x, &y,
                                                                 &w, &h) &&
                                 w == 67 && h == 29);
        snprintf(label, sizeof(label), "slot%d next status box zone for gutter", slot);
        ok &= expect_true(label, M11_GameView_GetV1StatusBoxZone(slot + 1,
                                                                 &nextX, &nextY,
                                                                 &nextW, &nextH) &&
                                 nextY == y && nextW == 67 && nextH == 29);
        if (!ok) {
            return 0;
        }
        gutterX = x + w;
        gutterW = nextX - gutterX;
        snprintf(label, sizeof(label), "slot%d status box two-pixel gutter width",
                 slot);
        ok &= expect_int(label, gutterW, 2);
        snprintf(label, sizeof(label), "slot%d status box gutter remains black",
                 slot);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     gutterX, y,
                                     gutterW, h,
                                     0),
                         gutterW * h);
    }
    return ok;
}

static int check_bar_pixels(const M11_GameViewState* game,
                            const unsigned char* fb,
                            int slot,
                            int stat) {
    const struct ChampionState_Compat* champ = &game->world.party.champions[slot];
    int current[3];
    int maximum[3];
    int x, y, w, h;
    int fillHeight;
    int blankHeight;
    int fillColor = M11_GameView_GetV1ChampionBarColor(slot);
    int blankColor = M11_GameView_GetV1StatusBarBlankColor();
    int ok = 1;
    char label[128];

    current[0] = champ->hp.current;
    current[1] = champ->stamina.current;
    current[2] = champ->mana.current;
    maximum[0] = champ->hp.maximum;
    maximum[1] = champ->stamina.maximum;
    maximum[2] = champ->mana.maximum;

    snprintf(label, sizeof(label), "slot%d stat%d bar zone", slot, stat);
    ok &= expect_true(label, M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
                             w == 4 && h == 25);
    fillHeight = expected_fill_height(current[stat], maximum[stat], h);
    blankHeight = h - fillHeight;
    if (blankHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d blank top", slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W, x, y, w, blankHeight, blankColor),
                         w * blankHeight);
    }
    if (fillHeight > 0) {
        snprintf(label, sizeof(label), "slot%d stat%d colored bottom", slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     x, y + blankHeight,
                                     w, fillHeight,
                                     fillColor),
                         w * fillHeight);
    }
    return ok;
}

static int check_hand_slot_asset_pixels(const M11_GameViewState* game,
                                        const unsigned char* fb,
                                        int slot,
                                        int hand) {
    int gfx = M11_GameView_GetV1StatusHandSlotGraphic(game, slot, hand);
    const M11_AssetSlot* asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                                     (unsigned int)gfx);
    int x, y, w, h;
    int expected = 0;
    int matched = 0;
    int yy;
    char label[128];
    int ok = 1;

    snprintf(label, sizeof(label), "slot%d hand%d expected graphic", slot, hand);
    ok &= expect_true(label, gfx == M11_GameView_GetV1SlotBoxNormalGraphicId() ||
                             gfx == M11_GameView_GetV1SlotBoxWoundedGraphicId() ||
                             gfx == M11_GameView_GetV1SlotBoxActingHandGraphicId());
    if (!ok) {
        return 0;
    }
    snprintf(label, sizeof(label), "slot%d hand%d slot-box asset", slot, hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 18 && asset->height == 18);
    snprintf(label, sizeof(label), "slot%d hand%d slot-box zone", slot, hand);
    ok &= expect_true(label, M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                                     &x, &y, &w, &h) &&
                             w == 18 && h == 18);
    if (!ok) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (xx > 0 && xx < 17 && yy > 0 && yy < 17) {
                continue; /* object/empty-hand icon inset may overdraw */
            }
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) {
                continue;
            }
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d hand%d GRAPHICS.DAT perimeter match", slot, hand);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 85);
    printf("slot%d hand%d gfx=%d perimeter=%d/%d\n", slot, hand, gfx, matched, expected);
    return ok;
}

/* ReDMCSB: CHAMDRAW.C F0291 lines 632-651 draws the C033/C034/C035
 * status-hand slot box, then draws the selected 16x16 icon one pixel
 * inside that box.  For empty ready/action hands the source icon
 * ordinals are C212/C213 and C214/C215 respectively.  This check
 * pixel-matches the on-screen inset against the GRAPHICS.DAT object
 * icon sheet for every live party hand slot. */
static int check_hand_slot_icon_pixels(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int slot,
                                       int hand) {
    int iconIndex = M11_GameView_GetV1StatusHandIconIndex(game, slot, hand);
    int graphicIndex;
    int srcX, srcY, srcW, srcH;
    const M11_AssetSlot* asset;
    int x, y, w, h;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d hand%d empty-hand icon index", slot, hand);
    ok &= expect_true(label,
                      iconIndex == 212 || iconIndex == 213 ||
                      iconIndex == 214 || iconIndex == 215);
    snprintf(label, sizeof(label), "slot%d hand%d icon source zone", slot, hand);
    ok &= expect_true(label,
                      M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                             &graphicIndex,
                                                             &srcX, &srcY,
                                                             &srcW, &srcH) &&
                      srcW == 16 && srcH == 16);
    snprintf(label, sizeof(label), "slot%d hand%d icon screen zone", slot, hand);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusHandIconZone(slot, hand,
                                                           &x, &y, &w, &h) &&
                      w == 16 && h == 16);
    if (!ok) {
        return 0;
    }

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphicIndex);
    snprintf(label, sizeof(label), "slot%d hand%d icon sheet asset", slot, hand);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             srcX + srcW <= (int)asset->width &&
                             srcY + srcH <= (int)asset->height);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src = (unsigned char)
                (asset->pixels[(srcY + yy) * (int)asset->width + srcX + xx] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++total;
            if (dst == src) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d hand%d GRAPHICS.DAT icon inset match",
             slot, hand);
    ok &= expect_true(label, total == 256 && matched == total);
    printf("slot%d hand%d icon=%d gfx=%d pixels=%d/%d\n",
           slot, hand, iconIndex, graphicIndex, matched, total);
    return ok;
}

static int check_champion_icon_pixels(const M11_GameViewState* game,
                                      const unsigned char* fb,
                                      int slot) {
    int x, y, w, h;
    int iconIndex;
    int gfxId;
    int iconStripCellW;
    int expected = 0;
    int matched = 0;
    int transparentMatch = 0;
    int transparentPixels = 0;
    const M11_AssetSlot* asset;
    int baseColor = M11_GameView_GetV1ChampionBarColor(slot);
    int xx;
    int yy;
    const int transparentColor = 12; /* M11_COLOR_DARK_GRAY */
    char label[128];
    int ok = 1;
    snprintf(label, sizeof(label), "slot%d champion icon source", slot);
    iconIndex = M11_GameView_GetV1ChampionIconSourceIndex(game, slot);
    ok &= expect_true(label, iconIndex >= 0);
    snprintf(label, sizeof(label), "slot%d champion icon zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
                              w == 19 && h == 14);
    gfxId = M11_GameView_GetV1ChampionIconGraphicId();
    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)gfxId);
    snprintf(label, sizeof(label), "slot%d champion icon strip asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->height >= 14 && asset->width >= 76 &&
                             asset->width % 4 == 0 &&
                             asset->height == (unsigned short)h);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    iconStripCellW = (int)asset->width / 4;

    /* ReDMCSB: CHAMDRAW.C F0622 lines 41-58 fills a 19x14 temporary
     * bitmap with the champion base color, then blends the C028
     * GRAPHIC_CHAMPION_ICONS strip over it with C12 dark-gray
     * transparency.  Reproduce that composite rule across the full
     * C113..C116 cell so the right-edge columns cannot retain stale
     * pixels from a prior HUD frame. */
    for (yy = 0; yy < h; ++yy) {
        int srcX0 = iconIndex * iconStripCellW;
        for (xx = 0; xx < w; ++xx) {
            int srcX = srcX0 + xx;
            unsigned char src = (unsigned char)(asset->pixels[yy * (int)asset->width + srcX] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            if (src == transparentColor) {
                ++transparentPixels;
                if ((int)dst == baseColor) {
                    ++transparentMatch;
                }
            } else {
                ++expected;
                if ((int)dst == (int)src) {
                    ++matched;
                }
            }
        }
    }
    snprintf(label, sizeof(label), "slot%d champion icon opaque pixel match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 95);
    snprintf(label, sizeof(label), "slot%d champion icon no stale pixels", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, x, y, w, h,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);
    if (transparentPixels > 0) {
        snprintf(label, sizeof(label), "slot%d champion icon transparent fill match", slot);
        ok &= expect_true(label,
                          transparentMatch * 100 >= transparentPixels * 95);
    } else {
        snprintf(label, sizeof(label), "slot%d champion icon has transparent pixels", slot);
        ok &= expect_true(label, 0);
    }
    printf("slot%d icon src=%d gfx=%d opaque=%d/%d transparent=%d/%d base=%d\n",
           slot, iconIndex, gfxId, matched, expected,
           transparentMatch, transparentPixels, baseColor);
    return ok;
}

/* ReDMCSB: MENUS.C F0386 draws action-hand cells C089..C092 for live
 * champions, fills the action icon bitmap with C04 cyan for an empty hand,
 * then blits the empty-hand object icon through the action-area palette
 * remap.  This is a Firestaff runtime pixel gate for the champion panel's
 * action icon row; it does not claim original DOS screenshot parity. */
static int check_action_icon_cell_pixels(const M11_GameViewState* game,
                                         const unsigned char* fb,
                                         int slot) {
    int cellX, cellY, cellW, cellH;
    int innerX, innerY, innerW, innerH;
    int graphicIndex;
    int srcX, srcY, srcW, srcH;
    const M11_AssetSlot* asset;
    int matched = 0;
    int total = 0;
    int cyanPixels;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d action icon cell id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1ActionIconCellZoneId(slot),
                     89 + slot);
    snprintf(label, sizeof(label), "slot%d action icon inner id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1ActionIconInnerZoneId(slot),
                     93 + slot);
    snprintf(label, sizeof(label), "slot%d action icon cell zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1ActionIconCellZone(slot,
                                                           &cellX,
                                                           &cellY,
                                                           &cellW,
                                                           &cellH) &&
                      cellW == 20 && cellH == 35);
    snprintf(label, sizeof(label), "slot%d action icon inner zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1ActionIconInnerZone(slot,
                                                            &innerX,
                                                            &innerY,
                                                            &innerW,
                                                            &innerH) &&
                      innerW == 16 && innerH == 16);
    snprintf(label, sizeof(label), "slot%d action icon backdrop color", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1ActionIconCellBackdropColor(game, slot),
                     PROBE_ACTION_CELL_CYAN);
    snprintf(label, sizeof(label), "slot%d action empty-hand source zone", slot);
    ok &= expect_true(label,
                      M11_GameView_GetV1ObjectIconSourceZone(
                          PROBE_ACTION_EMPTY_HAND_ICON,
                          &graphicIndex,
                          &srcX,
                          &srcY,
                          &srcW,
                          &srcH) &&
                      srcW == 16 && srcH == 16);
    if (!ok) {
        return 0;
    }

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphicIndex);
    snprintf(label, sizeof(label), "slot%d action empty-hand icon asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             srcX + srcW <= (int)asset->width &&
                             srcY + srcH <= (int)asset->height);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    cyanPixels = count_color(fb, PROBE_FB_W,
                             cellX, cellY, cellW, cellH,
                             PROBE_ACTION_CELL_CYAN);
    snprintf(label, sizeof(label), "slot%d action icon cyan backdrop visible",
             slot);
    ok &= expect_true(label, cyanPixels > 350);

    for (yy = 0; yy < innerH; ++yy) {
        int xx;
        for (xx = 0; xx < innerW; ++xx) {
            unsigned char src = (unsigned char)
                (asset->pixels[(srcY + yy) * (int)asset->width + srcX + xx] & 0x0F);
            unsigned char expected = (unsigned char)
                M11_GameView_MapV1ActionIconPaletteColor(src, 1);
            unsigned char dst = px_index(fb, PROBE_FB_W, innerX + xx, innerY + yy);
            ++total;
            if (dst == expected) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label),
             "slot%d action empty-hand GRAPHICS.DAT icon match", slot);
    ok &= expect_true(label, total == 256 && matched == total);
    printf("slot%d action cell C%d inner C%d icon=%d gfx=%d cyan=%d pixels=%d/%d\n",
           slot,
           M11_GameView_GetV1ActionIconCellZoneId(slot),
           M11_GameView_GetV1ActionIconInnerZoneId(slot),
           PROBE_ACTION_EMPTY_HAND_ICON,
           graphicIndex,
           cyanPixels,
           matched,
           total);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int slot;
    int stat;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr, "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game);
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_status_box_pixels(&game, fb, slot);
        ok &= check_champion_icon_pixels(&game, fb, slot);
        for (stat = 0; stat < 3; ++stat) {
            ok &= check_bar_pixels(&game, fb, slot, stat);
        }
        ok &= check_hand_slot_asset_pixels(&game, fb, slot, 0);
        ok &= check_hand_slot_asset_pixels(&game, fb, slot, 1);
        ok &= check_hand_slot_icon_pixels(&game, fb, slot, 0);
        ok &= check_hand_slot_icon_pixels(&game, fb, slot, 1);
    }
    ok &= check_status_box_gutter_pixels(fb);

    /* ReDMCSB: MENUS.C F0387 draws either the acting champion action
     * menu or the four C089..C092 action icon cells.  The first pass keeps
     * actingChampionOrdinal=2 to cover C035 in the status hand slot; this
     * second pass clears the acting champion so the idle icon-mode branch
     * is pixel-checked with the same real assets and seeded party. */
    game.actingChampionOrdinal = 0;
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_action_icon_cell_pixels(&game, fb, slot);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel runtime pixel probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
