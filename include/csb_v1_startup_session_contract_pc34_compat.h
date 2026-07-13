#ifndef FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H

#include "csb_v1_boot.h"

typedef struct CSB_V1_StartupSessionTerminalReceipt_PC34 {
    int valid;
    int c001_complete;
    int terminal_f0807_complete;
    int c017_ready;
    int c040_ready;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionTerminalReceipt_PC34;

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
