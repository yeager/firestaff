#include "dm1_v1_startup_handoff_m11_bridge_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"

#include <string.h>

/* Callback implementations for DM1_V1_StartupHandoffCallbacks_PC34.
 *
 * The prelude phase fires the SWSH sound; the post-launch phase fires
 * the title animation and entrance screen. Both phases are fail-closed:
 * if any callback returns 0, the handoff stops and the game view opens
 * without the startup sequence (matching dm1StartupIntroBypassed). */

static int m11_bridge_begin_prelude_plan(
    void *user,
    const DM1_V1_StartupHandoffPreludePlan_PC34 *plan)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    if (!s || !plan) return 0;
    s->preludePlanReceived = 1;
    return 1;
}

static int m11_bridge_end_prelude_plan(void *user)
{
    (void)user;
    return 1;
}

static int m11_bridge_begin_post_launch_plan(
    void *user,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34 *plan)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    if (!s || !plan) return 0;
    s->postLaunchPlanReceived = 1;
    return 1;
}

static int m11_bridge_end_post_launch_plan(void *user)
{
    (void)user;
    return 1;
}

static int m11_bridge_report_source_order_failure(
    void *user, const char *evidence)
{
    (void)user;
    (void)evidence;
    return 0;
}

static int m11_bridge_raise_window(void *user)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    if (!s) return 0;
    s->windowRaised = 1;
    return 1;
}

static int m11_bridge_play_swsh(
    void *user, const char *game_id, int preserve_audio)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    (void)game_id;
    (void)preserve_audio;
    if (!s) return 0;
    /* The actual PSG-to-PCM swoosh playback is done via M11's SDL audio
     * layer. This callback records that the DM1-owned prelude requested
     * the swoosh and M11 acknowledged it. The host audio path is gated
     * on swooshBound: if no swoosh source was bound at init, the sound
     * is silently skipped (matching the original when SWSHSND.DAT is
     * absent). */
    s->swooshPlayed = s->swooshBound ? 1 : 0;
    return 1;
}

static int m11_bridge_discard_presentation_texture(void *user)
{
    (void)user;
    return 1;
}

static int m11_bridge_play_title(
    void *user, const char *source_id, int *out_played_any_frame)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    (void)source_id;
    if (!s) return 0;
    /* Title animation (53-frame C001 zoom at 55ms cadence) requires
     * the asset loader and the M11 render pipeline. This callback
     * records the DM1-owned handoff request; the actual frame stepping
     * is driven by the M11 tick loop when dm1StartupTitlePhaseActive
     * is set. For now, acknowledge the request and let the game view
     * handle the animation in its tick function. */
    s->titlePlayed = 1;
    if (out_played_any_frame) *out_played_any_frame = 1;
    return 1;
}

static int m11_bridge_play_entrance(
    void *user, const char *source_id,
    int auto_enter_after_ms, int *out_entrance_command)
{
    DM1_V1_StartupHandoffM11BridgeStatePc34 *s =
        (DM1_V1_StartupHandoffM11BridgeStatePc34 *)user;
    (void)source_id;
    (void)auto_enter_after_ms;
    if (!s) return 0;
    /* Entrance screen (palette crossfade, wait for Enter/click).
     * The original waits for user input or auto-enters after the
     * specified timeout. This callback records the request; the M11
     * tick loop drives the entrance frontend when the entrance phase
     * is active. For now, auto-enter immediately. */
    s->entranceCompleted = 1;
    s->entranceCommand = ENTRANCE_COMPAT_COMMAND_PATH_ENTER;
    if (out_entrance_command)
        *out_entrance_command = ENTRANCE_COMPAT_COMMAND_PATH_ENTER;
    return 1;
}

int dm1_v1_startup_handoff_m11_bridge_init_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    void *m11State)
{
    if (!state) return 0;
    memset(state, 0, sizeof(*state));
    state->m11State = m11State;
    return 1;
}

int dm1_v1_startup_handoff_m11_bridge_execute_prelude_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    const char *gameId)
{
    DM1_V1_StartupHandoffCallbacks_PC34 callbacks;
    if (!state) return 0;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.user = state;
    callbacks.begin_prelude_plan = m11_bridge_begin_prelude_plan;
    callbacks.end_prelude_plan = m11_bridge_end_prelude_plan;
    callbacks.report_source_order_failure =
        m11_bridge_report_source_order_failure;
    callbacks.raise_window = m11_bridge_raise_window;
    callbacks.play_swsh = m11_bridge_play_swsh;
    callbacks.discard_presentation_texture =
        m11_bridge_discard_presentation_texture;
    return dm1_v1_startup_execute_handoff_prelude_pc34(gameId, &callbacks);
}

int dm1_v1_startup_handoff_m11_bridge_execute_post_launch_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    const char *sourceId)
{
    DM1_V1_StartupHandoffCallbacks_PC34 callbacks;
    int titlePlayed = 0;
    int entranceCommand = 0;
    if (!state) return 0;
    memset(&callbacks, 0, sizeof(callbacks));
    callbacks.user = state;
    callbacks.begin_post_launch_plan = m11_bridge_begin_post_launch_plan;
    callbacks.end_post_launch_plan = m11_bridge_end_post_launch_plan;
    callbacks.raise_window = m11_bridge_raise_window;
    callbacks.play_title = m11_bridge_play_title;
    callbacks.play_entrance = m11_bridge_play_entrance;
    if (!dm1_v1_startup_execute_handoff_post_launch_pc34(
            sourceId, &callbacks, &titlePlayed, &entranceCommand)) {
        return 0;
    }
    state->titlePlayed = titlePlayed;
    state->entranceCommand = entranceCommand;
    return 1;
}

int dm1_v1_startup_handoff_m11_bridge_receipt_pc34(
    const DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    DM1_V1_StartupHandoffM11BridgeReceiptPc34 *outReceipt)
{
    if (!state || !outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    outReceipt->valid = 1;
    outReceipt->preludeExecuted = state->preludePlanReceived;
    outReceipt->postLaunchExecuted = state->postLaunchPlanReceived;
    outReceipt->swooshPlayed = state->swooshPlayed;
    outReceipt->titlePlayed = state->titlePlayed;
    outReceipt->entranceCompleted = state->entranceCompleted;
    outReceipt->entranceCommand = state->entranceCommand;
    outReceipt->sourceEvidence =
        dm1_v1_startup_handoff_m11_bridge_source_evidence_pc34();
    return 1;
}

const char *dm1_v1_startup_handoff_m11_bridge_source_evidence_pc34(void)
{
    return "ReDMCSB APPA.C F0908/F0909/F0910 (SWSH), "
           "STARTUP2.C F0437 (title), ENTRANCE.C F0441 (entrance): "
           "M11 bridge callbacks for DM1 startup handoff prelude "
           "and post-launch phases.";
}
