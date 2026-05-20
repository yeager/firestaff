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
    if (env && env[0] != '\0') return env;
    return "/home/trv2/.openclaw/data/firestaff-original-games/DM/_canonical/dm1/GRAPHICS.DAT";
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
    memset(junk, 0, sizeof(*junk));
    junk->next = THING_ENDOFLIST;
    junk->type = 32; /* Cheese: C171 icon, food amount 820. */

    seed_base_inventory_state(state, things);
    things->junks = junk;
    things->junkCount = 1;
    ASSERT_TRUE(M11_GameView_SetV1LeaderHandObject(state, cheeseThing),
                "leader hand accepts cheese junk");
}

static void seed_waterskin_mouth_state(M11_GameViewState* state,
                                       struct DungeonThings_Compat* things,
                                       struct DungeonJunk_Compat* junk) {
    unsigned short waterskinThing = (unsigned short)((THING_TYPE_JUNK << 10) | 0);
    memset(junk, 0, sizeof(*junk));
    junk->next = THING_ENDOFLIST;
    junk->type = 1;
    junk->chargeCount = 3;

    seed_base_inventory_state(state, things);
    things->junks = junk;
    things->junkCount = 1;
    ASSERT_TRUE(M11_GameView_SetV1LeaderHandObject(state, waterskinThing),
                "leader hand accepts charged waterskin junk");
}

static void seed_water_flask_mouth_state(M11_GameViewState* state,
                                         struct DungeonThings_Compat* things,
                                         struct DungeonPotion_Compat* potion) {
    unsigned short potionThing = (unsigned short)((THING_TYPE_POTION << 10) | 0);
    memset(potion, 0, sizeof(*potion));
    potion->next = THING_ENDOFLIST;
    potion->power = 80;
    potion->type = 15;

    seed_base_inventory_state(state, things);
    things->potions = potion;
    things->potionCount = 1;
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

    seed_water_flask_mouth_state(&state, &things, &potion);
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
}

int main(void) {
    printf("=== M11 Inventory Mouth Visual Blit Source-Lock Gate ===\n");
    printf("ReDMCSB: PANEL.C F0349/F0332, COMMAND.C C545 mouth zone\n\n");

    test_food_click_blits_source_mouth_frames();
    test_water_and_potion_do_not_start_mouth_visual();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
