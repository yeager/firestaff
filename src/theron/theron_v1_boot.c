/*
 * theron_v1_boot.c — Theron's Quest V1 Boot Profile Implementation
 *
 * Phase 1: Runtime profile split for Theron's Quest.
 * Separates TQ boot from DM1/CSB/DM2/Nexus, including:
 *   - Asset discovery (PC Engine HuCard data format)
 *   - Menu launch routing (M12 game-card → boot profile pipeline)
 *   - Save namespace (saves/theron/ — between-dungeon only)
 *   - Platform/region diagnostics (PC Engine JP / TurboGrafx-16 US)
 *   - Deterministic config (PC Engine fixed-tick, no chivalry)
 *   - No in-dungeon saves (TQ design restriction — save at dungeon entrance only)
 *
 * Provenance gate:
 *   The supported launch source is the hash-verified Track 02 BIN
 *   route. Generic legacy asset probing remains diagnostic only and
 *   must not authorize production rendering.
 *
 * Platform reference:
 *   PC Engine HuCard — Hudson Soft, 1992 (JP) / 1993 (US)
 *   CPU: 7.16 MHz HuC6280 (65C02 derivative, 8-bit bus)
 *   Resolution: 256x224 (NTSC) / 320x224 hires modes
 *   Palette: 512 colors, 16-color/sprite tiles, 64 sprites max
 *   Memory map: HuCard ROM mapped at 0xE000-0xFFFF, SRAM at 0xC000
 *
 * Source references:
 *   THQUEST.ASM T000  — title/startup entry
 *   THQUEST.ASM T200  — platform detection / version label
 *   THQUEST.ASM T400  — dungeon bank loading (HuCard ROM mapping)
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T800  — champion persistence between dungeons
 *   THQUEST.ASM T080  — between-dungeon save/load (no in-dungeon)
 */

#include "theron_v1_boot.h"
#include "theron_v1_asset_loader.h"
#include "asset_find_by_hash.h"
#include "firestaff_theron_media_classify.h"
#include "theron_v1_mechanics.h"
#include "theron_v1_stage2_runtime_handoff.h"
#include "theron_v1_startup_runtime_entry.h"
#include "theron_v2_hud_launch_mode_pc34.h"
#include "theron_v2_hud_overlay_pc34.h"
#include "theron_v2_hud_widget_assets_pc34.h"
#include "menu_input_m12.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/* ── Path separator ─────────────────────────────────────────────── */

#if defined(_WIN32)
#define TRV_PATH_SEP '\\'
#else
#define TRV_PATH_SEP '/'
#endif

#define THERON_V1_RUNTIME_TRACE_MAX_BYTES (1024u * 1024u)
#define THERON_V1_RUNTIME_MEDIA_MAX_BYTES (512u * 1024u * 1024u)
#define THERON_V1_RUNTIME_CAPTURE_MANIFEST_MAX_BYTES 4096u

static unsigned char *theron_v1_boot_read_evidence_file(
    const char *path, size_t maximum_bytes, size_t *out_size) {
    struct stat st;
    FILE *file = NULL;
    unsigned char *bytes;

    if (!path || !path[0] || !out_size || stat(path, &st) != 0 ||
        !S_ISREG(st.st_mode) || st.st_size <= 0 ||
        (uintmax_t)st.st_size > maximum_bytes ||
        !(file = fopen(path, "rb")) ||
        !(bytes = (unsigned char *)malloc((size_t)st.st_size + 1u))) {
        if (file) fclose(file);
        return NULL;
    }
    if (fread(bytes, 1u, (size_t)st.st_size, file) != (size_t)st.st_size) {
        fclose(file);
        free(bytes);
        return NULL;
    }
    fclose(file);
    bytes[st.st_size] = '\0';
    *out_size = (size_t)st.st_size;
    return bytes;
}

static int theron_v1_boot_md5_is_canonical(const char *md5_hex) {
    size_t index;

    if (!md5_hex || strlen(md5_hex) != 32u) return 0;
    for (index = 0u; index < 32u; ++index) {
        const char value = md5_hex[index];
        if (!((value >= '0' && value <= '9') ||
              (value >= 'a' && value <= 'f'))) {
            return 0;
        }
    }
    return 1;
}

int theron_v1_boot_runtime_trace_files_match_declared_hashes(
    const char *track02_path,
    const char *track02_md5_hex,
    const char *system_card_path,
    const char *system_card_md5_hex,
    const char *trace_path,
    const char *trace_md5_hex) {
    char actual_track02_md5[33];
    char actual_system_card_md5[33];
    char actual_trace_md5[33];

    if (!track02_path || !track02_md5_hex || !system_card_path ||
        !system_card_md5_hex || !trace_path || !trace_md5_hex ||
        !m12_file_md5_hex(track02_path,
                                                   actual_track02_md5) ||
        !m12_file_md5_hex(system_card_path, actual_system_card_md5) ||
        !m12_file_md5_hex(trace_path, actual_trace_md5)) {
        return 0;
    }
    return strcmp(actual_track02_md5, track02_md5_hex) == 0 &&
           strcmp(actual_system_card_md5, system_card_md5_hex) == 0 &&
           strcmp(actual_trace_md5, trace_md5_hex) == 0;
}

int theron_v1_boot_track02_runtime_trace_intake_from_files(
    const char *track02_path,
    const char *track02_md5_hex,
    const char *system_card_path,
    const char *system_card_md5_hex,
    const char *trace_path,
    const char *trace_md5_hex,
    Theron_V1_BootTrack02RuntimeTraceIntakeReceipt *out_receipt) {
    unsigned char *trace = NULL;
    unsigned char *track02 = NULL;
    unsigned char *system_card = NULL;
    size_t trace_size = 0u;
    size_t track02_size = 0u;
    size_t system_card_size = 0u;
    int accepted = 0;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    /* The parser below receives bytes after these hashes are checked. The
     * explicit rehash prevents a caller from pairing a known hash label with
     * changed Track 02, System Card, or Mednafen trace evidence. */
    if (!theron_v1_boot_runtime_trace_files_match_declared_hashes(
            track02_path, track02_md5_hex, system_card_path,
            system_card_md5_hex, trace_path, trace_md5_hex)) {
        return 0;
    }
    out_receipt->trace_file_hash_verified = 1;
    snprintf(out_receipt->trace_md5, sizeof(out_receipt->trace_md5), "%s",
             trace_md5_hex);
    out_receipt->system_card_file_hash_verified = 1;
    snprintf(out_receipt->system_card_md5,
             sizeof(out_receipt->system_card_md5), "%s",
             system_card_md5_hex);
    trace = theron_v1_boot_read_evidence_file(
        trace_path, THERON_V1_RUNTIME_TRACE_MAX_BYTES, &trace_size);
    if (!trace || trace_size == 0u) goto done;
    (void)theron_v1_system_card_controller_wait_from_mednafen_capture(
        (const char *)trace, &out_receipt->controller_wait);
    track02 = theron_v1_boot_read_evidence_file(
        track02_path, THERON_V1_RUNTIME_MEDIA_MAX_BYTES, &track02_size);
    system_card = theron_v1_boot_read_evidence_file(
        system_card_path, THERON_V1_RUNTIME_MEDIA_MAX_BYTES,
        &system_card_size);
    if (!track02 || !system_card ||
        !theron_v1_irq2_live_branch_from_mednafen_capture_and_full_track02_media(
            track02, track02_size, track02_md5_hex, system_card,
            system_card_size, system_card_md5_hex, (const char *)trace,
            &out_receipt->runtime_handoff)) {
        /* Preserve only the independently proven controller wait.  It is a
         * diagnostic receipt and cannot be mistaken for a runtime handoff. */
        out_receipt->valid = 0;
        out_receipt->trace_file_consumed = 0;
        memset(&out_receipt->runtime_handoff, 0,
               sizeof(out_receipt->runtime_handoff));
        goto done;
    }
    out_receipt->valid = 1;
    out_receipt->trace_file_consumed = 1;
    accepted = 1;

done:
    free(trace);
    free(track02);
    free(system_card);
    return accepted;
}

int theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
    const Theron_V1_BootProfile *profile) {
    Theron_Track02Variant expected_variant;
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *initial_level;

    if (!profile || !profile->track02_runtime_trace_handoff_ready) {
        return 0;
    }
    expected_variant = theron_v1_track02_variant_for_md5(
        profile->graphics_md5);
    initial_level = &profile->track02_initial_level_handoff;
    return (expected_variant == THERON_TRACK02_VARIANT_JP_BIN ||
            expected_variant == THERON_TRACK02_VARIANT_US_BIN) &&
           theron_v1_boot_md5_is_canonical(
               profile->track02_runtime_system_card_md5) &&
           theron_v1_boot_md5_is_canonical(
               profile->track02_runtime_trace_md5) &&
           initial_level->valid &&
           initial_level->variant == expected_variant &&
           strcmp(initial_level->track02_md5, profile->graphics_md5) == 0 &&
           initial_level->observed_track02_record == 0x0b52u &&
           initial_level->loader_intake.track02_variant == expected_variant &&
           initial_level->loader_payload.handed_off &&
           initial_level->loader_payload.no_fallback &&
           initial_level->loader_payload.track02_variant == expected_variant &&
           initial_level->loader_payload.record ==
               initial_level->observed_track02_record &&
           initial_level->loader_payload.payload_bytes ==
               THERON_TRACK02_RAW_USER_DATA_BYTES &&
           initial_level->loader_payload.payload_checksum ==
               initial_level->complete_payload_checksum &&
           initial_level->loader_post_envelope.handed_off &&
           initial_level->loader_post_envelope.no_fallback &&
           initial_level->loader_post_envelope.track02_variant == expected_variant &&
           initial_level->loader_post_envelope.record ==
               initial_level->observed_track02_record &&
           initial_level->loader_post_envelope.record_user_data_offset ==
               initial_level->initial_level_boundary
                   .object_boundary_user_data_offset_in_record &&
           initial_level->loader_post_envelope.byte_count ==
               initial_level->initial_level_boundary
                   .following_user_data_bytes_in_record &&
           initial_level->loader_post_envelope.checksum ==
               initial_level->initial_level_boundary.following_user_data_hash &&
           !initial_level->object_tail_semantics_proven &&
           !initial_level->fallback_visuals_allowed &&
           theron_v1_raw_loader_trace_manifest_initial_level_handoff_is_complete(
               initial_level);
}

int theron_v1_boot_track02_capture_admission_allows_initial_level(
    const Theron_V1_BootProfile *profile,
    const uint8_t *track02_data,
    size_t track02_size,
    int dungeon_id,
    int sub_level_index) {
    Theron_Track02InitialLevelLoaderRoute route;
    char observed_track02_md5[33];
    const Theron_V1RawLoaderTraceInitialLevelHandoffReceipt *initial_level;

    if (!profile || !track02_data || track02_size == 0u ||
        !profile->graphics_path[0] || !profile->graphics_md5[0] ||
        !theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
            profile) ||
        !m12_file_md5_hex(profile->graphics_path, observed_track02_md5) ||
        strcmp(observed_track02_md5, profile->graphics_md5) != 0) {
        return 0;
    }

    initial_level = &profile->track02_initial_level_handoff;
    if (!initial_level->initial_level_semantics_proven) {
        return 0;
    }
    memset(&route, 0, sizeof(route));
    if (theron_v1_track02_load_initial_level_loader_route(
            track02_data, track02_size, profile->graphics_md5,
            dungeon_id, sub_level_index, &route) != THERON_TRACK02_SIGNAL_OK ||
        !route.valid ||
        route.route_hash != initial_level->initial_level_route.route_hash ||
        route.dungeon_id != initial_level->initial_level_route.dungeon_id ||
        route.sub_level_index !=
            initial_level->initial_level_route.sub_level_index ||
        route.semantics.envelope.track02_record !=
            initial_level->observed_track02_record ||
        route.semantics.envelope.track02_record !=
            initial_level->initial_level_boundary.track02_record ||
        route.object_tail_semantics_proven || route.fallback_visuals_allowed) {
        return 0;
    }
    /* The source-locked stage-two code jumps to $3800 after this sector is
     * loaded. Its level-shaped subrange remains observational, not a game
     * level/object consumer, so no dungeon handoff may be admitted yet. */
    return 0;
}

/* ── PC Engine file candidates ───────────────────────────────────── */

/* Theron's Quest data files — Phase 0 locked (2026-05-27).
 *
 * CD-ROM disc structure: CUE/BIN or CUE/ISO, 18 tracks, Track 02 = data track.
 * The data track contains the HuC6280 executable, graphics tiles
 * (tile/sprite format), and dungeon data — embedded in one binary blob
 * per region.  Unlike DM1's dual-file structure (GRAPHICS.DAT +
 * DUNGEON.DAT), Theron's Quest uses a single Track 02 per region.
 *
 * JP Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
 * US Track 02 MD5: f23601102138f87c33025877767ebf76
 * JP Rev 1 Track 02 ISO MD5: 397039af02d50d15c70b74088eb8a1cb
 * US Track 02 ISO MD5:       ceb02343868f80cec899e9b239aff2da
 * Source: cdromance.org (2026-05-27)
 * Additional ISO names: MyAbandonware TG-CD English/Japanese Rev 1 page
 * checked 2026-06-03.
 *
 * Subdir candidates: theron/jp/, theron/us/, theron/
 *
 * File candidates (in order of precedence):
 *   track02.bin                    — canonical CD-ROM data track name
 *   track02.iso                    — ISO 2048-byte sector Track 02
 *   "Theron's Quest (Japan) (Track 02).bin"
 *   "Theron's Quest (US) (Track 02).bin"
 *   TQJP02.iso / TQJP02End.iso     — MyAbandonware JP Rev 1 Track 02
 *   TQUS02.iso / TQUS02End.iso     — MyAbandonware US Track 02
 *   THQUEST.BIN
 *   No legacy GRAPHICS.DAT / DUNGEON.DAT fallback is accepted here; only
 *   hash-verified Track 02 media is a launchable Theron source.
 */
static const char *const g_theron_track02_candidates[] = {
    "track02.bin",
    "track02.iso",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "Theron's Quest (Japan) (Track 02).iso",
    "Theron's Quest (US) (Track 02).iso",
    "TQJP02.bin",
    "TQUS02.bin",
    "TQJP02.iso",
    "TQJP02End.iso",
    "TQUS02.iso",
    "TQUS02End.iso",
    "THQUEST.BIN",
    NULL
};

/* Recognised Track 02 MD5s for Theron's Quest.  Mirrors the four hashes
 * in asset_status_m12.c::g_theronVersions. */
static const char *const g_theron_known_md5s[] = {
    "b7afb338ad31be1025b53f9aff12d73a", /* JP Track 02 BIN */
    "f23601102138f87c33025877767ebf76", /* US Track 02 BIN */
    "397039af02d50d15c70b74088eb8a1cb", /* JP Rev 1 ISO */
    "ceb02343868f80cec899e9b239aff2da", /* US ISO */
    NULL
};

/* ── Probe: file exists and non-empty ─────────────────────────────── */

/* Process-local counter of stat() / file-probe calls performed by the
 * boot module.  Used by the direct-launch regression test to assert
 * that theron_v1_boot_load_verified_path() does NOT re-walk the data
 * root.  Initialised to 0 and incremented by every file_exists() call. */
static unsigned long g_theron_rescan_count = 0UL;

unsigned long theron_v1_boot_rescan_call_count(void) {
    return g_theron_rescan_count;
}

void theron_v1_boot_rescan_call_count_reset(void) {
    g_theron_rescan_count = 0UL;
}

static int file_exists(const char *path) {
    struct stat st;
    g_theron_rescan_count++;
    return stat(path, &st) == 0 && st.st_size > 0;
}

static size_t file_size_of(const char *path) {
    struct stat st;
    g_theron_rescan_count++;
    if (stat(path, &st) != 0) return 0;
    return (size_t)st.st_size;
}

/* ── Resolve asset path (probe all candidates in order) ───────────── */

static int resolve_asset(const char *base_dir,
                          const char *subdir,
                          const char *const candidates[],
                          char resolved[512],
                          size_t *out_size) {
    char path[512];
    size_t i;
    for (i = 0; candidates[i]; i++) {
        if (subdir && subdir[0]) {
            snprintf(path, sizeof(path), "%s%c%s%c%s",
                     base_dir, TRV_PATH_SEP, subdir, TRV_PATH_SEP, candidates[i]);
        } else {
            snprintf(path, sizeof(path), "%s%c%s",
                     base_dir, TRV_PATH_SEP, candidates[i]);
        }
        if (file_exists(path)) {
            strncpy(resolved, path, 511);
            resolved[511] = '\0';
            if (out_size) *out_size = file_size_of(path);
            return 1;
        }
    }
    return 0;
}

/* ── Platform label table ──────────────────────────────────────────── */

static const char *const g_platform_labels[THERON_PLATFORM_COUNT] = {
    [THERON_PLATFORM_PCE_JP] = "PC Engine HuCard (JP)",
    [THERON_PLATFORM_PCE_US] = "TurboGrafx-16 HuCard (US)",
};

static void theron_v1_boot_apply_known_md5_identity(
    Theron_V1_BootProfile *profile,
    const char *md5)
{
    if (!profile || !md5) {
        return;
    }
    if (strcmp(md5, "b7afb338ad31be1025b53f9aff12d73a") == 0 ||
        strcmp(md5, "397039af02d50d15c70b74088eb8a1cb") == 0) {
        profile->platform = THERON_PLATFORM_PCE_JP;
        strncpy(profile->platform_label,
                g_platform_labels[THERON_PLATFORM_PCE_JP],
                sizeof(profile->platform_label) - 1);
        strncpy(profile->version_id, "pce-jp", sizeof(profile->version_id) - 1);
    } else {
        profile->platform = THERON_PLATFORM_PCE_US;
        strncpy(profile->platform_label,
                g_platform_labels[THERON_PLATFORM_PCE_US],
                sizeof(profile->platform_label) - 1);
        strncpy(profile->version_id, "pce-en", sizeof(profile->version_id) - 1);
    }
    if (strcmp(md5, "397039af02d50d15c70b74088eb8a1cb") == 0) {
        strncpy(profile->version_id, "pce-jp-rev1-iso",
                sizeof(profile->version_id) - 1);
    } else if (strcmp(md5, "ceb02343868f80cec899e9b239aff2da") == 0) {
        strncpy(profile->version_id, "pce-en-iso",
                sizeof(profile->version_id) - 1);
    }
}

/* A complete CUE package contains more provenance than a loose Track 02.
 * Use it first when it declares one readable MODE1 data track whose bytes are
 * still pinned to the canonical hash catalog. Unknown or malformed CUE files
 * do not block the existing hash-first loose/container scan. */
static int theron_v1_boot_apply_verified_track02_path(
    Theron_V1_BootProfile *profile,
    const char *track02_path)
{
    char md5[33] = {0};
    size_t i;

    if (!profile || !track02_path || !track02_path[0] ||
        !m12_file_md5_hex(track02_path, md5)) {
        return 0;
    }
    for (i = 0u; g_theron_known_md5s[i]; ++i) {
        if (strcmp(md5, g_theron_known_md5s[i]) == 0) {
            break;
        }
    }
    if (!g_theron_known_md5s[i]) {
        return 0;
    }
    snprintf(profile->graphics_path, sizeof(profile->graphics_path), "%s",
             track02_path);
    snprintf(profile->dungeon_path, sizeof(profile->dungeon_path), "%s",
             track02_path);
    profile->graphics_size = file_size_of(profile->graphics_path);
    profile->dungeon_size = profile->graphics_size;
    snprintf(profile->graphics_md5, sizeof(profile->graphics_md5), "%s", md5);
    snprintf(profile->dungeon_md5, sizeof(profile->dungeon_md5), "%s", md5);
    profile->assets_verified = 1;
    theron_v1_boot_apply_known_md5_identity(profile, md5);
    return 1;
}

static int theron_v1_boot_apply_verified_cue_package(
    Theron_V1_BootProfile *profile,
    const FirestaffTheronMediaStatus *media)
{
    if (!profile || !media || !media->has_cue ||
        !media->has_valid_track02_mode1 || !media->cue_path[0] ||
        !media->track02_path[0] ||
        !theron_v1_boot_apply_verified_track02_path(profile,
                                                     media->track02_path)) {
        return 0;
    }
    snprintf(profile->track02_cue_path, sizeof(profile->track02_cue_path), "%s",
             media->cue_path);
    profile->track02_cue_consumed = 1;
    return 1;
}

static int theron_v1_boot_scan_verified_cue_package(
    Theron_V1_BootProfile *profile,
    const char *base)
{
    FirestaffTheronMediaStatus media;

    if (!profile || !base) {
        return 0;
    }
    /* A file-picker can hand boot the CUE itself.  Classify that exact sheet
     * before considering a directory scan so the selected Track 01/02 pair
     * remains the only package consulted.  Both routes retain the same strict
     * MODE1, payload and canonical-MD5 gates. */
    if (FirestaffTheronMedia_ClassifyPath(base, &media) == 0 &&
        theron_v1_boot_apply_verified_cue_package(profile, &media)) {
        return 1;
    }
    if (FirestaffTheronMedia_ClassifyDirectory(base, &media) == 0 &&
        theron_v1_boot_apply_verified_cue_package(profile, &media)) {
        return 1;
    }
    return 0;
}

static int theron_v1_boot_scan_verified_track02_file(
    Theron_V1_BootProfile *profile,
    const char *path)
{
    FirestaffTheronMediaStatus media;

    if (!profile || !path ||
        FirestaffTheronMedia_ClassifyPath(path, &media) != 0 ||
        media.has_cue || !media.has_track02_data || !media.candidate_path[0]) {
        return 0;
    }
    return theron_v1_boot_apply_verified_track02_path(profile,
                                                        media.candidate_path);
}

/* ── Init defaults ────────────────────────────────────────────────── */

void theron_v1_boot_profile_init(Theron_V1_BootProfile *profile) {
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));

    strncpy(profile->game_id, "theron", sizeof(profile->game_id) - 1);
    profile->platform = THERON_PLATFORM_PCE_JP;
    strncpy(profile->platform_label,
            g_platform_labels[THERON_PLATFORM_PCE_JP],
            sizeof(profile->platform_label) - 1);
    strncpy(profile->version_id, "pce-jp", sizeof(profile->version_id) - 1);

    profile->in_dungeon_save_allowed = 0; /* TQ design restriction */

    /* Deterministic defaults: PC Engine VBlank tick rate (18.2 Hz = same as DM1/CSB) */
    profile->deterministic.tick_rate_hz      = 18;
    profile->deterministic.tick_rate_hz_frac = 2;   /* 18.2 Hz */
    profile->deterministic.tick_ms           = 55;  /* ~55ms per tick */
    profile->deterministic.dungeon_move_speed = 0x0080; /* Q8: 0.5 sq/tick */
    profile->deterministic.outdoor_move_speed = 0x0080; /* Q8: same (no outdoor) */
    profile->deterministic.max_champions      = 4;   /* Theron + 3 champions */
    profile->deterministic.max_party_members = 4;
    profile->deterministic.dungeon_count      = 7;   /* 7 mini-dungeons */
    profile->deterministic.max_levels         = 3;   /* ~3 levels per mini-dungeon */
    profile->dungeon_size = 0;
    profile->graphics_size = 0;
    /* Do not seed runtime state with the retired placeholder value 313.
     * T560 header parsing fills this only after a verified original header
     * is supplied; the real Track 02 startup candidate carries its own
     * 32-bit level seed and is bound by the Track 02 handoff path. */
    profile->deterministic.dungeon_seed       = 0;
    profile->deterministic.quest_items_collected = 0;
}

/* ── Scan assets ──────────────────────────────────────────────────── */

/*
 * theron_v1_boot_scan_assets — probe for TQ assets.
 *
 * Searches data_dir/theron/ for Track 02 BIN (CD-ROM data track).
 * Phase 0: verifies Track 02 against known JP/US MD5 hashes.
 * Unverified extracted GRAPHICS.DAT / DUNGEON.DAT files are not launchable.
 *
 * Returns 0 on success (assets found), -1 if none detected.
 */
int theron_v1_boot_scan_assets(Theron_V1_BootProfile *profile,
                                const char *data_dir) {
    const char *base = data_dir && data_dir[0] ? data_dir : ".";
    int match_index = -1;

    if (!profile) {
        return -1;
    }

    /* Primary path: find Track 02 by MD5 anywhere under the data root.
     * THQUEST.ASM T400 cares about the verified data track bytes, not
     * whether the host file is named track02.bin. */
    if (theron_v1_boot_scan_verified_cue_package(profile, base)) {
        /* Strict CUE package selected above; retain its source provenance. */
    } else if (theron_v1_boot_scan_verified_track02_file(profile, base)) {
        /* An explicit BIN/ISO picker selection is already the complete,
         * hash-verified Track 02 source. Do not recurse through its parent. */
    } else if (asset_find_by_md5_list(base,
                               g_theron_known_md5s,
                               profile->graphics_path,
                               (int)sizeof(profile->graphics_path),
                               &match_index,
                               8)) {
        strncpy(profile->dungeon_path, profile->graphics_path,
                sizeof(profile->dungeon_path) - 1);
        profile->dungeon_path[sizeof(profile->dungeon_path) - 1] = '\0';
        profile->graphics_size = file_size_of(profile->graphics_path);
        profile->dungeon_size = profile->graphics_size;
        strncpy(profile->graphics_md5,
                g_theron_known_md5s[match_index],
                sizeof(profile->graphics_md5) - 1);
        strncpy(profile->dungeon_md5,
                g_theron_known_md5s[match_index],
                sizeof(profile->dungeon_md5) - 1);
        profile->assets_verified = 1;
        theron_v1_boot_apply_known_md5_identity(
            profile,
            g_theron_known_md5s[match_index]);
    } else if (resolve_asset(base, "theron", g_theron_track02_candidates,
                       profile->graphics_path,
                       &profile->graphics_size)) {
        /* Track 02 found — same file for graphics + dungeon */
        strncpy(profile->dungeon_path, profile->graphics_path,
                sizeof(profile->dungeon_path) - 1);
        profile->dungeon_size = profile->graphics_size;
    } else if (resolve_asset(base, "theron/jp", g_theron_track02_candidates,
                              profile->graphics_path,
                              &profile->graphics_size)) {
        strncpy(profile->dungeon_path, profile->graphics_path,
                sizeof(profile->dungeon_path) - 1);
        profile->dungeon_size = profile->graphics_size;
    } else if (resolve_asset(base, "theron/us", g_theron_track02_candidates,
                              profile->graphics_path,
                              &profile->graphics_size)) {
        strncpy(profile->dungeon_path, profile->graphics_path,
                sizeof(profile->dungeon_path) - 1);
        profile->dungeon_size = profile->graphics_size;
    } else if (resolve_asset(base, "", g_theron_track02_candidates,
                              profile->graphics_path,
                              &profile->graphics_size)) {
        strncpy(profile->dungeon_path, profile->graphics_path,
                sizeof(profile->dungeon_path) - 1);
        profile->dungeon_size = profile->graphics_size;
    }

    /* Build asset root */
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "%s%ctheron", base, TRV_PATH_SEP);

    /* Phase 0 gate: verify Track 02 MD5 against known hashes.
     * JP: b7afb338ad31be1025b53f9aff12d73a
     * US: f23601102138f87c33025877767ebf76
     * JP Rev 1 ISO: 397039af02d50d15c70b74088eb8a1cb
     * US ISO:       ceb02343868f80cec899e9b239aff2da */
    if (profile->graphics_path[0]) {
        char md5hex[33] = {0};
        if (m12_file_md5_hex(profile->graphics_path, md5hex)) {
            if (strcmp(md5hex, "b7afb338ad31be1025b53f9aff12d73a") == 0 ||
                strcmp(md5hex, "f23601102138f87c33025877767ebf76") == 0 ||
                strcmp(md5hex, "397039af02d50d15c70b74088eb8a1cb") == 0 ||
                strcmp(md5hex, "ceb02343868f80cec899e9b239aff2da") == 0) {
                profile->assets_verified = 1;
                strncpy(profile->graphics_md5, md5hex, 32);
                profile->graphics_md5[32] = '\0';
                strncpy(profile->dungeon_md5, md5hex, 32);
                profile->dungeon_md5[32] = '\0';
                theron_v1_boot_apply_known_md5_identity(profile, md5hex);
            } else {
                profile->assets_verified = 0;
            }
        } else {
            profile->assets_verified = 0;
        }
    } else {
        profile->assets_verified = 0;
    }

    /* Filename heuristics are only diagnostic for unverified legacy
     * candidates. Hash-verified Track 02 identity above is authoritative. */
    if (!profile->assets_verified) {
        if (strstr(profile->graphics_path, "Japan") ||
            strstr(profile->graphics_path, "TQJP") ||
            strstr(profile->graphics_path, "pce-jp")) {
            profile->platform = THERON_PLATFORM_PCE_JP;
            strncpy(profile->platform_label,
                    g_platform_labels[THERON_PLATFORM_PCE_JP],
                    sizeof(profile->platform_label) - 1);
            strncpy(profile->version_id, "pce-jp", sizeof(profile->version_id) - 1);
        } else if (strstr(profile->graphics_path, "US") ||
                   strstr(profile->graphics_path, "TQUS") ||
                   strstr(profile->graphics_path, "pce-en")) {
            profile->platform = THERON_PLATFORM_PCE_US;
            strncpy(profile->platform_label,
                    g_platform_labels[THERON_PLATFORM_PCE_US],
                    sizeof(profile->platform_label) - 1);
            strncpy(profile->version_id, "pce-en", sizeof(profile->version_id) - 1);
        }
    }

    /* Require both graphics and dungeon before returning success */
    if (profile->graphics_path[0] && profile->dungeon_path[0]) {
        return 0; /* success */
    }
    return -1;   /* missing assets */
}

/* ── Probe availability ───────────────────────────────────────────── */

/*
 * theron_v1_boot_probe_available — quick probe without full scan.
 * Used by M12 launcher menu to show TQ availability.
 */
int theron_v1_boot_probe_available(const char *data_dir) {
    Theron_V1_BootProfile profile;
    theron_v1_boot_profile_init(&profile);
    return theron_v1_boot_scan_assets(&profile, data_dir) == 0 &&
           profile.assets_verified ? 1 : 0;
}

/* ── Direct launch (verified path) ────────────────────────────────── */

static int theron_md5_is_known(const char *md5) {
    size_t i;
    if (!md5) return 0;
    for (i = 0; g_theron_known_md5s[i]; ++i) {
        if (strcmp(md5, g_theron_known_md5s[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

/*
 * theron_v1_boot_verified_path_is_stale -- decide whether a
 * previously verified Track 02 path/MD5 pair still matches the
 * bytes on disk.  Mirrors src/shared/asset_status_m12.c
 * ::m12_theron_tracked_path_is_stale so the boot profile and the M12
 * reuse gate agree on what counts as "stale".
 *
 *   1 -- path is missing, no longer a regular file, or its on-disk
 *        MD5 no longer matches expected_md5.  The cached verified
 *        entry is unsafe to trust; callers should fall through to a
 *        full scan.
 *   0 -- path exists as a regular file and its current MD5 matches.
 *
 * Counts one stat() in g_theron_rescan_count so callers can assert
 * the helper does no work beyond the existence / hash check.
 */
int theron_v1_boot_verified_path_is_stale(const char *track02_path,
                                           const char *expected_md5) {
    char fresh_md5[33];
    char payload_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    if (!track02_path || !track02_path[0]) return 1;
    if (!expected_md5 || !expected_md5[0]) return 1;
    if (theron_v1_track02_resolve_media_path(track02_path, payload_path) !=
            THERON_TRACK02_SIGNAL_OK ||
        !file_exists(payload_path)) {
        return 1;
    }
    /* Re-hash the file: same-size-different-content swaps pass an
     * mtime/size check but must not pass the verified-launch contract.
     * Using the same m12_file_md5_hex helper that the M12 reuse gate
     * uses keeps both sides byte-comparable.  Expected cost on a real
     * Track 02 BIN is one full MD5 over a few-MB payload; this is the
     * correct price for "reuse safely" rather than "reuse blindly". */
    if (!m12_file_md5_hex(payload_path, fresh_md5)) {
        return 1;
    }
    if (strcmp(fresh_md5, expected_md5) != 0) {
        return 1;
    }
    return 0;
}

/*
 * theron_v1_boot_load_verified_path — fill a boot profile from an
 * already-hash-verified Track 02 path.  Skips the data-dir fallback
 * search that theron_v1_boot_scan_assets() performs.
 *
 * Contract:
 *   - track02_path must be a non-empty path.
 *   - expected_md5 must be a 32-char hex string matching one of the
 *     four known TQ Track 02 MD5s in g_theron_known_md5s.
 *   - The caller owns the verification step (M12 asset catalog, or
 *     the upstream m12_try_match_direct_theron_request() helper).
 *
 * Stale-path guard: refuses (-1) if track02_path no longer exists
 * as a regular file.  This is a one-shot stat() check (counted in
 * theron_v1_boot_rescan_call_count()) so direct launch cannot
 * silently accept a path whose file has been deleted/moved between
 * the previous catalog scan and the current launch.
 *
 * The success path performs ZERO additional stat() calls — the data
 * fields stay empty, the deterministic defaults are reset, and the
 * caller can rely on assets_verified == 1 + the exact MD5 it passed
 * in.  Callers can read theron_v1_boot_rescan_call_count() before
 * and after to assert the optimisation.
 *
 * Source: THQUEST.ASM T400 — once the verified data track is located
 * the runtime does not re-walk the data root. */
int theron_v1_boot_load_verified_path(Theron_V1_BootProfile *profile,
                                       const char *track02_path,
                                       const char *expected_md5) {
    char payload_path[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    const char *resolved_path;
    const char *resolved_md5 = expected_md5;
    char parent[512];
    char grandparent[512];
    const char *base;

    if (!profile || !track02_path || !track02_path[0]) return -1;
    if (!expected_md5 || !expected_md5[0]) return -1;
    if (!theron_md5_is_known(expected_md5)) return -1;

    if (theron_v1_track02_resolve_media_path(track02_path, payload_path) !=
        THERON_TRACK02_SIGNAL_OK) return -1;
    resolved_path = payload_path;

    /* Stale-path guard: refuse if the previously verified file is no
     * longer on disk as a regular file.  Without this, the boot
     * profile would happily report assets_verified = 1 with a path
     * that points at nothing, and the downstream M11_Theron_Load
     * would fail at asset-load time with a less actionable error.
     * One stat() per direct-launch call; the rescan counter is the
     * way tests prove the rest of the function still skips the data
     * root.  Bumping the counter via file_exists() (rather than open-
     * coding stat()) keeps the count consistent with the scan path. */
    if (!file_exists(resolved_path)) {
        return -1;
    }

    /* Re-initialise to defaults, then fill in only the fields a direct
     * launch needs.  Avoids the fallback chain + MD5 stat call. */
    theron_v1_boot_profile_init(profile);

    /* Track 02 is the single Theron data blob — same file for both
     * graphics and dungeon roles.  See include/theron_v1_boot.h for
     * the format description. */
    strncpy(profile->graphics_path, resolved_path,
            sizeof(profile->graphics_path) - 1);
    profile->graphics_path[sizeof(profile->graphics_path) - 1] = '\0';
    strncpy(profile->dungeon_path, resolved_path,
            sizeof(profile->dungeon_path) - 1);
    profile->dungeon_path[sizeof(profile->dungeon_path) - 1] = '\0';

    /* Caller already hashed the file; we don't stat to learn the size
     * in the success path.  If the file size is needed for downstream
     * code it can be re-measured at load time inside tr_asset_load().
     * Leaving the *_size fields at 0 here is acceptable — the boot
     * profile only reads them for diagnostics (theron_v1_diagnostic_
     * report), not for runtime dispatch. */
    profile->graphics_size = 0;
    profile->dungeon_size  = 0;

    /* Caller-supplied verification already matched a known MD5. */
    strncpy(profile->graphics_md5, resolved_md5, 32);
    profile->graphics_md5[32] = '\0';
    strncpy(profile->dungeon_md5, resolved_md5, 32);
    profile->dungeon_md5[32] = '\0';
    profile->assets_verified = 1;

    /* Derive a parent-only asset_root.  We strip the trailing path
     * component with hand-rolled code so we do not depend on a libc
     * dirname() that may modify its input or not be thread-safe.
     * Layout used:
     *   <root>/track02.bin      -> <root>
     *   <root>/theron/track02.bin -> <root>  (we do not collapse
     *       trailing /theron/ here; the launcher passes the runtime
     *       data root, not the per-game subdir, in direct launch).
     *   <root>/theron/jp/x.bin  -> <root>  (drop the last two
     *       components when the path ends in theron/jp or theron/us).
     */
    base = resolved_path;
    {
        size_t plen = strlen(resolved_path);
        const char *slash;
        if (plen >= sizeof(parent)) plen = sizeof(parent) - 1;
        memcpy(parent, resolved_path, plen);
        parent[plen] = '\0';
        slash = strrchr(parent, '/');
        if (!slash) slash = strrchr(parent, '\\');
        if (slash) {
            size_t par_len = (size_t)(slash - parent);
            memcpy(grandparent, parent, par_len);
            grandparent[par_len] = '\0';
            base = grandparent;
        } else {
            base = ".";
        }
    }
    snprintf(profile->asset_root, sizeof(profile->asset_root), "%s",
             base);

    theron_v1_boot_apply_known_md5_identity(profile, expected_md5);

    return 0;
}

/* ── Save root ────────────────────────────────────────────────────── */

void theron_v1_boot_set_save_root(Theron_V1_BootProfile *profile,
                                    const char *save_dir) {
    if (!profile) return;
    if (save_dir && save_dir[0]) {
        strncpy(profile->save_root, save_dir, sizeof(profile->save_root) - 1);
    } else {
        /* Default: <data_dir>/../saves/theron/ */
        /* Note: saves/theron/ is distinct from saves/dm1/, saves/csb/,
         * saves/dm2/, saves/nexus/ — TQ has its own save namespace. */
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s%c..%csaves%ctheron",
                 profile->asset_root[0] ? profile->asset_root : ".",
                 TRV_PATH_SEP, TRV_PATH_SEP, TRV_PATH_SEP);
    }
}

static int theron_v1_boot_save_root_from_save_path(const char *save_path,
                                                   char *out_root,
                                                   size_t out_root_cap) {
    const char *slash;
    size_t len;

    if (!out_root || out_root_cap == 0u) {
        return 0;
    }
    out_root[0] = '\0';
    if (!save_path || save_path[0] == '\0') {
        return 0;
    }
    slash = strrchr(save_path, '/');
#if defined(_WIN32)
    {
        const char *backslash = strrchr(save_path, '\\');
        if (!slash || (backslash && backslash > slash)) {
            slash = backslash;
        }
    }
#endif
    if (!slash || slash <= save_path) {
        return 0;
    }
    len = (size_t)(slash - save_path);
    if (len >= out_root_cap) {
        len = out_root_cap - 1u;
    }
    memcpy(out_root, save_path, len);
    out_root[len] = '\0';
    return out_root[0] != '\0';
}

int theron_v1_boot_prepare_startup_profile(
    Theron_V1_BootProfile *profile,
    const char *data_dir,
    const char *verified_path,
    const char *verified_md5,
    const char *save_path,
    TrAssetBundle *assets,
    Theron_V1StartupSaveResume *out_save_resume,
    int *out_save_resume_ready,
    Theron_V1BootStartupPrepareResult *out_result) {

    Theron_V1BootStartupPrepareResult result =
        THERON_V1_BOOT_STARTUP_PREPARE_OK;
    char save_root[512];

    if (out_save_resume_ready) {
        *out_save_resume_ready = 0;
    }
    if (out_result) {
        *out_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
    }
    if (!profile || !data_dir || data_dir[0] == '\0' || !assets ||
        !out_save_resume) {
        return 0;
    }

    theron_v1_boot_profile_init(profile);
    if (verified_path && verified_path[0] != '\0' &&
        verified_md5 && verified_md5[0] != '\0') {
        if (theron_v1_boot_load_verified_path(profile,
                                              verified_path,
                                              verified_md5) != 0) {
            result = THERON_V1_BOOT_STARTUP_PREPARE_VERIFY_FAILED;
            goto fail;
        }
    } else if (theron_v1_boot_scan_assets(profile, data_dir) != 0 ||
               !profile->assets_verified) {
        result = THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02;
        goto fail;
    }

    if (theron_v1_boot_save_root_from_save_path(save_path,
                                                save_root,
                                                sizeof(save_root))) {
        theron_v1_boot_set_save_root(profile, save_root);
    } else {
        theron_v1_boot_set_save_root(profile, NULL);
    }

    memset(out_save_resume, 0, sizeof(*out_save_resume));
    if (out_save_resume_ready) {
        *out_save_resume_ready =
            theron_v1_boot_startup_save_resume(profile, out_save_resume);
    }
    if (theron_v1_startup_save_resume_apply_explicit_path(
            out_save_resume,
            save_path,
            profile->save_root)) {
        if (out_save_resume_ready) {
            *out_save_resume_ready = 1;
        }
    }

    if (tr_asset_load(profile->graphics_path, assets) != TR_ASSET_OK) {
        result = THERON_V1_BOOT_STARTUP_PREPARE_ASSET_LOAD_FAILED;
        goto fail;
    }

    /* Verified JP/US Track 02 bytes are authoritative original media.
     * Block generated palette/tile/UI rendering when no source-locked
     * graphics bank has been decoded. Legacy no-media tests may still
     * exercise their deterministic fixture path, but that path is not
     * a production launch permission.
     * Source: THQUEST.ASM T400/T410 boundary. */
    if (profile->assets_verified) {
        tr_asset_block_synthetic_rendering_for_verified_media(
            assets, profile->graphics_md5);
    }

    if (out_result) {
        *out_result = result;
    }
    return 1;

fail:
    if (out_result) {
        *out_result = result;
    }
    return 0;
}

/* Production boot boundary for the raw capture chain (restored after the
 * df88dbda4 clobber).  Binds the original loader transaction to the already
 * materialized startup media receipt; admits only a successful final-bind
 * and reports every provenance field for the caller to compare. */
int theron_v1_boot_startup_raw_media_graphics_receipt_from_loader_trace(
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_V1RawLoaderTraceReceipt *trace_receipt,
    Theron_V1_BootStartupRawMediaGraphicsReceipt *out_receipt)
{
    Theron_V1RawLoaderTraceReceipt bound;
    Theron_Track02Variant variant;

    if (out_receipt) {
        memset(out_receipt, 0, sizeof(*out_receipt));
        out_receipt->status = "TRACK02 RAW MEDIA RECEIPT REQUIRED";
    }
    if (!startup_media_receipt || !out_receipt) {
        return 0;
    }
    variant = (Theron_Track02Variant)startup_media_receipt->track02_variant;
    out_receipt->track02_variant = (int)variant;
    snprintf(out_receipt->track02_md5, sizeof(out_receipt->track02_md5), "%s",
             startup_media_receipt->track02_md5);
    out_receipt->raw_track02_verified =
        startup_media_receipt->startup_media_ready &&
                (variant == THERON_TRACK02_VARIANT_JP_BIN ||
                 variant == THERON_TRACK02_VARIANT_US_BIN)
            ? 1
            : 0;
    out_receipt->bitmap_route_mask =
        startup_media_receipt->startup_bitmap_raw_route_mask;
    out_receipt->bitmap_atlas_checksum =
        startup_media_receipt->startup_bitmap_atlas_checksum;
    out_receipt->bitmap_route_receipt_verified =
        theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            startup_media_receipt) &&
                out_receipt->bitmap_route_mask != 0u &&
                out_receipt->bitmap_atlas_checksum != 0u
            ? 1
            : 0;
    out_receipt->no_fallback_visuals = 1;
    if (!theron_v1_raw_loader_trace_final_bind(trace_receipt,
                                                startup_media_receipt,
                                                &bound)) {
        return 0;
    }
    out_receipt->cd_read_receipt_verified = bound.valid ? 1 : 0;
    out_receipt->palette_descriptor_relation_verified =
        bound.palette_descriptor_relation_verified ? 1 : 0;
    out_receipt->raw_media_verified = out_receipt->raw_track02_verified;
    out_receipt->valid = out_receipt->raw_track02_verified &&
                         out_receipt->cd_read_receipt_verified &&
                         out_receipt->bitmap_route_receipt_verified;
    out_receipt->status = out_receipt->valid
                              ? (out_receipt->palette_descriptor_relation_verified
                                     ? "TRACK02 RAW MEDIA GRAPHICS READY"
                                     : "TRACK02 PALETTE DESCRIPTOR UNPROVEN")
                              : "TRACK02 RAW MEDIA RECEIPT REQUIRED";
    return out_receipt->valid;
}

int theron_v1_boot_validate_track02_loader_receipt(
    const Theron_Track02StartupLoaderReceipt *receipt,
    const char *verified_md5) {
    char payload[THERON_TRACK02_MOUNT_PATH_CAPACITY];
    char md5[33];
    FILE *file;
    unsigned char *bytes;
    size_t required;
    size_t stage2_required;
    uint32_t expected_dynamic_record;
    Theron_Track02Variant variant;
    Theron_Track02IplLoaderReceipt observed;
    Theron_Track02Stage2DynamicPayloadReceipt dynamic_observed;
    Theron_V1Stage2RuntimeHandoff stage2_handoff;

    if (!receipt || !receipt->valid || !receipt->cue_backed ||
        !receipt->track02_md5_verified || !receipt->mode1_2352 ||
        !receipt->no_synthetic_cache || !verified_md5 ||
        strcmp(receipt->track02_md5, verified_md5) != 0 ||
        receipt->cue_path[0] == '\0' || receipt->track02_path[0] == '\0') return 0;
    if (theron_v1_track02_resolve_media_path(receipt->cue_path, payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        strcmp(payload, receipt->track02_path) != 0 ||
        !m12_file_md5_hex(payload, md5) || strcmp(md5, verified_md5) != 0) return 0;
    variant = theron_v1_track02_variant_for_md5(verified_md5);
    if (variant == THERON_TRACK02_VARIANT_JP_BIN) {
        expected_dynamic_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_JP;
    } else if (variant == THERON_TRACK02_VARIANT_US_BIN) {
        expected_dynamic_record = THERON_TRACK02_IPL_STAGE2_CD_READ_RECORD_US;
    } else {
        /* The dynamic $4090 trace is proven only for raw JP/US media. */
        return 0;
    }
    if (!receipt->ipl_loader.stage2_cd_read_record_proven ||
        receipt->ipl_loader.stage2_cd_read_record != expected_dynamic_record) {
        return 0;
    }
    required = ((size_t)THERON_TRACK02_IPL_STAGE2_RECORD +
                (size_t)THERON_TRACK02_IPL_STAGE2_SECTOR_COUNT) *
               THERON_TRACK02_RAW_SECTOR_BYTES;
    stage2_required = ((size_t)expected_dynamic_record + 1u) *
        THERON_TRACK02_RAW_SECTOR_BYTES;
    if (stage2_required > required) required = stage2_required;
    file = fopen(payload, "rb");
    if (!file) return 0;
    bytes = (unsigned char*)malloc(required);
    if (!bytes || fread(bytes, 1u, required, file) != required) {
        free(bytes);
        fclose(file);
        return 0;
    }
    fclose(file);
    memset(&observed, 0, sizeof(observed));
    if (theron_v1_track02_find_ipl_loader(bytes, required, verified_md5,
                                           &observed) != THERON_TRACK02_SIGNAL_OK ||
        !observed.valid ||
        observed.executable_user_data_hash != receipt->ipl_loader.executable_user_data_hash ||
        observed.stage2_user_data_hash != receipt->ipl_loader.stage2_user_data_hash ||
        observed.stage2_record != receipt->ipl_loader.stage2_record ||
        observed.stage2_sector_count != receipt->ipl_loader.stage2_sector_count ||
        observed.stage2_destination != receipt->ipl_loader.stage2_destination ||
        observed.stage2_load_address != receipt->ipl_loader.stage2_load_address ||
        observed.stage2_entry_address != receipt->ipl_loader.stage2_entry_address ||
        observed.stage2_cd_read_cpu_address !=
            receipt->ipl_loader.stage2_cd_read_cpu_address ||
        observed.stage2_cd_read_sector_count !=
            receipt->ipl_loader.stage2_cd_read_sector_count ||
        observed.stage2_cd_read_destination !=
            receipt->ipl_loader.stage2_cd_read_destination ||
        observed.stage2_cd_read_local_destination !=
            receipt->ipl_loader.stage2_cd_read_local_destination ||
        observed.stage2_cd_read_record !=
            receipt->ipl_loader.stage2_cd_read_record ||
        observed.stage2_cd_read_raw_sector !=
            receipt->ipl_loader.stage2_cd_read_raw_sector ||
        observed.stage2_cd_read_record_proven !=
            receipt->ipl_loader.stage2_cd_read_record_proven ||
        observed.stage2_cd_read_dynamic_boundary_valid !=
            receipt->ipl_loader.stage2_cd_read_dynamic_boundary_valid ||
        observed.stage2_cd_read_live_record_register_mask !=
            receipt->ipl_loader.stage2_cd_read_live_record_register_mask ||
        observed.vram_transfer_proven != receipt->ipl_loader.vram_transfer_proven) {
        free(bytes);
        return 0;
    }
    memset(&dynamic_observed, 0, sizeof(dynamic_observed));
    memset(&stage2_handoff, 0, sizeof(stage2_handoff));
    if (theron_v1_track02_inspect_stage2_dynamic_payload(
            bytes, required, verified_md5, &dynamic_observed) !=
            THERON_TRACK02_SIGNAL_OK ||
        !dynamic_observed.valid ||
        dynamic_observed.track02_record != expected_dynamic_record ||
        dynamic_observed.raw_sector != receipt->ipl_loader.stage2_cd_read_raw_sector ||
        dynamic_observed.user_data_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        dynamic_observed.header_word0 != 0x00ffu ||
        dynamic_observed.header_word1 != 0x0308u ||
        dynamic_observed.manifest_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_BYTES ||
        dynamic_observed.manifest_entry_count !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_MANIFEST_ENTRY_COUNT ||
        dynamic_observed.user_data_hash == 0u) {
        free(bytes);
        return 0;
    }
    /* CUE/M11 boot reaches runtime only through the same authentic stage-two
     * transfer contract as direct Track 02 entry: cleared work RAM then the
     * physical $3800 BRK $ff dispatch. */
    if (!theron_v1_stage2_runtime_handoff_from_original_media(
            bytes, required, verified_md5, &stage2_handoff) ||
        stage2_handoff.track02_record != dynamic_observed.track02_record ||
        stage2_handoff.user_data_hash != dynamic_observed.user_data_hash ||
        !stage2_handoff.ipl_preload_local_read_verified ||
        stage2_handoff.ipl_preload_cpu_address != 0x40cdu ||
        stage2_handoff.ipl_preload_destination != 0x3000u ||
        !stage2_handoff.ipl_preload_record_proven ||
        stage2_handoff.ipl_preload_record != 0x0003e3u ||
        stage2_handoff.ipl_preload_sector_count != 2u ||
        !stage2_handoff.ipl_preload_returns_to_ipl_proven ||
        stage2_handoff.ipl_preload_user_data_bytes != 4096u ||
        stage2_handoff.ipl_preload_first_nonzero_offset != 243u ||
        stage2_handoff.ipl_preload_nonzero_byte_count != 2911u ||
        stage2_handoff.ipl_preload_user_data_hash == 0u ||
        !stage2_handoff.stage2_cd_exec_table_verified ||
        !stage2_handoff.stage2_cd_read_setup_verified ||
        !stage2_handoff.stage2_post_read_transfer_verified ||
        !stage2_handoff.work_ram_cleared_before_entry ||
        stage2_handoff.cleared_work_ram_start != 0x2700u ||
        stage2_handoff.cleared_work_ram_bytes != 0x1100u ||
        stage2_handoff.cleared_work_ram_end != 0x3800u ||
        !stage2_handoff.physical_stage3_entry_verified ||
        stage2_handoff.stage3_entry_opcode != 0x00u ||
        stage2_handoff.stage3_irq2_selector != 0xffu ||
        stage2_handoff.stage3_continuation_address != 0x3802u ||
        !stage2_handoff.stage3_mode1_header_verified ||
        !stage2_handoff.stage3_selector_catalog_complete ||
        stage2_handoff.stage3_resolved_descriptor_selector_count == 0u ||
        stage2_handoff.stage3_out_of_bounds_descriptor_selector_count != 0u ||
        stage2_handoff.stage3_resolved_descriptor_selector_count !=
            stage2_handoff.stage3_nonzero_descriptor_selector_count ||
        stage2_handoff.stage3_resolved_descriptor_selector_hash == 0u ||
        !stage2_handoff.stage3_first_descriptor_record_boundary_verified ||
        stage2_handoff.stage3_first_descriptor_raw_sector !=
            stage2_handoff.stage3_cd_read_raw_sector ||
        stage2_handoff.stage3_first_descriptor_user_data_offset !=
            stage2_handoff.stage3_cd_read_user_data_offset ||
        stage2_handoff.stage3_first_descriptor_user_data_bytes !=
            stage2_handoff.user_data_bytes ||
        stage2_handoff.stage3_first_descriptor_user_data_hash !=
            stage2_handoff.user_data_hash ||
        stage2_handoff.stage3_first_descriptor_semantics_proven) {
        free(bytes);
        return 0;
    }
    free(bytes);
    return 1;
}

const char *theron_v1_boot_startup_prepare_result_name(
    Theron_V1BootStartupPrepareResult result) {
    switch (result) {
        case THERON_V1_BOOT_STARTUP_PREPARE_OK:
            return "OK";
        case THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT:
            return "BAD_INPUT";
        case THERON_V1_BOOT_STARTUP_PREPARE_VERIFY_FAILED:
            return "VERIFY_FAILED";
        case THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02:
            return "MISSING_TRACK02";
        case THERON_V1_BOOT_STARTUP_PREPARE_ASSET_LOAD_FAILED:
            return "ASSET_LOAD_FAILED";
        case THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED:
            return "STATE_FAILED";
        default:
            return "UNKNOWN";
    }
}

int theron_v1_boot_startup_session_facts_from_runtime_state(
    Theron_StartupSessionFacts *session,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    if (!session) {
        return 0;
    }
    theron_v1_startup_session_facts_from_runtime(
        session,
        (Theron_StartupPhase)startup_phase,
        selected_dungeon,
        boot_profile,
        world,
        assets,
        startup_cursor,
        continue_focus,
        resume_claim,
        tqsv_slot,
        srm_slot,
        srm_import_status,
        srm_root,
        startup_text_prompt,
        startup_roster_names,
        startup_roster_titles,
        startup_roster_name_count,
        selected_mirrors_mask,
        companion_count,
        selected_mirror_order,
        selected_mirror_order_count);
    return 1;
}

static int theron_v1_boot_startup_session_from_runtime_state(
    Theron_StartupSessionFacts *session,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    return theron_v1_boot_startup_session_facts_from_runtime_state(
        session,
        startup_phase,
        selected_dungeon,
        boot_profile,
        world,
        assets,
        startup_cursor,
        continue_focus,
        resume_claim,
        tqsv_slot,
        srm_slot,
        srm_import_status,
        srm_root,
        startup_text_prompt,
        startup_roster_names,
        startup_roster_titles,
        startup_roster_name_count,
        selected_mirrors_mask,
        companion_count,
        selected_mirror_order,
        selected_mirror_order_count);
}

static int theron_v1_boot_startup_session_from_snapshot(
    Theron_StartupSessionFacts *session,
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot)
{
    if (!snapshot) {
        return 0;
    }
    return theron_v1_boot_startup_session_from_runtime_state(
        session,
        snapshot->startup_phase,
        snapshot->selected_dungeon,
        snapshot->boot_profile,
        snapshot->world,
        snapshot->assets,
        snapshot->startup_cursor,
        snapshot->continue_focus,
        snapshot->resume_claim,
        snapshot->tqsv_slot,
        snapshot->srm_slot,
        snapshot->srm_import_status,
        snapshot->srm_root,
        snapshot->startup_text_prompt,
        snapshot->startup_roster_names,
        snapshot->startup_roster_titles,
        snapshot->startup_roster_name_count,
        snapshot->selected_mirrors_mask,
        snapshot->companion_count,
        snapshot->selected_mirror_order,
        snapshot->selected_mirror_order_count);
}

int theron_v1_boot_startup_execute_input_from_runtime_state(
    Theron_StartupActionHostReceipt *out_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_runtime_state(
            &view_model,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            startup_text_prompt,
            startup_roster_names,
            startup_roster_titles,
            startup_roster_name_count,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
        &view_model,
        theron_v1_startup_input_from_firestaff_menu_code(input),
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_runtime_state_with_media_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
            &full_start,
            startup_media_receipt,
            NULL,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = "FULL START RECEIPT MISSING";
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_input_from_full_start_receipt(
        &full_start,
        theron_v1_startup_input_from_firestaff_menu_code(input),
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    int input,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_snapshot(snapshot,
                                                         &view_model)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
        &view_model,
        theron_v1_startup_input_from_firestaff_menu_code(input),
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int input,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = theron_v1_startup_result_name(
                THERON_STARTUP_ERR_NULL);
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
        &view_model,
        theron_v1_startup_input_from_firestaff_menu_code(input),
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_runtime_state(
    Theron_StartupActionHostReceipt *out_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_runtime_state(
            &view_model,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            startup_text_prompt,
            startup_roster_names,
            startup_roster_titles,
            startup_roster_name_count,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
        &view_model,
        x,
        y,
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_runtime_state_with_media_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int x,
    int y)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
            &full_start,
            startup_media_receipt,
            NULL,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = "FULL START RECEIPT MISSING";
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_pointer_from_full_start_receipt(
        &full_start,
        x,
        y,
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_snapshot(snapshot,
                                                         &view_model)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
        &view_model,
        x,
        y,
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = theron_v1_startup_result_name(
                THERON_STARTUP_ERR_NULL);
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
        &view_model,
        x,
        y,
        out_receipt);
}

void theron_v1_boot_startup_view_model_clear(
    Theron_V1_BootStartupViewModel *view_model)
{
    if (!view_model) {
        return;
    }
    memset(view_model, 0, sizeof(*view_model));
}

int theron_v1_boot_startup_view_model_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    Theron_V1_BootStartupViewModel *out_view_model)
{
    return theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
        snapshot,
        NULL,
        out_view_model);
}

int theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupViewModel *out_view_model)
{
    Theron_V1_BootRuntimeStartupSnapshot effective_snapshot;
    Theron_StartupSessionFacts session;
    int i;

    if (!out_view_model) {
        return 0;
    }
    theron_v1_boot_startup_view_model_clear(out_view_model);
    if (!snapshot) {
        return 0;
    }
    effective_snapshot = *snapshot;
    if (startup_media_receipt) {
        if ((!effective_snapshot.startup_text_prompt ||
             effective_snapshot.startup_text_prompt[0] == '\0') &&
            startup_media_receipt->startup_text_prompt_status ==
                THERON_TRACK02_SIGNAL_OK &&
            startup_media_receipt->startup_text_prompt[0] != '\0') {
            effective_snapshot.startup_text_prompt =
                startup_media_receipt->startup_text_prompt;
        }
        if (!effective_snapshot.startup_roster_names &&
            startup_media_receipt->startup_roster_name_status ==
                THERON_TRACK02_SIGNAL_OK &&
            startup_media_receipt->startup_roster_name_count > 0) {
            effective_snapshot.startup_roster_names =
                startup_media_receipt->startup_roster_names;
            effective_snapshot.startup_roster_titles =
                startup_media_receipt->startup_roster_titles;
            effective_snapshot.startup_roster_name_count =
                startup_media_receipt->startup_roster_name_count;
        }
    }

    if (!theron_v1_boot_startup_session_from_snapshot(
            &session,
            &effective_snapshot)) {
        return 0;
    }

    out_view_model->layout_count =
        theron_v1_startup_layout_build_from_session(
            &session,
            out_view_model->layout,
            THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP);
    out_view_model->row_count =
        theron_v1_startup_render_rows_build_from_session(
            &session,
            out_view_model->rows,
            THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP);
    out_view_model->render_plan_valid =
        theron_v1_startup_render_plan_build_from_session(
            &session,
            &out_view_model->render_plan);
    (void)theron_v1_startup_presentation_receipt(
        (Theron_StartupPhase)session.phase,
        out_view_model->phase,
        (int)sizeof(out_view_model->phase),
        &out_view_model->startup_active,
        out_view_model->animation,
        (int)sizeof(out_view_model->animation),
        &out_view_model->animation_active,
        &out_view_model->title_frame,
        &out_view_model->title_frame_max,
        &out_view_model->title_ready);
    out_view_model->runtime_level =
        effective_snapshot.world ? effective_snapshot.world->current_level : -1;
    out_view_model->runtime_champion_count =
        effective_snapshot.world
            ? effective_snapshot.world->party.champion_count
            : -1;
    out_view_model->continue_focus = effective_snapshot.continue_focus;
    out_view_model->resume_claim = effective_snapshot.resume_claim;
    out_view_model->tqsv_slot = effective_snapshot.tqsv_slot;
    out_view_model->srm_slot = effective_snapshot.srm_slot;
    out_view_model->srm_import_status =
        effective_snapshot.srm_import_status;
    out_view_model->startup_phase = effective_snapshot.startup_phase;
    out_view_model->selected_dungeon = effective_snapshot.selected_dungeon;
    out_view_model->boot_profile = effective_snapshot.boot_profile;
    out_view_model->world = effective_snapshot.world;
    out_view_model->assets = effective_snapshot.assets;
    out_view_model->startup_cursor = effective_snapshot.startup_cursor;
    out_view_model->selected_mirrors_mask =
        effective_snapshot.selected_mirrors_mask;
    out_view_model->companion_count = effective_snapshot.companion_count;
    out_view_model->selected_mirror_order_count =
        effective_snapshot.selected_mirror_order_count;
    if (out_view_model->selected_mirror_order_count < 0) {
        out_view_model->selected_mirror_order_count = 0;
    }
    if (out_view_model->selected_mirror_order_count >
        THERON_STARTUP_MAX_COMPANIONS) {
        out_view_model->selected_mirror_order_count =
            THERON_STARTUP_MAX_COMPANIONS;
    }
    for (i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        out_view_model->selected_mirror_order[i] =
            effective_snapshot.selected_mirror_order &&
                    i < out_view_model->selected_mirror_order_count
                ? effective_snapshot.selected_mirror_order[i]
                : -1;
    }
    out_view_model->srm_root = effective_snapshot.srm_root;
    out_view_model->runtime_level_source =
        effective_snapshot.runtime_level_source;
    out_view_model->runtime_track02_semantic_handoff =
        effective_snapshot.runtime_track02_semantic_handoff ? 1 : 0;
    out_view_model->runtime_fallback_visuals_blocked =
        effective_snapshot.runtime_fallback_visuals_blocked ? 1 : 0;
    out_view_model->runtime_structured_route =
        effective_snapshot.runtime_structured_route ? 1 : 0;
    out_view_model->runtime_receipt_text_route =
        effective_snapshot.runtime_receipt_text_route ? 1 : 0;
    out_view_model->all_dungeon_real_data_capture_ready =
        effective_snapshot.all_dungeon_real_data_capture_ready ? 1 : 0;
    out_view_model->all_dungeon_capture_count =
        effective_snapshot.all_dungeon_capture_count;
    out_view_model->all_dungeon_capture_mask =
        effective_snapshot.all_dungeon_capture_mask;
    out_view_model->exact_level_semantics_ready =
        effective_snapshot.exact_level_semantics_ready ? 1 : 0;
    out_view_model->exact_object_semantics_ready =
        effective_snapshot.exact_object_semantics_ready ? 1 : 0;
    out_view_model->no_fallback_semantic_role_mask =
        effective_snapshot.no_fallback_semantic_role_mask;
    out_view_model->track02_state_predicates_consumed =
        effective_snapshot.track02_state_predicates_consumed ? 1 : 0;
    out_view_model->track02_bitmap_routes_complete =
        effective_snapshot.track02_bitmap_routes_complete ? 1 : 0;
    out_view_model->track02_no_fallback_runtime_route_ready =
        effective_snapshot.track02_no_fallback_runtime_route_ready ? 1 : 0;
    out_view_model->object_table_no_fallback_ready =
        effective_snapshot.object_table_no_fallback_ready ? 1 : 0;
    out_view_model->object_table_blocked_anchor_mask =
        effective_snapshot.object_table_blocked_anchor_mask;
    out_view_model->object_table_blocked_anchor_count =
        effective_snapshot.object_table_blocked_anchor_count;
    out_view_model->nonstartup_level_no_fallback_ready =
        effective_snapshot.nonstartup_level_no_fallback_ready ? 1 : 0;
    out_view_model->nonstartup_level_blocked_anchor_mask =
        effective_snapshot.nonstartup_level_blocked_anchor_mask;
    out_view_model->nonstartup_level_blocked_anchor_count =
        effective_snapshot.nonstartup_level_blocked_anchor_count;
    out_view_model->startup_level_blocked_anchor_mask =
        effective_snapshot.startup_level_blocked_anchor_mask;
    out_view_model->startup_level_blocked_anchor_count =
        effective_snapshot.startup_level_blocked_anchor_count;
    out_view_model->object_table_route_hash =
        effective_snapshot.object_table_route_hash;
    out_view_model->level_route_hash = effective_snapshot.level_route_hash;
    if (out_view_model->runtime_level_source ==
            THERON_V1_STARTUP_RUNTIME_LEVEL_NONE &&
        effective_snapshot.world) {
        const Theron_V1_World *world = effective_snapshot.world;
        int dungeon_index = world->current_dungeon - 1;
        if (dungeon_index >= 0 &&
            dungeon_index < THERON_DUNGEON_COUNT &&
            world->current_level >= 0 &&
            world->current_level < THERON_MAX_LEVELS_PER_DUNGEON &&
            world->level_loaded[dungeon_index][world->current_level]) {
            out_view_model->runtime_level_source =
                THERON_V1_STARTUP_RUNTIME_LEVEL_FALLBACK_ROOM;
        }
    }
    if (startup_media_receipt) {
        out_view_model->startup_media_state_valid = 1;
        out_view_model->startup_media_state_receipt =
            *startup_media_receipt;
    }
    return 1;
}

int theron_v1_boot_startup_view_model_from_runtime_state(
    Theron_V1_BootStartupViewModel *out_view_model,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    return theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
        out_view_model,
        NULL,
        startup_phase,
        selected_dungeon,
        boot_profile,
        world,
        assets,
        startup_cursor,
        continue_focus,
        resume_claim,
        tqsv_slot,
        srm_slot,
        srm_import_status,
        srm_root,
        startup_text_prompt,
        startup_roster_names,
        startup_roster_titles,
        startup_roster_name_count,
        selected_mirrors_mask,
        companion_count,
        selected_mirror_order,
        selected_mirror_order_count);
}

int theron_v1_boot_startup_view_model_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupViewModel *out_view_model,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    const char *startup_text_prompt,
    const char startup_roster_names[][THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY],
    const char startup_roster_titles[][THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY],
    int startup_roster_name_count,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    Theron_V1_BootRuntimeStartupSnapshot snapshot;

    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.startup_phase = startup_phase;
    snapshot.selected_dungeon = selected_dungeon;
    snapshot.boot_profile = boot_profile;
    snapshot.world = world;
    snapshot.assets = assets;
    snapshot.startup_cursor = startup_cursor;
    snapshot.continue_focus = continue_focus;
    snapshot.resume_claim = resume_claim;
    snapshot.tqsv_slot = tqsv_slot;
    snapshot.srm_slot = srm_slot;
    snapshot.srm_import_status = srm_import_status;
    snapshot.srm_root = srm_root;
    snapshot.startup_text_prompt = startup_text_prompt;
    snapshot.startup_roster_names = startup_roster_names;
    snapshot.startup_roster_titles = startup_roster_titles;
    snapshot.startup_roster_name_count = startup_roster_name_count;
    snapshot.selected_mirrors_mask = selected_mirrors_mask;
    snapshot.companion_count = companion_count;
    snapshot.selected_mirror_order = selected_mirror_order;
    snapshot.selected_mirror_order_count = selected_mirror_order_count;
    return theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
        &snapshot,
        startup_media_receipt,
        out_view_model);
}

int theron_v1_boot_startup_layout_build_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupLayoutElement *elements,
    int max_elements)
{
    int count;

    if (!view_model || !elements || max_elements <= 0) {
        if (elements && max_elements > 0) {
            memset(elements, 0, (size_t)max_elements * sizeof(elements[0]));
        }
        return 0;
    }
    count = view_model->layout_count;
    if (count < 0) {
        count = 0;
    }
    if (count > THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP) {
        count = THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP;
    }
    if (count > max_elements) {
        count = max_elements;
    }
    memset(elements, 0, (size_t)max_elements * sizeof(elements[0]));
    if (count > 0) {
        memcpy(elements,
               view_model->layout,
               (size_t)count * sizeof(elements[0]));
    }
    return count;
}

int theron_v1_boot_startup_render_rows_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows)
{
    int count;

    if (!view_model || !rows || max_rows <= 0) {
        if (rows && max_rows > 0) {
            memset(rows,
                   0,
                   (size_t)max_rows * THERON_STARTUP_RENDER_ROW_CAPACITY);
        }
        return 0;
    }
    count = view_model->row_count;
    if (count < 0) {
        count = 0;
    }
    if (count > THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP) {
        count = THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP;
    }
    if (count > max_rows) {
        count = max_rows;
    }
    memset(rows, 0, (size_t)max_rows * THERON_STARTUP_RENDER_ROW_CAPACITY);
    if (count > 0) {
        memcpy(rows,
               view_model->rows,
               (size_t)count * THERON_STARTUP_RENDER_ROW_CAPACITY);
    }
    return count;
}

int theron_v1_boot_startup_render_plan_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupRenderPlan *out_plan)
{
    if (!view_model || !out_plan || !view_model->render_plan_valid) {
        if (out_plan) {
            memset(out_plan, 0, sizeof(*out_plan));
        }
        return 0;
    }
    *out_plan = view_model->render_plan;
    return 1;
}

void theron_v1_boot_startup_render_route_receipt_init(
    Theron_V1_BootStartupRenderRouteReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status_scope = "STARTUP";
    receipt->status = "NO RENDER ROUTE";
}

int theron_v1_boot_startup_render_route_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_V1_BootStartupRenderRouteReceipt *out_receipt)
{
    Theron_StartupStateReceipt state_receipt;
    Theron_V2_HudOverlay hud_overlay;
    Theron_V2_HudSeedGate hud_seed_gate;
    int startup_bitmap_routes_complete;

    if (out_receipt) {
        theron_v1_boot_startup_render_route_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        return 0;
    }
    startup_bitmap_routes_complete =
        view_model->startup_media_state_valid &&
        theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            &view_model->startup_media_state_receipt);

    out_receipt->runtime_level_source = view_model->runtime_level_source;
    out_receipt->runtime_track02_semantic_handoff =
        view_model->runtime_track02_semantic_handoff;
    out_receipt->runtime_fallback_visuals_blocked =
        view_model->runtime_fallback_visuals_blocked;
    out_receipt->runtime_structured_route =
        view_model->runtime_structured_route;
    out_receipt->runtime_receipt_text_route =
        view_model->runtime_receipt_text_route;
    out_receipt->all_dungeon_real_data_capture_ready =
        view_model->all_dungeon_real_data_capture_ready;
    out_receipt->all_dungeon_capture_count =
        view_model->all_dungeon_capture_count;
    out_receipt->all_dungeon_capture_mask =
        view_model->all_dungeon_capture_mask;
    out_receipt->exact_level_semantics_ready =
        view_model->exact_level_semantics_ready;
    out_receipt->exact_object_semantics_ready =
        view_model->exact_object_semantics_ready;
    out_receipt->no_fallback_semantic_role_mask =
        view_model->no_fallback_semantic_role_mask;
    out_receipt->object_table_no_fallback_ready =
        view_model->object_table_no_fallback_ready;
    out_receipt->object_table_blocked_anchor_mask =
        view_model->object_table_blocked_anchor_mask;
    out_receipt->object_table_blocked_anchor_count =
        view_model->object_table_blocked_anchor_count;
    out_receipt->nonstartup_level_no_fallback_ready =
        view_model->nonstartup_level_no_fallback_ready;
    out_receipt->nonstartup_level_blocked_anchor_mask =
        view_model->nonstartup_level_blocked_anchor_mask;
    out_receipt->nonstartup_level_blocked_anchor_count =
        view_model->nonstartup_level_blocked_anchor_count;
    out_receipt->startup_level_blocked_anchor_mask =
        view_model->startup_level_blocked_anchor_mask;
    out_receipt->startup_level_blocked_anchor_count =
        view_model->startup_level_blocked_anchor_count;
    out_receipt->object_table_route_hash =
        view_model->object_table_route_hash;
    out_receipt->level_route_hash = view_model->level_route_hash;
    out_receipt->startup_menu_render_allowed =
        view_model->render_plan_valid ? 1 : 0;
    out_receipt->track02_title_menu_ready =
        view_model->startup_media_state_valid &&
                view_model->startup_media_state_receipt.startup_media_ready
            ? 1
            : 0;
    out_receipt->runtime_level_render_allowed =
        view_model->runtime_level_source !=
                THERON_V1_STARTUP_RUNTIME_LEVEL_NONE &&
        view_model->runtime_level_source !=
                THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED
            ? 1
            : 0;
    out_receipt->first_level_render_ready =
        out_receipt->runtime_level_render_allowed &&
        view_model->runtime_level == 0 ? 1 : 0;
    hud_seed_gate = theron_v2_hud_seed_from_v1_world(
        &hud_overlay,
        view_model->world,
        out_receipt->runtime_level_render_allowed);
    out_receipt->hud_seed_gate = (int)hud_seed_gate;
    out_receipt->hud_ready =
        hud_seed_gate == THERON_V2_HUD_SEED_V2_READY ? 1 : 0;
    out_receipt->runtime_readiness_ready =
        out_receipt->runtime_level_render_allowed &&
        out_receipt->hud_ready ? 1 : 0;
    out_receipt->title_menu_runtime_handoff_ready =
        out_receipt->track02_title_menu_ready &&
        out_receipt->runtime_readiness_ready ? 1 : 0;
    out_receipt->save_resume_claim = view_model->resume_claim;
    out_receipt->save_resume_tqsv_slot = view_model->tqsv_slot;
    out_receipt->save_resume_srm_slot = view_model->srm_slot;
    out_receipt->save_resume_srm_import_status =
        view_model->srm_import_status;
    out_receipt->save_resume_start_ready =
        view_model->resume_claim != THERON_V1_STARTUP_RESUME_NONE ? 1 : 0;
    out_receipt->save_resume_runtime_handoff_ready =
        out_receipt->save_resume_start_ready &&
        out_receipt->runtime_readiness_ready ? 1 : 0;
    out_receipt->no_fallback_visuals_enforced =
        view_model->runtime_fallback_visuals_blocked ||
                startup_bitmap_routes_complete ||
                view_model->runtime_level_source ==
                    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC ||
                view_model->runtime_level_source ==
                    THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME
            ? 1
            : 0;
    out_receipt->fallback_visuals_allowed =
        out_receipt->no_fallback_visuals_enforced ? 0 : 1;
    out_receipt->save_resume_track02_no_fallback_ready =
        out_receipt->save_resume_runtime_handoff_ready &&
        out_receipt->title_menu_runtime_handoff_ready &&
        out_receipt->no_fallback_visuals_enforced ? 1 : 0;
    out_receipt->status_scope = "STARTUP";
    out_receipt->status =
        view_model->runtime_fallback_visuals_blocked
            ? "TRACK02 RUNTIME BLOCKED"
            : (view_model->runtime_level_source ==
                       THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC
                   ? "TRACK02 RUNTIME READY"
                   : (view_model->runtime_level_source ==
                              THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME
                          ? "SAVE RESUME RUNTIME READY"
                   : (out_receipt->runtime_level_render_allowed
                          ? "THERON RUNTIME READY"
                          : "THERON STARTUP MENU")));
    if (theron_v1_boot_startup_render_plan_from_view_model(
            view_model,
            &out_receipt->render_plan)) {
        out_receipt->render_plan_valid = 1;
    }
    if (theron_v1_boot_startup_state_receipt_from_view_model(
            view_model,
            &state_receipt)) {
        out_receipt->state_receipt = state_receipt;
        out_receipt->state_receipt_valid = 1;
        out_receipt->track02_state_predicates_consumed = 1;
        out_receipt->track02_bitmap_routes_complete =
            theron_v1_startup_state_receipt_has_complete_track02_bitmap_routes(
                &state_receipt) ||
            view_model->track02_bitmap_routes_complete;
        out_receipt->track02_no_fallback_runtime_route_ready =
            theron_v1_startup_state_receipt_has_track02_no_fallback_runtime_route(
                &state_receipt) ||
            view_model->track02_no_fallback_runtime_route_ready;
    }
    return out_receipt->render_plan_valid ||
           out_receipt->state_receipt_valid ||
           out_receipt->runtime_fallback_visuals_blocked;
}

int theron_v1_boot_startup_render_route_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupRenderRouteReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        if (out_receipt) {
            theron_v1_boot_startup_render_route_receipt_init(out_receipt);
        }
        return 0;
    }
    return theron_v1_boot_startup_render_route_receipt_from_view_model(
        &view_model,
        out_receipt);
}

void theron_v1_boot_startup_host_view_receipt_init(
    Theron_V1_BootStartupHostViewReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status_scope = "STARTUP";
    receipt->status = "NO HOST VIEW";
}

int theron_v1_boot_startup_host_view_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_V1_BootStartupHostViewReceipt *out_receipt)
{
    if (out_receipt) {
        theron_v1_boot_startup_host_view_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        return 0;
    }

    out_receipt->host_consumes_view_model = 1;
    out_receipt->view_model_valid = 1;
    out_receipt->view_model = *view_model;
    out_receipt->layout_count = view_model->layout_count;
    out_receipt->row_count = view_model->row_count;
    out_receipt->render_plan_valid = view_model->render_plan_valid;
    out_receipt->presentation_ready = 1;
    snprintf(out_receipt->phase,
             sizeof(out_receipt->phase),
             "%s",
             view_model->phase);
    out_receipt->startup_active = view_model->startup_active;
    snprintf(out_receipt->animation,
             sizeof(out_receipt->animation),
             "%s",
             view_model->animation);
    out_receipt->animation_active = view_model->animation_active;
    out_receipt->title_frame = view_model->title_frame;
    out_receipt->title_frame_max = view_model->title_frame_max;
    out_receipt->title_ready = view_model->title_ready;
    if (theron_v1_boot_startup_render_route_receipt_from_view_model(
            view_model,
            &out_receipt->render_route)) {
        out_receipt->render_route_valid = 1;
        out_receipt->runtime_readiness_ready =
            out_receipt->render_route.runtime_readiness_ready;
        out_receipt->runtime_level_render_allowed =
            out_receipt->render_route.runtime_level_render_allowed;
        out_receipt->title_menu_runtime_handoff_ready =
            out_receipt->render_route.title_menu_runtime_handoff_ready;
        out_receipt->save_resume_runtime_handoff_ready =
            out_receipt->render_route.save_resume_runtime_handoff_ready;
        out_receipt->save_resume_track02_no_fallback_ready =
            out_receipt->render_route.save_resume_track02_no_fallback_ready;
        out_receipt->no_fallback_visuals_enforced =
            out_receipt->render_route.no_fallback_visuals_enforced;
        out_receipt->fallback_visuals_allowed =
            out_receipt->render_route.fallback_visuals_allowed;
        out_receipt->runtime_level_source =
            out_receipt->render_route.runtime_level_source;
        out_receipt->runtime_graphics_handoff =
            out_receipt->runtime_readiness_ready &&
            out_receipt->no_fallback_visuals_enforced &&
            (out_receipt->runtime_level_source ==
                 THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC ||
             out_receipt->runtime_level_source ==
                 THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME) ? 1 : 0;
        out_receipt->track02_runtime_graphics_handoff =
            out_receipt->runtime_graphics_handoff &&
                    out_receipt->runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC
                ? 1
                : 0;
        out_receipt->save_resume_runtime_graphics_handoff =
            out_receipt->runtime_graphics_handoff &&
                    out_receipt->runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME
                ? 1
                : 0;
        out_receipt->runtime_track02_semantic_handoff =
            out_receipt->render_route.runtime_track02_semantic_handoff;
        out_receipt->runtime_fallback_visuals_blocked =
            out_receipt->render_route.runtime_fallback_visuals_blocked;
        out_receipt->runtime_structured_route =
            out_receipt->render_route.runtime_structured_route;
        out_receipt->runtime_receipt_text_route =
            out_receipt->render_route.runtime_receipt_text_route;
        out_receipt->all_dungeon_real_data_capture_ready =
            out_receipt->render_route.all_dungeon_real_data_capture_ready;
        out_receipt->all_dungeon_capture_count =
            out_receipt->render_route.all_dungeon_capture_count;
        out_receipt->all_dungeon_capture_mask =
            out_receipt->render_route.all_dungeon_capture_mask;
        out_receipt->exact_level_semantics_ready =
            out_receipt->render_route.exact_level_semantics_ready;
        out_receipt->exact_object_semantics_ready =
            out_receipt->render_route.exact_object_semantics_ready;
        out_receipt->no_fallback_semantic_role_mask =
            out_receipt->render_route.no_fallback_semantic_role_mask;
        out_receipt->track02_state_predicates_consumed =
            out_receipt->render_route.track02_state_predicates_consumed;
        out_receipt->track02_bitmap_routes_complete =
            out_receipt->render_route.track02_bitmap_routes_complete;
        out_receipt->track02_no_fallback_runtime_route_ready =
            out_receipt->render_route
                .track02_no_fallback_runtime_route_ready;
        out_receipt->object_table_no_fallback_ready =
            out_receipt->render_route.object_table_no_fallback_ready;
        out_receipt->object_table_blocked_anchor_mask =
            out_receipt->render_route.object_table_blocked_anchor_mask;
        out_receipt->object_table_blocked_anchor_count =
            out_receipt->render_route.object_table_blocked_anchor_count;
        out_receipt->nonstartup_level_no_fallback_ready =
            out_receipt->render_route.nonstartup_level_no_fallback_ready;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            out_receipt->render_route.nonstartup_level_blocked_anchor_mask;
        out_receipt->nonstartup_level_blocked_anchor_count =
            out_receipt->render_route.nonstartup_level_blocked_anchor_count;
        out_receipt->startup_level_blocked_anchor_mask =
            out_receipt->render_route.startup_level_blocked_anchor_mask;
        out_receipt->startup_level_blocked_anchor_count =
            out_receipt->render_route.startup_level_blocked_anchor_count;
        out_receipt->object_table_route_hash =
            out_receipt->render_route.object_table_route_hash;
        out_receipt->level_route_hash =
            out_receipt->render_route.level_route_hash;
        out_receipt->hud_ready = out_receipt->render_route.hud_ready;
        out_receipt->status_scope = out_receipt->render_route.status_scope;
        out_receipt->status = out_receipt->render_route.status;
    }
    if (theron_v1_boot_startup_state_receipt_from_view_model(
            view_model,
            &out_receipt->state_receipt)) {
        out_receipt->state_receipt_valid = 1;
        out_receipt->track02_state_predicates_consumed = 1;
        out_receipt->track02_bitmap_routes_complete =
            theron_v1_startup_state_receipt_has_complete_track02_bitmap_routes(
                &out_receipt->state_receipt) ||
            view_model->track02_bitmap_routes_complete;
        out_receipt->track02_no_fallback_runtime_route_ready =
            theron_v1_startup_state_receipt_has_track02_no_fallback_runtime_route(
                &out_receipt->state_receipt) ||
            view_model->track02_no_fallback_runtime_route_ready;
    }
    out_receipt->track02_media_consumed =
        view_model->startup_media_state_valid &&
        (view_model->startup_media_state_receipt.startup_media_ready ||
         view_model->startup_media_state_receipt.startup_text_prompt_status ==
             THERON_TRACK02_SIGNAL_OK ||
         view_model->startup_media_state_receipt.startup_roster_name_status ==
             THERON_TRACK02_SIGNAL_OK)
            ? 1
            : 0;
    out_receipt->raw_prompt_roster_required =
        out_receipt->track02_media_consumed ? 0 : 1;
    out_receipt->raw_session_rebuild_required = 0;
    if (!out_receipt->render_route_valid) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "HOST VIEW READY";
    }
    return 1;
}

int theron_v1_boot_startup_host_view_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_V1_BootStartupHostViewReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        if (out_receipt) {
            theron_v1_boot_startup_host_view_receipt_init(out_receipt);
        }
        return 0;
    }
    return theron_v1_boot_startup_host_view_receipt_from_view_model(
        &view_model,
        out_receipt);
}

int theron_v1_boot_startup_state_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupStateReceipt *out_receipt)
{
    int i;

    if (out_receipt) {
        theron_v1_startup_state_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        return 0;
    }

    out_receipt->flow_changed = 1;
    out_receipt->flow.phase = (Theron_StartupPhase)view_model->startup_phase;
    out_receipt->flow.selected_dungeon =
        (Theron_DungeonID)view_model->selected_dungeon;
    out_receipt->flow.selected_mirrors_mask =
        (uint8_t)view_model->selected_mirrors_mask;
    out_receipt->flow.companion_count =
        (uint8_t)view_model->companion_count;
    for (i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        out_receipt->flow.selected_mirror_order[i] =
            view_model->selected_mirror_order[i];
    }

    out_receipt->set_startup_cursor = 1;
    out_receipt->startup_cursor = view_model->startup_cursor;
    out_receipt->set_continue_focus = 1;
    out_receipt->continue_focus = view_model->continue_focus;

    if (view_model->runtime_level_source !=
        THERON_V1_STARTUP_RUNTIME_LEVEL_NONE) {
        out_receipt->set_runtime_level_route = 1;
        out_receipt->runtime_level_source =
            view_model->runtime_level_source;
        out_receipt->runtime_track02_semantic_handoff =
            view_model->runtime_track02_semantic_handoff;
        out_receipt->runtime_fallback_visuals_blocked =
            view_model->runtime_fallback_visuals_blocked;
        out_receipt->runtime_structured_route =
            view_model->runtime_structured_route;
        out_receipt->runtime_receipt_text_route =
            view_model->runtime_receipt_text_route;
        if (view_model->runtime_level_source !=
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED) {
            out_receipt->set_level_loaded = 1;
            out_receipt->level_loaded =
                view_model->runtime_level >= 0 ? view_model->runtime_level : 0;
        }
    }

    if (view_model->resume_claim != THERON_V1_STARTUP_RESUME_NONE) {
        out_receipt->set_save_resume = 1;
        out_receipt->save_resume_claim = view_model->resume_claim;
        out_receipt->save_resume_active_slot = view_model->tqsv_slot;
        out_receipt->save_resume_srm_active_slot = view_model->srm_slot;
        out_receipt->save_resume_srm_import_status =
            view_model->srm_import_status;
        out_receipt->save_resume_srm_current_dungeon =
            view_model->selected_dungeon;
        out_receipt->save_resume_srm_current_level =
            view_model->runtime_level;
        out_receipt->save_resume_srm_party_restored =
            view_model->runtime_champion_count > 0 ? 1 : 0;
        out_receipt->save_resume_srm_party_champion_count =
            view_model->runtime_champion_count;
        if (view_model->srm_root) {
            snprintf(out_receipt->save_resume_srm_root,
                     sizeof(out_receipt->save_resume_srm_root),
                     "%s",
                     view_model->srm_root);
        }
    }
    return 1;
}

int theron_v1_boot_startup_state_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    Theron_StartupStateReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        if (out_receipt) {
            theron_v1_startup_state_receipt_init(out_receipt);
        }
        return 0;
    }
    return theron_v1_boot_startup_state_receipt_from_view_model(
        &view_model,
        out_receipt);
}

static void tqr_boot_startup_input_receipt_clear(
    Theron_StartupInputReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = THERON_STARTUP_OK;
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
}

static void tqr_boot_startup_input_receipt_null(
    Theron_StartupInputReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    tqr_boot_startup_input_receipt_clear(receipt);
    receipt->result = THERON_STARTUP_ERR_NULL;
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    receipt->status_scope = "STARTUP";
    receipt->status = theron_v1_startup_result_name(
        THERON_STARTUP_ERR_NULL);
}

static const Theron_StartupLayoutElement *
tqr_boot_startup_view_model_focused_element(
    const Theron_V1_BootStartupViewModel *view_model)
{
    int i;

    if (!view_model || view_model->layout_count <= 0) {
        return NULL;
    }
    for (i = 0; i < view_model->layout_count; ++i) {
        if (view_model->layout[i].cursor) {
            return &view_model->layout[i];
        }
    }
    for (i = 0; i < view_model->layout_count; ++i) {
        if (view_model->layout[i].selected &&
            (view_model->layout[i].kind ==
                 THERON_STARTUP_LAYOUT_ELEMENT_STAGE ||
             view_model->layout[i].kind ==
                 THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE)) {
            return &view_model->layout[i];
        }
    }
    return &view_model->layout[0];
}

static const Theron_StartupLayoutElement *
tqr_boot_startup_view_model_first_element(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupLayoutElementKind kind)
{
    int i;

    if (!view_model) {
        return NULL;
    }
    for (i = 0; i < view_model->layout_count; ++i) {
        if (view_model->layout[i].kind == kind &&
            view_model->layout[i].enabled) {
            return &view_model->layout[i];
        }
    }
    return NULL;
}

static const Theron_StartupLayoutElement *
tqr_boot_startup_view_model_move_stage_focus(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupLayoutElement *focused,
    int delta,
    int *out_continue_focus)
{
    int i;
    int focused_index = -1;
    const Theron_StartupLayoutElement *continue_element = NULL;
    const Theron_StartupLayoutElement *first_stage = NULL;
    const Theron_StartupLayoutElement *last_stage = NULL;

    if (out_continue_focus) {
        *out_continue_focus = 0;
    }
    if (!view_model || !focused) {
        return NULL;
    }
    for (i = 0; i < view_model->layout_count; ++i) {
        const Theron_StartupLayoutElement *element = &view_model->layout[i];
        if (element == focused) {
            focused_index = i;
        }
        if (element->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE &&
            element->enabled) {
            continue_element = element;
        }
        if (element->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE &&
            element->enabled) {
            if (!first_stage) {
                first_stage = element;
            }
            last_stage = element;
        }
    }
    if (focused->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE) {
        return first_stage;
    }
    if (focused_index < 0) {
        return first_stage;
    }
    if (delta < 0) {
        for (i = focused_index - 1; i >= 0; --i) {
            const Theron_StartupLayoutElement *element =
                &view_model->layout[i];
            if (element->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE &&
                element->enabled) {
                return element;
            }
            if (element->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE &&
                element->enabled) {
                if (out_continue_focus) {
                    *out_continue_focus = 1;
                }
                return focused;
            }
        }
        if (continue_element) {
            if (out_continue_focus) {
                *out_continue_focus = 1;
            }
            return focused;
        }
        return first_stage ? first_stage : focused;
    }
    for (i = focused_index + 1; i < view_model->layout_count; ++i) {
        const Theron_StartupLayoutElement *element = &view_model->layout[i];
        if (element->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE &&
            element->enabled) {
            return element;
        }
    }
    return last_stage ? last_stage : focused;
}

static int tqr_boot_startup_session_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupSessionFacts *out_session)
{
    const char (*roster_names)
        [THERON_TRACK02_STARTUP_ROSTER_NAME_CAPACITY] = NULL;
    const char (*roster_titles)
        [THERON_TRACK02_STARTUP_ROSTER_TITLE_CAPACITY] = NULL;
    const char *startup_prompt = NULL;
    int roster_count = 0;

    if (!view_model || !out_session) {
        return 0;
    }
    if (view_model->startup_media_state_valid &&
        view_model->startup_media_state_receipt.startup_roster_name_status ==
            THERON_TRACK02_SIGNAL_OK) {
        roster_names =
            view_model->startup_media_state_receipt.startup_roster_names;
        roster_titles =
            view_model->startup_media_state_receipt.startup_roster_titles;
        roster_count =
            view_model->startup_media_state_receipt.startup_roster_name_count;
    }
    if (view_model->startup_media_state_valid &&
        view_model->startup_media_state_receipt.startup_text_prompt_status ==
            THERON_TRACK02_SIGNAL_OK &&
        view_model->startup_media_state_receipt.startup_text_prompt[0] !=
            '\0') {
        startup_prompt =
            view_model->startup_media_state_receipt.startup_text_prompt;
    }
    theron_v1_boot_startup_session_from_runtime_state(
        out_session,
        view_model->startup_phase,
        view_model->selected_dungeon,
        view_model->boot_profile,
        view_model->world,
        view_model->assets,
        view_model->startup_cursor,
        view_model->continue_focus,
        view_model->resume_claim,
        view_model->tqsv_slot,
        view_model->srm_slot,
        view_model->srm_import_status,
        view_model->srm_root,
        startup_prompt,
        roster_names,
        roster_titles,
        roster_count,
        view_model->selected_mirrors_mask,
        view_model->companion_count,
        view_model->selected_mirror_order,
        view_model->selected_mirror_order_count);
    return 1;
}

int theron_v1_boot_startup_execute_pointer_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    int x,
    int y,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt)
{
    Theron_StartupHit hit;
    Theron_StartupPhase phase = THERON_STARTUP_PHASE_TITLE;
    int handled;

    if (out_action) {
        theron_v1_startup_action_init(out_action);
    }
    if (out_receipt) {
        tqr_boot_startup_input_receipt_clear(out_receipt);
    }
    if (!view_model || !out_action || !out_receipt) {
        tqr_boot_startup_input_receipt_null(out_receipt);
        return 0;
    }

    if (view_model->layout_count > 0) {
        phase = view_model->layout[0].phase;
    }

    handled = theron_v1_startup_layout_hit_at(
        phase,
        view_model->layout,
        view_model->layout_count,
        x,
        y,
        &hit);
    if (!handled) {
        return 0;
    }

    switch (hit.kind) {
    case THERON_STARTUP_HIT_TITLE:
        out_action->kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    case THERON_STARTUP_HIT_CONTINUE:
        out_action->kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    case THERON_STARTUP_HIT_STAGE:
        out_action->kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
        out_action->selected_dungeon = hit.selected_dungeon;
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    case THERON_STARTUP_HIT_MIRROR:
        out_action->kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
        out_action->mirror_index = hit.mirror_index;
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    case THERON_STARTUP_HIT_FORCEFIELD:
        out_action->kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    case THERON_STARTUP_HIT_PANEL:
    case THERON_STARTUP_HIT_NONE:
    default:
        return 0;
    }
}

int theron_v1_boot_startup_execute_input_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupInput input,
    Theron_StartupAction *out_action,
    Theron_StartupInputReceipt *out_receipt)
{
    const Theron_StartupLayoutElement *focused;
    Theron_StartupPhase phase = THERON_STARTUP_PHASE_TITLE;
    int continue_focus = 0;

    if (out_action) {
        theron_v1_startup_action_init(out_action);
    }
    tqr_boot_startup_input_receipt_clear(out_receipt);
    if (!view_model || !out_action || !out_receipt) {
        tqr_boot_startup_input_receipt_null(out_receipt);
        return 0;
    }
    focused = tqr_boot_startup_view_model_focused_element(view_model);
    if (focused) {
        phase = focused->phase;
    }
    if (input == THERON_STARTUP_INPUT_NONE) {
        return 1;
    }
    if (input == THERON_STARTUP_INPUT_BACK) {
        out_action->kind =
            (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
             phase == THERON_STARTUP_PHASE_READY)
                ? THERON_STARTUP_ACTION_SHOW_STAGE_SELECT
                : THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER;
        out_receipt->input_result =
            out_action->kind == THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER
                ? THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER
                : THERON_STARTUP_INPUT_RESULT_REDRAW;
        return 1;
    }
    if (phase == THERON_STARTUP_PHASE_TITLE) {
        if (input == THERON_STARTUP_INPUT_ACCEPT ||
            input == THERON_STARTUP_INPUT_ACTION) {
            out_action->kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        }
        return 1;
    }
    if (phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        const Theron_StartupLayoutElement *target = focused;
        if (input == THERON_STARTUP_INPUT_UP ||
            input == THERON_STARTUP_INPUT_DOWN) {
            target = tqr_boot_startup_view_model_move_stage_focus(
                view_model,
                focused,
                input == THERON_STARTUP_INPUT_UP ? -1 : 1,
                &continue_focus);
            out_action->kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            out_action->continue_focus = continue_focus;
            if (target &&
                target->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE) {
                out_action->selected_dungeon = target->dungeon_id;
            }
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
            return 1;
        }
        if (input == THERON_STARTUP_INPUT_ACCEPT ||
            input == THERON_STARTUP_INPUT_ACTION) {
            if (focused &&
                focused->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE) {
                out_action->kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
            } else {
                target = focused && focused->kind ==
                                      THERON_STARTUP_LAYOUT_ELEMENT_STAGE
                             ? focused
                             : tqr_boot_startup_view_model_first_element(
                                   view_model,
                                   THERON_STARTUP_LAYOUT_ELEMENT_STAGE);
                out_action->kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
                if (target) {
                    out_action->selected_dungeon = target->dungeon_id;
                }
            }
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        }
        return 1;
    }
    if (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
        phase == THERON_STARTUP_PHASE_READY) {
        int cursor = 0;
        if (focused &&
            focused->kind == THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD) {
            cursor = THERON_STARTUP_HERO_MIRROR_COUNT;
        } else if (focused &&
                   focused->kind == THERON_STARTUP_LAYOUT_ELEMENT_MIRROR) {
            cursor = focused->mirror_index;
        }
        if (input == THERON_STARTUP_INPUT_LEFT ||
            input == THERON_STARTUP_INPUT_UP) {
            out_action->kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            out_action->cursor =
                (cursor + THERON_STARTUP_HERO_MIRROR_COUNT) %
                (THERON_STARTUP_HERO_MIRROR_COUNT + 1);
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
            return 1;
        }
        if (input == THERON_STARTUP_INPUT_RIGHT ||
            input == THERON_STARTUP_INPUT_DOWN) {
            out_action->kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            out_action->cursor =
                (cursor + 1) % (THERON_STARTUP_HERO_MIRROR_COUNT + 1);
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
            return 1;
        }
        if (input == THERON_STARTUP_INPUT_ACCEPT &&
            cursor < THERON_STARTUP_HERO_MIRROR_COUNT) {
            out_action->kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
            out_action->cursor = cursor;
            out_action->mirror_index = cursor;
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
            return 1;
        }
        if ((input == THERON_STARTUP_INPUT_ACCEPT &&
             cursor == THERON_STARTUP_HERO_MIRROR_COUNT) ||
            input == THERON_STARTUP_INPUT_ACTION) {
            out_action->kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
            out_action->cursor = cursor;
            out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
            return 1;
        }
    }
    return 1;
}

int theron_v1_boot_startup_execute_action_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupAction *action,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_StartupSessionFacts session;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!view_model || !action || !out_receipt) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
        }
        return 0;
    }
    if (!tqr_boot_startup_session_from_view_model(view_model, &session)) {
        out_receipt->result = THERON_STARTUP_ERR_NULL;
        out_receipt->host_receipt.input_result =
            THERON_STARTUP_INPUT_RESULT_REDRAW;
        out_receipt->host_receipt.status_scope = "STARTUP";
        out_receipt->host_receipt.status = theron_v1_startup_result_name(
            THERON_STARTUP_ERR_NULL);
        return 0;
    }
    return theron_v1_startup_execute_action_from_session_with_host_receipt(
        action,
        &session,
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_StartupInput input,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_StartupAction action;
    Theron_StartupInputReceipt input_receipt;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = theron_v1_startup_result_name(
                THERON_STARTUP_ERR_NULL);
        }
        return 0;
    }
    if (!theron_v1_boot_startup_execute_input_from_view_model(
            view_model,
            input,
            &action,
            &input_receipt)) {
        out_receipt->result = input_receipt.result;
        out_receipt->host_receipt.input_result =
            input_receipt.input_result;
        out_receipt->host_receipt.status_scope =
            input_receipt.status_scope;
        out_receipt->host_receipt.status = input_receipt.status;
        return 0;
    }
    return theron_v1_boot_startup_execute_action_from_view_model_with_host_receipt(
        view_model,
        &action,
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_StartupAction action;
    Theron_StartupInputReceipt pointer_receipt;

    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = theron_v1_startup_result_name(
                THERON_STARTUP_ERR_NULL);
        }
        return 0;
    }
    if (!theron_v1_boot_startup_execute_pointer_from_view_model(
            view_model,
            x,
            y,
            &action,
            &pointer_receipt)) {
        out_receipt->result = pointer_receipt.result;
        out_receipt->host_receipt.input_result =
            pointer_receipt.input_result;
        out_receipt->host_receipt.status_scope =
            pointer_receipt.status_scope;
        out_receipt->host_receipt.status = pointer_receipt.status;
        return 0;
    }
    return theron_v1_boot_startup_execute_action_from_view_model_with_host_receipt(
        view_model,
        &action,
        out_receipt);
}

int theron_v1_boot_startup_execute_graphics_plan_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor)
{
    Theron_V1_BootStartupGraphicsRouteReceipt receipt;

    return theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
        view_model,
        executor,
        &receipt);
}

void theron_v1_boot_startup_graphics_route_receipt_init(
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status_scope = "STARTUP";
    receipt->status = "NO GRAPHICS ROUTE";
}

static void theron_v1_boot_startup_mark_bitmap_routes(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt);
static void theron_v1_boot_startup_capture_track02_graphic_receipt(
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt);
static void theron_v1_boot_startup_copy_required_bitmap_routes(
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt);
static void theron_v1_boot_startup_copy_package_bitmap_routes(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int required_route_mask,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt);
static int theron_v1_boot_startup_media_has_required_atlas_routes(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int required_route_mask);

static int theron_v1_boot_startup_prepare_graphics_route_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    Theron_V1_BootStartupGraphicsRouteReceipt *out_receipt)
{
    Theron_V1_BootStartupRenderRouteReceipt render_route;

    if (!view_model || !out_receipt) {
        return 0;
    }
    out_receipt->host_consumes_view_model = 1;
    out_receipt->track02_real_media_ready =
        view_model->startup_media_state_valid &&
                view_model->startup_media_state_receipt.startup_media_ready
            ? 1
            : 0;
    if (!theron_v1_boot_startup_render_route_receipt_from_view_model(
            view_model,
            &render_route)) {
        return 0;
    }
    out_receipt->render_route = render_route;
    out_receipt->render_route_valid = 1;
    out_receipt->startup_menu_render_allowed =
        render_route.startup_menu_render_allowed;
    out_receipt->runtime_readiness_ready =
        render_route.runtime_readiness_ready;
    out_receipt->no_fallback_visuals_enforced =
        render_route.no_fallback_visuals_enforced;
    out_receipt->fallback_visuals_allowed =
        render_route.fallback_visuals_allowed;
    out_receipt->runtime_level_source =
        render_route.runtime_level_source;
    out_receipt->runtime_graphics_handoff =
        out_receipt->runtime_readiness_ready &&
        out_receipt->no_fallback_visuals_enforced &&
        (out_receipt->runtime_level_source ==
             THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC ||
         out_receipt->runtime_level_source ==
             THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME) ? 1 : 0;
    out_receipt->track02_runtime_graphics_handoff =
        out_receipt->runtime_graphics_handoff &&
                out_receipt->runtime_level_source ==
                    THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC
            ? 1
            : 0;
    out_receipt->save_resume_runtime_graphics_handoff =
        out_receipt->runtime_graphics_handoff &&
                out_receipt->runtime_level_source ==
                    THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME
            ? 1
            : 0;
    out_receipt->runtime_track02_semantic_handoff =
        render_route.runtime_track02_semantic_handoff;
    out_receipt->runtime_fallback_visuals_blocked =
        render_route.runtime_fallback_visuals_blocked;
    out_receipt->runtime_structured_route =
        render_route.runtime_structured_route;
    out_receipt->runtime_receipt_text_route =
        render_route.runtime_receipt_text_route;
    out_receipt->all_dungeon_real_data_capture_ready =
        render_route.all_dungeon_real_data_capture_ready;
    out_receipt->all_dungeon_capture_count =
        render_route.all_dungeon_capture_count;
    out_receipt->all_dungeon_capture_mask =
        render_route.all_dungeon_capture_mask;
    out_receipt->exact_level_semantics_ready =
        render_route.exact_level_semantics_ready;
    out_receipt->exact_object_semantics_ready =
        render_route.exact_object_semantics_ready;
    out_receipt->no_fallback_semantic_role_mask =
        render_route.no_fallback_semantic_role_mask;
    out_receipt->track02_state_predicates_consumed =
        render_route.track02_state_predicates_consumed;
    out_receipt->track02_bitmap_routes_complete =
        render_route.track02_bitmap_routes_complete;
    out_receipt->track02_no_fallback_runtime_route_ready =
        render_route.track02_no_fallback_runtime_route_ready;
    out_receipt->object_table_no_fallback_ready =
        render_route.object_table_no_fallback_ready;
    out_receipt->object_table_blocked_anchor_mask =
        render_route.object_table_blocked_anchor_mask;
    out_receipt->object_table_blocked_anchor_count =
        render_route.object_table_blocked_anchor_count;
    out_receipt->nonstartup_level_no_fallback_ready =
        render_route.nonstartup_level_no_fallback_ready;
    out_receipt->nonstartup_level_blocked_anchor_mask =
        render_route.nonstartup_level_blocked_anchor_mask;
    out_receipt->nonstartup_level_blocked_anchor_count =
        render_route.nonstartup_level_blocked_anchor_count;
    out_receipt->startup_level_blocked_anchor_mask =
        render_route.startup_level_blocked_anchor_mask;
    out_receipt->startup_level_blocked_anchor_count =
        render_route.startup_level_blocked_anchor_count;
    out_receipt->object_table_route_hash =
        render_route.object_table_route_hash;
    out_receipt->level_route_hash = render_route.level_route_hash;
    out_receipt->status_scope = render_route.status_scope;
    out_receipt->status = render_route.status;
    out_receipt->real_bitmap_startup_graphics_ready =
        render_route.render_plan_valid &&
                theron_v1_boot_startup_media_has_required_atlas_routes(
                    &view_model->startup_media_state_receipt,
                    render_route.render_plan.required_bitmap_route_mask)
            ? 1
            : 0;
    out_receipt->track02_atlas_startup_graphics_ready =
        out_receipt->real_bitmap_startup_graphics_ready;
    theron_v1_boot_startup_copy_package_bitmap_routes(
        &view_model->startup_media_state_receipt,
        render_route.render_plan.required_bitmap_route_mask,
        out_receipt);
    theron_v1_boot_startup_mark_bitmap_routes(
        view_model,
        &render_route.render_plan,
        out_receipt);
    theron_v1_boot_startup_copy_required_bitmap_routes(
        &render_route.render_plan,
        out_receipt);
    if (!out_receipt->real_bitmap_startup_graphics_ready) {
        out_receipt->required_bitmap_routes_ready = 0;
    }
    theron_v1_boot_startup_capture_track02_graphic_receipt(
        &render_route.render_plan,
        out_receipt);
    out_receipt->raw_graphics_plan_consumer_required =
        out_receipt->real_bitmap_startup_graphics_ready &&
                out_receipt->required_bitmap_routes_ready &&
                out_receipt->track02_atlas_startup_graphics_ready
            ? 0
            : 1;
    return 1;
}

static void theron_v1_boot_startup_mark_bitmap_route_ready(
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt,
    unsigned int route_bit)
{
    if (!receipt || route_bit == 0u) {
        return;
    }
    if ((receipt->bitmap_route_mask & route_bit) == 0u) {
        receipt->bitmap_route_mask |= route_bit;
        ++receipt->bitmap_route_count;
    }
    switch (route_bit) {
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
        receipt->title_bitmap_route_ready = 1;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
        receipt->stage_bitmap_route_ready = 1;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
        receipt->soul_room_bitmap_route_ready = 1;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
        receipt->forcefield_bitmap_route_ready = 1;
        break;
    default:
        break;
    }
}

static void theron_v1_boot_startup_mark_bitmap_routes(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    int i;
    unsigned int ready_mask;
    unsigned int required_mask;

    if (!view_model || !plan || !receipt ||
        !view_model->startup_media_state_valid) {
        return;
    }
    ready_mask =
        view_model->startup_media_state_receipt.startup_bitmap_route_mask;
    required_mask = plan->required_bitmap_route_mask;
    for (i = 0; i < plan->graphic_count &&
                i < THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX; ++i) {
        switch (plan->graphics[i].kind) {
        case THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK:
            if ((ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) == 0u) {
                break;
            }
            theron_v1_boot_startup_mark_bitmap_route_ready(
                receipt,
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL:
            if ((ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) == 0u) {
                break;
            }
            theron_v1_boot_startup_mark_bitmap_route_ready(
                receipt,
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME:
            if ((ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) == 0u) {
                break;
            }
            theron_v1_boot_startup_mark_bitmap_route_ready(
                receipt,
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD:
            if ((ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) == 0u) {
                break;
            }
            theron_v1_boot_startup_mark_bitmap_route_ready(
                receipt,
                THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD);
            break;
        default:
            break;
        }
    }
    if ((required_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE) &&
        (ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE)) {
        theron_v1_boot_startup_mark_bitmap_route_ready(
            receipt,
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE);
    }
    if ((required_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE) &&
        (ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE)) {
        theron_v1_boot_startup_mark_bitmap_route_ready(
            receipt,
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE);
    }
    if ((required_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM) &&
        (ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM)) {
        theron_v1_boot_startup_mark_bitmap_route_ready(
            receipt,
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM);
    }
    if ((required_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) &&
        (ready_mask & THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD)) {
        theron_v1_boot_startup_mark_bitmap_route_ready(
            receipt,
            THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD);
    }
}

static void theron_v1_boot_startup_copy_required_bitmap_routes(
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    unsigned int required;

    if (!plan || !receipt) {
        return;
    }
    required = plan->required_bitmap_route_mask;
    receipt->required_bitmap_route_mask = required;
    receipt->required_bitmap_route_count = plan->required_bitmap_route_count;
    receipt->required_bitmap_routes_ready =
        required != 0u &&
                (receipt->bitmap_route_mask & required) == required
            ? 1
            : 0;
}

static void theron_v1_boot_startup_capture_track02_graphic_receipt(
    const Theron_StartupRenderPlan *plan,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    int i;

    if (!plan || !receipt || !receipt->real_bitmap_startup_graphics_ready) {
        return;
    }
    for (i = 0; i < plan->graphic_count &&
                i < THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX; ++i) {
        const Theron_StartupRenderGraphicCommand *command =
            &plan->graphics[i];
        switch (command->kind) {
        case THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK:
        case THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL:
        case THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME:
        case THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD:
            receipt->track02_startup_graphic_receipt = *command;
            receipt->track02_startup_graphic_receipt_valid = 1;
            return;
        default:
            break;
        }
    }
}

static unsigned int theron_v1_boot_startup_graphic_route_bit(
    Theron_StartupRenderGraphicKind kind)
{
    switch (kind) {
    case THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK:
        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
    case THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL:
        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
    case THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME:
        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
    case THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD:
        return THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD;
    default:
        return 0u;
    }
}

static const Theron_Track02StartupBitmapAtlasRoute *
theron_v1_boot_startup_atlas_find_route(
    const Theron_Track02StartupBitmapAtlas *atlas,
    unsigned int route_bit)
{
    size_t i;

    if (!atlas || route_bit == 0u) {
        return NULL;
    }
    for (i = 0u; i < atlas->route_count; ++i) {
        if (atlas->routes[i].route_bit == route_bit &&
            atlas->routes[i].width > 0u &&
            atlas->routes[i].height > 0u &&
            atlas->routes[i].nonzero_pixel_count > 0u) {
            return &atlas->routes[i];
        }
    }
    return NULL;
}

static int theron_v1_boot_startup_media_route_has_dense_bitmap(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int route_bit);

static int theron_v1_boot_startup_media_has_required_atlas_routes(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int required_route_mask)
{
    unsigned int bit;

    if (!media_receipt || required_route_mask == 0u ||
        !media_receipt->startup_media_ready ||
        media_receipt->startup_bitmap_decode_status !=
            THERON_TRACK02_SIGNAL_OK ||
        !media_receipt->startup_bitmap_atlas_ready ||
        media_receipt->startup_bitmap_atlas.route_count == 0u ||
        media_receipt->startup_bitmap_atlas_nonzero_pixel_count == 0u ||
        media_receipt->startup_bitmap_atlas_checksum == 0u ||
        (media_receipt->startup_bitmap_route_mask & required_route_mask) !=
            required_route_mask ||
        (media_receipt->startup_bitmap_atlas_route_mask &
             required_route_mask) != required_route_mask ||
        (media_receipt->startup_bitmap_atlas.route_mask &
             required_route_mask) != required_route_mask) {
        return 0;
    }

    for (bit = 1u; bit != 0u; bit <<= 1u) {
        if ((required_route_mask & bit) == 0u) {
            continue;
        }
        if (!theron_v1_boot_startup_atlas_find_route(
                &media_receipt->startup_bitmap_atlas,
                bit)) {
            return 0;
        }
        if (!theron_v1_boot_startup_media_route_has_dense_bitmap(
                media_receipt,
                bit)) {
            return 0;
        }
        if (bit >= THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD) {
            break;
        }
    }
    switch (media_receipt->track02_variant) {
    case THERON_TRACK02_VARIANT_JP_BIN:
    case THERON_TRACK02_VARIANT_US_BIN:
        return (media_receipt->startup_bitmap_raw_route_mask &
                required_route_mask) == required_route_mask;
    case THERON_TRACK02_VARIANT_JP_REV1_ISO:
    case THERON_TRACK02_VARIANT_US_ISO:
        return (media_receipt->startup_bitmap_iso_route_mask &
                required_route_mask) == required_route_mask;
    default:
        return 0;
    }
}

static void theron_v1_boot_startup_copy_package_bitmap_routes(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int required_route_mask,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    unsigned int package_mask;

    if (!media_receipt || !receipt) {
        return;
    }
    receipt->raw_bitmap_route_mask =
        media_receipt->startup_bitmap_raw_route_mask;
    receipt->raw_bitmap_route_count =
        media_receipt->startup_bitmap_raw_route_count;
    receipt->raw_bitmap_atlas_tile_count =
        media_receipt->startup_bitmap_raw_atlas_tile_count;
    receipt->iso_bitmap_route_mask =
        media_receipt->startup_bitmap_iso_route_mask;
    receipt->iso_bitmap_route_count =
        media_receipt->startup_bitmap_iso_route_count;
    receipt->iso_bitmap_atlas_tile_count =
        media_receipt->startup_bitmap_iso_atlas_tile_count;
    switch (media_receipt->track02_variant) {
    case THERON_TRACK02_VARIANT_JP_BIN:
    case THERON_TRACK02_VARIANT_US_BIN:
        package_mask = receipt->raw_bitmap_route_mask;
        break;
    case THERON_TRACK02_VARIANT_JP_REV1_ISO:
    case THERON_TRACK02_VARIANT_US_ISO:
        package_mask = receipt->iso_bitmap_route_mask;
        break;
    default:
        package_mask = 0u;
        break;
    }
    receipt->bitmap_package_route_ready =
        required_route_mask != 0u &&
                (package_mask & required_route_mask) == required_route_mask
            ? 1
            : 0;
}

static int theron_v1_boot_startup_media_route_has_dense_bitmap(
    const Theron_StartupMediaStateReceipt *media_receipt,
    unsigned int route_bit)
{
    size_t tile_count = 0u;
    size_t min_tile_count = 2u;
    uint16_t width = 0u;
    uint16_t min_width = 16u;

    if (!media_receipt) {
        return 0;
    }
    switch (route_bit) {
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE:
        tile_count = media_receipt->startup_bitmap_title_atlas_tile_count;
        width = media_receipt->startup_bitmap_title_atlas_width;
        min_tile_count = 8u;
        min_width = 64u;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE:
        tile_count = media_receipt->startup_bitmap_stage_atlas_tile_count;
        width = media_receipt->startup_bitmap_stage_atlas_width;
        min_tile_count = 8u;
        min_width = 64u;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM:
        tile_count = media_receipt->startup_bitmap_soul_room_atlas_tile_count;
        width = media_receipt->startup_bitmap_soul_room_atlas_width;
        min_tile_count = 8u;
        min_width = 64u;
        break;
    case THERON_TRACK02_STARTUP_BITMAP_ROUTE_FORCEFIELD:
        tile_count = media_receipt->startup_bitmap_forcefield_atlas_tile_count;
        width = media_receipt->startup_bitmap_forcefield_atlas_width;
        min_tile_count = 8u;
        min_width = 64u;
        break;
    default:
        return 0;
    }
    return tile_count >= min_tile_count && width >= min_width;
}

static void theron_v1_boot_startup_draw_atlas_route(
    const Theron_StartupRenderGraphicCommand *command,
    const Theron_Track02StartupBitmapAtlasRoute *route,
    const Theron_StartupGraphicExecutor *executor)
{
    int dx;
    int dy;

    if (!command || !route || !executor || !executor->plot_pixel ||
        command->w <= 0 || command->h <= 0 ||
        route->width == 0u || route->height == 0u) {
        return;
    }
    for (dy = 0; dy < command->h; ++dy) {
        size_t sy = ((size_t)dy * (size_t)route->height) /
                    (size_t)command->h;
        for (dx = 0; dx < command->w; ++dx) {
            size_t sx = ((size_t)dx * (size_t)route->width) /
                        (size_t)command->w;
            uint8_t color =
                route->pixels[sy *
                              THERON_TRACK02_STARTUP_BITMAP_ATLAS_MAX_WIDTH +
                              sx];
            executor->plot_pixel(executor->userdata,
                                 command->x + dx,
                                 command->y + dy,
                                 (int)(color & 0x0fu));
        }
    }
}

static int theron_v1_boot_startup_execute_atlas_graphics_plan(
    const Theron_StartupRenderPlan *plan,
    const Theron_StartupMediaStateReceipt *media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupGraphicsRouteReceipt *receipt)
{
    int i;
    int executed = 0;

    if (!plan || !media_receipt || !executor || !receipt ||
        !media_receipt->startup_bitmap_atlas_ready ||
        media_receipt->startup_bitmap_atlas.route_count == 0u) {
        return 0;
    }
    for (i = 0; i < plan->graphic_count &&
                i < THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX; ++i) {
        const Theron_StartupRenderGraphicCommand *command =
            &plan->graphics[i];
        unsigned int route_bit =
            theron_v1_boot_startup_graphic_route_bit(command->kind);
        const Theron_Track02StartupBitmapAtlasRoute *route;

        if (command->kind == THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT ||
            command->kind == THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT) {
            /* The plan's panel and cursor primitives are Firestaff-owned
             * placeholders.  A complete, authenticated Track 02 atlas owns
             * this frame, so leave regions without source pixels untouched
             * until an original loader/CD capture identifies their art. */
            continue;
        }
        route = theron_v1_boot_startup_atlas_find_route(
            &media_receipt->startup_bitmap_atlas,
            route_bit);
        if (!route) {
            continue;
        }
        theron_v1_boot_startup_draw_atlas_route(command, route, executor);
        if (!receipt->track02_startup_graphic_receipt_valid) {
            receipt->track02_startup_graphic_receipt = *command;
            receipt->track02_startup_graphic_receipt_valid = 1;
        }
        receipt->track02_atlas_graphics_route_mask |= route_bit;
        ++receipt->track02_atlas_graphics_route_count;
        receipt->track02_atlas_graphics_pixel_count +=
            route->nonzero_pixel_count;
        receipt->track02_atlas_graphics_checksum ^=
            route->checksum + (uint32_t)(route_bit * 2654435761u);
        executed = 1;
    }
    return executed;
}

int theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupGraphicsRouteReceipt *out_receipt)
{
    Theron_StartupRenderPlan plan;
    Theron_V1_BootStartupRenderRouteReceipt render_route;

    if (out_receipt) {
        theron_v1_boot_startup_graphics_route_receipt_init(out_receipt);
    }
    if (!view_model || !executor || !out_receipt) {
        return 0;
    }
    out_receipt->host_consumes_view_model = 1;
    out_receipt->track02_real_media_ready =
        view_model->startup_media_state_valid &&
                view_model->startup_media_state_receipt.startup_media_ready
            ? 1
            : 0;
    if (theron_v1_boot_startup_render_route_receipt_from_view_model(
            view_model,
            &render_route)) {
        out_receipt->render_route = render_route;
        out_receipt->render_route_valid = 1;
        out_receipt->startup_menu_render_allowed =
            render_route.startup_menu_render_allowed;
        out_receipt->runtime_readiness_ready =
            render_route.runtime_readiness_ready;
        out_receipt->no_fallback_visuals_enforced =
            render_route.no_fallback_visuals_enforced;
        out_receipt->fallback_visuals_allowed =
            render_route.fallback_visuals_allowed;
        out_receipt->runtime_level_source =
            render_route.runtime_level_source;
        out_receipt->runtime_graphics_handoff =
            out_receipt->runtime_readiness_ready &&
            out_receipt->no_fallback_visuals_enforced &&
            (out_receipt->runtime_level_source ==
                 THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC ||
             out_receipt->runtime_level_source ==
                 THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME) ? 1 : 0;
        out_receipt->track02_runtime_graphics_handoff =
            out_receipt->runtime_graphics_handoff &&
                    out_receipt->runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC
                ? 1
                : 0;
        out_receipt->save_resume_runtime_graphics_handoff =
            out_receipt->runtime_graphics_handoff &&
                    out_receipt->runtime_level_source ==
                        THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME
                ? 1
                : 0;
        out_receipt->runtime_track02_semantic_handoff =
            render_route.runtime_track02_semantic_handoff;
        out_receipt->runtime_fallback_visuals_blocked =
            render_route.runtime_fallback_visuals_blocked;
        out_receipt->runtime_structured_route =
            render_route.runtime_structured_route;
        out_receipt->runtime_receipt_text_route =
            render_route.runtime_receipt_text_route;
        out_receipt->all_dungeon_real_data_capture_ready =
            render_route.all_dungeon_real_data_capture_ready;
        out_receipt->all_dungeon_capture_count =
            render_route.all_dungeon_capture_count;
        out_receipt->all_dungeon_capture_mask =
            render_route.all_dungeon_capture_mask;
        out_receipt->exact_level_semantics_ready =
            render_route.exact_level_semantics_ready;
        out_receipt->exact_object_semantics_ready =
            render_route.exact_object_semantics_ready;
        out_receipt->no_fallback_semantic_role_mask =
            render_route.no_fallback_semantic_role_mask;
        out_receipt->track02_state_predicates_consumed =
            render_route.track02_state_predicates_consumed;
        out_receipt->track02_bitmap_routes_complete =
            render_route.track02_bitmap_routes_complete;
        out_receipt->track02_no_fallback_runtime_route_ready =
            render_route.track02_no_fallback_runtime_route_ready;
        out_receipt->object_table_no_fallback_ready =
            render_route.object_table_no_fallback_ready;
        out_receipt->object_table_blocked_anchor_mask =
            render_route.object_table_blocked_anchor_mask;
        out_receipt->object_table_blocked_anchor_count =
            render_route.object_table_blocked_anchor_count;
        out_receipt->nonstartup_level_no_fallback_ready =
            render_route.nonstartup_level_no_fallback_ready;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            render_route.nonstartup_level_blocked_anchor_mask;
        out_receipt->nonstartup_level_blocked_anchor_count =
            render_route.nonstartup_level_blocked_anchor_count;
        out_receipt->startup_level_blocked_anchor_mask =
            render_route.startup_level_blocked_anchor_mask;
        out_receipt->startup_level_blocked_anchor_count =
            render_route.startup_level_blocked_anchor_count;
        out_receipt->object_table_route_hash =
            render_route.object_table_route_hash;
        out_receipt->level_route_hash = render_route.level_route_hash;
        out_receipt->status_scope = render_route.status_scope;
        out_receipt->status = render_route.status;
        out_receipt->real_bitmap_startup_graphics_ready =
            render_route.render_plan_valid &&
                    theron_v1_boot_startup_media_has_required_atlas_routes(
                        &view_model->startup_media_state_receipt,
                        render_route.render_plan.required_bitmap_route_mask)
                ? 1
                : 0;
        out_receipt->track02_atlas_startup_graphics_ready =
            out_receipt->real_bitmap_startup_graphics_ready;
        theron_v1_boot_startup_copy_package_bitmap_routes(
            &view_model->startup_media_state_receipt,
            render_route.render_plan.required_bitmap_route_mask,
            out_receipt);
        theron_v1_boot_startup_mark_bitmap_routes(
            view_model,
            &render_route.render_plan,
            out_receipt);
        theron_v1_boot_startup_copy_required_bitmap_routes(
            &render_route.render_plan,
            out_receipt);
        if (!out_receipt->real_bitmap_startup_graphics_ready) {
            out_receipt->required_bitmap_routes_ready = 0;
        }
        theron_v1_boot_startup_capture_track02_graphic_receipt(
            &render_route.render_plan,
            out_receipt);
        out_receipt->raw_graphics_plan_consumer_required =
            out_receipt->real_bitmap_startup_graphics_ready &&
                    out_receipt->required_bitmap_routes_ready &&
                    out_receipt->track02_atlas_startup_graphics_ready
                ? 0
                : 1;
    }
    if (out_receipt->runtime_fallback_visuals_blocked ||
        out_receipt->runtime_level_source ==
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_BLOCKED) {
        out_receipt->graphics_blocked = 1;
        out_receipt->no_fallback_startup_graphics_proof =
            out_receipt->no_fallback_visuals_enforced ? 1 : 0;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "TRACK02 GRAPHICS BLOCKED";
        return 0;
    }
    if (out_receipt->runtime_readiness_ready &&
        out_receipt->no_fallback_visuals_enforced &&
        out_receipt->runtime_level_source ==
            THERON_V1_STARTUP_RUNTIME_LEVEL_TRACK02_SEMANTIC) {
        out_receipt->graphics_blocked = 1;
        out_receipt->no_fallback_startup_graphics_proof = 1;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "TRACK02 RUNTIME GRAPHICS HANDOFF";
        return 0;
    }
    if (out_receipt->runtime_readiness_ready &&
        out_receipt->no_fallback_visuals_enforced &&
        out_receipt->runtime_level_source ==
            THERON_V1_STARTUP_RUNTIME_LEVEL_SAVE_RESUME) {
        out_receipt->graphics_blocked = 1;
        out_receipt->no_fallback_startup_graphics_proof = 1;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "SAVE RESUME RUNTIME GRAPHICS HANDOFF";
        return 0;
    }

    if (!theron_v1_boot_startup_render_plan_from_view_model(
            view_model,
            &plan)) {
        return 0;
    }
    out_receipt->graphics_plan_valid = 1;
    if (!out_receipt->startup_menu_render_allowed &&
        !out_receipt->runtime_readiness_ready) {
        out_receipt->graphics_blocked = 1;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "GRAPHICS ROUTE BLOCKED";
        return 0;
    }
    if (out_receipt->track02_atlas_startup_graphics_ready &&
        out_receipt->required_bitmap_routes_ready) {
        out_receipt->graphics_executed =
            theron_v1_boot_startup_execute_atlas_graphics_plan(
                &plan,
                &view_model->startup_media_state_receipt,
                executor,
                out_receipt)
                ? 1
                : 0;
        out_receipt->track02_atlas_startup_graphics_executed =
            out_receipt->graphics_executed ? 1 : 0;
    } else if (out_receipt->track02_real_media_ready) {
        /* A hash-verified Track 02 is authoritative.  Missing or thin
         * regions are a data-route failure, not permission to substitute
         * command-drawn title, stage, Soul Room, or forcefield graphics. */
        out_receipt->graphics_blocked = 1;
        out_receipt->no_fallback_startup_graphics_proof = 1;
        out_receipt->fallback_visuals_allowed = 0;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "TRACK02 ATLAS ROUTE MISSING";
        return 0;
    } else {
        /* No verified Track 02 media is present.  Do not manufacture a
         * title/stage/Soul Room/forcefield surface; startup is unavailable
         * until the real atlas route is bound. */
        out_receipt->graphics_blocked = 1;
        out_receipt->no_fallback_startup_graphics_proof = 1;
        out_receipt->fallback_visuals_allowed = 0;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "NO VERIFIED TRACK02 GRAPHICS";
        return 0;
    }
    out_receipt->track02_startup_graphics_executed =
        out_receipt->graphics_executed &&
                out_receipt->track02_atlas_startup_graphics_executed &&
                out_receipt->track02_startup_graphic_receipt_valid
            ? 1
            : 0;
    out_receipt->fallback_startup_graphics_executed =
        out_receipt->graphics_executed &&
                out_receipt->fallback_visuals_allowed &&
                !out_receipt->track02_startup_graphics_executed
            ? 1
            : 0;
    if (!out_receipt->graphics_executed) {
        out_receipt->graphics_blocked = 1;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "GRAPHICS EXECUTOR BLOCKED";
    } else if (out_receipt->track02_atlas_startup_graphics_executed) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "TRACK02 ATLAS GRAPHICS EXECUTED";
    }
    return out_receipt->graphics_executed;
}

int theron_v1_boot_startup_execute_graphics_plan_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupGraphicsRouteReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        theron_v1_boot_startup_graphics_route_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        return 0;
    }
    return theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
        &view_model,
        executor,
        out_receipt);
}

void theron_v1_boot_startup_full_start_receipt_init(
    Theron_V1_BootStartupFullStartReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status_scope = "STARTUP";
    receipt->status = "NO FULL START";
}

int theron_v1_boot_startup_full_start_receipt_from_view_model(
    const Theron_V1_BootStartupViewModel *view_model,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupFullStartReceipt *out_receipt)
{
    if (out_receipt) {
        theron_v1_boot_startup_full_start_receipt_init(out_receipt);
    }
    if (!view_model || !out_receipt) {
        return 0;
    }

    out_receipt->host_consumes_view_model = 1;
    out_receipt->view_model_valid = 1;
    out_receipt->view_model = *view_model;
    out_receipt->stage_menu_ready =
        view_model->startup_phase != THERON_STARTUP_PHASE_TITLE ? 1 : 0;
    out_receipt->soul_room_menu_ready =
        view_model->startup_phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
                view_model->startup_phase == THERON_STARTUP_PHASE_READY
            ? 1
            : 0;
    out_receipt->forcefield_menu_ready =
        view_model->startup_phase == THERON_STARTUP_PHASE_READY ? 1 : 0;

    if (theron_v1_boot_startup_host_view_receipt_from_view_model(
            view_model,
            &out_receipt->host_view)) {
        out_receipt->host_view_valid = 1;
        out_receipt->title_menu_ready =
            out_receipt->host_view.render_route.track02_title_menu_ready;
        out_receipt->save_resume_start_ready =
            out_receipt->host_view.render_route.save_resume_start_ready;
        out_receipt->save_resume_runtime_handoff_ready =
            out_receipt->host_view.save_resume_runtime_handoff_ready;
        out_receipt->no_fallback_visuals_enforced =
            out_receipt->host_view.no_fallback_visuals_enforced;
        out_receipt->fallback_visuals_allowed =
            out_receipt->host_view.fallback_visuals_allowed;
        out_receipt->runtime_readiness_ready =
            out_receipt->host_view.runtime_readiness_ready;
        out_receipt->runtime_level_render_allowed =
            out_receipt->host_view.runtime_level_render_allowed;
        out_receipt->runtime_level = view_model->runtime_level;
        out_receipt->runtime_champion_count =
            view_model->runtime_champion_count;
        out_receipt->runtime_level_source =
            out_receipt->host_view.runtime_level_source;
        out_receipt->runtime_track02_semantic_handoff =
            out_receipt->host_view.runtime_track02_semantic_handoff;
        out_receipt->runtime_fallback_visuals_blocked =
            out_receipt->host_view.runtime_fallback_visuals_blocked;
        out_receipt->runtime_structured_route =
            out_receipt->host_view.runtime_structured_route;
        out_receipt->runtime_receipt_text_route =
            out_receipt->host_view.runtime_receipt_text_route;
        out_receipt->all_dungeon_real_data_capture_ready =
            out_receipt->host_view.all_dungeon_real_data_capture_ready;
        out_receipt->all_dungeon_capture_count =
            out_receipt->host_view.all_dungeon_capture_count;
        out_receipt->all_dungeon_capture_mask =
            out_receipt->host_view.all_dungeon_capture_mask;
        out_receipt->exact_level_semantics_ready =
            out_receipt->host_view.exact_level_semantics_ready;
        out_receipt->exact_object_semantics_ready =
            out_receipt->host_view.exact_object_semantics_ready;
        out_receipt->no_fallback_semantic_role_mask =
            out_receipt->host_view.no_fallback_semantic_role_mask;
        out_receipt->track02_state_predicates_consumed =
            out_receipt->host_view.track02_state_predicates_consumed;
        out_receipt->track02_bitmap_routes_complete =
            out_receipt->host_view.track02_bitmap_routes_complete;
        out_receipt->track02_no_fallback_runtime_route_ready =
            out_receipt->host_view
                .track02_no_fallback_runtime_route_ready;
        out_receipt->object_table_no_fallback_ready =
            out_receipt->host_view.object_table_no_fallback_ready;
        out_receipt->object_table_blocked_anchor_mask =
            out_receipt->host_view.object_table_blocked_anchor_mask;
        out_receipt->object_table_blocked_anchor_count =
            out_receipt->host_view.object_table_blocked_anchor_count;
        out_receipt->nonstartup_level_no_fallback_ready =
            out_receipt->host_view.nonstartup_level_no_fallback_ready;
        out_receipt->nonstartup_level_blocked_anchor_mask =
            out_receipt->host_view.nonstartup_level_blocked_anchor_mask;
        out_receipt->nonstartup_level_blocked_anchor_count =
            out_receipt->host_view.nonstartup_level_blocked_anchor_count;
        out_receipt->startup_level_blocked_anchor_mask =
            out_receipt->host_view.startup_level_blocked_anchor_mask;
        out_receipt->startup_level_blocked_anchor_count =
            out_receipt->host_view.startup_level_blocked_anchor_count;
        out_receipt->object_table_route_hash =
            out_receipt->host_view.object_table_route_hash;
        out_receipt->level_route_hash =
            out_receipt->host_view.level_route_hash;
        out_receipt->hud_ready = out_receipt->host_view.hud_ready;
        out_receipt->runtime_graphics_handoff =
            out_receipt->host_view.runtime_graphics_handoff;
        out_receipt->track02_runtime_graphics_handoff =
            out_receipt->host_view.track02_runtime_graphics_handoff;
        out_receipt->save_resume_runtime_graphics_handoff =
            out_receipt->host_view.save_resume_runtime_graphics_handoff;
        out_receipt->raw_prompt_roster_required =
            out_receipt->host_view.raw_prompt_roster_required;
        out_receipt->raw_session_rebuild_required =
            out_receipt->host_view.raw_session_rebuild_required;
        out_receipt->forcefield_runtime_handoff_ready =
            out_receipt->forcefield_menu_ready &&
            out_receipt->host_view.runtime_readiness_ready ? 1 : 0;
        out_receipt->status_scope = out_receipt->host_view.status_scope;
        out_receipt->status = out_receipt->host_view.status;
    }
    if (executor &&
        theron_v1_boot_startup_execute_graphics_plan_from_view_model_with_route_receipt(
            view_model,
            executor,
            &out_receipt->graphics_route)) {
        out_receipt->graphics_route_valid = 1;
    } else if (executor &&
               out_receipt->graphics_route.render_route_valid) {
        out_receipt->graphics_route_valid = 1;
    } else if (!executor &&
               theron_v1_boot_startup_prepare_graphics_route_receipt(
                   view_model,
                   &out_receipt->graphics_route)) {
        out_receipt->graphics_route_valid = 1;
    }
    if (out_receipt->graphics_route_valid) {
        out_receipt->full_start_graphics_executed =
            out_receipt->graphics_route.graphics_executed;
        out_receipt->full_start_graphics_blocked =
            out_receipt->graphics_route.graphics_blocked;
        out_receipt->track02_real_media_ready =
            out_receipt->graphics_route.track02_real_media_ready;
        out_receipt->real_bitmap_startup_graphics_ready =
            out_receipt->graphics_route.real_bitmap_startup_graphics_ready;
        out_receipt->track02_atlas_startup_graphics_ready =
            out_receipt->graphics_route.track02_atlas_startup_graphics_ready;
        out_receipt->track02_atlas_startup_graphics_executed =
            out_receipt->graphics_route.track02_atlas_startup_graphics_executed;
        out_receipt->track02_atlas_graphics_route_mask =
            out_receipt->graphics_route.track02_atlas_graphics_route_mask;
        out_receipt->track02_atlas_graphics_route_count =
            out_receipt->graphics_route.track02_atlas_graphics_route_count;
        out_receipt->track02_atlas_graphics_pixel_count =
            out_receipt->graphics_route.track02_atlas_graphics_pixel_count;
        out_receipt->track02_atlas_graphics_checksum =
            out_receipt->graphics_route.track02_atlas_graphics_checksum;
        out_receipt->track02_startup_graphics_executed =
            out_receipt->graphics_route.track02_startup_graphics_executed;
        out_receipt->track02_startup_graphic_receipt_valid =
            out_receipt->graphics_route.track02_startup_graphic_receipt_valid;
        out_receipt->track02_startup_graphic_receipt =
            out_receipt->graphics_route.track02_startup_graphic_receipt;
        out_receipt->required_bitmap_route_mask =
            out_receipt->graphics_route.required_bitmap_route_mask;
        out_receipt->required_bitmap_route_count =
            out_receipt->graphics_route.required_bitmap_route_count;
        out_receipt->required_bitmap_routes_ready =
            out_receipt->graphics_route.required_bitmap_routes_ready;
        out_receipt->bitmap_route_mask =
            out_receipt->graphics_route.bitmap_route_mask;
        out_receipt->bitmap_route_count =
            out_receipt->graphics_route.bitmap_route_count;
        out_receipt->raw_bitmap_route_mask =
            out_receipt->graphics_route.raw_bitmap_route_mask;
        out_receipt->raw_bitmap_route_count =
            out_receipt->graphics_route.raw_bitmap_route_count;
        out_receipt->raw_bitmap_atlas_tile_count =
            out_receipt->graphics_route.raw_bitmap_atlas_tile_count;
        out_receipt->iso_bitmap_route_mask =
            out_receipt->graphics_route.iso_bitmap_route_mask;
        out_receipt->iso_bitmap_route_count =
            out_receipt->graphics_route.iso_bitmap_route_count;
        out_receipt->iso_bitmap_atlas_tile_count =
            out_receipt->graphics_route.iso_bitmap_atlas_tile_count;
        out_receipt->bitmap_package_route_ready =
            out_receipt->graphics_route.bitmap_package_route_ready;
        out_receipt->title_bitmap_route_ready =
            out_receipt->graphics_route.title_bitmap_route_ready;
        out_receipt->stage_bitmap_route_ready =
            out_receipt->graphics_route.stage_bitmap_route_ready;
        out_receipt->soul_room_bitmap_route_ready =
            out_receipt->graphics_route.soul_room_bitmap_route_ready;
        out_receipt->forcefield_bitmap_route_ready =
            out_receipt->graphics_route.forcefield_bitmap_route_ready;
        out_receipt->raw_graphics_plan_consumer_required =
            out_receipt->graphics_route.raw_graphics_plan_consumer_required;
        out_receipt->no_fallback_startup_graphics_proof =
            out_receipt->graphics_route.no_fallback_startup_graphics_proof;
        out_receipt->fallback_startup_graphics_executed =
            out_receipt->graphics_route.fallback_startup_graphics_executed;
    }
    out_receipt->full_start_graphics_ready =
        out_receipt->full_start_graphics_executed ||
        out_receipt->runtime_graphics_handoff ||
        out_receipt->forcefield_runtime_handoff_ready ? 1 : 0;
    if (out_receipt->forcefield_runtime_handoff_ready &&
        out_receipt->runtime_graphics_handoff) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "FORCEFIELD RUNTIME HANDOFF";
    } else if (out_receipt->full_start_graphics_ready) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "FULL START GRAPHICS READY";
    } else if (out_receipt->host_view_valid) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "FULL START MENU READY";
    }
    return out_receipt->host_view_valid || out_receipt->graphics_route_valid;
}

int theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    Theron_V1_BootStartupFullStartReceipt *out_receipt)
{
    Theron_V1_BootStartupViewModel view_model;

    if (out_receipt) {
        theron_v1_boot_startup_full_start_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_view_model_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            &view_model)) {
        return 0;
    }
    return theron_v1_boot_startup_full_start_receipt_from_view_model(
        &view_model,
        executor,
        out_receipt);
}

int theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupFullStartReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    Theron_V1_BootRuntimeStartupSnapshot snapshot;

    if (out_receipt) {
        theron_v1_boot_startup_full_start_receipt_init(out_receipt);
    }
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.startup_phase = startup_phase;
    snapshot.selected_dungeon = selected_dungeon;
    snapshot.boot_profile = boot_profile;
    snapshot.world = world;
    snapshot.assets = assets;
    snapshot.startup_cursor = startup_cursor;
    snapshot.continue_focus = continue_focus;
    snapshot.resume_claim = resume_claim;
    snapshot.tqsv_slot = tqsv_slot;
    snapshot.srm_slot = srm_slot;
    snapshot.srm_import_status = srm_import_status;
    snapshot.srm_root = srm_root;
    snapshot.selected_mirrors_mask = selected_mirrors_mask;
    snapshot.companion_count = companion_count;
    snapshot.selected_mirror_order = selected_mirror_order;
    snapshot.selected_mirror_order_count = selected_mirror_order_count;
    return theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
        &snapshot,
        startup_media_receipt,
        executor,
        out_receipt);
}

int theron_v1_boot_startup_full_start_receipt_from_runtime_route_with_media_receipt(
    Theron_V1_BootStartupFullStartReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int runtime_level_source,
    int runtime_track02_semantic_handoff,
    int runtime_fallback_visuals_blocked,
    int runtime_structured_route,
    int runtime_receipt_text_route,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    Theron_V1_BootRuntimeStartupSnapshot snapshot;

    if (out_receipt) {
        theron_v1_boot_startup_full_start_receipt_init(out_receipt);
    }
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.startup_phase = startup_phase;
    snapshot.selected_dungeon = selected_dungeon;
    snapshot.boot_profile = boot_profile;
    snapshot.world = world;
    snapshot.assets = assets;
    snapshot.startup_cursor = startup_cursor;
    snapshot.continue_focus = continue_focus;
    snapshot.resume_claim = resume_claim;
    snapshot.tqsv_slot = tqsv_slot;
    snapshot.srm_slot = srm_slot;
    snapshot.srm_import_status = srm_import_status;
    snapshot.srm_root = srm_root;
    snapshot.runtime_level_source = runtime_level_source;
    snapshot.runtime_track02_semantic_handoff =
        runtime_track02_semantic_handoff ? 1 : 0;
    snapshot.runtime_fallback_visuals_blocked =
        runtime_fallback_visuals_blocked ? 1 : 0;
    snapshot.runtime_structured_route = runtime_structured_route ? 1 : 0;
    snapshot.runtime_receipt_text_route =
        runtime_receipt_text_route ? 1 : 0;
    snapshot.selected_mirrors_mask = selected_mirrors_mask;
    snapshot.companion_count = companion_count;
    snapshot.selected_mirror_order = selected_mirror_order;
    snapshot.selected_mirror_order_count = selected_mirror_order_count;
    return theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
        &snapshot,
        startup_media_receipt,
        executor,
        out_receipt);
}

int theron_v1_boot_startup_host_view_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupHostViewReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    Theron_V1_BootStartupFullStartReceipt receipt;

    if (out_receipt) {
        theron_v1_boot_startup_host_view_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
            &receipt,
            startup_media_receipt,
            NULL,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        return 0;
    }
    return theron_v1_boot_startup_host_view_from_full_start_receipt(
        &receipt,
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_StartupInput input,
    Theron_StartupActionHostReceipt *out_receipt)
{
    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!receipt || !receipt->view_model_valid || !out_receipt) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = "FULL START RECEIPT MISSING";
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_input_from_view_model_with_host_receipt(
        &receipt->view_model,
        input,
        out_receipt);
}

int theron_v1_boot_startup_execute_pointer_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int x,
    int y,
    Theron_StartupActionHostReceipt *out_receipt)
{
    if (out_receipt) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
    }
    if (!receipt || !receipt->view_model_valid || !out_receipt) {
        if (out_receipt) {
            out_receipt->result = THERON_STARTUP_ERR_NULL;
            out_receipt->host_receipt.input_result =
                THERON_STARTUP_INPUT_RESULT_REDRAW;
            out_receipt->host_receipt.status_scope = "STARTUP";
            out_receipt->host_receipt.status = "FULL START RECEIPT MISSING";
        }
        return 0;
    }
    return theron_v1_boot_startup_execute_pointer_from_view_model_with_host_receipt(
        &receipt->view_model,
        x,
        y,
        out_receipt);
}

int theron_v1_boot_startup_layout_build_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_StartupLayoutElement *elements,
    int max_elements)
{
    if (!receipt || !receipt->view_model_valid) {
        if (elements && max_elements > 0) {
            memset(elements, 0, (size_t)max_elements * sizeof(elements[0]));
        }
        return 0;
    }
    return theron_v1_boot_startup_layout_build_from_view_model(
        &receipt->view_model,
        elements,
        max_elements);
}

int theron_v1_boot_startup_render_rows_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows)
{
    if (!receipt || !receipt->view_model_valid) {
        if (rows && max_rows > 0) {
            memset(rows,
                   0,
                   (size_t)max_rows * THERON_STARTUP_RENDER_ROW_CAPACITY);
        }
        return 0;
    }
    return theron_v1_boot_startup_render_rows_from_view_model(
        &receipt->view_model,
        rows,
        max_rows);
}

int theron_v1_boot_startup_host_view_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_V1_BootStartupHostViewReceipt *out_receipt)
{
    if (out_receipt) {
        theron_v1_boot_startup_host_view_receipt_init(out_receipt);
    }
    if (!receipt || !receipt->host_view_valid || !out_receipt) {
        return 0;
    }
    *out_receipt = receipt->host_view;
    return 1;
}

void theron_v1_boot_startup_host_render_receipt_init(
    Theron_V1_BootStartupHostRenderReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->status_scope = "STARTUP";
    receipt->status = "NO HOST RENDER";
}

int theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    Theron_V1_BootStartupHostRenderReceipt *out_receipt)
{
    int layout_count;
    int row_count;

    if (out_receipt) {
        theron_v1_boot_startup_host_render_receipt_init(out_receipt);
    }
    if (!receipt || !receipt->view_model_valid || !out_receipt) {
        return 0;
    }

    out_receipt->host_consumes_full_start_receipt = 1;
    out_receipt->full_start_valid = 1;
    layout_count = receipt->view_model.layout_count;
    if (layout_count > THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP) {
        layout_count = THERON_V1_BOOT_STARTUP_VIEW_MODEL_LAYOUT_CAP;
    }
    row_count = receipt->view_model.row_count;
    if (row_count > THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP) {
        row_count = THERON_V1_BOOT_STARTUP_VIEW_MODEL_ROW_CAP;
    }
    out_receipt->layout_count = layout_count;
    out_receipt->row_count = row_count;
    if (layout_count > 0) {
        memcpy(out_receipt->layout,
               receipt->view_model.layout,
               (size_t)layout_count * sizeof(out_receipt->layout[0]));
    }
    if (row_count > 0) {
        memcpy(out_receipt->rows,
               receipt->view_model.rows,
               (size_t)row_count * THERON_STARTUP_RENDER_ROW_CAPACITY);
    }
    if (receipt->view_model.render_plan_valid) {
        out_receipt->render_plan = receipt->view_model.render_plan;
        out_receipt->render_plan_valid = 1;
    }
    if (receipt->host_view.render_route_valid) {
        out_receipt->render_route = receipt->host_view.render_route;
        out_receipt->render_route_valid = 1;
    }
    out_receipt->graphics_route_valid = receipt->graphics_route_valid;
    out_receipt->graphics_executor_consumed =
        receipt->graphics_route_valid &&
        (receipt->full_start_graphics_executed ||
         receipt->full_start_graphics_blocked)
            ? 1
            : 0;
    out_receipt->full_start_graphics_ready =
        receipt->full_start_graphics_ready;
    out_receipt->full_start_graphics_executed =
        receipt->full_start_graphics_executed;
    out_receipt->full_start_graphics_blocked =
        receipt->full_start_graphics_blocked;
    out_receipt->required_bitmap_route_mask =
        receipt->required_bitmap_route_mask;
    out_receipt->required_bitmap_route_count =
        receipt->required_bitmap_route_count;
    out_receipt->required_bitmap_routes_ready =
        receipt->required_bitmap_routes_ready;
    out_receipt->bitmap_route_mask = receipt->bitmap_route_mask;
    out_receipt->bitmap_route_count = receipt->bitmap_route_count;
    out_receipt->raw_bitmap_route_mask = receipt->raw_bitmap_route_mask;
    out_receipt->raw_bitmap_route_count = receipt->raw_bitmap_route_count;
    out_receipt->raw_bitmap_atlas_tile_count =
        receipt->raw_bitmap_atlas_tile_count;
    out_receipt->iso_bitmap_route_mask = receipt->iso_bitmap_route_mask;
    out_receipt->iso_bitmap_route_count = receipt->iso_bitmap_route_count;
    out_receipt->iso_bitmap_atlas_tile_count =
        receipt->iso_bitmap_atlas_tile_count;
    out_receipt->bitmap_package_route_ready =
        receipt->bitmap_package_route_ready;
    out_receipt->title_bitmap_route_ready =
        receipt->title_bitmap_route_ready;
    out_receipt->stage_bitmap_route_ready =
        receipt->stage_bitmap_route_ready;
    out_receipt->soul_room_bitmap_route_ready =
        receipt->soul_room_bitmap_route_ready;
    out_receipt->forcefield_bitmap_route_ready =
        receipt->forcefield_bitmap_route_ready;
    out_receipt->track02_real_media_ready =
        receipt->track02_real_media_ready;
    out_receipt->real_bitmap_startup_graphics_ready =
        receipt->real_bitmap_startup_graphics_ready;
    out_receipt->track02_atlas_startup_graphics_ready =
        receipt->track02_atlas_startup_graphics_ready;
    out_receipt->track02_atlas_startup_graphics_executed =
        receipt->track02_atlas_startup_graphics_executed;
    out_receipt->track02_atlas_graphics_route_mask =
        receipt->track02_atlas_graphics_route_mask;
    out_receipt->track02_atlas_graphics_route_count =
        receipt->track02_atlas_graphics_route_count;
    out_receipt->track02_atlas_graphics_pixel_count =
        receipt->track02_atlas_graphics_pixel_count;
    out_receipt->track02_atlas_graphics_checksum =
        receipt->track02_atlas_graphics_checksum;
    out_receipt->track02_startup_graphics_executed =
        receipt->track02_startup_graphics_executed;
    out_receipt->track02_startup_graphic_receipt_valid =
        receipt->track02_startup_graphic_receipt_valid;
    out_receipt->track02_startup_graphic_receipt =
        receipt->track02_startup_graphic_receipt;
    out_receipt->no_fallback_startup_graphics_proof =
        receipt->no_fallback_startup_graphics_proof;
    out_receipt->no_fallback_visuals_enforced =
        receipt->no_fallback_visuals_enforced;
    out_receipt->fallback_visuals_allowed =
        receipt->fallback_visuals_allowed;
    out_receipt->raw_prompt_roster_required =
        receipt->raw_prompt_roster_required;
    out_receipt->raw_session_rebuild_required =
        receipt->raw_session_rebuild_required;
    out_receipt->raw_graphics_plan_consumer_required =
        receipt->raw_graphics_plan_consumer_required;
    out_receipt->status_scope = receipt->status_scope;
    out_receipt->status = receipt->status;
    return out_receipt->render_plan_valid ||
           out_receipt->layout_count > 0 ||
           out_receipt->row_count > 0 ||
           out_receipt->render_route_valid;
}

int theron_v1_boot_startup_host_render_receipt_from_snapshot_with_media_receipt_and_executor(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_boot_startup_host_render_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            executor,
            &full_start)) {
        return 0;
    }
    return theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
        &full_start,
        out_receipt);
}

int theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    return theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt_and_executor(
        out_receipt,
        startup_media_receipt,
        NULL,
        startup_phase,
        selected_dungeon,
        boot_profile,
        world,
        assets,
        startup_cursor,
        continue_focus,
        resume_claim,
        tqsv_slot,
        srm_slot,
        srm_import_status,
        srm_root,
        selected_mirrors_mask,
        companion_count,
        selected_mirror_order,
        selected_mirror_order_count);
}

int theron_v1_boot_startup_host_render_receipt_from_runtime_state_with_media_receipt_and_executor(
    Theron_V1_BootStartupHostRenderReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_boot_startup_host_render_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
            &full_start,
            startup_media_receipt,
            executor,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        return 0;
    }
    return theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
        &full_start,
        out_receipt);
}

void theron_v1_boot_startup_menu_runtime_handoff_receipt_init(
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    theron_v1_startup_action_host_receipt_init(&receipt->input_route);
    theron_v1_startup_action_host_receipt_init(&receipt->pointer_route);
    receipt->status_scope = "STARTUP";
    receipt->status = "NO MENU RUNTIME HANDOFF";
}

static void theron_v1_boot_startup_menu_runtime_handoff_copy_media_spans(
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *media_receipt)
{
    if (!out_receipt || !media_receipt) {
        return;
    }
    out_receipt->track02_media_route_mask =
        media_receipt->startup_bitmap_atlas_route_mask;
    out_receipt->track02_media_checksum =
        media_receipt->startup_bitmap_atlas_checksum;
    out_receipt->track02_media_title_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        (uint64_t)media_receipt->startup_bitmap_forcefield_last_user_data_offset;
}

int theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int input_code,
    int pointer_x,
    int pointer_y,
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *out_receipt)
{
    int host_render_ok;

    if (out_receipt) {
        theron_v1_boot_startup_menu_runtime_handoff_receipt_init(
            out_receipt);
    }
    if (!receipt || !out_receipt) {
        return 0;
    }

    out_receipt->host_consumes_full_start_receipt = 1;
    out_receipt->full_start_valid = receipt->view_model_valid ? 1 : 0;
    out_receipt->full_start = *receipt;
    host_render_ok =
        theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
            receipt,
            &out_receipt->host_render);
    out_receipt->host_render_valid = host_render_ok ? 1 : 0;

    out_receipt->track02_media_consumed =
        receipt->host_view_valid &&
        receipt->host_view.track02_media_consumed ? 1 : 0;
    if (out_receipt->track02_media_consumed &&
        receipt->view_model_valid &&
        receipt->view_model.startup_media_state_valid) {
        theron_v1_boot_startup_menu_runtime_handoff_copy_media_spans(
            out_receipt,
            &receipt->view_model.startup_media_state_receipt);
    }
    out_receipt->startup_menu_render_allowed =
        receipt->host_view_valid &&
        receipt->host_view.render_route_valid &&
        receipt->host_view.render_route.startup_menu_render_allowed ? 1 : 0;
    out_receipt->title_menu_ready = receipt->title_menu_ready ? 1 : 0;
    out_receipt->stage_menu_ready = receipt->stage_menu_ready ? 1 : 0;
    out_receipt->soul_room_menu_ready =
        receipt->soul_room_menu_ready ? 1 : 0;
    out_receipt->runtime_handoff_ready =
        receipt->runtime_graphics_handoff ||
        receipt->forcefield_runtime_handoff_ready ||
        receipt->runtime_readiness_ready ? 1 : 0;
    out_receipt->track02_runtime_handoff_ready =
        receipt->track02_runtime_graphics_handoff ? 1 : 0;
    out_receipt->save_resume_runtime_handoff_ready =
        !out_receipt->track02_runtime_handoff_ready &&
        (receipt->save_resume_runtime_graphics_handoff ||
         receipt->save_resume_runtime_handoff_ready) ? 1 : 0;
    out_receipt->real_graphics_handoff_ready =
        receipt->real_bitmap_startup_graphics_ready ||
        receipt->runtime_graphics_handoff ? 1 : 0;
    out_receipt->bitmap_package_route_ready =
        receipt->bitmap_package_route_ready ? 1 : 0;
    out_receipt->raw_bitmap_route_mask = receipt->raw_bitmap_route_mask;
    out_receipt->iso_bitmap_route_mask = receipt->iso_bitmap_route_mask;
    out_receipt->startup_graphics_executed =
        receipt->full_start_graphics_executed ? 1 : 0;
    out_receipt->startup_graphics_blocked =
        receipt->full_start_graphics_blocked ? 1 : 0;
    out_receipt->no_fallback_visuals_enforced =
        receipt->no_fallback_visuals_enforced ? 1 : 0;
    out_receipt->fallback_visuals_allowed =
        receipt->fallback_visuals_allowed ? 1 : 0;
    out_receipt->fallback_startup_graphics_executed =
        receipt->fallback_startup_graphics_executed ? 1 : 0;
    out_receipt->host_may_draw_fallback_visuals =
        out_receipt->fallback_visuals_allowed &&
        !out_receipt->no_fallback_visuals_enforced &&
        !out_receipt->track02_media_consumed &&
        !out_receipt->real_graphics_handoff_ready ? 1 : 0;
    out_receipt->host_must_not_draw_fallback_visuals =
        out_receipt->no_fallback_visuals_enforced ||
        !out_receipt->fallback_visuals_allowed ? 1 : 0;
    out_receipt->raw_prompt_roster_required =
        receipt->raw_prompt_roster_required ? 1 : 0;
    out_receipt->raw_session_rebuild_required =
        receipt->raw_session_rebuild_required ? 1 : 0;
    out_receipt->raw_graphics_plan_consumer_required =
        receipt->raw_graphics_plan_consumer_required ? 1 : 0;

    if (input_code >= 0) {
        out_receipt->input_route_requested = 1;
        out_receipt->input_route_valid =
            theron_v1_boot_startup_execute_input_from_full_start_receipt(
                receipt,
                theron_v1_startup_input_from_firestaff_menu_code(input_code),
                &out_receipt->input_route)
                ? 1
                : 0;
    }
    if (pointer_x >= 0 && pointer_y >= 0) {
        out_receipt->pointer_route_requested = 1;
        out_receipt->pointer_route_valid =
            theron_v1_boot_startup_execute_pointer_from_full_start_receipt(
                receipt,
                pointer_x,
                pointer_y,
                &out_receipt->pointer_route)
                ? 1
                : 0;
    }

    if (out_receipt->host_must_not_draw_fallback_visuals &&
        out_receipt->runtime_handoff_ready) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "THERON RUNTIME HANDOFF NO FALLBACK";
    } else if (out_receipt->startup_menu_render_allowed) {
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = "THERON STARTUP MENU HANDOFF";
    } else if (receipt->status) {
        out_receipt->status_scope = receipt->status_scope;
        out_receipt->status = receipt->status;
    }

    return out_receipt->full_start_valid && out_receipt->host_render_valid;
}

int theron_v1_boot_startup_menu_runtime_handoff_from_runtime_state_with_media_receipt(
    Theron_V1_BootStartupMenuRuntimeHandoffReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input_code,
    int pointer_x,
    int pointer_y)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_boot_startup_menu_runtime_handoff_receipt_init(
            out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_state_with_media_receipt(
            &full_start,
            startup_media_receipt,
            executor,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        return 0;
    }
    return theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
        &full_start,
        input_code,
        pointer_x,
        pointer_y,
        out_receipt);
}

void theron_v1_boot_startup_ui_caller_receipt_init(
    Theron_V1_BootStartupUiCallerReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    theron_v1_boot_startup_host_render_receipt_init(&receipt->host_render);
    theron_v1_boot_startup_menu_runtime_handoff_receipt_init(
        &receipt->menu_runtime_handoff);
    receipt->status_scope = "STARTUP";
    receipt->status = "NO UI CALLER HANDOFF";
}

int theron_v1_boot_startup_host_render_plan_fallback_allowed(
    const Theron_V1_BootStartupHostRenderReceipt *receipt)
{
    return receipt && !receipt->track02_startup_graphics_executed;
}

int theron_v1_boot_startup_ui_caller_from_full_start_receipt(
    const Theron_V1_BootStartupFullStartReceipt *receipt,
    int input_code,
    int pointer_x,
    int pointer_y,
    Theron_V1_BootStartupUiCallerReceipt *out_receipt)
{
    int level_bit;

    if (out_receipt) {
        theron_v1_boot_startup_ui_caller_receipt_init(out_receipt);
    }
    if (!receipt || !out_receipt) {
        return 0;
    }

    out_receipt->host_render_valid =
        theron_v1_boot_startup_host_render_receipt_from_full_start_receipt(
            receipt,
            &out_receipt->host_render)
            ? 1
            : 0;
    out_receipt->menu_runtime_handoff_valid =
        theron_v1_boot_startup_menu_runtime_handoff_from_full_start_receipt(
            receipt,
            input_code,
            pointer_x,
            pointer_y,
            &out_receipt->menu_runtime_handoff)
            ? 1
            : 0;

    out_receipt->track02_media_consumed =
        out_receipt->menu_runtime_handoff.track02_media_consumed ? 1 : 0;
    out_receipt->track02_media_route_mask =
        out_receipt->menu_runtime_handoff.track02_media_route_mask;
    out_receipt->track02_media_checksum =
        out_receipt->menu_runtime_handoff.track02_media_checksum;
    out_receipt->track02_media_title_first_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_title_first_raw_offset;
    out_receipt->track02_media_title_last_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_title_last_raw_offset;
    out_receipt->track02_media_title_first_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_title_first_user_data_offset;
    out_receipt->track02_media_title_last_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_title_last_user_data_offset;
    out_receipt->track02_media_stage_first_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_stage_first_raw_offset;
    out_receipt->track02_media_stage_last_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_stage_last_raw_offset;
    out_receipt->track02_media_stage_first_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_stage_first_user_data_offset;
    out_receipt->track02_media_stage_last_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_stage_last_user_data_offset;
    out_receipt->track02_media_soul_room_first_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_soul_room_first_raw_offset;
    out_receipt->track02_media_soul_room_last_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_soul_room_last_raw_offset;
    out_receipt->track02_media_soul_room_first_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_soul_room_first_user_data_offset;
    out_receipt->track02_media_soul_room_last_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_soul_room_last_user_data_offset;
    out_receipt->track02_media_forcefield_first_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_forcefield_first_raw_offset;
    out_receipt->track02_media_forcefield_last_raw_offset =
        out_receipt->menu_runtime_handoff.track02_media_forcefield_last_raw_offset;
    out_receipt->track02_media_forcefield_first_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_forcefield_first_user_data_offset;
    out_receipt->track02_media_forcefield_last_user_data_offset =
        out_receipt->menu_runtime_handoff.track02_media_forcefield_last_user_data_offset;
    out_receipt->title_prompt_ready =
        receipt->view_model_valid &&
                receipt->view_model.startup_media_state_valid &&
                receipt->view_model.startup_media_state_receipt
                        .startup_text_prompt_status ==
                    THERON_TRACK02_SIGNAL_OK &&
                receipt->view_model.startup_media_state_receipt
                        .startup_text_prompt_count > 0
            ? 1
            : 0;
    out_receipt->roster_ready =
        receipt->view_model_valid &&
                receipt->view_model.startup_media_state_valid &&
                receipt->view_model.startup_media_state_receipt
                        .startup_roster_name_status ==
                    THERON_TRACK02_SIGNAL_OK &&
                receipt->view_model.startup_media_state_receipt
                        .startup_roster_name_count > 0
            ? 1
            : 0;
    out_receipt->title_menu_ready = receipt->title_menu_ready ? 1 : 0;
    out_receipt->stage_menu_ready = receipt->stage_menu_ready ? 1 : 0;
    out_receipt->soul_room_menu_ready =
        receipt->soul_room_menu_ready ? 1 : 0;
    out_receipt->forcefield_menu_ready =
        receipt->forcefield_menu_ready ? 1 : 0;
    out_receipt->runtime_handoff_ready =
        out_receipt->menu_runtime_handoff.runtime_handoff_ready ? 1 : 0;
    out_receipt->track02_runtime_handoff_ready =
        out_receipt->menu_runtime_handoff.track02_runtime_handoff_ready ? 1 : 0;
    out_receipt->save_resume_runtime_handoff_ready =
        out_receipt->menu_runtime_handoff.save_resume_runtime_handoff_ready
            ? 1
            : 0;
    out_receipt->real_graphics_handoff_ready =
        out_receipt->menu_runtime_handoff.real_graphics_handoff_ready ? 1 : 0;
    out_receipt->real_bitmap_decode_ready =
        receipt->track02_real_media_ready &&
                receipt->real_bitmap_startup_graphics_ready &&
                receipt->required_bitmap_routes_ready
            ? 1
            : 0;
    out_receipt->required_bitmap_routes_ready =
        receipt->required_bitmap_routes_ready ? 1 : 0;
    out_receipt->required_bitmap_route_mask =
        (int)receipt->required_bitmap_route_mask;
    out_receipt->required_bitmap_route_count =
        receipt->required_bitmap_route_count;
    out_receipt->bitmap_route_mask = (int)receipt->bitmap_route_mask;
    out_receipt->bitmap_route_count = receipt->bitmap_route_count;
    out_receipt->bitmap_package_route_ready =
        receipt->bitmap_package_route_ready ? 1 : 0;
    out_receipt->raw_bitmap_route_mask = (int)receipt->raw_bitmap_route_mask;
    out_receipt->raw_bitmap_route_count = receipt->raw_bitmap_route_count;
    out_receipt->iso_bitmap_route_mask = (int)receipt->iso_bitmap_route_mask;
    out_receipt->iso_bitmap_route_count = receipt->iso_bitmap_route_count;
    out_receipt->title_bitmap_route_ready =
        receipt->title_bitmap_route_ready ? 1 : 0;
    out_receipt->stage_bitmap_route_ready =
        receipt->stage_bitmap_route_ready ? 1 : 0;
    out_receipt->soul_room_bitmap_route_ready =
        receipt->soul_room_bitmap_route_ready ? 1 : 0;
    out_receipt->forcefield_bitmap_route_ready =
        receipt->forcefield_bitmap_route_ready ? 1 : 0;
    out_receipt->runtime_level = receipt->runtime_level;
    out_receipt->runtime_level_source = receipt->runtime_level_source;
    out_receipt->runtime_track02_semantic_handoff =
        receipt->runtime_track02_semantic_handoff ? 1 : 0;
    out_receipt->semantic_first_level_ready =
        out_receipt->runtime_track02_semantic_handoff &&
                receipt->runtime_level == 0
            ? 1
            : 0;
    out_receipt->semantic_nonzero_level_ready =
        out_receipt->runtime_track02_semantic_handoff &&
                receipt->runtime_level > 0
            ? 1
            : 0;
    if (out_receipt->runtime_track02_semantic_handoff &&
        receipt->runtime_level >= 0 &&
        receipt->runtime_level < 31) {
        level_bit = 1 << receipt->runtime_level;
        out_receipt->semantic_level_coverage_mask = level_bit;
    }
    out_receipt->all_dungeon_real_data_capture_ready =
        receipt->all_dungeon_real_data_capture_ready ? 1 : 0;
    out_receipt->all_dungeon_capture_count =
        receipt->all_dungeon_capture_count;
    out_receipt->all_dungeon_capture_mask =
        receipt->all_dungeon_capture_mask;
    out_receipt->exact_level_semantics_ready =
        receipt->exact_level_semantics_ready ? 1 : 0;
    out_receipt->exact_object_semantics_ready =
        receipt->exact_object_semantics_ready ? 1 : 0;
    out_receipt->no_fallback_semantic_role_mask =
        receipt->no_fallback_semantic_role_mask;
    out_receipt->track02_state_predicates_consumed =
        receipt->track02_state_predicates_consumed ? 1 : 0;
    out_receipt->track02_bitmap_routes_complete =
        receipt->track02_bitmap_routes_complete ? 1 : 0;
    out_receipt->track02_no_fallback_runtime_route_ready =
        receipt->track02_no_fallback_runtime_route_ready ? 1 : 0;
    out_receipt->object_table_no_fallback_ready =
        receipt->object_table_no_fallback_ready ? 1 : 0;
    out_receipt->object_table_blocked_anchor_mask =
        receipt->object_table_blocked_anchor_mask;
    out_receipt->object_table_blocked_anchor_count =
        receipt->object_table_blocked_anchor_count;
    out_receipt->nonstartup_level_no_fallback_ready =
        receipt->nonstartup_level_no_fallback_ready ? 1 : 0;
    out_receipt->nonstartup_level_blocked_anchor_mask =
        receipt->nonstartup_level_blocked_anchor_mask;
    out_receipt->nonstartup_level_blocked_anchor_count =
        receipt->nonstartup_level_blocked_anchor_count;
    out_receipt->startup_level_blocked_anchor_mask =
        receipt->startup_level_blocked_anchor_mask;
    out_receipt->startup_level_blocked_anchor_count =
        receipt->startup_level_blocked_anchor_count;
    out_receipt->object_table_route_hash = receipt->object_table_route_hash;
    out_receipt->level_route_hash = receipt->level_route_hash;
    out_receipt->complete_runtime_support_ready =
        out_receipt->all_dungeon_real_data_capture_ready &&
        out_receipt->all_dungeon_capture_count == THERON_DUNGEON_COUNT &&
        out_receipt->exact_level_semantics_ready &&
        out_receipt->exact_object_semantics_ready &&
        out_receipt->track02_state_predicates_consumed &&
        out_receipt->track02_bitmap_routes_complete &&
        out_receipt->track02_no_fallback_runtime_route_ready &&
        out_receipt->object_table_no_fallback_ready &&
        out_receipt->nonstartup_level_no_fallback_ready
            ? 1
            : 0;
    out_receipt->no_fallback_visuals_enforced =
        receipt->no_fallback_visuals_enforced ? 1 : 0;
    out_receipt->fallback_visuals_allowed =
        receipt->fallback_visuals_allowed ? 1 : 0;
    out_receipt->fallback_startup_graphics_executed =
        receipt->fallback_startup_graphics_executed ? 1 : 0;
    out_receipt->host_must_not_draw_fallback_visuals =
        out_receipt->menu_runtime_handoff.host_must_not_draw_fallback_visuals
            ? 1
            : 0;
    out_receipt->raw_prompt_roster_required =
        receipt->raw_prompt_roster_required ? 1 : 0;
    out_receipt->raw_session_rebuild_required =
        receipt->raw_session_rebuild_required ? 1 : 0;
    out_receipt->raw_graphics_plan_consumer_required =
        receipt->raw_graphics_plan_consumer_required ? 1 : 0;
    out_receipt->ui_callers_ready =
        out_receipt->host_render_valid &&
        out_receipt->menu_runtime_handoff_valid &&
        out_receipt->track02_media_consumed &&
        out_receipt->title_prompt_ready &&
        out_receipt->roster_ready &&
        out_receipt->real_bitmap_decode_ready &&
        out_receipt->track02_state_predicates_consumed &&
        out_receipt->track02_bitmap_routes_complete &&
        out_receipt->track02_no_fallback_runtime_route_ready &&
        out_receipt->host_must_not_draw_fallback_visuals &&
        !out_receipt->fallback_visuals_allowed &&
        !out_receipt->fallback_startup_graphics_executed &&
        !out_receipt->raw_prompt_roster_required &&
        !out_receipt->raw_session_rebuild_required &&
        !out_receipt->raw_graphics_plan_consumer_required
            ? 1
            : 0;
    out_receipt->status_scope = "STARTUP";
    out_receipt->status =
        out_receipt->ui_callers_ready
            ? "THERON UI CALLERS TRACK02 READY"
            : "THERON UI CALLERS INCOMPLETE";
    return out_receipt->ui_callers_ready;
}

int theron_v1_boot_startup_ui_caller_from_snapshot_with_media_receipt(
    Theron_V1_BootStartupUiCallerReceipt *out_receipt,
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int input_code,
    int pointer_x,
    int pointer_y)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_boot_startup_ui_caller_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_snapshot_with_media_receipt(
            snapshot,
            startup_media_receipt,
            executor,
            &full_start)) {
        return 0;
    }
    return theron_v1_boot_startup_ui_caller_from_full_start_receipt(
        &full_start,
        input_code,
        pointer_x,
        pointer_y,
        out_receipt);
}

int theron_v1_boot_startup_ui_caller_from_runtime_route_with_media_receipt(
    Theron_V1_BootStartupUiCallerReceipt *out_receipt,
    const Theron_StartupMediaStateReceipt *startup_media_receipt,
    const Theron_StartupGraphicExecutor *executor,
    int startup_phase,
    int selected_dungeon,
    const void *boot_profile,
    const Theron_V1_World *world,
    const void *assets,
    int startup_cursor,
    int continue_focus,
    int resume_claim,
    int tqsv_slot,
    int srm_slot,
    int srm_import_status,
    const char *srm_root,
    int runtime_level_source,
    int runtime_track02_semantic_handoff,
    int runtime_fallback_visuals_blocked,
    int runtime_structured_route,
    int runtime_receipt_text_route,
    int selected_mirrors_mask,
    int companion_count,
    const int *selected_mirror_order,
    int selected_mirror_order_count,
    int input_code,
    int pointer_x,
    int pointer_y)
{
    Theron_V1_BootStartupFullStartReceipt full_start;

    if (out_receipt) {
        theron_v1_boot_startup_ui_caller_receipt_init(out_receipt);
    }
    if (!theron_v1_boot_startup_full_start_receipt_from_runtime_route_with_media_receipt(
            &full_start,
            startup_media_receipt,
            executor,
            startup_phase,
            selected_dungeon,
            boot_profile,
            world,
            assets,
            startup_cursor,
            continue_focus,
            resume_claim,
            tqsv_slot,
            srm_slot,
            srm_import_status,
            srm_root,
            runtime_level_source,
            runtime_track02_semantic_handoff,
            runtime_fallback_visuals_blocked,
            runtime_structured_route,
            runtime_receipt_text_route,
            selected_mirrors_mask,
            companion_count,
            selected_mirror_order,
            selected_mirror_order_count)) {
        return 0;
    }
    return theron_v1_boot_startup_ui_caller_from_full_start_receipt(
        &full_start,
        input_code,
        pointer_x,
        pointer_y,
        out_receipt);
}

int theron_v1_boot_startup_presentation_receipt_from_runtime_state(
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready,
    int startup_phase)
{
    return theron_v1_startup_presentation_receipt(
        (Theron_StartupPhase)startup_phase,
        out_phase,
        out_phase_size,
        out_startup_active,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready);
}

int theron_v1_boot_startup_presentation_receipt_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    if (!snapshot) {
        return 0;
    }
    return theron_v1_boot_startup_presentation_receipt_from_runtime_state(
        out_phase,
        out_phase_size,
        out_startup_active,
        out_animation,
        out_animation_size,
        out_animation_active,
        out_title_frame,
        out_title_frame_max,
        out_title_ready,
        snapshot->startup_phase);
}

static void theron_v1_boot_runtime_world_receipt(
    const Theron_V1_World *world,
    int *out_x,
    int *out_y,
    int *out_dir,
    int *out_tick)
{
    if (!world) {
        return;
    }
    if (out_x) *out_x = world->party.leader_x;
    if (out_y) *out_y = world->party.leader_y;
    if (out_dir) *out_dir = world->party.leader_dir;
    if (out_tick) *out_tick = (int)world->world_tick;
}

int theron_v1_boot_runtime_tick_world(Theron_V1_World *world,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick)
{
    if (!world) {
        return 0;
    }
    theron_v1_world_tick(world);
    theron_v1_boot_runtime_world_receipt(world,
                                         out_x,
                                         out_y,
                                         out_dir,
                                         out_tick);
    return 1;
}

int theron_v1_boot_runtime_turn_party(Theron_V1_World *world,
                                      int turn,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick)
{
    if (!world) {
        return 0;
    }
    (void)theron_v1_turn_party(world, turn);
    theron_v1_boot_runtime_world_receipt(world,
                                         out_x,
                                         out_y,
                                         out_dir,
                                         out_tick);
    return 1;
}

int theron_v1_boot_runtime_move_party(Theron_V1_World *world,
                                      int direction,
                                      int restore_direction,
                                      int *out_x,
                                      int *out_y,
                                      int *out_dir,
                                      int *out_tick)
{
    int result;
    if (!world) {
        return THERON_MOVE_BLOCKED;
    }
    result = theron_v1_move_party(world, direction);
    if (result != THERON_MOVE_BLOCKED && restore_direction >= 0) {
        world->party.leader_dir = (int8_t)(restore_direction & 3);
    }
    theron_v1_boot_runtime_world_receipt(world,
                                         out_x,
                                         out_y,
                                         out_dir,
                                         out_tick);
    return result;
}

void theron_v1_boot_runtime_input_receipt_init(
    Theron_V1_BootRuntimeInputReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->result = THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED;
    receipt->party_x = -1;
    receipt->party_y = -1;
    receipt->party_dir = -1;
    receipt->tick_count = -1;
    receipt->status_scope = "THERON";
    receipt->status = "NO-OP";
}

int theron_v1_boot_runtime_handle_m12_input(
    Theron_V1_World *world,
    const void *boot_profile,
    int m12_input,
    Theron_V1_BootRuntimeInputReceipt *out_receipt)
{
    int move_result;
    int dir;
    int old_dir;

    if (!world || !out_receipt) {
        return 0;
    }
    theron_v1_boot_runtime_input_receipt_init(out_receipt);

    /* v2.8.x: arrow Left/Right now mean strafe (matches original DM1 PC 3.4
     * convention).  TURN_LEFT/RIGHT comes from Home / End / Q / E / KP_4 /
     * KP_6.  Theron's track-02 world has no strafe route, so
     * STRAFE_LEFT/RIGHT and the legacy LEFT/RIGHT tokens are ignored. */
    if (m12_input == M12_MENU_INPUT_TURN_LEFT) {
        (void)theron_v1_boot_runtime_turn_party(
            world, -1,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        out_receipt->handled = 1;
        out_receipt->turned = 1;
        out_receipt->result = THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW;
        out_receipt->status_scope = "TURN";
        out_receipt->status = "LEFT";
        return 1;
    }
    if (m12_input == M12_MENU_INPUT_TURN_RIGHT) {
        (void)theron_v1_boot_runtime_turn_party(
            world, 1,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        out_receipt->handled = 1;
        out_receipt->turned = 1;
        out_receipt->result = THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW;
        out_receipt->status_scope = "TURN";
        out_receipt->status = "RIGHT";
        return 1;
    }
    if (m12_input == M12_MENU_INPUT_LEFT ||
        m12_input == M12_MENU_INPUT_RIGHT ||
        m12_input == M12_MENU_INPUT_STRAFE_LEFT ||
        m12_input == M12_MENU_INPUT_STRAFE_RIGHT) {
        /* Theron has no strafe route: keep the current pose in the receipt
         * and report ignored. */
        theron_v1_boot_runtime_world_receipt(
            world,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        out_receipt->status_scope = "NO-OP";
        out_receipt->status = "THERON HAS NO STRAFE";
        return 1;
    }
    if (m12_input == M12_MENU_INPUT_UP) {
        dir = world->party.leader_dir & 3;
        move_result = theron_v1_boot_runtime_move_party(
            world, dir, -1,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        if (move_result == THERON_MOVE_EXIT) {
            (void)theron_v1_boot_startup_return_to_stage_select_after_exit_profile_host_receipt(
                &out_receipt->exit_receipt,
                boot_profile,
                world);
            out_receipt->handled = 1;
            out_receipt->exited = 1;
            out_receipt->result =
                THERON_V1_BOOT_RUNTIME_INPUT_RESULT_EXIT_DUNGEON;
            out_receipt->status_scope = "MOVE";
            out_receipt->status = "EXIT DUNGEON";
            return 1;
        }
        out_receipt->blocked = (move_result == THERON_MOVE_BLOCKED) ? 1 : 0;
        out_receipt->moved = out_receipt->blocked ? 0 : 1;
        out_receipt->result = out_receipt->moved
            ? THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW
            : THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED;
        out_receipt->status_scope = "MOVE";
        out_receipt->status = out_receipt->moved
            ? "THERON ADVANCED"
            : "BLOCKED";
        return 1;
    }
    if (m12_input == M12_MENU_INPUT_DOWN) {
        old_dir = world->party.leader_dir & 3;
        dir = (old_dir + 2) & 3;
        move_result = theron_v1_boot_runtime_move_party(
            world, dir, old_dir,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        if (move_result == THERON_MOVE_EXIT) {
            (void)theron_v1_boot_startup_return_to_stage_select_after_exit_profile_host_receipt(
                &out_receipt->exit_receipt,
                boot_profile,
                world);
            out_receipt->handled = 1;
            out_receipt->exited = 1;
            out_receipt->result =
                THERON_V1_BOOT_RUNTIME_INPUT_RESULT_EXIT_DUNGEON;
            out_receipt->status_scope = "MOVE";
            out_receipt->status = "EXIT DUNGEON";
            return 1;
        }
        out_receipt->blocked = (move_result == THERON_MOVE_BLOCKED) ? 1 : 0;
        out_receipt->moved = out_receipt->blocked ? 0 : 1;
        out_receipt->result = out_receipt->moved
            ? THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW
            : THERON_V1_BOOT_RUNTIME_INPUT_RESULT_IGNORED;
        out_receipt->status_scope = "MOVE";
        out_receipt->status = out_receipt->moved
            ? "THERON STEPPED BACK"
            : "BLOCKED";
        return 1;
    }
    if (m12_input == M12_MENU_INPUT_ACCEPT ||
        m12_input == M12_MENU_INPUT_ACTION) {
        (void)theron_v1_boot_runtime_tick_world(
            world,
            &out_receipt->party_x,
            &out_receipt->party_y,
            &out_receipt->party_dir,
            &out_receipt->tick_count);
        out_receipt->handled = 1;
        out_receipt->waited = 1;
        out_receipt->result = THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW;
        out_receipt->status_scope = "WAIT";
        out_receipt->status = "THERON TICK";
        return 1;
    }

    /* Unknown token: keep current pose, report ignored. */
    theron_v1_boot_runtime_world_receipt(
        world,
        &out_receipt->party_x,
        &out_receipt->party_y,
        &out_receipt->party_dir,
        &out_receipt->tick_count);
    out_receipt->status_scope = "THERON";
    out_receipt->status = "UNKNOWN INPUT";
    return 1;
}

int theron_v1_boot_runtime_handle_idle_tick(
    Theron_V1_World *world,
    Theron_V1_BootRuntimeInputReceipt *out_receipt)
{
    if (!world || !out_receipt) {
        return 0;
    }
    theron_v1_boot_runtime_input_receipt_init(out_receipt);
    (void)theron_v1_boot_runtime_tick_world(
        world,
        &out_receipt->party_x,
        &out_receipt->party_y,
        &out_receipt->party_dir,
        &out_receipt->tick_count);
    out_receipt->handled = 1;
    out_receipt->waited = 1;
    out_receipt->result = THERON_V1_BOOT_RUNTIME_INPUT_RESULT_REDRAW;
    out_receipt->status_scope = "WAIT";
    out_receipt->status = "THERON TICK";
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * Startup host-receipt apply facade
 *
 * M11 passes a small callback table; the facade owns the host-receipt
 * semantics (status, inspect readout, log lines, input result) without
 * including any M11 headers.
 * ══════════════════════════════════════════════════════════════════════ */

int theron_v1_boot_apply_startup_host_receipt(
    const Theron_StartupHostReceipt *receipt,
    const char *runtime_receipt,
    const Theron_V1_BootHostReceiptCallbacks *callbacks,
    Theron_V1_BootHostReceiptResult *out_result)
{
    if (out_result) {
        *out_result = THERON_V1_BOOT_HOST_RECEIPT_RESULT_IGNORED;
    }
    if (!receipt || !callbacks) {
        return 0;
    }

    if (callbacks->set_status &&
        (receipt->status_scope || receipt->status)) {
        callbacks->set_status(
            callbacks->userdata,
            receipt->status_scope ? receipt->status_scope : "STARTUP",
            receipt->status ? receipt->status : "");
    }

    if (callbacks->set_inspect && receipt->inspect_scope) {
        callbacks->set_inspect(
            callbacks->userdata,
            receipt->inspect_scope,
            receipt->inspect_detail[0] ? receipt->inspect_detail : "");
    }

    if (callbacks->log_event && receipt->log_first_line) {
        /* M11_COLOR_YELLOW = 11 (DM PC VGA slot 11).  The host-receipt
         * diagnostic path uses yellow for chapter/scan log lines. */
        callbacks->log_event(callbacks->userdata, 11U, receipt->log_first_line);
    }
    if (callbacks->log_event && receipt->log_receipt &&
        runtime_receipt && runtime_receipt[0]) {
        callbacks->log_event(callbacks->userdata, 11U, runtime_receipt);
    }

    if (out_result) {
        if (receipt->input_result == THERON_STARTUP_INPUT_RESULT_REDRAW) {
            *out_result = THERON_V1_BOOT_HOST_RECEIPT_RESULT_REDRAW;
        } else if (receipt->input_result ==
                   THERON_STARTUP_INPUT_RESULT_RETURN_TO_LAUNCHER) {
            *out_result = THERON_V1_BOOT_HOST_RECEIPT_RESULT_RETURN_TO_MENU;
        }
    }
    return 1;
}

int theron_v1_boot_apply_startup_action_host_receipt(
    const Theron_StartupActionHostReceipt *receipt,
    const Theron_V1_BootActionReceiptCallbacks *callbacks,
    Theron_V1_BootHostReceiptResult *out_result)
{
    Theron_V1_BootHostReceiptCallbacks host_callbacks;

    if (out_result) {
        *out_result = THERON_V1_BOOT_HOST_RECEIPT_RESULT_IGNORED;
    }
    if (!receipt || !callbacks) {
        return 0;
    }

    /* Apply embedded state receipt first.  The state receipt carries flow
     * snapshot, level-loaded flag, cursor, continue focus, party pose,
     * tick count, and save/resume fields produced by the startup flow.
     * Source: THQUEST.ASM T400 startup state handoff. */
    if (receipt->state_receipt_valid && callbacks->apply_state_receipt) {
        callbacks->apply_state_receipt(
            callbacks->userdata, &receipt->state_receipt);
    }

    /* Forward the host receipt through the shared status/inspect/log
     * hooks.  The runtime receipt string is carried inside the action
     * receipt and is logged only when host_receipt.log_receipt is set. */
    host_callbacks.userdata     = callbacks->userdata;
    host_callbacks.set_status   = callbacks->set_status;
    host_callbacks.set_inspect  = callbacks->set_inspect;
    host_callbacks.log_event    = callbacks->log_event;
    (void)theron_v1_boot_apply_startup_host_receipt(
        &receipt->host_receipt,
        receipt->runtime_receipt,
        &host_callbacks,
        out_result);

    /* Notify the host that startup phase may have changed so it can
     * re-evaluate Track 01 CDDA playback state. */
    if (callbacks->update_track01_cdda_lifecycle) {
        callbacks->update_track01_cdda_lifecycle(callbacks->userdata);
    }

    return 1;
}

static void theron_v1_boot_runtime_render_v2_hud(
    const Theron_V1_World *world,
    Theron_V1_Viewport *viewport,
    int presentation_is_v2,
    int hud_launch_mode)
{
    Theron_V2_HudOverlay hud;
    Theron_V2_HudSeedGate gate;

    if (!world || !viewport || !viewport->fb.data ||
        viewport->fb.w <= 0 || viewport->fb.h <= 0) {
        return;
    }
    if (!presentation_is_v2 ||
        hud_launch_mode == THERON_V2_HUD_LAUNCH_MODE_OFF) {
        return;
    }
    /* The overlay implementation is procedural presentation scaffolding.
     * Do not expose it in the production boot path until every HUD widget is
     * backed by a real, non-placeholder asset manifest.  The native Track 02
     * data currently proves startup surfaces only; it does not bind a HUD
     * chrome/font/portrait bank. */
    if (theron_v2_hud_widget_assets_gate() !=
        THERON_V2_HUD_WIDGET_GATE_COMPLETE) {
        return;
    }
    gate = theron_v2_hud_seed_from_v1_world(&hud, world, 1);
    if (gate != THERON_V2_HUD_SEED_V2_READY) {
        return;
    }
    theron_v2_hud_render(&hud,
                         viewport->fb.data,
                         viewport->fb.w,
                         viewport->fb.h);
}

static int theron_v1_boot_asset_bundle_allows_v1_rendering(
    const TrAssetBundle *assets)
{
    if (!assets || !assets->assets_verified ||
        !assets->hucard_rom || assets->hucard_rom_size == 0u) {
        return 0;
    }
    /* Verified original Track 02 bytes are authoritative. When synthetic
     * rendering has been blocked because no source-locked graphics bank is
     * bound, refuse generated V1 artwork until a real tile bank is decoded.
     * Source: theron_v1_asset_loader.h synthetic_rendering_blocked contract;
     * THQUEST.ASM T400/T410 graphics-bank boundary. */
    if (assets->synthetic_rendering_blocked &&
        !tr_asset_generated_v1_rendering_allowed(assets)) {
        return 0;
    }
    return 1;
}

int theron_v1_boot_runtime_render_frame(Theron_V1_World *world,
                                        Theron_V1_Viewport *viewport,
                                        const TrAssetBundle *assets,
                                        int presentation_is_v2,
                                        int hud_launch_mode,
                                        unsigned char *framebuffer,
                                        int framebuffer_width,
                                        int framebuffer_height)
{
    if (!world || !viewport || !assets || !framebuffer ||
        framebuffer_width <= 0 || framebuffer_height <= 0) {
        return 0;
    }
    /* A caller can present a viewport-only indexed frame without asking for
     * generated V1 artwork. If an asset bundle is supplied, it must still be
     * original-data backed before its palette/tile path is consumed. The
     * render gate requires both a decoded tile bank and a verified HuC6260
     * palette route; a default deterministic palette is not source-locked. */
    theron_vp_set_synthetic_rendering_blocked(
        viewport,
        assets->synthetic_rendering_blocked &&
            !tr_asset_generated_v1_rendering_allowed(assets));
    if (!theron_v1_boot_asset_bundle_allows_v1_rendering(assets)) {
        return 0;
    }
    /* When the runtime owns an asset bundle with decoded tiles, make the
     * viewport palette reflect that bank before drawing. This is a shallow
     * copy of tile metadata; the asset bundle retains ownership of the
     * underlying Track 02/03 bytes. Source: THQUEST.ASM T400 tile bank load
     * followed by T520 viewport tile selection. */
    theron_vp_set_palette(viewport, &assets->palette);
    /* THQUEST.ASM T560/T600/T800 runtime owns dungeon draw, UI draw, and
     * optional V2 HUD overlay before M11 presents the indexed viewport. */
    theron_vp_render_dungeon(viewport, world);
    theron_vp_render_ui(viewport, world, TQR_UI_ALL);
    theron_v1_boot_runtime_render_v2_hud(world,
                                         viewport,
                                         presentation_is_v2,
                                         hud_launch_mode);
    theron_vp_present(viewport,
                      &assets->palette,
                      framebuffer,
                      framebuffer_width,
                      framebuffer_height);
    return 1;
}

int theron_v1_boot_startup_execute_graphics_plan(
    const Theron_StartupRenderPlan *plan,
    const Theron_StartupGraphicExecutor *executor)
{
    return theron_v1_startup_execute_graphics_plan(plan, executor);
}

int theron_v1_boot_startup_return_to_stage_select_after_exit_host_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    Theron_V1_World *world)
{
    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_action_host_receipt_init(out_receipt);
    if (!world) {
        return 0;
    }
    return theron_v1_startup_return_to_stage_select_after_exit_host_receipt(
        world,
        out_receipt);
}

int theron_v1_boot_startup_return_to_stage_select_after_exit_profile_host_receipt(
    Theron_StartupActionHostReceipt *out_receipt,
    const void *boot_profile,
    Theron_V1_World *world)
{
    Theron_StartupChapterInspectReceipt inspect;
    char prefix[128];

    if (!theron_v1_boot_startup_return_to_stage_select_after_exit_host_receipt(
            out_receipt,
            world)) {
        return 0;
    }
    if (!out_receipt || !world) {
        return 0;
    }
    snprintf(prefix,
             sizeof(prefix),
             "%s",
             out_receipt->runtime_receipt[0]
                 ? out_receipt->runtime_receipt
                 : "dungeon complete");
    if (theron_v1_startup_chapter_inspect_receipt_from_facts(
            boot_profile,
            world,
            prefix,
            &inspect)) {
        out_receipt->host_receipt.inspect_scope = "DUNGEON COMPLETE";
        snprintf(out_receipt->host_receipt.inspect_detail,
                 sizeof(out_receipt->host_receipt.inspect_detail),
                 "%s",
                 inspect.inspect_detail);
    }
    return 1;
}

static void theron_v1_boot_startup_launch_host_receipt_init(
    Theron_StartupHostReceipt *receipt) {
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
}

static const char *theron_v1_boot_startup_prepare_host_status(
    Theron_V1BootStartupPrepareResult result) {
    switch (result) {
        case THERON_V1_BOOT_STARTUP_PREPARE_VERIFY_FAILED:
            return "THERON TRACK 02 VERIFY FAILED";
        case THERON_V1_BOOT_STARTUP_PREPARE_MISSING_TRACK02:
            return "THERON TRACK 02 MISSING";
        case THERON_V1_BOOT_STARTUP_PREPARE_ASSET_LOAD_FAILED:
            return "THERON ASSET LOAD FAILED";
        case THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED:
            return "THERON STARTUP STATE FAILED";
        default:
            return theron_v1_boot_startup_prepare_result_name(result);
    }
}

static void theron_v1_boot_startup_launch_build_failure_host_receipt(
    Theron_V1BootStartupPrepareResult result,
    Theron_StartupHostReceipt *receipt) {
    if (!receipt) {
        return;
    }
    theron_v1_boot_startup_launch_host_receipt_init(receipt);
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
    receipt->status_scope = "BOOT";
    receipt->status = theron_v1_boot_startup_prepare_host_status(result);
    receipt->inspect_scope = "THERON STARTUP";
    snprintf(receipt->inspect_detail,
             sizeof(receipt->inspect_detail),
             "%s",
             receipt->status ? receipt->status : "THERON STARTUP FAILED");
}

static void theron_v1_boot_startup_launch_build_host_receipt(
    Theron_V1_BootStartupLaunch *launch) {
    Theron_StartupHostReceipt *receipt;
    Theron_StartupChapterInspectReceipt inspect;
    char prefix[256];
    if (!launch) {
        return;
    }
    receipt = &launch->launch_host_receipt;
    theron_v1_boot_startup_launch_host_receipt_init(receipt);
    receipt->status_scope = "BOOT";
    receipt->status = "THERON STARTUP";
    receipt->inspect_scope = "THERON STARTUP";
    receipt->log_first_line = "T0: THERON STARTUP READY";
    snprintf(prefix,
             sizeof(prefix),
             "THERON TRACK 02 VERIFIED; variant=%s bytes=%lu media_ready=%d md5=%s; SAVE %s tqsv=%d srm=%d; roster_names=%d status=%s text_prompts=%d text_status=%s; CHOOSE A STAGE",
             theron_v1_track02_variant_name(
                 (Theron_Track02Variant)
                     launch->startup_media_state_receipt.track02_variant),
             (unsigned long)launch->startup_media_state_receipt.track02_size,
             launch->startup_media_state_receipt.startup_media_ready,
             launch->startup_media_state_receipt.track02_md5[0]
                 ? launch->startup_media_state_receipt.track02_md5
                 : "UNKNOWN",
             launch->save_resume_ready
                 ? launch->save_resume.resume_claim_name
                 : "UNKNOWN",
             launch->save_resume_ready ? launch->save_resume.tqsv_valid_slots
                                       : 0,
             launch->save_resume_ready
                 ? launch->save_resume.srm_recognized_slots
                 : 0,
             launch->startup_media_state_receipt.startup_roster_name_count,
             theron_v1_track02_signal_status_name(
                 (Theron_Track02SignalStatus)
                     launch->startup_media_state_receipt.
                         startup_roster_name_status),
             launch->startup_media_state_receipt.startup_text_prompt_count,
             theron_v1_track02_signal_status_name(
                 (Theron_Track02SignalStatus)
                     launch->startup_media_state_receipt.
                         startup_text_prompt_status));
    if (theron_v1_startup_chapter_inspect_receipt_from_facts(
            launch->profile,
            launch->world,
            prefix,
            &inspect)) {
        snprintf(receipt->inspect_detail,
                 sizeof(receipt->inspect_detail),
                 "%s",
                 inspect.inspect_detail);
        receipt->inspect_scope = "STARTUP";
    } else {
        snprintf(receipt->inspect_detail,
                 sizeof(receipt->inspect_detail),
                 "%s",
                 prefix);
    }
}

int theron_v1_boot_startup_launch_apply_irq2_preflight_receipt(
    Theron_V1_BootStartupLaunch *launch,
    const char *redacted_receipt) {
    Theron_V1Irq2PreflightLauncherReceipt receipt;

    if (!launch || !theron_v1_irq2_preflight_launcher_receipt_from_redacted_receipt(
            redacted_receipt, &receipt) || !receipt.runtime_blocked) {
        return 0;
    }
    launch->irq2_preflight_receipt = receipt;
    /* Preserve the normal profile/media state for diagnosis, but do not let
     * this capture-only boundary detach a runtime. */
    theron_v1_boot_startup_launch_host_receipt_init(&launch->launch_host_receipt);
    launch->launch_host_receipt.input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
    launch->launch_host_receipt.status_scope = "IRQ2 PREFLIGHT";
    launch->launch_host_receipt.status =
        theron_v1_irq2_preflight_status_name(receipt.status);
    launch->launch_host_receipt.inspect_scope = "IRQ2 PREFLIGHT";
    snprintf(launch->launch_host_receipt.inspect_detail,
             sizeof(launch->launch_host_receipt.inspect_detail),
             "%s",
             launch->launch_host_receipt.status);
    launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED;
    return 1;
}

int theron_v1_boot_startup_launch_apply_track02_runtime_trace_from_files(
    Theron_V1_BootStartupLaunch *launch,
    const char *system_card_path,
    const char *system_card_md5_hex,
    const char *trace_path,
    const char *trace_md5_hex) {
    Theron_V1_BootTrack02RuntimeTraceIntakeReceipt intake;
    Theron_Track02Variant expected_variant;

    if (!launch || !launch->profile || !launch->assets ||
        !launch->assets->hucard_rom || !launch->profile->graphics_path[0] ||
        !launch->profile->graphics_md5[0]) {
        return 0;
    }
    memset(&intake, 0, sizeof(intake));
    launch->profile->track02_runtime_trace_handoff_ready = 0;
    memset(launch->profile->track02_runtime_system_card_md5, 0,
           sizeof(launch->profile->track02_runtime_system_card_md5));
    memset(launch->profile->track02_runtime_trace_md5, 0,
           sizeof(launch->profile->track02_runtime_trace_md5));
    memset(&launch->profile->track02_runtime_trace_handoff,
           0,
           sizeof(launch->profile->track02_runtime_trace_handoff));
    memset(&launch->profile->track02_initial_level_handoff,
           0,
           sizeof(launch->profile->track02_initial_level_handoff));
    if (!theron_v1_boot_track02_runtime_trace_intake_from_files(
            launch->profile->graphics_path,
            launch->profile->graphics_md5,
            system_card_path,
            system_card_md5_hex,
            trace_path,
            trace_md5_hex,
            &intake)) {
        return 0;
    }
    expected_variant = theron_v1_track02_variant_for_md5(
        launch->profile->graphics_md5);
    if (!intake.valid || !intake.trace_file_hash_verified ||
        !intake.system_card_file_hash_verified ||
        !intake.trace_file_consumed ||
        intake.runtime_handoff.variant != expected_variant) {
        return 0;
    }
    launch->profile->track02_runtime_trace_handoff = intake.runtime_handoff;
    snprintf(launch->profile->track02_runtime_system_card_md5,
             sizeof(launch->profile->track02_runtime_system_card_md5), "%s",
             intake.system_card_md5);
    snprintf(launch->profile->track02_runtime_trace_md5,
             sizeof(launch->profile->track02_runtime_trace_md5), "%s",
             intake.trace_md5);
    launch->profile->track02_runtime_trace_handoff_ready = 1;
    return theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
        launch->profile);
}

int theron_v1_boot_runtime_capture_manifest_from_file(
    const Theron_V1_BootProfile *profile,
    const char *manifest_path,
    Theron_V1CaptureManifest *out_manifest) {
    unsigned char *text;
    size_t text_size = 0u;
    Theron_V1CaptureManifest parsed;
    char observed_track02_md5[33];
    int accepted = 0;

    if (out_manifest) memset(out_manifest, 0, sizeof(*out_manifest));
    if (!profile || !manifest_path || !out_manifest ||
        !profile->assets_verified || !profile->graphics_path[0] ||
        !profile->graphics_md5[0]) {
        return 0;
    }
    /* A capture manifest is not allowed to revive a profile whose Track 02
     * bytes changed after boot discovery. This is provenance only: no
     * Track 02 record, bitmap, palette, or dungeon semantics are inferred. */
    if (!m12_file_md5_hex(profile->graphics_path, observed_track02_md5) ||
        strcmp(observed_track02_md5, profile->graphics_md5) != 0) {
        return 0;
    }
    text = theron_v1_boot_read_evidence_file(
        manifest_path, THERON_V1_RUNTIME_CAPTURE_MANIFEST_MAX_BYTES,
        &text_size);
    if (!text || text_size == 0u ||
        !theron_v1_capture_manifest_parse((const char *)text, &parsed) ||
        strcmp(parsed.track02_path, profile->graphics_path) != 0 ||
        strcmp(parsed.track02_md5, profile->graphics_md5) != 0) {
        goto done;
    }
    *out_manifest = parsed;
    accepted = 1;

done:
    free(text);
    return accepted;
}

int theron_v1_boot_startup_launch_apply_track02_runtime_capture_manifest_from_file(
    Theron_V1_BootStartupLaunch *launch,
    const char *manifest_path) {
    return theron_v1_boot_startup_launch_apply_track02_initial_level_capture_manifest_from_file(
        launch, manifest_path);
}

int theron_v1_boot_startup_launch_apply_track02_initial_level_capture_manifest_from_file(
    Theron_V1_BootStartupLaunch *launch,
    const char *manifest_path) {
    Theron_V1_BootProfile staged_profile;
    Theron_V1_BootProfile *profile;
    Theron_V1CaptureManifest manifest;
    unsigned char *track02 = NULL;
    unsigned char *trace = NULL;
    size_t track02_size = 0u;
    size_t trace_size = 0u;
    char observed_track02_md5[33];
    char observed_system_card_md5[33];
    char observed_trace_md5[33];
    Theron_V1RawLoaderTraceCoalescedLaterReceipt coalesced;
    Theron_V1RawLoaderTraceInitialLevelHandoffReceipt initial_level;
    int accepted = 0;

    if (!launch || !launch->profile) return 0;

    /* An operator may retry capture intake while a prior authentic handoff is
     * still live.  Keep that handoff intact until every artifact, trace edge,
     * and Track 02 span has been revalidated. */
    staged_profile = *launch->profile;
    profile = &staged_profile;
    memset(&profile->track02_initial_level_handoff, 0,
           sizeof(profile->track02_initial_level_handoff));
    profile->track02_runtime_trace_handoff_ready = 0;
    memset(profile->track02_runtime_system_card_md5, 0,
           sizeof(profile->track02_runtime_system_card_md5));
    memset(profile->track02_runtime_trace_md5, 0,
           sizeof(profile->track02_runtime_trace_md5));
    memset(&profile->track02_runtime_trace_handoff, 0,
           sizeof(profile->track02_runtime_trace_handoff));
    if (!theron_v1_boot_runtime_capture_manifest_from_file(
            profile, manifest_path, &manifest) ||
        !m12_file_md5_hex(profile->graphics_path,
                          observed_track02_md5) ||
        !m12_file_md5_hex(manifest.system_card_path,
                          observed_system_card_md5) ||
        !m12_file_md5_hex(manifest.trace_path, observed_trace_md5) ||
        !theron_v1_raw_loader_trace_capture_manifest_matches(
            &manifest, profile->graphics_path,
            observed_track02_md5, manifest.system_card_path,
            observed_system_card_md5, manifest.trace_path,
            observed_trace_md5)) {
        return 0;
    }
    track02 = theron_v1_boot_read_evidence_file(
        profile->graphics_path, THERON_V1_RUNTIME_MEDIA_MAX_BYTES,
        &track02_size);
    trace = theron_v1_boot_read_evidence_file(
        manifest.trace_path, THERON_V1_RUNTIME_TRACE_MAX_BYTES, &trace_size);
    if (!track02 || !trace || memchr(trace, '\0', trace_size) != NULL ||
        !theron_v1_raw_loader_trace_bind_coalesced_later_e009_raw_sector(
            (const char *)trace, track02, track02_size,
            profile->graphics_md5, &coalesced) ||
        !theron_v1_raw_loader_trace_bind_initial_level_handoff(
            &coalesced, track02, track02_size,
            profile->graphics_md5, &initial_level) ||
        !theron_v1_raw_loader_trace_bind_capture_manifest_to_initial_level_handoff(
            &initial_level, &manifest, profile->graphics_path,
            observed_track02_md5, manifest.system_card_path,
            observed_system_card_md5, manifest.trace_path, observed_trace_md5,
            &initial_level)) {
        goto done;
    }
    profile->track02_initial_level_handoff = initial_level;
    snprintf(profile->track02_runtime_system_card_md5,
             sizeof(profile->track02_runtime_system_card_md5), "%s",
             observed_system_card_md5);
    snprintf(profile->track02_runtime_trace_md5,
             sizeof(profile->track02_runtime_trace_md5), "%s",
             observed_trace_md5);
    profile->track02_runtime_trace_handoff_ready = 1;
    accepted = theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
        profile);
    if (accepted) *launch->profile = staged_profile;

done:
    free(track02);
    free(trace);
    return accepted;
}

void theron_v1_boot_startup_launch_cleanup(
    Theron_V1_BootStartupLaunch *launch) {
    if (!launch) {
        return;
    }
    theron_v1_boot_runtime_release(&launch->profile,
                                   &launch->world,
                                   &launch->viewport,
                                   &launch->assets);
    memset(launch, 0, sizeof(*launch));
}

void theron_v1_boot_runtime_release(
    Theron_V1_BootProfile **profile,
    Theron_V1_World **world,
    Theron_V1_Viewport **viewport,
    TrAssetBundle **assets)
{
    if (viewport && *viewport) {
        theron_vp_free(*viewport);
        free(*viewport);
        *viewport = NULL;
    }
    if (assets && *assets) {
        tr_asset_free(*assets);
        free(*assets);
        *assets = NULL;
    }
    if (world && *world) {
        free(*world);
        *world = NULL;
    }
    if (profile && *profile) {
        theron_v1_boot_cleanup(*profile);
        free(*profile);
        *profile = NULL;
    }
}

int theron_v1_boot_startup_launch_bind_campaign_media(
    Theron_V1_BootStartupLaunch *launch,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan)
{
    if (!launch) {
        return 0;
    }
    memset(&launch->campaign_media_discovery, 0,
           sizeof(launch->campaign_media_discovery));
    launch->campaign_media_launchable = 0;
    if (!media) {
        return 0;
    }
    launch->campaign_media_discovery = *media;
    if (!launch->profile || media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
        media->ambiguous || media->virtual_container || media->no_media_extracted ||
        !media->launchable_direct_media || !media->exact_layout_bound ||
        media->track02_variant == THERON_TRACK02_VARIANT_UNKNOWN ||
        !media->candidate_path[0] || !media->direct_media.payload_path[0] ||
        strcmp(launch->profile->graphics_md5, media->track02_md5) ||
        !theron_v1_track02_campaign_media_bind_capture_plan(media, plan)) {
        return 0;
    }
    launch->campaign_media_launchable = 1;
    return 1;
}

int theron_v1_boot_startup_launch_alloc_from_campaign_media(
    const char *data_dir,
    const char *search_path,
    const char *expected_track02_md5,
    const char *save_path,
    const Theron_V1Track02CaptureTargetPlan *plan,
    Theron_V1_BootStartupLaunch *out_launch)
{
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;

    if (!out_launch) {
        return 0;
    }
    memset(out_launch, 0, sizeof(*out_launch));
    if (!theron_v1_track02_campaign_media_discover(search_path,
                                                     expected_track02_md5,
                                                     4, &media)) {
        return 0;
    }
    out_launch->campaign_media_discovery = media;
    if (media.status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
        media.virtual_container || !media.launchable_direct_media ||
        !theron_v1_track02_campaign_media_bind_capture_plan(&media, plan) ||
        !theron_v1_boot_startup_launch_alloc(data_dir, media.candidate_path,
                                             expected_track02_md5, save_path,
                                             out_launch)) {
        out_launch->campaign_media_discovery = media;
        out_launch->campaign_media_launchable = 0;
        return 0;
    }
    if (!theron_v1_boot_startup_launch_bind_campaign_media(out_launch, &media,
                                                            plan)) {
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->campaign_media_discovery = media;
        return 0;
    }
    return 1;
}

int theron_v1_boot_startup_launch_alloc(
    const char *data_dir,
    const char *verified_path,
    const char *verified_md5,
    const char *save_path,
    Theron_V1_BootStartupLaunch *out_launch) {

    if (!out_launch) {
        return 0;
    }
    memset(out_launch, 0, sizeof(*out_launch));
    out_launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
    theron_v1_boot_startup_launch_build_failure_host_receipt(
        out_launch->prepare_result,
        &out_launch->launch_host_receipt);
    if (!data_dir || data_dir[0] == '\0') {
        return 0;
    }

    out_launch->profile =
        (Theron_V1_BootProfile*)calloc(1, sizeof(*out_launch->profile));
    out_launch->world =
        (Theron_V1_World*)calloc(1, sizeof(*out_launch->world));
    out_launch->viewport =
        (Theron_V1_Viewport*)calloc(1, sizeof(*out_launch->viewport));
    out_launch->assets =
        (TrAssetBundle*)calloc(1, sizeof(*out_launch->assets));
    if (!out_launch->profile || !out_launch->world ||
        !out_launch->viewport || !out_launch->assets) {
        Theron_StartupHostReceipt receipt;
        out_launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
        theron_v1_boot_startup_launch_build_failure_host_receipt(
            out_launch->prepare_result,
            &receipt);
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
        out_launch->launch_host_receipt = receipt;
        return 0;
    }

    if (!theron_v1_boot_prepare_startup_profile(
            out_launch->profile,
            data_dir,
            verified_path,
            verified_md5,
            save_path,
            out_launch->assets,
            &out_launch->save_resume,
            &out_launch->save_resume_ready,
            &out_launch->prepare_result)) {
        Theron_V1BootStartupPrepareResult result = out_launch->prepare_result;
        Theron_StartupHostReceipt receipt;
        theron_v1_boot_startup_launch_build_failure_host_receipt(
            result,
            &receipt);
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->prepare_result = result;
        out_launch->launch_host_receipt = receipt;
        return 0;
    }

    theron_v1_world_init(out_launch->world);
    if (!theron_vp_init(out_launch->viewport)) {
        Theron_StartupHostReceipt receipt;
        out_launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
        theron_v1_boot_startup_launch_build_failure_host_receipt(
            out_launch->prepare_result,
            &receipt);
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->prepare_result = THERON_V1_BOOT_STARTUP_PREPARE_BAD_INPUT;
        out_launch->launch_host_receipt = receipt;
        return 0;
    }
    if (!theron_v1_startup_initial_title_state_receipt(
            out_launch->world,
            &out_launch->startup_flow,
            &out_launch->initial_state_receipt)) {
        Theron_StartupHostReceipt receipt;
        out_launch->prepare_result =
            THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED;
        theron_v1_boot_startup_launch_build_failure_host_receipt(
            out_launch->prepare_result,
            &receipt);
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->prepare_result =
            THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED;
        out_launch->launch_host_receipt = receipt;
        return 0;
    }
    if (!theron_v1_startup_save_resume_state_receipt(
            &out_launch->save_resume,
            out_launch->save_resume_ready,
            &out_launch->save_resume_state_receipt)) {
        Theron_StartupHostReceipt receipt;
        out_launch->prepare_result =
            THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED;
        theron_v1_boot_startup_launch_build_failure_host_receipt(
            out_launch->prepare_result,
            &receipt);
        theron_v1_boot_startup_launch_cleanup(out_launch);
        out_launch->prepare_result =
            THERON_V1_BOOT_STARTUP_PREPARE_STATE_FAILED;
        out_launch->launch_host_receipt = receipt;
        return 0;
    }
    theron_v1_startup_media_capture_track02_state_receipt(
        out_launch->assets ? out_launch->assets->hucard_rom : NULL,
        out_launch->assets ? out_launch->assets->hucard_rom_size : 0u,
        out_launch->profile->graphics_md5,
        &out_launch->startup_media_state_receipt);
    /* Bind the captured atlas into the world's runtime media so the M11
     * verified-surfaces gate (TRACK02 ATLAS ROUTES) can pass: nothing else
     * on the start path binds it, so direct launches died at the gate even
     * with a complete, anchored media receipt.  Fail-closed is preserved:
     * without complete source-backed bitmap routes nothing is bound. */
    if (theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(
            &out_launch->startup_media_state_receipt)) {
        (void)theron_v1_startup_media_bind_runtime_receipt(
            out_launch->world,
            &out_launch->startup_media_state_receipt);
    }
    theron_v1_boot_startup_launch_build_host_receipt(out_launch);
    return 1;
}

int theron_v1_boot_startup_launch_detach_runtime(
    Theron_V1_BootStartupLaunch *launch,
    Theron_V1_BootStartupRuntimeReceipt *out_receipt) {

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!launch || !launch->profile || !launch->world ||
        !launch->viewport || !launch->assets) {
        return 0;
    }

    out_receipt->profile = launch->profile;
    out_receipt->world = launch->world;
    out_receipt->viewport = launch->viewport;
    out_receipt->assets = launch->assets;
    out_receipt->initial_state_receipt = launch->initial_state_receipt;
    out_receipt->save_resume_state_receipt = launch->save_resume_state_receipt;
    out_receipt->startup_media_state_receipt =
        launch->startup_media_state_receipt;
    out_receipt->launch_host_receipt = launch->launch_host_receipt;
    out_receipt->campaign_media_discovery = launch->campaign_media_discovery;
    out_receipt->campaign_media_launchable = launch->campaign_media_launchable;
    snprintf(out_receipt->boot_asset_md5,
             sizeof(out_receipt->boot_asset_md5),
             "%s",
             launch->profile->graphics_md5);
    snprintf(out_receipt->title,
             sizeof(out_receipt->title),
             "THERON'S QUEST");
    snprintf(out_receipt->source_id,
             sizeof(out_receipt->source_id),
             "theron");
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s",
             launch->profile->graphics_path);

    launch->profile = NULL;
    launch->world = NULL;
    launch->viewport = NULL;
    launch->assets = NULL;
    return 1;
}

int theron_v1_boot_startup_save_resume(
    const Theron_V1_BootProfile *profile,
    Theron_V1StartupSaveResume *out_snapshot) {

    if (!profile || !out_snapshot) {
        return 0;
    }
    return theron_v1_startup_save_resume_evaluate(profile->save_root,
                                                  out_snapshot);
}

/* ── Deterministic config from dungeon header ─────────────────────── */

/*
 * Theron's Quest DUNGEON.DAT header layout — mirrors DM2 structure:
 *   offset 0-1: reserved (0x0000)
 *   offset 2-3: magic ("T1" — TQ specific, TBD after Phase 2 extraction)
 *   offset 4-5: first level data offset
 *   offset 6-7: dungeon_count (= 7 mini-dungeons)
 *   offset 8-9: dungeon_seed (word)
 *   offset 10-11: metadata
 *
 * Source: THQUEST.ASM T560
 */
static uint16_t trv_rd16_le(const uint8_t *p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

void theron_v1_boot_build_deterministic_config(
        Theron_V1_BootProfile *profile,
        const uint8_t *dungeon_header,
        int dungeon_size) {
    if (!profile || !dungeon_header || dungeon_size < 12) return;

    /* Read dungeon seed from header offset 8 */
    uint16_t seed = trv_rd16_le(dungeon_header + 8);
    profile->deterministic.dungeon_seed = seed;

    /* Read dungeon count from header offset 6 — TQ has 7 mini-dungeons */
    uint16_t dc = trv_rd16_le(dungeon_header + 6);
    if (dc > 0 && dc <= 64) {
        profile->deterministic.dungeon_count = dc;
        /* Set max levels per mini-dungeon heuristically */
        profile->deterministic.max_levels =
            (dc >= 1) ? 3 : 1; /* TQ mini-dungeons are shallow */
    }
}

/* ── Enter game ────────────────────────────────────────────────────── */

/*
 * theron_v1_boot_enter_game — transition from boot to game state.
 *
 * Sets profile->theron_state and profile->dungeon_data.
 * This is the lowest-level entry point — the caller (M12 game dispatch)
 * holds the boot profile until this call completes.
 *
 * A verified Track 02 runtime handoff owns the actual transition.  This
 * legacy low-level entry point must not report success while its state is
 * still unbound.
 */
int theron_v1_boot_enter_game(Theron_V1_BootProfile *profile) {
    if (!profile) return -1;

    /* No source-locked state owner is wired to this legacy API. */
    profile->theron_state = NULL;
    profile->dungeon_data = NULL;
    return -1;
}

/* ── Cleanup ──────────────────────────────────────────────────────── */

void theron_v1_boot_cleanup(Theron_V1_BootProfile *profile) {
    (void)profile;
    /* Phase 2 will free dungeon_data and theron_state here */
}

/* ── Diagnostics ──────────────────────────────────────────────────── */

size_t theron_v1_diagnostic_report(const Theron_V1_BootProfile *profile,
                                    char *buf, size_t buf_size) {
    if (!profile || !buf || buf_size == 0) return 0;

    int n = snprintf(buf, buf_size,
        "=== Theron V1 Boot Profile ===\n"
        "Game:          %s\n"
        "Platform:      %s (%s)\n"
        "Asset root:     %s\n"
        "GRAPHICS:       %s\n"
        "  size:         %zu bytes\n"
        "  MD5:          %.32s%s\n"
        "DUNGEON:        %s\n"
        "  size:         %zu bytes\n"
        "  MD5:          %.32s%s\n"
        "Hash verified:  %s\n"
        "Save root:      %s\n"
        "In-dungeon save:%s (TQ design restriction)\n"
        "\n"
        "=== Deterministic Config ===\n"
        "Tick rate:      %u.%u Hz (~%u ms/tick)\n"
        "Dungeon move:   0x%04x Q8\n"
        "Max champions:   %u  Max party: %u\n"
        "Dungeon count:  %u  Max levels: %u\n"
        "Dungeon seed:    %u\n"
        "Quest items:    %u\n"
        "\n"
        "=== Phase Gate ===\n"
        "Phase 0 provenance gate: PASSED\n"
        "Reference:      docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md\n"
        "JP MD5:         b7afb338ad31be1025b53f9aff12d73a\n"
        "US MD5:         f23601102138f87c33025877767ebf76\n"
        "Next step:       bind source-locked TQ dungeon/graphics semantics\n"
        "                 to the verified Track 02 BIN route\n"
        "Asset verdict:   %s\n",
        profile->game_id,
        profile->platform_label,
        profile->version_id,
        profile->asset_root[0] ? profile->asset_root : "(none)",

        profile->graphics_path[0] ? profile->graphics_path : "(none)",
        profile->graphics_size,
        profile->graphics_md5[0] ? profile->graphics_md5 : "????????",
        profile->graphics_md5[0] ? "" : " [no verified Track 02 source]",

        profile->dungeon_path[0] ? profile->dungeon_path : "(none)",
        profile->dungeon_size,
        profile->dungeon_md5[0] ? profile->dungeon_md5 : "????????",
        profile->dungeon_md5[0] ? "" : " [no verified Track 02 source]",

        profile->assets_verified ? "YES" : "NO (verified Track 02 source required)",

        profile->save_root[0] ? profile->save_root : "(none)",
        profile->in_dungeon_save_allowed ? "ALLOWED" : "BLOCKED",

        profile->deterministic.tick_rate_hz,
        profile->deterministic.tick_rate_hz_frac,
        profile->deterministic.tick_ms,
        profile->deterministic.dungeon_move_speed,
        profile->deterministic.max_champions,
        profile->deterministic.max_party_members,
        profile->deterministic.dungeon_count,
        profile->deterministic.max_levels,
        profile->deterministic.dungeon_seed,
        profile->deterministic.quest_items_collected,

        profile->assets_verified ? "PASSED" : "blocked (verified Track 02 source required)"
    );

    /* Truncate to buf_size */
    if (n < 0 || (size_t)n >= buf_size) {
        return buf_size > 0 ? buf_size - 1 : 0;
    }
    return (size_t)n;
}

void theron_v1_boot_print_summary(const Theron_V1_BootProfile *profile) {
    if (!profile) return;
    printf("Theron V1 boot: platform=%s version=%s "
           "graphics=%zu dungeon=%zu save=%s\n",
           profile->platform_label,
           profile->version_id,
           profile->graphics_size,
           profile->dungeon_size,
           profile->save_root[0] ? profile->save_root : "(default)");
}

const char *theron_v1_boot_source_evidence(void) {
    /* Source evidence citation string — used in assert comments and debug output.
     * The runtime now locks identity to the verified Track 02 route;
     * remaining work is semantic dungeon/graphics binding. */
    return "theron_v1_boot.c: "
           "THQUEST.ASM T000 (startup), T080 (save ns), "
           "T200 (platform diag), T400 (bank load), "
           "T520 (party placement), T560 (dungeon load), "
           "T800 (champion persistence) — "
           "verified Track 02 identity locked; awaiting semantic dungeon "
           "format binding (TQR data extracted from Track 02 BIN)";
}
