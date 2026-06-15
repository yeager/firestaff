/*
 * DM1 V1 champion panel action-cell slot-box runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, seeds a deterministic four champion
 * party, clicks a right-column action icon cell through the real M11 pointer
 * route, renders the V1 HUD, and exact-matches the affected status-panel
 * action-hand slot-box perimeter against the GRAPHICS.DAT C035 asset. A second
 * click on the same action cell clears the acting champion and the same
 * perimeter must return to C033. It does not claim original DOS screenshot
 * parity.
 *
 * Source evidence:
 *   ReDMCSB MENU.C F0388/F0389 stores/clears the acting champion ordinal
 *   from the action icon cells C089..C092.
 *   ReDMCSB CHAMDRAW.C F0291 lines 632-651 draws the C033/C034/C035
 *   hand-slot box and lets C035 override the action hand of the acting
 *   champion only.
 *   ReDMCSB DEFS.H defines C00_SLOT_READY_HAND/C01_SLOT_ACTION_HAND and
 *   C033/C034/C035 as the status-panel hand routes used by F0291.
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
    PROBE_TARGET_SLOT = 2
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
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
                          int direction) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = 90;
    champ->hp.maximum = 100;
    champ->stamina.current = 70;
    champ->stamina.maximum = 80;
    champ->mana.current = 40;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    int slot;
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        game->championDamageTimer[slot] = 0;
        game->championDamageAmount[slot] = 0;
    }

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST);
}

static int check_slotbox_perimeter_against_asset(const M11_GameViewState* game,
                                                 const unsigned char* fb,
                                                 int championSlot,
                                                 int handIndex,
                                                 int expectedGfx,
                                                 const char* phaseLabel) {
    int gfx;
    const M11_AssetSlot* asset;
    int x, y, w, h;
    int expected = 0;
    int matched = 0;
    int yy;
    int ok = 1;
    char label[192];

    snprintf(label, sizeof(label), "%s slot%d hand%d selected graphic",
             phaseLabel, championSlot, handIndex);
    gfx = M11_GameView_GetV1StatusHandSlotGraphic(game, championSlot, handIndex);
    ok &= expect_int(label, gfx, expectedGfx);

    snprintf(label, sizeof(label), "%s slot%d hand%d box zone",
             phaseLabel, championSlot, handIndex);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusHandSlotBoxZone(
                          championSlot, handIndex, &x, &y, &w, &h) &&
                      w == 18 && h == 18);

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)expectedGfx);
    snprintf(label, sizeof(label), "%s slot%d hand%d GRAPHICS.DAT box",
             phaseLabel, championSlot, handIndex);
    ok &= expect_true(label,
                      asset && asset->loaded && asset->pixels &&
                      asset->width == 18 && asset->height == 18);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src;
            unsigned char dst;
            if (xx > 0 && xx < w - 1 && yy > 0 && yy < h - 1) {
                continue;
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

    snprintf(label, sizeof(label), "%s slot%d hand%d perimeter exact pixels",
             phaseLabel, championSlot, handIndex);
    ok &= expect_true(label, expected > 0 && matched == expected);
    printf("%s slot%d hand%d gfx=%d perimeter=%d/%d\n",
           phaseLabel, championSlot, handIndex, expectedGfx, matched, expected);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int cellX, cellY, cellW, cellH;
    int clickX, clickY;
    int slot;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_int("initial acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);
    ok &= expect_true("target action icon cell zone",
                      M11_GameView_GetV1ActionIconCellZone(
                          PROBE_TARGET_SLOT, &cellX, &cellY, &cellW, &cellH) &&
                      cellW > 0 && cellH > 0);
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }
    clickX = cellX + cellW / 2;
    clickY = cellY + cellH / 2;

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_slotbox_perimeter_against_asset(
            &game, fb, slot, 1,
            M11_GameView_GetV1SlotBoxNormalGraphicId(),
            "initial");
    }

    ok &= expect_int("click target action cell selects acting champion",
                     (int)M11_GameView_HandlePointerButton(
                         &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("selected acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game),
                     PROBE_TARGET_SLOT + 1);

    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int expected = (slot == PROBE_TARGET_SLOT)
                           ? M11_GameView_GetV1SlotBoxActingHandGraphicId()
                           : M11_GameView_GetV1SlotBoxNormalGraphicId();
        ok &= check_slotbox_perimeter_against_asset(
            &game, fb, slot, 1, expected, "selected");
    }

    ok &= expect_int("second click target action cell clears acting champion",
                     (int)M11_GameView_HandlePointerButton(
                         &game, clickX, clickY, M11_DM1_MOUSE_MASK_LEFT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("cleared acting ordinal",
                     (int)M11_GameView_GetActingChampionOrdinal(&game), 0);

    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_slotbox_perimeter_against_asset(
            &game, fb, slot, 1,
            M11_GameView_GetV1SlotBoxNormalGraphicId(),
            "cleared");
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel action-cell slot-box runtime probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
