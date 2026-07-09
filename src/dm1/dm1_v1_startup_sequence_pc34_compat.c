#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "entrance_frontend_pc34_compat.h"
#include "swsh_frontend_pc34_compat.h"
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

int dm1_v1_startup_graphics_bind_receipt_pc34(
    const DM1_V1_StartupGraphicsBindFacts_PC34* facts,
    DM1_V1_StartupGraphicsBindReceipt_PC34* out_receipt) {
    DM1_V1_StartupGraphicsBindReceipt_PC34 receipt;
    size_t dungeon_len;
    size_t slash_pos;

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
    if (!facts->dungeon_path || facts->dungeon_path[0] == '\0') {
        *out_receipt = receipt;
        return 1;
    }

    dungeon_len = strlen(facts->dungeon_path);
    slash_pos = dungeon_len;
    while (slash_pos > 0 &&
           facts->dungeon_path[slash_pos - 1] != '/' &&
           facts->dungeon_path[slash_pos - 1] != '\\') {
        --slash_pos;
    }
    if (slash_pos > 0 &&
        slash_pos + 13 < sizeof(receipt.graphics_dat_path)) {
        memcpy(receipt.graphics_dat_path,
               facts->dungeon_path,
               slash_pos);
        memcpy(receipt.graphics_dat_path + slash_pos,
               "GRAPHICS.DAT",
               13);
        receipt.bind_graphics_dat = 1;
    }
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_dungeon_load_receipt_pc34(
    const DM1_V1_StartupDungeonLoadFacts_PC34* facts,
    DM1_V1_StartupDungeonLoadReceipt_PC34* out_receipt) {
    DM1_V1_StartupDungeonLoadReceipt_PC34 receipt;

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
    receipt.load_succeeded = facts->load_succeeded ? 1 : 0;
    snprintf(receipt.status_title,
             sizeof(receipt.status_title),
             "%s",
             "BOOT");
    snprintf(receipt.status_detail,
             sizeof(receipt.status_detail),
             "%s",
             receipt.load_succeeded ? "GAME DATA LOADED"
                                    : "FAILED TO LOAD DUNGEON.DAT");
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_runtime_ready_receipt_pc34(
    const DM1_V1_StartupRuntimeReadyFacts_PC34* facts,
    DM1_V1_StartupRuntimeReadyReceipt_PC34* out_receipt) {
    DM1_V1_StartupRuntimeReadyReceipt_PC34 receipt;
    DM1_V1_StartupDungeonLoadFacts_PC34 load_facts;
    DM1_V1_StartupGraphicsBindFacts_PC34 graphics_facts;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->runtime_start.game_id)) {
        *out_receipt = receipt;
        return 1;
    }

    memset(&load_facts, 0, sizeof(load_facts));
    load_facts.game_id = facts->runtime_start.game_id;
    load_facts.load_succeeded = facts->load_succeeded;
    if (!dm1_v1_startup_dungeon_load_receipt_pc34(
            &load_facts,
            &receipt.load_receipt) ||
        !receipt.load_receipt.handled ||
        !receipt.load_receipt.load_succeeded) {
        return 0;
    }

    if (!dm1_v1_startup_runtime_start_receipt_pc34(
            &facts->runtime_start,
            &receipt.runtime_start_receipt) ||
        !receipt.runtime_start_receipt.handled) {
        return 0;
    }

    memset(&graphics_facts, 0, sizeof(graphics_facts));
    graphics_facts.game_id = facts->runtime_start.game_id;
    graphics_facts.dungeon_path = facts->runtime_start.dungeon_path;
    if (!dm1_v1_startup_graphics_bind_receipt_pc34(
            &graphics_facts,
            &receipt.graphics_bind_receipt) ||
        !receipt.graphics_bind_receipt.handled) {
        return 0;
    }

    receipt.handled = 1;
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
    if (required &&
        !dm1_v1_startup_full_graphics_media_receipt_pc34(
            game_id,
            &out_plan->media_receipt)) {
        return 0;
    }
    out_plan->play_swsh =
        (required && out_plan->media_receipt.play_swsh) ? 1 : 0;
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
    DM1_V1_StartupTitleMenuEligibilityFacts_PC34 title_facts;
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 title_receipt;
    DM1_V1_EntranceCtxPc34 entrance_ctx;

    if (!out_plan) {
        return 0;
    }
    memset(out_plan, 0, sizeof(*out_plan));
    required = dm1_v1_startup_source_visible_handoff_required_pc34(source_id);
    out_plan->required = required;
    out_plan->source_id = source_id;
    if (required) {
        if (!dm1_v1_startup_full_graphics_media_receipt_pc34(
                source_id,
                &out_plan->media_receipt)) {
            return 0;
        }
        out_plan->play_title = out_plan->media_receipt.play_title ? 1 : 0;
        out_plan->play_entrance =
            out_plan->media_receipt.play_entrance ? 1 : 0;
        out_plan->entrance_auto_enter_ms =
            out_plan->media_receipt.entrance_auto_enter_ms;
        memset(&title_facts, 0, sizeof(title_facts));
        memset(&title_receipt, 0, sizeof(title_receipt));
        title_facts.title_frame =
            out_plan->media_receipt.title_menu_boundary_frame;
        title_facts.title_frame_max =
            out_plan->media_receipt.title_frame_bank_equivalent_steps;
        title_facts.advance_requested = 1;
        title_facts.title_handoff_ready = 1;
        if (!dm1_v1_startup_title_menu_eligibility_receipt_pc34(
                &title_facts,
                &title_receipt)) {
            return 0;
        }
        DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
        if (!DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(
                &entrance_ctx,
                &out_plan->entrance_full_start_receipt)) {
            return 0;
        }
        out_plan->title_menu_boundary_frame =
            (int)out_plan->media_receipt.title_menu_boundary_frame;
        out_plan->title_menu_eligible = title_receipt.menu_eligible;
        out_plan->title_keep_surface = title_receipt.keep_title_surface;
        out_plan->title_consume_pending_input =
            title_receipt.consume_pending_input;
        out_plan->title_next_stage = title_receipt.next_stage;
        out_plan->title_menu_reason = title_receipt.reason;
        out_plan->entrance_wait_stage = DM1_V1_STARTUP_STAGE_ENTRANCE_WAIT_PC34;
        /* ReDMCSB TITLE.C F0437:319-409 finishes PRESENTS/title/guard before
         * ENTRANCE.C F0441:850-883 discards input and waits on entrance.
         * F0797:68-80 builds the C255 5x5 entrance micro-dungeon that M11/M12
         * should consume from this DM1-owned startup plan. */
    }
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
    if (callbacks->begin_prelude_plan &&
        !callbacks->begin_prelude_plan(callbacks->user, &plan)) {
        return 0;
    }
    /* ReDMCSB: APPA.C loads SWSH before TITLE/APPB, and STARTUP2.C later
     * calls F0437_STARTEND_DrawTitle before entrance processing.  Keep the
     * host-side calls behind this DM1-owned execution facade so M11 cannot
     * silently reorder the visible DM1 startup path. */
    if (!plan.source_order_valid &&
        callbacks->report_source_order_failure &&
        !callbacks->report_source_order_failure(callbacks->user,
                                                plan.failure_evidence)) {
        if (callbacks->end_prelude_plan) {
            (void)callbacks->end_prelude_plan(callbacks->user);
        }
        return 0;
    }
    if (plan.play_swsh) {
        if (!callbacks->raise_window || !callbacks->play_swsh) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_swsh(callbacks->user, plan.game_id, 0)) {
            if (callbacks->end_prelude_plan) {
                (void)callbacks->end_prelude_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (plan.discard_presentation_after_swsh &&
        (!callbacks->discard_presentation_texture ||
         !callbacks->discard_presentation_texture(callbacks->user))) {
        if (callbacks->end_prelude_plan) {
            (void)callbacks->end_prelude_plan(callbacks->user);
        }
        return 0;
    }
    if (callbacks->end_prelude_plan &&
        !callbacks->end_prelude_plan(callbacks->user)) {
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
    if (callbacks->begin_post_launch_plan &&
        !callbacks->begin_post_launch_plan(callbacks->user, &plan)) {
        return 0;
    }
    /* ReDMCSB STARTUP2.C: F0437_STARTEND_DrawTitle precedes the later
     * F0441_STARTEND_ProcessEntrance gate. */
    if (plan.play_title) {
        if (!callbacks->raise_window || !callbacks->play_title) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->raise_window(callbacks->user)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_title(callbacks->user,
                                   plan.source_id,
                                   &title_played)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (plan.play_entrance) {
        if (!callbacks->play_entrance) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
        if (!callbacks->play_entrance(callbacks->user,
                                      plan.source_id,
                                      plan.entrance_auto_enter_ms,
                                      &entrance_command)) {
            if (callbacks->end_post_launch_plan) {
                (void)callbacks->end_post_launch_plan(callbacks->user);
            }
            return 0;
        }
    }
    if (callbacks->end_post_launch_plan &&
        !callbacks->end_post_launch_plan(callbacks->user)) {
        return 0;
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

int dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
    const char* selected_game_id,
    const char* opened_source_id,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    const DM1_V1_StartupHostApplyResult_PC34* host_result,
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34* out_receipt) {
    DM1_V1_StartupFullGraphicsRuntimeHandoffReceipt_PC34 receipt;
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 media;
    DM1_V1_EntranceCtxPc34 entrance_ctx;

    if (!out_receipt || !outcome || !host_result) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    memset(&media, 0, sizeof(media));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            selected_game_id)) {
        *out_receipt = receipt;
        return 1;
    }
    if (!dm1_v1_startup_full_graphics_media_receipt_pc34(opened_source_id,
                                                         &media)) {
        return 0;
    }

    /* ReDMCSB source order:
     * SWSH.C runs START.PRG, TITLE.C F0437 lines 319-409 completes
     * PRESENTS/title/guard, and ENTRANCE.C F0441 lines 850-883 returns an
     * entrance command before the dungeon/HoC runtime is redrawn.  This
     * receipt is the DM1-owned boundary from full-graphics startup media to
     * the live Hall of Champions/runtime frame. */
    receipt.handled = 1;
    receipt.full_graphics_required = media.handled ? 1 : 0;
    receipt.swsh_consumed = media.play_swsh ? 1 : 0;
    receipt.title_consumed =
        (media.play_title && outcome->title_played) ? 1 : 0;
    receipt.entrance_consumed =
        (media.play_entrance &&
         outcome->action != DM1_V1_STARTUP_HANDOFF_ACTION_NONE_PC34)
            ? 1
            : 0;
    receipt.full_graphics_consumed =
        receipt.full_graphics_required &&
        receipt.swsh_consumed &&
        receipt.title_consumed &&
        receipt.entrance_consumed;
    receipt.entrance_command = outcome->entrance_command;
    receipt.action = outcome->action;
    receipt.status = outcome->status;
    receipt.return_to_launcher =
        (outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_QUIT_PC34 ||
         host_result->quit_requested)
            ? 1
            : 0;
    receipt.hoc_runtime_ready =
        receipt.full_graphics_consumed &&
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34 &&
        !receipt.return_to_launcher;
    receipt.resumed_runtime_ready =
        receipt.full_graphics_consumed &&
        outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_RESUME_GAME_PC34 &&
        host_result->resume_loaded &&
        !receipt.return_to_launcher;
    if (receipt.hoc_runtime_ready) {
        DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
        entrance_ctx.state = DM1_ENTRANCE_VIEWING;
        if (!DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
                &entrance_ctx,
                &receipt.champion_mirror_startup_route)) {
            return 0;
        }
        /* ReDMCSB ENTRANCE.C F0441 lines 850-883 enters the Hall of
         * Champions route before mirror selection.  REVIVE.C F0280 later owns
         * the candidate champion route after a mirror is selected.  The first
         * runtime frame must therefore start from the DM1-owned Hall route,
         * not from stale entrance/title host state. */
        receipt.champion_mirror_startup_handoff_ready =
            (receipt.champion_mirror_startup_route.handled &&
             receipt.champion_mirror_startup_route.route ==
                 DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
             receipt.champion_mirror_startup_route.showHall &&
             receipt.champion_mirror_startup_route.needsRedraw)
                ? 1
                : 0;
        receipt.champion_mirror_startup_input_ready =
            (receipt.champion_mirror_startup_handoff_ready &&
             receipt.champion_mirror_startup_route.state ==
                 DM1_ENTRANCE_VIEWING &&
             receipt.champion_mirror_startup_route.selectedMirrorIndex < 0)
                ? 1
                : 0;
        receipt.champion_mirror_startup_panel_clear =
            (receipt.champion_mirror_startup_handoff_ready &&
             !receipt.champion_mirror_startup_route.showChampionPanel &&
             !receipt.champion_mirror_startup_route
                  .showResurrectReincarnateChoices)
                ? 1
                : 0;
        receipt.champion_mirror_startup_blocks_enter =
            (receipt.champion_mirror_startup_handoff_ready &&
             receipt.champion_mirror_startup_route.partyChampionCount == 0 &&
             !receipt.champion_mirror_startup_route.canEnterDungeon)
                ? 1
                : 0;
        receipt.champion_mirror_startup_overlay_command_count =
            receipt.champion_mirror_startup_route.renderOverlayCommandCount;
        if (receipt.champion_mirror_startup_overlay_command_count > 0 &&
            receipt.champion_mirror_startup_overlay_command_count <=
                DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34) {
            int i;
            for (i = 0; i < receipt.champion_mirror_startup_overlay_command_count;
                 ++i) {
                receipt.champion_mirror_startup_overlay_commands[i] =
                    receipt.champion_mirror_startup_route.renderOverlayCommands[i];
            }
            receipt.champion_mirror_startup_overlay_commands_ready =
                (receipt.champion_mirror_startup_overlay_commands[0].valid &&
                 receipt.champion_mirror_startup_overlay_commands[0].kind ==
                     DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .clearStalePanelFirst &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .suppressThingPayloads &&
                 receipt.champion_mirror_startup_overlay_commands[0]
                     .blockEnterUntilChampionSelected)
                    ? 1
                    : 0;
        }
    }
    receipt.hoc_first_frame_ready =
        receipt.hoc_runtime_ready &&
        receipt.champion_mirror_startup_handoff_ready &&
        receipt.champion_mirror_startup_input_ready &&
        receipt.champion_mirror_startup_panel_clear &&
        receipt.champion_mirror_startup_blocks_enter &&
        receipt.champion_mirror_startup_overlay_commands_ready;
    receipt.runtime_first_frame_ready =
        (receipt.hoc_first_frame_ready || receipt.resumed_runtime_ready) ? 1 : 0;
    receipt.draw_opened_runtime =
        receipt.runtime_first_frame_ready ? 1 : 0;
    receipt.suppress_draw_opened = receipt.draw_opened_runtime ? 0 : 1;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_first_frame_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFirstFrameReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFirstFrameReceipt_PC34 receipt;
    DM1_V1_EntranceCtxPc34 entrance_ctx;

    if (!out_receipt || !post_plan || !outcome) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.full_graphics_required = 1;
    receipt.title_surface_released =
        (post_plan->required &&
         post_plan->play_title &&
         post_plan->title_menu_eligible &&
         !post_plan->title_keep_surface &&
         post_plan->title_consume_pending_input)
            ? 1
            : 0;
    receipt.entrance_wait_consumed =
        (post_plan->play_entrance &&
         outcome->action == DM1_V1_STARTUP_HANDOFF_ACTION_ENTER_GAME_PC34)
            ? 1
            : 0;

    DM1_V1_Entrance_InitPc34Compat(&entrance_ctx);
    entrance_ctx.state = DM1_ENTRANCE_VIEWING;
    entrance_ctx.doorAnim.complete = 1;
    entrance_ctx.doorAnim.animationStep = entrance_ctx.doorAnim.totalSteps - 1;
    if (!DM1_V1_Entrance_BuildFullStartRenderReceiptPc34Compat(
            &entrance_ctx,
            &receipt.entrance_full_start_receipt) ||
        !DM1_V1_Entrance_BuildMenuRouteReceiptPc34Compat(
            &entrance_ctx,
            &receipt.champion_select_route)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437:319-409 releases the title surface only after
     * PRESENTS/title/guard. ENTRANCE.C F0441:850-883 waits for a fresh
     * entrance command, and F0797:68-80 builds the C255 5x5 entrance map.
     * The first HoC frame is therefore the entrance VIEWING state with hall
     * mirror UI, not a host fallback title/door frame. */
    receipt.full_start_render_ready =
        receipt.entrance_full_start_receipt.valid ? 1 : 0;
    receipt.entrance_map_ready =
        (receipt.entrance_full_start_receipt.mapIndex ==
             DM1_V1_ENTRANCE_MAP_INDEX_PC34 &&
         receipt.entrance_full_start_receipt.width ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 &&
         receipt.entrance_full_start_receipt.height ==
             DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 &&
         receipt.entrance_full_start_receipt.corridorCount == 6)
            ? 1
            : 0;
    receipt.entrance_music_requested =
        receipt.entrance_full_start_receipt.entranceMusicRequested ? 1 : 0;
    receipt.entrance_door_open_frame_ready =
        (receipt.entrance_full_start_receipt.drawDoorFrame &&
         receipt.entrance_full_start_receipt.doorFrameIndex ==
             entrance_ctx.doorAnim.totalSteps - 1)
            ? 1
            : 0;
    receipt.hoc_menu_route_ready =
        (receipt.champion_select_route.handled &&
         receipt.champion_select_route.route ==
             DM1_V1_ENTRANCE_MENU_ROUTE_HALL_PC34 &&
         receipt.champion_select_route.state == DM1_ENTRANCE_VIEWING &&
         receipt.champion_select_route.selectedMirrorIndex < 0)
            ? 1
            : 0;
    receipt.champion_select_ui_ready =
        (receipt.hoc_menu_route_ready &&
         receipt.champion_select_route.showHall &&
         receipt.champion_select_route.needsRedraw)
            ? 1
            : 0;
    receipt.render_hall_mirrors =
        receipt.champion_select_route.renderHallMirrorOverlay ? 1 : 0;
    receipt.render_overlay_command_count =
        receipt.champion_select_route.renderOverlayCommandCount;
    if (receipt.render_overlay_command_count > 0 &&
        receipt.render_overlay_command_count <=
            DM1_V1_ENTRANCE_OVERLAY_COMMAND_MAX_PC34) {
        int i;
        for (i = 0; i < receipt.render_overlay_command_count; ++i) {
            receipt.render_overlay_commands[i] =
                receipt.champion_select_route.renderOverlayCommands[i];
        }
        receipt.render_overlay_commands_ready =
            (receipt.render_overlay_commands[0].valid &&
             receipt.render_overlay_commands[0].kind ==
                 DM1_V1_ENTRANCE_OVERLAY_HALL_MIRRORS_PC34 &&
             receipt.render_overlay_commands[0].clearStalePanelFirst &&
             receipt.render_overlay_commands[0].suppressThingPayloads &&
             receipt.render_overlay_commands[0].blockEnterUntilChampionSelected)
                ? 1
                : 0;
    }
    receipt.clear_stale_champion_panel =
        receipt.champion_select_route.clearStaleChampionMirrorOverlay ? 1 : 0;
    receipt.block_enter_until_champion_selected =
        receipt.champion_select_route.blockEnterUntilChampionSelected ? 1 : 0;
    receipt.runtime_first_frame_ready =
        receipt.title_surface_released &&
        receipt.entrance_wait_consumed &&
        receipt.full_start_render_ready &&
        receipt.entrance_map_ready &&
        receipt.entrance_music_requested &&
        receipt.entrance_door_open_frame_ready &&
        receipt.champion_select_ui_ready &&
        receipt.render_hall_mirrors &&
        receipt.render_overlay_commands_ready &&
        receipt.clear_stale_champion_panel &&
        receipt.block_enter_until_champion_selected;
    receipt.suppress_host_fallback_visuals =
        receipt.runtime_first_frame_ready ? 1 : 0;
    if (receipt.runtime_first_frame_ready) {
        DM1_V1_StartupHoCRenderCommand_PC34* command;

        /* ReDMCSB ENTRANCE.C F0797 draws the C255 micro-dungeon behind the
         * fully opened doors, then ENTRANCE.C F0441 leaves the Hall route in
         * VIEWING state.  Expose the exact first-frame draw plan here so the
         * host cannot fall back to stale TITLE/door/panel visuals. */
        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind =
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34;
        command->map_index = receipt.entrance_full_start_receipt.mapIndex;
        command->door_frame_index =
            receipt.entrance_full_start_receipt.doorFrameIndex;
        command->suppress_host_fallback_visuals = 1;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:68-80 F0797 C255 entrance draw";

        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind = DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34;
        command->clear_stale_panel_first = 1;
        command->suppress_host_fallback_visuals = 1;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:850-883 starts Hall before mirror selection";

        command = &receipt.hoc_render_commands[receipt.hoc_render_command_count++];
        command->valid = 1;
        command->kind = DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34;
        command->overlay_kind = receipt.render_overlay_commands[0].kind;
        command->overlay_command_index = 0;
        command->clear_stale_panel_first =
            receipt.render_overlay_commands[0].clearStalePanelFirst;
        command->suppress_host_fallback_visuals = 1;
        command->block_enter_until_champion_selected =
            receipt.render_overlay_commands[0].blockEnterUntilChampionSelected;
        command->source_evidence =
            "ReDMCSB ENTRANCE.C:850-883 Hall waits for champion choice";
    }
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:850-883; ENTRANCE.C:68-80";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
    const DM1_V1_StartupHoCFirstFrameReceipt_PC34* receipt,
    DM1_V1_StartupHoCHostRenderPlan_PC34* out_plan) {
    DM1_V1_StartupHoCHostRenderPlan_PC34 plan;
    const DM1_V1_StartupHoCRenderCommand_PC34* entrance_command;
    const DM1_V1_StartupHoCRenderCommand_PC34* clear_command;
    const DM1_V1_StartupHoCRenderCommand_PC34* mirror_command;

    if (!receipt || !out_plan) {
        return 0;
    }
    memset(&plan, 0, sizeof(plan));
    if (!receipt->handled) {
        *out_plan = plan;
        return 1;
    }
    plan.handled = 1;
    plan.command_count = receipt->hoc_render_command_count;
    plan.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!receipt->runtime_first_frame_ready ||
        receipt->hoc_render_command_count != 3) {
        *out_plan = plan;
        return 1;
    }

    entrance_command = &receipt->hoc_render_commands[0];
    clear_command = &receipt->hoc_render_commands[1];
    mirror_command = &receipt->hoc_render_commands[2];
    /* ReDMCSB source order gives the host one legal first HoC frame:
     * TITLE.C has released the title surface, ENTRANCE.C F0797 has drawn the
     * opened C255 entrance view, and F0441 is waiting in the Hall before any
     * mirror/C040 champion panel exists.  Collapse the DM1 render commands
     * into a host-ready plan so M11/M12 do not infer from loose fields. */
    if (!entrance_command->valid ||
        entrance_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_ENTRANCE_OPEN_FRAME_PC34 ||
        !clear_command->valid ||
        clear_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_CLEAR_CHAMPION_PANEL_PC34 ||
        !mirror_command->valid ||
        mirror_command->kind !=
            DM1_V1_STARTUP_HOC_RENDER_COMMAND_HALL_MIRRORS_PC34) {
        *out_plan = plan;
        return 1;
    }

    plan.ready = 1;
    plan.consume_dm1_receipt_only = 1;
    plan.draw_opened_entrance_frame = 1;
    plan.entrance_map_index = entrance_command->map_index;
    plan.entrance_door_frame_index = entrance_command->door_frame_index;
    plan.clear_champion_panel = clear_command->clear_stale_panel_first;
    plan.render_hall_mirror_overlay = 1;
    plan.hall_mirror_overlay_kind = mirror_command->overlay_kind;
    plan.suppress_host_fallback_visuals =
        entrance_command->suppress_host_fallback_visuals &&
        clear_command->suppress_host_fallback_visuals &&
        mirror_command->suppress_host_fallback_visuals;
    plan.block_enter_until_champion_selected =
        mirror_command->block_enter_until_champion_selected;
    *out_plan = plan;
    return 1;
}

int dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
    const DM1_V1_StartupHoCHostRenderPlan_PC34* plan,
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* out_proof) {
    DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34 proof;

    if (!plan || !out_proof) {
        return 0;
    }
    memset(&proof, 0, sizeof(proof));
    if (!plan->handled) {
        *out_proof = proof;
        return 1;
    }

    proof.handled = 1;
    proof.capture_required = 1;
    proof.command_count = plan->command_count;
    proof.capture_phase = "dm1-v1-hoc-first-frame-full-graphics";
    proof.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!plan->ready || !plan->consume_dm1_receipt_only) {
        *out_proof = proof;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 has already released the title surface, while
     * ENTRANCE.C F0797 and F0441 leave the opened C255 entrance/Hall state.
     * Package builds can consume this proof receipt directly instead of
     * re-inferring first-frame capture rules in the host. */
    proof.ready = 1;
    proof.consume_host_render_plan_only = 1;
    proof.packaged_full_graphics_proof_ready = 1;
    proof.expected_map_index = plan->entrance_map_index;
    proof.expected_map_width = DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34;
    proof.expected_map_height = DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34;
    proof.expected_entrance_door_frame_index =
        plan->entrance_door_frame_index;
    proof.expected_hall_overlay_kind = plan->hall_mirror_overlay_kind;
    proof.require_opened_entrance_frame = plan->draw_opened_entrance_frame;
    proof.require_clear_champion_panel = plan->clear_champion_panel;
    proof.require_hall_mirror_overlay = plan->render_hall_mirror_overlay;
    proof.require_no_title_surface = 1;
    proof.require_no_closed_door_frame = 1;
    proof.require_no_host_fallback_visuals =
        plan->suppress_host_fallback_visuals;
    proof.block_enter_until_champion_selected =
        plan->block_enter_until_champion_selected;
    *out_proof = proof;
    return 1;
}

int dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
    const DM1_V1_StartupHoCPackagedFullGraphicsProof_PC34* proof,
    DM1_V1_StartupHoCProductionFullStartHook_PC34* out_hook) {
    DM1_V1_StartupHoCProductionFullStartHook_PC34 hook;

    if (!proof || !out_hook) {
        return 0;
    }
    memset(&hook, 0, sizeof(hook));
    if (!proof->handled) {
        *out_hook = hook;
        return 1;
    }

    hook.handled = 1;
    hook.capture_phase = proof->capture_phase;
    hook.source_evidence =
        "ReDMCSB TITLE.C:385-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!proof->ready || !proof->consume_host_render_plan_only ||
        !proof->packaged_full_graphics_proof_ready ||
        !proof->capture_required ||
        proof->expected_map_index != DM1_V1_ENTRANCE_MAP_INDEX_PC34 ||
        proof->expected_map_width != DM1_V1_ENTRANCE_MICRO_DUNGEON_WIDTH_PC34 ||
        proof->expected_map_height != DM1_V1_ENTRANCE_MICRO_DUNGEON_HEIGHT_PC34 ||
        !proof->require_opened_entrance_frame ||
        !proof->require_clear_champion_panel ||
        !proof->require_hall_mirror_overlay ||
        !proof->require_no_title_surface ||
        !proof->require_no_closed_door_frame ||
        !proof->require_no_host_fallback_visuals) {
        *out_hook = hook;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 finishes/freezes title work before ENTRANCE.C
     * F0797/F0441 enters Hall.  This is the production hook consumed by
     * Firestaff packaging/capture: render the DM1-owned HoC first frame, then
     * capture and publish proof, before accepting Hall input. */
    hook.ready = 1;
    hook.consume_dm1_startup_receipts_only = 1;
    hook.run_before_hoc_input = 1;
    hook.draw_opened_entrance_frame = 1;
    hook.clear_champion_panel = 1;
    hook.render_hall_mirror_overlay = 1;
    hook.suppress_host_fallback_visuals = 1;
    hook.capture_after_first_frame_render = 1;
    hook.publish_packaged_full_graphics_proof = 1;
    hook.expected_map_index = proof->expected_map_index;
    hook.expected_map_width = proof->expected_map_width;
    hook.expected_map_height = proof->expected_map_height;
    hook.expected_entrance_door_frame_index =
        proof->expected_entrance_door_frame_index;
    hook.expected_hall_overlay_kind = proof->expected_hall_overlay_kind;
    hook.block_enter_until_champion_selected =
        proof->block_enter_until_champion_selected;
    *out_hook = hook;
    return 1;
}

int dm1_v1_startup_hoc_full_start_production_receipt_pc34(
    const char* source_id,
    const DM1_V1_StartupHandoffPostLaunchPlan_PC34* post_plan,
    const DM1_V1_StartupHandoffOutcome_PC34* outcome,
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34* out_receipt) {
    DM1_V1_StartupHoCFullStartProductionReceipt_PC34 receipt;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_hoc_first_frame_receipt_pc34(source_id,
                                                     post_plan,
                                                     outcome,
                                                     &receipt.first_frame)) {
        return 0;
    }
    if (!receipt.first_frame.handled) {
        *out_receipt = receipt;
        return 1;
    }

    receipt.handled = 1;
    receipt.consumed_post_launch_plan = post_plan ? 1 : 0;
    receipt.consumed_handoff_outcome = outcome ? 1 : 0;
    receipt.title_surface_released = receipt.first_frame.title_surface_released;
    receipt.entrance_wait_consumed = receipt.first_frame.entrance_wait_consumed;
    receipt.first_frame_ready = receipt.first_frame.runtime_first_frame_ready;
    receipt.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";

    if (!dm1_v1_startup_hoc_host_render_plan_from_first_frame_pc34(
            &receipt.first_frame,
            &receipt.host_render_plan) ||
        !dm1_v1_startup_hoc_packaged_full_graphics_proof_from_host_plan_pc34(
            &receipt.host_render_plan,
            &receipt.packaged_proof) ||
        !dm1_v1_startup_hoc_production_full_start_hook_from_proof_pc34(
            &receipt.packaged_proof,
            &receipt.production_hook)) {
        return 0;
    }

    /* ReDMCSB TITLE.C F0437 hands off only after title/PRESENTS are complete;
     * ENTRANCE.C F0797/F0441 then owns the first Hall frame.  Keep that full
     * production chain behind one DM1 receipt so M11/M12/package capture do
     * not mix title, entrance, and HoC overlay decisions independently. */
    receipt.host_render_plan_ready = receipt.host_render_plan.ready;
    receipt.packaged_full_graphics_proof_ready = receipt.packaged_proof.ready;
    receipt.production_hook_ready = receipt.production_hook.ready;
    receipt.ready = receipt.first_frame_ready &&
                    receipt.host_render_plan_ready &&
                    receipt.packaged_full_graphics_proof_ready &&
                    receipt.production_hook_ready;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_hoc_full_graphics_capture_artifact_from_production_pc34(
    const DM1_V1_StartupHoCFullStartProductionReceipt_PC34* receipt,
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34* out_artifact) {
    DM1_V1_StartupHoCFullGraphicsCaptureArtifact_PC34 artifact;

    if (!receipt || !out_artifact) {
        return 0;
    }
    memset(&artifact, 0, sizeof(artifact));
    if (!receipt->handled) {
        *out_artifact = artifact;
        return 1;
    }

    artifact.handled = 1;
    artifact.capture_phase = receipt->production_hook.capture_phase;
    artifact.source_evidence =
        "ReDMCSB TITLE.C:319-409; ENTRANCE.C:68-80; ENTRANCE.C:850-883";
    if (!receipt->ready ||
        !receipt->title_surface_released ||
        !receipt->entrance_wait_consumed ||
        !receipt->first_frame_ready ||
        !receipt->host_render_plan_ready ||
        !receipt->packaged_full_graphics_proof_ready ||
        !receipt->production_hook_ready ||
        !receipt->production_hook.consume_dm1_startup_receipts_only ||
        !receipt->production_hook.capture_after_first_frame_render ||
        !receipt->production_hook.publish_packaged_full_graphics_proof ||
        !receipt->packaged_proof.require_no_title_surface ||
        !receipt->packaged_proof.require_no_closed_door_frame ||
        !receipt->packaged_proof.require_no_host_fallback_visuals ||
        receipt->first_frame.hoc_render_command_count != 3) {
        *out_artifact = artifact;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437 completes the title surface before ENTRANCE.C
     * F0797/F0441 draws and waits in Hall.  This artifact is the capture-side
     * consumer: one DM1-owned manifest says what the packaged full-graphics
     * proof must capture, and what stale host surfaces are forbidden. */
    artifact.ready = 1;
    artifact.consume_full_start_production_receipt_only = 1;
    artifact.capture_manifest_ready = 1;
    artifact.capture_after_first_frame_render = 1;
    artifact.publish_packaged_full_graphics_proof = 1;
    artifact.title_surface_forbidden = 1;
    artifact.closed_door_frame_forbidden = 1;
    artifact.host_fallback_visuals_forbidden = 1;
    artifact.opened_entrance_frame_required =
        receipt->production_hook.draw_opened_entrance_frame;
    artifact.hall_mirror_overlay_required =
        receipt->production_hook.render_hall_mirror_overlay;
    artifact.clear_champion_panel_required =
        receipt->production_hook.clear_champion_panel;
    artifact.block_enter_until_champion_selected =
        receipt->production_hook.block_enter_until_champion_selected;
    artifact.expected_map_index = receipt->production_hook.expected_map_index;
    artifact.expected_map_width = receipt->production_hook.expected_map_width;
    artifact.expected_map_height = receipt->production_hook.expected_map_height;
    artifact.expected_entrance_door_frame_index =
        receipt->production_hook.expected_entrance_door_frame_index;
    artifact.expected_hall_overlay_kind =
        receipt->production_hook.expected_hall_overlay_kind;
    artifact.expected_hoc_render_command_count =
        receipt->first_frame.hoc_render_command_count;
    *out_artifact = artifact;
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
    if (!dm1_v1_startup_full_graphics_runtime_handoff_receipt_pc34(
            selected_game_id,
            opened_source_id,
            &result.handoff_outcome,
            &result.host_apply_result,
            &result.runtime_handoff_receipt)) {
        return 0;
    }
    if (result.runtime_handoff_receipt.draw_opened_runtime &&
        callbacks->draw_opened &&
        !callbacks->draw_opened(callbacks->user)) {
        return 0;
    }
    *out_result = result;
    return 1;
}

int dm1_v1_startup_selected_launch_route_receipt_pc34(
    const DM1_V1_StartupSelectedLaunchRouteFacts_PC34* facts,
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedLaunchRouteReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.requires_source_visible_intro =
        dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->selected_game_id);
    receipt.use_dm1_transaction =
        receipt.requires_source_visible_intro ? 1 : 0;
    receipt.use_generic_launch =
        receipt.requires_source_visible_intro ? 0 : 1;
    *out_receipt = receipt;
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

int dm1_v1_startup_selected_boot_probe_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedBootProbeReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.active = facts->active ? 1 : 0;
    receipt.started_from_launcher = facts->started_from_launcher ? 1 : 0;
    receipt.source_matches =
        (facts->expected_game_id &&
         facts->actual_source_id &&
         strcmp(facts->actual_source_id, facts->expected_game_id) == 0)
            ? 1
            : 0;
    receipt.selected_entry_receipt_valid =
        dm1_v1_startup_selected_entry_receipt_valid_pc34(
            facts->expected_game_id,
            facts->intro_bypassed);
    receipt.valid =
        receipt.active &&
        receipt.source_matches &&
        receipt.started_from_launcher &&
        receipt.selected_entry_receipt_valid;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_selected_boot_probe_source_kind_receipt_pc34(
    const DM1_V1_StartupSelectedBootProbeSourceKindFacts_PC34* facts,
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34* out_receipt) {
    DM1_V1_StartupSelectedBootProbeSourceKindReceipt_PC34 receipt;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.actual_source_kind = facts->actual_source_kind;
    receipt.expected_source_kind = facts->dm1_builtin_source_kind;
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(
            facts->expected_game_id)) {
        receipt.valid = 1;
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB: TITLE.C F0437 and ENTRANCE.C F0441 are part of the DM1
     * built-in source startup path. Firestaff keeps M11's enum private, but
     * DM1 owns the receipt requiring selected boot probes to come from the
     * built-in catalog source kind. */
    receipt.handled = 1;
    receipt.valid =
        facts->actual_source_kind == facts->dm1_builtin_source_kind;
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_title_menu_eligibility_receipt_pc34(
    const DM1_V1_StartupTitleMenuEligibilityFacts_PC34* facts,
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34* out_receipt) {
    DM1_V1_StartupTitleMenuEligibilityReceipt_PC34 receipt;
    unsigned int frame_max;

    if (!facts || !out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    receipt.handled = 1;
    receipt.keep_title_surface = 1;
    receipt.next_stage = DM1_V1_STARTUP_STAGE_TITLE_LAST_FRAME_PC34;
    receipt.reason = "title-active";

    frame_max = facts->title_frame_max
                    ? facts->title_frame_max
                    : dm1_v1_startup_title_frame_bank_equivalent_steps_pc34();
    if (!facts->title_handoff_ready || facts->title_frame <= frame_max) {
        *out_receipt = receipt;
        return 1;
    }
    if (!facts->advance_requested) {
        receipt.reason = "title-held-after-handoff";
        *out_receipt = receipt;
        return 1;
    }

    /* ReDMCSB TITLE.C F0437:319-409 completes PRESENTS, title zoom,
     * STRIKES BACK, and the final VBlank guard before ENTRANCE.C F0441:
     * 850-883 enters the entrance-wait loop.  The handoff click/key is a
     * title/menu transition only; it must not become the first entrance
     * command. */
    receipt.menu_eligible = 1;
    receipt.keep_title_surface = 0;
    receipt.consume_pending_input = 1;
    receipt.next_stage = DM1_V1_STARTUP_STAGE_MENU_ELIGIBLE_PC34;
    receipt.reason = "title-complete-menu-eligible";
    *out_receipt = receipt;
    return 1;
}

int dm1_v1_startup_full_graphics_media_receipt_pc34(
    const char* source_id,
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34* out_receipt) {
    DM1_V1_StartupFullGraphicsMediaReceipt_PC34 receipt;
    V1_TitleFrontendSourceTiming title_timing;
    EntranceCompatSourceAnimationStep entrance_pre_open_step;
    int presents_palette = 0;
    int title_palette = 0;

    if (!out_receipt) {
        return 0;
    }
    memset(&receipt, 0, sizeof(receipt));
    if (!dm1_v1_startup_source_visible_handoff_required_pc34(source_id)) {
        *out_receipt = receipt;
        return 1;
    }

    title_timing = V1_TitleFrontend_GetSourceTimingEvidence();
    (void)V1_TitleFrontend_GetStepPalette(
        V1_TITLE_FRONTEND_SOURCE_EVENT_PRESENTS,
        &presents_palette);
    (void)V1_TitleFrontend_GetStepPalette(
        V1_TITLE_FRONTEND_SOURCE_EVENT_ZOOM_BLIT,
        &title_palette);

    receipt.handled = 1;
    receipt.play_swsh = 1;
    receipt.play_title = 1;
    receipt.play_entrance = 1;
    receipt.swsh_vblank_ms = SWSH_COMPAT_RUNTIME_VBLANK_MS;
    receipt.swsh_initial_logo_hold_ms =
        SWSH_Compat_GetRuntimeInitialLogoHoldMs();
    receipt.swsh_palette_wait_ms =
        SWSH_Compat_GetRuntimeDelayMsForVblankCount(
            SWSH_Compat_GetSourceTimingEvidence().paletteWaitVblankCount);
    receipt.swsh_sound_wait_ms =
        SWSH_Compat_GetRuntimeDelayMsForVblankCount(
            SWSH_Compat_GetSourceTimingEvidence().soundWaitVblankCount);
    receipt.swsh_final_hold_ms = SWSH_Compat_GetRuntimeFinalHoldMs();
    receipt.title_presents_hold_ms =
        V1_TitleFrontend_GetRuntimePresentsHoldDelayMs(&title_timing);
    receipt.title_zoom_frame_delay_ms =
        V1_TitleFrontend_GetRuntimeFrameDelayMs(&title_timing);
    receipt.title_zoom_step_count = title_timing.zoomStepCount;
    receipt.title_post_zoom_guard_ms =
        V1_TitleFrontend_GetRuntimeFinalGuardDelayMs(&title_timing);
    receipt.title_c001_cadence_pad_ms =
        V1_TitleFrontend_GetRuntimeC001CadencePadDelayMs(&title_timing);
    receipt.title_source_animation_steps =
        title_timing.sourceAnimationStepCount;
    receipt.title_frame_bank_equivalent_steps =
        title_timing.frameBankEquivalentStepCount;
    receipt.title_menu_boundary_frame =
        title_timing.firstMenuEligibleStep;
    receipt.title_presents_palette = presents_palette;
    receipt.title_zoom_palette = title_palette;
    receipt.title_menu_eligible = 1;
    receipt.title_consume_pending_input = 1;
    receipt.entrance_auto_enter_ms = 1200;
    receipt.entrance_source_animation_steps =
        ENTRANCE_Compat_GetSourceAnimationStepCount();
    receipt.entrance_door_step_count =
        ENTRANCE_Compat_GetDoorAnimationStepCount();
    receipt.entrance_vblank_ms = ENTRANCE_Compat_GetVblankDelayMs();
    memset(&entrance_pre_open_step, 0, sizeof(entrance_pre_open_step));
    if (ENTRANCE_Compat_GetSourceAnimationStep(6u, &entrance_pre_open_step) &&
        entrance_pre_open_step.kind ==
            ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY) {
        receipt.entrance_pre_open_delay_ms =
            ENTRANCE_Compat_GetRuntimeDelayMs(&entrance_pre_open_step);
    }
    /* ReDMCSB NECIO.C lines 3592-3609: SWSH sets black/normal curtain,
     * expands the FTL logo, waits F0022_MAIN_SwooshDelay(20), starts sound,
     * then applies palette waits. TITLE.C F0437:319-409 owns PRESENTS,
     * zoom, STRIKES BACK, and final guard before ENTRANCE.C F0441. */
    receipt.source_evidence =
        "ReDMCSB NECIO.C:3592-3609; TITLE.C:319-409; ENTRANCE.C:850-883";
    *out_receipt = receipt;
    return 1;
}

unsigned int dm1_v1_startup_entrance_step_delay_ms_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count) {
    if (!media_receipt || !media_receipt->handled ||
        media_receipt->entrance_vblank_ms == 0U) {
        return 0U;
    }
    if (entrance_event_kind == ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY &&
        media_receipt->entrance_pre_open_delay_ms > 0U) {
        return media_receipt->entrance_pre_open_delay_ms;
    }
    if (delay_ticks > 0U) {
        if (delay_ticks > 0xffffffffU / media_receipt->entrance_vblank_ms) {
            return 0U;
        }
        return delay_ticks * media_receipt->entrance_vblank_ms;
    }
    if (vblank_loop_count > 0U) {
        if (vblank_loop_count >
            0xffffffffU / media_receipt->entrance_vblank_ms) {
            return 0U;
        }
        return vblank_loop_count * media_receipt->entrance_vblank_ms;
    }
    return 0U;
}

int dm1_v1_startup_entrance_timing_receipt_valid_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt) {
    EntranceCompatSourceAnimationStep pre_open_step;

    if (!media_receipt || !media_receipt->handled) {
        return 0;
    }
    memset(&pre_open_step, 0, sizeof(pre_open_step));
    if (!ENTRANCE_Compat_GetSourceAnimationStep(6U, &pre_open_step) ||
        pre_open_step.kind != ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY) {
        return 0;
    }
    return
        media_receipt->entrance_source_animation_steps ==
            ENTRANCE_Compat_GetSourceAnimationStepCount() &&
        media_receipt->entrance_door_step_count ==
            ENTRANCE_Compat_GetDoorAnimationStepCount() &&
        media_receipt->entrance_vblank_ms ==
            ENTRANCE_Compat_GetVblankDelayMs() &&
        media_receipt->entrance_pre_open_delay_ms ==
            ENTRANCE_Compat_GetRuntimeDelayMs(&pre_open_step);
}

int dm1_v1_startup_entrance_render_audio_command_pc34(
    const DM1_V1_StartupFullGraphicsMediaReceipt_PC34* media_receipt,
    unsigned int source_step,
    int entrance_event_kind,
    unsigned int delay_ticks,
    unsigned int vblank_loop_count,
    DM1_V1_StartupEntranceRenderAudioCommand_PC34* out_command) {
    DM1_V1_StartupEntranceRenderAudioCommand_PC34 command;

    /* ReDMCSB ENTRANCE.C F0441 lines 850-883 drives the entrance as an
     * ordered render/wait loop before the dungeon handoff.  Keep M11 on this
     * DM1 receipt command path so render, palette, sound marker, and delay
     * decisions come from the same source-locked entrance step. */
    if (!out_command || !media_receipt ||
        !dm1_v1_startup_entrance_timing_receipt_valid_pc34(media_receipt) ||
        source_step == 0U ||
        source_step > media_receipt->entrance_source_animation_steps) {
        return 0;
    }
    memset(&command, 0, sizeof(command));
    command.handled = 1;
    command.source_step = source_step;
    command.present_entrance_palette = 1;
    command.delay_ms =
        dm1_v1_startup_entrance_step_delay_ms_pc34(media_receipt,
                                                   entrance_event_kind,
                                                   delay_ticks,
                                                   vblank_loop_count);

    switch ((EntranceCompatSourceEventKind)entrance_event_kind) {
        case ENTRANCE_COMPAT_SOURCE_EVENT_FADE_TO_BLACK:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_FADE_BLACK_PC34;
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_ENTRANCE_SCREEN:
        case ENTRANCE_COMPAT_SOURCE_EVENT_WAIT_FOR_INPUT:
        case ENTRANCE_COMPAT_SOURCE_EVENT_SWITCH_SOUND:
        case ENTRANCE_COMPAT_SOURCE_EVENT_PRE_OPEN_DELAY:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_CLOSED_DOORS_PC34;
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_OPEN_DOOR_STEP:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_OPENING_DOOR_PC34;
            command.door_animation_step = source_step - 6U;
            command.play_door_rattle_sound =
                (command.door_animation_step > 0U &&
                 command.door_animation_step <=
                     media_receipt->entrance_door_step_count &&
                 ((command.door_animation_step % 3U) == 1U));
            break;
        case ENTRANCE_COMPAT_SOURCE_EVENT_DRAW_MICRO_DUNGEON:
        case ENTRANCE_COMPAT_SOURCE_EVENT_FINAL_DUNGEON_VIEW:
        default:
            command.render_kind =
                DM1_V1_STARTUP_ENTRANCE_RENDER_DUNGEON_FRAME_PC34;
            break;
    }

    *out_command = command;
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
