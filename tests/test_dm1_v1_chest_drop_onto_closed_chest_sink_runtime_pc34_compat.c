#include "dm1_v1_chest_drop_onto_closed_chest_sink_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestDropOntoClosedChestSinkProbePc34 g_probe;
static int g_assertions;
static int g_failures;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_str(const char* label,
                      const char* got,
                      const char* want,
                      const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0') {
        ++g_failures;
        printf("FAIL %s missing-anchor\n", label);
        return 0;
    }
    if (!got || !want || strcmp(got, want) != 0) {
        ++g_failures;
        printf("FAIL %s got=%s want=%s anchor=%s\n",
               label, got ? got : "(null)", want ? want : "(null)",
               anchor);
        return 0;
    }
    printf("PASS %s=%s anchor=%s\n", label, got, anchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* anchor)
{
    ++g_assertions;
    if (!anchor || anchor[0] == '\0' || !haystack || !needle ||
        !strstr(haystack, needle)) {
        ++g_failures;
        printf("FAIL %s missing=%s anchor=%s\n",
               label, needle ? needle : "(null)",
               anchor ? anchor : "(null)");
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, anchor);
    return 1;
}

static int expected_initial_type(int slot)
{
    if (slot < DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT) {
        return DM1_PC34_CHEST_DROP_CLOSED_SINK_FIRST_CHEST_ITEM + slot;
    }
    return 0;
}

static int expected_initial_weight(int slot)
{
    if (slot < DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT) {
        return 5 + slot;
    }
    return 0;
}

static int expected_initial_charge(int slot)
{
    if (slot < DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT) {
        return 20 + slot;
    }
    return 0;
}

static int expected_open_after_drop_type(int slot)
{
    if (slot == DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX) {
        return DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT;
    }
    return expected_initial_type(slot);
}

static int test_spec_and_evidence(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    const char* evidence =
        dm1_v1_chest_drop_onto_closed_chest_sink_source_evidence_pc34();
    int ok = 1;

    ok &= expect_str("contract marker", spec->contractMarker,
                     "Runtime gate: C537..C544 drops require an open G0426 chest; closed chest manifests reject without absorbing the leader hand object.",
                     spec->f0302Anchor);
    ok &= expect_contains("evidence F0333", evidence, "CHEST.C F0333:31-67",
                          spec->f0333Anchor);
    ok &= expect_contains("evidence F0334", evidence, "CHEST.C F0334:113-132",
                          spec->f0334Anchor);
    ok &= expect_contains("evidence F0374", evidence, "CLIKVIEW.C F0374:131-188",
                          spec->f0374Anchor);
    ok &= expect_contains("evidence F0297/F0298", evidence,
                          "CHAMPION.C F0297/F0298:243-298",
                          spec->f0297F0298Anchor);
    ok &= expect_contains("evidence F0302", evidence,
                          "CHAMPION.C F0302:688-710",
                          spec->f0302Anchor);
    ok &= expect_contains("evidence F0033", evidence, "OBJECT.C F0033:147-212",
                          spec->f0033Anchor);
    ok &= expect_contains("evidence F0133", evidence, "BLITMASK.C F0133:30-33",
                          spec->f0133Anchor);
    ok &= expect_contains("evidence defs slots", evidence, "DEFS.H:810-817",
                          spec->defsSlotAnchor);
    ok &= expect_contains("evidence defs zones", evidence, "DEFS.H:3906-3913",
                          spec->defsZoneAnchor);
    ok &= expect_contains("evidence data masks", evidence, "DATA.C:1080-1087",
                          spec->dataSlotMaskAnchor);
    ok &= expect_contains("evidence F0163", evidence, "DUNGEON.C F0163:1796-1837",
                          spec->f0163Anchor);
    ok &= expect_contains("evidence no F0336", evidence, "No CHEST.C F0336",
                          spec->f0336Note);
    ok &= expect_contains("spec no F0336 note", spec->f0336Note,
                          "PANEL.C F0336", spec->f0336Note);
    ok &= expect_int("spec chest thing", spec->chestThing,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("spec closed reason", spec->closedReasonCode,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_NO_G0426,
                     spec->f0302Anchor);
    ok &= expect_int("spec open reason", spec->openReasonCode,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_ACCEPTED_OPEN_G0426,
                     spec->f0302Anchor);
    ok &= expect_int("spec target slot index", spec->targetChestSlotIndex,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX,
                     spec->defsZoneAnchor);
    ok &= expect_int("spec target pc34 slot", spec->targetPc34Slot,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT,
                     spec->defsSlotAnchor);
    ok &= expect_int("spec C537 pc34 slot", spec->c537Pc34Slot,
                     DM1_PC34_SLOT_CHEST_1, spec->defsSlotAnchor);
    ok &= expect_int("spec C544 pc34 slot", spec->c544Pc34Slot,
                     DM1_PC34_SLOT_CHEST_8, spec->defsSlotAnchor);
    ok &= expect_int("spec closed keeps hand",
                     spec->closedDropKeepsLeaderHand, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("spec open stores in G0425", spec->openDropStoresInG0425,
                     1, spec->f0302Anchor);
    return ok;
}

static int test_setup(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    int ok = 1;

    ok &= expect_int("setup open result", g_probe.setupOpenResult, 1,
                     spec->f0333Anchor);
    ok &= expect_int("setup open thing", g_probe.setupOpenThing,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("setup close count", g_probe.setupCloseCount,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT,
                     spec->f0334Anchor);
    ok &= expect_int("setup closed open thing", g_probe.setupClosedOpenThing,
                     0, spec->f0334Anchor);
    ok &= expect_int("closed chest has manifest",
                     g_probe.closedChestHasManifest, 1,
                     spec->f0334Anchor);
    ok &= expect_int("closed G0426 before drop",
                     g_probe.closedChestG0426BeforeDrop, 0,
                     spec->f0334Anchor);
    ok &= expect_int("target slot initially empty",
                     g_probe.closedBeforeTypes[
                         DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX],
                     0, spec->f0334Anchor);
    return ok;
}

static int test_initial_manifest(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    int ok = 1;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        char label[96];
        int wantType = expected_initial_type(i);
        int wantWeight = expected_initial_weight(i);
        int wantCharge = expected_initial_charge(i);
        int wantAllowed = wantType ? DM1_PC34_ALLOWED_CONTAINER : 0;

        snprintf(label, sizeof(label), "initial type C%d", 537 + i);
        ok &= expect_int(label, g_probe.initialTypes[i], wantType,
                         spec->f0333Anchor);
        snprintf(label, sizeof(label), "initial weight C%d", 537 + i);
        ok &= expect_int(label, g_probe.initialWeights[i], wantWeight,
                         spec->f0333Anchor);
        snprintf(label, sizeof(label), "initial charge C%d", 537 + i);
        ok &= expect_int(label, g_probe.initialCharges[i], wantCharge,
                         spec->f0333Anchor);
        snprintf(label, sizeof(label), "initial allowed C%d", 537 + i);
        ok &= expect_int(label, g_probe.initialAllowedSlots[i], wantAllowed,
                         spec->dataSlotMaskAnchor);
        snprintf(label, sizeof(label), "closed before type C%d", 537 + i);
        ok &= expect_int(label, g_probe.closedBeforeTypes[i], wantType,
                         spec->f0334Anchor);
        snprintf(label, sizeof(label), "closed before weight C%d", 537 + i);
        ok &= expect_int(label, g_probe.closedBeforeWeights[i], wantWeight,
                         spec->f0334Anchor);
    }
    return ok;
}

static int test_closed_rejection(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("closed drop result", g_probe.closedDropResult, 0,
                     spec->f0302Anchor);
    ok &= expect_int("closed leader hand before",
                     g_probe.closedLeaderHandBefore,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed leader hand after",
                     g_probe.closedLeaderHandAfter,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed manifest unchanged",
                     g_probe.closedManifestUnchanged, 1,
                     spec->f0334Anchor);
    ok &= expect_int("closed no absorb", g_probe.closedNoAbsorb, 1,
                     spec->f0302Anchor);
    ok &= expect_int("closed no floor fallback",
                     g_probe.closedNoFloorFallback, 1,
                     spec->f0163Anchor);
    ok &= expect_int("closed G0426 after drop",
                     g_probe.closedG0426AfterDrop, 0,
                     spec->f0334Anchor);
    ok &= expect_int("closed hash stable",
                     g_probe.closedManifestHashAfter,
                     g_probe.closedManifestHashBefore,
                     spec->f0334Anchor);

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "closed after type C%d", 537 + i);
        ok &= expect_int(label, g_probe.closedAfterTypes[i],
                         g_probe.closedBeforeTypes[i], spec->f0334Anchor);
        snprintf(label, sizeof(label), "closed after weight C%d", 537 + i);
        ok &= expect_int(label, g_probe.closedAfterWeights[i],
                         g_probe.closedBeforeWeights[i], spec->f0334Anchor);
    }
    return ok;
}

static int test_closed_event(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    const DM1_V1_ChestDropOntoClosedChestSinkEventPc34* event =
        &g_probe.events[0];
    int ok = 1;

    ok &= expect_int("closed event count", g_probe.closedEventCount, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed event row", event->rowIndex, 0,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed event result", event->result, 0,
                     spec->f0302Anchor);
    ok &= expect_int("closed event reason", g_probe.closedEventReason,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_NO_G0426,
                     spec->f0302Anchor);
    ok &= expect_int("closed event target slot", event->targetPc34Slot,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT,
                     spec->defsSlotAnchor);
    ok &= expect_int("closed event target index", event->targetChestSlotIndex,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX,
                     spec->defsZoneAnchor);
    ok &= expect_int("closed event leader before", event->leaderHandBefore,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed event leader after", event->leaderHandAfter,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed event leader preserved",
                     g_probe.closedEventLeaderHandPreserved, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("closed event open before", event->openChestBefore, 0,
                     spec->f0334Anchor);
    ok &= expect_int("closed event open after", event->openChestAfter, 0,
                     spec->f0334Anchor);
    ok &= expect_int("closed event absorbed", event->absorbedByChest, 0,
                     spec->f0302Anchor);
    ok &= expect_int("closed event floor", event->droppedToFloor, 0,
                     spec->f0163Anchor);
    ok &= expect_int("closed event manifest stable",
                     g_probe.closedEventManifestStable, 1,
                     spec->f0334Anchor);
    ok &= expect_int("closed event hash before",
                     event->manifestHashBefore,
                     g_probe.closedManifestHashBefore,
                     spec->f0334Anchor);
    ok &= expect_int("closed event hash after", event->manifestHashAfter,
                     g_probe.closedManifestHashAfter,
                     spec->f0334Anchor);
    return ok;
}

static int test_open_acceptance(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    int ok = 1;
    int i;

    ok &= expect_int("open result", g_probe.openResult, 1,
                     spec->f0333Anchor);
    ok &= expect_int("open thing before drop", g_probe.openThingBeforeDrop,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("open target slot before",
                     g_probe.openTargetSlotBefore, 0,
                     spec->f0302Anchor);
    ok &= expect_int("open drop result", g_probe.openDropResult, 1,
                     spec->f0302Anchor);
    ok &= expect_int("open leader hand before",
                     g_probe.openLeaderHandBefore,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open leader hand after",
                     g_probe.openLeaderHandAfter, 0,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open target slot after",
                     g_probe.openTargetSlotAfter,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT,
                     spec->f0302Anchor);
    ok &= expect_int("open manifest changed",
                     g_probe.openManifestChanged, 1,
                     spec->f0302Anchor);
    ok &= expect_int("open stored held object",
                     g_probe.openStoredHeldObject, 1,
                     spec->f0302Anchor);
    ok &= expect_int("open no floor fallback",
                     g_probe.openNoFloorFallback, 1,
                     spec->f0163Anchor);

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "open before type C%d", 537 + i);
        ok &= expect_int(label, g_probe.openBeforeTypes[i],
                         expected_initial_type(i), spec->f0333Anchor);
        snprintf(label, sizeof(label), "open after type C%d", 537 + i);
        ok &= expect_int(label, g_probe.openAfterTypes[i],
                         expected_open_after_drop_type(i),
                         spec->f0302Anchor);
    }
    return ok;
}

static int test_open_event_and_close(
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec)
{
    const DM1_V1_ChestDropOntoClosedChestSinkEventPc34* event =
        &g_probe.events[1];
    int ok = 1;
    int i;

    ok &= expect_int("open event count", g_probe.openEventCount, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open event row", event->rowIndex, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open event result", event->result, 1,
                     spec->f0302Anchor);
    ok &= expect_int("open event reason", g_probe.openEventReason,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_REASON_ACCEPTED_OPEN_G0426,
                     spec->f0302Anchor);
    ok &= expect_int("open event target slot", event->targetPc34Slot,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_PC34_SLOT,
                     spec->defsSlotAnchor);
    ok &= expect_int("open event target index", event->targetChestSlotIndex,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX,
                     spec->defsZoneAnchor);
    ok &= expect_int("open event leader before", event->leaderHandBefore,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_OPEN_HELD_OBJECT,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open event leader after", event->leaderHandAfter, 0,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open event leader cleared",
                     g_probe.openEventLeaderHandCleared, 1,
                     spec->f0297F0298Anchor);
    ok &= expect_int("open event open before", event->openChestBefore,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("open event open after", event->openChestAfter,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_CHEST_THING,
                     spec->f0333Anchor);
    ok &= expect_int("open event absorbed", event->absorbedByChest, 1,
                     spec->f0302Anchor);
    ok &= expect_int("open event floor", event->droppedToFloor, 0,
                     spec->f0163Anchor);
    ok &= expect_int("open event manifest changed",
                     g_probe.openEventManifestChanged, 1,
                     spec->f0302Anchor);

    ok &= expect_int("open close count", g_probe.openCloseCount,
                     DM1_PC34_CHEST_DROP_CLOSED_SINK_INITIAL_COUNT + 1,
                     spec->f0334Anchor);
    ok &= expect_int("open closed count includes drop",
                     g_probe.openClosedCountIncludesDrop, 1,
                     spec->f0334Anchor);
    ok &= expect_int("open dropped object in closed manifest",
                     g_probe.openDroppedObjectInClosedManifest, 1,
                     spec->f0334Anchor);

    for (i = 0; i < DM1_PC34_CHEST_DROP_CLOSED_SINK_SLOT_COUNT; ++i) {
        char label[96];

        snprintf(label, sizeof(label), "open closed type C%d", 537 + i);
        ok &= expect_int(label, g_probe.openClosedTypes[i],
                         expected_open_after_drop_type(i),
                         spec->f0334Anchor);
        snprintf(label, sizeof(label), "open closed weight C%d", 537 + i);
        ok &= expect_int(label, g_probe.openClosedWeights[i],
                         i == DM1_PC34_CHEST_DROP_CLOSED_SINK_TARGET_SLOT_INDEX
                             ? 29
                             : expected_initial_weight(i),
                         spec->f0334Anchor);
    }
    return ok;
}

int main(void)
{
    const DM1_V1_ChestDropOntoClosedChestSinkSpecPc34* spec =
        dm1_v1_chest_drop_onto_closed_chest_sink_spec_pc34();
    int ok = 1;

    printf("probe=dm1_v1_chest_drop_onto_closed_chest_sink_runtime_pc34_compat\n");
    printf("sourceEvidence=%s\n",
           dm1_v1_chest_drop_onto_closed_chest_sink_source_evidence_pc34());

    ok &= expect_int("probe run",
                     dm1_v1_chest_drop_onto_closed_chest_sink_pc34(&g_probe),
                     1, spec->f0302Anchor);
    ok &= test_spec_and_evidence(spec);
    ok &= test_setup(spec);
    ok &= test_initial_manifest(spec);
    ok &= test_closed_rejection(spec);
    ok &= test_closed_event(spec);
    ok &= test_open_acceptance(spec);
    ok &= test_open_event_and_close(spec);
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 100 ? 1 : 0, 1,
                     spec->f0302Anchor);

    printf("assertionCount=%d\n", g_assertions);
    printf("failureCount=%d\n", g_failures);
    printf("closedChestDropSinkInvariantOk=%d\n", ok && !g_failures ? 1 : 0);
    printf("PASS dm1_v1_chest_drop_onto_closed_chest_sink_runtime_pc34_compat assertions=%d\n",
           g_assertions);
    return ok && !g_failures ? 0 : 1;
}
