#include "dm1_v1_viewport_d0c_ceiling_f0098_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.  F0098 owns the base
 * floor/ceiling rows; F0112/F0104 overlay pixels are accepted only inside
 * those owned rows and preserve C10 transparency.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset floor/ceiling bitmap parity claim. "
    "DUNVIEW.C F0098:2962-3002 DrawFloorAndCeiling clears/copies the viewport "
    "floor and ceiling source rows; ceiling rows 0-28, gap rows 29-65, "
    "floor rows 66-135 for the 224x136 viewport contract. "
    "DUNVIEW.C F0112:4341-4470 DrawCeilingPit tests the upper-level open pit "
    "and dispatches through F0104/F0105, which preserve C10 transparent pixels. "
    "DEFS.H:4041-4043 anchors C701 viewport floor plus C702/C703 adjacent "
    "wall-zone constants used to keep this D0C row-ownership slice separate "
    "from wall/floor-ornament routes; DEFS.H:2088 C10_COLOR_FLESH. "
    "DUNVIEW.C F0098 D0C row-ownership contract: depth 0 lane 0, top and "
    "bottom row bounds, C10 transparency, not F0108 and not F0107.";

static const DM1_V1_D0CCeilingF0098SpecPc34 s_spec = {
    DM1_V1_D0C_CEILING_F0098_VIEWPORT_WIDTH_PC34,
    DM1_V1_D0C_CEILING_F0098_VIEWPORT_HEIGHT_PC34,
    DM1_V1_D0C_CEILING_F0098_CEILING_FIRST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_CEILING_LAST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_GAP_FIRST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_GAP_LAST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_FLOOR_FIRST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_FLOOR_LAST_ROW_PC34,
    DM1_V1_D0C_CEILING_F0098_DEPTH_PC34,
    DM1_V1_D0C_CEILING_F0098_LANE_PC34,
    DM1_V1_D0C_CEILING_F0098_C10_COLOR_FLESH_PC34,
    DM1_V1_D0C_CEILING_F0098_ZONE_VIEWPORT_CEILING_PC34,
    DM1_V1_D0C_CEILING_F0098_ZONE_VIEWPORT_FLOOR_PC34,
    DM1_V1_D0C_CEILING_F0098_ZONE_WALL_D3L2_PC34,
    DM1_V1_D0C_CEILING_F0098_ZONE_WALL_D3R2_PC34,
    true,
    false,
    true,
    true,
    true,
    false,
    false,
    s_source_evidence
};

static DM1_V1_D0CCeilingF0098RowOwnerPc34 row_owner_for(int row)
{
    if (row < 0 || row >= DM1_V1_D0C_CEILING_F0098_VIEWPORT_HEIGHT_PC34) {
        return DM1_V1_D0C_CEILING_F0098_ROW_OUTSIDE_PC34;
    }
    if (row <= DM1_V1_D0C_CEILING_F0098_CEILING_LAST_ROW_PC34) {
        return DM1_V1_D0C_CEILING_F0098_ROW_CEILING_PC34;
    }
    if (row <= DM1_V1_D0C_CEILING_F0098_GAP_LAST_ROW_PC34) {
        return DM1_V1_D0C_CEILING_F0098_ROW_GAP_PC34;
    }
    return DM1_V1_D0C_CEILING_F0098_ROW_FLOOR_PC34;
}

const DM1_V1_D0CCeilingF0098SpecPc34 *
dm1_v1_viewport_d0c_ceiling_f0098_spec_pc34(void)
{
    return &s_spec;
}

bool dm1_v1_viewport_d0c_ceiling_f0098_apply_pixel_pc34(
    const DM1_V1_D0CCeilingF0098PixelProbePc34 *probe,
    DM1_V1_D0CCeilingF0098PixelResultPc34 *out)
{
    DM1_V1_D0CCeilingF0098RowOwnerPc34 owner;
    bool source_matches_row;

    if (!out) return false;
    memset(out, 0, sizeof(*out));
    if (!probe) return false;

    owner = row_owner_for(probe->row);
    out->row_owner = owner;
    out->valid_row = owner != DM1_V1_D0C_CEILING_F0098_ROW_OUTSIDE_PC34;
    out->pixel_after = probe->destination_pixel;
    if (!out->valid_row) return false;

    switch (probe->source) {
    case DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_BASE_PC34:
        source_matches_row = owner == DM1_V1_D0C_CEILING_F0098_ROW_CEILING_PC34;
        out->calls_f0098 = true;
        break;
    case DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_BASE_PC34:
        source_matches_row = owner == DM1_V1_D0C_CEILING_F0098_ROW_FLOOR_PC34;
        out->calls_f0098 = true;
        break;
    case DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_PIT_F0112_PC34:
        source_matches_row = owner == DM1_V1_D0C_CEILING_F0098_ROW_CEILING_PC34;
        out->calls_f0112 = true;
        break;
    case DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_PIT_D0C_PC34:
        source_matches_row = owner == DM1_V1_D0C_CEILING_F0098_ROW_FLOOR_PC34;
        out->calls_f0104_floor_pit = true;
        break;
    case DM1_V1_D0C_CEILING_F0098_PIXEL_F0108_FLOOR_ORNAMENT_PC34:
        out->calls_f0108_floor_ornament = true;
        return true;
    case DM1_V1_D0C_CEILING_F0098_PIXEL_F0107_WALL_ORNAMENT_PC34:
        out->calls_f0107_wall_ornament = true;
        return true;
    default:
        return false;
    }

    if (!source_matches_row) return true;

    out->row_owned_by_f0098_d0c = true;
    if ((probe->source == DM1_V1_D0C_CEILING_F0098_PIXEL_CEILING_PIT_F0112_PC34 ||
         probe->source == DM1_V1_D0C_CEILING_F0098_PIXEL_FLOOR_PIT_D0C_PC34) &&
        probe->source_pixel == DM1_V1_D0C_CEILING_F0098_C10_COLOR_FLESH_PC34) {
        out->transparent_skip = true;
        return true;
    }

    out->writes_pixel = true;
    out->pixel_after = probe->source_pixel;
    return true;
}

const char *dm1_v1_viewport_d0c_ceiling_f0098_source_evidence_pc34(void)
{
    return s_source_evidence;
}
