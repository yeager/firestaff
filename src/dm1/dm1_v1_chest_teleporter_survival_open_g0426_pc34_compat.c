#include "firestaff/dm1/v1/chest/dm1_v1_chest_teleporter_survival_open_g0426_pc34_compat.h"

#include <stdint.h>
#include <string.h>

enum {
    DM1_OPEN_G0426_SLOT_COUNT = 8,
    DM1_OPEN_G0426_VISIBLE_SLOT_COUNT = 3,
    DM1_OPEN_G0426_THING_NONE = 0xFFFF,
    DM1_OPEN_G0426_CHEST_THING = 0x7426,
    DM1_OPEN_G0426_LEADER_HAND_THING = 0x6111,
    DM1_OPEN_G0426_MAP_A = 4,
    DM1_OPEN_G0426_MAP_B = 6,
    DM1_OPEN_G0426_SCOPE_CREATURES = 0x0001,
    DM1_OPEN_G0426_SCOPE_OBJECTS_OR_PARTY = 0x0002
};

typedef struct {
    int thing;
    int count;
    int stackable;
    int icon_pixels;
} OpenG0426ItemPc34;

typedef struct {
    int scope;
    int audible;
    int target_map;
    int target_x;
    int target_y;
} OpenG0426TeleporterPc34;

typedef struct {
    int current_map;
    int party_x;
    int party_y;
    int open_chest;
    OpenG0426ItemPc34 chain[DM1_OPEN_G0426_SLOT_COUNT];
    OpenG0426ItemPc34 visible_slots[DM1_OPEN_G0426_SLOT_COUNT];
    OpenG0426ItemPc34 leader_hand;
    int command_dispatches;
    int cell_fetches;
    int teleporter_activations;
    int audible_buzzes;
    int mutation_rejections;
} OpenG0426RuntimePc34;

static DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34 g_last;

static const char s_source_anchors[] =
    "CHEST.C F0333:30-67 opens the chest panel, anchors G0426, and copies the "
    "visible C537-C544 chain from the container list; "
    "CHEST.C F0334:113-132 rewrites the chest slot chain from G0425 while "
    "closing G0426; "
    "MOVE.C F0291:4350-4420 party-scoped audible teleporter handoff changes "
    "party location without owning inventory globals; "
    "DUNGEON.C F0163:1769-1838 fetches/links the preserved cell chain; "
    "COMMAND.C F0380 dispatches queued movement into the move/teleporter path";

static OpenG0426ItemPc34 make_item(int thing,
                                   int count,
                                   int stackable,
                                   int icon_pixels)
{
    OpenG0426ItemPc34 item;

    item.thing = thing;
    item.count = count;
    item.stackable = stackable;
    item.icon_pixels = icon_pixels;
    return item;
}

static int item_is_empty(const OpenG0426ItemPc34* item)
{
    return !item || item->thing == DM1_OPEN_G0426_THING_NONE;
}

static void clear_slots(OpenG0426ItemPc34* slots)
{
    int i;

    for (i = 0; i < DM1_OPEN_G0426_SLOT_COUNT; ++i) {
        slots[i] = make_item(DM1_OPEN_G0426_THING_NONE, 0, 0, 0);
    }
}

static void init_runtime(OpenG0426RuntimePc34* runtime)
{
    memset(runtime, 0, sizeof(*runtime));
    runtime->current_map = DM1_OPEN_G0426_MAP_A;
    runtime->party_x = 11;
    runtime->party_y = 17;
    runtime->open_chest = DM1_OPEN_G0426_THING_NONE;
    clear_slots(runtime->chain);
    clear_slots(runtime->visible_slots);
    runtime->chain[0] = make_item(0x4401, 1, 0, 29);
    runtime->chain[1] = make_item(0x4402, 2, 1, 31);
    runtime->chain[2] = make_item(0x4403, 1, 0, 37);
    runtime->leader_hand =
        make_item(DM1_OPEN_G0426_LEADER_HAND_THING, 1, 0, 43);
}

static int visible_slot_count(const OpenG0426ItemPc34* slots)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_OPEN_G0426_SLOT_COUNT; ++i) {
        if (!item_is_empty(&slots[i])) {
            ++count;
        }
    }
    return count;
}

static int visible_chain_pixels(const OpenG0426ItemPc34* slots)
{
    int pixels = 0;
    int i;

    for (i = 0; i < DM1_OPEN_G0426_SLOT_COUNT; ++i) {
        if (!item_is_empty(&slots[i])) {
            pixels += slots[i].icon_pixels;
        }
    }
    return pixels;
}

static int slot_chain_matches_seed(const OpenG0426ItemPc34* slots)
{
    return slots[0].thing == 0x4401 && slots[0].count == 1 &&
           slots[1].thing == 0x4402 && slots[1].count == 2 &&
           slots[1].stackable == 1 &&
           slots[2].thing == 0x4403 && slots[2].count == 1 &&
           item_is_empty(&slots[3]) && item_is_empty(&slots[4]) &&
           item_is_empty(&slots[5]) && item_is_empty(&slots[6]) &&
           item_is_empty(&slots[7]);
}

static void open_chest_g0426(OpenG0426RuntimePc34* runtime)
{
    int i;

    /*
     * ReDMCSB CHEST.C F0333 lines 30-67 is the open-panel contract: set
     * G0426, draw the C025 open-chest panel, then materialize the first
     * eight linked entries into G0425/C537-C544 without touching the leader
     * hand.
     */
    runtime->open_chest = DM1_OPEN_G0426_CHEST_THING;
    for (i = 0; i < DM1_OPEN_G0426_SLOT_COUNT; ++i) {
        runtime->visible_slots[i] = runtime->chain[i];
    }
}

static int teleporter_cell_accepts_party(OpenG0426RuntimePc34* runtime,
                                         const OpenG0426TeleporterPc34* t)
{
    /*
     * ReDMCSB DUNGEON.C F0163 lines 1769-1838 is the cell-chain anchor used
     * by this contract gate: fetching/linking the cell chain is separate from
     * the live G0426/G0425 inventory panel state.
     */
    ++runtime->cell_fetches;
    return (t->scope & DM1_OPEN_G0426_SCOPE_OBJECTS_OR_PARTY) != 0;
}

static int process_command_queue_move_f0380_to_move_f0291(
    OpenG0426RuntimePc34* runtime,
    const OpenG0426TeleporterPc34* t)
{
    /*
     * ReDMCSB COMMAND.C F0380 dequeues and dispatches movement commands into
     * the move path. The party teleporter handoff cited for this gate is
     * MOVE.C F0291:4350-4420: an audible, party-scoped teleporter updates
     * party position/sound state, not CHEST.C's G0426 or the leader hand.
     */
    ++runtime->command_dispatches;
    if (!teleporter_cell_accepts_party(runtime, t)) {
        ++runtime->mutation_rejections;
        return 0;
    }
    runtime->current_map = t->target_map;
    runtime->party_x = t->target_x;
    runtime->party_y = t->target_y;
    ++runtime->teleporter_activations;
    if (t->audible) {
        ++runtime->audible_buzzes;
    }
    return 1;
}

static int leader_hand_is_seed(const OpenG0426RuntimePc34* runtime)
{
    return runtime->leader_hand.thing == DM1_OPEN_G0426_LEADER_HAND_THING &&
           runtime->leader_hand.count == 1 &&
           runtime->leader_hand.stackable == 0 &&
           runtime->leader_hand.icon_pixels == 43;
}

static void add_hash(uint64_t* hash, uint64_t value)
{
    *hash ^= value;
    *hash *= UINT64_C(1099511628211);
}

static uint64_t compute_hash(
    const DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34* r)
{
    uint64_t hash = UINT64_C(1469598103934665603);

    add_hash(&hash, (uint64_t)r->g0426_kept_open);
    add_hash(&hash, (uint64_t)r->leader_hand_preserved);
    add_hash(&hash, (uint64_t)r->chest_slot_count_preserved);
    add_hash(&hash, (uint64_t)r->teleporter_activations);
    add_hash(&hash, (uint64_t)r->mutation_rejections);
    add_hash(&hash, (uint64_t)r->command_dispatches);
    add_hash(&hash, (uint64_t)r->audible_buzzes);
    add_hash(&hash, (uint64_t)r->creature_only_transition_blocked);
    add_hash(&hash, (uint64_t)r->stacked_pair_preserved);
    add_hash(&hash, (uint64_t)r->visible_chain_pixels_preserved);
    add_hash(&hash, (uint64_t)r->initial_chest_slot_count);
    add_hash(&hash, (uint64_t)r->final_chest_slot_count);
    add_hash(&hash, (uint64_t)r->initial_stack_count);
    add_hash(&hash, (uint64_t)r->final_stack_count);
    add_hash(&hash, (uint64_t)r->initial_visible_chain_pixels);
    add_hash(&hash, (uint64_t)r->final_visible_chain_pixels);
    add_hash(&hash, (uint64_t)r->initial_leader_hand_thing);
    add_hash(&hash, (uint64_t)r->final_leader_hand_thing);
    add_hash(&hash, (uint64_t)r->final_map_index);
    add_hash(&hash, (uint64_t)r->final_map_x);
    add_hash(&hash, (uint64_t)r->final_map_y);
    return hash;
}

static void check_result(int condition,
                         DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34* r)
{
    ++r->assertions;
    if (!condition) {
        ++r->failures;
    }
}

int run_dm1_v1_chest_teleporter_survival_open_g0426_self_test(void)
{
    OpenG0426RuntimePc34 runtime;
    OpenG0426TeleporterPc34 party_teleporter;
    OpenG0426TeleporterPc34 creature_teleporter;
    int initial_slots;
    int initial_pixels;
    int initial_stack_count;
    int map_after_party;
    int x_after_party;
    int y_after_party;
    int i;

    memset(&g_last, 0, sizeof(g_last));
    g_last.contract_only = 1;
    g_last.source_anchors = s_source_anchors;

    init_runtime(&runtime);
    open_chest_g0426(&runtime);
    initial_slots = visible_slot_count(runtime.visible_slots);
    initial_pixels = visible_chain_pixels(runtime.visible_slots);
    initial_stack_count = runtime.visible_slots[1].count;

    party_teleporter.scope = DM1_OPEN_G0426_SCOPE_OBJECTS_OR_PARTY;
    party_teleporter.audible = 1;
    party_teleporter.target_map = DM1_OPEN_G0426_MAP_B;
    party_teleporter.target_x = 3;
    party_teleporter.target_y = 9;
    process_command_queue_move_f0380_to_move_f0291(&runtime,
                                                    &party_teleporter);
    map_after_party = runtime.current_map;
    x_after_party = runtime.party_x;
    y_after_party = runtime.party_y;

    creature_teleporter.scope = DM1_OPEN_G0426_SCOPE_CREATURES;
    creature_teleporter.audible = 1;
    creature_teleporter.target_map = DM1_OPEN_G0426_MAP_A;
    creature_teleporter.target_x = 19;
    creature_teleporter.target_y = 5;
    process_command_queue_move_f0380_to_move_f0291(&runtime,
                                                    &creature_teleporter);

    g_last.g0426_kept_open =
        runtime.open_chest == DM1_OPEN_G0426_CHEST_THING;
    g_last.leader_hand_preserved = leader_hand_is_seed(&runtime);
    g_last.chest_slot_count_preserved =
        visible_slot_count(runtime.visible_slots) == initial_slots;
    g_last.teleporter_activations = runtime.teleporter_activations;
    g_last.mutation_rejections = runtime.mutation_rejections;
    g_last.command_dispatches = runtime.command_dispatches;
    g_last.audible_buzzes = runtime.audible_buzzes;
    g_last.creature_only_transition_blocked =
        runtime.current_map == map_after_party &&
        runtime.party_x == x_after_party &&
        runtime.party_y == y_after_party &&
        runtime.teleporter_activations == 1;
    g_last.stacked_pair_preserved =
        runtime.visible_slots[1].thing == 0x4402 &&
        runtime.visible_slots[1].count == initial_stack_count &&
        runtime.visible_slots[1].stackable == 1;
    g_last.visible_chain_pixels_preserved =
        visible_chain_pixels(runtime.visible_slots) == initial_pixels;
    g_last.initial_chest_slot_count = initial_slots;
    g_last.final_chest_slot_count = visible_slot_count(runtime.visible_slots);
    g_last.initial_stack_count = initial_stack_count;
    g_last.final_stack_count = runtime.visible_slots[1].count;
    g_last.initial_visible_chain_pixels = initial_pixels;
    g_last.final_visible_chain_pixels =
        visible_chain_pixels(runtime.visible_slots);
    g_last.initial_leader_hand_thing = DM1_OPEN_G0426_LEADER_HAND_THING;
    g_last.final_leader_hand_thing = runtime.leader_hand.thing;
    g_last.final_map_index = runtime.current_map;
    g_last.final_map_x = runtime.party_x;
    g_last.final_map_y = runtime.party_y;

    check_result(g_last.contract_only == 1, &g_last);
    check_result(strstr(s_source_anchors, "CHEST.C F0333") != NULL, &g_last);
    check_result(strstr(s_source_anchors, "CHEST.C F0334") != NULL, &g_last);
    check_result(strstr(s_source_anchors, "MOVE.C F0291:4350-4420") != NULL,
                 &g_last);
    check_result(strstr(s_source_anchors, "DUNGEON.C F0163:1769-1838") != NULL,
                 &g_last);
    check_result(strstr(s_source_anchors, "COMMAND.C F0380") != NULL, &g_last);
    check_result(runtime.open_chest == DM1_OPEN_G0426_CHEST_THING, &g_last);
    check_result(initial_slots == DM1_OPEN_G0426_VISIBLE_SLOT_COUNT, &g_last);
    check_result(initial_pixels == 97, &g_last);
    check_result(initial_stack_count == 2, &g_last);
    check_result(slot_chain_matches_seed(runtime.visible_slots), &g_last);
    check_result(leader_hand_is_seed(&runtime), &g_last);
    check_result(runtime.leader_hand.stackable == 0, &g_last);
    check_result(map_after_party == DM1_OPEN_G0426_MAP_B, &g_last);
    check_result(x_after_party == 3, &g_last);
    check_result(y_after_party == 9, &g_last);
    check_result(runtime.cell_fetches == 2, &g_last);
    check_result(runtime.command_dispatches == 2, &g_last);
    check_result(runtime.audible_buzzes == 1, &g_last);
    check_result(runtime.teleporter_activations == 1, &g_last);
    check_result(runtime.mutation_rejections == 1, &g_last);
    check_result(g_last.g0426_kept_open == 1, &g_last);
    check_result(g_last.leader_hand_preserved == 1, &g_last);
    check_result(g_last.chest_slot_count_preserved == 1, &g_last);
    check_result(g_last.creature_only_transition_blocked == 1, &g_last);
    check_result(g_last.stacked_pair_preserved == 1, &g_last);
    check_result(g_last.visible_chain_pixels_preserved == 1, &g_last);
    check_result(g_last.final_chest_slot_count == initial_slots, &g_last);
    check_result(g_last.final_stack_count == 2, &g_last);
    check_result(g_last.final_visible_chain_pixels == initial_pixels, &g_last);
    check_result(g_last.final_leader_hand_thing ==
                     DM1_OPEN_G0426_LEADER_HAND_THING,
                 &g_last);
    check_result(g_last.final_map_index == DM1_OPEN_G0426_MAP_B, &g_last);
    check_result(g_last.final_map_x == 3, &g_last);
    check_result(g_last.final_map_y == 9, &g_last);
    for (i = 0; i < DM1_OPEN_G0426_VISIBLE_SLOT_COUNT; ++i) {
        check_result(!item_is_empty(&runtime.visible_slots[i]), &g_last);
        check_result(runtime.visible_slots[i].icon_pixels > 0, &g_last);
    }
    for (i = DM1_OPEN_G0426_VISIBLE_SLOT_COUNT;
         i < DM1_OPEN_G0426_SLOT_COUNT; ++i) {
        check_result(item_is_empty(&runtime.visible_slots[i]), &g_last);
    }

    g_last.deterministic_hash = compute_hash(&g_last);
    check_result(g_last.deterministic_hash != 0, &g_last);
    return g_last.failures == 0 ? 1 : 0;
}

const DM1_V1_ChestTeleporterSurvivalOpenG0426SelfTestResultPc34*
dm1_v1_chest_teleporter_survival_open_g0426_last_self_test_result_pc34(void)
{
    return &g_last;
}
