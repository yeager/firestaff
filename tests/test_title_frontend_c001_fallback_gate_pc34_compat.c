/*
 * DM1 V1 TITLE C001-vs-TITLE.DAT fallback selection gate.
 *
 * ReDMCSB TITLE.C F0437 PC/F20 source-lock:
 *   - lines 309-310 load/decompress C001_GRAPHIC_TITLE.
 *   - lines 319-324 blit PRESENTS from C001 source y=137.
 *   - lines 333-340 prepare the 320x57 MASTER/STRIKES BACK strip from
 *     C001 source y=80.
 *   - lines 340-360 build the 18 C001 zoom bitmaps.
 *   - lines 362-367 switch to C13_DUNGEON + C14_MASTER before the zoom.
 *
 * The decoded TITLE.DAT bank is Firestaff's visible fallback when GRAPHICS.DAT
 * C001 is absent or too small.  It must not replace a usable C001 runtime path.
 * pass842 still owns deep TITLE.DAT frame/palette regression coverage, pass897
 * still owns SDL/Metal special-palette readback, and the SWSH handoff probe
 * still owns RGBA->indexed texture-state reset.  This gate only pins the source
 * selection seam those probes intentionally do not own.
 */

#include "asset_loader_m11.h"
#include "entrance_frontend_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
#include "title_frontend_v1.h"
#include "vga_palette_pc34_compat.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int g_pass = 0;
static int g_fail = 0;

static const char* runtime_source_name(V1_TitleFrontendRuntimeSource source) {
    switch (source) {
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001: return "GRAPHICS_C001";
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK: return "TITLE_DAT_FALLBACK";
        case V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP: return "SKIP";
    }
    return "UNKNOWN";
}

static void expect_i(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %d want %d\n", label, got, want);
    }
}

static void expect_u(const char* label, unsigned int got, unsigned int want) {
    if (got == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %u want %u\n", label, got, want);
    }
}

static void expect_truth(const char* label, int ok) {
    if (ok) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s\n", label);
    }
}

static void expect_source(const char* label,
                          V1_TitleFrontendRuntimeSourceDecision decision,
                          V1_TitleFrontendRuntimeSource want) {
    if (decision.source == want) {
        ++g_pass;
    } else {
        ++g_fail;
        printf("FAIL %s: got %s want %s\n",
               label,
               runtime_source_name(decision.source),
               runtime_source_name(want));
    }
}

static void check_selection_contract(void) {
    V1_TitleFrontendRuntimeSourceDecision c001Only =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 175u, 0);
    V1_TitleFrontendRuntimeSourceDecision c001BeatsFallback =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 175u, 1);
    V1_TitleFrontendRuntimeSourceDecision widthTooSmall =
        V1_TitleFrontend_SelectRuntimeSource(1, 319u, 175u, 1);
    V1_TitleFrontendRuntimeSourceDecision heightTooSmall =
        V1_TitleFrontend_SelectRuntimeSource(1, 320u, 174u, 1);
    V1_TitleFrontendRuntimeSourceDecision missingC001 =
        V1_TitleFrontend_SelectRuntimeSource(0, 0u, 0u, 1);
    V1_TitleFrontendRuntimeSourceDecision noCandidateDespiteDims =
        V1_TitleFrontend_SelectRuntimeSource(0, 320u, 175u, 0);
    V1_TitleFrontendRuntimeSourceDecision nothingPlayable =
        V1_TitleFrontend_SelectRuntimeSource(1, 319u, 174u, 0);

    expect_source("usable C001 without TITLE.DAT selects GRAPHICS.DAT C001",
                  c001Only,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001);
    expect_i("usable C001 flag is set", c001Only.graphicsC001Usable, 1);
    expect_i("TITLE.DAT fallback flag is clear", c001Only.titleDatFallbackUsable, 0);

    expect_source("usable C001 wins even when TITLE.DAT exists",
                  c001BeatsFallback,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_GRAPHICS_C001);
    expect_i("TITLE.DAT availability is still recorded",
             c001BeatsFallback.titleDatFallbackUsable,
             1);

    expect_source("319-wide C001 falls back to TITLE.DAT",
                  widthTooSmall,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("319-wide C001 is not usable", widthTooSmall.graphicsC001Usable, 0);

    expect_source("174-high C001 falls back to TITLE.DAT",
                  heightTooSmall,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("174-high C001 is not usable", heightTooSmall.graphicsC001Usable, 0);

    expect_source("missing C001 with TITLE.DAT selects fallback",
                  missingC001,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_TITLE_DAT_FALLBACK);
    expect_i("missing C001 is not usable", missingC001.graphicsC001Usable, 0);

    expect_source("candidate flag is authoritative even with source dimensions",
                  noCandidateDespiteDims,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP);
    expect_source("invalid C001 without TITLE.DAT skips title",
                  nothingPlayable,
                  V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP);

    expect_truth("source evidence cites TITLE.C",
                 c001BeatsFallback.sourceLineEvidence &&
                 strstr(c001BeatsFallback.sourceLineEvidence, "TITLE.C") != 0);
    expect_truth("source evidence cites F0437",
                 c001BeatsFallback.sourceLineEvidence &&
                 strstr(c001BeatsFallback.sourceLineEvidence, "F0437") != 0);
}

static void check_runtime_asset_receipt(void) {
    unsigned char c001[320u * 200u];
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 receipt;
    DM1_V1_StartupTitleRuntimeSourceReceipt_PC34 source_receipt;

    memset(c001, 0, sizeof(c001));
    c001[0] = 1u;
    c001[80u * 320u] = 2u;
    c001[137u * 320u] = 3u;
    memset(&receipt, 0, sizeof(receipt));
    expect_i("C001 asset receipt accepts complete PC34 source regions",
             dm1_v1_startup_title_runtime_asset_receipt_pc34(
                 "dm1", c001, 320u, 200u, &receipt) &&
                 receipt.handled && receipt.graphics_c001_dimensions_valid &&
                 receipt.dungeon_source_pixels_present &&
                 receipt.master_source_pixels_present &&
                 receipt.presents_source_pixels_present &&
                 receipt.graphics_c001_pixel_fingerprint != 0u &&
                 receipt.release_c001_ready,
             1);
    c001[137u * 320u] = 0u;
    expect_i("C001 asset receipt rejects missing PRESENTS source pixels",
             dm1_v1_startup_title_runtime_asset_receipt_pc34(
                 "dm1", c001, 320u, 200u, &receipt) &&
                 !receipt.release_c001_ready,
             1);
    expect_i("C001 asset receipt remains DM1-only",
             dm1_v1_startup_title_runtime_asset_receipt_pc34(
                 "csb", c001, 320u, 200u, &receipt) && !receipt.handled,
             1);
    memset(&source_receipt, 0, sizeof(source_receipt));
    expect_i("DM1 startup rejects TITLE.DAT when C001 is unavailable",
             dm1_v1_startup_title_runtime_source_receipt_pc34(
                 "dm1", 0, 0u, 0u, 1, &source_receipt) &&
                 source_receipt.handled &&
                 source_receipt.title_dat_fallback_usable &&
                 source_receipt.selected_runtime_source ==
                     (int)V1_TITLE_FRONTEND_RUNTIME_SOURCE_SKIP &&
                 !source_receipt.fallback_is_visible_last_resort,
             1);
}

static void check_title_presentation_command(void) {
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 asset;
    DM1_V1_StartupTitlePresentationCommand_PC34 command;

    memset(&media, 0, sizeof(media));
    memset(&asset, 0, sizeof(asset));
    memset(&command, 0, sizeof(command));
    expect_i("TITLE presentation command obtains DM1 PC34 media receipt",
             dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", &media),
             1);
    asset.release_c001_ready = 1;
    expect_i("TITLE presentation PRESENTS keeps C12 and source hold",
             dm1_v1_startup_title_presentation_command_pc34(
                 &media, &asset, 1u, &command) && command.present_frame &&
                 command.clear_before_present &&
                 command.special_palette ==
                     VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS &&
                 command.pre_present_delay_ms == 0u &&
                 command.post_present_delay_ms == media.title_presents_hold_ms &&
                 command.source_timing_receipt_consumed &&
                 command.source_asset_receipt_consumed,
             1);
    expect_i("TITLE presentation zoom keeps C13/C14 VBlank cadence",
             dm1_v1_startup_title_presentation_command_pc34(
                 &media, &asset, 2u, &command) && command.present_frame &&
                 command.special_palette == VGA_PALETTE_PC34_SPECIAL_TITLE &&
                 command.pre_present_delay_ms == media.title_zoom_frame_delay_ms &&
                 command.post_present_delay_ms == 0u,
             1);
    expect_i("TITLE post-zoom wait remains a non-present VBlank event",
             dm1_v1_startup_title_presentation_command_pc34(
                 &media, &asset, 20u, &command) && !command.present_frame &&
                 command.pre_present_delay_ms == media.title_zoom_frame_delay_ms,
             1);
    expect_i("TITLE Strikes Back keeps DM1 C13/C14 palette",
             dm1_v1_startup_title_presentation_command_pc34(
                 &media, &asset, 22u, &command) && command.present_frame &&
                 command.special_palette == VGA_PALETTE_PC34_SPECIAL_TITLE,
             1);
    media.title_zoom_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    expect_i("TITLE presentation rejects a CSB palette substitution",
             !dm1_v1_startup_title_presentation_command_pc34(
                 &media, &asset, 2u, &command),
             1);
}

static void check_entrance_credits_presentation_command(void) {
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34 command;
    DM1_V1_StartupEntranceCreditsReturnCommand_PC34 return_command;
    unsigned char c005[320U * 200U];

    memset(&media, 0, sizeof(media));
    memset(&command, 0, sizeof(command));
    memset(&return_command, 0, sizeof(return_command));
    memset(c005, 0, sizeof(c005));
    c005[320U * 100U + 160U] = 15U;
    expect_i("entrance credits command obtains DM1 PC34 media receipt",
             dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", &media),
             1);
    expect_i("entrance C005 command keeps DM1 credits palette and 1800 VBlanks",
             dm1_v1_startup_entrance_credits_presentation_command_pc34(
                 &media, c005, 320U, 200U, &command) &&
                 command.present_credits_frame &&
                 command.special_palette == VGA_PALETTE_PC34_SPECIAL_CREDITS &&
                 command.credits_wait_ticks == 1800U &&
                 command.vblank_delay_ms == media.entrance_vblank_ms &&
                 command.source_asset_receipt_consumed &&
                 command.source_palette_receipt_consumed &&
                 command.source_timing_receipt_consumed &&
                 command.graphics_c005_pixel_fingerprint != 0U,
             1);
    expect_i("credits return redraws DM1 entrance after the F0442 phase",
             dm1_v1_startup_entrance_credits_return_command_pc34(
                 &media, &command, &return_command) &&
                 return_command.credits_phase_receipt_consumed &&
                 return_command.redraw_closed_entrance &&
                 return_command.discard_pending_input &&
                 return_command.present_entrance_palette &&
                 return_command.special_palette ==
                     VGA_PALETTE_PC34_SPECIAL_ENTRANCE &&
                 return_command.wait_vblank_delay_ms ==
                     media.entrance_vblank_ms,
             1);
    c005[320U * 100U + 160U] = 0U;
    expect_i("entrance C005 command rejects an empty synthetic credits frame",
             !dm1_v1_startup_entrance_credits_presentation_command_pc34(
                 &media, c005, 320U, 200U, &command),
             1);
    media.entrance_credits_palette = VGA_PALETTE_PC34_SPECIAL_CSB_TITLE_CHAOS;
    c005[320U * 100U + 160U] = 15U;
    expect_i("entrance C005 command rejects a CSB palette substitution",
             !dm1_v1_startup_entrance_credits_presentation_command_pc34(
                 &media, c005, 320U, 200U, &command),
             1);
    expect_i("credits return rejects a CSB credits phase",
             !dm1_v1_startup_entrance_credits_return_command_pc34(
                 &media, &command, &return_command),
             1);
}

static void check_palette_cross_source_contract(void) {
    V1_TitleFrontendSourceAnimationStep presentsStep;
    V1_TitleFrontendSourceAnimationStep zoomStep;
    V1_TitleFrontendSourceAnimationStep strikesStep;
    V1_TitleFrontendC001BlitPlan presentsPlan;
    V1_TitleFrontendC001BlitPlan zoomPlan;
    V1_TitleFrontendC001BlitPlan strikesPlan;
    V1_TitleFrontendC001BlitPlan waitPlan;
    int presentsPalette = -1;
    int fallbackPresentsPalette = -1;
    int zoomPalette = -1;
    int fallbackZoomPalette = -1;
    int fallbackLatePalette = -1;

    memset(&presentsStep, 0, sizeof(presentsStep));
    memset(&zoomStep, 0, sizeof(zoomStep));
    memset(&strikesStep, 0, sizeof(strikesStep));
    memset(&presentsPlan, 0, sizeof(presentsPlan));
    memset(&zoomPlan, 0, sizeof(zoomPlan));
    memset(&strikesPlan, 0, sizeof(strikesPlan));
    memset(&waitPlan, 0, sizeof(waitPlan));

    expect_i("C001 source step 1 exists",
             V1_TitleFrontend_GetSourceAnimationStep(1u, &presentsStep),
             1);
    expect_u("C001 source step 1 is PRESENTS",
             (unsigned int)presentsStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS);
    expect_i("C001 source step 19 exists",
             V1_TitleFrontend_GetSourceAnimationStep(19u, &zoomStep),
             1);
    expect_u("C001 source step 19 is ZOOM",
             (unsigned int)zoomStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT);
    expect_i("C001 source step 22 exists",
             V1_TitleFrontend_GetSourceAnimationStep(22u, &strikesStep),
             1);
    expect_u("C001 source step 22 is STRIKES BACK",
             (unsigned int)strikesStep.kind,
             (unsigned int)V1_TITLE_FRONTEND_SOURCE_EVENT_MASTER_STRIKES_BACK_BLIT);

    expect_i("C001 PRESENTS palette resolves",
             V1_TitleFrontend_GetStepPalette(presentsStep.kind, &presentsPalette),
             1);
    expect_i("fallback frame 1 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(1u, &fallbackPresentsPalette),
             1);
    expect_i("C001 ZOOM palette resolves",
             V1_TitleFrontend_GetStepPalette(zoomStep.kind, &zoomPalette),
             1);
    expect_i("fallback frame 2 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(2u, &fallbackZoomPalette),
             1);
    expect_i("fallback frame 53 palette resolves",
             V1_TitleFrontend_GetFallbackFramePalette(V1_TITLE_DAT_FRAME_MAX,
                                                      &fallbackLatePalette),
             1);

    expect_i("C001 PRESENTS and TITLE.DAT frame 1 share C12_PRESENTS",
             presentsPalette,
             fallbackPresentsPalette);
    expect_i("C001 ZOOM and TITLE.DAT frame 2 share C13+C14",
             zoomPalette,
             fallbackZoomPalette);
    expect_i("TITLE.DAT late frame stays on C13+C14",
             fallbackLatePalette,
             zoomPalette);
    expect_i("PRESENTS uses the special PRESENTS palette",
             presentsPalette,
             VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS);
    expect_i("ZOOM uses the special TITLE palette",
             zoomPalette,
             VGA_PALETTE_PC34_SPECIAL_TITLE);
    expect_truth("PRESENTS and ZOOM palettes remain distinct",
                 presentsPalette != zoomPalette);

    expect_i("PRESENTS C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&presentsStep, &presentsPlan),
             1);
    expect_u("PRESENTS blit kind",
             (unsigned int)presentsPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_REGION);
    expect_u("PRESENTS source y",
             presentsPlan.srcY,
             137u);
    expect_u("PRESENTS destination y",
             presentsPlan.dstY,
             90u);
    expect_u("PRESENTS height",
             presentsPlan.srcH,
             16u);
    expect_i("PRESENTS clears first",
             presentsPlan.clearBeforeBlit,
             1);

    expect_i("ZOOM C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&zoomStep, &zoomPlan),
             1);
    expect_u("ZOOM blit kind",
             (unsigned int)zoomPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_SCALED_REGION);
    expect_u("ZOOM source x is full C001 title origin",
             zoomPlan.srcX,
             0u);
    expect_u("ZOOM source y is full C001 title origin",
             zoomPlan.srcY,
             0u);
    expect_u("ZOOM source width is full C001 title width",
             zoomPlan.srcW,
             320u);
    expect_u("ZOOM source height is full C001 title height",
             zoomPlan.srcH,
             80u);
    expect_u("ZOOM destination x is source-centred box",
             zoomPlan.dstX,
             zoomStep.x);
    expect_u("ZOOM destination y is source-centred box",
             zoomPlan.dstY,
             zoomStep.y);
    expect_u("ZOOM destination width is source-centred box width",
             zoomPlan.dstW,
             zoomStep.width);
    expect_u("ZOOM destination height is source-centred box height",
             zoomPlan.dstH,
             zoomStep.height);
    expect_i("ZOOM clears first",
             zoomPlan.clearBeforeBlit,
             1);

    expect_i("STRIKES BACK C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&strikesStep, &strikesPlan),
             1);
    expect_u("STRIKES BACK blit kind",
             (unsigned int)strikesPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_REGION);
    expect_u("STRIKES BACK source y",
             strikesPlan.srcY,
             80u);
    expect_u("STRIKES BACK destination y",
             strikesPlan.dstY,
             118u);
    expect_i("STRIKES BACK uses black transparency",
             strikesPlan.transparentColor,
             0);

    expect_i("wait step C001 blit plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&((V1_TitleFrontendSourceAnimationStep){
                 .kind = V1_TITLE_FRONTEND_SOURCE_EVENT_POST_ZOOM_VBLANK
             }), &waitPlan),
             1);
    expect_u("wait step has no blit",
             (unsigned int)waitPlan.kind,
             (unsigned int)V1_TITLE_FRONTEND_C001_BLIT_NONE);
}

static void check_startup_source_timing_contract(void) {
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    SWSH_CompatSourceAnimationStep swshStep;
    EntranceCompatSourceAnimationStep doorStep;
    EntranceCompatSourceAnimationStep finalDoorStep;
    EntranceCompatSourceAnimationStep switchStep;
    EntranceCompatSourceAnimationStep preOpenStep;
    DM1_V1_StartupEntranceRenderAudioCommand_PC34 command;
    unsigned int sourceStep;
    unsigned int swshColorSetCount = 0u;
    unsigned int swshWaitCount = 0u;
    unsigned int swshWaitVblankCount = 0u;
    unsigned int doorCommandCount = 0u;
    unsigned int doorRattleCount = 0u;

    memset(&media, 0, sizeof(media));
    memset(&swshStep, 0, sizeof(swshStep));
    memset(&doorStep, 0, sizeof(doorStep));
    memset(&finalDoorStep, 0, sizeof(finalDoorStep));
    memset(&switchStep, 0, sizeof(switchStep));
    memset(&preOpenStep, 0, sizeof(preOpenStep));
    memset(&command, 0, sizeof(command));

    expect_i("DM1 startup source media receipt builds",
             dm1_v1_startup_full_graphics_media_receipt_pc34("dm1", &media),
             1);
    expect_truth("startup receipt keeps all original startup surfaces",
                 media.handled && media.play_swsh && media.play_title &&
                     media.play_entrance);
    expect_u("startup SWSH uses source VBlank cadence",
             media.swsh_vblank_ms,
             SWSH_COMPAT_RUNTIME_VBLANK_MS);
    expect_u("startup SWSH initial hold comes from source helper",
             media.swsh_initial_logo_hold_ms,
             SWSH_Compat_GetRuntimeInitialLogoHoldMs());
    expect_u("startup SWSH palette wait comes from source helper",
             media.swsh_palette_wait_ms,
             SWSH_Compat_GetRuntimeDelayMsForVblankCount(
                 SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT));
    expect_u("startup SWSH sound wait comes from source helper",
             media.swsh_sound_wait_ms,
             SWSH_Compat_GetRuntimeDelayMsForVblankCount(
                 SWSH_COMPAT_SOURCE_SOUND_WAIT_VBLANK_COUNT));
    expect_u("startup SWSH final hold comes from source helper",
             media.swsh_final_hold_ms,
             SWSH_Compat_GetRuntimeFinalHoldMs());
    for (sourceStep = 1u;
         sourceStep <= SWSH_Compat_GetSourceAnimationStepCount();
         ++sourceStep) {
        memset(&swshStep, 0, sizeof(swshStep));
        expect_i("startup SWSH source event resolves",
                 SWSH_Compat_GetSourceAnimationStep(sourceStep, &swshStep),
                 1);
        if (swshStep.kind == SWSH_COMPAT_SOURCE_EVENT_SET_PALETTE_COLOR) {
            ++swshColorSetCount;
        } else if (swshStep.kind == SWSH_COMPAT_SOURCE_EVENT_WAIT_VBLANKS) {
            ++swshWaitCount;
            swshWaitVblankCount += swshStep.vblankCount;
        }
    }
    expect_u("startup SWSH executes every original palette write",
             swshColorSetCount,
             SWSH_COMPAT_SOURCE_PALETTE_COLOR_SET_COUNT);
    expect_u("startup SWSH executes every original VBlank wait",
             swshWaitCount,
             SWSH_COMPAT_SOURCE_PALETTE_WAIT_COMMAND_COUNT);
    expect_u("startup SWSH preserves cumulative VBlank wait count",
             swshWaitVblankCount,
             SWSH_COMPAT_SOURCE_PALETTE_WAIT_VBLANK_COUNT);
    expect_u("startup TITLE has 23 source events",
             media.title_source_animation_steps,
             dm1_v1_startup_title_source_animation_steps_pc34());
    expect_u("startup TITLE uses 18 source zoom steps",
             media.title_zoom_step_count,
             18u);
    expect_u("startup TITLE has no synthetic PRESENTS hold",
             media.title_presents_hold_ms,
             0u);
    expect_u("startup TITLE uses 20ms VBlank zoom cadence",
             media.title_zoom_frame_delay_ms,
             dm1_v1_startup_title_vblank_tick_ms_pc34());
    expect_u("startup TITLE retains only source post-zoom guard",
             media.title_post_zoom_guard_ms,
             dm1_v1_startup_title_post_zoom_vblanks_pc34() *
                 dm1_v1_startup_title_vblank_tick_ms_pc34() +
                 dm1_v1_startup_title_final_guard_vblanks_pc34() *
                     dm1_v1_startup_title_vblank_tick_ms_pc34());
    expect_u("startup TITLE has no fallback cadence padding",
             media.title_c001_cadence_pad_ms,
             0u);
    expect_u("startup TITLE enters menu after final source event",
             media.title_menu_boundary_frame,
             dm1_v1_startup_title_source_animation_steps_pc34() + 1u);
    expect_i("startup TITLE palette transition is C12 then C13/C14",
             media.title_presents_palette ==
                     VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS &&
                 media.title_zoom_palette == VGA_PALETTE_PC34_SPECIAL_TITLE,
             1);
    expect_i("startup TITLE receipt is safe for palette/timing consumption",
             dm1_v1_startup_title_timing_receipt_valid_pc34(&media),
             1);
    {
        DM1_V1_StartupFullGraphicsMediaReceipt_PC34 stale = media;
        stale.title_presents_palette = VGA_PALETTE_PC34_SPECIAL_CREDITS;
        expect_i("stale handled receipt cannot substitute a non-C12 PRESENTS palette",
                 dm1_v1_startup_title_timing_receipt_valid_pc34(&stale),
                 0);
        stale = media;
        stale.title_zoom_frame_delay_ms++;
        expect_i("stale handled receipt cannot alter TITLE.C VBlank cadence",
                 dm1_v1_startup_title_timing_receipt_valid_pc34(&stale),
                 0);
        stale = media;
        stale.title_zoom_palette = VGA_PALETTE_PC34_SPECIAL_TITLE_PRESENTS;
        expect_i("stale handled receipt cannot collapse C12 and C13/C14 phases",
                 dm1_v1_startup_title_timing_receipt_valid_pc34(&stale),
                 0);
    }
    expect_i("startup entrance has no interactive auto-advance",
             media.entrance_auto_enter_ms,
             0);
    expect_truth("startup entrance receipt is source-timed",
                 dm1_v1_startup_entrance_timing_receipt_valid_pc34(&media));
    expect_i("startup entrance switch source step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(5u, &switchStep),
             1);
    expect_i("startup entrance executes source switch sound",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media, switchStep.sourceStepOrdinal, (int)switchStep.kind,
                 switchStep.delayTicks, switchStep.vblankLoopCount, &command),
             1);
    expect_truth("startup entrance switch is C01 at source volume 112",
                 command.render_kind ==
                         DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34 &&
                     command.audio_request_ready && command.audio_sound_index == 1 &&
                     command.audio_volume == 112u && command.delay_ms == 0u);
    expect_i("startup entrance pre-open source step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(6u, &preOpenStep),
             1);
    expect_i("startup entrance executes source pre-open delay",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media, preOpenStep.sourceStepOrdinal, (int)preOpenStep.kind,
                 preOpenStep.delayTicks, preOpenStep.vblankLoopCount, &command),
             1);
    expect_truth("startup entrance pre-open delay follows switch without audio",
                 command.render_kind ==
                         DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34 &&
                     !command.audio_request_ready && command.delay_ms ==
                         media.entrance_pre_open_delay_ms);
    expect_i("startup entrance first door source step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(7u, &doorStep),
             1);
    expect_i("startup entrance final door source step exists",
             ENTRANCE_Compat_GetSourceAnimationStep(
                 6u + ENTRANCE_Compat_GetDoorAnimationStepCount(),
                 &finalDoorStep),
             1);
    expect_i("startup entrance executes canonical first door step",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media, doorStep.sourceStepOrdinal, (int)doorStep.kind,
                 doorStep.delayTicks, doorStep.vblankLoopCount, &command),
             1);
    expect_u("startup entrance door command keeps source VBlank delay",
             command.delay_ms,
             media.entrance_vblank_ms);
    expect_i("startup entrance executes final source door step",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media, finalDoorStep.sourceStepOrdinal,
                 (int)finalDoorStep.kind, finalDoorStep.delayTicks,
                 finalDoorStep.vblankLoopCount, &command),
             1);
    expect_truth("startup entrance final door step is source step 31 with rattle",
                 command.door_animation_step ==
                     ENTRANCE_Compat_GetDoorAnimationStepCount() &&
                     command.play_door_rattle_sound &&
                     command.audio_request_ready && command.audio_sound_index == 2 &&
                     command.delay_ms == media.entrance_vblank_ms);
    for (sourceStep = 7u;
         sourceStep < 7u + ENTRANCE_Compat_GetDoorAnimationStepCount();
         ++sourceStep) {
        EntranceCompatSourceAnimationStep scheduledDoor;
        unsigned int expectedDoorStep = sourceStep - 6u;

        memset(&scheduledDoor, 0, sizeof(scheduledDoor));
        expect_i("startup entrance scheduled door event resolves",
                 ENTRANCE_Compat_GetSourceAnimationStep(sourceStep,
                                                         &scheduledDoor),
                 1);
        expect_i("startup entrance executes scheduled door event",
                 dm1_v1_startup_entrance_render_audio_command_pc34(
                     &media, scheduledDoor.sourceStepOrdinal,
                     (int)scheduledDoor.kind, scheduledDoor.delayTicks,
                     scheduledDoor.vblankLoopCount, &command),
                 1);
        expect_truth("startup entrance door command keeps source step and VBlank",
                     command.render_kind ==
                             DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34 &&
                         command.door_animation_step == expectedDoorStep &&
                         command.delay_ms == media.entrance_vblank_ms);
        if (expectedDoorStep <= 26u) {
            expect_truth("startup entrance left strip follows F0438 signed bound",
                         command.door_left_box_x == 0u &&
                             command.door_left_box_w ==
                                 101u - 4u * (expectedDoorStep - 1u) &&
                             command.door_left_box_h == 161u);
        } else {
            expect_truth("startup entrance left strip stops after step 26",
                         command.door_left_box_w == 0u &&
                             command.door_left_box_h == 0u);
        }
        expect_truth("startup entrance right strip follows F0438 through step 31",
                     command.door_right_box_x ==
                             109u + 4u * (expectedDoorStep - 1u) &&
                         command.door_right_box_w ==
                             123u - 4u * (expectedDoorStep - 1u) &&
                         command.door_right_box_h == 161u);
        ++doorCommandCount;
        if (command.play_door_rattle_sound) {
            ++doorRattleCount;
            expect_truth("startup entrance rattle command is source-owned",
                         command.audio_request_ready &&
                             command.audio_sound_index == 2 &&
                             command.audio_volume == 145u);
        }
    }
    expect_u("startup entrance executes all 31 original door steps",
             doorCommandCount,
             ENTRANCE_Compat_GetDoorAnimationStepCount());
    expect_u("startup entrance preserves 11 original door rattles",
             doorRattleCount,
             11u);
    expect_i("startup entrance rejects synthetic zero-delay door step",
             dm1_v1_startup_entrance_render_audio_command_pc34(
                 &media, doorStep.sourceStepOrdinal, (int)doorStep.kind,
                 doorStep.delayTicks, 0u, &command),
             0);
}

static void check_entrance_credits_runtime_boundary(void) {
    DM1_V1_StartupHandoffOutcome_PC34 outcome;

    memset(&outcome, 0, sizeof(outcome));
    /* ReDMCSB ENTRANCE.C F0442:993 and :1067-1091 display credits with
     * L1406=1800 VBlanks, then restore C202 so F0441 redraws the entrance.
     * Credits is an entrance-local loop, never a game/runtime handoff. */
    expect_u("entrance credits preserves F0442 1800-VBlank duration",
             ENTRANCE_Compat_GetCreditsWaitTicks(),
             1800u);
    expect_i("entrance credits source command stays on credits route",
             (int)ENTRANCE_Compat_CommandPathFromSourceCommand(
                 ENTRANCE_COMPAT_RUNTIME_COMMAND_DRAW_CREDITS),
             (int)ENTRANCE_COMPAT_COMMAND_PATH_CREDITS);
    expect_i("startup outcome accepts credits boundary receipt",
             dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
                 ENTRANCE_COMPAT_COMMAND_PATH_CREDITS, &outcome),
             1);
    expect_truth("credits outcome remains an explicit entrance loop",
                 outcome.action == DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 &&
                     outcome.status &&
                     strcmp(outcome.status, "DM1 ENTRANCE CREDITS LOOP") == 0);
    expect_truth("credits cannot become dungeon, resume, or quit handoff",
                 outcome.action != DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34 &&
                     outcome.action != DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34 &&
                     outcome.action != DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34);
}

static unsigned int count_non_black(const unsigned char* pixels,
                                    unsigned int width,
                                    unsigned int x,
                                    unsigned int y,
                                    unsigned int region_width,
                                    unsigned int region_height) {
    unsigned int count = 0u;
    unsigned int row;
    unsigned int col;

    for (row = 0u; row < region_height; ++row) {
        for (col = 0u; col < region_width; ++col) {
            if (pixels[(y + row) * width + x + col] != 0u) {
                ++count;
            }
        }
    }
    return count;
}

static void check_real_pc34_c001(const char* graphics_path) {
    M11_AssetLoader loader;
    const M11_AssetSlot* c001;
    const M11_AssetSlot* c005;
    unsigned char framebuffer[320u * 200u];
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_StartupEntranceCreditsPresentationCommand_PC34 credits_command;
    V1_TitleFrontendSourceAnimationStep first_zoom;
    V1_TitleFrontendSourceAnimationStep last_zoom;
    V1_TitleFrontendSourceAnimationStep strikes_back;
    V1_TitleFrontendC001BlitPlan plan;

    if (!graphics_path || !graphics_path[0]) {
        printf("SKIP real C001 fixture: pass hash-verified GRAPHICS.DAT path\n");
        return;
    }
    memset(&loader, 0, sizeof(loader));
    memset(&media, 0, sizeof(media));
    memset(&credits_command, 0, sizeof(credits_command));
    expect_i("real C001: loader opens supplied PC34 GRAPHICS.DAT",
             M11_AssetLoader_Init(&loader, graphics_path), 1);
    if (!M11_AssetLoader_IsReady(&loader)) {
        return;
    }
    c001 = M11_AssetLoader_Load(&loader, 1u);
    expect_truth("real C001: graphic 1 decodes", c001 != NULL);
    if (!c001) {
        M11_AssetLoader_Shutdown(&loader);
        return;
    }
    expect_u("real C001: width", c001->width, 320u);
    expect_u("real C001: height", c001->height, 200u);
    {
        DM1_V1_StartupTitleRuntimeAssetReceipt_PC34 receipt;
        memset(&receipt, 0, sizeof(receipt));
        expect_truth("real C001: source regions satisfy startup asset receipt",
                     dm1_v1_startup_title_runtime_asset_receipt_pc34(
                         "dm1", c001->pixels, c001->width, c001->height,
                         &receipt) && receipt.release_c001_ready);
    }
    expect_truth("real C001: PRESENTS source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 137u, 320u, 16u) > 0u);
    expect_truth("real C001: DUNGEON MASTER source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 0u, 320u, 80u) > 0u);
    expect_truth("real C001: MASTER STRIKES BACK source strip has original pixels",
                 count_non_black(c001->pixels, c001->width,
                                 0u, 80u, 320u, 57u) > 0u);

    c005 = M11_AssetLoader_Load(&loader, 5u);
    expect_truth("real C005: credits graphic decodes", c005 != NULL);
    if (c005) {
        expect_truth("real C005: original credits pixels satisfy DM1 receipt",
                     dm1_v1_startup_full_graphics_media_receipt_pc34(
                         "dm1", &media) &&
                     dm1_v1_startup_entrance_credits_presentation_command_pc34(
                         &media, c005->pixels, c005->width, c005->height,
                         &credits_command) &&
                     credits_command.present_credits_frame &&
                     credits_command.graphics_c005_pixel_fingerprint != 0u);
    }

    expect_i("real C001: first zoom step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(2u, &first_zoom), 1);
    expect_i("real C001: last zoom step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(19u, &last_zoom), 1);
    expect_i("real C001: strikes-back step resolves",
             V1_TitleFrontend_GetSourceAnimationStep(22u, &strikes_back), 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: first zoom plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&first_zoom, &plan), 1);
    M11_AssetLoader_BlitSubRectScaled(c001, framebuffer, 320, 200,
                                      (int)plan.dstX, (int)plan.dstY,
                                      (int)plan.dstW, (int)plan.dstH,
                                      (int)plan.srcX, (int)plan.srcY,
                                      (int)plan.srcW, (int)plan.srcH,
                                      plan.transparentColor);
    expect_truth("real C001: first source-ordered zoom blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: final zoom plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&last_zoom, &plan), 1);
    M11_AssetLoader_BlitSubRectScaled(c001, framebuffer, 320, 200,
                                      (int)plan.dstX, (int)plan.dstY,
                                      (int)plan.dstW, (int)plan.dstH,
                                      (int)plan.srcX, (int)plan.srcY,
                                      (int)plan.srcW, (int)plan.srcH,
                                      plan.transparentColor);
    expect_truth("real C001: final source-ordered zoom blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_i("real C001: strikes-back plan resolves",
             V1_TitleFrontend_GetC001BlitPlanForStep(&strikes_back, &plan), 1);
    M11_AssetLoader_BlitRegion(c001, (int)plan.srcX, (int)plan.srcY,
                               (int)plan.srcW, (int)plan.srcH,
                               framebuffer, 320, 200,
                               (int)plan.dstX, (int)plan.dstY,
                               plan.transparentColor);
    expect_truth("real C001: strikes-back source blit is visible",
                 count_non_black(framebuffer, 320u, plan.dstX, plan.dstY,
                                 plan.dstW, plan.dstH) > 0u);
    M11_AssetLoader_Shutdown(&loader);
}

int main(int argc, char** argv) {
    check_selection_contract();
    check_runtime_asset_receipt();
    check_title_presentation_command();
    check_entrance_credits_presentation_command();
    check_palette_cross_source_contract();
    check_startup_source_timing_contract();
    check_entrance_credits_runtime_boundary();
    check_real_pc34_c001(argc > 1 ? argv[1] : getenv("FIRESTAFF_DM1_GRAPHICS_DAT"));

    if (g_fail) {
        printf("summary=%d passed %d failed\n", g_pass, g_fail);
        return 1;
    }
    printf("ok: DM1 V1 TITLE C001 fallback gate passed (%d checks)\n", g_pass);
    return 0;
}
