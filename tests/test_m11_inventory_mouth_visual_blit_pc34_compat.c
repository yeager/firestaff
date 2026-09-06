/*
 * Source-lock gate for the M11 inventory mouth visual blit.
 *
 * ReDMCSB evidence:
 *   PANEL.C F0349 lines 1918-1919: food icons C168..C175 add food
 *   PANEL.C F0349 lines 1832-1844: water/waterskin and potions keep
 *     removeLeaderHandObject false, so they do not enter the C545 animation gate
 *   PANEL.C F0349 lines 1928-1938: removed leader-hand food animates
 *     C205_ICON_MOUTH_OPEN + !(counter & 1), four frames, delay 8
 *   PANEL.C F0332 lines 145-158: I34/PC path loads C545 zone and blits
 *     the extracted 16x16 icon into the viewport
 *   DEFS.H lines 1952-1957: C205 mouth icon base
 *   DEFS.H lines 3914-3915 and COMMAND.C line 426: C545 mouth zone
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
    if (expr) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s\n", (msg)); } \
} while (0)

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { ++g_pass; } \
    else { ++g_fail; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
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
        /* Keep the source-lock gate on the production no-extraction route. */
        snprintf(homePath, sizeof(homePath),
                 "%s/.firestaff/data/dm1/"
                 "Dungeon-Master_DOS_EN_Version-34.zip::DATA/GRAPHICS.DAT",
                 home);
        return homePath;
    }
    return "/nonexistent/firestaff-original-media/GRAPHICS.DAT";
}

static void seed_base_inventory_state(M11_GameViewState* state,
                                      struct DungeonThings_Compat* things) {
    int i;
    memset(things, 0, sizeof(*things));

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
    state->world.party.champions[0].stamina.current = 80;
    state->world.party.champions[0].stamina.maximum = 100;
    state->world.party.champions[0].mana.current = 30;
    state->world.party.champions[0].mana.maximum = 40;
    state->world.party.champions[0].food = 1000;
    state->world.party.champions[0].water = 1000;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        state->world.party.champions[0].inventory[i] = THING_NONE;
    }
}

static void seed_food_mouth_state(M11_GameViewState* state,
                                  struct DungeonThings_Compat* things,
                                  struct DungeonJunk_Compat* junk) {
    unsigned short cheeseThing = (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    static unsigned char rawJunk[4];
    memset(junk, 0, sizeof(*junk));
    junk->next = THING_ENDOFLIST;
    junk->type = 32; /* Cheese: C171 icon, food amount 820. */

    seed_base_inventory_state(state, things);
    things->junks = junk;
    things->junkCount = 1;
    memset(rawJunk, 0, sizeof(rawJunk));
    rawJunk[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawJunk[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawJunk[2] = 32;
    things->loaded = 1;
    things->rawThingData[THING_TYPE_JUNK] = rawJunk;
    things->thingCounts[THING_TYPE_JUNK] = 1;
    ASSERT_TRUE(M11_GameView_SetV1LeaderHandObject(state, cheeseThing),
                "leader hand accepts cheese junk");
}

static void seed_waterskin_mouth_state(M11_GameViewState* state,
                                       struct DungeonThings_Compat* things,
                                       struct DungeonJunk_Compat* junk) {
    unsigned short waterskinThing = (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    /* The hardened PC3.4 icon lookup (raw G0237 rows) rejects struct-only
     * junk mirrors, so the waterskin subtype must also exist as a raw
     * 4-byte junk record: next LE in bytes 0-1, subtype in byte 2. */
    static unsigned char rawJunks[4];
    memset(junk, 0, sizeof(*junk));
    junk->next = THING_ENDOFLIST;
    junk->type = 1;
    junk->chargeCount = 3;

    seed_base_inventory_state(state, things);
    things->junks = junk;
    things->junkCount = 1;
    memset(rawJunks, 0, sizeof(rawJunks));
    rawJunks[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawJunks[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawJunks[2] = 1;
    rawJunks[3] = (unsigned char)(3u << 6);
    things->loaded = 1;
    things->rawThingData[THING_TYPE_JUNK] = rawJunks;
    things->thingCounts[THING_TYPE_JUNK] = 1;
    ASSERT_TRUE(M11_GameView_SetV1LeaderHandObject(state, waterskinThing),
                "leader hand accepts charged waterskin junk");
}

static void seed_water_flask_mouth_state(M11_GameViewState* state,
                                         struct DungeonThings_Compat* things,
                                         struct DungeonPotion_Compat* potion) {
    unsigned short potionThing = (unsigned short)((THING_TYPE_POTION << 10) | 0);
    static unsigned char rawPotion[4];
    memset(potion, 0, sizeof(*potion));
    potion->next = THING_ENDOFLIST;
    potion->power = 80;
    potion->type = 15;

    seed_base_inventory_state(state, things);
    things->potions = potion;
    things->potionCount = 1;
    memset(rawPotion, 0, sizeof(rawPotion));
    rawPotion[0] = (unsigned char)(THING_ENDOFLIST & 0xFFu);
    rawPotion[1] = (unsigned char)((THING_ENDOFLIST >> 8) & 0xFFu);
    rawPotion[2] = 80;
    rawPotion[3] = 15;
    things->loaded = 1;
    things->rawThingData[THING_TYPE_POTION] = rawPotion;
    things->thingCounts[THING_TYPE_POTION] = 1;
    ASSERT_TRUE(M11_GameView_SetV1LeaderHandObject(state, potionThing),
                "leader hand accepts water flask potion");
}

static int framebuffer_matches_icon(const M11_GameViewState* state,
                                    const unsigned char* framebuffer,
                                    int iconIndex) {
    int graphicIndex = 0;
    int srcX = 0, srcY = 0, srcW = 0, srcH = 0;
    int x, y;
    const M11_AssetSlot* slot;

    if (!M11_GameView_GetV1ObjectIconSourceZone(iconIndex,
                                                &graphicIndex,
                                                &srcX,
                                                &srcY,
                                                &srcW,
                                                &srcH)) {
        return 0;
    }
    slot = M11_AssetLoader_Load((M11_AssetLoader*)&state->assetLoader,
                                (unsigned int)graphicIndex);
    if (!slot || !slot->pixels || srcW != 16 || srcH != 16) {
        return 0;
    }

    for (y = 0; y < 16; ++y) {
        for (x = 0; x < 16; ++x) {
            unsigned char want = slot->pixels[(srcY + y) * (int)slot->width + srcX + x];
            unsigned char got = framebuffer[(33 + 13 + y) * 320 + (56 + x)];
            if (got != want) {
                return 0;
            }
        }
    }
    return 1;
}

static void test_food_click_blits_source_mouth_frames(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junk;
    unsigned char framebuffer[320 * 200];
    int i;

    seed_food_mouth_state(&state, &things, &junk);
    ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                "GRAPHICS.DAT asset loader is available for source icon blit");
    state.assetsAvailable = 1;

    ASSERT_EQ(M11_GameView_HandlePointer(&state, 56 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "mouth click consumes leader-hand food");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), THING_NONE,
              "food consumption clears leader hand");
    ASSERT_EQ(state.world.party.champions[0].food, 1820,
              "cheese food amount applies before visual blit");
    ASSERT_EQ(state.v1MouthVisualIconIndex, 206,
              "first source mouth frame is C206");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(framebuffer_matches_icon(&state, framebuffer, 206),
                "inventory draw blits C206 into C545 mouth zone");

    for (i = 0; i < 8; ++i) {
        (void)M11_GameView_AdvanceIdleTick(&state);
    }
    ASSERT_EQ(state.v1MouthVisualIconIndex, 205,
              "8-delay gate advances to C205");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    ASSERT_TRUE(framebuffer_matches_icon(&state, framebuffer, 205),
                "inventory draw blits C205 into C545 mouth zone");

    M11_AssetLoader_Shutdown(&state.assetLoader);
}

static void test_water_and_potion_do_not_start_mouth_visual(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junk;
    struct DungeonPotion_Compat potion;

    seed_waterskin_mouth_state(&state, &things, &junk);
    ASSERT_TRUE(M11_GameView_BindI34EVgaFoodClock(&state, 1000, 1),
                "test clock available for retained waterskin");
    ASSERT_EQ(state.v1FoodCommandPending, 0, "waterskin begins without pending command");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 56 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "mouth click drinks waterskin");
    ASSERT_EQ(state.world.party.champions[0].water, 1800,
              "waterskin water amount applies");
    ASSERT_EQ(junk.chargeCount, 2,
              "waterskin charge decrements");
    ASSERT_TRUE(M11_GameView_GetV1LeaderHandThing(&state) != THING_NONE,
                "waterskin stays in leader hand");
    ASSERT_EQ(state.v1MouthAnimationFrameCount, 0,
              "waterskin does not start transient mouth animation");
    ASSERT_EQ(state.v1MouthVisualIconIndex, 0,
              "waterskin does not blit C545 mouth visual");
    ASSERT_EQ(state.v1FoodCommandPending, 0, "waterskin never defers completion");

    seed_water_flask_mouth_state(&state, &things, &potion);
    ASSERT_TRUE(M11_GameView_BindI34EVgaFoodClock(&state, 1000, 1),
                "test clock available for retained potion");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, 56 + 8, 33 + 13 + 8, 1),
              M11_GAME_INPUT_REDRAW,
              "mouth click drinks water flask potion");
    ASSERT_EQ(state.world.party.champions[0].water, 2048,
              "water flask potion caps water");
    ASSERT_EQ(potion.type, 20,
              "water flask potion becomes empty flask");
    ASSERT_TRUE(M11_GameView_GetV1LeaderHandThing(&state) != THING_NONE,
                "empty flask stays in leader hand");
    ASSERT_EQ(state.v1MouthAnimationFrameCount, 0,
              "potion does not start transient mouth animation");
    ASSERT_EQ(state.v1MouthVisualIconIndex, 0,
              "potion does not blit C545 mouth visual");
    ASSERT_EQ(state.v1FoodCommandPending, 0, "potion never defers completion");
}

static void test_i34e_food_source_wait_order(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junk;
    int edge, route;
    for (route = 0; route < 2; ++route) {
        seed_food_mouth_state(&state, &things, &junk);
        /* Test-injected clock, NOT a claim about the authentic VGA raster.
         * This test drives explicit source edges; cadence binding is pending. */
        ASSERT_TRUE(M11_GameView_BindI34EVgaFoodClock(&state, 1000, 1),
                    "explicit test clock binds without a guessed default");
        state.audioState.lastSoundIndex = -1;
        if (route == 0) {
            (void)M11_GameView_HandlePointer(&state, 64, 54, 1);
        } else {
            unsigned short food = M11_GameView_GetV1LeaderHandThing(&state);
            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
            state.world.party.champions[0].inventory[CHAMPION_SLOT_HAND_RIGHT] = food;
            state.inventorySelectedSlot = CHAMPION_SLOT_HAND_RIGHT;
            ASSERT_TRUE(M11_GameView_UseItem(&state), "alternate food command admitted");
        }
        ASSERT_EQ(state.world.party.champions[0].food, 1820,
                  "food effect precedes source waits");
        ASSERT_EQ(state.v1FoodCommandPending, 1, "food command owns the source loop");
        ASSERT_EQ(M11_GameView_QuickSave(&state), 0,
                  "direct save shortcut cannot persist unfinished food command");
        ASSERT_EQ(M11_GameView_QuickLoad(&state), 0,
                  "direct load shortcut cannot replace pending food world");
        ASSERT_EQ(state.v1FoodCommandPending, 1, "shortcuts preserve pending command");
        ASSERT_EQ(state.world.party.champions[0].food, 1820,
                  "shortcuts preserve committed consumption effect");
        ASSERT_EQ(state.v1MouthVisualIconIndex, 0, "palette wait precedes first frame");
        ASSERT_EQ(state.audioState.lastSoundIndex, -1, "no early swallow request");
        ASSERT_EQ(M11_GameView_BindI34EVgaFoodClock(&state, 60, 1), 0,
                  "clock cannot change mid-command");
        ASSERT_EQ(M11_GameView_UseItem(&state), 0, "repeat use is blocked");
        ASSERT_EQ(M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP),
                  M11_GAME_INPUT_IGNORED, "movement is blocked during source command");
        ASSERT_EQ(M11_GameView_ToggleInventoryPanel(&state), 1,
                  "inventory cannot close during source command");
        (void)M11_GameView_HandlePointerButtonRelease(&state, 64, 54,
                                                    DM1_V1_MOUSE_MASK_LEFT_PC34);
        ASSERT_EQ(state.v1FoodCommandPending, 1, "release does not cancel consumption");
        for (edge = 1; edge <= 36; ++edge) {
            uint32_t tick = state.world.gameTick;
            (void)M11_GameView_AdvanceIdleTick(&state);
            ASSERT_EQ(state.world.gameTick, tick, "idle cannot advance simulation during waits");
            (void)M11_GameView_AdvanceFoodSourceVblank(&state);
            ASSERT_EQ(state.v1FoodCompletionCount, edge == 36 ? 1 : 0,
                      "exactly one completion after 4 times (palette plus 8 waits)");
            if (edge < 36) ASSERT_EQ(state.audioState.lastSoundIndex, -1,
                                      "all pre-completion edges stay silent");
            if (edge == 1 || edge == 10 || edge == 19 || edge == 28) {
                ASSERT_EQ(state.v1MouthVisualIconIndex,
                          ((edge - 1) / 9) % 2 ? 205 : 206,
                          "palette edge exposes the next original mouth icon");
                ASSERT_EQ(state.v1FoodAwaitingPresentation, 1,
                          "composition alone cannot begin the source delay");
                ASSERT_EQ(M11_GameView_AcknowledgeFoodPresentation(
                              &state, state.v1FoodPresentationSerial - 1), 0,
                          "stale presentation acknowledgement rejected");
                (void)M11_GameView_AdvanceFoodSourceVblank(&state);
                ASSERT_EQ(state.v1MouthAnimationDelayRemaining, 8,
                          "source edge before presentation cannot consume delay");
                ASSERT_EQ(M11_GameView_AcknowledgeFoodPresentation(
                              &state, state.v1FoodPresentationSerial), 1,
                          "matching explicit presentation starts delay");
                ASSERT_EQ(M11_GameView_AcknowledgeFoodPresentation(
                              &state, state.v1FoodPresentationSerial), 0,
                          "duplicate acknowledgement rejected");
            }
        }
        (void)M11_GameView_AdvanceFoodSourceVblank(&state);
        ASSERT_EQ(state.v1FoodCompletionCount, 1, "extra edges cannot duplicate swallow");
        ASSERT_EQ(state.v1FoodCommandPending, 0, "final delay releases source command");
    }
    seed_food_mouth_state(&state, &things, &junk);
    ASSERT_EQ(M11_GameView_BindI34EVgaFoodClock(&state, 0, 1), 0,
              "missing clock cannot activate pending command");
    ASSERT_TRUE(M11_GameView_BindI34EVgaFoodClock(&state, 3, 2),
                "rational test clock accepted");
    (void)M11_GameView_HandlePointer(&state, 64, 54, 1);
    (void)M11_GameView_AdvanceFoodClockMs(&state, 666);
    ASSERT_EQ(state.v1MouthVisualIconIndex, 0, "fractional source period is not rounded early");
    (void)M11_GameView_AdvanceFoodClockMs(&state, 1);
    ASSERT_EQ(state.v1MouthVisualIconIndex, 206, "fractional source edge retains remainder");
    (void)M11_GameView_AdvanceFoodClockMs(&state, 60000);
    ASSERT_EQ(state.v1FoodCompletionCount, 0, "host stall cannot collapse all visible frames");
    ASSERT_EQ(state.v1MouthAnimationDelayRemaining, 8,
              "all elapsed edges while presentation fails are excluded");
    ASSERT_EQ(state.v1FoodVblankPhase, 1,
              "unpresented frame preserves free-running rational remainder");
    ASSERT_EQ(M11_GameView_AcknowledgeFoodPresentation(
                  &state, state.v1FoodPresentationSerial), 1,
              "presentation finally succeeds after host stall");
    (void)M11_GameView_AdvanceFoodClockMs(&state, 666);
    ASSERT_EQ(state.v1MouthAnimationDelayRemaining, 8,
              "post-ack subperiod remains below next edge");
    (void)M11_GameView_AdvanceFoodClockMs(&state, 1);
    ASSERT_EQ(state.v1MouthAnimationDelayRemaining, 7,
              "first actual post-presentation edge starts counting");
    /* Stack-owned fixture things must not be released by session teardown. */
    state.world.things = NULL;
    M11_GameView_Shutdown(&state);
    ASSERT_EQ(state.v1FoodCommandPending, 0, "shutdown cancels pending command");
    ASSERT_EQ(state.v1FoodVblankHzNumerator, 0, "shutdown clears clock binding");
}

static int panel_pixels_equal(const unsigned char* a, const unsigned char* b) {
    int x, y, w, h, row;
    if (!M11_GameView_GetV1InventoryPanelZone(&x, &y, &w, &h)) return 0;
    for (row = 0; row < h; ++row)
        if (memcmp(a + (y + 33 + row) * 320 + x,
                   b + (y + 33 + row) * 320 + x, (size_t)w)) return 0;
    return 1;
}

static void test_food_panel_waits_for_completion(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonJunk_Compat junk;
    unsigned char before[320 * 200], frame[320 * 200], other[320 * 200];
    int mode, route, edge;
    for (mode = 0; mode < 2; ++mode) for (route = 0; route < 2; ++route) {
        seed_food_mouth_state(&state, &things, &junk);
        ASSERT_TRUE(M11_AssetLoader_Init(&state.assetLoader, graphics_dat_path()),
                    "original graphics available for food panel ordering");
        state.assetsAvailable = 1;
        M11_Font_Init(&state.originalFont);
        ASSERT_TRUE(M11_Font_LoadFromGraphicsDat(&state.originalFont,
                    state.assetLoader.fileState, state.assetLoader.runtimeState),
                    "original font admits authentic inventory rendering");
        state.originalFontAvailable = M11_Font_IsLoaded(&state.originalFont);
        state.presentationMode = mode ? M12_PRESENTATION_V21_UPSCALED :
                                        M12_PRESENTATION_V1_ORIGINAL;
        state.world.party.championCount = 2;
        state.world.party.champions[1] = state.world.party.champions[0];
        state.world.party.champions[1].food = 0;
        state.dm1InventoryChampionOrdinal = 2;
        state.v1FoodWaterPanelActive = 1;
        ASSERT_TRUE(M11_GameView_BindI34EVgaFoodClock(&state, 1000, 1),
                    "panel ordering uses explicit test edge clock");
        memset(before, 0, sizeof(before));
        memset(other, 0, sizeof(other));
        M11_GameView_Draw(&state, before, 320, 200);
        state.dm1InventoryChampionOrdinal = 1;
        M11_GameView_Draw(&state, other, 320, 200);
        state.dm1InventoryChampionOrdinal = 2;
        if (route) {
            unsigned short food = M11_GameView_GetV1LeaderHandThing(&state);
            DM1_V1_M11Runtime_ClearLeaderHandObjectPc34Compat(&state);
            state.v1FoodWaterPanelActive = 1;
            state.world.party.activeChampionIndex = 1;
            state.world.party.champions[1].inventory[CHAMPION_SLOT_HAND_RIGHT] = food;
            state.inventorySelectedSlot = CHAMPION_SLOT_HAND_RIGHT;
            ASSERT_TRUE(M11_GameView_UseItem(&state), "second champion use admitted");
        } else {
            (void)M11_GameView_HandlePointer(&state, 64, 54, 1);
        }
        ASSERT_EQ(state.world.party.champions[1].food, 820,
                  "consumer simulation food changes immediately");
        ASSERT_EQ(state.world.party.champions[0].food, 1000,
                  "nonconsumer food remains unchanged");
        ASSERT_TRUE(state.v1FoodCommandPending, "consumer command pending");
        ASSERT_EQ(state.v1FoodWaterPanelActive, 1, "food command preserves displayed panel");
        for (edge = 1; edge <= 36; ++edge) {
            (void)M11_GameView_AdvanceFoodSourceVblank(&state);
            if (state.v1FoodAwaitingPresentation) {
                memset(frame, 0, sizeof(frame));
                M11_GameView_Draw(&state, frame, 320, 200);
                ASSERT_TRUE(panel_pixels_equal(before, frame),
                            "displayed food pixels stay unchanged through four frames");
                state.dm1InventoryChampionOrdinal = 1;
                memset(frame, 0, sizeof(frame));
                M11_GameView_Draw(&state, frame, 320, 200);
                ASSERT_TRUE(panel_pixels_equal(other, frame),
                            "consumer snapshot does not leak to another champion");
                state.dm1InventoryChampionOrdinal = 2;
                ASSERT_TRUE(M11_GameView_AcknowledgeFoodPresentation(
                    &state, state.v1FoodPresentationSerial), "test presenter acknowledges frame");
            }
        }
        memset(frame, 0, sizeof(frame));
        M11_GameView_Draw(&state, frame, 320, 200);
        ASSERT_TRUE(!panel_pixels_equal(before, frame),
                    "completed food command changes actual panel pixels");
        state.world.things = NULL;
        M11_GameView_Shutdown(&state);
        ASSERT_EQ(state.v1FoodCommandPending, 0, "shutdown disables display snapshot");
    }
}

int main(void) {
    printf("=== M11 Inventory Mouth Visual Blit Source-Lock Gate ===\n");
    printf("ReDMCSB: PANEL.C F0349/F0332, COMMAND.C C545 mouth zone\n\n");

    test_food_click_blits_source_mouth_frames();
    test_water_and_potion_do_not_start_mouth_visual();
    test_i34e_food_source_wait_order();
    test_food_panel_waits_for_completion();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
