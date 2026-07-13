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
