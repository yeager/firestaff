/*
 * DM1 V1 champion panel status-state runtime pixel probe.
 *
 * This is Firestaff-side evidence only.  It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs deterministic four
 * champion parties that drive every status-box secondary state the
 * existing champion-panel pixel probe intentionally does not cover, and
 * pixel-checks the resulting on-screen blits through the real M11 draw
 * stack.  It does not claim original DOS screenshot parity.
 *
 * The existing firestaff_dm1_v1_champion_panel_pixels_runtime_probe
 * covers the four alive status boxes (C151..C154), the per-champion
 * 4x25 bar graphs (C195..C206), the C033/C034/C035 hand slot boxes,
 * and the C113..C116 19x14 champion icons.  This new probe extends
 * that evidence with the secondary status-box states the source
 * CHAMDRAW.C F0292 path renders conditionally:
 *
 *   1. C008 dead status box            — slot with HP=0
 *   2. C032 POISONED label             — 96x15, GRAPHICS.DAT
 *   3. C037 / C038 / C039 shield stack — fire, spell, party shields
 *   4. C015 damage indicator           — 45x7 over status box
 *   5. C211/C213/C215/C217 status
 *      hand icon                       — 16x16 inside the 18x18 box
 *   6. Name text glyph pixels          — drawn through m11_draw_text
 *   7. Dead slot bar columns           — C008 pixels are not overdrawn
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 draws C008 for dead champions and
 *   the C037/C038/C039 shield border stack in fire/spell/party
 *   reverse order over alive champion status boxes.
 *   ReDMCSB INVNTORY.C draws the C032 POISONED label below the
 *   status box when champion.poisonDose > 0.
 *   ReDMCSB CHAMPION.C F0291 schedules the C015 damage indicator
 *   (zones C167..C170) over the status box while
 *   championDamageTimer > 0.
 *   ReDMCSB CHAMDRAW.C F0291 draws the 16x16 hand-slot object icon
 *   at +1,+1 inside the 18x18 C033/C034/C035 hand slot box.
 *   ReDMCSB CHAMDRAW.C F0292 lines 816-842 exits the dead champion
 *   route before F0287 bar graphs and F0291 hand slots can overdraw C008.
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
    PROBE_CHAMPION_COUNT = 4
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
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
                          unsigned short wounds,
                          int poisonDose) {
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
    champ->poisonDose = (unsigned short)poisonDose;
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
    game->actingChampionOrdinal = 2; /* slot 1 action hand uses C035 */
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    game->championDamageTimer[0] = 0;
    game->championDamageTimer[1] = 0;
    game->championDamageTimer[2] = 0;
    game->championDamageTimer[3] = 0;
    /* Slot 0: dead  HP=0, no bars, no poison, no damage, no shield.
     * Slot 1: alive, poisoned, no shield.
     * Slot 2: alive, all three party shields active.
     * Slot 3: alive, damage timer > 0, no shield. */
    seed_champion(&game->world.party.champions[0],
                  "GHOST", 0, DIR_NORTH, 0, 100, 0, 80, 0, 60, 0, 0);
    seed_champion(&game->world.party.champions[1],
                  "VENOM", 1, DIR_EAST, 60, 100, 50, 80, 30, 60, 0, 7);
    seed_champion(&game->world.party.champions[2],
                  "AEGIS", 2, DIR_SOUTH, 80, 100, 60, 80, 50, 60, 0, 0);
    seed_champion(&game->world.party.champions[3],
                  "HURT", 3, DIR_WEST, 35, 100, 25, 80, 10, 60, 0, 0);
    game->world.magic.fireShieldDefense = 4;  /* slot 2 -> C038 fire */
    game->world.magic.spellShieldDefense = 4; /* slot 2 -> C039 spell */
    game->world.magic.partyShieldDefense = 4; /* slot 2 -> C037 party */
    game->championDamageTimer[3] = 8;         /* slot 3 -> C015 damage */
}

/* Verify that graphic 8 (C008 dead status box) is blitted to slot 0
 * when champion HP=0.  Compares the on-screen 67x29 zone against the
 * raw asset pixel buffer.  Uses a perimeter-only comparison so the
 * name text drawn on top does not invalidate the match. */
static int check_dead_status_box(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int slot) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1StatusBoxBaseGraphic(game, slot);
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d dead status box base graphic", slot);
    ok &= expect_true(label, gfx == M11_GameView_GetV1DeadStatusBoxGraphicId());
    snprintf(label, sizeof(label), "slot%d status box zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                             w == 67 && h == 29);
    snprintf(label, sizeof(label), "slot%d dead status box asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 67 && asset->height == 29);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            int onPerimeter =
                (xx == 0) || (yy == 0) || (xx == w - 1) || (yy == h - 1);
            if (!onPerimeter) continue;
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) continue;
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) ++matched;
        }
    }
    snprintf(label, sizeof(label), "slot%d C008 dead box perimeter match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 80);
    printf("slot%d dead box gfx=%d perimeter=%d/%d\n", slot, gfx, matched, expected);
    return ok;
}

/* ReDMCSB: CHAMDRAW.C F0292 lines 816-842 blits C008 for a dead
 * champion and then jumps out of the alive status route before
 * F0287_CHAMPION_DrawBarGraphs (lines 72-156) can redraw the three
 * 4x25 stat columns.  Verify the dead slot's HP/stamina/mana bar
 * zones still contain C008 asset pixels after the full M11 draw. */
static int check_dead_status_bar_columns_unmodified(const M11_GameViewState* game,
                                                    const unsigned char* fb,
                                                    int slot) {
    int boxX, boxY, boxW, boxH;
    int gfx = M11_GameView_GetV1DeadStatusBoxGraphicId();
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int ok = 1;
    int stat;
    char label[128];

    snprintf(label, sizeof(label), "slot%d dead bar c008 status zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusBoxZone(
                                  slot, &boxX, &boxY, &boxW, &boxH) &&
                             boxW == 67 && boxH == 29);
    snprintf(label, sizeof(label), "slot%d dead bar c008 asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 67 && asset->height == 29);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (stat = 0; stat < 3; ++stat) {
        int barX, barY, barW, barH;
        int yy;
        int matched = 0;
        int expected = 0;
        snprintf(label, sizeof(label), "slot%d dead stat%d bar c008 zone", slot, stat);
        ok &= expect_true(label, M11_GameView_GetV1StatusBarZone(
                                      slot, stat, &barX, &barY, &barW, &barH) &&
                                 barW == 4 && barH == 25);
        if (!ok) {
            return 0;
        }
        for (yy = 0; yy < barH; ++yy) {
            int xx;
            for (xx = 0; xx < barW; ++xx) {
                int srcX = (barX - boxX) + xx;
                int srcY = (barY - boxY) + yy;
                unsigned char src;
                unsigned char dst;
                if (srcX < 0 || srcX >= boxW || srcY < 0 || srcY >= boxH) {
                    continue;
                }
                src = (unsigned char)(asset->pixels[srcY * (int)asset->width + srcX] & 0x0F);
                dst = px_index(fb, PROBE_FB_W, barX + xx, barY + yy);
                ++expected;
                if (dst == src) ++matched;
            }
        }
        snprintf(label, sizeof(label), "slot%d dead stat%d c008 column preserved",
                 slot, stat);
        ok &= expect_true(label, expected == barW * barH && matched == expected);
        printf("slot%d dead stat%d C008 column=%d/%d\n",
               slot, stat, matched, expected);
    }
    return ok;
}

/* Verify the C032 POISONED label is blitted for a poisoned champion.
 * The 96x15 graphic has at least 100 non-transparent pixels; the
 * match ratio is bounded by the source's actual opaque area, so a
 * conservative 70% threshold is appropriate. */
static int check_poison_label(const M11_GameViewState* game,
                              const unsigned char* fb,
                              int slot) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1PoisonLabelGraphicId();
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d poison label zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1PoisonLabelZone(
                                  slot, 96, 15, &x, &y, &w, &h) &&
                             w == 96 && h == 15);
    snprintf(label, sizeof(label), "slot%d poison label asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 96 && asset->height == 15);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }
    /* Some label sub-rectangles may be clipped by the framebuffer
     * when the label spans the right edge of the screen; clamp the
     * comparison rect to the framebuffer. */
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > PROBE_FB_W) w = PROBE_FB_W - x;
    if (y + h > PROBE_FB_H) h = PROBE_FB_H - y;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) continue;
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) ++matched;
        }
    }
    snprintf(label, sizeof(label), "slot%d C032 poison label match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 70);
    printf("slot%d poison label gfx=%d matched=%d/%d\n",
           slot, gfx, matched, expected);
    return ok;
}

/* Verify the C037/C038/C039 shield-border stack is drawn on top of
 * the alive status box for the slot that has all three party effects
 * active.  The F0292 draw order (topmost first) is party/spell/fire
 * reverse-appended.  Counts the number of distinct non-C12/non-C00
 * pixels in the shield zone that match the topmost expected graphic
 * to confirm at least one shield overlay is on screen. */
static int check_shield_border_stack(const M11_GameViewState* game,
                                     const unsigned char* fb,
                                     int slot) {
    int x, y, w, h;
    int count = M11_GameView_GetV1StatusShieldBorderGraphicCountForChampion(
        game, slot);
    int borderOrdinal;
    int matchedTotal = 0;
    int expectedTotal = 0;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d shield border count", slot);
    ok &= expect_true(label, count == 3);
    snprintf(label, sizeof(label), "slot%d shield border zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusShieldBorderZone(
                                  slot, &x, &y, &w, &h) &&
                             w == 67 && h == 29);
    if (!ok) {
        return 0;
    }
    /* The shield draw order is reverse-append: party, spell, fire.
     * Compare the on-screen pixels against the LAST graphic in the
     * visible draw stack (fire) to confirm at least the topmost
     * border is visible.  The full reverse-stack would need
     * overdraw-aware tracking beyond the scope of this probe. */
    for (borderOrdinal = 0; borderOrdinal < count; ++borderOrdinal) {
        int gfx = M11_GameView_GetV1StatusShieldBorderGraphicForChampionAt(
            game, slot, borderOrdinal);
        const M11_AssetSlot* asset = M11_AssetLoader_Load(
            (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
        int yy;
        int matched = 0;
        int expected = 0;
        if (!asset || !asset->loaded || !asset->pixels ||
            asset->width != (unsigned short)w || asset->height != (unsigned short)h) {
            continue;
        }
        for (yy = 0; yy < h; ++yy) {
            int xx;
            for (xx = 0; xx < w; ++xx) {
                unsigned char src;
                unsigned char dst;
                src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
                if (src == 0) continue;
                dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
                ++expected;
                if (dst == src) ++matched;
            }
        }
        printf("slot%d shield[%d] gfx=%d matched=%d/%d\n",
               slot, borderOrdinal, gfx, matched, expected);
        matchedTotal += matched;
        expectedTotal += expected;
    }
    snprintf(label, sizeof(label), "slot%d shield border stack visible", slot);
    ok &= expect_true(label, expectedTotal > 0 && matchedTotal * 100 >= expectedTotal * 30);
    return ok;
}

/* Verify the C015 damage indicator (45x7) is blitted over the alive
 * status box for the slot whose championDamageTimer > 0. */
static int check_damage_indicator(const M11_GameViewState* game,
                                  const unsigned char* fb,
                                  int slot) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1ChampionSmallDamageGraphicId();
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[128];

    snprintf(label, sizeof(label), "slot%d damage indicator zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1DamageIndicatorZone(
                                  slot, 45, 7, &x, &y, &w, &h) &&
                             w == 45 && h == 7);
    snprintf(label, sizeof(label), "slot%d damage indicator asset", slot);
    ok &= expect_true(label, asset && asset->loaded && asset->pixels &&
                             asset->width == 45 && asset->height == 7);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + w > PROBE_FB_W) w = PROBE_FB_W - x;
    if (y + h > PROBE_FB_H) h = PROBE_FB_H - y;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            if (src == 0) continue;
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) ++matched;
        }
    }
    snprintf(label, sizeof(label), "slot%d C015 damage indicator match", slot);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 70);
    printf("slot%d damage gfx=%d matched=%d/%d\n",
           slot, gfx, matched, expected);
    return ok;
}

/* Verify the 16x16 status hand icon zone (C211..C218) sits inside
 * the 18x18 hand slot box and renders some non-background pixel for
 * an alive champion.  Uses a coarse "any non-zero color" check so
 * the probe does not depend on a specific THING icon palette. */
static int check_status_hand_icon_zone(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int slot) {
    (void)game;
    int handIdx;
    int ok = 1;
    for (handIdx = 0; handIdx < 2; ++handIdx) {
        int boxX, boxY, boxW, boxH;
        int iconX, iconY, iconW, iconH;
        int nonBlack = 0;
        int yy;
        char label[128];
        snprintf(label, sizeof(label), "slot%d hand%d slot box zone", slot, handIdx);
        ok &= expect_true(label, M11_GameView_GetV1StatusHandSlotBoxZone(
                                      slot, handIdx, &boxX, &boxY, &boxW, &boxH) &&
                                 boxW == 18 && boxH == 18);
        snprintf(label, sizeof(label), "slot%d hand%d icon zone inside box", slot, handIdx);
        ok &= expect_true(label, M11_GameView_GetV1StatusHandIconZone(
                                      slot, handIdx, &iconX, &iconY, &iconW, &iconH) &&
                                 iconX == boxX + 1 && iconY == boxY + 1 &&
                                 iconW == 16 && iconH == 16);
        if (!ok) {
            return 0;
        }
        if (iconX < 0) iconX = 0;
        if (iconY < 0) iconY = 0;
        if (iconX + iconW > PROBE_FB_W) iconW = PROBE_FB_W - iconX;
        if (iconY + iconH > PROBE_FB_H) iconH = PROBE_FB_H - iconY;
        for (yy = 0; yy < iconH; ++yy) {
            int xx;
            for (xx = 0; xx < iconW; ++xx) {
                if ((int)px_index(fb, PROBE_FB_W, iconX + xx, iconY + yy) != 0) {
                    ++nonBlack;
                }
            }
        }
        snprintf(label, sizeof(label), "slot%d hand%d icon zone has content", slot, handIdx);
        ok &= expect_true(label, nonBlack > 0);
    }
    return ok;
}

/* Verify the champion name text was actually drawn in the name clear
 * zone, not just that the right palette index shows up.  Detects
 * glyph pixels by counting non-clear-color non-zero pixels in the
 * text sub-rectangle. */
static int check_name_text_glyphs(const M11_GameViewState* game,
                                   const unsigned char* fb,
                                   int slot) {
    int clearX, clearY, clearW, clearH;
    int textX, textY, textW, textH;
    int nameColor = M11_GameView_GetV1StatusNameColor(game, slot);
    int clearColor = M11_GameView_GetV1StatusNameClearColor();
    int nameGlyphPixels = 0;
    int textBgPixels = 0;
    int yy;
    int ok = 1;
    char label[128];

    (void)clearColor;
    snprintf(label, sizeof(label), "slot%d name text zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusNameZone(
                                  slot, &clearX, &clearY, &clearW, &clearH) &&
                             clearW == 43 && clearH == 7);
    snprintf(label, sizeof(label), "slot%d name text sub-zone", slot);
    ok &= expect_true(label, M11_GameView_GetV1StatusNameTextZone(
                                  slot, &textX, &textY, &textW, &textH) &&
                             textW > 0 && textH > 0);
    if (!ok) {
        return 0;
    }
    if (textX < 0) textX = 0;
    if (textY < 0) textY = 0;
    if (textX + textW > PROBE_FB_W) textW = PROBE_FB_W - textX;
    if (textY + textH > PROBE_FB_H) textH = PROBE_FB_H - textY;
    for (yy = 0; yy < textH; ++yy) {
        int xx;
        for (xx = 0; xx < textW; ++xx) {
            int idx = (int)px_index(fb, PROBE_FB_W, textX + xx, textY + yy);
            if (idx == nameColor) ++nameGlyphPixels;
            else if (idx != 0) ++textBgPixels;
        }
    }
    /* Glyph pixels depend on the small font; alive and dead champions
     * in V1 mode both have their name drawn through the
     * m11_draw_text_centered_in_rect path.  Threshold is loose so
     * narrow-glyph names like "ALEX" still pass. */
    snprintf(label, sizeof(label), "slot%d name glyph pixels drawn", slot);
    ok &= expect_true(label, nameGlyphPixels >= 5);
    printf("slot%d name color=%d glyph=%d bg=%d\n",
           slot, nameColor, nameGlyphPixels, textBgPixels);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
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
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    /* Slot 0: dead — must use C008 dead status box. */
    ok &= check_dead_status_box(&game, fb, 0);
    /* Slot 0: dead — C008 bar columns must not be overdrawn by F0287. */
    ok &= check_dead_status_bar_columns_unmodified(&game, fb, 0);
    /* Slot 1: alive poisoned — must blit C032 POISONED label. */
    ok &= check_poison_label(&game, fb, 1);
    /* Slot 2: alive with full shield stack — must show shield border(s). */
    ok &= check_shield_border_stack(&game, fb, 2);
    /* Slot 3: alive with damage timer — must blit C015 damage indicator. */
    ok &= check_damage_indicator(&game, fb, 3);
    /* Slots 1..3 (alive): hand-icon zone inside 18x18 box. */
    ok &= check_status_hand_icon_zone(&game, fb, 1);
    ok &= check_status_hand_icon_zone(&game, fb, 2);
    ok &= check_status_hand_icon_zone(&game, fb, 3);
    /* Name text glyph pixels: every champion's name is drawn through
     * the V1 m11_draw_text_centered_in_rect path, including the dead
     * slot 0 (CHAMDRAW.C F0292:816-842).  Verify the name actually
     * reaches the framebuffer for all four slots. */
    ok &= check_name_text_glyphs(&game, fb, 0);
    ok &= check_name_text_glyphs(&game, fb, 1);
    ok &= check_name_text_glyphs(&game, fb, 2);
    ok &= check_name_text_glyphs(&game, fb, 3);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel status-states runtime pixel probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
