#ifndef FIRESTAFF_NEXUS_V1_BOOT_PROFILE_H
#define FIRESTAFF_NEXUS_V1_BOOT_PROFILE_H
/*
 * nexus_v1_boot_profile.h — Nexus V1 boot/runtime profile
 *
 * Dungeon Master Nexus Sega Saturn — Phase 1
 * Boot profile: separates Nexus runtime concerns from DM1/CSB/DM2.
 *
 * Scope:
 *   • Menu launch entry and availability gating
 *   • Asset roots (Saturn CD extraction target directories)
 *   • Save namespace per game
 *   • Platform diagnostics (Saturn hardware quirks)
 *   • Deterministic config defaults
 *   • Unsupported-feature messaging
 *
 * Reference: ReDMCSB COMMAND.C / ENTRANCE.C; Greatstone DM Nexus map data;
 *            Sega Saturn developer documentation
 *
 * Game ID for this profile: "nexus"
 * Saturn assets live under: FIRESTAFF_DATA/nexus/ or
 *                           FIRESTAFF_DATA/<saturn-cd-hash>/
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game identifier ─────────────────────────────────────────────── */
#define NEXUS_V1_GAME_ID          "nexus"
#define NEXUS_V1_GAME_LABEL       "DUNGEON MASTER NEXUS"
#define NEXUS_V1_PLATFORM_LABEL   "SEGA SATURN"

/* ── Config directory root ─────────────────────────────────────────── */
#define NEXUS_V1_CONFIG_DIR       "firestaff/nexus"
#define NEXUS_V1_SAVE_SUB_DIR     "saves/nexus"
#define NEXUS_V1_USER_DATA_SUB_DIR "userdata/nexus"

/* ── Deterministic tick rate for V1 engine ─────────────────────────── */
/*
 * Unlike DM1/CSB which run at 18.2 Hz (55 ms/tick), Nexus on Saturn
 * uses a fixed frame pacing tied to the Saturn's 30 fps refresh when
 * in 2D mode, but the V1 game logic ticks at a derived rate.
 * Deterministic config: use the same 55 ms tick base but with
 * Saturn CD loading latency compensated by pre-cached sector reads.
 */
#define NEXUS_V1_TICK_RATE_MS     55   /* identical to DM1/CSB — world tick */
#define NEXUS_V1_RENDER_RATE_MS   33   /* Saturn 30 fps — render tick */

/* ── Boot profile runtime config flags ─────────────────────────────── */
typedef enum {
    /*
     * NEXUS_V1_RF_USE_SATURN_CD:
     *   Load dungeon assets directly from extracted Saturn CD image
     *   rather than pre-built .bin files. Must verify DM.BIN header
     *   magic and checksum before loading.
     */
    NEXUS_V1_RF_USE_SATURN_CD          = (1 << 0),

    /*
     * NEXUS_V1_RF_NO_3D_ENGINE:
     *   Nexus on Saturn 2D mode: 3D rasterizer is not available or
     *   not performance-viable. All rendering uses pre rasterized
     *   2D sprite frames loaded from DMDF archives.
     */
    NEXUS_V1_RF_NO_3D_ENGINE           = (1 << 1),

    /*
     * NEXUS_V1_RF_STANDARD_CHAMPION_LIMIT:
     *   Nexus supports up to 6 champions (vs DM1's 4). When set, the
     *   champion roster UI and party management allow 6 slots.
     */
    NEXUS_V1_RF_EXTENDED_CHAMPION_SLOTS = (1 << 2),

    /*
     * NEXUS_V1_RF_NEXUS_CHAMPION_STATS:
     *   Champion stat growth formulas differ from DM1. Nexus uses
     *   a different LP/spellpont gain table (see nexus_v1_champions.c
     *   F0128 equivalent). Enable Nexus stat rules.
     */
    NEXUS_V1_RF_NEXUS_CHAMPION_STATS    = (1 << 3),

    /*
     * NEXUS_V1_RF_NO_MNEMONIC_RUNE:
     *   The Nexus magic system replaces Scrolls with a rune-magic
     *   book system. When this flag is set the rune book UI is
     *   active and the standard DM1 scroll system is unavailable.
     */
    NEXUS_V1_RF_NO_MNEMONIC_RUNE        = (1 << 4),

    /*
     * NEXUS_V1_RF_NO_PARTY_SWAP:
     *   Nexus does not support in-dungeon party reordering via the
     *   F-keys used in DM1. Party order is fixed at dungeon entry.
     */
    NEXUS_V1_RF_NO_PARTY_SWAP          = (1 << 5),

    /*
     * NEXUS_V1_RF_NO_KINGS_WISDOM:
     *   The Dungeon Master's Wiseman "Kings Wisdom" effect (revicify
     *   all champions) rune text differs in Nexus. Disables the KW
     *   scroll equivalent.
     */
    NEXUS_V1_RF_NO_KINGS_WISDOM        = (1 << 6),

    /*
     * NEXUS_V1_RF_NO_GANGULF_REVIVAL:
     *   Gangulf revival mechanic (DM1 F0028 equivalent) is absent
     *   in Nexus; use standard Phoenix Down / temple recover flow.
     */
    NEXUS_V1_RF_NO_GANGULF_REVIVAL      = (1 << 7),

    /*
     * NEXUS_V1_RF_RESTRICTED_DOOR_CLOSES:
     *   In Nexus, doors opened by actuators stay open for a fixed
     *   duration before auto-closing (vs DM1's stay-open). Emulates
     *   Saturn timing.
     */
    NEXUS_V1_RF_RESTRICTED_DOOR_CLOSES  = (1 << 8),

    /*
     * NEXUS_V1_RF_SATURN_CDDA_AUDIO:
     *   Audio playback routes through Saturn CDDA (audio track) when
     *   available, falling back to ADX/BRAM audio otherwise.
     */
    NEXUS_V1_RF_SATURN_CDDA_AUDIO       = (1 << 9),
} Nexus_V1_RuntimeFlags;

/* ── Boot profile struct ──────────────────────────────────────────── */
typedef struct {
    /*
     * runtimeFlags:
     *   Bitwise OR of NEXUS_V1_RF_* flags. Default for a clean
     *   Nexus game start is NEXUS_V1_RF_USE_SATURN_CD |
     *   NEXUS_V1_RF_NO_3D_ENGINE |
     *   NEXUS_V1_RF_EXTENDED_CHAMPION_SLOTS |
     *   NEXUS_V1_RF_NEXUS_CHAMPION_STATS |
     *   NEXUS_V1_RF_NO_MNEMONIC_RUNE |
     *   NEXUS_V1_RF_NO_PARTY_SWAP |
     *   NEXUS_V1_RF_NO_KINGS_WISDOM |
     *   NEXUS_V1_RF_NO_GANGULF_REVIVAL |
     *   NEXUS_V1_RF_RESTRICTED_DOOR_CLOSES |
     *   NEXUS_V1_RF_SATURN_CDDA_AUDIO
     */
    unsigned int runtimeFlags;

    /* dataDir:
     *   Root path for nexus game assets. When USE_SATURN_CD is
     *   active, this directory must contain a DM.BIN or a
     *   SATURNDAT/ directory with extracted CD image contents.
     *   Defaults to $FIRESTAFF_DATA/nexus/ or ~/.firestaff/data/nexus/
     */
    const char *dataDir;

    /* saveDir:
     *   Directory for Nexus savegames. Saved to
     *   $saveDir/nexus/slot_N.fssv
     *   Defaults to $FIRESTAFF_SAVES/nexus/
     */
    const char *saveDir;

    /* configPath:
     *   Per-game config file for Nexus (graphics mode, audio settings,
     *   Saturn-specific display output). Path is game-namespaced.
     *   Defaults to $FIRESTAFF_CONFIG/nexus/firestaff.ini
     */
    const char *configPath;

    /* presentationMode:
     *   Graphics presentation mode. Index into M12 presentation mode
     *   table; M12_PRESENTATION_V1_ORIGINAL is the only fully supported
     *   mode in Phase 1. V2.0/V2.1/V2.2 render paths are not yet
     *   available for Nexus.
     */
    int presentationMode;

    /* tickRateMs / renderRateMs:
     *   Override tick rates from defaults if the underlying Saturn
     *   emulation or real hardware needs pacing adjustments.
     *   Set to 0 to use built-in defaults (55 ms / 33 ms).
     */
    unsigned int tickRateMs;
    unsigned int renderRateMs;

    /* unsupportedFeatureMask:
     *   Features that are unsupported and should be greyed out or
     *   replaced with explanatory messaging in the UI.
     *   Bitwise OR of NX_UNSUPPFEAT_* feature flags (see below).
     */
    unsigned int unsupportedFeatureMask;
} Nexus_V1_BootProfile;

/* ── Unsupported feature flags ─────────────────────────────────── */
typedef enum {
    /* 3D rasterizer not available */
    NX_UNSUPPFEAT_3D_RASTERIZER   = (1 << 0),
    /* Smooth movement / camera interpolation */
    NX_UNSUPPFEAT_SMOOTH_MOVEMENT = (1 << 1),
    /* Per-champion spell book UI (replaced by rune book) */
    NX_UNSUPPFEAT_SPELL_BOOK_UI   = (1 << 2),
    /* Party reordering in dungeon */
    NX_UNSUPPFEAT_PARTY_REORDER   = (1 << 3),
    /* Gangulf revival at temple */
    NX_UNSUPPFEAT_GANGULF_REVIVAL  = (1 << 4),
    /* Kings Wisdom scroll equivalent */
    NX_UNSUPPFEAT_KINGS_WISDOM    = (1 << 5),
    /* Cross-game champion import from DM1/CSB */
    NX_UNSUPPFEAT_CROSS_IMPORT    = (1 << 6),
    /* V2.0/V2.1/V2.2 enhanced graphics modes */
    NX_UNSUPPFEAT_V2_MODES        = (1 << 7),
} Nexus_V1_UnsupportedFeatureFlags;

/* ── Saturn platform diagnostics ──── ────────────────────────────── */
typedef enum {
    NEXUS_V1_DIAG_OK = 0,
    NEXUS_V1_DIAG_MISSING_DM_BIN,
    NEXUS_V1_DIAG_MISSING_DMDF_ARCHIVE,
    NEXUS_V1_DIAG_MISSING_DGN_LEVEL,
    NEXUS_V1_DIAG_MISSING_CHAMPION_DATA,
    NEXUS_V1_DIAG_INVALID_DM_BIN_MAGIC,
    NEXUS_V1_DIAG_INVALID_DMDF,
    NEXUS_V1_DIAG_CORRUPT_SATURN_CD_IMAGE,
    NEXUS_V1_DIAG_NO_CDDA_TRACK,
    NEXUS_V1_DIAG_OUT_OF_MEMORY,       /* Saturn has constrained RAM */
    NEXUS_V1_DIAG_SAVE_WRITE_FAILED,
    NEXUS_V1_DIAG_COUNT
} Nexus_V1_DiagnosticCode;

typedef struct {
    Nexus_V1_DiagnosticCode code;
    char message[256];
    char detail[512];
    char suggestion[256];
} Nexus_V1_Diagnostic;

/* ── Function declarations ───────────────────────────────────────── */

/*
 * Nexus_V1_BootProfile_GetDefault:
 *   Returns a static default boot profile for Nexus. This is the
 *   recommended starting config for a fresh Nexus game session.
 *
 *   ReDMCSB: COMMAND.C F0359 "LoadGameSettings" — settings initialized
 *   at program start. Nexus Saturn equivalent.
 */
const Nexus_V1_BootProfile*
Nexus_V1_BootProfile_GetDefault(void);

/*
 * Nexus_V1_BootProfile_Init:
 *   Initializes a boot profile struct from a base directory path and
 *   an optional overrides struct. Resolves dataDir/saveDir/configPath
 *   to concrete paths following Firestaff conventions.
 *
 * Parameters:
 *   profile        — pointer to profile struct to fill (must not be NULL)
 *   baseDataDir    — FIRESTAFF_DATA path or NULL to use default
 *   baseSaveDir    — FIRESTAFF_SAVES path or NULL to use default
 *   runtimeFlags   — OR of NEXUS_V1_RF_* flags, or 0 for defaults
 *
 * Returns: 0 on success, -1 on NULL profile pointer or on failure
 *          to resolve any required directory.
 */
int Nexus_V1_BootProfile_Init(Nexus_V1_BootProfile *profile,
                               const char *baseDataDir,
                               const char *baseSaveDir,
                               unsigned int runtimeFlags);

/*
 * Nexus_V1_BootProfile_ValidateAssets:
 *   Performs a quick diagnostic scan of the Nexus data directory
 *   looking for required files (DM.BIN, SEGADATA.BIN, DMDF archives,
 *   DGN level files, champion data directory). Emits diagnostics
 *   for any missing required assets.
 *
 * Parameters:
 *   profile  — validated boot profile with populated dataDir
 *   diags    — array of Nexus_V1_Diagnostic (capacity NEXUS_V1_DIAG_COUNT)
 *   maxDiags — number of slots in diags array
 *
 * Returns: number of diagnostics written (0 = all clear).
 */
int Nexus_V1_BootProfile_ValidateAssets(const Nexus_V1_BootProfile *profile,
                                          Nexus_V1_Diagnostic *diags,
                                          size_t maxDiags);

/*
 * Nexus_V1_BootProfile_GetAssetRoot:
 *   Returns the resolved data directory path for the profile.
 *   Allocates a static buffer on first call; subsequent calls reuse
 *   the buffer without reallocation.
 *
 * Returns: pointer to static buffer containing the resolved path,
 *          or NULL if profile is NULL.
 */
const char* Nexus_V1_BootProfile_GetAssetRoot(const Nexus_V1_BootProfile *profile);

/*
 * Nexus_V1_BootProfile_GetSaveRoot:
 *   Returns the resolved save-game directory path for the profile.
 *   Creates the directory tree if it does not exist.
 *
 * Returns: pointer to static buffer, or NULL on error.
 */
const char* Nexus_V1_BootProfile_GetSaveRoot(const Nexus_V1_BootProfile *profile);

/*
 * Nexus_V1_BootProfile_SupportedFeatures:
 *   Returns the set of supported features given the current runtime
 *   flags. This is the inverse of unsupportedFeatureMask — features
 *   NOT flagged as unsupported are considered supported.
 */
unsigned int Nexus_V1_BootProfile_SupportedFeatures(const Nexus_V1_BootProfile *profile);

/*
 * Nexus_V1_BootProfile_GetDiagnosticString:
 *   Human-readable string for a diagnostic code.
 */
const char* Nexus_V1_BootProfile_GetDiagnosticString(Nexus_V1_DiagnosticCode code);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_NEXUS_V1_BOOT_PROFILE_H */
