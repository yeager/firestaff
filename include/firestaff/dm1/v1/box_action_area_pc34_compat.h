#ifndef FIRESTAFF_DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_BOX_SPELL_AREA_PC34_COMPAT_H

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

/*
 * ReDMCSB: ACTIDRAW.C F0387 lines ~320-382 fills
 * G0001_ai_Graphic562_Box_ActionArea then blits C010 through
 * G0499/G0500/G0501 and prints the header/action rows. F0386 lines
 * ~218-300 owns the action-hand icon cell and inner 16x16 icon boxes.
 */
enum {
    DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 = 10,
    DM1_V1_ACTION_AREA_ZONE_ID_PC34 = 11,
    DM1_V1_ACTION_AREA_CLEAR_COLOR_PC34 = 0,
    DM1_V1_ACTION_AREA_CYAN_PC34 = 4,

    DM1_V1_ACTION_MENU_HEADER_ZONE_ID_PC34 = 80,
    DM1_V1_ACTION_MENU_ROW_ZONE_ID_BASE_PC34 = 85,
    DM1_V1_ACTION_MENU_ROW_BASE_ZONE_ID_PC34 = 82,
    DM1_V1_ACTION_MENU_ROW_COUNT_PC34 = 3,

    DM1_V1_ACTION_ICON_PARENT_ZONE_ID_PC34 = 88,
    DM1_V1_ACTION_ICON_CELL_ZONE_ID_BASE_PC34 = 89,
    DM1_V1_ACTION_ICON_INNER_ZONE_ID_BASE_PC34 = 93
};

static inline DM1_V1_ActionAreaRectPc34
dm1_v1_action_area_rect_pc34(void)
{
    DM1_V1_ActionAreaRectPc34 r = { 224, 77, 96, 45 };
    return r;
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
    DM1_V1_ActionAreaRectPc34 r = { 224, 77, 96, 45 };
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
