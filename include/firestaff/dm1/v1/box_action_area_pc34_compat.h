#ifndef FIRESTAFF_DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_H

/*
 * ReDMCSB source-lock pin for Graphics.dat item 562 init var
 * G0001_ai_Graphic562_Box_ActionArea[4].
 *
 * G0001 is the {X, Y, W, H} byte-coordinate sub-rectangle used by
 * CASTER.C M520_F0021_MAIN_BlitToScreen to draw the action-area
 * background on the champion panel. The X coordinate is a byte
 * offset into the row, and Y is the byte width of the blit area.
 * Init value (DATA.C:121 + DATA.C:541): { 224, 319, 77, 121 }.
 *
 * Read sites:
 * - ACTIDRAW.C:73 M520_F0021_MAIN_BlitToScreen(C009_GRAPHIC_MENU_
 *   SPELL_AREA_BACKGROUND, G0001, C048_BYTE_WIDTH, ...) — blit the
 *   action-area background.
 * - ACTIDRAW.C:320 M524_FillScreenBox(G0001, C00_COLOR_BLACK) — clear
 *   the action area to black.
 * - STARTUP2.C:377 F0136_VIDEO_HatchScreenBox(G0001, C00_COLOR_BLACK)
 *   — hatch the action area during startup.
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/811/812/813/814/815/816/817/818/819/820/
 * 821 (Graphics.dat init-table gates batches 1+2+3+4+5+6+7+8+9).
 * This gate is a non-mirror-candidate contract for the G0001
 * action-area box.
 */

#define DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE 4

typedef struct DM1_V1_BoxActionAreaResultPc34 {
    int accepted;
    int assertionCount;
    int tableEntries[DM1_V1_BOX_ACTION_AREA_PC34_COMPAT_SIZE];
    int tableSize;
    int tableMatchesDeclaration;
    int xIs224;
    int yIs319;
    int wIs77;
    int hIs121;
    int allComponentsNonNegative;
    int widthPositive;
    int heightPositive;
    int byteAligned;
    int withinRowRange;
    int withinBoxBounds;
} DM1_V1_BoxActionAreaResultPc34;

typedef struct DM1_V1_ActionAreaRectPc34 {
    int x;
    int y;
    int w;
    int h;
} DM1_V1_ActionAreaRectPc34;

typedef struct DM1_V1_ActionAreaTextOriginPc34 {
    int x;
    int y;
} DM1_V1_ActionAreaTextOriginPc34;

typedef struct DM1_V1_ActionMenuRenderPlanPc34 {
    DM1_V1_ActionAreaRectPc34 clear_rect;
    DM1_V1_ActionAreaRectPc34 graphic_rect;
    DM1_V1_ActionAreaRectPc34 header_rect;
    DM1_V1_ActionAreaTextOriginPc34 header_text;
    DM1_V1_ActionAreaRectPc34 row_rects[3];
    DM1_V1_ActionAreaTextOriginPc34 row_text[3];
    int graphic_zone_id;
    int graphic_id;
    int clear_color;
    int header_fill_color;
    int header_text_color;
    int row_fill_color;
    int row_text_color;
    int row_count;
} DM1_V1_ActionMenuRenderPlanPc34;

typedef struct DM1_V1_ActionMenuStatePc34 {
    int acting_champion_ordinal;
    int champion_count;
    int acting_champion_present;
    int action_row_count;
} DM1_V1_ActionMenuStatePc34;

typedef struct DM1_V1_ActionMenuReceiptPc34 {
    int accepted;
    int acting_champion_index;
    int visible_row_count;
    DM1_V1_ActionMenuRenderPlanPc34 render_plan;
} DM1_V1_ActionMenuReceiptPc34;

typedef struct DM1_V1_ActionIconStatePc34 {
    int champion_slot;
    int champion_count;
    int champion_present;
    int champion_dead;
    int global_hatch;
} DM1_V1_ActionIconStatePc34;

typedef struct DM1_V1_ActionIconReceiptPc34 {
    int accepted;
    int champion_slot;
    int draw_dead_only;
    int hatch;
    int cell_fill_color;
    int inner_fill_color;
    DM1_V1_ActionAreaRectPc34 cell_rect;
    DM1_V1_ActionAreaRectPc34 inner_rect;
} DM1_V1_ActionIconReceiptPc34;

/*
 * ReDMCSB PC34: ACTIDRAW.C F0387 fills G0001_ai_Graphic562_Box_ActionArea
 * then blits C010 through C011/C077/C079 zones. The visible C011 screen
 * graphic is 87 pixels wide at x=233..319; G0001 remains the older/full
 * byte box used by clear/hatch paths.
 */
enum {
    DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 = 10,
    DM1_V1_ACTION_AREA_ZONE_ID_PC34 = 11,
    DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34 = 0,
    DM1_V1_ACTION_AREA_CYAN_PC34 = 4,
    DM1_V1_ACTION_RESULT_ZONE_ID_PC34 = 75,
    DM1_V1_ACTION_PASS_ZONE_ID_PC34 = 98,

    DM1_V1_ACTION_MENU_HEADER_ZONE_ID_PC34 = 80,
    DM1_V1_ACTION_MENU_ROW_ZONE_ID_BASE_PC34 = 85,
    DM1_V1_ACTION_MENU_ROW_BASE_ZONE_ID_PC34 = 82,
    DM1_V1_ACTION_MENU_ROW_COUNT_PC34 = 3,

    DM1_V1_ACTION_ICON_PARENT_ZONE_ID_PC34 = 88,
    DM1_V1_ACTION_ICON_CELL_ZONE_ID_BASE_PC34 = 89,
    DM1_V1_ACTION_ICON_INNER_ZONE_ID_BASE_PC34 = 93,

    DM1_V1_ACTION_MENU_HEADER_TEXT_LEN_PC34 = 7,
    DM1_V1_ACTION_MENU_ROW_TEXT_LEN_PC34 = 12
};

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_area_rect_pc34(void)
{
    DM1_V1_ActionAreaRectPc34 r = { 233, 77, 87, 45 };
    return r;
}

static inline int
dm1_v1_action_area_zone_id_pc34(void)
{
    return DM1_V1_ACTION_AREA_ZONE_ID_PC34;
}

static inline int
dm1_v1_action_area_graphic_id_pc34(void)
{
    return DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
}

static inline int
dm1_v1_action_area_clear_color_pc34(void)
{
    return DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
}

static inline int
dm1_v1_action_result_zone_id_pc34(void)
{
    return DM1_V1_ACTION_RESULT_ZONE_ID_PC34;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_result_rect_pc34(void)
{
    return dm1_v1_action_area_rect_pc34();
}

static inline int
dm1_v1_action_pass_zone_id_pc34(void)
{
    return DM1_V1_ACTION_PASS_ZONE_ID_PC34;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_pass_rect_pc34(void)
{
    DM1_V1_ActionAreaRectPc34 r = { 285, 77, 34, 7 };
    return r;
}

static inline int
dm1_v1_action_menu_header_zone_id_pc34(void)
{
    return DM1_V1_ACTION_MENU_HEADER_ZONE_ID_PC34;
}

static inline int
dm1_v1_action_menu_row_count_pc34(void)
{
    return DM1_V1_ACTION_MENU_ROW_COUNT_PC34;
}

static inline int
dm1_v1_action_menu_row_base_zone_id_pc34(int row_index)
{
    if (row_index < 0 || row_index >= DM1_V1_ACTION_MENU_ROW_COUNT_PC34) {
        return 0;
    }
    return DM1_V1_ACTION_MENU_ROW_BASE_ZONE_ID_PC34 + row_index;
}

static inline int
dm1_v1_action_menu_row_zone_id_pc34(int row_index)
{
    if (!dm1_v1_action_menu_row_base_zone_id_pc34(row_index)) {
        return 0;
    }
    return DM1_V1_ACTION_MENU_ROW_ZONE_ID_BASE_PC34 + row_index;
}

static inline DM1_V1_ActionAreaTextOriginPc34
dm1_v1_action_menu_text_inset_pc34(void)
{
    DM1_V1_ActionAreaTextOriginPc34 o = { 2, 6 };
    return o;
}

static inline int
dm1_v1_action_menu_header_fill_color_pc34(void)
{
    return DM1_V1_ACTION_AREA_CYAN_PC34;
}

static inline int
dm1_v1_action_menu_header_text_color_pc34(void)
{
    return DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
}

static inline int
dm1_v1_action_menu_row_fill_color_pc34(void)
{
    return DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
}

static inline int
dm1_v1_action_menu_row_text_color_pc34(void)
{
    return DM1_V1_ACTION_AREA_CYAN_PC34;
}

static inline int
dm1_v1_action_icon_parent_zone_id_pc34(void)
{
    return DM1_V1_ACTION_ICON_PARENT_ZONE_ID_PC34;
}

static inline int
dm1_v1_action_menu_graphic_zone_id_pc34(int action_row_count)
{
    if (action_row_count <= 1) return 79;
    if (action_row_count == 2) return 77;
    return DM1_V1_ACTION_AREA_ZONE_ID_PC34;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_menu_graphic_rect_pc34(int action_row_count)
{
    DM1_V1_ActionAreaRectPc34 r = { 233, 77, 87, 45 };
    int zone_id = dm1_v1_action_menu_graphic_zone_id_pc34(action_row_count);
    if (zone_id == 79) {
        r.h = 21;
    } else if (zone_id == 77) {
        r.h = 33;
    }
    return r;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_menu_header_rect_pc34(void)
{
    DM1_V1_ActionAreaRectPc34 r = { 224, 77, 96, 9 };
    return r;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_menu_row_rect_pc34(int row_index)
{
    DM1_V1_ActionAreaRectPc34 r = { 234, 86 + row_index * 12, 85, 11 };
    if (row_index < 0 || row_index >= 3) {
        r.x = r.y = r.w = r.h = 0;
    }
    return r;
}

static inline DM1_V1_ActionAreaTextOriginPc34
dm1_v1_action_menu_header_text_origin_pc34(void)
{
    DM1_V1_ActionAreaTextOriginPc34 o = { 235, 83 };
    return o;
}

static inline DM1_V1_ActionAreaTextOriginPc34
dm1_v1_action_menu_row_text_origin_pc34(int row_index)
{
    DM1_V1_ActionAreaTextOriginPc34 o = { 241, 93 + row_index * 12 };
    if (row_index < 0 || row_index >= 3) {
        o.x = o.y = 0;
    }
    return o;
}

static inline int
dm1_v1_action_menu_build_render_plan_pc34(
    int action_row_count,
    DM1_V1_ActionMenuRenderPlanPc34 *out_plan)
{
    int i;
    if (!out_plan) return 0;
    if (action_row_count <= 0) return 0;
    if (action_row_count > 3) action_row_count = 3;
    out_plan->clear_rect = dm1_v1_action_area_rect_pc34();
    out_plan->graphic_rect =
        dm1_v1_action_menu_graphic_rect_pc34(action_row_count);
    out_plan->header_rect = dm1_v1_action_menu_header_rect_pc34();
    out_plan->header_text = dm1_v1_action_menu_header_text_origin_pc34();
    for (i = 0; i < 3; ++i) {
        out_plan->row_rects[i] = dm1_v1_action_menu_row_rect_pc34(i);
        out_plan->row_text[i] = dm1_v1_action_menu_row_text_origin_pc34(i);
    }
    out_plan->graphic_zone_id =
        dm1_v1_action_menu_graphic_zone_id_pc34(action_row_count);
    out_plan->graphic_id = DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34;
    out_plan->clear_color = DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
    out_plan->header_fill_color = DM1_V1_ACTION_AREA_CYAN_PC34;
    out_plan->header_text_color = DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
    out_plan->row_fill_color = DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
    out_plan->row_text_color = DM1_V1_ACTION_AREA_CYAN_PC34;
    out_plan->row_count = 3;
    return 1;
}

static inline DM1_V1_ActionMenuReceiptPc34
dm1_v1_action_menu_reject_pc34(void)
{
    DM1_V1_ActionMenuReceiptPc34 receipt;
    int i;
    receipt.accepted = 0;
    receipt.acting_champion_index = -1;
    receipt.visible_row_count = 0;
    receipt.render_plan.clear_rect.x = 0;
    receipt.render_plan.clear_rect.y = 0;
    receipt.render_plan.clear_rect.w = 0;
    receipt.render_plan.clear_rect.h = 0;
    receipt.render_plan.graphic_rect = receipt.render_plan.clear_rect;
    receipt.render_plan.header_rect = receipt.render_plan.clear_rect;
    receipt.render_plan.header_text.x = 0;
    receipt.render_plan.header_text.y = 0;
    for (i = 0; i < 3; ++i) {
        receipt.render_plan.row_rects[i] = receipt.render_plan.clear_rect;
        receipt.render_plan.row_text[i].x = 0;
        receipt.render_plan.row_text[i].y = 0;
    }
    receipt.render_plan.graphic_zone_id = 0;
    receipt.render_plan.graphic_id = 0;
    receipt.render_plan.clear_color = 0;
    receipt.render_plan.header_fill_color = 0;
    receipt.render_plan.header_text_color = 0;
    receipt.render_plan.row_fill_color = 0;
    receipt.render_plan.row_text_color = 0;
    receipt.render_plan.row_count = 0;
    return receipt;
}

static inline DM1_V1_ActionMenuReceiptPc34
dm1_v1_action_menu_build_receipt_pc34(
    const DM1_V1_ActionMenuStatePc34 *state)
{
    DM1_V1_ActionMenuReceiptPc34 receipt =
        dm1_v1_action_menu_reject_pc34();
    int acting_index;
    int visible_rows;
    if (!state) return receipt;
    if (state->acting_champion_ordinal <= 0) return receipt;
    acting_index = state->acting_champion_ordinal - 1;
    if (acting_index < 0 || acting_index >= 4) return receipt;
    if (acting_index >= state->champion_count) return receipt;
    if (!state->acting_champion_present) return receipt;
    visible_rows = state->action_row_count;
    if (visible_rows <= 0) return receipt;
    if (visible_rows > DM1_V1_ACTION_MENU_ROW_COUNT_PC34) {
        visible_rows = DM1_V1_ACTION_MENU_ROW_COUNT_PC34;
    }
    if (!dm1_v1_action_menu_build_render_plan_pc34(
            visible_rows, &receipt.render_plan)) {
        return receipt;
    }
    receipt.accepted = 1;
    receipt.acting_champion_index = acting_index;
    receipt.visible_row_count = visible_rows;
    return receipt;
}

static inline int
dm1_v1_action_icon_cell_zone_id_pc34(int champion_slot)
{
    if (champion_slot < 0 || champion_slot >= 4) return 0;
    return DM1_V1_ACTION_ICON_CELL_ZONE_ID_BASE_PC34 + champion_slot;
}

static inline int
dm1_v1_action_icon_inner_zone_id_pc34(int champion_slot)
{
    if (champion_slot < 0 || champion_slot >= 4) return 0;
    return DM1_V1_ACTION_ICON_INNER_ZONE_ID_BASE_PC34 + champion_slot;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_icon_cell_rect_pc34(int champion_slot)
{
    DM1_V1_ActionAreaRectPc34 r = { 233 + champion_slot * 22, 86, 20, 35 };
    if (champion_slot < 0 || champion_slot >= 4) {
        r.x = r.y = r.w = r.h = 0;
    }
    return r;
}

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_icon_inner_rect_pc34(int champion_slot)
{
    DM1_V1_ActionAreaRectPc34 cell =
        dm1_v1_action_icon_cell_rect_pc34(champion_slot);
    DM1_V1_ActionAreaRectPc34 r = { cell.x + 2, 95, 16, 16 };
    if (cell.w == 0) {
        r.x = r.y = r.w = r.h = 0;
    }
    return r;
}

static inline DM1_V1_ActionIconReceiptPc34
dm1_v1_action_icon_reject_pc34(void)
{
    DM1_V1_ActionIconReceiptPc34 receipt;
    receipt.accepted = 0;
    receipt.champion_slot = -1;
    receipt.draw_dead_only = 0;
    receipt.hatch = 0;
    receipt.cell_fill_color = DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34;
    receipt.inner_fill_color = DM1_V1_ACTION_AREA_CYAN_PC34;
    receipt.cell_rect.x = 0;
    receipt.cell_rect.y = 0;
    receipt.cell_rect.w = 0;
    receipt.cell_rect.h = 0;
    receipt.inner_rect = receipt.cell_rect;
    return receipt;
}

static inline DM1_V1_ActionIconReceiptPc34
dm1_v1_action_icon_build_receipt_pc34(
    const DM1_V1_ActionIconStatePc34 *state)
{
    DM1_V1_ActionIconReceiptPc34 receipt =
        dm1_v1_action_icon_reject_pc34();
    if (!state) return receipt;
    if (state->champion_slot < 0 || state->champion_slot >= 4) {
        return receipt;
    }
    if (state->champion_slot >= state->champion_count) return receipt;
    if (!state->champion_present) return receipt;

    receipt.accepted = 1;
    receipt.champion_slot = state->champion_slot;
    receipt.draw_dead_only = state->champion_dead ? 1 : 0;
    receipt.hatch = state->global_hatch ? 1 : 0;
    receipt.cell_fill_color = state->champion_dead
                                  ? DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34
                                  : DM1_V1_ACTION_AREA_CYAN_PC34;
    receipt.inner_fill_color = DM1_V1_ACTION_AREA_CYAN_PC34;
    receipt.cell_rect =
        dm1_v1_action_icon_cell_rect_pc34(state->champion_slot);
    receipt.inner_rect =
        dm1_v1_action_icon_inner_rect_pc34(state->champion_slot);
    return receipt;
}

const int *
dm1_v1_box_action_area_table_pc34(void);

int
dm1_v1_box_action_area_size_pc34(void);

int
dm1_v1_box_action_area_get_pc34(int component, int *out_value);

int
dm1_v1_box_action_area_x_pc34(void);

int
dm1_v1_box_action_area_y_pc34(void);

int
dm1_v1_box_action_area_w_pc34(void);

int
dm1_v1_box_action_area_h_pc34(void);

int
dm1_v1_box_action_area_run_pc34(
    DM1_V1_BoxActionAreaResultPc34 *out);

#endif
