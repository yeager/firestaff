#include "dm1_v1_viewport_d0c_stairs_pit_dispatch_pc34_compat.h"

/*
 * Source-locked contract_only=1 gate: ReDMCSB DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView
 * lines 8336-8339 and 8538-8542 dispatch into DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C
 * lines 8164-8310; no real-asset bitmap parity is modeled here.
 */

static const DM1_V1_D0CStairsPitDispatchContractPc34 s_contract = {
    true,
    "Source-locked contract_only=1; DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339,8538-8542; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310",
    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339 calls F0098_DUNGEONVIEW_DrawFloorAndCeiling before D0C dispatch",
    "DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8538-8542 dispatches D0R then F0127_DUNGEONVIEW_DrawSquareD0C",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8164-8310 L0222 square aspect switch and tail",
    "DEFS.H:1007-1017 C02_ELEMENT_PIT, C05_ELEMENT_TELEPORTER, C19_ELEMENT_STAIRS_FRONT",
    "DEFS.H:2547-2559 MEDIA720 square-aspect indices; DEFS.H:2573-2602 M609_VIEW_SQUARE_D0C",
    "DEFS.H:2440-2454 C06/C13 D0C left stairs bitmap slots",
    "DEFS.H:2656-2663 C0x0021_CELL_ORDER_BACKLEFT_BACKRIGHT",
    "DEFS.H:4150-4151 C811/C812 stairs-up D0L/D0R; DEFS.H:4163-4164 C824/C825 stairs-down D0L/D0R; DEFS.H:4209 C862 floor pit; DEFS.H:4218 C871 ceiling pit; DEFS.H:4055 C715 D0C field",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273 stairs-front switch case",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8242-8254 stairs-up calls F0104 left and F0105 right",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273 stairs-down calls F0104 left and F0105 right, then breaks before F0115 tail",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8284 pit case calls F0104 before shared tail",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293 shared F0112_DUNGEONVIEW_DrawCeilingPit tail has no F0099 row flip",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8294 shared F0115_DUNGEONVIEW_DrawObjectsCreaturesProjectilesExplosions_CPSEF tail",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8295-8310 teleporter-only F0113_DUNGEONVIEW_DrawField tail",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293 ceiling pit dispatch excludes F0099_DUNGEONVIEW_CopyBitmapAndFlipHorizontal",
    "DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8273 stairs-down branch breaks before DUNVIEW.C:F0108_DUNGEONVIEW_DrawFloorOrnament:3940-3980 can be reached",
    "Source-locked contract_only=1; DUNVIEW.C:F0128_DUNGEONVIEW_DrawDungeonView:8336-8339 F0098 precedes DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8241-8284 stairs/pit overrides; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8255-8273 stairs-down excludes F0115 and F0108; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8274-8294 pit reaches F0115; DUNVIEW.C:F0127_DUNGEONVIEW_DrawSquareD0C:8285-8293 ceiling pit excludes F0099",
    10,
    20,
    30,
    30,
    40,
    50,
    60,
    DM1_V1_D0C_STAIRS_PIT_PC34_STAIRS_UP_SLOT_LEFT,
    DM1_V1_D0C_STAIRS_PIT_PC34_STAIRS_DOWN_SLOT_LEFT,
    DM1_V1_D0C_STAIRS_PIT_PC34_FLOOR_PIT_D0C_GRAPHIC,
    DM1_V1_D0C_STAIRS_PIT_PC34_INVISIBLE_FLOOR_PIT_D0C_GRAPHIC,
    DM1_V1_D0C_STAIRS_PIT_PC34_CEILING_PIT_D0C_GRAPHIC,
    DM1_V1_D0C_STAIRS_PIT_PC34_MEDIA720_VIEW_SQUARE_D0C,
    DM1_V1_D0C_STAIRS_PIT_PC34_LEGACY_VIEW_SQUARE_D0C,
    DM1_V1_D0C_STAIRS_PIT_PC34_CELL_ORDER_BACKLEFT_BACKRIGHT,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D0L,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_UP_D0R,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D0L,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_STAIRS_DOWN_D0R,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_FLOOR_PIT_D0C,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_CEILING_PIT_D0C,
    DM1_V1_D0C_STAIRS_PIT_PC34_ZONE_FIELD_D0C,
    true,
    true,
    false,
    true,
    false,
    false
};

const DM1_V1_D0CStairsPitDispatchContractPc34 *
dm1_v1_viewport_d0c_stairs_pit_dispatch_contract_pc34_compat(void)
{
    return &s_contract;
}
