#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    }
}

static void expect_u32(const char *id, uint32_t got, uint32_t want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=0x%08X want=0x%08X at %s\n", id,
               (unsigned)got, (unsigned)want, anchor);
        ++g_failures;
    }
}

static void expect_bool(const char *id, bool got, bool want,
                        const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n", id,
               needle ? needle : "(null)", anchor);
        ++g_failures;
    }
}

static const dm1_v1_cppbrs_event_row_pc34_compat_t *
require_row(const dm1_v1_cppbrs_model_pc34_compat_t *model,
            dm1_v1_cppbrs_event_pc34_compat_t event)
{
    const dm1_v1_cppbrs_event_row_pc34_compat_t *row =
        dm1_v1_cppbrs_find_event_pc34(model, event);
    expect_bool("find.event", row != NULL, true, "matrix event lookup");
    return row;
}

static void test_source_and_contract(void)
{
    dm1_v1_cppbrs_model_pc34_compat_t model;
    const char *source = dm1_v1_cppbrs_source_pc34();

    expect_bool("build.null", dm1_v1_cppbrs_build_model_pc34(NULL), false,
                "null safety");
    expect_bool("build.model", dm1_v1_cppbrs_build_model_pc34(&model), true,
                "contract model");
    expect_bool("contract.only", model.contract_only, true,
                "contract-only synthetic framebuffer");
    expect_bool("disjoint.pass673", model.disjoint_from_pass673_portrait_state,
                true, "does not duplicate pass673");
    expect_bool("disjoint.pass683", model.disjoint_from_pass683_name_color,
                true, "does not duplicate pass683");
    expect_bool("disjoint.pass764",
                model.disjoint_from_pass764_second_leader_slot_priority, true,
                "does not duplicate pass764");
    expect_bool("disjoint.pass765",
                model.disjoint_from_pass765_status_hand_rotation, true,
                "does not duplicate pass765");
    expect_bool("disjoint.f0354", model.disjoint_from_f0354_blit_gate, true,
                "does not duplicate portrait-box blit gate");

    expect_contains("source.f0291", source, "CHAMDRAW.C F0291:498-677",
                    "F0291 slot-box mapping");
    expect_contains("source.f0292.status", source, "CHAMDRAW.C F0292:757-815",
                    "F0292 status branch");
    expect_contains("source.f0292.inventory", source,
                    "arms only STATISTICS for that owner",
                    "F0292:810-812 inventory-owner continuation");
    expect_contains("source.f0292.name", source, "CHAMDRAW.C F0292:843-895",
                    "F0292 name-color cascade");
    expect_contains("source.f0292.stats", source, "CHAMDRAW.C F0292:898-935",
                    "F0292 statistics chrome");
    expect_contains("source.f0292.action", source,
                    "CHAMDRAW.C F0292:1080-1110",
                    "F0292 action hand and clear");
    expect_contains("source.f0293", source, "CHAMDRAW.C F0293:1117-1143",
                    "F0293 order");
    expect_contains("source.f0296", source, "CHAMDRAW.C F0295/F0296:1153-1260",
                    "F0296 changed icons");
    expect_contains("source.f0302", source, "CHAMPION.C F0302:662-714",
                    "F0302 hand pointer");
    expect_contains("source.defs", source, "C151..C154 67x29",
                    "DEFS.H C151..C154");
}

static void test_geometry(void)
{
    dm1_v1_cppbrs_model_pc34_compat_t model;
    int i;

    (void)dm1_v1_cppbrs_build_model_pc34(&model);
    for (i = 0; i < DM1_V1_CPPBRS_CHAMPION_COUNT_PC34; ++i) {
        const dm1_v1_cppbrs_geometry_pc34_compat_t *g = &model.geometry[i];
        const int left = i * DM1_V1_CPPBRS_STATUS_BOX_STRIDE_X_PC34;

        expect_int("geometry.champion", g->champion_index, i,
                   "CHAMDRAW.C F0293:1134 champion index");
        expect_int("geometry.status.zone", g->status_box_zone, 151 + i,
                   "DEFS.H C151..C154");
        expect_int("geometry.name.zone", g->status_name_zone, 159 + i,
                   "CHAMDRAW.C F0292:893 name zone");
        expect_int("geometry.text.zone", g->status_text_zone, 163 + i,
                   "CHAMDRAW.C F0292:833 text zone");
        expect_int("geometry.left", g->status_left, left,
                   "CHAMDRAW.C F0292 status-box x");
        expect_int("geometry.right", g->status_right, left + 66,
                   "67x29 status-box width");
        expect_int("geometry.bottom", g->status_bottom, 28,
                   "67x29 status-box height");
        expect_int("geometry.ready", g->ready_hand_left, left + 4,
                   "F0291 ready hand local x");
        expect_int("geometry.action", g->action_hand_left, left + 24,
                   "F0291 action hand local x");
        expect_int("geometry.hand.top", g->hand_top, 10,
                   "F0291 hand local y");
        expect_int("geometry.hand.right", g->hand_right, left + 41,
                   "18x18 C033/C034/C035 hand box");
        expect_int("geometry.hand.bottom", g->hand_bottom, 27,
                   "18x18 C033/C034/C035 hand box");
        expect_int("geometry.icon.zone", g->champion_icon_zone, 113 + i,
                   "DEFS.H C113..C116");
        expect_int("geometry.icon.width", g->champion_icon_width, 16,
                   "DEFS.H 16x14 champion icon");
        expect_int("geometry.icon.height", g->champion_icon_height, 14,
                   "DEFS.H 16x14 champion icon");
    }
}

static void test_event_matrix(void)
{
    dm1_v1_cppbrs_model_pc34_compat_t model;
    const dm1_v1_cppbrs_event_row_pc34_compat_t *row;
    int i;

    (void)dm1_v1_cppbrs_build_model_pc34(&model);

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_PARTY_LEADER_ROTATION_PC34);
    expect_int("leader.before", row->leader_before, 0, "leader rotation");
    expect_int("leader.after", row->leader_after, 2, "leader rotation");
    expect_bool("leader.f0292", row->calls_f0292, true,
                "CHAMDRAW.C F0293 -> F0292");
    expect_bool("leader.status.fill", row->fills_status_box, false,
                "NAME_TITLE does not enter F0292:771");
    expect_bool("leader.f0354", row->calls_f0354, false,
                "no status-box portrait blit");
    expect_bool("leader.name.cascade", row->name_color_cascade, true,
                "CHAMDRAW.C F0292:843-895");
    expect_int("leader.color.2", row->leader_name_color[2], 11,
               "PC34 C11 leader");
    expect_int("leader.color.0", row->leader_name_color[0], 9,
               "PC34 C09 nonleader");
    expect_int("leader.ops.count", row->operation_count, 2,
               "name then clear");
    expect_int("leader.ops.0", row->operations[0],
               DM1_V1_CPPBRS_OP_F0292_NAME_COLOR_CASCADE_PC34,
               "CHAMDRAW.C F0292:843-895");

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_HAND_SLOT_SWAP_PC34);
    expect_bool("swap.f0302", row->calls_f0302, true,
                "CHAMPION.C F0302:662-714");
    expect_bool("swap.f0291", row->calls_f0291, true,
                "CHAMDRAW.C F0291:498-677");
    expect_bool("swap.action", row->action_hand_redraw, true,
                "CHAMDRAW.C F0292:1080-1091");
    expect_bool("swap.status.fill", row->fills_status_box, false,
                "ACTION_HAND does not enter status-box fill");
    expect_bool("swap.f0354", row->calls_f0354, false,
                "no F0354 for hand swap");
    expect_int("swap.ops.0", row->operations[0],
               DM1_V1_CPPBRS_OP_F0302_RESOLVE_HAND_POINTER_PC34,
               "F0302 resolves before F0291");
    expect_int("swap.ops.1", row->operations[1],
               DM1_V1_CPPBRS_OP_F0291_DRAW_SLOT_PC34,
               "F0291 after pointer resolution");

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_STATUS_HAND_ROTATION_PC34);
    expect_int("statusrot.leader.after", row->leader_after, 3,
               "leader rotation target");
    expect_bool("statusrot.f0302", row->calls_f0302, true,
                "F0302 order evidence only");
    expect_bool("statusrot.f0354", row->calls_f0354, false,
                "pass765 owns M516/M070 route");
    expect_bool("statusrot.status.fill", row->fills_status_box, false,
                "ACTION_HAND-only row");
    expect_int("statusrot.color.3", row->leader_name_color[3], 11,
               "C11 new leader color");

    row = require_row(&model,
                      DM1_V1_CPPBRS_EVENT_MIRROR_CANDIDATE_OPEN_CLOSE_PC34);
    expect_bool("mirror.path", row->mirror_candidate_path, true,
                "mirror-candidate path");
    expect_bool("mirror.f0296", row->calls_f0296, true,
                "F0296 chrome transition");
    expect_bool("mirror.transition", row->f0296_chrome_transition, true,
                "owner changes");
    expect_bool("mirror.status.fill", row->fills_status_box, true,
                "MASK0x1000_STATUS_BOX");
    expect_bool("mirror.f0354", row->calls_f0354, false,
                "non-inventory continuation");
    expect_int("mirror.continuation", row->continuation_mask,
               DM1_V1_CPPBRS_MASK_NON_INVENTORY_CONTINUATION_PC34,
               "F0292:813-814");

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_CHEST_OPEN_CLOSE_PC34);
    expect_bool("chest.path", row->chest_panel_path, true,
                "F0296 chest slots");
    expect_bool("chest.f0296", row->calls_f0296, true,
                "CHAMDRAW.C F0296:1249-1252");
    expect_bool("chest.status.fill", row->fills_status_box, false,
                "viewport redraw, not status fill");
    expect_bool("chest.f0354", row->calls_f0354, false,
                "no portrait blit");
    expect_int("chest.mask", row->input_mask, DM1_V1_CPPBRS_MASK_VIEWPORT_PC34,
               "F0296 sets viewport");

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_RESURRECT_PENDING_PC34);
    expect_bool("resurrect.path", row->resurrect_pending_path, true,
                "G0299 candidate pending");
    expect_bool("resurrect.suppressed", row->f0296_suppressed_by_candidate,
                true, "F0296 early return");
    expect_bool("resurrect.f0292", row->calls_f0292, false,
                "suppressed before F0292");
    expect_bool("resurrect.f0354", row->calls_f0354, false,
                "suppressed before F0354");

    row = require_row(&model, DM1_V1_CPPBRS_EVENT_CANDIDATE_PICK_PC34);
    expect_bool("pick.path", row->candidate_pick_path, true,
                "candidate pick");
    expect_bool("pick.status.fill", row->fills_status_box, true,
                "new champion status box");
    expect_bool("pick.f0354", row->calls_f0354, false,
                "non-inventory candidate");
    expect_bool("pick.f0291", row->calls_f0291, true,
                "F0291 candidate action hand exception");
    expect_int("pick.redraw", row->redraw_champion, 3,
               "candidate index");

    row = require_row(&model,
                      DM1_V1_CPPBRS_EVENT_INVENTORY_OWNER_STATUS_BOX_PC34);
    expect_bool("inventory.path", row->inventory_owner_status_box_path, true,
                "CHAMDRAW.C F0292:810 inventory owner branch");
    expect_int("inventory.owner.before", row->owner_before, 2,
               "M001 inventory owner ordinal");
    expect_int("inventory.owner.after", row->owner_after, 2,
               "F0292 does not switch owner");
    expect_int("inventory.redraw", row->redraw_champion, 1,
               "M001 ordinal 2 -> champion index 1");
    expect_bool("inventory.status.fill", row->fills_status_box, true,
                "CHAMDRAW.C F0292:757-809");
    expect_bool("inventory.f0354", row->calls_f0354, true,
                "CHAMDRAW.C F0292:810-811");
    expect_bool("inventory.statistics", row->statistics_chrome, true,
                "CHAMDRAW.C F0292:812 then 898-935");
    expect_bool("inventory.name", row->name_color_cascade, false,
                "F0292:812 does not set NAME_TITLE");
    expect_bool("inventory.action", row->action_hand_redraw, false,
                "F0292:812 does not set ACTION_HAND");
    expect_int("inventory.continuation", row->continuation_mask,
               DM1_V1_CPPBRS_MASK_STATISTICS_PC34,
               "CHAMDRAW.C F0292:812 statistics-only continuation");
    expect_int("inventory.ops.count", row->operation_count, 4,
               "status fill -> F0354 -> statistics -> clear");
    expect_int("inventory.ops.0", row->operations[0],
               DM1_V1_CPPBRS_OP_F0292_STATUS_FILL_PC34,
               "CHAMDRAW.C F0292:757-809");
    expect_int("inventory.ops.1", row->operations[1],
               DM1_V1_CPPBRS_OP_F0354_PORTRAIT_BLIT_PC34,
               "CHAMDRAW.C F0292:810-811");
    expect_int("inventory.ops.2", row->operations[2],
               DM1_V1_CPPBRS_OP_F0292_STATISTICS_CHROME_PC34,
               "CHAMDRAW.C F0292:898-935");
    expect_int("inventory.ops.3", row->operations[3],
               DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34,
               "CHAMDRAW.C F0292:1110");

    for (i = 0; i < DM1_V1_CPPBRS_EVENT_COUNT_PC34; ++i) {
        char id[64];
        const dm1_v1_cppbrs_event_row_pc34_compat_t *r = &model.rows[i];

        snprintf(id, sizeof(id), "row.%d.event", i);
        expect_int(id, r->event, i, "stable event order");
        snprintf(id, sizeof(id), "row.%d.clear.last", i);
        expect_int(id, r->operations[r->operation_count - 1],
                   (r->f0296_suppressed_by_candidate)
                       ? DM1_V1_CPPBRS_OP_F0296_SCAN_CHANGED_ICONS_PC34
                       : DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34,
                   "F0292 clear unless F0296 suppresses before redraw");
        snprintf(id, sizeof(id), "row.%d.status.implies.f0292", i);
        expect_bool(id, !r->fills_status_box || r->calls_f0292, true,
                    "status fill requires F0292");
        snprintf(id, sizeof(id), "row.%d.f0354.implies.status", i);
        expect_bool(id, !r->calls_f0354 || r->fills_status_box, true,
                    "F0354 is reachable only from status branch");
        snprintf(id, sizeof(id), "row.%d.f0354.implies.owner", i);
        expect_bool(id, !r->calls_f0354 || r->owner_after > 0, true,
                    "F0354 requires inventory owner ordinal");
        snprintf(id, sizeof(id), "row.%d.colors.leader.range", i);
        expect_bool(id,
                    r->leader_name_color[0] == 9 ||
                        r->leader_name_color[0] == 11,
                    true, "C11/C09 color domain");
    }
}

int main(void)
{
    dm1_v1_cppbrs_model_pc34_compat_t model;

    test_source_and_contract();
    test_geometry();
    test_event_matrix();
    (void)dm1_v1_cppbrs_build_model_pc34(&model);
    expect_u32("deterministic.hash", model.deterministic_hash, 0xE75CC4C6u,
               "pass766plus hash");

    printf("Assertions: %d\n", g_assertions);
    printf("Failures: %d\n", g_failures);
    printf("DeterministicHash: 0x%08X\n",
           (unsigned)model.deterministic_hash);
    if (g_assertions < 80) {
        printf("FAIL assertion floor got=%d want>=80\n", g_assertions);
        return 1;
    }
    if (g_failures) {
        return 1;
    }
    printf("DM1_V1_CHAMPION_PANEL_PORTRAIT_BOX_REDRAW_STATES_PC34_COMPAT_OK\n");
    return 0;
}
