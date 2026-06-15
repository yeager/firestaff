/* ReDMCSB CHEST.C F0333:30-67 and F0334:117-132; CHAMPION.C
 * F0297:243-268, F0298:270-298, F0300:511-515, F0301:606-614,
 * F0302:662-714, and F0284:93-131; PANEL.C F0344:1390-1406,
 * F0345, F0352, and F0354:2307-2344; UTAMSCR.C F0077:147-151
 * and F0078:141-145; OBJECT.C F0033:147-212; BLITMASK.C
 * F0133:30-33; DEFS.H C10, C30..C36, C38, C040, M568/M569,
 * C537..C544, G0299, and G0425/G0426. */
#include "dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "CHEST.C F0333:30-67 opens G0426 and materializes visible C537..C544 from a container chain\n"
    "CHEST.C F0334:117-132 clears G0426, clears G0425, and rewrites only non-empty visible C537..C544 entries\n"
    "CHAMPION.C F0297:243-268 owns leader-hand put state while visible slots are live\n"
    "CHAMPION.C F0298:270-298 owns leader-hand remove state while visible slots are live\n"
    "CHAMPION.C F0300:511-515 removes C30+ chest slots through G0425\n"
    "CHAMPION.C F0301:606-614 adds C30+ chest slots through G0425\n"
    "CHAMPION.C F0302:662-714 walks C30+ slot boxes and rejects candidate-owned hand boxes\n"
    "CHAMPION.C F0284:93-131 switches inventory champions around panel/chest state\n"
    "PANEL.C F0344:1390-1406 and F0345 route C040 panel draw/click hooks\n"
    "PANEL.C F0352 redraws C040 candidate state after close side effects\n"
    "PANEL.C F0354:2307-2344 closes inventory, calls F0334, and redraws the C025 chest panel on reopen\n"
    "UTAMSCR.C F0077:147-151 and F0078:141-145 bracket hand/panel redraw updates\n"
    "OBJECT.C F0033:147-212 maps object identity to visible chest icons\n"
    "BLITMASK.C F0133:30-33 documents transparency blit dispatch used by reopen redraws\n"
    "DEFS.H:2088 C10_COLOR_FLESH; 810-816 C30..C36; 1874-1878 C38; 2200 C040; "
    "3001-3008 M568/M569; 3906-3913 C537..C544; 5694 G0299; 5876-5881 G0425/G0426";

const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat
    dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_data_pc34_compat = {
        "pass723 contract-only close G0426 while C040 candidate live then reopen on another champion",
        0xC537C040u,
        2,
        1,
        2,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C544_LAST,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C040_PANEL_CANDIDATE,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C025_PANEL_CHEST
    };

static uint32_t next_seed_pc34_compat(uint32_t* seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return *seed;
}

static M11_Item make_seeded_item_pc34_compat(uint32_t* seed,
                                             int tag,
                                             int weightBase)
{
    M11_Item item;

    memset(&item, 0, sizeof(item));
    item.itemType = tag | (int)(next_seed_pc34_compat(seed) & 0x00ffu);
    item.weight = weightBase + (int)(next_seed_pc34_compat(seed) % 5u);
    item.charges = 1 + (int)(next_seed_pc34_compat(seed) % 7u);
    item.identified = 1;
    item.allowedSlots = DM1_PC34_ALLOWED_CONTAINER;
    return item;
}

static void item_types_pc34_compat(const M11_Item* items,
                                   int count,
                                   int* types)
{
    int i;

    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        types[i] = (items && i < count) ? items[i].itemType : 0;
    }
}

static int read_visible_types_pc34_compat(const M11_InventoryState* runtime,
                                          int champion,
                                          int* types)
{
    int i;

    if (!runtime || !types) {
        return 0;
    }
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        M11_Item item;

        if (!m11_inventory_get_item_in_chest_slot(runtime, champion, i,
                                                  &item)) {
            return 0;
        }
        types[i] = item.itemType;
    }
    return 1;
}

static int visible_count_pc34_compat(const int* types)
{
    int count = 0;
    int i;

    if (!types) {
        return 0;
    }
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        if (types[i] != 0) {
            ++count;
        }
    }
    return count;
}

static int contains_any_type_pc34_compat(const int* haystack,
                                         const int* needles)
{
    int i;
    int j;

    if (!haystack || !needles) {
        return 0;
    }
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        if (haystack[i] == 0) {
            continue;
        }
        for (j = 0;
             j < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
             ++j) {
            if (needles[j] != 0 && haystack[i] == needles[j]) {
                return 1;
            }
        }
    }
    return 0;
}

static int array_equal_pc34_compat(const int* a,
                                   const int* b)
{
    int i;

    if (!a || !b) {
        return 0;
    }
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static void fnv_add_u32_pc34_compat(uint64_t* hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        *hash ^= (uint64_t)((value >> (i * 8)) & 0xffu);
        *hash *= UINT64_C(1099511628211);
    }
}

static uint64_t hash_probe_pc34_compat(
    const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat*
        p)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    int i;

    if (!p) {
        return 0;
    }
    fnv_add_u32_pc34_compat(&hash, p->deterministicSeed);
    fnv_add_u32_pc34_compat(&hash, (uint32_t)p->firstClosedCount);
    fnv_add_u32_pc34_compat(&hash, (uint32_t)p->secondClosedCount);
    fnv_add_u32_pc34_compat(&hash, (uint32_t)p->reopenedVisibleCount);
    fnv_add_u32_pc34_compat(&hash, (uint32_t)p->candidateAfterReopen);
    fnv_add_u32_pc34_compat(&hash, (uint32_t)p->panelGraphicAfterReopen);
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->firstBeforeTypes[i]);
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->firstClosedTypes[i]);
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->secondBeforeTypes[i]);
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->secondClosedTypes[i]);
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->reopenedTypes[i]);
        fnv_add_u32_pc34_compat(&hash, (uint32_t)p->reopenedZoneOrdinals[i]);
    }
    return hash;
}

static void record_check_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* p,
    int condition)
{
    if (!p) {
        return;
    }
    ++p->totalAssertions;
    if (condition) {
        ++p->passedAssertions;
    } else {
        ++p->failedAssertions;
    }
}

static void record_assertions_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* p)
{
    int expectedFirst[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int expectedSecond[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int expectedReopened[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int i;

    if (!p) {
        return;
    }
    p->totalAssertions = 0;
    p->passedAssertions = 0;
    p->failedAssertions = 0;
    memset(expectedFirst, 0, sizeof(expectedFirst));
    memset(expectedSecond, 0, sizeof(expectedSecond));
    memset(expectedReopened, 0, sizeof(expectedReopened));

    expectedFirst[0] = p->firstBeforeTypes[0];
    expectedFirst[1] = p->firstBeforeTypes[2];
    expectedFirst[2] = p->firstBeforeTypes[4];
    expectedSecond[0] = p->secondBeforeTypes[0];
    expectedSecond[1] = p->secondBeforeTypes[1];
    expectedSecond[2] = p->secondBeforeTypes[3];
    expectedSecond[3] = p->secondBeforeTypes[5];
    expectedReopened[0] = p->firstClosedTypes[0];
    expectedReopened[1] = p->firstClosedTypes[1];
    expectedReopened[2] = p->firstClosedTypes[2];

    record_check_pc34_compat(p, p->initResult == 1);
    record_check_pc34_compat(p, p->exerciseResult == 1);
    record_check_pc34_compat(p, p->sourceLockedContractOnly == 1);
    record_check_pc34_compat(p, p->deterministicSeed == 0xC537C040u);
    record_check_pc34_compat(p, p->partyChampionCount == 2);
    record_check_pc34_compat(p, p->firstChampionOrdinal == 1);
    record_check_pc34_compat(p, p->reopenChampionOrdinal == 2);
    record_check_pc34_compat(
        p, p->candidateBefore ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL);
    record_check_pc34_compat(
        p, p->candidateAfterClose ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL);
    record_check_pc34_compat(
        p, p->candidateAfterReopen ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL);
    record_check_pc34_compat(p, p->c040LiveBefore == 1);
    record_check_pc34_compat(p, p->c040LiveAfterClose == 1);
    record_check_pc34_compat(p, p->c040LiveAfterReopen == 1);
    record_check_pc34_compat(
        p, p->panelBefore ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE);
    record_check_pc34_compat(
        p, p->panelAfterClose ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE);
    record_check_pc34_compat(
        p, p->panelAfterReopen ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M569_CHEST);
    record_check_pc34_compat(
        p, p->panelGraphicAfterClose ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C040_PANEL_CANDIDATE);
    record_check_pc34_compat(
        p, p->panelGraphicAfterReopen ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C025_PANEL_CHEST);
    record_check_pc34_compat(p, p->c040RedrawOnCloseCount == 1);
    record_check_pc34_compat(p, p->c025RedrawOnReopenCount == 1);
    record_check_pc34_compat(
        p, p->firstOpenThingBeforeClose ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST);
    record_check_pc34_compat(p, p->firstOpenThingAfterClose == 0);
    record_check_pc34_compat(
        p, p->reopenOpenThing ==
               DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST);
    record_check_pc34_compat(p, p->firstClosedCount == 3);
    record_check_pc34_compat(p, p->secondClosedCount == 4);
    record_check_pc34_compat(p, p->reopenedVisibleCount == 3);
    record_check_pc34_compat(p, p->firstCloseClearedG0426 == 1);
    record_check_pc34_compat(p, p->firstCloseCompactedVisibleChain == 1);
    record_check_pc34_compat(p, p->secondCloseCompactedVisibleChain == 1);
    record_check_pc34_compat(p, p->reopenedFirstChestOnDifferentChampion == 1);
    record_check_pc34_compat(p, p->championZeroVisibleCleared == 1);
    record_check_pc34_compat(p, p->noFirstLeakIntoSecondClosedChain == 1);
    record_check_pc34_compat(p, p->noSecondLeakIntoReopenedFirstChain == 1);
    record_check_pc34_compat(p, p->c537ToC544ReboundToReopenedChest == 1);
    record_check_pc34_compat(p, array_equal_pc34_compat(p->firstClosedTypes,
                                                        expectedFirst));
    record_check_pc34_compat(p, array_equal_pc34_compat(p->secondClosedTypes,
                                                        expectedSecond));
    record_check_pc34_compat(p, array_equal_pc34_compat(p->reopenedTypes,
                                                        expectedReopened));
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        record_check_pc34_compat(
            p, p->reopenedZoneOrdinals[i] ==
                   DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST + i);
    }
    for (i = 3;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        record_check_pc34_compat(p, p->firstClosedTypes[i] == 0);
        record_check_pc34_compat(p, p->reopenedTypes[i] == 0);
    }
    for (i = 4;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        record_check_pc34_compat(p, p->secondClosedTypes[i] == 0);
    }
}

const char*
dm1_v1_chest_close_while_candidate_open_reopen_side_effects_source_evidence_pc34_compat(
    void)
{
    return s_source_evidence;
}

const DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsSpecPc34Compat*
dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_pc34_compat(
    void)
{
    return &dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_data_pc34_compat;
}

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_init_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat* state,
    unsigned int deterministicSeed)
{
    uint32_t seed = deterministicSeed;
    int i;

    if (!state) {
        return 0;
    }
    memset(state, 0, sizeof(*state));
    m11_inventory_init(&state->runtime, 2);
    state->deterministicSeed = deterministicSeed;
    state->partyChampionCount = 2;
    state->inventoryChampionOrdinal = 1;
    state->candidateChampionOrdinal =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0299_ORDINAL;
    state->panelContent =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE;
    state->panelGraphic =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C040_PANEL_CANDIDATE;
    state->c040CandidateLive = 1;
    state->firstLinkedCount = 5;
    state->secondLinkedCount = 6;
    for (i = 0; i < state->firstLinkedCount; ++i) {
        state->firstLinked[i] =
            make_seeded_item_pc34_compat(&seed, 0x5100 + (i << 4), 2 + i);
    }
    for (i = 0; i < state->secondLinkedCount; ++i) {
        state->secondLinked[i] =
            make_seeded_item_pc34_compat(&seed, 0x6200 + (i << 4), 7 + i);
    }
    return 1;
}

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_exercise_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat* state,
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* out)
{
    M11_Item firstClosed[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    M11_Item secondClosed[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int secondLinkedTypes[DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT];
    int i;

    if (!state || !out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(firstClosed, 0, sizeof(firstClosed));
    memset(secondClosed, 0, sizeof(secondClosed));
    memset(secondLinkedTypes, 0, sizeof(secondLinkedTypes));
    out->initResult = 1;
    out->sourceLockedContractOnly = 1;
    out->deterministicSeed = state->deterministicSeed;
    out->partyChampionCount = state->partyChampionCount;
    out->firstChampionOrdinal = 1;
    out->reopenChampionOrdinal = 2;
    out->candidateBefore = state->candidateChampionOrdinal;
    out->c040LiveBefore = state->c040CandidateLive;
    out->panelBefore = state->panelContent;

    /* ReDMCSB CHEST.C F0333 lines 30-67 materializes linked chest A into
     * visible C537..C544 while PANEL.C F0344/F0345 keep C040 alive. */
    if (!m11_inventory_open_chest(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST,
            state->firstLinked, state->firstLinkedCount)) {
        record_assertions_pc34_compat(out);
        return 0;
    }
    if (!m11_inventory_set_item_in_chest_slot(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION,
            1, 0, 0, 0, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_set_item_in_chest_slot(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION,
            3, 0, 0, 0, DM1_PC34_ALLOWED_CONTAINER) ||
        !read_visible_types_pc34_compat(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION,
            out->firstBeforeTypes)) {
        record_assertions_pc34_compat(out);
        return 0;
    }
    out->firstOpenThingBeforeClose =
        m11_inventory_get_open_chest_thing(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION);

    /* ReDMCSB CHEST.C F0334 lines 117-132 compacts non-empty C537..C544
     * entries; PANEL.C F0352 redraws the still-live C040 candidate panel. */
    out->firstClosedCount = m11_inventory_close_chest(
        &state->runtime,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION,
        firstClosed,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT);
    if (out->firstClosedCount < 0) {
        record_assertions_pc34_compat(out);
        return 0;
    }
    item_types_pc34_compat(firstClosed, out->firstClosedCount,
                           out->firstClosedTypes);
    out->firstOpenThingAfterClose =
        m11_inventory_get_open_chest_thing(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION);
    out->firstCloseClearedG0426 =
        out->firstOpenThingAfterClose == 0 ? 1 : 0;
    ++state->c040RedrawOnCloseCount;
    state->panelContent =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M568_CANDIDATE;
    state->panelGraphic =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C040_PANEL_CANDIDATE;
    out->candidateAfterClose = state->candidateChampionOrdinal;
    out->c040LiveAfterClose = state->c040CandidateLive;
    out->panelAfterClose = state->panelContent;
    out->panelGraphicAfterClose = state->panelGraphic;
    out->c040RedrawOnCloseCount = state->c040RedrawOnCloseCount;

    /* ReDMCSB CHEST.C F0333 lines 34-67 binds a different G0426/C537..C544
     * chain to champion 2 before the original chest is reopened there. */
    if (!m11_inventory_open_chest(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_SECOND,
            state->secondLinked, state->secondLinkedCount) ||
        !m11_inventory_set_item_in_chest_slot(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
            2, 0, 0, 0, DM1_PC34_ALLOWED_CONTAINER) ||
        !m11_inventory_set_item_in_chest_slot(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
            4, 0, 0, 0, DM1_PC34_ALLOWED_CONTAINER) ||
        !read_visible_types_pc34_compat(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
            out->secondBeforeTypes)) {
        record_assertions_pc34_compat(out);
        return 0;
    }
    item_types_pc34_compat(state->secondLinked, state->secondLinkedCount,
                           secondLinkedTypes);

    /* ReDMCSB CHEST.C F0333 lines 34-39 closes the different open chest
     * through F0334 before reopening the original compacted chain. */
    out->secondClosedCount = m11_inventory_open_chest_replacing_current(
        &state->runtime,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST,
        firstClosed, out->firstClosedCount,
        secondClosed,
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT);
    if (out->secondClosedCount < 0 ||
        !read_visible_types_pc34_compat(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION,
            out->reopenedTypes)) {
        record_assertions_pc34_compat(out);
        return 0;
    }
    item_types_pc34_compat(secondClosed, out->secondClosedCount,
                           out->secondClosedTypes);
    out->reopenOpenThing =
        m11_inventory_get_open_chest_thing(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_REOPEN_CHAMPION);
    out->reopenedVisibleCount = visible_count_pc34_compat(out->reopenedTypes);
    state->inventoryChampionOrdinal = 2;
    state->panelContent = DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_M569_CHEST;
    state->panelGraphic =
        DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C025_PANEL_CHEST;
    ++state->c025RedrawOnReopenCount;
    out->candidateAfterReopen = state->candidateChampionOrdinal;
    out->c040LiveAfterReopen = state->c040CandidateLive;
    out->panelAfterReopen = state->panelContent;
    out->panelGraphicAfterReopen = state->panelGraphic;
    out->c025RedrawOnReopenCount = state->c025RedrawOnReopenCount;
    for (i = 0;
         i < DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_SLOT_COUNT;
         ++i) {
        out->reopenedZoneOrdinals[i] =
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_C537_FIRST + i;
    }
    out->firstCloseCompactedVisibleChain =
        out->firstClosedCount == 3 &&
        out->firstClosedTypes[0] == out->firstBeforeTypes[0] &&
        out->firstClosedTypes[1] == out->firstBeforeTypes[2] &&
        out->firstClosedTypes[2] == out->firstBeforeTypes[4] ? 1 : 0;
    out->secondCloseCompactedVisibleChain =
        out->secondClosedCount == 4 &&
        out->secondClosedTypes[0] == out->secondBeforeTypes[0] &&
        out->secondClosedTypes[1] == out->secondBeforeTypes[1] &&
        out->secondClosedTypes[2] == out->secondBeforeTypes[3] &&
        out->secondClosedTypes[3] == out->secondBeforeTypes[5] ? 1 : 0;
    out->reopenedFirstChestOnDifferentChampion =
        out->reopenOpenThing ==
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_G0426_FIRST &&
        out->reopenedTypes[0] == out->firstClosedTypes[0] &&
        out->reopenedTypes[1] == out->firstClosedTypes[1] &&
        out->reopenedTypes[2] == out->firstClosedTypes[2] ? 1 : 0;
    out->championZeroVisibleCleared =
        m11_inventory_get_open_chest_thing(
            &state->runtime,
            DM1_PC34_CHEST_CLOSE_CANDIDATE_REOPEN_FIRST_CHAMPION) == 0 ? 1 : 0;
    out->noFirstLeakIntoSecondClosedChain =
        !contains_any_type_pc34_compat(out->secondClosedTypes,
                                       out->firstClosedTypes);
    out->noSecondLeakIntoReopenedFirstChain =
        !contains_any_type_pc34_compat(out->reopenedTypes,
                                       secondLinkedTypes);
    out->c537ToC544ReboundToReopenedChest =
        out->reopenedTypes[0] == out->firstClosedTypes[0] &&
        out->reopenedTypes[1] == out->firstClosedTypes[1] &&
        out->reopenedTypes[2] == out->firstClosedTypes[2] &&
        out->reopenedTypes[3] == 0 ? 1 : 0;
    out->exerciseResult = 1;
    out->deterministicHash = hash_probe_pc34_compat(out);
    record_assertions_pc34_compat(out);
    return out->failedAssertions == 0 ? 1 : 0;
}

int dm1_v1_chest_close_while_candidate_open_reopen_side_effects_pc34_compat(
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsProbePc34Compat* out)
{
    DM1_V1_ChestCloseWhileCandidateOpenReopenSideEffectsRuntimePc34Compat state;

    if (!out) {
        return 0;
    }
    if (!dm1_v1_chest_close_while_candidate_open_reopen_side_effects_init_pc34_compat(
            &state,
            dm1_v1_chest_close_while_candidate_open_reopen_side_effects_spec_data_pc34_compat
                .deterministicSeed)) {
        memset(out, 0, sizeof(*out));
        record_assertions_pc34_compat(out);
        return 0;
    }
    return dm1_v1_chest_close_while_candidate_open_reopen_side_effects_exercise_pc34_compat(
        &state, out);
}
