/*
 * test_theron_v2_hud_target_size_pc34.c
 *
 * Unit test for Theron V2 Phase 6 UI target-size audit.
 *
 * Source-lock:
 *  - include/theron_v2_hud_target_size_pc34.h
 *  - src/theron/theron_v2_hud_target_size_pc34.c
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

/* V1 native canvas: 320x240 (TQR_SCREEN_W x TQR_SCREEN_H). */
static void check_v1_native_blocked_for_small_zones(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord msg;
    THERON_V2_HudTargetSizeRecord topbar;
    THERON_V2_HudTargetSizeRecord viewport;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 320;
    ctx.canvasHeight = 240;
    ctx.mode = THERON_V2_PM_V1_FAITHFUL;

    /* MESSAGE_BAR is 16 px tall in V1 — must be BLOCKED (< 24). */
    msg = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    check_int("v1.msg.x", msg.x, 0);
    check_int("v1.msg.y", msg.y, TQR_CHAMP_SLOT_Y);
    check_int("v1.msg.w", msg.w, TQR_SCREEN_W);
    check_int("v1.msg.h", msg.h, TQR_MSG_H);
    check_int("v1.msg.min", msg.minEdge, TQR_MSG_H);
    check_int("v1.msg.scaled", msg.scaledMinEdge, TQR_MSG_H);
    check_int("v1.msg.tier", msg.tier, THERON_V2_HUD_TARGET_BLOCKED);

    /* TOPBAR is 24 px tall — borderline. */
    topbar = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_TOPBAR);
    check_int("v1.topbar.h", topbar.h, TQR_TOPBAR_H);
    check_int("v1.topbar.scaled", topbar.scaledMinEdge, TQR_TOPBAR_H);
    /* 24 px falls in TIGHT (24..31) */
    check_int("v1.topbar.tier", topbar.tier, THERON_V2_HUD_TARGET_TIGHT);

    /* VIEWPORT is 192x160 — comfortable. */
    viewport = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_VIEWPORT);
    check_int("v1.viewport.min", viewport.minEdge, TQR_VP_W < TQR_VP_H ? TQR_VP_W : TQR_VP_H);
    check_int("v1.viewport.scaled", viewport.scaledMinEdge,
              TQR_VP_W < TQR_VP_H ? TQR_VP_W : TQR_VP_H);
    /* min(192, 160) = 160 → COMFORTABLE */
    check_int("v1.viewport.tier", viewport.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    audit = theron_v2_hud_target_size_audit(&ctx);
    check_int("v1.audit.zoneCount", audit.zoneCount, THERON_V2_HUD_ZONE_COUNT);
    check_int("v1.audit.blocked", audit.blockedCount, 1);
    check_true("v1.audit.tight_or_better", audit.tightCount >= 1);
    check_int("v1.audit.minEdge", audit.minScaledEdge, TQR_MSG_H);
    check_int("v1.audit.worstTier", audit.worstTier, THERON_V2_HUD_TARGET_BLOCKED);
}

/* V2.2 modern 1920x1080 canvas: scale ≈ 6x by min(scaleX, scaleY).
 * scaleX = 1920/320 = 6, scaleY = 1080/240 = 4.5 → min = 4.
 * 24 * 4 = 96 → COMFORTABLE for topbar; 16 * 4 = 64 → COMFORTABLE for msg. */
static void check_v22_1920x1080_all_comfortable(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord msg;
    THERON_V2_HudTargetSizeRecord topbar;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 1920;
    ctx.canvasHeight = 1080;
    ctx.mode = THERON_V2_PM_V22_MODERN;

    msg = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    /* min(scaleX=6, scaleY=4) = 4 → 16 * 4 = 64 */
    check_int("v22.msg.scaled", msg.scaledMinEdge, TQR_MSG_H * 4);
    check_int("v22.msg.tier", msg.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    topbar = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_TOPBAR);
    /* 24 * 4 = 96 */
    check_int("v22.topbar.scaled", topbar.scaledMinEdge, TQR_TOPBAR_H * 4);
    check_int("v22.topbar.tier", topbar.tier, THERON_V2_HUD_TARGET_COMFORTABLE);

    audit = theron_v2_hud_target_size_audit(&ctx);
    check_int("v22.audit.blocked", audit.blockedCount, 0);
    check_int("v22.audit.tight", audit.tightCount, 0);
    check_int("v22.audit.comfortable", audit.comfortableCount, THERON_V2_HUD_ZONE_COUNT);
    check_int("v22.audit.minEdge", audit.minScaledEdge, TQR_MSG_H * 4);
    check_int("v22.audit.worstTier", audit.worstTier, THERON_V2_HUD_TARGET_COMFORTABLE);
}

/* V2.1 upscaled 640x480 canvas: scaleX = 2, scaleY = 2 → 16 * 2 = 32 ACCEPTABLE. */
static void check_v21_640x480_msg_acceptable(void)
{
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeAudit audit;
    THERON_V2_HudTargetSizeRecord msg;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 640;
    ctx.canvasHeight = 480;
    ctx.mode = THERON_V2_PM_V21_UPSCALED;

    msg = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_MESSAGE_BAR);
    check_int("v21.msg.scaled", msg.scaledMinEdge, TQR_MSG_H * 2);
    /* 32 is in [32..47] → ACCEPTABLE */
    check_int("v21.msg.tier", msg.tier, THERON_V2_HUD_TARGET_ACCEPTABLE);

    audit = theron_v2_hud_target_size_audit(&ctx);
    /* No zone should be BLOCKED at 640x480. */
    check_int("v21.audit.blocked", audit.blockedCount, 0);
    check_int("v21.audit.minEdge", audit.minScaledEdge, TQR_MSG_H * 2);
}

/* Null/zero context: BLOCKED with zero size (does not crash). */
static void check_null_context_safe(void)
{
    THERON_V2_HudTargetSizeRecord rec;
    THERON_V2_HudTargetSizeAudit audit;

    rec = theron_v2_hud_target_size_for_zone(0, THERON_V2_HUD_ZONE_VIEWPORT);
    check_int("null.rec.tier", rec.tier, THERON_V2_HUD_TARGET_BLOCKED);
    check_int("null.rec.scaled", rec.scaledMinEdge, 0);

    rec = theron_v2_hud_target_size_for_zone(0, THERON_V2_HUD_ZONE_TOPBAR);
    check_int("null.rec.tier", rec.tier, THERON_V2_HUD_TARGET_BLOCKED);

    /* Zero canvas size: also BLOCKED with zero size. */
    {
        THERON_V2_HudTargetSizeContext ctx;
        memset(&ctx, 0, sizeof(ctx));
        ctx.canvasWidth = 0;
        ctx.canvasHeight = 0;
        ctx.mode = THERON_V2_PM_V1_FAITHFUL;
        rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_VIEWPORT);
        check_int("zero.rec.tier", rec.tier, THERON_V2_HUD_TARGET_BLOCKED);
        check_int("zero.rec.scaled", rec.scaledMinEdge, 0);
    }

    audit = theron_v2_hud_target_size_audit(0);
    check_int("null.audit.zoneCount", audit.zoneCount, THERON_V2_HUD_ZONE_COUNT);
    check_int("null.audit.blocked", audit.blockedCount, THERON_V2_HUD_ZONE_COUNT);
    check_int("null.audit.worstTier", audit.worstTier, THERON_V2_HUD_TARGET_BLOCKED);
}

/* Tier thresholds: 23 → BLOCKED, 24..31 → TIGHT, 32..47 → ACCEPTABLE, 48+ → COMFORTABLE. */
static void check_tier_thresholds(void)
{
    int edge;
    THERON_V2_HudTargetTier tier;

    /* Test through the audit by constructing canvases that produce
     * the desired scaled min-edge for the message bar (16 px source).
     * Scale = min(canvasW/320, canvasH/240). */
    edge = 23; tier = 23 <= THERON_V2_HUD_TARGET_BLOCKED_MAX ? THERON_V2_HUD_TARGET_BLOCKED : -1;
    check_int("threshold.23", tier, THERON_V2_HUD_TARGET_BLOCKED);
    edge = 24; tier = 24 <= THERON_V2_HUD_TARGET_TIGHT_MAX ? THERON_V2_HUD_TARGET_TIGHT : -1;
    check_int("threshold.24", tier, THERON_V2_HUD_TARGET_TIGHT);
    edge = 31; tier = 31 <= THERON_V2_HUD_TARGET_TIGHT_MAX ? THERON_V2_HUD_TARGET_TIGHT : -1;
    check_int("threshold.31", tier, THERON_V2_HUD_TARGET_TIGHT);
    edge = 32; tier = 32 <= THERON_V2_HUD_TARGET_ACCEPTABLE_MAX ? THERON_V2_HUD_TARGET_ACCEPTABLE : -1;
    check_int("threshold.32", tier, THERON_V2_HUD_TARGET_ACCEPTABLE);
    edge = 47; tier = 47 <= THERON_V2_HUD_TARGET_ACCEPTABLE_MAX ? THERON_V2_HUD_TARGET_ACCEPTABLE : -1;
    check_int("threshold.47", tier, THERON_V2_HUD_TARGET_ACCEPTABLE);
    /* 48+ → COMFORTABLE (else branch) */
    edge = 48; tier = (edge <= THERON_V2_HUD_TARGET_BLOCKED_MAX) ? THERON_V2_HUD_TARGET_BLOCKED
                       : (edge <= THERON_V2_HUD_TARGET_TIGHT_MAX) ? THERON_V2_HUD_TARGET_TIGHT
                       : (edge <= THERON_V2_HUD_TARGET_ACCEPTABLE_MAX) ? THERON_V2_HUD_TARGET_ACCEPTABLE
                       : THERON_V2_HUD_TARGET_COMFORTABLE;
    check_int("threshold.48", tier, THERON_V2_HUD_TARGET_COMFORTABLE);
    edge = 96; tier = (edge <= THERON_V2_HUD_TARGET_BLOCKED_MAX) ? THERON_V2_HUD_TARGET_BLOCKED
                       : (edge <= THERON_V2_HUD_TARGET_TIGHT_MAX) ? THERON_V2_HUD_TARGET_TIGHT
                       : (edge <= THERON_V2_HUD_TARGET_ACCEPTABLE_MAX) ? THERON_V2_HUD_TARGET_ACCEPTABLE
                       : THERON_V2_HUD_TARGET_COMFORTABLE;
    check_int("threshold.96", tier, THERON_V2_HUD_TARGET_COMFORTABLE);
}

static void check_zone_names_and_tiers(void)
{
    /* All tier names are non-NULL and non-empty. */
    check_true("tier.BLOCKED.name",
        strcmp(theron_v2_hud_target_size_tier_name(THERON_V2_HUD_TARGET_BLOCKED),
               "BLOCKED") == 0);
    check_true("tier.TIGHT.name",
        strcmp(theron_v2_hud_target_size_tier_name(THERON_V2_HUD_TARGET_TIGHT),
               "TIGHT") == 0);
    check_true("tier.ACCEPTABLE.name",
        strcmp(theron_v2_hud_target_size_tier_name(THERON_V2_HUD_TARGET_ACCEPTABLE),
               "ACCEPTABLE") == 0);
    check_true("tier.COMFORTABLE.name",
        strcmp(theron_v2_hud_target_size_tier_name(THERON_V2_HUD_TARGET_COMFORTABLE),
               "COMFORTABLE") == 0);
    /* Unknown tier: UNKNOWN string (non-empty). */
    check_true("tier.unknown",
        strcmp(theron_v2_hud_target_size_tier_name((THERON_V2_HudTargetTier)9999),
               "UNKNOWN") == 0);

    /* Zone names are non-NULL and non-empty. */
    check_true("zone.topbar",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_TOPBAR),
               "TOPBAR") == 0);
    check_true("zone.viewport",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_VIEWPORT),
               "VIEWPORT") == 0);
    check_true("zone.right_panel",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_RIGHT_PANEL),
               "RIGHT_PANEL") == 0);
    check_true("zone.bottom_panel",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_BOTTOM_PANEL),
               "BOTTOM_PANEL") == 0);
    check_true("zone.message_bar",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_MESSAGE_BAR),
               "MESSAGE_BAR") == 0);
    check_true("zone.champion_slot",
        strcmp(theron_v2_hud_target_size_zone_name(THERON_V2_HUD_ZONE_CHAMPION_SLOT),
               "CHAMPION_SLOT") == 0);
    check_true("zone.unknown",
        strcmp(theron_v2_hud_target_size_zone_name((THERON_V2_HudZoneKind)9999),
               "UNKNOWN") == 0);
}

static void check_source_evidence(void)
{
    const char *e = theron_v2_hud_target_size_source_evidence();
    check_true("evidence.present", e != 0);
    check_true("evidence.nonempty", e && e[0] != 0);
    check_true("evidence.THQUEST_T600", strstr(e, "T600") != 0);
    check_true("evidence.COMMAND_C_F0380", strstr(e, "F0380") != 0);
    check_true("evidence.theron_v1_viewport", strstr(e, "theron_v1_viewport") != 0);
    check_true("evidence.TQR_VP_X", strstr(e, "TQR_VP_X") != 0);
    check_true("evidence.TQR_TOPBAR_H", strstr(e, "TQR_TOPBAR_H") != 0);
    check_true("evidence.TQR_RIGHT_W", strstr(e, "TQR_RIGHT_W") != 0);
    check_true("evidence.TQR_BOTTOM_H", strstr(e, "TQR_BOTTOM_H") != 0);
    check_true("evidence.TQR_MSG_H", strstr(e, "TQR_MSG_H") != 0);
    check_true("evidence.TQR_CHAMP_SLOT", strstr(e, "TQR_CHAMP_SLOT") != 0);
    check_true("evidence.android_min", strstr(e, "Android minimum") != 0);
    check_true("evidence.ios_min", strstr(e, "iOS HIG") != 0);
}

static void check_zone_geometry_constant_match(void)
{
    /* Source rectangles must match theron_v1_viewport.h. */
    THERON_V2_HudTargetSizeContext ctx;
    THERON_V2_HudTargetSizeRecord rec;

    memset(&ctx, 0, sizeof(ctx));
    ctx.canvasWidth = 320;
    ctx.canvasHeight = 240;
    ctx.mode = THERON_V2_PM_V1_FAITHFUL;

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_VIEWPORT);
    check_int("geom.viewport.x", rec.x, TQR_VP_X);
    check_int("geom.viewport.y", rec.y, TQR_VP_Y);
    check_int("geom.viewport.w", rec.w, TQR_VP_W);
    check_int("geom.viewport.h", rec.h, TQR_VP_H);

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_TOPBAR);
    check_int("geom.topbar.x", rec.x, 0);
    check_int("geom.topbar.y", rec.y, 0);
    check_int("geom.topbar.w", rec.w, TQR_SCREEN_W);
    check_int("geom.topbar.h", rec.h, TQR_TOPBAR_H);

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_RIGHT_PANEL);
    check_int("geom.right.x", rec.x, TQR_SCREEN_W - TQR_RIGHT_W);
    check_int("geom.right.y", rec.y, TQR_TOPBAR_H);
    check_int("geom.right.w", rec.w, TQR_RIGHT_W);
    check_int("geom.right.h", rec.h, TQR_SCREEN_H - TQR_TOPBAR_H - TQR_BOTTOM_H);

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_BOTTOM_PANEL);
    check_int("geom.bottom.x", rec.x, 0);
    check_int("geom.bottom.y", rec.y, TQR_SCREEN_H - TQR_BOTTOM_H);
    check_int("geom.bottom.w", rec.w, TQR_SCREEN_W);
    check_int("geom.bottom.h", rec.h, TQR_BOTTOM_H);

    rec = theron_v2_hud_target_size_for_zone(&ctx, THERON_V2_HUD_ZONE_CHAMPION_SLOT);
    check_int("geom.champ.x", rec.x, 0);
    check_int("geom.champ.y", rec.y, TQR_CHAMP_SLOT_Y);
    check_int("geom.champ.w", rec.w, TQR_CHAMP_SLOT_W);
    check_int("geom.champ.h", rec.h, TQR_CHAMP_SLOT_H);
}

int main(void)
{
    printf("=== Theron V2 Phase 6 HUD target-size audit unit test ===\n");
    check_v1_native_blocked_for_small_zones();
    check_v22_1920x1080_all_comfortable();
    check_v21_640x480_msg_acceptable();
    check_null_context_safe();
    check_tier_thresholds();
    check_zone_names_and_tiers();
    check_zone_geometry_constant_match();
    check_source_evidence();
    printf("--- %d / %d passed ---\n", g_passed, g_assertions);
    if (g_passed != g_assertions) {
        return 1;
    }
    return 0;
}
