/*
 * Firestaff DM1 V1 viewport palette/as-before cadence probe.
 *
 * Primary source audit: ReDMCSB WIP20210206, Toolchains/Common/Source.
 *   DRAWVIEW.C:F0097_DUNGEONVIEW_DrawViewport lines 715-722: ST-like ports
 *     translate C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME into the current
 *     G0322_B_PaletteSwitchingEnabled state, request the viewport draw, then
 *     wait one vertical blank before returning.
 *   BASE.C:E0017_MAIN_Exception28Handler_VerticalBlank_CPSDF lines 834-836:
 *     a pending draw copies G0323_B_EnablePaletteSwitchingRequested into
 *     G0322_B_PaletteSwitchingEnabled during vertical blank, not at call time.
 *   BASE.C:E0017 lines 961-973: the same pending-draw vertical blank clears
 *     G0342_B_RefreshDungeonViewPaletteRequested and copies the selected
 *     G0304_i_DungeonViewPaletteIndex palette into G0346 middle-screen palette
 *     before copying viewport rows to screen.
 *   DRAWVIEW.C:F0097 lines 730-798: Amiga path treats C2 as the current palette
 *     state, consumes G0342_B_RefreshDungeonViewPaletteRequested when refreshing
 *     the dungeon palette pointer, and only toggles G0322 when the requested
 *     state differs.
 *   DRAWVIEW.C:F0097 lines 821-839: PC/I34 route applies dungeon palette for
 *     state 1, inventory/light palette for state 0, and state 2 performs no
 *     palette case work before the viewport blit.
 *   DRAWVIEW.C:F0097 lines 840-858: the viewport buffer is blitted after palette
 *     state handling and mouse-screen-update protection.
 *   GAMELOOP.C:F0002_MAIN_GameLoop_CPSDF lines 80-102: main loop draws the
 *     viewport only when not resting/inventory, then updates object/refresh mouse
 *     pointer state after F0128/F0097 have returned.
 *   GAMELOOP.C:F0002 lines 128-131: FreezeLifeTicks is decremented before the
 *     action area refresh, not by the viewport present routine.
 *   COMMAND.C:F0380_COMMAND_ProcessQueue_CPSC lines 2336-2347 and 2398-2410:
 *     both rest-screen entry and freeze overlay call F0097 with C2 as-before,
 *     so C2 must restore/preserve whichever palette-switch state was active.
 *
 * This closes the remaining redraw/cadence gap around C2 "as before" during
 * rest/freeze style redraws: the presenter must preserve the prior palette state,
 * issue/present the viewport, and not consume the main-loop mouse update before
 * the present has completed. The follow-up locks the ST vertical-blank refresh
 * flag path too: disabled palette switching remains disabled, but a pending
 * dungeon-palette refresh is still consumed before viewport rows are copied.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    C0_VIEWPORT_NOT_DUNGEON_VIEW = 0,
    C1_VIEWPORT_DUNGEON_VIEW = 1,
    C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME = 2
};

typedef struct {
    int palette_switching_enabled;
    int palette_switching_requested;
    int dungeon_palette_index;
    int applied_palette_index;
    int refresh_palette_requested;
    int draw_viewport_requested;
    int vblank_waits;
    int viewport_presented;
    int mouse_pointer_refresh_in_main_loop;
    int mouse_pointer_updated;
    int freeze_life_ticks;
    int action_area_refreshed;
} Sim;

typedef struct {
    const char *phase;
    int value;
} Event;

#define MAX_EVENTS 96
static Event events[MAX_EVENTS];
static int event_count;
static int failures;

static void reset_events(void) {
    event_count = 0;
}

static void record(const char *phase, int value) {
    if (event_count >= MAX_EVENTS) {
        fprintf(stderr, "too many events\n");
        exit(2);
    }
    events[event_count++] = (Event){ phase, value };
    printf("%02d phase=%-36s value=%d\n", event_count - 1, phase, value);
}

static int index_of(const char *phase) {
    for (int i = 0; i < event_count; ++i) {
        if (strcmp(events[i].phase, phase) == 0) return i;
    }
    return -1;
}

static void check_true(const char *label, int condition) {
    if (!condition) {
        fprintf(stderr, "FAIL %s\n", label);
        failures++;
    }
}

static void check_int(const char *label, int got, int expected) {
    if (got != expected) {
        fprintf(stderr, "FAIL %s: got %d expected %d\n", label, got, expected);
        failures++;
    }
}

static int resolve_as_before_request(const Sim *s, int requested) {
    if (requested == C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME) {
        return s->palette_switching_enabled;
    }
    return requested;
}

static void present_st_like(Sim *s, int requested) {
    int resolved = resolve_as_before_request(s, requested);
    s->palette_switching_requested = resolved;
    record("palette_request_resolved", resolved);

    s->draw_viewport_requested = 1;
    record("viewport_draw_requested", requested);

    s->vblank_waits++;
    record("wait_one_vblank_before_return", s->vblank_waits);

    s->palette_switching_enabled = resolved;
    record("vblank_palette_switch_committed", s->palette_switching_enabled);

    if (s->refresh_palette_requested) {
        s->refresh_palette_requested = 0;
        s->applied_palette_index = s->dungeon_palette_index;
        record("st_dungeon_palette_refresh_consumed", s->applied_palette_index);
    }

    s->viewport_presented = 1;
    record("viewport_presented", s->palette_switching_enabled);
}

static void present_amiga_like(Sim *s, int requested) {
    int resolved = resolve_as_before_request(s, requested);
    record("amiga_request_resolved", resolved);

    if (s->refresh_palette_requested) {
        s->applied_palette_index = s->dungeon_palette_index;
        s->refresh_palette_requested = 0;
        record("dungeon_palette_refresh_consumed", s->applied_palette_index);
        s->palette_switching_enabled = C0_VIEWPORT_NOT_DUNGEON_VIEW;
        record("palette_switch_disabled_for_refresh", s->palette_switching_enabled);
    }

    if (resolved != s->palette_switching_enabled) {
        s->palette_switching_enabled = resolved;
        record("palette_switch_toggled", s->palette_switching_enabled);
    } else {
        record("palette_switch_unchanged", s->palette_switching_enabled);
    }

    s->draw_viewport_requested = 1;
    s->vblank_waits++;
    s->viewport_presented = 1;
    record("amiga_viewport_presented", s->palette_switching_enabled);
}

static void present_pc_i34_like(Sim *s, int requested) {
    switch (requested) {
        case C1_VIEWPORT_DUNGEON_VIEW:
            s->applied_palette_index = s->dungeon_palette_index;
            record("pc_apply_dungeon_palette", s->applied_palette_index);
            break;
        case C0_VIEWPORT_NOT_DUNGEON_VIEW:
            s->applied_palette_index = -1;
            record("pc_apply_inventory_or_light", s->applied_palette_index);
            break;
        case C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME:
            record("pc_as_before_no_palette_case", s->applied_palette_index);
            break;
        default:
            record("pc_unknown_palette_request", requested);
            break;
    }
    s->draw_viewport_requested = 1;
    s->viewport_presented = 1;
    record("pc_viewport_blit_after_palette", requested);
}

static void game_loop_after_present(Sim *s) {
    if (s->mouse_pointer_refresh_in_main_loop) {
        s->mouse_pointer_refresh_in_main_loop = 0;
        s->mouse_pointer_updated = 1;
        record("mouse_pointer_update_after_present", s->mouse_pointer_updated);
    }
    if (s->freeze_life_ticks > 0) {
        s->freeze_life_ticks--;
        record("freeze_life_ticks_decrement", s->freeze_life_ticks);
    }
    s->action_area_refreshed = 1;
    record("action_area_refresh", s->action_area_refreshed);
}

static void verify_st_as_before_preserves_current_state(void) {
    reset_events();
    Sim s;
    memset(&s, 0, sizeof s);
    s.palette_switching_enabled = C1_VIEWPORT_DUNGEON_VIEW;
    s.mouse_pointer_refresh_in_main_loop = 1;
    s.freeze_life_ticks = 2;

    record("scenario_st_as_before_start", s.palette_switching_enabled);
    present_st_like(&s, C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);
    game_loop_after_present(&s);

    check_int("ST C2 resolved to current dungeon palette", s.palette_switching_requested, C1_VIEWPORT_DUNGEON_VIEW);
    check_true("ST present requested", s.draw_viewport_requested && s.viewport_presented);
    check_int("ST waited exactly one vblank", s.vblank_waits, 1);
    check_true("pointer update follows viewport present", index_of("viewport_presented") < index_of("mouse_pointer_update_after_present"));
    check_true("freeze decrement follows pointer/update cadence", index_of("mouse_pointer_update_after_present") < index_of("freeze_life_ticks_decrement"));
    check_int("freeze tick decremented outside present", s.freeze_life_ticks, 1);
}

static void verify_st_as_before_preserves_disabled_state_and_refreshes_palette(void) {
    reset_events();
    Sim s;
    memset(&s, 0, sizeof s);
    s.palette_switching_enabled = C0_VIEWPORT_NOT_DUNGEON_VIEW;
    s.dungeon_palette_index = 2;
    s.applied_palette_index = 5;
    s.refresh_palette_requested = 1;

    record("scenario_st_disabled_refresh_start", s.palette_switching_enabled);
    present_st_like(&s, C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);

    check_int("ST disabled C2 resolved to prior disabled state", s.palette_switching_requested, C0_VIEWPORT_NOT_DUNGEON_VIEW);
    check_int("ST vblank committed disabled palette switch state", s.palette_switching_enabled, C0_VIEWPORT_NOT_DUNGEON_VIEW);
    check_int("ST vblank consumed dungeon palette refresh flag", s.refresh_palette_requested, 0);
    check_int("ST vblank copied selected dungeon palette", s.applied_palette_index, 2);
    check_true("ST palette switch commit precedes refresh", index_of("vblank_palette_switch_committed") < index_of("st_dungeon_palette_refresh_consumed"));
    check_true("ST palette refresh precedes viewport copy", index_of("st_dungeon_palette_refresh_consumed") < index_of("viewport_presented"));
}

static void verify_amiga_refresh_then_as_before_reenables_current_state(void) {
    reset_events();
    Sim s;
    memset(&s, 0, sizeof s);
    s.palette_switching_enabled = C1_VIEWPORT_DUNGEON_VIEW;
    s.dungeon_palette_index = 5;
    s.applied_palette_index = 3;
    s.refresh_palette_requested = 1;

    record("scenario_amiga_refresh_start", s.palette_switching_enabled);
    present_amiga_like(&s, C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);

    check_int("Amiga refresh consumed", s.refresh_palette_requested, 0);
    check_int("Amiga refreshed requested dungeon palette", s.applied_palette_index, 5);
    check_int("Amiga C2 returns to previous enabled state", s.palette_switching_enabled, C1_VIEWPORT_DUNGEON_VIEW);
    check_true("Amiga refresh before toggle", index_of("dungeon_palette_refresh_consumed") < index_of("palette_switch_toggled"));
    check_true("Amiga toggle before present", index_of("palette_switch_toggled") < index_of("amiga_viewport_presented"));
}

static void verify_pc_i34_as_before_does_not_reapply_palette(void) {
    reset_events();
    Sim s;
    memset(&s, 0, sizeof s);
    s.dungeon_palette_index = 7;
    s.applied_palette_index = 4;

    record("scenario_pc_as_before_start", s.applied_palette_index);
    present_pc_i34_like(&s, C2_VIEWPORT_AS_BEFORE_REST_OR_FREEZE_GAME);

    check_int("PC/I34 C2 keeps prior applied palette", s.applied_palette_index, 4);
    check_true("PC/I34 C2 palette no-op before blit", index_of("pc_as_before_no_palette_case") < index_of("pc_viewport_blit_after_palette"));
    check_true("PC/I34 present completed", s.draw_viewport_requested && s.viewport_presented);
}

int main(void) {
    printf("Firestaff DM1 V1 viewport palette/as-before cadence probe\n");
    printf("primary_source=ReDMCSB_WIP20210206/Toolchains/Common/Source\n");
    printf("locks=DRAWVIEW.C:715-722,BASE.C:834-836,BASE.C:961-973,DRAWVIEW.C:730-798,DRAWVIEW.C:821-858,GAMELOOP.C:80-102,GAMELOOP.C:128-131,COMMAND.C:2336-2347,COMMAND.C:2398-2410\n\n");

    verify_st_as_before_preserves_current_state();
    verify_st_as_before_preserves_disabled_state_and_refreshes_palette();
    verify_amiga_refresh_then_as_before_reenables_current_state();
    verify_pc_i34_as_before_does_not_reapply_palette();

    printf("\n[result] failures=%d events=%d\n", failures, event_count);
    return failures ? 1 : 0;
}
