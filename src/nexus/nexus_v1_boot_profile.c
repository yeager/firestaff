/*
 * nexus_v1_boot_profile.c — Nexus V1 boot/runtime profile implementation
 *
 * Dungeon Master Nexus Sega Saturn — Phase 1
 * Source reference: ReDMCSB COMMAND.C F0359 ("LoadGameSettings");
 *   Greatstone DM Nexus map data; Sega Saturn developer docs
 *   Reference path: ~/.openclaw/data/firestaff-redmcsb-source/ReDMCSB_WIP20210206/
 *
 * This module provides:
 *   • Default boot profile for Nexus Sega Saturn runtime
 *   • Directory resolution (data/save/config roots)
 *   • Asset validation with Saturn-specific diagnostics
 *   • Feature flag queries
 *
 * Game ID:      "nexus1"
 * Platform:     Sega Saturn (SH-2 × 2 @ 28.6 MHz, 2 MB RAM)
 * Engine tick:  55 ms  (matches DM1/CSB world tick)
 * Render tick: 33 ms  (Saturn 30 fps 2D mode)
 */

#include "firestaff_nexus_v1_boot_profile.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Default runtime flags ──────────────────────────────────────────
 *
 * A clean Nexus Sega Saturn boot uses these flags by default.
 * Runtime flag documentation:
 *   • USE_SATURN_CD          — load from extracted Saturn CD
 *   • NO_3D_ENGINE          — Saturn 2D mode; rasterizer not used
 *   • EXTENDED_CHAMPION_SLOTS — 6 champion slots (vs DM1's 4)
 *   • NEXUS_CHAMPION_STATS   — Nexus-specific stat growth table
 *   • NO_MNEMONIC_RUNE       — rune book replaces scroll magic
 *   • NO_PARTY_SWAP         — party order fixed at dungeon entry
 *   • NO_KINGS_WISDOM       — no Kings Wisdom scroll equivalent
 *   • NO_GANGULF_REVIVAL    — Gangulf mechanic absent
 *   • RESTRICTED_DOOR_CLOSES — doors auto-close after fixed duration
 *   • SATURN_CDDA_AUDIO     — audio via CDDA track when available
 */
static const unsigned int g_defaultRuntimeFlags =
    NEXUS_V1_RF_USE_SATURN_CD
    | NEXUS_V1_RF_NO_3D_ENGINE
    | NEXUS_V1_RF_EXTENDED_CHAMPION_SLOTS
    | NEXUS_V1_RF_NEXUS_CHAMPION_STATS
    | NEXUS_V1_RF_NO_MNEMONIC_RUNE
    | NEXUS_V1_RF_NO_PARTY_SWAP
    | NEXUS_V1_RF_NO_KINGS_WISDOM
    | NEXUS_V1_RF_NO_GANGULF_REVIVAL
    | NEXUS_V1_RF_RESTRICTED_DOOR_CLOSES
    | NEXUS_V1_RF_SATURN_CDDA_AUDIO;

/* ── Default boot profile ───────────────────────────────────────── */
static const Nexus_V1_BootProfile g_defaultProfile = {
    /* runtimeFlags */  g_defaultRuntimeFlags,
    /* dataDir */       NULL,
    /* saveDir */       NULL,
    /* configPath */     NULL,
    /* presentationMode */ 0,  /* mapped to M12_PRESENTATION_V1_ORIGINAL */
    /* tickRateMs */     NEXUS_V1_TICK_RATE_MS,
    /* renderRateMs */  NEXUS_V1_RENDER_RATE_MS,
    /* unsupportedFeatureMask */ NX_UNSUPPFEAT_3D_RASTERIZER
                                         | NX_UNSUPPFEAT_SMOOTH_MOVEMENT
                                         | NX_UNSUPPFEAT_SPELL_BOOK_UI
                                         | NX_UNSUPPFEAT_PARTY_REORDER
                                         | NX_UNSUPPFEAT_GANGULF_REvival
                                         | NX_UNSUPPFEAT_KINGS_WISDOM
                                         | NX_UNSUPPFEAT_CROSS_IMPORT
                                         | NX_UNSUPPFEAT_V2_MODES,
};

/* ── Path buffers ───────────────────────────────────────────────── */
static char g_resolvedDataDir[512];
static char g_resolvedSaveDir[512];
static char g_resolvedConfigPath[512];
static int  g_pathsInitialized = 0;

/* ── Static diagnostic strings ──────────────────────────────────── */
static const char* const g_diagStrings[NEXUS_V1_DIAG_COUNT] = {
    [NEXUS_V1_DIAG_OK]                       = "OK",
    [NEXUS_V1_DIAG_MISSING_DM_BIN]           = "DM.BIN not found",
    [NEXUS_V1_DIAG_MISSING_DMDF_ARCHIVE]     = "DMDF archive not found",
    [NEXUS_V1_DIAG_MISSING_DGN_LEVEL]         = "DGN level file not found",
    [NEXUS_V1_DIAG_MISSING_CHAMPION_DATA]     = "Champion data not found",
    [NEXUS_V1_DIAG_INVALID_DM_BIN_MAGIC]     = "DM.BIN has invalid magic header",
    [NEXUS_V1_DIAG_INVALID_DMDF]             = "DMDF archive is invalid or corrupt",
    [NEXUS_V1_DIAG_CORRUPT_SATURN_CD_IMAGE]   = "Saturn CD image appears corrupt",
    [NEXUS_V1_DIAG_NO_CDDA_TRACK]             = "CDDA audio track not found in image",
    [NEXUS_V1_DIAG_OUT_OF_MEMORY]             = "Saturn emulation out of memory",
    [NEXUS_V1_DIAG_SAVE_WRITE_FAILED]         = "Save write failed",
};

/* ── Forward declarations ──────────────────────────────────────── */
static int  nexus_v1_boot_resolve_paths(Nexus_V1_BootProfile *profile,
                                        const char *baseDataDir,
                                        const char *baseSaveDir);
static int  nexus_v1_boot_check_asset_file(const char *dir,
                                            const char *filename,
                                            Nexus_V1_DiagnosticCode okCode,
                                            Nexus_V1_Diagnostic *diags,
                                            size_t *outIndex);

/* ── Public API ────────────────────────────────────────────────── */

const Nexus_V1_BootProfile*
Nexus_V1_BootProfile_GetDefault(void) {
    return &g_defaultProfile;
}

int Nexus_V1_BootProfile_Init(Nexus_V1_BootProfile *profile,
                               const char *baseDataDir,
                               const char *baseSaveDir,
                               unsigned int runtimeFlags) {
    if (!profile) {
        return -1;
    }

    /* Seed from defaults */
    *profile = g_defaultProfile;

    /* Override flags if supplied (0 = use defaults) */
    if (runtimeFlags != 0U) {
        profile->runtimeFlags = runtimeFlags;
    }

    /* Resolve and store directory paths */
    if (nexus_v1_boot_resolve_paths(profile, baseDataDir, baseSaveDir) != 0) {
        return -1;
    }

    return 0;
}

int Nexus_V1_BootProfile_ValidateAssets(const Nexus_V1_BootProfile *profile,
                                          Nexus_V1_Diagnostic *diags,
                                          size_t maxDiags) {
    size_t diagCount = 0;
    const char *dataDir;
    const char *sep = "/";

    if (!profile || !diags || maxDiags == 0) {
        return 0;
    }

    dataDir = Nexus_V1_BootProfile_GetAssetRoot(profile);
    if (!dataDir) {
        diags[diagCount].code = NEXUS_V1_DIAG_MISSING_DM_BIN;
        snprintf(diags[diagCount].message, sizeof(diags[0].message),
                 "%s", g_diagStrings[NEXUS_V1_DIAG_MISSING_DM_BIN]);
        snprintf(diags[diagCount].detail, sizeof(diags[0].detail),
                 "Could not resolve data directory for nexus1 assets.");
        snprintf(diags[diagCount].suggestion, sizeof(diags[0].suggestion),
                 "Set --data-dir or FIRESTAFF_DATA environment variable.");
        return 1;
    }

    /* Check for DM.BIN (primary Saturn CD image / archive marker) */
    if (nexus_v1_boot_check_asset_file(dataDir, "DM.BIN",
                                      NEXUS_V1_DIAG_OK, diags, &diagCount) != 0
        && diagCount < maxDiags) {
        /* Missing DM.BIN is non-fatal if SEGADATA.BIN exists */
        (void)nexus_v1_boot_check_asset_file(dataDir, "SEGADATA.BIN",
                                            NEXUS_V1_DIAG_OK, diags, &diagCount);
    }

    /* Check for DMDF archives — required for wall/floor/object sprites */
    if (nexus_v1_boot_check_asset_file(dataDir, "WALLS.DMDF",
                                      NEXUS_V1_DIAG_OK, diags, &diagCount) != 0
        && diagCount < maxDiags) {
        (void)nexus_v1_boot_check_asset_file(dataDir, "FLOORS.DMDF",
                                            NEXUS_V1_DIAG_OK, diags, &diagCount);
        (void)nexus_v1_boot_check_asset_file(dataDir, "OBJECTS.DMDF",
                                            NEXUS_V1_DIAG_OK, diags, &diagCount);
        (void)nexus_v1_boot_check_asset_file(dataDir, "CHARS.DMDF",
                                            NEXUS_V1_DIAG_OK, diags, &diagCount);
    }

    /* Check for DGN level directory — at least one level must exist */
    {
        char dgnPath[512];
        snprintf(dgnPath, sizeof(dgnPath), "%s%sLEVEL1.DGN", dataDir, sep);
        FILE *f = fopen(dgnPath, "rb");
        if (!f && diagCount < maxDiags) {
            diags[diagCount].code = NEXUS_V1_DIAG_MISSING_DGN_LEVEL;
            snprintf(diags[diagCount].message, sizeof(diags[0].message),
                     "%s", g_diagStrings[NEXUS_V1_DIAG_MISSING_DGN_LEVEL]);
            snprintf(diags[diagCount].detail, sizeof(diags[0].detail),
                     "Level file not found: %s", dgnPath);
            snprintf(diags[diagCount].suggestion, sizeof(diags[0].suggestion),
                     "Ensure Nexus CD image extracted to data directory.");
            diagCount++;
        } else if (f) {
            fclose(f);
        }
    }

    /* Check for champion data directory */
    {
        char champPath[512];
        snprintf(champPath, sizeof(champPath), "%s%sCHAMPIONS.DAT", dataDir, sep);
        FILE *f = fopen(champPath, "rb");
        if (!f && diagCount < maxDiags) {
            diags[diagCount].code = NEXUS_V1_DIAG_MISSING_CHAMPION_DATA;
            snprintf(diags[diagCount].message, sizeof(diags[0].message),
                     "%s", g_diagStrings[NEXUS_V1_DIAG_MISSING_CHAMPION_DATA]);
            snprintf(diags[diagCount].detail, sizeof(diags[0].detail),
                     "Champion data file not found: %s", champPath);
            snprintf(diags[diagCount].suggestion, sizeof(diags[0].suggestion),
                     "Verify Nexus CD image extraction includes champion data.");
            diagCount++;
        } else if (f) {
            fclose(f);
        }
    }

    return (int)diagCount;
}

const char* Nexus_V1_BootProfile_GetAssetRoot(const Nexus_V1_BootProfile *profile) {
    if (!profile || !profile->dataDir) {
        return NULL;
    }
    return profile->dataDir;
}

const char* Nexus_V1_BootProfile_GetSaveRoot(const Nexus_V1_BootProfile *profile) {
    const char *saveDir;
    if (!profile || !profile->saveDir) {
        return NULL;
    }
    saveDir = profile->saveDir;

    /* Ensure directory exists */
    {
        char cmd[1024];
        int rc = snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", saveDir);
        if (rc > 0 && rc < (int)sizeof(cmd)) {
            (void)system(cmd);
        }
    }

    return saveDir;
}

unsigned int Nexus_V1_BootProfile_SupportedFeatures(const Nexus_V1_BootProfile *profile) {
    /*
     * Any feature NOT listed in unsupportedFeatureMask is considered
     * supported in this Nexus V1 runtime profile. The full feature
     * set is the logical inverse of the mask.
     */
    if (!profile) {
        return 0U;
    }
    return ~profile->unsupportedFeatureMask;
}

const char* Nexus_V1_BootProfile_GetDiagnosticString(Nexus_V1_DiagnosticCode code) {
    if (code < 0 || code >= NEXUS_V1_DIAG_COUNT) {
        return "UNKNOWN";
    }
    return g_diagStrings[code];
}

/* ── Internal helpers ───────────────────────────────────────────── */

static int nexus_v1_boot_resolve_paths(Nexus_V1_BootProfile *profile,
                                        const char *baseDataDir,
                                        const char *baseSaveDir) {
    const char *dataDir  = baseDataDir  ? baseDataDir  : ".";
    const char *saveDir  = baseSaveDir  ? baseSaveDir  : ".";

    g_pathsInitialized = 0;

    /* Resolve data directory: baseDataDir/nexus1/ or baseDataDir/ */
    {
        size_t len = strlen(dataDir);
        int needsSep = (len == 0) || (dataDir[len - 1] != '/');
        if (needsSep) {
            snprintf(g_resolvedDataDir, sizeof(g_resolvedDataDir),
                     "%s/%s", dataDir, "nexus1");
        } else {
            snprintf(g_resolvedDataDir, sizeof(g_resolvedDataDir),
                     "%s%s", dataDir, "nexus1");
        }
        profile->dataDir = g_resolvedDataDir;
    }

    /* Resolve save directory: baseSaveDir/saves/nexus1/ */
    {
        snprintf(g_resolvedSaveDir, sizeof(g_resolvedSaveDir),
                 "%s/%s", saveDir, "saves/nexus1");
        profile->saveDir = g_resolvedSaveDir;
    }

    /* Resolve config path: baseSaveDir/nexus1/firestaff.ini */
    {
        snprintf(g_resolvedConfigPath, sizeof(g_resolvedConfigPath),
                 "%s/%s/firestaff.ini", saveDir, "nexus1");
        profile->configPath = g_resolvedConfigPath;
    }

    g_pathsInitialized = 1;
    return 0;
}

static int nexus_v1_boot_check_asset_file(const char *dir,
                                            const char *filename,
                                            Nexus_V1_DiagnosticCode okCode,
                                            Nexus_V1_Diagnostic *diags,
                                            size_t *outIndex) {
    char path[512];
    Nexus_V1_DiagnosticCode diagCode;
    int missing = 0;

    if (!dir || !filename || !diags || !outIndex) {
        return -1;
    }

    snprintf(path, sizeof(path), "%s/%s", dir, filename);

    {
        FILE *f = fopen(path, "rb");
        if (!f) {
            missing = 1;
            diagCode = NEXUS_V1_DIAG_MISSING_DM_BIN;  /* reuse sentinel */
        } else {
            fclose(f);
        }
    }

    if (missing) {
        if (*outIndex < (size_t)NEXUS_V1_DIAG_OK) {
            return -1;
        }
        if (diagCode != okCode) {
            diags[*outIndex].code = diagCode;
            snprintf(diags[*outIndex].message, sizeof(diags[*outIndex].message),
                     "Asset not found: %s", filename);
            snprintf(diags[*outIndex].detail, sizeof(diags[*outIndex].detail),
                     "Tried path: %s", path);
            snprintf(diags[*outIndex].suggestion, sizeof(diags[*outIndex].suggestion),
                     "Verify the %s file is present in the Nexus data directory.", filename);
            (*outIndex)++;
        }
        return 1;
    }

    return 0;  /* file found — no diagnostic needed */
}
