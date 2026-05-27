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
 * Provenance gate (Phase 0 — PASSED):
 *   No hash-verified asset set yet. This module probes for assets
 *   but marks them unverified (assets_verified=0) until Phase 2
 *   locks the exact THQUEST.GFX / THQUEST.DUN hashes from a known
 *   good PC Engine HuCard image.
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

/* ── PC Engine file candidates ───────────────────────────────────── */

/* Theron's Quest data files — Phase 0 locked (2026-05-27).
 *
 * CD-ROM disc structure: CUE/BIN, 18 tracks, Track 02 = data track.
 * The data track contains the HuC6280 executable, graphics tiles
 * (tile/sprite format), and dungeon data — embedded in one binary blob
 * per region.  Unlike DM1's dual-file structure (GRAPHICS.DAT +
 * DUNGEON.DAT), Theron's Quest uses a single Track 02 per region.
 *
 * JP Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
 * US Track 02 MD5: f23601102138f87c33025877767ebf76
 * Source: cdromance.org (2026-05-27)
 *
 * Subdir candidates: theron/jp/, theron/us/, theron/
 *
 * File candidates (in order of precedence):
 *   track02.bin                    — canonical CD-ROM data track name
 *   "Theron's Quest (Japan) (Track 02).bin"
 *   "Theron's Quest (US) (Track 02).bin"
 *   THQUEST.BIN
 *   GRAPHICS.DAT / DUNGEON.DAT   — legacy fallback (extracted from Track 02)
 */
static const char *const g_theron_track02_candidates[] = {
    "track02.bin",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "THQUEST.BIN",
    NULL
};

static const char *const g_theron_graphics_fallback[] = {
    "GRAPHICS.DAT",
    "graphics.dat",
    NULL
};

static const char *const g_theron_dungeon_fallback[] = {
    "DUNGEON.DAT",
    "dungeon.dat",
    NULL
};

/* ── Probe: file exists and non-empty ─────────────────────────────── */

static int file_exists(const char *path) {
    struct stat st;
    return stat(path, &st) == 0 && st.st_size > 0;
}

static size_t file_size_of(const char *path) {
    struct stat st;
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
    profile->deterministic.dungeon_seed       = 313; /* default fallback */
    profile->deterministic.quest_items_collected = 0;
}

/* ── Scan assets ──────────────────────────────────────────────────── */

/*
 * theron_v1_boot_scan_assets — probe for TQ assets.
 *
 * Searches data_dir/theron/ for Track 02 BIN (CD-ROM data track).
 * Phase 0: verifies Track 02 against known JP/US MD5 hashes.
 * Fallback: extracted GRAPHICS.DAT / DUNGEON.DAT (no hash verification).
 *
 * Returns 0 on success (assets found), -1 if none detected.
 */
int theron_v1_boot_scan_assets(Theron_V1_BootProfile *profile,
                                const char *data_dir) {
    const char *base = data_dir && data_dir[0] ? data_dir : ".";

    /* Resolve Track 02 BIN (primary CD-ROM data track) */
    if (resolve_asset(base, "theron", g_theron_track02_candidates,
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
    } else {
        /* Second fallback: extracted GRAPHICS.DAT / DUNGEON.DAT */
        resolve_asset(base, "theron", g_theron_graphics_fallback,
                       profile->graphics_path,
                       &profile->graphics_size);
        resolve_asset(base, "theron", g_theron_dungeon_fallback,
                       profile->dungeon_path,
                       &profile->dungeon_size);
        if (!profile->graphics_path[0]) {
            resolve_asset(base, "", g_theron_graphics_fallback,
                           profile->graphics_path,
                           &profile->graphics_size);
        }
        if (!profile->dungeon_path[0]) {
            resolve_asset(base, "", g_theron_dungeon_fallback,
                           profile->dungeon_path,
                           &profile->dungeon_size);
        }
    }

    /* Build asset root */
    snprintf(profile->asset_root, sizeof(profile->asset_root),
             "%s%ctheron", base, TRV_PATH_SEP);

    /* Phase 0 gate: verify Track 02 MD5 against known hashes.
     * JP: b7afb338ad31be1025b53f9aff12d73a
     * US: f23601102138f87c33025877767ebf76 */
    if (profile->graphics_path[0]) {
        char md5hex[33] = {0};
        if (m12_file_md5_hex(profile->graphics_path, md5hex)) {
            if (strcmp(md5hex, "b7afb338ad31be1025b53f9aff12d73a") == 0 ||
                strcmp(md5hex, "f23601102138f87c33025877767ebf76") == 0) {
                profile->assets_verified = 1;
                strncpy(profile->graphics_md5, md5hex, 32);
                profile->graphics_md5[32] = '\0';
                strncpy(profile->dungeon_md5, md5hex, 32);
                profile->dungeon_md5[32] = '\0';
            } else {
                profile->assets_verified = 0;
            }
        } else {
            profile->assets_verified = 0;
        }
    } else {
        profile->assets_verified = 0;
    }

    /* Platform detection from filename heuristics */
    if (strstr(profile->graphics_path, "Japan") ||
        strstr(profile->graphics_path, "pce-jp")) {
        profile->platform = THERON_PLATFORM_PCE_JP;
        strncpy(profile->platform_label,
                g_platform_labels[THERON_PLATFORM_PCE_JP],
                sizeof(profile->platform_label) - 1);
        strncpy(profile->version_id, "pce-jp", sizeof(profile->version_id) - 1);
    } else if (strstr(profile->graphics_path, "US") ||
               strstr(profile->graphics_path, "pce-en")) {
        profile->platform = THERON_PLATFORM_PCE_US;
        strncpy(profile->platform_label,
                g_platform_labels[THERON_PLATFORM_PCE_US],
                sizeof(profile->platform_label) - 1);
        strncpy(profile->version_id, "pce-en", sizeof(profile->version_id) - 1);
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

/* ── Save root ────────────────────────────────────────────────────── */

void theron_v1_boot_set_save_root(Theron_V1_BootProfile *profile,
                                    const char *save_dir) {
    if (!profile) return;
    if (save_dir && save_dir[0]) {
        strncpy(profile->save_root, save_dir, sizeof(profile->save_root) - 1);
    } else {
        /* Default: <data_dir>/../saves/theron/ */
        /* Note: saves/theron/ is distinct from saves/dm1/, saves/csb/,
         * saves/dm2/, saves/nexus1/ — TQ has its own save namespace. */
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s%c..%csaves%ctheron",
                 profile->asset_root[0] ? profile->asset_root : ".",
                 TRV_PATH_SEP, TRV_PATH_SEP, TRV_PATH_SEP);
    }
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
 * Phase 2 will wire the dungeon loader; Phase 1 is purely structural.
 */
int theron_v1_boot_enter_game(Theron_V1_BootProfile *profile) {
    if (!profile) return -1;

    /* Placeholder: allocate structs once Phase 2 dungeon format is known.
     * For Phase 1, theron_state and dungeon_data remain NULL.
     * The menu card can still be displayed via status strings
     * using the boot profile alone (no game state needed). */
    profile->theron_state = NULL;
    profile->dungeon_data = NULL;

    return 0;
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
        "Next step:       Phase 2 — source-lock TQ dungeon/graphics data formats\n"
        "                 from extracted Track 02 BIN (CDRomance JP/US images)\n"
        "Asset verdict:   %s\n",
        profile->game_id,
        profile->platform_label,
        profile->version_id,
        profile->asset_root[0] ? profile->asset_root : "(none)",

        profile->graphics_path[0] ? profile->graphics_path : "(none)",
        profile->graphics_size,
        profile->graphics_md5[0] ? profile->graphics_md5 : "????????",
        profile->graphics_md5[0] ? "" : " [Phase 2 extraction pending]",

        profile->dungeon_path[0] ? profile->dungeon_path : "(none)",
        profile->dungeon_size,
        profile->dungeon_md5[0] ? profile->dungeon_md5 : "????????",
        profile->dungeon_md5[0] ? "" : " [Phase 2 extraction pending]",

        profile->assets_verified ? "YES" : "NO (Phase 2 extraction pending)",

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

        profile->assets_verified ? "PASSED" : "pending (awaiting Phase 2 asset extraction)"
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
     * Phase 1: locks to THQUEST.ASM structure; Phase 2 will add hashes.
     * Phase 1 placeholder citing the reference roadmap. */
    return "theron_v1_boot.c: "
           "THQUEST.ASM T000 (startup), T080 (save ns), "
           "T200 (platform diag), T400 (bank load), "
           "T520 (party placement), T560 (dungeon load), "
           "T800 (champion persistence) — "
           "Phase 1 COMPLETE; awaiting Phase 2 dungeon format lock (TQR "
           "data extracted from Track 02 BIN, cdromance.org JP/US)";
}
