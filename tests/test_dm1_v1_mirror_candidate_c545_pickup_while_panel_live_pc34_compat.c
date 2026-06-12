#include "dm1_v1_mirror_candidate_c545_pickup_while_panel_live_pc34_compat.h"

#include <stdio.h>
#include <string.h>

/* ReDMCSB source-lock evidence for this contract test:
 * CHEST.C F0333:30-67, F0334:117-132; CHAMPION.C F0297:243-268,
 * F0298:270-298, F0300:511-584, F0301:606-660, F0302:662-713; COMMAND.C
 * F0378:1973-1983, F0380:2045-2159; REVIVE.C F0280:124-132,
 * F0282:744-806; PANEL.C F0346/F0347:1619-1657; UTAMSCR.C
 * F0077/F0078:141-150; OBJECT.C F0033:147-212; BLITMASK.C F0133:30-33;
 * DEFS.H:338-340, 810-817, 1874-1878, 2200, 3001-3008, 3906-3913,
 * 4205-4207, 5694, and 5876-5881.
 */

static int gAssertions;
static int gFailures;

static void check_true(int condition, const char *message, const char *anchor)
{
    ++gAssertions;
    if (!condition) {
        ++gFailures;
        printf("FAIL: %s [%s]\n", message, anchor ? anchor : "(null)");
    }
}

static void check_int_eq(int actual, int expected, const char *message,
                         const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=%d expected=%d [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_uint_eq(unsigned int actual, unsigned int expected,
                          const char *message, const char *anchor)
{
    ++gAssertions;
    if (actual != expected) {
        ++gFailures;
        printf("FAIL: %s actual=0x%08X expected=0x%08X [%s]\n",
               message,
               actual,
               expected,
               anchor ? anchor : "(null)");
    }
}

static void check_contains(const char *haystack, const char *needle,
                           const char *message, const char *anchor)
{
    ++gAssertions;
    if (!haystack || !needle || !strstr(haystack, needle)) {
        ++gFailures;
        printf("FAIL: %s missing=%s [%s]\n",
               message,
               needle ? needle : "(null)",
               anchor ? anchor : "(null)");
    }
}

static int expected_chest_slot(int index)
{
    return 0x7200 + index;
}

static void test_source_metadata(void)
{
    const Dm1V1MirrorC545PickupEvidencePc34Compat *e =
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_EvidencePc34Compat();
    const char *text =
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SourceEvidencePc34Compat();

    check_true(e != NULL, "evidence accessor returns metadata",
               "COMMAND.C F0380:2045-2159");
    check_int_eq(e ? e->sourceLockedContractOnly : 0, 1,
                 "source_locked_contract_only=1",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(e ? e->noRealAssetBitmapParity : 0, 1,
                 "no_real_asset_bitmap_parity=1",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(e ? e->noGameDataLoad : 0, 1,
                 "no_game_data_load=1",
                 "COMMAND.C F0380:2045-2159");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "evidence cites chest open",
                   "CHEST.C F0333:30-67");
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:117-132",
                   "evidence cites chest close",
                   "CHEST.C F0334:117-132");
    check_contains(e->championPutAnchor, "CHAMPION.C F0297:243-268",
                   "evidence cites leader-hand put",
                   "CHAMPION.C F0297:243-268");
    check_contains(e->championRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "evidence cites leader-hand remove",
                   "CHAMPION.C F0298:270-298");
    check_contains(e->championSlotRemoveAnchor, "CHAMPION.C F0300:511-584",
                   "evidence cites slot removal",
                   "CHAMPION.C F0300:511-584");
    check_contains(e->championSlotAddAnchor, "CHAMPION.C F0301:606-660",
                   "evidence cites slot add",
                   "CHAMPION.C F0301:606-660");
    check_contains(e->championSlotDispatchAnchor, "CHAMPION.C F0302:662-713",
                   "evidence cites slot dispatch",
                   "CHAMPION.C F0302:662-713");
    check_contains(e->commandDispatchAnchor, "COMMAND.C F0378:1973-1983",
                   "evidence cites command dispatch",
                   "COMMAND.C F0378:1973-1983");
    check_contains(e->commandPickupAnchor, "COMMAND.C F0380:2045-2159",
                   "evidence cites pickup queue",
                   "COMMAND.C F0380:2045-2159");
    check_contains(e->reviveOpenAnchor, "REVIVE.C F0280:124-132",
                   "evidence cites revive open",
                   "REVIVE.C F0280:124-132");
    check_contains(e->reviveClickAnchor, "REVIVE.C F0282:744-806",
                   "evidence cites revive click",
                   "REVIVE.C F0282:744-806");
    check_contains(e->panelAnchor, "PANEL.C F0346/F0347:1619-1657",
                   "evidence cites C040 panel",
                   "PANEL.C F0346/F0347:1619-1657");
    check_contains(e->utilityAnchor, "UTAMSCR.C F0077/F0078:141-150",
                   "evidence cites utility dispatch",
                   "UTAMSCR.C F0077/F0078:141-150");
    check_contains(e->objectAnchor, "OBJECT.C F0033:147-212",
                   "evidence cites object lookup",
                   "OBJECT.C F0033:147-212");
    check_contains(e->blitmaskAnchor, "BLITMASK.C F0133:30-33",
                   "evidence cites masked redraw",
                   "BLITMASK.C F0133:30-33");
    check_contains(e->defsAnchor, "DEFS.H:338-340 C162",
                   "evidence cites C162", "DEFS.H:338-340");
    check_contains(e->defsAnchor, "DEFS.H:810-817 C30..C37",
                   "evidence cites C30..C37", "DEFS.H:810-817");
    check_contains(e->defsAnchor, "DEFS.H:1874-1878 C38",
                   "evidence cites C38", "DEFS.H:1874-1878");
    check_contains(e->defsAnchor, "DEFS.H:2200 C040",
                   "evidence cites C040", "DEFS.H:2200");
    check_contains(e->defsAnchor, "DEFS.H:3001-3008 M568/M569",
                   "evidence cites M568/M569", "DEFS.H:3001-3008");
    check_contains(e->defsAnchor, "DEFS.H:3906-3913 C537..C544",
                   "evidence cites C537..C544", "DEFS.H:3906-3913");
    check_contains(e->defsAnchor, "DEFS.H:4205-4207 ornament",
                   "evidence cites ornament", "DEFS.H:4205-4207");
    check_contains(e->defsAnchor, "DEFS.H:5694 G0299",
                   "evidence cites G0299", "DEFS.H:5694");
    check_contains(e->defsAnchor, "DEFS.H:5876-5881 G0425/G0426",
                   "evidence cites G0425/G0426", "DEFS.H:5876-5881");
    check_contains(e->contractScope, "source_locked_contract_only=1",
                   "contract scope records source lock", e->contractScope);
    check_contains(e->contractScope, "no_real_asset_bitmap_parity=1",
                   "contract scope excludes real-asset parity",
                   e->contractScope);
    check_contains(e->contractScope, "no_game_data_load=1",
                   "contract scope excludes game-data load", e->contractScope);
    check_contains(e->contractScope, "complementary to the C545 drop",
                   "contract scope is complementary to drop direction",
                   e->contractScope);

    check_contains(text, "CHEST.C F0333:30-67",
                   "source text cites F0333", "CHEST.C F0333:30-67");
    check_contains(text, "CHEST.C F0334:117-132",
                   "source text cites F0334", "CHEST.C F0334:117-132");
    check_contains(text, "CHAMPION.C F0297:243-268",
                   "source text cites F0297", "CHAMPION.C F0297:243-268");
    check_contains(text, "CHAMPION.C F0298:270-298",
                   "source text cites F0298", "CHAMPION.C F0298:270-298");
    check_contains(text, "CHAMPION.C F0300:511-584",
                   "source text cites F0300", "CHAMPION.C F0300:511-584");
    check_contains(text, "CHAMPION.C F0301:606-660",
                   "source text cites F0301", "CHAMPION.C F0301:606-660");
    check_contains(text, "CHAMPION.C F0302:662-713",
                   "source text cites F0302", "CHAMPION.C F0302:662-713");
    check_contains(text, "COMMAND.C F0378:1973-1983",
                   "source text cites F0378", "COMMAND.C F0378:1973-1983");
    check_contains(text, "COMMAND.C F0380:2045-2159",
                   "source text cites F0380", "COMMAND.C F0380:2045-2159");
    check_contains(text, "REVIVE.C F0280:124-132",
                   "source text cites F0280", "REVIVE.C F0280:124-132");
    check_contains(text, "REVIVE.C F0282:744-806",
                   "source text cites F0282", "REVIVE.C F0282:744-806");
    check_contains(text, "PANEL.C F0346/F0347:1619-1657",
                   "source text cites panel", "PANEL.C F0346/F0347:1619-1657");
    check_contains(text, "UTAMSCR.C F0077/F0078:141-150",
                   "source text cites utility",
                   "UTAMSCR.C F0077/F0078:141-150");
    check_contains(text, "OBJECT.C F0033:147-212",
                   "source text cites object", "OBJECT.C F0033:147-212");
    check_contains(text, "BLITMASK.C F0133:30-33",
                   "source text cites blitmask", "BLITMASK.C F0133:30-33");
    check_contains(text, "G0299", "source text cites G0299", "DEFS.H:5694");
}

static void test_spec_metadata(void)
{
    const Dm1V1MirrorC545PickupSpecPc34Compat *spec =
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_SpecPc34Compat();

    check_true(spec != NULL, "spec accessor returns metadata",
               "COMMAND.C F0380:2045-2159");
    check_uint_eq(spec->deterministicSeed, 0x0C545727u,
                  "spec deterministic seed",
                  "COMMAND.C F0380:2045-2159");
    check_int_eq(spec->leaderIndex, 0, "spec leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(spec->partyChampionCount, 3, "spec party count leaves room",
                 "REVIVE.C F0280:124-132");
    check_uint_eq(spec->candidateOrdinal, 3, "spec candidate ordinal",
                  "REVIVE.C F0280:124-132");
    check_int_eq(spec->c545FloorThing, 0x7545, "spec C545 floor thing",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(spec->alreadyHeldThing, 0x6A31, "spec negative hand thing",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(spec->previousFloorThing, 0x5120,
                 "spec previous floor thing",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(spec->c040PanelGraphic,
                 DM1_V1_MIRROR_C545_PICKUP_C040_PANEL_PC34_COMPAT,
                 "spec C040 graphic", "DEFS.H:2200");
    check_int_eq(spec->c040PanelContent,
                 DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT,
                 "spec M568 panel content", "DEFS.H:3001-3008");
    check_int_eq(spec->c545Zone,
                 DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT,
                 "spec C545 zone", "COMMAND.C F0378:1973-1983");
    check_int_eq(spec->c162CloseCommand,
                 DM1_V1_MIRROR_C545_PICKUP_C162_CANCEL_PC34_COMPAT,
                 "spec close command", "DEFS.H:338-340");
}

static void test_initial_state(void)
{
    Dm1V1MirrorC545PickupStatePc34Compat state;
    int i;

    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);

    check_int_eq(state.sourceLockedContractOnly, 1,
                 "initial contract-only flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.noRealAssetBitmapParity, 1,
                 "initial no bitmap parity flag",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.noGameDataLoad, 1, "initial no game-data flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.leaderIndex, 0, "initial leader index",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.partyChampionCount, 3, "initial party count",
                 "REVIVE.C F0280:124-132");
    check_uint_eq(state.candidateOrdinal, 3, "initial candidate ordinal",
                  "REVIVE.C F0280:124-132");
    check_uint_eq(state.g0299CandidateOrdinal, 3, "initial G0299 ordinal",
                  "DEFS.H:5694");
    check_int_eq(state.leaderHandThing,
                 DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT,
                 "initial leader hand empty",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.floorThings[0], 0x7545, "initial C545 floor item",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.floorThings[1], 0x5120,
                 "initial previous floor item preserved",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.floorThingCount, 2, "initial floor thing count",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.c040PanelOpen, 1, "initial C040 live",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.c040PanelContent,
                 DM1_V1_MIRROR_C545_PICKUP_M568_CANDIDATE_PANEL_PC34_COMPAT,
                 "initial M568 panel", "DEFS.H:3001-3008");
    check_int_eq(state.c545Zone,
                 DM1_V1_MIRROR_C545_PICKUP_C545_ZONE_PC34_COMPAT,
                 "initial C545 zone", "COMMAND.C F0378:1973-1983");
    check_int_eq(state.f0280ReviveOpenCount, 1,
                 "initial candidate was already opened",
                 "REVIVE.C F0280:124-132");
    check_int_eq(state.f0282ReviveClickCount, 0,
                 "initial revive click count",
                 "REVIVE.C F0282:744-806");
    check_int_eq(state.f0346PanelDrawCount, 1, "initial panel draw",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.f0347PanelRefreshCount, 1,
                 "initial panel refresh", "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.f0133MaskCount, 1, "initial masked redraw",
                 "BLITMASK.C F0133:30-33");
    for (i = 0; i < DM1_V1_MIRROR_C545_PICKUP_SLOT_COUNT_PC34_COMPAT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "initial C537..C544 slot %d", i);
        check_int_eq(state.visibleC537ToC544[i], expected_chest_slot(i),
                     label, "DEFS.H:3906-3913");
        snprintf(label, sizeof(label), "initial G0425 slot %d", i);
        check_int_eq(state.g0425ChestSlots[i], expected_chest_slot(i),
                     label, "DEFS.H:5876-5881");
    }
}

static void check_run_result(
    const Dm1V1MirrorC545PickupResultPc34Compat *result)
{
    const Dm1V1MirrorC545PickupEvidencePc34Compat *e = result->evidence;

    check_true(result->accepted, "runtime pickup accepted",
               e->contractScope);
    check_int_eq(result->initialLeaderHand,
                 DM1_V1_MIRROR_C545_PICKUP_NONE_PC34_COMPAT,
                 "leader hand starts empty", e->championPutAnchor);
    check_int_eq(result->finalLeaderHand, 0x7545,
                 "leader hand receives C545", e->championPutAnchor);
    check_int_eq(result->pickedThing, 0x7545,
                 "picked thing is C545 floor item", e->commandPickupAnchor);
    check_int_eq(result->initialFloorThingCount, 2,
                 "initial floor count captured", e->commandPickupAnchor);
    check_int_eq(result->floorThingCountAfterPickup, 1,
                 "pickup removes one floor item", e->commandPickupAnchor);
    check_int_eq(result->floorThingCountAfterClose, 1,
                 "close does not lose floor item", e->commandPickupAnchor);
    check_uint_eq(result->initialCandidateOrdinal, 3,
                  "initial candidate ordinal captured", e->reviveOpenAnchor);
    check_uint_eq(result->candidateOrdinalAfterPickup, 3,
                  "G0299 unchanged after pickup", e->reviveOpenAnchor);
    check_uint_eq(result->candidateOrdinalAfterClose, 3,
                  "G0299 unchanged after clean close", e->reviveClickAnchor);
    check_int_eq(result->panelOpenAfterPickup, 1,
                 "C040 remains open after pickup", e->panelAnchor);
    check_int_eq(result->panelOpenAfterClose, 0,
                 "C040 closes cleanly after pickup", e->panelAnchor);
    check_int_eq(result->panelStableAfterPickup, 1,
                 "panel hash stable after pickup", e->panelAnchor);
    check_int_eq(result->noPanelFlicker, 1,
                 "no C040 flicker recorded", e->panelAnchor);
    check_int_eq(result->noZOrderCorruption, 1,
                 "no C040 z-order corruption", e->panelAnchor);
    check_int_eq(result->noMirrorCandidateSideEffect, 1,
                 "mirror candidate not silently set", e->reviveOpenAnchor);
    check_int_eq(result->noReviveTriggerOnPickup, 1,
                 "pickup does not trigger revive path", e->reviveClickAnchor);
    check_int_eq(result->closeCleanWithHandFull, 1,
                 "C040 close succeeds with C545 in hand", e->panelAnchor);
    check_int_eq(result->c040Redraws, 4, "C040 redraw count",
                 e->panelAnchor);
    check_int_eq(result->handTransitions, 1, "hand transition count",
                 e->championPutAnchor);
    check_int_eq(result->panelRejections, 1,
                 "negative full-hand panel rejection count",
                 e->championSlotDispatchAnchor);
    check_int_eq(result->mirrorCandidateGuard, 1,
                 "mirror-candidate guard matrix passed",
                 e->commandPickupAnchor);

    check_int_eq(result->f0378DispatchCount, 2,
                 "dispatch covers pickup and close", e->commandDispatchAnchor);
    check_int_eq(result->f0380PickupFlowCount, 2,
                 "queue flow covers pickup and close", e->commandPickupAnchor);
    check_int_eq(result->f0297PutCount, 1,
                 "F0297 put called once", e->championPutAnchor);
    check_int_eq(result->f0298RemoveCount, 0,
                 "F0298 remove not used for pickup", e->championRemoveAnchor);
    check_int_eq(result->f0300SlotRemoveCount, 0,
                 "F0300 slot removal not used", e->championSlotRemoveAnchor);
    check_int_eq(result->f0301SlotAddCount, 0,
                 "F0301 slot add not used", e->championSlotAddAnchor);
    check_int_eq(result->f0302SlotDispatchCount, 0,
                 "F0302 slot dispatch not used", e->championSlotDispatchAnchor);
    check_int_eq(result->f0333OpenCount, 0,
                 "F0333 chest open untouched", e->chestOpenAnchor);
    check_int_eq(result->f0334CloseCount, 0,
                 "F0334 chest close untouched", e->chestCloseAnchor);
    check_int_eq(result->f0280ReviveOpenCount, 1,
                 "F0280 remains initial open only", e->reviveOpenAnchor);
    check_int_eq(result->f0282ReviveClickCount, 0,
                 "F0282 never fires for pickup", e->reviveClickAnchor);
    check_int_eq(result->f0346PanelDrawCount, 1,
                 "F0346 draw count stable", e->panelAnchor);
    check_int_eq(result->f0347PanelRefreshCount, 3,
                 "F0347 refresh covers initial, pickup, close",
                 e->panelAnchor);
    check_int_eq(result->f0077EnableCount, 2,
                 "F0077 brackets pickup and close", e->utilityAnchor);
    check_int_eq(result->f0078DisableCount, 2,
                 "F0078 brackets pickup and close", e->utilityAnchor);
    check_int_eq(result->f0033ObjectLookupCount, 1,
                 "F0033 object lookup used for pickup", e->objectAnchor);
    check_int_eq(result->f0133MaskCount, 3,
                 "F0133 masks initial, pickup, close", e->blitmaskAnchor);

    check_int_eq(result->negativeRejected, 1,
                 "negative full-hand pickup rejected",
                 e->championSlotDispatchAnchor);
    check_int_eq(result->negativeInitialHand, 0x6A31,
                 "negative starts with occupied hand",
                 e->championSlotDispatchAnchor);
    check_int_eq(result->negativeFinalHand, 0x6A31,
                 "negative hand remains occupied",
                 e->championSlotDispatchAnchor);
    check_int_eq(result->negativeInitialFloorCount, 2,
                 "negative initial floor count", e->commandPickupAnchor);
    check_int_eq(result->negativeFinalFloorCount, 2,
                 "negative floor count preserved", e->commandPickupAnchor);
    check_int_eq(result->negativePanelStable, 1,
                 "negative keeps C040 stable", e->panelAnchor);
    check_uint_eq(result->negativeCandidateOrdinal, 3,
                  "negative keeps G0299 stable", e->reviveOpenAnchor);
    check_int_eq(result->negativeItemLost, 0,
                 "negative does not lose C545", e->commandPickupAnchor);

    check_int_eq(result->rejectsNullState, 1,
                 "guard rejects null state", e->commandPickupAnchor);
    check_int_eq(result->rejectsNullResult, 1,
                 "guard rejects null result", e->commandPickupAnchor);
    check_int_eq(result->rejectsNonContract, 1,
                 "guard rejects non-contract", e->commandPickupAnchor);
    check_int_eq(result->rejectsNoPanel, 1,
                 "guard rejects missing C040 panel", e->panelAnchor);
    check_int_eq(result->rejectsNoCandidate, 1,
                 "guard rejects missing G0299", e->reviveOpenAnchor);
    check_int_eq(result->rejectsHandFull, 1,
                 "guard rejects full hand", e->championSlotDispatchAnchor);
    check_int_eq(result->rejectsNoFloorItem, 1,
                 "guard rejects missing floor item", e->commandPickupAnchor);
    check_int_eq(result->rejectsWrongZone, 1,
                 "guard rejects wrong C545 zone", e->commandDispatchAnchor);

    check_true(result->deterministicHash != 0,
               "deterministic FNV-1a hash is non-zero",
               e->commandPickupAnchor);
}

static unsigned int test_run_sequence(void)
{
    Dm1V1MirrorC545PickupStatePc34Compat state;
    Dm1V1MirrorC545PickupResultPc34Compat result;
    Dm1V1MirrorC545PickupStatePc34Compat state2;
    Dm1V1MirrorC545PickupResultPc34Compat result2;
    int ok;
    int ok2;

    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);
    ok = DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
        &state, &result);
    check_int_eq(ok, 1, "run returns accepted",
                 "COMMAND.C F0380:2045-2159");
    check_run_result(&result);

    check_int_eq(state.leaderHandThing, 0x7545,
                 "state has C545 in hand after close",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(state.floorThingCount, 1,
                 "state floor count after pickup and close",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(state.floorThings[0], 0x5120,
                 "state previous floor item remains",
                 "COMMAND.C F0380:2045-2159");
    check_uint_eq(state.g0299CandidateOrdinal, 3,
                  "state G0299 preserved", "DEFS.H:5694");
    check_int_eq(state.c040PanelOpen, 0,
                 "state C040 closed after clean close",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(state.f0282ReviveClickCount, 0,
                 "state never enters revive click path",
                 "REVIVE.C F0282:744-806");

    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state2);
    ok2 = DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
        &state2, &result2);
    check_int_eq(ok2, 1, "second run returns accepted",
                 "COMMAND.C F0380:2045-2159");
    check_uint_eq(result2.deterministicHash, result.deterministicHash,
                  "FNV-1a hash is deterministic across identical runs",
                  "COMMAND.C F0380:2045-2159");
    check_int_eq(result2.c040Redraws, result.c040Redraws,
                 "redraw count deterministic",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(result2.handTransitions, result.handTransitions,
                 "hand transition count deterministic",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(result2.panelRejections, result.panelRejections,
                 "panel rejection count deterministic",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(result2.mirrorCandidateGuard, result.mirrorCandidateGuard,
                 "mirror guard count deterministic",
                 "REVIVE.C F0280:124-132");

    return result.deterministicHash;
}

static void test_rejects_invalid_inputs(void)
{
    Dm1V1MirrorC545PickupStatePc34Compat state;
    Dm1V1MirrorC545PickupResultPc34Compat result;

    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            NULL, &result),
        0,
        "run rejects null state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);
    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            &state, NULL),
        0,
        "run rejects null result",
        "COMMAND.C F0380:2045-2159");
    state.sourceLockedContractOnly = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects non-contract state",
        "COMMAND.C F0380:2045-2159");
    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);
    state.c040PanelOpen = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects state with no live C040 panel",
        "PANEL.C F0346/F0347:1619-1657");
    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);
    state.g0299CandidateOrdinal = 0;
    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects missing candidate context",
        "REVIVE.C F0280:124-132");
    DM1_V1_MirrorCandidateC545PickupWhilePanelLive_InitPc34Compat(&state);
    state.leaderHandThing = 0x6A31;
    check_int_eq(
        DM1_V1_MirrorCandidateC545PickupWhilePanelLive_RunPc34Compat(
            &state, &result),
        0,
        "run rejects occupied hand",
        "CHAMPION.C F0302:662-713");
    check_int_eq(state.leaderHandThing, 0x6A31,
                 "occupied hand remains unchanged after rejection",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(state.floorThings[0], 0x7545,
                 "rejected pickup leaves C545 on floor",
                 "COMMAND.C F0380:2045-2159");
}

int main(void)
{
    unsigned int hash;

    test_source_metadata();
    test_spec_metadata();
    test_initial_state();
    hash = test_run_sequence();
    test_rejects_invalid_inputs();

    if (gFailures) {
        printf("FAIL test_dm1_v1_mirror_candidate_c545_pickup_while_panel_live_pc34_compat assertions=%d failures=%d hash=0x%08X\n",
               gAssertions,
               gFailures,
               hash);
        return 1;
    }

    printf("PASS test_dm1_v1_mirror_candidate_c545_pickup_while_panel_live_pc34_compat assertions=%d failures=0 c040_redraws=4 hand_transitions=1 panel_rejections=1 mirror_candidate_guard=1 hash=0x%08X\n",
           gAssertions,
           hash);
    return 0;
}
