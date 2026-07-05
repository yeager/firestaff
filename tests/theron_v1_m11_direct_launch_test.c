/*
 * theron_v1_m11_direct_launch_test.c -- M11 Theron direct-launch handoff.
 *
 * Verifies that M11 consumes the M12 catalog's already hash-verified
 * Track 02 path/MD5 and reaches the native Theron viewport path without
 * re-walking the data root through theron_v1_boot_scan_assets().
 *
 * Source-lock: THQUEST.ASM T400 (data-track loading).  ReDMCSB has no
 * Theron code; Firestaff's source-faithful contract here is that the
 * verified Track 02 blob is handed to runtime directly once located.
 */

#include "m11_game_view.h"
#include "theron_v1_boot.h"
#include "theron_v1_save_load.h"
#include "theron_v1_srm_classifier.h"
#include "theron_v1_startup_flow.h"
#include "theron_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#define TEST_MKDIR(path) _mkdir(path)
#define PATH_SEP "\\"
#else
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define PATH_SEP "/"
#endif

#ifndef FIRESTAFF_HAS_ZLIB
#define FIRESTAFF_HAS_ZLIB 0
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures = 0;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int write_file(const char* path, const char* text) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (text && text[0]) {
        fwrite(text, 1, strlen(text), fp);
    }
    fclose(fp);
    return 1;
}

static int write_bytes(const char* path,
                       const unsigned char* bytes,
                       size_t size) {
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (size > 0 && bytes) {
        if (fwrite(bytes, 1, size, fp) != size) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return 1;
}

static int make_temp_dir(char out[512]) {
#if defined(_WIN32)
    const char* tmp = getenv("TEMP");
    snprintf(out, 512, "%s\\firestaff_theron_m11_%lu",
             tmp ? tmp : ".", (unsigned long)rand());
    return TEST_MKDIR(out) == 0;
#else
    snprintf(out, 512, "/tmp/firestaff_theron_m11_XXXXXX");
    return mkdtemp(out) != NULL;
#endif
}

static int test_set_home(const char* path) {
#if defined(_WIN32)
    char envbuf[1024];
    snprintf(envbuf, sizeof(envbuf), "HOME=%s", path ? path : "");
    return _putenv(envbuf) == 0;
#else
    return path ? setenv("HOME", path, 1) == 0 : unsetenv("HOME") == 0;
#endif
}

static int test_setenv_name(const char* name, const char* value) {
#if defined(_WIN32)
    return _putenv_s(name, value ? value : "") == 0;
#else
    return value ? setenv(name, value, 1) == 0 : unsetenv(name) == 0;
#endif
}

static const unsigned char g_valid_gzip_srm[] = {
    0x1f, 0x8b, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xff,
    0x73, 0x0b, 0x0e, 0x09, 0x0c, 0x08, 0x72, 0x37, 0x64, 0x64,
    0x66, 0x66, 0xd4, 0x61, 0x64, 0x60, 0x60, 0x14, 0x60, 0x60,
    0x60, 0x02, 0x62, 0x66, 0x20, 0x66, 0x01, 0x62, 0x56, 0x20,
    0x66, 0x03, 0x62, 0x76, 0x20, 0x06, 0x00, 0x50, 0x8a, 0x0c,
    0xc3, 0x2c, 0x00, 0x00, 0x00
};

static int count_nonzero_pixels(const unsigned char* pixels, size_t count) {
    int n = 0;
    size_t i;
    if (!pixels) return 0;
    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0) {
            ++n;
        }
    }
    return n;
}

static int startup_rows_contain(
    char rows[][M11_THERON_STARTUP_RENDER_ROW_CAPACITY],
    int row_count,
    const char* needle) {
    int i;
    if (!rows || !needle) {
        return 0;
    }
    for (i = 0; i < row_count; ++i) {
        if (strstr(rows[i], needle) != NULL) {
            return 1;
        }
    }
    return 0;
}

static const M11_TheronStartupElement* find_startup_element(
    const M11_TheronStartupElement* elements,
    int count,
    M11_TheronStartupElementKind kind,
    int id) {
    int i;
    if (!elements) {
        return NULL;
    }
    for (i = 0; i < count; ++i) {
        if (elements[i].kind != kind) {
            continue;
        }
        if (kind == M11_THERON_STARTUP_ELEMENT_STAGE &&
            elements[i].dungeonId != id) {
            continue;
        }
        if (kind == M11_THERON_STARTUP_ELEMENT_MIRROR &&
            elements[i].mirrorIndex != id) {
            continue;
        }
        return &elements[i];
    }
    return NULL;
}

int main(void) {
    enum { FB_W = 320, FB_H = 200 };
    char temp_dir[512];
    char theron_dir[512];
    char track_path[512];
    char save_root[512];
    char selected_save_path[512];
    unsigned char saved_champions[
        THERON_MAX_CHAMPIONS * sizeof(Theron_V1_Champion)];
    Theron_V1_Party saved_party;
    Theron_DungeonProgression saved_progression;
    unsigned char framebuffer[FB_W * FB_H];
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    Theron_V1_BootProfile* profile;
    Theron_V1_World* world;
    unsigned long rescans_before;
    unsigned long rescans_after;
    int render_pixels;
    char startup_rows[16][M11_THERON_STARTUP_RENDER_ROW_CAPACITY];
    M11_TheronStartupElement startup_layout[16];
    int startup_row_count;
    int startup_layout_count;
    int i;

    expect_true(make_temp_dir(temp_dir), "temporary root created");
    snprintf(theron_dir, sizeof(theron_dir), "%s%stheron", temp_dir, PATH_SEP);
    expect_true(TEST_MKDIR(theron_dir) == 0, "theron subdir created");
    snprintf(track_path, sizeof(track_path),
             "%s%s%s", theron_dir, PATH_SEP,
             "Theron's Quest (US) (Track 02).bin");
    expect_true(write_file(track_path, "fake-track02-without-bank-markers"),
                "fake Track 02 file written");
    expect_true(test_set_home(temp_dir), "test HOME points at Theron temp root");

    theron_v1_party_init(&saved_party, THERON_DUNGEON_1_HALL_OF_RECORDS);
    saved_party.champions[0].health = 77;
    saved_party.champions[0].max_health = 88;
    memset(saved_champions, 0, sizeof(saved_champions));
    expect_true(theron_v1_party_pack(&saved_party,
                                     saved_champions,
                                     sizeof(saved_champions)) ==
                    theron_v1_party_pack_size(),
                "test packs Theron champion save data");
    theron_v1_dungeon_progression_init(&saved_progression);
    saved_progression.current_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    saved_progression.current_level = 1;
    saved_progression.dungeon_playtime_seconds = 1234;
    snprintf(save_root, sizeof(save_root),
             "%s%ssaves%stheron",
             temp_dir, PATH_SEP, PATH_SEP);
    expect_true(theron_v1_save_to_slot(save_root,
                                       2,
                                       saved_champions,
                                       sizeof(saved_champions),
                                       &saved_progression,
                                       "Continue Test") == 0,
                "test writes a valid Theron .tqsv continue slot");
    saved_party.champions[0].health = 66;
    memset(saved_champions, 0, sizeof(saved_champions));
    expect_true(theron_v1_party_pack(&saved_party,
                                     saved_champions,
                                     sizeof(saved_champions)) ==
                    theron_v1_party_pack_size(),
                "test packs selected Theron champion save data");
    saved_progression.dungeon_playtime_seconds = 5678;
    expect_true(theron_v1_save_to_slot(save_root,
                                       5,
                                       saved_champions,
                                       sizeof(saved_champions),
                                       &saved_progression,
                                       "Selected Continue") == 0,
                "test writes selected Theron .tqsv continue slot");
    theron_v1_save_slot_path(save_root,
                             5,
                             selected_save_path,
                             sizeof(selected_save_path));

    memset(&spec, 0, sizeof(spec));
    spec.title = "THERON'S QUEST";
    spec.gameId = "theron";
    spec.sourceId = "theron";
    spec.dataDir = temp_dir;
    spec.verifiedAssetPath = track_path;
    spec.verifiedAssetMd5 = "f23601102138f87c33025877767ebf76";
    spec.savePath = selected_save_path;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    theron_v1_boot_rescan_call_count_reset();
    rescans_before = theron_v1_boot_rescan_call_count();
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 Theron verified-path start succeeds");
    rescans_after = theron_v1_boot_rescan_call_count();

    /* The verified-path boot adds a single stat() to confirm the
     * cached Track 02 is still on disk (stale-path guard added in
     * the 2026-06-28 Theron direct-launch reuse-gate pass).  What
     * this assertion proves is that M11_GameView_Start does NOT
     * re-walk the data root — the count delta is bounded by the
     * one-shot stale guard on the supplied path, not by the
     * g_theron_track02_candidates chain in theron_v1_boot_scan_
     * assets().  The exact bound is rescans_after == rescans_before
     * + 1 (the file_exists() call); other M11 probes would push it
     * higher. */
    expect_true(rescans_after == rescans_before + 1UL,
                "M11 verified-path start runs only the stale-path guard "
                "(no data-root fallback walk)");
    expect_true(view.active == 1, "M11 view is active");
    expect_true(view.sourceKind == M11_GAME_SOURCE_THERON_TRACK02,
                "M11 source kind is Theron Track 02");
    expect_true(strcmp(view.dungeonPath, track_path) == 0,
                "M11 stores the verified Track 02 path");
    expect_true(view.theronBootProfile != NULL,
                "M11 stores a Theron boot profile");
    expect_true(view.theronWorld != NULL && view.theronViewport != NULL,
                "M11 builds Theron world and viewport");

    profile = (Theron_V1_BootProfile*)view.theronBootProfile;
    world = (Theron_V1_World*)view.theronWorld;
    expect_true(profile->assets_verified == 1,
                "boot profile remains assets_verified");
    expect_true(strcmp(profile->graphics_md5, spec.verifiedAssetMd5) == 0,
                "boot profile carries the verified MD5");
    expect_true(strcmp(profile->graphics_path, track_path) == 0,
                "boot profile carries the verified path");
    expect_true(world != NULL && world->level_loaded[0][0] == 0,
                "M11 waits at Theron stage select before level load");
    expect_true(view.theronState.selected_dungeon ==
                THERON_DUNGEON_1_HALL_OF_RECORDS,
                "M11 stage cursor starts on dungeon 1");
    expect_true(view.theronState.companion_count == 0,
                "M11 startup begins with no companions selected");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_STAGE_SELECT,
                "M11 direct launch starts at visible stage select");
    expect_true(view.theronState.save_resume_verdict >= 0,
                "M11 Theron direct launch evaluates startup save/resume verdict");
    expect_true(view.theronState.save_resume_claim >= 0,
                "M11 Theron direct launch evaluates startup save/resume claim");
    expect_true(view.theronState.save_resume_active_slot == 5 &&
                view.theronState.save_resume_tqsv_slots >= 2,
                "M11 Theron direct launch exposes selected .tqsv continue slot");
    expect_true(strstr(view.inspectDetail, "SAVE ") != NULL,
                "M11 Theron startup inspect readout reports save/resume claim");
    expect_true(strstr(view.inspectDetail, "Chapter 1") != NULL &&
                strstr(view.inspectDetail, "Hall of Records") != NULL,
                "M11 Theron startup inspect readout reports chapter marker");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, FB_W, FB_H);
    render_pixels = count_nonzero_pixels(framebuffer, sizeof(framebuffer));
    expect_true(render_pixels > 1000,
                "M11 Theron startup screen produces a nonblank framebuffer");
    startup_row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_row_count >= 5 &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "Chapter 1: Hall of Records") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "CHOOSE A STAGE") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "CONTINUE  TQSV SLOT 5") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "> 1  Hall of Records"),
                "M11 Theron startup render rows expose stage selection state");
    view.theronState.startup_roster_name_count = 8;
    snprintf(view.theronState.startup_roster_names[0],
             sizeof(view.theronState.startup_roster_names[0]),
             "THERON");
    snprintf(view.theronState.startup_roster_names[1],
             sizeof(view.theronState.startup_roster_names[1]),
             "MARA");
    snprintf(view.theronState.startup_roster_names[2],
             sizeof(view.theronState.startup_roster_names[2]),
             "LINOS");
    snprintf(view.theronState.startup_roster_names[3],
             sizeof(view.theronState.startup_roster_names[3]),
             "HEXA");
    snprintf(view.theronState.startup_roster_names[4],
             sizeof(view.theronState.startup_roster_names[4]),
             "HAKAR");
    snprintf(view.theronState.startup_roster_names[5],
             sizeof(view.theronState.startup_roster_names[5]),
             "TIRAN");
    snprintf(view.theronState.startup_roster_names[6],
             sizeof(view.theronState.startup_roster_names[6]),
             "DOTAN");
    snprintf(view.theronState.startup_roster_names[7],
             sizeof(view.theronState.startup_roster_names[7]),
             "PENTAI");
    snprintf(view.theronState.startup_roster_titles[1],
             sizeof(view.theronState.startup_roster_titles[1]),
             "GUARDIAN OF WISDO");
    snprintf(view.theronState.startup_roster_titles[4],
             sizeof(view.theronState.startup_roster_titles[4]),
             "THE BRAVE");
    snprintf(view.theronState.startup_roster_titles[7],
             sizeof(view.theronState.startup_roster_titles[7]),
             "THE SURVIVOR");
    startup_row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_rows_contain(startup_rows,
                                     startup_row_count,
                                     "TRACK 02 ROSTER: THERON MARA LINOS HEXA HAKAR TIRAN DOTAN PENTAI"),
                "M11 Theron startup render rows expose decoded Track 02 roster names");
    expect_true(startup_rows_contain(startup_rows,
                                     startup_row_count,
                                     "TRACK 02 TITLES: MARA=GUARDIAN OF WISDO; PENTAI=THE SURVIVOR"),
                "M11 Theron startup render rows expose decoded Track 02 roster titles");
    startup_layout_count = M11_GameView_GetTheronStartupLayout(
        &view, startup_layout, 16);
    {
        const M11_TheronStartupElement* title =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_TITLE,
                                 0);
        const M11_TheronStartupElement* chapter =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_CHAPTER,
                                 0);
        const M11_TheronStartupElement* cont =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_CONTINUE,
                                 0);
        const M11_TheronStartupElement* stage1 =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_STAGE,
                                 THERON_DUNGEON_1_HALL_OF_RECORDS);
        expect_true(startup_layout_count >= 10 &&
                    title != NULL &&
                    title->enabled == 1 &&
                    strcmp(title->label, "THERON'S QUEST") == 0 &&
                    chapter != NULL &&
                    chapter->enabled == 1 &&
                    strstr(chapter->label, "Chapter 1") != NULL &&
                    chapter->x == 34 &&
                    chapter->y == 38 &&
                    cont != NULL &&
                    cont->saveKind == 1 &&
                    cont->saveSlot == 5 &&
                    cont->x == 40 &&
                    cont->y == 66 &&
                    cont->w > 0 &&
                    cont->h > 0 &&
                    stage1 != NULL &&
                    stage1->enabled == 1 &&
                    stage1->selected == 1 &&
                    stage1->cursor == 1 &&
                    stage1->x == 40 &&
                    stage1->y == 78 &&
                    stage1->w > 0 &&
                    stage1->h > 0,
                    "M11 Theron startup layout exposes machine-readable stage state");
    }

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron stage cursor can focus Continue");
    expect_true(view.theronState.save_resume_continue_focus == 1,
                "M11 Theron Continue row receives focus");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Continue loads active .tqsv slot");
    expect_true(view.theronState.save_resume_continue_focus == 0 &&
                view.theronState.startup_phase ==
                    THERON_STARTUP_PHASE_STAGE_SELECT,
                "M11 Theron Continue returns to stage select");
    expect_true(world->progression.dungeon_playtime_seconds == 5678 &&
                world->party.champions[0].health == 66,
                "M11 Theron Continue applies selected progression and Theron champion data");
    expect_true(strstr(view.inspectDetail, "continued slot=5") != NULL &&
                strstr(view.inspectDetail, "Chapter 1") != NULL,
                "M11 Theron Continue inspect readout keeps selected slot and chapter marker");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron stage accept opens Soul Room");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_SOUL_ROOM,
                "M11 Theron startup enters Soul Room before dungeon");
    startup_row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_row_count >= 12 &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "Chapter 1: Hall of Records") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "SOUL ROOM") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "> HAKAR") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "MARA") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "PENTAI") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "ENTER FORCEFIELD"),
                "M11 Theron Soul Room render rows expose decoded Track 02 mirror names");
    startup_layout_count = M11_GameView_GetTheronStartupLayout(
        &view, startup_layout, 16);
    {
        const M11_TheronStartupElement* mirror0 =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_MIRROR,
                                 0);
        const M11_TheronStartupElement* forcefield =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_FORCEFIELD,
                                 0);
        expect_true(startup_layout_count >= 10 &&
                    mirror0 != NULL &&
                    mirror0->cursor == 1 &&
                    mirror0->selected == 0 &&
                    mirror0->x == 46 &&
                    mirror0->y == 78 &&
                    mirror0->w > 0 &&
                    mirror0->h > 0 &&
                    mirror0->portraitIndex == 1 &&
                    mirror0->primaryClass == THERON_CLASS_FIGHTER &&
                    strcmp(mirror0->label, "HAKAR") == 0 &&
                    strcmp(mirror0->decodedName, "HAKAR") == 0 &&
                    strcmp(mirror0->decodedTitle, "THE BRAVE") == 0 &&
                    forcefield != NULL &&
                    forcefield->enabled == 0 &&
                    forcefield->x == 46 &&
                    forcefield->y == 160 &&
                    forcefield->w > 0 &&
                    forcefield->h > 0,
                    "M11 Theron startup layout exposes Soul Room mirrors and gated forcefield");
    }
    for (i = 0; i < 6; ++i) {
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                    M11_GAME_INPUT_REDRAW,
                    "M11 Theron Soul Room cursor moves to mirror 7");
    }
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room mirror 7 accept selects a companion");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_READY &&
                view.theronState.companion_count == 1 &&
                (view.theronState.selected_mirrors_mask & (1 << 6)) != 0 &&
                view.theronState.selected_mirror_order[0] == 6,
                "M11 Theron Soul Room records first selected mirror order");
    startup_row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_rows_contain(startup_rows, startup_row_count,
                                     "PENTAI") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "RESURRECTED #1"),
                "M11 Theron Soul Room render rows show selected decoded mirror order");
    startup_layout_count = M11_GameView_GetTheronStartupLayout(
        &view, startup_layout, 16);
    {
        const M11_TheronStartupElement* pental =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_MIRROR,
                                 6);
        const M11_TheronStartupElement* forcefield =
            find_startup_element(startup_layout,
                                 startup_layout_count,
                                 M11_THERON_STARTUP_ELEMENT_FORCEFIELD,
                                 0);
        expect_true(pental != NULL &&
                    pental->selected == 1 &&
                    pental->selectedOrder == 1 &&
                    pental->cursor == 1 &&
                    pental->portraitIndex == 7 &&
                    pental->primaryClass == THERON_CLASS_FIGHTER &&
                    pental->x == 46 &&
                    pental->y == 144 &&
                    pental->w > 0 &&
                    pental->h > 0 &&
                    strcmp(pental->decodedName, "PENTAI") == 0 &&
                    strcmp(pental->decodedTitle, "THE SURVIVOR") == 0 &&
                    forcefield != NULL &&
                    forcefield->enabled == 1,
                    "M11 Theron startup layout exposes selected mirror order and enabled forcefield");
    }
    expect_true(M11_GameView_HandlePointer(&view, 46 + 115, 144 + 5, 1) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room selected mirror click deselects companion");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_SOUL_ROOM &&
                view.theronState.companion_count == 0 &&
                (view.theronState.selected_mirrors_mask & (1 << 6)) == 0,
                "M11 Theron Soul Room deselect clears mirror mask and returns to soul-room phase");
    startup_row_count = M11_GameView_GetTheronStartupRenderRows(
        &view, startup_rows, 16);
    expect_true(startup_rows_contain(startup_rows, startup_row_count,
                                     "PENTAI") &&
                startup_rows_contain(startup_rows, startup_row_count,
                                     "AVAILABLE"),
                "M11 Theron Soul Room render rows show deselected decoded mirror as available");
    expect_true(M11_GameView_HandlePointer(&view, 46 + 115, 144 + 5, 1) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room mirror 7 click can reselect after deselect");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_READY &&
                view.theronState.companion_count == 1 &&
                view.theronState.selected_mirror_order[0] == 6,
                "M11 Theron Soul Room reselect restores first selected mirror order");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room cursor moves to forcefield");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room cursor wraps to mirror 1");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room mirror 1 accept selects a companion");
    expect_true(view.theronState.companion_count == 2 &&
                view.theronState.selected_mirror_order[1] == 0,
                "M11 Theron Soul Room records second selected mirror order");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room cursor moves to mirror 2");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room cursor moves to mirror 3");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron Soul Room mirror 3 accept selects a companion");
    expect_true(view.theronState.companion_count == 3 &&
                view.theronState.selected_mirror_order[2] == 2,
                "M11 Theron Soul Room records third selected mirror order");
    expect_true(M11_GameView_HandlePointer(&view, 46 + 77, 158 + 5, 1) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron forcefield click loads the dungeon");
    expect_true(world != NULL && world->level_loaded[0][0] == 1,
                "M11 loaded the initial Theron level after forcefield");
    expect_true(world != NULL &&
                world->party.champion_count == 4 &&
                strcmp(world->party.champions[0].name, "Theron") == 0 &&
                world->party.champions[0].health == 66,
                "M11 forcefield materializes selected .tqsv Theron plus mirrors");
    expect_true(strcmp(world->party.champions[1].name, "PENTAI") == 0 &&
                strcmp(world->party.champions[2].name, "HAKAR") == 0 &&
                strcmp(world->party.champions[3].name, "TIRAN") == 0,
                "M11 forcefield preserves decoded Track 02 resurrection order");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_IN_DUNGEON,
                "M11 mirrors startup forcefield handoff phase");
    expect_true(view.theronState.party_x == 3 &&
                view.theronState.party_y == 5 &&
                view.theronState.party_dir == 0 &&
                view.theronState.tick_count == 0,
                "M11 starts at the deterministic Theron runtime pose");
    expect_true(world->levels[
                    THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0].width == 8 &&
                world->levels[
                    THERON_DUNGEON_1_HALL_OF_RECORDS - 1][0].height == 8 &&
                world->progression.dungeon_seeds[
                    THERON_DUNGEON_1_HALL_OF_RECORDS - 1] == 313,
                "M11 Theron stage 1 fallback room keeps the legacy 8x8 seed-313 contract");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron turn command requests redraw");
    expect_true(view.theronState.party_dir == 1 &&
                world->party.leader_dir == 1,
                "M11 Theron turn command updates world and mirror state");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron forward command requests redraw");
    expect_true(view.theronState.party_x == 4 &&
                view.theronState.party_y == 5 &&
                view.theronState.party_dir == 1,
                "M11 Theron forward command advances through mechanics");
    expect_true(view.theronState.tick_count == 1 &&
                world->world_tick == 1,
                "M11 Theron forward command runs post-move tick effects");

    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "M11 Theron idle tick requests redraw");
    expect_true(view.theronState.tick_count == 2 &&
                world->world_tick == 2,
                "M11 Theron idle tick stays synced with world tick");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_DOWN) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron backward command requests redraw");
    expect_true(view.theronState.party_x == 3 &&
                view.theronState.party_y == 5 &&
                view.theronState.party_dir == 1,
                "M11 Theron backward command moves without changing facing");
    expect_true(view.theronState.tick_count == 3 &&
                world->world_tick == 3,
                "M11 Theron backward command also runs post-move tick effects");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, FB_W, FB_H);
    render_pixels = count_nonzero_pixels(framebuffer, sizeof(framebuffer));
    expect_true(render_pixels > 1000,
                "M11 Theron draw path produces a nonblank framebuffer");

    world->object_count = 1;
    world->objects[0].type = THERON_OBJTYPE_QUEST_ITEM;
    world->objects[0].level = world->current_level;
    world->objects[0].x = world->party.leader_x;
    world->objects[0].y = world->party.leader_y;
    world->timer_count = 1;
    world->timers[0].id = 1;
    world->timers[0].flags = THERON_TIMER_F_ACTIVE;
    world->timers[0].remaining_ticks = 10;

    expect_true(theron_v1_quest_item_collect(
                    &world->progression,
                    (Theron_QuestItem)
                        THERON_QUEST_ITEM_MASK_FROM_DUNGEON(
                            world->current_dungeon)) == 1,
                "M11 Theron test collects current dungeon quest item");
    world->dungeon_complete = 1;
    world->party.leader_x = 3;
    world->party.leader_y = 2;
    world->party.leader_dir = 0;
    view.theronState.party_x = world->party.leader_x;
    view.theronState.party_y = world->party.leader_y;
    view.theronState.party_dir = world->party.leader_dir;
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron exit returns to startup after quest item");
    expect_true(view.theronState.startup_phase ==
                THERON_STARTUP_PHASE_STAGE_SELECT &&
                view.theronState.level_loaded == 0,
                "M11 Theron exit unloads dungeon and shows stage select");
    expect_true(strstr(view.inspectDetail, "dungeon complete") != NULL &&
                strstr(view.inspectDetail, "Chapter 2") != NULL &&
                strstr(view.inspectDetail, "Crypt of Shadows") != NULL,
                "M11 Theron exit inspect readout advances chapter marker");
    expect_true(world->progression.dungeon_states[
                    THERON_DUNGEON_1_HALL_OF_RECORDS - 1] ==
                    THERON_DUNGEON_STATE_COMPLETE &&
                world->progression.dungeon_states[
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] ==
                    THERON_DUNGEON_STATE_AVAILABLE,
                "M11 Theron exit advances progression and unlocks middle stages");
    expect_true(world->party.champion_count == 1 &&
                view.theronState.companion_count == 0,
                "M11 Theron exit clears selected companions before next startup");

    expect_true(view.theronState.selected_dungeon ==
                THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                "M11 Theron stage cursor returns on unlocked dungeon 2");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron stage 2 opens Soul Room");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                M11_GAME_INPUT_REDRAW,
                "M11 Theron stage 2 forcefield loads selected dungeon");
    expect_true(world->object_count == 0 && world->timer_count == 0,
                "M11 Theron stage 2 forcefield clears prior dungeon runtime state");
    expect_true(world->current_dungeon == THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                world->progression.current_dungeon ==
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS &&
                view.theronState.selected_dungeon ==
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS,
                "M11 Theron stage 2 forcefield does not fall back to dungeon 1");
    expect_true(world->level_loaded[
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][0] == 1,
                "M11 Theron stage 2 loads the selected dungeon level slot");
    expect_true(world->levels[
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][0].width == 8 &&
                world->levels[
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1][0].height == 8 &&
                world->progression.dungeon_seeds[
                    THERON_DUNGEON_2_CRYPT_OF_SHADOWS - 1] == 414 &&
                theron_v1_world_get_square(world, 6, 4) == THERON_SQUARE_POOL,
                "M11 Theron stage 2 fallback room carries stage-specific seed and marker");
    expect_true(theron_v1_world_get_square(world,
                                           world->party.leader_x,
                                           world->party.leader_y - 1) ==
                THERON_SQUARE_FLOOR,
                "M11 Theron stage 2 movement samples selected dungeon map");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                M11_GAME_INPUT_REDRAW &&
                view.theronState.party_x == 3 &&
                view.theronState.party_y == 4,
                "M11 Theron stage 2 can move inside selected dungeon map");
    expect_true(world->party.champion_count == 1 &&
                strcmp(world->party.champions[0].name, "Theron") == 0,
                "M11 Theron stage 2 forcefield starts with current Soul Room selection");

    M11_GameView_Shutdown(&view);

#if FIRESTAFF_HAS_ZLIB
    {
        char srm_temp_dir[512];
        char srm_theron_dir[512];
        char srm_track_path[512];
        char srm_root[512];
        char srm_slot_path[512];
        M11_GameViewState srm_view;
        Theron_V1_World* srm_world;

        expect_true(make_temp_dir(srm_temp_dir),
                    "SRM temporary root created");
        snprintf(srm_theron_dir,
                 sizeof(srm_theron_dir),
                 "%s%stheron",
                 srm_temp_dir,
                 PATH_SEP);
        expect_true(TEST_MKDIR(srm_theron_dir) == 0,
                    "SRM theron subdir created");
        snprintf(srm_track_path,
                 sizeof(srm_track_path),
                 "%s%s%s",
                 srm_theron_dir,
                 PATH_SEP,
                 "Theron's Quest (US) (Track 02).bin");
        expect_true(write_file(srm_track_path,
                               "fake-track02-without-bank-markers"),
                    "SRM fake Track 02 file written");
        snprintf(srm_root,
                 sizeof(srm_root),
                 "%s%ssrm",
                 srm_temp_dir,
                 PATH_SEP);
        expect_true(TEST_MKDIR(srm_root) == 0,
                    "SRM save root created");
        snprintf(srm_slot_path,
                 sizeof(srm_slot_path),
                 "%s%sslot1.srm",
                 srm_root,
                 PATH_SEP);
        expect_true(write_bytes(srm_slot_path,
                                g_valid_gzip_srm,
                                sizeof(g_valid_gzip_srm)) == 1,
                    "SRM slot 1 written");
        expect_true(test_setenv_name("FIRESTAFF_THERON_SRM_DIR", NULL),
                    "SRM env root cleared");

        memset(&spec, 0, sizeof(spec));
        spec.title = "THERON'S QUEST";
        spec.gameId = "theron";
        spec.sourceId = "theron";
        spec.dataDir = srm_temp_dir;
        spec.verifiedAssetPath = srm_track_path;
        spec.verifiedAssetMd5 = "f23601102138f87c33025877767ebf76";
        spec.savePath = srm_slot_path;
        spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
        spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
        spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

        M11_GameView_Init(&srm_view);
        expect_true(M11_GameView_Start(&srm_view, &spec),
                    "M11 Theron SRM start succeeds");
        srm_world = (Theron_V1_World*)srm_view.theronWorld;
        expect_true(srm_view.theronState.save_resume_claim ==
                        THERON_V1_STARTUP_RESUME_SRM &&
                    srm_view.theronState.save_resume_srm_active_slot == 1 &&
                    strcmp(srm_view.theronState.save_resume_srm_root,
                           srm_root) == 0 &&
                    srm_view.theronState.save_resume_srm_import_status ==
                        THERON_V1_SRM_PROGRESS_IMPORT_OK,
                    "M11 Theron exposes selected decoded SRM Continue slot");
        expect_true(M11_GameView_HandleInput(&srm_view, M12_MENU_INPUT_UP) ==
                    M11_GAME_INPUT_REDRAW &&
                    srm_view.theronState.save_resume_continue_focus == 1,
                    "M11 Theron SRM Continue row receives focus");
        expect_true(M11_GameView_HandleInput(&srm_view, M12_MENU_INPUT_ACCEPT) ==
                    M11_GAME_INPUT_REDRAW,
                    "M11 Theron SRM Continue loads decoded envelope");
        expect_true(srm_world != NULL &&
                    srm_world->progression.current_dungeon ==
                        THERON_DUNGEON_3_ABYSS_OF_FLAMES &&
                    srm_world->progression.quest_items_collected == 0x03,
                    "M11 Theron SRM Continue applies decoded progression");
        M11_GameView_Shutdown(&srm_view);
    }
#endif

    if (g_failures) {
        fprintf(stderr, "Theron V1 M11 direct-launch checks FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("Theron V1 M11 direct-launch checks passed");
    return 0;
}
