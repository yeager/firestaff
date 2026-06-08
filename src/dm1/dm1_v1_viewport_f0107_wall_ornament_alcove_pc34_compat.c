#include "dm1_v1_viewport_f0107_wall_ornament_alcove_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source lock:
 * DUNVIEW.C:3502-3938 F0107 returns F0149's alcove BOOLEAN for a nonzero
 * wall ornament ordinal, after resolving the wall ornament coordinate set.
 * DUNVIEW.C:6432/6568/6968/7119/7459/7627 pass M551/M553 side-wall
 * ordinals into F0107; DUNVIEW.C:6433/6569/6969/7120/7842 pass M552
 * front-wall ordinals only to select the alcove object pass. DEFS.H:2551-2554
 * defines the PC34 square-aspect slots and DEFS.H:2696-2710 defines the PC34
 * view-wall codes used below.
 */
static const DM1_V1_F0107WallOrnamentAlcoveCasePc34
dm1_v1_viewport_f0107_alcove_decide_table[] = {
    {
        DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_C00_D3L2_RIGHT_PC34,
        DM1_V1_F0107_COORDINATE_D3L2_RIGHT_PC34,
        DM1_V1_F0107_WALLSET_C11_D3L2_PC34,
        true,
        "DUNVIEW.C:6263; DEFS.H:2696,3434,4042",
        "C00 D3L2 right wall uses M551 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_C01_D3R2_LEFT_PC34,
        DM1_V1_F0107_COORDINATE_D3R2_LEFT_PC34,
        DM1_V1_F0107_WALLSET_C10_D3R2_PC34,
        true,
        "DUNVIEW.C:6330; DEFS.H:2697,3433,4043",
        "C01 D3R2 left wall uses M553 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M575_D3L_RIGHT_PC34,
        DM1_V1_F0107_COORDINATE_D3L_RIGHT_PC34,
        DM1_V1_F0107_WALLSET_C13_D3L_PC34,
        true,
        "DUNVIEW.C:6432; DEFS.H:2698,3436,4045",
        "M575 D3L right wall uses M551 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M576_D3R_LEFT_PC34,
        DM1_V1_F0107_COORDINATE_D3R_LEFT_PC34,
        DM1_V1_F0107_WALLSET_C12_D3R_PC34,
        true,
        "DUNVIEW.C:6568; DEFS.H:2699,3435,4046",
        "M576 D3R left wall uses M553 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34,
        DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
        DM1_V1_F0107_WALLSET_C08_D2L_PC34,
        true,
        "DUNVIEW.C:6968; DEFS.H:2703,3431,4050",
        "M580 D2L right wall uses M551 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34,
        DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34,
        DM1_V1_F0107_WALLSET_C07_D2R_PC34,
        true,
        "DUNVIEW.C:7119; DEFS.H:2704,3430,4051",
        "M581 D2R left wall uses M553 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M551_RIGHT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M585_D1L_RIGHT_PC34,
        DM1_V1_F0107_COORDINATE_D1L_RIGHT_PC34,
        DM1_V1_F0107_WALLSET_C03_D1L_PC34,
        true,
        "DUNVIEW.C:7459; DEFS.H:2708,3426,4053",
        "M585 D1L right wall uses M551 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M553_LEFT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M586_D1R_LEFT_PC34,
        DM1_V1_F0107_COORDINATE_D1R_LEFT_PC34,
        DM1_V1_F0107_WALLSET_C02_D1R_PC34,
        true,
        "DUNVIEW.C:7627; DEFS.H:2709,3425,4054",
        "M586 D1R left wall uses M553 side ornament and can return F0107 alcove"
    },
    {
        DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34,
        DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
        DM1_V1_F0107_WALLSET_C08_D2L_PC34,
        false,
        "DUNVIEW.C:6969-6971; DEFS.H:2705",
        "M552 front wall controls alcove thing-pass order, not side alcove detection"
    },
    {
        DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M584_D2R_FRONT_PC34,
        DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
        DM1_V1_F0107_WALLSET_C07_D2R_PC34,
        false,
        "DUNVIEW.C:7120-7122; DEFS.H:2707",
        "M552 front wall controls alcove thing-pass order, not side alcove detection"
    },
    {
        DM1_V1_F0107_SLOT_M552_FRONT_WALL_ORNAMENT_PC34,
        DM1_V1_F0107_WALL_CELL_M587_D1C_FRONT_PC34,
        DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34,
        DM1_V1_F0107_WALLSET_C03_D1L_PC34,
        false,
        "DUNVIEW.C:7842; DEFS.H:2710",
        "M552 D1C front route is a front-wall order gate, not side alcove detection"
    },
    {
        DM1_V1_F0107_SLOT_M554_MIRROR_FRONT_WALL_PC34,
        DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34,
        DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34,
        DM1_V1_F0107_WALLSET_C08_D2L_PC34,
        false,
        "DUNVIEW.C:6432-6433,6968-6969; DEFS.H:2551-2554",
        "F0107 wall-ornament call sites use M551/M552/M553; mirror/front M554 is rejected"
    }
};

static size_t dm1_v1_f0107_case_count(void)
{
    return sizeof(dm1_v1_viewport_f0107_alcove_decide_table) /
           sizeof(dm1_v1_viewport_f0107_alcove_decide_table[0]);
}

int dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(int wall_cell_code)
{
    switch (wall_cell_code) {
    case DM1_V1_F0107_WALL_CELL_C00_D3L2_RIGHT_PC34:
        return DM1_V1_F0107_COORDINATE_D3L2_RIGHT_PC34;
    case DM1_V1_F0107_WALL_CELL_C01_D3R2_LEFT_PC34:
        return DM1_V1_F0107_COORDINATE_D3R2_LEFT_PC34;
    case DM1_V1_F0107_WALL_CELL_M575_D3L_RIGHT_PC34:
        return DM1_V1_F0107_COORDINATE_D3L_RIGHT_PC34;
    case DM1_V1_F0107_WALL_CELL_M576_D3R_LEFT_PC34:
        return DM1_V1_F0107_COORDINATE_D3R_LEFT_PC34;
    case DM1_V1_F0107_WALL_CELL_M580_D2L_RIGHT_PC34:
        return DM1_V1_F0107_COORDINATE_D2L_RIGHT_PC34;
    case DM1_V1_F0107_WALL_CELL_M581_D2R_LEFT_PC34:
        return DM1_V1_F0107_COORDINATE_D2R_LEFT_PC34;
    case DM1_V1_F0107_WALL_CELL_M585_D1L_RIGHT_PC34:
        return DM1_V1_F0107_COORDINATE_D1L_RIGHT_PC34;
    case DM1_V1_F0107_WALL_CELL_M586_D1R_LEFT_PC34:
        return DM1_V1_F0107_COORDINATE_D1R_LEFT_PC34;
    case DM1_V1_F0107_WALL_CELL_M582_D2L_FRONT_PC34:
    case DM1_V1_F0107_WALL_CELL_M584_D2R_FRONT_PC34:
    case DM1_V1_F0107_WALL_CELL_M587_D1C_FRONT_PC34:
        return DM1_V1_F0107_COORDINATE_BACK_WALL_FRONT_PC34;
    default:
        return DM1_V1_F0107_COORDINATE_UNKNOWN_PC34;
    }
}

bool dm1_v1_viewport_f0107_wall_ornament_alcove_decide(
    int ordinal_slot,
    int wall_cell_code,
    int coordinate_set,
    int wall_set)
{
    size_t i;

    if (coordinate_set == DM1_V1_F0107_COORDINATE_UNKNOWN_PC34 ||
        dm1_v1_viewport_f0107_wall_cell_coordinate_set_pc34(wall_cell_code) ==
            DM1_V1_F0107_COORDINATE_UNKNOWN_PC34) {
        return false;
    }

    for (i = 0; i < dm1_v1_f0107_case_count(); ++i) {
        const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *entry =
            &dm1_v1_viewport_f0107_alcove_decide_table[i];
        if (entry->ordinal_slot == ordinal_slot &&
            entry->wall_cell_code == wall_cell_code &&
            entry->coordinate_set == coordinate_set &&
            entry->wall_set == wall_set) {
            return entry->returns_alcove;
        }
    }
    return false;
}

const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *
dm1_v1_viewport_f0107_wall_ornament_alcove_cases_pc34(size_t *count)
{
    if (count) *count = dm1_v1_f0107_case_count();
    return dm1_v1_viewport_f0107_alcove_decide_table;
}

const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *
dm1_v1_viewport_f0107_wall_ornament_alcove_case_at_pc34(size_t index)
{
    if (index >= dm1_v1_f0107_case_count()) return NULL;
    return &dm1_v1_viewport_f0107_alcove_decide_table[index];
}

bool dm1_v1_viewport_f0107_wall_ornament_alcove_decide_case_pc34(
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *entry)
{
    if (!entry) return false;
    return dm1_v1_viewport_f0107_wall_ornament_alcove_decide(
        entry->ordinal_slot,
        entry->wall_cell_code,
        entry->coordinate_set,
        entry->wall_set);
}

bool dm1_v1_viewport_f0107_wall_ornament_apply_pixel_pc34(
    const DM1_V1_F0107WallOrnamentPixelInputPc34 *input,
    const uint8_t *source,
    size_t source_len,
    uint8_t *viewport,
    size_t viewport_len,
    DM1_V1_F0107WallOrnamentPixelResultPc34 *out)
{
    const DM1_V1_F0107WallOrnamentAlcoveCasePc34 *entry;
    uint8_t transparent_color;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!input) return false;

    out->row = input->row;
    out->viewport_x = input->viewport_x;
    entry = dm1_v1_viewport_f0107_wall_ornament_alcove_case_at_pc34(
        input->case_index);
    out->source_case = entry;
    if (!entry) return false;

    out->route_valid = true;
    out->returns_alcove =
        dm1_v1_viewport_f0107_wall_ornament_alcove_decide_case_pc34(entry);
    out->draw_attempted = entry->ordinal_slot != 0;
    transparent_color = input->transparent_color;
    if (transparent_color == 0) {
        transparent_color = DM1_V1_F0107_WALL_ORNAMENT_C10_COLOR_FLESH_PC34;
    }

    if (input->row < 0 ||
        input->row >= DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_HEIGHT_PC34 ||
        input->viewport_x < 0 ||
        input->viewport_x >= DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34) {
        out->no_write_metadata = true;
        return true;
    }
    if (!source || !viewport) return false;

    out->in_clip = true;
    out->source_x = input->viewport_x;
    out->source_y = input->row;
    out->source_offset =
        (size_t)out->source_y *
            (size_t)DM1_V1_F0107_WALL_ORNAMENT_SYNTHETIC_WIDTH_PC34 +
        (size_t)out->source_x;
    out->viewport_offset =
        (size_t)input->row *
            (size_t)DM1_V1_F0107_WALL_ORNAMENT_VIEWPORT_WIDTH_PC34 +
        (size_t)input->viewport_x;
    if (out->source_offset >= source_len || out->viewport_offset >= viewport_len) {
        return false;
    }

    out->pixel_before = viewport[out->viewport_offset];
    out->source_pixel = source[out->source_offset];
    out->transparent_skip = out->source_pixel == transparent_color;
    out->writes_pixel = !out->transparent_skip;
    viewport[out->viewport_offset] =
        dm1_v1_viewport_f0107_wall_ornament_blend_pixel_pc34(
            viewport[out->viewport_offset], out->source_pixel, transparent_color);
    out->pixel_after = viewport[out->viewport_offset];
    return true;
}

uint8_t dm1_v1_viewport_f0107_wall_ornament_blend_pixel_pc34(
    uint8_t destination_pixel,
    uint8_t source_pixel,
    uint8_t transparent_color)
{
    if (source_pixel == transparent_color) return destination_pixel;
    return source_pixel;
}

const char *dm1_v1_viewport_f0107_wall_ornament_alcove_source_evidence_pc34(void)
{
    return
        "ReDMCSB DUNVIEW.C:3502-3938 F0107_DUNGEONVIEW_"
        "IsDrawnWallOrnamentAnAlcove_CPSF returns the F0149 alcove BOOLEAN "
        "at line 3933 after resolving the wall ornament coordinate set at "
        "line 3578 and the alcove flag at line 3589. DUNVIEW.C:6263/6330 "
        "bind C00_VIEW_WALL_D3L2_RIGHT and C01_VIEW_WALL_D3R2_LEFT; "
        "DUNVIEW.C:6432/6568 bind M575/M576 D3 side walls; "
        "DUNVIEW.C:6968/7119 bind M580/M581 D2 side walls; "
        "DUNVIEW.C:7459/7627 bind M585/M586 D1 side walls. "
        "DUNVIEW.C:6433/6569/6969/7120/7842 use M552 front walls as "
        "front-alcove order gates. DEFS.H:2551-2554 defines M551/M552/M553 "
        "slots and the adjacent M554 slot; DEFS.H:2696-2710 defines the "
        "PC34 wall-cell codes; DEFS.H:3423-3438 defines C00-C15 wall sets; "
        "DEFS.H:4042-4057 defines C702-C717 wall zones. F0107 draws wall "
        "ornament pixels through C10-transparent blits at DUNVIEW.C:3907, "
        "DUNVIEW.C:3910, and the F0791 zone path at DUNVIEW.C:3922; "
        "DEFS.H:2088 defines C10_COLOR_FLESH=10.";
}
