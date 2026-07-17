#include "nexus_v1_launcher.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    Nexus_V1_MenuBpkRendererHandoffReceipt handoff;
    Nexus_V1_LauncherM12M11Prs3MaterialCaptureRouteReceipt capture;
    Nexus_V1_LauncherPrs3StartupStateReceipt receipt;
    Nexus_V1_LauncherM12Prs3StartupTransitionReceipt transition;

    memset(&handoff, 0, sizeof(handoff));
    handoff.status = NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_BLOCKED_PRS3;
    expect(nexus_v1_launcher_prs3_startup_state(&handoff, NULL, &receipt) &&
               receipt.valid && receipt.scan_complete && receipt.startup_can_settle &&
               receipt.capture_required && !receipt.capture_consumed &&
               receipt.no_draw_only && !receipt.decoder_permitted &&
               !receipt.synthetic_menu_image_permitted &&
               strcmp(receipt.status, "menu-bpk-prs3-capture-required") == 0,
           "missing PRS3 trace settles startup at capture-required without a draw route");
    expect(nexus_v1_launcher_m12_prs3_startup_transition(
               &handoff, NULL,
               NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RETURN_TO_IDLE,
               &transition) && transition.valid && transition.m12_handled &&
               transition.terminal && transition.return_to_idle &&
               !transition.rescan_complete && transition.no_draw_only &&
               !transition.decoder_permitted &&
               !transition.synthetic_menu_image_permitted &&
               strcmp(transition.status,
                      "menu-bpk-prs3-capture-required") == 0,
           "M12 returns a missing-trace startup to idle without waiting or drawing");
    expect(nexus_v1_launcher_m12_prs3_startup_transition(
               &handoff, NULL, NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_RESCAN,
               &transition) && transition.terminal && !transition.return_to_idle &&
               transition.rescan_complete && transition.no_draw_only &&
               transition.prs3.scan_complete && transition.prs3.capture_required,
           "M12 rescan completes at the same missing-trace capture-required state");

    memset(&capture, 0, sizeof(capture));
    capture.valid = capture.capture_imported = capture.resume_ready = 1;
    capture.operator_only = capture.no_draw_only = 1;
    capture.capture.valid = capture.capture.payload_opaque = 1;
    capture.capture.no_draw_only = 1;
    expect(nexus_v1_launcher_prs3_startup_state(&handoff, &capture, &receipt) &&
               receipt.valid && receipt.scan_complete && receipt.startup_can_settle &&
               !receipt.capture_required && receipt.capture_consumed &&
               receipt.no_draw_only && !receipt.decoder_permitted &&
               !receipt.synthetic_menu_image_permitted &&
               strcmp(receipt.status, "menu-bpk-prs3-capture-consumed") == 0,
           "verified PRS3 capture is consumed but remains fail-closed");
    expect(nexus_v1_launcher_m12_prs3_startup_transition(
               &handoff, &capture,
               NEXUS_V1_LAUNCHER_PRS3_STARTUP_ACTION_PRESENT,
               &transition) && transition.terminal && !transition.return_to_idle &&
               !transition.rescan_complete && transition.prs3.capture_consumed &&
               transition.no_draw_only && !transition.decoder_permitted &&
               !transition.synthetic_menu_image_permitted,
           "M12 consumes a verified capture as a terminal no-draw state");

    capture.decoder_permitted = 1;
    expect(nexus_v1_launcher_prs3_startup_state(&handoff, &capture, &receipt) &&
               receipt.capture_required && !receipt.capture_consumed &&
               receipt.no_draw_only,
           "decoder promotion claim cannot consume the PRS3 startup blocker");

    handoff.status = NEXUS_V1_MENU_BPK_RENDERER_HANDOFF_READY_STORED;
    expect(nexus_v1_launcher_prs3_startup_state(&handoff, NULL, &receipt) &&
               receipt.state == NEXUS_V1_LAUNCHER_PRS3_STARTUP_NOT_REQUIRED &&
               receipt.scan_complete && receipt.startup_can_settle &&
               !receipt.capture_required && !receipt.capture_consumed &&
               !receipt.no_draw_only,
           "non-PRS3 route completes the startup scan without a PRS3 request");

    if (failures) return 1;
    puts("Nexus PRS3 startup state: PASS");
    return 0;
}
