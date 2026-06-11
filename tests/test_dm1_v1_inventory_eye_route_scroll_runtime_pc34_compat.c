/*
 * DM1 V1 inventory eye-route scroll runtime regression.
 *
 * Source evidence (ReDMCSB):
 *   PANEL.C F0352 lines 2153-2158: non-empty leader hand dispatches
 *     the eye click into F0342 (the C07 SCROLL branch goes to
 *     F0341, the C09 CONTAINER branch goes to F0333, everything
 *     else lands on the object-description path).
 *   PANEL.C F0342 lines 1126-1136: leader-hand scrolls (M012_TYPE
 *     C07_THING_TYPE_SCROLL) route to F0341_INVENTORY_DrawPanel_Scroll,
 *     which decodes Scroll.TextStringThingIndex with the
 *     C2_TEXT_TYPE_SCROLL flag set.  The scroll is NOT placed in
 *     the panel: F0341 only reads the text and renders the open
 *     scroll graphic.
 *   CHAMPION.C F0302 lines 662-710: occupied-slot click dispatch
 *     keeps the leader-hand scroll in hand (no put/remove into the
 *     panel).
 *   CHAMPION.C F0297/F0298 lines 243-298: leader-hand put/remove
 *     invariants -- removing the leader hand clears both
 *     v1ScrollPanelActive and v1ScrollPanelThing.
 *   OBJECT.C F0033 lines 147-212: object identity and
 *     scroll-type detection (M012_TYPE == 7).
 *   MOUSE.C F0077/F0078 lines 97-168: the mouse queue serialises
 *     C071 eye clicks.
 *   DEFS.H C07_THING_TYPE_SCROLL, C30..C37/C42/C40/C145 panel
 *     constants, and the scroll-type object table.
 *
 * The C071 eye route is the *only* DM1 V1 way to read a scroll
 * from the inventory panel.  Unlike the action-hand scroll path
 * (which mounts the scroll in the panel), the eye route keeps
 * the scroll in the leader hand and toggles a separate
 * v1ScrollPanelActive state machine that survives only as long as
 * the inspected scroll stays in the leader hand.
 *
 * Important runtime contract: the C071 eye click on a SCROLL does
 * NOT push a dialog overlay (F0342 scroll branch returns 1 without
 * M11_GameView_ShowDialogOverlay).  The empty-hand (F0351) and
 * object-description (F0342 non-scroll) branches DO push a dialog
 * overlay, so the dialog dismiss path is only relevant for those.
 *
 * This probe pins four contract cases that the existing
 * test_dm1_v1_inventory_panel_mouse_routes_pc34_compat and
 * test_m11_inventory_eye_closes_open_chest_runtime_pc34_compat do
 * not cover:
 *   A. C071 eye click on a fresh SCROLL: scroll panel active,
 *      text-index bound to the scroll, scroll stays in leader
 *      hand, stale object-description fields cleared, no dialog
 *      overlay, follow-up C071 click is idempotent.
 *   B. C071 eye click on a DIFFERENT scroll after Scenario A:
 *      scroll panel rebinds to the new scroll, leader hand
 *      updated, follow-up click is idempotent on scroll B.
 *   C. C071 eye click on a non-scroll (torch) while the scroll
 *      panel is still active: scroll panel CLOSES, leader hand
 *      carries the torch, object-description panel opens.
 *      After dialog dismiss, follow-up C071 click re-activates
 *      the object-description panel and does NOT re-activate the
 *      scroll panel.
 *   D. C071 eye click on an empty leader hand: scroll panel
 *      stays closed (F0351 champion-stats branch is taken; the
 *      scroll panel must NEVER be activated by an empty hand).
 *      After dialog dismiss, follow-up C071 click re-takes the
 *      F0351 path with no scroll panel state mutation.
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

#define ASSERT_EQ(actual, expected, msg) do { \
    int a_ = (int)(actual); \
    int e_ = (int)(expected); \
    if (a_ == e_) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got %d expected %d\n", (msg), a_, e_); } \
} while (0)

#define ASSERT_STR_EQ(actual, expected, msg) do { \
    const char* a_ = (actual); \
    const char* e_ = (expected); \
    if (a_ && e_ && strcmp(a_, e_) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: expected '%s', got '%s'\n", \
                             (msg), e_ ? e_ : "(null)", a_ ? a_ : "(null)"); } \
} while (0)

/* Source-locked eye-icon screen coordinate inside the 320x200 viewport.
 * m11_v1_mouse_route_zone_rect binds C546_ZONE_EYE to (12, 13, 16, 16)
 * relative to the panel origin; the panel origin in screen coordinates
 * is (0, 33) per M11_VIEWPORT_Y. */
static const int EYE_SCREEN_X = 12 + 8;
static const int EYE_SCREEN_Y = 33 + 13 + 8;

static void seed_eye_route_world(M11_GameViewState* state,
                                 struct DungeonThings_Compat* things,
                                 struct DungeonWeapon_Compat* weapons,
                                 struct DungeonTextString_Compat* textStrings,
                                 struct DungeonScroll_Compat* scrolls) {
    int i;

    memset(things, 0, sizeof(*things));
    memset(weapons, 0, sizeof(*weapons) * 2);
    memset(textStrings, 0, sizeof(*textStrings) * 2);
    memset(scrolls, 0, sizeof(*scrolls) * 2);

    /* Two source-locked scroll records: scroll A reads textStringThingIndex 7,
     * scroll B reads textStringThingIndex 42.  Each is its own thing so the
     * runtime can rebind v1ScrollPanelThing between them. */
    scrolls[0].next = THING_ENDOFLIST;
    scrolls[0].textStringThingIndex = 7;
    scrolls[0].closed = 0;
    scrolls[1].next = THING_ENDOFLIST;
    scrolls[1].textStringThingIndex = 42;
    scrolls[1].closed = 0;

    /* Two source-locked weapon records: weapon 0 is a generic melee
     * placeholder, weapon 1 is the torch stand-in used by Scenario C. */
    weapons[0].type = 31;     /* dagger-like base type */
    weapons[0].next = THING_ENDOFLIST;
    weapons[0].cursed = 0;
    weapons[0].poisoned = 0;
    weapons[0].broken = 0;
    weapons[0].chargeCount = 0;
    weapons[1].type = 4;      /* torch-like weapon, used as the "torch" stand-in */
    weapons[1].next = THING_ENDOFLIST;
    weapons[1].cursed = 0;
    weapons[1].poisoned = 0;
    weapons[1].broken = 0;
    weapons[1].chargeCount = 12;

    things->weapons = weapons;
    things->weaponCount = 2;
    things->scrolls = scrolls;
    things->scrollCount = 2;
    things->textStrings = textStrings;
    things->textStringCount = 2;
    things->loaded = 1;

    M11_GameView_Init(state);
    state->active = 1;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 1;
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
    snprintf((char*)state->world.party.champions[0].name,
             sizeof(state->world.party.champions[0].name), "BOB");
}

static int scroll_text_index_for(const M11_GameViewState* state, unsigned short thing) {
    int index;
    if (!state || !thing || thing == THING_NONE || thing == THING_ENDOFLIST) return -1;
    if (THING_GET_TYPE(thing) != THING_TYPE_SCROLL) return -1;
    if (!state->world.things || !state->world.things->scrolls) return -1;
    index = (int)THING_GET_INDEX(thing);
    if (index < 0 || index >= state->world.things->scrollCount) return -1;
    return (int)state->world.things->scrolls[index].textStringThingIndex;
}

static unsigned short scroll_thing_a;
static unsigned short scroll_thing_b;
static unsigned short torch_thing;

/* Scenario A: C071 eye click on a fresh SCROLL in the leader hand.
 * The scroll panel must activate, the scroll must stay in the
 * leader hand, stale object-description fields must be cleared,
 * no dialog overlay must be pushed, and a follow-up C071 click
 * must be idempotent. */
static void test_eye_route_fresh_scroll_activates_panel_keeps_hand(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonTextString_Compat textStrings[2];
    struct DungeonScroll_Compat scrolls[2];

    seed_eye_route_world(&state, &things, weapons, textStrings, scrolls);
    scroll_thing_a = (unsigned short)((THING_TYPE_SCROLL << 10) | 0);
    scroll_thing_b = (unsigned short)((THING_TYPE_SCROLL << 10) | 1);
    torch_thing    = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);

    /* Pre-stale object-description fields so the runtime must clear them
     * (mirrors the bug this regression pins). */
    state.v1ObjectDescriptionPanelActive = 1;
    state.v1ObjectDescriptionThing = torch_thing;
    state.v1ObjectDescriptionIconIndex = 30;
    snprintf(state.v1ObjectDescriptionName,
             sizeof(state.v1ObjectDescriptionName), "STALE");
    snprintf(state.v1ObjectDescriptionBody,
             sizeof(state.v1ObjectDescriptionBody), "STALE BODY");
    state.v1ChampionStatsPanelActive = 1;
    state.v1FoodWaterPanelActive = 1;
    snprintf(state.inspectTitle, sizeof(state.inspectTitle), "STALE TITLE");
    snprintf(state.inspectDetail, sizeof(state.inspectDetail), "STALE DETAIL");

    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, scroll_thing_a), 1,
              "leader hand accepts source scroll A");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), scroll_thing_a,
              "leader hand now holds scroll A");
    ASSERT_EQ(scroll_text_index_for(&state, scroll_thing_a), 7,
              "scroll A's textStringThingIndex is 7 in the fixture");

    /* Trigger the C071 eye click via the real M11 mouse dispatcher. */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click on scroll A redraws the inventory panel");

    /* The eye route must NOT place the scroll in the panel; the scroll
     * stays in the leader hand. */
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), scroll_thing_a,
              "eye route preserves leader-hand scroll A (NOT placed in panel)");
    /* Stale object-description fields must be cleared before the
     * scroll panel state is recorded. */
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "eye route clears stale object-description panel");
    ASSERT_EQ(state.v1ObjectDescriptionThing, (int)THING_NONE,
              "eye route clears stale object-description thing");
    ASSERT_EQ(state.v1ObjectDescriptionIconIndex, -1,
              "eye route clears stale object-description icon");
    ASSERT_STR_EQ(state.v1ObjectDescriptionName, "",
                  "eye route clears stale object-description name");
    ASSERT_STR_EQ(state.v1ObjectDescriptionBody, "",
                  "eye route clears stale object-description body");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "eye route clears stale champion-stats panel");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 0,
              "eye route clears stale food/water panel");
    /* Scroll panel state machine: v1ScrollPanelActive toggles on and
     * v1ScrollPanelThing names the inspected scroll. */
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "eye route activates v1ScrollPanelActive for scroll A");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_a,
              "v1ScrollPanelThing binds to the inspected scroll A");
    /* The text-index recorded by the panel is the textStringThingIndex
     * read from the scroll record (F0341 lines 890-895). */
    ASSERT_EQ(scroll_text_index_for(&state, state.v1ScrollPanelThing), 7,
              "scroll panel text-index matches scroll A's textStringThingIndex");
    ASSERT_TRUE(strstr(state.inspectTitle, "SCROLL:") != NULL,
                "inspect title records the scroll panel family");
    ASSERT_TRUE(strstr(state.inspectDetail, "SCROLL TEXT PANEL") != NULL,
                "inspect detail records the SCROLL TEXT PANEL token");
    /* The F0342 scroll branch returns 1 without pushing a dialog
     * overlay.  This is the source-locked difference between the
     * scroll eye route and the object-description / champion-stats
     * routes: a C071 click on a scroll is *not* a dialog. */
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "scroll eye route does not push a dialog overlay");

    /* Idempotency: a second C071 click on the same scroll must NOT
     * toggle the panel off, NOT remove the leader hand, NOT clear
     * unrelated state, and NOT poison a follow-up scroll. */
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "follow-up eye click on the same scroll still redraws");
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "follow-up eye click keeps v1ScrollPanelActive on");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_a,
              "follow-up eye click keeps v1ScrollPanelThing bound to scroll A");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), scroll_thing_a,
              "follow-up eye click leaves leader-hand scroll A in hand");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "follow-up eye click does not flip object-description on");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "follow-up eye click does not flip champion-stats on");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 0,
              "follow-up eye click does not flip food/water on");
    ASSERT_TRUE(strstr(state.inspectDetail, "SCROLL TEXT PANEL") != NULL,
                "follow-up eye click still records SCROLL TEXT PANEL");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "follow-up eye click on scroll A still does not push a dialog");
}

/* Scenario B: After A, swap the leader hand to a different scroll
 * and C071 eye.  v1ScrollPanelThing must rebind. */
static void test_eye_route_rebinds_to_different_scroll(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonTextString_Compat textStrings[2];
    struct DungeonScroll_Compat scrolls[2];

    seed_eye_route_world(&state, &things, weapons, textStrings, scrolls);
    scroll_thing_a = (unsigned short)((THING_TYPE_SCROLL << 10) | 0);
    scroll_thing_b = (unsigned short)((THING_TYPE_SCROLL << 10) | 1);
    torch_thing    = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);

    /* Place scroll A first to mirror the post-Scenario A world. */
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, scroll_thing_a), 1,
              "scroll A mounted in leader hand before swap");
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "scroll A panel state established before swap");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_a,
              "scroll A v1ScrollPanelThing established before swap");

    /* Swap leader hand to a different scroll.  F0352 -> F0342 must
     * detect the different scroll, leave the panel active, and
     * rebind v1ScrollPanelThing. */
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, scroll_thing_b), 1,
              "scroll B mounted in leader hand");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click on scroll B redraws the inventory panel");
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), scroll_thing_b,
              "leader hand now holds scroll B after swap+eye");
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "scroll panel stays active across scroll A->B rebind");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_b,
              "v1ScrollPanelThing rebinds to scroll B");
    ASSERT_EQ(scroll_text_index_for(&state, state.v1ScrollPanelThing), 42,
              "scroll panel text-index rebinds to scroll B's textStringThingIndex");

    /* Cross-checks: scroll A's record is unchanged, scroll B's
     * record is still pointing at textStringThingIndex 42. */
    ASSERT_EQ(scroll_text_index_for(&state, scroll_thing_a), 7,
              "scroll A still references its own textStringThingIndex");
    ASSERT_EQ(scroll_text_index_for(&state, scroll_thing_b), 42,
              "scroll B still references its own textStringThingIndex");
    /* A follow-up C071 click on scroll B is idempotent. */
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "follow-up eye click on scroll B keeps panel active");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_b,
              "follow-up eye click on scroll B keeps v1ScrollPanelThing on scroll B");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 0,
              "scroll B eye route does not push a dialog overlay");
}

/* Scenario C: With the scroll panel still active, swap the leader
 * hand to a non-scroll item (a torch) and C071 eye.  The scroll
 * panel must CLOSE, the torch's object-description must open.
 * After dialog dismiss, follow-up C071 click re-activates the
 * object-description panel and does NOT re-activate the scroll
 * panel. */
static void test_eye_route_non_scroll_closes_scroll_panel(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonTextString_Compat textStrings[2];
    struct DungeonScroll_Compat scrolls[2];

    seed_eye_route_world(&state, &things, weapons, textStrings, scrolls);
    scroll_thing_a = (unsigned short)((THING_TYPE_SCROLL << 10) | 0);
    scroll_thing_b = (unsigned short)((THING_TYPE_SCROLL << 10) | 1);
    torch_thing    = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);

    /* Establish the scroll A panel state. */
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, scroll_thing_a), 1,
              "scroll A mounted before non-scroll swap");
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);
    ASSERT_EQ(state.v1ScrollPanelActive, 1,
              "scroll A panel state established before non-scroll swap");
    ASSERT_EQ(state.v1ScrollPanelThing, scroll_thing_a,
              "scroll A v1ScrollPanelThing established before non-scroll swap");

    /* Swap leader hand to the torch (weapon record 1). */
    ASSERT_EQ(M11_GameView_SetV1LeaderHandObject(&state, torch_thing), 1,
              "leader hand accepts the torch");
    ASSERT_EQ(M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1),
              M11_GAME_INPUT_REDRAW,
              "eye click on torch redraws the inventory panel");

    /* The scroll panel must CLOSE -- a non-scroll item is not
     * eligible for the scroll panel.  This mirrors the eye-closes-open-
     * chest pattern, applied to the scroll panel state. */
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "eye route on non-scroll closes v1ScrollPanelActive");
    ASSERT_EQ(state.v1ScrollPanelThing, (int)THING_NONE,
              "eye route on non-scroll clears v1ScrollPanelThing");
    /* The torch's object-description panel must now be the active
     * detail panel (F0352 -> F0342 non-scroll non-container branch).
     * The object-description path DOES push a dialog overlay. */
    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), torch_thing,
              "leader hand now holds the torch after eye route");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "eye route on torch opens object-description panel");
    ASSERT_EQ(state.v1ObjectDescriptionThing, torch_thing,
              "object-description panel binds to the torch");
    ASSERT_TRUE(state.v1ObjectDescriptionIconIndex >= 0,
                "object-description panel records a torch icon index");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 1,
              "torch eye route pushes a dialog overlay (object-description branch)");

    /* A follow-up eye click must not flip the scroll panel back on.
     * The dialog dismiss path clears v1ObjectDescriptionPanelActive,
     * so a re-click re-activates it.  The contract we pin is that the
     * scroll panel is NOT re-activated, and the object-description
     * panel re-binds to the torch. */
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss torch dialog before the torch C071 follow-up");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "dismissing the dialog does not flip the scroll panel on");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "dismissing the dialog clears object-description panel");
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "follow-up eye click on torch keeps scroll panel closed");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 1,
              "follow-up eye click on torch re-activates object-description");
    ASSERT_EQ(state.v1ObjectDescriptionThing, torch_thing,
              "follow-up eye click on torch rebinds object-description thing");
}

/* Scenario D: C071 eye click on an empty leader hand.  The scroll
 * panel must stay closed, no scroll panel state mutation.  The
 * F0351 champion-stats path is taken; that path is the only valid
 * empty-hand eye behaviour, and it must not leave any scroll panel
 * state behind. */
static void test_eye_route_empty_hand_no_state_mutation(void) {
    M11_GameViewState state;
    struct DungeonThings_Compat things;
    struct DungeonWeapon_Compat weapons[2];
    struct DungeonTextString_Compat textStrings[2];
    struct DungeonScroll_Compat scrolls[2];

    seed_eye_route_world(&state, &things, weapons, textStrings, scrolls);
    scroll_thing_a = (unsigned short)((THING_TYPE_SCROLL << 10) | 0);
    scroll_thing_b = (unsigned short)((THING_TYPE_SCROLL << 10) | 1);
    torch_thing    = (unsigned short)((THING_TYPE_WEAPON << 10) | 1);

    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), (int)THING_NONE,
              "leader hand is empty before the empty-hand eye click");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "scroll panel is not active before the empty-hand eye click");
    ASSERT_EQ(state.v1ScrollPanelThing, (int)THING_NONE,
              "scroll panel thing is clear before the empty-hand eye click");

    /* F0352 / F0351 with an empty leader hand activates the
     * champion-stats overlay.  The scroll panel state must stay
     * untouched. */
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);

    ASSERT_EQ(M11_GameView_GetV1LeaderHandThing(&state), (int)THING_NONE,
              "empty-hand eye click does not promote anything into the leader hand");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "empty-hand eye click does not activate the scroll panel");
    ASSERT_EQ(state.v1ScrollPanelThing, (int)THING_NONE,
              "empty-hand eye click does not bind v1ScrollPanelThing");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "empty-hand eye click does not open the object-description panel");
    ASSERT_EQ(state.v1FoodWaterPanelActive, 0,
              "empty-hand eye click does not flip the food/water panel");
    /* The empty-hand F0351 path legitimately activates the
     * champion-stats panel; that is the correct empty-hand eye
     * behaviour, not a regression.  Re-verify the scroll panel
     * never went active. */
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 1,
              "empty-hand eye click takes the F0351 champion-stats path");
    ASSERT_EQ(M11_GameView_IsDialogOverlayActive(&state), 1,
              "empty-hand eye click pushes a dialog overlay (F0351 path)");

    /* The dialog dismissal must not flip the scroll panel back on
     * nor any other field.  A second eye click must re-take the
     * F0351 path. */
    ASSERT_EQ(M11_GameView_DismissDialogOverlay(&state), 1,
              "dismiss the champion-stats dialog");
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "dismissing the F0351 overlay does not activate scroll panel");
    ASSERT_EQ(state.v1ScrollPanelThing, (int)THING_NONE,
              "dismissing the F0351 overlay does not bind v1ScrollPanelThing");
    ASSERT_EQ(state.v1ObjectDescriptionPanelActive, 0,
              "dismissing the F0351 overlay does not flip object-description");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 0,
              "dismissing the F0351 overlay clears the champion-stats panel");
    (void)M11_GameView_HandlePointer(&state, EYE_SCREEN_X, EYE_SCREEN_Y, 1);
    ASSERT_EQ(state.v1ScrollPanelActive, 0,
              "follow-up empty-hand eye click still does not activate scroll panel");
    ASSERT_EQ(state.v1ScrollPanelThing, (int)THING_NONE,
              "follow-up empty-hand eye click still does not bind v1ScrollPanelThing");
    ASSERT_EQ(state.v1ChampionStatsPanelActive, 1,
              "follow-up empty-hand eye click re-takes the F0351 path");
}

int main(void) {
    printf("=== DM1 V1 Inventory Eye Route Scroll Runtime Gate ===\n");
    printf("ReDMCSB: PANEL.C F0352 2153-2158, F0342 1126-1136, "
           "F0341 890-895, 969-1043, OBJECT.C F0033 147-212, "
           "CHAMPION.C F0302 662-710, F0297/F0298 243-298, "
           "MOUSE.C F0077/F0078 97-168, DEFS.H C07/C30/C40/C42/C71/C145\n\n");

    test_eye_route_fresh_scroll_activates_panel_keeps_hand();
    test_eye_route_rebinds_to_different_scroll();
    test_eye_route_non_scroll_closes_scroll_panel();
    test_eye_route_empty_hand_no_state_mutation();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
