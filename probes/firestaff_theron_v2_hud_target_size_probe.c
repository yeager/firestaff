/*
 * firestaff_theron_v2_hud_target_size_probe.c
 *
 * Headless Theron V2 Phase 6 UI target-size audit probe. Data-free.
 *
 * Locks:
 *   - V1 native 320x240: MESSAGE_BAR (16 px) BLOCKED, TOPBAR (24 px) TIGHT
 *   - V2.1 upscaled 640x480: MESSAGE_BAR (32 px) ACCEPTABLE
 *   - V2.2 modern 1920x1080: every zone COMFORTABLE
 *   - Source-lock citations
 *
 * Source-lock: THQUEST.ASM T600 + theron_v1_viewport.h chrome zones.
 */

#include "theron_v2_hud_target_size_pc34.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_passed = 0;

static void check_int(const char *id, int got, int want)
{
    ++g_assertions;
    if (got == want) {
        ++g_passed;
    } else {
        printf("FAIL %s got=%d want=%d\n", id, got, want);
    }
}

static void check_true(const char *id, int condition)
{
    ++g_assertions;
    if (condition) {
        ++g_passed;
    } else {
        printf("FAIL %s\n", id);
    }
}

static void check_v1_native(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord rec;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 320;
    ctx.canvasHeight = 240;
    ctx.mode = THERON_V2_PM_V1_FAITHFUL;

    /* MESSAGE_BAR (16 px) BLOCKED */
    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    check_int("v1.msg.scaled", rec.scaledMinEdge, TQR_MSG_H);
    check_int("v1.msg.tier", rec.tier, THERON_V2_HUD_TARGET_BLOCKED);

    /* TOPBAR (24 px) TIGHT */
    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_TOPBAR);
    check_int("v1.topbar.scaled", rec.scaledMinEdge, TQR_TOPBAR_H);
    check_int("v1.topbar.tier", rec.tier, THERON_V2_HUD_TARGET_TIGHT);

    /* VIEWPORT (192x160) min=160 COMFORTABLE */
    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_VIEWPORT);
    check_int("v1.viewport.scaled", rec.scaledMinEdge, TQR_VP_W < TQR_VP_H ? TQR_VP_W : TQR_VP_H);
    check_int("v1.viewport.tier", rec.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    /* Audit roll-up */
    audit = theron_v2_hud_target_size_audit(&ctx);
    check_int("v1.audit.blocked", audit.blockedCount, 1);
    check_int("v1.audit.worstTier", audit.worstTier, THERON_V2_HUD_TARGET_BLOCKED);
    check_int("v1.audit.minEdge", audit.minScaledEdge, TQR_MSG_H);
}

static void check_v21_upscaled(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord rec;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 640;
    ctx.canvasHeight = 480;
    ctx.mode = THERON_V2_PM_V21_UPSCALED;

    /* Scale 2x: 16 * 2 = 32 ACCEPTABLE */
    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    check_int("v21.msg.scaled", rec.scaledMinEdge, TQR_MSG_H * 2);
    check_int("v21.msg.tier", rec.tier, THERON_V2_HUD_TARGET_ACCEPTABLE);

    /* 24 * 2 = 48 COMFORTABLE */
    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_TOPBAR);
    check_int("v21.topbar.scaled", rec.scaledMinEdge, TQR_TOPBAR_H * 2);
    check_int("v21.topbar.tier", rec.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    audit = theron_v2_hud_target_size_audit(&ctx);
    check_int("v21.audit.blocked", audit.blockedCount, 0);
}

static void check_v22_modern(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord rec;
    int scale;
    int expected_msg;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 1920;
    ctx.canvasHeight = 1080;
    ctx.mode = THERON_V2_PM_V22_MODERN;

    /* scaleX = 1920/320 = 6, scaleY = 1080/240 = 4.5 → min = 4 */
    scale = 4;
    expected_msg = TQR_MSG_H * scale;

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    check_int("v22.msg.scaled", rec.scaledMinEdge, expected_msg);
    check_int("v22.msg.tier", rec.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    audit = theron_v2_hud_target_size_audit(&ctx);
    check_int("v22.audit.blocked", audit.blockedCount, 0);
    check_int("v22.audit.tight", audit.tightCount, 0);
    check_int("v22.audit.comfortable", audit.comfortableCount, THERON_V2_HUD_ZONE_COUNT);
}

static void check_source_evidence(void)
{
    const char *e = theron_v2_hud_target_size_source_evidence();
    check_true("evidence.present", e != 0 && e[0] != 0);
    check_true("evidence.THQUEST_T600", strstr(e, "T600") != 0);
    check_true("evidence.COMMAND_C_F0380", strstr(e, "F0380") != 0);
    check_true("evidence.theron_v1_viewport", strstr(e, "theron_v1_viewport") != 0);
    check_true("evidence.TQR_TOPBAR_H", strstr(e, "TQR_TOPBAR_H") != 0);
    check_true("evidence.TQR_MSG_H", strstr(e, "TQR_MSG_H") != 0);
}

int main(void)
{
    printf("=== firestaff_theron_v2_hud_target_size_probe ===\n");
    check_v1_native();
    check_v21_upscaled();
    check_v22_modern();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
