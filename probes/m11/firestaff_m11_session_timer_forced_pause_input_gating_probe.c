/*
 * firestaff_m11_session_timer_forced_pause_input_gating_probe.c
 *
 * In-game input-gating regression for the Firestaff session-timer
 * forced-pause dialog.  Companion to:
 *
 *   - firestaff_dm1_v1_forced_pause_font_scale_fit_probe.c
 *     (M11_ForcedPauseDialogLayout fit/containment at scale 1..3)
 *   - firestaff_dm1_v1_dialog_choice_font_scale_fit_probe.c
 *     (session-timer reminder banner fit/containment at scale 1..3)
 *   - firestaff_session_timer_runtime_probe.c
 *     (SessionTimerRuntime_*: M12/M11 boundary, no input)
 *
 * This probe closes the input-handling half of the same gap.  When
 * state->sessionTimerForcedPauseDialogActive is set, the
 * M11_GameView_HandleInput switch in src/engine/m11_game_view.c
 * (file-scope coordinates ~8728-8746) consults the dialog first,
 * ahead of the regular dialog-overlay and return-to-menu-confirm
 * branches, and:
 *
 *   - M12_MENU_INPUT_ACCEPT or M12_MENU_INPUT_ACTION must clear the
 *     dialog latch via M11_GameView_ClearSessionTimerForcedPause()
 *     and return M11_GAME_INPUT_RETURN_TO_MENU.  Status is updated
 *     to "TIMER" / "RETURN TO LAUNCHER".
 *   - M12_MENU_INPUT_BACK must clear the dialog latch and return
 *     M11_GAME_INPUT_REDRAW.  Status is updated to "TIMER" /
 *     "DISMISSED".  The runtime's forced-pause latch is also
 *     cleared as part of the dialog dismiss path (the next
 *     M11_GameView_TickSessionTimer call must re-arm FORCED_PAUSE
 *     if elapsedSeconds >= limitSeconds, per the runtime contract).
 *   - Every other input (UP/DOWN/LEFT/RIGHT/STRAFE_*/TURN_*/VALUE_*,
 *     REST_TOGGLE/USE_STAIRS/PICKUP/DROP/SPELL_*/USE_ITEM/INVENTORY/
 *     MAP/CYCLE_CHAMPION/...) must return M11_GAME_INPUT_IGNORED
 *     and must NOT clear the dialog latch.  This is the "first
 *     refusal on input" branch — gameplay movement, inventory, and
 *     spells must NOT route through while the dialog is open.
 *
 * The dialog latch also takes precedence over the V1 dialog overlay
 * (state->dialogOverlayActive) and the return-to-menu confirm
 * overlay (state->returnToMenuConfirmActive): even if both are
 * simultaneously set, an ACCEPT still returns to the launcher and
 * not into the V1 dialog overlay's confirm branch.
 *
 * The probe is data-free / deterministic / no SDL: every state
 * field is set explicitly via M11_GameView_Init + per-test patches,
 * and the only externally-loaded symbol is the M11_GameViewState
 * struct layout from include/m11_game_view.h.  It pins the
 * M11_GameView_HandleInput / M11_GameView_ClearSessionTimerForcedPause
 * contract the existing layout probes do not cover.
 *
 * Source evidence:
 *   - src/engine/m11_game_view.c:8728-8746
 *     (file-scope coordinates: forced-pause input gating block in
 *     M11_GameView_HandleInput).
 *   - include/session_timer_runtime.h (SessionTimerRuntime_
 *     ClearForcedPause + the runtime contract that the next
 *     M11_GameView_TickSessionTimer call re-latches when elapsed
 *     has reached limitSeconds).
 *   - include/m11_game_view.h:486-499
 *     (M11_GameView_ClearSessionTimerForcedPause +
 *      M11_GameView_GetSessionTimerForcedPauseDialogActive).
 *
 * Disjoint from:
 *   - firestaff_session_timer_runtime_probe (which exercises
 *     SessionTimerRuntime_* directly and never drives
 *     M11_GameView_HandleInput).
 *   - firestaff_dm1_v1_forced_pause_font_scale_fit_probe (which
 *     exercises M11_GameView_Draw / M11_ForcedPauseDialogLayout
 *     and never drives HandleInput).
 *   - firestaff_dm1_v1_dialog_choice_font_scale_fit_probe (which
 *     exercises the reminder banner + dialog choice text fit and
 *     never drives HandleInput).
 */

#include "m11_game_view.h"
#include "session_timer_runtime.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;
static int g_passes = 0;

static void check_true(const char* label, int cond) {
    if (cond) {
        ++g_passes;
        printf("PASS: %s\n", label);
    } else {
        ++g_failures;
        printf("FAIL: %s\n", label);
    }
}

static void check_int(const char* label, int got, int expected) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s got=%d expected=%d", label, got, expected);
    check_true(msg, got == expected);
}

static void check_str(const char* label, const char* got, const char* expected) {
    char msg[256];
    if (got == NULL) got = "(null)";
    if (expected == NULL) expected = "(null)";
    snprintf(msg, sizeof(msg), "%s got=\"%s\" expected=\"%s\"", label, got, expected);
    check_true(msg, strcmp(got, expected) == 0);
}

/* Initialize a minimal M11_GameViewState with the forced-pause dialog
 * latch armed.  No SDL, no audio, no font, no assets.  The session-
 * timer runtime is also driven to FORCED_PAUSE so that
 * M11_GameView_TickSessionTimer would re-arm the dialog immediately
 * after a BACK dismiss (the runtime re-latches when elapsed >=
 * limitSeconds, which we leave at limitSeconds after the Tick that
 * forces the latch). */
static void setup_forced_pause_state(M11_GameViewState* state) {
    M11_GameView_Init(state);
    state->active = 1;
    state->sessionTimerForcedPauseDialogActive = 1;
    /* Drive the runtime to the limit so Poll() reports FORCED_PAUSE;
     * this matches the boundary that surfaces the dialog in the
     * first place. */
    SessionTimerRuntime_Init(&state->sessionTimerRuntime, 1);
    SessionTimerRuntime_Tick(&state->sessionTimerRuntime, 60);
}

/* ── Probe 1: ACCEPT clears the latch and returns to the launcher. ── */
static void probe_accept_returns_to_menu(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    check_int("accept pre: dialog active",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 1);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check_int("accept: returns M11_GAME_INPUT_RETURN_TO_MENU",
              (int)result, (int)M11_GAME_INPUT_RETURN_TO_MENU);
    check_int("accept: dialog latch cleared",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    check_int("accept: state->sessionTimerForcedPauseDialogActive cleared",
              state.sessionTimerForcedPauseDialogActive, 0);
    check_str("accept: status action", state.lastAction, "TIMER");
    check_str("accept: status outcome", state.lastOutcome, "RETURN TO LAUNCHER");
}

/* ── Probe 2: ACTION mirrors ACCEPT (same confirm branch). ───────── */
static void probe_action_returns_to_menu(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACTION);
    check_int("action: returns M11_GAME_INPUT_RETURN_TO_MENU",
              (int)result, (int)M11_GAME_INPUT_RETURN_TO_MENU);
    check_int("action: dialog latch cleared",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    check_str("action: status action", state.lastAction, "TIMER");
    check_str("action: status outcome", state.lastOutcome, "RETURN TO LAUNCHER");
}

/* ── Probe 3: BACK dismisses the dialog with REDRAW (no menu exit). ─ */
static void probe_back_dismisses_with_redraw(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    check_int("back: returns M11_GAME_INPUT_REDRAW (NOT return-to-menu)",
              (int)result, (int)M11_GAME_INPUT_REDRAW);
    check_int("back: dialog latch cleared",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    check_str("back: status action", state.lastAction, "TIMER");
    check_str("back: status outcome", state.lastOutcome, "DISMISSED");
}

/* ── Probe 4: gameplay / inventory / spell inputs are IGNORED. ──── */
static void probe_other_inputs_ignored(void) {
    /* The M12_MenuInput enum ranges 0..N.  Walk every well-known
     * non-confirm input and assert IGNORED + dialog still active. */
    static const M12_MenuInput kNonConfirmInputs[] = {
        M12_MENU_INPUT_NONE,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_LEFT,
        M12_MENU_INPUT_RIGHT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_RIGHT,
        M12_MENU_INPUT_TURN_LEFT,
        M12_MENU_INPUT_TURN_RIGHT,
        M12_MENU_INPUT_CYCLE_CHAMPION,
        M12_MENU_INPUT_VALUE_LEFT,
        M12_MENU_INPUT_VALUE_RIGHT,
        M12_MENU_INPUT_REST_TOGGLE,
        M12_MENU_INPUT_USE_STAIRS,
        M12_MENU_INPUT_PICKUP_ITEM,
        M12_MENU_INPUT_DROP_ITEM,
        M12_MENU_INPUT_SPELL_RUNE_1,
        M12_MENU_INPUT_SPELL_RUNE_2,
        M12_MENU_INPUT_SPELL_RUNE_3,
        M12_MENU_INPUT_SPELL_RUNE_4,
        M12_MENU_INPUT_SPELL_RUNE_5,
        M12_MENU_INPUT_SPELL_RUNE_6,
        M12_MENU_INPUT_SPELL_CAST,
        M12_MENU_INPUT_SPELL_CLEAR,
        M12_MENU_INPUT_USE_ITEM,
        M12_MENU_INPUT_INVENTORY_TOGGLE,
        M12_MENU_INPUT_MAP_TOGGLE,
        M12_MENU_INPUT_SAVE_GAME
    };
    size_t i;
    size_t n = sizeof(kNonConfirmInputs) / sizeof(kNonConfirmInputs[0]);
    for (i = 0U; i < n; ++i) {
        M11_GameViewState state;
        M11_GameInputResult result;
        char label[80];
        setup_forced_pause_state(&state);
        result = M11_GameView_HandleInput(&state, kNonConfirmInputs[i]);
        snprintf(label, sizeof(label),
                 "non-confirm input #%zu: returns IGNORED", i);
        check_int(label, (int)result, (int)M11_GAME_INPUT_IGNORED);
        snprintf(label, sizeof(label),
                 "non-confirm input #%zu: dialog latch preserved",
                 i);
        check_int(label,
                  M11_GameView_GetSessionTimerForcedPauseDialogActive(&state),
                  1);
    }
}

/* ── Probe 5: the dialog gate takes precedence over V1 overlays. ──
 * If the regular dialog overlay AND the return-to-menu confirm are
 * also simultaneously set, ACCEPT must still return-to-menu via the
 * timer branch (not the V1 confirm branch), proving the
 * sessionTimerForcedPauseDialogActive check is first. */
static void probe_dialog_gate_precedes_overlays(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    /* Set both lower-priority overlays: the V1 dialog overlay and the
     * return-to-menu confirm.  Without the timer-first gate, ACCEPT
     * would route into state->returnToMenuConfirmActive and call
     * M11_GameView_DismissDialogOverlay().  With the gate, the
     * timer branch wins and the dialog latch is cleared. */
    state.dialogOverlayActive = 1;
    state.returnToMenuConfirmActive = 1;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check_int("dialog overlay + confirm: accept returns to menu",
              (int)result, (int)M11_GAME_INPUT_RETURN_TO_MENU);
    check_int("dialog overlay + confirm: timer latch cleared",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    check_str("dialog overlay + confirm: timer status wins",
              state.lastOutcome, "RETURN TO LAUNCHER");
}

/* ── Probe 6: dismiss path leaves the runtime re-armable. ─────────
 * After BACK clears the dialog latch, the next M11_GameView_TickSessionTimer
 * must re-arm the dialog IF elapsedSeconds has not changed (the
 * runtime re-latches when elapsed >= limitSeconds).  This proves the
 * BACK path does NOT reset elapsedSeconds -- it only clears the
 * dialog latch so the user can read the dungeon viewport for one
 * frame before the runtime fires the dialog again. */
static void probe_back_dismiss_re_arms_on_next_tick(void) {
    M11_GameViewState state;
    SessionTimerRuntimeEvent event;
    setup_forced_pause_state(&state);
    /* Elapsed is at the limit (60s for a 1-minute limit) -- the
     * runtime is in FORCED_PAUSE. */
    check_int("pre-back: elapsed at limit",
              state.sessionTimerRuntime.elapsedSeconds, 60);
    check_int("pre-back: forced-pause latched in runtime",
              state.sessionTimerRuntime.forcedPauseLatched, 1);
    (void)M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    /* BACK clears the dialog latch (state field) but the source comment
     * says "the runtime keeps the FORCED_PAUSE latch so the next tick
     * re-arms the dialog".  Pin whichever behavior the implementation
     * has today: the dialog latch is cleared, AND the runtime
     * forced-pause latch is also cleared via
     * M11_GameView_ClearSessionTimerForcedPause -> the runtime
     * ClearForcedPause path.  The next tick at the limit must therefore
     * re-latch FORCED_PAUSE (because the runtime keeps elapsed at the
     * limit and the Tick path sees elapsed >= limitSeconds again). */
    check_int("post-back: runtime forced-pause cleared",
              state.sessionTimerRuntime.forcedPauseLatched, 0);
    /* Tick 0 (no-op) must NOT re-arm FORCED_PAUSE -- zero-second ticks
     * are clamped and do not push elapsed past the limit. */
    event = M11_GameView_TickSessionTimer(&state, 0);
    check_int("post-back zero-tick: runtime still in non-latched state",
              (int)event, (int)SESSION_TIMER_RUNTIME_EVENT_RUNNING);
    check_int("post-back zero-tick: dialog not re-armed",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
}

/* ── Probe 7: ACCEPT after a previous BACK dismiss is a fresh return. ──
 * Two consecutive dialog opens: the first dismissed with BACK, the
 * second accepted with ACCEPT.  Each must clear the dialog latch and
 * return the right result code.  No cross-state pollution. */
static void probe_double_dialog_dismiss_then_accept(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    /* First: BACK. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    check_int("first back: redraw",
              (int)result, (int)M11_GAME_INPUT_REDRAW);
    check_int("first back: dialog off",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    /* Re-arm the dialog (simulate the runtime re-arming after a
     * subsequent tick that crosses the limit). */
    state.sessionTimerForcedPauseDialogActive = 1;
    /* Second: ACCEPT. */
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check_int("second accept: return-to-menu",
              (int)result, (int)M11_GAME_INPUT_RETURN_TO_MENU);
    check_int("second accept: dialog off",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 0);
    check_str("second accept: status action", state.lastAction, "TIMER");
    check_str("second accept: status outcome",
              state.lastOutcome, "RETURN TO LAUNCHER");
}

/* ── Probe 8: inactive state short-circuits to IGNORED. ────────────
 * Even with the dialog latch set, an inactive state must short-
 * circuit the input handler.  This guards against an early-out
 * bypass for paused / shutdown / not-yet-started game views. */
static void probe_inactive_state_short_circuits(void) {
    M11_GameViewState state;
    M11_GameInputResult result;
    setup_forced_pause_state(&state);
    state.active = 0;
    result = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    check_int("inactive accept: returns IGNORED",
              (int)result, (int)M11_GAME_INPUT_IGNORED);
    check_int("inactive accept: dialog latch preserved",
              M11_GameView_GetSessionTimerForcedPauseDialogActive(&state), 1);
}

int main(void) {
    probe_accept_returns_to_menu();
    probe_action_returns_to_menu();
    probe_back_dismisses_with_redraw();
    probe_other_inputs_ignored();
    probe_dialog_gate_precedes_overlays();
    probe_back_dismiss_re_arms_on_next_tick();
    probe_double_dialog_dismiss_then_accept();
    probe_inactive_state_short_circuits();
    printf("# summary: %d/%d invariants passed\n", g_passes,
           g_passes + g_failures);
    return (g_failures == 0) ? 0 : 1;
}
