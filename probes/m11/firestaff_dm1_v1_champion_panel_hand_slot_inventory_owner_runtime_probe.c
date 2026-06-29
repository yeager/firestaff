/*
 * DM1 V1 champion-panel hand-slot inventory-owner runtime probe.
 *
 * Firestaff-side runtime evidence for the live M11 frame path behind the
 * source-locked CHAMDRAW.C F0296 hand-slot owner gate.  The data-free
 * `dm1_v1_champion_panel_hand_slot_refresh_pc34_compat` CTest pins the
 * F0296 rule: when G0423_i_InventoryChampionOrdinal names the inventory
 * owner, that owner's status action-hand slotbox is skipped by the changed
 * icon walk while the other champions keep walking.  This probe does not
 * re-model F0296; it opens hash-verified DM1 V1 assets, opens an inventory
 * by right-clicking the owner's source C151..C154 status box, renders the
 * real M11 V1 frame, and verifies:
 *
 *   1. the top status-panel action-hand slotboxes remain byte-stable while
 *      the inventory viewport overlay is open;
 *   2. the inventory owner's source C508 action-hand slotbox is drawn from
 *      the GRAPHICS.DAT C033 normal slot-box asset inside the viewport
 *      inventory panel.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0296:1184-1262 changed-object-icon walk;
 *   F0296:1217-1219 inventory-champion ordinal owner skip;
 *   CHAMDRAW.C F0292/F0291 draws C151..C154 status boxes and C211..C218
 *   ready/action hand slotboxes; INVNTORY.C/PANEL.C draw inventory source
 *   slot boxes C507..C536 including C508 for C01_SLOT_ACTION_HAND; DEFS.H
 *   C033/C151..C154/C507..C536/G0423/M000_INDEX_TO_ORDINAL.
 *
 * This is Firestaff runtime + real-asset pixel evidence only.  It makes no
 * original DOS screenshot parity claim.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_champion_state_pc34_compat.h"
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
    PROBE_OWNER_SLOT = 2,
    PROBE_DM1_VIEWPORT_X = 0,
    PROBE_DM1_VIEWPORT_Y = 33
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y)
{
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int expect_true(const char* label, int ok)
{
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static unsigned long hash_rect(const unsigned char* fb,
                               int x,
                               int y,
                               int w,
                               int h)
{
    unsigned long hash = 2166136261u;
    int yy;

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            hash ^= (unsigned long)px_index(fb, PROBE_FB_W, x + xx, y + yy);
            hash *= 16777619u;
        }
    }
    return hash;
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
                          int manaMax)
{
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
    champ->wounds = 0;
    champ->poisonDose = 0;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game)
{
    int slot;

    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = PROBE_OWNER_SLOT;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->inventorySelectedSlot = -1;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorPartyIndex = -1;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    game->world.magic.event71CountInvisibility = 0;
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        game->championDamageTimer[slot] = 0;
        game->championDamageAmount[slot] = 0;
    }

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 100, 80, 80, 60, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 90, 100, 70, 80, 50, 60);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 75, 100, 55, 80, 35, 60);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 65, 100, 45, 80, 25, 60);
}

static int check_asset_perimeter(const M11_GameViewState* game,
                                 const unsigned char* fb,
                                 int x,
                                 int y,
                                 int expectedGfx,
                                 const char* labelPrefix)
{
    const M11_AssetSlot* asset;
    int matched = 0;
    int expected = 0;
    int yy;
    int ok = 1;
    char label[160];

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)expectedGfx);
    snprintf(label, sizeof(label), "%s asset", labelPrefix);
    ok &= expect_true(label,
                      asset && asset->loaded && asset->pixels &&
                      asset->width == 18 && asset->height == 18);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < 18; ++yy) {
        int xx;
        for (xx = 0; xx < 18; ++xx) {
            unsigned char src;
            unsigned char dst;

            if (xx > 0 && xx < 17 && yy > 0 && yy < 17) {
                continue;
            }
            src = (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0f);
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

    snprintf(label, sizeof(label), "%s perimeter exact pixels", labelPrefix);
    ok &= expect_true(label, expected > 0 && matched == expected);
    printf("%s perimeter=%d/%d gfx=%d\n",
           labelPrefix, matched, expected, expectedGfx);
    return ok;
}

static int collect_action_hand_hashes(const unsigned char* fb,
                                      unsigned long outHashes[PROBE_CHAMPION_COUNT])
{
    int slot;
    int ok = 1;

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        char label[128];

        snprintf(label, sizeof(label), "slot%d action-hand zone", slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusHandSlotBoxZone(
                              slot, 1, &x, &y, &w, &h) &&
                          w == 18 && h == 18);
        if (!ok) {
            return 0;
        }
        outHashes[slot] = hash_rect(fb, x, y, w, h);
        printf("slot%d action-hand hash=0x%08lx\n", slot, outHashes[slot]);
    }
    return 1;
}

int main(int argc, char** argv)
{
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char baselineFb[PROBE_FB_W * PROBE_FB_H];
    unsigned char inventoryFb[PROBE_FB_W * PROBE_FB_H];
    unsigned long baselineHashes[PROBE_CHAMPION_COUNT];
    unsigned long inventoryHashes[PROBE_CHAMPION_COUNT];
    int ownerStatusX = 0;
    int ownerStatusY = 0;
    int ownerStatusW = 0;
    int ownerStatusH = 0;
    int sourceSlotBox;
    int sourceX = 0;
    int sourceY = 0;
    int sourceW = 0;
    int sourceH = 0;
    int slot;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_panel_hand_slot_inventory_owner_runtime "
               "no hash-verified DM1 data under %s\n",
               dataDir);
        return 0;
    }

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
    memset(baselineFb, 0, sizeof(baselineFb));
    M11_GameView_Draw(&game, baselineFb, PROBE_FB_W, PROBE_FB_H);
    ok &= collect_action_hand_hashes(baselineFb, baselineHashes);

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x = 0;
        int y = 0;
        int w = 0;
        int h = 0;
        int gfx = M11_GameView_GetV1StatusHandSlotGraphic(&game, slot, 1);
        char label[160];

        (void)M11_GameView_GetV1StatusHandSlotBoxZone(slot, 1, &x, &y, &w, &h);
        snprintf(label, sizeof(label), "baseline slot%d action-hand C033", slot);
        ok &= expect_int(label, gfx, M11_GameView_GetV1SlotBoxNormalGraphicId());
        ok &= check_asset_perimeter(&game, baselineFb, x, y, gfx, label);
    }

    ok &= expect_true("owner status box zone",
                      M11_GameView_GetV1StatusBoxZone(PROBE_OWNER_SLOT,
                                                      &ownerStatusX,
                                                      &ownerStatusY,
                                                      &ownerStatusW,
                                                      &ownerStatusH));
    if (!ok) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    ok &= expect_int("right-click owner status opens inventory",
                     (int)M11_GameView_HandlePointerButton(
                         &game,
                         ownerStatusX + ownerStatusW / 2,
                         ownerStatusY + ownerStatusH / 2,
                         M11_DM1_MOUSE_MASK_RIGHT),
                     M11_GAME_INPUT_REDRAW);
    ok &= expect_int("inventory panel active", game.inventoryPanelActive, 1);
    ok &= expect_int("inventory owner active champion",
                     game.world.party.activeChampionIndex,
                     PROBE_OWNER_SLOT);
    ok &= expect_int("inventory selected slot reset", game.inventorySelectedSlot, 0);

    memset(inventoryFb, 0, sizeof(inventoryFb));
    M11_GameView_Draw(&game, inventoryFb, PROBE_FB_W, PROBE_FB_H);
    ok &= collect_action_hand_hashes(inventoryFb, inventoryHashes);

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        char label[128];

        snprintf(label, sizeof(label),
                 "slot%d top action-hand stable while inventory owns viewport",
                 slot);
        ok &= expect_true(label, baselineHashes[slot] == inventoryHashes[slot]);
    }

    sourceSlotBox = M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(
        CHAMPION_SLOT_HAND_RIGHT);
    ok &= expect_int("source action-hand slotbox index", sourceSlotBox, 9);
    ok &= expect_true("source C508 inventory action-hand zone",
                      M11_GameView_GetV1InventorySourceSlotBoxZone(
                          sourceSlotBox, &sourceX, &sourceY, &sourceW, &sourceH) &&
                      sourceW == 16 && sourceH == 16);
    if (ok) {
        ok &= check_asset_perimeter(
            &game,
            inventoryFb,
            PROBE_DM1_VIEWPORT_X + sourceX - 1,
            PROBE_DM1_VIEWPORT_Y + sourceY - 1,
            M11_GameView_GetV1SlotBoxNormalGraphicId(),
            "inventory owner C508 action-hand slotbox");
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion-panel hand-slot inventory-owner runtime "
           "probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
