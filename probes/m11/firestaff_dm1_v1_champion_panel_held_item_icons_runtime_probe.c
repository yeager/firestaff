/*
 * DM1 V1 champion panel held-item icon runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, seeds deterministic non-empty
 * ready/action hands, renders one V1 frame through the real M11 draw stack,
 * and exact-pixel matches the party HUD 16x16 hand-icon insets against the
 * GRAPHICS.DAT object-icon cells. It does not claim original DOS screenshot
 * parity.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0291 draws the C033/C034/C035 status hand slot
 *   box, then draws the selected object icon one pixel inside the 18x18
 *   box through F0038_OBJECT_DrawIconInSlotBox.
 *   ReDMCSB OBJECT.C F0033 resolves object info records to icon indices.
 *   ReDMCSB DEFS.H defines C00_SLOT_READY_HAND and C01_SLOT_ACTION_HAND
 *   as the status-panel hand routes used by F0291.
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
    PROBE_CHAMPION_COUNT = 2
};

typedef struct {
    int slot;
    int hand;
    unsigned short thing;
    int expectedIcon;
    const char* label;
} HeldItemIconCase;

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static unsigned short thing_ref(int thingType, int thingIndex) {
    return (unsigned short)(((thingType & 0x0F) << 10) | (thingIndex & 0x03FF));
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

static int seed_item_records(M11_GameViewState* game) {
    struct DungeonThings_Compat* things;
    int ok = 1;

    if (!game) return 0;
    things = game->world.things;
    ok &= expect_true("dungeon things loaded", things && things->loaded);
    ok &= expect_true("weapon record available",
                      things && things->weapons && things->weaponCount > 0);
    ok &= expect_true("container record available",
                      things && things->containers && things->containerCount > 0);
    ok &= expect_true("potion record available",
                      things && things->potions && things->potionCount > 0);
    ok &= expect_true("junk record available",
                      things && things->junks && things->junkCount > 0);
    if (!ok || !things) {
        return 0;
    }

    memset(&things->weapons[0], 0, sizeof(things->weapons[0]));
    things->weapons[0].next = THING_ENDOFLIST;
    things->weapons[0].type = 0;        /* object-info 23 -> icon 16 */

    memset(&things->containers[0], 0, sizeof(things->containers[0]));
    things->containers[0].next = THING_ENDOFLIST;
    things->containers[0].slot = THING_ENDOFLIST;
    things->containers[0].type = 0;     /* object-info 1 -> icon 144 */

    memset(&things->potions[0], 0, sizeof(things->potions[0]));
    things->potions[0].next = THING_ENDOFLIST;
    things->potions[0].type = 0;        /* object-info 2 -> icon 148 */

    memset(&things->junks[0], 0, sizeof(things->junks[0]));
    things->junks[0].next = THING_ENDOFLIST;
    things->junks[0].type = 1;          /* object-info 128 -> icon 8 */
    return 1;
}

static void seed_party(M11_GameViewState* game, HeldItemIconCase* cases) {
    int i;
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;

    seed_champion(&game->world.party.champions[0], "TIGGY", 0, DIR_NORTH);
    seed_champion(&game->world.party.champions[1], "HALK", 1, DIR_EAST);

    cases[0] = (HeldItemIconCase){0, 0, thing_ref(THING_TYPE_WEAPON, 0),
                                  16, "weapon ready hand"};
    cases[1] = (HeldItemIconCase){0, 1, thing_ref(THING_TYPE_CONTAINER, 0),
                                  144, "container action hand"};
    cases[2] = (HeldItemIconCase){1, 0, thing_ref(THING_TYPE_POTION, 0),
                                  148, "potion ready hand"};
    cases[3] = (HeldItemIconCase){1, 1, thing_ref(THING_TYPE_JUNK, 0),
                                  8, "junk action hand"};

    for (i = 0; i < 4; ++i) {
        struct ChampionState_Compat* champ =
            &game->world.party.champions[cases[i].slot];
        int sourceSlot = (cases[i].hand == 0)
                             ? CHAMPION_SLOT_HAND_LEFT
                             : CHAMPION_SLOT_ACTION_HAND;
        champ->inventory[sourceSlot] = cases[i].thing;
    }
}

static int check_icon_case(const M11_GameViewState* game,
                           const unsigned char* fb,
                           const HeldItemIconCase* tc) {
    int iconIndex;
    int graphicIndex;
    int srcX, srcY, srcW, srcH;
    int dstX, dstY, dstW, dstH;
    const M11_AssetSlot* asset;
    int matched = 0;
    int total = 0;
    int yy;
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "%s icon index", tc->label);
    iconIndex = M11_GameView_GetV1StatusHandIconIndex(game, tc->slot, tc->hand);
    ok &= expect_int(label, iconIndex, tc->expectedIcon);

    snprintf(label, sizeof(label), "%s source zone", tc->label);
    ok &= expect_true(label,
                      M11_GameView_GetV1ObjectIconSourceZone(
                          iconIndex, &graphicIndex, &srcX, &srcY, &srcW, &srcH) &&
                      srcW == 16 && srcH == 16);
    snprintf(label, sizeof(label), "%s screen zone", tc->label);
    ok &= expect_true(label,
                      M11_GameView_GetV1StatusHandIconZone(
                          tc->slot, tc->hand, &dstX, &dstY, &dstW, &dstH) &&
                      dstW == 16 && dstH == 16);
    if (!ok) {
        return 0;
    }

    asset = M11_AssetLoader_Load((M11_AssetLoader*)&game->assetLoader,
                                 (unsigned int)graphicIndex);
    snprintf(label, sizeof(label), "%s GRAPHICS.DAT icon sheet", tc->label);
    ok &= expect_true(label,
                      asset && asset->loaded && asset->pixels &&
                      srcX + srcW <= (int)asset->width &&
                      srcY + srcH <= (int)asset->height);
    if (!ok || !asset || !asset->pixels) {
        return 0;
    }

    for (yy = 0; yy < dstH; ++yy) {
        int xx;
        for (xx = 0; xx < dstW; ++xx) {
            unsigned char src = (unsigned char)
                (asset->pixels[(srcY + yy) * (int)asset->width + srcX + xx] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, dstX + xx, dstY + yy);
            ++total;
            if (dst == src) {
                ++matched;
            }
        }
    }

    snprintf(label, sizeof(label), "%s on-screen icon pixels", tc->label);
    ok &= expect_true(label, total == 256 && matched == total);
    printf("%s icon=%d gfx=%d src=%d,%d matched=%d/%d\n",
           tc->label, iconIndex, graphicIndex, srcX, srcY, matched, total);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    HeldItemIconCase cases[4];
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int i;
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
    if (!seed_item_records(&game)) {
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game, cases);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    for (i = 0; i < 4; ++i) {
        ok &= check_icon_case(&game, fb, &cases[i]);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel held-item icon runtime pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
