#include "firestaff/dm1/v1/chest/dm1_v1_chest_scroll_wheel_resurrect_confirmation_pc34_compat.h"

#include <stdint.h>
#include <string.h>

/*
 * DM1 V1 contract-only source-lock gate: C040 mirror candidate panel is open
 * on the leader and already in the F0282 resurrect-confirmation window while
 * a different non-leader champion owns the currently open G0426 chest. A
 * scroll-wheel pickup over C538 must reach the mouse/queue path, then be
 * rejected before the C30+/leader-hand exchange because F0282 owns the
 * candidate command and leader hand until it consumes C160/C161/C162.
 *
 * ReDMCSB anchors:
 * REVIVE.C F0282:744-806, REVIVE.C F0280:124-132, REVIVE.C F0281;
 * CHEST.C F0333:30-67 and CHEST.C F0334:117-132; CHAMPION.C
 * F0297:243-298, CHAMPION.C F0298:270-298, CHAMPION.C F0300:511-584,
 * CHAMPION.C F0301:606-660, CHAMPION.C F0302:662-713; COMMAND.C
 * F0359:1985-1990, COMMAND.C F0378:1973-1983, COMMAND.C F0380:2045-2159;
 * PANEL.C F0344/F0345 and
 * F0346/F0347:1619-1657; UTAMSCR.C F0077/F0078:141-150; DEFS.H:338-340,
 * 810-817, 873/876, 1878, 2088, 2200, 3001-3008, 3906-3913, 4205-4207,
 * 5694, 5876-5881.
 *
 * Sibling non-overlap: C040+C545 pickup/drop panel-live gates,
 * mirror_candidate_scroll_pickup_non_leader_panel_live, and
 * mirror_candidate_resurrect_reselect_with_inventory_pickup cover adjacent
 * browse-state and inventory pickup races, not this F0282 confirmation
 * ownership window with a different non-leader open chest.
 */

enum PanelModePc34 {
    kPanelModeBrowsePc34 = 0,
    kPanelModeConfirmPc34 = 1,
    kPanelModeClearedPc34 = 2
};

typedef struct {
    int alive;
    int c30_chain[DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34];
} ChampionPc34;

typedef struct {
    ChampionPc34 champions[DM1_V1_CSWRC_CHAMPION_COUNT_PC34];
    int chest_slots[DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34];
    int leader_index;
    int chest_owner_index;
    int open_chest_thing;
    int leader_hand_thing;
    int candidate_ordinal;
    int candidate_command;
    int candidate_command_consumed;
    int c040_panel_open;
    enum PanelModePc34 panel_mode;
    int f0077_wheel_source;
    int f0078_wheel_read;
    int f0378_queue_write;
    int f0380_queue_dispatch;
    int f0282_gate_reject;
    int f0297_put_count;
    int f0298_remove_count;
    int f0300_remove_count;
    int f0301_add_count;
    int f0302_dispatch_count;
    int f0333_open_count;
    int f0334_close_count;
    int queued_slot;
    int queued_thing;
} RuntimePc34;

static Dm1V1ChestScrollWheelResurrectConfirmationResultPc34 g_last;

static const char s_source_evidence[] =
    "REVIVE.C F0282:744-806 confirmation owns C160/C161/C162 and leader hand\n"
    "REVIVE.C F0280:124-132 candidate publish and F0281 state set/clear\n"
    "CHEST.C F0333:30-67 opens G0426 into G0425 C537..C544 slots\n"
    "CHEST.C F0334:117-132 closes and recompacts G0425 into the container\n"
    "CHAMPION.C F0297:243-298 and CHAMPION.C F0298:270-298 hand put/remove\n"
    "CHAMPION.C F0300:511-584, CHAMPION.C F0301:606-660, "
    "CHAMPION.C F0302:662-713 C30+ exchange\n"
    "COMMAND.C F0359:1985-1990 empty-hand gated M568/C040 dispatch\n"
    "COMMAND.C F0378:1973-1983 scroll pickup panel dispatch\n"
    "COMMAND.C F0380:2045-2159 queued command dispatch\n"
    "PANEL.C F0344/F0345 click routing; F0346/F0347:1619-1657 redraw C040\n"
    "UTAMSCR.C F0077/F0078:141-150 pointer update bracket\n"
    "DEFS.H:338-340 C160/C161/C162; 810-817 C30..C37; 873/876 M516; "
    "1878 M070; 2088 C10; 2200 C040; 3001-3008 M568/M569; "
    "3906-3913 C537..C544; 4205-4207 floor zones; 5694 G0299; "
    "5876-5881 G0423/G0425/G0426";

static uint32_t mix_hash(uint32_t hash, uint32_t value)
{
    hash ^= value + 0x9e3779b9u + (hash << 6) + (hash >> 2);
    hash *= 16777619u;
    return hash;
}

static void hash_value(int tag, int value)
{
    g_last.deterministic_hash =
        mix_hash(g_last.deterministic_hash, (uint32_t)tag);
    g_last.deterministic_hash =
        mix_hash(g_last.deterministic_hash, (uint32_t)value);
}

static int text_has(const char *needle)
{
    return strstr(s_source_evidence, needle) != 0;
}

static void check_true(int condition, int tag, int value)
{
    ++g_last.assertions;
    if (!condition) {
        ++g_last.failures;
    }
    hash_value(tag, value ^ (condition ? 0x5a5a : 0xa5a5));
}

static void check_eq(int actual, int expected, int tag)
{
    ++g_last.assertions;
    if (actual != expected) {
        ++g_last.failures;
    }
    hash_value(tag, actual);
    hash_value(tag + 1000, expected);
}

static void copy_ints(int dst[], const int src[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        dst[i] = src[i];
    }
}

static int arrays_equal(const int a[], const int b[], int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (a[i] != b[i]) {
            return 0;
        }
    }
    return 1;
}

static int count_chest_items(const RuntimePc34 *runtime)
{
    int i;
    int count = 0;

    for (i = 0; i < DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34; ++i) {
        if (runtime->chest_slots[i] != DM1_V1_CSWRC_NONE_PC34) {
            ++count;
        }
    }
    return count;
}

static void seed_runtime(RuntimePc34 *runtime, enum PanelModePc34 mode)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->leader_index = 0;
    runtime->chest_owner_index = 1;
    runtime->open_chest_thing = 0x6b41;
    runtime->leader_hand_thing = DM1_V1_CSWRC_NONE_PC34;
    runtime->candidate_ordinal =
        mode == kPanelModeClearedPc34 ? 0 : 1;
    runtime->candidate_command =
        mode == kPanelModeConfirmPc34 ?
            DM1_V1_CSWRC_C160_RESURRECT_PC34 :
            DM1_V1_CSWRC_NONE_PC34;
    runtime->c040_panel_open = mode != kPanelModeClearedPc34;
    runtime->panel_mode = mode;
    runtime->queued_slot = 1;
    runtime->queued_thing = DM1_V1_CSWRC_NONE_PC34;
    runtime->f0333_open_count = 1;

    for (i = 0; i < DM1_V1_CSWRC_CHAMPION_COUNT_PC34; ++i) {
        runtime->champions[i].alive = i == 0 ? 0 : 1;
    }
    for (i = 0; i < DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34; ++i) {
        runtime->champions[0].c30_chain[i] = 0x7100 + i;
    }
    for (i = 0; i < DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34; ++i) {
        runtime->chest_slots[i] = i < 4 ? 0x7200 + i :
            DM1_V1_CSWRC_NONE_PC34;
    }
}

static void wheel_queue_pickup(RuntimePc34 *runtime, int slot)
{
    ++runtime->f0077_wheel_source;
    ++runtime->f0378_queue_write;
    runtime->queued_slot = slot;
    runtime->queued_thing = runtime->chest_slots[slot];
}

static int process_queued_pickup(RuntimePc34 *runtime)
{
    int picked;

    ++runtime->f0078_wheel_read;
    ++runtime->f0380_queue_dispatch;

    if (runtime->panel_mode == kPanelModeConfirmPc34 &&
        runtime->c040_panel_open &&
        runtime->candidate_command != DM1_V1_CSWRC_NONE_PC34) {
        ++runtime->f0282_gate_reject;
        return 0;
    }

    ++runtime->f0302_dispatch_count;
    if (runtime->queued_slot < 0 ||
        runtime->queued_slot >= DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34 ||
        runtime->leader_hand_thing != DM1_V1_CSWRC_NONE_PC34 ||
        runtime->chest_slots[runtime->queued_slot] ==
            DM1_V1_CSWRC_NONE_PC34) {
        return 0;
    }

    picked = runtime->chest_slots[runtime->queued_slot];
    ++runtime->f0300_remove_count;
    runtime->chest_slots[runtime->queued_slot] = DM1_V1_CSWRC_NONE_PC34;
    ++runtime->f0301_add_count;
    runtime->leader_hand_thing = picked;
    ++runtime->f0297_put_count;
    return 1;
}

static void record_common_route(const RuntimePc34 *runtime)
{
    g_last.scroll_events_reached_f0077 += runtime->f0077_wheel_source;
    g_last.scroll_events_reached_f0078 += runtime->f0078_wheel_read;
    g_last.scroll_events_queued_to_f0378 += runtime->f0378_queue_write;
    g_last.queue_dispatches_f0380 += runtime->f0380_queue_dispatch;
}

static void run_positive_confirmation_reject(void)
{
    RuntimePc34 runtime;
    int before_chest[DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34];
    int before_c30[DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34];
    int before_hand;
    int before_command;
    int accepted;

    seed_runtime(&runtime, kPanelModeConfirmPc34);
    copy_ints(before_chest, runtime.chest_slots,
              DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34);
    copy_ints(before_c30, runtime.champions[0].c30_chain,
              DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34);
    before_hand = runtime.leader_hand_thing;
    before_command = runtime.candidate_command;

    check_eq(runtime.leader_index, 0, 1);
    check_eq(runtime.chest_owner_index, 1, 2);
    check_eq(runtime.champions[0].alive, 0, 3);
    check_eq(runtime.champions[1].alive, 1, 4);
    check_eq(runtime.champions[2].alive, 1, 5);
    check_eq(runtime.champions[3].alive, 1, 6);
    check_true(runtime.chest_owner_index != runtime.leader_index, 7, 1);
    check_true(count_chest_items(&runtime) >= 3, 8, 1);
    check_eq(runtime.leader_hand_thing, DM1_V1_CSWRC_NONE_PC34, 9);

    wheel_queue_pickup(&runtime, 1);
    accepted = process_queued_pickup(&runtime);
    record_common_route(&runtime);

    check_eq(accepted, 0, 10);
    check_eq(runtime.f0282_gate_reject, 1, 11);
    check_eq(runtime.f0302_dispatch_count, 0, 12);
    check_eq(runtime.f0300_remove_count, 0, 13);
    check_eq(runtime.f0301_add_count, 0, 14);
    check_eq(runtime.f0297_put_count, 0, 15);
    check_eq(runtime.f0298_remove_count, 0, 16);
    check_eq(runtime.f0333_open_count, 1, 17);
    check_eq(runtime.f0334_close_count, 0, 18);
    check_eq(runtime.c040_panel_open, 1, 19);
    check_eq(runtime.panel_mode, (int)kPanelModeConfirmPc34, 20);
    check_eq(runtime.candidate_command_consumed, 0, 21);

    if (runtime.leader_hand_thing == before_hand) {
        ++g_last.leader_hand_unchanged_checks;
    }
    if (arrays_equal(runtime.champions[0].c30_chain, before_c30,
                     DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34)) {
        ++g_last.c30_chain_unchanged_checks;
    }
    if (arrays_equal(runtime.chest_slots, before_chest,
                     DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34)) {
        ++g_last.g0426_chain_unchanged_checks;
    }
    if (runtime.candidate_command == before_command &&
        runtime.candidate_command_consumed == 0) {
        ++g_last.candidate_command_unchanged_checks;
    }

    check_eq(g_last.leader_hand_unchanged_checks, 1, 22);
    check_eq(g_last.c30_chain_unchanged_checks, 1, 23);
    check_eq(g_last.g0426_chain_unchanged_checks, 1, 24);
    check_eq(g_last.candidate_command_unchanged_checks, 1, 25);
    check_eq(runtime.f0077_wheel_source, 1, 26);
    check_eq(runtime.f0078_wheel_read, 1, 27);
    check_eq(runtime.f0378_queue_write, 1, 28);
    check_eq(runtime.f0380_queue_dispatch, 1, 29);
    ++g_last.positive_rejections;
    ++g_last.f0282_gate_rejections;
}

static void run_allowed_case(enum PanelModePc34 mode)
{
    RuntimePc34 runtime;
    int before_thing;
    int accepted;

    seed_runtime(&runtime, mode);
    before_thing = runtime.chest_slots[1];
    wheel_queue_pickup(&runtime, 1);
    accepted = process_queued_pickup(&runtime);
    record_common_route(&runtime);

    check_eq(accepted, 1, 40 + (int)mode);
    check_eq(runtime.leader_hand_thing, before_thing, 50 + (int)mode);
    check_eq(runtime.chest_slots[1], DM1_V1_CSWRC_NONE_PC34,
             60 + (int)mode);
    check_eq(runtime.f0302_dispatch_count, 1, 70 + (int)mode);
    check_eq(runtime.f0301_add_count, 1, 80 + (int)mode);
    check_eq(runtime.f0282_gate_reject, 0, 90 + (int)mode);
    check_eq(runtime.candidate_command_consumed, 0, 100 + (int)mode);

    ++g_last.f0301_reached_when_allowed;
    ++g_last.f0302_reached_when_allowed;
    if (mode == kPanelModeBrowsePc34) {
        ++g_last.negative_browse_allowed;
        check_eq(runtime.c040_panel_open, 1, 110);
        check_eq(runtime.candidate_ordinal, 1, 111);
    } else {
        ++g_last.negative_cancelled_allowed;
        check_eq(runtime.c040_panel_open, 0, 120);
        check_eq(runtime.candidate_ordinal, 0, 121);
    }
}

static void check_source_evidence(void)
{
    check_true(text_has("REVIVE.C F0282:744-806"), 200, 1);
    check_true(text_has("REVIVE.C F0280:124-132"), 201, 1);
    check_true(text_has("F0281"), 202, 1);
    check_true(text_has("CHEST.C F0333:30-67"), 203, 1);
    check_true(text_has("CHEST.C F0334:117-132"), 204, 1);
    check_true(text_has("CHAMPION.C F0297:243-298"), 205, 1);
    check_true(text_has("CHAMPION.C F0298:270-298"), 206, 1);
    check_true(text_has("CHAMPION.C F0300:511-584"), 207, 1);
    check_true(text_has("CHAMPION.C F0301:606-660"), 208, 1);
    check_true(text_has("CHAMPION.C F0302:662-713"), 209, 1);
    check_true(text_has("COMMAND.C F0359:1985-1990"), 210, 1);
    check_true(text_has("COMMAND.C F0378:1973-1983"), 211, 1);
    check_true(text_has("COMMAND.C F0380:2045-2159"), 212, 1);
    check_true(text_has("PANEL.C F0344/F0345"), 213, 1);
    check_true(text_has("F0346/F0347:1619-1657"), 214, 1);
    check_true(text_has("UTAMSCR.C F0077/F0078:141-150"), 215, 1);
    check_true(text_has("DEFS.H:338-340"), 216, 1);
    check_true(text_has("810-817 C30..C37"), 217, 1);
    check_true(text_has("873/876 M516"), 218, 1);
    check_true(text_has("1878 M070"), 219, 1);
    check_true(text_has("2088 C10"), 220, 1);
    check_true(text_has("2200 C040"), 221, 1);
    check_true(text_has("3001-3008 M568/M569"), 222, 1);
    check_true(text_has("3906-3913 C537..C544"), 223, 1);
    check_true(text_has("4205-4207 floor zones"), 224, 1);
    check_true(text_has("5694 G0299"), 225, 1);
    check_true(text_has("5876-5881 G0423/G0425/G0426"), 226, 1);
}

int run_dm1_v1_chest_scroll_wheel_resurrect_confirmation_self_test(void)
{
    memset(&g_last, 0, sizeof(g_last));
    g_last.deterministic_hash = 2166136261u;

    check_source_evidence();
    run_positive_confirmation_reject();
    run_allowed_case(kPanelModeBrowsePc34);
    run_allowed_case(kPanelModeClearedPc34);

    check_eq(g_last.positive_rejections, 1, 300);
    check_eq(g_last.negative_browse_allowed, 1, 301);
    check_eq(g_last.negative_cancelled_allowed, 1, 302);
    check_eq(g_last.f0301_reached_when_allowed, 2, 303);
    check_eq(g_last.f0302_reached_when_allowed, 2, 304);
    check_true(g_last.deterministic_hash != 0u, 305, 1);
    return g_last.failures == 0 ? 0 : 1;
}

const Dm1V1ChestScrollWheelResurrectConfirmationResultPc34 *
dm1_v1_chest_scroll_wheel_resurrect_confirmation_last_self_test_result_pc34(
    void)
{
    return &g_last;
}
