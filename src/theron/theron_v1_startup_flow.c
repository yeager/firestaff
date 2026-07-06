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

static Theron_DungeonID tqr_startup_clamp_stage(Theron_DungeonID dungeon_id) {
    if (dungeon_id < THERON_DUNGEON_1_HALL_OF_RECORDS ||
        dungeon_id > THERON_DUNGEON_COUNT) {
        return THERON_DUNGEON_1_HALL_OF_RECORDS;
    }
    return dungeon_id;
}

Theron_StartupResult theron_v1_startup_handle_input(
    Theron_StartupPhase phase,
    Theron_DungeonID selected_dungeon,
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
            } else if (selected > THERON_DUNGEON_1_HALL_OF_RECORDS) {
                selected = (Theron_DungeonID)(selected - 1);
            } else if (has_continue) {
                out_action->continue_focus = 1;
            }
            out_action->selected_dungeon = selected;
            out_action->kind = THERON_STARTUP_ACTION_MOVE_STAGE_CURSOR;
            return THERON_STARTUP_OK;
        }
        if (input == THERON_STARTUP_INPUT_DOWN) {
            if (continue_focus) {
                out_action->continue_focus = 0;
            } else if (selected < THERON_DUNGEON_COUNT) {
                selected = (Theron_DungeonID)(selected + 1);
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
