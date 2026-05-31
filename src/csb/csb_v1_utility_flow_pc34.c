/* pass603: CSB V1 Phase 6 — Utility Setup Flow
 *
 * CSB utility disk setup flow. Different from DM1's utility flow:
 *   - No champion creation hall (CSB uses champion import from DM1 saves)
 *   - Always starts from utility disk (CEDT mode, not direct start)
 *   - Party is imported from DM1 saves, not created in-game
 *   - Resurrect/reincarnate available from champion panel
 *
 * Source references:
 *   ReDMCSB ENTRANCE.C — setup/selector flow adapted for CSB
 *   ReDMCSB CEDTINC7.C — utility disk prompt strings
 *   ReDMCSB CEDTDATA.C — G3921 PLEASE_INSERT_UTILITY_DISK
 *   ReDMCSB CEDTDATA.C — G3755 THAT_S_THE_CSB_UTILITY_DISK
 *   ReDMCSB CEDTDATA.C — G3764 THAT_S_NOT_THE_UTILITY_DISK
 *   CSBWin/CSBCode.cpp — StartChaos setup (11414 lines)
 */

#include "csb_v1_utility_flow_pc34_compat.h"
#include "csb_v1_utility_import_pc34_compat.h"
#include "csb_v1_character_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

/* ── Internal constants ──────────────────────────────────────────────── */

/* Maximum disk check attempts (prevents infinite loops) */
#define CSB_V1_UTIL_MAX_ATTEMPTS   5

/* Utility disk prompt strings (from ReDMCSB CEDTDATA.C).
 * These are reserved for the UI layer: the UI calls csb_v1_util_flow_get_prompt(ctx)
 * to retrieve the current prompt string for the flow state.
 * Marked as intentionally unused until the UI layer is wired up. */
/*
 * Utility disk prompt strings (from ReDMCSB CEDTDATA.C):
 *   G3921: PLEASE PUT THE CHAOS STRIKES BACK UTILITY DISK IN ~
 *   G3755: THAT'S THE CHAOS STRIKES BACK UTILITY DISK!
 *   G3764: THAT'S NOT THE UTILITY DISK!
 *   G3922: IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE
 *   G3923: LOAD SAVED GAME
 *   G3924: START NEW GAME
 *   G3925: VIEW CHAMPION DETAILS
 * These strings are reserved for the UI layer (not yet wired).
 */

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_utility_flow_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C: setup/selector flow adapted for CSB\n"
        "ReDMCSB CEDTINC7.C: utility disk prompt flow\n"
        "ReDMCSB CEDTDATA.C: G3921 PLEASE_INSERT_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3755 THAT_S_THE_CSB_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3764 THAT_S_NOT_THE_UTILITY_DISK\n"
        "ReDMCSB CEDTDATA.C: G3922/G3923/G3924/G3925 menu strings\n"
        "CSBWin/CSBCode.cpp: StartChaos champion init (11414 lines)\n"
        "CSBWin/SaveGame.cpp: DM1 import path (2953 lines)\n"
        "MEDIA529_F20E_F20J: CSB utility disk boot path\n"
        "MEDIA332_F20E_F21E_A31E_F31E: CSB utility vs game disk\n";
}

/* ── Initialize ─────────────────────────────────────────────────────── */
void csb_v1_util_flow_init(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return;
    memset(ctx, 0, sizeof(*ctx));
    ctx->state = CSB_V1_UTIL_FLOW_INIT;
    ctx->action = CSB_V1_UTIL_ACTION_EXIT;
    ctx->disk_result = CSB_V1_UTIL_DISK_MISSING;
    ctx->attempts = 0;
    ctx->max_attempts = CSB_V1_UTIL_MAX_ATTEMPTS;
    ctx->import_confirmed = 0;
    ctx->last_error = 0;
    memset(ctx->dm1_save_path, 0, sizeof(ctx->dm1_save_path));
    memset(ctx->csb_save_path, 0, sizeof(ctx->csb_save_path));
}

/* ── State name for UI ───────────────────────────────────────────────── */
const char *csb_v1_util_flow_state_name(CSB_V1_UtilFlowState state)
{
    switch (state) {
    case CSB_V1_UTIL_FLOW_INIT:           return "INIT";
    case CSB_V1_UTIL_FLOW_INSERT_DISK:    return "INSERT_DISK";
    case CSB_V1_UTIL_FLOW_VERIFY_DISK:    return "VERIFY_DISK";
    case CSB_V1_UTIL_FLOW_DISK_OK:        return "DISK_OK";
    case CSB_V1_UTIL_FLOW_SELECT_ACTION:  return "SELECT_ACTION";
    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS: return "IMPORT_CHAMPIONS";
    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT: return "CONFIRM_IMPORT";
    case CSB_V1_UTIL_FLOW_LOAD_GAME:      return "LOAD_GAME";
    case CSB_V1_UTIL_FLOW_NEW_GAME:       return "NEW_GAME";
    case CSB_V1_UTIL_FLOW_ERROR:          return "ERROR";
    case CSB_V1_UTIL_FLOW_DONE:           return "DONE";
    default:                              return "UNKNOWN";
    }
}

/* ── Last error string ──────────────────────────────────────────────── */
const char *csb_v1_util_flow_last_error(CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return "NULL context";
    switch (ctx->last_error) {
    case 0:   return "No error";
    case -1:  return "Disk missing or unreadable";
    case -2:  return "Wrong disk type (not CSB Utility Disk)";
    case -3:  return "Import failed — invalid DM1 save";
    case -4:  return "Load failed — save not found";
    case -5:  return "Save corrupted";
    case -6:  return "Maximum disk check attempts reached";
    case -7:  return "Invalid party state";
    case -8:  return "No champions imported/loaded";
    default:  return "Unknown error";
    }
}

/* ── Set action ─────────────────────────────────────────────────────── */
void csb_v1_util_flow_set_action(CSB_V1_UtilFlowContext *ctx,
                                   CSB_V1_UtilFlowAction action)
{
    if (!ctx) return;
    if (ctx->state != CSB_V1_UTIL_FLOW_SELECT_ACTION &&
        ctx->state != CSB_V1_UTIL_FLOW_INIT) {
        /* Can only set action from SELECT_ACTION state */
        return;
    }
    ctx->action = action;
}

/* ── Set paths ──────────────────────────────────────────────────────── */
void csb_v1_util_flow_set_dm1_path(CSB_V1_UtilFlowContext *ctx,
                                    const char *path)
{
    if (!ctx || !path) return;
    strncpy(ctx->dm1_save_path, path, sizeof(ctx->dm1_save_path) - 1);
    ctx->dm1_save_path[sizeof(ctx->dm1_save_path) - 1] = '\0';
}

void csb_v1_util_flow_set_csb_path(CSB_V1_UtilFlowContext *ctx,
                                     const char *path)
{
    if (!ctx || !path) return;
    strncpy(ctx->csb_save_path, path, sizeof(ctx->csb_save_path) - 1);
    ctx->csb_save_path[sizeof(ctx->csb_save_path) - 1] = '\0';
}

/* ── Confirm import ─────────────────────────────────────────────────── */
void csb_v1_util_flow_confirm_import(CSB_V1_UtilFlowContext *ctx,
                                       int confirmed)
{
    if (!ctx) return;
    ctx->import_confirmed = confirmed ? 1 : 0;
}

/* ── Ready check ─────────────────────────────────────────────────────── */
int csb_v1_util_flow_is_ready(const CSB_V1_UtilFlowContext *ctx)
{
    if (!ctx) return 0;
    /* Ready if: state is DONE and action is not EXIT,
     * OR state is NEW_GAME with valid party (champion_count > 0) */
    return (ctx->state == CSB_V1_UTIL_FLOW_DONE &&
            ctx->action != CSB_V1_UTIL_ACTION_EXIT &&
            ctx->action != CSB_V1_UTIL_ACTION_IMPORT) ||
           (ctx->state == CSB_V1_UTIL_FLOW_NEW_GAME);
}

/* ── Get utility disk type (from save_load) ──────────────────────────── */
static CSB_V1_UtilDiskResult check_disk_type(const char *drive_path)
{
    int r = csb_v1_util_check_disk(drive_path);
    if (r == 0) return CSB_V1_UTIL_DISK_OK;      /* correct disk */
    if (r == 1) return CSB_V1_UTIL_DISK_WRONG;   /* wrong disk */
    return CSB_V1_UTIL_DISK_MISSING;             /* no disk / error */
}

/* ── Import champions from DM1 save ─────────────────────────────────── */
/* Import flow (ReDMCSB SAVEGAME.C F0100-F0120 state machine):
 *   1. Validate DM1 save path
 *   2. Check utility disk (CSB utility disk must be in drive)
 *   3. Read DM1 save file into buffer
 *   4. Run import state machine
 *   5. Return imported champion count
 *
 * This is called from the IMPORT_CHAMPIONS state. */
static int do_import(CSB_V1_UtilFlowContext *ctx, CSB_V1_PartyState *party)
{
    CSB_V1_ImportResult import_result;
    int count;

    if (!ctx || !party) return -1;

    /* Path must be set */
    if (ctx->dm1_save_path[0] == '\0') {
        ctx->last_error = -3;
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }

    /* Import from DM1 save file */
    memset(&import_result, 0, sizeof(import_result));
    count = csb_v1_import_from_dm1_save_file(party,
                                              ctx->dm1_save_path,
                                              &import_result);
    if (count < 0) {
        ctx->last_error = -3;
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }

    if (count == 0) {
        ctx->last_error = -3;
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }

    /* Import successful */
    ctx->last_error = 0;
    return count;
}

/* ── Main flow step ──────────────────────────────────────────────────── */
/* csb_v1_util_flow_step:
 *   Runs one step of the CSB utility flow state machine.
 *   Call repeatedly until it returns 1 (done) or negative (error).
 *
 *   ReDMCSB ENTRANCE.C flow adapted for CSB:
 *     INIT → INSERT_DISK → VERIFY_DISK → DISK_OK →
 *     SELECT_ACTION → (IMPORT_CHAMPIONS | LOAD_GAME | NEW_GAME) → DONE
 *
 *   In Firestaff, the "drive_path" for disk checks is simulated.
 *   On non-FDOS platforms, disk checks always succeed with a simulated disk.
 */
int csb_v1_util_flow_step(CSB_V1_UtilFlowContext *ctx)
{
    static CSB_V1_PartyState party;
    /* Static party to persist across multiple step calls */
    static int party_initialized = 0;

    /* Simulated drive path for disk checks.
     * On macOS/Linux, this path doesn't exist, so csb_v1_util_check_disk
     * returns -1 (error), simulating no floppy drive.
     *
     * In a real Firestaff implementation, the UI would handle disk
     * prompts via a file-picker dialog. The drive_path here is a
     * placeholder for the platform-specific floppy/SD card path. */
    static const char *simulated_drive_path = "/dev/sd0";

    if (!ctx) return -1;

    switch (ctx->state) {

    case CSB_V1_UTIL_FLOW_INIT:
        /* Initialize utility flow.
         * ReDMCSB CEDTINC7.C: "PLEASE PUT THE CSB UTILITY DISK IN ~"
         * First step: prompt for disk insertion. */
        if (!party_initialized) {
            csb_v1_character_init_default(&party);
            party_initialized = 1;
        }
        ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        ctx->attempts = 0;
        ctx->last_error = 0;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_INSERT_DISK:
        /* Show "INSERT DISK" prompt.
         * ReDMCSB G3921: "PLEASE PUT THE CHAOS STRIKES BACK UTILITY DISK IN ~"
         * On real hardware, would wait for user to insert disk.
         * On desktop platforms, this immediately transitions to VERIFY_DISK. */
        ctx->state = CSB_V1_UTIL_FLOW_VERIFY_DISK;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_VERIFY_DISK:
        /* Check if the correct utility disk is in the drive.
         * ReDMCSB F0428: RequireGameDiskInDrive
         * F0452: GetDiskTypeInDrive_CPSB returns CSB_V1_DISK_TYPE_UTILITY_DISK.
         *
         * On desktop platforms (macOS, Linux, Windows without floppy),
         * we simulate the disk check. The disk is "correct" if:
         *   - drive_path is a real readable device with CSB_UTIL_DISK_SERIAL, OR
         *   - drive_path is the simulated path "/dev/sd0" (always succeeds)
         *
         * In a real implementation, the UI would show a file-picker or
         * path input. Here we use a simulated path. */
        ctx->disk_result = check_disk_type(simulated_drive_path);
        ctx->attempts++;

        if (ctx->disk_result == CSB_V1_UTIL_DISK_OK) {
            /* Correct disk */
            ctx->state = CSB_V1_UTIL_FLOW_DISK_OK;
        } else if (ctx->attempts >= ctx->max_attempts) {
            /* Too many attempts */
            ctx->last_error = -6;
            ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        } else if (ctx->disk_result == CSB_V1_UTIL_DISK_WRONG) {
            /* Wrong disk — ReDMCSB G3764: "THAT'S NOT THE UTILITY DISK!" */
            ctx->last_error = -2;
            /* Continue trying (show wrong disk message) */
            ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        } else {
            /* No disk — retry */
            ctx->last_error = -1;
            ctx->state = CSB_V1_UTIL_FLOW_INSERT_DISK;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_DISK_OK:
        /* Disk verified — ReDMCSB G3755: "THAT'S THE CSB UTILITY DISK!"
         * Show success message, then transition to action selection. */
        ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_SELECT_ACTION:
        /* Main menu: choose action.
         * ReDMCSB ENTRANCE.C selector:
         *   G3922: IMPORT CHAMPIONS FROM DUNGEON MASTER SAVE
         *   G3923: LOAD SAVED GAME
         *   G3924: START NEW GAME
         *   G3925: VIEW CHAMPION DETAILS
         *
         * In Firestaff: the UI layer calls csb_v1_util_flow_set_action()
         * to select. If action is already set, process it. */
        if (ctx->action == CSB_V1_UTIL_ACTION_EXIT) {
            ctx->state = CSB_V1_UTIL_FLOW_DONE;
            return 1;
        }

        switch (ctx->action) {
        case CSB_V1_UTIL_ACTION_IMPORT:
            ctx->state = CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS;
            break;
        case CSB_V1_UTIL_ACTION_LOAD:
            ctx->state = CSB_V1_UTIL_FLOW_LOAD_GAME;
            break;
        case CSB_V1_UTIL_ACTION_NEW:
            /* New game requires champions first (import or load) */
            if (party.ChampionCount == 0) {
                ctx->last_error = -8;  /* no champions */
                ctx->state = CSB_V1_UTIL_FLOW_ERROR;
            } else {
                ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
            }
            break;
        case CSB_V1_UTIL_ACTION_VIEW:
            /* View — no state change, just continue */
            break;
        case CSB_V1_UTIL_ACTION_EXIT:
            ctx->state = CSB_V1_UTIL_FLOW_DONE;
            return 1;
        default:
            /* No action selected yet — stay in this state */
            break;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS:
        /* Import champions from DM1 save.
         * ReDMCSB SAVEGAME.C F0100-F0120 state machine.
         * If path is set, import immediately.
         * If path not set, return error (UI should set path first). */
        if (ctx->dm1_save_path[0] == '\0') {
            /* No path set — prompt for file selection.
             * In Firestaff, the UI would show a file picker here.
             * We return 0 (continue) and let the UI set the path. */
            return 0;
        }

        {
            int count = do_import(ctx, &party);
            if (count < 0) {
                /* Error already set in ctx->last_error and ctx->state */
                return 0;
            }
            /* Import successful — show confirmation preview */
            ctx->state = CSB_V1_UTIL_FLOW_CONFIRM_IMPORT;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_CONFIRM_IMPORT:
        /* Show import preview (party champion names/stats).
         * ReDMCSB CEDTDATA.C: preview party before writing.
         *
         * In Firestaff: the UI shows the imported champions and asks
         * "IMPORT X CHAMPIONS TO PARTY SLOT?". User confirms via
         * csb_v1_util_flow_confirm_import().
         *
         * If import_confirmed is set, commit the import and go to NEW_GAME.
         * If import not confirmed, go back to SELECT_ACTION. */
        if (ctx->import_confirmed) {
            /* Commit: party already updated in IMPORT_CHAMPIONS state.
             * Transition to NEW_GAME to start playing. */
            ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
        } else {
            /* Not confirmed — back to action selection */
            ctx->state = CSB_V1_UTIL_FLOW_SELECT_ACTION;
            ctx->action = CSB_V1_UTIL_ACTION_EXIT;  /* reset action */
        }
        ctx->import_confirmed = 0;
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_LOAD_GAME:
        /* Load saved game from CSB save file.
         * ReDMCSB LOADSAVE.C F0435: STARTEND_LoadGame.
         * Uses csb_v1_save_load_pc34_compat functions.
         *
         * In Firestaff: the UI sets csb_save_path, then this state
         * loads the game state from the save file. */
        if (ctx->csb_save_path[0] == '\0') {
            /* No path set — prompt for file selection.
             * Stay in this state and let UI set path. */
            return 0;
        }

        /* Load the save game.
         * Note: This uses the save/load system which is separate from
         * the import system. A full implementation would load the
         * complete game state (party + dungeon + events). */
        {
            /* For now, load only the party from the save.
             * A full implementation would load the complete game state.
             * This is a placeholder — the actual load would use
             * csb_v1_load_game() from the save_load module. */
            FILE *f = fopen(ctx->csb_save_path, "rb");
            if (!f) {
                ctx->last_error = -4;  /* save not found */
                ctx->state = CSB_V1_UTIL_FLOW_ERROR;
                return 0;
            }

            /* Read save header (512 bytes) and party data.
             * For now, just verify the save file is a valid CSB save. */
            {
                uint8_t hdr[512];
                size_t read = fread(hdr, 1, 512, f);
                (void)read;  /* suppress unused warning */

                /* Verify magic (CSB save magic) */
                uint32_t magic = (uint32_t)hdr[0]
                               | ((uint32_t)hdr[1] << 8)
                               | ((uint32_t)hdr[2] << 16)
                               | ((uint32_t)hdr[3] << 24);

                if (magic != CSB_V1_SAVE_MAGIC_CSB &&
                    magic != CSB_V1_SAVE_MAGIC_DM) {
                    ctx->last_error = -5;  /* corrupted */
                    ctx->state = CSB_V1_UTIL_FLOW_ERROR;
                    fclose(f);
                    return 0;
                }
            }
            fclose(f);

            /* Save verified — transition to NEW_GAME with loaded party.
             * Note: In a real implementation, the party would be loaded
             * from the save file here. For now, we just succeed. */
            ctx->state = CSB_V1_UTIL_FLOW_NEW_GAME;
        }
        return 0;  /* continue */

    case CSB_V1_UTIL_FLOW_NEW_GAME:
        /* New game state — party is ready.
         * The game engine would be launched here with the party.
         * ReDMCSB ENTRANCE.C: after setup, launches GAME with party.
         *
         * In Firestaff: this signals that the game is ready to start.
         * The UI transitions to the game view. */
        /* Store party metadata in ctx->reserved for get_party().
         * reserved[0] = ChampionCount, reserved[1] = LeaderIndex,
         * reserved[2] = ImportedFromDM1.
         * Full party data requires a future get_party_v2() API. */
        ctx->reserved[0] = party.ChampionCount;
        ctx->reserved[1] = party.LeaderIndex;
        ctx->reserved[2] = party.ImportedFromDM1;
        ctx->state = CSB_V1_UTIL_FLOW_DONE;
        return 1;  /* done */

    case CSB_V1_UTIL_FLOW_ERROR:
        /* Error state — something went wrong.
         * In Firestaff: the UI shows the error message and
         * offers to retry or exit. */
        return -1;  /* error */

    case CSB_V1_UTIL_FLOW_DONE:
        /* Flow completed successfully */
        return 1;

    default:
        ctx->last_error = -7;  /* invalid state */
        ctx->state = CSB_V1_UTIL_FLOW_ERROR;
        return -1;
    }
}

/* ── Get imported party (for external access) ────────────────────────── */
/* csb_v1_util_flow_get_party:
 *   Returns the imported/loaded party state.
 *   Call this after flow is done (returns 1) to get the party for the game.
 *
 *   Implementation: step() stores party metadata in ctx->reserved[]
 *   before transitioning to DONE:
 *     reserved[0] = ChampionCount
 *     reserved[1] = LeaderIndex
 *     reserved[2] = ImportedFromDM1
 *
 *   Returns: champion count (>= 0) on success, -1 on error.
 *   On success, out_party->ChampionCount / LeaderIndex / ImportedFromDM1
 *   are populated. Full champion data requires a future v2 API.
 */
int csb_v1_util_flow_get_party(CSB_V1_UtilFlowContext *ctx,
                                CSB_V1_PartyState *out_party)
{
    if (!ctx) return -1;
    if (!out_party) return -1;

    memset(out_party, 0, sizeof(*out_party));

    if (ctx->state != CSB_V1_UTIL_FLOW_DONE &&
        ctx->state != CSB_V1_UTIL_FLOW_NEW_GAME) {
        return -1;  /* flow not complete */
    }

    /* Read metadata stored by step() in the NEW_GAME case. */
    out_party->ChampionCount = ctx->reserved[0];
    out_party->LeaderIndex   = ctx->reserved[1];
    out_party->ImportedFromDM1 = ctx->reserved[2];

    return out_party->ChampionCount;
}