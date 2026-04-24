#include "title_frontend_v1.h"

#include <stdio.h>
#include <string.h>

typedef struct ProbeTally {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

int main(void) {
    ProbeTally tally;
    V1_TitleFrontendHandoffDecision zero;
    V1_TitleFrontendHandoffDecision last;
    V1_TitleFrontendHandoffDecision firstMenu;
    V1_TitleFrontendHandoffDecision laterMenu;
    V1_TitleFrontendHandoffDecision holdOnlyAfter;

    memset(&tally, 0, sizeof(tally));

    zero = V1_TitleFrontend_DecideTitleMenuHandoffStep(0u, 1);
    last = V1_TitleFrontend_DecideTitleMenuHandoffStep(53u, 1);
    firstMenu = V1_TitleFrontend_DecideTitleMenuHandoffStep(54u, 1);
    laterMenu = V1_TitleFrontend_DecideTitleMenuHandoffStep(106u, 1);
    holdOnlyAfter = V1_TitleFrontend_DecideTitleMenuHandoffStep(54u, 0);

    probe_record(&tally, "P61_TITLE_MENU_HANDOFF_01",
                 zero.surface == V1_TITLE_FRONTEND_SURFACE_TITLE &&
                     zero.title.renderFrameOrdinal == 1u &&
                     !zero.enteredMenuAfterHandoff,
                 "step 0/1 remains on TITLE surface and maps to first source frame");
    probe_record(&tally, "P61_TITLE_MENU_HANDOFF_02",
                 last.surface == V1_TITLE_FRONTEND_SURFACE_TITLE &&
                     last.title.renderFrameOrdinal == 53u &&
                     last.title.handoffReady &&
                     !last.enteredMenuAfterHandoff,
                 "source DO boundary renders frame 53 before any menu transition");
    probe_record(&tally, "P61_TITLE_MENU_HANDOFF_03",
                 firstMenu.surface == V1_TITLE_FRONTEND_SURFACE_MENU &&
                     firstMenu.title.renderFrameOrdinal == 53u &&
                     firstMenu.title.handoffReady &&
                     firstMenu.enteredMenuAfterHandoff,
                 "first post-boundary step enters menu surface while retaining last TITLE frame evidence");
    probe_record(&tally, "P61_TITLE_MENU_HANDOFF_04",
                 laterMenu.surface == V1_TITLE_FRONTEND_SURFACE_MENU &&
                     laterMenu.title.renderFrameOrdinal == 53u &&
                     laterMenu.title.completedAnimationLoops == 1u,
                 "later post-boundary steps stay deterministically on menu surface");
    probe_record(&tally, "P61_TITLE_MENU_HANDOFF_05",
                 holdOnlyAfter.surface == V1_TITLE_FRONTEND_SURFACE_TITLE &&
                     holdOnlyAfter.title.action == V1_TITLE_FRONTEND_SEQUENCE_HOLD_LAST_FRAME &&
                     !holdOnlyAfter.enteredMenuAfterHandoff,
                 "callers can still choose pass-59 hold-last-frame policy instead of entering menu");

    printf("# pass61 handoff decisions: step53 surface=%u frame=%u ready=%d; step54 enter surface=%u frame=%u entered=%d; step54 hold surface=%u action=%u\n",
           (unsigned int)last.surface,
           last.title.renderFrameOrdinal,
           last.title.handoffReady,
           (unsigned int)firstMenu.surface,
           firstMenu.title.renderFrameOrdinal,
           firstMenu.enteredMenuAfterHandoff,
           (unsigned int)holdOnlyAfter.surface,
           (unsigned int)holdOnlyAfter.title.action);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
