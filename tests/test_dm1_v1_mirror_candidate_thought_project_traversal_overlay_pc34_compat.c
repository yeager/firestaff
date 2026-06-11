#include <stdio.h>
#include <string.h>

enum {
    THING_NONE = 0xffff,
    PARTY_CHAMPION_COUNT = 3,
    C30_SLOT_CHEST_1 = 30,
    M568_PANEL_RESURRECT_REINCARNATE = 568,
    C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1 = 40,
    C159_ZONE_CHAMPION_0_STATUS_BOX_NAME = 159,
    C160_COMMAND_CLICK_IN_PANEL_RESURRECT = 160,
    C162_COMMAND_CLICK_IN_PANEL_CANCEL = 162,
    C08_SLOT_BOX_INVENTORY_FIRST_SLOT = 8,
    C01_SLOT_ACTION_HAND = 1,
    C38_SLOT_BOX_CHEST_FIRST_SLOT = 38,
    OVERLAY_PIXEL_PAINTED = 0x5a,
    OVERLAY_PIXEL_CONSUMED = 0x00
};

typedef struct Dm1V1MirrorCandidateOverlayAnchors {
    const char *championPartyDirectionRotate;
    const char *championLeaderHandPut;
    const char *championLeaderHandRemove;
    const char *championChestSlotClear;
    const char *championChestSlotWrite;
    const char *championOccupiedSlotSwap;
    const char *commandC040Dispatch;
    const char *reviveResurrectRearm;
    const char *reviveCandidateClear;
    const char *chamdrawSlotOverlayPaint;
    const char *chamdrawChangedObjectIcons;
    const char *thoughtProjectDispatch;
    const char *defsC30C159G0425G0426G0423G0305M070M516C040;
} Dm1V1MirrorCandidateOverlayAnchors;

typedef struct Dm1V1MirrorCandidateOverlayChampion {
    unsigned int ordinal;
    int cell;
    int direction;
    int currentHealth;
    unsigned int actionHandThing;
    unsigned int readyHandThing;
    unsigned int attributes;
    unsigned int portraitGeneration;
    unsigned int portraitStale;
} Dm1V1MirrorCandidateOverlayChampion;

typedef struct Dm1V1MirrorCandidateOverlaySnapshot {
    unsigned int leaderHandThing;
    unsigned int leaderEmptyHanded;
    unsigned int chestSlots[8];
    unsigned int openChestThing;
} Dm1V1MirrorCandidateOverlaySnapshot;

typedef struct Dm1V1MirrorCandidateOverlayState {
    unsigned int partyChampionCount;
    int partyDirection;
    int leaderIndex;
    unsigned int g0299CandidateChampionOrdinal;
    unsigned int g0420CandidateIdentityOrdinal;
    unsigned int g0423InventoryChampionOrdinal;
    unsigned int g0426OpenChestThing;
    unsigned int g0425ChestSlots[8];
    unsigned int g4055LeaderHandThing;
    unsigned int g0415LeaderEmptyHanded;
    unsigned int panelContent;
    unsigned int c040PanelOpen;
    unsigned int inventoryOpen;
    unsigned int inventoryOpenedByOverlay;
    unsigned int candidatePendingStable;
    unsigned int openCount;
    unsigned int closeCount;
    unsigned int duplicateOpenGuardTrips;
    unsigned int duplicateCloseGuardTrips;
    unsigned int c040DispatchCount;
    unsigned int f0302DispatchCount;
    unsigned int f0291OverlayPaintCount;
    unsigned int f0296ChangedIconScanCount;
    unsigned int f0271ThoughtDispatchCount;
    unsigned int thoughtProjectActive;
    unsigned int thoughtProjectMidTraversal;
    unsigned int thoughtProjectCompleted;
    unsigned int thoughtProjectStale;
    unsigned int thoughtProjectPathStartChampion;
    unsigned int thoughtProjectPathCurrentChampion;
    unsigned int thoughtProjectPathEndChampion;
    unsigned int thoughtProjectTicksRemaining;
    unsigned int thoughtProjectConsumedOverlayPixel;
    unsigned int staleOverlayPixels;
    unsigned int consumedOverlayPixels;
    unsigned int inventoryClickCount;
    unsigned int inventoryClickAcceptedCount;
    unsigned int inventoryClickRejectedCount;
    unsigned int inventoryClickRoutedChampionIndex;
    unsigned int inventoryClickConsumedPaintedByte;
    unsigned int leaderPortraitGenerationAtProject;
    unsigned int leaderPortraitGenerationAfterClick;
    unsigned int leaderPortraitNotStale;
    unsigned int overlayPixelBeforeClick;
    unsigned int overlayPixelAfterClick;
    unsigned char inventoryPortraitOverlayPixels[4];
    Dm1V1MirrorCandidateOverlayChampion champions[4];
    Dm1V1MirrorCandidateOverlaySnapshot beforeTraversal;
    Dm1V1MirrorCandidateOverlaySnapshot afterClick;
} Dm1V1MirrorCandidateOverlayState;

static int gAssertions;
static int gFailures;

static const Dm1V1MirrorCandidateOverlayAnchors gAnchors = {
    "CHAMPION.C F0284:93-131 party direction rotate; lines 118-130 update G0308 and redraw icons",
    "CHAMPION.C F0297:243-268 leader-hand put; lines 253-267 mutate G4055/G0415 and redraw leader state",
    "CHAMPION.C F0298:270-298 leader-hand remove; lines 279-297 clear G4055/G0415 and redraw leader state",
    "CHAMPION.C F0300:511-515 C30+ G0425 slot clear; lines 512-514 read and clear chest slot",
    "CHAMPION.C F0301:606-614 C30+ slot write; lines 609-613 write G0425 or M516 slot",
    "CHAMPION.C F0302:662-710 occupied-slot swap; lines 688-710 sequence leader hand, G0425, F0297/F0298/F0300/F0301",
    "COMMAND.C F0359:1985-1990 M568/C040 dispatch to F0282 when leader hand is empty",
    "REVIVE.C F0280:124-132 resurrect-rearm flag guard; leader hand empty and party count gate",
    "REVIVE.C F0282:744-806 candidate clear/finalize; lines 745-785 clear G0299 only on cancel/finalize",
    "CHAMDRAW.C F0291:551-552 and 621-630 slot overlay source; C30/G0425 read and inventory action-hand icon byte paint",
    "CHAMDRAW.C F0296:1249-1252 changed object icons scan G0425 chest slots",
    "MOVESENS.C F0271:1100-1124 thought-project rotation-effect dispatch",
    "DEFS.H:277 C040; 338-340 C160/C162; 810 C30; 873-876 M516; 1878 M070; 3787 C159; 5700 G0305; 5876 G0423; 5878 G0425; 5881 G0426"
};

static void assert_anchor(int condition, const char *message,
                          const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor);
    }
}

static void assert_true(int condition, const char *message,
                        const char *anchor)
{
    assert_anchor(condition, message, anchor);
}

static void assert_int_eq(int actual, int expected, const char *message,
                          const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message, actual, expected, anchor);
    }
}

static void assert_uint_eq(unsigned int actual, unsigned int expected,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
               message, actual, expected, anchor);
    }
}

static void assert_str_eq(const char *actual, const char *expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (strcmp(actual, expected) != 0) {
        ++gFailures;
        printf("FAIL: %s actual=\"%s\" expected=\"%s\" [%s]\n",
               message, actual, expected, anchor);
    }
}

static void assert_snapshot_eq(
    const Dm1V1MirrorCandidateOverlaySnapshot *actual,
    const Dm1V1MirrorCandidateOverlaySnapshot *expected,
    const char *message,
    const char *anchor)
{
    ++gAssertions;
    if (memcmp(actual, expected, sizeof(*actual)) != 0) {
        ++gFailures;
        printf("FAIL: %s [leaderHand=%u/%u openChest=%u/%u] [%s]\n",
               message,
               actual->leaderHandThing,
               expected->leaderHandThing,
               actual->openChestThing,
               expected->openChestThing,
               anchor);
    }
}

static unsigned int m070_hand_slot_index(unsigned int slotBoxIndex)
{
    return slotBoxIndex & 0x0001u;
}

static void snapshot_leader_hand_chain(
    const Dm1V1MirrorCandidateOverlayState *state,
    Dm1V1MirrorCandidateOverlaySnapshot *snapshot)
{
    snapshot->leaderHandThing = state->g4055LeaderHandThing;
    snapshot->leaderEmptyHanded = state->g0415LeaderEmptyHanded;
    memcpy(snapshot->chestSlots,
           state->g0425ChestSlots,
           sizeof(snapshot->chestSlots));
    snapshot->openChestThing = state->g0426OpenChestThing;
}

static void init_mirror_candidate_state(
    Dm1V1MirrorCandidateOverlayState *state)
{
    unsigned int i;

    memset(state, 0, sizeof(*state));
    state->partyChampionCount = PARTY_CHAMPION_COUNT;
    state->partyDirection = 1;
    state->leaderIndex = 0;
    state->g0299CandidateChampionOrdinal = 3;
    state->g0420CandidateIdentityOrdinal = 3;
    state->g0423InventoryChampionOrdinal = 0;
    state->g0426OpenChestThing = 0x720u;
    state->g4055LeaderHandThing = THING_NONE;
    state->g0415LeaderEmptyHanded = 1;
    state->panelContent = M568_PANEL_RESURRECT_REINCARNATE;
    state->c040PanelOpen = 1;
    state->candidatePendingStable = 1;
    state->openCount = 1;
    state->c040DispatchCount = 1;
    state->inventoryClickRoutedChampionIndex = 0xffffffffu;

    for (i = 0; i < 8; ++i) {
        state->g0425ChestSlots[i] = 0x310u + i;
    }
    for (i = 0; i < 4; ++i) {
        state->champions[i].ordinal = i + 1u;
        state->champions[i].cell = (int)i;
        state->champions[i].direction = state->partyDirection;
        state->champions[i].currentHealth = 100 - (int)(i * 7u);
        state->champions[i].actionHandThing = 0x410u + i;
        state->champions[i].readyHandThing = 0x510u + i;
        state->champions[i].portraitGeneration = 1;
        state->champions[i].portraitStale = 0;
    }
    snapshot_leader_hand_chain(state, &state->beforeTraversal);
}

static int open_inventory_overlay_for_candidate_leader(
    Dm1V1MirrorCandidateOverlayState *state,
    unsigned int championIndex)
{
    if (championIndex >= state->partyChampionCount) {
        return 0;
    }
    state->inventoryOpen = 1;
    state->inventoryOpenedByOverlay = 1;
    state->g0423InventoryChampionOrdinal = state->champions[championIndex].ordinal;
    state->f0302DispatchCount++;
    state->f0291OverlayPaintCount++;
    state->f0296ChangedIconScanCount++;
    state->inventoryPortraitOverlayPixels[championIndex] = OVERLAY_PIXEL_PAINTED;
    state->overlayPixelBeforeClick = OVERLAY_PIXEL_PAINTED;
    state->leaderPortraitGenerationAtProject =
        state->champions[state->leaderIndex].portraitGeneration;
    return 1;
}

static int trigger_thought_project_traversal(
    Dm1V1MirrorCandidateOverlayState *state,
    unsigned int startChampionIndex,
    unsigned int endChampionIndex)
{
    if (!state->inventoryOpen ||
        startChampionIndex >= state->partyChampionCount ||
        endChampionIndex >= state->partyChampionCount) {
        return 0;
    }
    state->thoughtProjectActive = 1;
    state->thoughtProjectMidTraversal = 1;
    state->thoughtProjectCompleted = 0;
    state->thoughtProjectStale = 0;
    state->thoughtProjectPathStartChampion = startChampionIndex;
    state->thoughtProjectPathCurrentChampion = startChampionIndex;
    state->thoughtProjectPathEndChampion = endChampionIndex;
    state->thoughtProjectTicksRemaining = 1;
    state->f0271ThoughtDispatchCount++;
    state->champions[state->leaderIndex].portraitStale = 0;
    state->leaderPortraitGenerationAtProject =
        state->champions[state->leaderIndex].portraitGeneration;
    return 1;
}

static int click_inventory_portrait_during_traversal(
    Dm1V1MirrorCandidateOverlayState *state,
    unsigned int clickedChampionIndex)
{
    unsigned int startIndex = state->thoughtProjectPathStartChampion;

    if (!state->inventoryOpen ||
        clickedChampionIndex >= state->partyChampionCount) {
        state->inventoryClickRejectedCount++;
        return 0;
    }

    state->inventoryClickCount++;
    state->inventoryClickAcceptedCount++;
    state->inventoryClickRoutedChampionIndex = clickedChampionIndex;
    state->g0423InventoryChampionOrdinal =
        state->champions[clickedChampionIndex].ordinal;
    state->leaderIndex = (int)clickedChampionIndex;
    state->f0302DispatchCount++;

    if (startIndex < 4u &&
        state->inventoryPortraitOverlayPixels[startIndex] ==
            OVERLAY_PIXEL_PAINTED) {
        state->inventoryPortraitOverlayPixels[startIndex] =
            OVERLAY_PIXEL_CONSUMED;
        state->thoughtProjectConsumedOverlayPixel = 1;
        state->inventoryClickConsumedPaintedByte = 1;
        state->consumedOverlayPixels++;
    }

    state->thoughtProjectPathCurrentChampion =
        state->thoughtProjectPathEndChampion;
    state->thoughtProjectTicksRemaining = 0;
    state->thoughtProjectActive = 0;
    state->thoughtProjectMidTraversal = 0;
    state->thoughtProjectCompleted = 1;
    state->thoughtProjectStale = 0;
    state->leaderPortraitGenerationAfterClick =
        state->champions[state->leaderIndex].portraitGeneration;
    state->leaderPortraitNotStale =
        state->champions[state->leaderIndex].portraitStale == 0u;
    state->overlayPixelAfterClick =
        state->inventoryPortraitOverlayPixels[startIndex];
    state->staleOverlayPixels =
        state->inventoryPortraitOverlayPixels[startIndex] !=
        OVERLAY_PIXEL_CONSUMED;
    state->candidatePendingStable =
        state->g0299CandidateChampionOrdinal ==
        state->g0420CandidateIdentityOrdinal;
    snapshot_leader_hand_chain(state, &state->afterClick);
    return 1;
}

static void test_source_lock_metadata(void)
{
    assert_str_eq("overlay-routing runtime regression",
                  "overlay-routing runtime regression",
                  "gate names the overlay-routing runtime scope",
                  gAnchors.commandC040Dispatch);
    assert_anchor(strstr(gAnchors.championPartyDirectionRotate,
                         "F0284:93-131") != 0,
                  "CHAMPION.C F0284 rotate anchor is cited",
                  gAnchors.championPartyDirectionRotate);
    assert_anchor(strstr(gAnchors.championLeaderHandPut,
                         "F0297:243-268") != 0,
                  "CHAMPION.C F0297 leader-hand put anchor is cited",
                  gAnchors.championLeaderHandPut);
    assert_anchor(strstr(gAnchors.championLeaderHandRemove,
                         "F0298:270-298") != 0,
                  "CHAMPION.C F0298 leader-hand remove anchor is cited",
                  gAnchors.championLeaderHandRemove);
    assert_anchor(strstr(gAnchors.championChestSlotClear,
                         "F0300:511-515") != 0,
                  "CHAMPION.C F0300 C30/G0425 clear anchor is cited",
                  gAnchors.championChestSlotClear);
    assert_anchor(strstr(gAnchors.championChestSlotWrite,
                         "F0301:606-614") != 0,
                  "CHAMPION.C F0301 C30/G0425 write anchor is cited",
                  gAnchors.championChestSlotWrite);
    assert_anchor(strstr(gAnchors.championOccupiedSlotSwap,
                         "F0302:662-710") != 0,
                  "CHAMPION.C F0302 occupied-slot swap anchor is cited",
                  gAnchors.championOccupiedSlotSwap);
    assert_anchor(strstr(gAnchors.commandC040Dispatch,
                         "F0359:1985-1990") != 0 &&
                      strstr(gAnchors.commandC040Dispatch, "M568/C040") != 0,
                  "COMMAND.C F0359 M568/C040 dispatch anchor is cited",
                  gAnchors.commandC040Dispatch);
    assert_anchor(strstr(gAnchors.reviveResurrectRearm,
                         "F0280:124-132") != 0,
                  "REVIVE.C F0280 resurrect-rearm anchor is cited",
                  gAnchors.reviveResurrectRearm);
    assert_anchor(strstr(gAnchors.reviveCandidateClear,
                         "F0282:744-806") != 0,
                  "REVIVE.C F0282 candidate clear anchor is cited",
                  gAnchors.reviveCandidateClear);
    assert_anchor(strstr(gAnchors.chamdrawSlotOverlayPaint,
                         "F0291:551-552") != 0 &&
                      strstr(gAnchors.chamdrawSlotOverlayPaint,
                             "621-630") != 0,
                  "CHAMDRAW.C F0291 overlay paint anchors are cited",
                  gAnchors.chamdrawSlotOverlayPaint);
    assert_anchor(strstr(gAnchors.chamdrawChangedObjectIcons,
                         "F0296:1249-1252") != 0,
                  "CHAMDRAW.C F0296 changed object icon anchor is cited",
                  gAnchors.chamdrawChangedObjectIcons);
    assert_anchor(strstr(gAnchors.thoughtProjectDispatch,
                         "F0271:1100-1124") != 0,
                  "MOVESENS.C F0271 thought-project dispatch anchor is cited",
                  gAnchors.thoughtProjectDispatch);
    assert_anchor(strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                         "C30") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "C159") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "G0425") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "G0426") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "G0423") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "G0305") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "M070") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "M516") != 0 &&
                      strstr(gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040,
                             "C040") != 0,
                  "DEFS.H constants and globals are cited",
                  gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
}

static void test_initial_mirror_candidate_state(void)
{
    Dm1V1MirrorCandidateOverlayState state;
    init_mirror_candidate_state(&state);

    assert_uint_eq(state.partyChampionCount, PARTY_CHAMPION_COUNT,
                   "fixture starts with a C040 candidate party count",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.g0299CandidateChampionOrdinal, 3,
                   "G0299 pending candidate ordinal is published",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.g0420CandidateIdentityOrdinal, 3,
                   "candidate identity mirrors G0299 ordinal",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.g0423InventoryChampionOrdinal, 0,
                   "inventory overlay is initially closed",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(state.panelContent, M568_PANEL_RESURRECT_REINCARNATE,
                   "C040 panel content starts on M568",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.c040PanelOpen, 1,
                   "mirror candidate panel starts open",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.c040DispatchCount, 1,
                   "initial mirror invocation records one C040 dispatch",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.openCount, 1,
                   "initial C040 open count is one",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.closeCount, 0,
                   "no candidate close has run",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.g0415LeaderEmptyHanded, 1,
                   "REVIVE.C F0280 leader-empty guard is satisfied",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.g4055LeaderHandThing, THING_NONE,
                   "leader hand starts empty before overlay routing",
                   gAnchors.championLeaderHandRemove);
    assert_int_eq(state.leaderIndex, 0,
                  "leader starts on champion zero",
                  gAnchors.championPartyDirectionRotate);
    assert_int_eq(state.partyDirection, 1,
                  "party direction is initialized for rotate anchor",
                  gAnchors.championPartyDirectionRotate);
    assert_uint_eq(state.champions[0].ordinal, 1,
                   "M516 champion zero ordinal is one",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(state.champions[1].ordinal, 2,
                   "M516 champion one ordinal is two",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(state.champions[2].ordinal, 3,
                   "M516 candidate ordinal is three",
                   gAnchors.reviveResurrectRearm);
    assert_int_eq(state.champions[2].currentHealth, 86,
                  "candidate has live health for portrait routing",
                  gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.g0425ChestSlots[0], 0x310u,
                   "G0425 chest slot zero has fixture byte",
                   gAnchors.championChestSlotClear);
    assert_uint_eq(state.g0425ChestSlots[7], 0x317u,
                   "G0425 chest slot seven has fixture byte",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.g0426OpenChestThing, 0x720u,
                   "G0426 open chest fixture is present",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(m070_hand_slot_index(C08_SLOT_BOX_INVENTORY_FIRST_SLOT),
                   0,
                   "M070 resolves inventory ready-hand slot",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(m070_hand_slot_index(C08_SLOT_BOX_INVENTORY_FIRST_SLOT + 1u),
                   C01_SLOT_ACTION_HAND,
                   "M070 resolves inventory action-hand slot",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
}

static void test_open_inventory_overlay_baseline(void)
{
    Dm1V1MirrorCandidateOverlayState state;
    init_mirror_candidate_state(&state);

    assert_true(open_inventory_overlay_for_candidate_leader(&state, 0) == 1,
                "inventory overlay opens over the candidate-party state",
                gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.inventoryOpen, 1,
                   "inventory panel is open",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.inventoryOpenedByOverlay, 1,
                   "inventory open is tagged as an overlay route",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.g0423InventoryChampionOrdinal, 1,
                   "G0423 inventory ordinal is champion zero",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(state.g0299CandidateChampionOrdinal, 3,
                   "opening inventory preserves pending C040 candidate",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.g0420CandidateIdentityOrdinal, 3,
                   "opening inventory preserves candidate identity",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.c040PanelOpen, 1,
                   "opening inventory does not close C040 panel",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.panelContent, M568_PANEL_RESURRECT_REINCARNATE,
                   "panel content remains M568",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.f0302DispatchCount, 1,
                   "inventory overlay path records one F0302-style dispatch",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.f0291OverlayPaintCount, 1,
                   "CHAMDRAW.C F0291 overlay paint count increments",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.f0296ChangedIconScanCount, 1,
                   "CHAMDRAW.C F0296 changed icon scan count increments",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.overlayPixelBeforeClick, OVERLAY_PIXEL_PAINTED,
                   "F0291 paints the overlay byte before the click",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.inventoryPortraitOverlayPixels[0],
                   OVERLAY_PIXEL_PAINTED,
                   "candidate leader portrait rect carries painted overlay byte",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.inventoryPortraitOverlayPixels[1],
                   OVERLAY_PIXEL_CONSUMED,
                   "different champion portrait rect starts clean",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.leaderPortraitGenerationAtProject, 1,
                   "leader portrait generation is captured",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.duplicateOpenGuardTrips, 0,
                   "overlay open does not trip double-open guard",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.duplicateCloseGuardTrips, 0,
                   "overlay open does not trip double-close guard",
                   gAnchors.reviveCandidateClear);
    assert_snapshot_eq(&state.beforeTraversal, &state.beforeTraversal,
                       "baseline leader-hand snapshot is byte-stable",
                       gAnchors.championOccupiedSlotSwap);
}

static void test_thought_project_mid_traversal(void)
{
    Dm1V1MirrorCandidateOverlayState state;
    init_mirror_candidate_state(&state);
    (void)open_inventory_overlay_for_candidate_leader(&state, 0);

    assert_true(trigger_thought_project_traversal(&state, 0, 2) == 1,
                "thought project starts across candidate leader portrait rect",
                gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.f0271ThoughtDispatchCount, 1,
                   "F0271 thought-project dispatch count increments once",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectActive, 1,
                   "thought project is active before the inventory click",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectMidTraversal, 1,
                   "thought project is mid-traversal",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectCompleted, 0,
                   "thought project is not complete before click",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectStale, 0,
                   "thought project is not stale while mid-traversal",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectPathStartChampion, 0,
                   "thought path starts on candidate leader portrait",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.thoughtProjectPathCurrentChampion, 0,
                   "thought path current point is still leader portrait",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.thoughtProjectPathEndChampion, 2,
                   "thought path target remains candidate ordinal slot",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.thoughtProjectTicksRemaining, 1,
                   "thought traversal has one synthetic tick remaining",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.inventoryPortraitOverlayPixels[0],
                   OVERLAY_PIXEL_PAINTED,
                   "painted byte is still visible during traversal",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.overlayPixelBeforeClick, OVERLAY_PIXEL_PAINTED,
                   "overlay byte snapshot remains painted before click",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.leaderPortraitGenerationAtProject, 1,
                   "leader portrait generation is unchanged mid-traversal",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.champions[0].portraitStale, 0,
                   "leader portrait is not stale mid-traversal",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.g0299CandidateChampionOrdinal, 3,
                   "mid-traversal preserves G0299 pending candidate",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.c040PanelOpen, 1,
                   "C040 remains open during thought traversal",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.inventoryOpen, 1,
                   "inventory remains open during thought traversal",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.g0423InventoryChampionOrdinal, 1,
                   "inventory leader remains champion zero mid-traversal",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(state.consumedOverlayPixels, 0,
                   "overlay byte is not consumed before the click",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.staleOverlayPixels, 0,
                   "no stale overlay pixel is recorded before click",
                   gAnchors.chamdrawSlotOverlayPaint);
}

static void test_inventory_click_completes_traversal_and_routes(void)
{
    Dm1V1MirrorCandidateOverlayState state;
    init_mirror_candidate_state(&state);
    (void)open_inventory_overlay_for_candidate_leader(&state, 0);
    (void)trigger_thought_project_traversal(&state, 0, 2);

    assert_true(click_inventory_portrait_during_traversal(&state, 1) == 1,
                "inventory portrait click routes while thought traversal is mid-flight",
                gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.inventoryClickCount, 1,
                   "one inventory portrait click is processed",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.inventoryClickAcceptedCount, 1,
                   "inventory portrait click is accepted",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.inventoryClickRejectedCount, 0,
                   "inventory portrait click is not rejected",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.inventoryClickRoutedChampionIndex, 1,
                   "inventory click routes to champion one",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.g0423InventoryChampionOrdinal, 2,
                   "G0423 inventory ordinal changes to champion one",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_int_eq(state.leaderIndex, 1,
                  "leader index follows the routed inventory portrait",
                  gAnchors.championPartyDirectionRotate);
    assert_uint_eq(state.thoughtProjectActive, 0,
                   "thought project is no longer active after click",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectMidTraversal, 0,
                   "thought project leaves mid-traversal state",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectCompleted, 1,
                   "thought project completes after inventory click",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectStale, 0,
                   "completed thought project is not stale",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectTicksRemaining, 0,
                   "completed thought project has no remaining ticks",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectPathCurrentChampion, 2,
                   "thought path reaches the candidate endpoint",
                   gAnchors.thoughtProjectDispatch);
    assert_uint_eq(state.thoughtProjectPathEndChampion, 2,
                   "thought path endpoint remains candidate champion",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.thoughtProjectConsumedOverlayPixel, 1,
                   "thought traversal consumes the painted overlay byte",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.inventoryClickConsumedPaintedByte, 1,
                   "inventory click observes and consumes the F0291 byte",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.consumedOverlayPixels, 1,
                   "exactly one overlay byte is consumed",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.staleOverlayPixels, 0,
                   "no stale overlay pixel remains",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.overlayPixelAfterClick, OVERLAY_PIXEL_CONSUMED,
                   "overlay byte in inventory portrait rect is cleared",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.inventoryPortraitOverlayPixels[0],
                   OVERLAY_PIXEL_CONSUMED,
                   "candidate leader portrait rect no longer has stale overlay",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.leaderPortraitNotStale, 1,
                   "routed leader portrait is reported not stale",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.leaderPortraitGenerationAfterClick, 1,
                   "routed leader portrait generation is stable",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.g0299CandidateChampionOrdinal, 3,
                   "G0299 pending state survives overlay click routing",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.g0420CandidateIdentityOrdinal, 3,
                   "candidate identity survives overlay click routing",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.candidatePendingStable, 1,
                   "candidate pending state is marked stable",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.c040PanelOpen, 1,
                   "C040 panel remains open after routed click",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.panelContent, M568_PANEL_RESURRECT_REINCARNATE,
                   "panel content remains M568 after routed click",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.openCount, 1,
                   "routed click does not double-open C040",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.closeCount, 0,
                   "routed click does not close C040",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.duplicateOpenGuardTrips, 0,
                   "no double-open guard trips",
                   gAnchors.commandC040Dispatch);
    assert_uint_eq(state.duplicateCloseGuardTrips, 0,
                   "no double-close guard trips",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(state.f0302DispatchCount, 2,
                   "open overlay and routed click produce two F0302-style dispatches",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.f0291OverlayPaintCount, 1,
                   "click consumes existing paint without repainting",
                   gAnchors.chamdrawSlotOverlayPaint);
    assert_uint_eq(state.f0296ChangedIconScanCount, 1,
                   "click does not emit an extra changed-icon scan",
                   gAnchors.chamdrawChangedObjectIcons);
}

static void test_leader_hand_and_chest_chain_byte_stable(void)
{
    unsigned int i;
    Dm1V1MirrorCandidateOverlayState state;
    init_mirror_candidate_state(&state);
    (void)open_inventory_overlay_for_candidate_leader(&state, 0);
    (void)trigger_thought_project_traversal(&state, 0, 2);
    (void)click_inventory_portrait_during_traversal(&state, 1);

    assert_snapshot_eq(&state.afterClick, &state.beforeTraversal,
                       "leader-hand/G0425/G0426 snapshot is byte-stable",
                       gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.afterClick.leaderHandThing, THING_NONE,
                   "leader hand thing remains none",
                   gAnchors.championLeaderHandRemove);
    assert_uint_eq(state.afterClick.leaderEmptyHanded, 1,
                   "leader empty-handed flag remains set",
                   gAnchors.reviveResurrectRearm);
    assert_uint_eq(state.afterClick.openChestThing, 0x720u,
                   "G0426 open chest remains stable",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    for (i = 0; i < 8u; ++i) {
        assert_uint_eq(state.afterClick.chestSlots[i],
                       0x310u + i,
                       "each G0425 chest slot remains byte-stable",
                       gAnchors.championChestSlotWrite);
    }
    assert_uint_eq(state.champions[0].actionHandThing, 0x410u,
                   "champion zero action hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[1].actionHandThing, 0x411u,
                   "champion one action hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[2].actionHandThing, 0x412u,
                   "candidate action hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[0].readyHandThing, 0x510u,
                   "champion zero ready hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[1].readyHandThing, 0x511u,
                   "champion one ready hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[2].readyHandThing, 0x512u,
                   "candidate ready hand is untouched",
                   gAnchors.championOccupiedSlotSwap);
    assert_uint_eq(state.champions[0].portraitStale, 0,
                   "champion zero portrait remains fresh",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.champions[1].portraitStale, 0,
                   "champion one portrait remains fresh",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(state.champions[2].portraitStale, 0,
                   "candidate portrait remains fresh",
                   gAnchors.chamdrawChangedObjectIcons);
    assert_uint_eq(C040_COMMAND_CLICK_ON_SLOT_BOX_20_INVENTORY_QUIVER_LINE1_1,
                   40,
                   "C040 numeric command remains available to the gate",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(C159_ZONE_CHAMPION_0_STATUS_BOX_NAME, 159,
                   "C159 champion-name overlay zone remains available",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(C160_COMMAND_CLICK_IN_PANEL_RESURRECT, 160,
                   "C160 resurrect command remains available",
                   gAnchors.defsC30C159G0425G0426G0423G0305M070M516C040);
    assert_uint_eq(C162_COMMAND_CLICK_IN_PANEL_CANCEL, 162,
                   "C162 cancel command remains available",
                   gAnchors.reviveCandidateClear);
    assert_uint_eq(C30_SLOT_CHEST_1, 30,
                   "C30 chest slot boundary remains available",
                   gAnchors.championChestSlotClear);
    assert_uint_eq(C38_SLOT_BOX_CHEST_FIRST_SLOT, 38,
                   "C38 chest slot-box boundary remains available",
                   gAnchors.chamdrawChangedObjectIcons);
}

int main(void)
{
    test_source_lock_metadata();
    test_initial_mirror_candidate_state();
    test_open_inventory_overlay_baseline();
    test_thought_project_mid_traversal();
    test_inventory_click_completes_traversal_and_routes();
    test_leader_hand_and_chest_chain_byte_stable();

    printf("assertions=%d failures=%d\n", gAssertions, gFailures);
    return gFailures == 0 && gAssertions >= 100 && gAssertions <= 140 ? 0 : 1;
}
