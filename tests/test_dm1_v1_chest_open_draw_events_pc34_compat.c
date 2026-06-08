#include "dm1_v1_chest_open_draw_events_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static DM1_V1_ChestOpenDrawProbePc34 g_probe;
static int g_assertions;

static int expect_int(const char* label,
                      int got,
                      int want,
                      const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (got != want) {
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, redmcsbAnchor);
        return 0;
    }
    printf("PASS %s=%d anchor=%s\n", label, got, redmcsbAnchor);
    return 1;
}

static int expect_contains(const char* label,
                           const char* haystack,
                           const char* needle,
                           const char* redmcsbAnchor)
{
    ++g_assertions;
    if (!redmcsbAnchor || redmcsbAnchor[0] == '\0') {
        printf("FAIL %s missing ReDMCSB anchor\n", label);
        return 0;
    }
    if (!haystack || !needle || !strstr(haystack, needle)) {
        printf("FAIL %s missing=%s anchor=%s\n", label,
               needle ? needle : "(null)", redmcsbAnchor);
        return 0;
    }
    printf("PASS %s contains=%s anchor=%s\n", label, needle, redmcsbAnchor);
    return 1;
}

static int expect_event(const char* label,
                        const DM1_V1_ChestOpenDrawEventPc34* event,
                        int kind,
                        int slotBox,
                        int graphicOrIcon,
                        const char* redmcsbAnchor)
{
    int ok = 1;

    ok &= expect_int(label, event ? event->eventKind : -999, kind,
                     redmcsbAnchor);
    ok &= expect_int("event slot box", event ? event->slotBox : -999,
                     slotBox, redmcsbAnchor);
    ok &= expect_int("event graphic/icon",
                     event ? event->graphicOrIcon : -999,
                     graphicOrIcon, redmcsbAnchor);
    return ok;
}

static int test_source_evidence(void)
{
    const char* evidence =
        dm1_v1_chest_open_draw_events_source_evidence_pc34();
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-76";
    int ok = 1;

    ok &= expect_contains("evidence same-open", evidence, "F0333:30-32",
                          f0333);
    ok &= expect_contains("evidence action icon", evidence, "C145 in C09",
                          f0333);
    ok &= expect_contains("evidence panel", evidence, "C025 open-chest",
                          f0333);
    ok &= expect_contains("evidence slots", evidence, "C38..C45", f0333);
    ok &= expect_contains("evidence eight-slot limit", evidence,
                          "first eight", f0333);
    ok &= expect_contains("evidence panel route", evidence, "PANEL.C",
                          "ReDMCSB PANEL.C F0347/F0354 inventory panel route");
    return ok;
}

static int test_spec_constants(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 43-76";
    int ok = 1;

    ok &= expect_int("contract marker", g_probe.sourceLockedContractOnly, 1,
                     f0333);
    ok &= expect_int("C09 action hand", g_probe.c09ActionHandSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND, f0333);
    ok &= expect_int("C38 chest first", g_probe.c38ChestFirstSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST, f0333);
    ok &= expect_int("C45 chest last", g_probe.c45ChestLastSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST, f0333);
    ok &= expect_int("C025 panel", g_probe.c025OpenChestPanelGraphic,
                     DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL,
                     f0333);
    ok &= expect_int("C145 open icon", g_probe.c145OpenChestIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST, f0333);
    return ok;
}

static int test_normal_open_draws_action_panel_and_slots(void)
{
    const char* f0333Action = "ReDMCSB CHEST.C F0333 lines 43-48";
    const char* f0333Slots = "ReDMCSB CHEST.C F0333 lines 53-76";
    const DM1_V1_ChestOpenDrawCasePc34* c = &g_probe.normalOpen;
    int ok = 1;
    int i;

    ok &= expect_int("normal opens", c->openResult, 1, f0333Action);
    ok &= expect_int("normal not pressing eye", c->pressingEye, 0,
                     f0333Action);
    ok &= expect_int("normal event count", c->eventCount, 10,
                     f0333Slots);
    ok &= expect_int("normal action icon count",
                     c->actionHandOpenIconCount, 1, f0333Action);
    ok &= expect_int("normal panel blit count", c->panelBlitCount, 1,
                     f0333Action);
    ok &= expect_int("normal slot icon count", c->slotIconCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333Slots);
    ok &= expect_int("normal filled slot count", c->filledSlotIconCount, 3,
                     f0333Slots);
    ok &= expect_int("normal cleared slot count", c->clearedSlotIconCount, 5,
                     f0333Slots);
    ok &= expect_int("normal first slot box", c->firstSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST, f0333Slots);
    ok &= expect_int("normal last slot box", c->lastSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST, f0333Slots);
    ok &= expect_int("normal first filled icon", c->firstFilledIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST, f0333Slots);
    ok &= expect_int("normal last filled icon", c->lastFilledIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST + 2, f0333Slots);
    ok &= expect_int("normal last cleared icon", c->lastClearedIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE, f0333Slots);
    ok &= expect_int("normal materialized slots", c->materializedSlotCount,
                     3, f0333Slots);
    ok &= expect_int("normal overflow inputs", c->overflowInputCount, 0,
                     f0333Slots);

    ok &= expect_event("normal event 0 action", &c->events[0],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_ACTION_ICON,
                       DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND,
                       DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST,
                       f0333Action);
    ok &= expect_event("normal event 1 panel", &c->events[1],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT, 0,
                       DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL,
                       f0333Action);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        const int wantIcon =
            i < 3 ? DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST + i :
            DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE;

        ok &= expect_event("normal slot event", &c->events[2 + i],
                           DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON,
                           DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST + i,
                           wantIcon, f0333Slots);
    }
    return ok;
}

static int test_overflow_open_draws_only_first_eight_slots(void)
{
    const char* f0333Action = "ReDMCSB CHEST.C F0333 lines 43-48";
    const char* f0333Slots =
        "ReDMCSB CHEST.C F0333 lines 53-76 CHANGE8_08_FIX";
    const DM1_V1_ChestOpenDrawCasePc34* c = &g_probe.overflowOpen;
    int ok = 1;
    int i;

    ok &= expect_int("overflow opens", c->openResult, 1, f0333Action);
    ok &= expect_int("overflow linked input count", c->linkedItemCount,
                     DM1_PC34_CHEST_OPEN_DRAW_LINKED_ITEM_MAX, f0333Slots);
    ok &= expect_int("overflow event count", c->eventCount, 10,
                     f0333Slots);
    ok &= expect_int("overflow action icon count",
                     c->actionHandOpenIconCount, 1, f0333Action);
    ok &= expect_int("overflow panel blit count", c->panelBlitCount, 1,
                     f0333Action);
    ok &= expect_int("overflow slot icon count", c->slotIconCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333Slots);
    ok &= expect_int("overflow filled slot count", c->filledSlotIconCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333Slots);
    ok &= expect_int("overflow cleared slot count", c->clearedSlotIconCount,
                     0, f0333Slots);
    ok &= expect_int("overflow materialized slots",
                     c->materializedSlotCount, DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Slots);
    ok &= expect_int("overflow hidden input count",
                     c->overflowInputCount,
                     DM1_PC34_CHEST_OPEN_DRAW_LINKED_ITEM_MAX -
                     DM1_PC34_CHEST_SLOT_COUNT,
                     f0333Slots);
    ok &= expect_int("overflow tail not materialized",
                     c->overflowTailMaterialized, 0, f0333Slots);
    ok &= expect_int("overflow first filled icon", c->firstFilledIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST, f0333Slots);
    ok &= expect_int("overflow last filled icon", c->lastFilledIcon,
                     DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST +
                     DM1_PC34_CHEST_SLOT_COUNT - 1, f0333Slots);
    ok &= expect_int("overflow first slot box", c->firstSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST, f0333Slots);
    ok &= expect_int("overflow last slot box", c->lastSlotBox,
                     DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST, f0333Slots);
    ok &= expect_event("overflow event 0 action", &c->events[0],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_ACTION_ICON,
                       DM1_PC34_CHEST_OPEN_DRAW_SLOT_ACTION_HAND,
                       DM1_PC34_CHEST_OPEN_DRAW_ICON_OPEN_CHEST,
                       f0333Action);
    ok &= expect_event("overflow event 1 panel", &c->events[1],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT, 0,
                       DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL,
                       f0333Action);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        ok &= expect_event("overflow slot event", &c->events[2 + i],
                           DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON,
                           DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST + i,
                           DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST + i,
                           f0333Slots);
    }
    return ok;
}

static int test_pressing_eye_suppresses_action_icon_only(void)
{
    const char* f0333Action = "ReDMCSB CHEST.C F0333 lines 43-48";
    const char* f0333Slots = "ReDMCSB CHEST.C F0333 lines 53-76";
    const DM1_V1_ChestOpenDrawCasePc34* c = &g_probe.pressingEyeOpen;
    int ok = 1;

    ok &= expect_int("pressing-eye opens", c->openResult, 1, f0333Action);
    ok &= expect_int("pressing-eye flag", c->pressingEye, 1, f0333Action);
    ok &= expect_int("pressing-eye event count", c->eventCount, 9,
                     f0333Slots);
    ok &= expect_int("pressing-eye action icon count",
                     c->actionHandOpenIconCount, 0, f0333Action);
    ok &= expect_int("pressing-eye panel blit count", c->panelBlitCount, 1,
                     f0333Action);
    ok &= expect_int("pressing-eye slot icon count", c->slotIconCount,
                     DM1_PC34_CHEST_SLOT_COUNT, f0333Slots);
    ok &= expect_int("pressing-eye filled slot count",
                     c->filledSlotIconCount, 3, f0333Slots);
    ok &= expect_int("pressing-eye cleared slot count",
                     c->clearedSlotIconCount, 5, f0333Slots);
    ok &= expect_event("pressing-eye first event panel", &c->events[0],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_PANEL_BLIT, 0,
                       DM1_PC34_CHEST_OPEN_DRAW_GRAPHIC_OPEN_CHEST_PANEL,
                       f0333Action);
    ok &= expect_event("pressing-eye first slot event", &c->events[1],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON,
                       DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_FIRST,
                       DM1_PC34_CHEST_OPEN_DRAW_ITEM_FIRST, f0333Slots);
    ok &= expect_event("pressing-eye last clear event", &c->events[8],
                       DM1_PC34_CHEST_OPEN_DRAW_EVENT_SLOT_ICON,
                       DM1_PC34_CHEST_OPEN_DRAW_SLOT_CHEST_LAST,
                       DM1_PC34_CHEST_OPEN_DRAW_ICON_NONE, f0333Slots);
    return ok;
}

static int test_same_chest_noop_draws_nothing(void)
{
    const char* f0333Same = "ReDMCSB CHEST.C F0333 lines 30-32";
    const DM1_V1_ChestOpenDrawCasePc34* c = &g_probe.sameChestNoop;
    int ok = 1;

    ok &= expect_int("same-chest opens", c->openResult, 1, f0333Same);
    ok &= expect_int("same-chest marker", c->sameChestBeforeOpen, 1,
                     f0333Same);
    ok &= expect_int("same-chest event count", c->eventCount, 0,
                     f0333Same);
    ok &= expect_int("same-chest action icon count",
                     c->actionHandOpenIconCount, 0, f0333Same);
    ok &= expect_int("same-chest panel blit count", c->panelBlitCount, 0,
                     f0333Same);
    ok &= expect_int("same-chest slot icon count", c->slotIconCount, 0,
                     f0333Same);
    ok &= expect_int("same-chest first slot box", c->firstSlotBox, 0,
                     f0333Same);
    ok &= expect_int("same-chest last slot box", c->lastSlotBox, 0,
                     f0333Same);
    return ok;
}

int main(void)
{
    const char* f0333 = "ReDMCSB CHEST.C F0333 lines 30-76";
    int ok = 1;

    printf("probe=dm1_v1_chest_open_draw_events_pc34_compat\n");
    ok &= expect_int("probe run",
                     dm1_v1_chest_open_draw_events_run_pc34(&g_probe),
                     1, f0333);
    if (!ok) {
        return 1;
    }

    ok &= test_source_evidence();
    ok &= test_spec_constants();
    ok &= test_normal_open_draws_action_panel_and_slots();
    ok &= test_pressing_eye_suppresses_action_icon_only();
    ok &= test_same_chest_noop_draws_nothing();
    ok &= test_overflow_open_draws_only_first_eight_slots();
    ok &= expect_int("minimum assertion count",
                     g_assertions >= 110 ? 1 : 0, 1, f0333);

    printf("assertionCount=%d\n", g_assertions);
    printf("chestOpenDrawEventsInvariantOk=%d\n", ok ? 1 : 0);
    return ok ? 0 : 1;
}
