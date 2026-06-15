/*
 * Source-lock gate for M11 inventory scroll panel rendering.
 *
 * ReDMCSB evidence:
 *   PANEL.C F0347 lines 1658-1691: action-hand scroll selects scroll panel
 *   PANEL.C F0342 lines 1126-1131: scroll object dispatches to F0341
 *   PANEL.C F0341 lines 890-895: decodes Scroll.TextStringThingIndex
 *     with C2_TEXT_TYPE_SCROLL | MASK0x8000_DECODE_EVEN_IF_INVISIBLE
 *   PANEL.C F0341 lines 969-1043: blits C023 open-scroll panel into C101 with red transparency
 *     and renders each decoded newline-separated line
 *   PANEL.C F0352 lines 2124-2157: eye-click routes leader-hand objects
 *     through F0342 instead of opening a Firestaff dialog overlay
 *   PANEL.C F0342 lines 1126-1136 + CHEST.C F0333 lines 43-76:
 *     leader-hand containers open the C025 chest panel while pressing-eye
 *     suppresses the C09 action-hand open-chest icon write
 */

#include "m11_game_view.h"
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
    if (expr) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected %d, got %d\n", (msg), (int)(b), (int)(a)); } \
} while (0)

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected '%s', got '%s'\n", (msg), (b), (a)); } \
} while (0)

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

static int point_is_in_scroll_text_band(int y) {
    /* The source C023 panel blit happens before F0340 text rendering.  The
     * fallback text renderer may overdraw a wider band than the original font,
     * so compare the source bitmap outside the centered scroll text rows. */
    return y >= 28 && y < 56;
}

static int framebuffer_matches_open_scroll_panel_pixels(
    const M11_GameViewState* state,
    const unsigned char* framebuffer) {
    const M11_AssetSlot* panel;
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int matched = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetViewportRect(&viewportX, &viewportY, &viewportW, &viewportH) ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH)) {
        return 0;
    }
    (void)viewportW;
    (void)viewportH;
    panel = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                 (unsigned int)M11_GameView_GetV1OpenScrollPanelGraphicId());
    if (!panel || !panel->pixels ||
        panel->width != (unsigned short)panelW ||
        panel->height != (unsigned short)panelH) {
        return 0;
    }

    for (y = 0; y < panelH; ++y) {
        for (x = 0; x < panelW; ++x) {
            unsigned char want = panel->pixels[y * (int)panel->width + x];
            unsigned char got;
            if (want == 8 || point_is_in_scroll_text_band(y)) continue;
            got = framebuffer[(viewportY + panelY + y) * 320 + (viewportX + panelX + x)];
            if (got != want) return 0;
            matched++;
        }
    }
    return matched > 1000;
}

static int point_is_in_chest_slot_overdraw(const M11_GameViewState* state,
                                           int viewportLocalX,
                                           int viewportLocalY) {
    int i;
    (void)state;
    for (i = 0; i < M11_GameView_GetV1ChestSlotBoxZoneCount(); ++i) {
        int zx = 0, zy = 0, zw = 0, zh = 0;
        if (!M11_GameView_GetV1ChestSlotBoxZone(i, &zx, &zy, &zw, &zh)) {
            continue;
        }
        if (viewportLocalX >= zx - 1 && viewportLocalX < zx + zw + 1 &&
            viewportLocalY >= zy - 1 && viewportLocalY < zy + zh + 1) {
            return 1;
        }
    }
    return 0;
}

static int framebuffer_matches_open_chest_panel_pixels(
    const M11_GameViewState* state,
    const unsigned char* framebuffer) {
    const M11_AssetSlot* panel;
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int panelX = 0, panelY = 0, panelW = 0, panelH = 0;
    int ax = 0, ay = 0, aw = 0, ah = 0;
    int matched = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetViewportRect(&viewportX, &viewportY, &viewportW, &viewportH) ||
        !M11_GameView_GetV1InventoryPanelZone(&panelX, &panelY, &panelW, &panelH)) {
        return 0;
    }
    (void)viewportW;
    (void)viewportH;
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
            int viewportLocalX = panelX + x;
            int viewportLocalY = panelY + y;
            unsigned char want = panel->pixels[y * (int)panel->width + x];
            unsigned char got;
            if (want == 8 ||
                point_is_in_chest_slot_overdraw(state, viewportLocalX, viewportLocalY)) {
                continue;
            }
            if (viewportLocalX >= ax && viewportLocalX < ax + aw &&
                viewportLocalY >= ay && viewportLocalY < ay + ah) {
                continue;
            }
            got = framebuffer[(viewportY + viewportLocalY) * 320 +
                              (viewportX + viewportLocalX)];
            if (got != want) return 0;
            matched++;
        }
    }
    return matched > 1000;
}

static int framebuffer_matches_object_icon_in_chest_slot(
    const M11_GameViewState* state,
    const unsigned char* framebuffer,
    unsigned short thing,
    int chestSlotOrdinal) {
    const M11_AssetSlot* iconSheet;
    int iconIndex;
    int graphicIndex = 0;
    int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
    int viewportX = 0, viewportY = 0, viewportW = 0, viewportH = 0;
    int slotX = 0, slotY = 0, slotW = 0, slotH = 0;
    int matched = 0;
    int total = 0;
    int x, y;

    if (!state || !framebuffer ||
        !M11_GameView_GetViewportRect(&viewportX, &viewportY, &viewportW, &viewportH) ||
        !M11_GameView_GetV1ChestSlotBoxZone(chestSlotOrdinal, &slotX, &slotY,
                                            &slotW, &slotH)) {
        return 0;
    }
    (void)viewportW;
    (void)viewportH;
    if (slotW != 16 || slotH != 16) return 0;

    iconIndex = M11_GameView_GetObjectIconIndexForThing(state, thing);
    if (iconIndex < 0 ||
        !M11_GameView_GetV1ObjectIconSourceZone(iconIndex, &graphicIndex,
                                                &srcX, &srcY, &srcW, &srcH) ||
        srcW != 16 || srcH != 16) {
        return 0;
    }
    iconSheet = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                     (unsigned int)graphicIndex);
    if (!iconSheet || !iconSheet->pixels ||
        srcX + srcW > (int)iconSheet->width ||
        srcY + srcH > (int)iconSheet->height) {
        return 0;
    }

    for (y = 0; y < srcH; ++y) {
        for (x = 0; x < srcW; ++x) {
            unsigned char want =
                iconSheet->pixels[(srcY + y) * (int)iconSheet->width + srcX + x];
            unsigned char got;
            if (want == 12) continue;
            got = framebuffer[(viewportY + slotY + y) * 320 +
                              (viewportX + slotX + x)];
            total++;
            if (got == want) matched++;
        }
    }
    return total > 0 && matched == total;
}

static int framebuffer_zone_has_nonzero_pixel(const unsigned char* framebuffer,
                                              int framebufferWidth,
                                              int x,
                                              int y,
                                              int w,
                                              int h) {
    int px, py;
    if (!framebuffer || framebufferWidth <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (py = y; py < y + h; ++py) {
        for (px = x; px < x + w; ++px) {
            if (framebuffer[py * framebufferWidth + px] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

static unsigned short pack3(int a, int b, int c) {
    return (unsigned short)(((a & 31) << 10) | ((b & 31) << 5) | (c & 31));
}

static void seed_scroll_world(M11_GameViewState* state,
                              struct DungeonThings_Compat* things,
                              struct DungeonTextString_Compat* textStrings,
                              struct DungeonScroll_Compat* scrolls,
                              unsigned short* textData) {
    int i;
    memset(things, 0, sizeof(*things));

    textData[0] = pack3(14, 13, 4);  /* ONE */
    textData[1] = pack3(28, 19, 22); /* separator, TW */
    textData[2] = pack3(14, 31, 31); /* O, end */

    textStrings[0].next = THING_ENDOFLIST;
    textStrings[0].visible = 0;
    textStrings[0].textDataWordOffset = 0;

    scrolls[0].next = THING_ENDOFLIST;
    scrolls[0].textStringThingIndex = 0;
    scrolls[0].closed = 0;

    things->textData = textData;
    things->textDataWordCount = 3;
    things->textStrings = textStrings;
    things->textStringCount = 1;
    things->scrolls = scrolls;
    things->scrollCount = 1;

    state->active = 1;
    state->inventoryPanelActive = 1;
    state->showDebugHUD = 0;
    state->assetsAvailable = 0;
    state->originalFontAvailable = 0;
    state->world.things = things;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 100;
    state->world.party.champions[0].hp.maximum = 100;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
    state->world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        (unsigned short)(THING_TYPE_SCROLL << 10);
}

static void seed_chest_world(M11_GameViewState* state,
                             struct DungeonThings_Compat* things,
                             struct DungeonContainer_Compat* containers,
                             struct DungeonJunk_Compat* junks) {
    int i;
    memset(things, 0, sizeof(*things));
    memset(containers, 0, sizeof(containers[0]) * 2);
    memset(junks, 0, sizeof(junks[0]) * 2);

    containers[0].next = THING_ENDOFLIST;
    containers[0].slot = (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    containers[0].type = 0;
    containers[1].next = THING_ENDOFLIST;
    containers[1].slot = THING_ENDOFLIST;
    containers[1].type = 0;
    junks[0].next = (unsigned short)((THING_TYPE_JUNK << 10) | 1);
    junks[0].type = 4;
    junks[1].next = THING_ENDOFLIST;
    junks[1].type = 5;

    things->containers = containers;
    things->containerCount = 2;
    things->junks = junks;
    things->junkCount = 2;

    state->active = 1;
    state->inventoryPanelActive = 1;
    state->showDebugHUD = 0;
    state->assetsAvailable = 0;
    state->originalFontAvailable = 0;
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

static void test_action_hand_scroll_decode_reaches_m11_panel_state(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    char decoded[64];

    M11_GameView_Init(&state);
    seed_scroll_world(&state, &things, textStrings, scrolls, textData);

    ASSERT_EQ(M11_GameView_GetV1OpenScrollPanelGraphicId(), 23,
              "open-scroll panel graphic id");
    ASSERT_TRUE(M11_GameView_DecodeV1InventoryActionHandScrollText(
                    &state, decoded, sizeof(decoded)),
                "action-hand scroll decodes for inventory panel");
    ASSERT_STR_EQ(decoded, "ONE\nTWO", "decoded scroll panel text");
}

static void test_inventory_draw_overlays_scroll_panel_region(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    unsigned char framebuffer[320 * 200];
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int zx = 0, zy = 0, zw = 0, zh = 0;

    M11_GameView_Init(&state);
    seed_scroll_world(&state, &things, textStrings, scrolls, textData);
    memset(framebuffer, 0, sizeof(framebuffer));

    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh),
                "viewport rect helper is available");
    ASSERT_TRUE(M11_GameView_GetV1InventoryPanelZone(&zx, &zy, &zw, &zh),
                "C101 inventory panel zone helper is available");
    ASSERT_TRUE(framebuffer[(vy + zy + 1) * 320 + (vx + zx + 1)] != 0,
                "scroll panel fallback fills C101 panel region");
    ASSERT_TRUE(framebuffer[(vy + zy + zh - 2) * 320 + (vx + zx + zw - 2)] != 0,
                "scroll panel render path reaches C101 lower interior");
}

static void test_inventory_draw_blits_source_open_scroll_panel_pixels(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    unsigned char framebuffer[320 * 200];

    M11_GameView_Init(&state);
    seed_scroll_world(&state, &things, textStrings, scrolls, textData);
    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source C023 scroll panel blit");
    state.assetsAvailable = 1;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);

    ASSERT_TRUE(framebuffer_matches_open_scroll_panel_pixels(&state, framebuffer),
                "action-hand scroll render blits source C023 open-scroll panel into C101");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_eye_click_scroll_routes_without_dialog_overlay(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonTextString_Compat textStrings[1];
    struct DungeonScroll_Compat scrolls[1];
    unsigned short textData[3];
    unsigned char framebuffer[320 * 200];
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int zx = 0, zy = 0, zw = 0, zh = 0;

    M11_GameView_Init(&state);
    seed_scroll_world(&state, &things, textStrings, scrolls, textData);
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] = THING_NONE;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state,
              (unsigned short)(THING_TYPE_SCROLL << 10)), 1,
              "leader hand accepts source scroll thing");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click with leader-hand scroll redraws the inventory panel");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "scroll eye route does not open Firestaff dialog overlay");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "scroll eye route does not mark object-description panel active");
    ASSERT_TRUE(strstr(state.inspectDetail, "SCROLL TEXT PANEL") != NULL,
                "scroll eye route records source scroll-panel detail");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh),
                "viewport rect helper remains available after scroll eye click");
    ASSERT_TRUE(M11_GameView_GetV1InventoryPanelZone(&zx, &zy, &zw, &zh),
                "C101 panel zone remains available after scroll eye click");
    ASSERT_TRUE(framebuffer[(vy + zy + 1) * 320 + (vx + zx + 1)] != 0,
                "scroll eye click leaves C023 scroll panel renderable");
}

static void test_eye_click_chest_opens_panel_without_action_hand_icon_swap(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonContainer_Compat containers[2];
    struct DungeonJunk_Compat junks[2];
    const unsigned short leaderHandChest =
        (unsigned short)(THING_TYPE_CONTAINER << 10);
    const unsigned short actionHandChest =
        (unsigned short)((THING_TYPE_CONTAINER << 10) | 1);
    const unsigned short firstChestItem =
        (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    unsigned char framebuffer[320 * 200];
    int vx = 0, vy = 0, vw = 0, vh = 0;
    int zx = 0, zy = 0, zw = 0, zh = 0;

    M11_GameView_Init(&state);
    seed_chest_world(&state, &things, containers, junks);
    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source C025 chest panel blit");
    state.assetsAvailable = 1;
    state.world.party.champions[0].inventory[CHAMPION_SLOT_ACTION_HAND] =
        actionHandChest;
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, leaderHandChest), 1,
              "leader hand accepts source container thing");
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(
                  &state, CHAMPION_SLOT_ACTION_HAND), 144,
              "action hand starts with a closed chest before pressing-eye chest open");

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 12 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click with leader-hand chest redraws the inventory panel");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "chest eye route does not open Firestaff dialog overlay");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "chest eye route does not mark object-description panel active");
    ASSERT_EQ(M11_GameView_GetV1OpenChestThing(&state), leaderHandChest,
              "pressing-eye chest route opens the leader-hand container panel");
    /* ReDMCSB CHEST.C F0333 lines 43-46 writes C145 to C09 only when
     * P0694_B_PressingEye is false.  With the eye held, a closed action-hand
     * chest remains C144 even though the leader-hand chest panel is open. */
    ASSERT_EQ(M11_GameView_GetV1InventorySlotIconIndex(
                  &state, CHAMPION_SLOT_ACTION_HAND), 144,
              "pressing-eye chest open does not remap the action-hand chest to C145");
    ASSERT_TRUE(strstr(state.inspectDetail, "CONTAINER CHEST PANEL") != NULL,
                "chest eye route records source chest-panel detail");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(M11_GameView_GetViewportRect(&vx, &vy, &vw, &vh),
                "viewport rect helper remains available after chest eye click");
    ASSERT_TRUE(M11_GameView_GetV1InventoryPanelZone(&zx, &zy, &zw, &zh),
                "C101 panel zone remains available after chest eye click");
    ASSERT_TRUE(M11_GameView_GetV1ChestSlotBoxZone(0, &zx, &zy, &zw, &zh),
                "C537 first chest slot zone remains available after chest eye click");
    ASSERT_TRUE(framebuffer_zone_has_nonzero_pixel(framebuffer, 320,
                vx + zx, vy + zy, zw, zh),
                "chest eye click leaves C537 chest slot renderable");
    ASSERT_TRUE(framebuffer_matches_open_chest_panel_pixels(&state, framebuffer),
                "pressing-eye chest route blits source C025 panel outside slot overdraw");
    ASSERT_TRUE(framebuffer_matches_object_icon_in_chest_slot(
                    &state, framebuffer, firstChestItem, 0),
                "pressing-eye chest route blits first source chest item into C537");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

int main(void) {
    printf("=== M11 Inventory Object Panel Render Source-Lock Gate ===\n");
    printf("ReDMCSB: PANEL.C F0352/F0347 -> F0342 -> F0341/CHEST.C F0333, DUNGEON.C F0168\n\n");

    test_action_hand_scroll_decode_reaches_m11_panel_state();
    test_inventory_draw_overlays_scroll_panel_region();
    test_inventory_draw_blits_source_open_scroll_panel_pixels();
    test_eye_click_scroll_routes_without_dialog_overlay();
    test_eye_click_chest_opens_panel_without_action_hand_icon_swap();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
