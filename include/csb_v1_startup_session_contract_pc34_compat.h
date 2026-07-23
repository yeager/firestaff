#ifndef FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H

#include "csb_v1_boot.h"
#include "csb_v1_startup_real_asset_receipt.h"

typedef struct CSB_V1_StartupSessionTerminalReceipt_PC34 {
    int valid;
    int c001_complete;
    int terminal_f0807_complete;
    int c017_ready;
    int c040_ready;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionTerminalReceipt_PC34;

/* ReDMCSB PANEL.C F0347 hands C017/C040 to the live dungeon only as their
 * original PC34 crops.  This surface-only predicate is usable immediately
 * before the entrance -> HUD stage transition. */
int csb_v1_startup_session_hud_surface_contract_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session);

/* TITLE.C F0437 and ENTRANCE.C F0806/F0807 must still be backed by the
 * resident package C001/C004/C002/C003 surfaces before PANEL.C's C017/C040
 * HUD can become live. This predicate does not require the playback stage to
 * have already mutated to HUD. */
int csb_v1_startup_session_full_surface_contract_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session);

/* The terminal F0807 handoff is only usable when the complete C001 title and
 * C017/C040 HUD package was consumed from the same hash-verified session. */
typedef struct CSB_V1_StartupSessionTerminalPackageReceipt_PC34 {
    int valid;
    int real_package_matched;
    int c001_title_consumed;
    int c017_hud_consumed;
    int c040_hud_consumed;
    int terminal_f0807_complete;
    int no_legacy_wrappers;
    int no_fallback_routes;
    unsigned int source_tick;
    unsigned int session_generation;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionTerminalPackageReceipt_PC34;

/* TITLE.C F0437 retains one C001 bitmap for PRESENTS, CHAOS and STRIKES
 * BACK. This receipt pins those real regions to the package identity that
 * later reaches the terminal C017/C040 HUD session. */
typedef struct CSB_V1_StartupSessionPackageTitleReceipt_PC34 {
    int valid;
    int real_package_matched;
    int c001_title_ready;
    int c001_presents_ready;
    int c001_chaos_ready;
    int c001_strikes_back_ready;
    int title_to_hud_same_session;
    int no_legacy_wrappers;
    int no_fallback_routes;
    unsigned int source_tick;
    unsigned int session_generation;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionPackageTitleReceipt_PC34;

/* ReDMCSB ENTRANCE.C F0806 keeps C004 behind the moving C002/C003 door
 * strips. This receipt proves that the opening frame still belongs to the
 * same verified C001 package session rather than a replacement raster. */
typedef struct CSB_V1_StartupSessionOpeningDoorReceipt_PC34 {
    int valid;
    int real_package_matched;
    int c004_entrance_ready;
    int c002_left_door_ready;
    int c003_right_door_ready;
    int opening_to_title_same_session;
    int no_legacy_wrappers;
    int no_fallback_routes;
    unsigned int source_tick;
    unsigned int session_generation;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionOpeningDoorReceipt_PC34;

/* F0438/F0807 advances the captured C004/C002/C003 page exactly once per
 * source VBlank. This chained receipt prevents a host from replaying a
 * valid page at a later door step or from skipping source-owned frames. */
typedef struct CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34 {
    int valid;
    int first_source_frame;
    int real_package_matched;
    int c004_c002_c003_consumed;
    int no_legacy_wrappers;
    int no_fallback_routes;
    unsigned int previous_door_step;
    unsigned int door_step;
    unsigned int source_tick;
    unsigned int session_generation;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34;

/* TITLE.C F0437 presents each C001 phase before ENTRANCE.C F0806 advances
 * C004/C002/C003. These are host-consumption facts, not substitute frames. */
typedef struct CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 {
    int valid;
    int real_package_matched;
    int presents_consumed;
    int chaos_consumed;
    int strikes_back_consumed;
    int c004_c002_c003_consumed;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    unsigned int session_generation;
    uint32_t presents_host_surface_hash;
    uint32_t chaos_host_surface_hash;
    uint32_t strikes_host_surface_hash;
    uint32_t opening_host_surface_hash;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34;

/* PANEL.C restores C017 after its C040 clear; the first DUNGEON.C door tick
 * and COMMAND.C input must retain that same package-owned HUD session. */
typedef struct CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 {
    int valid;
    int real_package_matched;
    int c017_hud_consumed;
    int c040_hud_consumed;
    int first_live_door_frame;
    int first_runtime_input;
    int no_legacy_wrappers;
    int no_synthetic_surface;
    unsigned int session_generation;
    unsigned int hud_source_tick;
    unsigned int first_runtime_tick;
    uint32_t hud_host_surface_hash;
    uint64_t real_asset_receipt_hash;
    uint64_t consumed_surface_hash;
} CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34;

typedef struct CSB_V1_StartupSessionLiveHudReceipt_PC34 {
    int valid;
    int c040_cleared_once;
    int c017_live_base_only;
    int c017_source_asset_id;
    int c017_width;
    int c017_height;
    int special_palette;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionLiveHudReceipt_PC34;

typedef struct CSB_V1_StartupSessionDoorHudTickReceipt_PC34 {
    int valid;
    int first_live_door_tick;
    unsigned int previous_door_step;
    unsigned int door_step;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionDoorHudTickReceipt_PC34;

typedef enum CSB_V1_StartupSessionMovementCommand_PC34 {
    CSB_V1_STARTUP_SESSION_MOVEMENT_NONE_PC34 = 0,
    CSB_V1_STARTUP_SESSION_MOVEMENT_FORWARD_PC34,
    CSB_V1_STARTUP_SESSION_MOVEMENT_BACKWARD_PC34,
    CSB_V1_STARTUP_SESSION_MOVEMENT_TURN_LEFT_PC34,
    CSB_V1_STARTUP_SESSION_MOVEMENT_TURN_RIGHT_PC34
} CSB_V1_StartupSessionMovementCommand_PC34;

typedef struct CSB_V1_StartupSessionInputReceipt_PC34 {
    int valid;
    int first_post_c040_input;
    CSB_V1_StartupSessionMovementCommand_PC34 command;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionInputReceipt_PC34;

typedef enum CSB_V1_StartupSessionActionCommand_PC34 {
    CSB_V1_STARTUP_SESSION_ACTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_SESSION_ACTION_LEFT_HAND_PC34,
    CSB_V1_STARTUP_SESSION_ACTION_RIGHT_HAND_PC34,
    CSB_V1_STARTUP_SESSION_ACTION_CAST_PC34
} CSB_V1_StartupSessionActionCommand_PC34;

typedef struct CSB_V1_StartupSessionActionReceipt_PC34 {
    int valid;
    int first_post_live_hud_action;
    CSB_V1_StartupSessionActionCommand_PC34 command;
    int c017_source_asset_id;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionActionReceipt_PC34;

typedef enum CSB_V1_StartupSessionSelectionKind_PC34 {
    CSB_V1_STARTUP_SESSION_SELECTION_NONE_PC34 = 0,
    CSB_V1_STARTUP_SESSION_SELECTION_SPELL_RUNE_PC34,
    CSB_V1_STARTUP_SESSION_SELECTION_ACTION_SLOT_PC34
} CSB_V1_StartupSessionSelectionKind_PC34;

typedef struct CSB_V1_StartupSessionSelectionReceipt_PC34 {
    int valid;
    CSB_V1_StartupSessionSelectionKind_PC34 kind;
    int selection_index;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionSelectionReceipt_PC34;

/* ReDMCSB TITLE.C F0437, ENTRANCE.C F0807, PANEL.C F0347. */
int csb_v1_startup_session_terminal_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupSessionTerminalReceipt_PC34 *out_receipt);

/* ReDMCSB TITLE.C F0437, ENTRANCE.C F0807 and PANEL.C F0347: retain the
 * hash-verified C001/C017/C040 package through the terminal entrance edge. */
int csb_v1_startup_session_terminal_package_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupSessionTerminalPackageReceipt_PC34 *out_receipt);

int csb_v1_startup_session_package_title_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    CSB_V1_StartupSessionPackageTitleReceipt_PC34 *out_receipt);

int csb_v1_startup_session_opening_door_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface,
    CSB_V1_StartupSessionOpeningDoorReceipt_PC34 *out_receipt);

int csb_v1_startup_session_opening_door_tick_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_surface,
    const CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34 *previous_receipt,
    CSB_V1_StartupSessionOpeningDoorTickReceipt_PC34 *out_receipt);

int csb_v1_startup_session_title_opening_consumption_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *presents_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *chaos_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *strikes_host,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *opening_host,
    CSB_V1_StartupSessionTitleOpeningConsumptionReceipt_PC34 *out_receipt);

int csb_v1_startup_session_hud_door_input_package_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *hud_host,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    const CSB_V1_StartupSessionDoorHudTickReceipt_PC34 *door_receipt,
    const CSB_V1_StartupSessionInputReceipt_PC34 *input_receipt,
    CSB_V1_StartupSessionHudDoorInputPackageReceipt_PC34 *out_receipt);

/* ReDMCSB PANEL.C F0346/F0347: one C040 clear returns to neutral C017. */
int csb_v1_startup_session_live_hud_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionTerminalReceipt_PC34 *terminal_receipt,
    unsigned int c040_clear_count,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionLiveHudReceipt_PC34 *out_receipt);

/* ReDMCSB DUNGEON.C advances a door one source tick at a time after PANEL.C
 * has returned from candidate C040 to the live C017 surface. */
int csb_v1_startup_session_first_door_hud_tick_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    unsigned int previous_door_step,
    unsigned int door_step,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionDoorHudTickReceipt_PC34 *out_receipt);

/* ReDMCSB COMMAND.C dispatches movement only after PANEL.C has returned the
 * live C017 surface. */
int csb_v1_startup_session_first_input_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionMovementCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionInputReceipt_PC34 *out_receipt);

/* ReDMCSB COMMAND.C dispatches HUD actions only after PANEL.C has restored
 * the normal C017 session surface. */
int csb_v1_startup_session_first_action_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    CSB_V1_StartupSessionActionCommand_PC34 command,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionActionReceipt_PC34 *out_receipt);

/* ReDMCSB COMMAND.C selection follows the C017-owned action dispatch on its
 * next tick; spell runes require CAST, while action slots require a hand. */
int csb_v1_startup_session_selection_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupSessionLiveHudReceipt_PC34 *live_hud_receipt,
    const CSB_V1_StartupSessionActionReceipt_PC34 *action_receipt,
    CSB_V1_StartupSessionSelectionKind_PC34 kind,
    int selection_index,
    unsigned int source_tick,
    unsigned int session_generation,
    CSB_V1_StartupSessionSelectionReceipt_PC34 *out_receipt);

#endif
