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
#include "asset_find_by_hash.h"
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
 * CD-ROM disc structure: CUE/BIN or CUE/ISO, 18 tracks, Track 02 = data track.
 * The data track contains the HuC6280 executable, graphics tiles
 * (tile/sprite format), and dungeon data — embedded in one binary blob
 * per region.  Unlike DM1's dual-file structure (GRAPHICS.DAT +
 * DUNGEON.DAT), Theron's Quest uses a single Track 02 per region.
 *
 * JP Track 02 MD5: b7afb338ad31be1025b53f9aff12d73a
 * US Track 02 MD5: f23601102138f87c33025877767ebf76
 * JP Rev 1 Track 02 ISO MD5: 397039af02d50d15c70b74088eb8a1cb
 * US Track 02 ISO MD5:       3d8b78571dcd0e6eb8eb4b01eeb7fbba
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
 *   GRAPHICS.DAT / DUNGEON.DAT   — legacy fallback (extracted from Track 02)
 */
static const char *const g_theron_track02_candidates[] = {
    "track02.bin",
    "track02.iso",
    "Theron's Quest (Japan) (Track 02).bin",
    "Theron's Quest (US) (Track 02).bin",
    "Theron's Quest (Japan) (Track 02).iso",
    "Theron's Quest (US) (Track 02).iso",
    "TQJP02.iso",
    "TQJP02End.iso",
    "TQUS02.iso",
    "TQUS02End.iso",
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

/* Recognised Track 02 MD5s for Theron's Quest.  Mirrors the four hashes
 * in asset_status_m12.c::g_theronVersions. */
static const char *const g_theron_known_md5s[] = {
    "b7afb338ad31be1025b53f9aff12d73a", /* JP Track 02 BIN */
    "f23601102138f87c33025877767ebf76", /* US Track 02 BIN */
    "397039af02d50d15c70b74088eb8a1cb", /* JP Rev 1 ISO */
    "3d8b78571dcd0e6eb8eb4b01eeb7fbba", /* US ISO */
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
    } else if (strcmp(md5, "3d8b78571dcd0e6eb8eb4b01eeb7fbba") == 0) {
        strncpy(profile->version_id, "pce-en-iso",
                sizeof(profile->version_id) - 1);
    }
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
    int match_index = -1;

    if (!profile) {
        return -1;
    }

    /* Primary path: find Track 02 by MD5 anywhere under the data root.
     * THQUEST.ASM T400 cares about the verified data track bytes, not
     * whether the host file is named track02.bin. */
    if (asset_find_by_md5_list(base,
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
     * US: f23601102138f87c33025877767ebf76
     * JP Rev 1 ISO: 397039af02d50d15c70b74088eb8a1cb
     * US ISO:       3d8b78571dcd0e6eb8eb4b01eeb7fbba */
    if (profile->graphics_path[0]) {
        char md5hex[33] = {0};
        if (m12_file_md5_hex(profile->graphics_path, md5hex)) {
            if (strcmp(md5hex, "b7afb338ad31be1025b53f9aff12d73a") == 0 ||
                strcmp(md5hex, "f23601102138f87c33025877767ebf76") == 0 ||
                strcmp(md5hex, "397039af02d50d15c70b74088eb8a1cb") == 0 ||
                strcmp(md5hex, "3d8b78571dcd0e6eb8eb4b01eeb7fbba") == 0) {
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
    if (!track02_path || !track02_path[0]) return 1;
    if (!expected_md5 || !expected_md5[0]) return 1;
    if (!file_exists(track02_path)) {
        return 1;
    }
    /* Re-hash the file: same-size-different-content swaps pass an
     * mtime/size check but must not pass the verified-launch contract.
     * Using the same m12_file_md5_hex helper that the M12 reuse gate
     * uses keeps both sides byte-comparable.  Expected cost on a real
     * Track 02 BIN is one full MD5 over a few-MB payload; this is the
     * correct price for "reuse safely" rather than "reuse blindly". */
    if (!m12_file_md5_hex(track02_path, fresh_md5)) {
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
    const char *resolved_path = track02_path;
    const char *resolved_md5 = expected_md5;
    char parent[512];
    char grandparent[512];
    const char *base;

    if (!profile || !track02_path || !track02_path[0]) return -1;
    if (!expected_md5 || !expected_md5[0]) return -1;
    if (!theron_md5_is_known(expected_md5)) return -1;

    /* Stale-path guard: refuse if the previously verified file is no
     * longer on disk as a regular file.  Without this, the boot
     * profile would happily report assets_verified = 1 with a path
     * that points at nothing, and the downstream M11_Theron_Load
     * would fail at asset-load time with a less actionable error.
     * One stat() per direct-launch call; the rescan counter is the
     * way tests prove the rest of the function still skips the data
     * root.  Bumping the counter via file_exists() (rather than open-
     * coding stat()) keeps the count consistent with the scan path. */
    if (!file_exists(track02_path)) {
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
    base = track02_path;
    {
        size_t plen = strlen(track02_path);
        const char *slash;
        if (plen >= sizeof(parent)) plen = sizeof(parent) - 1;
        memcpy(parent, track02_path, plen);
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_runtime_state(
            &session,
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
    return theron_v1_startup_execute_input_from_session_with_host_receipt(
        &session,
        theron_v1_startup_input_from_firestaff_menu_code(input),
        out_receipt);
}

int theron_v1_boot_startup_execute_input_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    int input,
    Theron_StartupActionHostReceipt *out_receipt)
{
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_snapshot(&session, snapshot)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_startup_execute_input_from_session_with_host_receipt(
        &session,
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_runtime_state(
            &session,
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
    return theron_v1_startup_execute_pointer_from_session_with_host_receipt(
        &session,
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_snapshot(&session, snapshot)) {
        theron_v1_startup_action_host_receipt_init(out_receipt);
        return 0;
    }
    return theron_v1_startup_execute_pointer_from_session_with_host_receipt(
        &session,
        x,
        y,
        out_receipt);
}

int theron_v1_boot_startup_layout_build_from_runtime_state(
    Theron_StartupLayoutElement *elements,
    int max_elements,
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_runtime_state(
            &session,
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
        return 0;
    }
    return theron_v1_startup_layout_build_from_session(
        &session,
        elements,
        max_elements);
}

int theron_v1_boot_startup_layout_build_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    Theron_StartupLayoutElement *elements,
    int max_elements)
{
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_snapshot(&session, snapshot)) {
        return 0;
    }
    return theron_v1_startup_layout_build_from_session(
        &session,
        elements,
        max_elements);
}

int theron_v1_boot_startup_render_rows_from_runtime_state(
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows,
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_runtime_state(
            &session,
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
        return 0;
    }
    return theron_v1_startup_render_rows_build_from_session(
        &session,
        rows,
        max_rows);
}

int theron_v1_boot_startup_render_rows_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows)
{
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_snapshot(&session, snapshot)) {
        return 0;
    }
    return theron_v1_startup_render_rows_build_from_session(
        &session,
        rows,
        max_rows);
}

int theron_v1_boot_startup_render_plan_from_runtime_state(
    Theron_StartupRenderPlan *out_plan,
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
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_runtime_state(
            &session,
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
        return 0;
    }
    return theron_v1_startup_render_plan_build_from_session(
        &session,
        out_plan);
}

int theron_v1_boot_startup_render_plan_from_snapshot(
    const Theron_V1_BootRuntimeStartupSnapshot *snapshot,
    Theron_StartupRenderPlan *out_plan)
{
    Theron_StartupSessionFacts session;

    if (!theron_v1_boot_startup_session_from_snapshot(&session, snapshot)) {
        return 0;
    }
    return theron_v1_startup_render_plan_build_from_session(
        &session,
        out_plan);
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

void theron_v1_boot_startup_launch_cleanup(
    Theron_V1_BootStartupLaunch *launch) {
    if (!launch) {
        return;
    }
    if (launch->viewport) {
        theron_vp_free(launch->viewport);
        free(launch->viewport);
    }
    if (launch->assets) {
        tr_asset_free(launch->assets);
        free(launch->assets);
    }
    if (launch->world) {
        free(launch->world);
    }
    if (launch->profile) {
        theron_v1_boot_cleanup(launch->profile);
        free(launch->profile);
    }
    memset(launch, 0, sizeof(*launch));
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
