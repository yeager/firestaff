#include "theron_v1_startup_flow.h"

#include <stdio.h>
#include <string.h>

static Theron_DungeonID tqr_startup_clamp_stage(Theron_DungeonID dungeon_id);

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

Theron_StartupInput theron_v1_startup_input_from_firestaff_menu_code(
    int menu_input)
{
    enum {
        FIRESTAFF_MENU_INPUT_NONE = 0,
        FIRESTAFF_MENU_INPUT_UP = 1,
        FIRESTAFF_MENU_INPUT_DOWN = 2,
        FIRESTAFF_MENU_INPUT_LEFT = 3,
        FIRESTAFF_MENU_INPUT_RIGHT = 4,
        FIRESTAFF_MENU_INPUT_TURN_LEFT = 7,
        FIRESTAFF_MENU_INPUT_TURN_RIGHT = 8,
        FIRESTAFF_MENU_INPUT_ACCEPT = 9,
        FIRESTAFF_MENU_INPUT_BACK = 10,
        FIRESTAFF_MENU_INPUT_ACTION = 11
    };

    switch (menu_input) {
    case FIRESTAFF_MENU_INPUT_UP:
        return THERON_STARTUP_INPUT_UP;
    case FIRESTAFF_MENU_INPUT_DOWN:
        return THERON_STARTUP_INPUT_DOWN;
    case FIRESTAFF_MENU_INPUT_LEFT:
    case FIRESTAFF_MENU_INPUT_TURN_LEFT:
        return THERON_STARTUP_INPUT_LEFT;
    case FIRESTAFF_MENU_INPUT_RIGHT:
    case FIRESTAFF_MENU_INPUT_TURN_RIGHT:
        return THERON_STARTUP_INPUT_RIGHT;
    case FIRESTAFF_MENU_INPUT_ACCEPT:
        return THERON_STARTUP_INPUT_ACCEPT;
    case FIRESTAFF_MENU_INPUT_ACTION:
        return THERON_STARTUP_INPUT_ACTION;
    case FIRESTAFF_MENU_INPUT_BACK:
        return THERON_STARTUP_INPUT_BACK;
    case FIRESTAFF_MENU_INPUT_NONE:
    default:
        return THERON_STARTUP_INPUT_NONE;
    }
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

void theron_v1_startup_flow_snapshot_init(Theron_StartupFlowSnapshot *snapshot) {
    int i;
    if (!snapshot) {
        return;
    }
    memset(snapshot, 0, sizeof(*snapshot));
    snapshot->phase = THERON_STARTUP_PHASE_TITLE;
    snapshot->selected_dungeon = THERON_DUNGEON_1_HALL_OF_RECORDS;
    for (i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        snapshot->selected_mirror_order[i] = -1;
    }
}

void theron_v1_startup_state_receipt_init(
    Theron_StartupStateReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    theron_v1_startup_flow_snapshot_init(&receipt->flow);
}

void theron_v1_startup_flow_capture_snapshot(
    const Theron_StartupFlow *flow,
    Theron_StartupFlowSnapshot *snapshot) {

    int i;
    if (!snapshot) {
        return;
    }
    theron_v1_startup_flow_snapshot_init(snapshot);
    if (!flow) {
        return;
    }
    snapshot->phase = flow->phase;
    snapshot->selected_dungeon = flow->selected_dungeon;
    snapshot->selected_mirrors_mask = flow->selected_mirrors_mask;
    snapshot->companion_count = flow->companion_count;
    for (i = 0; i < THERON_STARTUP_MAX_COMPANIONS; ++i) {
        snapshot->selected_mirror_order[i] =
            flow->selected_mirror_order[i] == 0xffu
                ? -1
                : (int)flow->selected_mirror_order[i];
    }
}

int theron_v1_startup_state_receipt_from_flow(
    const Theron_StartupFlow *flow,
    Theron_StartupStateReceipt *out_receipt) {

    if (!flow || !out_receipt) {
        return 0;
    }
    theron_v1_startup_state_receipt_init(out_receipt);
    out_receipt->flow_changed = 1;
    theron_v1_startup_flow_capture_snapshot(flow, &out_receipt->flow);
    return 1;
}

Theron_StartupResult theron_v1_startup_flow_rebuild_from_snapshot(
    const Theron_StartupFlowSnapshot *snapshot,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow) {

    Theron_StartupResult result;
    Theron_DungeonID selected;
    int replayed_mask = 0;
    int slot;
    int mirror;

    if (!snapshot || !progression || !flow) {
        return THERON_STARTUP_ERR_NULL;
    }

    selected = tqr_startup_clamp_stage(snapshot->selected_dungeon);
    theron_v1_startup_flow_init(flow);
    result = theron_v1_startup_choose_stage(flow, progression, selected);
    if (result != THERON_STARTUP_OK) {
        return result;
    }

    for (slot = 0;
         slot < snapshot->companion_count &&
         slot < THERON_STARTUP_MAX_COMPANIONS;
         ++slot) {
        mirror = snapshot->selected_mirror_order[slot];
        if (mirror < 0 || mirror >= THERON_STARTUP_HERO_MIRROR_COUNT ||
            (replayed_mask & (1 << mirror)) != 0 ||
            (snapshot->selected_mirrors_mask & (uint8_t)(1u << mirror)) == 0u) {
            continue;
        }
        result = theron_v1_startup_select_mirror(flow, mirror);
        if (result != THERON_STARTUP_OK) {
            return result;
        }
        replayed_mask |= (1 << mirror);
    }

    for (mirror = 0; mirror < THERON_STARTUP_HERO_MIRROR_COUNT; ++mirror) {
        if ((snapshot->selected_mirrors_mask & (uint8_t)(1u << mirror)) == 0u ||
            (replayed_mask & (1 << mirror)) != 0) {
            continue;
        }
        result = theron_v1_startup_select_mirror(flow, mirror);
        if (result != THERON_STARTUP_OK) {
            return result;
        }
    }

    if (snapshot->phase == THERON_STARTUP_PHASE_READY &&
        flow->phase == THERON_STARTUP_PHASE_SOUL_ROOM) {
        flow->phase = THERON_STARTUP_PHASE_READY;
    }
    return THERON_STARTUP_OK;
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

void theron_v1_startup_action_plan_init(Theron_StartupActionPlan *plan) {
    if (!plan) {
        return;
    }
    memset(plan, 0, sizeof(*plan));
    plan->kind = THERON_STARTUP_PLAN_IGNORE;
    plan->selected_dungeon = THERON_DUNGEON_INVALID;
    plan->cursor = 0;
    plan->continue_focus = 0;
    plan->mirror_index = -1;
}

void theron_v1_startup_execution_init(Theron_StartupExecution *execution) {
    if (!execution) {
        return;
    }
    memset(execution, 0, sizeof(*execution));
    execution->result = THERON_STARTUP_OK;
    execution->cursor = -1;
}

void theron_v1_startup_apply_receipt_init(
    Theron_StartupApplyReceipt *receipt) {

    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
    receipt->input_result = THERON_STARTUP_INPUT_RESULT_IGNORED;
    receipt->cursor = -1;
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

static const char *tqr_startup_layout_roster_name(
    const Theron_StartupLayoutState *state,
    int mirror_index) {

    int roster_index;

    if (!state || mirror_index < 0 ||
        mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return NULL;
    }
    roster_index = theron_v1_startup_roster_index_for_mirror(mirror_index);
    if (roster_index < 0 ||
        roster_index >= state->startup_roster_name_count ||
        roster_index >= THERON_STARTUP_LAYOUT_ROSTER_CAPACITY) {
        return NULL;
    }
    return state->startup_roster_names[roster_index];
}

static const char *tqr_startup_layout_roster_title(
    const Theron_StartupLayoutState *state,
    int mirror_index) {

    int roster_index;

    if (!state || mirror_index < 0 ||
        mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return NULL;
    }
    roster_index = theron_v1_startup_roster_index_for_mirror(mirror_index);
    if (roster_index < 0 ||
        roster_index >= state->startup_roster_name_count ||
        roster_index >= THERON_STARTUP_LAYOUT_ROSTER_CAPACITY) {
        return NULL;
    }
    return state->startup_roster_titles[roster_index];
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
    tqr_startup_layout_set_label(
        &elements[count],
        state->chapter_label[0]
            ? state->chapter_label
            : "Chapter ?");
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
        const char *decoded_name =
            tqr_startup_layout_roster_name(state, i);
        const char *decoded_title =
            tqr_startup_layout_roster_title(state, i);
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
        if (decoded_name && decoded_name[0]) {
            snprintf(elements[count].decoded_name,
                     sizeof(elements[count].decoded_name),
                     "%s",
                     decoded_name);
        }
        if (decoded_title && decoded_title[0]) {
            snprintf(elements[count].decoded_title,
                     sizeof(elements[count].decoded_title),
                     "%s",
                     decoded_title);
        }
        tqr_startup_layout_set_label(
            &elements[count],
            elements[count].decoded_name[0]
                ? elements[count].decoded_name
                : (meta ? meta->name : "Hero Mirror"));
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

static int tqr_startup_render_add_row(
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows,
    int count,
    const char *text) {

    if (!rows || count < 0 || count >= max_rows) {
        return count;
    }
    snprintf(rows[count],
             THERON_STARTUP_RENDER_ROW_CAPACITY,
             "%s",
             text ? text : "");
    return count + 1;
}

static const Theron_StartupLayoutElement *tqr_startup_find_layout_element(
    const Theron_StartupLayoutElement *elements,
    int element_count,
    Theron_StartupLayoutElementKind kind) {

    int i;

    if (!elements || element_count <= 0) {
        return NULL;
    }
    for (i = 0; i < element_count; ++i) {
        if (elements[i].kind == kind) {
            return &elements[i];
        }
    }
    return NULL;
}

static const char *tqr_startup_selected_status(
    const Theron_StartupLayoutElement *element) {

    if (!element || !element->selected) {
        return "AVAILABLE";
    }
    switch (element->selected_order) {
    case 1: return "RESURRECTED #1";
    case 2: return "RESURRECTED #2";
    case 3: return "RESURRECTED #3";
    default: return "RESURRECTED";
    }
}

int theron_v1_startup_apply_receipt_from_flow_execution(
    const Theron_StartupActionPlan *plan,
    const Theron_StartupExecution *execution,
    Theron_StartupApplyReceipt *out_receipt) {

    if (!out_receipt) {
        return 0;
    }
    theron_v1_startup_apply_receipt_init(out_receipt);
    if (!plan || !execution) {
        return 0;
    }
    if (execution->result != THERON_STARTUP_OK) {
        out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
        out_receipt->status_scope = "STARTUP";
        out_receipt->status = theron_v1_startup_result_name(
            execution->result);
        return 1;
    }
    if (!execution->handled) {
        return 1;
    }

    out_receipt->input_result = THERON_STARTUP_INPUT_RESULT_REDRAW;
    out_receipt->status_scope = plan->status_scope
        ? plan->status_scope
        : "STARTUP";
    out_receipt->status = plan->status ? plan->status : "STARTUP";
    if (plan->kind == THERON_STARTUP_PLAN_TOGGLE_MIRROR) {
        out_receipt->status = execution->mirror_selected
            ? (plan->status ? plan->status : "HERO RESURRECTED")
            : (plan->alternate_status
                   ? plan->alternate_status
                   : "HERO RELEASED");
    }
    out_receipt->flow_changed = execution->flow_changed;
    out_receipt->cursor_changed = execution->cursor_changed;
    out_receipt->cursor = execution->cursor;
    out_receipt->continue_focus_changed =
        execution->continue_focus_changed;
    out_receipt->continue_focus = execution->continue_focus;
    return 1;
}

static void tqr_startup_render_plan_reset(Theron_StartupRenderPlan *plan)
{
    if (!plan) {
        return;
    }
    memset(plan, 0, sizeof(*plan));
    plan->background_color = 0;
    plan->border_x = 12;
    plan->border_y = 10;
    plan->border_w = 296;
    plan->border_h = 180;
    plan->border_color = 11;
}

static int tqr_startup_render_plan_add_text(
    Theron_StartupRenderPlan *plan,
    int x,
    int y,
    Theron_StartupRenderTextStyle style,
    const char *text)
{
    Theron_StartupRenderTextCommand *command;

    if (!plan ||
        plan->text_count >= THERON_STARTUP_RENDER_TEXT_CAPACITY_MAX) {
        return 0;
    }
    command = &plan->text[plan->text_count++];
    command->x = x;
    command->y = y;
    command->style = style;
    snprintf(command->text,
             sizeof(command->text),
             "%s",
             text ? text : "");
    return 1;
}

static int tqr_startup_render_plan_add_graphic(
    Theron_StartupRenderPlan *plan,
    Theron_StartupRenderGraphicKind kind,
    int x,
    int y,
    int w,
    int h,
    int color,
    int color2,
    int selected,
    int cursor,
    int ordinal)
{
    Theron_StartupRenderGraphicCommand *command;

    if (!plan ||
        plan->graphic_count >= THERON_STARTUP_RENDER_GRAPHIC_CAPACITY_MAX) {
        return 0;
    }
    command = &plan->graphics[plan->graphic_count++];
    command->kind = kind;
    command->x = x;
    command->y = y;
    command->w = w;
    command->h = h;
    command->color = color;
    command->color2 = color2;
    command->selected = selected ? 1 : 0;
    command->cursor = cursor ? 1 : 0;
    command->ordinal = ordinal;
    return 1;
}

static void tqr_startup_render_plan_add_title_and_chapter(
    Theron_StartupRenderPlan *plan,
    const Theron_StartupLayoutElement *elements,
    int element_count)
{
    int i;

    if (!plan || !elements || element_count <= 0) {
        return;
    }
    for (i = 0; i < element_count; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_TITLE) {
            (void)tqr_startup_render_plan_add_text(
                plan, e->x, e->y, THERON_STARTUP_RENDER_TEXT_TITLE,
                e->label);
        } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_CHAPTER) {
            (void)tqr_startup_render_plan_add_text(
                plan, e->x, e->y, THERON_STARTUP_RENDER_TEXT_SMALL,
                e->label);
        }
    }
}

static void tqr_startup_render_plan_add_title_graphics(
    Theron_StartupRenderPlan *plan)
{
    if (!plan) {
        return;
    }

    /* THQUEST.ASM T000/T080 title and startup entry:
     * keep startup imagery in the Theron-owned render plan.  This bounded
     * command layer is the handoff point for raw Track 02 / Track 03 title
     * surfaces when their bitmap decoder is complete. */
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT,
        0, 0, 320, 200, 0, 0, 0, 0, 0);
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT,
        18, 14, 284, 172, 1, 0, 0, 0, 0);
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT,
        18, 14, 284, 172, 11, 0, 0, 0, 0);
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK,
        74, 62, 172, 70, 14, 5, 0, 0, 0);
}

static void tqr_startup_render_plan_add_stage_graphics(
    Theron_StartupRenderPlan *plan,
    const Theron_StartupLayoutElement *elements,
    int element_count)
{
    int i;

    if (!plan || !elements) {
        return;
    }
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT,
        22, 48, 276, 138, 1, 0, 0, 0, 0);
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT,
        22, 48, 276, 138, 11, 0, 0, 0, 0);
    for (i = 0; i < element_count; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        if (e->kind != THERON_STARTUP_LAYOUT_ELEMENT_STAGE &&
            e->kind != THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE) {
            continue;
        }
        (void)tqr_startup_render_plan_add_graphic(
            plan, THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL,
            e->x - 6, e->y - 2, 236, e->h + 4,
            e->enabled ? 3 : 8,
            e->cursor ? 14 : 7,
            e->selected,
            e->cursor,
            e->dungeon_id);
    }
}

static void tqr_startup_render_plan_add_soul_room_graphics(
    Theron_StartupRenderPlan *plan,
    const Theron_StartupLayoutElement *elements,
    int element_count)
{
    int i;

    if (!plan || !elements) {
        return;
    }
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT,
        18, 48, 284, 138, 1, 0, 0, 0, 0);
    (void)tqr_startup_render_plan_add_graphic(
        plan, THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT,
        18, 48, 284, 138, 11, 0, 0, 0, 0);
    for (i = 0; i < element_count; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_MIRROR) {
            (void)tqr_startup_render_plan_add_graphic(
                plan, THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME,
                28 + (e->mirror_index % 4) * 70,
                82 + (e->mirror_index / 4) * 36,
                54,
                28,
                e->selected ? 10 : 3,
                e->cursor ? 14 : 7,
                e->selected,
                e->cursor,
                e->portrait_index);
        } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD) {
            (void)tqr_startup_render_plan_add_graphic(
                plan, THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD,
                108,
                150,
                104,
                28,
                e->enabled ? 14 : 8,
                e->cursor ? 15 : 5,
                e->enabled,
                e->cursor,
                0);
        }
    }
}

int theron_v1_startup_render_plan_build(
    const Theron_StartupLayoutState *state,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    Theron_StartupRenderPlan *out_plan)
{
    int i;

    if (!state || !out_plan) {
        return 0;
    }
    tqr_startup_render_plan_reset(out_plan);
    out_plan->phase = state->phase;
    tqr_startup_render_plan_add_title_and_chapter(
        out_plan, elements, element_count);

    if (state->phase == THERON_STARTUP_PHASE_TITLE) {
        tqr_startup_render_plan_add_title_graphics(out_plan);
        return tqr_startup_render_plan_add_text(
            out_plan,
            34,
            76,
            THERON_STARTUP_RENDER_TEXT_SHADOW,
            "PRESS ENTER TO START");
    }

    if (state->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        int has_continue =
            state->has_tqsv_continue || state->has_srm_continue;

        tqr_startup_render_plan_add_stage_graphics(
            out_plan, elements, element_count);
        (void)tqr_startup_render_plan_add_text(
            out_plan,
            34,
            52,
            THERON_STARTUP_RENDER_TEXT_SHADOW,
            "CHOOSE A STAGE");
        if (state->startup_roster_name_count > 0) {
            char roster_row[THERON_STARTUP_RENDER_TEXT_CAPACITY];
            int written = snprintf(roster_row,
                                   sizeof(roster_row),
                                   "ROSTER:");
            for (i = 0;
                 i < state->startup_roster_name_count &&
                 i < THERON_STARTUP_LAYOUT_ROSTER_CAPACITY;
                 ++i) {
                const char *name = state->startup_roster_names[i];
                if (written < 0 ||
                    written >= (int)sizeof(roster_row) ||
                    !name || !name[0]) {
                    continue;
                }
                written += snprintf(roster_row + written,
                                    sizeof(roster_row) - (size_t)written,
                                    " %s",
                                    name);
            }
            (void)tqr_startup_render_plan_add_text(
                out_plan,
                34,
                164,
                THERON_STARTUP_RENDER_TEXT_SMALL,
                roster_row);
        }
        for (i = 0; i < element_count; ++i) {
            const Theron_StartupLayoutElement *e = &elements[i];
            char row[THERON_STARTUP_RENDER_TEXT_CAPACITY];
            if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE) {
                snprintf(row,
                         sizeof(row),
                         "%c CONTINUE  %s SLOT %d",
                         e->cursor ? '>' : ' ',
                         e->save_kind == 1 ? "TQSV" : "SRM",
                         e->save_slot);
                (void)tqr_startup_render_plan_add_text(
                    out_plan,
                    e->x,
                    e->y,
                    e->cursor ? THERON_STARTUP_RENDER_TEXT_ACTIVE
                              : THERON_STARTUP_RENDER_TEXT_PICKED,
                    row);
            } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE) {
                snprintf(row,
                         sizeof(row),
                         "%c %d  %s%s",
                         e->cursor ? '>' : ' ',
                         e->dungeon_id,
                         e->label,
                         e->enabled ? "" : "  LOCKED");
                (void)tqr_startup_render_plan_add_text(
                    out_plan,
                    e->x,
                    e->y,
                    e->cursor
                        ? THERON_STARTUP_RENDER_TEXT_ACTIVE
                        : (e->enabled ? THERON_STARTUP_RENDER_TEXT_SMALL
                                      : THERON_STARTUP_RENDER_TEXT_LOCKED),
                    row);
            }
        }
        return tqr_startup_render_plan_add_text(
            out_plan,
            34,
            176,
            THERON_STARTUP_RENDER_TEXT_SMALL,
            has_continue
                ? "UP/DOWN SELECT  ENTER CONTINUE/STAGE"
                : "UP/DOWN SELECT  ENTER OPEN SOUL ROOM");
    }

    tqr_startup_render_plan_add_soul_room_graphics(
        out_plan, elements, element_count);
    (void)tqr_startup_render_plan_add_text(
        out_plan, 34, 52, THERON_STARTUP_RENDER_TEXT_SHADOW, "SOUL ROOM");
    (void)tqr_startup_render_plan_add_text(
        out_plan,
        34,
        64,
        THERON_STARTUP_RENDER_TEXT_SMALL,
        state->startup_text_prompt[0] != '\0'
            ? state->startup_text_prompt
            : "THERON WAITS AT THE FORCEFIELD");
    for (i = 0; i < element_count; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        char row[THERON_STARTUP_RENDER_TEXT_CAPACITY];
        if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_MIRROR) {
            snprintf(row,
                     sizeof(row),
                     "%c %-22s %-7s %s",
                     e->cursor ? '>' : ' ',
                     e->label,
                     e->primary_class >= 0
                        ? theron_v1_startup_class_name(
                              (Theron_ChampionClass)e->primary_class)
                        : "UNKNOWN",
                     tqr_startup_selected_status(e));
            (void)tqr_startup_render_plan_add_text(
                out_plan,
                e->x,
                e->y,
                e->selected ? THERON_STARTUP_RENDER_TEXT_PICKED
                            : (e->cursor
                                   ? THERON_STARTUP_RENDER_TEXT_ACTIVE
                                   : THERON_STARTUP_RENDER_TEXT_SMALL),
                row);
        } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD) {
            snprintf(row,
                     sizeof(row),
                     "%c ENTER FORCEFIELD",
                     e->cursor ? '>' : ' ');
            (void)tqr_startup_render_plan_add_text(
                out_plan,
                e->x,
                e->y,
                e->cursor ? THERON_STARTUP_RENDER_TEXT_ACTIVE
                          : THERON_STARTUP_RENDER_TEXT_SMALL,
                row);
        }
    }
    return tqr_startup_render_plan_add_text(
        out_plan,
        34,
        176,
        THERON_STARTUP_RENDER_TEXT_SMALL,
        "ENTER SELECTS MIRROR  ACTION ENTERS");
}

static void tqr_startup_exec_fill(
    const Theron_StartupGraphicExecutor *executor,
    int x,
    int y,
    int w,
    int h,
    int color)
{
    if (executor && executor->fill_rect) {
        executor->fill_rect(executor->userdata, x, y, w, h, color);
    }
}

static void tqr_startup_exec_rect(
    const Theron_StartupGraphicExecutor *executor,
    int x,
    int y,
    int w,
    int h,
    int color)
{
    if (executor && executor->draw_rect) {
        executor->draw_rect(executor->userdata, x, y, w, h, color);
    }
}

static void tqr_startup_exec_pixel(
    const Theron_StartupGraphicExecutor *executor,
    int x,
    int y,
    int color)
{
    if (executor && executor->plot_pixel) {
        executor->plot_pixel(executor->userdata, x, y, color);
    }
}

static void tqr_startup_exec_title_mark(
    const Theron_StartupRenderGraphicCommand *command,
    const Theron_StartupGraphicExecutor *executor)
{
    int cx;
    int cy;
    int i;

    if (!command || !executor) {
        return;
    }
    tqr_startup_exec_rect(executor,
                          command->x,
                          command->y,
                          command->w,
                          command->h,
                          command->color);
    tqr_startup_exec_rect(executor,
                          command->x + 4,
                          command->y + 4,
                          command->w - 8,
                          command->h - 8,
                          command->color2);
    cx = command->x + command->w / 2;
    cy = command->y + command->h / 2;
    for (i = -28; i <= 28; ++i) {
        tqr_startup_exec_pixel(executor, cx + i, cy + i / 2, command->color);
        tqr_startup_exec_pixel(executor, cx + i, cy - i / 2, command->color2);
    }
    tqr_startup_exec_fill(executor, cx - 18, cy - 5, 36, 10, command->color);
}

static void tqr_startup_exec_mirror_frame(
    const Theron_StartupRenderGraphicCommand *command,
    const Theron_StartupGraphicExecutor *executor)
{
    int portrait_color;
    int inset_color;

    if (!command || !executor) {
        return;
    }
    portrait_color = 2 + ((command->ordinal > 0 ? command->ordinal : 1) % 12);
    inset_color = command->selected ? 10 : portrait_color;
    tqr_startup_exec_fill(executor,
                          command->x,
                          command->y,
                          command->w,
                          command->h,
                          0);
    tqr_startup_exec_rect(executor,
                          command->x,
                          command->y,
                          command->w,
                          command->h,
                          command->cursor ? command->color2
                                          : command->color);
    tqr_startup_exec_fill(executor,
                          command->x + 4,
                          command->y + 4,
                          18,
                          command->h - 8,
                          inset_color);
    tqr_startup_exec_rect(executor,
                          command->x + 26,
                          command->y + 5,
                          command->w - 32,
                          command->h - 10,
                          command->selected ? 10 : 8);
}

static void tqr_startup_exec_forcefield(
    const Theron_StartupRenderGraphicCommand *command,
    const Theron_StartupGraphicExecutor *executor)
{
    int i;
    int cx;

    if (!command || !executor) {
        return;
    }
    tqr_startup_exec_rect(executor,
                          command->x,
                          command->y,
                          command->w,
                          command->h,
                          command->color2);
    cx = command->x + command->w / 2;
    for (i = 0; i < command->w / 2; i += 6) {
        int x = cx - i;
        int w = i * 2;
        if (w <= 0) {
            continue;
        }
        tqr_startup_exec_rect(executor,
                              x,
                              command->y + 2 + (i % 12) / 2,
                              w,
                              command->h - 4 - (i % 12),
                              command->color);
    }
}

int theron_v1_startup_execute_graphics_plan(
    const Theron_StartupRenderPlan *plan,
    const Theron_StartupGraphicExecutor *executor)
{
    int i;

    if (!plan || !executor || !executor->fill_rect ||
        !executor->draw_rect || !executor->plot_pixel) {
        return 0;
    }
    for (i = 0; i < plan->graphic_count; ++i) {
        const Theron_StartupRenderGraphicCommand *command =
            &plan->graphics[i];
        switch (command->kind) {
        case THERON_STARTUP_RENDER_GRAPHIC_FILL_RECT:
            tqr_startup_exec_fill(executor,
                                  command->x,
                                  command->y,
                                  command->w,
                                  command->h,
                                  command->color);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_DRAW_RECT:
            tqr_startup_exec_rect(executor,
                                  command->x,
                                  command->y,
                                  command->w,
                                  command->h,
                                  command->color);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_TITLE_MARK:
            tqr_startup_exec_title_mark(command, executor);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_STAGE_PANEL:
            tqr_startup_exec_fill(executor,
                                  command->x,
                                  command->y,
                                  command->w,
                                  command->h,
                                  command->selected ? command->color : 0);
            tqr_startup_exec_rect(executor,
                                  command->x,
                                  command->y,
                                  command->w,
                                  command->h,
                                  command->cursor ? command->color2
                                                  : command->color);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_MIRROR_FRAME:
            tqr_startup_exec_mirror_frame(command, executor);
            break;
        case THERON_STARTUP_RENDER_GRAPHIC_FORCEFIELD:
            tqr_startup_exec_forcefield(command, executor);
            break;
        default:
            break;
        }
    }
    return 1;
}

int theron_v1_startup_render_rows_build(
    const Theron_StartupLayoutState *state,
    const Theron_StartupLayoutElement *elements,
    int element_count,
    char rows[][THERON_STARTUP_RENDER_ROW_CAPACITY],
    int max_rows) {

    const Theron_StartupLayoutElement *title;
    const Theron_StartupLayoutElement *chapter;
    int count = 0;
    int i;

    if (!state || !rows || max_rows <= 0) {
        return 0;
    }
    memset(rows, 0,
           (size_t)max_rows * THERON_STARTUP_RENDER_ROW_CAPACITY);

    title = tqr_startup_find_layout_element(
        elements, element_count, THERON_STARTUP_LAYOUT_ELEMENT_TITLE);
    chapter = tqr_startup_find_layout_element(
        elements, element_count, THERON_STARTUP_LAYOUT_ELEMENT_CHAPTER);
    if (title && count < max_rows) {
        count = tqr_startup_render_add_row(
            rows, max_rows, count, title->label);
    }
    if (chapter && count < max_rows) {
        count = tqr_startup_render_add_row(
            rows, max_rows, count, chapter->label);
    }
    if (count >= max_rows) {
        return count;
    }

    if (state->phase == THERON_STARTUP_PHASE_TITLE) {
        return tqr_startup_render_add_row(
            rows, max_rows, count, "PRESS ENTER TO START");
    }

    if (state->phase == THERON_STARTUP_PHASE_STAGE_SELECT) {
        int has_continue =
            state->has_tqsv_continue || state->has_srm_continue;

        count = tqr_startup_render_add_row(
            rows, max_rows, count, "CHOOSE A STAGE");
        if (count >= max_rows) {
            return count;
        }
        if (state->startup_roster_name_count > 0) {
            int written = snprintf(rows[count],
                                   THERON_STARTUP_RENDER_ROW_CAPACITY,
                                   "TRACK 02 ROSTER:");
            for (i = 0;
                 i < state->startup_roster_name_count &&
                 i < THERON_STARTUP_LAYOUT_ROSTER_CAPACITY;
                 ++i) {
                const char *name = state->startup_roster_names[i];
                if (written < 0 ||
                    written >= THERON_STARTUP_RENDER_ROW_CAPACITY ||
                    !name || !name[0]) {
                    continue;
                }
                written += snprintf(
                    rows[count] + written,
                    (size_t)THERON_STARTUP_RENDER_ROW_CAPACITY -
                        (size_t)written,
                    " %s",
                    name);
            }
            ++count;
            if (count >= max_rows) {
                return count;
            }
        }
        if (state->startup_roster_name_count > 7 &&
            state->startup_roster_titles[1] &&
            state->startup_roster_titles[7] &&
            state->startup_roster_titles[1][0] != '\0' &&
            state->startup_roster_titles[7][0] != '\0') {
            snprintf(rows[count++],
                     THERON_STARTUP_RENDER_ROW_CAPACITY,
                     "TRACK 02 TITLES: MARA=%s; PENTAI=%s",
                     state->startup_roster_titles[1],
                     state->startup_roster_titles[7]);
            if (count >= max_rows) {
                return count;
            }
        }
        for (i = 0; i < element_count && count < max_rows; ++i) {
            const Theron_StartupLayoutElement *e = &elements[i];
            if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_CONTINUE) {
                snprintf(rows[count++],
                         THERON_STARTUP_RENDER_ROW_CAPACITY,
                         "%c CONTINUE  %s SLOT %d",
                         e->cursor ? '>' : ' ',
                         e->save_kind == 1 ? "TQSV" : "SRM",
                         e->save_slot);
            } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_STAGE) {
                snprintf(rows[count++],
                         THERON_STARTUP_RENDER_ROW_CAPACITY,
                         "%c %d  %s%s",
                         e->cursor ? '>' : ' ',
                         e->dungeon_id,
                         e->label,
                         e->enabled ? "" : "  LOCKED");
            }
        }
        if (count < max_rows) {
            count = tqr_startup_render_add_row(
                rows,
                max_rows,
                count,
                has_continue
                    ? "UP/DOWN SELECT  ENTER CONTINUE/STAGE"
                    : "UP/DOWN SELECT  ENTER OPEN SOUL ROOM");
        }
        return count;
    }

    count = tqr_startup_render_add_row(
        rows, max_rows, count, "SOUL ROOM");
    if (count >= max_rows) {
        return count;
    }
    count = tqr_startup_render_add_row(
        rows,
        max_rows,
        count,
        state->startup_text_prompt[0] != '\0'
            ? state->startup_text_prompt
            : "THERON WAITS AT THE FORCEFIELD");
    if (count >= max_rows) {
        return count;
    }
    for (i = 0; i < element_count && count < max_rows; ++i) {
        const Theron_StartupLayoutElement *e = &elements[i];
        if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_MIRROR) {
            snprintf(rows[count++],
                     THERON_STARTUP_RENDER_ROW_CAPACITY,
                     "%c %-22s %-7s %s",
                     e->cursor ? '>' : ' ',
                     e->label,
                     e->primary_class >= 0
                        ? theron_v1_startup_class_name(
                              (Theron_ChampionClass)e->primary_class)
                        : "UNKNOWN",
                     tqr_startup_selected_status(e));
        } else if (e->kind == THERON_STARTUP_LAYOUT_ELEMENT_FORCEFIELD) {
            snprintf(rows[count++],
                     THERON_STARTUP_RENDER_ROW_CAPACITY,
                     "%c ENTER FORCEFIELD",
                     e->cursor ? '>' : ' ');
        }
    }
    if (count < max_rows) {
        count = tqr_startup_render_add_row(
            rows, max_rows, count, "ENTER SELECTS MIRROR  ACTION ENTERS");
    }
    return count;
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

Theron_StartupResult theron_v1_startup_show_stage_select(
    Theron_StartupFlow *flow,
    Theron_DungeonID dungeon_id) {

    if (!flow) {
        return THERON_STARTUP_ERR_NULL;
    }

    theron_v1_startup_flow_init(flow);
    flow->phase = THERON_STARTUP_PHASE_STAGE_SELECT;
    flow->selected_dungeon = tqr_startup_clamp_stage(dungeon_id);
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

Theron_StartupResult theron_v1_startup_toggle_mirror(
    Theron_StartupFlow *flow,
    int mirror_index,
    int *out_selected) {

    Theron_StartupResult result;
    uint8_t bit;

    if (out_selected) {
        *out_selected = 0;
    }
    if (!flow) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return THERON_STARTUP_ERR_BAD_MIRROR;
    }

    bit = (uint8_t)(1u << mirror_index);
    if ((flow->selected_mirrors_mask & bit) != 0u) {
        result = theron_v1_startup_deselect_mirror(flow, mirror_index);
        if (out_selected) {
            *out_selected = 0;
        }
        return result;
    }

    result = theron_v1_startup_select_mirror(flow, mirror_index);
    if (out_selected && result == THERON_STARTUP_OK) {
        *out_selected = 1;
    }
    return result;
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

Theron_StartupResult theron_v1_startup_enter_forcefield_with_roster(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party,
    const char *const roster_names[],
    int roster_name_count) {

    Theron_V1_Champion persisted_theron;
    Theron_StartupResult result;
    int slot = 1;

    if (!flow || !party) {
        return THERON_STARTUP_ERR_NULL;
    }

    memset(&persisted_theron, 0, sizeof(persisted_theron));
    persisted_theron = party->champions[THERON_CHAMPION_SLOT_THERON];

    result = theron_v1_startup_enter_forcefield(flow, party);
    if (result != THERON_STARTUP_OK) {
        return result;
    }
    if (persisted_theron.name[0] != '\0') {
        party->champions[THERON_CHAMPION_SLOT_THERON] = persisted_theron;
    }

    for (int order = 0;
         order < flow->companion_count && slot < THERON_MAX_CHAMPIONS;
         ++order) {
        int mirror = flow->selected_mirror_order[order];
        int roster_index;
        if (mirror < 0 ||
            mirror >= THERON_STARTUP_HERO_MIRROR_COUNT ||
            (flow->selected_mirrors_mask & (uint8_t)(1u << mirror)) == 0u) {
            continue;
        }
        roster_index = theron_v1_startup_roster_index_for_mirror(mirror);
        if (roster_names &&
            roster_index >= 0 &&
            roster_index < roster_name_count &&
            roster_names[roster_index] &&
            roster_names[roster_index][0] != '\0') {
            snprintf(party->champions[slot].name,
                     sizeof(party->champions[slot].name),
                     "%s",
                     roster_names[roster_index]);
        }
        ++slot;
    }
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_enter_world_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world) {

    if (!flow || !world) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (flow->selected_dungeon == THERON_DUNGEON_INVALID ||
        flow->phase != THERON_STARTUP_PHASE_IN_DUNGEON ||
        !flow->forcefield_entered) {
        return THERON_STARTUP_ERR_NOT_READY;
    }
    if (theron_v1_dungeon_enter(&world->progression,
                                flow->selected_dungeon) != 0) {
        return THERON_STARTUP_ERR_DUNGEON_ENTRY;
    }

    theron_v1_world_reset_for_dungeon(world, flow->selected_dungeon);
    memset(world->level_loaded[flow->selected_dungeon - 1],
           0,
           sizeof(world->level_loaded[flow->selected_dungeon - 1]));
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_enter_runtime_from_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_World *world,
    Theron_StartupLevelLoadFn load_level,
    void *userdata,
    char *receipt,
    size_t receipt_cap) {

    Theron_StartupResult result;
    char level_receipt[160];

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!flow || !world || !load_level) {
        return THERON_STARTUP_ERR_NULL;
    }

    level_receipt[0] = '\0';
    result = theron_v1_startup_enter_world_from_forcefield(flow, world);
    if (result != THERON_STARTUP_OK) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt,
                     receipt_cap,
                     "startup-flow dungeon enter failed: %s",
                     theron_v1_startup_result_name(result));
        }
        return result;
    }

    if (!load_level(world,
                    flow->selected_dungeon,
                    userdata,
                    level_receipt,
                    sizeof(level_receipt))) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "Theron level load failed");
        }
        return THERON_STARTUP_ERR_LEVEL_LOAD;
    }

    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 "startup-flow stage=%d phase=%s party=%d companions=%d; %s",
                 (int)flow->selected_dungeon,
                 theron_v1_startup_phase_name(flow->phase),
                 world->party.champion_count,
                 (int)flow->companion_count,
                 level_receipt);
    }
    return THERON_STARTUP_OK;
}

Theron_StartupResult theron_v1_startup_return_to_stage_select_after_exit(
    Theron_V1_World *world,
    Theron_StartupFlow *flow,
    char *receipt,
    size_t receipt_cap) {
    Theron_DungeonID next;

    if (receipt && receipt_cap > 0u) {
        receipt[0] = '\0';
    }
    if (!world || !flow) {
        return THERON_STARTUP_ERR_NULL;
    }

    next = theron_v1_dungeon_exit(&world->progression);
    if (next == THERON_DUNGEON_INVALID &&
        !theron_v1_quest_complete(&world->progression)) {
        if (receipt && receipt_cap > 0u) {
            snprintf(receipt, receipt_cap, "dungeon exit rejected");
        }
        return THERON_STARTUP_ERR_DUNGEON_ENTRY;
    }

    theron_v1_party_dungeon_exit(&world->party);
    world->party.champion_count = 1;
    world->party.active_slot = THERON_CHAMPION_SLOT_THERON;
    world->current_dungeon = world->progression.current_dungeon;
    world->current_level = 0;
    world->dungeon_complete = 0;
    world->quest_items_in_dungeon = 0;
    memset(world->level_loaded, 0, sizeof(world->level_loaded));

    (void)theron_v1_startup_show_stage_select(
        flow,
        world->progression.current_dungeon);
    if (receipt && receipt_cap > 0u) {
        snprintf(receipt,
                 receipt_cap,
                 theron_v1_quest_complete(&world->progression)
                    ? "quest complete"
                    : "dungeon complete; next stage=%d",
                 (int)world->progression.current_dungeon);
    }
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
    case THERON_STARTUP_ERR_DUNGEON_ENTRY: return "dungeon-entry";
    case THERON_STARTUP_ERR_LEVEL_LOAD: return "level-load";
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

int theron_v1_startup_plan_for_action(
    const Theron_StartupAction *action,
    Theron_StartupActionPlan *out_plan) {

    if (!out_plan) {
        return 0;
    }
    theron_v1_startup_action_plan_init(out_plan);
    if (!action) {
        return 0;
    }

    out_plan->selected_dungeon = action->selected_dungeon;
    out_plan->cursor = action->cursor;
    out_plan->continue_focus = action->continue_focus ? 1 : 0;
    out_plan->mirror_index = action->mirror_index;

    switch (action->kind) {
    case THERON_STARTUP_ACTION_NONE:
        out_plan->kind = THERON_STARTUP_PLAN_IGNORE;
        return 1;
    case THERON_STARTUP_ACTION_RETURN_TO_LAUNCHER:
        out_plan->kind = THERON_STARTUP_PLAN_RETURN_TO_LAUNCHER;
        out_plan->status_scope = "RETURN";
        out_plan->status = "BACK TO LAUNCHER";
        return 1;
    case THERON_STARTUP_ACTION_SHOW_STAGE_SELECT:
        out_plan->kind = THERON_STARTUP_PLAN_SHOW_STAGE_SELECT;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "STAGE SELECT";
        return 1;
    case THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR:
        out_plan->kind = THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "STAGE CURSOR";
        return 1;
    case THERON_STARTUP_ACTION_CONTINUE_SAVE:
        out_plan->kind = THERON_STARTUP_PLAN_CONTINUE_SAVE;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "CONTINUE LOADED";
        out_plan->failure_status = "CONTINUE FAILED";
        return 1;
    case THERON_STARTUP_ACTION_CHOOSE_STAGE:
        out_plan->kind = THERON_STARTUP_PLAN_CHOOSE_STAGE;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "SOUL ROOM";
        out_plan->failure_status = "STAGE LOCKED";
        return 1;
    case THERON_STARTUP_ACTION_MOVE_SOUL_CURSOR:
        out_plan->kind = THERON_STARTUP_PLAN_MOVE_SOUL_CURSOR;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "SOUL ROOM CURSOR";
        return 1;
    case THERON_STARTUP_ACTION_TOGGLE_MIRROR:
        out_plan->kind = THERON_STARTUP_PLAN_TOGGLE_MIRROR;
        out_plan->status_scope = "STARTUP";
        out_plan->status = "HERO RESURRECTED";
        out_plan->alternate_status = "HERO RELEASED";
        out_plan->failure_status = "SOUL ROOM ERROR";
        return 1;
    case THERON_STARTUP_ACTION_ENTER_FORCEFIELD:
        out_plan->kind = THERON_STARTUP_PLAN_ENTER_FORCEFIELD;
        out_plan->status_scope = "BOOT";
        out_plan->status = "THERON READY";
        out_plan->failure_status = "FORCEFIELD FAILED";
        return 1;
    default:
        break;
    }
    return 0;
}

int theron_v1_startup_execute_flow_plan(
    const Theron_StartupActionPlan *plan,
    const Theron_DungeonProgression *progression,
    Theron_StartupFlow *flow,
    Theron_StartupExecution *out_execution) {

    Theron_StartupResult result = THERON_STARTUP_OK;
    int selected = 0;

    if (out_execution) {
        theron_v1_startup_execution_init(out_execution);
    }
    if (!plan || !flow) {
        if (out_execution) {
            out_execution->result = THERON_STARTUP_ERR_NULL;
        }
        return 0;
    }

    switch (plan->kind) {
    case THERON_STARTUP_PLAN_SHOW_STAGE_SELECT:
        result = theron_v1_startup_show_stage_select(flow,
                                                     plan->selected_dungeon);
        if (out_execution) {
            out_execution->handled = 1;
            out_execution->flow_changed = result == THERON_STARTUP_OK;
            out_execution->cursor_changed = 1;
            out_execution->cursor = plan->cursor;
            out_execution->continue_focus_changed = 1;
            out_execution->continue_focus = plan->continue_focus;
            out_execution->result = result;
        }
        return 1;
    case THERON_STARTUP_PLAN_MOVE_STAGE_CURSOR:
        flow->selected_dungeon = plan->selected_dungeon;
        if (out_execution) {
            out_execution->handled = 1;
            out_execution->flow_changed = 1;
            out_execution->continue_focus_changed = 1;
            out_execution->continue_focus = plan->continue_focus;
            out_execution->result = THERON_STARTUP_OK;
        }
        return 1;
    case THERON_STARTUP_PLAN_CHOOSE_STAGE:
        result = theron_v1_startup_choose_stage(flow,
                                                progression,
                                                plan->selected_dungeon);
        if (out_execution) {
            out_execution->handled = 1;
            out_execution->flow_changed = result == THERON_STARTUP_OK;
            out_execution->cursor_changed = result == THERON_STARTUP_OK;
            out_execution->cursor = 0;
            out_execution->continue_focus_changed =
                result == THERON_STARTUP_OK;
            out_execution->continue_focus = 0;
            out_execution->result = result;
        }
        return 1;
    case THERON_STARTUP_PLAN_MOVE_SOUL_CURSOR:
        if (out_execution) {
            out_execution->handled = 1;
            out_execution->cursor_changed = 1;
            out_execution->cursor = plan->cursor;
            out_execution->result = THERON_STARTUP_OK;
        }
        return 1;
    case THERON_STARTUP_PLAN_TOGGLE_MIRROR:
        result = theron_v1_startup_toggle_mirror(flow,
                                                 plan->mirror_index,
                                                 &selected);
        if (out_execution) {
            out_execution->handled = 1;
            out_execution->flow_changed = result == THERON_STARTUP_OK;
            out_execution->mirror_selected = selected;
            out_execution->result = result;
        }
        return 1;
    case THERON_STARTUP_PLAN_IGNORE:
    case THERON_STARTUP_PLAN_RETURN_TO_LAUNCHER:
    case THERON_STARTUP_PLAN_CONTINUE_SAVE:
    case THERON_STARTUP_PLAN_ENTER_FORCEFIELD:
    default:
        if (out_execution) {
            out_execution->handled = 0;
            out_execution->result = THERON_STARTUP_OK;
        }
        return 0;
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
