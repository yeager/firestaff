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

const Theron_StartupMirrorMeta *theron_v1_startup_mirror_meta(int mirror_index) {
    if (mirror_index < 0 || mirror_index >= THERON_STARTUP_HERO_MIRROR_COUNT) {
        return NULL;
    }
    return &g_tqr_mirror_meta[mirror_index];
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
    flow->phase = THERON_STARTUP_PHASE_STAGE_SELECT;
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

Theron_StartupResult theron_v1_startup_enter_forcefield(
    Theron_StartupFlow *flow,
    Theron_V1_Party *party) {

    int slot = 1;
    int mirror;

    if (!flow || !party) {
        return THERON_STARTUP_ERR_NULL;
    }
    if (flow->selected_dungeon == THERON_DUNGEON_INVALID ||
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
    default: return "unknown";
    }
}

const char *theron_v1_startup_flow_source_evidence(void) {
    return "Theron's Quest manual: choose stage, Soul Room, seven hero mirrors, "
           "resurrect up to three heroes, enter central forcefield; "
           "manual/dmweb champion list: Hakar, Mara, Tiran, Linos, Dotan, "
           "Hexa, Pental as the seven selectable heroes; "
           "dmweb Theron's Quest: Theron plus three champions, companions reset "
           "after dungeon completion; PC Engine Software Bible: console control "
           "profile for Dungeon Master: Theron's Quest.";
}
