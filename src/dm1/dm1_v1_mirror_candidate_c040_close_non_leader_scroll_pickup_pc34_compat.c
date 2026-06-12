#include "dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum {
    kLeaderIndex = 0,
    kPartyCount = 4,
    kCandidateOrdinalBase = 2,
    kLeaderHandNone = 0,
    kScrollIconC038 = 38,
    kInitialPanelRedrawCount = 1,
    kInitialChestOpenCount = 1
};

typedef struct CloseFixture {
    int contractOnly;
    unsigned int seed;
    int route;
    int leaderIndex;
    int nonLeaderIndex;
    int partyCount;
    unsigned int candidateOrdinal;
    unsigned int g0299CandidateOrdinal;
    int panelContent;
    int panelGraphic;
    int c040Visible;
    int cleanRedraw;
    int leaderHandThing;
    int leaderHandIcon;
    int openChestThing;
    int inventoryChampionOrdinal;
    int scrollThing;
    int scrollIcon;
    int activeSlotIndex;
    int activeC30Slot;
    int activeC38SlotBox;
    int activeC537Zone;
    int pickupActive;
    int pickupRollbackCount;
    int pickupCommitCount;
    int chestSlots[DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int zoneChain[DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int f0333OpenCount;
    int f0334CloseCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCleanupCount;
    int f0344PanelClickCount;
    int f0345PanelReleaseCount;
    int f0354PanelCloseRedrawCount;
    int f0378PanelDispatchCount;
    int f0380QueueCount;
    int f0077MouseEnableCount;
    int f0078MouseDisableCount;
    int f0033ObjectIconCount;
    int f0133BlitMaskCount;
    int mouthCloseCount;
    int panelClickCloseCount;
    int panelRedrawCount;
} CloseFixture;

typedef struct CloseSnapshot {
    unsigned int g0299CandidateOrdinal;
    int panelContent;
    int panelGraphic;
    int c040Visible;
    int leaderHandThing;
    int leaderHandIcon;
    int openChestThing;
    int inventoryChampionOrdinal;
    int scrollThing;
    int scrollIcon;
    int activeSlotIndex;
    int activeC30Slot;
    int activeC38SlotBox;
    int activeC537Zone;
    int pickupActive;
    int pickupRollbackCount;
    int pickupCommitCount;
    int chestSlots[DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int zoneChain[DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT];
    int f0333OpenCount;
    int f0334CloseCount;
} CloseSnapshot;

static int gAssertions;
static int gFailures;

static const Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat
    s_evidence = {
        1,
        "CHEST.C F0333:30-67 G0426 open and G0425 materialization",
        "CHEST.C F0334:117-132 close/relink path must stay unused",
        "CHAMPION.C F0284:93-130 G0305 party count",
        "CHAMPION.C F0287:243-268 candidate find/insert",
        "CHAMPION.C F0297:243-268 leader-hand put remains untouched",
        "CHAMPION.C F0298:270-298 leader-hand remove remains untouched",
        "CHAMPION.C F0300:511-584 C30+ remove not rolled back",
        "CHAMPION.C F0301:606-660 C30+ add/write-back not rolled back",
        "CHAMPION.C F0302:662-713 slot-box identity",
        "COMMAND.C F0378:1973-1983 C545/panel dispatch",
        "COMMAND.C F0380:2045-2159 queued command identity",
        "REVIVE.C F0280:124-132 G0299 candidate open",
        "REVIVE.C F0282:744-806 C162 candidate cleanup",
        "PANEL.C F0344/F0345 panel click/release routing",
        "PANEL.C F0346/F0347:1619-1657 C040 redraw while G0299 is set",
        "PANEL.C F0354:2307-2344 panel close/redraw",
        "UTAMSCR.C F0077:147-151/F0078:141-145 mouth-route pointer bracket",
        "OBJECT.C F0033:147-212 object icon identity",
        "BLITMASK.C F0133:30-33 redraw mask",
        "DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; "
            "2085-2088 G0305; 2088-2096 G0423/G0425/G0426; 2200 C040; "
            "3001-3008 M568/M569; 3906-3913 C537..C544; 5694 G0299; "
            "5876-5881 G0425/G0426",
        "contract_only=1 close-after-candidate-open with non-leader active "
            "C038 pickup; distinct from pass711 live C038 pickup, pass710 "
            "C045 drop, pass706 occupied-slot swap, pass701 C539 drop, "
            "pass685 scroll-wheel pull, pass682 leader stack rotation, and "
            "pass672 encumbrance partial-stack"
    };

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes G0425\n"
    "CHEST.C F0334:117-132 closes/relinks G0425 but must not run here\n"
    "CHAMPION.C F0284:93-130 owns G0305 party state\n"
    "CHAMPION.C F0287:243-268 owns candidate find/insert\n"
    "CHAMPION.C F0297:243-268 and CHAMPION.C F0298:270-298 own leader hand put/remove\n"
    "CHAMPION.C F0300:511-584, CHAMPION.C F0301:606-660, "
    "CHAMPION.C F0302:662-713 own C30+ slots\n"
    "COMMAND.C F0378:1973-1983 dispatches C545/panel input\n"
    "COMMAND.C F0380:2045-2159 preserves queued command identity\n"
    "REVIVE.C F0280:124-132 opens G0299/C040\n"
    "REVIVE.C F0282:744-806 clears G0299 via C162 cleanup\n"
    "PANEL.C F0344/F0345, F0346/F0347:1619-1657, F0354:2307-2344 "
    "route clicks, redraw C040, and close/redraw the panel\n"
    "UTAMSCR.C F0077:147-151/F0078:141-145 brackets mouth-route redraw\n"
    "OBJECT.C F0033:147-212 and BLITMASK.C F0133:30-33 preserve redraw identity\n"
    "DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; "
    "2085-2088 G0305; 2088-2096 G0423/G0425/G0426; 2200 C040; "
    "3001-3008 M568/M569; 3906-3913 C537..C544; 5694 G0299; "
    "5876-5881 G0425/G0426";

static uint32_t next_seed(uint32_t *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static int thing_from_seed(uint32_t *seed, int tag)
{
    return tag | (int)(next_seed(seed) & 0x0fffu);
}

static void copy_ints(int dst[], const int src[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static int arrays_equal(const int a[], const int b[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void snapshot_fixture(const CloseFixture *fixture,
                             CloseSnapshot *snapshot)
{
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->g0299CandidateOrdinal = fixture->g0299CandidateOrdinal;
    snapshot->panelContent = fixture->panelContent;
    snapshot->panelGraphic = fixture->panelGraphic;
    snapshot->c040Visible = fixture->c040Visible;
    snapshot->leaderHandThing = fixture->leaderHandThing;
    snapshot->leaderHandIcon = fixture->leaderHandIcon;
    snapshot->openChestThing = fixture->openChestThing;
    snapshot->inventoryChampionOrdinal = fixture->inventoryChampionOrdinal;
    snapshot->scrollThing = fixture->scrollThing;
    snapshot->scrollIcon = fixture->scrollIcon;
    snapshot->activeSlotIndex = fixture->activeSlotIndex;
    snapshot->activeC30Slot = fixture->activeC30Slot;
    snapshot->activeC38SlotBox = fixture->activeC38SlotBox;
    snapshot->activeC537Zone = fixture->activeC537Zone;
    snapshot->pickupActive = fixture->pickupActive;
    snapshot->pickupRollbackCount = fixture->pickupRollbackCount;
    snapshot->pickupCommitCount = fixture->pickupCommitCount;
    copy_ints(snapshot->chestSlots,
              fixture->chestSlots,
              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT);
    copy_ints(snapshot->zoneChain,
              fixture->zoneChain,
              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT);
    snapshot->f0333OpenCount = fixture->f0333OpenCount;
    snapshot->f0334CloseCount = fixture->f0334CloseCount;
}

static void seed_fixture(CloseFixture *fixture, unsigned int seedValue,
                         int route)
{
    uint32_t seed = seedValue;
    int i;

    memset(fixture, 0, sizeof(*fixture));
    fixture->contractOnly = 1;
    fixture->seed = seedValue;
    fixture->route = route;
    fixture->leaderIndex = kLeaderIndex;
    fixture->nonLeaderIndex = 1 + (int)(next_seed(&seed) % 3u);
    fixture->partyCount = kPartyCount;
    fixture->candidateOrdinal =
        (unsigned int)(kCandidateOrdinalBase + (int)(next_seed(&seed) % 2u));
    fixture->g0299CandidateOrdinal = fixture->candidateOrdinal;
    fixture->panelContent =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M568_CANDIDATE_PC34_COMPAT;
    fixture->panelGraphic =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C040_PC34_COMPAT;
    fixture->c040Visible = 1;
    fixture->leaderHandThing = kLeaderHandNone;
    fixture->leaderHandIcon =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT;
    fixture->openChestThing = thing_from_seed(&seed, 0x6400);
    fixture->inventoryChampionOrdinal = fixture->nonLeaderIndex + 1;
    fixture->scrollThing = thing_from_seed(&seed, 0x7038);
    fixture->scrollIcon = kScrollIconC038;
    fixture->activeSlotIndex = (int)(next_seed(&seed) %
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT);
    fixture->activeC30Slot =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C30_FIRST_PC34_COMPAT +
        fixture->activeSlotIndex;
    fixture->activeC38SlotBox =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C38_FIRST_PC34_COMPAT +
        fixture->activeSlotIndex;
    fixture->activeC537Zone =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT +
        fixture->activeSlotIndex;
    fixture->pickupActive = 1;
    for (i = 0;
         i <
         DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        fixture->chestSlots[i] = thing_from_seed(&seed, 0x7200 + (i << 8));
        fixture->zoneChain[i] =
            DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT +
            i;
    }
    fixture->chestSlots[fixture->activeSlotIndex] = fixture->scrollThing;
    fixture->f0333OpenCount = kInitialChestOpenCount;
    fixture->f0280CandidateOpenCount = 1;
    fixture->panelRedrawCount = kInitialPanelRedrawCount;
}

static int fixture_ready(const CloseFixture *fixture)
{
    if (!fixture || !fixture->contractOnly) {
        return 0;
    }
    if (fixture->route !=
            DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT &&
        fixture->route !=
            DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT) {
        return 0;
    }
    return fixture->leaderIndex == kLeaderIndex &&
           fixture->nonLeaderIndex != fixture->leaderIndex &&
           fixture->partyCount == kPartyCount &&
           fixture->g0299CandidateOrdinal == fixture->candidateOrdinal &&
           fixture->panelContent ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M568_CANDIDATE_PC34_COMPAT &&
           fixture->panelGraphic ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C040_PC34_COMPAT &&
           fixture->c040Visible &&
           fixture->leaderHandThing == kLeaderHandNone &&
           fixture->leaderHandIcon ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT &&
           fixture->openChestThing !=
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT &&
           fixture->scrollIcon == kScrollIconC038 &&
           fixture->pickupActive &&
           fixture->pickupRollbackCount == 0 &&
           fixture->pickupCommitCount == 0 &&
           fixture->activeSlotIndex >= 0 &&
           fixture->activeSlotIndex <
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT &&
           fixture->activeC30Slot ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C30_FIRST_PC34_COMPAT +
                   fixture->activeSlotIndex &&
           fixture->activeC38SlotBox ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C38_FIRST_PC34_COMPAT +
                   fixture->activeSlotIndex &&
           fixture->activeC537Zone ==
               DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT +
                   fixture->activeSlotIndex &&
           fixture->chestSlots[fixture->activeSlotIndex] ==
               fixture->scrollThing &&
           fixture->zoneChain[fixture->activeSlotIndex] ==
               fixture->activeC537Zone;
}

static int close_after_candidate_open(CloseFixture *fixture)
{
    if (!fixture_ready(fixture)) {
        return 0;
    }

    ++fixture->f0380QueueCount;
    ++fixture->f0378PanelDispatchCount;
    ++fixture->f0077MouseEnableCount;
    if (fixture->route ==
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT) {
        ++fixture->mouthCloseCount;
    } else {
        ++fixture->panelClickCloseCount;
        ++fixture->f0344PanelClickCount;
        ++fixture->f0345PanelReleaseCount;
    }
    ++fixture->f0282CandidateCleanupCount;
    fixture->g0299CandidateOrdinal = 0u;
    fixture->c040Visible = 0;
    fixture->panelContent =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M569_CHEST_PC34_COMPAT;
    fixture->panelGraphic = 0;
    ++fixture->f0354PanelCloseRedrawCount;
    ++fixture->panelRedrawCount;
    fixture->cleanRedraw = 1;
    fixture->f0033ObjectIconCount +=
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
    fixture->f0133BlitMaskCount += 1;
    ++fixture->f0078MouseDisableCount;
    return 1;
}

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
        printf("FAIL: %s actual=%u expected=%u [%s]\n",
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

static void check_evidence(void)
{
    const Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat *e =
        dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_evidence_pc34_compat();
    const char *text =
        dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_source_evidence_pc34_compat();

    check_true(e != NULL && e->contractOnly == 1,
               "evidence is contract-only", "COMMAND.C F0380:2045-2159");
    check_contains(e->chestOpenAnchor, "CHEST.C F0333:30-67",
                   "evidence cites F0333", e->chestOpenAnchor);
    check_contains(e->chestCloseAnchor, "CHEST.C F0334:117-132",
                   "evidence cites F0334", e->chestCloseAnchor);
    check_contains(e->partyAnchor, "CHAMPION.C F0284:93-130",
                   "evidence cites F0284", e->partyAnchor);
    check_contains(e->candidateFindInsertAnchor, "CHAMPION.C F0287:243-268",
                   "evidence cites F0287", e->candidateFindInsertAnchor);
    check_contains(e->leaderHandPutAnchor, "CHAMPION.C F0297:243-268",
                   "evidence cites F0297", e->leaderHandPutAnchor);
    check_contains(e->leaderHandRemoveAnchor, "CHAMPION.C F0298:270-298",
                   "evidence cites F0298", e->leaderHandRemoveAnchor);
    check_contains(e->slotRemoveAnchor, "CHAMPION.C F0300:511-584",
                   "evidence cites F0300", e->slotRemoveAnchor);
    check_contains(e->slotAddAnchor, "CHAMPION.C F0301:606-660",
                   "evidence cites F0301", e->slotAddAnchor);
    check_contains(e->slotDispatchAnchor, "CHAMPION.C F0302:662-713",
                   "evidence cites F0302", e->slotDispatchAnchor);
    check_contains(e->panelDispatchAnchor, "COMMAND.C F0378:1973-1983",
                   "evidence cites F0378", e->panelDispatchAnchor);
    check_contains(e->queueAnchor, "COMMAND.C F0380:2045-2159",
                   "evidence cites F0380", e->queueAnchor);
    check_contains(e->candidateOpenAnchor, "REVIVE.C F0280:124-132",
                   "evidence cites F0280", e->candidateOpenAnchor);
    check_contains(e->candidateCleanupAnchor, "REVIVE.C F0282:744-806",
                   "evidence cites F0282", e->candidateCleanupAnchor);
    check_contains(e->panelClickAnchor, "PANEL.C F0344/F0345",
                   "evidence cites F0344/F0345", e->panelClickAnchor);
    check_contains(e->panelC040RedrawAnchor, "PANEL.C F0346/F0347:1619-1657",
                   "evidence cites C040 redraw", e->panelC040RedrawAnchor);
    check_contains(e->panelCloseRedrawAnchor, "PANEL.C F0354:2307-2344",
                   "evidence cites close redraw", e->panelCloseRedrawAnchor);
    check_contains(e->mouseRouteAnchor, "UTAMSCR.C F0077:147-151",
                   "evidence cites F0077", e->mouseRouteAnchor);
    check_contains(e->mouseRouteAnchor, "F0078:141-145",
                   "evidence cites F0078", e->mouseRouteAnchor);
    check_contains(e->objectAnchor, "OBJECT.C F0033:147-212",
                   "evidence cites F0033", e->objectAnchor);
    check_contains(e->blitMaskAnchor, "BLITMASK.C F0133:30-33",
                   "evidence cites F0133", e->blitMaskAnchor);
    check_contains(e->defsAnchor, "338-340 C162",
                   "defs cites C162", e->defsAnchor);
    check_contains(e->defsAnchor, "810-817 C30..C37",
                   "defs cites C30..C37", e->defsAnchor);
    check_contains(e->defsAnchor, "1874-1878 C38",
                   "defs cites C38", e->defsAnchor);
    check_contains(e->defsAnchor, "2085-2088 G0305",
                   "defs cites G0305", e->defsAnchor);
    check_contains(e->defsAnchor, "2088-2096 G0423/G0425/G0426",
                   "defs cites G0423/G0425/G0426", e->defsAnchor);
    check_contains(e->defsAnchor, "2200 C040",
                   "defs cites C040", e->defsAnchor);
    check_contains(e->defsAnchor, "3001-3008 M568/M569",
                   "defs cites M568/M569", e->defsAnchor);
    check_contains(e->defsAnchor, "3906-3913 C537..C544",
                   "defs cites zones", e->defsAnchor);
    check_contains(e->defsAnchor, "5694 G0299",
                   "defs cites G0299", e->defsAnchor);
    check_contains(e->defsAnchor, "5876-5881 G0425/G0426",
                   "defs cites G0425/G0426", e->defsAnchor);
    check_contains(e->nonDuplicationScope, "pass711",
                   "scope distinguishes pass711", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass710",
                   "scope distinguishes pass710", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass706",
                   "scope distinguishes pass706", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass701",
                   "scope distinguishes pass701", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass685",
                   "scope distinguishes pass685", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass682",
                   "scope distinguishes pass682", e->nonDuplicationScope);
    check_contains(e->nonDuplicationScope, "pass672",
                   "scope distinguishes pass672", e->nonDuplicationScope);

    check_contains(text, "CHEST.C F0333:30-67",
                   "source cites F0333", e->chestOpenAnchor);
    check_contains(text, "CHEST.C F0334:117-132",
                   "source cites F0334", e->chestCloseAnchor);
    check_contains(text, "CHAMPION.C F0284:93-130",
                   "source cites F0284", e->partyAnchor);
    check_contains(text, "CHAMPION.C F0287:243-268",
                   "source cites F0287", e->candidateFindInsertAnchor);
    check_contains(text, "CHAMPION.C F0297:243-268",
                   "source cites F0297", e->leaderHandPutAnchor);
    check_contains(text, "CHAMPION.C F0298:270-298",
                   "source cites F0298", e->leaderHandRemoveAnchor);
    check_contains(text, "CHAMPION.C F0300:511-584",
                   "source cites F0300", e->slotRemoveAnchor);
    check_contains(text, "CHAMPION.C F0301:606-660",
                   "source cites F0301", e->slotAddAnchor);
    check_contains(text, "CHAMPION.C F0302:662-713",
                   "source cites F0302", e->slotDispatchAnchor);
    check_contains(text, "COMMAND.C F0378:1973-1983",
                   "source cites F0378", e->panelDispatchAnchor);
    check_contains(text, "COMMAND.C F0380:2045-2159",
                   "source cites F0380", e->queueAnchor);
    check_contains(text, "REVIVE.C F0280:124-132",
                   "source cites F0280", e->candidateOpenAnchor);
    check_contains(text, "REVIVE.C F0282:744-806",
                   "source cites F0282", e->candidateCleanupAnchor);
    check_contains(text, "PANEL.C F0344/F0345",
                   "source cites F0344/F0345", e->panelClickAnchor);
    check_contains(text, "F0346/F0347:1619-1657",
                   "source cites C040 redraw", e->panelC040RedrawAnchor);
    check_contains(text, "F0354:2307-2344",
                   "source cites close redraw", e->panelCloseRedrawAnchor);
    check_contains(text, "UTAMSCR.C F0077:147-151/F0078:141-145",
                   "source cites mouse route", e->mouseRouteAnchor);
    check_contains(text, "OBJECT.C F0033:147-212",
                   "source cites object", e->objectAnchor);
    check_contains(text, "BLITMASK.C F0133:30-33",
                   "source cites blitmask", e->blitMaskAnchor);
}

static void check_initial_fixture(const CloseFixture *fixture)
{
    int i;

    check_int_eq(fixture->contractOnly, 1, "initial contract-only flag",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(fixture->leaderIndex, kLeaderIndex, "initial leader index",
                 "CHAMPION.C F0297:243-268");
    check_true(fixture->nonLeaderIndex != fixture->leaderIndex,
               "initial pickup owner is non-leader",
               "CHAMPION.C F0302:662-713");
    check_int_eq(fixture->partyCount, kPartyCount, "initial party count",
                 "CHAMPION.C F0284:93-130");
    check_uint_eq(fixture->g0299CandidateOrdinal,
                  fixture->candidateOrdinal,
                  "initial G0299 candidate set",
                  "REVIVE.C F0280:124-132");
    check_int_eq(fixture->panelContent,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M568_CANDIDATE_PC34_COMPAT,
                 "initial panel is M568 candidate",
                 "DEFS.H:3001-3008 M568/M569");
    check_int_eq(fixture->panelGraphic,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C040_PC34_COMPAT,
                 "initial panel graphic is C040",
                 "DEFS.H:2200 C040");
    check_int_eq(fixture->c040Visible, 1, "initial C040 visible",
                 "PANEL.C F0346/F0347:1619-1657");
    check_int_eq(fixture->leaderHandThing, kLeaderHandNone,
                 "initial leader hand empty", "CHAMPION.C F0297:243-268");
    check_int_eq(fixture->leaderHandIcon,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT,
                 "initial leader hand icon empty",
                 "CHAMPION.C F0298:270-298");
    check_true(fixture->openChestThing !=
                   DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT,
               "initial G0426 open chest exists", "CHEST.C F0333:30-67");
    check_int_eq(fixture->scrollIcon, kScrollIconC038,
                 "initial active pickup icon is C038",
                 "OBJECT.C F0033:147-212");
    check_int_eq(fixture->pickupActive, 1,
                 "initial non-leader pickup active",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(fixture->pickupRollbackCount, 0,
                 "initial rollback count zero",
                 "CHAMPION.C F0300:511-584");
    check_int_eq(fixture->pickupCommitCount, 0,
                 "initial commit count zero",
                 "CHAMPION.C F0301:606-660");
    check_int_eq(fixture->activeC30Slot,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C30_FIRST_PC34_COMPAT +
                     fixture->activeSlotIndex,
                 "initial C30+ slot identity", "DEFS.H:810-817 C30..C37");
    check_int_eq(fixture->activeC38SlotBox,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C38_FIRST_PC34_COMPAT +
                     fixture->activeSlotIndex,
                 "initial C38+ slot-box identity", "DEFS.H:1874-1878 C38");
    check_int_eq(fixture->activeC537Zone,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT +
                     fixture->activeSlotIndex,
                 "initial C537+ redraw zone identity",
                 "DEFS.H:3906-3913 C537..C544");
    check_int_eq(fixture->activeC537Zone <=
                     DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C544_LAST_PC34_COMPAT,
                 1,
                 "initial active zone remains within C537..C544",
                 "DEFS.H:3906-3913 C537..C544");
    check_int_eq(fixture->chestSlots[fixture->activeSlotIndex],
                 fixture->scrollThing,
                 "initial active G0425 slot contains C038 scroll",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(fixture->zoneChain[fixture->activeSlotIndex],
                 fixture->activeC537Zone,
                 "initial C537..C544 chain maps active slot",
                 "DEFS.H:3906-3913 C537..C544");
    check_int_eq(fixture->f0333OpenCount, kInitialChestOpenCount,
                 "initial F0333 open count",
                 "CHEST.C F0333:30-67");
    check_int_eq(fixture->f0334CloseCount, 0,
                 "initial F0334 close count zero",
                 "CHEST.C F0334:117-132");
    check_int_eq(fixture->f0280CandidateOpenCount, 1,
                 "initial F0280 candidate open count",
                 "REVIVE.C F0280:124-132");
    check_int_eq(fixture->panelRedrawCount, kInitialPanelRedrawCount,
                 "initial C040 redraw count",
                 "PANEL.C F0346/F0347:1619-1657");
    for (i = 0;
         i <
         DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        check_int_eq(fixture->zoneChain[i],
                     DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT +
                         i,
                     "initial C537..C544 zone chain",
                     "DEFS.H:3906-3913 C537..C544");
        check_true(fixture->chestSlots[i] !=
                       DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT,
                   "initial G0425 slot occupied for preservation check",
                   "CHEST.C F0333:30-67");
    }
}

static void check_close_result(const CloseSnapshot *before,
                               const CloseFixture *after)
{
    int i;

    check_uint_eq(after->g0299CandidateOrdinal, 0u,
                  "close clears G0299",
                  "REVIVE.C F0282:744-806");
    check_int_eq(after->c040Visible, 0, "close hides C040 candidate panel",
                 "PANEL.C F0354:2307-2344");
    check_int_eq(after->panelContent,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M569_CHEST_PC34_COMPAT,
                 "close redraws clean M569 chest panel",
                 "DEFS.H:3001-3008 M568/M569");
    check_int_eq(after->cleanRedraw, 1,
                 "close records clean panel redraw",
                 "PANEL.C F0354:2307-2344");
    check_int_eq(after->leaderHandThing, before->leaderHandThing,
                 "leader hand thing untouched",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(after->leaderHandIcon, before->leaderHandIcon,
                 "leader hand icon untouched",
                 "CHAMPION.C F0298:270-298");
    check_int_eq(after->openChestThing, before->openChestThing,
                 "G0426 open chest unchanged",
                 "CHEST.C F0333:30-67");
    check_int_eq(after->inventoryChampionOrdinal,
                 before->inventoryChampionOrdinal,
                 "G0423 inventory champion unchanged",
                 "DEFS.H:2088-2096 G0423/G0425/G0426");
    check_int_eq(after->scrollThing, before->scrollThing,
                 "active C038 scroll identity unchanged",
                 "OBJECT.C F0033:147-212");
    check_int_eq(after->scrollIcon, before->scrollIcon,
                 "active C038 scroll icon unchanged",
                 "OBJECT.C F0033:147-212");
    check_int_eq(after->pickupActive, before->pickupActive,
                 "non-leader pickup remains active",
                 "CHAMPION.C F0302:662-713");
    check_int_eq(after->pickupRollbackCount, before->pickupRollbackCount,
                 "non-leader pickup not rolled back",
                 "CHAMPION.C F0300:511-584");
    check_int_eq(after->pickupCommitCount, before->pickupCommitCount,
                 "non-leader pickup not committed during close",
                 "CHAMPION.C F0301:606-660");
    check_int_eq(after->activeSlotIndex, before->activeSlotIndex,
                 "active slot index unchanged", "DEFS.H:810-817 C30..C37");
    check_int_eq(after->activeC30Slot, before->activeC30Slot,
                 "active C30+ slot unchanged", "DEFS.H:810-817 C30..C37");
    check_int_eq(after->activeC38SlotBox, before->activeC38SlotBox,
                 "active C38+ slot-box unchanged", "DEFS.H:1874-1878 C38");
    check_int_eq(after->activeC537Zone, before->activeC537Zone,
                 "active C537+ zone unchanged",
                 "DEFS.H:3906-3913 C537..C544");
    check_int_eq(after->f0333OpenCount, before->f0333OpenCount,
                 "F0333 open count unchanged by close",
                 "CHEST.C F0333:30-67");
    check_int_eq(after->f0334CloseCount, before->f0334CloseCount,
                 "F0334 chest close not called",
                 "CHEST.C F0334:117-132");
    check_int_eq(after->f0282CandidateCleanupCount, 1,
                 "F0282 cleanup called exactly once",
                 "REVIVE.C F0282:744-806");
    check_int_eq(after->f0354PanelCloseRedrawCount, 1,
                 "F0354 close/redraw called once",
                 "PANEL.C F0354:2307-2344");
    check_int_eq(after->f0378PanelDispatchCount, 1,
                 "panel dispatch count one",
                 "COMMAND.C F0378:1973-1983");
    check_int_eq(after->f0380QueueCount, 1,
                 "queue identity count one",
                 "COMMAND.C F0380:2045-2159");
    check_int_eq(after->f0077MouseEnableCount, 1,
                 "mouse enable bracket count one",
                 "UTAMSCR.C F0077:147-151");
    check_int_eq(after->f0078MouseDisableCount, 1,
                 "mouse disable bracket count one",
                 "UTAMSCR.C F0078:141-145");
    check_int_eq(after->f0033ObjectIconCount,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT,
                 "redraw checks all chest slot object icons",
                 "OBJECT.C F0033:147-212");
    check_int_eq(after->f0133BlitMaskCount, 1,
                 "redraw runs mask path once",
                 "BLITMASK.C F0133:30-33");
    check_int_eq(after->panelRedrawCount, before->panelContent ==
                     DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M568_CANDIDATE_PC34_COMPAT
                         ? kInitialPanelRedrawCount + 1
                         : after->panelRedrawCount,
                 "panel redraw count advanced from C040 to M569",
                 "PANEL.C F0354:2307-2344");
    check_int_eq(arrays_equal(after->chestSlots,
                              before->chestSlots,
                              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT),
                 1,
                 "G0425 chest slots unchanged",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(arrays_equal(after->zoneChain,
                              before->zoneChain,
                              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT),
                 1,
                 "C537..C544 slot chain unchanged",
                 "DEFS.H:3906-3913 C537..C544");
    check_int_eq(after->chestSlots[after->activeSlotIndex],
                 before->scrollThing,
                 "non-leader C30+ slot still has picked C038",
                 "CHAMPION.C F0302:662-713");
    if (after->route ==
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT) {
        check_int_eq(after->mouthCloseCount, 1,
                     "mouth close route observed",
                     "UTAMSCR.C F0077:147-151/F0078:141-145");
        check_int_eq(after->panelClickCloseCount, 0,
                     "mouth route does not count panel click",
                     "PANEL.C F0344/F0345");
        check_int_eq(after->f0344PanelClickCount, 0,
                     "mouth route does not call F0344",
                     "PANEL.C F0344/F0345");
        check_int_eq(after->f0345PanelReleaseCount, 0,
                     "mouth route does not call F0345",
                     "PANEL.C F0344/F0345");
    } else {
        check_int_eq(after->mouthCloseCount, 0,
                     "panel route does not count mouth close",
                     "UTAMSCR.C F0077:147-151/F0078:141-145");
        check_int_eq(after->panelClickCloseCount, 1,
                     "panel click close route observed",
                     "PANEL.C F0344/F0345");
        check_int_eq(after->f0344PanelClickCount, 1,
                     "panel route calls F0344",
                     "PANEL.C F0344/F0345");
        check_int_eq(after->f0345PanelReleaseCount, 1,
                     "panel route calls F0345",
                     "PANEL.C F0344/F0345");
    }
    for (i = 0;
         i <
         DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        check_int_eq(after->chestSlots[i], before->chestSlots[i],
                     "per-slot G0425 unchanged",
                     "CHEST.C F0333:30-67");
        check_int_eq(after->zoneChain[i], before->zoneChain[i],
                     "per-slot C537..C544 chain unchanged",
                     "DEFS.H:3906-3913 C537..C544");
    }
}

static void run_valid_route(unsigned int seed, int route)
{
    CloseFixture fixture;
    CloseSnapshot before;
    int accepted;

    seed_fixture(&fixture, seed, route);
    check_initial_fixture(&fixture);
    snapshot_fixture(&fixture, &before);
    accepted = close_after_candidate_open(&fixture);
    check_int_eq(accepted, 1, "close route accepted",
                 "COMMAND.C F0380:2045-2159");
    check_close_result(&before, &fixture);
}

static void check_rejection_unchanged(CloseFixture fixture,
                                      const char *message)
{
    CloseSnapshot before;
    int accepted;

    snapshot_fixture(&fixture, &before);
    accepted = close_after_candidate_open(&fixture);
    check_int_eq(accepted, 0, message, "COMMAND.C F0380:2045-2159");
    check_uint_eq(fixture.g0299CandidateOrdinal,
                  before.g0299CandidateOrdinal,
                  "rejected mutation leaves G0299 unchanged",
                  "REVIVE.C F0282:744-806");
    check_int_eq(fixture.panelContent, before.panelContent,
                 "rejected mutation leaves panel content unchanged",
                 "PANEL.C F0354:2307-2344");
    check_int_eq(fixture.leaderHandThing, before.leaderHandThing,
                 "rejected mutation leaves leader hand unchanged",
                 "CHAMPION.C F0297:243-268");
    check_int_eq(fixture.openChestThing, before.openChestThing,
                 "rejected mutation leaves G0426 unchanged",
                 "CHEST.C F0333:30-67");
    check_int_eq(fixture.pickupRollbackCount, before.pickupRollbackCount,
                 "rejected mutation does not roll back pickup",
                 "CHAMPION.C F0300:511-584");
    check_int_eq(arrays_equal(fixture.chestSlots,
                              before.chestSlots,
                              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT),
                 1,
                 "rejected mutation leaves G0425 unchanged",
                 "DEFS.H:5876-5881 G0425/G0426");
    check_int_eq(arrays_equal(fixture.zoneChain,
                              before.zoneChain,
                              DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT),
                 1,
                 "rejected mutation leaves C537..C544 unchanged",
                 "DEFS.H:3906-3913 C537..C544");
}

static void run_rejection_cases(void)
{
    CloseFixture fixture;

    seed_fixture(&fixture,
                 0x7150u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT);
    fixture.contractOnly = 0;
    check_rejection_unchanged(fixture, "rejects non-contract state");

    seed_fixture(&fixture,
                 0x7151u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT);
    fixture.g0299CandidateOrdinal = 0u;
    check_rejection_unchanged(fixture, "rejects missing G0299 candidate");

    seed_fixture(&fixture,
                 0x7152u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT);
    fixture.c040Visible = 0;
    check_rejection_unchanged(fixture, "rejects hidden C040 panel");

    seed_fixture(&fixture,
                 0x7153u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT);
    fixture.panelContent =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M569_CHEST_PC34_COMPAT;
    check_rejection_unchanged(fixture, "rejects pre-redrawn M569 panel");

    seed_fixture(&fixture,
                 0x7154u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT);
    fixture.leaderHandThing = 0x6601;
    check_rejection_unchanged(fixture, "rejects occupied leader hand");

    seed_fixture(&fixture,
                 0x7155u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT);
    fixture.pickupActive = 0;
    check_rejection_unchanged(fixture, "rejects inactive non-leader pickup");

    seed_fixture(&fixture,
                 0x7156u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT);
    fixture.chestSlots[fixture.activeSlotIndex] = 0x7711;
    check_rejection_unchanged(fixture, "rejects active slot without C038");

    seed_fixture(&fixture,
                 0x7157u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT);
    fixture.openChestThing =
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT;
    check_rejection_unchanged(fixture, "rejects missing G0426 open chest");

    seed_fixture(&fixture,
                 0x7158u,
                 DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT);
    fixture.nonLeaderIndex = fixture.leaderIndex;
    check_rejection_unchanged(fixture, "rejects leader-owned pickup");

    seed_fixture(&fixture,
                 0x7159u,
                 99);
    check_rejection_unchanged(fixture, "rejects unknown close route");
}

const Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat *
dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_evidence_pc34_compat(
    void)
{
    return &s_evidence;
}

const char *
dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_source_evidence_pc34_compat(
    void)
{
    return s_source_evidence;
}

int run_dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat_self_test(
    void)
{
    unsigned int seeds[] = {
        0x71500001u,
        0x71500002u,
        0x71500003u,
        0x71500004u,
        0x71500005u,
        0x71500006u,
        0x71500007u,
        0x71500008u
    };
    int routes[] = {
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT,
        DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT
    };
    int i;
    int j;

    gAssertions = 0;
    gFailures = 0;
    check_evidence();
    for (i = 0; i < (int)(sizeof(seeds) / sizeof(seeds[0])); ++i) {
        for (j = 0; j < (int)(sizeof(routes) / sizeof(routes[0])); ++j) {
            run_valid_route(seeds[i], routes[j]);
        }
    }
    run_rejection_cases();
    return gFailures == 0 && gAssertions >= 150;
}

int dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_assertions_pc34_compat(
    void)
{
    return gAssertions;
}

int dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_failures_pc34_compat(
    void)
{
    return gFailures;
}
