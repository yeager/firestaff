#include "csb_v1_startup_session_contract_pc34_compat.h"
#include "vga_palette_pc34_compat.h"

#include <string.h>

#define CSB_V1_CONTRACT_C017_WIDTH_PC34 224
#define CSB_V1_CONTRACT_C017_HEIGHT_PC34 136
#define CSB_V1_CONTRACT_C040_WIDTH_PC34 144
#define CSB_V1_CONTRACT_C040_HEIGHT_PC34 73
#define CSB_V1_CONTRACT_C040_TRANSPARENT_PC34 6

int csb_v1_startup_session_terminal_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupSessionTerminalReceipt_PC34 *out_receipt)
{
    const CSB_V1_StartupRuntimeSurface_PC34 *c017;
    const CSB_V1_StartupRuntimeSurface_PC34 *c040;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!session || !out_receipt || !session->valid ||
        !session->real_asset_matched || !session->title_presents_ready ||
        !session->title_chaos_ready || !session->title_strikes_back_ready ||
        !session->entrance_assets_ready || !session->door_assets_ready ||
        !session->rejects_legacy_wrappers ||
        !session->playback.no_fallback_routes ||
        session->playback.title_phase_mask != 0x0b ||
        session->playback.stage != CSB_V1_STARTUP_PLAYBACK_STAGE_HUD_PC34 ||
        !session->playback.entrance_complete ||
        !session->hud_assets_bound || session->generation == 0u) return 0;
    c017 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_INVENTORY_PC34];
    c040 = &session->surfaces.surfaces[
        CSB_V1_STARTUP_RUNTIME_SURFACE_HUD_RESURRECT_PC34];
    if (!c017->valid || !c017->pixels || c017->source_asset_id != 17 ||
        c017->width != CSB_V1_CONTRACT_C017_WIDTH_PC34 ||
        c017->height != CSB_V1_CONTRACT_C017_HEIGHT_PC34 ||
        c017->transparent_color != -1 || !c040->valid || !c040->pixels ||
        c040->source_asset_id != 40 ||
        c040->width != CSB_V1_CONTRACT_C040_WIDTH_PC34 ||
        c040->height != CSB_V1_CONTRACT_C040_HEIGHT_PC34 ||
        c040->transparent_color !=
            CSB_V1_CONTRACT_C040_TRANSPARENT_PC34) return 0;
    out_receipt->valid = 1;
    out_receipt->c001_complete = 1;
    out_receipt->terminal_f0807_complete = 1;
    out_receipt->c017_ready = 1;
    out_receipt->c040_ready = 1;
    out_receipt->source_tick = session->source_tick;
    out_receipt->session_generation = session->generation;
    return 1;
}

int csb_v1_startup_session_live_hud_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionTerminalReceipt_PC34 *terminal_receipt,
    unsigned int c040_clear_count,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionLiveHudReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB PANEL.C F0346 clears the candidate C040 surface once; F0347
     * then restores the normal C017 panel. The return cannot borrow a later
     * session or tick, even if its source assets still happen to match. */
    if (!session || !terminal_receipt || !out_receipt ||
        c040_clear_count != 1u || !terminal_receipt->valid ||
        !terminal_receipt->c017_ready || !terminal_receipt->c040_ready ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        source_tick != terminal_receipt->source_tick ||
        session_generation != terminal_receipt->session_generation ||
        current.source_tick != terminal_receipt->source_tick ||
        current.session_generation != terminal_receipt->session_generation) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->c040_cleared_once = 1;
    out_receipt->c017_live_base_only = 1;
    out_receipt->c017_source_asset_id = 17;
    out_receipt->c017_width = CSB_V1_CONTRACT_C017_WIDTH_PC34;
    out_receipt->c017_height = CSB_V1_CONTRACT_C017_HEIGHT_PC34;
    out_receipt->special_palette = -1;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    unsigned int previous_door_step,
    unsigned int door_step,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionDoorHudTickReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB DUNGEON.C advances the live door state by one tick. The first
     * action after C040's clear begins a new runtime door at step zero; it
     * cannot replay F0807 or consume a newer C017 session. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        previous_door_step >= 31u || door_step != previous_door_step + 1u ||
        source_tick <= live_hud_receipt->source_tick ||
        door_step != source_tick - live_hud_receipt->source_tick ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_live_door_tick = previous_door_step == 0u ? 1 : 0;
    out_receipt->previous_door_step = previous_door_step;
    out_receipt->door_step = door_step;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_input_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionMovementCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionInputReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB COMMAND.C consumes the first movement only after PANEL.C
     * restores C017. Keep this action in the immediate next source tick;
     * a delayed command must re-enter through a later runtime receipt. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        command <= CSB_V1_STARTUP_SESSION_MOVEMENT_NONE_PC34 ||
        command > CSB_V1_STARTUP_SESSION_MOVEMENT_TURN_RIGHT_PC34 ||
        source_tick != live_hud_receipt->source_tick + 1u ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_post_c040_input = 1;
    out_receipt->command = command;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_first_action_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionActionCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    /* ReDMCSB COMMAND.C follows PANEL.C's C040 clear with C017-owned hand
     * or cast dispatch. This first action is bound to exactly the next tick,
     * preventing a stale action surface from crossing a session boundary. */
    if (!session || !live_hud_receipt || !out_receipt ||
        !live_hud_receipt->valid || !live_hud_receipt->c040_cleared_once ||
        !live_hud_receipt->c017_live_base_only ||
        command <= CSB_V1_STARTUP_SESSION_ACTION_NONE_PC34 ||
        command > CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34 ||
        source_tick != live_hud_receipt->source_tick + 1u ||
        session_generation != live_hud_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->first_post_live_hud_action = 1;
    out_receipt->command = command;
    out_receipt->c017_source_asset_id = 17;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}

int csb_v1_startup_session_selection_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    const CSB_V1_StartupSessionActionReceipt_PC34 *action_receipt,
    CSB_V1_StartupSessionSelectionKind_PC34 kind,
    int selection_index,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionSelectionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupSessionTerminalReceipt_PC34 current;
    int action_slot;
    int spell_rune;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    action_slot = kind == CSB_V1_STARTUP_SESSION_SELECTION_ACTION_SLOT_PC34 &&
        selection_index >= 0 && selection_index < 4 &&
        (action_receipt &&
         (action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_LEFT_HAND_PC34 ||
          action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_RIGHT_HAND_PC34));
    spell_rune = kind == CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34 &&
        selection_index >= 1 && selection_index <= 6 && action_receipt &&
        action_receipt->command == CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34;
    /* ReDMCSB COMMAND.C resolves spell/action selection after the action
     * command. A selection cannot borrow an earlier action or a later C017
     * session merely because its visual panel remains present. */
    if (!session || !live_hud_receipt || !action_receipt || !out_receipt ||
        !live_hud_receipt->valid || !action_receipt->valid ||
        (!action_slot && !spell_rune) ||
        action_receipt->source_tick != live_hud_receipt->source_tick + 1u ||
        action_receipt->session_generation != live_hud_receipt->session_generation ||
        source_tick != action_receipt->source_tick + 1u ||
        session_generation != action_receipt->session_generation ||
        session->source_tick != source_tick ||
        session->generation != session_generation ||
        !csb_v1_startup_session_terminal_receipt_pc34(session, &current) ||
        current.session_generation != session_generation) return 0;
    out_receipt->valid = 1;
    out_receipt->kind = kind;
    out_receipt->selection_index = selection_index;
    out_receipt->source_tick = source_tick;
    out_receipt->session_generation = session_generation;
    return 1;
}
