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
