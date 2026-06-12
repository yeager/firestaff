#include "dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat.h"

#include <stddef.h>
#include <string.h>

/* ReDMCSB source-lock anchors for this contract_only=1 runtime regression:
 * CHEST.C F0333:30-67 and F0334:117-132 pin G0426/G0425 open/close state.
 * CHAMPION.C F0284:93-130 pins G0305 party-order mutation; F0287:243-268
 * pins candidate identity lookup; F0297/F0298:243-298 pin untouched
 * leader-hand state; F0300/F0301/F0302:511-713 pin rejected slot mutation.
 * COMMAND.C F0378:1973-1983 and F0380:2045-2159 pin panel command identity.
 * REVIVE.C F0280:124-132 and F0282:744-806 pin G0299 open/close lifetime.
 * PANEL.C F0344/F0345 and F0346/F0347:1619-1657 pin C040 routing/redraw.
 * UTAMSCR.C F0077/F0078:141-150 pins redraw bracketing.
 * DEFS.H:338-340, 810-817, 1874-1878, 2085-2088, 2088-2096, 2200,
 * 3001-3008, 5694, 5876-5881 pin constants/globals.
 */

enum {
    kSeed = 0x713,
    kCandidateOrdinal = 2,
    kOpenChestThing = 0x6d42,
    kLeaderHandThing = 0x5a71,
    kFirstChestThing = 0x7200,
    kChampionA = 101,
    kChampionB = 202,
    kChampionC = 303,
    kChampionD = 404
};

typedef struct ReshuffleState {
    int contractOnly;
    unsigned int rng;
    int party[DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT];
    int partyCount;
    int candidateOrdinal;
    int g0299CandidateOrdinal;
    int candidateOwnerChampion;
    int panelOpen;
    int panelGraphic;
    int panelContent;
    int lastPanelRedrawChampion;
    int openChestThing;
    int chestSlots[DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT];
    int leaderHandThing;
    int m569ChestPanelSeen;
    int f0284PartyReshuffleCount;
    int f0287CandidateResolveCount;
    int f0280CandidateOpenCount;
    int f0282CandidateCloseCount;
    int f0344PanelRouteCount;
    int f0347PanelRedrawCount;
    int f0378PanelDispatchCount;
    int f0380QueueIdentityCount;
    int mouseEnableCount;
    int mouseDisableCount;
} ReshuffleState;

static const char s_source_anchors[] =
    "CHEST.C F0333:30-67 G0426 open into G0425 visible slots\n"
    "CHEST.C F0334:117-132 G0426 close/relink path remains unchanged\n"
    "CHAMPION.C F0284:93-130 G0305 party order mutation\n"
    "CHAMPION.C F0287:243-268 candidate find/insert identity\n"
    "CHAMPION.C F0297:243-268 leader-hand put path remains untouched\n"
    "CHAMPION.C F0298:270-298 leader-hand remove path remains untouched\n"
    "CHAMPION.C F0300:511-584 slot remove rejected while candidate live\n"
    "CHAMPION.C F0301:606-660 slot add rejected while candidate live\n"
    "CHAMPION.C F0302:662-713 slot-box dispatch rejected while G0299 live\n"
    "COMMAND.C F0378:1973-1983 panel dispatch\n"
    "COMMAND.C F0380:2045-2159 queued command identity\n"
    "REVIVE.C F0280:124-132 candidate open publishes G0299\n"
    "REVIVE.C F0282:744-806 C162 candidate cleanup\n"
    "PANEL.C F0344/F0345 panel click/release routing\n"
    "PANEL.C F0346/F0347:1619-1657 C040 redraw while G0299 is set\n"
    "UTAMSCR.C F0077/F0078:141-150 mouse update bracket\n"
    "DEFS.H:338-340 C162; 810-817 C30..C37; 1874-1878 C38; "
    "2085-2088 G0305 party; 2088-2096 G0423/G0425/G0426; 2200 C040; "
    "3001-3008 M568/M569; 5694 G0299; 5876-5881 G0425/G0426";

static const Dm1V1MirrorCandidateReshuffleContractPc34Compat s_contract = {
    1,
    kSeed,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT,
    kCandidateOrdinal,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C040_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M569_PANEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C162_CANCEL_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C30_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C37_PC34_COMPAT,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C38_PC34_COMPAT,
    kOpenChestThing,
    kLeaderHandThing,
    s_source_anchors,
    "contract_only=1 pass713 DM1 V1 mirror-candidate order reshuffle while "
        "G0299/C040 live; distinct from pass710 C545 drop, pass711 "
        "non-leader C038 pickup, pass707 cancel-after-pickup, pass671 "
        "post-rotation anchor, pass666 thought-project overlay, pass674 "
        "rotation+inventory-click+open-chest, pass702 cross-candidate clear, "
        "pass686 keyboard-browse occupied slot, and resurrect double-race"
};

static Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat s_lastStats;

static void expect_true(int condition)
{
    ++s_lastStats.assertions;
    if (!condition) {
        ++s_lastStats.failures;
    }
}

static void expect_int_eq(int actual, int expected)
{
    ++s_lastStats.assertions;
    if (actual != expected) {
        ++s_lastStats.failures;
    }
}

static unsigned int next_u32(unsigned int *seed)
{
    *seed = (*seed * 1103515245u) + 12345u;
    return *seed;
}

static void seed_state(ReshuffleState *state)
{
    int i;

    memset(state, 0, sizeof(*state));
    state->contractOnly = 1;
    state->rng = kSeed;
    state->partyCount =
        DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT;
    state->party[0] = kChampionA;
    state->party[1] = kChampionB;
    state->party[2] = kChampionC;
    state->party[3] = kChampionD;
    state->candidateOrdinal = kCandidateOrdinal;
    state->g0299CandidateOrdinal = kCandidateOrdinal;
    state->candidateOwnerChampion = state->party[kCandidateOrdinal - 1];
    state->panelOpen = 1;
    state->panelGraphic =
        DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C040_PANEL_PC34_COMPAT;
    state->panelContent =
        DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT;
    state->lastPanelRedrawChampion = state->candidateOwnerChampion;
    state->openChestThing = kOpenChestThing;
    state->leaderHandThing = kLeaderHandThing;
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        state->chestSlots[i] = kFirstChestThing + i;
    }
    state->f0280CandidateOpenCount = 1;
    state->f0287CandidateResolveCount = 1;
    state->f0347PanelRedrawCount = 1;
}

static int candidate_owner_from_order(const ReshuffleState *state)
{
    if (!state || state->candidateOrdinal <= 0 ||
        state->candidateOrdinal > state->partyCount) {
        return DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_NONE_PC34_COMPAT;
    }
    return state->party[state->candidateOrdinal - 1];
}

static int chest_matches_seed(const ReshuffleState *state)
{
    int i;

    if (!state || state->openChestThing != kOpenChestThing) {
        return 0;
    }
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        if (state->chestSlots[i] != kFirstChestThing + i) {
            return 0;
        }
    }
    return 1;
}

static int live_contract_ready(const ReshuffleState *state)
{
    return state && state->contractOnly && state->partyCount == 4 &&
           state->candidateOrdinal == kCandidateOrdinal &&
           state->g0299CandidateOrdinal == kCandidateOrdinal &&
           state->panelOpen &&
           state->panelGraphic ==
               DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C040_PANEL_PC34_COMPAT &&
           state->panelContent ==
               DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT &&
           !state->m569ChestPanelSeen &&
           state->leaderHandThing == kLeaderHandThing &&
           chest_matches_seed(state);
}

static int redraw_for_current_candidate(ReshuffleState *state)
{
    if (!live_contract_ready(state)) {
        return 0;
    }
    ++state->f0287CandidateResolveCount;
    state->candidateOwnerChampion = candidate_owner_from_order(state);
    ++state->mouseEnableCount;
    ++state->f0344PanelRouteCount;
    ++state->f0347PanelRedrawCount;
    state->lastPanelRedrawChampion = state->candidateOwnerChampion;
    ++state->mouseDisableCount;
    return state->candidateOwnerChampion !=
           DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_NONE_PC34_COMPAT;
}

static int swap_party_slots(ReshuffleState *state, int a, int b)
{
    int tmp;

    if (!live_contract_ready(state) || a < 0 || b < 0 ||
        a >= state->partyCount || b >= state->partyCount || a == b) {
        return 0;
    }
    ++state->f0380QueueIdentityCount;
    ++state->f0378PanelDispatchCount;
    ++state->f0284PartyReshuffleCount;
    tmp = state->party[a];
    state->party[a] = state->party[b];
    state->party[b] = tmp;
    return redraw_for_current_candidate(state);
}

static int move_party_slot(ReshuffleState *state, int from, int to)
{
    int moved;
    int i;

    if (!live_contract_ready(state) || from < 0 || to < 0 ||
        from >= state->partyCount || to >= state->partyCount || from == to) {
        return 0;
    }
    ++state->f0380QueueIdentityCount;
    ++state->f0378PanelDispatchCount;
    ++state->f0284PartyReshuffleCount;
    moved = state->party[from];
    if (from < to) {
        for (i = from; i < to; ++i) {
            state->party[i] = state->party[i + 1];
        }
    } else {
        for (i = from; i > to; --i) {
            state->party[i] = state->party[i - 1];
        }
    }
    state->party[to] = moved;
    return redraw_for_current_candidate(state);
}

static int close_candidate_panel(ReshuffleState *state)
{
    if (!live_contract_ready(state) ||
        state->candidateOwnerChampion != candidate_owner_from_order(state)) {
        return 0;
    }
    ++state->f0380QueueIdentityCount;
    ++state->f0378PanelDispatchCount;
    ++state->mouseEnableCount;
    ++state->f0282CandidateCloseCount;
    state->g0299CandidateOrdinal = 0;
    state->panelOpen = 0;
    state->panelGraphic = 0;
    state->panelContent = 0;
    ++state->mouseDisableCount;
    return state->g0299CandidateOrdinal == 0 && !state->panelOpen;
}

static int rejected_mutation_ok(const ReshuffleState *base, int guardKind)
{
    ReshuffleState probe;
    int beforeSlots[DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT];
    int beforeParty[DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT];
    int accepted;
    int i;

    probe = *base;
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        beforeSlots[i] = probe.chestSlots[i];
    }
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        beforeParty[i] = probe.party[i];
    }

    switch (guardKind) {
    case 0:
        probe.contractOnly = 0;
        break;
    case 1:
        probe.g0299CandidateOrdinal = 0;
        break;
    case 2:
        probe.panelOpen = 0;
        break;
    case 3:
        probe.panelContent =
            DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M569_PANEL_PC34_COMPAT;
        probe.m569ChestPanelSeen = 1;
        break;
    case 4:
        probe.openChestThing = 0x1111;
        break;
    case 5:
        probe.chestSlots[3] = 0x2222;
        break;
    case 6:
        probe.leaderHandThing = 0x3333;
        break;
    case 7:
        probe.candidateOrdinal = 5;
        break;
    case 8:
        probe.partyCount = 3;
        break;
    case 9:
        probe.panelGraphic = 0;
        break;
    default:
        return 0;
    }

    accepted = swap_party_slots(&probe, 0, 1);
    expect_int_eq(accepted, 0);
    expect_int_eq(probe.openChestThing,
                  guardKind == 4 ? 0x1111 : kOpenChestThing);
    expect_int_eq(probe.leaderHandThing,
                  guardKind == 6 ? 0x3333 : kLeaderHandThing);
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(probe.chestSlots[i],
                      guardKind == 5 && i == 3 ? 0x2222 : beforeSlots[i]);
    }
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(probe.party[i], beforeParty[i]);
    }
    return !accepted;
}

static void assert_party_state(const ReshuffleState *state,
                               const int expectedParty[],
                               int expectedOwner)
{
    int i;

    expect_true(state != NULL);
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(state->party[i], expectedParty[i]);
    }
    expect_int_eq(state->candidateOwnerChampion, expectedOwner);
    expect_int_eq(state->lastPanelRedrawChampion, expectedOwner);
    expect_int_eq(candidate_owner_from_order(state), expectedOwner);
    expect_int_eq(state->g0299CandidateOrdinal, kCandidateOrdinal);
    expect_int_eq(state->panelOpen, 1);
    expect_int_eq(state->panelGraphic,
                  DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C040_PANEL_PC34_COMPAT);
    expect_int_eq(state->panelContent,
                  DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT);
    expect_int_eq(state->m569ChestPanelSeen, 0);
    expect_int_eq(state->leaderHandThing, kLeaderHandThing);
    expect_true(chest_matches_seed(state));
}

static void assert_seed_contract(void)
{
    const Dm1V1MirrorCandidateReshuffleContractPc34Compat *contract =
        &s_contract;

    expect_int_eq(contract->contractOnly, 1);
    expect_int_eq(contract->seed, kSeed);
    expect_int_eq(contract->partyCount, 4);
    expect_int_eq(contract->candidateOrdinal, kCandidateOrdinal);
    expect_int_eq(contract->c040PanelGraphic, 40);
    expect_int_eq(contract->candidatePanelId, 5);
    expect_int_eq(contract->chestPanelId, 4);
    expect_int_eq(contract->cancelCommand, 162);
    expect_int_eq(contract->firstChestSlotId, 30);
    expect_int_eq(contract->lastChestSlotId, 37);
    expect_int_eq(contract->firstChestSlotBox, 38);
    expect_int_eq(contract->openChestThing, kOpenChestThing);
    expect_int_eq(contract->leaderHandThing, kLeaderHandThing);
    expect_true(contract->sourceAnchors != NULL);
    expect_true(contract->scope != NULL);
}

static void assert_initial_state(const ReshuffleState *state)
{
    int party[] = { kChampionA, kChampionB, kChampionC, kChampionD };
    int i;

    assert_party_state(state, party, kChampionB);
    expect_int_eq(state->contractOnly, 1);
    expect_int_eq(state->rng, kSeed);
    expect_int_eq(state->partyCount, 4);
    expect_int_eq(state->candidateOrdinal, kCandidateOrdinal);
    expect_int_eq(state->f0280CandidateOpenCount, 1);
    expect_int_eq(state->f0287CandidateResolveCount, 1);
    expect_int_eq(state->f0347PanelRedrawCount, 1);
    expect_int_eq(state->f0284PartyReshuffleCount, 0);
    expect_int_eq(state->f0282CandidateCloseCount, 0);
    expect_int_eq(state->f0378PanelDispatchCount, 0);
    expect_int_eq(state->f0380QueueIdentityCount, 0);
    expect_int_eq(state->mouseEnableCount, 0);
    expect_int_eq(state->mouseDisableCount, 0);
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(state->chestSlots[i], kFirstChestThing + i);
    }
}

static void assert_deterministic_noise(ReshuffleState *state)
{
    unsigned int first = next_u32(&state->rng);
    unsigned int second = next_u32(&state->rng);
    unsigned int third = next_u32(&state->rng);

    expect_int_eq((int)(first & 3u), 0);
    expect_int_eq((int)(second & 3u), 1);
    expect_int_eq((int)(third & 3u), 2);
    expect_true(first != second);
    expect_true(second != third);
    expect_int_eq((int)state->rng, (int)third);
}

static void assert_after_close(const ReshuffleState *state)
{
    expect_int_eq(state->g0299CandidateOrdinal, 0);
    expect_int_eq(state->panelOpen, 0);
    expect_int_eq(state->panelGraphic, 0);
    expect_int_eq(state->panelContent, 0);
    expect_int_eq(state->candidateOwnerChampion, kChampionC);
    expect_int_eq(state->lastPanelRedrawChampion, kChampionC);
    expect_int_eq(state->leaderHandThing, kLeaderHandThing);
    expect_int_eq(state->openChestThing, kOpenChestThing);
    expect_true(chest_matches_seed(state));
    expect_int_eq(state->m569ChestPanelSeen, 0);
    expect_int_eq(state->f0282CandidateCloseCount, 1);
    expect_int_eq(state->f0284PartyReshuffleCount, 2);
    expect_int_eq(state->f0287CandidateResolveCount, 3);
    expect_int_eq(state->f0347PanelRedrawCount, 3);
    expect_int_eq(state->f0378PanelDispatchCount, 3);
    expect_int_eq(state->f0380QueueIdentityCount, 3);
    expect_int_eq(state->mouseEnableCount, 3);
    expect_int_eq(state->mouseDisableCount, 3);
}

const Dm1V1MirrorCandidateReshuffleContractPc34Compat *
DM1_V1_MirrorCandidateReshufflePanelLive_ContractPc34Compat(void)
{
    return &s_contract;
}

int run_dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat_self_test(
    void)
{
    ReshuffleState state;
    ReshuffleState guardBase;
    int afterSwap[] = { kChampionB, kChampionA, kChampionC, kChampionD };
    int afterMove[] = { kChampionB, kChampionC, kChampionD, kChampionA };
    int i;
    int accepted;

    memset(&s_lastStats, 0, sizeof(s_lastStats));
    assert_seed_contract();
    seed_state(&state);
    assert_initial_state(&state);
    assert_deterministic_noise(&state);

    guardBase = state;
    for (i = 0; i < 10; ++i) {
        expect_true(rejected_mutation_ok(&guardBase, i));
    }

    accepted = swap_party_slots(&state, 0, 1);
    expect_int_eq(accepted, 1);
    assert_party_state(&state, afterSwap, kChampionA);
    expect_int_eq(state.f0284PartyReshuffleCount, 1);
    expect_int_eq(state.f0287CandidateResolveCount, 2);
    expect_int_eq(state.f0347PanelRedrawCount, 2);
    expect_int_eq(state.f0378PanelDispatchCount, 1);
    expect_int_eq(state.f0380QueueIdentityCount, 1);
    expect_int_eq(state.mouseEnableCount, 1);
    expect_int_eq(state.mouseDisableCount, 1);

    accepted = move_party_slot(&state, 1, 3);
    expect_int_eq(accepted, 1);
    assert_party_state(&state, afterMove, kChampionC);
    expect_int_eq(state.f0284PartyReshuffleCount, 2);
    expect_int_eq(state.f0287CandidateResolveCount, 3);
    expect_int_eq(state.f0347PanelRedrawCount, 3);
    expect_int_eq(state.f0378PanelDispatchCount, 2);
    expect_int_eq(state.f0380QueueIdentityCount, 2);
    expect_int_eq(state.mouseEnableCount, 2);
    expect_int_eq(state.mouseDisableCount, 2);

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(state.chestSlots[i], guardBase.chestSlots[i]);
    }
    expect_int_eq(state.openChestThing, guardBase.openChestThing);
    expect_int_eq(state.leaderHandThing, guardBase.leaderHandThing);
    expect_int_eq(state.panelContent,
                  DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT);
    expect_int_eq(state.m569ChestPanelSeen, 0);

    accepted = close_candidate_panel(&state);
    expect_int_eq(accepted, 1);
    assert_after_close(&state);

    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(state.chestSlots[i], guardBase.chestSlots[i]);
    }
    for (i = 0;
         i < DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT;
         ++i) {
        expect_int_eq(state.party[i], afterMove[i]);
    }
    expect_true(s_lastStats.assertions >= 150);
    return s_lastStats.failures == 0;
}

Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat
DM1_V1_MirrorCandidateReshufflePanelLive_LastSelfTestStatsPc34Compat(void)
{
    return s_lastStats;
}
