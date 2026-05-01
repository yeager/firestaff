#include <stdio.h>
#include <string.h>

#include "touch_click_zone_matrix_pc34_compat.h"

/* Source-locked touch/pointer dispatch probe.
 *
 * This intentionally stays one layer above the command queue: a physical
 * pointer point is normalized to the original 320x200 DM1 screen and mapped to
 * the same command ids used by ReDMCSB V1 mouse routing.
 *
 * Primary source anchors:
 * - COMMAND.C:375-387 G0447 maps champion status-box right-click zones.
 * - COMMAND.C:396-405 G0448 maps C080 viewport and C083 right-click screen.
 * - COMMAND.C:1403-1431 F0358 scans MOUSE_INPUT boxes and button masks.
 * - COMMAND.C:1641-1644 F0359 searches primary input, then secondary input.
 * - COMMAND.C:2158-2163 dispatches champion status-box commands.
 * - COMMAND.C:2296-2300 dispatches C083 inventory-toggle-leader.
 * - COMMAND.C:2322-2324 dispatches C080 click-in-dungeon-view.
 * - COORD.C:1693-1722 locks viewport origin/extent to x=0 y=33 w=224 h=136.
 * - COORD.C:1915-1920 F0798 performs the inclusive source zone hit-test.
 * - MENUDRAW.C:5-17 draws the source movement-arrow/menu zone family.
 */

typedef struct PointerCase {
    const char* name;
    int physicalX;
    int physicalY;
    int surfaceW;
    int surfaceH;
    unsigned int buttonMask;
    unsigned int expectedCommand;
    unsigned int expectedZone;
    const char* expectedGroup;
} PointerCase;

static int expect_dispatch(const PointerCase* c) {
    TouchClickDispatchPc34Compat d;
    int ok = TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(
        c->physicalX,
        c->physicalY,
        c->surfaceW,
        c->surfaceH,
        c->buttonMask,
        &d);
    if (!ok) {
        printf("case=%s result=miss expectedCommand=%u expectedZone=%u\n",
               c->name,
               c->expectedCommand,
               c->expectedZone);
        return 0;
    }
    printf("case=%s screen=%d,%d command=%u zone=%u group=%s button=0x%04x\n",
           c->name,
           d.screenX,
           d.screenY,
           d.commandId,
           d.zoneIndex,
           d.groupName ? d.groupName : "(null)",
           d.buttonStatus);
    return d.commandId == c->expectedCommand &&
           d.zoneIndex == c->expectedZone &&
           d.groupName && strcmp(d.groupName, c->expectedGroup) == 0;
}

int main(void) {
    static const PointerCase cases[] = {
        { "viewport.c080_left",        320, 216, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT,  80u,   7u, "viewport.dungeon" },
        { "inventory.c083_right",     1144, 684, 1280, 720, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 83u,   2u, "inventory.toggle_leader" },
        { "champion0.right_primary",   100,  39, 1280, 800, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT,  7u, 151u, "champion0.toggle_box" },
        { "champion3.right_primary",   900,  39, 1280, 800, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, 10u, 154u, "champion3.toggle_box" },
    };
    unsigned int i;
    int ok = 1;
    TouchClickDispatchPc34Compat guard;

    printf("probe=firestaff_m11_touch_pointer_dispatch\n");
    printf("sourceEvidence=%s\n", TOUCHCLICK_Compat_GetSourceEvidence());
    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (!expect_dispatch(&cases[i])) ok = 0;
    }

    if (TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(
            320, 216, 1280, 720, TOUCH_CLICK_BUTTON_RIGHT_PC34_COMPAT, &guard)) {
        if (guard.commandId != 83u || guard.zoneIndex != 2u) {
            printf("case=viewport_right_should_toggle screen=%d,%d command=%u zone=%u group=%s\n",
                   guard.screenX,
                   guard.screenY,
                   guard.commandId,
                   guard.zoneIndex,
                   guard.groupName ? guard.groupName : "(null)");
            ok = 0;
        } else {
            printf("case=viewport_right_should_toggle result=pass\n");
        }
    } else {
        printf("case=viewport_right_should_toggle result=miss\n");
        ok = 0;
    }

    if (TOUCHCLICK_Compat_MapScaledScreenPointToDispatch(
            20, 20, 1280, 720, TOUCH_CLICK_BUTTON_LEFT_PC34_COMPAT, &guard)) {
        printf("case=letterbox_guard result=unexpected-hit\n");
        ok = 0;
    } else {
        printf("case=letterbox_guard result=miss\n");
    }

    printf("touchPointerDispatchInvariantOk=%u\n", ok ? 1u : 0u);
    return ok ? 0 : 1;
}
