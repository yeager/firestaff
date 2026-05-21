/*
 * Source-lock gate for M11 inventory scroll panel rendering.
 *
 * ReDMCSB evidence:
 *   PANEL.C F0347 lines 1658-1691: action-hand scroll selects scroll panel
 *   PANEL.C F0342 lines 1126-1131: scroll object dispatches to F0341
 *   PANEL.C F0341 lines 890-895: decodes Scroll.TextStringThingIndex
 *     with C2_TEXT_TYPE_SCROLL | MASK0x8000_DECODE_EVEN_IF_INVISIBLE
 *   PANEL.C F0341 lines 969-1043: blits C023 open-scroll panel and
 *     renders each decoded newline-separated line
 */

#include "m11_game_view.h"
#include "memory_champion_state_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#include <stdio.h>
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

int main(void) {
    printf("=== M11 Inventory Scroll Panel Render Source-Lock Gate ===\n");
    printf("ReDMCSB: PANEL.C F0347 -> F0342 -> F0341, DUNGEON.C F0168\n\n");

    test_action_hand_scroll_decode_reaches_m11_panel_state();
    test_inventory_draw_overlays_scroll_panel_region();
    test_eye_click_scroll_routes_without_dialog_overlay();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
