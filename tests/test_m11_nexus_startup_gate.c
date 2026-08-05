/*
 * test_m11_nexus_startup_gate.c -- M11 Nexus startup ownership gate.
 *
 * Scope: startup handoff only. Empty or partial Nexus data must not leave
 * M11 in an active half-started state; a real extracted/ISO data directory,
 * when staged locally, must reach M11_GAME_SOURCE_NEXUS_DGN.
 *
 * Source: src/nexus/nexus_v1_launcher.c owns init/load-level sequencing;
 * src/engine/m11_game_view.c M11_GameView_StartNexus owns the M11 handoff.
 */

#include "m11_game_view.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_save.h"
#include "nexus_v1_startup_layout.h"
#include "nexus_v1_title.h"
#include "nexus_v1_title_sequence.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <stdlib.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;
/* Keep the raw-capture receipt out of main's already large fixture frame. */
static Nexus_V1_DgnStructure3RawCaptureHostReceipt g_raw_capture_receipt;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static void wr16_be(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v >> 8);
    p[1] = (unsigned char)v;
}

static void wr32_be(unsigned char* p, unsigned int v) {
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)v;
}

static int make_temp_root(char root[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_nexus_m11_startup_%lu",
             (unsigned long)rand());
    return TEST_MKDIR(root) == 0;
#else
    char tmpl[] = "/tmp/firestaff_nexus_m11_startup_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
    return 1;
#endif
}

static int write_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    size_t n = bytes ? strlen(bytes) : 0u;
    int ok;
    if (!f) {
        return 0;
    }
    ok = n == 0u || fwrite(bytes, 1u, n, f) == n;
    fclose(f);
    return ok;
}

static int count_nonzero_pixels(const unsigned char* pixels, size_t count) {
    size_t i;
    int nonzero = 0;
    if (!pixels) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0u) {
            ++nonzero;
        }
    }
    return nonzero;
}

static int advance_nexus_title_to_frame(M11_GameViewState* view,
                                        unsigned int target_frame,
                                        int max_steps) {
    int step;
    int target = (int)target_frame;
    if (!view) {
        return 0;
    }
    for (step = 0;
         step < max_steps && view->nexusState.title_frame < target;
         ++step) {
        (void)M11_GameView_AdvanceIdleTick(view);
    }
    return view->nexusState.title_frame >= target;
}

static int count_nonzero_region(const unsigned char* pixels,
                                int fb_w,
                                int fb_h,
                                int x,
                                int y,
                                int w,
                                int h) {
    int xx;
    int yy;
    int nonzero = 0;
    if (!pixels || fb_w <= 0 || fb_h <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (yy = 0; yy < h; ++yy) {
        int py = y + yy;
        if (py < 0 || py >= fb_h) continue;
        for (xx = 0; xx < w; ++xx) {
            int px = x + xx;
            if (px < 0 || px >= fb_w) continue;
            if (pixels[py * fb_w + px] != 0u) {
                ++nonzero;
            }
        }
    }
    return nonzero;
}

static int count_diff_pixels(const unsigned char* a,
                             const unsigned char* b,
                             size_t count) {
    size_t i;
    int diff = 0;
    if (!a || !b) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            ++diff;
        }
    }
    return diff;
}

static void expect_title_render_is_frame_dependent(void) {
    Nexus_TitleScreen title;
    Nexus_Framebuffer frame0;
    Nexus_Framebuffer frame16;
    unsigned char pixels[NEXUS_FB_W * NEXUS_FB_H];
    int i;

    for (i = 0; i < (int)sizeof(pixels); ++i) {
        pixels[i] = (unsigned char)((i % 251) + 1);
    }
    memset(&title, 0, sizeof(title));
    title.pixels = pixels;
    title.width = NEXUS_FB_W;
    title.height = NEXUS_FB_H;
    title.loaded = 1;
    nexus_fb_init(&frame0);
    nexus_fb_init(&frame16);
    nexus_render_title(&title, &frame0, 0);
    nexus_render_title(&title, &frame16, 16);
    expect_true(count_nonzero_pixels(frame0.color_buffer,
                                     sizeof(frame0.color_buffer)) > 500,
                "Nexus title reveal frame 0 remains visible");
    expect_true(count_diff_pixels(frame0.color_buffer,
                                  frame16.color_buffer,
                                  sizeof(frame0.color_buffer)) > 500,
                "Nexus title render changes across startup frames");
}

static void expect_title_sequence_contract(void) {
    Nexus_V1_TitleFrame frame0;
    Nexus_V1_TitleFrame frame16;
    Nexus_V1_TitleFrame frame30;
    Nexus_V1_TitleFrame frame54;
    Nexus_V1_BootFrame boot0;
    Nexus_V1_BootFrame boot48;
    Nexus_V1_BootFrame boot102;

    expect_true(nexus_v1_title_min_boot_frames() == 30,
                "Nexus title sequence owns the minimum boot reveal frame");
    expect_true(nexus_v1_title_start_ready_frames() == 54,
                "Nexus title sequence owns the start-ready hold frame");
    expect_true(nexus_v1_title_frame(0, NEXUS_FB_H, &frame0) &&
                    frame0.phase == NEXUS_V1_TITLE_PHASE_BOOT_REVEAL &&
                    frame0.frame_in_phase == 0 &&
                    frame0.frames_until_ready == 54 &&
                    frame0.reveal_h == 80 &&
                    frame0.reveal_y0 == (NEXUS_FB_H - 80) / 2 &&
                    frame0.reveal_y1 == (NEXUS_FB_H - 80) / 2 + 80 &&
                    frame0.edge_color == 0 &&
                    !frame0.boot_reveal_complete,
                "Nexus title sequence frame 0 reveal contract is stable");
    expect_true(nexus_v1_title_frame(16, NEXUS_FB_H, &frame16) &&
                    frame16.phase == NEXUS_V1_TITLE_PHASE_BOOT_REVEAL &&
                    frame16.frame_in_phase == 16 &&
                    frame16.frames_until_ready == 38 &&
                    frame16.reveal_h == 144 &&
                    frame16.reveal_y0 == (NEXUS_FB_H - 144) / 2 &&
                    frame16.reveal_y1 == (NEXUS_FB_H - 144) / 2 + 144 &&
                    frame16.edge_color == 0 &&
                    !frame16.boot_reveal_complete,
                "Nexus title sequence frame 16 reveal contract is stable");
    expect_true(nexus_v1_title_frame(30, NEXUS_FB_H, &frame30) &&
                    frame30.phase == NEXUS_V1_TITLE_PHASE_HOLD &&
                    frame30.frame_in_phase == 0 &&
                    frame30.frames_until_ready == 24 &&
                    frame30.reveal_h == 200 &&
                    frame30.reveal_y0 == (NEXUS_FB_H - 200) / 2 &&
                    frame30.reveal_y1 == (NEXUS_FB_H - 200) / 2 + 200 &&
                    frame30.boot_reveal_complete &&
                    frame30.hold_frame == 0 &&
                    !frame30.start_ready &&
                    frame30.prompt_visible,
                "Nexus title sequence reaches full reveal at boot gate");
    expect_true(nexus_v1_title_frame(54, NEXUS_FB_H, &frame54) &&
                    frame54.phase == NEXUS_V1_TITLE_PHASE_START_READY &&
                    frame54.frame_in_phase == 0 &&
                    frame54.frames_until_ready == 0 &&
                    frame54.reveal_h == NEXUS_FB_H &&
                    frame54.boot_reveal_complete &&
                    frame54.hold_frame == 24 &&
                    frame54.start_ready,
                "Nexus title sequence reaches start-ready after title hold");
    expect_true(strcmp(nexus_v1_title_phase_name(frame0.phase),
                       "BOOT_REVEAL") == 0 &&
                    strcmp(nexus_v1_title_phase_name(frame30.phase),
                           "HOLD") == 0 &&
                    strcmp(nexus_v1_title_phase_name(frame54.phase),
                           "START_READY") == 0,
                "Nexus title sequence exposes stable boot phase names");
    expect_true(nexus_title_min_boot_frames() ==
                    nexus_v1_title_min_boot_frames() &&
                    !nexus_title_boot_reveal_complete(29) &&
                    nexus_title_boot_reveal_complete(30) &&
                    nexus_title_start_ready_frames() ==
                        nexus_v1_title_start_ready_frames() &&
                    !nexus_title_start_ready(53) &&
                    nexus_title_start_ready(54),
                "legacy Nexus title API delegates reveal and start gates");
    expect_true(nexus_v1_boot_warning_frames() == 48 &&
                    nexus_v1_boot_start_ready_frames() == 102 &&
                    nexus_v1_boot_frame(0, NEXUS_FB_H, &boot0) &&
                    boot0.phase == NEXUS_V1_BOOT_PHASE_WARNING &&
                    boot0.warning_visible &&
                    !boot0.start_ready,
                "Nexus full boot starts with a WARNING.BIN phase");
    expect_true(nexus_v1_boot_frame(48, NEXUS_FB_H, &boot48) &&
                    boot48.phase == NEXUS_V1_BOOT_PHASE_TITLE &&
                    boot48.title_frame == 0 &&
                    boot48.title.phase == NEXUS_V1_TITLE_PHASE_BOOT_REVEAL,
                "Nexus full boot hands WARNING to TITLE.CG reveal");
    expect_true(nexus_v1_boot_frame(102, NEXUS_FB_H, &boot102) &&
                    boot102.phase == NEXUS_V1_BOOT_PHASE_TITLE &&
                    boot102.title_frame == 54 &&
                    boot102.start_ready &&
                    nexus_title_boot_start_ready_frames() ==
                        nexus_v1_boot_start_ready_frames() &&
                    !nexus_title_full_boot_start_ready(101) &&
                    nexus_title_full_boot_start_ready(102),
                "Nexus full boot start gate waits for warning plus title hold");
}

static void expect_startup_layout_contract(void) {
    Nexus_V1_StartupRect rect;
    Nexus_V1_StartupHit hit;

    expect_true(nexus_v1_startup_save_row_rect(0, &rect) &&
                    rect.x == 18 && rect.y == 42 &&
                    rect.w == 284 && rect.h == 12,
                "Nexus startup save row 0 layout is owned by Nexus module");
    expect_true(nexus_v1_startup_save_row_rect(1, &rect) &&
                    rect.y == 55,
                "Nexus startup save rows keep 13px cadence");
    expect_true(nexus_v1_startup_save_hit(2, 24, 43, &hit) &&
                    hit.kind == NEXUS_V1_STARTUP_HIT_SAVE_ROW &&
                    hit.row == 0,
                "Nexus startup save hit resolves row 0");
    expect_true(nexus_v1_startup_save_hit(2, 24, 20, &hit) &&
                    hit.kind == NEXUS_V1_STARTUP_HIT_SAVE_PANEL,
                "Nexus startup save panel consumes whitespace");
    expect_true(!nexus_v1_startup_save_hit(2, 4, 4, &hit),
                "Nexus startup save hit rejects outside point");

    expect_true(nexus_v1_startup_champion_row_rect(0, &rect) &&
                    rect.x == 18 && rect.y == 37 &&
                    rect.w == 284 && rect.h == 11,
                "Nexus startup champion row 0 layout is owned by Nexus module");
    expect_true(nexus_v1_startup_champion_row_rect(1, &rect) &&
                    rect.y == 48,
                "Nexus startup champion rows keep 11px cadence");
    expect_true(nexus_v1_startup_champion_hit(12, 24, 38, &hit) &&
                    hit.kind == NEXUS_V1_STARTUP_HIT_CHAMPION_ROW &&
                    hit.row == 0,
                "Nexus startup champion hit resolves row 0");
    expect_true(nexus_v1_startup_champion_hit(12, 24, 184, &hit) &&
                    hit.kind == NEXUS_V1_STARTUP_HIT_CHAMPION_FOOTER,
                "Nexus startup champion footer resolves action hit");
    expect_true(nexus_v1_startup_champion_hit(12, 24, 24, &hit) &&
                    hit.kind == NEXUS_V1_STARTUP_HIT_CHAMPION_PANEL,
                "Nexus startup champion panel consumes whitespace");
}

static void expect_champion_startup_selection_contract(void) {
    Nexus_V1_ChampionPool pool;
    int next_cursor = -1;

    nexus_v1_champions_init(&pool);
    expect_true(pool.champion_count == NEXUS_MAX_CHAMPIONS,
                "Nexus startup champion fixture exposes all 24 mirror rows");
    expect_true(!nexus_v1_champion_in_party(&pool, 0),
                "Nexus startup champion starts outside party");
    expect_true(nexus_v1_champion_next_selectable(&pool, 0, 1) == 0,
                "Nexus startup next selectable returns first open champion");
    expect_true(nexus_v1_champion_recruit_and_advance(
                    &pool,
                    0,
                    &next_cursor) >= 0,
                "Nexus startup recruit helper adds champion");
    expect_true(pool.party_count == 1 &&
                    nexus_v1_champion_in_party(&pool, 0),
                "Nexus startup recruit helper updates party membership");
    expect_true(next_cursor == 1,
                "Nexus startup recruit helper advances to next open champion");
    expect_true(nexus_v1_champion_recruit_and_advance(
                    &pool,
                    0,
                    &next_cursor) < 0,
                "Nexus startup recruit helper rejects duplicate champion");
    expect_true(nexus_v1_champion_next_selectable(&pool, 0, 1) == 1,
                "Nexus startup next selectable skips recruited champion");
    expect_true(nexus_v1_champion_unrecruit_last(&pool) == 0 &&
                    pool.party_count == 0,
                "Nexus startup back helper can remove last recruit");
}

static void fill_nexus_spec(M11_GameLaunchSpec* spec, const char* data_dir) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "DUNGEON MASTER NEXUS";
    spec->gameId = "nexus";
    spec->sourceId = "nexus";
    spec->dataDir = data_dir;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static int set_test_home(const char* home) {
#ifdef _WIN32
    return _putenv_s("HOME", home ? home : "") == 0;
#else
    return home ? setenv("HOME", home, 1) == 0 : unsetenv("HOME") == 0;
#endif
}

static void cleanup_nexus_default_save_dir(const char* home) {
    char save_dir[512];
    char path[512];
    int i;
    char old_home[512];
    const char* old_home_env = getenv("HOME");

    if (!home || !home[0]) {
        return;
    }
    snprintf(old_home, sizeof(old_home), "%s",
             old_home_env ? old_home_env : "");
    (void)set_test_home(home);
    nexus_v1_save_default_dir(save_dir, sizeof(save_dir));
    for (i = 0; i < NEXUS_SAVE_MAX_SLOTS; ++i) {
        snprintf(path, sizeof(path), "%s%snexus_save_%02d.dat",
                 save_dir, TEST_PATH_SEP, i);
        remove(path);
    }
    (void)TEST_RMDIR(save_dir);
#ifndef _WIN32
    snprintf(path, sizeof(path), "%s/Library/Application Support/Firestaff/nexus",
             home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library/Application Support/Firestaff",
             home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library/Application Support", home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library", home);
    (void)TEST_RMDIR(path);
#endif
    if (old_home[0]) {
        (void)set_test_home(old_home);
    } else {
        (void)set_test_home(NULL);
    }
}

static void expect_face_loader_counts_real_vs_fallback(void) {
    Nexus_UI_Manager ui;
    unsigned char face_bytes[48 * 48];
    unsigned char compact_face[4096];
    unsigned char short_bytes[16];
    Nexus_UI_FaceLayout layout;
    Nexus_UI_FaceRecordDecodeInfo decode_info;
    Nexus_UI_FaceCompactRecordDescriptor compact_last;
    Nexus_UI_FacePrs3CorpusReceipt corpus;
    Nexus_UI_FacePrs3CaptureTarget capture_target;
    Nexus_UI_FacePrs3CaptureCampaignReceipt capture_campaign;
    unsigned char expanded_face[48 * 48];
    int compact_size;
    int compact_cursor;
    int i;

    for (i = 0; i < (int)sizeof(face_bytes); ++i) {
        face_bytes[i] = (unsigned char)((i % 31) + 1);
    }
    memset(compact_face, 0, sizeof(compact_face));
    memcpy(compact_face, "FACE", 4);
    compact_face[9] = 20;
    compact_cursor = 56;
    for (i = 0; i < 20; ++i) {
        int prs3 = (compact_cursor + 128 + 3) & ~3;
        memcpy(compact_face + prs3, "PRS3", 4);
        compact_face[prs3 + 7] = 1;
        compact_face[prs3 + 10] = 0x0c;
        compact_face[prs3 + 11] = 0x40;
        compact_face[prs3 + 15] = 1;
        compact_face[prs3 + 16] = (unsigned char)(i + 1);
        compact_cursor = prs3 + 17;
    }
    compact_face[compact_cursor] = 0;
    compact_face[compact_cursor + 1] = 0;
    compact_size = compact_cursor + 2;
    compact_face[4] = (unsigned char)(compact_size >> 24);
    compact_face[5] = (unsigned char)(compact_size >> 16);
    compact_face[6] = (unsigned char)(compact_size >> 8);
    compact_face[7] = (unsigned char)compact_size;
    memset(short_bytes, 3, sizeof(short_bytes));
    nexus_ui_manager_init(&ui);
    expect_true(nexus_ui_face_full_entry_count((int)sizeof(face_bytes),
                                               48,
                                               48) == 0,
                "Nexus FACE loader rejects raw 48x48 portrait tables");
    expect_true(nexus_ui_face_full_entry_count((48 * 48 * 19) + 1328,
                                               48,
                                               48) == 0,
                "Nexus FACE loader rejects partial raw portrait tables");
    expect_true(nexus_ui_face_full_entry_count(48 * 48 * 30,
                                               48,
                                               48) == 0,
                "Nexus FACE loader rejects raw portrait roster tables");
    memset(&layout, 0, sizeof(layout));
    expect_true(nexus_ui_face_layout_detect(compact_face,
                                            compact_size,
                                            &layout) &&
                    layout.valid &&
                    layout.header_size == 56 &&
                    layout.entry_count == 20 && layout.entry_size == 0 &&
                    layout.portrait_w == 56 && layout.portrait_h == 56,
                "Nexus FACE loader detects canonical variable PRS3 frame layout");
    expect_true(nexus_ui_face_compact_record_descriptor(
                    compact_face, compact_size, 19, &compact_last) &&
                    compact_last.valid && compact_last.prefix_size >= 128 &&
                    compact_last.prefix_size <= 131 &&
                    compact_last.prs3_size == 17 && compact_last.stream_size == 1 &&
                    compact_last.prs3_version == 1U &&
                    compact_last.declared_pixel_count == 56 * 56,
                "Nexus FACE descriptor preserves compact version-1 frame boundaries");
    memset(&corpus, 0, sizeof(corpus));
    expect_true(nexus_ui_face_prs3_corpus_receipt(compact_face, compact_size,
                                                   &corpus) == 1 &&
                    corpus.valid && corpus.frame_count == 20 &&
                    corpus.source_byte_count == (size_t)compact_size &&
                    corpus.total_stream_byte_count == 20U &&
                    corpus.declared_total_pixel_count == 20U * 56U * 56U &&
                    !corpus.decoder_permitted && corpus.no_draw_only &&
                    !corpus.fallback_visuals_permitted,
                "Nexus FACE PRS3 corpus retains all authenticated frame boundaries without decoding");
    expect_true(nexus_ui_face_prs3_capture_target(
                    compact_face, compact_size, 19, 1, &capture_target) == 1 &&
                    capture_target.valid && capture_target.face_index == 19 &&
                    capture_target.descriptor.prs3_offset == compact_last.prs3_offset &&
                    capture_target.prefix_bytes_fnv1a64 != 0U &&
                    capture_target.prs3_header_fnv1a64 != 0U &&
                    capture_target.stream_bytes_fnv1a64 != 0U &&
                    capture_target.capture_producer_required &&
                    capture_target.original_saturn_capture_required &&
                    !capture_target.decoder_permitted && capture_target.no_draw_only &&
                    !capture_target.fallback_visuals_permitted,
                "Nexus FACE frame target retains separate source lanes without decoding");
    expect_true(nexus_ui_face_prs3_capture_target(
                    compact_face, compact_size, 19, 0, &capture_target) == 0 &&
                    !capture_target.valid && capture_target.no_draw_only,
                "Nexus FACE frame target requires a caller-owned source hash gate");
    expect_true(nexus_ui_face_prs3_capture_campaign(
                    compact_face, compact_size, 1, &capture_campaign) == 1 &&
                    capture_campaign.valid && capture_campaign.frame_count == 20 &&
                    capture_campaign.source_byte_count == (size_t)compact_size &&
                    capture_campaign.total_stream_byte_count == 20U &&
                    capture_campaign.ordered_target_fnv1a64 != 0U &&
                    capture_campaign.source_lanes_fnv1a64 != 0U &&
                    capture_campaign.capture_producer_required &&
                    capture_campaign.original_saturn_capture_required &&
                    !capture_campaign.decoder_permitted &&
                    capture_campaign.no_draw_only &&
                    !capture_campaign.fallback_visuals_permitted,
                "Nexus FACE campaign locks every source frame without decoding");
    memset(&decode_info, 0, sizeof(decode_info));
    memset(expanded_face, 0xaa, sizeof(expanded_face));
    expect_true(nexus_ui_expand_face_record_48x48(compact_face + compact_last.prs3_offset,
                                                  17,
                                                  expanded_face,
                                                  (int)sizeof(expanded_face),
                                                  &decode_info) == 0 &&
                    decode_info.kind == NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN &&
                    decode_info.source_size == 17 &&
                    decode_info.copied_pixels == 0 &&
                    expanded_face[0] == 0xaa,
                "Nexus FACE PRS3 frame is recognized but cannot become a partial portrait");
    expect_true(nexus_ui_load_face_record(&ui,
                                          compact_face + compact_last.prs3_offset,
                                          17,
                                          23,
                                          48,
                                          48,
                                          NULL) < 0 &&
                    ui.surfaces[NEXUS_SURFACE_FACE19].data == NULL,
                "Nexus FACE PRS3 frame leaves no synthetic portrait surface");
    memset(&decode_info, 0, sizeof(decode_info));
    expect_true(nexus_ui_expand_face_record_48x48(face_bytes,
                                                  (int)sizeof(face_bytes),
                                                  expanded_face,
                                                  (int)sizeof(expanded_face),
                                                  &decode_info) == 0 &&
                    decode_info.kind == NEXUS_UI_FACE_RECORD_NONE &&
                    decode_info.copied_pixels == 0,
                "Nexus FACE raw record cannot become a portrait surface");
    expect_true(nexus_ui_load_faces(&ui,
                                    face_bytes,
                                    0,
                                    (int)sizeof(face_bytes),
                                    0,
                                    48,
                                    48,
                                    NULL) < 0,
                "Nexus FACE loader rejects raw pixels without DMWeb palette+PRS3 framing");
    expect_true(nexus_ui_load_faces(&ui,
                                    short_bytes,
                                    0,
                                    (int)sizeof(short_bytes),
                                    1,
                                    48,
                                    48,
                                    NULL) < 0,
                "Nexus FACE loader rejects a truncated original portrait without a placeholder");
    nexus_ui_manager_free(&ui);
}

static void make_bpk_surface_archive(unsigned char* data,
                                     size_t size,
                                     int prs3)
{
    const unsigned int trailer_off = 48U;
    const unsigned int surface_off = 80U;
    unsigned int i;
    memset(data, 0, size);
    memcpy(data, "BPPK", 4);
    wr32_be(data + 4, (unsigned int)size);
    memcpy(data + 12, "BMPD", 4);
    wr32_be(data + 16, (unsigned int)size - 20U);
    wr32_be(data + 20, 2U);
    wr32_be(data + 24, trailer_off);
    wr32_be(data + 28, surface_off);
    data[trailer_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_TRAILER;

    wr16_be(data + surface_off + NEXUS_V1_BPK_PREFIX_WIDTH_OFFSET, 4U);
    data[surface_off + NEXUS_V1_BPK_PREFIX_HEIGHT_OFFSET] = 4U;
    data[surface_off + NEXUS_V1_BPK_PREFIX_MODE_OFFSET] =
        NEXUS_V1_BPK_MODE_8BPP;
    if (prs3) {
        memcpy(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES,
               "PRS3",
               4);
        wr32_be(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 4U,
                1U);
        wr32_be(data + surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + 8U,
                16U);
    } else {
        for (i = 0; i < 16U; ++i) {
            data[surface_off + NEXUS_V1_BPK_ENTRY_PREFIX_BYTES + i] =
                (unsigned char)(0x30U + i);
        }
    }
}

static void expect_bpk_runtime_surface_import(void) {
    Nexus_UI_Manager ui;
    unsigned char archive[128];
    Nexus_V1_BpkRuntimeSurfaceHandoff rows[2];
    Nexus_V1_BpkRuntimeSurfaceHandoffSummary summary;
    Nexus_UI_BpkImportReceipt receipt;
    const Nexus_UI_Surface* surface;
    int rc;

    nexus_ui_manager_init(&ui);
    make_bpk_surface_archive(archive, sizeof(archive), 0);
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));
    rc = nexus_v1_bpk_archive_runtime_surface_handoff(
        archive, sizeof(archive), rows, 2U, &summary);
    expect_true(rc == 0 &&
                    summary.ready_stored_surfaces == 1U &&
                    rows[0].status ==
                        NEXUS_V1_BPK_SURFACE_HANDOFF_READY_STORED,
                "Nexus BPK handoff exposes one ready stored surface");
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_ui_load_bpk_runtime_surface(&ui,
                                           NEXUS_SURFACE_STABG,
                                           archive,
                                           sizeof(archive),
                                           &rows[0],
                                           32,
                                           16,
                                           "MENU.BPK[1]",
                                           &receipt);
    surface = &ui.surfaces[NEXUS_SURFACE_STABG];
    expect_true(rc == NEXUS_UI_BPK_IMPORT_OK &&
                    receipt.loaded == 1 &&
                    receipt.bytes_loaded == 16U &&
                    receipt.width == 4 &&
                    receipt.height == 4 &&
                    surface->data != NULL &&
                    surface->w == 4 &&
                    surface->h == 4 &&
                    surface->data[0] == 0x30 &&
                    surface->data[15] == 0x3f,
                "Nexus UI imports ready stored BPK surface bytes");

    make_bpk_surface_archive(archive, sizeof(archive), 1);
    memset(rows, 0, sizeof(rows));
    memset(&summary, 0, sizeof(summary));
    rc = nexus_v1_bpk_archive_runtime_surface_handoff(
        archive, sizeof(archive), rows, 2U, &summary);
    expect_true(rc == 0 &&
                    summary.blocked_prs3_surfaces == 1U &&
                    rows[0].status ==
                        NEXUS_V1_BPK_SURFACE_HANDOFF_BLOCKED_PRS3,
                "Nexus BPK handoff marks PRS3 surface blocked");
    memset(&receipt, 0, sizeof(receipt));
    rc = nexus_ui_load_bpk_runtime_surface(&ui,
                                           NEXUS_SURFACE_WARNING,
                                           archive,
                                           sizeof(archive),
                                           &rows[0],
                                           32,
                                           16,
                                           "MENU.BPK[1]",
                                           &receipt);
    expect_true(rc == NEXUS_UI_BPK_IMPORT_ERR_NOT_READY &&
                    receipt.blocked_prs3 == 1 &&
                    receipt.loaded == 0 &&
                    strcmp(nexus_ui_bpk_import_status_name(rc),
                           "not-ready") == 0,
                "Nexus UI refuses PRS3 BPK surface until decoder exists");
    nexus_ui_manager_free(&ui);
}

static void expect_failed_start_is_inactive(const char* data_dir,
                                            const char* expected_status) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;

    fill_nexus_spec(&spec, data_dir);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "Nexus M11 startup rejects invalid data");
    expect_true(view.active == 0,
                "failed Nexus startup leaves M11 inactive");
    expect_true(view.startedFromLauncher == 0,
                "failed Nexus startup does not claim launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG,
                "failed Nexus startup does not retain Nexus sourceKind");
    expect_true(view.nexusEngine == NULL,
                "failed Nexus startup does not expose a Nexus engine");
    expect_true(strstr(view.lastOutcome, expected_status) != NULL,
                "failed Nexus startup reports expected blocker");
    M11_GameView_Shutdown(&view);
    nexus_v1_launcher_shutdown();
}

static const char* nexus_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_NEXUS_V1_DATA_DIR");
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    (void)fallback;
    return NULL;
}

static void expect_canonical_face_media_is_blocked(const char* data_dir) {
    char path[640];
    unsigned char bytes[45104];
    unsigned char expanded[48 * 48];
    Nexus_UI_FaceLayout layout;
    Nexus_UI_FaceRecordDecodeInfo decode_info;
    Nexus_UI_FaceCompactRecordDescriptor descriptor;
    Nexus_UI_FacePrs3CorpusReceipt corpus;
    Nexus_UI_FacePrs3CaptureTarget capture_target;
    Nexus_UI_FacePrs3CaptureCampaignReceipt capture_campaign;
    FILE* file;

    snprintf(path, sizeof(path), "%s%sFACE.BIN", data_dir, TEST_PATH_SEP);
    file = fopen(path, "rb");
    if (!file) {
        puts("skip: canonical Nexus FACE.BIN is not staged");
        return;
    }
    expect_true(fread(bytes, 1u, sizeof(bytes), file) == sizeof(bytes) &&
                    fgetc(file) == EOF,
                "real Nexus FACE.BIN has its canonical 45,104-byte extent");
    fclose(file);
    memset(&layout, 0, sizeof(layout));
    expect_true(memcmp(bytes, "FACE", 4) == 0 &&
                    bytes[6] == 0xb0 && bytes[7] == 0x30 &&
                    nexus_ui_face_layout_detect(bytes, (int)sizeof(bytes),
                                                &layout) &&
                    layout.header_size == 56 && layout.entry_count == 20 &&
                    layout.entry_size == 0 && layout.portrait_w == 56 &&
                    layout.portrait_h == 56,
                "real Nexus FACE.BIN has the observed Saturn PRS3 frame layout");
    expect_true(nexus_ui_face_compact_record_descriptor(
                    bytes, (int)sizeof(bytes), 19, &descriptor) &&
                    descriptor.valid && descriptor.prefix_size >= 128 &&
                    descriptor.prefix_size <= 131 &&
                    descriptor.prs3_version == 1U &&
                    descriptor.declared_pixel_count == 56 * 56 &&
                    descriptor.prs3_offset == 42960 && descriptor.stream_size == 2126,
                "real Nexus FACE.BIN final PRS3 frame has locked version-1 bounds");
    memset(&decode_info, 0, sizeof(decode_info));
    memset(expanded, 0xa5, sizeof(expanded));
    expect_true(nexus_ui_expand_face_record_48x48(
                    bytes + descriptor.prs3_offset, (int)descriptor.prs3_size,
                    expanded, (int)sizeof(expanded),
                    &decode_info) == 0 &&
                    decode_info.kind == NEXUS_UI_FACE_RECORD_PRS3_UNPROVEN &&
                    decode_info.copied_pixels == 0 && expanded[0] == 0xa5,
                "real Nexus FACE.BIN PRS3 frame is blocked without a partial portrait");
    memset(&corpus, 0, sizeof(corpus));
    expect_true(nexus_ui_face_prs3_corpus_receipt(bytes, (int)sizeof(bytes),
                                                   &corpus) == 1 &&
                    corpus.valid && corpus.frame_count == 20 &&
                    corpus.source_byte_count == sizeof(bytes) &&
                    corpus.total_stream_byte_count > 0U &&
                    corpus.declared_total_pixel_count == 20U * 56U * 56U &&
                    !corpus.decoder_permitted && corpus.no_draw_only &&
                    !corpus.fallback_visuals_permitted,
                "real Nexus FACE.BIN yields a complete no-draw PRS3 corpus receipt");
    expect_true(nexus_ui_face_prs3_capture_target(
                    bytes, (int)sizeof(bytes), 19, 1, &capture_target) == 1 &&
                    capture_target.valid && capture_target.face_index == 19 &&
                    capture_target.descriptor.stream_offset == descriptor.stream_offset &&
                    capture_target.descriptor.stream_size == descriptor.stream_size &&
                    capture_target.prefix_bytes_fnv1a64 != 0U &&
                    capture_target.prs3_header_fnv1a64 != 0U &&
                    capture_target.stream_bytes_fnv1a64 != 0U &&
                    capture_target.capture_producer_required &&
                    capture_target.original_saturn_capture_required &&
                    !capture_target.decoder_permitted && capture_target.no_draw_only &&
                    !capture_target.fallback_visuals_permitted,
                "real Nexus FACE.BIN yields one source-bound no-draw capture target");
    expect_true(nexus_ui_face_prs3_capture_campaign(
                    bytes, (int)sizeof(bytes), 1, &capture_campaign) == 1 &&
                    capture_campaign.valid && capture_campaign.frame_count == 20 &&
                    capture_campaign.source_byte_count == sizeof(bytes) &&
                    capture_campaign.total_stream_byte_count == corpus.total_stream_byte_count &&
                    capture_campaign.ordered_target_fnv1a64 != 0U &&
                    capture_campaign.source_lanes_fnv1a64 != 0U &&
                    capture_campaign.capture_producer_required &&
                    capture_campaign.original_saturn_capture_required &&
                    !capture_campaign.decoder_permitted &&
                    capture_campaign.no_draw_only &&
                    !capture_campaign.fallback_visuals_permitted,
                "real Nexus FACE.BIN yields a complete source-only capture campaign");
}

int main(void) {
    char empty_root[512];
    char partial_root[512];
    char partial_dm_bin[512];
    char real_fallback[512];
    const char* real_dir;

    expect_true(nexus_v1_launcher_startup_structure3_raw_capture_intake(
                    NULL, 0U, NULL, NULL, &g_raw_capture_receipt) == 0 &&
                    g_raw_capture_receipt.no_draw_only &&
                    !g_raw_capture_receipt.raw_reader.import_ready &&
                    !g_raw_capture_receipt.host.importer_invoked,
                "Nexus launcher rejects raw Structure3 capture before a real level is loaded");

    expect_face_loader_counts_real_vs_fallback();
    expect_bpk_runtime_surface_import();
    expect_title_sequence_contract();
    expect_title_render_is_frame_dependent();
    expect_startup_layout_contract();
    expect_champion_startup_selection_contract();

    if (make_temp_root(empty_root)) {
        expect_failed_start_is_inactive(empty_root, "NEXUS DATA ERROR");
        (void)TEST_RMDIR(empty_root);
    } else {
        expect_true(0, "created empty Nexus temp root");
    }

    if (make_temp_root(partial_root)) {
        snprintf(partial_dm_bin, sizeof(partial_dm_bin), "%s%sDM.BIN",
                 partial_root, TEST_PATH_SEP);
        expect_true(write_file(partial_dm_bin, "not-a-real-nexus-dm-bin"),
                    "seeded partial Nexus DM.BIN");
        expect_failed_start_is_inactive(partial_root, "NEXUS LEVEL ERROR");
        remove(partial_dm_bin);
        (void)TEST_RMDIR(partial_root);
    } else {
        expect_true(0, "created partial Nexus temp root");
    }

    real_dir = nexus_data_dir(real_fallback);
    if (real_dir && real_dir[0]) {
        static M11_GameViewState view;
        M11_GameLaunchSpec spec;
        fill_nexus_spec(&spec, real_dir);
        M11_GameView_Init(&view);
        if (M11_GameView_Start(&view, &spec)) {
            char save_root[512];
            char save_path[512];
            static Nexus_V1_World resume_world;
            Nexus_SaveResult save_result;
            int resume_fixture_ready = 0;
            int visual_raster_available = 0;
            unsigned char framebuffer[320 * 200];

            expect_true(view.active == 1,
                        "real Nexus startup leaves M11 active");
            expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                        "real Nexus startup claims Nexus sourceKind");
            expect_true(view.nexusEngine != NULL,
                        "real Nexus startup exposes engine");
            expect_true(view.nexusState.level_loaded == 1,
                        "real Nexus startup loads level zero");
            expect_true(strstr(view.dungeonPath, "LEV00.DGN") != NULL,
                        "real Nexus startup exposes level path");
            expect_true(view.nexusState.title_active == 1,
                        "real Nexus startup enters title phase");
            expect_true(view.nexusEngine &&
                            nexus_v1_startup_surfaces_expected_count(
                                view.nexusEngine) >= 4 &&
                            nexus_v1_startup_surfaces_loaded_count(
                                view.nexusEngine) >= 4 &&
                            nexus_v1_startup_surfaces_fallback_count(
                                view.nexusEngine) == 0 &&
                            nexus_v1_startup_surfaces_ready(view.nexusEngine),
                        "real Nexus startup caches full TITLE/WARNING/GAMEOVER/STABG graphics");
            expect_canonical_face_media_is_blocked(real_dir);
            if (view.nexusEngine &&
                !nexus_v1_startup_faces_ready(view.nexusEngine)) {
                expect_true(nexus_v1_startup_faces_expected_count(view.nexusEngine) ==
                                view.nexusEngine->champions.champion_count &&
                                nexus_v1_startup_faces_loaded_count(view.nexusEngine) == 0 &&
                                nexus_v1_startup_faces_fallback_count(view.nexusEngine) ==
                                    view.nexusEngine->champions.champion_count,
                            "real Nexus FACE.BIN decoder gate denies every compact portrait");
                expect_true(advance_nexus_title_to_frame(
                                &view,
                                (unsigned int)nexus_title_boot_start_ready_frames(),
                                128),
                            "real Nexus title reaches the compact-FACE readiness gate");
                expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                                M11_GAME_INPUT_REDRAW &&
                                view.nexusState.title_active == 1 &&
                                view.nexusState.startup_save_select_active == 0 &&
                                view.nexusState.champion_select_active == 0 &&
                                strstr(view.lastOutcome, "blocked-faces") != NULL,
                            "real Nexus compact FACE.BIN blocks startup before champion rendering");
                M11_GameView_Shutdown(&view);
                nexus_v1_launcher_shutdown();
                goto real_media_complete;
            }
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            visual_raster_available =
                count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 500;
            if (!visual_raster_available) {
                puts("SKIP: Nexus Saturn palette/raster decode has no indexed proof");
            } else {
                expect_true(visual_raster_available,
                            "real Nexus title phase draws a nonblank frame");
            }
            {
                unsigned char frame_later[320 * 200];
                int tick_before = view.nexusState.tick_count;
                int t;
                for (t = 0;
                     t < 128 &&
                         view.nexusState.title_frame <
                             nexus_title_boot_warning_frames() + 16;
                     ++t) {
                    expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                                    M11_GAME_INPUT_REDRAW,
                                "real Nexus title idle advances title animation");
                }
                expect_true(view.nexusState.tick_count == tick_before,
                            "real Nexus title animation does not tick runtime");
                expect_true(view.nexusState.title_frame >=
                                nexus_title_boot_warning_frames() + 16,
                            "real Nexus title animation advances frame counter");
                memset(frame_later, 0, sizeof(frame_later));
                M11_GameView_Draw(&view, frame_later, 320, 200);
                if (visual_raster_available) {
                    expect_true(count_diff_pixels(framebuffer,
                                                  frame_later,
                                                  sizeof(framebuffer)) > 100,
                                "real Nexus TITLE.CG reveal changes after warning phase");
                }
            }
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                            M11_GAME_INPUT_IGNORED,
                        "real Nexus title ignores movement input before explicit start");
            (void)M11_GameView_HandlePointerButton(
                &view, 40, 40, DM1_V1_MOUSE_MASK_RIGHT_PC34);
            expect_true(view.nexusState.title_active == 1 &&
                            view.nexusState.champion_select_active == 0 &&
                            view.nexusState.startup_save_select_active == 0,
                        "real Nexus title non-start input does not open startup menus");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus title cannot skip before reveal completes");
            expect_true(view.nexusState.title_active == 1,
                        "real Nexus title stays active after early accept");
            expect_true(nexus_title_min_boot_frames() == 30 &&
                            !nexus_title_boot_reveal_complete(29) &&
                            nexus_title_boot_reveal_complete(30) &&
                            nexus_title_start_ready_frames() == 54 &&
                            !nexus_title_start_ready(53) &&
                            nexus_title_start_ready(54),
                        "Nexus title module exposes reveal and start gates");
            expect_true(advance_nexus_title_to_frame(
                            &view,
                            (unsigned int)(nexus_title_boot_warning_frames() +
                                           nexus_title_min_boot_frames()),
                            128),
                        "real Nexus title completes boot reveal");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus title blocks accept during title hold");
            expect_true(view.nexusState.title_active == 1,
                        "real Nexus title stays active during title hold");
            expect_true(advance_nexus_title_to_frame(
                            &view,
                            (unsigned int)nexus_title_boot_start_ready_frames(),
                            128),
                        "real Nexus title completes startup hold");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus title phase advances on accept after hold");
            expect_true(view.nexusState.title_active == 0,
                        "real Nexus title phase clears after input");
            if (view.nexusState.startup_save_select_active) {
                while (view.nexusState.startup_save_selected_row + 1 <
                       view.nexusState.startup_save_row_count) {
                    expect_true(M11_GameView_HandleInput(&view,
                                                         M12_MENU_INPUT_DOWN) ==
                                    M11_GAME_INPUT_REDRAW,
                                "real Nexus startup save menu moves toward NEW GAME");
                }
                expect_true(M11_GameView_HandleInput(&view,
                                                     M12_MENU_INPUT_ACCEPT) ==
                                M11_GAME_INPUT_REDRAW,
                            "real Nexus startup save menu NEW GAME enters champion selection");
            }
            expect_true(view.nexusState.champion_select_active == 1,
                        "real Nexus startup enters champion selection");
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            if (visual_raster_available) {
                expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 500,
                            "real Nexus champion selection draws a nonblank frame");
            }
            expect_true(view.nexusEngine &&
                            nexus_v1_startup_faces_loaded_count(view.nexusEngine) > 0,
                        "real Nexus startup loads FACE.BIN champion portraits");
            expect_true(view.nexusEngine &&
                            nexus_v1_startup_faces_expected_count(view.nexusEngine) ==
                                view.nexusEngine->champions.champion_count &&
                            nexus_v1_startup_faces_loaded_count(view.nexusEngine) >= 19 &&
                            nexus_v1_startup_faces_fallback_count(view.nexusEngine) ==
                                view.nexusEngine->champions.champion_count -
                                    nexus_v1_startup_faces_loaded_count(view.nexusEngine) &&
                            nexus_v1_startup_faces_ready(view.nexusEngine),
                        "real Nexus startup FACE.BIN surfaces cover the 24-row roster");
            if (visual_raster_available) {
                expect_true(count_nonzero_region(framebuffer, 320, 200,
                                                 22, 38, 10, 10) > 0,
                            "real Nexus champion selection draws FACE.BIN portrait pixels");
            }
            expect_true(M11_GameView_HandlePointer(&view, 24, 24, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection panel consumes non-row pointer hits");
            expect_true(view.nexusState.champion_select_active == 1 &&
                            view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 0,
                        "real Nexus champion selection panel hit does not recruit");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection recruits selected champion");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 1,
                        "real Nexus champion selection fills party");
            expect_true(view.nexusState.champion_cursor == 1,
                        "real Nexus champion selection advances cursor after recruit");
            expect_true(M11_GameView_HandlePointer(&view, 24, 49, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection accepts pointer row click");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 2,
                        "real Nexus pointer selection recruits second champion");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection Back removes last recruit");
            expect_true(view.nexusState.champion_select_active == 1 &&
                            view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 1 &&
                            view.nexusState.champion_cursor == 1,
                        "real Nexus champion selection Back keeps local cursor on removed champion");
            expect_true(M11_GameView_HandlePointer(&view, 24, 49, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection can reselect removed champion");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 2,
                        "real Nexus champion selection restore second recruit after Back");
            expect_true(M11_GameView_HandlePointer(&view, 24, 184, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection pointer footer starts dungeon");
            expect_true(view.nexusState.champion_select_active == 0,
                        "real Nexus champion selection clears for dungeon");
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            if (visual_raster_available) {
                expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 100,
                            "real Nexus dungeon phase draws a nonblank frame");
            }

            if (view.nexusEngine && make_temp_root(save_root)) {
                snprintf(save_path, sizeof(save_path), "%s%snexus_resume.fnxs",
                         save_root, TEST_PATH_SEP);
                nexus_v1_world_init(&resume_world);
                nexus_v1_party_place(&resume_world, 0, 18, 12, 3);
                resume_world.world_tick = 77u;
                resume_world.state_hash = nexus_v1_world_hash(&resume_world);
                save_result = nexus_v1_save_full_to_path(
                    save_path,
                    resume_world.party_level,
                    resume_world.party_x,
                    resume_world.party_y,
                    resume_world.party_dir,
                    (uint32_t)resume_world.world_tick,
                    resume_world.state_hash,
                    &view.nexusEngine->champions,
                    &resume_world);
                expect_true(save_result == NEXUS_SAVE_OK,
                            "wrote Nexus FNXS resume fixture");
                resume_fixture_ready = (save_result == NEXUS_SAVE_OK);
            }
            M11_GameView_Shutdown(&view);
            nexus_v1_launcher_shutdown();

            if (resume_fixture_ready) {
                fill_nexus_spec(&spec, real_dir);
                spec.savePath = save_path;
                M11_GameView_Init(&view);
                expect_true(M11_GameView_Start(&view, &spec),
                            "M11 Nexus FNXS resume succeeds");
                expect_true(view.active == 1,
                            "resumed Nexus startup leaves M11 active");
                expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                            "resumed Nexus startup claims Nexus sourceKind");
                expect_true(strstr(view.lastOutcome, "NEXUS RESUMED") != NULL,
                            "resumed Nexus startup reports resumed status");
                expect_true(view.nexusState.title_active == 0,
                            "resumed Nexus startup skips title phase");
                expect_true(view.nexusState.champion_select_active == 0,
                            "resumed Nexus startup skips champion selection");
                expect_true(view.nexusState.party_x == 18 &&
                            view.nexusState.party_y == 12 &&
                            view.nexusState.party_dir == 3,
                            "resumed Nexus startup mirrors saved party pose");
                expect_true(view.nexusState.tick_count == 77,
                            "resumed Nexus startup mirrors saved tick");
                expect_true(view.nexusEngine &&
                            view.nexusEngine->mechanics &&
                            view.nexusEngine->mechanics->party_x == 18 &&
                            view.nexusEngine->mechanics->party_y == 12 &&
                            view.nexusEngine->mechanics->party_dir == 3,
                            "resumed Nexus startup applies mechanics pose");
                M11_GameView_Shutdown(&view);
                nexus_v1_launcher_shutdown();
                remove(save_path);
                (void)TEST_RMDIR(save_root);
            }

            {
                char temp_home[512];
                char old_home[512];
                char default_save_dir[512];
                const char* old_home_env = getenv("HOME");
                static Nexus_V1_SaveManager slot_mgr;
                static Nexus_V1_ChampionPool slot_champions;
                static Nexus_V1_World slot_world;
                int slot_fixture_ready = 0;

                snprintf(old_home, sizeof(old_home), "%s",
                         old_home_env ? old_home_env : "");
                if (make_temp_root(temp_home) && set_test_home(temp_home)) {
                    nexus_v1_save_default_dir(default_save_dir,
                                              sizeof(default_save_dir));
                    nexus_v1_champions_init(&slot_champions);
                    nexus_v1_world_init(&slot_world);
                    nexus_v1_party_place(&slot_world, 0, 19, 13, 2);
                    slot_world.world_tick = 91u;
                    slot_world.state_hash = nexus_v1_world_hash(&slot_world);
                    nexus_v1_save_init(&slot_mgr, default_save_dir);
                    slot_fixture_ready =
                        nexus_v1_save_full(&slot_mgr,
                                           3,
                                           slot_world.party_level,
                                           slot_world.party_x,
                                           slot_world.party_y,
                                           slot_world.party_dir,
                                           (uint32_t)slot_world.world_tick,
                                           slot_world.state_hash,
                                           &slot_champions,
                                           &slot_world) == NEXUS_SAVE_OK;
                    expect_true(slot_fixture_ready,
                                "wrote Nexus default-save slot fixture");
                    if (slot_fixture_ready) {
                        fill_nexus_spec(&spec, real_dir);
                        M11_GameView_Init(&view);
                        expect_true(M11_GameView_Start(&view, &spec),
                                    "M11 Nexus startup with default save slot succeeds");
                        expect_true(view.nexusState.title_active == 1,
                                    "M11 Nexus save-slot startup starts on title");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_DOWN) ==
                                        M11_GAME_INPUT_IGNORED,
                                    "M11 Nexus save-slot title ignores movement input");
                        expect_true(view.nexusState.title_active == 1 &&
                                        view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus save-slot title movement does not open save menu");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus save-slot title blocks early accept");
                        expect_true(view.nexusState.title_active == 1 &&
                                        view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus save-slot title remains active after early accept");
                        expect_true(advance_nexus_title_to_frame(
                                        &view,
                                        (unsigned int)nexus_title_boot_start_ready_frames(),
                                        128),
                                    "M11 Nexus save-slot title completes startup hold");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus title advances to save selection after hold");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus startup exposes save selection when slots exist");
                        expect_true(view.nexusState.startup_save_slot_mask ==
                                        (1u << 3),
                                    "M11 Nexus startup save selection sees slot 03");
                        memset(framebuffer, 0, sizeof(framebuffer));
                        M11_GameView_Draw(&view, framebuffer, 320, 200);
                        if (visual_raster_available) {
                            expect_true(count_nonzero_pixels(framebuffer,
                                                             sizeof(framebuffer)) > 500,
                                        "M11 Nexus startup save selection draws nonblank frame");
                        }
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_BACK) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup save selection Back returns to title");
                        expect_true(view.nexusState.title_active == 1 &&
                                        view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus startup save selection Back restores title phase");
                        expect_true(advance_nexus_title_to_frame(
                                        &view,
                                        (unsigned int)nexus_title_boot_start_ready_frames(),
                                        128),
                                    "M11 Nexus startup title hold completes after Back");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup title reopens save selection after Back hold");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus startup save selection is active again after title");
                        expect_true(M11_GameView_HandlePointer(
                                        &view, 24, 20, 1) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup save panel consumes non-row pointer hits");
                        expect_true(view.nexusState.startup_save_select_active == 1 &&
                                        view.nexusState.startup_save_selected_row == 0,
                                    "M11 Nexus startup save panel hit keeps menu selection");
                        expect_true(M11_GameView_HandlePointer(
                                        &view, 24, 43, 1) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup pointer loads slot row");
                        expect_true(view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus startup save selection closes after load");
                        expect_true(strstr(view.lastOutcome, "NEXUS RESUMED") != NULL,
                                    "M11 Nexus startup slot load reports resumed status");
                        expect_true(view.nexusState.party_x == 19 &&
                                    view.nexusState.party_y == 13 &&
                                    view.nexusState.party_dir == 2,
                                    "M11 Nexus startup slot load mirrors saved pose");
                        expect_true(view.nexusState.tick_count == 91,
                                    "M11 Nexus startup slot load mirrors saved tick");
                        M11_GameView_Shutdown(&view);
                        nexus_v1_launcher_shutdown();

                        fill_nexus_spec(&spec, real_dir);
                        M11_GameView_Init(&view);
                        expect_true(M11_GameView_Start(&view, &spec),
                                    "M11 Nexus startup with save slot restarts for NEW GAME");
                        expect_true(view.nexusState.title_active == 1,
                                    "M11 Nexus save-slot NEW GAME path starts on title");
                        expect_true(advance_nexus_title_to_frame(
                                        &view,
                                        (unsigned int)nexus_title_boot_start_ready_frames(),
                                        128),
                                    "M11 Nexus save-slot NEW GAME title completes startup hold");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus save-slot NEW GAME path advances title after hold");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus save-slot NEW GAME path exposes save menu");
                        {
                            int tick_before = view.nexusState.tick_count;
                            int engine_tick_before = view.nexusEngine
                                ? view.nexusEngine->game.tick_count
                                : -1;
                            expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                                            M11_GAME_INPUT_IGNORED,
                                        "M11 Nexus save-select blocks idle runtime tick");
                            expect_true(view.nexusState.startup_save_select_active == 1 &&
                                            view.nexusState.tick_count == tick_before &&
                                            view.nexusEngine &&
                                            view.nexusEngine->game.tick_count ==
                                                engine_tick_before,
                                        "M11 Nexus save-select keeps runtime tick frozen");
                        }
                        while (view.nexusState.startup_save_selected_row + 1 <
                               view.nexusState.startup_save_row_count) {
                            expect_true(M11_GameView_HandleInput(
                                            &view, M12_MENU_INPUT_DOWN) ==
                                            M11_GAME_INPUT_REDRAW,
                                        "M11 Nexus save-slot NEW GAME path moves down");
                        }
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus save-slot NEW GAME path accepts NEW GAME");
                        expect_true(view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus save-slot NEW GAME path closes save menu");
                        expect_true(view.nexusState.champion_select_active == 1,
                                    "M11 Nexus save-slot NEW GAME path enters champion selection");
                        expect_true(view.nexusEngine &&
                                        view.nexusEngine->champions.party_count == 0,
                                    "M11 Nexus save-slot NEW GAME path keeps empty new party");
                        memset(framebuffer, 0, sizeof(framebuffer));
                        M11_GameView_Draw(&view, framebuffer, 320, 200);
                        if (visual_raster_available) {
                            expect_true(count_nonzero_pixels(framebuffer,
                                                             sizeof(framebuffer)) > 500,
                                        "M11 Nexus save-slot NEW GAME path draws champion select");
                        }
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_BACK) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus empty champion selection Back returns to save menu");
                        expect_true(view.nexusState.champion_select_active == 0 &&
                                        view.nexusState.startup_save_select_active == 1 &&
                                        view.nexusState.startup_save_selected_row + 1 ==
                                            view.nexusState.startup_save_row_count,
                                    "M11 Nexus empty champion selection Back keeps NEW GAME selected");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus NEW GAME can re-enter champion selection after Back");
                        expect_true(view.nexusState.champion_select_active == 1 &&
                                        view.nexusEngine &&
                                        view.nexusEngine->champions.party_count == 0,
                                    "M11 Nexus re-entered champion selection remains a new party");
                        M11_GameView_Shutdown(&view);
                        nexus_v1_launcher_shutdown();
                    }
                    if (old_home[0]) {
                        (void)set_test_home(old_home);
                    }
                    cleanup_nexus_default_save_dir(temp_home);
                    (void)TEST_RMDIR(temp_home);
                } else {
                    if (old_home[0]) {
                        (void)set_test_home(old_home);
                    }
                    expect_true(0, "created isolated Nexus startup HOME");
                }
            }
        } else {
            printf("skip: no launchable Nexus V1 data at %s\n", real_dir);
            M11_GameView_Shutdown(&view);
            nexus_v1_launcher_shutdown();
        }
    }

real_media_complete:
    if (g_failures) {
        fprintf(stderr, "M11 Nexus startup gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 Nexus startup gate");
    return 0;
}
