#include "firestaff/dm1/v1/champion_panel/dm1_v1_champion_panel_portrait_box_redraw_states_pc34_compat.h"

#include <string.h>

/*
 * Contract-only source-lock evidence for the portrait-box redraw-state matrix.
 *
 * ReDMCSB CHAMDRAW.C F0291:498-677 decides whether a slot belongs to the
 * inventory owner or the top-row status box, maps C033/C034/C035 hand-slot
 * chrome, and then draws the object icon.
 *
 * ReDMCSB CHAMDRAW.C F0292:757-815 first gates all nine dirty bits, resolves
 * the C151+championIndex 67x29 status-box zone, fills live boxes with C12,
 * dispatches F0354 only for the inventory champion, and arms the
 * NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND continuation for non-inventory
 * champions.
 *
 * ReDMCSB CHAMDRAW.C F0292:843-895 performs the name-color cascade after a
 * leader change. For the PC34-compatible route this fixture pins C11 for the
 * leader and C09 for nonleaders, but does not duplicate pass683 pixel tests.
 *
 * ReDMCSB CHAMDRAW.C F0292:898-935 recomputes statistics, then blits C033 or
 * C034 mouth/eye chrome. F0292:1080-1110 redraws the action hand through
 * F0291 and clears all nine dirty bits at T0292042.
 *
 * ReDMCSB CHAMDRAW.C F0293:1117-1143 is represented only as party-index
 * ordering evidence: active champions receive the mask and call F0292 in
 * index order.
 *
 * ReDMCSB CHAMDRAW.C F0295/F0296:1153-1260 scans leader-hand, status-hand,
 * inventory, and chest slot-box icons. When an inventory-owned icon changed,
 * F0296 sets MASK0x4000_VIEWPORT and calls F0292 for the inventory owner.
 * The G0299 candidate/no-inventory early return is pinned as a suppression row.
 *
 * ReDMCSB CHAMPION.C F0302:662-714 resolves the hand-slot pointer before the
 * final F0292 redraw. This slice records the order only; pass764 and pass765
 * own the slot-priority and M516/M070 route details.
 */
static const char s_source_evidence[] =
    "pass766plus contract_only=1; no original DOS pixel parity claim; "
    "synthetic framebuffer evidence only. ReDMCSB CHAMDRAW.C F0291:498-677 "
    "maps inventory/status slots and draws C033/C034/C035 hand-slot chrome; "
    "CHAMDRAW.C F0292:757-815 gates the 67x29 C151..C154 status-box branch, "
    "fills live boxes with C12, calls F0354 only for the inventory champion, "
    "and arms NAME_TITLE|STATISTICS|WOUNDS|ACTION_HAND for non-inventory "
    "champions; CHAMDRAW.C F0292:843-895 pins the PC34 C11 leader/C09 "
    "nonleader name-color cascade; CHAMDRAW.C F0292:898-935 recomputes "
    "statistics and C033/C034 mouth/eye chrome; CHAMDRAW.C F0292:1080-1110 "
    "redraws the action hand through F0291 and clears all nine dirty bits; "
    "CHAMDRAW.C F0293:1117-1143 iterates active champions in index order; "
    "CHAMDRAW.C F0295/F0296:1153-1260 scans changed leader/status/inventory/"
    "chest icons, suppresses G0299-without-inventory, and hands viewport "
    "redraws to F0292 for the owner; CHAMPION.C F0302:662-714 resolves the "
    "hand-slot pointer before final F0292; DEFS.H C113..C116 16x14 champion "
    "icons, C033/C034/C035 hand-slot box chrome, and C151..C154 67x29 "
    "status-box stride.";

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t hash_model(const dm1_v1_cppbrs_model_pc34_compat_t *model)
{
    uint32_t hash = 2166136261u;
    int i;
    int j;

    hash = mix_u32(hash, model->contract_only ? 1u : 0u);
    hash = mix_u32(hash, model->disjoint_from_pass673_portrait_state ? 1u : 0u);
    hash = mix_u32(hash, model->disjoint_from_pass683_name_color ? 1u : 0u);
    hash = mix_u32(hash,
                   model->disjoint_from_pass764_second_leader_slot_priority ? 1u
                                                                           : 0u);
    hash = mix_u32(hash, model->disjoint_from_pass765_status_hand_rotation ? 1u
                                                                           : 0u);
    hash = mix_u32(hash, model->disjoint_from_f0354_blit_gate ? 1u : 0u);

    for (i = 0; i < DM1_V1_CPPBRS_CHAMPION_COUNT_PC34; ++i) {
        const dm1_v1_cppbrs_geometry_pc34_compat_t *g = &model->geometry[i];
        hash = mix_u32(hash, (uint32_t)g->champion_index);
        hash = mix_u32(hash, (uint32_t)g->status_box_zone);
        hash = mix_u32(hash, (uint32_t)g->status_left);
        hash = mix_u32(hash, (uint32_t)g->status_right);
        hash = mix_u32(hash, (uint32_t)g->action_hand_left);
        hash = mix_u32(hash, (uint32_t)g->hand_right);
        hash = mix_u32(hash, (uint32_t)g->champion_icon_zone);
    }

    for (i = 0; i < DM1_V1_CPPBRS_EVENT_COUNT_PC34; ++i) {
        const dm1_v1_cppbrs_event_row_pc34_compat_t *row = &model->rows[i];
        hash = mix_u32(hash, (uint32_t)row->event);
        hash = mix_u32(hash, (uint32_t)row->leader_before);
        hash = mix_u32(hash, (uint32_t)row->leader_after);
        hash = mix_u32(hash, (uint32_t)row->owner_before);
        hash = mix_u32(hash, (uint32_t)row->owner_after);
        hash = mix_u32(hash, (uint32_t)row->redraw_champion);
        hash = mix_u32(hash, (uint32_t)row->input_mask);
        hash = mix_u32(hash, (uint32_t)row->continuation_mask);
        hash = mix_u32(hash, row->calls_f0296 ? 1u : 0u);
        hash = mix_u32(hash, row->calls_f0302 ? 1u : 0u);
        hash = mix_u32(hash, row->calls_f0291 ? 1u : 0u);
        hash = mix_u32(hash, row->calls_f0292 ? 1u : 0u);
        hash = mix_u32(hash, row->fills_status_box ? 1u : 0u);
        hash = mix_u32(hash, row->calls_f0354 ? 1u : 0u);
        hash = mix_u32(hash, row->f0296_chrome_transition ? 1u : 0u);
        hash = mix_u32(hash, row->f0296_suppressed_by_candidate ? 1u : 0u);
        hash = mix_u32(hash, (uint32_t)row->operation_count);
        for (j = 0; j < row->operation_count; ++j) {
            hash = mix_u32(hash, (uint32_t)row->operations[j]);
        }
        for (j = 0; j < DM1_V1_CPPBRS_CHAMPION_COUNT_PC34; ++j) {
            hash = mix_u32(hash, (uint32_t)row->leader_name_color[j]);
        }
    }

    return hash;
}

static void fill_geometry(dm1_v1_cppbrs_geometry_pc34_compat_t *g,
                          int champion)
{
    const int left = champion * DM1_V1_CPPBRS_STATUS_BOX_STRIDE_X_PC34;

    memset(g, 0, sizeof(*g));
    g->champion_index = champion;
    g->status_box_zone = DM1_V1_CPPBRS_STATUS_BOX_ZONE_BASE_PC34 + champion;
    g->status_name_zone = DM1_V1_CPPBRS_STATUS_NAME_ZONE_BASE_PC34 + champion;
    g->status_text_zone = DM1_V1_CPPBRS_STATUS_TEXT_ZONE_BASE_PC34 + champion;
    g->status_left = left;
    g->status_top = 0;
    g->status_right = left + DM1_V1_CPPBRS_STATUS_BOX_WIDTH_PC34 - 1;
    g->status_bottom = DM1_V1_CPPBRS_STATUS_BOX_HEIGHT_PC34 - 1;
    g->ready_hand_left = left + DM1_V1_CPPBRS_HAND_READY_LOCAL_X_PC34;
    g->action_hand_left = left + DM1_V1_CPPBRS_HAND_ACTION_LOCAL_X_PC34;
    g->hand_top = DM1_V1_CPPBRS_HAND_LOCAL_Y_PC34;
    g->hand_right = g->action_hand_left + DM1_V1_CPPBRS_HAND_BOX_SIZE_PC34 - 1;
    g->hand_bottom = g->hand_top + DM1_V1_CPPBRS_HAND_BOX_SIZE_PC34 - 1;
    g->champion_icon_zone = DM1_V1_CPPBRS_CHAMPION_ICON_ZONE_BASE_PC34 +
                            champion;
    g->champion_icon_width = DM1_V1_CPPBRS_CHAMPION_ICON_WIDTH_PC34;
    g->champion_icon_height = DM1_V1_CPPBRS_CHAMPION_ICON_HEIGHT_PC34;
}

static void set_name_colors(dm1_v1_cppbrs_event_row_pc34_compat_t *row,
                            int leader)
{
    int i;
    for (i = 0; i < DM1_V1_CPPBRS_CHAMPION_COUNT_PC34; ++i) {
        row->leader_name_color[i] =
            (i == leader) ? DM1_V1_CPPBRS_COLOR_LEADER_NAME_PC34
                          : DM1_V1_CPPBRS_COLOR_NONLEADER_NAME_PC34;
    }
}

static void fill_ops(dm1_v1_cppbrs_event_row_pc34_compat_t *row,
                     const dm1_v1_cppbrs_op_pc34_compat_t *ops,
                     int count)
{
    int i;
    row->operation_count = count;
    for (i = 0; i < DM1_V1_CPPBRS_MAX_OPS_PC34; ++i) {
        row->operations[i] =
            (i < count) ? ops[i] : DM1_V1_CPPBRS_OP_NONE_PC34;
    }
}

static void fill_row(dm1_v1_cppbrs_event_row_pc34_compat_t *row,
                     dm1_v1_cppbrs_event_pc34_compat_t event,
                     const char *name)
{
    memset(row, 0, sizeof(*row));
    row->event = event;
    row->name = name;
    row->owner_before = 1;
    row->owner_after = 1;
    row->leader_before = 0;
    row->leader_after = 0;
    row->redraw_champion = 1;
    set_name_colors(row, row->leader_after);
}

bool dm1_v1_cppbrs_build_model_pc34(
    dm1_v1_cppbrs_model_pc34_compat_t *out)
{
    int i;

    static const dm1_v1_cppbrs_op_pc34_compat_t leader_ops[] = {
        DM1_V1_CPPBRS_OP_F0292_NAME_COLOR_CASCADE_PC34,
        DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34
    };
    static const dm1_v1_cppbrs_op_pc34_compat_t hand_swap_ops[] = {
        DM1_V1_CPPBRS_OP_F0302_RESOLVE_HAND_POINTER_PC34,
        DM1_V1_CPPBRS_OP_F0291_DRAW_SLOT_PC34,
        DM1_V1_CPPBRS_OP_F0292_ACTION_HAND_PC34,
        DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34
    };
    static const dm1_v1_cppbrs_op_pc34_compat_t status_box_ops[] = {
        DM1_V1_CPPBRS_OP_F0292_STATUS_FILL_PC34,
        DM1_V1_CPPBRS_OP_F0292_NAME_COLOR_CASCADE_PC34,
        DM1_V1_CPPBRS_OP_F0292_STATISTICS_CHROME_PC34,
        DM1_V1_CPPBRS_OP_F0291_DRAW_SLOT_PC34,
        DM1_V1_CPPBRS_OP_F0292_ACTION_HAND_PC34,
        DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34
    };
    static const dm1_v1_cppbrs_op_pc34_compat_t chest_ops[] = {
        DM1_V1_CPPBRS_OP_F0296_SCAN_CHANGED_ICONS_PC34,
        DM1_V1_CPPBRS_OP_F0292_STATISTICS_CHROME_PC34,
        DM1_V1_CPPBRS_OP_F0292_CLEAR_MASK_PC34
    };
    static const dm1_v1_cppbrs_op_pc34_compat_t suppress_ops[] = {
        DM1_V1_CPPBRS_OP_F0296_SCAN_CHANGED_ICONS_PC34
    };

    if (!out) {
        return false;
    }

    memset(out, 0, sizeof(*out));
    out->contract_only = true;
    out->disjoint_from_pass673_portrait_state = true;
    out->disjoint_from_pass683_name_color = true;
    out->disjoint_from_pass764_second_leader_slot_priority = true;
    out->disjoint_from_pass765_status_hand_rotation = true;
    out->disjoint_from_f0354_blit_gate = true;

    for (i = 0; i < DM1_V1_CPPBRS_CHAMPION_COUNT_PC34; ++i) {
        fill_geometry(&out->geometry[i], i);
    }

    fill_row(&out->rows[0], DM1_V1_CPPBRS_EVENT_PARTY_LEADER_ROTATION_PC34,
             "party leader rotation");
    out->rows[0].leader_before = 0;
    out->rows[0].leader_after = 2;
    out->rows[0].redraw_champion = -1;
    out->rows[0].input_mask = DM1_V1_CPPBRS_MASK_NAME_TITLE_PC34;
    out->rows[0].calls_f0292 = true;
    out->rows[0].name_color_cascade = true;
    set_name_colors(&out->rows[0], out->rows[0].leader_after);
    fill_ops(&out->rows[0], leader_ops, 2);

    fill_row(&out->rows[1], DM1_V1_CPPBRS_EVENT_HAND_SLOT_SWAP_PC34,
             "hand-slot swap");
    out->rows[1].input_mask = DM1_V1_CPPBRS_MASK_ACTION_HAND_PC34;
    out->rows[1].calls_f0302 = true;
    out->rows[1].calls_f0291 = true;
    out->rows[1].calls_f0292 = true;
    out->rows[1].action_hand_redraw = true;
    fill_ops(&out->rows[1], hand_swap_ops, 4);

    fill_row(&out->rows[2], DM1_V1_CPPBRS_EVENT_STATUS_HAND_ROTATION_PC34,
             "status-hand rotation");
    out->rows[2].leader_before = 1;
    out->rows[2].leader_after = 3;
    out->rows[2].redraw_champion = 3;
    out->rows[2].input_mask = DM1_V1_CPPBRS_MASK_ACTION_HAND_PC34;
    out->rows[2].calls_f0302 = true;
    out->rows[2].calls_f0291 = true;
    out->rows[2].calls_f0292 = true;
    out->rows[2].action_hand_redraw = true;
    set_name_colors(&out->rows[2], out->rows[2].leader_after);
    fill_ops(&out->rows[2], hand_swap_ops, 4);

    fill_row(&out->rows[3],
             DM1_V1_CPPBRS_EVENT_MIRROR_CANDIDATE_OPEN_CLOSE_PC34,
             "mirror-candidate open/close");
    out->rows[3].owner_before = 1;
    out->rows[3].owner_after = 2;
    out->rows[3].redraw_champion = 2;
    out->rows[3].input_mask = DM1_V1_CPPBRS_MASK_STATUS_BOX_PC34;
    out->rows[3].continuation_mask =
        DM1_V1_CPPBRS_MASK_NON_INVENTORY_CONTINUATION_PC34;
    out->rows[3].calls_f0296 = true;
    out->rows[3].calls_f0291 = true;
    out->rows[3].calls_f0292 = true;
    out->rows[3].fills_status_box = true;
    out->rows[3].name_color_cascade = true;
    out->rows[3].statistics_chrome = true;
    out->rows[3].action_hand_redraw = true;
    out->rows[3].f0296_chrome_transition = true;
    out->rows[3].mirror_candidate_path = true;
    set_name_colors(&out->rows[3], out->rows[3].leader_after);
    fill_ops(&out->rows[3], status_box_ops, 6);

    fill_row(&out->rows[4], DM1_V1_CPPBRS_EVENT_CHEST_OPEN_CLOSE_PC34,
             "chest open/close");
    out->rows[4].input_mask = DM1_V1_CPPBRS_MASK_VIEWPORT_PC34;
    out->rows[4].continuation_mask = DM1_V1_CPPBRS_MASK_VIEWPORT_PC34;
    out->rows[4].calls_f0296 = true;
    out->rows[4].calls_f0292 = true;
    out->rows[4].statistics_chrome = false;
    out->rows[4].chest_panel_path = true;
    fill_ops(&out->rows[4], chest_ops, 3);

    fill_row(&out->rows[5], DM1_V1_CPPBRS_EVENT_RESURRECT_PENDING_PC34,
             "resurrect pending");
    out->rows[5].owner_before = 0;
    out->rows[5].owner_after = 0;
    out->rows[5].redraw_champion = -1;
    out->rows[5].calls_f0296 = true;
    out->rows[5].f0296_suppressed_by_candidate = true;
    out->rows[5].resurrect_pending_path = true;
    fill_ops(&out->rows[5], suppress_ops, 1);

    fill_row(&out->rows[6], DM1_V1_CPPBRS_EVENT_CANDIDATE_PICK_PC34,
             "candidate pick");
    out->rows[6].owner_before = 0;
    out->rows[6].owner_after = 4;
    out->rows[6].redraw_champion = 3;
    out->rows[6].input_mask = DM1_V1_CPPBRS_MASK_STATUS_BOX_PC34;
    out->rows[6].continuation_mask =
        DM1_V1_CPPBRS_MASK_NON_INVENTORY_CONTINUATION_PC34;
    out->rows[6].calls_f0291 = true;
    out->rows[6].calls_f0292 = true;
    out->rows[6].fills_status_box = true;
    out->rows[6].name_color_cascade = true;
    out->rows[6].statistics_chrome = true;
    out->rows[6].action_hand_redraw = true;
    out->rows[6].candidate_pick_path = true;
    set_name_colors(&out->rows[6], out->rows[6].leader_after);
    fill_ops(&out->rows[6], status_box_ops, 6);

    out->deterministic_hash = hash_model(out);
    return true;
}

const dm1_v1_cppbrs_event_row_pc34_compat_t *
dm1_v1_cppbrs_find_event_pc34(
    const dm1_v1_cppbrs_model_pc34_compat_t *model,
    dm1_v1_cppbrs_event_pc34_compat_t event)
{
    int i;

    if (!model) {
        return NULL;
    }

    for (i = 0; i < DM1_V1_CPPBRS_EVENT_COUNT_PC34; ++i) {
        if (model->rows[i].event == event) {
            return &model->rows[i];
        }
    }

    return NULL;
}

const char *dm1_v1_cppbrs_source_pc34(void)
{
    return s_source_evidence;
}
