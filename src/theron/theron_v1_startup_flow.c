#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <string.h>

static int tqr_stage_is_available(const Theron_DungeonProgression *progression,
                                  Theron_DungeonID dungeon_id) {
    Theron_DungeonState state;
    if (!progression || dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) {
        return 0;
    }
    state = progression->dungeon_states[dungeon_id - 1];
    return state == THERON_DUNGEON_STATE_AVAILABLE ||
           state == THERON_DUNGEON_STATE_IN_PROGRESS;
}

int theron_v1_startup_stage_available(
    const Theron_DungeonProgression *progression,
    Theron_DungeonID dungeon_id) {

    return tqr_stage_is_available(progression, dungeon_id);
}

static const Theron_StartupMirrorMeta g_tqr_mirror_meta[THERON_STARTUP_HERO_MIRROR_COUNT] = {
    { "Hakar",  THERON_CLASS_FIGHTER, 1 },
    { "Mara",   THERON_CLASS_PRIEST,  2 },
    { "Tiran",  THERON_CLASS_FIGHTER, 3 },
    { "Linos",  THERON_CLASS_NINJA,   4 },
    { "Dotan",  THERON_CLASS_WIZARD,  5 },
    { "Hexa",   THERON_CLASS_FIGHTER, 6 },
    { "Pental", THERON_CLASS_FIGHTER, 7 }
};

static const int g_tqr_mirror_to_track02_roster_index[THERON_STARTUP_HERO_MIRROR_COUNT] = {
    4, /* Hakar -> HAKAR */
    1, /* Mara -> MARA */
    5, /* Tiran -> TIRAN */
    2, /* Linos -> LINOS */
    6, /* Dotan -> DOTAN */
    3, /* Hexa -> HEXA */
    7  /* Pental fallback label -> raw Track 02 PENTAI */
};

const Theron_StartupMirrorMeta *theron_v1_startup_mirror_meta(int mirror_index) {
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return NULL;
    }
    return &g_tqr_mirror_meta[mirror_index];
}

int theron_v1_startup_roster_index_for_mirror(int mirror_index) {
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return -1;
    }
    return g_tqr_mirror_to_track02_roster_index[mirror_index];
}

int theron_v1_startup_mirror_index_for_roster(int roster_index) {
    int mirror;

    for (mirror = 0; mirror < THERON_STARTUP_HERO_MIRROR_COUNT; ++mirror) {
        if (g_tqr_mirror_to_track02_roster_index[mirror] == roster_index) {
            return mirror;
        }
    }
    return -1;
}

const char *theron_v1_startup_class_name(Theron_ChampionClass cls) {
    switch (cls) {
    case THERON_CLASS_FIGHTER: return "FIGHTER";
    case THERON_CLASS_NINJA: return "NINJA";
    case THERON_CLASS_PRIEST: return "PRIEST";
    case THERON_CLASS_WIZARD: return "WIZARD";
    case THERON_CLASS_COUNT: break;
    }
    return "UNKNOWN";
}

void theron_v1_startup_flow_init(Theron_StartupFlow *flow) {
    int i;
    if (!flow) {
        return;
    }
    memset(flow, 0, sizeof(*flow));
    for (i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        flow->selected_mirror_order[i] = 0xffu;
    }
    flow->phase = THERON_STARTUP_PHASE_TITLE;
}

void theron_v1_startup_action_init(Theron_StartupAction *action) {
    if (!action) {
        return;
    }
    memset(action, 0, sizeof(*action));
    action->kind = THERON_STARTUP_ACTION_NONE;
    action->selected_dungeon = THERON_DUNGEON_INVALID;
    action->cursor = 0;
    action->continue_focus = 0;
    action->mirror_index = -1;
}

void theron_v1_startup_hit_init(Theron_StartupHit *hit) {
    if (!hit) {
        return;
    }
    memset(hit, 0, sizeof(*hit));
    hit->kind = THERON_STARTUP_HIT_NONE;
    hit->selected_dungeon = THERON_DUNGEON_INVALID;
    hit->mirror_index = -1;
}

void theron_v1_startup_layout_state_init(Theron_StartupLayoutState *state) {
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->phase = THERON_STARTUP_PHASE_TITLE;
    state->selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    state->soul_cursor = 0;
    state->continue_focus = 0;
    state->tqsv_slot = -1;
    state->srm_slot = -1;
}

static Theron_DungeonID tqr_startup_clamp_stage(Theron_DungeonID dungeon_id) {
    if (dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        return THERON_DUNGEON_1_HALL_OF_RECORDS;
    }
    return dungeon_id;
}

static Theron_DungeonID tqr_startup_move_stage_cursor(
    const Theron_DungeonProgression *progression,
    Theron_DungeonID selected,
    int delta) {

    int id;

    selected = tqr_startup_clamp_stage(selected);
    if (!progression || delta == 0) {
        if (delta < 0 && selected > THERON_DUNGEON_1_HALL_OF_RECORDS) {
            return (Theron_DungeonID)(selected - 1);
        }
        if (delta > 0 && selected < THERON_DUNGEON_COUNT) {
            return (Theron_DungeonID)(selected + 1);
        }
        return selected;
    }

    if (delta < 0) {
        for (id = (int)selected - 1;
             id >= THERON_DUNGEON_1_HALL_OF_RECORDS;
             --id) {
            if (tqr_stage_is_available(progression, (Theron_DungeonID)id)) {
                return (Theron_DungeonID)id;
            }
        }
        return selected;
    }

    for (id = (int)selected + 1; id <= THERON_DUNGEON_COUNT; ++id) {
        if (tqr_stage_is_available(progression, (Theron_DungeonID)id)) {
            return (Theron_DungeonID)id;
        }
    }
    return selected;
}

static void tqr_startup_layout_set_label(
    Theron_StartupLayoutElement *element,
    const char *label) {

    if (!element) {
        return;
    }
    snprintf(element->label,
             sizeof(element->label),
             "%s",
             label ? label : "");
}

static void tqr_startup_layout_set_rect(
    Theron_StartupLayoutElement *element,
    int x,
    int y,
    int w,
    int h) {

    if (!element) {
        return;
    }
    element->x = x;
    element->y = y;
    element->w = w;
    element->h = h;
}

static int tqr_startup_layout_selected_order(
    const Theron_StartupLayoutState *state,
    int mirror_index) {

    int i;

    if (!state || !state->selected_mirror_order ||
        mirror_index < 0 ||
        mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return 0;
    }
    for (i = 0; i < state->selected_mirror_order_count; ++i) {
        if (state->selected_mirror_order[i] == mirror_index) {
            return i + 1;
        }
    }
    return 0;
}

int theron_v1_startup_layout_build(
    const Theron_StartupLayoutState *state,
    Theron_StartupLayoutElement *elements,
    int max_elements) {

    int count = 0;
    int i;
    int row_y;
    int has_continue;
    Theron_DungeonID selected;

    if (!state || !elements || max_elements <= 0) {
        return 0;
    }
    memset(elements, 0, (size_t)max_elements * sizeof(elements[0]));
    for (i = 0; i < max_elements; ++i) {
        elements[i].portrait_index = -1;
        elements[i].primary_class = -1;
        elements[i].dungeon_id = THERON_DUNGEON_INVALID;
        elements[i].mirror_index = -1;
        elements[i].save_slot = -1;
    }

    elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_TITLE;
    elements[count].phase = state->phase;
    elements[count].enabled = 1;
    tqr_startup_layout_set_label(&elements[count], "THERON'S QUEST");
    tqr_startup_layout_set_rect(&elements[count], 34, 22, 152, 12);
    ++count;
    if (count >= max_elements) {
        return count;
    }

    elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_CHAPTER;
    elements[count].phase = state->phase;
    elements[count].enabled = 1;
    tqr_startup_layout_set_label(&elements[count], "Chapter ?");
    tqr_startup_layout_set_rect(&elements[count], 34, 38, 220, 10);
    ++count;
    if (count >= max_elements ||
        state->phase == THERON_STARTUP_PHASE_TITLE) {
        return count;
    }

    if (state->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        row_y = 66;
        has_continue =
            state->has_tqsv_continue || state->has_srm_continue;
        selected = tqr_startup_clamp_stage(state->selected_dungeon);
        if (has_continue && count < max_elements) {
            elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE;
            elements[count].phase = state->phase;
            elements[count].cursor = state->continue_focus ? 1 : 0;
            elements[count].enabled = 1;
            elements[count].selected = state->continue_focus ? 1 : 0;
            elements[count].save_kind = state->has_tqsv_continue ? 1 : 2;
            elements[count].save_slot = state->has_tqsv_continue
                ? state->tqsv_slot
                : state->srm_slot;
            tqr_startup_layout_set_label(
                &elements[count],
                state->has_tqsv_continue ? "CONTINUE TQSV" : "CONTINUE SRM");
            tqr_startup_layout_set_rect(&elements[count], 40, row_y, 168, 10);
            ++count;
            row_y += 12;
        }
        for (i = THERON_DUNGEON_1_HALL_OF_RECORDS;
             i <= THERON_DUNGEON_COUNT && count < max_elements;
             ++i) {
            const Theron_DungeonMeta *meta =
                theron_v1_dungeon_meta((Theron_DungeonID)i);
            int available = theron_v1_startup_stage_available(
                state->progression,
                (Theron_DungeonID)i);
            elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_STAGE;
            elements[count].phase = state->phase;
            elements[count].cursor =
                (!state->continue_focus && i == (int)selected) ? 1 : 0;
            elements[count].enabled = available ? 1 : 0;
            elements[count].selected = (i == (int)selected) ? 1 : 0;
            elements[count].dungeon_id = (Theron_DungeonID)i;
            tqr_startup_layout_set_label(
                &elements[count],
                meta ? meta->name : "Unknown");
            tqr_startup_layout_set_rect(
                &elements[count], 40, row_y + (i - 1) * 13, 220, 10);
            ++count;
        }
        return count;
    }

    for (i = 0;
         i < THERON_STARTUP_HERO_MIRROR_COUNT && count < max_elements;
         ++i) {
        const Theron_StartupMirrorMeta *meta =
            theron_v1_startup_mirror_meta(i);
        int selected_mirror =
            (state->selected_mirrors_mask & (1 << i)) != 0;
        elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_MIRROR;
        elements[count].phase = state->phase;
        elements[count].cursor = (state->soul_cursor == i) ? 1 : 0;
        elements[count].enabled = 1;
        elements[count].selected = selected_mirror ? 1 : 0;
        elements[count].mirror_index = i;
        elements[count].selected_order =
            tqr_startup_layout_selected_order(state, i);
        elements[count].portrait_index = meta ? meta->portrait_index : -1;
        elements[count].primary_class = meta ? (int)meta->primary_class : -1;
        tqr_startup_layout_set_label(
            &elements[count],
            meta ? meta->name : "Hero Mirror");
        tqr_startup_layout_set_rect(
            &elements[count], 46, 78 + i * 11, 230, 10);
        ++count;
    }
    if (count < max_elements) {
        elements[count].kind = THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD;
        elements[count].phase = state->phase;
        elements[count].cursor =
            (state->soul_cursor == THERON_STARTUP_HERO_MIRROR_COUNT) ? 1 : 0;
        elements[count].enabled =
            (state->phase == THERON_STARTUP_PHASE_READY) ? 1 : 0;
        elements[count].selected = elements[count].cursor;
        tqr_startup_layout_set_label(&elements[count], "FORCEFIELD");
        tqr_startup_layout_set_rect(&elements[count], 46, 160, 154, 10);
        ++count;
    }
    return count;
}

static int tqr_startup_point_in_rect(
    int px,
    int py,
    int x,
    int y,
    int w,
    int h) {

    return w > 0 && h > 0 &&
           px >= x && py >= y &&
           px < x + w && py < y + h;
}

int theron_v1_startup_layout_hit_at(
    Theron_StartupPhase phase,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    int x,
    int y,
    Theron_StartupHit *out_hit) {

    int i;

    if (!out_hit) {
        return 0;
    }
    theron_v1_startup_hit_init(out_hit);
    if (phase == THERON_STARTUP_PHASE_TITLE &&
        tqr_startup_point_in_rect(x, y, 34, 22, 242, 150)) {
        out_hit->kind = THERON_STARTUP_HIT_TITLE;
        return 1;
    }
    if (!elements || element_count <= 0) {
        return 0;
    }
    for (i = 0; i < element_count; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        if (!tqr_startup_point_in_rect(x, y, e->x, e->y, e->w, e->h)) {
            continue;
        }
        switch (e->kind) {
        case THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE:
            out_hit->kind = e->enabled
                ? THERON_STARTUP_HIT_CONTINUE
                : THERON_STARTUP_HIT_PANEL;
            return 1;
        case THERON_STARTUP_LAYOUT_ELEMENT_STAGE:
            out_hit->kind = e->enabled
                ? THERON_STARTUP_HIT_STAGE
                : THERON_STARTUP_HIT_PANEL;
            out_hit->selected_dungeon = e->dungeon_id;
            return 1;
        case THERON_STARTUP_LAYOUT_ELEMENT_MIRROR:
            out_hit->kind = e->enabled
                ? THERON_STARTUP_HIT_MIRROR
                : THERON_STARTUP_HIT_PANEL;
            out_hit->mirror_index = e->mirror_index;
            return 1;
        case THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD:
            out_hit->kind = e->enabled
                ? THERON_STARTUP_HIT_FORCEFIELD
                : THERON_STARTUP_HIT_PANEL;
            return 1;
        case THERON_STARTUP_LAYOUT_ELEMENT_TITLE:
        case THERON_STARTUP_LAYOUT_ELEMENT_CHAPTER:
        default:
            out_hit->kind = THERON_STARTUP_HIT_PANEL;
            return 1;
        }
    }
    if (tqr_startup_point_in_rect(x, y, 34, 22, 242, 150)) {
        out_hit->kind = THERON_STARTUP_HIT_PANEL;
        return 1;
    }
    return 0;
}

Theron_StartupResult theron_v1_startup_handle_input_with_progression(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    const Theron_DungeonProgression *progression,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    Theron_StartupInput input,
    Theron_StartupAction *out_action) {

    Theron_DungeonID selected;

    if (!out_action) {
        return THERON_STARTUP_ERR_NULL;
    }
    theron_v1_startup_action_init(out_action);
    selected = tqr_startup_clamp_stage(selected_dungeon);
    out_action->selected_dungeon = selected;
    out_action->cursor = soul_cursor;
    out_action->continue_focus = continue_focus ? 1 : 0;

    if (input == THERON_STARTUP_INPUT_NONE) {
        return THERON_STARTUP_OK;
    }
    if (input == THERON_STARTUP_INPUT_BACK) {
        if (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
            phase == THERON_STARTUP_PHASE_READY) {
            out_action->kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            out_action->cursor = 0;
            out_action->continue_focus = 0;
        } else {
            out_action->kind = THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER;
        }
        return THERON_STARTUP_OK;
    }

    if (phase == THERON_STARTUP_PHASE_TITLE) {
        if (input == THERON_STARTUP_INPUT_ACCEPT ||
            input == THERON_STARTUP_INPUT_ACTION) {
            out_action->kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            out_action->cursor = 0;
            out_action->continue_focus = 0;
        }
        return THERON_STARTUP_OK;
    }

    if (phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        if (input == THERON_STARTUP_INPUT_UP) {
            if (continue_focus) {
                out_action->continue_focus = 0;
            } else if (has_continue) {
                Theron_DungeonID moved =
                    tqr_startup_move_stage_cursor(progression, selected, -1);
                if (moved == selected) {
                    out_action->continue_focus = 1;
                } else {
                    selected = moved;
                }
            } else {
                selected =
                    tqr_startup_move_stage_cursor(progression, selected, -1);
            }
            out_action->selected_dungeon = selected;
            out_action->kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            return THERON_STARTUP_OK;
        }
        if (input == THERON_STARTUP_INPUT_DOWN) {
            if (continue_focus) {
                out_action->continue_focus = 0;
            } else {
                selected =
                    tqr_startup_move_stage_cursor(progression, selected, 1);
            }
            out_action->selected_dungeon = selected;
            out_action->kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            return THERON_STARTUP_OK;
        }
        if (input == THERON_STARTUP_INPUT_ACCEPT ||
            input == THERON_STARTUP_INPUT_ACTION) {
            out_action->kind = continue_focus
                ? THERON_STARTUP_ACTION_CONTINUE_SAVE
                : THERON_STARTUP_ACTION_CHOOSE_STAGE;
            out_action->selected_dungeon = selected;
            return THERON_STARTUP_OK;
        }
        return THERON_STARTUP_OK;
    }

    if (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
        phase == THERON_STARTUP_PHASE_READY) {
        int cursor = soul_cursor;
        if (cursor < 0 || cursor > THERON_STARTUP_HERO_MIRROR_COUNT) {
            cursor = 0;
        }
        if (input == THERON_STARTUP_INPUT_LEFT ||
            input == THERON_STARTUP_INPUT_UP) {
            out_action->cursor =
                (cursor + THERON_STARTUP_HERO_MIRROR_COUNT) %
                (THERON_STARTUP_HERO_MIRROR_COUNT + 1);
            out_action->kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            return THERON_STARTUP_OK;
        }
        if (input == THERON_STARTUP_INPUT_RIGHT ||
            input == THERON_STARTUP_INPUT_DOWN) {
            out_action->cursor =
                (cursor + 1) % (THERON_STARTUP_HERO_MIRROR_COUNT + 1);
            out_action->kind = THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR;
            return THERON_STARTUP_OK;
        }
        if (input == THERON_STARTUP_INPUT_ACCEPT &&
            cursor < THERON_STARTUP_HERO_MIRROR_COUNT) {
            out_action->cursor = cursor;
            out_action->mirror_index = cursor;
            out_action->kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
            return THERON_STARTUP_OK;
        }
        if ((input == THERON_STARTUP_INPUT_ACCEPT &&
             cursor == THERON_STARTUP_HERO_MIRROR_COUNT) ||
            input == THERON_STARTUP_INPUT_ACTION) {
            out_action->cursor = cursor;
            out_action->kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
            return THERON_STARTUP_OK;
        }
    }
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_handle_input(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    Theron_StartupInput input,
    Theron_StartupAction *out_action) {

    return theron_v1_startup_handle_input_with_progression(phase,
                                                           selected_dungeon,
                                                           NULL,
                                                           soul_cursor,
                                                           continue_focus,
                                                           has_continue,
                                                           input,
                                                           out_action);
}

Theron_StartupResult theron_v1_startup_handle_hit_with_progression(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    const Theron_DungeonProgression *progression,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    const Theron_StartupHit *hit,
    Theron_StartupAction *out_action) {

    if (!hit || !out_action) {
        return THERON_STARTUP_ERR_NULL;
    }
    theron_v1_startup_action_init(out_action);
    out_action->selected_dungeon = tqr_startup_clamp_stage(selected_dungeon);
    out_action->cursor = soul_cursor;
    out_action->continue_focus = continue_focus ? 1 : 0;

    if (hit->kind == THERON_STARTUP_HIT_NONE) {
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_PANEL) {
        out_action->kind = THERON_STARTUP_ACTION_NONE;
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_TITLE) {
        if (phase == THERON_STARTUP_PHASE_TITLE) {
            out_action->kind = THERON_STARTUP_ACTION_SHOW_STAGE_SELECT;
            out_action->cursor = 0;
            out_action->continue_focus = 0;
        }
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_CONTINUE) {
        if (phase == THERON_STARTUP_PHASE_STAGE_SELECT && has_continue) {
            out_action->kind = THERON_STARTUP_ACTION_CONTINUE_SAVE;
            out_action->continue_focus = 1;
        }
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_STAGE) {
        if (phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
            Theron_DungeonID hit_stage =
                tqr_startup_clamp_stage(hit->selected_dungeon);
            out_action->selected_dungeon = hit_stage;
            out_action->continue_focus = 0;
            if (!progression || tqr_stage_is_available(progression, hit_stage)) {
                out_action->kind = THERON_STARTUP_ACTION_CHOOSE_STAGE;
            } else {
                out_action->kind = THERON_STARTUP_ACTION_NONE;
            }
        }
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_MIRROR) {
        if (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
            phase == THERON_STARTUP_PHASE_READY) {
            out_action->kind = THERON_STARTUP_ACTION_TOGGLE_MIRROR;
            out_action->mirror_index = hit->mirror_index;
            out_action->cursor = hit->mirror_index;
        }
        return THERON_STARTUP_OK;
    }
    if (hit->kind == THERON_STARTUP_HIT_FORCEFIELD) {
        if (phase == THERON_STARTUP_PHASE_SOUL_ROOM ||
            phase == THERON_STARTUP_PHASE_READY) {
            out_action->kind = THERON_STARTUP_ACTION_ENTER_FORCEFIELD;
            out_action->cursor = THERON_STARTUP_HERO_MIRROR_COUNT;
        }
        return THERON_STARTUP_OK;
    }
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_handle_hit(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
    int soul_cursor,
    int continue_focus,
    int has_continue,
    const Theron_StartupHit *hit,
    Theron_StartupAction *out_action) {

    return theron_v1_startup_handle_hit_with_progression(phase,
                                                         selected_dungeon,
                                                         NULL,
                                                         soul_cursor,
                                                         continue_focus,
                                                         has_continue,
                                                         hit,
                                                         out_action);
}

Theron_StartupResult theron_v1_startup_choose_stage(
    Theron_StartupFlow *flow,
    const Theron_DungeonProgression *progression,
    Theron_DungeonID dungeon_id) {

    if (!flow || !progression) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (dungeon_id < 1 || dungeon_id > THERON_DUNGEON_COUNT) {
        return THERON_STARTUP_ERR_BAD_STAGE;
    }
    if (!tqr_stage_is_available(progression, dungeon_id)) {
        return THERON_STARTUP_ERR_STAGE_LOCKED;
    }

    flow->selected_dungeon = dungeon_id;
    flow->selected_mirrors_mask = 0;
    flow->companion_count = 0;
    for (int i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        flow->selected_mirror_order[i] = 0xffu;
    }
    flow->theron_present = 1;
    flow->forcefield_entered = 0;
    flow->phase = THERON_STARTUP_PHASE_SOUL_ROOM;
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_select_mirror(
    Theron_StartupFlow *flow,
    int mirror_index) {

    uint8_t bit;
    if (!flow) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (flow->selected_dungeon == THERON_DUNGEON_INVALID ||
        flow->phase == THERON_STARTUP_PHASE_TITLE ||
        flow->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        return THERON_STARTUP_ERR_NO_STAGE;
    }
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return THERON_STARTUP_ERR_BAD_MIRROR;
    }

    bit = (uint8_t)(1u << mirror_index);
    if ((flow->selected_mirrors_mask & bit) != 0u) {
        return THERON_STARTUP_ERR_DUPLICATE_MIRROR;
    }
    if (flow->companion_count >= THERON_STARTUP_MAX_COMPANIONS) {
        return THERON_STARTUP_ERR_PARTY_FULL;
    }

    flow->selected_mirrors_mask |= bit;
    flow->selected_mirror_order[flow->companion_count] = (uint8_t)mirror_index;
    flow->companion_count++;
    flow->phase = THERON_STARTUP_PHASE_READY;
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_deselect_mirror(
    Theron_StartupFlow *flow,
    int mirror_index) {

    uint8_t bit;
    int write = 0;

    if (!flow) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (flow->selected_dungeon == THERON_DUNGEON_INVALID ||
        flow->phase == THERON_STARTUP_PHASE_TITLE ||
        flow->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        return THERON_STARTUP_ERR_NO_STAGE;
    }
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return THERON_STARTUP_ERR_BAD_MIRROR;
    }

    bit = (uint8_t)(1u << mirror_index);
    if ((flow->selected_mirrors_mask & bit) == 0u) {
        return THERON_STARTUP_ERR_MIRROR_NOT_SELECTED;
    }

    flow->selected_mirrors_mask = (uint8_t)(flow->selected_mirrors_mask & ~bit);
    for (int read = 0; read < THERON_STARTUP_MAX_COMPANIONS; ++read) {
        uint8_t value = flow->selected_mirror_order[read];
        if (value == 0xffu || value == (uint8_t)mirror_index) {
            continue;
        }
        flow->selected_mirror_order[write++] = value;
    }
    while (write < THERON_STARTUP_MAX_COMPANIONS) {
        flow->selected_mirror_order[write++] = 0xffu;
    }
    flow->companion_count = 0;
    for (int i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        if (flow->selected_mirror_order[i] != 0xffu) {
            ++flow->companion_count;
        }
    }
    flow->phase = flow->companion_count > 0
        ? THERON_STARTUP_PHASE_READY
        : THERON_STARTUP_PHASE_SOUL_ROOM;
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_enter_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party) {

    int slot = 1;
    int mirror;

    if (!flow || !party) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (flow->selected_dungeon == THERON_DUNGEON_INVALID ||
        flow->phase == THERON_STARTUP_PHASE_TITLE ||
        flow->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        return THERON_STARTUP_ERR_NO_STAGE;
    }
    if (!flow->theron_present) {
        return THERON_STARTUP_ERR_NOT_READY;
    }

    theron_v1_party_init(party, (int)flow->selected_dungeon);
    for (int order = 0;
         order < flow->companion_count && slot < THERON_MAX_CHAMPIONS;
         ++order) {
        mirror = flow->selected_mirror_order[order];
        if (mirror >= 0 && mirror < THERON_STARTUP_HERO_MIRROR_COUNT &&
            (flow->selected_mirrors_mask & (uint8_t)(1u << mirror)) != 0u) {
            const Theron_StartupMirrorMeta *meta = theron_v1_startup_mirror_meta(mirror);
            Theron_V1_Champion *champion = &party->champions[slot];
            snprintf(champion->name,
                     sizeof(champion->name),
                     "%s",
                     meta ? meta->name : "Hero Mirror");
            champion->portrait_index = meta ? meta->portrait_index : (uint8_t)(mirror + 1);
            champion->primary_class = meta ? meta->primary_class : THERON_CLASS_FIGHTER;
            champion->alive = 1;
            ++slot;
        }
    }
    party->champion_count = slot;
    party->active_slot = THERON_CHAMPION_SLOT_THERON;

    flow->forcefield_entered = 1;
    flow->phase = THERON_STARTUP_PHASE_IN_DUNGEON;
    return THERON_STARTUP_OK;
}

const char *theron_v1_startup_phase_name(Theron_StartupPhase phase) {
    switch (phase) {
    case THERON_STARTUP_PHASE_TITLE: return "title";
    case THERON_STARTUP_PHASE_STAGE_SELECT: return "stage-select";
    case THERON_STARTUP_PHASE_SOUL_ROOM: return "soul-room";
    case THERON_STARTUP_PHASE_READY: return "ready";
    case THERON_STARTUP_PHASE_IN_DUNGEON: return "in-dungeon";
    default: return "unknown";
    }
}

int theron_v1_startup_receipt_phase(
    Theron_StartupPhase phase,
    char *out_phase,
    int out_phase_size,
    int *out_startup_active) {

    int startup_active;

    if (!out_phase || out_phase_size <= 0) {
        return 0;
    }
    startup_active =
        phase != THERON_STARTUP_PHASE_IN_DUNGEON ? 1 : 0;
    if (startup_active) {
        snprintf(out_phase,
                 (size_t)out_phase_size,
                 "theron-startup-%d",
                 (int)phase);
    } else {
        snprintf(out_phase,
                 (size_t)out_phase_size,
                 "%s",
                 "theron-runtime");
    }
    if (out_startup_active) {
        *out_startup_active = startup_active;
    }
    return 1;
}

const char *theron_v1_startup_result_name(Theron_StartupResult result) {
    switch (result) {
    case THERON_STARTUP_OK: return "ok";
    case THERON_STARTUP_ERR_NULL: return "null";
    case THERON_STARTUP_ERR_BAD_STAGE: return "bad-stage";
    case THERON_STARTUP_ERR_STAGE_LOCKED: return "stage-locked";
    case THERON_STARTUP_ERR_BAD_MIRROR: return "bad-mirror";
    case THERON_STARTUP_ERR_DUPLICATE_MIRROR: return "duplicate-mirror";
    case THERON_STARTUP_ERR_PARTY_FULL: return "party-full";
    case THERON_STARTUP_ERR_NO_STAGE: return "no-stage";
    case THERON_STARTUP_ERR_NOT_READY: return "not-ready";
    case THERON_STARTUP_ERR_MIRROR_NOT_SELECTED: return "mirror-not-selected";
    default: return "unknown";
    }
}

const char *theron_v1_startup_action_name(Theron_StartupActionKind action) {
    switch (action) {
    case THERON_STARTUP_ACTION_NONE: return "none";
    case THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER: return "return-to-launcher";
    case THERON_STARTUP_ACTION_SHOW_STAGE_SELECT: return "show-stage-select";
    case THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR: return "move-stage-cursor";
    case THERON_STARTUP_ACTION_CONTINUE_SAVE: return "continue-save";
    case THERON_STARTUP_ACTION_CHOOSE_STAGE: return "choose-stage";
    case THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR: return "move-soul-cursor";
    case THERON_STARTUP_ACTION_TOGGLE_MIRROR: return "toggle-mirror";
    case THERON_STARTUP_ACTION_ENTER_FORCEFIELD: return "enter-forcefield";
    default: return "unknown";
    }
}

const char *theron_v1_startup_flow_source_evidence(void) {
    return "Theron's Quest manual: choose stage, Soul Room, seven hero mirrors, "
           "resurrect up to three heroes, enter central forcefield; "
           "manual/dmweb champion list: Hakar, Mara, Tiran, Linos, Dotan, "
           "Hexa, Pental as the seven selectable heroes; "
           "raw JP Track 02 roster order: THERON, MARA, LINOS, HEXA, "
           "HAKAR, TIRAN, DOTAN, PENTAI; "
           "dmweb Theron's Quest: Theron plus three champions, companions reset "
           "after dungeon completion; PC Engine Software Bible: console control "
           "profile for Dungeon Master: Theron's Quest.";
}
