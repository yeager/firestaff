#include "csb_v1_boot.h"

#include "asset_find_by_hash.h"
#include "csb_v1_cmp_import_pc34_compat.h"
#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_m11_runtime_plan.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_engine_version_display_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"
#include "entrance_mouse_routes_pc34_compat.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ReDMCSB source-lock for this boot/profile boundary:
 * ENTRANCE.C F0806 lines 409-441 builds the entrance micro-dungeon and
 * selects C28_ENTRANCE_CSB for CSB media.
 * ENTRANCE.C F0806 lines 857-883 waits on the entrance state machine and
 * switches G0298_B_NewGame to C001_MODE_LOAD_DUNGEON.
 * LOADSAVE.C F0435 lines 1940-1944 loads the initial party location from
 * DUNGEON.DAT and sets G0309_i_PartyMapIndex to map 0 for new games.
 */

static const char *const g_csb_boot_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611",
    "ebf6a57af3f27782e358c0490bfd2f2e",
    "e0ce7ac5160ca5540e90cf09ab9fad49",
    "291e1bc6803e3dc4b974c60117ca5d68",
    "cefaddfdf5651df2c91f61b5611a8362",
    NULL
};

static const CSB_V1_VariantId g_csb_boot_graphics_variants[] = {
    CSB_V1_VARIANT_PC34_EN,
    CSB_V1_VARIANT_ST21_EN,
    CSB_V1_VARIANT_ST21_EN,
    CSB_V1_VARIANT_AMIGA35_EN,
    CSB_V1_VARIANT_AMIGA35_MULTI
};

static uint32_t csb_v1_boot_packaged_capture_hash_step_pc34(uint32_t hash,
                                                            uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static int csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt);
static int csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt);
extern int csb_v1_startup_execute_render_plan_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_StartupRenderExecutor_PC34 *executor);

static int csb_v1_boot_count_unique_hashes_pc34(const uint32_t *hashes,
                                                int count)
{
    int unique_count = 0;
    int i;
    if (!hashes || count <= 0) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        int j;
        int seen = 0;
        if (hashes[i] == 0u) {
            continue;
        }
        for (j = 0; j < i; ++j) {
            if (hashes[j] == hashes[i]) {
                seen = 1;
                break;
            }
        }
        if (!seen) {
            ++unique_count;
        }
    }
    return unique_count;
}

static const char *const g_csb_boot_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de",
    NULL
};

static const char *const g_csb_boot_fast_scan_subdirs[] = {
    "csb",
    "csb-atari-st-2x",
    "csb-amiga-35-en",
    "csb-amiga-35-multilingual",
    "csb-extras",
    NULL
};

enum {
    CSB_V1_GRAPHIC_TITLE = 1u,
    CSB_V1_GRAPHIC_ENTRANCE_LEFT_DOOR = 2u,
    CSB_V1_GRAPHIC_ENTRANCE_RIGHT_DOOR = 3u,
    CSB_V1_GRAPHIC_ENTRANCE_SCREEN = 4u,
    CSB_V1_GRAPHIC_ENTRANCE_CREDITS = 5u,
    CSB_V1_CSBGRAPHICS_HUD_INVENTORY = 17u,
    CSB_V1_CSBGRAPHICS_HUD_RESURRECT = 40u
};

static void csb_v1_boot_startup_asset_binding_set_pc34(
    CSB_V1_StartupAssetSelection_PC34 *selection,
    CSB_V1_StartupAssetRole_PC34 role,
    CSB_V1_StartupAssetSource_PC34 source,
    uint32_t graphic_index,
    const char *path,
    int verified)
{
    CSB_V1_StartupAssetBinding_PC34 *binding;
    if (!selection || role <= CSB_V1_STARTUP_ASSET_ROLE_NONE_PC34 ||
        role >= CSB_V1_STARTUP_ASSET_ROLE_COUNT_PC34) {
        return;
    }
    binding = &selection->bindings[role];
    memset(binding, 0, sizeof(*binding));
    binding->role = role;
    binding->source = source;
    binding->graphic_index = graphic_index;
    binding->verified = verified ? 1 : 0;
    binding->rejects_generic_or_test_asset =
        selection->reject_generic_or_test_assets;
    snprintf(binding->path, sizeof(binding->path), "%s", path ? path : "");
}

static int csb_v1_boot_csbgraphics_has_m11_entry_pc34(
    const CSB_V1_BootProfile *profile,
    uint32_t entry_index)
{
    return profile && profile->csbgraphics_cache.loaded &&
           profile->csbgraphics_m11_plan.ready &&
           csb_v1_csbgraphics_m11_runtime_plan_find_entry(
               &profile->csbgraphics_m11_plan, entry_index) != NULL;
}

void csb_v1_boot_startup_assets_resolve_pc34(CSB_V1_BootProfile *profile)
{
    CSB_V1_StartupAssetSelection_PC34 *selection;
    const char *graphics_path;
    const char *hud_path;
    int graphics_ready;
    int inventory_override;
    int resurrect_override;

    if (!profile) {
        return;
    }
    selection = &profile->startup_assets;
    memset(selection, 0, sizeof(*selection));
    graphics_ready = profile->assets_verified && profile->graphics_verified &&
                     profile->graphics_path[0] != '\0';
    selection->real_graphics_available = graphics_ready ? 1 : 0;
    selection->reject_generic_or_test_assets = graphics_ready ? 1 : 0;
    graphics_path = graphics_ready ? profile->graphics_path : NULL;

    /* ReDMCSB TITLE.C F0437 lines 424-461 loads C001 once, then presents
     * its C424/C425/C426 zones. ENTRANCE.C F0806 lines 721-778 loads
     * C002-C005 and derives all opening frames from C002/C003. */
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_TITLE, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_TITLE, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_TITLE_STRIKES_BACK_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_TITLE, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_ENTRANCE_LEFT_DOOR, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_ENTRANCE_RIGHT_DOOR, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_ENTRANCE_SCREEN, graphics_path, graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34,
        CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        CSB_V1_GRAPHIC_ENTRANCE_CREDITS, graphics_path, graphics_ready);

    /* CSBWin Graphics.cpp:1918-1983 accepts an override only after its
     * index is parsed. Restrict the startup HUD handoff to the existing
     * geometry-validated M11 entries instead of accepting an arbitrary
     * CSBgraphics.dat payload as a generic test surface. */
    inventory_override = csb_v1_boot_csbgraphics_has_m11_entry_pc34(
        profile, CSB_V1_CSBGRAPHICS_HUD_INVENTORY);
    resurrect_override = csb_v1_boot_csbgraphics_has_m11_entry_pc34(
        profile, CSB_V1_CSBGRAPHICS_HUD_RESURRECT);
    selection->csbgraphics_available =
        inventory_override || resurrect_override;
    hud_path = selection->csbgraphics_available
        ? profile->csbgraphics_cache.resolved_path : graphics_path;
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34,
        inventory_override ? CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34
                           : CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        inventory_override ? CSB_V1_CSBGRAPHICS_HUD_INVENTORY : 0u,
        inventory_override ? hud_path : graphics_path,
        inventory_override || graphics_ready);
    csb_v1_boot_startup_asset_binding_set_pc34(
        selection, CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34,
        resurrect_override ? CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34
                           : CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34,
        resurrect_override ? CSB_V1_CSBGRAPHICS_HUD_RESURRECT : 0u,
        resurrect_override ? hud_path : graphics_path,
        resurrect_override || graphics_ready);
}

const CSB_V1_StartupAssetBinding_PC34 *
csb_v1_boot_startup_asset_binding_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_StartupAssetRole_PC34 role)
{
    if (!profile || role <= CSB_V1_STARTUP_ASSET_ROLE_NONE_PC34 ||
        role >= CSB_V1_STARTUP_ASSET_ROLE_COUNT_PC34) {
        return NULL;
    }
    return &profile->startup_assets.bindings[role];
}

const char *csb_v1_boot_startup_asset_source_name_pc34(
    CSB_V1_StartupAssetSource_PC34 source)
{
    switch (source) {
    case CSB_V1_STARTUP_ASSET_SOURCE_GRAPHICS_DAT_PC34: return "graphics.dat";
    case CSB_V1_STARTUP_ASSET_SOURCE_CSBGRAPHICS_DAT_PC34: return "csbgraphics.dat";
    case CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34: return "fallback";
    case CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34:
    default: return "none";
    }
}

static CSB_V1_StartupAssetRole_PC34
csb_v1_boot_startup_role_for_command_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan,
    const CSB_V1_StartupAssetCommand_PC34 *command)
{
    if (!plan || !command) {
        return CSB_V1_STARTUP_ASSET_ROLE_NONE_PC34;
    }
    if (command->kind == CSB_V1_STARTUP_ASSET_CLOSED_LEFT_DOOR_PC34) {
        return CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34;
    }
    if (command->kind == CSB_V1_STARTUP_ASSET_CLOSED_RIGHT_DOOR_PC34) {
        return CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34;
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_TITLE_PC34) {
        switch (plan->title_stage) {
        case CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34:
            return CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34;
        case CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34:
            return CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34;
        case CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34:
            return CSB_V1_STARTUP_ASSET_ROLE_TITLE_STRIKES_BACK_PC34;
        default:
            return CSB_V1_STARTUP_ASSET_ROLE_NONE_PC34;
        }
    }
    if (plan->surface == CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34) {
        return CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34;
    }
    return CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34;
}

int csb_v1_boot_startup_render_plan_uses_real_assets_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    int seen = 0;
    if (!profile || !plan || !profile->startup_assets.real_graphics_available) {
        return 0;
    }
    for (i = 0; i < plan->asset_command_count &&
                i < CSB_V1_STARTUP_ASSET_COMMAND_CAP_PC34; ++i) {
        const CSB_V1_StartupAssetBinding_PC34 *binding;
        CSB_V1_StartupAssetRole_PC34 role;
        if (!plan->asset_commands[i].visible) {
            continue;
        }
        ++seen;
        role = csb_v1_boot_startup_role_for_command_pc34(
            plan, &plan->asset_commands[i]);
        binding = csb_v1_boot_startup_asset_binding_pc34(profile, role);
        if (!binding || !binding->verified ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34 ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34 ||
            binding->path[0] == '\0') {
            return 0;
        }
        if (plan->asset_commands[i].asset_id !=
            (int)binding->graphic_index) {
            return 0;
        }
    }
    return seen > 0;
}

void csb_v1_boot_startup_runtime_asset_gate_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 *receipt)
{
    if (receipt) {
        memset(receipt, 0, sizeof(*receipt));
        csb_v1_runtime_startup_session_state_receipt_init_pc34(
            &receipt->session_state);
        csb_v1_startup_real_receipt_init(&receipt->real_asset_receipt);
    }
}

static int csb_v1_boot_startup_asset_roles_owned_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupAssetRole_PC34 *roles,
    size_t role_count)
{
    size_t i;
    if (!profile || !roles) {
        return 0;
    }
    for (i = 0; i < role_count; ++i) {
        const CSB_V1_StartupAssetBinding_PC34 *binding =
            csb_v1_boot_startup_asset_binding_pc34(profile, roles[i]);
        if (!binding || !binding->verified || !binding->rejects_generic_or_test_asset ||
            binding->path[0] == '\0' ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34 ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34) {
            return 0;
        }
    }
    return 1;
}

static uint32_t csb_v1_boot_startup_asset_binding_hash_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_StartupAssetRole_PC34 *roles,
    size_t role_count,
    int *out_bound_count)
{
    uint32_t hash = 2166136261u;
    int bound_count = 0;
    size_t i;

    if (out_bound_count) {
        *out_bound_count = 0;
    }
    if (!profile || !roles || role_count == 0) {
        return 0u;
    }
    for (i = 0; i < role_count; ++i) {
        const CSB_V1_StartupAssetBinding_PC34 *binding =
            csb_v1_boot_startup_asset_binding_pc34(profile, roles[i]);
        const unsigned char *p;
        if (!binding || !binding->verified ||
            !binding->rejects_generic_or_test_asset ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_NONE_PC34 ||
            binding->source == CSB_V1_STARTUP_ASSET_SOURCE_FALLBACK_PC34 ||
            binding->path[0] == '\0') {
            return 0u;
        }
        hash = csb_v1_boot_packaged_capture_hash_step_pc34(
            hash, (uint32_t)roles[i]);
        hash = csb_v1_boot_packaged_capture_hash_step_pc34(
            hash, (uint32_t)binding->source);
        hash = csb_v1_boot_packaged_capture_hash_step_pc34(
            hash, binding->graphic_index);
        for (p = (const unsigned char *)binding->path; *p; ++p) {
            hash = csb_v1_boot_packaged_capture_hash_step_pc34(hash, *p);
        }
        ++bound_count;
    }
    if (out_bound_count) {
        *out_bound_count = bound_count;
    }
    return hash ? hash : 1u;
}

int csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_BootStartupLaunchReceipts_PC34 *launch_receipts,
    CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 *out_receipt)
{
    static const CSB_V1_StartupAssetRole_PC34 title_roles[] = {
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_STRIKES_BACK_PC34
    };
    static const CSB_V1_StartupAssetRole_PC34 entrance_roles[] = {
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34
    };
    static const CSB_V1_StartupAssetRole_PC34 hud_roles[] = {
        CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34
    };

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_runtime_asset_gate_receipt_init_pc34(out_receipt);
    if (!profile || !launch_receipts || !profile->assets_verified ||
        !profile->graphics_verified || !profile->dungeon_verified ||
        !profile->startup_assets.real_graphics_available) {
        return 0;
    }
    out_receipt->real_asset_matched =
        csb_v1_startup_real_receipt_from_profile_fields(
            profile->asset_root, profile->graphics_path, profile->dungeon_path,
            profile->graphics_md5, profile->dungeon_md5, 0u, 0u,
            profile->variant_id, profile->graphics_kind, 4,
            profile->assets_verified, profile->graphics_verified,
            profile->dungeon_verified, &out_receipt->real_asset_receipt);
    out_receipt->title_assets_owned = csb_v1_boot_startup_asset_roles_owned_pc34(
        profile, title_roles, sizeof(title_roles) / sizeof(title_roles[0]));
    out_receipt->entrance_assets_owned = csb_v1_boot_startup_asset_roles_owned_pc34(
        profile, entrance_roles, sizeof(entrance_roles) / sizeof(entrance_roles[0]));
    out_receipt->hud_assets_owned = csb_v1_boot_startup_asset_roles_owned_pc34(
        profile, hud_roles, sizeof(hud_roles) / sizeof(hud_roles[0]));
    out_receipt->asset_ownership_valid =
        out_receipt->title_assets_owned && out_receipt->entrance_assets_owned &&
                out_receipt->hud_assets_owned ? 1 : 0;
    out_receipt->session_state = launch_receipts->session_state;
    out_receipt->session_state_valid =
        (!out_receipt->session_state.entrance_resume_available ||
         out_receipt->session_state.entrance_resume_path[0] != '\0') &&
                (!out_receipt->session_state.import_available ||
                 out_receipt->session_state.import_champion_count > 0) &&
                out_receipt->session_state.import_selected_action_index >= 0
            ? 1 : 0;
    out_receipt->rejects_fallback_sources =
        profile->startup_assets.reject_generic_or_test_assets ? 1 : 0;
    out_receipt->real_asset_receipt_hash =
        out_receipt->real_asset_receipt.receipt_hash;
    out_receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 94-95; ENTRANCE.C F0438 lines 116-152; "
        "CSBWin Viewport.cpp startup HUD/menu ownership";
    out_receipt->valid = out_receipt->real_asset_matched &&
                         out_receipt->real_asset_receipt.matched &&
                         out_receipt->asset_ownership_valid &&
                         out_receipt->session_state_valid &&
                         out_receipt->rejects_fallback_sources ? 1 : 0;
    return out_receipt->valid;
}

static void csb_v1_boot_startup_route_from_presentation_pc34(
    const CSB_V1_StartupPresentationReceipt_PC34 *presentation,
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *out_receipt);
static void csb_v1_boot_startup_hud_menu_state_init_pc34(
    CSB_V1_BootStartupHudMenuStateReceipt_PC34 *state);
static int csb_v1_boot_startup_closed_door_menu_render_plan_from_view_receipt_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt,
    CSB_V1_StartupRenderPlan_PC34 *out_plan);
int csb_v1_boot_startup_presentation_state_receipt_from_runtime_state_pc34(
    CSB_V1_StartupPresentationReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile);

/* ── DM1-assumption rejection strings ────────────────────────────────────
 *
 * Each csb_v1_boot_assume_no_dm1_runtime() failure has a stable reason
 * string.  The probe and any future diagnostics depend on these exact
 * phrases, so do not rename them without updating
 * probes/firestaff_csb_v1_no_dm1_runtime_assumption_gate_probe.c.
 *
 *   "csb_boot/assume_no_dm1_runtime: game_id is not 'csb'"
 *       Rejects profiles routed from DM1/DM2 launchers.
 *   "csb_boot/assume_no_dm1_runtime: variant_id outside CSB range"
 *       Catches raw enum values that escape the CSB_V1_VARIANT_* enum.
 *   "csb_boot/assume_no_dm1_runtime: party default matches DM1 HoC (11,29)"
 *       Catches (11,29) DM1 Hall of Champions defaults leaking into CSB.
 *   "csb_boot/assume_no_dm1_runtime: tick_ms is not CSB nominal (55)"
 *       Catches non-CSB tick quantum (DM1 is also 55; this makes CSB origin explicit).
 *   "csb_boot/assume_no_dm1_runtime: entrance_map_index != 255"
 *       ReDMCSB ENTRANCE.C F0806 selects C255_MAP_INDEX_ENTRANCE for CSB.
 *   "csb_boot/assume_no_dm1_runtime: start_map_index != 0"
 *       ReDMCSB LOADSAVE.C F0435 line 1940-1944 sets map 0 for new games. */
static const char *g_csb_assume_last_reason = "csb_boot/assume_no_dm1_runtime: ok";

#define CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX 564u
#define CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES 65535u
#define CSB_V1_GRAPHICS_LZW_MAX_CODE 4096
#define CSB_V1_GRAPHICS_LZW_CLEAR_CODE 256
#define CSB_V1_GRAPHICS_LZW_END_CODE 257
#define CSB_V1_GRAPHICS_LZW_FIRST_CODE 258

typedef struct {
    const uint8_t *bytes;
    size_t size;
    size_t byte_pos;
    uint8_t chunk[12];
    int chunk_bit_idx;
    int chunk_bit_count;
    int needs_refill;
} CSB_V1_GraphicsBitReader;

static uint16_t csb_v1_graphics_read_le16(const uint8_t *bytes)
{
    return (uint16_t)((uint16_t)bytes[0] | ((uint16_t)bytes[1] << 8));
}

static int csb_v1_graphics_read_bits(CSB_V1_GraphicsBitReader *br,
                                     int bit_count,
                                     uint16_t *out_code)
{
    static const uint8_t lsb_masks[9] = {
        0x00u, 0x01u, 0x03u, 0x07u, 0x0fu, 0x1fu, 0x3fu, 0x7fu, 0xffu
    };
    uint32_t value = 0u;
    int bit_index;
    int required;
    const uint8_t *p;
    if (!br || !out_code || bit_count <= 0 || bit_count > 12) {
        return -1;
    }
    /* ReDMCSB LZW.C F0495 reads codeBitCount bytes into a chunk and
     * extracts up to eight variable-width codes from that chunk. Width
     * changes force a refill instead of continuing through a flat stream. */
    if (br->needs_refill || br->chunk_bit_idx >= br->chunk_bit_count) {
        int chunk_bytes = bit_count;
        if (br->byte_pos + (size_t)chunk_bytes > br->size) {
            chunk_bytes = (int)(br->size - br->byte_pos);
        }
        if (chunk_bytes <= 0) {
            return -1;
        }
        memset(br->chunk, 0, sizeof(br->chunk));
        memcpy(br->chunk, br->bytes + br->byte_pos, (size_t)chunk_bytes);
        br->byte_pos += (size_t)chunk_bytes;
        br->chunk_bit_idx = 0;
        br->chunk_bit_count = (chunk_bytes << 3) - (bit_count - 1);
        br->needs_refill = 0;
    }
    bit_index = br->chunk_bit_idx;
    required = bit_count;
    p = br->chunk + (bit_index >> 3);
    bit_index &= 7;

    value = (uint32_t)(*p++ >> bit_index);
    required -= (8 - bit_index);
    bit_index = 8 - bit_index;
    if (required >= 8) {
        value |= (uint32_t)(*p++) << bit_index;
        bit_index += 8;
        required -= 8;
    }
    if (required > 0) {
        value |= (uint32_t)(*p & lsb_masks[required]) << bit_index;
    }
    br->chunk_bit_idx += bit_count;
    *out_code = (uint16_t)value;
    return 0;
}

static void csb_v1_graphics_lzw_reset(uint16_t *prefix,
                                      uint8_t *append,
                                      int *next_code,
                                      int *code_bits)
{
    int i;
    for (i = 0; i < 256; i++) {
        prefix[i] = 0xffffu;
        append[i] = (uint8_t)i;
    }
    *next_code = CSB_V1_GRAPHICS_LZW_FIRST_CODE;
    *code_bits = 9;
}

static int csb_v1_graphics_lzw_emit(uint16_t code,
                                    const uint16_t *prefix,
                                    const uint8_t *append,
                                    uint8_t *stack,
                                    uint8_t *out,
                                    size_t out_capacity,
                                    size_t *out_pos,
                                    uint8_t *out_first)
{
    int stack_len = 0;
    uint16_t cursor = code;

    while (cursor >= 256u) {
        if (cursor >= CSB_V1_GRAPHICS_LZW_MAX_CODE ||
            prefix[cursor] == 0xffffu ||
            stack_len >= CSB_V1_GRAPHICS_LZW_MAX_CODE) {
            return -1;
        }
        stack[stack_len++] = append[cursor];
        cursor = prefix[cursor];
    }
    if (cursor >= 256u) {
        return -1;
    }
    if (out_first) {
        *out_first = (uint8_t)cursor;
    }
    if (*out_pos >= out_capacity) {
        return -1;
    }
    out[(*out_pos)++] = (uint8_t)cursor;
    while (stack_len > 0) {
        if (*out_pos >= out_capacity) {
            return -1;
        }
        out[(*out_pos)++] = stack[--stack_len];
    }
    return 0;
}

static int csb_v1_graphics_lzw_decode(const uint8_t *input,
                                      size_t input_size,
                                      uint8_t *out,
                                      size_t out_capacity,
                                      size_t *out_size)
{
    uint16_t prefix[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t append[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    uint8_t stack[CSB_V1_GRAPHICS_LZW_MAX_CODE];
    CSB_V1_GraphicsBitReader br;
    int next_code;
    int code_bits;
    int old_code = -1;
    uint8_t old_first = 0u;
    size_t out_pos = 0u;

    if (!input || !out || !out_size || input_size == 0u) {
        return -1;
    }
    br.bytes = input;
    br.size = input_size;
    br.byte_pos = 0u;
    br.chunk_bit_idx = 0;
    br.chunk_bit_count = 0;
    br.needs_refill = 1;
    csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);

    for (;;) {
        uint16_t code;
        uint8_t first = 0u;
        if (csb_v1_graphics_read_bits(&br, code_bits, &code) != 0) {
            return -1;
        }
        if (code == CSB_V1_GRAPHICS_LZW_CLEAR_CODE) {
            csb_v1_graphics_lzw_reset(prefix, append, &next_code, &code_bits);
            old_code = -1;
            continue;
        }
        if (code == CSB_V1_GRAPHICS_LZW_END_CODE) {
            *out_size = out_pos;
            return 0;
        }
        if (code < (uint16_t)next_code) {
            if (csb_v1_graphics_lzw_emit(code, prefix, append, stack,
                                         out, out_capacity, &out_pos,
                                         &first) != 0) {
                return -1;
            }
        } else if (code == (uint16_t)next_code && old_code >= 0) {
            first = old_first;
            if (csb_v1_graphics_lzw_emit((uint16_t)old_code, prefix, append,
                                         stack, out, out_capacity, &out_pos,
                                         NULL) != 0 ||
                out_pos >= out_capacity) {
                return -1;
            }
            out[out_pos++] = first;
        } else {
            return -1;
        }

        if (old_code >= 0 && next_code < CSB_V1_GRAPHICS_LZW_MAX_CODE) {
            prefix[next_code] = (uint16_t)old_code;
            append[next_code] = first;
            next_code++;
            if (next_code > ((1 << code_bits) - 1) && code_bits < 12) {
                code_bits++;
                br.needs_refill = 1;
            }
        }
        old_code = (int)code;
        old_first = first;
    }
}

static int csb_v1_graphics_decode_entry_m564(const uint8_t *file_bytes,
                                             size_t file_size,
                                             uint8_t *out,
                                             size_t out_capacity,
                                             size_t *out_size)
{
    uint16_t signature;
    uint16_t count;
    size_t compressed_table;
    size_t decompressed_table;
    size_t dimensions_table;
    size_t payload_offset;
    size_t entry_offset;
    size_t i;
    uint16_t compressed_size;
    uint16_t decompressed_size;
    size_t decoded_size = 0u;

    if (!file_bytes || !out || !out_size || file_size < 4u) {
        return -1;
    }
    *out_size = 0u;
    signature = csb_v1_graphics_read_le16(file_bytes);
    count = csb_v1_graphics_read_le16(file_bytes + 2u);
    if ((signature & 0x8000u) == 0u ||
        count <= CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX) {
        return -1;
    }
    compressed_table = 4u;
    decompressed_table = compressed_table + (size_t)count * 2u;
    dimensions_table = decompressed_table + (size_t)count * 2u;
    payload_offset = dimensions_table + (size_t)count * 4u;
    if (payload_offset > file_size) {
        return -1;
    }

    entry_offset = payload_offset;
    for (i = 0u; i < CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX; i++) {
        entry_offset +=
            (size_t)csb_v1_graphics_read_le16(file_bytes + compressed_table + i * 2u);
        if (entry_offset > file_size) {
            return -1;
        }
    }
    compressed_size = csb_v1_graphics_read_le16(
        file_bytes + compressed_table + CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX * 2u);
    decompressed_size = csb_v1_graphics_read_le16(
        file_bytes + decompressed_table + CSB_V1_GRAPHICS_OBJECT_NAMES_INDEX * 2u);
    if (compressed_size == 0u || decompressed_size == 0u ||
        decompressed_size > out_capacity ||
        entry_offset + (size_t)compressed_size > file_size) {
        return -1;
    }

    if (csb_v1_graphics_lzw_decode(file_bytes + entry_offset,
                                   (size_t)compressed_size,
                                   out,
                                   (size_t)decompressed_size,
                                   &decoded_size) != 0 ||
        decoded_size != (size_t)decompressed_size) {
        return -1;
    }
    *out_size = decoded_size;
    return 0;
}

static int csb_v1_boot_load_object_names_m564(CSB_V1_BootProfile *profile)
{
    FILE *file;
    long file_len;
    uint8_t *file_bytes = NULL;
    uint8_t *decoded = NULL;
    size_t read_count;
    size_t decoded_size = 0u;
    int ok = 0;

    if (!profile || profile->graphics_path[0] == '\0') {
        return 0;
    }
    file = fopen(profile->graphics_path, "rb");
    if (!file) {
        return 0;
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return 0;
    }
    file_len = ftell(file);
    if (file_len <= 0 || fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return 0;
    }
    file_bytes = (uint8_t *)malloc((size_t)file_len);
    decoded = (uint8_t *)malloc(CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES);
    if (!file_bytes || !decoded) {
        free(decoded);
        free(file_bytes);
        fclose(file);
        return 0;
    }
    read_count = fread(file_bytes, 1u, (size_t)file_len, file);
    fclose(file);
    if (read_count == (size_t)file_len &&
        csb_v1_graphics_decode_entry_m564(
            file_bytes,
            (size_t)file_len,
            decoded,
            CSB_V1_GRAPHICS_OBJECT_NAMES_MAX_BYTES,
            &decoded_size) == 0) {
        /* ReDMCSB OBJECT.C F0031 loads M564_GRAPHIC_OBJECT_NAMES from
         * GRAPHICS.DAT and decodes C199 high-bit-terminated object names
         * before object UI asks F0033 for icon-indexed names. */
        ok = csb_v1_runtime_load_object_names_m564(&profile->runtime,
                                                   decoded,
                                                   decoded_size);
    }
    free(decoded);
    free(file_bytes);
    return ok == 1 ? 1 : 0;
}

const char *csb_v1_boot_last_assumption_reason(void)
{
    return g_csb_assume_last_reason;
}

static void csb_v1_boot_assume_fail(const char *reason)
{
    g_csb_assume_last_reason = reason ? reason : "csb_boot/assume_no_dm1_runtime: (null)";
}

int csb_v1_boot_assume_no_dm1_runtime(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: NULL profile");
        return -1;
    }
    /* game_id must be the CSB literal.  ReDMCSB ENTRANCE.C F0806 selects
     * C28_ENTRANCE_CSB and the loader branches on game_id; a DM1 or DM2
     * profile routed here would otherwise inherit the CSB runtime
     * structure but keep DM1 start defaults. */
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: game_id is not 'csb'");
        return -1;
    }
    /* variant_id must stay inside the CSB enum.  A raw integer from a
     * DM1 or DM2 catalog that happens to land in this range would
     * otherwise resolve to a CSB variant silently.  We use a strict
     * inclusive check: variant_id >= 0 && variant_id < CSB_V1_VARIANT_COUNT. */
    if (profile->variant_id < 0 ||
        (unsigned)profile->variant_id >= (unsigned)CSB_V1_VARIANT_COUNT) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: variant_id outside CSB range");
        return -1;
    }
    /* DM1 Hall of Champions default is (11,29) facing North.  CSB's
     * Hall of Champions is (5,5) facing North.  These are disjoint
     * source-locked defaults — a profile carrying (11,29) almost
     * certainly came from a DM1 launcher side-effect.
     * Source: ReDMCSB ENTRANCE.C (DM1) line ~430 vs (CSB) line ~430
     * Source: csb_v1_runtime_pc34_compat.h CSB_V1_START_PARTY_{X,Y} */
    if (profile->default_party_x == 11U &&
        profile->default_party_y == 29U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: party default matches DM1 HoC (11,29)");
        return -1;
    }
    /* CSB V1 tick is 55ms nominal (shared with DM1, but the assertion
     * makes CSB origin explicit; the runtime must use CSB_V1_TICK_MS_NOMINAL
     * exactly, not a DM2 or Nexus tick quantum). */
    if (profile->tick_ms != CSB_V1_TICK_MS_NOMINAL) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: tick_ms is not CSB nominal (55)");
        return -1;
    }
    /* entrance_map_index is 255 (C255_MAP_INDEX_ENTRANCE) for CSB.
     * Rejecting any other value catches DM1 maps 0..14 or future variants
     * that escape the source-locked entrance selection. */
    if (profile->entrance_map_index != 255U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: entrance_map_index != 255");
        return -1;
    }
    /* start_map_index is 0 for new games.  Anything else implies DM1-style
     * dungeon-of-doom selection or a corrupted scan. */
    if (profile->start_map_index != 0U) {
        csb_v1_boot_assume_fail(
            "csb_boot/assume_no_dm1_runtime: start_map_index != 0");
        return -1;
    }
    csb_v1_boot_assume_fail("csb_boot/assume_no_dm1_runtime: ok");
    return 0;
}

static void csb_v1_boot_copy(char *dst, size_t dst_size, const char *src)
{
    if (!dst || dst_size == 0U) return;
    if (!src) src = "";
    snprintf(dst, dst_size, "%s", src);
}

static CSB_V1_AssetGfxArchiveType csb_v1_boot_graphics_kind(const char *path)
{
    const char *name;
    if (!path) return CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    name = strrchr(path, '/');
#if defined(_WIN32)
    {
        const char *slash = strrchr(path, '\\');
        if (slash && (!name || slash > name)) name = slash;
    }
#endif
    name = name ? name + 1 : path;
    if (strcmp(name, "CSB.DAT") == 0 || strcmp(name, "csb.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSB;
    }
    if (strcmp(name, "CSBGRAPH.DAT") == 0 || strcmp(name, "csbgraph.dat") == 0) {
        return CSB_V1_ASSET_GFX_ARCHIVE_CSBGRAF;
    }
    return CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
}

void csb_v1_boot_profile_init(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    memset(profile, 0, sizeof(*profile));
    csb_v1_boot_copy(profile->game_id, sizeof(profile->game_id), CSB_V1_BOOT_GAME_ID);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id), "unknown");
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label), "Unknown");
    profile->tick_ms = CSB_V1_TICK_MS_NOMINAL;
    profile->entrance_map_index = 255U;
    profile->start_map_index = 0U;
    profile->default_party_x = CSB_V1_START_PARTY_X;
    profile->default_party_y = CSB_V1_START_PARTY_Y;
    profile->default_party_dir = CSB_V1_START_PARTY_DIR;
    profile->imported_party_ready = 0;
    profile->cmp_import_attempted = 0;
    profile->cmp_import_succeeded = 0;
    profile->cmp_imported_slot = -1;
    profile->cmp_imported_champion_count = 0;
    profile->engine_version_displayed = 0;
    profile->csbgraphics_scan_attempted = 0;
    profile->csbgraphics_scan_result =
        CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    profile->csbgraphics_plan_result =
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE;
    profile->csbgraphics_skin_def_loaded = 0;
    profile->csbgraphics_skin_def_word_count = 0u;
    memset(profile->csbgraphics_skin_def_words, 0,
           sizeof(profile->csbgraphics_skin_def_words));
    csb_v1_csbgraphics_dat_real_cache_init(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_m11_runtime_plan_init(&profile->csbgraphics_m11_plan);
    csb_v1_boot_startup_assets_resolve_pc34(profile);
    csb_v1_character_init_default(&profile->imported_party);
    csb_v1_runtime_init(&profile->runtime, NULL);
    csb_v1_engine_version_display_set_csb(0);
}

static void csb_v1_boot_reset_csbgraphics(CSB_V1_BootProfile *profile)
{
    if (!profile) {
        return;
    }
    csb_v1_csbgraphics_dat_real_cache_free(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_dat_real_cache_init(&profile->csbgraphics_cache);
    csb_v1_csbgraphics_m11_runtime_plan_init(&profile->csbgraphics_m11_plan);
    profile->csbgraphics_scan_attempted = 0;
    profile->csbgraphics_scan_result =
        CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_NOT_FOUND;
    profile->csbgraphics_plan_result =
        CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_ERR_NO_CACHE;
    profile->csbgraphics_skin_def_loaded = 0;
    profile->csbgraphics_skin_def_word_count = 0u;
    memset(profile->csbgraphics_skin_def_words, 0,
           sizeof(profile->csbgraphics_skin_def_words));
}

static int csb_v1_boot_scan_required_paths(const char *root,
                                           char *graphics_path,
                                           size_t graphics_path_size,
                                           int *graphics_match,
                                           char *dungeon_path,
                                           size_t dungeon_path_size,
                                           int *dungeon_match)
{
    int graphics_verified;
    int dungeon_verified;
    if (!root || !graphics_path || !graphics_match ||
        !dungeon_path || !dungeon_match) {
        return 0;
    }

    graphics_path[0] = '\0';
    dungeon_path[0] = '\0';
    *graphics_match = -1;
    *dungeon_match = -1;
    graphics_verified =
        asset_find_by_md5_list(root, g_csb_boot_graphics_hashes,
                               graphics_path, (int)graphics_path_size,
                               graphics_match, 4);
    dungeon_verified =
        asset_find_by_md5_list(root, g_csb_boot_dungeon_hashes,
                               dungeon_path, (int)dungeon_path_size,
                               dungeon_match, 4);
    return graphics_verified && dungeon_verified;
}

static int csb_v1_boot_scan_required_paths_fast(const char *root,
                                                char *graphics_path,
                                                size_t graphics_path_size,
                                                int *graphics_match,
                                                char *dungeon_path,
                                                size_t dungeon_path_size,
                                                int *dungeon_match)
{
    size_t i;
    char candidate[ASSET_PATH_MAX];
    if (!root || !root[0]) {
        return 0;
    }
    for (i = 0U; g_csb_boot_fast_scan_subdirs[i] != NULL; ++i) {
        if (snprintf(candidate, sizeof(candidate), "%s/%s", root,
                     g_csb_boot_fast_scan_subdirs[i]) >=
            (int)sizeof(candidate)) {
            continue;
        }
        if (csb_v1_boot_scan_required_paths(candidate,
                                            graphics_path,
                                            graphics_path_size,
                                            graphics_match,
                                            dungeon_path,
                                            dungeon_path_size,
                                            dungeon_match)) {
            return 1;
        }
    }
    return 0;
}

int csb_v1_boot_scan_csbgraphics(CSB_V1_BootProfile *profile,
                                 const char *cache_dir)
{
    const char *root;

    if (!profile) {
        return CSB_V1_CSBGRAPHICS_DAT_REAL_ERR_ARGUMENT;
    }
    root = profile->asset_root[0] ? profile->asset_root : NULL;
    csb_v1_boot_reset_csbgraphics(profile);
    profile->csbgraphics_scan_attempted = 1;
    profile->csbgraphics_scan_result =
        csb_v1_csbgraphics_dat_real_scan_and_load(
            root, cache_dir, 4, &profile->csbgraphics_cache);
    if (profile->csbgraphics_scan_result !=
        CSB_V1_CSBGRAPHICS_DAT_REAL_OK) {
        return profile->csbgraphics_scan_result;
    }
    profile->csbgraphics_plan_result =
        csb_v1_csbgraphics_m11_runtime_plan_build_from_cache(
            &profile->csbgraphics_cache,
            &profile->csbgraphics_m11_plan);
    {
        size_t skin_def_word_count = 0u;
        int skin_rc =
            csb_v1_csbgraphics_m11_runtime_plan_decode_custom_background_skin_def(
                &profile->csbgraphics_cache,
                profile->csbgraphics_skin_def_words,
                CSB_V1_CSBGRAPHICS_M11_SKIN_DEF_MAX_WORDS,
                &skin_def_word_count);
        if (skin_rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
            int add_rc;
            profile->csbgraphics_skin_def_loaded = 1;
            profile->csbgraphics_skin_def_word_count = skin_def_word_count;
            add_rc =
                csb_v1_csbgraphics_m11_runtime_plan_add_custom_background_skin_def(
                    &profile->csbgraphics_cache,
                    profile->csbgraphics_skin_def_words,
                    profile->csbgraphics_skin_def_word_count,
                    &profile->csbgraphics_m11_plan);
            if (add_rc == CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK) {
                profile->csbgraphics_plan_result =
                    CSB_V1_CSBGRAPHICS_M11_RUNTIME_PLAN_OK;
            }
        }
    }
    return profile->csbgraphics_plan_result;
}

const CSB_V1_CSBGraphicsM11RuntimePlan *
csb_v1_boot_csbgraphics_m11_plan(const CSB_V1_BootProfile *profile)
{
    return profile ? &profile->csbgraphics_m11_plan : NULL;
}

const CSB_V1_CSBGraphicsDatRealCache *
csb_v1_boot_csbgraphics_cache(const CSB_V1_BootProfile *profile)
{
    return profile ? &profile->csbgraphics_cache : NULL;
}

const uint16_t *
csb_v1_boot_csbgraphics_skin_def_words(const CSB_V1_BootProfile *profile,
                                       size_t *out_word_count)
{
    if (out_word_count) {
        *out_word_count = 0u;
    }
    if (!profile || !profile->csbgraphics_skin_def_loaded ||
        profile->csbgraphics_skin_def_word_count == 0u) {
        return NULL;
    }
    if (out_word_count) {
        *out_word_count = profile->csbgraphics_skin_def_word_count;
    }
    return profile->csbgraphics_skin_def_words;
}

int csb_v1_boot_render_viewport_frame_pc34(
    void *boot_profile,
    unsigned char *framebuffer,
    int framebuffer_width,
    int framebuffer_height,
    const CSB_V1_ViewportRuntimeDrawerBinding *drawer_binding,
    CSB_V1_ViewportRuntimeDrawCounts *out_counts)
{
    CSB_V1_BootProfile *profile = (CSB_V1_BootProfile *)boot_profile;
    CSB_V1_ViewportConfig cfg;
    uint8_t dungeon_grid[32 * 32];
    uint8_t custom_background_cell_skins[32 * 32];
    uint32_t i;

    if (out_counts) {
        csb_v1_viewport_runtime_draw_counts_reset(out_counts);
    }
    if (!profile || !framebuffer || framebuffer_width < 320 ||
        framebuffer_height < 200 || !profile->runtime.dungeon_handle) {
        return 0;
    }

    (void)csb_v1_viewport_build_dungeon_grid(
        profile->runtime.dungeon_handle,
        profile->runtime.current_level,
        dungeon_grid);

    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = framebuffer_width;
    cfg.dungeon_grid = dungeon_grid;
    cfg.dungeon_width = 32;
    cfg.dungeon_height = 32;
    cfg.wall_set_index = 0;
    cfg.runtime_profile = &profile->runtime;
    cfg.runtime_projectiles = &profile->runtime.projectiles;
    cfg.runtime_explosions = &profile->runtime.explosions;
    if (drawer_binding) {
        csb_v1_viewport_apply_runtime_drawer_binding(&cfg, drawer_binding);
    }
    cfg.csbgraphics_plan = csb_v1_boot_csbgraphics_m11_plan(profile);
    cfg.csbgraphics_cache = csb_v1_boot_csbgraphics_cache(profile);
    cfg.custom_background_skin_def_words =
        csb_v1_boot_csbgraphics_skin_def_words(
            profile,
            &cfg.custom_background_skin_def_word_count);

    if (csb_v1_runtime_custom_background_skin_grid(
            &profile->runtime,
            custom_background_cell_skins,
            (int)sizeof(custom_background_cell_skins),
            &cfg.custom_background_cell_skin_width,
            &cfg.custom_background_cell_skin_height,
            &cfg.custom_background_loaded_level,
            &cfg.custom_background_default_skin)) {
        cfg.custom_background_cell_skins = custom_background_cell_skins;
    }

    /* Source-lock: ReDMCSB DUNVIEW.C F0128 is the CSB viewport draw
     * boundary; CSBWin Viewport.cpp keeps the same party pose contract.
     * The boot profile owns the runtime pose and CSBGRAPHICS state. */
    csb_v1_viewport_render_frame(&cfg,
                                 profile->runtime.party_dir,
                                 profile->runtime.party_x,
                                 profile->runtime.party_y);
    if (out_counts) {
        csb_v1_viewport_runtime_draw_counts_from_config(&cfg, out_counts);
    }

    if (profile->csbgraphics_m11_plan.ready &&
        profile->csbgraphics_cache.loaded) {
        for (i = 0u; i < profile->csbgraphics_m11_plan.planned_count; ++i) {
            CSB_V1_CSBGraphicsM11Binding binding;
            if (profile->csbgraphics_m11_plan.entries[i]
                    .deferred_masked_composite) {
                continue;
            }
            (void)csb_v1_csbgraphics_m11_runtime_plan_apply_entry(
                &profile->csbgraphics_m11_plan,
                &profile->csbgraphics_cache,
                profile->csbgraphics_m11_plan.entries[i].entry_index,
                framebuffer,
                framebuffer_width,
                framebuffer_height,
                framebuffer_width,
                &binding);
        }
    }
    return 1;
}

int csb_v1_boot_apply_startup_handoff_pc34(
    CSB_V1_BootProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    CSB_V1_RuntimeStartupHandoffReceipt_PC34 *out_receipt)
{
    return profile ? csb_v1_runtime_apply_startup_handoff_pc34(
                         &profile->runtime,
                         save_path,
                         import_dm1_save_path,
                         out_receipt)
                   : 0;
}

int csb_v1_boot_build_startup_session_state_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    const CSB_V1_RuntimeStartupHandoffReceipt_PC34 *handoff,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_RuntimeStartupSessionStateReceipt_PC34 *out_receipt)
{
    return profile ? csb_v1_runtime_build_startup_session_state_receipt_pc34(
                         &profile->runtime,
                         handoff,
                         import_dm1_save_path,
                         resume_save_path,
                         out_receipt)
                   : 0;
}

int csb_v1_boot_build_startup_launch_receipts_pc34(
    CSB_V1_BootProfile *profile,
    const char *save_path,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_BootStartupLaunchReceipts_PC34 *out_receipts)
{
    int direct_resume;

    if (!out_receipts) {
        return 0;
    }
    memset(out_receipts, 0, sizeof(*out_receipts));
    csb_v1_runtime_startup_handoff_receipt_init_pc34(
        &out_receipts->handoff);
    csb_v1_startup_init_state_receipt_init_pc34(
        &out_receipts->init_state);
    csb_v1_runtime_startup_session_state_receipt_init_pc34(
        &out_receipts->session_state);
    csb_v1_startup_host_receipt_init_pc34(
        &out_receipts->launch_host_receipt);
    if (!profile) {
        return 0;
    }
    if (!csb_v1_boot_apply_startup_handoff_pc34(profile,
                                                save_path,
                                                import_dm1_save_path,
                                                &out_receipts->handoff)) {
        return 0;
    }
    direct_resume = (save_path && save_path[0] != '\0') ? 1 : 0;
    if (!csb_v1_startup_init_state_receipt_pc34(
            direct_resume,
            &out_receipts->init_state)) {
        return 0;
    }
    if (!csb_v1_boot_build_startup_session_state_receipt_pc34(
            profile,
            &out_receipts->handoff,
            import_dm1_save_path,
            resume_save_path,
            &out_receipts->session_state)) {
        return 0;
    }
    out_receipts->launch_host_receipt.input_result =
        CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34;
    out_receipts->launch_host_receipt.status_scope = "BOOT";
    out_receipts->launch_host_receipt.status =
        direct_resume ? "CSB RESUMED" : "CSB ENTRANCE";
    out_receipts->launch_host_receipt.log_color = 11U;
    out_receipts->launch_host_receipt.log_line =
        out_receipts->launch_host_receipt.status;
    return 1;
}

static void csb_v1_boot_startup_failure_host_receipt_pc34(
    CSB_V1_StartupHostReceipt_PC34 *receipt,
    const char *status)
{
    if (!receipt) {
        return;
    }
    csb_v1_startup_host_receipt_init_pc34(receipt);
    receipt->input_result = CSB_V1_STARTUP_ENTRANCE_INPUT_IGNORE_PC34;
    receipt->status_scope = "BOOT";
    receipt->status = status ? status : "CSB STARTUP FAILED";
    receipt->log_color = 8U;
    receipt->log_line = receipt->status;
}

void csb_v1_boot_startup_launch_cleanup_pc34(
    CSB_V1_BootStartupLaunch_PC34 *launch)
{
    if (!launch) {
        return;
    }
    if (launch->profile) {
        csb_v1_boot_cleanup(launch->profile);
        free(launch->profile);
    }
    memset(launch, 0, sizeof(*launch));
}

int csb_v1_boot_startup_launch_alloc_pc34(
    const char *data_dir,
    const char *save_path,
    const char *import_dm1_save_path,
    const char *resume_save_path,
    CSB_V1_BootStartupLaunch_PC34 *out_launch)
{
    CSB_V1_StartupHostReceipt_PC34 failure_receipt;

    if (!out_launch) {
        return 0;
    }
    memset(out_launch, 0, sizeof(*out_launch));
    csb_v1_boot_startup_failure_host_receipt_pc34(
        &out_launch->failure_host_receipt,
        "CSB STARTUP FAILED");
    out_launch->profile =
        (CSB_V1_BootProfile *)calloc(1, sizeof(*out_launch->profile));
    if (!out_launch->profile) {
        csb_v1_boot_startup_failure_host_receipt_pc34(
            &out_launch->failure_host_receipt,
            "CSB OOM");
        return 0;
    }
    csb_v1_boot_profile_init(out_launch->profile);
    if (csb_v1_boot_scan_assets(out_launch->profile, data_dir) != 0) {
        csb_v1_boot_startup_failure_host_receipt_pc34(
            &failure_receipt,
            "CSB ASSETS MISSING");
        csb_v1_boot_startup_launch_cleanup_pc34(out_launch);
        out_launch->failure_host_receipt = failure_receipt;
        return 0;
    }
    if (csb_v1_boot_enter_game(out_launch->profile) != 0) {
        csb_v1_boot_startup_failure_host_receipt_pc34(
            &failure_receipt,
            "CSB ENTER GAME FAILED");
        csb_v1_boot_startup_launch_cleanup_pc34(out_launch);
        out_launch->failure_host_receipt = failure_receipt;
        return 0;
    }
    if (!csb_v1_boot_build_startup_launch_receipts_pc34(
            out_launch->profile,
            save_path,
            import_dm1_save_path,
            resume_save_path,
            &out_launch->receipts)) {
        csb_v1_boot_startup_failure_host_receipt_pc34(
            &failure_receipt,
            out_launch->receipts.handoff.status
                ? out_launch->receipts.handoff.status
                : "CSB STARTUP FAILED");
        failure_receipt.status_scope =
            out_launch->receipts.handoff.status_scope
                ? out_launch->receipts.handoff.status_scope
                : "BOOT";
        csb_v1_boot_startup_launch_cleanup_pc34(out_launch);
        out_launch->failure_host_receipt = failure_receipt;
        return 0;
    }
    return 1;
}

int csb_v1_boot_startup_launch_detach_runtime_pc34(
    CSB_V1_BootStartupLaunch_PC34 *launch,
    CSB_V1_BootStartupRuntimeReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!launch || !launch->profile) {
        return 0;
    }
    csb_v1_boot_startup_assets_resolve_pc34(launch->profile);
    out_receipt->startup_asset_gate_valid =
        csb_v1_boot_startup_runtime_asset_gate_from_launch_receipts_pc34(
            launch->profile, &launch->receipts,
            &out_receipt->startup_asset_gate);
    if (!out_receipt->startup_asset_gate_valid) {
        return 0;
    }
    out_receipt->profile = launch->profile;
    out_receipt->receipts = launch->receipts;
    snprintf(out_receipt->boot_asset_md5,
             sizeof(out_receipt->boot_asset_md5),
             "%s",
             launch->profile->graphics_md5);
    snprintf(out_receipt->title,
             sizeof(out_receipt->title),
             "CHAOS STRIKES BACK");
    snprintf(out_receipt->source_id,
             sizeof(out_receipt->source_id),
             "csb");
    out_receipt->bind_graphics_to_m11_asset_loader =
        launch->profile->graphics_path[0] != '\0';
    out_receipt->load_original_font_from_graphics =
        out_receipt->bind_graphics_to_m11_asset_loader;
    out_receipt->real_asset_receipt_valid =
        csb_v1_startup_real_receipt_from_profile_fields(
            launch->profile->asset_root,
            launch->profile->graphics_path,
            launch->profile->dungeon_path,
            launch->profile->graphics_md5,
            launch->profile->dungeon_md5,
            0u,
            0u,
            launch->profile->variant_id,
            launch->profile->graphics_kind,
            4,
            launch->profile->assets_verified,
            launch->profile->graphics_verified,
            launch->profile->dungeon_verified,
            &out_receipt->real_asset_receipt);
    snprintf(out_receipt->graphics_path,
             sizeof(out_receipt->graphics_path),
             "%s",
             launch->profile->graphics_path);
    snprintf(out_receipt->dungeon_path,
             sizeof(out_receipt->dungeon_path),
             "%s",
             launch->profile->dungeon_path[0]
                 ? launch->profile->dungeon_path
                 : "DUNGEON.DAT");
    launch->profile = NULL;
    return 1;
}

static int csb_v1_boot_startup_host_facts_from_runtime_state_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile)
{
    return csb_v1_startup_host_facts_from_runtime_state_pc34(
        facts,
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        entrance_frame,
        utility_overlay_active,
        utility_selected_action_index,
        utility_imported_champion_count,
        utility_preview_active,
        utility_prompt,
        resume_available,
        resume_path,
        boot_profile);
}

static int csb_v1_boot_startup_runtime_facts_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile)
{
    return csb_v1_boot_startup_host_facts_from_runtime_state_pc34(
        facts,
        title_active,
        title_frame,
        title_source_step,
        entrance_active,
        entrance_source_step,
        entrance_dismissed,
        credits_active,
        credits_remaining_ticks,
        opening_active,
        opening_delay_ticks,
        opening_step,
        pending_command,
        entrance_frame,
        utility_overlay_active,
        utility_selected_action_index,
        utility_imported_champion_count,
        utility_preview_active,
        utility_prompt,
        resume_available,
        resume_path,
        boot_profile);
}

static int csb_v1_boot_startup_facts_from_snapshot_pc34(
    CSB_V1_StartupHostFacts_PC34 *facts,
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot)
{
    if (!snapshot) {
        csb_v1_startup_host_facts_init_pc34(facts);
        return 0;
    }
    return csb_v1_boot_startup_runtime_facts_pc34(
        facts,
        snapshot->title_active,
        snapshot->title_frame,
        snapshot->title_source_step,
        snapshot->entrance_active,
        snapshot->entrance_source_step,
        snapshot->entrance_dismissed,
        snapshot->credits_active,
        snapshot->credits_remaining_ticks,
        snapshot->opening_active,
        snapshot->opening_delay_ticks,
        snapshot->opening_step,
        snapshot->pending_command,
        snapshot->entrance_frame,
        snapshot->utility_overlay_active,
        snapshot->utility_selected_action_index,
        snapshot->utility_imported_champion_count,
        snapshot->utility_preview_active,
        snapshot->utility_prompt,
        snapshot->resume_available,
        snapshot->resume_path,
        snapshot->boot_profile);
}

int csb_v1_boot_startup_advance_idle_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupIdleReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        csb_v1_startup_idle_receipt_init_pc34(out_receipt);
        return 0;
    }
    return csb_v1_startup_advance_idle_from_host_facts_with_receipt_pc34(
        &facts,
        out_receipt);
}

int csb_v1_boot_startup_entrance_accepts_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        return 0;
    }
    return csb_v1_startup_entrance_accepts_input_from_host_facts_pc34(
        &facts);
}

int csb_v1_boot_startup_presentation_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active,
    int *out_startup_frame,
    char *out_animation,
    int out_animation_size,
    int *out_animation_active,
    int *out_title_frame,
    int *out_title_frame_max,
    int *out_title_ready)
{
    CSB_V1_StartupPresentationReceipt_PC34 receipt;

    if (!csb_v1_boot_startup_presentation_state_receipt_from_snapshot_pc34(
            snapshot,
            &receipt)) {
        return 0;
    }
    if (out_phase && out_phase_size > 0) {
        snprintf(out_phase, (size_t)out_phase_size, "%s", receipt.phase);
    }
    if (out_startup_active) {
        *out_startup_active = receipt.startup_active;
    }
    if (out_startup_frame) {
        *out_startup_frame = receipt.startup_frame;
    }
    if (out_animation && out_animation_size > 0) {
        snprintf(out_animation, (size_t)out_animation_size, "%s",
                 receipt.animation);
    }
    if (out_animation_active) {
        *out_animation_active = receipt.animation_active;
    }
    if (out_title_frame) {
        *out_title_frame = receipt.title_frame;
    }
    if (out_title_frame_max) {
        *out_title_frame_max = receipt.title_frame_max;
    }
    if (out_title_ready) {
        *out_title_ready = receipt.title_ready;
    }
    return 1;
}

int csb_v1_boot_startup_presentation_state_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_StartupPresentationReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_startup_presentation_receipt_init_pc34(out_receipt);
    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        return 0;
    }
    return csb_v1_startup_presentation_receipt_from_host_facts_pc34(
        &facts,
        out_receipt);
}

static int csb_v1_boot_startup_presentation_route_receipt_from_facts_pc34(
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupPresentationReceipt_PC34 presentation;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_presentation_route_receipt_init_pc34(out_receipt);
    if (!facts ||
        !csb_v1_startup_presentation_receipt_from_host_facts_pc34(facts,
                                                                  &presentation)) {
        return 0;
    }
    csb_v1_boot_startup_route_from_presentation_pc34(&presentation,
                                                     facts,
                                                     out_receipt);
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 keeps input waiting
     * inside the entrance loop; CSBWin/Viewport.cpp mirrors CSB HUD/menu
     * ownership in the CSB view layer.  Export the utility render plan in
     * the same route receipt so M11 does not rebuild startup menu state. */
    if (out_receipt->route ==
            CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
        facts->utility_overlay_active &&
        csb_v1_runtime_util_render_plan_from_startup_host_facts_pc34(
            facts,
            &out_receipt->utility_plan)) {
        out_receipt->utility_plan_valid = 1;
        out_receipt->draw_utility_panel = 1;
        out_receipt->hud_menu_state.valid = 1;
        out_receipt->hud_menu_state.kind =
            CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34;
        out_receipt->hud_menu_state.utility_selected_action_index =
            facts->utility_selected_action_index;
        out_receipt->hud_menu_state.utility_preview_active =
            facts->utility_preview_active ? 1 : 0;
        out_receipt->hud_menu_state.utility_menu_row_count =
            out_receipt->utility_plan.menu_row_count;
        out_receipt->hud_menu_state.option_count =
            out_receipt->utility_plan.menu_row_count;
        snprintf(out_receipt->hud_menu_state.prompt,
                 sizeof(out_receipt->hud_menu_state.prompt),
                 "%s",
                 out_receipt->utility_plan.has_prompt_row
                     ? out_receipt->utility_plan.prompt_row.text
                     : "");
    }
    return out_receipt->valid;
}

int csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        if (out_receipt) {
            csb_v1_boot_startup_presentation_route_receipt_init_pc34(
                out_receipt);
        }
        return 0;
    }
    return csb_v1_boot_startup_presentation_route_receipt_from_facts_pc34(
        &facts,
        out_receipt);
}

void csb_v1_boot_startup_render_view_receipt_init_pc34(
    CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_presentation_route_receipt_init_pc34(
        &receipt->route_receipt);
}

void csb_v1_boot_startup_host_decision_receipt_init_pc34(
    CSB_V1_BootStartupHostDecisionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->utility_selected_action_index = -1;
    receipt->entrance_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->pre_render_route =
        CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->post_render_route =
        CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
}

void csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->selected_utility_action_index = -1;
    csb_v1_boot_startup_host_decision_receipt_init_pc34(
        &receipt->host_decision);
}

void csb_v1_boot_startup_input_render_receipt_init_pc34(
    CSB_V1_BootStartupInputRenderReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_action_receipt_init_pc34(&receipt->action);
    csb_v1_boot_startup_host_decision_receipt_init_pc34(
        &receipt->host_decision);
    csb_v1_boot_startup_readiness_receipt_init_pc34(
        &receipt->pre_input_readiness);
    csb_v1_boot_startup_readiness_receipt_init_pc34(
        &receipt->post_input_readiness);
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
        &receipt->hud_menu_draw);
}

void csb_v1_boot_startup_input_gate_receipt_init_pc34(
    CSB_V1_BootStartupInputGateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_readiness_receipt_init_pc34(&receipt->readiness);
    csb_v1_boot_startup_input_render_receipt_init_pc34(
        &receipt->input_render);
}

void csb_v1_boot_startup_capture_receipt_init_pc34(
    CSB_V1_BootStartupCaptureReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_presentation_route_receipt_init_pc34(
        &receipt->route);
    csb_v1_boot_startup_render_view_receipt_init_pc34(
        &receipt->render_view);
    csb_v1_boot_startup_readiness_receipt_init_pc34(
        &receipt->readiness);
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
        &receipt->hud_menu_draw);
    csb_v1_startup_real_receipt_init(&receipt->real_asset_receipt);
    receipt->render_route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->hud_menu_kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    receipt->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->selected_utility_action_index = -1;
}

void csb_v1_boot_startup_packaged_capture_proof_init_pc34(
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *proof)
{
    if (!proof) {
        return;
    }
    memset(proof, 0, sizeof(*proof));
    proof->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    proof->hud_menu_kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    proof->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    proof->selected_utility_action_index = -1;
    proof->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 850-883";
}

void csb_v1_boot_startup_visual_sequence_capture_receipt_init_pc34(
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 850-883; "
        "ENTRANCE.C F0438/F0807 door-opening frames";
}

void csb_v1_boot_startup_runtime_visual_capture_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_visual_sequence_capture_receipt_init_pc34(
        &receipt->visual_sequence);
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 850-883; "
        "ENTRANCE.C F0438/F0807 door-opening frames; "
        "CSBWin Viewport startup/HUD ownership";
}

void csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 850-883; "
        "ENTRANCE.C F0438/F0807 door-opening frames; "
        "CSBWin Viewport startup/HUD ownership";
}

void csb_v1_boot_startup_runtime_host_capture_gate_receipt_init_pc34(
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_runtime_visual_capture_receipt_init_pc34(
        &receipt->runtime_visual);
    csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
        &receipt->title_route_hardening);
    csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
        &receipt->closed_door_route_hardening);
    csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
        &receipt->utility_route_hardening);
    csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
        &receipt->door_opening_route_hardening);
    receipt->source_evidence =
        "ReDMCSB TITLE.C F0437 lines 424-463; "
        "ENTRANCE.C F0441/F0806 lines 850-883; "
        "ENTRANCE.C F0438/F0807 door-opening frames; "
        "CSBWin startup host loop keeps title/HUD/opening on one route";
}

void csb_v1_boot_startup_readiness_receipt_init_pc34(
    CSB_V1_BootStartupReadinessReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->hud_menu_kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    receipt->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->selected_utility_action_index = -1;
    snprintf(receipt->animation, sizeof(receipt->animation), "%s", "none");
}

static int csb_v1_boot_startup_render_view_receipt_from_route_pc34(
    const CSB_V1_BootStartupPresentationRouteReceipt_PC34 *route,
    CSB_V1_BootStartupRenderViewReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_render_view_receipt_init_pc34(out_receipt);
    if (!route || !route->valid) {
        return 0;
    }

    out_receipt->valid = route->valid;
    out_receipt->route_receipt = *route;
    out_receipt->render_plan = route->presentation.render_plan;
    out_receipt->render_plan_valid = route->presentation.valid;
    out_receipt->boot_executor_route =
        route->draw_title || route->draw_surface || route->draw_closed_doors ||
                route->draw_opening_frame || route->draw_fallback_text ||
                route->draw_utility_panel
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 lines 424-463 is the post-FTL CSB title
     * transaction; ENTRANCE.C F0441/F0806 lines 409-447 and 850-883 own the
     * closed-door wait/menu; F0438/F0807 owns door-opening frames.
     * CSBWin/Viewport.cpp keeps those HUD/menu surfaces under the CSB view.
     * This view receipt lets M11 consume one CSB-owned render/HUD route
     * instead of rebuilding title, door, and utility fallback gates. */
    out_receipt->title_after_swoosh_route =
        route->route == CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34 &&
                route->draw_title &&
                strcmp(route->presentation.animation, "csb-title") == 0
            ? 1
            : 0;
    if (out_receipt->title_after_swoosh_route) {
        out_receipt->title_stage = route->presentation.render_plan.title_stage;
        out_receipt->title_source_step =
            route->presentation.render_plan.title_source_step;
        out_receipt->title_frame = route->presentation.title_frame;
        out_receipt->title_frame_max = route->presentation.title_frame_max;
        /* ReDMCSB TITLE.C F0437 lines 424-463 draws CM58 PRESENTS for 60
         * ticks, animates 18 PC-path CM59 CHAOS frames through source steps
         * 2..19, holds CHAOS for two vblanks, then blits C426 STRIKES BACK.
         * CSBWin/Viewport.cpp keeps the active
         * title/HUD surface as view-owned state. Publish the exact stage and
         * source/destination route here so M11 does not derive
         * PRESENTS/CHAOS/STRIKES from frame math. */
        out_receipt->title_presents_visible =
            out_receipt->title_stage ==
                    CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34
                ? 1
                : 0;
        out_receipt->title_chaos_visible =
            out_receipt->title_stage ==
                    CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34
                ? 1
                : 0;
        out_receipt->title_strikes_back_visible =
            out_receipt->title_stage ==
                    CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34
                ? 1
                : 0;
        if (out_receipt->title_presents_visible) {
            out_receipt->title_phase_tick = out_receipt->title_frame;
            out_receipt->title_phase_tick_count =
                csb_v1_startup_title_presents_ticks_pc34();
        } else if (out_receipt->title_chaos_visible) {
            const int chaos_frame =
                out_receipt->title_frame -
                csb_v1_startup_title_presents_ticks_pc34();
            const int zoom_ticks =
                csb_v1_startup_title_chaos_zoom_ticks_pc34();
            out_receipt->title_chaos_zoom_visible =
                chaos_frame < zoom_ticks ? 1 : 0;
            out_receipt->title_chaos_hold_visible =
                chaos_frame >= zoom_ticks ? 1 : 0;
            out_receipt->title_phase_tick =
                out_receipt->title_chaos_zoom_visible
                    ? chaos_frame
                    : chaos_frame - zoom_ticks;
            out_receipt->title_phase_tick_count =
                out_receipt->title_chaos_zoom_visible
                    ? zoom_ticks
                    : csb_v1_startup_title_chaos_hold_ticks_pc34();
        } else if (out_receipt->title_strikes_back_visible) {
            out_receipt->title_phase_tick =
                out_receipt->title_frame -
                csb_v1_startup_title_presents_ticks_pc34() -
                csb_v1_startup_title_chaos_zoom_ticks_pc34() -
                csb_v1_startup_title_chaos_hold_ticks_pc34();
            out_receipt->title_phase_tick_count =
                csb_v1_startup_title_strikes_back_ticks_pc34();
        }
        out_receipt->title_render_command_count =
            route->presentation.render_plan.render_command_count;
        out_receipt->title_blit_kind =
            route->presentation.render_plan.title_blit_kind;
        out_receipt->title_transparent_color =
            route->presentation.render_plan.title_transparent_color;
        out_receipt->title_special_palette =
            route->presentation.render_plan.title_special_palette;
        out_receipt->title_source_x =
            route->presentation.render_plan.title_source_x;
        out_receipt->title_source_y =
            route->presentation.render_plan.title_source_y;
        out_receipt->title_source_w =
            route->presentation.render_plan.title_source_w;
        out_receipt->title_source_h =
            route->presentation.render_plan.title_source_h;
        out_receipt->title_dest_x =
            route->presentation.render_plan.title_dest_x;
        out_receipt->title_dest_y =
            route->presentation.render_plan.title_dest_y;
        out_receipt->title_dest_w =
            route->presentation.render_plan.title_dest_w;
        out_receipt->title_dest_h =
            route->presentation.render_plan.title_dest_h;
    }
    out_receipt->closed_door_menu_route =
        route->route == CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
                route->draw_closed_doors && route->hud_menu_visible
            ? 1
            : 0;
    if (out_receipt->closed_door_menu_route) {
        out_receipt->closed_door_render_command_count =
            route->presentation.render_plan.render_command_count;
        out_receipt->closed_door_asset_command_count =
            route->presentation.render_plan.asset_command_count;
        out_receipt->closed_door_menu_option_count =
            route->hud_menu_state.option_count;
        out_receipt->closed_door_selected_command_id =
            route->hud_menu_state.selected_command_id;
        out_receipt->closed_door_resume_enabled =
            route->hud_menu_state.resume_enabled;
        out_receipt->closed_door_resume_available =
            route->hud_menu_state.resume_available;
        out_receipt->closed_door_resume_option_visible =
            route->hud_menu_state.resume_option_visible;
        out_receipt->closed_door_resume_option_selected =
            route->hud_menu_state.resume_option_selected;
        snprintf(out_receipt->closed_door_prompt,
                 sizeof(out_receipt->closed_door_prompt),
                 "%s",
                 route->hud_menu_state.prompt);
    }
    out_receipt->utility_menu_route =
        route->route == CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
                route->draw_utility_panel &&
                route->utility_plan_valid &&
                route->hud_menu_state.valid &&
                route->hud_menu_state.kind ==
                    CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34
            ? 1
            : 0;
    if (out_receipt->utility_menu_route) {
        out_receipt->utility_menu_row_count =
            route->hud_menu_state.utility_menu_row_count;
        out_receipt->utility_selected_action_index =
            route->hud_menu_state.utility_selected_action_index;
        out_receipt->utility_preview_active =
            route->hud_menu_state.utility_preview_active;
        snprintf(out_receipt->utility_prompt,
                 sizeof(out_receipt->utility_prompt),
                 "%s",
                 route->hud_menu_state.prompt);
    }
    out_receipt->opening_door_route =
        (route->route ==
             CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_DELAY_PC34 ||
         route->route ==
             CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_FRAME_PC34) &&
                (route->draw_closed_doors || route->draw_opening_frame) &&
                !route->accepts_input
            ? 1
            : 0;
    out_receipt->hud_menu_receipt_ready =
        route->hud_menu_visible && route->hud_menu_state.valid ? 1 : 0;
    out_receipt->suppress_legacy_utility_fallback =
        route->route == CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34 &&
                route->hud_menu_visible && !route->draw_utility_panel &&
                !route->utility_plan_valid
            ? 1
            : 0;
    return out_receipt->valid;
}

int csb_v1_boot_startup_readiness_receipt_from_view_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *view,
    CSB_V1_BootStartupReadinessReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupHudMenuStateReceipt_PC34 *hud = NULL;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_readiness_receipt_init_pc34(out_receipt);
    if (!view || !view->valid || !view->route_receipt.valid) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->startup_active =
        view->route_receipt.presentation.startup_active ? 1 : 0;
    out_receipt->route = view->route_receipt.route;
    out_receipt->post_ftl_title_active =
        view->title_after_swoosh_route ? 1 : 0;
    out_receipt->title_ready =
        view->route_receipt.presentation.title_ready ? 1 : 0;
    out_receipt->title_frame = view->title_frame;
    out_receipt->title_frame_max = view->title_frame_max;
    out_receipt->title_stage = view->title_stage;
    out_receipt->title_presents_visible =
        view->title_presents_visible ? 1 : 0;
    out_receipt->title_chaos_visible = view->title_chaos_visible ? 1 : 0;
    out_receipt->title_chaos_zoom_visible =
        view->title_chaos_zoom_visible ? 1 : 0;
    out_receipt->title_chaos_hold_visible =
        view->title_chaos_hold_visible ? 1 : 0;
    out_receipt->title_strikes_back_visible =
        view->title_strikes_back_visible ? 1 : 0;
    out_receipt->title_phase_tick = view->title_phase_tick;
    out_receipt->title_phase_tick_count = view->title_phase_tick_count;
    out_receipt->input_ready =
        view->route_receipt.accepts_input ? 1 : 0;
    out_receipt->hud_menu_ready =
        view->hud_menu_receipt_ready ? 1 : 0;
    out_receipt->host_startup_input_ready =
        out_receipt->startup_active && out_receipt->input_ready ? 1 : 0;
    out_receipt->host_input_blocked =
        out_receipt->startup_active && !out_receipt->input_ready ? 1 : 0;
    out_receipt->host_startup_hud_ready =
        out_receipt->startup_active && out_receipt->hud_menu_ready ? 1 : 0;
    out_receipt->host_hud_blocked =
        out_receipt->startup_active && !out_receipt->hud_menu_ready ? 1 : 0;
    out_receipt->suppress_legacy_utility_fallback =
        view->suppress_legacy_utility_fallback ? 1 : 0;
    snprintf(out_receipt->animation, sizeof(out_receipt->animation), "%s",
             view->route_receipt.presentation.animation);

    hud = &view->route_receipt.hud_menu_state;
    if (view->hud_menu_receipt_ready && hud->valid) {
        out_receipt->hud_menu_kind = hud->kind;
        out_receipt->hud_menu_option_count = hud->option_count;
        out_receipt->utility_menu_row_count = hud->utility_menu_row_count;
        out_receipt->selected_command_id = hud->selected_command_id;
        out_receipt->selected_utility_action_index =
            hud->utility_selected_action_index;
        out_receipt->resume_available = hud->resume_available ? 1 : 0;
    }

    /* ReDMCSB TITLE.C F0437 lines 424-463 owns the post-FTL PRESENTS,
     * CHAOS zoom, and STRIKES BACK title sequence. ENTRANCE.C F0441/F0806
     * lines 850-883 owns the later closed-door HUD/menu wait loop. Keep
     * those readiness gates in one CSB receipt for M11 boot probes. */
    return 1;
}

int csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupReadinessReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupRenderViewReceipt_PC34 view;
    int receipt_valid;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_readiness_receipt_init_pc34(out_receipt);
    if (!csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
            snapshot,
            &view)) {
        if (snapshot && snapshot->runtime_level_loaded &&
            !snapshot->title_active && !snapshot->entrance_active &&
            !snapshot->opening_active && !snapshot->credits_active) {
            /* ReDMCSB ENTRANCE.C F0438/F0807 lines 725-790 exits the door
             * sequence before normal dungeon redraw resumes; CSBWin keeps the
             * viewport/HUD readiness in the CSB view transition instead of
             * deriving it in the host. */
            out_receipt->valid = 1;
            out_receipt->title_ready = 1;
            out_receipt->runtime_handoff_ready = 1;
            out_receipt->runtime_viewport_ready = 1;
            out_receipt->runtime_hud_ready = 1;
            out_receipt->host_runtime_input_ready = 1;
            out_receipt->host_runtime_hud_ready = 1;
            out_receipt->runtime_level_loaded =
                snapshot->runtime_level_loaded;
            out_receipt->runtime_map_index = snapshot->runtime_map_index;
            out_receipt->runtime_party_x = snapshot->runtime_party_x;
            out_receipt->runtime_party_y = snapshot->runtime_party_y;
            out_receipt->runtime_party_dir = snapshot->runtime_party_dir;
            out_receipt->runtime_champion_count =
                snapshot->runtime_champion_count;
            out_receipt->runtime_tick_count = snapshot->runtime_tick_count;
            return 1;
        }
        return 0;
    }
    receipt_valid = csb_v1_boot_startup_readiness_receipt_from_view_pc34(
        &view,
        out_receipt);
    if (receipt_valid && snapshot && snapshot->runtime_level_loaded &&
        !snapshot->title_active && !snapshot->entrance_active &&
        !snapshot->opening_active && !snapshot->credits_active) {
        /* ReDMCSB ENTRANCE.C F0438/F0807 lines 725-790 exits the door
         * sequence before normal dungeon redraw resumes; CSBWin keeps the
         * viewport/HUD readiness in the CSB view transition instead of
         * deriving it in the host. */
        out_receipt->runtime_handoff_ready = 1;
        out_receipt->runtime_viewport_ready = 1;
        out_receipt->runtime_hud_ready = 1;
        out_receipt->host_input_blocked = 0;
        out_receipt->host_startup_input_ready = 0;
        out_receipt->host_runtime_input_ready = 1;
        out_receipt->host_hud_blocked = 0;
        out_receipt->host_startup_hud_ready = 0;
        out_receipt->host_runtime_hud_ready = 1;
        out_receipt->runtime_level_loaded = snapshot->runtime_level_loaded;
        out_receipt->runtime_map_index = snapshot->runtime_map_index;
        out_receipt->runtime_party_x = snapshot->runtime_party_x;
        out_receipt->runtime_party_y = snapshot->runtime_party_y;
        out_receipt->runtime_party_dir = snapshot->runtime_party_dir;
        out_receipt->runtime_champion_count =
            snapshot->runtime_champion_count;
        out_receipt->runtime_tick_count = snapshot->runtime_tick_count;
    }
    return receipt_valid;
}

int csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupCaptureReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_capture_receipt_init_pc34(out_receipt);
    if (!snapshot) {
        return 0;
    }
    out_receipt->route_valid =
        csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->route);
    out_receipt->render_view_valid =
        csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->render_view);
    out_receipt->readiness_valid =
        csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->readiness);
    if (!out_receipt->route_valid && !out_receipt->render_view_valid &&
        !out_receipt->readiness_valid) {
        return 0;
    }
    if (out_receipt->render_view_valid) {
        out_receipt->hud_menu_draw_valid =
            csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
                &out_receipt->render_view,
                &out_receipt->hud_menu_draw);
    }
    if (snapshot->boot_profile) {
        out_receipt->real_asset_receipt_valid =
            csb_v1_startup_real_receipt_from_profile_fields(
                snapshot->boot_profile->asset_root,
                snapshot->boot_profile->graphics_path,
                snapshot->boot_profile->dungeon_path,
                snapshot->boot_profile->graphics_md5,
                snapshot->boot_profile->dungeon_md5,
                0u,
                0u,
                snapshot->boot_profile->variant_id,
                snapshot->boot_profile->graphics_kind,
                4,
                snapshot->boot_profile->assets_verified,
                snapshot->boot_profile->graphics_verified,
                snapshot->boot_profile->dungeon_verified,
                &out_receipt->real_asset_receipt);
    }
    out_receipt->valid = 1;
    out_receipt->render_route =
        out_receipt->readiness_valid
            ? out_receipt->readiness.route
            : (out_receipt->route_valid
                   ? out_receipt->route.route
                   : CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34);
    out_receipt->hud_menu_kind =
        out_receipt->readiness_valid
            ? out_receipt->readiness.hud_menu_kind
            : CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    out_receipt->title_capture_ready =
        out_receipt->render_view_valid &&
                out_receipt->render_view.title_after_swoosh_route
            ? 1
            : 0;
    out_receipt->hud_menu_capture_ready =
        out_receipt->readiness_valid && out_receipt->hud_menu_draw_valid &&
                out_receipt->readiness.hud_menu_ready &&
                !out_receipt->readiness.host_hud_blocked
            ? 1
            : 0;
    out_receipt->runtime_capture_ready =
        out_receipt->readiness_valid &&
                out_receipt->readiness.runtime_handoff_ready &&
                out_receipt->readiness.host_runtime_hud_ready
            ? 1
            : 0;
    out_receipt->host_input_blocked =
        out_receipt->readiness_valid &&
                out_receipt->readiness.host_input_blocked
            ? 1
            : 0;
    out_receipt->host_hud_blocked =
        out_receipt->readiness_valid && out_receipt->readiness.host_hud_blocked
            ? 1
            : 0;
    out_receipt->startup_input_ready =
        out_receipt->readiness_valid &&
                out_receipt->readiness.host_startup_input_ready
            ? 1
            : 0;
    out_receipt->startup_hud_ready =
        out_receipt->readiness_valid &&
                out_receipt->readiness.host_startup_hud_ready
            ? 1
            : 0;
    out_receipt->title_stage =
        out_receipt->render_view_valid ? out_receipt->render_view.title_stage : 0;
    out_receipt->title_frame =
        out_receipt->render_view_valid ? out_receipt->render_view.title_frame : 0;
    out_receipt->title_source_step =
        out_receipt->render_view_valid
            ? out_receipt->render_view.title_source_step
            : 0;
    out_receipt->selected_command_id =
        out_receipt->readiness_valid
            ? out_receipt->readiness.selected_command_id
            : CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    out_receipt->selected_utility_action_index =
        out_receipt->readiness_valid
            ? out_receipt->readiness.selected_utility_action_index
            : -1;
    out_receipt->suppress_legacy_utility_fallback =
        out_receipt->readiness_valid &&
                out_receipt->readiness.suppress_legacy_utility_fallback
            ? 1
            : 0;
    if (out_receipt->hud_menu_draw_valid) {
        out_receipt->hud_menu_kind = out_receipt->hud_menu_draw.kind;
        if (out_receipt->hud_menu_draw.kind ==
            CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34) {
            out_receipt->selected_command_id =
                out_receipt->hud_menu_draw.selected_command_id;
        } else if (out_receipt->hud_menu_draw.kind ==
                   CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34) {
            out_receipt->selected_utility_action_index =
                out_receipt->hud_menu_draw.selected_utility_action_index;
        }
        out_receipt->suppress_legacy_utility_fallback =
            out_receipt->hud_menu_draw.suppress_legacy_utility_fallback
                ? 1
                : out_receipt->suppress_legacy_utility_fallback;
    }
    /* ReDMCSB TITLE.C F0437 lines 424-463 owns the post-FTL title draw,
     * and ENTRANCE.C F0441/F0806 lines 850-883 owns the closed-door
     * input/HUD loop. This aggregate receipt is the CSB-owned capture
     * package for M11/probes, so callers consume route, render-view,
     * readiness, real-asset proof, and HUD/menu draw gates together. */
    return 1;
}

static int csb_v1_boot_startup_render_view_receipt_from_runtime_state_pc34(
    CSB_V1_BootStartupRenderViewReceipt_PC34 *out_receipt,
    int title_active,
    int title_frame,
    int title_source_step,
    int entrance_active,
    int entrance_source_step,
    int entrance_dismissed,
    int credits_active,
    int credits_remaining_ticks,
    int opening_active,
    int opening_delay_ticks,
    int opening_step,
    int pending_command,
    int entrance_frame,
    int utility_overlay_active,
    int utility_selected_action_index,
    int utility_imported_champion_count,
    int utility_preview_active,
    const char *utility_prompt,
    int resume_available,
    const char *resume_path,
    const CSB_V1_BootProfile *boot_profile)
{
    CSB_V1_StartupHostFacts_PC34 facts;
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 route;

    if (!csb_v1_boot_startup_runtime_facts_pc34(
            &facts,
            title_active,
            title_frame,
            title_source_step,
            entrance_active,
            entrance_source_step,
            entrance_dismissed,
            credits_active,
            credits_remaining_ticks,
            opening_active,
            opening_delay_ticks,
            opening_step,
            pending_command,
            entrance_frame,
            utility_overlay_active,
            utility_selected_action_index,
            utility_imported_champion_count,
            utility_preview_active,
            utility_prompt,
            resume_available,
            resume_path,
            boot_profile) ||
        !csb_v1_boot_startup_presentation_route_receipt_from_facts_pc34(
            &facts,
            &route)) {
        if (out_receipt) {
            csb_v1_boot_startup_render_view_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_boot_startup_render_view_receipt_from_route_pc34(
        &route,
        out_receipt);
}

int csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupRenderViewReceipt_PC34 *out_receipt)
{
    if (!snapshot) {
        if (out_receipt) {
            csb_v1_boot_startup_render_view_receipt_init_pc34(out_receipt);
        }
        return 0;
    }
    return csb_v1_boot_startup_render_view_receipt_from_runtime_state_pc34(
        out_receipt,
        snapshot->title_active,
        snapshot->title_frame,
        snapshot->title_source_step,
        snapshot->entrance_active,
        snapshot->entrance_source_step,
        snapshot->entrance_dismissed,
        snapshot->credits_active,
        snapshot->credits_remaining_ticks,
        snapshot->opening_active,
        snapshot->opening_delay_ticks,
        snapshot->opening_step,
        snapshot->pending_command,
        snapshot->entrance_frame,
        snapshot->utility_overlay_active,
        snapshot->utility_selected_action_index,
        snapshot->utility_imported_champion_count,
        snapshot->utility_preview_active,
        snapshot->utility_prompt,
        snapshot->resume_available,
        snapshot->resume_path,
        snapshot->boot_profile);
}

static int csb_v1_boot_startup_title_render_plan_from_view_receipt_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    CSB_V1_StartupAssetCommand_PC34 *asset;
    CSB_V1_StartupRenderCommand_PC34 *render;
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!receipt || !receipt->valid || !receipt->render_plan_valid ||
        !receipt->title_after_swoosh_route ||
        receipt->title_render_command_count <= 0 ||
        receipt->title_source_w <= 0 || receipt->title_source_h <= 0 ||
        receipt->title_dest_w <= 0 || receipt->title_dest_h <= 0 ||
        receipt->render_plan.source_asset_id <= 0) {
        return 0;
    }

    *out_plan = receipt->render_plan;
    out_plan->surface = CSB_V1_STARTUP_RENDER_TITLE_PC34;
    out_plan->title_stage = receipt->title_stage;
    out_plan->title_source_step = receipt->title_source_step;
    out_plan->title_blit_kind = receipt->title_blit_kind;
    out_plan->title_transparent_color = receipt->title_transparent_color;
    out_plan->title_special_palette = receipt->title_special_palette;
    out_plan->special_palette = receipt->title_special_palette;
    out_plan->title_source_x = receipt->title_source_x;
    out_plan->title_source_y = receipt->title_source_y;
    out_plan->title_source_w = receipt->title_source_w;
    out_plan->title_source_h = receipt->title_source_h;
    out_plan->title_dest_x = receipt->title_dest_x;
    out_plan->title_dest_y = receipt->title_dest_y;
    out_plan->title_dest_w = receipt->title_dest_w;
    out_plan->title_dest_h = receipt->title_dest_h;
    out_plan->asset_command_count = 0;
    out_plan->render_command_count = 0;

    if (receipt->title_blit_kind ==
        CSB_V1_STARTUP_TITLE_BLIT_REGION_PC34) {
        asset = &out_plan->asset_commands[out_plan->asset_command_count++];
        asset->kind = CSB_V1_STARTUP_ASSET_TITLE_REGION_PC34;
    } else if (receipt->title_blit_kind ==
               CSB_V1_STARTUP_TITLE_BLIT_SCALED_REGION_PC34) {
        asset = &out_plan->asset_commands[out_plan->asset_command_count++];
        asset->kind = CSB_V1_STARTUP_ASSET_TITLE_SCALED_REGION_PC34;
    } else {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 lines 424-463 draws PRESENTS, CHAOS zoom,
     * then STRIKES BACK. M11 consumes this receipt-built asset command so
     * it no longer reconstructs post-FTL title rectangles from host state. */
    asset->asset_id = out_plan->source_asset_id;
    asset->source_x = receipt->title_source_x;
    asset->source_y = receipt->title_source_y;
    asset->source_w = receipt->title_source_w;
    asset->source_h = receipt->title_source_h;
    asset->dest_x = receipt->title_dest_x;
    asset->dest_y = receipt->title_dest_y;
    asset->dest_w = receipt->title_dest_w;
    asset->dest_h = receipt->title_dest_h;
    asset->transparent_color = receipt->title_transparent_color;
    asset->visible = 1;

    render = &out_plan->render_commands[out_plan->render_command_count++];
    render->kind = CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34;
    render = &out_plan->render_commands[out_plan->render_command_count++];
    render->kind = CSB_V1_STARTUP_RENDER_COMMAND_TITLE_PC34;
    return 1;
}

static int csb_v1_boot_startup_capture_title_render_plan_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!capture_receipt || !capture_receipt->valid ||
        !capture_receipt->title_capture_ready ||
        !capture_receipt->render_view_valid ||
        !capture_receipt->real_asset_receipt_valid ||
        !capture_receipt->real_asset_receipt.matched) {
        return 0;
    }
    /* ReDMCSB TITLE.C F0437 lines 424-463 owns the post-FTL PRESENTS,
     * CHAOS, and STRIKES BACK draw route. Require the aggregate capture's
     * verified asset proof before exposing a title render plan to M11. */
    return csb_v1_boot_startup_title_render_plan_from_view_receipt_pc34(
        &capture_receipt->render_view,
        out_plan);
}

static int csb_v1_boot_startup_capture_render_plan_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    int i;
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!capture_receipt || !capture_receipt->valid ||
        !capture_receipt->render_view_valid ||
        !capture_receipt->render_view.render_plan_valid ||
        !capture_receipt->real_asset_receipt_valid ||
        !capture_receipt->real_asset_receipt.matched) {
        return 0;
    }

    if (capture_receipt->title_capture_ready) {
        return csb_v1_boot_startup_capture_title_render_plan_pc34(
            capture_receipt,
            out_plan);
    }

    if (capture_receipt->hud_menu_capture_ready &&
        capture_receipt->hud_menu_draw_valid &&
        capture_receipt->hud_menu_draw.startup_render_plan_valid) {
        if (capture_receipt->hud_menu_kind ==
            CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34) {
            return csb_v1_boot_startup_closed_door_menu_render_plan_from_view_receipt_pc34(
                &capture_receipt->render_view,
                out_plan);
        }
        *out_plan = capture_receipt->hud_menu_draw.startup_render_plan;
        /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 keeps the utility
         * overlay in the same full startup surface as the closed entrance.
         * Return the CSB-built base render plan so M11 can draw full startup
         * graphics before consuming the HUD/menu draw receipt. */
        return 1;
    }

    if (capture_receipt->render_view.route_receipt.route ==
        CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34) {
        *out_plan = capture_receipt->render_view.render_plan;
        for (i = 0; i < out_plan->render_command_count &&
                    i < CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34; ++i) {
            if (out_plan->render_commands[i].kind ==
                CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34) {
                out_plan->render_commands[i].kind =
                    CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34;
            }
        }
        /* ReDMCSB ENTRANCE.C F0442 draws Graphic5 as a full-screen credits
         * page while startup input/HUD are blocked. Keep that surface on the
         * packaged render path instead of letting M11 fall back to text. */
        return 1;
    }

    if (!capture_receipt->host_hud_blocked &&
        capture_receipt->render_view.boot_executor_route) {
        *out_plan = capture_receipt->render_view.render_plan;
        return 1;
    }

    if (capture_receipt->render_view.opening_door_route) {
        *out_plan = capture_receipt->render_view.render_plan;
        for (i = 0; i < out_plan->render_command_count &&
                    i < CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34; ++i) {
            if (out_plan->render_commands[i].kind ==
                CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34) {
                out_plan->render_commands[i].kind =
                    CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34;
            }
        }
        /* ReDMCSB ENTRANCE.C F0806 lines 857-883 moves from title/menu into
         * the door-open startup animation before runtime handoff. Keep that
         * full-start graphic path available through the aggregate receipt,
         * but strip the old fallback-door branch from the capture plan. */
        return 1;
    }

    return 0;
}

static int csb_v1_boot_startup_render_draw_receipt_from_capture_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_BootStartupRenderDrawReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_render_draw_receipt_init_pc34(out_receipt);
    if (!capture_receipt || !capture_receipt->valid ||
        !capture_receipt->real_asset_receipt_valid ||
        !capture_receipt->real_asset_receipt.matched) {
        return 0;
    }
    if (!csb_v1_boot_startup_capture_render_plan_pc34(
            capture_receipt,
            &out_receipt->render_plan)) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->render_plan_valid = 1;
    out_receipt->route = capture_receipt->render_route;
    out_receipt->surface = out_receipt->render_plan.surface;
    out_receipt->real_asset_matched = 1;
    out_receipt->title_draw_ready =
        capture_receipt->title_capture_ready ? 1 : 0;
    out_receipt->hud_menu_draw_ready =
        capture_receipt->hud_menu_capture_ready &&
                capture_receipt->hud_menu_draw_valid
            ? 1
            : 0;
    out_receipt->opening_draw_ready =
        capture_receipt->render_view_valid &&
                capture_receipt->render_view.opening_door_route
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 lines 424-463 and ENTRANCE.C F0441/F0806
     * lines 850-883 own CSB startup title/HUD/opening draws. This receipt
     * is the M11-facing draw boundary: callers consume the CSB capture's
     * real-asset-gated render receipt instead of a title-only planned copy. */
    return 1;
}

static int csb_v1_boot_startup_execute_host_view_render_plan_pc34(
    const CSB_V1_BootStartupHostViewReceipt_PC34 *host_view,
    const CSB_V1_StartupRenderExecutor_PC34 *executor)
{
    if (!host_view || !host_view->valid || !host_view->render_draw_valid ||
        !host_view->render_draw.valid ||
        !host_view->render_draw.render_plan_valid || !executor) {
        return 0;
    }
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806 keep title,
     * utility, closed-door, and door-opening drawing inside the startup
     * host loop. This lets host code consume the packaged host-view receipt
     * directly instead of rebuilding capture/render-view decisions. */
    return csb_v1_startup_execute_render_plan_pc34(
        &host_view->render_draw.render_plan,
        executor);
}

int csb_v1_boot_startup_execute_host_view_receipt_pc34(
    const CSB_V1_BootStartupHostViewReceipt_PC34 *host_view,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupRenderExecutor_PC34 render_executor;
    CSB_V1_StartupRenderExecutor_PC34 hud_executor;
    int render_result = 0;
    int hud_result = 0;

    if (out_receipt) {
        csb_v1_boot_startup_host_view_draw_receipt_init_pc34(out_receipt);
    }
    if (!host_view || !host_view->valid || !executor) {
        return 0;
    }

    if (out_receipt) {
        out_receipt->host_view_valid = 1;
        out_receipt->render_draw_valid =
            host_view->render_draw_valid && host_view->render_draw.valid
                ? 1
                : 0;
        out_receipt->hud_menu_draw_valid =
            host_view->hud_menu_draw_valid && host_view->hud_menu_draw.valid
                ? 1
                : 0;
        out_receipt->route = host_view->route;
        out_receipt->surface =
            host_view->render_draw_valid && host_view->render_draw.valid
                ? host_view->render_draw.surface
                : CSB_V1_STARTUP_RENDER_NONE_PC34;
        out_receipt->hud_menu_kind = host_view->hud_menu_kind;
        out_receipt->real_asset_matched =
            host_view->capture_proof_valid &&
                    host_view->capture_proof.real_asset_matched
                ? 1
                : 0;
        out_receipt->suppress_legacy_utility_fallback =
            host_view->capture_proof_valid &&
                    host_view->capture_proof.suppress_legacy_utility_fallback
                ? 1
                : 0;
        out_receipt->title_asset_draw_ready =
            host_view->render_draw_valid &&
                    host_view->render_draw.title_draw_ready &&
                    host_view->capture_proof_valid &&
                    host_view->capture_proof.title_route &&
                    host_view->capture_proof.real_asset_matched
                ? 1
                : 0;
        out_receipt->closed_door_asset_draw_ready =
            host_view->render_draw_valid &&
                    host_view->render_draw.hud_menu_draw_ready &&
                    host_view->capture_proof_valid &&
                    host_view->capture_proof.closed_door_menu_route &&
                    host_view->capture_proof.real_asset_matched
                ? 1
                : 0;
        out_receipt->opening_frame_draw_ready =
            host_view->render_draw_valid &&
                    host_view->render_draw.opening_draw_ready &&
                    host_view->capture_proof_valid &&
                    host_view->capture_proof.opening_door_route &&
                    host_view->capture_proof.real_asset_matched
                ? 1
                : 0;
        out_receipt->fallback_text_suppressed =
            host_view->capture_proof_valid &&
                    host_view->capture_proof.real_asset_matched &&
                    (host_view->capture_proof.title_route ||
                     host_view->capture_proof.closed_door_menu_route ||
                     host_view->capture_proof.utility_menu_route ||
                     host_view->capture_proof.opening_door_route ||
                     host_view->capture_proof.credits_route)
                ? 1
                : 0;
        out_receipt->fallback_callbacks_stripped = 1;
    }

    render_executor = *executor;
    hud_executor = *executor;
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806 render CSB startup
     * from concrete title/HUD/door surfaces. Host-view receipts are the M11
     * runtime boundary, so strip legacy fallback callbacks before any lower
     * startup renderer can observe them. */
    render_executor.draw_door_fallback = NULL;
    render_executor.draw_fallback_text = NULL;
    hud_executor.draw_door_fallback = NULL;
    hud_executor.draw_fallback_text = NULL;
    if (host_view->hud_menu_draw_valid && host_view->hud_menu_draw.valid) {
        render_executor.draw_closed_doors = NULL;
        render_executor.draw_utility_panel = NULL;
    }

    render_result = csb_v1_boot_startup_execute_host_view_render_plan_pc34(
        host_view,
        &render_executor);
    if (render_result) {
        if (out_receipt) {
            out_receipt->render_executed = 1;
        }
    }

    if (host_view->hud_menu_draw_valid && host_view->readiness_valid) {
        hud_result = csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
            &host_view->hud_menu_draw,
            &host_view->readiness,
            &hud_executor);
        if (out_receipt) {
            out_receipt->hud_menu_executed = hud_result;
        }
    }

    if (out_receipt) {
        out_receipt->valid =
            out_receipt->render_executed || out_receipt->hud_menu_executed
                ? 1
                : 0;
        out_receipt->consumed_host_view_only = out_receipt->valid ? 1 : 0;
    }
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806 own the visible CSB
     * boot transaction. Consume render, HUD/menu, readiness, and packaged
     * real-asset gates from one CSB host-view receipt so callers do not keep
     * a compatibility layer that separately fetches render plans and HUD
     * readiness from raw startup facts. */
    return out_receipt ? out_receipt->valid : (render_result || hud_result > 0);
}

static int csb_v1_boot_startup_host_input_dispatch_from_gate_pc34(
    const CSB_V1_BootStartupInputGateReceipt_PC34 *gate_receipt,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt);

int csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int include_menu_input,
    int menu_input,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_ownership_receipt_init_pc34(out_receipt);
    if (!snapshot || !executor) {
        return 0;
    }
    if (!csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->host_view) ||
        !out_receipt->host_view.valid) {
        return 0;
    }
    out_receipt->snapshot_capture_valid = 1;
    out_receipt->host_view_valid = 1;
    out_receipt->route = out_receipt->host_view.route;
    out_receipt->hud_menu_kind = out_receipt->host_view.hud_menu_kind;
    out_receipt->capture_proof_valid =
        out_receipt->host_view.capture_proof_valid ? 1 : 0;
    if (out_receipt->host_view.capture_proof_valid) {
        const CSB_V1_BootStartupPackagedCaptureProof_PC34 *proof =
            &out_receipt->host_view.capture_proof;
        out_receipt->packaged_capture_hash = proof->packaged_capture_hash;
        out_receipt->real_asset_matched = proof->real_asset_matched ? 1 : 0;
        out_receipt->title_capture_ready =
            proof->title_capture_ready ? 1 : 0;
        out_receipt->hud_menu_capture_ready =
            proof->hud_menu_capture_ready ? 1 : 0;
        out_receipt->runtime_capture_ready =
            proof->runtime_capture_ready ? 1 : 0;
        out_receipt->packaged_visual_capture_ready =
            proof->real_asset_matched &&
                    (proof->title_capture_ready ||
                     proof->hud_menu_capture_ready ||
                     proof->runtime_capture_ready ||
                     proof->credits_route ||
                     proof->opening_door_route) &&
                    proof->packaged_capture_hash != 0u
                ? 1
                : 0;
        out_receipt->title_draw_ready = proof->title_route ? 1 : 0;
        out_receipt->closed_door_menu_draw_ready =
            proof->closed_door_menu_route &&
                    proof->hud_menu_draw_available
                ? 1
                : 0;
        out_receipt->utility_menu_draw_ready =
            proof->utility_menu_route && proof->hud_menu_draw_available
                ? 1
                : 0;
        out_receipt->opening_draw_ready =
            proof->opening_door_route && proof->draw_opening_frame
                ? 1
                : 0;
        out_receipt->suppress_legacy_utility_fallback =
            proof->suppress_legacy_utility_fallback ? 1 : 0;
    }

    if (csb_v1_boot_startup_execute_host_view_receipt_pc34(
            &out_receipt->host_view,
            executor,
            &out_receipt->host_draw)) {
        out_receipt->host_draw_valid = 1;
        out_receipt->render_executed =
            out_receipt->host_draw.render_executed ? 1 : 0;
        out_receipt->hud_menu_executed =
            out_receipt->host_draw.hud_menu_executed;
        out_receipt->draw_consumes_receipt_only =
            out_receipt->host_draw.consumed_host_view_only ? 1 : 0;
    }

    if (include_menu_input) {
        if (csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
                snapshot,
                menu_input,
                &out_receipt->host_input)) {
            out_receipt->host_input_dispatch_valid = 1;
            out_receipt->host_input_blocked =
                out_receipt->host_input.host_input_blocked ? 1 : 0;
            out_receipt->startup_input_ready =
                out_receipt->host_input.startup_input_ready ? 1 : 0;
            out_receipt->should_dispatch_input =
                out_receipt->host_input.should_dispatch_input ? 1 : 0;
            out_receipt->should_ignore_input =
                out_receipt->host_input.should_ignore_input ? 1 : 0;
            out_receipt->input_redraws_hud_menu =
                out_receipt->host_input.input_render_valid &&
                        out_receipt->host_input.input_render
                            .startup_hud_draw_ready
                    ? 1
                    : 0;
            out_receipt->input_consumes_receipt_only =
                out_receipt->host_input.valid &&
                        out_receipt->host_input.input_render_valid
                    ? 1
                    : 0;
        }
    }

    out_receipt->host_route_wrappers_retired =
        out_receipt->host_view_valid &&
                out_receipt->host_draw_valid &&
                out_receipt->host_draw.consumed_host_view_only &&
                out_receipt->host_draw.fallback_callbacks_stripped &&
                (!include_menu_input ||
                 out_receipt->input_consumes_receipt_only ||
                 out_receipt->host_input_blocked ||
                 out_receipt->host_input_dispatch_valid)
            ? 1
            : 0;
    out_receipt->no_loose_render_plan_exports =
        out_receipt->host_route_wrappers_retired &&
                out_receipt->draw_consumes_receipt_only &&
                out_receipt->host_draw.fallback_text_suppressed &&
                (out_receipt->host_draw.render_executed ||
                 out_receipt->host_draw.hud_menu_executed) &&
                out_receipt->capture_proof_valid &&
                out_receipt->packaged_visual_capture_ready
            ? 1
            : 0;
    out_receipt->valid =
        out_receipt->host_view_valid &&
                out_receipt->capture_proof_valid &&
                out_receipt->packaged_visual_capture_ready &&
                out_receipt->host_draw_valid &&
                out_receipt->host_route_wrappers_retired &&
                out_receipt->no_loose_render_plan_exports &&
                (!include_menu_input ||
                 out_receipt->host_input_dispatch_valid)
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 plus ENTRANCE.C F0441/F0806 define one startup
     * ownership boundary: title/PRESENTS, closed-door HUD/menu, utility
     * menu, door-opening draw, and input dispatch all belong to the CSB
     * loop. This package is the M11-facing proof that host draw/input
     * consumed CSB receipts instead of falling back to compatibility helpers
     * built from loose render plans or raw startup facts. */
    return out_receipt->valid;
}

static int csb_v1_boot_startup_closed_door_menu_render_plan_from_view_receipt_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt,
    CSB_V1_StartupRenderPlan_PC34 *out_plan)
{
    int i;
    int selected_seen = 0;
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!receipt || !receipt->valid || !receipt->render_plan_valid ||
        !receipt->closed_door_menu_route ||
        !receipt->hud_menu_receipt_ready ||
        receipt->closed_door_render_command_count <= 0 ||
        receipt->closed_door_menu_option_count <= 0) {
        return 0;
    }

    *out_plan = receipt->render_plan;
    out_plan->surface = CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34;
    out_plan->waiting_for_input = 1;
    out_plan->render_command_count =
        receipt->closed_door_render_command_count;
    out_plan->asset_command_count =
        receipt->closed_door_asset_command_count;
    out_plan->menu_option_count =
        receipt->closed_door_menu_option_count;
    out_plan->fallback_prompt_text =
        receipt->closed_door_prompt[0] != '\0'
            ? receipt->closed_door_prompt
            : out_plan->fallback_prompt_text;
    out_plan->blink_prompt_visible =
        receipt->closed_door_prompt[0] != '\0' ? 1 : out_plan->blink_prompt_visible;
    out_plan->render_command_count = 0;
    out_plan->render_commands[out_plan->render_command_count++].kind =
        CSB_V1_STARTUP_RENDER_COMMAND_CLEAR_BLACK_PC34;
    out_plan->render_commands[out_plan->render_command_count++].kind =
        CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_PC34;
    out_plan->render_commands[out_plan->render_command_count++].kind =
        CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_PC34;
    out_plan->render_commands[out_plan->render_command_count++].kind =
        CSB_V1_STARTUP_RENDER_COMMAND_UTILITY_PANEL_IF_WAITING_PC34;

    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 owns closed-door
     * waiting/menu state. Rebuild selected command and resume availability
     * from the CSB view receipt, and use an asset-only door command so
     * capture consumers cannot satisfy this route with the old no-asset
     * fallback text/door path. */
    for (i = 0; i < out_plan->menu_option_count &&
                i < CSB_V1_STARTUP_MENU_OPTION_CAP_PC34; ++i) {
        CSB_V1_StartupMenuOption_PC34 *option = &out_plan->menu_options[i];
        option->selected =
            option->command_id == receipt->closed_door_selected_command_id
                ? 1
                : 0;
        if (option->selected) {
            selected_seen = 1;
        }
        if (option->command_id ==
            CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34) {
            option->enabled = receipt->closed_door_resume_enabled ? 1 : 0;
            option->selected =
                receipt->closed_door_resume_option_selected ? 1 : option->selected;
            if (option->selected) {
                selected_seen = 1;
            }
        }
    }
    if (!selected_seen && out_plan->menu_option_count > 0) {
        out_plan->menu_options[0].selected = 1;
    }
    return 1;
}

static int csb_v1_boot_startup_utility_render_plan_from_view_receipt_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *receipt,
    CSB_V1_UtilRenderPlan *out_plan)
{
    int i;
    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    if (!receipt || !receipt->valid || !receipt->utility_menu_route ||
        !receipt->route_receipt.utility_plan_valid ||
        receipt->utility_menu_row_count <= 0) {
        return 0;
    }

    *out_plan = receipt->route_receipt.utility_plan;
    out_plan->menu_row_count = receipt->utility_menu_row_count;
    out_plan->preview_active = receipt->utility_preview_active ? 1 : 0;
    if (receipt->utility_prompt[0] != '\0') {
        out_plan->has_prompt_row = 1;
        snprintf(out_plan->prompt_row.text,
                 sizeof(out_plan->prompt_row.text),
                 "%s",
                 receipt->utility_prompt);
    }

    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 and CSB utility
     * CEDTINC7/CEDTDATA strings own this startup menu surface. Re-apply
     * selected row, preview state, and prompt from the CSB render-view
     * receipt so HUD/menu callers do not infer them from host facts. */
    for (i = 0; i < out_plan->menu_row_count &&
                i < CSB_V1_UTIL_MENU_ROW_COUNT; ++i) {
        out_plan->menu_rows[i].selected =
            i == receipt->utility_selected_action_index ? 1 : 0;
    }
    return 1;
}

int csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *view,
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(out_receipt);
    if (!view || !view->valid || !view->route_receipt.valid) {
        return 0;
    }

    if (view->utility_menu_route) {
        if (!csb_v1_boot_startup_utility_render_plan_from_view_receipt_pc34(
                view,
                &out_receipt->utility_render_plan)) {
            return 0;
        }
        out_receipt->valid = 1;
        out_receipt->kind = CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34;
        out_receipt->route = view->route_receipt.route;
        out_receipt->startup_render_plan = view->render_plan;
        out_receipt->startup_render_plan_valid =
            view->render_plan_valid ? 1 : 0;
        out_receipt->utility_render_plan_valid = 1;
        out_receipt->draw_utility_panel = 1;
        out_receipt->option_count = view->utility_menu_row_count;
        out_receipt->selected_utility_action_index =
            view->utility_selected_action_index;
        snprintf(out_receipt->prompt,
                 sizeof(out_receipt->prompt),
                 "%s",
                 view->utility_prompt);
        /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 keeps the utility
         * overlay inside the startup wait loop. Publish a draw-ready receipt
         * so host render code can consume the CSB utility plan without
         * reinterpreting utility rows, preview state, or prompt text. */
        return 1;
    }

    if (view->closed_door_menu_route) {
        if (!csb_v1_boot_startup_closed_door_menu_render_plan_from_view_receipt_pc34(
                view,
                &out_receipt->startup_render_plan)) {
            return 0;
        }
        out_receipt->valid = 1;
        out_receipt->kind = CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34;
        out_receipt->route = view->route_receipt.route;
        out_receipt->startup_render_plan_valid = 1;
        out_receipt->draw_closed_doors = 1;
        out_receipt->draw_fallback_text = 0;
        out_receipt->suppress_legacy_utility_fallback =
            view->suppress_legacy_utility_fallback ? 1 : 0;
        out_receipt->option_count = view->closed_door_menu_option_count;
        out_receipt->selected_command_id =
            view->closed_door_selected_command_id;
        out_receipt->resume_enabled =
            view->closed_door_resume_enabled ? 1 : 0;
        out_receipt->resume_available =
            view->closed_door_resume_available ? 1 : 0;
        out_receipt->resume_option_visible =
            view->closed_door_resume_option_visible ? 1 : 0;
        out_receipt->resume_option_selected =
            view->closed_door_resume_option_selected ? 1 : 0;
        snprintf(out_receipt->prompt,
                 sizeof(out_receipt->prompt),
                 "%s",
                 view->closed_door_prompt);
        /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 owns closed-door
         * command rows, Resume gating, and the prompt. This draw receipt is
         * the CSB-owned consumption boundary for the entrance HUD/menu; it
         * deliberately does not advertise the legacy fallback-text branch. */
        return 1;
    }

    return 0;
}

int csb_v1_boot_startup_hud_menu_draw_receipt_from_action_pc34(
    const CSB_V1_BootStartupActionReceipt_PC34 *action,
    int prefer_post_input_render_view,
    CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupRenderViewReceipt_PC34 *view = NULL;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(out_receipt);
    if (!action) {
        return 0;
    }

    if (prefer_post_input_render_view &&
        action->post_input_render_view_valid) {
        view = &action->post_input_render_view;
        out_receipt->from_post_input_render_view = 1;
    } else if (action->pre_input_render_view_valid) {
        view = &action->pre_input_render_view;
    } else if (action->post_input_render_view_valid) {
        view = &action->post_input_render_view;
        out_receipt->from_post_input_render_view = 1;
    }
    if (!view ||
        !csb_v1_boot_startup_hud_menu_draw_receipt_from_view_pc34(
            view,
            out_receipt)) {
        return 0;
    }
    out_receipt->from_post_input_render_view =
        view == &action->post_input_render_view ? 1 : 0;
    if (csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
            action,
            &out_receipt->host_decision)) {
        out_receipt->host_decision_valid = 1;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 couples startup input
     * results to the next visible HUD/menu surface. Consume the action's
     * pre/post render-view receipt and flattened host decision together so
     * a caller does not have to re-route utility vs entrance draw paths. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_execute_hud_menu_draw_receipt_pc34(
    const CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *draw_receipt,
    const CSB_V1_BootStartupReadinessReceipt_PC34 *readiness_receipt,
    const CSB_V1_StartupRenderExecutor_PC34 *executor)
{
    int drew = 0;

    if (!draw_receipt || !draw_receipt->valid || !executor) {
        return 0;
    }
    if (readiness_receipt) {
        if (!readiness_receipt->valid || !readiness_receipt->hud_menu_ready ||
            readiness_receipt->host_hud_blocked ||
            readiness_receipt->hud_menu_kind != draw_receipt->kind) {
            return 0;
        }
    }

    if (draw_receipt->kind == CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34) {
        if (!draw_receipt->draw_utility_panel ||
            !draw_receipt->startup_render_plan_valid ||
            !draw_receipt->startup_render_plan.waiting_for_input ||
            !executor->draw_utility_panel) {
            return 0;
        }
        executor->draw_utility_panel(executor->user,
                                     &draw_receipt->startup_render_plan,
                                     &draw_receipt->utility_render_plan);
        return 1;
    }

    if (draw_receipt->kind == CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34) {
        if (!draw_receipt->startup_render_plan_valid) {
            return 0;
        }
        if (draw_receipt->draw_fallback_text) {
            return 0;
        }
        if (draw_receipt->draw_closed_doors && executor->draw_closed_doors) {
            executor->draw_closed_doors(executor->user,
                                        &draw_receipt->startup_render_plan);
            ++drew;
        }
        /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 keeps the
         * closed-door wait/menu and CSB utility overlay under the entrance
         * loop. This is the CSB-owned draw-consumption point for M11: callers
         * pass render-view/readiness receipts instead of inferring utility vs
         * closed-door HUD state from host-side render branches. */
        return drew;
    }

    return 0;
}

int csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *out_proof)
{
    CSB_V1_StartupRenderPlan_PC34 render_plan;
    uint32_t hash = 2166136261u;

    if (!out_proof) {
        return 0;
    }
    csb_v1_boot_startup_packaged_capture_proof_init_pc34(out_proof);
    if (!capture_receipt || !capture_receipt->valid) {
        return 0;
    }

    out_proof->capture_valid = 1;
    out_proof->real_asset_matched =
        capture_receipt->real_asset_receipt_valid &&
                capture_receipt->real_asset_receipt.matched
            ? 1
            : 0;
    if (!out_proof->real_asset_matched) {
        return 0;
    }

    out_proof->valid = 1;
    out_proof->real_asset_receipt_hash =
        capture_receipt->real_asset_receipt.receipt_hash;
    out_proof->route = capture_receipt->render_route;
    out_proof->hud_menu_kind = capture_receipt->hud_menu_kind;
    out_proof->title_capture_ready =
        capture_receipt->title_capture_ready ? 1 : 0;
    out_proof->hud_menu_capture_ready =
        capture_receipt->hud_menu_capture_ready ? 1 : 0;
    out_proof->runtime_capture_ready =
        capture_receipt->runtime_capture_ready ? 1 : 0;
    out_proof->hud_menu_draw_available =
        capture_receipt->hud_menu_capture_ready &&
                capture_receipt->hud_menu_draw_valid
            ? 1
            : 0;
    out_proof->render_plan_available =
        csb_v1_boot_startup_capture_render_plan_pc34(
            capture_receipt,
            &render_plan);
    if (capture_receipt->render_view_valid) {
        out_proof->boot_executor_route =
            capture_receipt->render_view.boot_executor_route ? 1 : 0;
        out_proof->title_route =
            capture_receipt->render_view.title_after_swoosh_route ? 1 : 0;
        out_proof->closed_door_menu_route =
            capture_receipt->render_view.closed_door_menu_route ? 1 : 0;
        out_proof->utility_menu_route =
            capture_receipt->render_view.utility_menu_route ? 1 : 0;
        out_proof->credits_route =
            capture_receipt->render_view.route_receipt.route ==
                    CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34
                ? 1
                : 0;
        out_proof->opening_door_route =
            capture_receipt->render_view.opening_door_route ? 1 : 0;
        out_proof->draw_opening_frame =
            capture_receipt->render_view.route_receipt.draw_opening_frame
                ? 1
                : 0;
    }
    if (capture_receipt->hud_menu_draw_valid) {
        out_proof->draw_closed_doors =
            capture_receipt->hud_menu_draw.draw_closed_doors ? 1 : 0;
        out_proof->draw_utility_panel =
            capture_receipt->hud_menu_draw.draw_utility_panel ? 1 : 0;
        out_proof->draw_fallback_text =
            capture_receipt->hud_menu_draw.draw_fallback_text ? 1 : 0;
        out_proof->hud_menu_option_count =
            capture_receipt->hud_menu_draw.option_count;
        out_proof->resume_available =
            capture_receipt->hud_menu_draw.resume_available ? 1 : 0;
        out_proof->resume_option_visible =
            capture_receipt->hud_menu_draw.resume_option_visible ? 1 : 0;
        out_proof->suppress_legacy_utility_fallback =
            capture_receipt->hud_menu_draw.suppress_legacy_utility_fallback
                ? 1
                : 0;
    } else if (capture_receipt->readiness_valid) {
        out_proof->hud_menu_option_count =
            capture_receipt->readiness.hud_menu_option_count;
    }
    if (capture_receipt->readiness_valid) {
        out_proof->host_input_blocked =
            capture_receipt->readiness.host_input_blocked ? 1 : 0;
        out_proof->host_hud_blocked =
            capture_receipt->readiness.host_hud_blocked ? 1 : 0;
        out_proof->startup_input_ready =
            capture_receipt->readiness.host_startup_input_ready ? 1 : 0;
        out_proof->startup_hud_ready =
            capture_receipt->readiness.host_startup_hud_ready ? 1 : 0;
        out_proof->utility_menu_row_count =
            capture_receipt->readiness.utility_menu_row_count;
    }
    out_proof->title_stage = capture_receipt->title_stage;
    out_proof->title_frame = capture_receipt->title_frame;
    out_proof->selected_command_id = capture_receipt->selected_command_id;
    out_proof->selected_utility_action_index =
        capture_receipt->selected_utility_action_index;

    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)(out_proof->real_asset_receipt_hash & 0xffffffffu));
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)(out_proof->real_asset_receipt_hash >> 32));
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->hud_menu_kind);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->title_capture_ready);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->hud_menu_capture_ready);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->runtime_capture_ready);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->render_plan_available);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->hud_menu_draw_available);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->title_stage);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->title_frame);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->selected_command_id);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)(out_proof->selected_utility_action_index + 1));
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->boot_executor_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->title_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->closed_door_menu_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->utility_menu_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->credits_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->opening_door_route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->draw_closed_doors);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->draw_utility_panel);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->draw_fallback_text);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->draw_opening_frame);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->host_input_blocked);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->host_hud_blocked);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->startup_input_ready);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->startup_hud_ready);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)(out_proof->hud_menu_option_count + 1));
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)(out_proof->utility_menu_row_count + 1));
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->resume_available);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->resume_option_visible);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)out_proof->suppress_legacy_utility_fallback);
    out_proof->packaged_capture_hash = hash ? hash : 1u;
    /* ReDMCSB TITLE.C F0437 lines 424-463 and ENTRANCE.C F0441/F0806
     * lines 850-883 define the visible startup package. This proof is a
     * compact CSB-owned receipt for M11/tests: asset hash, render route,
     * title/HUD/runtime capture flags, draw availability, route consumer
     * selection, menu counts, Resume gating, and host input/HUD blockers are
     * bound together instead of inferred by a host consumer. */
    return out_proof->valid;
}

int csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *out_proof)
{
    CSB_V1_BootStartupCaptureReceipt_PC34 capture_receipt;

    if (!out_proof) {
        return 0;
    }
    csb_v1_boot_startup_packaged_capture_proof_init_pc34(out_proof);
    if (!snapshot ||
        !csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
            snapshot,
            &capture_receipt)) {
        return 0;
    }
    return csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
        &capture_receipt,
        out_proof);
}

static void csb_v1_boot_startup_visual_base_snapshot_pc34(
    CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    const CSB_V1_BootProfile *boot_profile)
{
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->boot_profile = boot_profile;
    snapshot->entrance_active = 1;
    snapshot->entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot->pending_command = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    snapshot->resume_available = 1;
    snapshot->resume_path = "/tmp/firestaff-csb-resume.dat";
}

static int csb_v1_boot_startup_visual_title_sample_pc34(
    const CSB_V1_BootProfile *boot_profile,
    int frame,
    int source_step,
    int expected_stage,
    uint32_t *out_hash)
{
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 proof;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.title_active = 1;
    snapshot.title_frame = frame;
    snapshot.title_source_step = source_step;
    if (!csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
            &snapshot,
            &proof) ||
        !proof.valid ||
        !proof.real_asset_matched ||
        !proof.title_capture_ready ||
        !proof.title_route ||
        !proof.render_plan_available ||
        proof.title_stage != expected_stage ||
        proof.draw_fallback_text ||
        proof.hud_menu_draw_available) {
        return 0;
    }
    if (out_hash) {
        *out_hash = proof.packaged_capture_hash;
    }
    return 1;
}

static int csb_v1_boot_startup_visual_packaged_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupPackagedCaptureProof_PC34 *proof,
    uint32_t *out_hash)
{
    if (!csb_v1_boot_startup_packaged_capture_proof_from_snapshot_pc34(
            snapshot,
            proof) ||
        !proof->valid ||
        !proof->real_asset_matched ||
        !proof->render_plan_available) {
        return 0;
    }
    if (out_hash) {
        *out_hash = proof->packaged_capture_hash;
    }
    return 1;
}

int csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *out_receipt)
{
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupPackagedCaptureProof_PC34 proof;
    uint32_t sequence_hash = 2166136261u;
    int i;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_visual_sequence_capture_receipt_init_pc34(out_receipt);
    if (!boot_profile) {
        return 0;
    }

    out_receipt->source_title_presents_ticks =
        csb_v1_startup_title_presents_ticks_pc34();
    out_receipt->source_title_chaos_zoom_ticks =
        csb_v1_startup_title_chaos_zoom_ticks_pc34();
    out_receipt->source_title_chaos_hold_ticks =
        csb_v1_startup_title_chaos_hold_ticks_pc34();
    out_receipt->source_title_strikes_back_ticks =
        csb_v1_startup_title_strikes_back_ticks_pc34();
    out_receipt->source_door_pre_open_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    out_receipt->source_door_step_count =
        ENTRANCE_Compat_GetDoorAnimationStepCount();

    out_receipt->title_presents_capture_ready =
        csb_v1_boot_startup_visual_title_sample_pc34(
            boot_profile,
            0,
            1,
            CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34,
            &out_receipt->title_sample_hashes[0]);
    out_receipt->title_chaos_zoom_capture_ready =
        csb_v1_boot_startup_visual_title_sample_pc34(
            boot_profile,
            out_receipt->source_title_presents_ticks,
            2,
            CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
            &out_receipt->title_sample_hashes[1]);
    out_receipt->title_chaos_hold_capture_ready =
        csb_v1_boot_startup_visual_title_sample_pc34(
            boot_profile,
            out_receipt->source_title_presents_ticks +
                out_receipt->source_title_chaos_zoom_ticks,
            19,
            CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34,
            &out_receipt->title_sample_hashes[2]);
    out_receipt->title_strikes_back_capture_ready =
        csb_v1_boot_startup_visual_title_sample_pc34(
            boot_profile,
            csb_v1_startup_title_total_ticks_pc34() - 1,
            20,
            CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34,
            &out_receipt->title_sample_hashes[3]);
    out_receipt->title_sample_count =
        CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
    out_receipt->title_unique_sample_hash_count =
        csb_v1_boot_count_unique_hashes_pc34(
            out_receipt->title_sample_hashes,
            CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34);
    out_receipt->title_all_stages_captured =
        out_receipt->title_presents_capture_ready &&
                out_receipt->title_chaos_zoom_capture_ready &&
                out_receipt->title_chaos_hold_capture_ready &&
                out_receipt->title_strikes_back_capture_ready &&
                out_receipt->title_unique_sample_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34
            ? 1
            : 0;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 0;
    out_receipt->closed_door_hud_capture_ready =
        csb_v1_boot_startup_visual_packaged_snapshot_pc34(
            &snapshot,
            &proof,
            &out_receipt->closed_door_hud_hash) &&
                proof.closed_door_menu_route &&
                proof.hud_menu_capture_ready &&
                proof.hud_menu_draw_available &&
                proof.draw_closed_doors &&
                !proof.draw_fallback_text;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.utility_prompt = "CHAOS STRIKES BACK READY";
    out_receipt->utility_hud_capture_ready =
        csb_v1_boot_startup_visual_packaged_snapshot_pc34(
            &snapshot,
            &proof,
            &out_receipt->utility_hud_hash) &&
                proof.utility_menu_route &&
                proof.hud_menu_capture_ready &&
                proof.hud_menu_draw_available &&
                proof.draw_utility_panel &&
                !proof.draw_fallback_text;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    snapshot.opening_step = 0;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    out_receipt->door_opening_delay_capture_ready =
        csb_v1_boot_startup_visual_packaged_snapshot_pc34(
            &snapshot,
            &proof,
            &out_receipt->door_opening_delay_hash) &&
                proof.opening_door_route &&
                !proof.hud_menu_draw_available &&
                !proof.draw_fallback_text;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 3;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    out_receipt->door_opening_frame_capture_ready =
        csb_v1_boot_startup_visual_packaged_snapshot_pc34(
            &snapshot,
            &proof,
            &out_receipt->door_opening_frame_hash) &&
                proof.opening_door_route &&
                proof.draw_opening_frame &&
                !proof.hud_menu_draw_available &&
                !proof.draw_fallback_text;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.credits_active = 1;
    snapshot.credits_remaining_ticks =
        csb_v1_startup_entrance_credits_ticks_pc34();
    out_receipt->credits_capture_ready =
        csb_v1_boot_startup_visual_packaged_snapshot_pc34(
            &snapshot,
            &proof,
            &out_receipt->credits_hash) &&
                proof.credits_route &&
                !proof.hud_menu_draw_available &&
                !proof.draw_fallback_text;

    out_receipt->real_asset_matched =
        out_receipt->title_all_stages_captured ? 1 : 0;
    out_receipt->hud_menu_draw_available =
        out_receipt->closed_door_hud_capture_ready &&
                out_receipt->utility_hud_capture_ready
            ? 1
            : 0;
    out_receipt->opening_frame_draw_available =
        out_receipt->door_opening_frame_capture_ready ? 1 : 0;
    out_receipt->no_fallback_text_routes =
        out_receipt->title_all_stages_captured &&
                out_receipt->closed_door_hud_capture_ready &&
                out_receipt->utility_hud_capture_ready &&
                out_receipt->door_opening_delay_capture_ready &&
                out_receipt->door_opening_frame_capture_ready &&
                out_receipt->credits_capture_ready
            ? 1
            : 0;
    out_receipt->no_legacy_door_fallback_routes =
        out_receipt->door_opening_delay_capture_ready &&
                out_receipt->door_opening_frame_capture_ready
            ? 1
            : 0;

    for (i = 0; i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34; ++i) {
        sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
            sequence_hash,
            out_receipt->title_sample_hashes[i]);
    }
    sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        sequence_hash,
        out_receipt->closed_door_hud_hash);
    sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        sequence_hash,
        out_receipt->utility_hud_hash);
    sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        sequence_hash,
        out_receipt->door_opening_delay_hash);
    sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        sequence_hash,
        out_receipt->door_opening_frame_hash);
    sequence_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        sequence_hash,
        out_receipt->credits_hash);
    out_receipt->sequence_capture_hash = sequence_hash ? sequence_hash : 1u;
    out_receipt->valid =
        out_receipt->title_all_stages_captured &&
                out_receipt->closed_door_hud_capture_ready &&
                out_receipt->utility_hud_capture_ready &&
                out_receipt->door_opening_delay_capture_ready &&
                out_receipt->door_opening_frame_capture_ready &&
                out_receipt->credits_capture_ready &&
                out_receipt->no_fallback_text_routes &&
                out_receipt->no_legacy_door_fallback_routes &&
                out_receipt->sequence_capture_hash != 0u
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 samples every title phase that CSB shows after
     * the FTL swoosh. ENTRANCE.C F0441/F0806 and F0438/F0807 own the closed
     * doors, utility HUD, credits, and door-opening frames. This aggregate
     * receipt binds those runtime-visible phases to the same packaged asset
     * capture path, so host callers cannot prove startup with a title-only
     * sample or a text/door fallback route. */
    return out_receipt->valid;
}

static int csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *ownership,
    uint32_t *runtime_hash)
{
    uint32_t hash;

    if (!snapshot || !executor || !ownership || !runtime_hash) {
        return 0;
    }
    if (!csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
            snapshot,
            0,
            CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
            executor,
            ownership) ||
        !ownership->valid ||
        !ownership->host_draw_valid ||
        !ownership->draw_consumes_receipt_only ||
        !ownership->capture_proof_valid ||
        !ownership->real_asset_matched ||
        !ownership->packaged_visual_capture_ready) {
        return 0;
    }

    hash = *runtime_hash ? *runtime_hash : 2166136261u;
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        ownership->packaged_capture_hash);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->hud_menu_kind);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->host_draw.render_executed);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->host_draw.hud_menu_executed);
    *runtime_hash = hash ? hash : 1u;
    return 1;
}

int csb_v1_boot_startup_runtime_visual_capture_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupRuntimeVisualCaptureReceipt_PC34 *out_receipt)
{
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 ownership;
    static const int title_source_steps[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34] = {
        1,
        2,
        19,
        20
    };
    int title_frames[CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34];
    uint32_t runtime_hash = 2166136261u;
    int fallback_callbacks_stripped = 1;
    int title_i;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_runtime_visual_capture_receipt_init_pc34(out_receipt);
    if (!boot_profile || !executor) {
        return 0;
    }
    if (!csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
            boot_profile,
            &out_receipt->visual_sequence) ||
        !out_receipt->visual_sequence.valid) {
        return 0;
    }
    title_frames[0] = 0;
    title_frames[1] = out_receipt->visual_sequence.source_title_presents_ticks;
    title_frames[2] =
        out_receipt->visual_sequence.source_title_presents_ticks +
        out_receipt->visual_sequence.source_title_chaos_zoom_ticks;
    title_frames[3] = csb_v1_startup_title_total_ticks_pc34() - 1;

    out_receipt->visual_sequence_valid = 1;
    out_receipt->real_asset_matched =
        out_receipt->visual_sequence.real_asset_matched ? 1 : 0;
    out_receipt->sequence_capture_hash =
        out_receipt->visual_sequence.sequence_capture_hash;

    for (title_i = 0;
         title_i < CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34;
         ++title_i) {
        csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
        snapshot.title_active = 1;
        snapshot.title_frame = title_frames[title_i];
        snapshot.title_source_step = title_source_steps[title_i];
        if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
                &snapshot,
                executor,
                &ownership,
                &runtime_hash) &&
            ownership.title_capture_ready &&
            ownership.title_draw_ready &&
            ownership.host_view.capture_proof.title_stage ==
                (title_i == 0
                     ? CSB_V1_STARTUP_STAGE_TITLE_PRESENTS_PC34
                     : (title_i == 3
                            ? CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34
                            : CSB_V1_STARTUP_STAGE_TITLE_CHAOS_ZOOM_PC34)) &&
            ownership.host_view.capture_proof.packaged_capture_hash ==
                out_receipt->visual_sequence.title_sample_hashes[title_i]) {
            out_receipt->title_runtime_sample_hashes[title_i] =
                ownership.host_view.capture_proof.packaged_capture_hash;
            ++out_receipt->title_runtime_sample_count;
            if (title_i == 0) {
                out_receipt->title_presents_runtime_consumed = 1;
            } else if (title_i == 1) {
                out_receipt->title_chaos_zoom_runtime_consumed = 1;
            } else if (title_i == 2) {
                out_receipt->title_chaos_hold_runtime_consumed = 1;
            } else if (title_i == 3) {
                out_receipt->title_strikes_back_runtime_consumed = 1;
            }
            fallback_callbacks_stripped =
                fallback_callbacks_stripped &&
                ownership.host_draw.fallback_callbacks_stripped;
        }
    }
    out_receipt->title_runtime_all_stages_consumed =
        out_receipt->title_runtime_sample_count ==
                CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34 &&
                out_receipt->title_presents_runtime_consumed &&
                out_receipt->title_chaos_zoom_runtime_consumed &&
                out_receipt->title_chaos_hold_runtime_consumed &&
                out_receipt->title_strikes_back_runtime_consumed &&
                csb_v1_boot_count_unique_hashes_pc34(
                    out_receipt->title_runtime_sample_hashes,
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34) ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34
            ? 1
            : 0;
    out_receipt->title_runtime_unique_sample_hash_count =
        csb_v1_boot_count_unique_hashes_pc34(
            out_receipt->title_runtime_sample_hashes,
            CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34);
    out_receipt->title_runtime_consumed =
        out_receipt->title_runtime_all_stages_consumed;
    out_receipt->title_draw_consumed =
        out_receipt->title_runtime_all_stages_consumed;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 0;
    if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
            &snapshot,
            executor,
            &ownership,
            &runtime_hash) &&
        ownership.hud_menu_capture_ready &&
        ownership.closed_door_menu_draw_ready) {
        out_receipt->closed_door_hud_runtime_consumed = 1;
        out_receipt->closed_door_hud_draw_consumed = 1;
        fallback_callbacks_stripped =
            fallback_callbacks_stripped &&
            ownership.host_draw.fallback_callbacks_stripped;
    }

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.utility_prompt = "CHAOS STRIKES BACK READY";
    if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
            &snapshot,
            executor,
            &ownership,
            &runtime_hash) &&
        ownership.hud_menu_capture_ready &&
        ownership.utility_menu_draw_ready) {
        out_receipt->utility_hud_runtime_consumed = 1;
        out_receipt->utility_hud_draw_consumed = 1;
        fallback_callbacks_stripped =
            fallback_callbacks_stripped &&
            ownership.host_draw.fallback_callbacks_stripped;
    }

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks =
        csb_v1_startup_entrance_pre_open_delay_ticks_pc34();
    snapshot.opening_step = 0;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
            &snapshot,
            executor,
            &ownership,
            &runtime_hash) &&
        ownership.host_view.capture_proof.opening_door_route) {
        out_receipt->door_opening_delay_runtime_consumed = 1;
        fallback_callbacks_stripped =
            fallback_callbacks_stripped &&
            ownership.host_draw.fallback_callbacks_stripped;
    }

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 3;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
            &snapshot,
            executor,
            &ownership,
            &runtime_hash) &&
        ownership.opening_draw_ready) {
        out_receipt->door_opening_frame_runtime_consumed = 1;
        out_receipt->door_opening_frame_draw_consumed = 1;
        fallback_callbacks_stripped =
            fallback_callbacks_stripped &&
            ownership.host_draw.fallback_callbacks_stripped;
    }

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.credits_active = 1;
    snapshot.credits_remaining_ticks =
        csb_v1_startup_entrance_credits_ticks_pc34();
    if (csb_v1_boot_startup_runtime_visual_consume_snapshot_pc34(
            &snapshot,
            executor,
            &ownership,
            &runtime_hash) &&
        ownership.host_view.capture_proof.credits_route) {
        out_receipt->credits_runtime_consumed = 1;
        out_receipt->credits_surface_draw_consumed = 1;
        fallback_callbacks_stripped =
            fallback_callbacks_stripped &&
            ownership.host_draw.fallback_callbacks_stripped;
    }

    out_receipt->runtime_capture_hash = runtime_hash ? runtime_hash : 1u;
    out_receipt->no_fallback_callbacks =
        out_receipt->title_runtime_consumed &&
                out_receipt->title_runtime_all_stages_consumed &&
                out_receipt->closed_door_hud_runtime_consumed &&
                out_receipt->utility_hud_runtime_consumed &&
                out_receipt->door_opening_delay_runtime_consumed &&
                out_receipt->door_opening_frame_runtime_consumed &&
                out_receipt->credits_runtime_consumed &&
                fallback_callbacks_stripped
            ? 1
            : 0;
    out_receipt->no_wrapper_fallback_routes =
        out_receipt->visual_sequence.no_fallback_text_routes &&
                out_receipt->visual_sequence.no_legacy_door_fallback_routes
            ? 1
            : 0;
    out_receipt->draw_consumes_receipt_only =
        out_receipt->no_fallback_callbacks ? 1 : 0;
    out_receipt->input_consumes_receipt_only = 1;
    out_receipt->valid =
                out_receipt->visual_sequence_valid &&
                out_receipt->real_asset_matched &&
                out_receipt->title_runtime_consumed &&
                out_receipt->title_runtime_all_stages_consumed &&
                out_receipt->closed_door_hud_runtime_consumed &&
                out_receipt->utility_hud_runtime_consumed &&
                out_receipt->door_opening_delay_runtime_consumed &&
                out_receipt->door_opening_frame_runtime_consumed &&
                out_receipt->credits_runtime_consumed &&
                out_receipt->no_fallback_callbacks &&
                out_receipt->no_wrapper_fallback_routes &&
                out_receipt->runtime_capture_hash != 0u
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806/F0438 are consumed
     * here through the same CSB host-view executor path used by M11. This
     * closes the old proof gap where title/HUD/door-opening capture could be
     * asserted after only the PRESENTS title frame, without executing the
     * CHAOS zoom/hold and STRIKES BACK runtime draw receipts. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
    const CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *visual_sequence,
    const CSB_V1_BootStartupHostOwnershipReceipt_PC34 *ownership,
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupPackagedCaptureProof_PC34 *proof = NULL;
    uint32_t hash = 2166136261u;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_runtime_route_hardening_receipt_init_pc34(
        out_receipt);
    if (!visual_sequence || !ownership || !ownership->capture_proof_valid) {
        return 0;
    }
    proof = &ownership->host_view.capture_proof;
    out_receipt->visual_sequence_valid =
        visual_sequence->valid &&
                visual_sequence->title_all_stages_captured &&
                visual_sequence->closed_door_hud_capture_ready &&
                visual_sequence->utility_hud_capture_ready &&
                visual_sequence->door_opening_delay_capture_ready &&
                visual_sequence->door_opening_frame_capture_ready &&
                visual_sequence->credits_capture_ready &&
                visual_sequence->no_fallback_text_routes &&
                visual_sequence->no_legacy_door_fallback_routes
            ? 1
            : 0;
    out_receipt->host_ownership_valid =
        ownership->valid && ownership->host_draw_valid &&
                ownership->capture_proof_valid &&
                ownership->packaged_visual_capture_ready
            ? 1
            : 0;
    out_receipt->real_asset_matched =
        visual_sequence->real_asset_matched && ownership->real_asset_matched
            ? 1
            : 0;
    out_receipt->title_route_covered =
        proof->title_route && visual_sequence->title_all_stages_captured &&
                ownership->title_capture_ready && ownership->title_draw_ready
            ? 1
            : 0;
    out_receipt->closed_door_hud_route_covered =
        proof->closed_door_menu_route &&
                visual_sequence->closed_door_hud_capture_ready &&
                ownership->hud_menu_capture_ready &&
                ownership->closed_door_menu_draw_ready
            ? 1
            : 0;
    out_receipt->utility_hud_route_covered =
        proof->utility_menu_route &&
                visual_sequence->utility_hud_capture_ready &&
                ownership->hud_menu_capture_ready &&
                ownership->utility_menu_draw_ready
            ? 1
            : 0;
    out_receipt->door_opening_route_covered =
        proof->opening_door_route &&
                visual_sequence->door_opening_delay_capture_ready &&
                visual_sequence->door_opening_frame_capture_ready
            ? 1
            : 0;
    out_receipt->credits_route_covered =
        proof->credits_route && visual_sequence->credits_capture_ready ? 1 : 0;
    out_receipt->route_covered_by_full_capture =
        out_receipt->title_route_covered ||
                out_receipt->closed_door_hud_route_covered ||
                out_receipt->utility_hud_route_covered ||
                out_receipt->door_opening_route_covered ||
                out_receipt->credits_route_covered
            ? 1
            : 0;
    out_receipt->no_fallback_text_route =
        !proof->draw_fallback_text &&
                visual_sequence->no_fallback_text_routes &&
                ownership->host_draw.fallback_text_suppressed
            ? 1
            : 0;
    out_receipt->no_legacy_door_fallback_route =
        visual_sequence->no_legacy_door_fallback_routes &&
                (proof->opening_door_route ? !proof->draw_fallback_text : 1)
            ? 1
            : 0;
    out_receipt->host_draw_consumes_receipt_only =
        ownership->draw_consumes_receipt_only &&
                ownership->host_draw.consumed_host_view_only
            ? 1
            : 0;
    out_receipt->input_consumes_receipt_only =
        ownership->input_consumes_receipt_only || ownership->host_input_blocked
            ? 1
            : 0;

    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        visual_sequence->sequence_capture_hash);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        ownership->packaged_capture_hash);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->route);
    hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        hash,
        (uint32_t)ownership->hud_menu_kind);
    out_receipt->route_hardening_hash = hash ? hash : 1u;
    out_receipt->valid =
        out_receipt->visual_sequence_valid &&
                out_receipt->host_ownership_valid &&
                out_receipt->real_asset_matched &&
                out_receipt->route_covered_by_full_capture &&
                out_receipt->no_fallback_text_route &&
                out_receipt->no_legacy_door_fallback_route &&
                out_receipt->host_draw_consumes_receipt_only &&
                out_receipt->route_hardening_hash != 0u
            ? 1
            : 0;
    /* ReDMCSB TITLE.C/ENTRANCE.C and CSBWin keep title, HUD, utility, door
     * opening, and credits on the CSB startup route. This receipt hardens the
     * current host-owned draw against proving a route that was not part of
     * the full visual capture, or that still wants fallback text/door draws. */
    return out_receipt->valid;
}

static int csb_v1_boot_startup_runtime_host_gate_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    const CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 *visual_sequence,
    CSB_V1_BootStartupRuntimeRouteHardeningReceipt_PC34 *out_hardening,
    uint32_t *gate_hash,
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *out_ownership)
{
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 ownership;
    if (!snapshot || !executor || !visual_sequence || !out_hardening ||
        !gate_hash) {
        return 0;
    }
    if (!csb_v1_boot_startup_execute_host_ownership_receipt_from_snapshot_pc34(
            snapshot,
            1,
            CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34,
            executor,
            &ownership) ||
        !ownership.valid ||
        !csb_v1_boot_startup_runtime_route_hardening_receipt_from_ownership_pc34(
            visual_sequence,
            &ownership,
            out_hardening) ||
        !out_hardening->valid) {
        return 0;
    }
    *gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        *gate_hash ? *gate_hash : 2166136261u,
        out_hardening->route_hardening_hash);
    *gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        *gate_hash,
        ownership.packaged_capture_hash);
    if (out_ownership) {
        *out_ownership = ownership;
    }
    return 1;
}

static void csb_v1_boot_startup_runtime_host_gate_copy_ownership_pc34(
    const CSB_V1_BootStartupHostOwnershipReceipt_PC34 *ownership,
    int *ownership_valid,
    int *draw_consumes_receipt_only,
    int *input_consumes_receipt_only,
    uint32_t *packaged_capture_hash)
{
    if (!ownership) {
        return;
    }
    if (ownership_valid) {
        *ownership_valid = ownership->valid &&
                ownership->host_view_valid &&
                ownership->host_draw_valid &&
                ownership->capture_proof_valid &&
                ownership->packaged_visual_capture_ready
            ? 1
            : 0;
    }
    if (draw_consumes_receipt_only) {
        *draw_consumes_receipt_only =
            ownership->draw_consumes_receipt_only ? 1 : 0;
    }
    if (input_consumes_receipt_only) {
        *input_consumes_receipt_only =
            ownership->input_consumes_receipt_only ||
                    ownership->host_input_blocked ||
                    ownership->host_input_dispatch_valid
                ? 1
                : 0;
    }
    if (packaged_capture_hash) {
        *packaged_capture_hash = ownership->packaged_capture_hash;
    }
}

int csb_v1_boot_startup_runtime_host_capture_gate_receipt_from_profile_pc34(
    const CSB_V1_BootProfile *boot_profile,
    const CSB_V1_StartupRenderExecutor_PC34 *executor,
    CSB_V1_BootStartupRuntimeHostCaptureGateReceipt_PC34 *out_receipt)
{
    static const CSB_V1_StartupAssetRole_PC34 startup_asset_roles[] = {
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_PRESENTS_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_CHAOS_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_TITLE_STRIKES_BACK_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_LEFT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_RIGHT_DOOR_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_SCREEN_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_ENTRANCE_CREDITS_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_HUD_INVENTORY_PC34,
        CSB_V1_STARTUP_ASSET_ROLE_HUD_RESURRECT_PC34
    };
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 title_ownership;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 closed_door_ownership;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 utility_ownership;
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 door_opening_ownership;
    CSB_V1_BootProfile resolved_profile;
    const CSB_V1_BootProfile *asset_profile;
    uint32_t gate_hash = 2166136261u;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_runtime_host_capture_gate_receipt_init_pc34(
        out_receipt);
    csb_v1_boot_startup_host_ownership_receipt_init_pc34(&title_ownership);
    csb_v1_boot_startup_host_ownership_receipt_init_pc34(&closed_door_ownership);
    csb_v1_boot_startup_host_ownership_receipt_init_pc34(&utility_ownership);
    csb_v1_boot_startup_host_ownership_receipt_init_pc34(&door_opening_ownership);
    if (!boot_profile || !executor) {
        return 0;
    }
    resolved_profile = *boot_profile;
    if (!resolved_profile.startup_assets.real_graphics_available) {
        csb_v1_boot_startup_assets_resolve_pc34(&resolved_profile);
    }
    asset_profile = &resolved_profile;
    if (!csb_v1_boot_startup_runtime_visual_capture_receipt_from_profile_pc34(
            boot_profile,
            executor,
            &out_receipt->runtime_visual) ||
        !out_receipt->runtime_visual.valid ||
        !out_receipt->runtime_visual.visual_sequence.valid) {
        return 0;
    }

    out_receipt->runtime_visual_valid = 1;
    out_receipt->visual_sequence_valid =
        out_receipt->runtime_visual.visual_sequence_valid ? 1 : 0;
    out_receipt->sequence_capture_hash =
        out_receipt->runtime_visual.sequence_capture_hash;
    out_receipt->runtime_capture_hash =
        out_receipt->runtime_visual.runtime_capture_hash;
    out_receipt->real_startup_asset_binding_hash =
        csb_v1_boot_startup_asset_binding_hash_pc34(
            asset_profile,
            startup_asset_roles,
            sizeof(startup_asset_roles) / sizeof(startup_asset_roles[0]),
            &out_receipt->real_startup_asset_role_count);
    out_receipt->real_startup_assets_bound =
        out_receipt->real_startup_asset_binding_hash != 0u &&
                out_receipt->real_startup_asset_role_count ==
                    (int)(sizeof(startup_asset_roles) /
                          sizeof(startup_asset_roles[0]))
            ? 1
            : 0;

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.title_active = 1;
    snapshot.title_frame = 0;
    snapshot.title_source_step = 1;
    if (!csb_v1_boot_startup_runtime_host_gate_snapshot_pc34(
            &snapshot,
            executor,
            &out_receipt->runtime_visual.visual_sequence,
            &out_receipt->title_route_hardening,
            &gate_hash,
            &title_ownership) ||
        !out_receipt->title_route_hardening.title_route_covered) {
        return 0;
    }
    csb_v1_boot_startup_runtime_host_gate_copy_ownership_pc34(
        &title_ownership,
        &out_receipt->title_host_ownership_valid,
        &out_receipt->title_host_draw_consumes_receipt_only,
        &out_receipt->title_host_input_consumes_receipt_only,
        &out_receipt->title_packaged_capture_hash);

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 0;
    if (!csb_v1_boot_startup_runtime_host_gate_snapshot_pc34(
            &snapshot,
            executor,
            &out_receipt->runtime_visual.visual_sequence,
            &out_receipt->closed_door_route_hardening,
            &gate_hash,
            &closed_door_ownership) ||
        !out_receipt->closed_door_route_hardening.closed_door_hud_route_covered) {
        return 0;
    }
    csb_v1_boot_startup_runtime_host_gate_copy_ownership_pc34(
        &closed_door_ownership,
        &out_receipt->closed_door_host_ownership_valid,
        &out_receipt->closed_door_host_draw_consumes_receipt_only,
        &out_receipt->closed_door_host_input_consumes_receipt_only,
        &out_receipt->closed_door_packaged_capture_hash);

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.utility_overlay_active = 1;
    snapshot.utility_selected_action_index = 0;
    snapshot.utility_imported_champion_count = 2;
    snapshot.utility_prompt = "CHAOS STRIKES BACK READY";
    if (!csb_v1_boot_startup_runtime_host_gate_snapshot_pc34(
            &snapshot,
            executor,
            &out_receipt->runtime_visual.visual_sequence,
            &out_receipt->utility_route_hardening,
            &gate_hash,
            &utility_ownership) ||
        !out_receipt->utility_route_hardening.utility_hud_route_covered) {
        return 0;
    }
    csb_v1_boot_startup_runtime_host_gate_copy_ownership_pc34(
        &utility_ownership,
        &out_receipt->utility_host_ownership_valid,
        &out_receipt->utility_host_draw_consumes_receipt_only,
        &out_receipt->utility_host_input_consumes_receipt_only,
        &out_receipt->utility_packaged_capture_hash);

    csb_v1_boot_startup_visual_base_snapshot_pc34(&snapshot, boot_profile);
    snapshot.opening_active = 1;
    snapshot.opening_delay_ticks = 0;
    snapshot.opening_step = 3;
    snapshot.pending_command =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34;
    if (!csb_v1_boot_startup_runtime_host_gate_snapshot_pc34(
            &snapshot,
            executor,
            &out_receipt->runtime_visual.visual_sequence,
            &out_receipt->door_opening_route_hardening,
            &gate_hash,
            &door_opening_ownership) ||
        !out_receipt->door_opening_route_hardening.door_opening_route_covered) {
        return 0;
    }
    csb_v1_boot_startup_runtime_host_gate_copy_ownership_pc34(
        &door_opening_ownership,
        &out_receipt->door_opening_host_ownership_valid,
        &out_receipt->door_opening_host_draw_consumes_receipt_only,
        &out_receipt->door_opening_host_input_consumes_receipt_only,
        &out_receipt->door_opening_packaged_capture_hash);

    out_receipt->route_hardening_valid =
        out_receipt->title_route_hardening.valid &&
                out_receipt->closed_door_route_hardening.valid &&
                out_receipt->utility_route_hardening.valid &&
                out_receipt->door_opening_route_hardening.valid
            ? 1
            : 0;
    out_receipt->title_runtime_captured =
        out_receipt->runtime_visual.title_runtime_consumed &&
                out_receipt->runtime_visual.title_draw_consumed
            ? 1
            : 0;
    out_receipt->title_runtime_unique_sample_hash_count =
        out_receipt->runtime_visual.title_runtime_unique_sample_hash_count;
    out_receipt->title_presents_runtime_captured =
        out_receipt->runtime_visual.title_presents_runtime_consumed;
    out_receipt->title_chaos_zoom_runtime_captured =
        out_receipt->runtime_visual.title_chaos_zoom_runtime_consumed;
    out_receipt->title_chaos_hold_runtime_captured =
        out_receipt->runtime_visual.title_chaos_hold_runtime_consumed;
    out_receipt->title_strikes_back_runtime_captured =
        out_receipt->runtime_visual.title_strikes_back_runtime_consumed;
    out_receipt->title_runtime_expected_phase_mask = 0x0f;
    out_receipt->title_runtime_phase_mask =
        (out_receipt->title_presents_runtime_captured ? 0x01 : 0) |
        (out_receipt->title_chaos_zoom_runtime_captured ? 0x02 : 0) |
        (out_receipt->title_chaos_hold_runtime_captured ? 0x04 : 0) |
        (out_receipt->title_strikes_back_runtime_captured ? 0x08 : 0);
    out_receipt->title_runtime_phase_route_complete =
        out_receipt->title_runtime_captured &&
                out_receipt->title_runtime_phase_mask ==
                    out_receipt->title_runtime_expected_phase_mask &&
                out_receipt->title_runtime_unique_sample_hash_count ==
                    CSB_V1_BOOT_STARTUP_TITLE_SAMPLE_COUNT_PC34
            ? 1
            : 0;
    out_receipt->title_runtime_phase_hash =
        csb_v1_boot_packaged_capture_hash_step_pc34(
            csb_v1_boot_packaged_capture_hash_step_pc34(
                csb_v1_boot_packaged_capture_hash_step_pc34(
                    (uint32_t)out_receipt->title_runtime_phase_mask,
                    (uint32_t)out_receipt->title_runtime_expected_phase_mask),
                (uint32_t)out_receipt->title_runtime_unique_sample_hash_count),
            (uint32_t)out_receipt->runtime_visual
                .title_runtime_sample_count);
    if (out_receipt->title_runtime_phase_hash == 0u) {
        out_receipt->title_runtime_phase_hash = 1u;
    }
    out_receipt->closed_door_hud_runtime_captured =
        out_receipt->runtime_visual.closed_door_hud_runtime_consumed &&
                out_receipt->runtime_visual.closed_door_hud_draw_consumed
            ? 1
            : 0;
    out_receipt->utility_hud_runtime_captured =
        out_receipt->runtime_visual.utility_hud_runtime_consumed &&
                out_receipt->runtime_visual.utility_hud_draw_consumed
            ? 1
            : 0;
    out_receipt->door_opening_runtime_captured =
        out_receipt->runtime_visual.door_opening_delay_runtime_consumed &&
                out_receipt->runtime_visual
                    .door_opening_frame_runtime_consumed &&
                out_receipt->runtime_visual.door_opening_frame_draw_consumed
            ? 1
            : 0;
    out_receipt->credits_runtime_captured =
        out_receipt->runtime_visual.credits_runtime_consumed &&
                out_receipt->runtime_visual.credits_surface_draw_consumed
            ? 1
            : 0;
    out_receipt->all_runtime_routes_consumed =
        out_receipt->title_runtime_phase_route_complete &&
                out_receipt->closed_door_hud_runtime_captured &&
                out_receipt->utility_hud_runtime_captured &&
                out_receipt->door_opening_runtime_captured &&
                out_receipt->credits_runtime_captured
            ? 1
            : 0;
    out_receipt->draw_consumes_receipt_only =
        out_receipt->runtime_visual.draw_consumes_receipt_only &&
                out_receipt->title_route_hardening.host_draw_consumes_receipt_only &&
                out_receipt->closed_door_route_hardening.host_draw_consumes_receipt_only &&
                out_receipt->utility_route_hardening.host_draw_consumes_receipt_only &&
                out_receipt->door_opening_route_hardening.host_draw_consumes_receipt_only &&
                out_receipt->title_host_draw_consumes_receipt_only &&
                out_receipt->closed_door_host_draw_consumes_receipt_only &&
                out_receipt->utility_host_draw_consumes_receipt_only &&
                out_receipt->door_opening_host_draw_consumes_receipt_only
            ? 1
            : 0;
    out_receipt->input_consumes_receipt_only =
        out_receipt->runtime_visual_valid &&
                out_receipt->title_route_hardening.valid &&
                out_receipt->closed_door_route_hardening.valid &&
                out_receipt->utility_route_hardening.valid &&
                out_receipt->door_opening_route_hardening.valid &&
                out_receipt->title_host_input_consumes_receipt_only &&
                out_receipt->closed_door_host_input_consumes_receipt_only &&
                out_receipt->utility_host_input_consumes_receipt_only &&
                out_receipt->door_opening_host_input_consumes_receipt_only
            ? 1
            : 0;
    out_receipt->no_fallback_callbacks =
        out_receipt->runtime_visual.no_fallback_callbacks ? 1 : 0;
    out_receipt->no_wrapper_fallback_routes =
        out_receipt->runtime_visual.no_wrapper_fallback_routes &&
                out_receipt->title_route_hardening.no_fallback_text_route &&
                out_receipt->closed_door_route_hardening.no_fallback_text_route &&
                out_receipt->utility_route_hardening.no_fallback_text_route &&
                out_receipt->door_opening_route_hardening
                    .no_legacy_door_fallback_route
            ? 1
            : 0;
    out_receipt->host_route_wrappers_retired =
        title_ownership.host_route_wrappers_retired &&
                closed_door_ownership.host_route_wrappers_retired &&
                utility_ownership.host_route_wrappers_retired &&
                door_opening_ownership.host_route_wrappers_retired
            ? 1
            : 0;
    out_receipt->no_loose_render_plan_exports =
        title_ownership.no_loose_render_plan_exports &&
                closed_door_ownership.no_loose_render_plan_exports &&
                utility_ownership.no_loose_render_plan_exports &&
                door_opening_ownership.no_loose_render_plan_exports
            ? 1
            : 0;
    out_receipt->route_hardening_hash = gate_hash ? gate_hash : 1u;
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        out_receipt->sequence_capture_hash);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        out_receipt->runtime_capture_hash);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        out_receipt->route_hardening_hash);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->title_runtime_phase_mask);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->title_runtime_expected_phase_mask);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->title_runtime_phase_route_complete);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        out_receipt->title_runtime_phase_hash);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        out_receipt->real_startup_asset_binding_hash);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->real_startup_asset_role_count);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->host_route_wrappers_retired);
    gate_hash = csb_v1_boot_packaged_capture_hash_step_pc34(
        gate_hash,
        (uint32_t)out_receipt->no_loose_render_plan_exports);
    out_receipt->runtime_host_gate_hash = gate_hash ? gate_hash : 1u;
    out_receipt->valid =
        out_receipt->runtime_visual_valid &&
                out_receipt->visual_sequence_valid &&
                out_receipt->route_hardening_valid &&
                out_receipt->title_host_ownership_valid &&
                out_receipt->closed_door_host_ownership_valid &&
                out_receipt->utility_host_ownership_valid &&
                out_receipt->door_opening_host_ownership_valid &&
                out_receipt->all_runtime_routes_consumed &&
                out_receipt->draw_consumes_receipt_only &&
                out_receipt->input_consumes_receipt_only &&
                out_receipt->no_fallback_callbacks &&
                out_receipt->no_wrapper_fallback_routes &&
                out_receipt->real_startup_assets_bound &&
                out_receipt->host_route_wrappers_retired &&
                out_receipt->no_loose_render_plan_exports &&
                out_receipt->runtime_host_gate_hash != 0u
            ? 1
            : 0;
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806/F0438 execute title,
     * HUD/menu, credits, and door-opening as one startup host path. CSBWin
     * keeps PRESENTS, CHAOS zoom/hold, and STRIKES BACK as runtime-visible
     * view states. This gate now requires every title phase bit plus the HUD
     * routes before M11 can treat CSB startup as captured. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
    const CSB_V1_BootStartupCaptureReceipt_PC34 *capture_receipt,
    CSB_V1_BootStartupHostViewReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupHudMenuDrawReceipt_PC34 *hud = NULL;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_view_receipt_init_pc34(out_receipt);
    if (!capture_receipt || !capture_receipt->valid) {
        return 0;
    }

    out_receipt->valid = 1;
    if (capture_receipt->route_valid &&
        capture_receipt->route.presentation.valid) {
        snprintf(out_receipt->phase,
                 sizeof(out_receipt->phase),
                 "%s",
                 capture_receipt->route.presentation.phase);
        snprintf(out_receipt->animation,
                 sizeof(out_receipt->animation),
                 "%s",
                 capture_receipt->route.presentation.animation);
        out_receipt->startup_active =
            capture_receipt->route.presentation.startup_active ? 1 : 0;
        out_receipt->startup_frame =
            capture_receipt->route.presentation.startup_frame;
        out_receipt->animation_active =
            capture_receipt->route.presentation.animation_active ? 1 : 0;
        out_receipt->title_frame =
            capture_receipt->route.presentation.title_frame;
        out_receipt->title_frame_max =
            capture_receipt->route.presentation.title_frame_max;
        out_receipt->title_ready =
            capture_receipt->route.presentation.title_ready ? 1 : 0;
        out_receipt->route = (int)capture_receipt->route.route;
        out_receipt->special_palette = capture_receipt->route.special_palette;
    }

    if (capture_receipt->readiness_valid) {
        out_receipt->readiness_valid = 1;
        out_receipt->readiness = capture_receipt->readiness;
        snprintf(out_receipt->animation,
                 sizeof(out_receipt->animation),
                 "%s",
                 capture_receipt->readiness.animation);
        out_receipt->startup_active =
            capture_receipt->readiness.startup_active ? 1 : 0;
        out_receipt->title_frame = capture_receipt->readiness.title_frame;
        out_receipt->title_frame_max =
            capture_receipt->readiness.title_frame_max;
        out_receipt->title_ready =
            capture_receipt->readiness.title_ready ? 1 : 0;
        out_receipt->startup_input_ready =
            capture_receipt->startup_input_ready ? 1 : 0;
        out_receipt->startup_hud_menu_ready =
            capture_receipt->startup_hud_ready ? 1 : 0;
        out_receipt->startup_hud_runtime_ready =
            capture_receipt->readiness.host_runtime_hud_ready ? 1 : 0;
        out_receipt->hud_menu_kind = capture_receipt->hud_menu_kind;
        out_receipt->hud_menu_option_count =
            capture_receipt->readiness.hud_menu_option_count;
        out_receipt->selected_command_id =
            capture_receipt->selected_command_id;
        out_receipt->selected_utility_action_index =
            capture_receipt->selected_utility_action_index;
        out_receipt->runtime_handoff_ready =
            capture_receipt->readiness.runtime_handoff_ready ? 1 : 0;
        out_receipt->runtime_level_loaded =
            capture_receipt->readiness.runtime_level_loaded;
        out_receipt->runtime_map_index =
            capture_receipt->readiness.runtime_map_index;
        out_receipt->runtime_party_x =
            capture_receipt->readiness.runtime_party_x;
        out_receipt->runtime_party_y =
            capture_receipt->readiness.runtime_party_y;
        out_receipt->runtime_party_dir =
            capture_receipt->readiness.runtime_party_dir;
        out_receipt->runtime_champion_count =
            capture_receipt->readiness.runtime_champion_count;
        out_receipt->runtime_tick_count =
            capture_receipt->readiness.runtime_tick_count;
    }

    if (capture_receipt->hud_menu_capture_ready &&
        capture_receipt->hud_menu_draw_valid) {
        hud = &capture_receipt->hud_menu_draw;
        out_receipt->hud_menu_draw_valid = 1;
        out_receipt->hud_menu_draw = *hud;
        out_receipt->hud_menu_kind = hud->kind;
        out_receipt->readiness.hud_menu_kind = hud->kind;
        out_receipt->readiness.hud_menu_ready = 1;
        out_receipt->readiness.host_hud_blocked = 0;
        out_receipt->readiness.host_startup_hud_ready = 1;
        out_receipt->hud_menu_option_count = hud->option_count;
        out_receipt->selected_command_id = hud->selected_command_id;
        out_receipt->selected_utility_action_index =
            hud->selected_utility_action_index;
    }

    out_receipt->render_plan_valid =
        csb_v1_boot_startup_capture_render_plan_pc34(
            capture_receipt,
            &out_receipt->render_plan);
    out_receipt->render_draw_valid =
        csb_v1_boot_startup_render_draw_receipt_from_capture_pc34(
            capture_receipt,
            &out_receipt->render_draw);
    out_receipt->capture_proof_valid =
        csb_v1_boot_startup_packaged_capture_proof_from_capture_pc34(
            capture_receipt,
            &out_receipt->capture_proof);
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0441/F0806 keep title,
     * input/HUD readiness, menu rows, and runtime handoff in one startup
     * loop. This host-view receipt is the CSB-owned consumer surface for
     * M11 boot probes and draw/input guards. */
    return 1;
}

int csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupHostViewReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupCaptureReceipt_PC34 capture_receipt;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_view_receipt_init_pc34(out_receipt);
    if (!snapshot ||
        !csb_v1_boot_startup_capture_receipt_from_snapshot_pc34(
            snapshot,
            &capture_receipt)) {
        return 0;
    }
    return csb_v1_boot_startup_host_view_receipt_from_capture_pc34(
        &capture_receipt,
        out_receipt);
}

int csb_v1_boot_startup_m11_presentation_receipt_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupM11PresentationReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupHostViewReceipt_PC34 host_view;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_m11_presentation_receipt_init_pc34(out_receipt);
    if (!csb_v1_boot_startup_host_view_receipt_from_snapshot_pc34(
            snapshot,
            &host_view) ||
        !host_view.valid) {
        return 0;
    }

    out_receipt->route = host_view.route;
    out_receipt->startup_render_plan_valid = host_view.render_plan_valid;
    if (out_receipt->startup_render_plan_valid) {
        out_receipt->startup_render_plan = host_view.render_plan;
    }
    out_receipt->hud_menu_draw_valid = host_view.hud_menu_draw_valid;
    if (out_receipt->hud_menu_draw_valid) {
        out_receipt->hud_menu_draw = host_view.hud_menu_draw;
        out_receipt->utility_render_plan_valid =
            host_view.hud_menu_draw.utility_render_plan_valid ? 1 : 0;
        if (out_receipt->utility_render_plan_valid) {
            out_receipt->utility_render_plan =
                host_view.hud_menu_draw.utility_render_plan;
        }
    }
    out_receipt->readiness_valid = host_view.readiness_valid;
    if (out_receipt->readiness_valid) {
        out_receipt->readiness = host_view.readiness;
    }
    out_receipt->capture_proof_valid = host_view.capture_proof_valid;
    if (out_receipt->capture_proof_valid) {
        out_receipt->capture_proof = host_view.capture_proof;
    }
    out_receipt->input_ready =
        host_view.startup_input_ready || host_view.readiness.host_runtime_input_ready;
    out_receipt->hud_ready =
        host_view.startup_hud_menu_ready || host_view.startup_hud_runtime_ready;
    out_receipt->runtime_ready = host_view.runtime_handoff_ready ? 1 : 0;
    out_receipt->selected_command_id = host_view.selected_command_id;
    out_receipt->selected_utility_action_index =
        host_view.selected_utility_action_index;
    out_receipt->source_evidence =
        "ReDMCSB TITLE.C F0437:424-463; ENTRANCE.C F0441/F0806:850-883; "
        "CSBWin Viewport.cpp startup HUD/menu presentation";
    out_receipt->valid = 1;

    /* The host consumes this immutable CSB transaction rather than adapting
     * separate render-plan, utility, HUD and capture compatibility facts. */
    return 1;
}

static int csb_v1_boot_startup_plan_has_no_fallback_pc34(
    const CSB_V1_StartupRenderPlan_PC34 *plan)
{
    int i;
    if (!plan || plan->fallback_status_visible || plan->fallback_frame_valid ||
        plan->fallback_detail_visible || plan->fallback_runtime_detail_visible) {
        return 0;
    }
    for (i = 0; i < plan->render_command_count &&
                i < CSB_V1_STARTUP_RENDER_COMMAND_CAP_PC34; ++i) {
        CSB_V1_StartupRenderCommandKind_PC34 kind = plan->render_commands[i].kind;
        if (kind == CSB_V1_STARTUP_RENDER_COMMAND_SURFACE_OR_TEXT_PC34 ||
            kind == CSB_V1_STARTUP_RENDER_COMMAND_DOORS_IF_SURFACE_ELSE_FALLBACK_PC34 ||
            kind == CSB_V1_STARTUP_RENDER_COMMAND_FALLBACK_IF_NO_SURFACE_PC34) {
            return 0;
        }
    }
    return 1;
}

int csb_v1_boot_startup_runtime_presentation_from_snapshot_pc34(
    const CSB_V1_BootStartupRuntimeAssetGateReceipt_PC34 *asset_gate,
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupRuntimePresentationReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootProfile *profile;

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    csb_v1_boot_startup_runtime_asset_gate_receipt_init_pc34(
        &out_receipt->asset_gate);
    if (!asset_gate || !asset_gate->valid || !snapshot ||
        !(profile = snapshot->boot_profile) ||
        !csb_v1_boot_startup_m11_presentation_receipt_from_snapshot_pc34(
            snapshot, &out_receipt->presentation)) {
        return 0;
    }
    out_receipt->asset_gate = *asset_gate;
    out_receipt->asset_gate_valid = 1;
    if (snapshot->resume_available !=
            asset_gate->session_state.entrance_resume_available ||
        strcmp(snapshot->resume_path ? snapshot->resume_path : "",
               asset_gate->session_state.entrance_resume_path) != 0) {
        return 0;
    }
    if (snapshot->utility_overlay_active &&
        (snapshot->utility_imported_champion_count !=
             asset_gate->session_state.import_champion_count ||
         snapshot->utility_selected_action_index !=
             asset_gate->session_state.import_selected_action_index ||
         snapshot->utility_preview_active !=
             asset_gate->session_state.import_preview_active ||
         strcmp(snapshot->utility_prompt ? snapshot->utility_prompt : "",
                asset_gate->session_state.import_utility_prompt) != 0)) {
        return 0;
    }
    out_receipt->render_plan_uses_owned_assets =
        out_receipt->presentation.startup_render_plan_valid &&
        csb_v1_boot_startup_render_plan_uses_real_assets_pc34(
            profile, &out_receipt->presentation.startup_render_plan);
    out_receipt->utility_plan_uses_owned_session =
        !snapshot->utility_overlay_active ||
        (out_receipt->presentation.utility_render_plan_valid &&
         out_receipt->presentation.hud_menu_draw_valid &&
         out_receipt->presentation.hud_menu_draw.kind ==
             CSB_V1_BOOT_STARTUP_HUD_MENU_UTILITY_PC34);
    out_receipt->door_plan_has_no_fallback =
        out_receipt->render_plan_uses_owned_assets &&
        csb_v1_boot_startup_plan_has_no_fallback_pc34(
            &out_receipt->presentation.startup_render_plan);
    if (out_receipt->render_plan_uses_owned_assets &&
        !csb_v1_boot_startup_runtime_surfaces_materialize_pc34(
            profile, &out_receipt->presentation.startup_render_plan,
            &out_receipt->surfaces)) {
        return 0;
    }
    out_receipt->valid = out_receipt->asset_gate_valid &&
                         out_receipt->render_plan_uses_owned_assets &&
                         out_receipt->utility_plan_uses_owned_session &&
                         out_receipt->door_plan_has_no_fallback &&
                         out_receipt->surfaces.valid ? 1 : 0;
    /* ReDMCSB TITLE.C F0437 and ENTRANCE.C F0438/F0441 publish title,
     * door animation, and entrance input as one live sequence.  CSBWin's
     * viewport owns the utility panel in that same session. */
    return out_receipt->valid;
}

static int csb_v1_boot_startup_host_input_dispatch_from_gate_pc34(
    const CSB_V1_BootStartupInputGateReceipt_PC34 *gate_receipt,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(out_receipt);
    if (!gate_receipt || !gate_receipt->valid) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->input_is_pointer = gate_receipt->input_is_pointer ? 1 : 0;
    out_receipt->pointer_button_relevant =
        gate_receipt->pointer_button_relevant ? 1 : 0;
    out_receipt->startup_active = gate_receipt->startup_active ? 1 : 0;
    out_receipt->startup_input_ready =
        gate_receipt->startup_input_ready ? 1 : 0;
    out_receipt->host_input_blocked =
        gate_receipt->host_input_blocked ? 1 : 0;
    out_receipt->should_dispatch_input =
        gate_receipt->should_dispatch_input ? 1 : 0;
    out_receipt->should_ignore_input =
        gate_receipt->should_ignore_input ? 1 : 0;
    out_receipt->input_render_valid =
        gate_receipt->input_render_valid ? 1 : 0;
    if (gate_receipt->input_render_valid) {
        out_receipt->input_render = gate_receipt->input_render;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 owns whether startup input is ignored,
     * blocked by title, routed to utility, or dispatched to entrance. Keep
     * that dispatch decision in a CSB receipt for M11 keyboard/pointer paths. */
    return 1;
}

int csb_v1_boot_startup_host_input_dispatch_firestaff_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupInputGateReceipt_PC34 gate_receipt;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(out_receipt);
    csb_v1_boot_startup_input_gate_receipt_init_pc34(&gate_receipt);
    if (!csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
            snapshot,
            menu_input,
            &gate_receipt)) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 keeps keyboard command dispatch inside
     * the startup loop. Expose that as the host receipt consumed by M11
     * instead of exporting the intermediate gate compatibility wrapper. */
    return csb_v1_boot_startup_host_input_dispatch_from_gate_pc34(
        &gate_receipt,
        out_receipt);
}

int csb_v1_boot_startup_host_input_dispatch_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupInputGateReceipt_PC34 gate_receipt;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(out_receipt);
    csb_v1_boot_startup_input_gate_receipt_init_pc34(&gate_receipt);
    if (!csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
            snapshot,
            x,
            y,
            button_mask,
            &gate_receipt)) {
        return 0;
    }
    /* CSBWin preserves the same entrance button boundary as ReDMCSB:
     * pointer handling resolves to a startup command before runtime play.
     * M11 therefore consumes the host dispatch receipt, not raw gate facts. */
    return csb_v1_boot_startup_host_input_dispatch_from_gate_pc34(
        &gate_receipt,
        out_receipt);
}

static int csb_v1_boot_startup_input_render_receipt_from_action_pc34(
    const CSB_V1_BootStartupActionReceipt_PC34 *action,
    CSB_V1_BootStartupInputRenderReceipt_PC34 *out_receipt)
{
    const CSB_V1_BootStartupReadinessReceipt_PC34 *draw_readiness = NULL;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_input_render_receipt_init_pc34(out_receipt);
    if (!action || (!action->pre_input_render_view_valid &&
                    !action->post_input_render_view_valid &&
                    !action->input_blocked_by_title &&
                    !action->handled)) {
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->action_valid = 1;
    out_receipt->action = *action;
    out_receipt->pre_input_readiness_valid =
        action->pre_input_render_view_valid &&
        csb_v1_boot_startup_readiness_receipt_from_view_pc34(
            &action->pre_input_render_view,
            &out_receipt->pre_input_readiness);
    out_receipt->post_input_readiness_valid =
        action->post_input_render_view_valid &&
        csb_v1_boot_startup_readiness_receipt_from_view_pc34(
            &action->post_input_render_view,
            &out_receipt->post_input_readiness);
    out_receipt->host_decision_valid =
        csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
            action,
            &out_receipt->host_decision);
    out_receipt->hud_menu_draw_valid =
        csb_v1_boot_startup_hud_menu_draw_receipt_from_action_pc34(
            action,
            1,
            &out_receipt->hud_menu_draw);
    out_receipt->draw_from_post_input =
        out_receipt->hud_menu_draw_valid &&
                out_receipt->hud_menu_draw.from_post_input_render_view
            ? 1
            : 0;
    out_receipt->input_consumed =
        out_receipt->host_decision_valid &&
                out_receipt->host_decision.consumed_input
            ? 1
            : 0;
    out_receipt->startup_redraw =
        out_receipt->host_decision_valid &&
                out_receipt->host_decision.redraw_startup
            ? 1
            : 0;
    out_receipt->runtime_handoff_ready =
        out_receipt->post_input_readiness_valid &&
                out_receipt->post_input_readiness.runtime_handoff_ready
            ? 1
            : 0;
    out_receipt->return_to_launcher =
        out_receipt->host_decision_valid &&
                out_receipt->host_decision.return_to_launcher
            ? 1
            : 0;
    if (out_receipt->return_to_launcher) {
        out_receipt->hud_menu_draw_valid = 0;
        csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
            &out_receipt->hud_menu_draw);
    }
    draw_readiness = out_receipt->draw_from_post_input
                         ? &out_receipt->post_input_readiness
                         : &out_receipt->pre_input_readiness;
    out_receipt->startup_hud_draw_ready =
        out_receipt->hud_menu_draw_valid &&
                ((out_receipt->draw_from_post_input &&
                  out_receipt->post_input_readiness_valid) ||
                 (!out_receipt->draw_from_post_input &&
                  out_receipt->pre_input_readiness_valid)) &&
                draw_readiness->hud_menu_ready &&
                !draw_readiness->host_hud_blocked
            ? 1
            : 0;
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 keeps input,
     * host decision, redraw, and HUD/menu readiness in one startup loop.
     * This receipt is the CSB-owned M11 handoff boundary, so callers no
     * longer combine action, decision, readiness, and HUD draw routes by
     * reinterpreting raw startup fields. */
    return 1;
}

int csb_v1_boot_runtime_util_apply_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
            out_receipt);
        return 0;
    }
    return csb_v1_runtime_util_apply_point_from_startup_host_facts_with_action_receipt_pc34(
        &facts,
        x,
        y,
        out_receipt);
}

int csb_v1_boot_runtime_util_apply_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
            out_receipt);
        return 0;
    }
    return csb_v1_runtime_util_apply_firestaff_input_from_startup_host_facts_with_action_receipt_pc34(
        &facts,
        menu_input,
        out_receipt);
}

int csb_v1_boot_runtime_execute_startup_entrance_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        csb_v1_startup_entrance_host_action_receipt_init_pc34(
            out_receipt);
        return 0;
    }
    return csb_v1_runtime_execute_startup_entrance_firestaff_input_from_host_facts_with_receipts_pc34(
        &facts,
        menu_input,
        out_receipt);
}

int csb_v1_boot_runtime_execute_startup_entrance_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_StartupEntranceHostActionReceipt_PC34 *out_receipt)
{
    CSB_V1_StartupHostFacts_PC34 facts;

    if (!csb_v1_boot_startup_facts_from_snapshot_pc34(&facts, snapshot)) {
        csb_v1_startup_entrance_host_action_receipt_init_pc34(
            out_receipt);
        return 0;
    }
    return csb_v1_runtime_execute_startup_entrance_pointer_from_host_facts_with_receipts_pc34(
        &facts,
        x,
        y,
        button_mask,
        out_receipt);
}

void csb_v1_boot_startup_action_receipt_init_pc34(
    CSB_V1_BootStartupActionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_presentation_route_receipt_init_pc34(
        &receipt->pre_input_route);
    csb_v1_boot_startup_render_view_receipt_init_pc34(
        &receipt->pre_input_render_view);
    csb_v1_boot_startup_render_view_receipt_init_pc34(
        &receipt->post_input_render_view);
    csb_v1_runtime_util_startup_host_action_receipt_init_pc34(
        &receipt->utility_receipt);
    csb_v1_startup_entrance_host_action_receipt_init_pc34(
        &receipt->entrance_receipt);
}

void csb_v1_boot_startup_presentation_route_receipt_init_pc34(
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->special_palette = -1;
    csb_v1_startup_presentation_receipt_init_pc34(
        &receipt->presentation);
    csb_v1_boot_startup_hud_menu_state_init_pc34(
        &receipt->hud_menu_state);
}

void csb_v1_boot_startup_host_view_receipt_init_pc34(
    CSB_V1_BootStartupHostViewReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    snprintf(receipt->phase, sizeof(receipt->phase), "%s", "inactive");
    snprintf(receipt->animation, sizeof(receipt->animation), "%s", "none");
    receipt->title_frame = -1;
    receipt->title_frame_max = -1;
    receipt->title_ready = 1;
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->special_palette = -1;
    receipt->runtime_map_index = -1;
    receipt->runtime_party_x = -1;
    receipt->runtime_party_y = -1;
    receipt->runtime_party_dir = -1;
    receipt->runtime_champion_count = -1;
    receipt->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->selected_utility_action_index = -1;
    csb_v1_boot_startup_readiness_receipt_init_pc34(
        &receipt->readiness);
    csb_v1_boot_startup_render_draw_receipt_init_pc34(
        &receipt->render_draw);
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
        &receipt->hud_menu_draw);
    csb_v1_boot_startup_packaged_capture_proof_init_pc34(
        &receipt->capture_proof);
}

void csb_v1_boot_startup_m11_presentation_receipt_init_pc34(
    CSB_V1_BootStartupM11PresentationReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->selected_command_id =
        CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
    receipt->selected_utility_action_index = -1;
    csb_v1_boot_startup_hud_menu_draw_receipt_init_pc34(
        &receipt->hud_menu_draw);
    csb_v1_boot_startup_readiness_receipt_init_pc34(&receipt->readiness);
    csb_v1_boot_startup_packaged_capture_proof_init_pc34(
        &receipt->capture_proof);
}

void csb_v1_boot_startup_host_view_draw_receipt_init_pc34(
    CSB_V1_BootStartupHostViewDrawReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->surface = CSB_V1_STARTUP_RENDER_NONE_PC34;
    receipt->hud_menu_kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
}

void csb_v1_boot_startup_render_draw_receipt_init_pc34(
    CSB_V1_BootStartupRenderDrawReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->surface = CSB_V1_STARTUP_RENDER_NONE_PC34;
}

void csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(
    CSB_V1_BootStartupHostInputDispatchReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    csb_v1_boot_startup_input_render_receipt_init_pc34(
        &receipt->input_render);
}

void csb_v1_boot_startup_host_ownership_receipt_init_pc34(
    CSB_V1_BootStartupHostOwnershipReceipt_PC34 *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->route = CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    receipt->hud_menu_kind = CSB_V1_BOOT_STARTUP_HUD_MENU_NONE_PC34;
    csb_v1_boot_startup_host_view_receipt_init_pc34(&receipt->host_view);
    csb_v1_boot_startup_host_view_draw_receipt_init_pc34(
        &receipt->host_draw);
    csb_v1_boot_startup_host_input_dispatch_receipt_init_pc34(
        &receipt->host_input);
}

static void csb_v1_boot_startup_hud_menu_state_init_pc34(
    CSB_V1_BootStartupHudMenuStateReceipt_PC34 *state)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->selected_command_id = CSB_V1_STARTUP_ENTRANCE_COMMAND_NONE_PC34;
}

static CSB_V1_BootStartupRenderRouteKind_PC34
csb_v1_boot_startup_route_for_surface_pc34(
    CSB_V1_StartupRenderSurface_PC34 surface)
{
    switch (surface) {
        case CSB_V1_STARTUP_RENDER_TITLE_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34;
        case CSB_V1_STARTUP_RENDER_ENTRANCE_BLACK_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_BLACK_PC34;
        case CSB_V1_STARTUP_RENDER_ENTRANCE_CLOSED_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34;
        case CSB_V1_STARTUP_RENDER_ENTRANCE_CREDITS_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34;
        case CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_DELAY_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_DELAY_PC34;
        case CSB_V1_STARTUP_RENDER_ENTRANCE_OPENING_FRAME_PC34:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_FRAME_PC34;
        case CSB_V1_STARTUP_RENDER_NONE_PC34:
        default:
            return CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34;
    }
}

static void csb_v1_boot_startup_hud_menu_from_presentation_pc34(
    const CSB_V1_StartupPresentationReceipt_PC34 *presentation,
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_BootStartupHudMenuStateReceipt_PC34 *out_state)
{
    int i;
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 owns the entrance
     * wait/input loop; CSBWin/Viewport.cpp keeps CSB HUD/menu state as
     * viewport-owned presentation data. Export the selected startup menu
     * command here so runtime consumers do not reconstruct it. */
    if (!presentation || !out_state || !presentation->waiting_for_input ||
        presentation->menu_option_count <= 0) {
        return;
    }
    csb_v1_boot_startup_hud_menu_state_init_pc34(out_state);
    out_state->valid = 1;
    out_state->kind = CSB_V1_BOOT_STARTUP_HUD_MENU_ENTRANCE_PC34;
    out_state->option_count = presentation->menu_option_count;
    for (i = 0; i < presentation->render_plan.menu_option_count &&
                i < CSB_V1_STARTUP_MENU_OPTION_CAP_PC34; ++i) {
        const CSB_V1_StartupMenuOption_PC34 *option =
            &presentation->render_plan.menu_options[i];
        if (option->command_id ==
            CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34) {
            out_state->resume_enabled = option->enabled ? 1 : 0;
            out_state->resume_option_visible = 1;
        }
        if (option->selected) {
            out_state->selected_command_id = option->command_id;
        }
    }
    out_state->resume_option_selected =
        out_state->selected_command_id ==
            CSB_V1_STARTUP_ENTRANCE_COMMAND_RESUME_PC34
            ? 1
            : 0;
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 gates the RESUME
     * command inside the entrance wait loop; CSBWin SaveGame.cpp:927/1711/2111
     * keeps resume as a save-load gate. Carry the loadable path in this CSB
     * receipt so host render/input code does not infer it from menu text. */
    if (facts) {
        out_state->resume_available = facts->resume_available ? 1 : 0;
        snprintf(out_state->resume_path, sizeof(out_state->resume_path), "%s",
                 facts->resume_path ? facts->resume_path : "");
    }
    snprintf(out_state->prompt, sizeof(out_state->prompt), "%s",
             presentation->render_plan.fallback_prompt_text
                 ? presentation->render_plan.fallback_prompt_text
                 : "");
}

static void csb_v1_boot_startup_route_from_presentation_pc34(
    const CSB_V1_StartupPresentationReceipt_PC34 *presentation,
    const CSB_V1_StartupHostFacts_PC34 *facts,
    CSB_V1_BootStartupPresentationRouteReceipt_PC34 *out_receipt)
{
    if (!presentation || !out_receipt || !presentation->valid) {
        return;
    }
    out_receipt->valid = 1;
    out_receipt->presentation = *presentation;
    out_receipt->route = csb_v1_boot_startup_route_for_surface_pc34(
        presentation->render_plan.surface);
    out_receipt->special_palette = presentation->render_plan.special_palette;
    out_receipt->accepts_input = presentation->accepts_input;
    out_receipt->waiting_for_input = presentation->waiting_for_input;
    out_receipt->menu_option_count = presentation->menu_option_count;
    csb_v1_boot_startup_hud_menu_from_presentation_pc34(
        presentation,
        facts,
        &out_receipt->hud_menu_state);

    /* ReDMCSB TITLE.C F0437 lines 424-463 draws PRESENTS, CHAOS zoom,
     * and STRIKES BACK; ENTRANCE.C F0441/F0806 lines 409-447 and
     * 850-883 then own the entrance surface/wait loop; ENTRANCE.C
     * F0438/F0807 supplies the door-opening frame.  This receipt keeps
     * that route decision in CSB boot instead of letting host/HUD code
     * infer it from raw startup fields. */
    switch (out_receipt->route) {
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_TITLE_PC34:
            out_receipt->draw_title = 1;
            break;
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CREDITS_PC34:
            out_receipt->draw_surface = 1;
            out_receipt->draw_fallback_text = 0;
            break;
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_CLOSED_PC34:
            out_receipt->draw_surface = 1;
            out_receipt->draw_closed_doors = 1;
            out_receipt->draw_fallback_text = 0;
            out_receipt->hud_menu_visible =
                presentation->render_plan.waiting_for_input ? 1 : 0;
            break;
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_DELAY_PC34:
            out_receipt->draw_surface = 1;
            out_receipt->draw_closed_doors = 1;
            break;
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_OPENING_FRAME_PC34:
            out_receipt->draw_surface = 1;
            out_receipt->draw_closed_doors = 1;
            out_receipt->draw_opening_frame = 1;
            break;
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_ENTRANCE_BLACK_PC34:
        case CSB_V1_BOOT_STARTUP_RENDER_ROUTE_NONE_PC34:
        default:
            break;
    }
}

static int csb_v1_boot_startup_utility_receipt_handled_pc34(
    const CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 *receipt)
{
    if (!receipt) {
        return 0;
    }
    return receipt->entrance_receipt_valid ||
           receipt->util_receipt.result != CSB_V1_UTIL_APPLY_IGNORED ||
           receipt->util_state_receipt.selected_action_index_changed ||
           receipt->util_state_receipt.preview_active_changed;
}

static int csb_v1_boot_startup_action_capture_pre_input_route_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    int input_is_pointer,
    int pointer_x,
    int pointer_y,
    unsigned int pointer_button_mask,
    CSB_V1_BootStartupActionReceipt_PC34 *receipt)
{
    int captured;

    if (!snapshot || !receipt) {
        return 0;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 dispatches input from
     * the same entrance wait loop that owns the visible screen. CSBWin keeps
     * that HUD/menu routing in the viewport layer, so every boot action
     * receipt captures the pre-input render route before utility/entrance
     * dispatch mutates startup state. */
    receipt->menu_input = menu_input;
    receipt->input_is_pointer = input_is_pointer ? 1 : 0;
    receipt->pointer_x = pointer_x;
    receipt->pointer_y = pointer_y;
    receipt->pointer_button_mask = pointer_button_mask;
    receipt->pointer_left_button =
        (pointer_button_mask & ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) ? 1 : 0;
    receipt->startup_input =
        csb_v1_startup_input_from_firestaff_menu_code_pc34(menu_input);
    receipt->entrance_command_id =
        csb_v1_startup_entrance_command_for_input_pc34(
            snapshot->credits_active,
            receipt->startup_input);
    captured = csb_v1_boot_startup_presentation_route_receipt_from_snapshot_pc34(
        snapshot,
        &receipt->pre_input_route);
    if (!captured) {
        return 0;
    }
    receipt->pre_input_render_view_valid =
        csb_v1_boot_startup_render_view_receipt_from_route_pc34(
            &receipt->pre_input_route,
            &receipt->pre_input_render_view);
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 maps input to C001/C200
     * startup commands only while the entrance wait loop accepts input.
     * TITLE.C F0437 keeps the post-FTL title/PRESENTS animation noninteractive.
     * Store the source input and command boundary with the render receipt so
     * host code does not re-run CSB input routing for title/menu decisions. */
    receipt->input_blocked_by_title =
        receipt->pre_input_render_view.title_after_swoosh_route &&
                !receipt->pre_input_route.accepts_input
            ? 1
            : 0;
    return captured;
}

static void csb_v1_boot_startup_snapshot_apply_command_state_pc34(
    CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    const CSB_V1_StartupCommandStateReceipt_PC34 *state)
{
    if (!snapshot || !state) {
        return;
    }
    snapshot->title_active = state->title_active;
    snapshot->title_frame = state->title_frame;
    snapshot->title_source_step = state->title_source_step;
    snapshot->entrance_active = state->entrance_active;
    snapshot->entrance_source_step = state->entrance_source_step;
    snapshot->entrance_dismissed = state->entrance_dismissed;
    snapshot->credits_active = state->credits_active;
    snapshot->credits_remaining_ticks = state->credits_remaining_ticks;
    snapshot->opening_active = state->opening_active;
    snapshot->opening_delay_ticks = state->opening_delay_ticks;
    snapshot->opening_step = state->opening_step;
    snapshot->pending_command = state->pending_command;
}

static void csb_v1_boot_startup_action_capture_host_receipt_pc34(
    CSB_V1_BootStartupActionReceipt_PC34 *receipt,
    const CSB_V1_StartupEntranceHostActionReceipt_PC34 *entrance)
{
    if (!receipt || !entrance || !entrance->handled) {
        return;
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 leaves the startup
     * loop with C001/C200/C216 commands and source-visible host status.
     * CSBWin keeps that menu result at the viewport/input boundary. Flatten
     * the host receipt here so M11 can consume one CSB boot action receipt. */
    receipt->host_receipt_valid = 1;
    receipt->host_input_result = entrance->host_receipt.input_result;
    receipt->host_status_scope = entrance->host_receipt.status_scope;
    receipt->host_status = entrance->host_receipt.status;
    receipt->host_clear_import_preview =
        entrance->host_receipt.clear_import_preview ? 1 : 0;
    receipt->host_bonus_requested_changed =
        entrance->host_receipt.bonus_requested_changed ? 1 : 0;
    receipt->host_bonus_requested =
        entrance->host_receipt.bonus_requested ? 1 : 0;
}

static int csb_v1_boot_startup_action_capture_post_input_render_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    CSB_V1_BootStartupActionReceipt_PC34 *receipt)
{
    CSB_V1_BootRuntimeStartupSnapshot_PC34 post_snapshot;
    const CSB_V1_StartupEntranceHostActionReceipt_PC34 *entrance = NULL;
    const CSB_V1_UtilStateReceipt *util_state = NULL;

    if (!snapshot || !receipt || !receipt->handled) {
        return 0;
    }
    post_snapshot = *snapshot;
    if (receipt->kind == CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34) {
        util_state = &receipt->utility_receipt.util_state_receipt;
        if (util_state->selected_action_index_changed) {
            post_snapshot.utility_selected_action_index =
                util_state->selected_action_index;
        }
        if (util_state->preview_active_changed) {
            post_snapshot.utility_preview_active = util_state->preview_active;
        }
        if (receipt->utility_receipt.entrance_receipt_valid) {
            entrance = &receipt->utility_receipt.entrance_receipt;
        }
    } else if (receipt->kind == CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34) {
        entrance = &receipt->entrance_receipt;
    }
    if (entrance && entrance->handled) {
        csb_v1_boot_startup_action_capture_host_receipt_pc34(
            receipt,
            entrance);
        csb_v1_boot_startup_snapshot_apply_command_state_pc34(
            &post_snapshot,
            &entrance->state_receipt);
        if (entrance->host_receipt.input_result ==
            CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34) {
            receipt->input_requests_launcher_return = 1;
            return 0;
        }
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 routes keyboard/mouse
     * input inside the same entrance loop that redraws the closed doors,
     * credits, and opening sequence. CSBWin/Viewport.cpp keeps menu/HUD
     * drawing coupled to that state. Capture the post-input render route here
     * so M11 does not have to reinterpret utility or entrance receipts. */
    receipt->post_input_render_view_valid =
        csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
            &post_snapshot,
            &receipt->post_input_render_view);
    receipt->input_stays_on_startup =
        receipt->post_input_render_view_valid &&
                receipt->post_input_render_view.route_receipt.valid
            ? 1
            : 0;
    return receipt->post_input_render_view_valid;
}

int csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupActionReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 utility_receipt;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;

    csb_v1_boot_startup_action_receipt_init_pc34(out_receipt);
    if (!snapshot || !out_receipt) {
        return 0;
    }
    if (!csb_v1_boot_startup_action_capture_pre_input_route_pc34(
            snapshot,
            menu_input,
            0,
            0,
            0,
            0U,
            out_receipt) ||
        !snapshot->entrance_active) {
        return 0;
    }
    if (csb_v1_boot_runtime_util_apply_firestaff_input_from_snapshot_pc34(
            snapshot,
            menu_input,
            &utility_receipt) &&
        csb_v1_boot_startup_utility_receipt_handled_pc34(
            &utility_receipt)) {
        out_receipt->kind = CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34;
        out_receipt->handled = 1;
        out_receipt->input_routed_to_utility = 1;
        out_receipt->utility_receipt = utility_receipt;
        if (utility_receipt.entrance_receipt_valid) {
            out_receipt->entrance_command_id =
                utility_receipt.entrance_receipt.command_receipt.command_id;
        }
        (void)csb_v1_boot_startup_action_capture_post_input_render_pc34(
            snapshot,
            out_receipt);
        return 1;
    }
    if (!csb_v1_boot_startup_entrance_accepts_input_from_snapshot_pc34(
            snapshot)) {
        return 0;
    }
    if (!csb_v1_boot_runtime_execute_startup_entrance_firestaff_input_from_snapshot_pc34(
            snapshot,
            menu_input,
            &entrance_receipt)) {
        return 0;
    }
    if (!entrance_receipt.handled) {
        return 0;
    }
    out_receipt->kind = CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34;
    out_receipt->handled = 1;
    out_receipt->input_routed_to_entrance = 1;
    out_receipt->entrance_receipt = entrance_receipt;
    out_receipt->entrance_command_id =
        entrance_receipt.command_receipt.command_id;
    (void)csb_v1_boot_startup_action_capture_post_input_render_pc34(
        snapshot,
        out_receipt);
    return 1;
}

static int csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupInputRenderReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupActionReceipt_PC34 action;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_action_receipt_init_pc34(&action);
    csb_v1_boot_startup_input_render_receipt_init_pc34(out_receipt);
    if (!csb_v1_boot_runtime_execute_startup_firestaff_input_from_snapshot_pc34(
            snapshot,
            menu_input,
            &action) &&
        !action.input_blocked_by_title) {
        return 0;
    }
    return csb_v1_boot_startup_input_render_receipt_from_action_pc34(
        &action,
        out_receipt);
}

static int csb_v1_boot_runtime_execute_startup_firestaff_input_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int menu_input,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_input_gate_receipt_init_pc34(out_receipt);
    if (!snapshot ||
        !csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->readiness)) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->startup_active =
        out_receipt->readiness.startup_active ? 1 : 0;
    out_receipt->startup_input_ready =
        out_receipt->readiness.host_startup_input_ready ? 1 : 0;
    out_receipt->host_input_blocked =
        out_receipt->readiness.host_input_blocked ? 1 : 0;
    out_receipt->should_dispatch_input =
        out_receipt->startup_active &&
                out_receipt->startup_input_ready &&
                !out_receipt->host_input_blocked
            ? 1
            : 0;
    out_receipt->should_ignore_input =
        out_receipt->startup_active &&
                (out_receipt->host_input_blocked ||
                 !out_receipt->startup_input_ready)
            ? 1
            : 0;
    if (out_receipt->should_dispatch_input ||
        out_receipt->host_input_blocked) {
        out_receipt->input_render_valid =
            csb_v1_boot_runtime_execute_startup_firestaff_input_render_from_snapshot_pc34(
                snapshot,
                menu_input,
                &out_receipt->input_render);
    }
    /* ReDMCSB TITLE.C F0437 lines 424-463 blocks title input until
     * ENTRANCE.C F0441/F0806 lines 850-883 reaches the menu/wait loop.
     * Keep M11's startup input gate and post-input render/HUD decision in
     * one CSB receipt beside the snapshot capture render executor. */
    return out_receipt->valid;
}

int csb_v1_boot_startup_host_decision_from_action_receipt_pc34(
    const CSB_V1_BootStartupActionReceipt_PC34 *receipt,
    CSB_V1_BootStartupHostDecisionReceipt_PC34 *out_decision)
{
    if (!out_decision) {
        return 0;
    }
    csb_v1_boot_startup_host_decision_receipt_init_pc34(out_decision);
    if (!receipt || (!receipt->pre_input_route.valid &&
                     !receipt->pre_input_render_view_valid &&
                     !receipt->handled &&
                     !receipt->input_blocked_by_title)) {
        return 0;
    }

    out_decision->valid = 1;
    out_decision->menu_input = receipt->menu_input;
    out_decision->input_is_pointer = receipt->input_is_pointer;
    out_decision->pointer_left_button = receipt->pointer_left_button;
    out_decision->blocked_by_title = receipt->input_blocked_by_title;
    out_decision->routed_to_utility = receipt->input_routed_to_utility;
    out_decision->routed_to_entrance = receipt->input_routed_to_entrance;
    out_decision->consumed_input =
        receipt->handled || receipt->input_blocked_by_title ? 1 : 0;
    out_decision->stays_on_startup = receipt->input_stays_on_startup;
    out_decision->return_to_launcher =
        receipt->input_requests_launcher_return;
    out_decision->clear_import_preview = receipt->host_clear_import_preview;
    out_decision->bonus_requested_changed =
        receipt->host_bonus_requested_changed;
    out_decision->bonus_requested = receipt->host_bonus_requested;
    out_decision->entrance_command_id = receipt->entrance_command_id;
    out_decision->host_input_result = receipt->host_input_result;
    out_decision->status_scope = receipt->host_status_scope;
    out_decision->status = receipt->host_status;
    if (!out_decision->status && receipt->kind ==
                                      CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34) {
        out_decision->status_scope = receipt->utility_receipt.util_receipt
                                         .status_scope;
        out_decision->status = receipt->utility_receipt.util_receipt.status;
    }
    out_decision->pre_render_route = receipt->pre_input_route.route;
    if (receipt->post_input_render_view_valid) {
        out_decision->post_render_route =
            receipt->post_input_render_view.route_receipt.route;
    }
    if (receipt->pre_input_render_view.utility_menu_route) {
        out_decision->utility_selected_action_index =
            receipt->pre_input_render_view.utility_selected_action_index;
    }
    if (receipt->post_input_render_view_valid &&
        receipt->post_input_render_view.utility_menu_route) {
        out_decision->utility_selected_action_index =
            receipt->post_input_render_view.utility_selected_action_index;
    }
    out_decision->redraw_startup =
        out_decision->stays_on_startup ||
                receipt->post_input_render_view_valid ||
                receipt->host_input_result ==
                    CSB_V1_STARTUP_ENTRANCE_INPUT_REDRAW_PC34 ||
                receipt->utility_receipt.util_receipt.result ==
                    CSB_V1_UTIL_APPLY_REDRAW
            ? 1
            : 0;
    if (receipt->host_input_result ==
        CSB_V1_STARTUP_ENTRANCE_INPUT_RETURN_TO_LAUNCHER_PC34) {
        out_decision->return_to_launcher = 1;
        out_decision->redraw_startup = 0;
        out_decision->stays_on_startup = 0;
    }

    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 routes all startup
     * input through the CSB entrance loop. Flatten the M11-facing decision
     * here so host code can consume redraw/close/status/route facts without
     * reinterpreting utility and entrance receipts. */
    return 1;
}

int csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupActionReceipt_PC34 *out_receipt)
{
    CSB_V1_RuntimeUtilStartupHostActionReceipt_PC34 utility_receipt;
    CSB_V1_StartupEntranceHostActionReceipt_PC34 entrance_receipt;

    csb_v1_boot_startup_action_receipt_init_pc34(out_receipt);
    if (!snapshot || !out_receipt) {
        return 0;
    }
    if (!csb_v1_boot_startup_action_capture_pre_input_route_pc34(
            snapshot,
            0,
            1,
            x,
            y,
            button_mask,
            out_receipt) ||
        !snapshot->entrance_active) {
        return 0;
    }
    if ((button_mask & ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) &&
        csb_v1_boot_runtime_util_apply_pointer_from_snapshot_pc34(
            snapshot,
            x,
            y,
            &utility_receipt) &&
        csb_v1_boot_startup_utility_receipt_handled_pc34(
            &utility_receipt)) {
        out_receipt->kind = CSB_V1_BOOT_STARTUP_ACTION_UTILITY_PC34;
        out_receipt->handled = 1;
        out_receipt->input_routed_to_utility = 1;
        out_receipt->utility_receipt = utility_receipt;
        if (utility_receipt.entrance_receipt_valid) {
            out_receipt->entrance_command_id =
                utility_receipt.entrance_receipt.command_receipt.command_id;
        }
        (void)csb_v1_boot_startup_action_capture_post_input_render_pc34(
            snapshot,
            out_receipt);
        return 1;
    }
    if (!csb_v1_boot_startup_entrance_accepts_input_from_snapshot_pc34(
            snapshot)) {
        return 0;
    }
    if (!csb_v1_boot_runtime_execute_startup_entrance_pointer_from_snapshot_pc34(
            snapshot,
            x,
            y,
            button_mask,
            &entrance_receipt)) {
        return 0;
    }
    if (!entrance_receipt.handled) {
        return 0;
    }
    out_receipt->kind = CSB_V1_BOOT_STARTUP_ACTION_ENTRANCE_PC34;
    out_receipt->handled = 1;
    out_receipt->input_routed_to_entrance = 1;
    out_receipt->entrance_receipt = entrance_receipt;
    out_receipt->entrance_command_id =
        entrance_receipt.command_receipt.command_id;
    (void)csb_v1_boot_startup_action_capture_post_input_render_pc34(
        snapshot,
        out_receipt);
    return 1;
}

static int csb_v1_boot_runtime_execute_startup_pointer_render_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupInputRenderReceipt_PC34 *out_receipt)
{
    CSB_V1_BootStartupActionReceipt_PC34 action;

    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_action_receipt_init_pc34(&action);
    csb_v1_boot_startup_input_render_receipt_init_pc34(out_receipt);
    if (!csb_v1_boot_runtime_execute_startup_pointer_from_snapshot_pc34(
            snapshot,
            x,
            y,
            button_mask,
            &action) &&
        !action.input_blocked_by_title) {
        return 0;
    }
    return csb_v1_boot_startup_input_render_receipt_from_action_pc34(
        &action,
        out_receipt);
}

static int csb_v1_boot_runtime_execute_startup_pointer_gate_from_snapshot_pc34(
    const CSB_V1_BootRuntimeStartupSnapshot_PC34 *snapshot,
    int x,
    int y,
    unsigned int button_mask,
    CSB_V1_BootStartupInputGateReceipt_PC34 *out_receipt)
{
    if (!out_receipt) {
        return 0;
    }
    csb_v1_boot_startup_input_gate_receipt_init_pc34(out_receipt);
    if (!snapshot ||
        !csb_v1_boot_startup_readiness_receipt_from_snapshot_pc34(
            snapshot,
            &out_receipt->readiness)) {
        return 0;
    }
    out_receipt->valid = 1;
    out_receipt->input_is_pointer = 1;
    out_receipt->pointer_left_button =
        (button_mask & ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT) ? 1 : 0;
    out_receipt->pointer_button_relevant =
        (button_mask & (ENTRANCE_MOUSE_BUTTON_LEFT_COMPAT |
                        ENTRANCE_MOUSE_BUTTON_BONUS_DUNGEON_COMPAT))
            ? 1
            : 0;
    out_receipt->startup_active =
        out_receipt->readiness.startup_active ? 1 : 0;
    out_receipt->startup_input_ready =
        out_receipt->readiness.host_startup_input_ready ? 1 : 0;
    out_receipt->host_input_blocked =
        out_receipt->readiness.host_input_blocked ? 1 : 0;
    out_receipt->should_dispatch_input =
        out_receipt->pointer_button_relevant &&
                out_receipt->startup_active &&
                out_receipt->startup_input_ready &&
                !out_receipt->host_input_blocked
            ? 1
            : 0;
    out_receipt->should_ignore_input =
        out_receipt->pointer_button_relevant &&
                out_receipt->startup_active &&
                (out_receipt->host_input_blocked ||
                 !out_receipt->startup_input_ready)
            ? 1
            : 0;
    if (out_receipt->should_dispatch_input ||
        (out_receipt->pointer_button_relevant &&
         out_receipt->host_input_blocked)) {
        out_receipt->input_render_valid =
            csb_v1_boot_runtime_execute_startup_pointer_render_from_snapshot_pc34(
                snapshot,
                x,
                y,
                button_mask,
                &out_receipt->input_render);
    }
    /* ReDMCSB ENTRANCE.C F0441/F0806 lines 850-883 owns mouse routing for
     * the utility/menu wait loop. M11 now receives one CSB gate receipt for
     * pointer relevance, title blocking, dispatch, and post-input HUD draw. */
    return out_receipt->valid;
}

int csb_v1_boot_runtime_save_game_to_path_pc34(
    const CSB_V1_BootProfile *profile,
    const char *path,
    uint32_t *out_game_time)
{
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    /* ReDMCSB LOADSAVE.C F0433: save is part of the CSB boot/runtime
     * lifecycle because GLOBAL_DATA and the loaded dungeon tables live
     * behind the boot profile. */
    result = csb_v1_runtime_save_game_to_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_boot_runtime_load_game_from_path_pc34(
    CSB_V1_BootProfile *profile,
    const char *path,
    uint32_t *out_game_time)
{
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile || !path) {
        return -1;
    }
    /* ReDMCSB LOADSAVE.C F0435: load resumes the already verified CSB
     * boot profile rather than reconstructing state in the M11 host. */
    result = csb_v1_runtime_load_game_from_path(&profile->runtime, path);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

static void csb_v1_boot_copy_receipt_path_pc34(char *dst,
                                               size_t dst_size,
                                               const char *src)
{
    if (!dst || dst_size == 0u) {
        return;
    }
    snprintf(dst, dst_size, "%s", src ? src : "");
}

int csb_v1_boot_runtime_save_import_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    const char *dm1_import_path,
    const char *resume_save_path,
    const char *csbwin_save_path,
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 *out_receipt)
{
    CSB_V1_CSBWinSaveDiscoveryResult csbwin;
    int csbwin_rc;

    if (!out_receipt) {
        return 0;
    }
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->csbwin_shape = CSB_V1_CSBWIN_SHAPE_COUNT;
    out_receipt->csbwin_file_kind = CSB_V1_CSBWIN_SAVE_FILE_NONE;
    out_receipt->csbwin_loader_code = CSB_SAVE_IMPORT_ERR_NULL;
    out_receipt->csbwin_decision_label = "no_csbwin_save_path";
    out_receipt->source_evidence =
        "ReDMCSB LOADSAVE.C F0433/F0435; ENTRANCE.C F0806; "
        "CSBWin CSBCode.cpp:421-422 csbgame.dat/csbgame.bak";
    if (!profile) {
        return 0;
    }

    out_receipt->boot_profile_ready =
        strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) == 0 &&
        profile->assets_verified &&
        profile->graphics_verified &&
        profile->dungeon_verified;
    out_receipt->runtime_ready =
        profile->state == CSB_V1_BOOT_STATE_RUNTIME_READY ? 1 : 0;
    out_receipt->save_root_bound = profile->save_root[0] != '\0';
    csb_v1_boot_copy_receipt_path_pc34(out_receipt->save_root,
                                       sizeof(out_receipt->save_root),
                                       profile->save_root);
    out_receipt->save_adapter_available = out_receipt->runtime_ready;
    out_receipt->load_adapter_available = out_receipt->runtime_ready;
    out_receipt->tick_adapter_available = out_receipt->runtime_ready;

    out_receipt->resume_path_present =
        resume_save_path && resume_save_path[0] != '\0';
    csb_v1_boot_copy_receipt_path_pc34(out_receipt->resume_path,
                                       sizeof(out_receipt->resume_path),
                                       resume_save_path);
    out_receipt->dm1_import_path_present =
        dm1_import_path && dm1_import_path[0] != '\0';
    csb_v1_boot_copy_receipt_path_pc34(out_receipt->dm1_import_path,
                                       sizeof(out_receipt->dm1_import_path),
                                       dm1_import_path);

    out_receipt->imported_party_ready = profile->imported_party_ready;
    out_receipt->cmp_import_attempted = profile->cmp_import_attempted;
    out_receipt->cmp_import_succeeded = profile->cmp_import_succeeded;
    out_receipt->cmp_imported_slot = profile->cmp_imported_slot;
    out_receipt->cmp_imported_champion_count =
        profile->cmp_imported_champion_count;

    out_receipt->csbwin_path_present =
        csbwin_save_path && csbwin_save_path[0] != '\0';
    csb_v1_boot_copy_receipt_path_pc34(out_receipt->csbwin_path,
                                       sizeof(out_receipt->csbwin_path),
                                       csbwin_save_path);
    if (out_receipt->csbwin_path_present) {
        memset(&csbwin, 0, sizeof(csbwin));
        csbwin_rc = csb_v1_csbwin_save_loader_boundary_classify_file(
            csbwin_save_path,
            0u,
            &csbwin);
        out_receipt->csbwin_file_kind = csbwin.file_kind;
        out_receipt->csbwin_filename_candidate =
            csbwin.filename_candidate ? 1 : 0;
        out_receipt->csbwin_should_attempt_import =
            csbwin.should_attempt_import ? 1 : 0;
        out_receipt->csbwin_loader_code = csbwin_rc;
        out_receipt->csbwin_contract_match =
            csbwin.loader.contract_match ? 1 : 0;
        out_receipt->csbwin_shape = csbwin.shape;
        out_receipt->csbwin_decision_label =
            csb_v1_csbwin_save_loader_boundary_decision_name(&csbwin);
    }

    /* LOADSAVE.C F0433/F0435 and the CSBWin CSBGAME filename surface are
     * now represented by this single boot-owned receipt.  Runtime consumers
     * can ask for save/load/import readiness without reconstructing paths or
     * poking the utility-flow internals. */
    out_receipt->valid =
        out_receipt->boot_profile_ready &&
        out_receipt->save_root_bound &&
        (out_receipt->runtime_ready ||
         out_receipt->dm1_import_path_present ||
         out_receipt->resume_path_present ||
         out_receipt->csbwin_path_present);
    return out_receipt->valid;
}

int csb_v1_boot_runtime_import_csbwin_save_from_path_pc34(
    CSB_V1_BootProfile *profile,
    const char *csbwin_save_path,
    CSB_V1_BootRuntimeSaveImportReceipt_PC34 *out_receipt)
{
    int result;

    if (!out_receipt) {
        return 0;
    }
    (void)csb_v1_boot_runtime_save_import_receipt_pc34(
        profile,
        NULL,
        NULL,
        csbwin_save_path,
        out_receipt);
    out_receipt->csbwin_runtime_load_code = CSB_V1_LOAD_ERR_UNREADABLE;
    if (!profile || !csbwin_save_path || csbwin_save_path[0] == '\0') {
        return 0;
    }
    out_receipt->csbwin_runtime_load_attempted =
        out_receipt->runtime_ready && out_receipt->csbwin_path_present;
    if (!out_receipt->csbwin_runtime_load_attempted) {
        return 0;
    }

    /* ReDMCSB LOADSAVE.C F0435 owns the live load path. CSBWin
     * SaveGame.cpp lines 1707-1770 first opens CSBGAME.DAT, then the
     * .BAK fallback, and copies the decoded save block into runtime state.
     * Keep that policy under boot/runtime so utility and M11 callers do not
     * branch on legacy filenames after the CSB profile is verified. */
    result = csb_v1_runtime_load_game_from_path(&profile->runtime,
                                                csbwin_save_path);
    out_receipt->csbwin_runtime_load_code = result;
    out_receipt->csbwin_runtime_load_succeeded =
        result == CSB_V1_LOAD_OK ? 1 : 0;
    out_receipt->runtime_party_loaded_after =
        profile->runtime.party_state_valid ? 1 : 0;
    out_receipt->runtime_import_source_after =
        profile->runtime.party_state.ImportSource;
    out_receipt->runtime_champion_count_after =
        profile->runtime.party_state.ChampionCount;
    out_receipt->runtime_leader_index_after =
        profile->runtime.party_state.LeaderIndex;
    out_receipt->runtime_current_level_after = profile->runtime.current_level;
    out_receipt->runtime_game_time_after = profile->runtime.game_time;
    out_receipt->runtime_party_x_after = profile->runtime.party_x;
    out_receipt->runtime_party_y_after = profile->runtime.party_y;
    out_receipt->runtime_party_dir_after = profile->runtime.party_dir;
    out_receipt->valid =
        out_receipt->valid && out_receipt->csbwin_runtime_load_succeeded;
    return out_receipt->csbwin_runtime_load_succeeded;
}

int csb_v1_boot_runtime_tick_pc34(
    CSB_V1_BootProfile *profile,
    uint32_t *out_game_time)
{
    int result;

    if (out_game_time) {
        *out_game_time = 0U;
    }
    if (!profile) {
        return 0;
    }
    /* ReDMCSB BASE.C/LOADSAVE.C keep the 55 ms CSB V1 tick under the
     * live game profile. M11 should only ask boot to advance it. */
    result = csb_v1_runtime_tick_v1(&profile->runtime);
    if (out_game_time) {
        *out_game_time = profile->runtime.game_time;
    }
    return result;
}

int csb_v1_boot_runtime_object_icon_index_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing)
{
    if (!profile) {
        return -1;
    }
    /* ReDMCSB OBJECT.C F0031/F0033 object identity stays inside the
     * CSB runtime profile loaded by boot. M11 only asks for the result. */
    return csb_v1_runtime_object_icon_index_from_boot_profile_pc34(
        profile,
        thing);
}

int csb_v1_boot_runtime_object_action_set_index_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing)
{
    if (!profile) {
        return -1;
    }
    return csb_v1_runtime_object_action_set_index_from_boot_profile_pc34(
        profile,
        thing);
}

uint16_t csb_v1_boot_runtime_object_allowed_slots_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing)
{
    if (!profile) {
        return 0U;
    }
    /* ReDMCSB DATA.C G0311/CHAMPION.C inventory placement uses source
     * object-info masks owned by the CSB runtime. */
    return csb_v1_runtime_object_allowed_slots_from_boot_profile_pc34(
        profile,
        thing);
}

int csb_v1_boot_runtime_object_name_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short thing,
    char *out,
    size_t out_size)
{
    if (!profile) {
        if (out && out_size > 0U) {
            out[0] = '\0';
        }
        return 0;
    }
    return csb_v1_runtime_object_name_from_boot_profile_pc34(
        profile,
        thing,
        out,
        out_size);
}

int csb_v1_boot_runtime_read_container_slots_pc34(
    const CSB_V1_BootProfile *profile,
    unsigned short container_thing,
    unsigned short out_slots[8])
{
    if (!profile) {
        return -1;
    }
    /* ReDMCSB CHEST.C F0333/F0334 traverses and rewrites container thing
     * chains in the live dungeon tables; boot owns that runtime handle. */
    return csb_v1_runtime_read_container_slots_from_boot_profile_pc34(
        profile,
        container_thing,
        out_slots);
}

int csb_v1_boot_runtime_write_container_slots_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short container_thing,
    const unsigned short slots[8])
{
    if (!profile) {
        return 0;
    }
    return csb_v1_runtime_write_container_slots_from_boot_profile_pc34(
        profile,
        container_thing,
        slots);
}

int csb_v1_boot_runtime_set_thing_next_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short thing,
    unsigned short next_thing)
{
    if (!profile) {
        return 0;
    }
    return csb_v1_runtime_set_thing_next_from_boot_profile_pc34(
        profile,
        thing,
        next_thing);
}

int csb_v1_boot_runtime_write_inventory_slot_pc34(
    CSB_V1_BootProfile *profile,
    int champion_index,
    int csb_slot,
    unsigned short thing)
{
    if (!profile) {
        return 0;
    }
    /* ReDMCSB CHAMPION.C F0302 mutates champion slots in the runtime
     * party tables. Keep M11 writes behind the boot/runtime boundary. */
    return csb_v1_runtime_write_inventory_slot_from_boot_profile_pc34(
        profile,
        champion_index,
        csb_slot,
        thing);
}

int csb_v1_boot_runtime_write_leader_hand_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short thing)
{
    if (!profile) {
        return 0;
    }
    return csb_v1_runtime_write_leader_hand_from_boot_profile_pc34(
        profile,
        thing);
}

int csb_v1_boot_runtime_write_champion_vitals_pc34(
    CSB_V1_BootProfile *profile,
    int champion_index,
    int current_health,
    int current_stamina,
    int current_mana)
{
    if (!profile) {
        return 0;
    }
    return csb_v1_runtime_write_champion_vitals_from_boot_profile_pc34(
        profile,
        champion_index,
        current_health,
        current_stamina,
        current_mana);
}

int csb_v1_boot_runtime_m11_mirror_receipt_pc34(
    const CSB_V1_BootProfile *profile,
    CSB_V1_RuntimeM11MirrorReceipt_PC34 *out_receipt)
{
    if (!profile) {
        csb_v1_runtime_m11_mirror_receipt_init_pc34(out_receipt);
        return 0;
    }
    /* ReDMCSB LOADSAVE.C F0435/CHAMPION.C globals are runtime-owned after
     * boot handoff. M11 should consume one boot-issued mirror receipt. */
    return csb_v1_runtime_m11_mirror_receipt_from_boot_profile_pc34(
        profile,
        out_receipt);
}

int csb_v1_boot_runtime_trigger_front_wall_ornament_click_pc34(
    CSB_V1_BootProfile *profile,
    unsigned short leader_hand_thing,
    unsigned short *out_leader_hand_thing)
{
    if (!profile) {
        if (out_leader_hand_thing) {
            *out_leader_hand_thing = leader_hand_thing;
        }
        return 0;
    }
    /* ReDMCSB MOVESENS.C F0276 lines 1737-1785: front-wall ornament clicks
     * enter the live CSB sensor path, so boot owns the runtime mutation. */
    return csb_v1_runtime_trigger_front_wall_ornament_click_from_boot_profile_pc34(
        profile,
        leader_hand_thing,
        out_leader_hand_thing);
}

int csb_v1_boot_set_imported_party(CSB_V1_BootProfile *profile,
                                   const CSB_V1_PartyState *party)
{
    if (!profile || !party) return -1;
    if (party->ChampionCount <= 0 ||
        party->ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    profile->imported_party = *party;
    profile->imported_party_ready = 1;
    return 0;
}

int csb_v1_boot_set_imported_party_from_cmp(CSB_V1_BootProfile *profile,
                                            const uint8_t *cmp_buf,
                                            size_t cmp_size)
{
    int slot;
    if (!profile || !cmp_buf) return -1;
    profile->cmp_import_attempted = 1;
    slot = csb_v1_cmp_import_to_party(&profile->imported_party,
                                      cmp_buf, cmp_size);
    if (slot < 0) {
        profile->cmp_import_succeeded = 0;
        profile->cmp_imported_slot = slot;
        return slot;
    }
    profile->cmp_import_succeeded = 1;
    profile->cmp_imported_slot = slot;
    profile->cmp_imported_champion_count =
        profile->imported_party.ChampionCount;
    profile->imported_party_ready = 1;
    return 0;
}

int csb_v1_boot_mark_imported_party_ready(CSB_V1_BootProfile *profile)
{
    if (!profile) return -1;
    profile->cmp_import_attempted = 1;
    if (profile->imported_party.ChampionCount <= 0 ||
        profile->imported_party.ChampionCount > CSB_V1_MAX_CHAMPIONS) {
        return -1;
    }
    profile->cmp_import_succeeded = 1;
    profile->cmp_imported_champion_count =
        profile->imported_party.ChampionCount;
    profile->imported_party_ready = 1;
    return 0;
}

void csb_v1_boot_reset_engine_version_to_dm1(void)
{
    csb_v1_engine_version_display_set_csb(0);
}

void csb_v1_boot_set_save_root(CSB_V1_BootProfile *profile, const char *save_dir)
{
    if (!profile) return;
    if (save_dir && save_dir[0] != '\0') {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root), save_dir);
    } else if (profile->asset_root[0] != '\0') {
        snprintf(profile->save_root, sizeof(profile->save_root),
                 "%s/../%s", profile->asset_root, CSB_V1_BOOT_SAVE_SUBDIR);
    } else {
        csb_v1_boot_copy(profile->save_root, sizeof(profile->save_root),
                         csb_v1_runtime_save_dir());
    }
}

int csb_v1_boot_scan_assets(CSB_V1_BootProfile *profile, const char *data_dir)
{
    char graphics_path[ASSET_PATH_MAX];
    char dungeon_path[ASSET_PATH_MAX];
    int graphics_match = -1;
    int dungeon_match = -1;
    const CSB_V1_VariantInfo *variant;
    const char *root;

    if (!profile) return -1;
    root = (data_dir && data_dir[0] != '\0') ? data_dir : ".";
    csb_v1_boot_copy(profile->asset_root, sizeof(profile->asset_root), root);
    /* A reused launcher profile must not carry stale CSB paths across scans.
     * ReDMCSB only enters the CSB load path after the current media probe has
     * selected CSB and found a dungeon to load.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    profile->assets_verified = 0;
    profile->graphics_verified = 0;
    profile->dungeon_verified = 0;
    profile->graphics_path[0] = '\0';
    profile->dungeon_path[0] = '\0';
    profile->graphics_md5[0] = '\0';
    profile->dungeon_md5[0] = '\0';
    profile->graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    csb_v1_boot_reset_csbgraphics(profile);

    /* A successful csb_v1_boot_enter_game() hands the verified DUNGEON.DAT
     * off to the runtime as profile->runtime.dungeon_handle and to the global
     * singleton via csb_v1_dungeon_set_current().  A follow-up rescan
     * (different data_dir, removed asset, launcher refresh) must not leave
     * that handoff alive: the runtime-owned handle would still point at the
     * previous heap allocation, the global singleton would still expose the
     * previous dungeon through csb_v1_dungeon_get_current(), and the next
     * enter_game() would either fail to replace the handle (when verification
     * fails) or silently keep serving the previous dungeon through the new
     * profile paths.  Release the handle and reset the singleton here, before
     * the rescan-driven profile fields are populated.  The full runtime
     * re-init still happens in csb_v1_boot_enter_game() on the next launch.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (profile->runtime.dungeon_handle != NULL ||
        csb_v1_dungeon_get_current() != NULL) {
        csb_v1_dungeon_unload();
        free(profile->runtime.dungeon_handle);
        profile->runtime.dungeon_handle = NULL;
    }

    if (csb_v1_boot_scan_required_paths_fast(root,
                                             graphics_path,
                                             sizeof(graphics_path),
                                             &graphics_match,
                                             dungeon_path,
                                             sizeof(dungeon_path),
                                             &dungeon_match) ||
        csb_v1_boot_scan_required_paths(root,
                                        graphics_path,
                                        sizeof(graphics_path),
                                        &graphics_match,
                                        dungeon_path,
                                        sizeof(dungeon_path),
                                        &dungeon_match)) {
        profile->graphics_verified = 1;
        profile->dungeon_verified = 1;
    }
    if (profile->graphics_verified) {
        csb_v1_boot_copy(profile->graphics_path, sizeof(profile->graphics_path),
                         graphics_path);
        csb_v1_boot_copy(profile->graphics_md5, sizeof(profile->graphics_md5),
                         g_csb_boot_graphics_hashes[graphics_match]);
        profile->graphics_kind = csb_v1_boot_graphics_kind(graphics_path);
        profile->variant_id = g_csb_boot_graphics_variants[graphics_match];
    }
    if (profile->dungeon_verified) {
        csb_v1_boot_copy(profile->dungeon_path, sizeof(profile->dungeon_path),
                         dungeon_path);
        csb_v1_boot_copy(profile->dungeon_md5, sizeof(profile->dungeon_md5),
                         g_csb_boot_dungeon_hashes[dungeon_match]);
    }

    profile->assets_verified = profile->graphics_verified && profile->dungeon_verified;
    if (profile->variant_id == CSB_V1_VARIANT_UNKNOWN && profile->dungeon_verified) {
        profile->variant_id = CSB_V1_VARIANT_UNKNOWN;
    }
    variant = csb_v1_runtime_get_variant_info(profile->variant_id);
    csb_v1_boot_copy(profile->variant_label, sizeof(profile->variant_label),
                     variant->name);
    csb_v1_boot_copy(profile->media_ref, sizeof(profile->media_ref),
                     variant->media_ref);
    csb_v1_boot_copy(profile->version_id, sizeof(profile->version_id),
                     profile->graphics_md5[0] ? profile->graphics_md5 : "unknown");
    if (profile->save_root[0] == '\0') {
        csb_v1_boot_set_save_root(profile, NULL);
    }
    (void)csb_v1_boot_scan_csbgraphics(profile, NULL);
    csb_v1_boot_startup_assets_resolve_pc34(profile);
    if (profile->assets_verified) {
        profile->state = CSB_V1_BOOT_STATE_ASSETS_READY;
        return 0;
    }
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    return -1;
}

int csb_v1_boot_probe_available(const char *data_dir)
{
    CSB_V1_BootProfile profile;
    csb_v1_boot_profile_init(&profile);
    return csb_v1_boot_scan_assets(&profile, data_dir) == 0 ? 1 : 0;
}

int csb_v1_boot_enter_game(CSB_V1_BootProfile *profile)
{
    if (!profile || !profile->assets_verified) return -1;
    /* The launcher may carry several game profiles at once.  Do not let an
     * aggregate READY bit alone hand a non-CSB or partial profile to the CSB
     * runtime: ReDMCSB enters the CSB dungeon only after the CSB entrance/media
     * path has selected C28_ENTRANCE_CSB and the load path has a dungeon header
     * to consume.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 */
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0 ||
        !profile->graphics_verified ||
        !profile->dungeon_verified ||
        profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        return -1;
    }
    /* Explicit launch-to-runtime assumption gate.  This is the source-locked
     * boundary that catches DM1-shape defaults leaking into the CSB profile
     * (e.g. (11,29) HoC start, raw DM1 variant ordinal, non-CSB tick quantum).
     * The reason string is exposed via csb_v1_boot_last_assumption_reason()
     * so a misrouted profile can be attributed to a specific CSB invariant.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944
     * Source: csb_v1_runtime_pc34_compat.h CSB_V1_TICK_MS_NOMINAL */
    if (csb_v1_boot_assume_no_dm1_runtime(profile) != 0) {
        return -1;
    }
    if (profile->save_root[0] == '\0') {
        csb_v1_boot_set_save_root(profile, NULL);
    }
    /* Re-entering the CSB profile replaces the live dungeon context just as
     * ReDMCSB's global dungeon/map state is replaced when a new game is
     * loaded.  Clear the previous heap-owned runtime before csb_v1_runtime_init
     * overwrites its handle fields.
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    csb_v1_runtime_init(&profile->runtime, profile->asset_root);
    profile->runtime.variant_id = profile->variant_id;
    profile->runtime.difficulty = CSB_V1_DIFFICULTY_HARD;
    profile->runtime.save_dir = profile->save_root;
    profile->runtime.dungeon_path = profile->dungeon_path;
    profile->runtime.graphics_path = profile->graphics_path;
    profile->runtime.dungeon_asset.path = profile->dungeon_path;
    profile->runtime.dungeon_asset.kind = CSB_V1_ASSET_GFX_ARCHIVE_NONE;
    profile->runtime.graphics_asset.path = profile->graphics_path;
    profile->runtime.graphics_asset.kind = profile->graphics_kind;
    /* Copy entrance/start map indices from the boot profile so the runtime
     * honours the source-locked new-game map selection.
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (C255_MAP_INDEX_ENTRANCE)
     * Source: ReDMCSB LOADSAVE.C F0435 lines 1940-1944 (new-game map 0) */
    profile->runtime.entrance_map_index = profile->entrance_map_index;
    profile->runtime.start_map_index = profile->start_map_index;
    profile->runtime.state = CSB_STATE_TITLE;
    profile->runtime.chaos_magic.magic_initialized = 1;
    profile->runtime.chaos_magic.spell_grid_version = 0U;
    profile->runtime.chaos_magic.chaos_level = 0U;
    /* CSB shows engine version 2.1 on the title/dialog surface.
     * Flip the shared helper when a verified CSB boot profile
     * actually enters the CSB runtime, then reset it on cleanup.
     * Source: ReDMCSB CHANGE8_13; DIALOG.C:2014-2023. */
    csb_v1_engine_version_display_set_csb(1);
    profile->engine_version_displayed = 1;
    if (profile->imported_party_ready) {
        (void)csb_v1_runtime_set_party_state(&profile->runtime,
                                             &profile->imported_party);
    }
    (void)csb_v1_boot_load_object_names_m564(profile);
    /* Load the verified DUNGEON.DAT into the runtime so that the
     * dungeon-layer accessors (csb_v1_dungeon_get_current_level,
     * csb_v1_dungeon_get_square_type, ...) become live immediately
     * after launch — without a second hash search or a follow-up
     * csb_v1_runtime_boot() call from the game-view.
     *
     * The dungeon is heap-allocated and owned by the runtime profile
     * (dungeon_handle).  csb_v1_runtime_cleanup() / csb_v1_boot_cleanup()
     * are responsible for releasing it.
     *
     * Failure is non-fatal: if the verified path cannot be opened
     * (e.g. archive-backed path not yet materialized by M12), the
     * runtime continues with dungeon_handle == NULL and the dungeon
     * accessors return ENDOF — matching csb_v1_runtime_boot()'s
     * pre-existing tolerant behaviour.
     *
     * Source: CSBWin/CSBCode.cpp:6800-6950 LoadDungeon
     * Source: ReDMCSB DUNGEON.C F0237 dungeon load entry
     * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 entrance micro-dungeon */
    {
        CSB_V1_DungeonData *dungeon = (CSB_V1_DungeonData *)calloc(1, sizeof(CSB_V1_DungeonData));
        if (dungeon) {
            if (csb_v1_dungeon_load_from_file(dungeon, profile->dungeon_path) == 0) {
                profile->runtime.dungeon_handle = dungeon;
                csb_v1_dungeon_set_current(dungeon);
                csb_v1_dungeon_set_current_level(0);
            } else {
                free(dungeon);
                profile->runtime.dungeon_handle = NULL;
            }
        }
    }
    profile->state = CSB_V1_BOOT_STATE_RUNTIME_READY;
    return 0;
}

void csb_v1_boot_cleanup(CSB_V1_BootProfile *profile)
{
    if (!profile) return;
    /* The boot profile owns the runtime handoff dungeon.  Release it through
     * the same runtime cleanup path used by csb_v1_runtime_boot() so the
     * singleton dungeon/map accessors do not retain a stale CSB context after
     * leaving the profile.
     * Source: ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2755 */
    csb_v1_runtime_cleanup(&profile->runtime);
    csb_v1_boot_reset_csbgraphics(profile);
    profile->state = CSB_V1_BOOT_STATE_PROFILE_READY;
    memset(&profile->runtime, 0, sizeof(profile->runtime));
    csb_v1_engine_version_display_set_csb(0);
    profile->engine_version_displayed = 0;
}

size_t csb_v1_boot_diagnostic_report(const CSB_V1_BootProfile *profile,
                                     char *buf,
                                     size_t buf_size)
{
    int n;
    const char *engine_version_str;
    if (!profile || !buf || buf_size == 0U) return 0U;
    engine_version_str = csb_v1_engine_version_display_get();
    n = snprintf(buf, buf_size,
                 "=== CSB V1 Boot Profile ===\n"
                 "state=%d verified=%s variant=%s media=%s\n"
                 "asset_root=%s\n"
                 "graphics=%s md5=%s\n"
                 "dungeon=%s md5=%s\n"
                 "save_root=%s tick_ms=%u entrance_map=%u start_map=%u\n"
                 "engine_version=%s flipped=%s\n"
                 "csbgraphics_scan attempted=%s result=%s plan=%s ready=%s planned=%u\n"
                 "cmp_import attempted=%s succeeded=%s slot=%d champions=%d\n"
                 "imported_party_ready=%s\n",
                 (int)profile->state,
                 profile->assets_verified ? "YES" : "NO",
                 profile->variant_label,
                 profile->media_ref,
                 profile->asset_root[0] ? profile->asset_root : "(unset)",
                 profile->graphics_path[0] ? profile->graphics_path : "(missing)",
                 profile->graphics_md5[0] ? profile->graphics_md5 : "(missing)",
                 profile->dungeon_path[0] ? profile->dungeon_path : "(missing)",
                 profile->dungeon_md5[0] ? profile->dungeon_md5 : "(missing)",
                 profile->save_root[0] ? profile->save_root : "(unset)",
                 (unsigned)profile->tick_ms,
                 (unsigned)profile->entrance_map_index,
                 (unsigned)profile->start_map_index,
                 engine_version_str ? engine_version_str : "(unset)",
                 profile->engine_version_displayed ? "YES" : "NO",
                 profile->csbgraphics_scan_attempted ? "YES" : "NO",
                 csb_v1_csbgraphics_dat_real_result_name(
                     profile->csbgraphics_scan_result),
                 csb_v1_csbgraphics_m11_runtime_plan_result_name(
                     profile->csbgraphics_plan_result),
                 profile->csbgraphics_m11_plan.ready ? "YES" : "NO",
                 (unsigned)profile->csbgraphics_m11_plan.planned_count,
                 profile->cmp_import_attempted ? "YES" : "NO",
                 profile->cmp_import_succeeded ? "YES" : "NO",
                 profile->cmp_imported_slot,
                 profile->cmp_imported_champion_count,
                 profile->imported_party_ready ? "YES" : "NO");
    if (n < 0) return 0U;
    return (size_t)n < buf_size ? (size_t)n : buf_size - 1U;
}

void csb_v1_boot_print_summary(const CSB_V1_BootProfile *profile)
{
    if (!profile) {
        printf("CSB: no boot profile\n");
        return;
    }
    printf("CSB: %s assets=%s dungeon=%s graphics=%s\n",
           profile->variant_label,
           profile->assets_verified ? "READY" : "missing",
           profile->dungeon_verified ? "ok" : "missing",
           profile->graphics_verified ? "ok" : "missing");
}

/* ── CSB V1 boot profile -> M11 entry guard ─────────────────────
 *
 * The canonical CSB V1 media hash registry below mirrors the supported
 * graphics md5s and the one dungeon md5 the launcher scans for.
 * The gate hashes here MUST stay in sync with g_csb_boot_graphics_hashes
 * and g_csb_boot_dungeon_hashes above; the registry is the source of
 * truth that the M11 dispatch consults before activating a CSB launch.
 *
 * Source: ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB media class
 *   detection by hash, not by filename/path).
 * Source: ReDMCSB LOADSAVE.C F0435 lines 1936-1944 (new-game dungeon
 *   load is gated on a hash-known dungeon header). */
static const char *const g_csb_m11_entry_graphics_hashes[] = {
    "61fbfd56887c94adc26888a9491c6611", /* PC DOS 3.4 English         MEDIA278 */
    "ebf6a57af3f27782e358c0490bfd2f2e", /* Atari ST 2.0/2.1 English   MEDIA332 */
    "e0ce7ac5160ca5540e90cf09ab9fad49", /* Atari ST 2.x hard-disk     MEDIA332 */
    "291e1bc6803e3dc4b974c60117ca5d68", /* Amiga 3.5 English          MEDIA529 */
    "cefaddfdf5651df2c91f61b5611a8362", /* Amiga 3.5 Multilanguage    MEDIA529 */
    NULL
};

static const char *const g_csb_m11_entry_dungeon_hashes[] = {
    "6695d2acebce49f95db1d8f3a5c733de", /* shared CSB V1 DUNGEON.DAT  MEDIA278/332/529 */
    NULL
};

static int csb_v1_md5_is_canonical_graphics(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_graphics_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_graphics_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int csb_v1_md5_is_canonical_dungeon(const char *md5)
{
    size_t i;
    if (!md5 || md5[0] == '\0') return 0;
    for (i = 0U; g_csb_m11_entry_dungeon_hashes[i] != NULL; ++i) {
        if (strcmp(md5, g_csb_m11_entry_dungeon_hashes[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static void csb_v1_boot_gate_set_reason(char *reason,
                                        size_t reason_size,
                                        const char *fmt,
                                        ...)
{
    if (!reason || reason_size == 0U) return;
    {
        va_list args;
        va_start(args, fmt);
        vsnprintf(reason, reason_size, fmt, args);
        va_end(args);
        reason[reason_size - 1U] = '\0';
    }
}

int csb_v1_boot_graphics_dungeon_m11_entry_gate(const char *graphics_md5,
                                                const char *dungeon_md5,
                                                char *reason,
                                                size_t reason_size)
{
    if (!graphics_md5 || graphics_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 is empty "
            "(scanner did not record a matched graphics hash)");
        return 0;
    }
    if (!dungeon_md5 || dungeon_md5[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 is empty "
            "(scanner did not record a matched dungeon hash)");
        return 0;
    }
    if (!csb_v1_md5_is_canonical_graphics(graphics_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: GRAPHICS md5 %s is not in the canonical "
            "CSB V1 media registry (PC3.4EN / Atari ST 2.x / Amiga 3.x)",
            graphics_md5);
        return 0;
    }
    if (!csb_v1_md5_is_canonical_dungeon(dungeon_md5)) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: DUNGEON md5 %s is not in the canonical "
            "CSB V1 media registry",
            dungeon_md5);
        return 0;
    }
    csb_v1_boot_gate_set_reason(reason, reason_size,
        "CSB M11 entry guard: GRAPHICS=%s DUNGEON=%s accepted",
        graphics_md5, dungeon_md5);
    return 1;
}

int csb_v1_boot_profile_m11_entry_gate(const CSB_V1_BootProfile *profile,
                                       char *reason,
                                       size_t reason_size)
{
    if (!profile) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: NULL boot profile "
            "(launcher did not initialize a CSB profile before dispatch)");
        return 0;
    }
    if (strcmp(profile->game_id, CSB_V1_BOOT_GAME_ID) != 0) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: profile game_id=%s != %s "
            "(foreign game reached the CSB entry path)",
            profile->game_id[0] ? profile->game_id : "(empty)",
            CSB_V1_BOOT_GAME_ID);
        return 0;
    }
    if (profile->state < CSB_V1_BOOT_STATE_ASSETS_READY) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: boot state=%d below ASSETS_READY "
            "(scanner did not record both required assets)",
            (int)profile->state);
        return 0;
    }
    if (!profile->assets_verified ||
        !profile->graphics_verified ||
        !profile->dungeon_verified) {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: assets_verified=%d graphics_verified=%d "
            "dungeon_verified=%d (one or more required files missing)",
            profile->assets_verified,
            profile->graphics_verified,
            profile->dungeon_verified);
        return 0;
    }
    if (profile->graphics_path[0] == '\0' ||
        profile->dungeon_path[0] == '\0') {
        csb_v1_boot_gate_set_reason(reason, reason_size,
            "CSB M11 entry guard: empty asset path "
            "(graphics_path=%s dungeon_path=%s)",
            profile->graphics_path[0] ? profile->graphics_path : "(empty)",
            profile->dungeon_path[0] ? profile->dungeon_path : "(empty)");
        return 0;
    }
    return csb_v1_boot_graphics_dungeon_m11_entry_gate(
        profile->graphics_md5, profile->dungeon_md5, reason, reason_size);
}

const char *csb_v1_boot_source_evidence(void)
{
    return
        "ReDMCSB ENTRANCE.C F0806 lines 409-441: CSB entrance setup and C28_ENTRANCE_CSB palette\n"
        "ReDMCSB ENTRANCE.C F0806 lines 857-883: entrance waits then switches G0298_B_NewGame\n"
        "ReDMCSB LOADSAVE.C F0435 lines 1940-1944: new-game party location and map 0\n"
        "ReDMCSB BASE.C lines 36-39: G0298_B_NewGame boot/load mode storage\n";
}
