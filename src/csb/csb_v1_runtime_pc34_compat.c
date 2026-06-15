/*
 * csb_v1_runtime_pc34_compat.c — CSB V1 Runtime Profile Implementation
 *
 * Source-lock anchors:
 *   ENTRANCE.C: F0806_F0806_ENTRANCE_int       (game boot sequence)
 *   ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)
 *   ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics init)
 *   SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful   (header verify)
 *   SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful (header write)
 *   LOADSAVE.C: F0435_STARTEND_LoadGame              (save load)
 *   LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame (save)
 *   DUNGEON.C:  F0237_DUNGEON_DungeonLoad            (hash-verified load)
 *   CASTER.C:   F0211_CASTER_ClearSpellEffects       (spell grid reset at boot)
 *   BugsAndChanges.htm: CHANGE7_29   (new header format: CSBGAME.DAT)
 *   MEDIA529_F20E_F20J: F20E/F21E  (ST save path)
 *   MEDIA332_F20E_F21E_A31E_F31E: CSB C29 key index
 */

#include "csb_v1_runtime_pc34_compat.h"
#include "asset_find_by_hash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

/* ── Known CSB hashes ──────────────────────────────────────────────────── */

/*
 * PC 3.4 English Atari ST + Amiga: same dungeon hash.
 * All CSB platforms share the same dungeon.dat content — only
 * graphics/assets vary by platform.
 */
static const char *const g_csb_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

/* ── Variant info table ─────────────────────────────────────────────── */

/*
 * CSB variant info.  Platform-specific; game logic is identical.
 * md5_gfx  = GRAPHICS.DAT hash for this variant
 * md5_graf = CSBGRAPH.DAT / CSB.DAT hash (same hash as GRAPHICS.DAT for
 *            the "graphics-only" variants — they lack the overlay archive)
 * md5_dungeon = DUNGEON.DAT hash (shared by all CSB platforms)
 *
 * ReDMCSB COMPILE.H MEDIA tags + CSBWin AssetCache variant mapping.
 */
static const CSB_V1_VariantInfo g_csb_variants[CSB_V1_VARIANT_COUNT] = {
    [CSB_V1_VARIANT_UNKNOWN] = {
        CSB_V1_VARIANT_UNKNOWN,
        "Unknown",
        "",
        "",
        "",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_EN] = {
        CSB_V1_VARIANT_PC34_EN,
        "PC DOS 3.4 English",
        "MEDIA278:P20JA_P20JB",
        "61fbfd56887c94adc26888a9491c6611",
        "61fbfd56887c94adc26888a9491c6611",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_PC34_MULTI] = {
        CSB_V1_VARIANT_PC34_MULTI,
        "PC DOS 3.4 Multilanguage",
        "MEDIA278:I34E_I34M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST20_EN] = {
        CSB_V1_VARIANT_ST20_EN,
        "Atari ST 2.0 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST21_EN] = {
        CSB_V1_VARIANT_ST21_EN,
        "Atari ST 2.1 English",
        "MEDIA332:S20E_S21E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_EN] = {
        CSB_V1_VARIANT_AMIGA35_EN,
        "Amiga 3.5 English",
        "MEDIA529:A35E",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "291e1bc6803e3dc4b974c60117ca5d68",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_AMIGA35_MULTI] = {
        CSB_V1_VARIANT_AMIGA35_MULTI,
        "Amiga 3.5 Multilanguage",
        "MEDIA529:A35M",
        "cefaddfdf5651df2c91f61b5611a8362",
        "cefaddfdf5651df2c91f61b5611a8362",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20J] = {
        CSB_V1_VARIANT_ST_F20J,
        "Atari ST TT (F20J)",
        "MEDIA529:F20J",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    },
    [CSB_V1_VARIANT_ST_F20E] = {
        CSB_V1_VARIANT_ST_F20E,
        "Atari ST (F20E)",
        "MEDIA529:F20E",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "ebf6a57af3f27782e358c0490bfd2f2e",
        "6695d2acebce49f95db1d8f3a5c733de"
    }
};

_Static_assert(CSB_V1_VARIANT_ST_F20E == CSB_V1_VARIANT_COUNT - 1,
               "CSB_V1_VARIANT_COUNT must match last enum value");

/* ── Platform-specific save dir ────────────────────────────────────── */

#if defined(_WIN32)
#define CSB_PATH_SEP '\\'
#else
#define CSB_PATH_SEP '/'
#endif

static char g_csb_save_dir_buf[512];
static char g_csb_save_path_buf[512];
static int  g_save_dir_init = 0;

static void csb_v1_init_save_dir(void)
{
    if (g_save_dir_init) return;
    g_save_dir_init = 1;

    if (0) {}
#if defined(_WIN32)
    {
        const char *appdata = getenv("APPDATA");
        const char *base = appdata ? appdata : "C:\\";
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s\\Firestaff\\csb\\saves\\", base);
    }
#elif defined(__APPLE__)
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/Library/Application Support/Firestaff/csb/saves/",
                home ? home : "");
    }
#else
    {
        const char *home = getenv("HOME");
        snprintf(g_csb_save_dir_buf, sizeof(g_csb_save_dir_buf),
                "%s/.local/share/firestaff/csb/saves/",
                home ? home : "");
    }
#endif
}

/* ── Internal MD5 helper ─────────────────────────────────────────────── */

/*
 * Compute MD5 hex of a file.  Returns 0 on success, -1 on error.
 * outHex must be at least 33 bytes.
 * Uses the same MD5 backend as asset_find_by_hash.c.
 */
static int csb_v1_file_md5_hex(const char *path, char *outHex, size_t hexSize)
{
    typedef struct {
        unsigned int state[4];
        unsigned int count[2];
        unsigned char buffer[64];
    } AssetMd5Ctx;

    typedef struct {
        unsigned int a, b, c, d;
    } AssetMD5State;

    typedef void (*AssetMd5UpdateFn)(AssetMd5Ctx *, const unsigned char *, unsigned int);
    typedef void (*AssetMd5FinalFn)(AssetMd5Ctx *, char *);

    /* Use asset_find_by_md5_list internally for file existence + MD5.
     * We only expose the hash computation through asset_status_m12.
     * For runtime use, the hash comes from M12_AssetStatus scan results.
     * This function stubs to the simplest cross-platform approach. */
    (void)path;
    (void)outHex;
    (void)hexSize;
    if (outHex && hexSize >= 33) {
        outHex[0] = '\0';
    }
    return -1;
}

/* ── Difficulty helpers ─────────────────────────────────────────────── */

int csb_v1_runtime_calc_difficulty(int champion_count)
{
    int x100;
    if (champion_count < 1) champion_count = 1;
    if (champion_count > 4) champion_count = 4;
    x100 = CSB_V1_DIFFICULTY_BASE + (champion_count - 1) * CSB_V1_DIFFICULTY_PER_CHAMP;
    return x100;
}

const char *csb_v1_runtime_difficulty_str(int difficulty_x100)
{
    switch (difficulty_x100) {
        case 100: return "Easy (1 champion)";
        case 125: return "Normal (2 champions)";
        case 150: return "Hard (3 champions)";
        case 200: return "Extreme (4 champions)";
        default:  return "Unknown";
    }
}

/* ── Variant diagnostics ────────────────────────────────────────────── */

const char *csb_v1_runtime_variant_name(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return g_csb_variants[id].name;
    }
    return "Unknown";
}

const CSB_V1_VariantInfo *csb_v1_runtime_get_variant_info(CSB_V1_VariantId id)
{
    if (id >= 0 && id < CSB_V1_VARIANT_COUNT) {
        return &g_csb_variants[id];
    }
    return &g_csb_variants[CSB_V1_VARIANT_UNKNOWN];
}

/*
 * Detect variant by matching gfx + dungeon MD5 hashes to known variants.
 * Falls back to UNKNOWN if no hash matches (assets not yet verified).
 * The dungeon hash is the primary filter (all CSB shares same dungeon hash).
 */
int csb_v1_runtime_detect_variant(const char *gfx_path,
                                    const char *dungeon_path,
                                    const char *md5_gfx,
                                    const char *md5_dungeon)
{
    int i;
    (void)gfx_path;  /* gfx_path used only for diagnostics, md5_gfx is the key */

    if (!md5_dungeon) return CSB_V1_VARIANT_UNKNOWN;
    if (strcmp(md5_dungeon, "6695d2acebce49f95db1d8f3a5c733de") != 0) {
        return CSB_V1_VARIANT_UNKNOWN;
    }

    if (md5_gfx) {
        for (i = 1; i < CSB_V1_VARIANT_COUNT; i++) {
            if (g_csb_variants[i].md5_gfx[0] != '\0' &&
                strcmp(md5_gfx, g_csb_variants[i].md5_gfx) == 0) {
                return g_csb_variants[i].id;
            }
        }
    }

    return CSB_V1_VARIANT_UNKNOWN;
}

/* ── Asset discovery ────────────────────────────────────────────────── */

/*
 * Search for CSB DUNGEON.DAT by hash.
 * ReDMCSB: DUNGEON.C F0237_DUNGEON_DungeonLoad (hash-gated open).
 *
 * Search order:
 *   data_dir/csb/       (canonical per-game subdirectory)
 *   data_dir/           (shared DM1/CSB/DM2 fallback)
 *   data_dir/csb/csb/   (nested double-drop, rare)
 */
const char *csb_v1_runtime_find_dungeon(const char *data_dir,
                                         CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    if (!asset_find_by_md5_list(data_dir, g_csb_dungeon_hashes,
                                 found_path, sizeof(found_path), NULL, 4)) {
        return NULL;
    }

    out_result->path = found_path;
    out_result->kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    return found_path;
}

/*
 * Search for CSB graphics archive.
 *
 * ReDMCSB asset search (DISK.C + CSBWin AssetCache):
 *   Floppy platforms: CSB.DAT replaces GRAPHICS.DAT entirely
 *   Data/CD platforms: CSBGRAPH.DAT overlays GRAPHICS.DAT
 *   Hybrid platforms:  CSBGRAPH.DAT takes precedence over GRAPHICS.DAT
 *
 * File search order; we try all known archive names and trust the
 * caller (asset_status_m12) to verify the hash.
 */
static const char *const g_csb_gfx_search[] = {
    "csb.dat",
    "CSB.DAT",
    "csbgraph.dat",
    "CSBGRAPH.DAT",
    "graphics.dat",
    "GRAPHICS.DAT",
    NULL
};

const char *csb_v1_runtime_find_graphics(const char *data_dir,
                                             const char *version_hint,
                                             CSB_V1_AssetResult *out_result)
{
    static char found_path[ASSET_PATH_MAX];
    const char *const *names;
    (void)version_hint; /* TODO: narrow search by version hint */

    if (!data_dir || !out_result) return NULL;
    memset(out_result, 0, sizeof(*out_result));

    for (names = g_csb_gfx_search; *names != NULL; names++) {
        char tmp[ASSET_PATH_MAX];
        struct stat st;
        CSB_V1_AssetGfxArchiveType kind;

        snprintf(tmp, sizeof(tmp), "%s/%s", data_dir, *names);
        if (stat(tmp, &st) != 0) continue;
        if (!S_ISREG(st.st_mode)) continue;

        strncpy(found_path, tmp, sizeof(found_path) - 1);
        found_path[sizeof(found_path) - 1] = '\0';
        out_result->path = found_path;

        if (strcasecmp(*names, "CSB.DAT") == 0 ||
            strcasecmp(*names, "csb.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSB;
        } else if (strcasecmp(*names, "CSBGRAPH.DAT") == 0 ||
                   strcasecmp(*names, "csbgraph.dat") == 0) {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
        } else {
            kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
        }
        out_result->kind = kind;
        return found_path;
    }
    return NULL;
}

/* ── Save namespace paths ───────────────────────────────────────────── */

const char *csb_v1_runtime_save_dir(void)
{
    csb_v1_init_save_dir();
    return g_csb_save_dir_buf;
}

const char *csb_v1_runtime_save_path(int slot)
{
    csb_v1_init_save_dir();
    if (slot < 0 || slot > 9) slot = 0;
    snprintf(g_csb_save_path_buf, sizeof(g_csb_save_path_buf),
             "%s%ccsb_save_%d.fsav",
             g_csb_save_dir_buf, CSB_PATH_SEP, slot);
    return g_csb_save_path_buf;
}

/* ── Internal tick helper ─────────────────────────────────────────────── */

static void csb_v1_fire_tick(CSB_V1_RuntimeProfile *profile)
{
    int dispatched;

    /* Source: ReDMCSB GAMELOOP.C F0002 lines 69-124 calls
     * F0261_TIMELINE_Process_CPSEF() before incrementing
     * G0313_ul_GameTime.  TIMELINE.C F0240 lines 702-708 expires the
     * first heap event when event_time <= G0313_ul_GameTime. */
    profile->timeline_queue.gameTick = profile->game_time;
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    dispatched = dm1v1_event_process_tick(&profile->timeline_queue,
                                          &profile->last_timeline_dispatch);
    if (dispatched > 0) {
        profile->timeline_dispatch_count += (uint32_t)dispatched;
    }

    profile->game_time++;
    profile->tick_count++;
    profile->game_ticks += CSB_V1_TICK_MS_NOMINAL;

    /* Chaos Magic spell grid is versioned on each tick.
     * F0211_CASTER_ClearSpellEffects increments spell_grid_version at world load.
     * We advance chaos_level here for ambient oscillation.
     * Source: CSBWin Magic.cpp ambient loop (no direct ReDMCSB ref). */
    if (profile->chaos_magic.magic_initialized) {
        uint32_t beat = profile->tick_count % 20U;
        profile->chaos_magic.chaos_level = (uint8_t)((beat < 10U) ? 0U : 1U);
        profile->chaos_magic.spell_grid_version++;
    }
}

static int csb_v1_runtime_input_command_target_dir(int current_dir, int command)
{
    if (command == DM1_V1_COMMAND_TURN_RIGHT) {
        return (current_dir + 1) & 3;
    }
    if (command == DM1_V1_COMMAND_TURN_LEFT) {
        return (current_dir + 3) & 3;
    }
    return -1;
}

/* ── Runtime profile API ────────────────────────────────────────────── */

void csb_v1_runtime_init(CSB_V1_RuntimeProfile *profile, const char *data_dir)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));

    profile->variant_id     = CSB_V1_VARIANT_UNKNOWN;
    profile->difficulty    = CSB_V1_DIFFICULTY_HARD; /* default: 3 champions */
    profile->current_level = 0;
    profile->current_world = 0;
    profile->level_count   = 1;
    profile->world_count   = 1;
    profile->champion_count = 3;
    profile->leader_index = -1;
    profile->magic_caster_index = -1;
    profile->party_state_valid = 0;
    csb_v1_character_init_default(&profile->party_state);

    profile->party_x = CSB_V1_START_PARTY_X;
    profile->party_y = CSB_V1_START_PARTY_Y;
    profile->party_z = CSB_V1_START_PARTY_Z;
    profile->party_dir = CSB_V1_START_PARTY_DIR;

    profile->state     = CSB_STATE_TITLE;
    profile->paused    = 0;
    profile->victory   = 0;
    profile->game_over = 0;

    profile->game_ticks    = 0;
    profile->game_time     = 0;
    profile->total_play_ms = 0;
    profile->tick_count    = 0;
    dm1v1_event_queue_init(&profile->timeline_queue, profile->game_time);
    memset(&profile->last_timeline_dispatch, 0,
           sizeof(profile->last_timeline_dispatch));
    profile->timeline_dispatch_count = 0;
    DM1_V1_InputCommandQueue_InitPc34Compat(&profile->input_command_queue);
    memset(&profile->last_input_dispatch, 0,
           sizeof(profile->last_input_dispatch));
    profile->input_dispatch_count = 0;

    profile->data_dir = data_dir;
    profile->save_dir = csb_v1_runtime_save_dir();
}

int csb_v1_runtime_add_timeline_event(CSB_V1_RuntimeProfile *profile,
                                      const struct DM1_Event_V1 *event)
{
    if (!profile || !event) return -1;
    profile->timeline_queue.gameTick = profile->game_time;
    return dm1v1_event_add(&profile->timeline_queue, event);
}

int csb_v1_runtime_get_last_timeline_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct DM1_TickDispatchResult_V1 *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_timeline_dispatch;
    return out_result->count;
}

int csb_v1_runtime_enqueue_input_command(CSB_V1_RuntimeProfile *profile,
                                         int command,
                                         int x,
                                         int y)
{
    if (!profile) return 0;
    return DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
        &profile->input_command_queue, command, x, y);
}

int csb_v1_runtime_process_one_input_command(
    CSB_V1_RuntimeProfile *profile,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction)
{
    int target_dir;

    if (!profile) return -1;
    memset(&profile->last_input_dispatch, 0,
           sizeof(profile->last_input_dispatch));

    /* Source-lock: ReDMCSB COMMAND.C F0380 lines 2075-2127 locks the
     * command queue, applies the movement-disabled gate, dequeues one
     * command, and lines 2150-2156 dispatch turns to CLIKMENU.C F0365 or
     * steps to F0366.  This CSB runtime boundary currently wires only the
     * F0365 turn route into live CSB state; step commands are reported as
     * dequeued/recognized but intentionally not applied here. */
    profile->last_input_dispatch = DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
        &profile->input_command_queue,
        profile->party_dir,
        disabled_movement_ticks,
        projectile_disabled_movement_ticks,
        last_projectile_disabled_movement_direction);

    if (!profile->last_input_dispatch.dequeued) {
        return 0;
    }

    profile->input_dispatch_count++;
    target_dir = csb_v1_runtime_input_command_target_dir(
        profile->party_dir, profile->last_input_dispatch.command);
    if (target_dir >= 0) {
        /* Source-lock: ReDMCSB CLIKMENU.C F0365 lines 156-173 turns
         * C001/C002 into (party_dir + 3/+1) & 3 and calls CHAMPION.C F0284
         * lines 117-130 to rotate every champion Cell/Direction before
         * committing G0308_i_PartyDirection. */
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            return -1;
        }
    }
    return 1;
}

int csb_v1_runtime_get_last_input_dispatch(
    const CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputQueueProcessResultPc34Compat *out_result)
{
    if (!profile || !out_result) return -1;
    *out_result = profile->last_input_dispatch;
    return out_result->dequeued;
}

static int csb_v1_runtime_first_living_champion(const CSB_V1_PartyState *party)
{
    int i;
    if (!party) return -1;
    for (i = 0; i < party->ChampionCount && i < CSB_V1_MAX_CHAMPIONS; i++) {
        if (!csb_v1_champion_is_dead(&party->Champions[i]) &&
            party->Champions[i].CurrentHealth > 0) {
            return i;
        }
    }
    return -1;
}

int csb_v1_runtime_set_party_state(CSB_V1_RuntimeProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    int leader;
    if (!profile || !party) return -1;
    if (party->ChampionCount < 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    profile->party_state = *party;
    profile->party_state_valid = 1;
    profile->champion_count = party->ChampionCount;
    profile->party_dir = party->PartyDirection & 3;
    profile->magic_caster_index = party->MagicCasterIndex;

    leader = party->LeaderIndex;
    if (leader < 0 || leader >= party->ChampionCount ||
        csb_v1_champion_is_dead(&party->Champions[leader]) ||
        party->Champions[leader].CurrentHealth <= 0) {
        leader = csb_v1_runtime_first_living_champion(party);
    }
    profile->leader_index = leader;
    profile->party_state.LeaderIndex = leader;
    return 0;
}

int csb_v1_runtime_get_party_state(const CSB_V1_RuntimeProfile *profile,
                                   CSB_V1_PartyState *out_party)
{
    if (!profile || !out_party || !profile->party_state_valid) return -1;
    *out_party = profile->party_state;
    return out_party->ChampionCount;
}

int csb_v1_runtime_set_leader(CSB_V1_RuntimeProfile *profile,
                              int champion_index)
{
    CSB_V1_Champion *champion;
    if (!profile || !profile->party_state_valid) return -1;
    if (champion_index == profile->leader_index) return 0;
    if (champion_index < 0) {
        profile->leader_index = -1;
        profile->party_state.LeaderIndex = -1;
        return 0;
    }
    if (champion_index >= profile->party_state.ChampionCount ||
        champion_index >= CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }

    champion = &profile->party_state.Champions[champion_index];
    /* Mirrors the source-locked F0368 guard and selection side effect:
     * ignore dead/empty champions, then write G0411_i_LeaderIndex and align
     * the selected champion direction to G0308_i_PartyDirection.
     * Source: ReDMCSB CLIKCHAM.C F0368_COMMAND_SetLeader lines 51-68. */
    if (csb_v1_champion_is_dead(champion) || champion->CurrentHealth <= 0) {
        return -1;
    }
    profile->leader_index = champion_index;
    profile->party_state.LeaderIndex = champion_index;
    champion->Direction = (uint8_t)(profile->party_dir & 3);
    return 0;
}

int csb_v1_runtime_rotate_party(CSB_V1_RuntimeProfile *profile,
                                 int target_dir)
{
    /* Source: ReDMCSB CHAMPION.C F0284_CHAMPION_SetPartyDirection lines
     * 117-130.  The PC 3.4 C version (MEDIA182) computes
     *   delta = (P0600_i_Direction - G0308_i_PartyDirection); if delta<0
     *   then delta += 4;
     * then loops over G0305_ui_PartyChampionCount champions, applying
     *   Champion.Cell      = (Champion.Cell + delta) & 3;
     *   Champion.Direction = (Champion.Direction + delta) & 3;
     * and finally writes G0308_i_PartyDirection = P0600_i_Direction.
     * The M021_NORMALIZE() macro is just (x & 3). */
    int delta;
    int current_dir;
    int i;

    if (!profile) return -1;
    if (!profile->party_state_valid) return -1;
    if (target_dir < 0 || target_dir > 3) return -1;

    current_dir = profile->party_dir & 3;
    if (target_dir == current_dir) {
        /* Source-locked F0284 early return.  No champion state is
         * touched and the caller still gets 0. */
        return 0;
    }

    delta = target_dir - current_dir;
    if (delta < 0) delta += 4;

    for (i = 0; i < profile->party_state.ChampionCount &&
                i < CSB_V1_MAX_CHAMPIONS; i++) {
        uint8_t *cell = &profile->party_state.Champions[i].Cell;
        uint8_t *dir = &profile->party_state.Champions[i].Direction;
        *cell = (uint8_t)(((int)*cell + delta) & 3);
        *dir  = (uint8_t)(((int)*dir  + delta) & 3);
    }

    profile->party_dir = (uint8_t)target_dir;
    profile->party_state.PartyDirection = (uint8_t)target_dir;
    return 0;
}

int csb_v1_runtime_process_input_queue(
    CSB_V1_RuntimeProfile *profile,
    struct Dm1V1InputCommandQueuePc34Compat *queue,
    int disabled_movement_ticks,
    int projectile_disabled_movement_ticks,
    int last_projectile_disabled_movement_direction,
    CSB_V1_InputCommandRuntimeResult *out_result)
{
    CSB_V1_InputCommandRuntimeResult local_result;
    int target_dir;

    if (!profile || !queue) return -1;
    memset(&local_result, 0, sizeof(local_result));

    local_result.old_party_x = profile->party_x;
    local_result.old_party_y = profile->party_y;
    local_result.old_party_dir = profile->party_dir & 3;
    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;

    /* Source: ReDMCSB COMMAND.C F0380 lines 2045-2156 owns the command
     * queue dequeue/gate/dispatch boundary.  The shared V1 queue helper
     * preserves that C001/C002 vs C003-C006 split before this CSB runtime
     * adapter applies only the state transition that has a CSB profile
     * boundary today. */
    local_result.queue_result =
        DM1_V1_InputCommandQueue_ProcessOnePc34Compat(
            queue,
            profile->party_dir,
            disabled_movement_ticks,
            projectile_disabled_movement_ticks,
            last_projectile_disabled_movement_direction);

    if (!local_result.queue_result.dequeued) {
        if (out_result) *out_result = local_result;
        return 0;
    }

    switch (local_result.queue_result.command) {
    case DM1_V1_COMMAND_TURN_LEFT:
        target_dir = (profile->party_dir + 3) & 3;
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        break;
    case DM1_V1_COMMAND_TURN_RIGHT:
        target_dir = (profile->party_dir + 1) & 3;
        if (csb_v1_runtime_rotate_party(profile, target_dir) != 0) {
            local_result.unsupported_runtime_command = 1;
        }
        break;
    default:
        local_result.unsupported_runtime_command = 1;
        break;
    }

    local_result.new_party_x = profile->party_x;
    local_result.new_party_y = profile->party_y;
    local_result.new_party_dir = profile->party_dir & 3;
    local_result.runtime_state_changed =
        (local_result.old_party_x != local_result.new_party_x) ||
        (local_result.old_party_y != local_result.new_party_y) ||
        (local_result.old_party_dir != local_result.new_party_dir);

    if (out_result) *out_result = local_result;
    return 1;
}

void csb_v1_runtime_cleanup(CSB_V1_RuntimeProfile *profile) {
    if (!profile) return;
    /*
     * Unload the dungeon loaded by csb_v1_runtime_boot().
     * csb_v1_dungeon_unload() frees the dungeon data (raw_data,
     * dsa_offsets) via csb_v1_dungeon_free() and clears s_current_dungeon,
     * resetting dungeon-layer accessors to ENDOF.
     *
     * FIX (pass608): dungeon is now heap-allocated in csb_v1_runtime_boot()
     * and profile->dungeon_handle owns the allocation.  After calling
     * csb_v1_dungeon_unload() we also free(profile->dungeon_handle) to
     * release the heap struct and NULL the pointer.
     *
     * csb_v1_dungeon_free() does NOT free the struct itself (only inner
     * pointers), so free(dungeon_handle) is safe after csb_v1_dungeon_unload().
     */
    csb_v1_dungeon_unload();
    if (profile->dungeon_handle) {
        free(profile->dungeon_handle);
        profile->dungeon_handle = NULL;
    }
}


int csb_v1_runtime_boot(CSB_V1_RuntimeProfile *profile,
                          const char *data_dir,
                          const char *version_hint)
{
    CSB_V1_AssetResult dun_result;
    CSB_V1_AssetResult gfx_result;
    const char *dun_path;
    const char *gfx_path;
    const char *search_dir;

    if (!profile) return -1;

    search_dir = data_dir ? data_dir : ".",

    /* Step 1: Find dungeon by CSB hash (ReDMCSB DUNGEON.C F0237) */
    dun_path = csb_v1_runtime_find_dungeon(search_dir, &dun_result);
    if (!dun_path) return -1;
    profile->dungeon_path = dun_path;
    profile->dungeon_asset = dun_result;

    /* Step 1b: Load the dungeon data (CSB V1 Phase 2 — real asset ingestion)
     * Uses csb_v1_dungeon_load_from_file() to read the actual DUNGEON.DAT
     * into the current dungeon context.  Dungeon-layer accessors in
     * csb_v1_dungeon_world_pc34_compat.h become live after this.
     *
     * FIX (pass608): dungeon MUST be heap-allocated.  The previous
     * implementation created a stack-local CSB_V1_DungeonData and passed
     * &dungeon to csb_v1_dungeon_set_current().  After boot() returns,
     * the stack variable goes out of scope and s_current_dungeon becomes
     * a dangling pointer.  This is a critical memory safety bug.
     *
     * The dungeon_handle field in CSB_V1_RuntimeProfile owns the heap
     * allocation.  csb_v1_dungeon_set_current() transfers inner-data
     * ownership (raw_data/dsa_offsets) but the struct itself is freed
     * by the profile in csb_v1_runtime_cleanup().
     *
     * Source: CSBWin/CSBCode.cpp LoadDungeon lines 6800-6950 */
    {
        /* Heap-allocate to avoid dangling pointer in s_current_dungeon */
        CSB_V1_DungeonData *dungeon = calloc(1, sizeof(CSB_V1_DungeonData));
        if (!dungeon) {
            /* Fall through — dungeon-layer accessors return ENDOF */
        } else if (csb_v1_dungeon_load_from_file(dungeon, dun_path) == 0) {
            profile->dungeon_handle = dungeon;
            csb_v1_dungeon_set_current(dungeon); /* singleton now points to heap */
            csb_v1_dungeon_set_current_level(0);   /* start at level 0 */
        } else {
            free(dungeon);
            profile->dungeon_handle = NULL;
        }
        /* If load fails (corrupt/missing file), boot continues without dungeon.
         * Dungeon-layer accessors will return ENDOF until a dungeon is loaded. */
    }

    /* Step 2: Find graphics (ReDMCSB DISK.C / CSBWin AssetCache) */
    gfx_path = csb_v1_runtime_find_graphics(search_dir, version_hint, &gfx_result);
    profile->graphics_path = gfx_path ? gfx_path : "";
    profile->graphics_asset = gfx_result;

    /* Step 3: Detect variant from asset hashes */
    profile->variant_id = csb_v1_runtime_detect_variant(
        gfx_path, dun_path, NULL, NULL);

    /* Step 4: Initialize Chaos Magic spell grid (ReDMCSB CASTER.C F0211) */
    profile->chaos_magic.magic_initialized = 1;
    profile->chaos_magic.spell_grid_version = 0;
    profile->chaos_magic.chaos_level = 0;

    /* Step 5: Set initial state to TITLE (ReDMCSB ENTRANCE.C G0298) */
    profile->state = CSB_STATE_TITLE;

    return 0;
}

void csb_v1_runtime_tick(CSB_V1_RuntimeProfile *profile, uint32_t dt_ms)
{
    if (!profile || profile->paused) return;
    if (profile->game_over || profile->victory) return;

    profile->total_play_ms += dt_ms;

    /* The original runtime gates event expiry against G0313_ul_GameTime,
     * not against a single frame delta.  Accumulate wall time first, then
     * fire every due 55ms quantum so common frame slices such as 16+16+23ms
     * still produce one V1 tick.
     * Source: ReDMCSB TIMELINE.C F0235 lines 702-708
     * Source: ReDMCSB COMMAND.C F0380 lines 2383-2429
     * (C147/C148 toggle G0301_B_GameTimeTicking). */
    while (csb_v1_runtime_tick_due(profile, 0U)) {
        csb_v1_fire_tick(profile);
    }
}

int csb_v1_runtime_tick_v1(CSB_V1_RuntimeProfile *profile)
{
    if (!profile || profile->paused) return 0;
    if (profile->game_over || profile->victory) return 0;

    profile->total_play_ms += CSB_V1_TICK_MS_NOMINAL;
    csb_v1_fire_tick(profile);
    return 1;
}

int csb_v1_runtime_tick_due(const CSB_V1_RuntimeProfile *profile, uint32_t now_ms)
{
    uint64_t wall_ms;
    uint64_t game_ticks_now;
    if (!profile) return 0;

    wall_ms = (now_ms != 0U) ? (uint64_t)now_ms : profile->total_play_ms;
    game_ticks_now = wall_ms / CSB_V1_TICK_MS_NOMINAL;
    return (profile->tick_count < game_ticks_now) ? 1 : 0;
}

/* ── Source evidence ────────────────────────────────────────────────── */

const char *csb_v1_runtime_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C: F0806_F0806_ENTRANCE_int (game boot)\n"
        "ReDMCSB ENTRANCE.C: F0807_ENTRANCE_DrawAnimationStep (intro animation)\n"
        "ReDMCSB ENTRANCE.C: F0579_ENTRANCE_InitializeBitPlanes (graphics)\n"
        "ReDMCSB ENTRANCE.C: F0580_ENTRANCE_DrawDoorAnimationStep\n"
        "ReDMCSB ENTRANCE.C: F0581_ENTRANCE_BlitDoors\n"
        "ReDMCSB ENTRANCE.C: C28_ENTRANCE_CSB palette index\n"
        "ReDMCSB ENTRANCE.C: G0298_B_NewGame state machine control\n"
        "ReDMCSB ENTRANCE.C: G0309_i_PartyMapIndex init (party start)\n"
        "ReDMCSB ENTRANCE.C: MEDIA529_F20E_F20J save path decision\n"
        "ReDMCSB ENTRANCE.C: M567_COMMAND_ENTRANCE_DRAW_CREDITS\n"
        "ReDMCSB SAVEHEAD.C: F0429_IsReadSaveHeaderSuccessful\n"
        "ReDMCSB SAVEHEAD.C: F0430_IsWriteObfuscatedSaveHeaderSuccessful\n"
        "ReDMCSB LOADSAVE.C: F0435_STARTEND_LoadGame\n"
        "ReDMCSB LOADSAVE.C: F0433_STARTEND_ProcessCommand140_SaveGame\n"
        "ReDMCSB DUNGEON.C: F0237_DUNGEON_DungeonLoad (hash-gated load)\n"
        "ReDMCSB CASTER.C: F0211_CASTER_ClearSpellEffects (spell grid boot)\n"
        "ReDMCSB CASTER.C: F0213 per-square invocation slots\n"
        "ReDMCSB BugsAndChanges.htm: CHANGE7_29 (new CSBGAME.DAT header)\n"
        "ReDMCSB CEDTINC7.C: G3764_THAT_S_THE_CSB_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTIL_DISK\n"
        "ReDMCSB CEDTINC8.C: G3921/G3755/G3764 utility disk strings\n"
        "ReDMCSB F0417: F0417_SAVEUTIL_GetChecksumAndObfuscate\n"
        "ReDMCSB COMPILE.H MEDIA332 (S20E/S21E Atari ST 2.0/2.1)\n"
        "ReDMCSB COMPILE.H MEDIA529 (A35E/A35M Amiga 3.5)\n"
        "ReDMCSB COMPILE.H MEDIA278 (P20JA/P20JB PC DOS 3.4)\n"
        "ReDMCSB COMPILE.H MEDIA278_I34E_I34M (PC DOS multilanguage)\n"
        "CSBWin SaveGame.cpp: LoadGame() / SaveGame() (2953 lines)\n"
        "CSBWin Character.cpp: Character::import_dm1_save()\n"
        "CSBWin Magic.cpp: ChaosMagic namespace\n"
        "CSBWin AssetCache: variant_id mapping for all platforms\n"
        "asset_status_m12.c: g_csbVersions[] MD5 table (all 4 variants)\n"
        "asset_find_by_hash.c: hash-based asset discovery API\n"
        "\n"
        "CSB vs DM1 runtime differences:\n"
        "  - Dungeon hash: 6695d2acebce49f95db1d8f3a5c733de (CSB)\n"
        "  - Save namespace: csb_save_N.fsav (CSB) vs save_NN.dat (DM1)\n"
        "  - Save header: CSB_MAGIC 0x43534201 (CSB) vs DM_MAGIC 0x444D0001\n"
        "  - Save key index: C29 (CSB) vs C10 (DM1) per MEDIA187/MEDIA332\n"
        "  - Difficulty scale: +25% per champion (CSB) vs DM1 flat\n"
        "  - Chaos Magic: present at CSB boot (F0211)\n"
        "  - Entry: same ENTRANCE, C28_ENTRANCE_CSB palette\n"
        "  - Champion import: F0153 from DM1 save supported at CSB boot\n";
}
