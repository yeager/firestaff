#include "dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label,
                       int got,
                       int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n", label, got, want, anchor);
    }
}

static void expect_nonzero(const char *label, int got, const char *anchor)
{
    ++g_assertions;
    if (!got) {
        ++g_failures;
        printf("FAIL %s got=0 anchor=%s\n", label, anchor);
    }
}

static void expect_contains(const char *label,
                            const char *text,
                            const char *needle,
                            const char *anchor)
{
    ++g_assertions;
    if (!text || !needle || !strstr(text, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    }
}

int main(void)
{
    Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34 probe;
    const Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34 *e =
        dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_evidence_pc34();
    int ok =
        dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_run_pc34(
            &probe);

    expect_nonzero("run result", ok, "COMMAND.C F0380:2045-2159");
    expect_nonzero("evidence available", e != 0, "CHEST.C F0333:30-67");
    expect_contains("contract-only evidence",
                    e ? e->contract : 0,
                    "source_locked_contract_only=1",
                    "CHEST.C F0333:30-67");
    expect_contains("no bitmap parity evidence",
                    e ? e->contract : 0,
                    "no_real_asset_bitmap_parity=1",
                    "OBJECT.C F0033:147-212");
    expect_contains("no data load evidence",
                    e ? e->contract : 0,
                    "no_game_data_load=1",
                    "UTAMSCR.C F0077/F0078:141-150");
    expect_contains("chest pickup anchor",
                    e ? e->chestPickup : 0,
                    "F0334:117-132",
                    "CHEST.C F0334:117-132");
    expect_contains("champion hand anchor",
                    e ? e->championHands : 0,
                    "F0302:662-713",
                    "CHAMPION.C F0302:662-713");
    expect_contains("command queue anchor",
                    e ? e->commandQueue : 0,
                    "F0380:2045-2159",
                    "COMMAND.C F0380:2045-2159");
    expect_contains("resurrect negative anchor",
                    e ? e->resurrectNotReached : 0,
                    "F0282:744-806",
                    "REVIVE.C F0282:744-806");
    expect_contains("panel c040 anchor",
                    e ? e->panelOnlyC040 : 0,
                    "F0346/F0347:1619-1657",
                    "PANEL.C F0346/F0347:1619-1657");
    expect_contains("defs anchor",
                    e ? e->utilityObjectDefs : 0,
                    "DEFS.H:338-340",
                    "DEFS.H:338-340 C162");

    expect_int("source_locked_contract_only",
               probe.source_locked_contract_only, 1, "CHEST.C F0333:30-67");
    expect_int("no_real_asset_bitmap_parity",
               probe.no_real_asset_bitmap_parity, 1,
               "OBJECT.C F0033:147-212");
    expect_int("no_game_data_load",
               probe.no_game_data_load, 1,
               "CHEST.C F0333:30-67");
    expect_int("same champion covered",
               probe.sameChampionCovered, 1,
               "CHAMPION.C F0302:662-713");
    expect_int("different champion covered",
               probe.differentChampionCovered, 1,
               "CHAMPION.C F0302:662-713");
    expect_int("reverse order covered",
               probe.reverseOrderCovered, 1,
               "COMMAND.C F0380:2045-2159");
    expect_int("failed pickup covered",
               probe.failedPickupCovered, 1,
               "CHAMPION.C F0302:694-698");
    expect_int("hand queue consumes chest pickup hand",
               probe.hand_queue_consumes, 0,
               "CHAMPION.C F0297:243-268");
    expect_int("chest pickup corrupts mirror",
               probe.chest_pickup_corrupts_mirror, 0,
               "PANEL.C F0346/F0347:1619-1657");
    expect_int("mirror slot intact cases",
               probe.mirror_slot_intact, 4,
               "REVIVE.C F0280:124-132");
    expect_int("panel redraw count",
               probe.panel_redraws, 6,
               "PANEL.C F0346/F0347:1619-1657");
    expect_int("resurrect triggers",
               probe.resurrectTriggers, 0,
               "REVIVE.C F0282:744-806");
    expect_int("queue consumed total",
               probe.queueConsumedTotal, 3,
               "COMMAND.C F0380:2045-2159");
    expect_int("chest pickup successes",
               probe.chestPickupSuccesses, 3,
               "CHEST.C F0333:30-67");
    expect_int("chest pickup failures",
               probe.chestPickupFailures, 1,
               "CHAMPION.C F0302:694-698");
    expect_int("only c040 panel live",
               probe.c040OnlyPanelLive, 0,
               "PANEL.C F0346/F0347:1619-1657");
    expect_int("leader hand item survived",
               probe.leaderHandItemSurvived, 3,
               "CHAMPION.C F0297:243-268");
    expect_int("pending swap item survived",
               probe.pendingSwapItemSurvived, 3,
               "CHAMPION.C F0300:511-584");
    expect_int("c30 chest slot first",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C30_SLOT_PC34,
               30,
               "DEFS.H:810-817 C30..C37");
    expect_int("c37 chest slot last",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C37_SLOT_PC34,
               37,
               "DEFS.H:810-817 C30..C37");
    expect_int("c38 slot box first",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C38_SLOT_BOX_PC34,
               38,
               "DEFS.H:1874-1878 C38");
    expect_int("c537 zone first",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C537_ZONE_PC34,
               537,
               "DEFS.H:3906-3913 C537..C544");
    expect_int("c544 zone last",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C544_ZONE_PC34,
               544,
               "DEFS.H:3906-3913 C537..C544");
    expect_int("c162 cancel constant",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C162_CANCEL_PC34,
               162,
               "DEFS.H:338-340 C162");
    expect_int("c040 graphic constant",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C040_GRAPHIC_PC34,
               40,
               "DEFS.H:2200 C040");
    expect_int("m568 panel constant",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M568_PANEL_PC34,
               568,
               "DEFS.H:3001-3008 M568/M569");
    expect_int("m569 panel constant",
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M569_PANEL_PC34,
               569,
               "DEFS.H:3001-3008 M568/M569");
    expect_nonzero("deterministic hash nonzero",
                   (int)(probe.hash != 0u),
                   "contract deterministic fnv1a");

    if (g_failures != 0) {
        printf("FAIL test_dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_pc34_compat "
               "assertions=%d failures=%d hand_queue_consumes=%d "
               "chest_pickup_corrupts_mirror=%d mirror_slot_intact=%d "
               "panel_redraws=%d hash=0x%08x\n",
               g_assertions,
               g_failures,
               probe.hand_queue_consumes,
               probe.chest_pickup_corrupts_mirror,
               probe.mirror_slot_intact,
               probe.panel_redraws,
               probe.hash);
        return 1;
    }

    printf("PASS test_dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_pc34_compat "
           "assertions=%d failures=0 hand_queue_consumes=%d "
           "chest_pickup_corrupts_mirror=%d mirror_slot_intact=%d "
           "panel_redraws=%d hash=0x%08x\n",
           g_assertions,
           probe.hand_queue_consumes,
           probe.chest_pickup_corrupts_mirror,
           probe.mirror_slot_intact,
           probe.panel_redraws,
           probe.hash);
    return 0;
}
