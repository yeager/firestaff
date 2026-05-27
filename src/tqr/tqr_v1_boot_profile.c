/*
 * tqr_v1_boot_profile.c — Theron's Quest V1 boot/runtime profile implementation
 *
 * Theron's Quest — Hudson Soft / Namuland (PC Engine/TurboGrafx-16 CD)
 * Phase 1: Separate Theron's Quest boot/runtime profile from DM1/CSB/DM2/Nexus.
 *
 * Reference: http://dmweb.free.fr/games/therons-quest/
 * Provenance gate: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *   JP MD5: b7afb338ad31be1025b53f9aff12d73a
 *   US MD5: f23601102138f87c33025877767ebf76
 *
 * This module provides:
 *   • Default boot profile for Theron's Quest PC Engine runtime
 *   • Directory resolution (data/save/config roots)
 *   • Asset validation with PC Engine-specific diagnostics
 *   • Feature flag queries
 *   • In-dungeon save enforcement
 *
 * Game ID:      "theron"
 * Platform:     PC Engine / TurboGrafx-16 (HuCard or CD)
 * Engine tick:  55 ms  (matches DM1/CSB world tick)
 * Render tick: ~16 ms  (PC Engine VDC ~60 fps)
 *
 * Key differences from DM1/CSB:
 *   • "Light" version — subset of DM1 items, creatures, spells
 *   • 7 mini-dungeons
 *   • Theron persistent across dungeons; companions reset per dungeon
 *   • No in-dungeon saves — only between-dungeons saves allowed
 *   • No Kings Wisdom, no Gangulf revival
 *   • PC Engine planar graphics (512-color palette, 16-color per tile)
 */

#include "firestaff_tqr_v1_boot_profile.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Default runtime flags ───────────────────────────────────────────
 *
 * A clean Theron's Quest boot uses these flags by default.
 * Runtime flag documentation:
 *   • USE_HUCARD              — load from HuCard ROM image (primary format)
 *   • USE_CD_IMAGE            — load from PC Engine CD image (CD version)
 *   • NO_IN_DUNGEON_SAVE      — saves only between dungeons
 *   • THERON_PERSISTENT       — Theron keeps stats across dungeons
 *   • LIGHT_ITEM_SET          — TQR subset of DM1 items
 *   • LIGHT_CREATURE_SET      — TQR subset of DM1 creatures
 *   • NO_KINGS_WISDOM         — absent in TQR
 *   • NO_GANGULF_REVIVAL      — absent in TQR
 *   • NO_PARTY_SWAP           — party order fixed at dungeon entry
 *   • NOMINAL_SCROLL_SYSTEM   — DM1-style scrolls (reduced set)
 */
static const unsigned int g_defaultRuntimeFlags =
    TQR_V1_RF_USE_HUCARD
    | TQR_V1_RF_NO_IN_DUNGEON_SAVE
    | TQR_V1_RF_THERON_PERSISTENT
    | TQR_V1_RF_LIGHT_ITEM_SET
    | TQR_V1_RF_LIGHT_CREATURE_SET
    | TQR_V1_RF_NO_KINGS_WISDOM
    | TQR_V1_RF_NO_GANGULF_REVIVAL
    | TQR_V1_RF_NO_PARTY_SWAP
    | TQR_V1_RF_NOMINAL_SCROLL_SYSTEM;

/* ── Default boot profile ─────────────────────────────────────────── */
static const TQR_V1_BootProfile g_defaultProfile = {
    /* runtimeFlags       */  g_defaultRuntimeFlags,
    /* dataDir            */  NULL,
    /* saveDir            */  NULL,
    /* configPath         */  NULL,
    /* presentationMode   */  0,  /* mapped to M12_PRESENTATION_V1_ORIGINAL */
    /* tickRateMs         */  TQR_V1_TICK_RATE_MS,
    /* renderRateMs       */  TQR_V1_RENDER_RATE_MS,
    /* unsupportedFeatureMask */ TQR_UNSUPPFEAT_IN_DUNGEON_SAVE
                                  | TQR_UNSUPPFEAT_KINGS_WISDOM
                                  | TQR_UNSUPPFEAT_GANGULF_REVIVAL
                                  | TQR_UNSUPPFEAT_PARTY_REORDER
                                  | TQR_UNSUPPFEAT_CROSS_IMPORT
                                  | TQR_UNSUPPFEAT_V2_MODES
                                  | TQR_UNSUPPFEAT_FULL_ITEM_SET
                                  | TQR_UNSUPPFEAT_FULL_CREATURE_SET
                                  | TQR_UNSUPPFEAT_FULL_SPELL_ROSTER
                                  | TQR_UNSUPPFEAT_SMOOTH_MOVEMENT,
    /* dungeonIndex       */  -1,
    /* inDungeon          */  0,
};

/* ── Path buffers ─────────────────────────────────────────────────── */
static char g_resolvedDataDir[512];
static char g_resolvedSaveDir[512];
static char g_resolvedConfigPath[512];
static int  g_pathsInitialized = 0;

/* ── Static diagnostic strings ──────────────────────────────────── */
static const char* const g_diagStrings[TQR_V1_DIAG_COUNT] = {
    [TQR_V1_DIAG_OK]                        = "OK",
    [TQR_V1_DIAG_MISSING_HUCARD_IMAGE]     = "HuCard image not found",
    [TQR_V1_DIAG_MISSING_CD_IMAGE]         = "CD image not found",
    [TQR_V1_DIAG_MISSING_DUNGEON_DATA]    = "Dungeon data not found",
    [TQR_V1_DIAG_MISSING_CHAMPION_DATA]    = "Champion data not found",
    [TQR_V1_DIAG_INVALID_HUCARD_HEADER]    = "HuCard image has invalid header",
    [TQR_V1_DIAG_INVALID_CD_IMAGE]         = "CD image is invalid or corrupt",
    [TQR_V1_DIAG_CORRUPT_DUNGEON_FILE]     = "Dungeon file appears corrupt",
    [TQR_V1_DIAG_NO_CDDA_TRACK]            = "CDDA audio track not found in image",
    [TQR_V1_DIAG_IN_DUNGEON_SAVE_ATTEMPTED]= "In-dungeon saves not supported",
    [TQR_V1_DIAG_OUT_OF_MEMORY]            = "PC Engine emulation out of memory",
    [TQR_V1_DIAG_SAVE_WRITE_FAILED]        = "Save write failed",
    [TQR_V1_DIAG_INCOMPATIBLE_VERSION]     = "Incompatible data version",
};

/* ── Forward declarations ──────────────────────────────────────── */
static int  tqr_v1_boot_resolve_paths(TQR_V1_BootProfile *profile,
                                        const char *baseDataDir,
                                        const char *baseSaveDir);
static int  tqr_v1_boot_check_asset_file(const char *dir,
                                          const char *filename,
                                          TQR_V1_Diagnostic *diags,
                                          size_t *outIndex);

/* ── Public API ─────────────────────────────────────────────────── */

const TQR_V1_BootProfile*
TQR_V1_BootProfile_GetDefault(void) {
    return &g_defaultProfile;
}

int TQR_V1_BootProfile_Init(TQR_V1_BootProfile *profile,
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
    if (tqr_v1_boot_resolve_paths(profile, baseDataDir, baseSaveDir) != 0) {
        return -1;
    }

    return 0;
}

int TQR_V1_BootProfile_ValidateAssets(const TQR_V1_BootProfile *profile,
                                        TQR_V1_Diagnostic *diags,
                                        size_t maxDiags) {
    size_t diagCount = 0;
    const char *dataDir;
    int useHucard = 0;
    int useCD = 0;

    if (!profile || !diags || maxDiags == 0) {
        return 0;
    }

    dataDir = TQR_V1_BootProfile_GetAssetRoot(profile);
    if (!dataDir) {
        diags[diagCount].code = TQR_V1_DIAG_MISSING_HUCARD_IMAGE;
        snprintf(diags[diagCount].message, sizeof(diags[0].message),
                 "%s", g_diagStrings[TQR_V1_DIAG_MISSING_HUCARD_IMAGE]);
        snprintf(diags[diagCount].detail, sizeof(diags[0].detail),
                 "Could not resolve data directory for theron assets.");
        snprintf(diags[diagCount].suggestion, sizeof(diags[0].suggestion),
                 "Set --data-dir or FIRESTAFF_DATA environment variable.");
        return 1;
    }

    /* Determine which media type the profile expects */
    useHucard = !!(profile->runtimeFlags & TQR_V1_RF_USE_HUCARD);
    useCD     = !!(profile->runtimeFlags & TQR_V1_RF_USE_CD_IMAGE);

    if (useHucard) {
        /* Check for HuCard ROM image */
        if (tqr_v1_boot_check_asset_file(dataDir, "THERON.HUC",
                                        diags, &diagCount) != 0
            && diagCount < maxDiags) {
            (void)tqr_v1_boot_check_asset_file(dataDir, "GAME.DAT",
                                               diags, &diagCount);
        }
    }

    if (useCD) {
        /* Check for CD image */
        if (tqr_v1_boot_check_asset_file(dataDir, "theron.iso",
                                        diags, &diagCount) != 0
            && diagCount < maxDiags) {
            (void)tqr_v1_boot_check_asset_file(dataDir, "theron.bin",
                                               diags, &diagCount);
        }
    }

    /* If neither flag set, check both as fallback */
    if (!useHucard && !useCD) {
        if (tqr_v1_boot_check_asset_file(dataDir, "THERON.HUC",
                                        diags, &diagCount) != 0
            && diagCount < maxDiags) {
            (void)tqr_v1_boot_check_asset_file(dataDir, "GAME.DAT",
                                               diags, &diagCount);
        }
        if (diagCount == 0) {
            /* No HuCard files found, check CD */
            if (tqr_v1_boot_check_asset_file(dataDir, "theron.iso",
                                            diags, &diagCount) != 0
                && diagCount < maxDiags) {
                (void)tqr_v1_boot_check_asset_file(dataDir, "theron.bin",
                                                   diags, &diagCount);
            }
        }
    }

    /* Check for dungeon data directory — at least one dungeon must exist */
    {
        char dungeonPath[512];
        /* TQR has 7 mini-dungeons named DUNGEON0.DAT through DUNGEON6.DAT */
        snprintf(dungeonPath, sizeof(dungeonPath),
                 "%s/DUNGEON0.DAT", dataDir);
        {
            FILE *f = fopen(dungeonPath, "rb");
            if (!f && diagCount < maxDiags) {
                diags[diagCount].code = TQR_V1_DIAG_MISSING_DUNGEON_DATA;
                snprintf(diags[diagCount].message,
                         sizeof(diags[0].message),
                         "%s", g_diagStrings[TQR_V1_DIAG_MISSING_DUNGEON_DATA]);
                snprintf(diags[diagCount].detail,
                         sizeof(diags[0].detail),
                         "Dungeon data file not found: %s", dungeonPath);
                snprintf(diags[diagCount].suggestion,
                         sizeof(diags[0].suggestion),
                         "Ensure Theron's Quest HuCard/CD image is extracted "
                         "to the theron/ data directory.");
                diagCount++;
            } else if (f) {
                fclose(f);
            }
        }
    }

    /* Check for champion data (Theron + up to 3 companions) */
    {
        char champPath[512];
        snprintf(champPath, sizeof(champPath),
                 "%s/CHAMPIONS.DAT", dataDir);
        {
            FILE *f = fopen(champPath, "rb");
            if (!f && diagCount < maxDiags) {
                diags[diagCount].code = TQR_V1_DIAG_MISSING_CHAMPION_DATA;
                snprintf(diags[diagCount].message,
                         sizeof(diags[0].message),
                         "%s", g_diagStrings[TQR_V1_DIAG_MISSING_CHAMPION_DATA]);
                snprintf(diags[diagCount].detail,
                         sizeof(diags[0].detail),
                         "Champion data file not found: %s", champPath);
                snprintf(diags[diagCount].suggestion,
                         sizeof(diags[0].suggestion),
                         "Verify Theron's Quest data includes champion records.");
                diagCount++;
            } else if (f) {
                fclose(f);
            }
        }
    }

    return (int)diagCount;
}

const char* TQR_V1_BootProfile_GetAssetRoot(const TQR_V1_BootProfile *profile) {
    if (!profile || !profile->dataDir) {
        return NULL;
    }
    return profile->dataDir;
}

const char* TQR_V1_BootProfile_GetSaveRoot(const TQR_V1_BootProfile *profile) {
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

unsigned int TQR_V1_BootProfile_SupportedFeatures(const TQR_V1_BootProfile *profile) {
    /*
     * Any feature NOT listed in unsupportedFeatureMask is considered
     * supported in this TQR V1 runtime profile.
     */
    if (!profile) {
        return 0U;
    }
    return ~profile->unsupportedFeatureMask;
}

const char* TQR_V1_BootProfile_GetDiagnosticString(TQR_V1_DiagnosticCode code) {
    if (code < 0 || code >= TQR_V1_DIAG_COUNT) {
        return "UNKNOWN";
    }
    return g_diagStrings[code];
}

int TQR_V1_BootProfile_CanSave(const TQR_V1_BootProfile *profile) {
    /*
     * Theron's Quest does NOT support in-dungeon saves.
     * Saves are only valid at the overworld / between-dungeon screen.
     * Enforce TQR_V1_RF_NO_IN_DUNGEON_SAVE.
     */
    if (!profile) {
        return 0;
    }

    /* If party is inside a dungeon, saving is prohibited */
    if (profile->inDungeon) {
        return 0;
    }

    /* If the flag is set, always return 0 when inDungeon is set */
    if (profile->runtimeFlags & TQR_V1_RF_NO_IN_DUNGEON_SAVE) {
        /* Only allow if explicitly at overworld (inDungeon == 0) */
        return profile->inDungeon ? 0 : 1;
    }

    /* Flag not set — allow save (be permissive if flag not set yet) */
    return 1;
}

/* ── Internal helpers ─────────────────────────────────────────────── */

static int tqr_v1_boot_resolve_paths(TQR_V1_BootProfile *profile,
                                        const char *baseDataDir,
                                        const char *baseSaveDir) {
    const char *dataDir  = baseDataDir  ? baseDataDir  : ".";
    const char *saveDir  = baseSaveDir  ? baseSaveDir  : ".";

    g_pathsInitialized = 0;

    /* Resolve data directory: baseDataDir/theron/ or baseDataDir/ */
    {
        size_t len = strlen(dataDir);
        int needsSep = (len == 0) || (dataDir[len - 1] != '/');
        if (needsSep) {
            snprintf(g_resolvedDataDir, sizeof(g_resolvedDataDir),
                     "%s/%s", dataDir, "theron");
        } else {
            snprintf(g_resolvedDataDir, sizeof(g_resolvedDataDir),
                     "%s%s", dataDir, "theron");
        }
        profile->dataDir = g_resolvedDataDir;
    }

    /* Resolve save directory: baseSaveDir/saves/theron/ */
    {
        snprintf(g_resolvedSaveDir, sizeof(g_resolvedSaveDir),
                 "%s/%s", saveDir, "saves/theron");
        profile->saveDir = g_resolvedSaveDir;
    }

    /* Resolve config path: baseSaveDir/theron/firestaff.ini */
    {
        snprintf(g_resolvedConfigPath, sizeof(g_resolvedConfigPath),
                 "%s/%s/firestaff.ini", saveDir, "theron");
        profile->configPath = g_resolvedConfigPath;
    }

    g_pathsInitialized = 1;
    return 0;
}

static int tqr_v1_boot_check_asset_file(const char *dir,
                                          const char *filename,
                                          TQR_V1_Diagnostic *diags,
                                          size_t *outIndex) {
    char path[512];
    TQR_V1_DiagnosticCode diagCode = TQR_V1_DIAG_MISSING_HUCARD_IMAGE;
    int missing = 0;

    if (!dir || !filename || !diags || !outIndex) {
        return -1;
    }

    snprintf(path, sizeof(path), "%s/%s", dir, filename);

    {
        FILE *f = fopen(path, "rb");
        if (!f) {
            missing = 1;
        } else {
            fclose(f);
        }
    }

    if (missing) {
        if (*outIndex < (size_t)TQR_V1_DIAG_COUNT) {
            diags[*outIndex].code = diagCode;
            snprintf(diags[*outIndex].message,
                     sizeof(diags[*outIndex].message),
                     "Asset not found: %s", filename);
            snprintf(diags[*outIndex].detail,
                     sizeof(diags[*outIndex].detail),
                     "Tried path: %s", path);
            snprintf(diags[*outIndex].suggestion,
                     sizeof(diags[*outIndex].suggestion),
                     "Verify the %s file is present in the Theron data directory.",
                     filename);
            (*outIndex)++;
        }
        return 1;
    }

    return 0;  /* file found — no diagnostic needed */
}