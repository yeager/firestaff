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
    V1_TitleFrontendSequenceDecision zero;
    V1_TitleFrontendSequenceDecision first;
    V1_TitleFrontendSequenceDecision penultimate;
    V1_TitleFrontendSequenceDecision last;
    V1_TitleFrontendSequenceDecision after;
    V1_TitleFrontendSequenceDecision later;

    memset(&tally, 0, sizeof(tally));

    zero = V1_TitleFrontend_DecideSequenceStep(0u);
    first = V1_TitleFrontend_DecideSequenceStep(1u);
    penultimate = V1_TitleFrontend_DecideSequenceStep(52u);
    last = V1_TitleFrontend_DecideSequenceStep(53u);
    after = V1_TitleFrontend_DecideSequenceStep(54u);
    later = V1_TitleFrontend_DecideSequenceStep(106u);

    probe_record(&tally, "P59_TITLE_HANDOFF_01",
                 zero.renderFrameOrdinal == 1u && first.renderFrameOrdinal == 1u &&
                     zero.action == V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE &&
                     first.action == V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE,
                 "sequence step 0/1 maps deterministically to first TITLE source frame");
    probe_record(&tally, "P59_TITLE_HANDOFF_02",
                 penultimate.renderFrameOrdinal == 52u && !penultimate.handoffReady &&
                     penultimate.action == V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE,
                 "pre-final TITLE source frame is not marked handoff-ready");
    probe_record(&tally, "P59_TITLE_HANDOFF_03",
                 last.renderFrameOrdinal == 53u && last.handoffReady &&
                     last.action == V1_TITLE_FRONTEND_SEQUENCE_RENDER_TITLE,
                 "source DO boundary marks frame 53 as handoff-ready without skipping it");
    probe_record(&tally, "P59_TITLE_HANDOFF_04",
                 after.renderFrameOrdinal == 53u && after.handoffReady &&
                     after.action == V1_TITLE_FRONTEND_SEQUENCE_HOLD_LAST_FRAME,
                 "post-DO presentation holds the last TITLE source frame instead of wrapping to frame 1");
    probe_record(&tally, "P59_TITLE_HANDOFF_05",
                 later.renderFrameOrdinal == 53u && later.handoffReady &&
                     later.completedAnimationLoops == 1u,
                 "later overrun steps remain stable on last frame for deterministic menu handoff");

    printf("# pass59 handoff decisions: step0->%u step1->%u step52->%u step53->%u/handoff=%d step54->%u/action=%u step106->%u\n",
           zero.renderFrameOrdinal,
           first.renderFrameOrdinal,
           penultimate.renderFrameOrdinal,
           last.renderFrameOrdinal,
           last.handoffReady,
           after.renderFrameOrdinal,
           (unsigned int)after.action,
           later.renderFrameOrdinal);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
