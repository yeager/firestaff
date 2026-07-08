#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"
#include "title_frontend_v1.h"
#include <stdio.h>
#include <string.h>

#define DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34 18u
#define DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34 23u
#define DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34 53u
#define DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34 2u
#define DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34 1u
#define DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34 55u

const char* dm1_v1_startup_stage_name_pc34(DM1_V1_StartupStage_PC34 stage) {
    switch (stage) {
        case DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34:
            return "SWSH_LOGO";
        case DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34:
            return "SWSH_RUN_START";
        case DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34:
            return "TITLE_BEGIN";
        case DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34:
            return "TITLE_LAST_FRAME";
        case DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34:
            return "MENU_ELIGIBLE";
        case DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34:
            return "ENTRANCE_WAIT";
    }
    return "UNKNOWN";
}

int dm1_v1_startup_stage_after_pc34(DM1_V1_StartupStage_PC34 later,
                                    DM1_V1_StartupStage_PC34 earlier) {
    return (unsigned int)later > (unsigned int)earlier;
}

int dm1_v1_startup_launch_path_bypasses_intro_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34:
            /* Firestaff --game dm1 bypasses only the M12 launcher surface.
             * main_loop_m11.c still routes through the source-visible
             * SWSH -> TITLE -> ENTRANCE startup sequence before gameplay. */
            return 0;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            /* M11_GameView_StartDm1() is a focused test/dev entry point.
             * It intentionally starts the DM1 game view directly and must
             * not be confused with the ReDMCSB SWSH -> TITLE -> ENTRANCE
             * launcher handoff used by normal DM1 startup. */
            return 1;
    }
    return 1;
}

static int dm1_v1_startup_launch_path_started_from_launcher_pc34(
    DM1_V1_StartupLaunchPath_PC34 path) {
    switch (path) {
        case DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34:
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_CLI_PC34:
            return 1;
        case DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34:
            return 0;
    }
    return 0;
}

int dm1_v1_startup_source_visible_handoff_required_pc34(const char* game_id) {
    return game_id && strcmp(game_id, "dm1") == 0 ? 1 : 0;
}

int dm1_v1_startup_intro_bypass_applies_to_source_pc34(const char* sourceId,
                                                       int bypassed) {
    return dm1_v1_startup_source_visible_handoff_required_pc34(sourceId) &&
           bypassed ? 1 : 0;
}

int dm1_v1_startup_selected_entry_receipt_valid_pc34(const char* game_id,
                                                     int intro_bypassed) {
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(game_id)) {
        return 1;
    }
    return intro_bypassed ? 0 : 1;
}

int dm1_v1_startup_launch_path_receipt_pc34(
    const DM1_V1_StartupLaunchPathFacts_PC34* facts,
    DM1_V1_StartupLaunchPathReceipt_PC34* out_receipt) {
    DM1_V1_StartupLaunchPathReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    receipt.handled = 1;
    receipt.intro_bypassed =
        dm1_v1_startup_launch_path_bypasses_intro_pc34(
            facts->launch_path);
    receipt.started_from_launcher =
        dm1_v1_startup_launch_path_started_from_launcher_pc34(
            facts->launch_path);
    receipt.selected_entry_receipt_valid =
        dm1_v1_startup_selected_entry_receipt_valid_pc34(
            facts->source_id,
            receipt.intro_bypassed);
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_runtime_start_receipt_pc34(
    const DM1_V1_StartupRuntimeStartFacts_PC34* facts,
    DM1_V1_StartupRuntimeStartReceipt_PC34* out_receipt) {
    DM1_V1_StartupRuntimeStartReceipt_PC34 receipt;
    DM1_V1_StartupLaunchPathFacts_PC34 launch_facts;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    memset(&launch_facts, 0, sizeof(launch_facts));
    launch_facts.source_id = facts->game_id;
    launch_facts.launch_path = facts->launch_path;
    if (!dm1_v1_startup_launch_path_receipt_pc34(
            &launch_facts,
            &receipt.launch_path_receipt)) {
        return 0;
    }

    /* ReDMCSB: ENTRANCE.C hands off to the loaded dungeon only after the
     * source-visible startup path. Firestaff keeps the host-owned dungeon
     * allocation in M11, but the DM1 module owns the launch receipt that
     * marks gameplay active after that load succeeds. */
    receipt.handled = 1;
    receipt.active = 1;
    receipt.started_from_launcher =
        receipt.launch_path_receipt.started_from_launcher;
    receipt.source_kind = facts->source_kind;
    snprintf(receipt.boot_asset_md5,
             sizeof(receipt.boot_asset_md5),
             "%s",
             facts->verified_asset_md5 ? facts->verified_asset_md5 : "");
    receipt.presentation_mode = facts->presentation_mode;
    receipt.presentation_width = facts->presentation_width;
    receipt.presentation_height = facts->presentation_height;
    receipt.font_scale =
        (facts->font_scale >= 1 && facts->font_scale <= 3)
            ? facts->font_scale
            : 0;
    snprintf(receipt.title,
             sizeof(receipt.title),
             "%s",
             facts->title ? facts->title : "DUNGEON MASTER");
    snprintf(receipt.source_id,
             sizeof(receipt.source_id),
             "%s",
             facts->source_id ? facts->source_id : "launcher");
    snprintf(receipt.dungeon_path,
             sizeof(receipt.dungeon_path),
             "%s",
             facts->dungeon_path ? facts->dungeon_path : "");
    snprintf(receipt.status_title,
             sizeof(receipt.status_title),
             "%s",
             "BOOT");
    snprintf(receipt.status_detail,
             sizeof(receipt.status_detail),
             "%s",
             "GAME DATA LOADED");
    snprintf(receipt.inspect_title,
             sizeof(receipt.inspect_title),
             "%s",
             "READY");
    snprintf(receipt.inspect_detail,
             sizeof(receipt.inspect_detail),
             "%s",
             "CLICK CENTER TO ADVANCE OR READ, CLICK SIDES TO TURN, TAB PICKS THE FRONT CHAMPION");
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_dungeon_path_receipt_pc34(
    const DM1_V1_StartupDungeonPathFacts_PC34* facts,
    DM1_V1_StartupDungeonPathReceipt_PC34* out_receipt) {
    DM1_V1_StartupDungeonPathReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->game_id)) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    if (facts->source_kind ==
        DM1_V1_STARTUP_SOURCE_KIND_DIRECT_DUNGEON_PC34) {
        receipt.explicit_path_required = 1;
        if (!facts->explicit_dungeon_path ||
            facts->explicit_dungeon_path[0] == '\0') {
            *out_receipt = receipt;
            return 1;
        }
    }
    if (facts->explicit_dungeon_path &&
        facts->explicit_dungeon_path[0] != '\0') {
        receipt.use_explicit_path = 1;
        snprintf(receipt.explicit_dungeon_path,
                 sizeof(receipt.explicit_dungeon_path),
                 "%s",
                 facts->explicit_dungeon_path);
    } else {
        receipt.resolve_builtin_path = 1;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_handoff_prelude_plan_pc34(
    const char* game_id,
    DM1_V1_StartupHandoffPreludePlan_PC34* out_plan) {
    int required;
    int source_order_valid;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    required = dm1_v1_startup_source_visible_handoff_required_pc34(game_id);
    source_order_valid = dm1_v1_startup_sequence_source_order_valid_pc34();
    out_plan->required = required;
    out_plan->source_order_valid = source_order_valid;
    out_plan->play_swsh = required ? 1 : 0;
    out_plan->discard_presentation_after_swsh = required ? 1 : 0;
    out_plan->game_id = game_id;
    out_plan->failure_evidence =
        source_order_valid ? "" : dm1_v1_startup_sequence_source_evidence_pc34();
    return 1;
}

int dm1_v1_startup_handoff_post_launch_plan_pc34(
    const char* source_id,
    DM1_V1_StartupHandoffPostLaunchPlan_PC34* out_plan) {
    int required;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    required = dm1_v1_startup_source_visible_handoff_required_pc34(source_id);
    out_plan->required = required;
    out_plan->play_title = required ? 1 : 0;
    out_plan->play_entrance = required ? 1 : 0;
    out_plan->entrance_auto_enter_ms = required ? 1200 : 0;
    out_plan->source_id = source_id;
    return 1;
}

int dm1_v1_startup_execute_handoff_prelude_pc34(
    const char* game_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks) {
    DM1_V1_StartupHandoffPreludePlan_PC34 plan;

    if (!callbacks ||
        !dm1_v1_startup_handoff_prelude_plan_pc34(game_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        return 1;
    }
    /* ReDMCSB: APPA.C loads SWSH before TITLE/APPB, and STARTUP2.C later
     * calls F0437_STARTEND_DrawTitle before entrance processing.  Keep the
     * host-side calls behind this DM1-owned execution facade so M11 cannot
     * silently reorder the visible DM1 startup path. */
    if (!plan.source_order_valid &&
        callbacks->report_source_order_failure &&
        !callbacks->report_source_order_failure(callbacks->user,
                                                plan.failure_evidence)) {
        return 0;
    }
    if (plan.play_swsh) {
        if (!callbacks->raise_window || !callbacks->play_swsh) {
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            return 0;
        }
        if (!callbacks->play_swsh(callbacks->user, plan.game_id, 0)) {
            return 0;
        }
    }
    if (plan.discard_presentation_after_swsh &&
        (!callbacks->discard_presentation_texture ||
         !callbacks->discard_presentation_texture(callbacks->user))) {
        return 0;
    }
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    int* out_title_played,
    int* out_entrance_command) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 plan;
    int title_played = 0;
    int entrance_command = 0;

    if (out_title_played) {
        *out_title_played = 0;
    }
    if (out_entrance_command) {
        *out_entrance_command = 0;
    }
    if (!callbacks ||
        !dm1_v1_startup_handoff_post_launch_plan_pc34(source_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        return 1;
    }
    /* ReDMCSB STARTUP2.C: F0437_STARTEND_DrawTitle precedes the later
     * F0441_STARTEND_ProcessEntrance gate. */
    if (plan.play_title) {
        if (!callbacks->raise_window || !callbacks->play_title) {
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            return 0;
        }
        if (!callbacks->play_title(callbacks->user,
                                   plan.source_id,
                                   &title_played)) {
            return 0;
        }
    }
    if (plan.play_entrance) {
        if (!callbacks->play_entrance) {
            return 0;
        }
        if (!callbacks->play_entrance(callbacks->user,
                                      plan.source_id,
                                      plan.entrance_auto_enter_ms,
                                      &entrance_command)) {
            return 0;
        }
    }
    if (out_title_played) {
        *out_title_played = title_played;
    }
    if (out_entrance_command) {
        *out_entrance_command = entrance_command;
    }
    return 1;
}

int dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
    int entrance_command,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome) {
    if (!out_outcome) {
        return 0;
    }
    memset(out_outcome, 0, sizeof(*out_outcome));
    out_outcome->entrance_command = entrance_command;
    switch (entrance_command) {
        case ENTRANCE_COMPAT_COMMAND_PATH_ENTER:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34;
            out_outcome->status = "DM1 ENTER";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_RESUME:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34;
            out_outcome->status = "DM1 RESUME";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_QUIT:
            out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34;
            out_outcome->status = "DM1 QUIT";
            break;
        case ENTRANCE_COMPAT_COMMAND_PATH_NONE:
            out_outcome->action =
                DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34;
            out_outcome->status = "DM1 ENTRANCE SKIPPED";
            break;
        default:
            out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34;
            out_outcome->status = "DM1 HANDOFF NONE";
            break;
    }
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome) {
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 plan;
    int title_played = 0;
    int entrance_command = 0;

    if (!out_outcome) {
        return 0;
    }
    memset(out_outcome, 0, sizeof(*out_outcome));
    if (!dm1_v1_startup_handoff_post_launch_plan_pc34(source_id, &plan)) {
        return 0;
    }
    if (!plan.required) {
        out_outcome->action = DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34;
        out_outcome->status = "DM1 HANDOFF NONE";
        return 1;
    }
    if (!dm1_v1_startup_execute_handoff_post_launch_pc34(source_id,
                                                         callbacks,
                                                         &title_played,
                                                         &entrance_command)) {
        return 0;
    }
    if (!dm1_v1_startup_handoff_outcome_from_entrance_command_pc34(
            entrance_command,
            out_outcome)) {
        return 0;
    }
    out_outcome->title_played = title_played;
    return 1;
}

int dm1_v1_startup_apply_handoff_outcome_pc34(
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const char* source_id,
    const DM1_V1_StartupHostCallbacks_PC34* callbacks,
    DM1_V1_StartupHostApplyResult_PC34* out_result) {
    DM1_V1_StartupHostApplyResult_PC34 result;
    int used_backup = 0;

    if (!outcome || !out_result) {
        return 0;
    }
    memset(&result, 0, sizeof(result));
    if (outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34 ||
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34) {
        *out_result = result;
        return 1;
    }
    if (!callbacks) {
        return 0;
    }
    result.handled = 1;
    switch (outcome->action) {
        case DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34:
            result.quit_requested = 1;
            if (!callbacks->set_game_active ||
                !callbacks->set_game_active(callbacks->user, 0)) {
                return 0;
            }
            break;
        case DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34:
            result.resume_requested = 1;
            /* ReDMCSB COMMAND.C M566: RESUME loads the saved game.  Firestaff
             * keeps host path resolution/load I/O behind callbacks, while DM1
             * owns the decision that RESUME should attempt this path. */
            if (!callbacks->resolve_resume_save_path ||
                !callbacks->load_resume_save_path) {
                return 0;
            }
            if (callbacks->resolve_resume_save_path(callbacks->user,
                                                    source_id,
                                                    result.resume_path,
                                                    (int)sizeof(result.resume_path)) &&
                callbacks->load_resume_save_path(callbacks->user,
                                                 result.resume_path,
                                                 &used_backup)) {
                result.resume_loaded = 1;
                result.resume_used_backup = used_backup;
                if (callbacks->set_game_active &&
                    !callbacks->set_game_active(callbacks->user, 1)) {
                    return 0;
                }
                if (callbacks->log_resume_loaded &&
                    !callbacks->log_resume_loaded(callbacks->user,
                                                  result.resume_path,
                                                  used_backup)) {
                    return 0;
                }
            } else if (callbacks->log_resume_missing &&
                       !callbacks->log_resume_missing(
                           callbacks->user,
                           result.resume_path[0] ? result.resume_path
                                                 : "(unresolved)")) {
                return 0;
            }
            break;
        case DM1_V1_STARTUP_HANDOFF_ACTION_SKIPPED_NONFATAL_PC34:
            if (callbacks->log_entrance_skipped &&
                !callbacks->log_entrance_skipped(callbacks->user)) {
                return 0;
            }
            break;
        default:
            result.handled = 0;
            break;
    }
    *out_result = result;
    return 1;
}

int dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffCallbacks_PC34* handoff_callbacks,
    const DM1_V1_StartupHostCallbacks_PC34* host_callbacks,
    DM1_V1_StartupHandoffOutcome_PC34* out_outcome,
    DM1_V1_StartupHostApplyResult_PC34* out_result) {
    DM1_V1_StartupHandoffOutcome_PC34 local_outcome;
    DM1_V1_StartupHostApplyResult_PC34 local_result;

    if (!out_outcome || !out_result) {
        return 0;
    }
    memset(&local_outcome, 0, sizeof(local_outcome));
    memset(&local_result, 0, sizeof(local_result));
    if (!dm1_v1_startup_execute_handoff_post_launch_outcome_pc34(
            source_id,
            handoff_callbacks,
            &local_outcome)) {
        return 0;
    }
    if (!dm1_v1_startup_apply_handoff_outcome_pc34(&local_outcome,
                                                   source_id,
                                                   host_callbacks,
                                                   &local_result)) {
        return 0;
    }
    *out_outcome = local_outcome;
    *out_result = local_result;
    return 1;
}

int dm1_v1_startup_execute_selected_launch_transaction_pc34(
    const char* selected_game_id,
    const DM1_V1_StartupSelectedLaunchCallbacks_PC34* callbacks,
    DM1_V1_StartupSelectedLaunchResult_PC34* out_result) {
    DM1_V1_StartupSelectedLaunchResult_PC34 result;
    char opened_source_id[64];

    if (!out_result) {
        return 0;
    }
    memset(&result, 0, sizeof(result));
    memset(opened_source_id, 0, sizeof(opened_source_id));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(selected_game_id)) {
        *out_result = result;
        return 1;
    }
    if (!callbacks ||
        !callbacks->handoff_callbacks ||
        !callbacks->host_callbacks ||
        !callbacks->open_selected_entry ||
        !callbacks->after_open) {
        return 0;
    }
    result.handled = 1;
    /* ReDMCSB source handoff chain: SWSH.C runs START.PRG, STARTUP1.C enters
     * TITLE.C F0437, then ENTRANCE.C F0441 handles the entrance command.  This
     * transaction keeps that sequence on the DM1 side of the M11 boundary. */
    if (!dm1_v1_startup_execute_handoff_prelude_pc34(
            selected_game_id,
            callbacks->handoff_callbacks)) {
        return 0;
    }
    if (!callbacks->open_selected_entry(callbacks->user,
                                        opened_source_id,
                                        (int)sizeof(opened_source_id))) {
        result.launch_failed = 1;
        if (callbacks->mark_launch_failed &&
            !callbacks->mark_launch_failed(callbacks->user)) {
            return 0;
        }
        *out_result = result;
        return 1;
    }
    result.opened = 1;
    if (!callbacks->after_open(callbacks->user)) {
        return 0;
    }
    if (!dm1_v1_startup_execute_handoff_post_launch_and_apply_pc34(
            opened_source_id,
            callbacks->handoff_callbacks,
            callbacks->host_callbacks,
            &result.handoff_outcome,
            &result.host_apply_result)) {
        return 0;
    }
    if (!result.host_apply_result.quit_requested &&
        callbacks->draw_opened &&
        !callbacks->draw_opened(callbacks->user)) {
        return 0;
    }
    *out_result = result;
    return 1;
}

int dm1_v1_startup_receipt_phase_pc34(int level_loaded,
                                      int intro_bypassed,
                                      char* out_phase,
                                      int out_phase_size) {
    const char* phase;

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    if (!level_loaded) {
        phase = "dm1-loading";
    } else {
        phase = intro_bypassed
            ? "dm1-runtime-direct"
            : "dm1-runtime";
    }
    snprintf(out_phase, (size_t)out_phase_size, "%s", phase);
    return 1;
}

int dm1_v1_startup_boot_probe_receipt_pc34(int level_loaded,
                                           int intro_bypassed,
                                           char* out_phase,
                                           int out_phase_size,
                                           int* out_startup_active,
                                           char* out_animation,
                                           int out_animation_size,
                                           int* out_animation_active,
                                           int* out_title_frame,
                                           int* out_title_frame_max,
                                           int* out_title_ready) {
    const char* animation;

    if (!out_phase || out_phase_size <= 0 ||
        !out_startup_active ||
        !out_animation || out_animation_size <= 0 ||
        !out_animation_active ||
        !out_title_frame ||
        !out_title_frame_max ||
        !out_title_ready) {
        return 0;
    }
    if (!dm1_v1_startup_receipt_phase_pc34(level_loaded,
                                           intro_bypassed,
                                           out_phase,
                                           out_phase_size)) {
        return 0;
    }
    /* ReDMCSB: SWSH.C -> STARTUP1.C -> TITLE.C F0437 -> ENTRANCE.C.
     * M11 reaches this receipt after the source-visible launcher/title
     * path has completed or through the explicit direct-view bypass. */
    animation = intro_bypassed ? "dm1-title-bypassed" : "dm1-title";
    snprintf(out_animation, (size_t)out_animation_size, "%s", animation);
    *out_startup_active = 0;
    *out_animation_active = 0;
    *out_title_frame = V1_TITLE_DAT_FRAME_MAX;
    *out_title_frame_max = V1_TITLE_DAT_FRAME_MAX;
    *out_title_ready = 1;
    return 1;
}

int dm1_v1_startup_boot_probe_receipt_from_facts_pc34(
    const DM1_V1_StartupBootProbeFacts_PC34* facts,
    DM1_V1_StartupBootProbeReceipt_PC34* out_receipt) {
    DM1_V1_StartupBootProbeReceipt_PC34 receipt;
    int intro_bypassed;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(facts->source_id)) {
        *out_receipt = receipt;
        return 1;
    }
    intro_bypassed = dm1_v1_startup_intro_bypass_applies_to_source_pc34(
        facts->source_id,
        facts->intro_bypassed);
    receipt.handled = 1;
    snprintf(receipt.source_id,
             sizeof(receipt.source_id),
             "%s",
             facts->source_id ? facts->source_id : "");
    receipt.dm1_startup_intro_bypassed = intro_bypassed;
    receipt.level_loaded = facts->level_loaded;
    receipt.map_index = facts->map_index;
    receipt.party_x = facts->party_x;
    receipt.party_y = facts->party_y;
    receipt.party_dir = facts->party_dir;
    receipt.champion_count = facts->champion_count;
    receipt.runtime_tick = facts->runtime_tick;
    receipt.world_tick = facts->world_tick;
    if (!dm1_v1_startup_boot_probe_receipt_pc34(
            facts->level_loaded,
            intro_bypassed,
            receipt.startup_phase,
            (int)sizeof(receipt.startup_phase),
            &receipt.startup_active,
            receipt.startup_animation,
            (int)sizeof(receipt.startup_animation),
            &receipt.startup_animation_active,
            &receipt.startup_title_frame,
            &receipt.startup_title_frame_max,
            &receipt.startup_title_ready)) {
        return 0;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_sequence_source_order_valid_pc34(void) {
    /* ReDMCSB startup source order:
     * SWSH.C:39-47 runs START.PRG after the FTL palette program;
     * STARTUP1.C:143 calls TITLE.C F0437_STARTEND_DrawTitle();
     * TITLE.C:319-409 draws PRESENTS, title zoom, STRIKES BACK and final
     * guard; ENTRANCE.C:850-883 then enters the entrance wait loop.
     */
    return dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_LOGO_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34,
                                           DM1_V1_STARTUP_STAGE_SWSH_RUN_START_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_BEGIN_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34,
                                           DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34) &&
           dm1_v1_startup_stage_after_pc34(DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34,
                                           DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34);
}

const char* dm1_v1_startup_sequence_source_evidence_pc34(void) {
    return "ReDMCSB SWSH.C:39-47 -> STARTUP1.C:143 -> TITLE.C:319-409 -> ENTRANCE.C:850-883";
}

unsigned int dm1_v1_startup_title_zoom_steps_pc34(void) {
    /* ReDMCSB: TITLE.C F0437 lines 340-360 prepares 18 shrinked title
     * bitmaps, then lines 385-387 blit them in reverse order. */
    return DM1_V1_STARTUP_TITLE_ZOOM_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_source_animation_steps_pc34(void) {
    /* PRESENTS + 18 zoom blits + 2 post-zoom waits + STRIKES BACK + final
     * guard. This is the source event count before Firestaff's frame-bank
     * cadence padding. */
    return DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_frame_bank_equivalent_steps_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FRAME_BANK_EQUIVALENT_STEPS_PC34 -
           DM1_V1_STARTUP_TITLE_SOURCE_ANIMATION_STEPS_PC34;
}

unsigned int dm1_v1_startup_title_vblank_tick_ms_pc34(void) {
    return DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_presents_hold_ms_pc34(void) {
    return dm1_v1_startup_title_presents_hold_vblanks_pc34() *
           DM1_V1_STARTUP_TITLE_VBLANK_TICK_MS_PC34;
}

unsigned int dm1_v1_startup_title_post_zoom_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_POST_ZOOM_VBLANKS_PC34;
}

unsigned int dm1_v1_startup_title_final_guard_vblanks_pc34(void) {
    return DM1_V1_STARTUP_TITLE_FINAL_GUARD_VBLANKS_PC34;
}
