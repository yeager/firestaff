#include <stdio.h>
#include <string.h>

enum {
    DM1_PASS760_NONE_PC34 = -1,
    DM1_PASS760_PARTY_COUNT_PC34 = 4,
    DM1_PASS760_SLOT_COUNT_PC34 = 8,
    DM1_PASS760_PIXEL_COUNT_PC34 = 32,
    DM1_PASS760_C040_GRAPHIC_PC34 = 40,
    DM1_PASS760_M568_PANEL_PC34 = 568,
    DM1_PASS760_REOPEN_SLOT_THING_PC34 = 0x3010,
    DM1_PASS760_C040_PIXEL_TAG_PC34 = 0xC0400000u,
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

void dm1_v1_mirror_candidate_pass760_init_pc34_compat(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state);
int dm1_v1_mirror_candidate_pass760_run_pc34_compat(
    Dm1V1MirrorCandidatePass760StatePc34Compat *state,
    Dm1V1MirrorCandidatePass760ResultPc34Compat *result);
unsigned int dm1_v1_mirror_candidate_pass760_hash_pc34_compat(
    const Dm1V1MirrorCandidatePass760ResultPc34Compat *result);
const char *dm1_v1_mirror_candidate_pass760_scope_pc34_compat(void);

static int g_assertions;
static int g_failures;

static void check_int(const char *label, int actual, int expected)
{
    ++g_assertions;
    if (actual != expected) {
        ++g_failures;
        printf("FAIL %s actual=%d expected=%d\n", label, actual, expected);
    }
}

static void check_true(const char *label, int condition)
{
    ++g_assertions;
    if (!condition) {
        ++g_failures;
        printf("FAIL %s\n", label);
    }
}

static void check_contains(const char *label, const char *text, const char *needle)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        ++g_failures;
        printf("FAIL %s missing '%s'\n", label, needle ? needle : "(null)");
    }
}

static void check_pixel_is_new_candidate(
    const char *label,
    unsigned int pixel,
    int index,
    int generation)
{
    unsigned int owner = (pixel >> 12) & 0xffu;
    unsigned int pixelGeneration = (pixel >> 4) & 0xffu;

    check_true(label, (pixel & 0xFFFF0000u) == DM1_PASS760_C040_PIXEL_TAG_PC34);
    check_int("pixel owner is reopened candidate", (int)owner,
              DM1_PASS760_REOPENED_CANDIDATE_ORDINAL_PC34);
    check_int("pixel generation is fresh", (int)pixelGeneration, generation);
    check_int("pixel low nibble preserves position", (int)(pixel & 0xfu),
              index & 0xf);
}

static void passXXX_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression(void)
{
    Dm1V1MirrorCandidatePass760StatePc34Compat state;
    Dm1V1MirrorCandidatePass760ResultPc34Compat result;
    int accepted;
    int i;

    /*
     * Runtime contract anchors:
     * CHAMDRAW.C F0291/F0296:551-552,1249-1252.
     * CHAMPION.C F0284:93-131.
     * CHAMPION.C F0297:243-268.
     * CHAMPION.C F0298:270-298.
     * CHAMPION.C F0300:511-515.
     * CHAMPION.C F0301:606-614.
     * CHAMPION.C F0302:662-714.
     * COMMAND.C F0359:1985-1990.
     * REVIVE.C F0280:124-132.
     * REVIVE.C F0282:744-806.
     * CHEST.C F0333:30-67.
     * CHEST.C F0334:113-132.
     * PANEL.C F0344:1895-1944.
     * PANEL.C F0345:1946-1999.
     * OBJECT.C F0033:147-212.
     * BLITMASK.C F0133:30-33.
     * DEFS.H:2088 C30/G0425/G0426/G0423/G0305/M070/M516/C040.
     */
    dm1_v1_mirror_candidate_pass760_init_pc34_compat(&state);

    check_int("initial pass number", state.passNumber, 760);
    check_int("initial party count", state.partyChampionCount, 4);
    check_int("initial G0305 party count", state.g0305PartyChampionCount, 4);
    check_int("initial candidate ordinal", state.candidateChampionOrdinal, 1);
    check_int("initial inventory ordinal", state.g0423InventoryChampionOrdinal, 1);
    check_int("initial panel open", state.c040PanelOpen, 1);
    check_int("initial panel content", state.panelContent, DM1_PASS760_M568_PANEL_PC34);
    check_int("initial C040 graphic", state.c040Graphic, DM1_PASS760_C040_GRAPHIC_PC34);
    check_int("initial chrome owner", state.chromeOwnerOrdinal, 1);
    check_int("initial chrome generation", state.chromeGeneration, 1);
    check_int("initial draw count", state.c040DrawCount, 1);
    check_int("initial partial-mask count", state.f0133PartialMaskDispatchCount, 1);

    accepted =
        dm1_v1_mirror_candidate_pass760_run_pc34_compat(&state, &result);
    check_int("run accepted", accepted, 1);
    check_int("result accepted", result.accepted, 1);
    check_int("assertion floor advertised", result.assertionsExpectedAtLeast, 80);
    check_int("result pass number", result.passNumber, 760);
    check_int("initial ordinal captured", result.initialCandidateOrdinal, 1);
    check_int("candidate close cleared ordinal", result.closedCandidateOrdinal, 0);
    check_int("transition selected inventory champion", result.transitionInventoryOrdinal, 2);
    check_int("reopened candidate ordinal", result.reopenedCandidateOrdinal, 2);
    check_int("final panel open", result.finalPanelOpen, 1);
    check_int("final panel content", result.finalPanelContent, DM1_PASS760_M568_PANEL_PC34);
    check_int("final C040 graphic", result.finalC040Graphic, DM1_PASS760_C040_GRAPHIC_PC34);
    check_int("final chrome owner", result.finalChromeOwnerOrdinal, 2);
    check_int("final chrome generation", result.finalChromeGeneration, 2);
    check_int("final chrome palette", result.finalChromePalette, 12);
    check_int("stale C040 pixels", result.staleC040PixelCount, 0);
    check_int("stale owner pixels", result.staleOwnerPixelCount, 0);
    check_int("stale generation pixels", result.staleGenerationPixelCount, 0);
    check_int("candidate pixel count", result.candidatePixelCount, DM1_PASS760_PIXEL_COUNT_PC34);
    check_int("non-candidate pixel count", result.nonCandidatePixelCount, 0);
    check_int("old pixels cleared before reopen", result.oldPixelsClearedBeforeReopen, 1);
    check_int("new chrome published after reopen", result.newChromePublishedAfterReopen, 1);
    check_int("non-candidate transition count", result.nonCandidateTransitionCount, 1);
    check_int("G0426 closed after transition", result.g0426OpenChestThing, DM1_PASS760_NONE_PC34);
    check_int("leader hand empty after reopen", result.leaderEmptyHanded, 1);
    check_int("leader hand thing cleared", result.leaderHandThing, DM1_PASS760_NONE_PC34);
    check_int("party direction rotated", result.partyDirection, 1);
    check_int("F0280 candidate state count", result.f0280CandidateStateCount, 2);
    check_int("F0282 candidate clear count", result.f0282CandidateClearCount, 1);
    check_int("F0284 party rotate count", result.f0284PartyRotateCount, 1);
    check_int("F0291 slot read count", result.f0291SlotReadCount, DM1_PASS760_SLOT_COUNT_PC34);
    check_int("F0296 changed icon redraw count", result.f0296ChangedIconRedrawCount, 1);
    check_int("F0297 leader hand put count", result.f0297LeaderHandPutCount, 1);
    check_int("F0298 leader hand remove count", result.f0298LeaderHandRemoveCount, 1);
    check_int("F0300 slot clear count", result.f0300SlotClearCount, DM1_PASS760_SLOT_COUNT_PC34);
    check_int("F0301 slot write count", result.f0301SlotWriteCount, DM1_PASS760_SLOT_COUNT_PC34);
    check_int("F0302 occupied slot dispatch count", result.f0302OccupiedSlotDispatchCount, 1);
    check_int("F0333 chest open/close count", result.f0333ChestOpenCloseCount, 1);
    check_int("F0334 close rewrite count", result.f0334CloseRewriteCount, 1);
    check_int("F0344 panel click count", result.f0344PanelClickCount, 1);
    check_int("F0345 highlight rotation count", result.f0345PerCellHighlightRotationCount, 1);
    check_int("F0359 dispatch count", result.f0359M568C040DispatchCount, 2);
    check_int("F0033 icon identity count", result.f0033IconIdentityCount, 1);
    check_int("F0133 partial-mask count", result.f0133PartialMaskDispatchCount, 2);
    check_int("C040 draw count", result.c040DrawCount, 2);
    check_int("C040 clear count", result.c040ClearCount, 1);
    check_int("state distinct from pass674", state.distinctFromPass674, 1);
    check_int("state distinct from pass686", state.distinctFromPass686, 1);
    check_int("state distinct from pass710/pass711", state.distinctFromPass710Pass711, 1);
    check_int("state distinct from pass736", state.distinctFromPass736, 1);
    check_contains("scope mentions pass674", result.scope, "pass674");
    check_contains("scope mentions pass686", result.scope, "pass686");
    check_contains("scope mentions pass710/pass711", result.scope, "pass710/pass711");
    check_contains("scope mentions pass736", result.scope, "pass736");
    check_contains("scope states non-candidate transition", result.scope, "non-candidate");
    check_true("deterministic hash nonzero", result.deterministicHash != 0u);
    check_int("hash helper matches result",
              (int)dm1_v1_mirror_candidate_pass760_hash_pc34_compat(&result),
              (int)result.deterministicHash);
    check_contains("scope helper matches result",
                   dm1_v1_mirror_candidate_pass760_scope_pc34_compat(),
                   "reopened C040 chrome");

    for (i = 0; i < DM1_PASS760_SLOT_COUNT_PC34; ++i) {
        check_int("G0425 slot rewritten after non-candidate transition",
                  result.g0425ChestSlots[i],
                  DM1_PASS760_REOPEN_SLOT_THING_PC34 + i);
    }

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        check_pixel_is_new_candidate("final framebuffer is C040",
                                     result.finalFramebuffer[i], i,
                                     result.finalChromeGeneration);
    }

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        check_true("after-close framebuffer has no C040 tag",
                   (state.afterCloseFramebuffer[i] & 0xFFFF0000u) !=
                       DM1_PASS760_C040_PIXEL_TAG_PC34);
    }

    for (i = 0; i < DM1_PASS760_PIXEL_COUNT_PC34; ++i) {
        check_true("after-transition framebuffer has no C040 tag",
                   (state.afterTransitionFramebuffer[i] & 0xFFFF0000u) !=
                       DM1_PASS760_C040_PIXEL_TAG_PC34);
    }
}

int main(void)
{
    passXXX_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression();
    if (g_failures) {
        printf("FAIL pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression assertions=%d failures=%d\n",
               g_assertions, g_failures);
        return 1;
    }
    printf("PASS pass760_dm1_v1_mirror_candidate_chrome_after_non_candidate_transition_runtime_regression assertions=%d failures=0\n",
           g_assertions);
    return 0;
}
