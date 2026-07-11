/*
 * Source-lock gate for the M11 normal V1 inventory full backpack panel.
 *
 * ReDMCSB evidence:
 *   DEFS.H lines 778-817: C00..C37 source slot namespace
 *   DATA.C lines 1049-1087: 30 champion inventory masks + 8 chest masks
 *   CHAMPION.C F0302 lines 677-699: inventory slot-box click resolves
 *     source slot index, reads champion slot/chest slot, no-ops empty/empty,
 *     and gates leader-hand placement through AllowedSlots & SlotMasks
 *   CHAMPION.C F0302 lines 700-712: leader hand/slot swap and redraw
 *   PANEL.C F0347 lines 1651-1691: inventory panel redraw is driven by
 *     the active inventory champion action hand and panel content route
 *   CHEST.C F0333 lines 43-46 and CHAMDRAW.C F0291 lines 621-630:
 *     an open action-hand chest uses C145 instead of closed chest C144
 *   CHEST.C F0334 lines 112-133 and DUNGEON.C F0163 lines 1796-1837:
 *     closing or rewriting an open chest compacts non-empty visible slots
 *   PANEL.C F0352 lines 1250-1254: weapon descriptions expose Cursed,
 *     Poisoned, Broken and ChargeCount-derived state from WEAPON data
 *   PANEL.C F0351 lines 2026-2108: eye click with empty leader hand draws
 *     champion base skills plus all six current/max statistic families
 *   PANEL.C F0342 lines 1132-1133 and 1472: container eye dispatches to
 *     F0333 then redraws arrow/eye chrome
 *   PANEL.C F0342 lines 1140-1145: object-description panel blits C020
 *     into C101 and C029 into C504 before icon/text overdraw
 *   CHEST.C F0333 lines 43-48 and 64-65: opened container state blits
 *     C025 open-chest panel, then C537..C544 slot boxes and object icons
 *   CHEST.C F0333 lines 58-75 and F0334 lines 112-133: only the first
 *     eight linked chest objects populate the visible chest-slot array,
 *     and close-time rewrite compacts that visible array back to the chest
 *   CHAMPION.C F0302 lines 688-710: occupied open-chest slot clicks swap
 *     the leader-hand object with the selected G0425_aT_ChestSlots entry
 *   CHAMPION.C F0301 lines 606-610 and CHEST.C F0334 lines 117-129:
 *     placing a leader-hand object into any empty C30+ chest slot writes
 *     that visible slot, then close promotes the first non-empty visible
 *     slot to the source container head
 *   CHAMPION.C F0302 lines 697-698 with DATA.C lines 1080-1087:
 *     objects whose AllowedSlots lack MASK0x0400_CONTAINER return before
 *     any chest-slot removal, leader-hand removal, relink, or redraw
 *   CHEST.C F0333 lines 58-75 and m11_process_v1_chest_slot_box_click:
 *     the open path is read-only on the source linked list, so the 9th
 *     and later tail items stay reachable through the 8th visible item's
 *     next pointer until either close or a last-slot pickup rewrites it
 *   CHEST.C F0333 lines 64-66 and DUNGEON.C lines 79-126:
 *     each visible chest slot advances to the next linked object and
 *     resolves that object's source icon from G0237_as_Graphic559_ObjectInfo
 *   CHEST.C F0333 lines 58-66, OBJECT.C F0033 lines 147-164, and
 *     DUNGEON.C F0141 lines 1145-1156/G0237 lines 106-135:
 *     C541/C542/C543/C544 prove the fifth/sixth/seventh/eighth linked visible
 *     objects advance through the source list and blit their own
 *     Slayer C052/Sling C053/Rock C054/Poison Dart C055 icons.
 *   PANEL.C F0349 lines 1788-1817: empty-hand mouth click redraws the
 *     food/water/poisoned panel; F0345 lines 1597-1615 blits C020/C030/
 *     C031/C032 into C101/C500/C501/C502 before drawing source bars
 *   COMMAND.C lines 498-507 G0456_as_Graphic561_MouseInput_PanelChest:
 *     open-chest C537..C544 zone routes map to source commands C058..C065
 *     with the original (Left,Right,Top,Bottom) hit boxes that produce
 *     the non-uniform layout-696 child zones in kV1ChestSlotBoxZones
 *   DUNGEON.C F0163 lines 1796-1837: chest close/rewrite relinks each
 *     THING through the generic Next field, so mixed weapon/potion/junk
 *     chains preserve visible slot order across type-specific storage.
 *   CHEST.C F0333 lines 58-75, F0334 lines 117-129, and DUNGEON.C
 *     F0163 lines 1796-1837: replacing visible C544 in an overfull chest
 *     rewrites only the eight visible slots and drops the hidden tail.
 */

#include "m11_game_view.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include "dm1_v1_skill_experience_pc34_compat.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_TRUE(expr, msg) do { \
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

static void assert_world_hash_matches(const M11_GameViewState* state,
                                      const char* msg) {
    uint32_t expected = 0;
    ASSERT_EQ(F0891_ORCH_WorldHash_Compat(&state->world, &expected), 1,
              "world hash helper accepts inventory world");
    ASSERT_EQ(state->lastWorldHash, expected, msg);
    ASSERT_TRUE(state->lastWorldHash != 0xBADF00Du,
                "inventory click replaced stale world hash sentinel");
}

static const char* graphics_dat_path(void) {
    const char* env = getenv("FIRESTAFF_DM1_GRAPHICS_DAT");
    const char* home = getenv("HOME");
    static char homePath[1024];
    if (env && env[0] != '\0') return env;
    if (home && home[0] != '\0') {
        FILE* f;
        snprintf(homePath, sizeof(homePath), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
        f = fopen(homePath, "rb");
        if (f) {
            fclose(f);
            return homePath;
        }
    }
    return "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT";
}

static int point_in_rect(int x, int y, int rx, int ry, int rw, int rh) {
    return x >= rx && x < rx + rw && y >= ry && y < ry + rh;
}

static int point_is_in_object_description_panel_overdraw(int panelX,
                                                        int panelY,
                                                        int x,
                                                        int y) {
    int circleX = 0, circleY = 0, circleW = 0, circleH = 0;
    int iconX = 0, iconY = 0, iconW = 0, iconH = 0;

    if (M11_GameView_GetV1ObjectDescriptionCircleZone(&circleX, &circleY,
                                                       &circleW, &circleH) &&
        point_in_rect(x, y, circleX - panelX, circleY - panelY, circleW, circleH)) {
        return 1;
    }
    if (M11_GameView_GetV1ObjectDescriptionIconZone(&iconX, &iconY, &iconW, &iconH) &&
        point_in_rect(x, y, iconX - panelX, iconY - panelY, iconW, iconH)) {
        return 1;
    }
    /* F0339 draws C019/C018 into C503 after the panel contents. */
    if (point_in_rect(x, y, 11, 10, 16, 16)) return 1;
    /* Text is rendered after the source panel and circle blits. */
    if (point_in_rect(x, y, 48, 4, 92, 24)) return 1;
    if (point_in_rect(x, y, 24, 30, 116, 43)) return 1;
    return 0;
}

static int framebuffer_matches_object_description_source_pixels(
    const M11_GameViewState* state,
    const unsigned char* framebuffer) {
    const M11_AssetSlot* panel;
    const M11_AssetSlot* circle;
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int circleX = 0, circleY = 0, circleW = 0, circleH = 0;
    int iconX = 0, iconY = 0, iconW = 0, iconH = 0;
    int panelMatched = 0;
    int circleMatched = 0;
    int circleAssetW = 0;
    int circleAssetH = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetViewportRect(&viewportX, &viewportY, &viewportW, &viewportH) ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH) ||
        !M11_GameView_GetV1ObjectDescriptionCircleZone(&circleX, &circleY,
                                                       &circleW, &circleH) ||
        !M11_GameView_GetV1ObjectDescriptionIconZone(&iconX, &iconY, &iconW, &iconH)) {
        return 0;
    }
    (void)viewportW;
    (void)viewportH;

    panel = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                 (unsigned int)M11_GameView_GetV1ObjectDescriptionPanelGraphicId());
    circle = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                  (unsigned int)M11_GameView_GetV1ObjectDescriptionCircleGraphicId());
    if (!panel || !panel->pixels || !circle || !circle->pixels ||
        panel->width != (unsigned short)panelW || panel->height != (unsigned short)panelH) {
        return 0;
    }
    circleAssetW = (int)circle->width;
    circleAssetH = (int)circle->height;
    if (circleAssetW <= 0 || circleAssetH <= 0 ||
        circleAssetW > circleW || circleAssetH > circleH) {
        return 0;
    }

    for (y = 0; y < panelH; ++y) {
        for (x = 0; x < panelW; ++x) {
            unsigned char want = panel->pixels[y * (int)panel->width + x];
            unsigned char got;
            if (want == 8 || point_is_in_object_description_panel_overdraw(panelX, panelY, x, y)) {
                continue;
            }
            got = framebuffer[(viewportY + panelY + y) * 320 + (viewportX + panelX + x)];
            if (got != want) {
                fprintf(stderr, "panel mismatch local=%d,%d screen=%d,%d want=%u got=%u matched=%d\n",
                        x, y, viewportX + panelX + x, viewportY + panelY + y,
                        (unsigned)want, (unsigned)got, panelMatched);
                return 0;
            }
            panelMatched++;
        }
    }

    for (y = 0; y < circleAssetH; ++y) {
        for (x = 0; x < circleAssetW; ++x) {
            unsigned char want = circle->pixels[y * (int)circle->width + x];
            unsigned char got;
            if (want == 12 || point_in_rect(circleX + x, circleY + y,
                                            iconX, iconY, iconW, iconH)) {
                continue;
            }
            got = framebuffer[(viewportY + circleY + y) * 320 + (viewportX + circleX + x)];
            if (got != want) {
                fprintf(stderr, "circle mismatch local=%d,%d screen=%d,%d want=%u got=%u matched=%d\n",
                        x, y, viewportX + circleX + x, viewportY + circleY + y,
                        (unsigned)want, (unsigned)got, circleMatched);
                return 0;
            }
            circleMatched++;
        }
    }

    return panelMatched > 1000 && circleMatched > 50;
}

static int point_is_in_chest_slot_frame(int panelX, int panelY, int x, int y) {
    int chestOrdinal;
    for (chestOrdinal = 0; chestOrdinal < M11_GameView_GetV1ChestSlotBoxZoneCount(); ++chestOrdinal) {
        int sx = 0, sy = 0, sw = 0, sh = 0;
        if (!M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal, &sx, &sy, &sw, &sh)) {
            continue;
        }
        /* F0038 draws the 18x18 slot-box around the 16x16 C537..C544
         * child icon zone, so skip those pixels when validating C025. */
        sx -= panelX + 1;
        sy -= panelY + 1;
        sw += 2;
        sh += 2;
        if (x >= sx && x < sx + sw && y >= sy && y < sy + sh) {
            return 1;
        }
    }
    return 0;
}

static int framebuffer_matches_open_chest_panel_pixels(const M11_GameViewState* state,
                                                       const unsigned char* framebuffer) {
    const M11_AssetSlot* panel;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int ax = 0, ay = 0, aw = 0, ah = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH)) {
        return 0;
    }
    panel = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader, 25u);
    if (!panel || !panel->pixels ||
        panel->width != (unsigned short)panelW ||
        panel->height != (unsigned short)panelH) {
        return 0;
    }
    /* PANEL.C F0339 lines 505-514 draws C018/C019 (arrow / pressing-eye)
     * at viewport-relative (83, 57, 16, 9) on top of the C025 panel.
     * Skip that zone so the test compares only C025-owned pixels. */
    (void)M11_GameView_GetV1ArrowOrEyeZone(&ax, &ay, &aw, &ah);

    for (y = 0; y < panelH; ++y) {
        for (x = 0; x < panelW; ++x) {
            unsigned char want = panel->pixels[y * (int)panel->width + x];
            unsigned char got;
            if (want == 8 || point_is_in_chest_slot_frame(panelX, panelY, x, y)) {
                continue;
            }
            /* Skip the arrow/eye overdraw zone (viewport-relative). */
            {
                int screenX = panelX + x;
                int screenY = panelY + y;
                if (screenX >= ax && screenX < ax + aw &&
                    screenY >= ay && screenY < ay + ah) {
                    continue;
                }
            }
            got = framebuffer[(33 + panelY + y) * 320 + (panelX + x)];
            if (got != want) {
                return 0;
            }
        }
    }
    return 1;
}

static int framebuffer_preserves_inventory_backdrop_through_open_chest_red(
    const M11_GameViewState* state,
    const unsigned char* framebuffer) {
    const M11_AssetSlot* chestPanel;
    const M11_AssetSlot* backdrop;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int matched = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY,
                                              &panelW, &panelH) ||
        !M11_GameView_GetV1InventoryBackdropZone(&viewportX, &viewportY,
                                                 &viewportW, &viewportH)) {
        return 0;
    }
    chestPanel = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                      25u);
    backdrop = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                    (unsigned int)M11_GameView_GetV1InventoryBackdropGraphicId());
    if (!chestPanel || !chestPanel->pixels || !backdrop || !backdrop->pixels ||
        chestPanel->width != (unsigned short)panelW ||
        chestPanel->height != (unsigned short)panelH ||
        backdrop->width != (unsigned short)viewportW ||
        backdrop->height != (unsigned short)viewportH ||
        panelX + panelW > viewportW || panelY + panelH > viewportH) {
        return 0;
    }

    for (y = 0; y < panelH; ++y) {
        for (x = 0; x < panelW; ++x) {
            unsigned char panelPixel = chestPanel->pixels[y * (int)chestPanel->width + x];
            unsigned char want;
            unsigned char got;

            if (panelPixel != 8 || point_is_in_chest_slot_frame(panelX, panelY, x, y)) {
                continue;
            }
            want = backdrop->pixels[(panelY + y) * (int)backdrop->width + panelX + x];
            got = framebuffer[(viewportY + panelY + y) * 320 + (viewportX + panelX + x)];
            if (got == want) {
                ++matched;
            }
        }
    }
    return matched > 200;
}

static int framebuffer_matches_object_icon_at(const M11_GameViewState* state,
                                              const unsigned char* framebuffer,
                                              int iconIndex,
                                              int dstX,
                                              int dstY) {
    const M11_AssetSlot* iconGraphic;
    int graphicIndex = 0;
    int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                &graphicIndex,
                                                &srcX,
                                                &srcY,
                                                &srcW,
                                                &srcH)) {
        return 0;
    }
    iconGraphic = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                       (unsigned int)graphicIndex);
    if (!iconGraphic || !iconGraphic->pixels ||
        srcW != 16 || srcH != 16 ||
        srcX + srcW > (int)iconGraphic->width ||
        srcY + srcH > (int)iconGraphic->height) {
        return 0;
    }
    for (y = 0; y < srcH; ++y) {
        for (x = 0; x < srcW; ++x) {
            unsigned char want =
                iconGraphic->pixels[(srcY + y) * (int)iconGraphic->width + srcX + x];
            unsigned char got = framebuffer[(dstY + y) * 320 + (dstX + x)];
            if (got != want) return 0;
        }
    }
    return 1;
}

static int framebuffer_matches_chest_slot_box_pixels(
    const M11_GameViewState* state,
    const unsigned char* framebuffer,
    int chestOrdinal,
    int skipIconInterior) {
    const M11_AssetSlot* slotBox;
    int zx = 0, zy = 0, zw = 0, zh = 0;
    int x, y;
    int matched = 0;

    if (!state || !framebuffer ||
        !M11_GameView_GetV1ChestSlotBoxZone(chestOrdinal, &zx, &zy, &zw, &zh)) {
        return 0;
    }
    (void)zw;
    (void)zh;
    slotBox = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                   (unsigned int)M11_GameView_GetV1SlotBoxNormalGraphicId());
    if (!slotBox || !slotBox->pixels || slotBox->width != 18 || slotBox->height != 18) {
        return 0;
    }
    for (y = 0; y < 18; ++y) {
        for (x = 0; x < 18; ++x) {
            unsigned char want = slotBox->pixels[y * (int)slotBox->width + x];
            unsigned char got;
            if (skipIconInterior && x >= 1 && x < 17 && y >= 1 && y < 17) {
                continue;
            }
            got = framebuffer[(33 + zy - 1 + y) * 320 + (zx - 1 + x)];
            if (got != want) return 0;
            matched++;
        }
    }
    return matched > 0;
}

static int point_is_in_food_water_panel_overdraw(int x, int y, int poisoned) {
    int foodX = 32, foodY = 8;
    int waterX = 32, waterY = 31;
    int poisonX = 32, poisonY = 50;
    int barX = 33;

    if (point_in_rect(x, y, foodX, foodY, 34, 9)) return 1;
    if (point_in_rect(x, y, waterX, waterY, 46, 9)) return 1;
    if (poisoned && point_in_rect(x, y, poisonX, poisonY, 96, 15)) return 1;
    if (point_in_rect(x, y, barX, 17, 36, 8)) return 1;
    if (point_in_rect(x, y, barX, 40, 48, 8)) return 1;
    return 0;
}

static int framebuffer_matches_asset_at(const M11_AssetSlot* asset,
                                        const unsigned char* framebuffer,
                                        int dstX,
                                        int dstY,
                                        int transparentColor,
                                        int* matched) {
    int x, y;
    if (!asset || !asset->pixels || !framebuffer) return 0;
    for (y = 0; y < (int)asset->height; ++y) {
        for (x = 0; x < (int)asset->width; ++x) {
            unsigned char want = asset->pixels[y * (int)asset->width + x];
            unsigned char got;
            if (want == (unsigned char)transparentColor) continue;
            got = framebuffer[(dstY + y) * 320 + (dstX + x)];
            if (got != want) return 0;
            if (matched) ++*matched;
        }
    }
    return 1;
}

static int framebuffer_matches_food_water_source_panel_pixels(
    const M11_GameViewState* state,
    const unsigned char* framebuffer,
    int poisoned) {
    const M11_AssetSlot* panel;
    const M11_AssetSlot* food;
    const M11_AssetSlot* water;
    const M11_AssetSlot* poison;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int panelMatched = 0;
    int foodMatched = 0;
    int waterMatched = 0;
    int poisonMatched = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH)) {
        return 0;
    }
    panel = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                 (unsigned int)M11_GameView_GetV1InventoryPanelGraphicId());
    food = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)M11_GameView_GetV1FoodLabelGraphicId());
    water = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                 (unsigned int)M11_GameView_GetV1WaterLabelGraphicId());
    poison = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                  (unsigned int)M11_GameView_GetV1PoisonLabelGraphicId());
    if (!panel || !panel->pixels || !food || !food->pixels ||
        !water || !water->pixels ||
        panel->width != (unsigned short)panelW ||
        panel->height != (unsigned short)panelH) {
        return 0;
    }

    for (y = 0; y < panelH; ++y) {
        for (x = 0; x < panelW; ++x) {
            unsigned char want = panel->pixels[y * (int)panel->width + x];
            unsigned char got;
            if (want == 8 || point_is_in_food_water_panel_overdraw(x, y, poisoned)) {
                continue;
            }
            got = framebuffer[(33 + panelY + y) * 320 + (panelX + x)];
            if (got != want) return 0;
            panelMatched++;
        }
    }

    if (!framebuffer_matches_asset_at(food, framebuffer,
                                      panelX + 32,
                                      33 + panelY + 13 - (((int)food->height + 1) / 2),
                                      12, &foodMatched)) {
        return 0;
    }
    if (!framebuffer_matches_asset_at(water, framebuffer,
                                      panelX + 32,
                                      33 + panelY + 36 - (((int)water->height + 1) / 2),
                                      12, &waterMatched)) {
        return 0;
    }
    if (poisoned) {
        if (!poison || !poison->pixels ||
            !framebuffer_matches_asset_at(poison, framebuffer,
                                          panelX + 32,
                                          33 + panelY + 58 - (((int)poison->height + 1) / 2),
                                          12, &poisonMatched)) {
            return 0;
        }
    }

    return panelMatched > 1000 &&
           foodMatched > 20 &&
           waterMatched > 20 &&
           (!poisoned || poisonMatched > 20);
}

static void seed_inventory_view(M11_GameViewState* state,
                                struct DungeonThings_Compat* things,
                                struct DungeonWeapon_Compat* weapon) {
    int i;
    memset(things, 0, sizeof(*things));
    memset(weapon, 0, sizeof(*weapon));
    weapon->type = 8; /* dagger: source AllowedSlots includes backpack/container */
    things->weapons = weapon;
    things->weaponCount = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
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
}

static void test_extended_backpack_source_mapping(void) {
    ASSERT_EQ(CHAMPION_SLOT_COUNT, 30,
              "champion runtime storage keeps 30 source inventory slots");
    ASSERT_EQ(CHAMPION_SLOT_ACTION_HAND, CHAMPION_SLOT_HAND_RIGHT,
              "action hand aliases the real C01 hand storage slot");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_9), 29,
              "BACKPACK_9 maps to source slot box C528");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxForChampionSlot(CHAMPION_SLOT_BACKPACK_17), 37,
              "BACKPACK_17 maps to source slot box C536");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(29), CHAMPION_SLOT_BACKPACK_9,
              "source slot box C528 maps back to BACKPACK_9");
    ASSERT_EQ(M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(37), CHAMPION_SLOT_BACKPACK_17,
              "source slot box C536 maps back to BACKPACK_17");
    ASSERT_EQ(M11_GameView_GetV1InventorySourceSlotBoxZoneCount(), 30,
              "source inventory exposes all C507..C536 slot-box zones");
}

static void test_extended_backpack_runtime_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;

    seed_inventory_view(&state, &things, &weapon);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9] = daggerThing;
    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(29, &sx, &sy, &sw, &sh),
                "C528 zone exists");
    ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                     sx + sw / 2,
                                                     33 + sy + sh / 2,
                                                     M11_DM1_MOUSE_MASK_LEFT,
                                                     &space,
                                                     &zone),
              49,
              "C528 resolves to source command C049");
    ASSERT_EQ(zone, 528, "C528 resolves to source zone id 528");
    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C528 picks the extended backpack item");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_9], THING_NONE,
              "C528 pickup clears BACKPACK_9 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "C528 pickup moves item to leader hand");
    assert_world_hash_matches(&state, "C528 pickup refreshes deterministic world hash");

    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(37, &sx, &sy, &sw, &sh),
                "C536 zone exists");
    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C536 places leader hand into BACKPACK_17");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_BACKPACK_17], daggerThing,
              "C536 placement fills BACKPACK_17 storage");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "C536 placement clears leader hand");
    assert_world_hash_matches(&state, "C536 placement refreshes deterministic world hash");
}


static void test_all_backpack_source_slots_round_trip_runtime(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[17];
    int ordinal;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    things.weapons = weapons;
    things.weaponCount = 17;

    for (ordinal = 0; ordinal < 17; ++ordinal) {
        const int sourceSlotBox = 21 + ordinal;
        const int sourceZone = 520 + ordinal;
        const int sourceCommand = 41 + ordinal;
        const int championSlot = M11_GameView_GetV1ChampionSlotForInventorySourceSlotBox(
            sourceSlotBox);
        const unsigned short thing = (unsigned short)((THING_TYPE_WEAPON << 10) | ordinal);
        int sx = 0, sy = 0, sw = 0, sh = 0;
        int space = 0, zone = 0;

        weapons[ordinal].type = 8; /* Dagger-like weapon: source mask allows backpack slots. */
        weapons[ordinal].next = THING_ENDOFLIST;

        ASSERT_TRUE(championSlot >= CHAMPION_SLOT_BACKPACK_1 &&
                    championSlot < CHAMPION_SLOT_COUNT,
                    "C520..C536 source slot maps into champion backpack storage");
        ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(sourceSlotBox,
                                                                &sx, &sy, &sw, &sh),
                    "C520..C536 inventory source zone exists");
        ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                         sx + sw / 2,
                                                         33 + sy + sh / 2,
                                                         M11_DM1_MOUSE_MASK_LEFT,
                                                         &space,
                                                         &zone),
                  sourceCommand,
                  "C520..C536 resolves to source inventory command C041..C057");
        ASSERT_EQ(zone, sourceZone,
                  "C520..C536 route returns the matching source zone id");

        state.world.party.champions[0].inventory[championSlot] = thing;
        ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
                  M11_GAME_INPUT_REDRAW,
                  "C520..C536 runtime click picks up the stored backpack object");
        ASSERT_EQ(state.world.party.champions[0].inventory[championSlot], THING_NONE,
                  "C520..C536 pickup clears the exact backpack storage slot");
        ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), thing,
                  "C520..C536 pickup moves the object to the source leader hand");
        ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
                  M11_GAME_INPUT_REDRAW,
                  "C520..C536 runtime click places leader-hand object back into backpack");
        ASSERT_EQ(state.world.party.champions[0].inventory[championSlot], thing,
                  "C520..C536 placement restores the exact backpack storage slot");
        ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
                  "C520..C536 placement clears the source leader hand");
    }
}

static void test_open_chest_runtime_routes_and_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[3];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short arrowThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 3;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2; /* ReDMCSB object-info index 25: container-compatible. */
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = THING_ENDOFLIST;
    weapons[2].type = 4; /* ReDMCSB object-info index 27: quiver-only, not container. */
    weapons[2].next = THING_ENDOFLIST;
    containers[0].type = 0;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand container opens source chest panel state");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "open chest thing mirrors the action-hand container");
    ASSERT_EQ(M11_GameView_GetV1ChestSlotBoxZoneCount(), 8,
              "source chest panel exposes eight C537..C544 slots");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest slot zone exists");
    ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                     sx + sw / 2,
                                                     33 + sy + sh / 2,
                                                     M11_DM1_MOUSE_MASK_LEFT,
                                                     &space,
                                                     &zone),
              58,
              "C537 resolves to source command C058");
    ASSERT_EQ(zone, 537, "C537 route returns the source zone id");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C537 picks the first visible chest item");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "chest pickup moves the slot object to the leader hand");
    ASSERT_EQ(containers[0].slot, axeThing,
              "open chest writeback compacts remaining visible slot objects");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "picked chest object is detached from the container list");

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &sx, &sy, &sw, &sh),
                "C538 chest slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C538 places the leader-hand item into the chest");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "placing into chest clears the leader hand");
    ASSERT_EQ(containers[0].slot, axeThing,
              "chest list keeps the first existing object first");
    ASSERT_EQ(weapons[1].next, daggerThing,
              "placed object is linked after the existing visible item");
    ASSERT_EQ(weapons[0].next, THING_ENDOFLIST,
              "placed object terminates the compacted chest list");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, arrowThing), 1,
              "leader hand accepts a source weapon thing for rejection test");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "quiver-only object is rejected from the container slot");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), arrowThing,
              "rejected object stays in the leader hand");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "closing the panel clears open chest state");
}

static void test_open_chest_empty_slot_empty_hand_noops(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    /* ReDMCSB CHEST.C F0333 lines 58-75 fills trailing visible chest slots
     * with C0xFFFF_THING_NONE, then CHAMPION.C F0302 lines 688-695 returns
     * before screen update when both the selected slot and leader hand are
     * empty. */
    seed_inventory_view(&state, &things, &weapon);
    memset(containers, 0, sizeof(containers));
    things.containers = containers;
    things.containerCount = 1;
    weapon.type = 8;
    weapon.next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before empty-slot no-op");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &sx, &sy, &sw, &sh),
                "C538 empty chest slot zone exists");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "leader hand starts empty for empty-slot no-op");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "clicking empty C538 with empty leader hand is source no-op");
    ASSERT_EQ(state.lastWorldHash, 0xBADF00Du,
              "empty-slot no-op does not refresh deterministic world hash");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "empty-slot no-op keeps leader hand empty");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "empty-slot no-op keeps the chest panel open");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "empty-slot no-op preserves the visible chest list head");
    ASSERT_EQ(weapon.next, THING_ENDOFLIST,
              "empty-slot no-op preserves the visible chest list tail");
}

static void test_open_chest_late_empty_slot_placement_promotes_on_close(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 routes C540 to slot index
     * C33 (C30 + 3), F0301 lines 606-610 accepts a non-empty
     * leader-hand object by writing only G0425_aT_ChestSlots[3], and
     * CHEST.C F0334 lines 117-129 later rebuilds the source container
     * by promoting the first non-empty visible slot to Container->Slot. */
    seed_inventory_view(&state, &things, &weapon);
    memset(containers, 0, sizeof(containers));
    things.containers = containers;
    things.containerCount = 1;
    weapon.type = 8; /* DUNGEON.C G0237 line 112: Dagger, Chest-allowed. */
    weapon.next = THING_ENDOFLIST;
    containers[0].slot = THING_ENDOFLIST;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before late empty-slot placement");
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, daggerThing), 1,
              "leader hand accepts a chest-compatible object for late empty-slot placement");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(3, &sx, &sy, &sw, &sh),
                "C540 late empty chest slot zone exists");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C540 places the leader-hand object into a late empty chest slot");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "late empty-slot placement clears the leader hand");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "late empty-slot placement keeps the chest panel open");
    assert_world_hash_matches(&state,
                              "late empty-slot placement refreshes deterministic world hash");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "close after late empty-slot placement clears panel state");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "close promotes the late non-empty visible slot to the container head");
    ASSERT_EQ(weapon.next, THING_ENDOFLIST,
              "close terminates the single promoted chest object");
}

static void test_open_chest_occupied_slot_swap_preserves_visible_order(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[4];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short maceThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short swordThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-710 reads C30+ slots from
     * G0425_aT_ChestSlots, gates leader-hand placement against DATA.C
     * lines 1080-1087 MASK0x0400_CONTAINER, removes the occupied slot
     * into the leader hand, then adds the previous leader-hand object
     * back to the same chest slot.  CHEST.C F0334 lines 112-133 then
     * rewrites the compact visible slot order on close. */
    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 4;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2;
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = maceThing;
    weapons[2].type = 2;
    weapons[2].next = THING_ENDOFLIST;
    weapons[3].type = 2;
    weapons[3].next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before occupied-slot swap");
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, swordThing), 1,
              "leader hand accepts a container-compatible object before chest swap");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &sx, &sy, &sw, &sh),
                "C538 occupied chest slot zone exists");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking occupied C538 swaps with the leader-hand object");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), axeThing,
              "occupied chest-slot swap moves the old C538 item to leader hand");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "occupied chest-slot swap keeps C537 as list head");
    ASSERT_EQ(weapons[0].next, swordThing,
              "occupied chest-slot swap places leader-hand object at C538");
    ASSERT_EQ(weapons[3].next, maceThing,
              "occupied chest-slot swap preserves the following visible item");
    ASSERT_EQ(weapons[2].next, THING_ENDOFLIST,
              "occupied chest-slot swap keeps the last visible item terminating");
    ASSERT_EQ(weapons[1].next, THING_ENDOFLIST,
              "occupied chest-slot swap detaches the picked C538 item");
    assert_world_hash_matches(&state, "occupied chest-slot swap refreshes deterministic world hash");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "close after occupied chest-slot swap clears panel state");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "close after occupied chest-slot swap keeps C537 as head");
    ASSERT_EQ(weapons[0].next, swordThing,
              "close after occupied chest-slot swap preserves replacement C538");
    ASSERT_EQ(weapons[3].next, maceThing,
              "close after occupied chest-slot swap preserves C539 after replacement");
    ASSERT_EQ(weapons[2].next, THING_ENDOFLIST,
              "close after occupied chest-slot swap terminates the visible list");
    ASSERT_EQ(weapons[1].next, THING_ENDOFLIST,
              "close after occupied chest-slot swap keeps picked C538 detached");
}

static void test_open_chest_rejects_incompatible_leader_hand_without_mutation(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[4];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short staffThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-698 reads the selected
     * G0425_aT_ChestSlots entry, then returns before F0077/F0300/F0297/
     * F0301 when the leader-hand object's AllowedSlots do not intersect
     * DATA.C lines 1080-1087 MASK0x0400_CONTAINER for C537..C544.
     * DUNGEON.C G0237 line 108 gives Staff Of Claws 0x0040 (Quiver 1),
     * so it must not swap with or disturb an occupied chest slot. */
    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 4;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 8; /* DUNGEON.C G0237 line 112: Dagger, Chest-allowed. */
    weapons[0].next = torchThing;
    weapons[1].type = 2; /* DUNGEON.C G0237 line 106: Torch, Chest-allowed. */
    weapons[1].next = axeThing;
    weapons[2].type = 8;
    weapons[2].next = THING_ENDOFLIST;
    weapons[3].type = 4; /* DUNGEON.C G0237 line 108: Staff Of Claws, Quiver 1 only. */
    weapons[3].next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before incompatible leader-hand rejection");
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, staffThing), 1,
              "leader hand accepts Staff Of Claws before chest rejection");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &sx, &sy, &sw, &sh),
                "C538 occupied chest slot zone exists for rejection");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "clicking C538 with quiver-only leader-hand object is source no-op");
    ASSERT_EQ(state.lastWorldHash, 0xBADF00Du,
              "incompatible chest-slot rejection does not refresh deterministic world hash");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), staffThing,
              "incompatible chest-slot rejection keeps leader-hand object");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "incompatible chest-slot rejection keeps the chest panel open");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "incompatible chest-slot rejection preserves C537 list head");
    ASSERT_EQ(weapons[0].next, torchThing,
              "incompatible chest-slot rejection preserves link before C538");
    ASSERT_EQ(weapons[1].next, axeThing,
              "incompatible chest-slot rejection preserves occupied C538 link");
    ASSERT_EQ(weapons[2].next, THING_ENDOFLIST,
              "incompatible chest-slot rejection preserves list terminator");
    ASSERT_EQ(weapons[3].next, THING_ENDOFLIST,
              "incompatible chest-slot rejection leaves rejected object detached");
}

static void test_leader_hand_container_eye_routes_to_chest_panel(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat container;
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned char framebuffer[320 * 200];
    int sx = 0, sy = 0, sw = 0, sh = 0;

    seed_inventory_view(&state, &things, &weapon);
    memset(&container, 0, sizeof(container));
    things.containers = &container;
    things.containerCount = 1;
    weapon.type = 2;
    weapon.next = THING_ENDOFLIST;
    container.type = 0;
    container.slot = daggerThing;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, chestThing), 1,
              "leader hand accepts source container thing");
    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source chest panel blit");
    state.assetsAvailable = 1;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click with leader-hand container redraws source chest panel");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "leader-hand container eye route opens chest panel state");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "container eye route does not open the generic dialog overlay");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "container eye route does not mark object-description panel active");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "container eye route does not mark scroll panel active");
    ASSERT_TRUE(strstr(state.inspectDetail, "CONTAINER CHEST PANEL") != NULL,
                "container eye route records source chest-panel detail");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest slot zone exists after leader-hand container eye click");
    (void)sw;
    (void)sh;
    ASSERT_TRUE(framebuffer_matches_open_chest_panel_pixels(&state, framebuffer),
                "leader-hand container eye render blits source C025 open-chest panel into C101");
    ASSERT_TRUE(framebuffer[(33 + sy) * 320 + sx] != 0,
                "leader-hand container eye render reaches C537 chest slot frame");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_slot_box_and_icon_source_pixels(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int daggerIcon;

    seed_inventory_view(&state, &things, &weapon);
    memset(&container, 0, sizeof(container));
    things.containers = &container;
    things.containerCount = 1;
    weapon.type = 8; /* DUNGEON.C G0237 object-info index 31: DAGGER, icon C032. */
    weapon.next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for chest slot-box/icon pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before slot-box/icon pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    ASSERT_EQ(daggerIcon, 32,
              "source dagger icon resolves through F0033_OBJECT_GetIconIndex");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest slot zone exists for populated source-pixel gate");
    (void)sw;
    (void)sh;
    /* ReDMCSB CHEST.C F0333 lines 58-65 draws each non-empty visible
     * chest object through F0038_OBJECT_DrawIconInSlotBox(C38 + n,
     * F0033_OBJECT_GetIconIndex(thing)); empty trailing slots are drawn
     * by lines 68-75 with no object-icon overdraw. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          0,
                                                          1),
                "open chest C537 keeps source C033 slot-box border pixels around the object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   sx,
                                                   33 + sy),
                "open chest C537 blits the exact source dagger icon subrect");
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          1,
                                                          0),
                "open chest empty C538 blits the full source C033 slot-box without icon overdraw");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_panel_red_transparency_preserves_inventory_backdrop(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    memset(&container, 0, sizeof(container));
    things.containers = &container;
    things.containerCount = 1;
    weapon.type = 8;
    weapon.next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for chest-panel transparency gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before panel transparency gate");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    /* ReDMCSB CHEST.C F0333 lines 47-51 blits C025 into C101 with
     * C08_COLOR_RED as transparent before lines 58-75 draw C537..C544.
     * Lock both the opaque panel pixels and the transparent red pixels on
     * the direct action-hand-open path, not only the leader-hand eye route. */
    ASSERT_TRUE(framebuffer_matches_open_chest_panel_pixels(&state,
                                                            framebuffer),
                "action-hand open chest blits source C025 non-red panel pixels into C101");
    /* PANEL.C F0355 lines 2375-2377 establishes C017 as the inventory
     * backdrop before panel content, so red pixels in the open-chest graphic
     * must reveal C017. */
    ASSERT_TRUE(framebuffer_preserves_inventory_backdrop_through_open_chest_red(
                    &state, framebuffer),
                "open chest C025 transparent red pixels preserve the C017 inventory backdrop");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_second_visible_slot_uses_second_object_icon(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int c537x = 0, c537y = 0, c537w = 0, c537h = 0;
    int c538x = 0, c538y = 0, c538w = 0, c538h = 0;
    int daggerIcon;
    int torchIcon;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = 2;
    things.containers = &container;
    things.containerCount = 1;
    weapons[0].type = 8; /* DUNGEON.C line 112: DAGGER, source icon C032. */
    weapons[0].next = torchThing;
    weapons[1].type = 2; /* DUNGEON.C line 106: TORCH, source icon C004, Chest-allowed. */
    weapons[1].next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for second visible chest-slot pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before second visible slot pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    torchIcon = M11_GameView_GetObjectIconIndexForThing(&state, torchThing);
    ASSERT_EQ(daggerIcon, 32,
              "first linked chest object resolves to DUNGEON.C G0237 dagger icon C032");
    ASSERT_EQ(torchIcon, 4,
              "second linked chest object resolves to DUNGEON.C G0237 torch icon C004");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &c537x, &c537y, &c537w, &c537h),
                "C537 first visible chest slot zone exists");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &c538x, &c538y, &c538w, &c538h),
                "C538 second visible chest slot zone exists");
    (void)c537w;
    (void)c537h;
    (void)c538w;
    (void)c538h;
    /* ReDMCSB CHEST.C F0333 lines 64-66 draws C38 + slotIndex with
     * F0033_OBJECT_GetIconIndex(currentThing), then advances through
     * F0159_DUNGEON_GetNextThing.  DUNGEON.C lines 79-112 map the
     * first object to dagger C032 and the second object to torch C004. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          1,
                                                          1),
                "open chest C538 keeps source C033 slot-box border pixels around the second object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   c537x,
                                                   33 + c537y),
                "open chest C537 still blits the first linked object's dagger icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   torchIcon,
                                                   c538x,
                                                   33 + c538y),
                "open chest C538 blits the second linked object's torch icon");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_third_visible_slot_uses_third_object_icon(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[3];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short morningstarThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    int c537x = 0, c537y = 0, c537w = 0, c537h = 0;
    int c538x = 0, c538y = 0, c538w = 0, c538h = 0;
    int c539x = 0, c539y = 0, c539w = 0, c539h = 0;
    int daggerIcon;
    int torchIcon;
    int morningstarIcon;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = 3;
    things.containers = &container;
    things.containerCount = 1;
    weapons[0].type = 8;  /* DUNGEON.C line 112: DAGGER, source icon C032. */
    weapons[0].next = torchThing;
    weapons[1].type = 2;  /* DUNGEON.C line 106: TORCH, source icon C004. */
    weapons[1].next = morningstarThing;
    weapons[2].type = 22; /* DUNGEON.C line 126: MORNINGSTAR, source icon C046. */
    weapons[2].next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for third visible chest-slot pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before third visible slot pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    torchIcon = M11_GameView_GetObjectIconIndexForThing(&state, torchThing);
    morningstarIcon = M11_GameView_GetObjectIconIndexForThing(&state, morningstarThing);
    ASSERT_EQ(daggerIcon, 32,
              "first linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 112 to C032");
    ASSERT_EQ(torchIcon, 4,
              "second linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 106 to C004");
    ASSERT_EQ(morningstarIcon, 46,
              "third linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 126 to C046");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &c537x, &c537y, &c537w, &c537h),
                "C537 first visible chest slot zone exists");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &c538x, &c538y, &c538w, &c538h),
                "C538 second visible chest slot zone exists");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(2, &c539x, &c539y, &c539w, &c539h),
                "C539 third visible chest slot zone exists");
    (void)c537w;
    (void)c537h;
    (void)c538w;
    (void)c538h;
    (void)c539w;
    (void)c539h;
    /* ReDMCSB CHEST.C F0333 lines 58-66 walks Slot -> Next -> Next
     * through F0159_DUNGEON_GetNextThing, stores those entries in
     * G0425_aT_ChestSlots, and draws C38 + slotIndex with
     * F0033_OBJECT_GetIconIndex(currentThing).  DUNGEON.C F0141
     * lines 1153-1154 maps each weapon subtype into G0237, whose
     * lines 106/112/126 give the three distinct source icons. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          2,
                                                          1),
                "open chest C539 keeps source C033 slot-box border pixels around the third object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   c537x,
                                                   33 + c537y),
                "open chest C537 still blits the first linked object's DUNGEON.C line 112 dagger icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   torchIcon,
                                                   c538x,
                                                   33 + c538y),
                "open chest C538 still blits the second linked object's DUNGEON.C line 106 torch icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   morningstarIcon,
                                                   c539x,
                                                   33 + c539y),
                "open chest C539 blits the third linked object's DUNGEON.C line 126 morningstar icon");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_fourth_visible_slot_uses_fourth_object_icon(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[4];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short morningstarThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short arrowThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    int c537x = 0, c537y = 0, c537w = 0, c537h = 0;
    int c538x = 0, c538y = 0, c538w = 0, c538h = 0;
    int c539x = 0, c539y = 0, c539w = 0, c539h = 0;
    int c540x = 0, c540y = 0, c540w = 0, c540h = 0;
    int daggerIcon;
    int torchIcon;
    int morningstarIcon;
    int arrowIcon;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = 4;
    things.containers = &container;
    things.containerCount = 1;
    weapons[0].type = 8;  /* DUNGEON.C line 112: DAGGER, source icon C032. */
    weapons[0].next = torchThing;
    weapons[1].type = 2;  /* DUNGEON.C line 106: TORCH, source icon C004. */
    weapons[1].next = morningstarThing;
    weapons[2].type = 22; /* DUNGEON.C line 126: MORNINGSTAR, source icon C046. */
    weapons[2].next = arrowThing;
    weapons[3].type = 27; /* DUNGEON.C line 131: ARROW, source icon C051. */
    weapons[3].next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for fourth visible chest-slot pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before fourth visible slot pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    torchIcon = M11_GameView_GetObjectIconIndexForThing(&state, torchThing);
    morningstarIcon = M11_GameView_GetObjectIconIndexForThing(&state, morningstarThing);
    arrowIcon = M11_GameView_GetObjectIconIndexForThing(&state, arrowThing);
    ASSERT_EQ(daggerIcon, 32,
              "first linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 112 to C032");
    ASSERT_EQ(torchIcon, 4,
              "second linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 106 to C004");
    ASSERT_EQ(morningstarIcon, 46,
              "third linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 126 to C046");
    ASSERT_EQ(arrowIcon, 51,
              "fourth linked object resolves via DUNGEON.C F0141 lines 1153-1154 and G0237 line 131 to C051");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &c537x, &c537y, &c537w, &c537h),
                "C537 first visible chest slot zone exists for fourth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &c538x, &c538y, &c538w, &c538h),
                "C538 second visible chest slot zone exists for fourth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(2, &c539x, &c539y, &c539w, &c539h),
                "C539 third visible chest slot zone exists for fourth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(3, &c540x, &c540y, &c540w, &c540h),
                "C540 fourth visible chest slot zone exists");
    (void)c537w;
    (void)c537h;
    (void)c538w;
    (void)c538h;
    (void)c539w;
    (void)c539h;
    (void)c540w;
    (void)c540h;
    /* ReDMCSB CHEST.C F0333 lines 58-66 walks Slot -> Next -> Next
     * -> Next through F0159_DUNGEON_GetNextThing, stores each thing in
     * G0425_aT_ChestSlots, and draws C38 + slotIndex with that thing's
     * F0033_OBJECT_GetIconIndex result.  DUNGEON.C F0141 lines 1153-1154
     * maps weapon subtypes into G0237, whose lines 106/112/126/131 give
     * these four distinct source icons. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          3,
                                                          1),
                "open chest C540 keeps source C033 slot-box border pixels around the fourth object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   c537x,
                                                   33 + c537y),
                "open chest C537 still blits the first linked object's DUNGEON.C line 112 dagger icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   torchIcon,
                                                   c538x,
                                                   33 + c538y),
                "open chest C538 still blits the second linked object's DUNGEON.C line 106 torch icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   morningstarIcon,
                                                   c539x,
                                                   33 + c539y),
                "open chest C539 still blits the third linked object's DUNGEON.C line 126 morningstar icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   arrowIcon,
                                                   c540x,
                                                   33 + c540y),
                "open chest C540 blits the fourth linked object's DUNGEON.C line 131 arrow icon");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_fifth_visible_slot_uses_fifth_object_icon(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[5];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short morningstarThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short arrowThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    unsigned short slayerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 4);
    int c537x = 0, c537y = 0, c537w = 0, c537h = 0;
    int c538x = 0, c538y = 0, c538w = 0, c538h = 0;
    int c539x = 0, c539y = 0, c539w = 0, c539h = 0;
    int c540x = 0, c540y = 0, c540w = 0, c540h = 0;
    int c541x = 0, c541y = 0, c541w = 0, c541h = 0;
    int daggerIcon;
    int torchIcon;
    int morningstarIcon;
    int arrowIcon;
    int slayerIcon;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = 5;
    things.containers = &container;
    things.containerCount = 1;
    weapons[0].type = 8;  /* DUNGEON.C line 112: DAGGER, source icon C032. */
    weapons[0].next = torchThing;
    weapons[1].type = 2;  /* DUNGEON.C line 106: TORCH, source icon C004. */
    weapons[1].next = morningstarThing;
    weapons[2].type = 22; /* DUNGEON.C line 126: MORNINGSTAR, source icon C046. */
    weapons[2].next = arrowThing;
    weapons[3].type = 27; /* DUNGEON.C line 131: ARROW, source icon C051. */
    weapons[3].next = slayerThing;
    weapons[4].type = 28; /* DUNGEON.C line 132: SLAYER, source icon C052. */
    weapons[4].next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for fifth visible chest-slot pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before fifth visible slot pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    torchIcon = M11_GameView_GetObjectIconIndexForThing(&state, torchThing);
    morningstarIcon = M11_GameView_GetObjectIconIndexForThing(&state, morningstarThing);
    arrowIcon = M11_GameView_GetObjectIconIndexForThing(&state, arrowThing);
    slayerIcon = M11_GameView_GetObjectIconIndexForThing(&state, slayerThing);
    ASSERT_EQ(daggerIcon, 32,
              "first linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 112 to C032");
    ASSERT_EQ(torchIcon, 4,
              "second linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 106 to C004");
    ASSERT_EQ(morningstarIcon, 46,
              "third linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 126 to C046");
    ASSERT_EQ(arrowIcon, 51,
              "fourth linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 131 to C051");
    ASSERT_EQ(slayerIcon, 52,
              "fifth linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 132 to C052");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &c537x, &c537y, &c537w, &c537h),
                "C537 first visible chest slot zone exists for fifth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &c538x, &c538y, &c538w, &c538h),
                "C538 second visible chest slot zone exists for fifth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(2, &c539x, &c539y, &c539w, &c539h),
                "C539 third visible chest slot zone exists for fifth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(3, &c540x, &c540y, &c540w, &c540h),
                "C540 fourth visible chest slot zone exists for fifth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(4, &c541x, &c541y, &c541w, &c541h),
                "C541 fifth visible chest slot zone exists");
    (void)c537w;
    (void)c537h;
    (void)c538w;
    (void)c538h;
    (void)c539w;
    (void)c539h;
    (void)c540w;
    (void)c540h;
    (void)c541w;
    (void)c541h;
    /* ReDMCSB CHEST.C F0333 lines 58-66 walks Slot -> Next -> Next
     * -> Next -> Next through F0159_DUNGEON_GetNextThing, stores each
     * visible thing in G0425_aT_ChestSlots, and draws C38 + slotIndex
     * with F0033_OBJECT_GetIconIndex(currentThing).  OBJECT.C F0033
     * lines 147-164 returns the G0237 Type column reached through
     * DUNGEON.C F0141 lines 1144-1156; G0237 lines 106/112/126/131/132
     * give these five distinct source icons. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          4,
                                                          1),
                "open chest C541 keeps source C033 slot-box border pixels around the fifth object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   c537x,
                                                   33 + c537y),
                "open chest C537 still blits the first linked object's DUNGEON.C line 112 dagger icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   torchIcon,
                                                   c538x,
                                                   33 + c538y),
                "open chest C538 still blits the second linked object's DUNGEON.C line 106 torch icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   morningstarIcon,
                                                   c539x,
                                                   33 + c539y),
                "open chest C539 still blits the third linked object's DUNGEON.C line 126 morningstar icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   arrowIcon,
                                                   c540x,
                                                   33 + c540y),
                "open chest C540 still blits the fourth linked object's DUNGEON.C line 131 arrow icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   slayerIcon,
                                                   c541x,
                                                   33 + c541y),
                "open chest C541 blits the fifth linked object's DUNGEON.C line 132 Slayer icon");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_sixth_visible_slot_uses_sixth_object_icon(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[6];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short torchThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short morningstarThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short arrowThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    unsigned short slayerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 4);
    unsigned short slingThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 5);
    int c537x = 0, c537y = 0, c537w = 0, c537h = 0;
    int c538x = 0, c538y = 0, c538w = 0, c538h = 0;
    int c539x = 0, c539y = 0, c539w = 0, c539h = 0;
    int c540x = 0, c540y = 0, c540w = 0, c540h = 0;
    int c541x = 0, c541y = 0, c541w = 0, c541h = 0;
    int c542x = 0, c542y = 0, c542w = 0, c542h = 0;
    int daggerIcon;
    int torchIcon;
    int morningstarIcon;
    int arrowIcon;
    int slayerIcon;
    int slingIcon;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = 6;
    things.containers = &container;
    things.containerCount = 1;
    weapons[0].type = 8;  /* DUNGEON.C line 112: DAGGER, source icon C032. */
    weapons[0].next = torchThing;
    weapons[1].type = 2;  /* DUNGEON.C line 106: TORCH, source icon C004. */
    weapons[1].next = morningstarThing;
    weapons[2].type = 22; /* DUNGEON.C line 126: MORNINGSTAR, source icon C046. */
    weapons[2].next = arrowThing;
    weapons[3].type = 27; /* DUNGEON.C line 131: ARROW, source icon C051. */
    weapons[3].next = slayerThing;
    weapons[4].type = 28; /* DUNGEON.C line 132: SLAYER, source icon C052. */
    weapons[4].next = slingThing;
    weapons[5].type = 29; /* DUNGEON.C line 133: SLING, source icon C053. */
    weapons[5].next = THING_ENDOFLIST;
    container.slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for sixth visible chest-slot pixel gate");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before sixth visible slot pixel gate");
    daggerIcon = M11_GameView_GetObjectIconIndexForThing(&state, daggerThing);
    torchIcon = M11_GameView_GetObjectIconIndexForThing(&state, torchThing);
    morningstarIcon = M11_GameView_GetObjectIconIndexForThing(&state, morningstarThing);
    arrowIcon = M11_GameView_GetObjectIconIndexForThing(&state, arrowThing);
    slayerIcon = M11_GameView_GetObjectIconIndexForThing(&state, slayerThing);
    slingIcon = M11_GameView_GetObjectIconIndexForThing(&state, slingThing);
    ASSERT_EQ(daggerIcon, 32,
              "first linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 112 to C032");
    ASSERT_EQ(torchIcon, 4,
              "second linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 106 to C004");
    ASSERT_EQ(morningstarIcon, 46,
              "third linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 126 to C046");
    ASSERT_EQ(arrowIcon, 51,
              "fourth linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 131 to C051");
    ASSERT_EQ(slayerIcon, 52,
              "fifth linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 132 to C052");
    ASSERT_EQ(slingIcon, 53,
              "sixth linked object resolves via DUNGEON.C F0141 lines 1144-1156 and G0237 line 133 to C053");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &c537x, &c537y, &c537w, &c537h),
                "C537 first visible chest slot zone exists for sixth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(1, &c538x, &c538y, &c538w, &c538h),
                "C538 second visible chest slot zone exists for sixth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(2, &c539x, &c539y, &c539w, &c539h),
                "C539 third visible chest slot zone exists for sixth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(3, &c540x, &c540y, &c540w, &c540h),
                "C540 fourth visible chest slot zone exists for sixth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(4, &c541x, &c541y, &c541w, &c541h),
                "C541 fifth visible chest slot zone exists for sixth-slot chain");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(5, &c542x, &c542y, &c542w, &c542h),
                "C542 sixth visible chest slot zone exists");
    (void)c537w;
    (void)c537h;
    (void)c538w;
    (void)c538h;
    (void)c539w;
    (void)c539h;
    (void)c540w;
    (void)c540h;
    (void)c541w;
    (void)c541h;
    (void)c542w;
    (void)c542h;
    /* ReDMCSB CHEST.C F0333 lines 58-66 walks Slot -> Next -> Next
     * -> Next -> Next -> Next through F0159_DUNGEON_GetNextThing,
     * stores each visible thing in G0425_aT_ChestSlots, and draws
     * C38 + slotIndex with F0033_OBJECT_GetIconIndex(currentThing).
     * OBJECT.C F0033 lines 147-164 returns the G0237 Type column
     * reached through DUNGEON.C F0141 lines 1144-1156; G0237 lines
     * 106/112/126/131/132/133 give these six distinct source icons. */
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          5,
                                                          1),
                "open chest C542 keeps source C033 slot-box border pixels around the sixth object icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   daggerIcon,
                                                   c537x,
                                                   33 + c537y),
                "open chest C537 still blits the first linked object's DUNGEON.C line 112 dagger icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   torchIcon,
                                                   c538x,
                                                   33 + c538y),
                "open chest C538 still blits the second linked object's DUNGEON.C line 106 torch icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   morningstarIcon,
                                                   c539x,
                                                   33 + c539y),
                "open chest C539 still blits the third linked object's DUNGEON.C line 126 morningstar icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   arrowIcon,
                                                   c540x,
                                                   33 + c540y),
                "open chest C540 still blits the fourth linked object's DUNGEON.C line 131 arrow icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   slayerIcon,
                                                   c541x,
                                                   33 + c541y),
                "open chest C541 still blits the fifth linked object's DUNGEON.C line 132 Slayer icon");
    ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                   framebuffer,
                                                   slingIcon,
                                                   c542x,
                                                   33 + c542y),
                "open chest C542 blits the sixth linked object's DUNGEON.C line 133 Sling icon");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

typedef struct ChestVisibleWeaponIconCase {
    int weaponType;
    int expectedIcon;
    int dungeonLine;
    const char* ordinalName;
    const char* objectName;
} ChestVisibleWeaponIconCase;

static const ChestVisibleWeaponIconCase kChestVisibleWeaponIconCases[] = {
    { 8,  32, 112, "first",   "dagger" },
    { 2,   4, 106, "second",  "torch" },
    { 22, 46, 126, "third",   "morningstar" },
    { 27, 51, 131, "fourth",  "arrow" },
    { 28, 52, 132, "fifth",   "Slayer" },
    { 29, 53, 133, "sixth",   "Sling" },
    { 30, 54, 134, "seventh", "Rock" },
    { 31, 55, 135, "eighth",  "Poison Dart" }
};

static unsigned short chest_visible_weapon_thing(int ordinal) {
    return (unsigned short)((THING_TYPE_WEAPON << 10) | ordinal);
}

static void assert_open_chest_visible_weapon_icon_chain(int targetOrdinal,
                                                        const char* gateLabel) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonContainer_Compat container;
    unsigned char framebuffer[320 * 200];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    int slotX[8];
    int slotY[8];
    int icons[8];
    int visibleCount;
    int i;
    char msg[224];

    ASSERT_TRUE(targetOrdinal >= 0 &&
                    targetOrdinal < (int)(sizeof(kChestVisibleWeaponIconCases) /
                                          sizeof(kChestVisibleWeaponIconCases[0])),
                "visible chest icon helper target is in range");
    if (targetOrdinal < 0 ||
        targetOrdinal >= (int)(sizeof(kChestVisibleWeaponIconCases) /
                               sizeof(kChestVisibleWeaponIconCases[0]))) {
        return;
    }
    visibleCount = targetOrdinal + 1;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(&container, 0, sizeof(container));
    things.weapons = weapons;
    things.weaponCount = visibleCount;
    things.containers = &container;
    things.containerCount = 1;
    for (i = 0; i < visibleCount; ++i) {
        weapons[i].type = kChestVisibleWeaponIconCases[i].weaponType;
        weapons[i].next = (i + 1 < visibleCount)
            ? chest_visible_weapon_thing(i + 1)
            : THING_ENDOFLIST;
    }
    container.slot = chest_visible_weapon_thing(0);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    snprintf(msg, sizeof(msg),
             "GRAPHICS.DAT asset loader is available for %s visible chest-slot pixel gate",
             kChestVisibleWeaponIconCases[targetOrdinal].ordinalName);
    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()), msg);
    state.assetsAvailable = 1;
    snprintf(msg, sizeof(msg),
             "action-hand chest opens before %s visible slot pixel gate",
             kChestVisibleWeaponIconCases[targetOrdinal].ordinalName);
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1, msg);

    for (i = 0; i < visibleCount; ++i) {
        icons[i] = M11_GameView_GetObjectIconIndexForThing(&state,
                                                           chest_visible_weapon_thing(i));
        snprintf(msg, sizeof(msg),
                 "%s linked object resolves via DUNGEON.C F0141 lines 1145-1154 "
                 "and G0237 line %d to C%03d",
                 kChestVisibleWeaponIconCases[i].ordinalName,
                 kChestVisibleWeaponIconCases[i].dungeonLine,
                 kChestVisibleWeaponIconCases[i].expectedIcon);
        ASSERT_EQ(icons[i], kChestVisibleWeaponIconCases[i].expectedIcon, msg);
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    for (i = 0; i < visibleCount; ++i) {
        int w = 0;
        int h = 0;
        snprintf(msg, sizeof(msg),
                 "C%d %s visible chest slot zone exists for %s chain",
                 537 + i,
                 kChestVisibleWeaponIconCases[i].ordinalName,
                 gateLabel);
        ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(i, &slotX[i], &slotY[i], &w, &h),
                    msg);
        (void)w;
        (void)h;
    }

    /* ReDMCSB CHEST.C F0333 lines 58-66 advances the visible chest
     * slots through F0159_DUNGEON_GetNextThing, stores each thing in
     * G0425_aT_ChestSlots, and draws C38 + slotIndex with
     * F0033_OBJECT_GetIconIndex(currentThing). OBJECT.C F0033 lines
     * 147-164 obtains the icon through DUNGEON.C F0141 lines 1145-1154;
     * G0237 lines 106/112/126/131/132/133/134/135 give the distinct
     * torch/dagger/morningstar/arrow/Slayer/Sling/Rock/Poison Dart source icons. */
    snprintf(msg, sizeof(msg),
             "open chest %s keeps source C033 slot-box border pixels around the %s object icon",
             gateLabel,
             kChestVisibleWeaponIconCases[targetOrdinal].ordinalName);
    ASSERT_TRUE(framebuffer_matches_chest_slot_box_pixels(&state,
                                                          framebuffer,
                                                          targetOrdinal,
                                                          1),
                msg);
    for (i = 0; i < visibleCount; ++i) {
        snprintf(msg, sizeof(msg),
                 "open chest C%d blits the %s linked object's DUNGEON.C G0237 line %d %s icon",
                 537 + i,
                 kChestVisibleWeaponIconCases[i].ordinalName,
                 kChestVisibleWeaponIconCases[i].dungeonLine,
                 kChestVisibleWeaponIconCases[i].objectName);
        ASSERT_TRUE(framebuffer_matches_object_icon_at(&state,
                                                       framebuffer,
                                                       icons[i],
                                                       slotX[i],
                                                       33 + slotY[i]),
                    msg);
    }

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_seventh_visible_slot_uses_seventh_object_icon(void) {
    assert_open_chest_visible_weapon_icon_chain(6, "C543");
}

static void test_open_chest_eighth_visible_slot_uses_eighth_object_icon(void) {
    assert_open_chest_visible_weapon_icon_chain(7, "C544");
}

static void test_empty_hand_mouth_blits_source_food_water_panel_pixels(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned char framebuffer[320 * 200];

    seed_inventory_view(&state, &things, &weapon);
    state.world.party.champions[0].food = 1200;
    state.world.party.champions[0].water = 700;
    state.world.party.champions[0].poisonDose = 9;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source food/water panel blits");
    state.assetsAvailable = 1;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 56 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "empty-hand mouth click routes to the source food/water panel");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 1,
              "empty-hand mouth click marks food/water panel active");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "food/water route clears object-description panel state");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "food/water route clears scroll panel state");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "food/water route clears champion-stats panel state");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "food/water route closes any open chest panel state");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(framebuffer_matches_food_water_source_panel_pixels(&state, framebuffer, 1),
                "empty-hand mouth render blits source C020/C030/C031/C032 pixels into C101/C500/C501/C502");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 56 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_IGNORED,
              "second empty-hand mouth click keeps the already-active food/water panel");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_open_chest_middle_pickup_compacts_visible_list(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[4];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    unsigned short swordThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 2);
    unsigned short maceThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 3);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 4;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2;
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = swordThing;
    weapons[2].type = 2;
    weapons[2].next = maceThing;
    weapons[3].type = 2;
    weapons[3].next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before middle-slot pickup");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(2, &sx, &sy, &sw, &sh),
                "C539 middle chest slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C539 picks the middle visible chest object");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), swordThing,
              "middle chest pickup moves the selected object to leader hand");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "middle chest pickup keeps first visible object as list head");
    ASSERT_EQ(weapons[0].next, axeThing,
              "middle chest pickup preserves object before the picked slot");
    ASSERT_EQ(weapons[1].next, maceThing,
              "middle chest pickup links around the emptied visible slot");
    ASSERT_EQ(weapons[2].next, THING_ENDOFLIST,
              "middle chest pickup detaches the picked object from chest list");
    ASSERT_EQ(weapons[3].next, THING_ENDOFLIST,
              "middle chest pickup terminates the compacted visible list");
}

static void test_open_chest_pickup_preserves_mixed_type_tail_order(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonPotion_Compat potion;
    struct DungeonJunk_Compat junk;
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short potionThing = (unsigned short)((THING_TYPE_POTION << 10) | 0);
    unsigned short junkThing = (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    /* ReDMCSB CHAMPION.C F0302 lines 688-705 removes the selected
     * C30+ G0425_aT_ChestSlots entry, and CHEST.C F0334 lines 117-129
     * rebuilds the non-empty visible slots in order.  DUNGEON.C F0163
     * lines 1796-1837 writes through GENERIC->Next, so the following
     * potion/junk tail must keep its order after a weapon head pickup. */
    seed_inventory_view(&state, &things, &weapon);
    memset(&potion, 0, sizeof(potion));
    memset(&junk, 0, sizeof(junk));
    memset(containers, 0, sizeof(containers));
    things.potions = &potion;
    things.potionCount = 1;
    things.junks = &junk;
    things.junkCount = 1;
    things.containers = containers;
    things.containerCount = 1;
    weapon.type = 8;
    weapon.next = potionThing;
    potion.type = 1;
    potion.next = junkThing;
    junk.type = 0;
    junk.next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before mixed-type tail pickup");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 mixed-type head slot zone exists");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C537 picks the weapon head from a mixed-type chest chain");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), daggerThing,
              "mixed-type pickup moves the weapon head to leader hand");
    ASSERT_EQ(weapon.next, THING_ENDOFLIST,
              "mixed-type pickup detaches the picked weapon head");
    ASSERT_EQ(containers[0].slot, potionThing,
              "mixed-type pickup promotes the potion tail to container head");
    ASSERT_EQ(potion.next, junkThing,
              "mixed-type pickup preserves potion-to-junk tail order");
    ASSERT_EQ(junk.next, THING_ENDOFLIST,
              "mixed-type pickup leaves junk tail terminating");
    assert_world_hash_matches(&state,
                              "mixed-type pickup refreshes deterministic world hash");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(containers[0].slot, potionThing,
              "close after mixed-type pickup keeps potion as head");
    ASSERT_EQ(potion.next, junkThing,
              "close after mixed-type pickup preserves generic potion next link");
    ASSERT_EQ(junk.next, THING_ENDOFLIST,
              "close after mixed-type pickup preserves generic junk terminator");
}

static void test_open_chest_close_trims_to_eight_visible_slots(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[9];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[9];
    int i;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 9;
    things.containers = containers;
    things.containerCount = 1;

    for (i = 0; i < 9; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2;
        weapons[i].next = (i < 8) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before visible-slot close rewrite");
    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "closing the source chest clears open panel state");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "close rewrite keeps the first visible chest object as head");
    for (i = 0; i < 7; ++i) {
        ASSERT_EQ(weapons[i].next, weaponThings[i + 1],
                  "close rewrite preserves visible C537..C543 order");
    }
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "close rewrite terminates after visible C544");
    ASSERT_EQ(weapons[8].next, THING_ENDOFLIST,
              "ninth linked object is detached because it never entered G0425");
}

static void test_open_chest_keeps_ninth_visible_chain_intact(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[12];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[12];
    int i;

    /* ReDMCSB CHEST.C F0333 lines 58-75 only walks the first 8 things
     * into G0425_aT_ChestSlots and leaves the 9th-and-later tail
     * untouched in the source list.  This regression proves the open
     * path does NOT call F0163_DUNGEON_LinkThingToList or otherwise
     * re-link the chain, so the hidden tail stays reachable through
     * the 8th visible item's next pointer until close rewrites it. */
    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 12;
    things.containers = containers;
    things.containerCount = 1;

    for (i = 0; i < 12; ++i) {
        unsigned short nextThing = (i < 11)
            ? (unsigned short)((THING_TYPE_WEAPON << 10) | (i + 1))
            : THING_ENDOFLIST;
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2;
        weapons[i].next = nextThing;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before chain-intact probe");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "open panel state records the source container thing");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "open does not rewrite the source container head pointer");
    for (i = 0; i < 11; ++i) {
        ASSERT_EQ(weapons[i].next, weaponThings[i + 1],
                  "open leaves the original 1..11 -> 2..12 chain intact");
    }
    ASSERT_EQ(weapons[7].next, weaponThings[8],
              "open keeps the 8th visible item linked to the 9th tail item");
    ASSERT_EQ(weapons[10].next, weaponThings[11],
              "open keeps the 11th item linked to the 12th tail item");
    ASSERT_EQ(weapons[11].next, THING_ENDOFLIST,
              "open leaves the 12th tail item terminating the chain");

    M11_GameView_CloseV1OpenChest(&state);
    {
        unsigned short probe = containers[0].slot;
        int reached8 = 0, reached9 = 0, reached10 = 0, reached11 = 0, reached12 = 0;
        int steps = 0;
        while (probe != THING_ENDOFLIST && probe != THING_NONE && steps < 16) {
            if (probe == weaponThings[7]) reached8 = 1;
            if (probe == weaponThings[8]) reached9 = 1;
            if (probe == weaponThings[9]) reached10 = 1;
            if (probe == weaponThings[10]) reached11 = 1;
            if (probe == weaponThings[11]) reached12 = 1;
            if (THING_GET_TYPE(probe) != THING_TYPE_WEAPON) break;
            probe = weapons[THING_GET_INDEX(probe)].next;
            ++steps;
        }
        ASSERT_EQ(reached8, 1, "close rewrite keeps 8th visible object reachable");
        ASSERT_EQ(reached9, 0, "close rewrite detaches 9th tail object from head");
        ASSERT_EQ(reached10, 0, "close rewrite detaches 10th tail object from head");
        ASSERT_EQ(reached11, 0, "close rewrite detaches 11th tail object from head");
        ASSERT_EQ(reached12, 0, "close rewrite detaches 12th tail object from head");
    }
}

static void test_open_chest_pickup_last_visible_slot_detaches_tail(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[9];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[9];
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int i;

    /* ReDMCSB CHEST.C F0334 lines 112-133 only re-links G0425_aT_ChestSlots
     * on close, so picking up the last visible C544 slot from a chest
     * that holds more than eight items detaches the 9th tail item the
     * same way that close does.  m11_process_v1_chest_slot_box_click
     * nulls slotThing->next before promoting it to the leader hand,
     * so the tail becomes unreachable. */
    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 9;
    things.containers = containers;
    things.containerCount = 1;

    for (i = 0; i < 9; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2;
        weapons[i].next = (i < 8) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before last-slot pickup probe");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(7, &sx, &sy, &sw, &sh),
                "C544 last chest slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C544 picks the last visible chest object");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), weaponThings[7],
              "last-slot pickup moves the 8th visible object to leader hand");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "last-slot pickup detaches the picked 8th item from the chain");
    ASSERT_EQ(weapons[8].next, THING_ENDOFLIST,
              "last-slot pickup detaches the 9th tail item from the chain");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "close after last-slot pickup keeps first object as head");
    for (i = 0; i < 6; ++i) {
        ASSERT_EQ(weapons[i].next, weaponThings[i + 1],
                  "close after last-slot pickup preserves visible C537..C542 order");
    }
    ASSERT_EQ(weapons[6].next, THING_ENDOFLIST,
              "close after last-slot pickup terminates chain at C543");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "close after last-slot pickup keeps 8th item detached");
    ASSERT_EQ(weapons[8].next, THING_ENDOFLIST,
              "close after last-slot pickup keeps 9th tail item detached");
}

static void test_open_chest_last_visible_swap_rewrites_hidden_tail(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[10];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[10];
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int i;

    /* ReDMCSB CHEST.C F0333 lines 58-75 loads only the first eight linked
     * things into G0425_aT_ChestSlots.  CHAMPION.C F0302 lines 688-710
     * swaps the leader-hand thing with the selected C30+ chest slot, then
     * F0334 lines 117-129 / DUNGEON.C F0163 lines 1796-1837 rewrite only
     * those visible slots, so replacing C544 in an overfull chest must drop
     * the hidden ninth tail from the rewritten source list. */
    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 10;
    things.containers = containers;
    things.containerCount = 1;

    for (i = 0; i < 10; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2;
        weapons[i].next = (i < 8) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before C544 replacement rewrite");
    ASSERT_EQ(weapons[7].next, weaponThings[8],
              "open chest still links visible C544 to hidden ninth tail before mutation");
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, weaponThings[9]), 1,
              "leader hand accepts a container-compatible C544 replacement object");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(7, &sx, &sy, &sw, &sh),
                "C544 last visible chest slot zone exists for replacement rewrite");

    state.lastWorldHash = 0xBADF00Du;
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C544 swaps the visible item with the leader-hand replacement");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), weaponThings[7],
              "C544 replacement moves the old eighth visible item to leader hand");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "C544 replacement keeps the first visible object as chest head");
    for (i = 0; i < 6; ++i) {
        ASSERT_EQ(weapons[i].next, weaponThings[i + 1],
                  "C544 replacement preserves visible C537..C542 order");
    }
    ASSERT_EQ(weapons[6].next, weaponThings[9],
              "C544 replacement links C543 directly to the leader-hand replacement");
    ASSERT_EQ(weapons[9].next, THING_ENDOFLIST,
              "C544 replacement terminates the rewritten visible list");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "C544 replacement detaches the picked eighth visible object");
    ASSERT_EQ(weapons[8].next, THING_ENDOFLIST,
              "C544 replacement leaves the hidden ninth tail detached");
    assert_world_hash_matches(&state,
                              "C544 replacement refreshes deterministic world hash");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "close after C544 replacement clears panel state");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "close after C544 replacement keeps first visible object as head");
    ASSERT_EQ(weapons[6].next, weaponThings[9],
              "close after C544 replacement keeps replacement as eighth object");
    ASSERT_EQ(weapons[9].next, THING_ENDOFLIST,
              "close after C544 replacement terminates at the replacement object");
    ASSERT_EQ(weapons[7].next, THING_ENDOFLIST,
              "close after C544 replacement keeps old C544 item detached");
    ASSERT_EQ(weapons[8].next, THING_ENDOFLIST,
              "close after C544 replacement keeps hidden ninth tail detached");
}

static void test_action_hand_chest_panel_state_follows_slot_clicks(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
    unsigned short axeThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);
    int sx = 0, sy = 0, sw = 0, sh = 0;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 2;
    things.containers = containers;
    things.containerCount = 1;
    weapons[0].type = 2;
    weapons[0].next = axeThing;
    weapons[1].type = 2;
    weapons[1].next = THING_ENDOFLIST;
    containers[0].slot = daggerThing;

    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "open action-hand chest before action-hand removal");
    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(9, &sx, &sy, &sw, &sh),
                "C508 action-hand source slot zone exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "clicking C508 picks up the open action-hand chest");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "removing the open action-hand chest clears panel state");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND], THING_NONE,
              "action hand is empty after picking up the chest");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), chestThing,
              "removed chest moves to the leader hand");
    ASSERT_EQ(containers[0].slot, daggerThing,
              "closing action-hand panel state preserves container list head");
    ASSERT_EQ(weapons[0].next, axeThing,
              "closing action-hand panel state preserves linked contents");

    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &sx, &sy, &sw, &sh),
                "C537 chest source slot zone exists after close");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_IGNORED,
              "closed chest panel ignores stale C537 clicks");

    ASSERT_TRUE(M11_GameView_GetV1InventorySourceSlotBoxZone(9, &sx, &sy, &sw, &sh),
                "C508 action-hand source slot zone still exists");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, sx + sw / 2, 33 + sy + sh / 2, 1),
              M11_GAME_INPUT_REDRAW,
              "placing the leader-hand chest into C508 reopens panel state");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), chestThing,
              "placing a container in the action hand opens its chest panel");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "placing the chest clears the leader hand");
    ASSERT_EQ(state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND], chestThing,
              "action hand stores the placed chest");
}

static void test_action_hand_open_chest_icon_runtime(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonContainer_Compat container;
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    memset(&container, 0, sizeof(container));
    things.containers = &container;
    things.containerCount = 1;
    container.type = 0;
    container.slot = THING_ENDOFLIST;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND), 144,
              "closed action-hand container uses source C144 before panel open");
    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "opening action-hand chest succeeds for icon remap");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND), 145,
              "open action-hand chest remaps source icon C144 to C145");
    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(&state, CHAMPION_SLOT_ACTION_HAND), 144,
              "closed panel restores action-hand chest icon C144");
}

static void test_object_description_layout_source_zones(void) {
    int x = -1, y = -1, w = -1, h = -1;
    const char* evidence = M11_GameView_GetV1ObjectDescriptionLayoutEvidence();

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionPanelGraphicId(), 20,
              "object description uses C020 panel-empty graphic");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleGraphicId(), 29,
              "object description uses C029 circle graphic");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleZoneId(), 504,
              "object description circle source zone is C504");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionCircleZone(&x, &y, &w, &h), 1,
              "C504 circle zone resolves");
    ASSERT_EQ(x, 103, "C504 circle x follows layout-696 F0635");
    ASSERT_EQ(y, 53, "C504 circle y follows layout-696 F0635");
    ASSERT_EQ(w, 32, "C029 circle width resolves to 32 pixels");
    ASSERT_EQ(h, 27, "C029 circle height resolves to 27 pixels");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionIconZoneId(), 505,
              "object description icon source zone is C505");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionIconZone(&x, &y, &w, &h), 1,
              "C505 icon zone resolves");
    ASSERT_EQ(x, 111, "C505 icon x follows layout-696 F0635");
    ASSERT_EQ(y, 59, "C505 icon y follows layout-696 F0635");
    ASSERT_EQ(w, 16, "C505 icon width is one object-icon cell");
    ASSERT_EQ(h, 16, "C505 icon height is one object-icon cell");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionNameZoneId(), 506,
              "object description name source zone is C506");
    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionNameZoneForText(80, 7, &x, &y, &w, &h), 1,
              "C506 text zone resolves for an 80x7 measured name");
    ASSERT_EQ(x, 134, "C506 name x follows TEXT.C/F0635");
    ASSERT_EQ(y, 64, "C506 name top follows type-8 vertical centering");
    ASSERT_EQ(w, 80, "C506 keeps caller-measured text width");
    ASSERT_EQ(h, 7, "C506 keeps caller-measured text height");

    ASSERT_EQ(M11_GameView_GetV1ObjectDescriptionContinuationOrigin(&x, &y), 1,
              "C556 continuation text origin resolves");
    ASSERT_EQ(x, 108, "C556 form-feed x includes one-pixel margin");
    ASSERT_EQ(y, 59, "C556 form-feed y includes one-pixel margin");

    ASSERT_TRUE(evidence && strstr(evidence, "PANEL.C:1136-1145") != NULL,
                "layout evidence cites object-description panel blits");
    ASSERT_TRUE(strstr(evidence, "TEXT.C:1937-1950") != NULL,
                "layout evidence cites C506 text zone resolver");
    ASSERT_TRUE(strstr(evidence, "COORD.C:2052-2412") != NULL,
                "layout evidence cites F0635 layout resolver");
}

static void test_eye_panel_weapon_attribute_flags(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned char framebuffer[320 * 200];
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    weapon.type = 8;
    weapon.cursed = 1;
    weapon.poisoned = 1;
    weapon.broken = 1;
    weapon.chargeCount = 7;

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, daggerThing), 1,
              "leader hand accepts source weapon thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source weapon description");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "leader-hand eye click marks object-description panel active");
    ASSERT_EQ(state.v1ObjectDescriptionThing, daggerThing,
              "object-description panel is locked to the leader-hand thing");
    ASSERT_EQ(state.v1ObjectDescriptionIconIndex, 32,
              "object-description panel uses the source object icon index");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_EQ(framebuffer[(33 + 52) * 320 + 80], 3,
              "drawn object-description panel border starts at source C101");
    ASSERT_EQ(framebuffer[(33 + 53) * 320 + 103], 12,
              "drawn object-description circle starts at source C504");
    ASSERT_EQ(framebuffer[(33 + 59) * 320 + 111], 13,
              "drawn object-description icon fallback is anchored at source C505");
    ASSERT_EQ(framebuffer[(33 + 64) * 320 + 134], 13,
              "drawn object-description name text starts in source C506");
    ASSERT_TRUE(strstr(state.inspectDetail, "CURSED") != NULL,
                "weapon eye panel reports source cursed flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "POISONED") != NULL,
                "weapon eye panel reports source poisoned flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "BROKEN") != NULL,
                "weapon eye panel reports source broken flag");
    ASSERT_TRUE(strstr(state.inspectDetail, "CHARGE 7") != NULL,
                "weapon eye panel reports source charge count");
}

static void test_leader_hand_weapon_eye_blits_source_object_description_pixels(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    unsigned char framebuffer[320 * 200];
    unsigned short daggerThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);

    seed_inventory_view(&state, &things, &weapon);
    weapon.type = 8;
    weapon.next = THING_ENDOFLIST;

    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source object-description blits");
    state.assetsAvailable = 1;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, daggerThing), 1,
              "leader hand accepts source weapon thing for object-description pixel gate");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click routes weapon to object-description panel");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "weapon eye route marks object-description panel active");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "weapon eye route does not use the scroll panel route");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "weapon eye route does not use the chest panel route");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(framebuffer_matches_object_description_source_pixels(&state, framebuffer),
                "leader-hand weapon eye render blits source C020 panel and C029 circle into C101/C504");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_eye_panel_potion_power_prefix_runtime(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct DungeonPotion_Compat potions[3];
    unsigned short rosPotionThing = (unsigned short)((THING_TYPE_POTION << 10) | 0);
    unsigned short waterFlaskThing = (unsigned short)((THING_TYPE_POTION << 10) | 1);
    unsigned short emptyFlaskThing = (unsigned short)((THING_TYPE_POTION << 10) | 2);

    seed_inventory_view(&state, &things, &weapon);
    memset(potions, 0, sizeof(potions));
    things.potions = potions;
    things.potionCount = 3;
    potions[0].type = 6;   /* ROS POTION: DUNGEON.C object-info index 8, icon C154. */
    potions[0].power = 80; /* PANEL.C:1184 => '_' + 2 == 'a'. */
    potions[1].type = 15;  /* WATER FLASK: DUNGEON.C object-info index 17, icon C163. */
    potions[1].power = 120;
    potions[2].type = 20;  /* EMPTY FLASK: DUNGEON.C object-info index 22, icon C195. */
    potions[2].power = 0;

    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_PRIEST] = 2;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 500;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, rosPotionThing), 1,
              "leader hand accepts source ROS potion thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source ROS potion description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: a ROS POTION") != NULL,
                "priest skill > 1 prefixes non-water potion name in eye panel");
    ASSERT_TRUE(strstr(state.inspectDetail, "PANEL a ROS POTION") != NULL,
                "runtime potion detail carries source PANEL.C description text");
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss ROS potion eye-panel overlay");

    state.world.party.champions[0].skillLevels[CHAMPION_SKILL_PRIEST] = 4;
    state.world.lifecycle.champions[0]
        .skills20[DM1_SKILL_IDX_PRIEST].experience = 2000;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, waterFlaskThing), 1,
              "leader hand accepts source water flask thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source water flask description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: WATER FLASK") != NULL,
                "water flask is excluded from the priest-skill power prefix");
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss water flask eye-panel overlay");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, emptyFlaskThing), 1,
              "leader hand accepts source empty flask thing");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click opens source empty flask description");
    ASSERT_TRUE(strstr(state.inspectTitle, "POTION: _ EMPTY FLASK") != NULL,
                "empty flask keeps the original non-water potion prefix quirk");
}

static void test_champion_statistic_maximum_row_runtime_state(void) {
    struct ChampionState_Compat champ;
    struct ChampionState_Compat roundTrip;
    unsigned char buf[CHAMPION_SERIALIZED_SIZE];
    unsigned short current = 0;
    unsigned short maximum = 0;

    F0600_CHAMPION_InitEmpty_Compat(&champ);
    champ.attributes[CHAMPION_ATTR_STRENGTH] = 41;
    champ.attributeMaximums[CHAMPION_ATTR_STRENGTH] = 50;
    champ.attributes[CHAMPION_ATTR_DEXTERITY] = 42;
    for (int i = 0; i < CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT; ++i) {
        champ.portraitBitmap[i] = (unsigned char)((i * 5 + 3) & 0xff);
    }
    champ.portraitBitmapValid = 1;

    ASSERT_EQ(F0677_CHAMPION_GetAttributeStatisticRow_Compat(
                  &champ, CHAMPION_ATTR_STRENGTH, &current, &maximum),
              1,
              "champion statistic row helper accepts strength");
    ASSERT_EQ(current, 41, "champion statistic row exposes current value");
    ASSERT_EQ(maximum, 50, "champion statistic row exposes maximum value");

    ASSERT_EQ(F0677_CHAMPION_GetAttributeStatisticRow_Compat(
                  &champ, CHAMPION_ATTR_DEXTERITY, &current, &maximum),
              1,
              "champion statistic row helper accepts legacy zero maximum");
    ASSERT_EQ(current, 42, "champion statistic fallback keeps current");
    ASSERT_EQ(maximum, 42, "champion statistic fallback uses current as maximum");

    ASSERT_EQ(F0602_CHAMPION_Serialize_Compat(&champ, buf, sizeof(buf)),
              CHAMPION_SERIALIZED_SIZE,
              "champion statistic maximum row serializes");
    ASSERT_EQ(F0603_CHAMPION_Deserialize_Compat(&roundTrip, buf, sizeof(buf)),
              CHAMPION_SERIALIZED_SIZE,
              "champion statistic maximum row deserializes");
    ASSERT_EQ(F0677_CHAMPION_GetAttributeStatisticRow_Compat(
                  &roundTrip, CHAMPION_ATTR_STRENGTH, &current, &maximum),
              1,
              "champion statistic round-trip row helper accepts strength");
    ASSERT_EQ(current, 41, "champion statistic round-trip current value");
    ASSERT_EQ(maximum, 50, "champion statistic round-trip maximum value");
    ASSERT_EQ(roundTrip.portraitBitmapValid, 1,
              "champion portrait bitmap validity round-trips");
    ASSERT_EQ(memcmp(roundTrip.portraitBitmap, champ.portraitBitmap,
                     CHAMPION_PORTRAIT_BITMAP_BYTE_COUNT),
              0,
              "champion portrait bitmap bytes round-trip");
}

static void test_open_chest_all_eight_slot_mouse_routes_and_pickup(void) {
    /* ReDMCSB COMMAND.C:498-507 G0456_as_Graphic561_MouseInput_PanelChest
     * defines the eight C058..C065 commands whose viewport-relative slot
     * boxes C537..C544 zigzag across the open chest panel C106 child
     * zones from layout-696.  Slots 0..2 sit at y=59, 76, 93 and the
     * last five are tightly stacked at y=98, 101, 103, 104, 105.  This
     * regression exercises all eight so future tweaks to kV1ChestSlotBoxZones
     * or the M11_DM1_MOUSE_LIST_INVENTORY routing table cannot silently
     * remap a chest slot to a wrong command/zone pair.
     *
     * Ref: ReDMCSB COMMAND.C:498-507, CHEST.C F0333:58-75,
     *      CHEST.C F0334:112-133, PANEL.C F0347:1651-1691. */
    static const int kSourceCommand[8] = { 58, 59, 60, 61, 62, 63, 64, 65 };
    static const int kSourceZoneId[8]  = { 537, 538, 539, 540, 541, 542, 543, 544 };
    static const int kExpectedX[8]      = { 117, 106, 111, 128, 145, 162, 179, 196 };
    static const int kExpectedY[8]      = {  59,  76,  93,  98, 101, 103, 104, 105 };
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[8];
    struct DungeonContainer_Compat containers[1];
    unsigned short chestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 0);
    unsigned short weaponThings[8];
    int sx = 0, sy = 0, sw = 0, sh = 0;
    int space = 0, zone = 0;
    int i;

    seed_inventory_view(&state, &things, &weapons[0]);
    memset(weapons, 0, sizeof(weapons));
    memset(containers, 0, sizeof(containers));
    things.weapons = weapons;
    things.weaponCount = 8;
    things.containers = containers;
    things.containerCount = 1;
    for (i = 0; i < 8; ++i) {
        weaponThings[i] = (unsigned short)((THING_TYPE_WEAPON << 10) | i);
        weapons[i].type = 2; /* object-info index 25: container-compatible. */
        weapons[i].next = (i < 7) ? weaponThings[i + 1] : THING_ENDOFLIST;
    }
    containers[0].slot = weaponThings[0];
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = chestThing;

    ASSERT_EQ(M11_GameView_OpenV1ActionHandChest(&state), 1,
              "action-hand chest opens before all-eight-slot mouse route probe");

    for (i = 0; i < 8; ++i) {
        int clickX = 0, clickY = 0;
        ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(i, &sx, &sy, &sw, &sh),
                    "C537..C544 chest slot zone exists for every ordinal");
        /* The runtime stores zone coordinates in viewport-relative form
         * (parent zone C101) but M11_GameView_GetV1MouseCommandForPoint
         * takes screen-relative coordinates, so the y origin must add
         * M11_VIEWPORT_Y=33 (DM1 viewport y offset). */
        ASSERT_EQ(sx, kExpectedX[i],
                  "C537..C544 zone x matches the source layout-696 chest panel C106 child");
        ASSERT_EQ(sy, kExpectedY[i],
                  "C537..C544 zone y matches the source layout-696 chest panel C106 child");
        clickX = sx + sw / 2;
        clickY = 33 + sy + sh / 2;
        ASSERT_EQ(M11_GameView_GetV1MouseCommandForPoint(M11_DM1_MOUSE_LIST_INVENTORY,
                                                         clickX,
                                                         clickY,
                                                         M11_DM1_MOUSE_MASK_LEFT,
                                                         &space,
                                                         &zone),
                  kSourceCommand[i],
                  "C537..C544 click resolves to the source C058..C065 mouse command");
        ASSERT_EQ(zone, kSourceZoneId[i],
                  "C537..C544 click returns the source C537..C544 zone id");
    }

    /* Pick the seventh visible slot (C544) — chest slots 3..7 are 1-2
     * pixels apart vertically, so a wrong remap could mis-pick the
     * sixth item while still claiming C544 routed correctly. */
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(6, &sx, &sy, &sw, &sh),
                "C543 chest slot zone exists for late-slot pickup probe");
    ASSERT_EQ(M11_GameView_HandlePointer(&state,
                                         sx + sw / 2,
                                         33 + sy + sh / 2,
                                         1),
              M11_GAME_INPUT_REDRAW,
              "clicking C543 picks the seventh visible chest object");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), weaponThings[6],
              "C543 pickup moves the seventh visible chest object to leader hand");
    ASSERT_EQ(weapons[5].next, weaponThings[7],
              "C543 pickup links slot 6 around the empty seventh slot");
    ASSERT_EQ(weapons[6].next, THING_ENDOFLIST,
              "C543 pickup detaches the picked object from the chest chain");

    M11_GameView_CloseV1OpenChest(&state);
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), THING_NONE,
              "closing the panel after all-eight-slot probe clears chest state");
    ASSERT_EQ(containers[0].slot, weaponThings[0],
              "close rewrite keeps the first visible chest object as the head");
    /* CHEST.C F0334:112-133 only re-emits the first 8 visible slots, so
     * after the C543 pickup the close writeback must yield a 7-item list. */
    ASSERT_EQ(weapons[6].next, THING_ENDOFLIST,
              "close writeback terminates after the seventh surviving visible item");
}

static void test_eye_panel_champion_stats_and_skills(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapon;
    struct ChampionState_Compat* champ;
    unsigned char framebuffer[320 * 200];
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;

    seed_inventory_view(&state, &things, &weapon);
    champ = &state.world.party.champions[0];
    champ->hp.current = 77;
    champ->hp.maximum = 100;
    champ->stamina.current = 66;
    champ->stamina.maximum = 90;
    champ->mana.current = 12;
    champ->mana.maximum = 33;
    champ->attributes[CHAMPION_ATTR_STRENGTH] = 41;
    champ->attributes[CHAMPION_ATTR_DEXTERITY] = 52;
    champ->attributes[CHAMPION_ATTR_WISDOM] = 43;
    champ->attributes[CHAMPION_ATTR_VITALITY] = 44;
    champ->attributes[CHAMPION_ATTR_ANTIMAGIC] = 45;
    champ->attributes[CHAMPION_ATTR_ANTIFIRE] = 46;
    champ->attributeMaximums[CHAMPION_ATTR_STRENGTH] = 50;
    champ->attributeMaximums[CHAMPION_ATTR_DEXTERITY] = 50;
    champ->attributeMaximums[CHAMPION_ATTR_WISDOM] = 43;
    champ->attributeMaximums[CHAMPION_ATTR_VITALITY] = 44;
    champ->attributeMaximums[CHAMPION_ATTR_ANTIMAGIC] = 45;
    champ->attributeMaximums[CHAMPION_ATTR_ANTIFIRE] = 46;
    champ->skillLevels[CHAMPION_SKILL_FIGHTER] = 2;
    champ->skillLevels[CHAMPION_SKILL_NINJA] = 3;
    champ->skillLevels[CHAMPION_SKILL_PRIEST] = 4;
    champ->skillLevels[CHAMPION_SKILL_WIZARD] = 5;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "inventory eye click with empty leader hand opens champion stats");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 1,
              "empty-hand eye click marks the drawn champion stats panel active");
    ASSERT_TRUE(strstr(state.inspectDetail, "MANA 12/33") != NULL,
                "champion stats panel reports mana");
    ASSERT_TRUE(strstr(state.inspectDetail, "STR  41/ 50") != NULL &&
                strstr(state.inspectDetail, "DEX  52/ 50") != NULL &&
                strstr(state.inspectDetail, "WIS  43/ 43") != NULL &&
                strstr(state.inspectDetail, "VIT  44/ 44") != NULL,
                "champion stats panel reports current/max statistic rows");
    ASSERT_TRUE(strstr(state.inspectDetail, "STRENGTH") != NULL &&
                strstr(state.inspectDetail, "DEXTERITY") != NULL &&
                strstr(state.inspectDetail, "WISDOM") != NULL &&
                strstr(state.inspectDetail, "VITALITY") != NULL,
                "champion stats panel reports source statistic names");
    ASSERT_TRUE(strstr(state.inspectDetail, "AM  45/ 45") != NULL &&
                strstr(state.inspectDetail, "AF  46/ 46") != NULL,
                "champion stats panel reports anti-magic and anti-fire current/max rows");
    ASSERT_TRUE(strstr(state.inspectDetail, "ANTI-MAGIC") != NULL &&
                strstr(state.inspectDetail, "ANTI-FIRE") != NULL,
                "champion stats panel reports source anti-statistic names");
    ASSERT_TRUE(strstr(state.inspectDetail, "FTR 2") != NULL &&
                strstr(state.inspectDetail, "NIN 3") != NULL &&
                strstr(state.inspectDetail, "PRI 4") != NULL &&
                strstr(state.inspectDetail, "WIZ 5") != NULL,
                "champion stats panel reports base skill levels");
    ASSERT_TRUE(strstr(state.inspectDetail, "NEOPHYTE FIGHTER") != NULL &&
                strstr(state.inspectDetail, "NOVICE NINJA") != NULL &&
                strstr(state.inspectDetail, "APPRENTICE PRIEST") != NULL &&
                strstr(state.inspectDetail, "JOURNEYMAN WIZARD") != NULL,
                "champion stats panel reports source skill level names");
    {
        DM1_ChampionPanel_StatisticTextRunModel strengthRun;
        DM1_ChampionPanel_StatisticTextRunModel dexterityRun;
        DM1_ChampionPanel_StatisticTextRunModel wisdomRun;

        ASSERT_EQ(DM1_ChampionPanel_BuildStatisticTextRunModel(
                      CHAMPION_ATTR_STRENGTH,
                      champ->attributes[CHAMPION_ATTR_STRENGTH],
                      champ->attributeMaximums[CHAMPION_ATTR_STRENGTH],
                      &strengthRun),
                  1,
                  "champion stats render helper accepts strength row");
        ASSERT_EQ(strengthRun.currentColor, DM1_COLOR_RED,
                  "champion stats render helper colors below-max current value red");
        ASSERT_EQ(strengthRun.maximumColor, DM1_COLOR_LIGHTEST_GRAY,
                  "champion stats render helper keeps maximum suffix gray");
        ASSERT_EQ(strengthRun.currentX, DM1_STATISTIC_CURRENT_REL_X,
                  "champion stats render helper source-locks current value x");
        ASSERT_EQ(strengthRun.maximumX,
                  DM1_STATISTIC_CURRENT_REL_X + DM1_PANEL_TEXT_CHAR_WIDTH * 3,
                  "champion stats render helper source-locks maximum suffix x");
        ASSERT_EQ(strengthRun.y, DM1_STATISTIC_FIRST_REL_Y,
                  "champion stats render helper source-locks first statistic y");

        ASSERT_EQ(DM1_ChampionPanel_BuildStatisticTextRunModel(
                      CHAMPION_ATTR_DEXTERITY,
                      champ->attributes[CHAMPION_ATTR_DEXTERITY],
                      champ->attributeMaximums[CHAMPION_ATTR_DEXTERITY],
                      &dexterityRun),
                  1,
                  "champion stats render helper accepts dexterity row");
        ASSERT_EQ(dexterityRun.currentColor, DM1_COLOR_LIGHT_GREEN,
                  "champion stats render helper colors above-max current value light green");
        ASSERT_EQ(dexterityRun.y, DM1_STATISTIC_FIRST_REL_Y + DM1_PANEL_TEXT_LINE_HEIGHT,
                  "champion stats render helper advances row y by source text height");

        ASSERT_EQ(DM1_ChampionPanel_BuildStatisticTextRunModel(
                      CHAMPION_ATTR_WISDOM,
                      champ->attributes[CHAMPION_ATTR_WISDOM],
                      champ->attributeMaximums[CHAMPION_ATTR_WISDOM],
                      &wisdomRun),
                  1,
                  "champion stats render helper accepts wisdom row");
        ASSERT_EQ(wisdomRun.currentColor, DM1_COLOR_LIGHTEST_GRAY,
                  "champion stats render helper colors equal current value gray");
    }

    memset(framebuffer, 0xEE, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH),
                "champion stats pixel test resolves C101 panel zone");
    ASSERT_EQ(framebuffer[(33 + panelY + DM1_STATISTIC_FIRST_REL_Y) * 320 +
                          (panelX + DM1_STATISTIC_CURRENT_REL_X + 9)],
              DM1_COLOR_RED,
              "drawn strength current digit pixel is red below maximum");
    ASSERT_EQ(framebuffer[(33 + panelY + DM1_STATISTIC_FIRST_REL_Y + DM1_PANEL_TEXT_LINE_HEIGHT) * 320 +
                          (panelX + DM1_STATISTIC_CURRENT_REL_X + 6)],
              DM1_COLOR_LIGHT_GREEN,
              "drawn dexterity current digit pixel is green above maximum");
    ASSERT_EQ(framebuffer[(33 + panelY + DM1_STATISTIC_FIRST_REL_Y + 2 * DM1_PANEL_TEXT_LINE_HEIGHT) * 320 +
                          (panelX + DM1_STATISTIC_CURRENT_REL_X + 9)],
              DM1_COLOR_LIGHTEST_GRAY,
              "drawn wisdom current digit pixel is gray at maximum");
    ASSERT_EQ(framebuffer[(33 + panelY + DM1_STATISTIC_FIRST_REL_Y) * 320 +
                          (panelX + DM1_STATISTIC_CURRENT_REL_X + DM1_PANEL_TEXT_CHAR_WIDTH * 3 + 4)],
              DM1_COLOR_LIGHTEST_GRAY,
              "drawn statistic maximum slash pixel is gray");
    ASSERT_EQ(framebuffer[(33 + panelY + DM1_STATISTIC_FIRST_REL_Y) * 320 +
                          (panelX + DM1_STATISTIC_NAME_REL_X + 1)],
              DM1_COLOR_LIGHTEST_GRAY,
              "drawn statistic name pixel is source gray");
}

int main(void) {
    printf("=== M11 Inventory Full Panel Runtime Source-Lock Gate ===\n");
    printf("ReDMCSB: DEFS.H 743-760,778-817, DATA.C 1049-1087, CHAMPION.C F0302 677-712, CHEST.C F0333 58-75, F0334 112-133, DUNGEON.C F0163 1796-1837, PANEL.C F0347 1651-1691, F0351 1965-2108, F0352 2111-2160\n\n");

    test_extended_backpack_source_mapping();
    test_extended_backpack_runtime_clicks();
    test_all_backpack_source_slots_round_trip_runtime();
    test_open_chest_runtime_routes_and_clicks();
    test_open_chest_empty_slot_empty_hand_noops();
    test_open_chest_late_empty_slot_placement_promotes_on_close();
    test_open_chest_occupied_slot_swap_preserves_visible_order();
    test_open_chest_rejects_incompatible_leader_hand_without_mutation();
    test_leader_hand_container_eye_routes_to_chest_panel();
    test_open_chest_slot_box_and_icon_source_pixels();
    test_open_chest_panel_red_transparency_preserves_inventory_backdrop();
    test_open_chest_second_visible_slot_uses_second_object_icon();
    test_open_chest_third_visible_slot_uses_third_object_icon();
    test_open_chest_fourth_visible_slot_uses_fourth_object_icon();
    test_open_chest_fifth_visible_slot_uses_fifth_object_icon();
    test_open_chest_sixth_visible_slot_uses_sixth_object_icon();
    test_open_chest_seventh_visible_slot_uses_seventh_object_icon();
    test_open_chest_eighth_visible_slot_uses_eighth_object_icon();
    test_empty_hand_mouth_blits_source_food_water_panel_pixels();
    test_open_chest_middle_pickup_compacts_visible_list();
    test_open_chest_pickup_preserves_mixed_type_tail_order();
    test_open_chest_close_trims_to_eight_visible_slots();
    test_open_chest_keeps_ninth_visible_chain_intact();
    test_open_chest_pickup_last_visible_slot_detaches_tail();
    test_open_chest_last_visible_swap_rewrites_hidden_tail();
    test_action_hand_chest_panel_state_follows_slot_clicks();
    test_action_hand_open_chest_icon_runtime();
    test_eye_panel_potion_power_prefix_runtime();
    test_object_description_layout_source_zones();
    test_eye_panel_weapon_attribute_flags();
    test_leader_hand_weapon_eye_blits_source_object_description_pixels();
    test_champion_statistic_maximum_row_runtime_state();
    test_eye_panel_champion_stats_and_skills();
    test_open_chest_all_eight_slot_mouse_routes_and_pickup();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
