#ifndef FIRESTAFF_TQR_V1_BOOT_PROFILE_H
#define FIRESTAFF_TQR_V1_BOOT_PROFILE_H
/*
 * firestaff_tqr_v1_boot_profile.h — Theron's Quest V1 boot/runtime profile
 *
 * Theron's Quest — Hudson Soft / Namuland (PC Engine/TurboGrafx-16 CD)
 * Phase 1: Separate Theron's Quest boot/runtime profile from DM1/CSB/DM2/Nexus.
 *
 * Scope:
 *   • Menu launch entry and availability gating
 *   • Asset roots (PC Engine HuCard / CD data directories)
 *   • Save namespace (no in-dungeon saves — only between dungeons)
 *   • Platform diagnostics (PC Engine HuCard/CD hardware quirks)
 *   • Deterministic config defaults
 *
 * Reference: http://dmweb.free.fr/games/therons-quest/
 * Provenance gate: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 *
 * Game ID for this profile: "theron"
 * PC Engine assets live under: FIRESTAFF_DATA/theron/ or ~/.firestaff/data/theron/
 *
 * Key differences from DM1/CSB:
 *   • "Light" version — subset of DM1 items, creatures, spells
 *   • 7 mini-dungeons, some copied/inspired by DM1/CSB
 *   • Theron persistent across dungeons; companion champions reset per dungeon
 *   • No in-dungeon saves — only save between dungeons (party state preserved
 *     in BRAM save file between dungeon sessions, not mid-dungeon)
 *   • No Kings Wisdom equivalent
 *   • No Gangulf revival
 *   • PC Engine planar graphics (PC Engine has 512-color palette, 16-color
 *     per tile plane, 2 plane layers + sprite layer)
 */

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── Game identifier ─────────────────────────────────────────────── */
#define TQR_V1_GAME_ID          "theron"
#define TQR_V1_GAME_LABEL       "THERON'S QUEST"
#define TQR_V1_PLATFORM_LABEL   "PC ENGINE / TURBOGRAFX-16"

/* ── Config directory root ─────────────────────────────────────────── */
#define TQR_V1_CONFIG_DIR       "firestaff/theron"
#define TQR_V1_SAVE_SUB_DIR     "saves/theron"
#define TQR_V1_USER_DATA_SUB_DIR "userdata/theron"

/* ── Deterministic tick rate ─────────────────────────────────────────── */
/*
 * PC Engine runs at ~7.16 MHz (HuCard) or ~7.16 MHz (CD) with slightly
 * different timing due to CD access latency. The V1 game logic for
 * Theron's Quest uses the same 55 ms world tick as DM1/CSB — the
 * dungeon logic is derived from the same FTL codebase.
 * Render rate: PC Engine outputs 59.94 Hz (NTSC) or 50 Hz (PAL) via
 * the VDC chip; the renderer syncs to display refresh.
 */
#define TQR_V1_TICK_RATE_MS     55   /* identical to DM1/CSB — world tick */
#define TQR_V1_RENDER_RATE_MS   16   /* PC Engine VDC ~60 fps render tick (approx) */

/* ── Boot profile runtime config flags ─────────────────────────────── */
typedef enum {
    /*
     * TQR_V1_RF_USE_HUCARD:
     *   Load dungeon assets from HuCard ROM image (single card).
     *   HuCard format: program ROM + data ROM on one card.
     *   File expected: THERON.HUC or GAME.DAT in theron/ data dir.
     */
    TQR_V1_RF_USE_HUCARD             = (1 << 0),

    /*
     * TQR_V1_RF_USE_CD_IMAGE:
     *   Load dungeon assets from PC Engine CD image (single track ISO).
     *   CD format: MODE1 CD-ROM with track data. The CD version has
     *   additional audio tracks (CDDA). File expected: theron.iso or
     *   theron.bin in theron/ data dir.
     */
    TQR_V1_RF_USE_CD_IMAGE            = (1 << 1),

    /*
     * TQR_V1_RF_NO_IN_DUNGEON_SAVE:
     *   Theron's Quest does NOT support saving mid-dungeon.
     *   Saves are only valid between dungeons (at the overworld /
     *   dungeon-select screen). Attempting an in-dungeon save should
     *   produce a diagnostic message and reject the save.
     *   This is a fundamental difference from DM1 which allows save
     *   at any time via the PAUSE menu.
     */
    TQR_V1_RF_NO_IN_DUNGEON_SAVE      = (1 << 2),

    /*
     * TQR_V1_RF_THERON_PERSISTENT:
     *   The champion named Theron persists across dungeons with his
     *   skills and stats intact. Companion champions reset (lose
     *   items, skill levels) when entering a new dungeon.
     *   When set, champion import logic uses TQR persistence rules.
     */
    TQR_V1_RF_THERON_PERSISTENT       = (1 << 3),

    /*
     * TQR_V1_RF_LIGHT_ITEM_SET:
     *   Theron's Quest uses only a subset of DM1 items. When set,
     *   the item database uses the TQR item list and missing items
     *   return "NOT AVAILABLE" in the UI.
     */
    TQR_V1_RF_LIGHT_ITEM_SET          = (1 << 4),

    /*
     * TQR_V1_RF_LIGHT_CREATURE_SET:
     *   Theron's Quest uses only a subset of DM1 creatures. When set,
     *   the creature roster is limited to TQR native creatures plus
     *   any DM1/CSB creatures that appear in the 7 mini-dungeons.
     */
    TQR_V1_RF_LIGHT_CREATURE_SET     = (1 << 5),

    /*
     * TQR_V1_RF_NO_KINGS_WISDOM:
     *   Kings Wisdom (DM1 F0307 equivalent) is absent in Theron's Quest.
     *   The temple recovery / Phoenix Down path is the only revival method.
     */
    TQR_V1_RF_NO_KINGS_WISDOM        = (1 << 6),

    /*
     * TQR_V1_RF_NO_GANGULF_REVIVAL:
     *   Gangulf revival mechanic (DM1 F0028 equivalent) is absent.
     *   Champions must be revived at a temple or via Phoenix Down.
     */
    TQR_V1_RF_NO_GANGULF_REVIVAL     = (1 << 7),

    /*
     * TQR_V1_RF_NO_PARTY_SWAP:
     *   In-dungeon party reordering via F-keys (DM1 convention) is
     *   not available in Theron's Quest. Party order is fixed at
     *   dungeon entry.
     */
    TQR_V1_RF_NO_PARTY_SWAP          = (1 << 8),

    /*
     * TQR_V1_RF_NOMINAL_SCROLL_SYSTEM:
     *   Theron's Quest uses the DM1 scroll/spell system (not the rune
     *   book of Nexus). This flag documents that scroll magic is
     *   available but in the reduced TQR item set.
     */
    TQR_V1_RF_NOMINAL_SCROLL_SYSTEM   = (1 << 9),

    /*
     * TQR_V1_RF_PCE_CDDA_AUDIO:
     *   Audio playback routes through PC Engine CDDA when using CD image.
     *   HuCard version has limited audio (PSG + ADPCM).
     */
    TQR_V1_RF_PCE_CDDA_AUDIO         = (1 << 10),
} TQR_V1_RuntimeFlags;

/* ── Boot profile struct ──────────────────────────────────────────── */
typedef struct {
    /*
     * runtimeFlags:
     *   Bitwise OR of TQR_V1_RF_* flags. Default for a clean
     *   Theron's Quest game start is:
     *     TQR_V1_RF_USE_HUCARD  (HuCard is the primary expected format)
     *     TQR_V1_RF_NO_IN_DUNGEON_SAVE
     *     TQR_V1_RF_THERON_PERSISTENT
     *     TQR_V1_RF_LIGHT_ITEM_SET
     *     TQR_V1_RF_LIGHT_CREATURE_SET
     *     TQR_V1_RF_NO_KINGS_WISDOM
     *     TQR_V1_RF_NO_GANGULF_REVIVAL
     *     TQR_V1_RF_NO_PARTY_SWAP
     *     TQR_V1_RF_NOMINAL_SCROLL_SYSTEM
     *   Set TQR_V1_RF_USE_CD_IMAGE if using the CD version instead.
     */
    unsigned int runtimeFlags;

    /* dataDir:
     *   Root path for theron game assets. When USE_HUCARD is active,
     *   this directory must contain a HuCard ROM image or GAME.DAT.
     *   When USE_CD_IMAGE is active, must contain theron.iso/theron.bin.
     *   Defaults to $FIRESTAFF_DATA/theron/ or ~/.firestaff/data/theron/
     */
    const char *dataDir;

    /* saveDir:
     *   Directory for Theron's Quest savegames. Saved to
     *   $saveDir/theron/slot_N.tqrs
     *   NOTE: Only between-dungeon saves are valid. In-dungeon saves
     *   produce an error diagnostic.
     *   Defaults to $FIRESTAFF_SAVES/theron/
     */
    const char *saveDir;

    /* configPath:
     *   Per-game config file (PC Engine display output, HuCard/CD
     *   selection, audio settings). Game-namespaced.
     *   Defaults to $FIRESTAFF_CONFIG/theron/firestaff.ini
     */
    const char *configPath;

    /* presentationMode:
     *   Graphics presentation mode. Index into M12 presentation mode
     *   table. M12_PRESENTATION_V1_ORIGINAL is the only supported
     *   mode in Phase 1. V2.0/V2.1/V2.2 render paths are not yet
     *   available for Theron's Quest.
     */
    int presentationMode;

    /* tickRateMs / renderRateMs:
     *   Override tick rates from defaults. Set to 0 to use built-in
     *   defaults (55 ms / 16 ms).
     */
    unsigned int tickRateMs;
    unsigned int renderRateMs;

    /* unsupportedFeatureMask:
     *   Features that are unsupported and should be greyed out or
     *   replaced with explanatory messaging in the UI.
     *   Bitwise OR of TQR_UNSUPPFEAT_* feature flags (see below).
     */
    unsigned int unsupportedFeatureMask;

    /* dungeonIndex:
     *   Current dungeon being played (0-6 for the 7 mini-dungeons).
     *   -1 means at the dungeon-select / overworld screen.
     *   Persists in save files to know where the party left off.
     */
    int dungeonIndex;

    /* inDungeon:
     *   Set to 1 when party is inside a dungeon (save prohibited).
     *   Set to 0 when at overworld/dungeon-select (save allowed).
     *   Mirrors TQR_V1_RF_NO_IN_DUNGEON_SAVE enforcement.
     */
    int inDungeon;
} TQR_V1_BootProfile;

/* ── Unsupported feature flags ─────────────────────────────────── */
typedef enum {
    /* In-dungeon saves (only between-dungeon saves allowed) */
    TQR_UNSUPPFEAT_IN_DUNGEON_SAVE   = (1 << 0),
    /* Kings Wisdom scroll equivalent */
    TQR_UNSUPPFEAT_KINGS_WISDOM      = (1 << 1),
    /* Gangulf revival mechanic */
    TQR_UNSUPPFEAT_GANGULF_REVIVAL   = (1 << 2),
    /* Party reordering in dungeon */
    TQR_UNSUPPFEAT_PARTY_REORDER     = (1 << 3),
    /* Cross-game champion import from DM1/CSB */
    TQR_UNSUPPFEAT_CROSS_IMPORT      = (1 << 4),
    /* V2.0/V2.1/V2.2 enhanced graphics modes */
    TQR_UNSUPPFEAT_V2_MODES          = (1 << 5),
    /* Full DM1 item set (TQR is "light" — subset only) */
    TQR_UNSUPPFEAT_FULL_ITEM_SET     = (1 << 6),
    /* Full DM1 creature roster (TQR is "light" — subset only) */
    TQR_UNSUPPFEAT_FULL_CREATURE_SET = (1 << 7),
    /* Full DM1 spell roster */
    TQR_UNSUPPFEAT_FULL_SPELL_ROSTER = (1 << 8),
    /* Smooth movement / camera interpolation */
    TQR_UNSUPPFEAT_SMOOTH_MOVEMENT   = (1 << 9),
} TQR_V1_UnsupportedFeatureFlags;

/* ── PC Engine platform diagnostics ──────────────────────────────── */
typedef enum {
    TQR_V1_DIAG_OK = 0,
    TQR_V1_DIAG_MISSING_HUCARD_IMAGE,
    TQR_V1_DIAG_MISSING_CD_IMAGE,
    TQR_V1_DIAG_MISSING_DUNGEON_DATA,
    TQR_V1_DIAG_MISSING_CHAMPION_DATA,
    TQR_V1_DIAG_INVALID_HUCARD_HEADER,
    TQR_V1_DIAG_INVALID_CD_IMAGE,
    TQR_V1_DIAG_CORRUPT_DUNGEON_FILE,
    TQR_V1_DIAG_NO_CDDA_TRACK,
    TQR_V1_DIAG_IN_DUNGEON_SAVE_ATTEMPTED,  /* save rejected mid-dungeon */
    TQR_V1_DIAG_OUT_OF_MEMORY,       /* PC Engine has constrained RAM (64 KB work RAM) */
    TQR_V1_DIAG_SAVE_WRITE_FAILED,
    TQR_V1_DIAG_INCOMPATIBLE_VERSION,
    TQR_V1_DIAG_COUNT
} TQR_V1_DiagnosticCode;

typedef struct {
    TQR_V1_DiagnosticCode code;
    char message[256];
    char detail[512];
    char suggestion[256];
} TQR_V1_Diagnostic;

/* ── Function declarations ───────────────────────────────────────── */

/*
 * TQR_V1_BootProfile_GetDefault:
 *   Returns a static default boot profile for Theron's Quest.
 *   This is the recommended starting config for a fresh TQR session.
 *
 * Reference: http://dmweb.free.fr/games/therons-quest/; Phase 0 provenance
 *   gate: docs/source-lock/tqr_v1_phase0_provenance_gate_H2339.md
 */
const TQR_V1_BootProfile*
TQR_V1_BootProfile_GetDefault(void);

/*
 * TQR_V1_BootProfile_Init:
 *   Initializes a boot profile struct from a base directory path and
 *   an optional overrides struct. Resolves dataDir/saveDir/configPath
 *   to concrete paths following Firestaff conventions.
 *
 * Parameters:
 *   profile        — pointer to profile struct to fill (must not be NULL)
 *   baseDataDir    — FIRESTAFF_DATA path or NULL to use default
 *   baseSaveDir    — FIRESTAFF_SAVES path or NULL to use default
 *   runtimeFlags   — OR of TQR_V1_RF_* flags, or 0 for defaults
 *
 * Returns: 0 on success, -1 on NULL profile pointer or on failure
 *          to resolve any required directory.
 */
int TQR_V1_BootProfile_Init(TQR_V1_BootProfile *profile,
                              const char *baseDataDir,
                              const char *baseSaveDir,
                              unsigned int runtimeFlags);

/*
 * TQR_V1_BootProfile_ValidateAssets:
 *   Performs a quick diagnostic scan of the Theron data directory
 *   looking for required files (HuCard image or CD ISO, dungeon data,
 *   champion data). Emits diagnostics for any missing required assets.
 *
 * Parameters:
 *   profile  — validated boot profile with populated dataDir
 *   diags    — array of TQR_V1_Diagnostic (capacity TQR_V1_DIAG_COUNT)
 *   maxDiags — number of slots in diags array
 *
 * Returns: number of diagnostics written (0 = all clear).
 */
int TQR_V1_BootProfile_ValidateAssets(const TQR_V1_BootProfile *profile,
                                        TQR_V1_Diagnostic *diags,
                                        size_t maxDiags);

/*
 * TQR_V1_BootProfile_GetAssetRoot:
 *   Returns the resolved data directory path for the profile.
 *   Allocates a static buffer on first call; subsequent calls reuse
 *   the buffer without reallocation.
 *
 * Returns: pointer to static buffer containing the resolved path,
 *          or NULL if profile is NULL.
 */
const char* TQR_V1_BootProfile_GetAssetRoot(const TQR_V1_BootProfile *profile);

/*
 * TQR_V1_BootProfile_GetSaveRoot:
 *   Returns the resolved save-game directory path for the profile.
 *   Creates the directory tree if it does not exist.
 *
 * Returns: pointer to static buffer, or NULL on error.
 */
const char* TQR_V1_BootProfile_GetSaveRoot(const TQR_V1_BootProfile *profile);

/*
 * TQR_V1_BootProfile_SupportedFeatures:
 *   Returns the set of supported features given the current runtime
 *   flags. This is the logical inverse of unsupportedFeatureMask.
 */
unsigned int TQR_V1_BootProfile_SupportedFeatures(const TQR_V1_BootProfile *profile);

/*
 * TQR_V1_BootProfile_GetDiagnosticString:
 *   Human-readable string for a diagnostic code.
 */
const char* TQR_V1_BootProfile_GetDiagnosticString(TQR_V1_DiagnosticCode code);

/*
 * TQR_V1_BootProfile_CanSave:
 *   Returns 1 if saving is currently allowed (party at overworld /
 *   between-dungeon screen), 0 if save is prohibited (party inside
 *   a dungeon). This enforces TQR_V1_RF_NO_IN_DUNGEON_SAVE.
 *
 *   When inDungeon is set in the profile and runtimeFlags includes
 *   TQR_V1_RF_NO_IN_DUNGEON_SAVE, this function returns 0 and
 *   emits a diagnostic with message "In-dungeon saves not supported".
 *
 * Parameters:
 *   profile — boot profile to check
 *
 * Returns: 1 if save allowed, 0 if prohibited.
 */
int TQR_V1_BootProfile_CanSave(const TQR_V1_BootProfile *profile);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_TQR_V1_BOOT_PROFILE_H */