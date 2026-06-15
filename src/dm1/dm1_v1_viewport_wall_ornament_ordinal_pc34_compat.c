#include "dm1_v1_viewport_wall_ornament_ordinal_pc34_compat.h"

#include <string.h>

static bool view_wall_is_right_side_flip(int view_wall_index)
{
    return view_wall_index == DM1_V1_VIEW_WALL_D3R2_LEFT_PC34 ||
           view_wall_index == DM1_V1_VIEW_WALL_D3R_LEFT_PC34 ||
           view_wall_index == DM1_V1_VIEW_WALL_D2R_LEFT_PC34;
}

static bool view_wall_uses_front_bitmap(int view_wall_index)
{
    return view_wall_index >= DM1_V1_VIEW_WALL_D3L_FRONT_PC34 &&
           view_wall_index != DM1_V1_VIEW_WALL_D2L_RIGHT_PC34;
}

bool dm1_v1_viewport_wall_ornament_resolve_f0107_pc34(
    const DM1_V1_WallOrnamentOrdinalInputPc34 *input,
    DM1_V1_WallOrnamentOrdinalResultPc34 *out)
{
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->source_lines = dm1_v1_viewport_wall_ornament_ordinal_source_lock_pc34();
    if (!input) return false;

    /* ReDMCSB: DUNVIEW.C F0107 line ~3571 returns false for ordinal 0. */
    if (input->wall_ornament_ordinal == 0) {
        return true;
    }
    if (input->view_wall_index < DM1_V1_VIEW_WALL_D3L2_RIGHT_PC34 ||
        input->view_wall_index > DM1_V1_VIEW_WALL_D1C_FRONT_PC34 ||
        input->coordinate_set < 0) {
        return false;
    }

    out->draws_ornament = true;
    /* ReDMCSB: DUNVIEW.C F0107 line ~3575 converts ordinal to zero-based index. */
    out->wall_ornament_index = input->wall_ornament_ordinal - 1;
    /* ReDMCSB: DUNVIEW.C F0107 line ~3587 uses C1004 + coordinateSet * 15 + viewWall. */
    out->zone_index = DM1_V1_PC34_WALL_ORNAMENT_ZONE_BASE +
        (input->coordinate_set * DM1_V1_PC34_WALL_ORNAMENT_VIEW_COUNT) +
        input->view_wall_index;
    out->native_bitmap_index = input->native_bitmap_index;
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3817-3821 flip right-side views,
     * otherwise front views use the second native bitmap. */
    out->flip_horizontal = view_wall_is_right_side_flip(input->view_wall_index);
    if (!out->flip_horizontal && view_wall_uses_front_bitmap(input->view_wall_index)) {
        out->native_bitmap_index++;
    }
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3834-3849 use D3 palette/14:32 scale
     * through D3R front, then D2 palette/21:32 scale for nearer walls. */
    if (input->view_wall_index <= DM1_V1_VIEW_WALL_D3R_FRONT_PC34) {
        out->scale_x32 = 14;
        out->palette_depth = 3;
    } else {
        out->scale_x32 = 21;
        out->palette_depth = 2;
    }
    /* ReDMCSB: DUNVIEW.C F0107 lines ~3589 and ~3933 return the alcove flag. */
    out->returns_alcove = input->is_alcove;
    return true;
}

const char *dm1_v1_viewport_wall_ornament_ordinal_source_lock_pc34(void)
{
    return
        "DUNVIEW.C:3571-3575 F0107 zero ordinal returns false, nonzero ordinal is decremented; "
        "DUNVIEW.C:3587 F0107 PC34 wall ornament zone = C1004 + coordinateSet * C15 + viewWall; "
        "DUNVIEW.C:3589,3933 F0107 returns F0149 alcove result; "
        "DUNVIEW.C:3817-3821 F0107 PC34 right-side flip/front-bitmap increment; "
        "DUNVIEW.C:3834-3849 F0107 D3/D2 palette scale split; "
        "DEFS.H:2696-2710 PC34 view wall ordinals; DEFS.H:4222 C1004_ZONE_WALL_ORNAMENT";
}
