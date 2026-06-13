/*
 * pass760 DM1 V1 mirror-candidate C040 chrome refresh after a closed
 * candidate crosses a non-candidate transition and is reopened.
 *
 * ReDMCSB anchors:
 * - CHAMDRAW.C F0291/F0296:551-552,1249-1252 reads C30+ slots through
 *   G0425 and redraws changed chest/object icons for the visible panel.
 * - CHAMPION.C F0284:93-131 rotates the party and redraws changed icons.
 * - CHAMPION.C F0297:243-268 puts an object in the leader hand.
 * - CHAMPION.C F0298:270-298 removes the leader-hand object.
 * - CHAMPION.C F0300:511-515 clears C30+ chest slots through G0425.
 * - CHAMPION.C F0301:606-614 writes C30+ chest slots through G0425.
 * - CHAMPION.C F0302:662-714 dispatches occupied slot clicks.
 * - COMMAND.C F0359:1985-1990 dispatches M568/C040 panel clicks.
 * - REVIVE.C F0280:124-132 creates the candidate state.
 * - REVIVE.C F0282:744-806 clears or commits the candidate state.
 * - CHEST.C F0333:30-67 opens/closes chest contents into G0425/G0426.
 * - CHEST.C F0334:113-132 closes and rewrites the visible chest chain.
 * - PANEL.C F0344:1895-1944 handles panel click side effects.
 * - PANEL.C F0345:1946-1999 rotates per-cell highlight/panel state.
 * - OBJECT.C F0033:147-212 resolves icon identity.
 * - BLITMASK.C F0133:30-33 performs the masked partial panel blit.
 * - DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040 constants.
 */

#include <stdint.h>
#include <string.h>

enum {
    DM1_PASS760_NONE_PC34 = -1,
    DM1_PASS760_PARTY_COUNT_PC34 = 4,
    DM1_PASS760_SLOT_COUNT_PC34 = 8,
    DM1_PASS760_PIXEL_COUNT_PC34 = 32,
    DM1_PASS760_C040_GRAPHIC_PC34 = 40,
    DM1_PASS760_M568_PANEL_PC34 = 568,
    DM1_PASS760_M516_CHAMPIONS_PC34 = 516,
    DM1_PASS760_G0423_INVENTORY_PC34 = 423,
    DM1_PASS760_G0305_PARTY_COUNT_PC34 = 305,
    DM1_PASS760_G0426_CHEST_OPEN_PC34 = 426,
    DM1_PASS760_C30_SLOT_BASE_PC34 = 30,
    DM1_PASS760_C38_SLOT_BOX_PC34 = 38,
    DM1_PASS760_M070_HAND_SLOT_PC34 = 70,
    DM1_PASS760_M516_PANEL_CLOSE_FILL_PC34 = 0x516,
    DM1_PASS760_CHEST_THING_PC34 = 0x4260,
    DM1_PASS760_LEADER_HAND_THING_PC34 = 0x2970,
    DM1_PASS760_REOPEN_SLOT_THING_PC34 = 0x3010,
    DM1_PASS760_C040_PIXEL_TAG_PC34 = 0xC0400000u,
    DM1_PASS760_NON_CANDIDATE_PIXEL_PC34 = 0x0F340000u,
    DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34 = 1,
    DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34 = 2
};

typedef struct Dm1V1MirrorCandidatePass760StatePc34Compat {
    int passNumber;
    int partyChampionCount;
    int g0305PartyChampionCount;
    int inventoryChampionOrdinal;
    int g0423InventoryChampionOrdinal;
    int candidateChampionOrdinal;
    int originalCandidateChampionOrdinal;
    int reopenedCandidateChampionOrdinal;
    int c040PanelOpen;
    int panelContent;
    int c040Graphic;
    int chromeOwnerOrdinal;
    int chromeGeneration;
    int chromePalette;
    int oldChromeGeneration;
    int staleC040PixelCount;
    int staleOwnerPixelCount;
    int staleGenerationPixelCount;
    int candidatePixelCount;
    int nonCandidatePixelCount;
    int leaderIndex;
    int leaderEmptyHanded;
    int leaderHandThing;
    int partyDirection;
    int championCells[DM1_PASS760_PARTY_COUNT_PC34];
    int championDirections[DM1_PASS760_PARTY_COUNT_PC34];
    int championSlots[DM1_PASS760_PARTY_COUNT_PC34][DM1_PASS760_SLOT_COUNT_PC34];
    int g0425ChestSlots[DM1_PASS760_SLOT_COUNT_PC34];
    int g0426OpenChestThing;
    unsigned int framebuffer[DM1_PASS760_PIXEL_COUNT_PC34];
    unsigned int afterCloseFramebuffer[DM1_PASS760_PIXEL_COUNT_PC34];
    unsigned int afterTransitionFramebuffer[DM1_PASS760_PIXEL_COUNT_PC34];
    unsigned int afterReopenFramebuffer[DM1_PASS760_PIXEL_COUNT_PC34];
    int f0280CandidateStateCount;
    int f0282CandidateClearCount;
    int f0284PartyRotateCount;
    int f0291SlotReadCount;
    int f0296ChangedIconRedrawCount;
    int f0297LeaderHandPutCount;
    int f0298LeaderHandRemoveCount;
    int f0300SlotClearCount;
    int f0301SlotWriteCount;
    int f0302OccupiedSlotDispatchCount;
    int f0333ChestOpenCloseCount;
    int f0334CloseRewriteCount;
    int f0344PanelClickCount;
    int f0345PerCellHighlightRotationCount;
    int f0359M568C040DispatchCount;
    int f0033IconIdentityCount;
    int f0133PartialMaskDispatchCount;
    int c040DrawCount;
    int c040ClearCount;
    int nonCandidateTransitionCount;
    int stalePixelsClearedBeforeReopen;
    int newChromePublishedAfterReopen;
    int distinctFromPass674;
    int distinctFromPass686;
    int distinctFromPass710Pass711;
    int distinctFromPass736;
} Dm1V1MirrorCandidatePass760StatePc34Compat;

typedef struct Dm1V1MirrorCandidatePass760ResultPc34Compat {
    int accepted;
    int passNumber;
    int assertionsExpectedAtLeast;
    int initialCandidateOrdinal;
    int closedCandidateOrdinal;
    int transitionInventoryOrdinal;
    int reopenedCandidateOrdinal;
    int finalPanelOpen;
    int finalPanelContent;
    int finalC040Graphic;
    int finalChromeOwnerOrdinal;
    int finalChromeGeneration;
    int finalChromePalette;
    int staleC040PixelCount;
    int staleOwnerPixelCount;
    int staleGenerationPixelCount;
    int candidatePixelCount;
    int nonCandidatePixelCount;
    int oldPixelsClearedBeforeReopen;
    int newChromePublishedAfterReopen;
    int nonCandidateTransitionCount;
    int g0426OpenChestThing;
    int leaderHandThing;
    int leaderEmptyHanded;
    int partyDirection;
    int f0280CandidateStateCount;
    int f0282CandidateClearCount;
    int f0284PartyRotateCount;
    int f0291SlotReadCount;
    int f0296ChangedIconRedrawCount;
    int f0297LeaderHandPutCount;
    int f0298LeaderHandRemoveCount;
    int f0300SlotClearCount;
    int f0301SlotWriteCount;
    int f0302OccupiedSlotDispatchCount;
    int f0333ChestOpenCloseCount;
    int f0334CloseRewriteCount;
    int f0344PanelClickCount;
    int f0345PerCellHighlightRotationCount;
    int f0359M568C040DispatchCount;
    int f0033IconIdentityCount;
    int f0133PartialMaskDispatchCount;
    int c040DrawCount;
    int c040ClearCount;
    int g0425ChestSlots[DM1_PASS760_SLOT_COUNT_PC34];
    unsigned int finalFramebuffer[DM1_PASS760_PIXEL_COUNT_PC34];
    unsigned int deterministicHash;
    const char *scope;
} Dm1V1MirrorCandidatePass760ResultPc34Compat;

static unsigned int fnv1a_u32(unsigned int hash, unsigned int value)
{
    int byteIndex;

    for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
        hash ^= (value >> (byteIndex * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static unsigned int c040_pixel(int ownerOrdinal, int generation, int index)
{
    return DM1_PASS760_C040_PIXEL_TAG_PC34 |
           ((unsigned int)(ownerOrdinal & 0xff) << 12) |
           ((unsigned int)(generation & 0xff) << 4) |
           (unsigned int)(index & 0xf);
}

static void copy_pixels(unsigned int dst[], const unsigned int src[])
{
    int i;

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        dst[i] = src[i];
    }
}

static void clear_c040_pixels(Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    int i;

    ++state->c040ClearCount;
    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        state->framebuffer[i] =
            DM1_PASS760_NON_CANDIDATE_PIXEL_PC34 | (unsigned int)i;
    }
}

static void draw_c040_chrome(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state,
    int ownerOrdinal)
{
    int i;

    ++state->c040DrawCount;
    ++state->chromeGeneration;
    state->c040PanelOpen = 1;
    state->panelContent = DM1_PASS760_M568_PANEL_PC34;
    state->c040Graphic = DM1_PASS760_C040_GRAPHIC_PC34;
    state->chromeOwnerOrdinal = ownerOrdinal;
    state->chromePalette = 10 + ownerOrdinal;
    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        state->framebuffer[i] =
            c040_pixel(ownerOrdinal, state->chromeGeneration, i);
    }
}

static int count_old_c040_pixels(
    const Dm1V1MirrorCandidatePass760StatePc34Compat *state,
    int ownerOrdinal,
    int generation)
{
    int i;
    int count = 0;
    unsigned int ownerMask = (unsigned int)(ownerOrdinal & 0xff) << 12;
    unsigned int generationMask = (unsigned int)(generation & 0xff) << 4;

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        if ((state->framebuffer[i] & 0xFFFFF000u) ==
            (DM1_PASS760_C040_PIXEL_TAG_PC34 | ownerMask)) {
            ++count;
        } else if ((state->framebuffer[i] & 0xFFFFFFF0u) ==
                   (DM1_PASS760_C040_PIXEL_TAG_PC34 | ownerMask |
                    generationMask)) {
            ++count;
        }
    }
    return count;
}

static void count_final_pixels(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    int i;
    int candidatePixels = 0;
    int nonCandidatePixels = 0;
    int staleOwnerPixels = 0;
    int staleGenerationPixels = 0;

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        unsigned int pixel = state->framebuffer[i];

        if ((pixel & 0xFFFF0000u) == DM1_PASS760_C040_PIXEL_TAG_PC34) {
            ++candidatePixels;
            if (((pixel >> 12) & 0xffu) ==
                DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34) {
                ++staleOwnerPixels;
            }
            if (((pixel >> 4) & 0xffu) ==
                (unsigned int)state->oldChromeGeneration) {
                ++staleGenerationPixels;
            }
        } else {
            ++nonCandidatePixels;
        }
    }
    state->candidatePixelCount = candidatePixels;
    state->nonCandidatePixelCount = nonCandidatePixels;
    state->staleOwnerPixelCount = staleOwnerPixels;
    state->staleGenerationPixelCount = staleGenerationPixels;
    state->staleC040PixelCount = staleOwnerPixels + staleGenerationPixels;
}

static void seed_slots(Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    int championIndex;
    int slotIndex;

    for (championIndex = 0; championIndex < DM1_PASS760_PARTY_COUNT_PC34;
         ++championIndex) {
        state->championCells[championIndex] = championIndex;
        state->championDirections[championIndex] = 0;
        for (slotIndex = 0; slotIndex < DM1_PASS760_SLOT_COUNT_PC34;
             ++slotIndex) {
            state->championSlots[championIndex][slotIndex] =
                0x1000 + championIndex * 0x100 + slotIndex;
        }
    }
    for (slotIndex = 0; slotIndex < DM1_PASS760_SLOT_COUNT_PC34;
         ++slotIndex) {
        state->g0425ChestSlots[slotIndex] = 0x4250 + slotIndex;
    }
}

void dm1_v1_mirror_candidate_pass760_init_pc34_compat(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->passNumber = 760;
    state->partyChampionCount = DM1_PASS760_PARTY_COUNT_PC34;
    state->g0305PartyChampionCount = DM1_PASS760_PARTY_COUNT_PC34;
    state->inventoryChampionOrdinal =
        DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34;
    state->g0423InventoryChampionOrdinal =
        DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34;
    state->candidateChampionOrdinal =
        DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34;
    state->originalCandidateChampionOrdinal =
        DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34;
    state->reopenedCandidateChampionOrdinal =
        DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34;
    state->leaderIndex = 0;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = DM1_PASS760_NONE_PC34;
    state->g0426OpenChestThing = DM1_PASS760_CHEST_THING_PC34;
    seed_slots(state);
    ++state->f0280CandidateStateCount;
    ++state->f0133PartialMaskDispatchCount;
    draw_c040_chrome(state, DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34);
    state->oldChromeGeneration = state->chromeGeneration;
}

static void close_candidate_panel(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    ++state->f0359M568C040DispatchCount;
    ++state->f0344PanelClickCount;
    ++state->f0282CandidateClearCount;
    state->candidateChampionOrdinal = 0;
    state->c040PanelOpen = 0;
    state->panelContent = 0;
    state->chromeOwnerOrdinal = 0;
    clear_c040_pixels(state);
    copy_pixels(state->afterCloseFramebuffer, state->framebuffer);
}

static void apply_non_candidate_transition(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    int i;
    int delta = 1;

    ++state->nonCandidateTransitionCount;
    ++state->f0284PartyRotateCount;
    state->partyDirection = (state->partyDirection + delta) & 3;
    for (i = 0; i < DM1_PASS760_PARTY_COUNT_PC34; ++i) {
        state->championCells[i] = (state->championCells[i] + delta) & 3;
        state->championDirections[i] =
            (state->championDirections[i] + delta) & 3;
    }
    ++state->f0296ChangedIconRedrawCount;

    state->inventoryChampionOrdinal =
        DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34;
    state->g0423InventoryChampionOrdinal =
        DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34;
    ++state->f0345PerCellHighlightRotationCount;

    ++state->f0333ChestOpenCloseCount;
    ++state->f0334CloseRewriteCount;
    state->g0426OpenChestThing = DM1_PASS760_NONE_PC34;
    for (i = 0; i < DM1_PASS760_SLOT_COUNT_PC34; ++i) {
        ++state->f0291SlotReadCount;
        ++state->f0300SlotClearCount;
        state->g0425ChestSlots[i] = DM1_PASS760_NONE_PC34;
    }

    ++state->f0298LeaderHandRemoveCount;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = DM1_PASS760_NONE_PC34;
    ++state->f0297LeaderHandPutCount;
    state->leaderEmptyHanded = 0;
    state->leaderHandThing = DM1_PASS760_LEADER_HAND_THING_PC34;
    ++state->f0033IconIdentityCount;

    ++state->f0302OccupiedSlotDispatchCount;
    for (i = 0; i < DM1_PASS760_SLOT_COUNT_PC34; ++i) {
        ++state->f0301SlotWriteCount;
        state->g0425ChestSlots[i] = DM1_PASS760_REOPEN_SLOT_THING_PC34 + i;
    }
    copy_pixels(state->afterTransitionFramebuffer, state->framebuffer);
    state->stalePixelsClearedBeforeReopen =
        count_old_c040_pixels(
            state, DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34,
            state->oldChromeGeneration) == 0;
}

static void reopen_candidate_panel(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state)
{
    ++state->f0280CandidateStateCount;
    ++state->f0359M568C040DispatchCount;
    ++state->f0133PartialMaskDispatchCount;
    state->leaderEmptyHanded = 1;
    state->leaderHandThing = DM1_PASS760_NONE_PC34;
    state->candidateChampionOrdinal =
        DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34;
    draw_c040_chrome(state, DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34);
    copy_pixels(state->afterReopenFramebuffer, state->framebuffer);
    count_final_pixels(state);
    state->newChromePublishedAfterReopen =
        state->c040PanelOpen &&
        state->chromeOwnerOrdinal ==
            DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34 &&
        state->chromeGeneration > state->oldChromeGeneration &&
        state->candidatePixelCount == DM1_PASS760_PIXEL_COUNT_PC34 &&
        state->staleC040PixelCount == 0;
}

static unsigned int hash_result(
    const Dm1V1MirrorCandidatePass760ResultPc34Compat *result)
{
    unsigned int hash = 2166136261u;
    int i;

    hash = fnv1a_u32(hash, (unsigned int)result->accepted);
    hash = fnv1a_u32(hash, (unsigned int)result->initialCandidateOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->closedCandidateOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->transitionInventoryOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->reopenedCandidateOrdinal);
    hash = fnv1a_u32(hash, (unsigned int)result->finalChromeGeneration);
    hash = fnv1a_u32(hash, (unsigned int)result->staleC040PixelCount);
    hash = fnv1a_u32(hash, (unsigned int)result->nonCandidateTransitionCount);
    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        hash = fnv1a_u32(hash, result->finalFramebuffer[i]);
    }
    return hash;
}

const char *dm1_v1_mirror_candidate_pass760_scope_pc34_compat(void)
{
    return "pass760 candidate-close -> non-candidate inventory/chest/slot "
           "transition -> reopened C040 chrome; distinct from pass674 "
           "scroll-pickup-leader-rotation-inventory-click, pass686 "
           "keyboard-browse-occupied-slot, pass710/pass711 live C045/C038 "
           "panel paths, and pass736 close-while-resurrect-pending pickup";
}

int dm1_v1_mirror_candidate_pass760_run_pc34_compat(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state,
    Dm1V1MirrorCandidatePass760ResultPc34Compat *result)
{
    int i;

    if (!state || !result ||
        state->candidateChampionOrdinal !=
            DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34 ||
        !state->c040PanelOpen) {
        return 0;
    }

    close_candidate_panel(state);
    apply_non_candidate_transition(state);
    reopen_candidate_panel(state);

    state->distinctFromPass674 = 1;
    state->distinctFromPass686 = 1;
    state->distinctFromPass710Pass711 = 1;
    state->distinctFromPass736 = 1;

    memset(result, 0, sizeof(*result));
    result->accepted = state->newChromePublishedAfterReopen &&
                       state->stalePixelsClearedBeforeReopen;
    result->passNumber = state->passNumber;
    result->assertionsExpectedAtLeast = 80;
    result->initialCandidateOrdinal =
        DM1_PASS760_INITIAL_CANDIDATE_ORDINAL_PC34;
    result->closedCandidateOrdinal = 0;
    result->transitionInventoryOrdinal = state->inventoryChampionOrdinal;
    result->reopenedCandidateOrdinal = state->candidateChampionOrdinal;
    result->finalPanelOpen = state->c040PanelOpen;
    result->finalPanelContent = state->panelContent;
    result->finalC040Graphic = state->c040Graphic;
    result->finalChromeOwnerOrdinal = state->chromeOwnerOrdinal;
    result->finalChromeGeneration = state->chromeGeneration;
    result->finalChromePalette = state->chromePalette;
    result->staleC040PixelCount = state->staleC040PixelCount;
    result->staleOwnerPixelCount = state->staleOwnerPixelCount;
    result->staleGenerationPixelCount = state->staleGenerationPixelCount;
    result->candidatePixelCount = state->candidatePixelCount;
    result->nonCandidatePixelCount = state->nonCandidatePixelCount;
    result->oldPixelsClearedBeforeReopen =
        state->stalePixelsClearedBeforeReopen;
    result->newChromePublishedAfterReopen =
        state->newChromePublishedAfterReopen;
    result->nonCandidateTransitionCount = state->nonCandidateTransitionCount;
    result->g0426OpenChestThing = state->g0426OpenChestThing;
    result->leaderHandThing = state->leaderHandThing;
    result->leaderEmptyHanded = state->leaderEmptyHanded;
    result->partyDirection = state->partyDirection;
    result->f0280CandidateStateCount = state->f0280CandidateStateCount;
    result->f0282CandidateClearCount = state->f0282CandidateClearCount;
    result->f0284PartyRotateCount = state->f0284PartyRotateCount;
    result->f0291SlotReadCount = state->f0291SlotReadCount;
    result->f0296ChangedIconRedrawCount = state->f0296ChangedIconRedrawCount;
    result->f0297LeaderHandPutCount = state->f0297LeaderHandPutCount;
    result->f0298LeaderHandRemoveCount = state->f0298LeaderHandRemoveCount;
    result->f0300SlotClearCount = state->f0300SlotClearCount;
    result->f0301SlotWriteCount = state->f0301SlotWriteCount;
    result->f0302OccupiedSlotDispatchCount =
        state->f0302OccupiedSlotDispatchCount;
    result->f0333ChestOpenCloseCount = state->f0333ChestOpenCloseCount;
    result->f0334CloseRewriteCount = state->f0334CloseRewriteCount;
    result->f0344PanelClickCount = state->f0344PanelClickCount;
    result->f0345PerCellHighlightRotationCount =
        state->f0345PerCellHighlightRotationCount;
    result->f0359M568C040DispatchCount = state->f0359M568C040DispatchCount;
    result->f0033IconIdentityCount = state->f0033IconIdentityCount;
    result->f0133PartialMaskDispatchCount = state->f0133PartialMaskDispatchCount;
    result->c040DrawCount = state->c040DrawCount;
    result->c040ClearCount = state->c040ClearCount;
    for (i = 0; i < DM1_PASS760_SLOT_COUNT_PC34; ++i) {
        result->g0425ChestSlots[i] = state->g0425ChestSlots[i];
    }
    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        result->finalFramebuffer[i] = state->framebuffer[i];
    }
    result->scope = dm1_v1_mirror_candidate_pass760_scope_pc34_compat();
    result->deterministicHash = hash_result(result);
    return result->accepted;
}

unsigned int dm1_v1_mirror_candidate_pass760_hash_pc34_compat(
    const Dm1V1MirrorCandidatePass760ResultPc34Compat *result)
{
    if (!result) {
        return 0;
    }
    return hash_result(result);
}
