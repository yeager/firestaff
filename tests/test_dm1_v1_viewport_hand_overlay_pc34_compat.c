#include "dm1_v1_viewport_hand_overlay_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static void expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "%s: got %d want %d\n", label, got, want);
        exit(1);
    }
}

int main(void) {
    DM1_V1_ViewportHandOverlayDecisionPc34Compat d;

    expect_int("region hand becomes object while carrying",
               DM1_V1_ViewportHandOverlay_RegionPointerPc34Compat(DM1_V1_POINTER_HAND, 1),
               DM1_V1_POINTER_OBJECT_ICON);
    expect_int("region arrow stays arrow while carrying",
               DM1_V1_ViewportHandOverlay_RegionPointerPc34Compat(DM1_V1_POINTER_ARROW, 1),
               DM1_V1_POINTER_ARROW);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(100, 100, 1, 0, 4, 0);
    expect_int("viewport hand pointer", d.pointerType, DM1_V1_POINTER_HAND);
    expect_int("hand hotspot x", d.hotspotX, 0);
    expect_int("hand last line", d.lastLineIndex, 15);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(100, 100, 1, 1, 4, 0);
    expect_int("object priority over hand in viewport", d.pointerType, DM1_V1_POINTER_OBJECT_ICON);
    expect_int("object hotspot x", d.hotspotX, 8);
    expect_int("object hotspot y", d.hotspotY, 8);
    expect_int("object width", d.width, 18);
    expect_int("object height", d.height, 18);
    expect_int("object last line", d.lastLineIndex, 17);
    expect_int("object custom bitmap", d.usesCustomBitmap, 1);
    expect_int("object palette split candidate", d.paletteSplitCandidate, 1);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(223, 168, 1, 1, 4, 0);
    expect_int("viewport lower-right inclusive object", d.pointerType, DM1_V1_POINTER_OBJECT_ICON);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(224, 100, 1, 1, 4, 0);
    expect_int("menu area arrow", d.pointerType, DM1_V1_POINTER_ARROW);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(100, 169, 1, 1, 4, 0);
    expect_int("message area arrow", d.pointerType, DM1_V1_POINTER_ARROW);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(69 + 43, 10, 1, 0, 4, 0);
    expect_int("top status non-name hand", d.pointerType, DM1_V1_POINTER_HAND);

    d = DM1_V1_ViewportHandOverlay_DecidePc34Compat(69 + 10, 6, 1, 0, 4, 0);
    expect_int("top status name arrow", d.pointerType, DM1_V1_POINTER_ARROW);

    return 0;
}
