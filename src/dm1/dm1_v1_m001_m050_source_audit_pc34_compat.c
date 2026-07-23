#include "dm1_v1_m001_m050_source_audit_pc34_compat.h"

#define M(number, name, anchor) {number, name, anchor, "macro/local state; no standalone verified Firestaff owner"}

static const Dm1V1M001M050SourceAuditPc34 kSymbols[] = {
    M(1, "M001_ORDINAL_TO_INDEX", "COMPILE.H:1039"),
    M(2, "M002_RANDOM", "DEFS.H:1"),
    M(3, "M003_RANDOM", "DEFS.H:2"),
    M(4, "M004_RANDOM", "DEFS.H:3"),
    M(5, "M005_RANDOM", "DEFS.H:4"),
    M(6, "M006_RANDOM", "DEFS.H:5"),
    M(7, "M007_GET", "COMPILE.H:1041"),
    M(8, "M008_SET", "COMPILE.H:1042"),
    M(9, "M009_CLEAR", "COMPILE.H:1044"),
    M(10, "M010_TOGGLE", "COMPILE.H:1043"),
    M(11, "M011_CELL", "DEFS.H:398"),
    M(12, "M012_TYPE", "DEFS.H:399"),
    M(13, "M013_INDEX", "DEFS.H:400"),
    M(14, "M014_TYPE_AND_INDEX", "DEFS.H:401"),
    M(15, "M015_THING_WITH_NEW_CELL", "DEFS.H:402"),
    M(16, "M016_IS_ORIENTED_WEST_EAST", "DEFS.H:455"),
    M(17, "M017_NEXT", "DEFS.H:458"),
    M(18, "M018_OPPOSITE", "DEFS.H:463"),
    M(19, "M019_PREVIOUS", "DEFS.H:464"),
    M(20, "M020_PREVIOUS", "DEFS.H:465"),
    M(21, "M021_NORMALIZE", "DEFS.H:466"),
    M(22, "M022_HORIZONTAL_OFFSET", "DEFS.H:591"),
    M(23, "M023_VERTICAL_OFFSET", "DEFS.H:592"),
    M(24, "M024_SET_HORIZONTAL_OFFSET", "DEFS.H:603"),
    M(25, "M025_SET_VERTICAL_OFFSET", "DEFS.H:604"),
    M(26, "M026_CHAMPION_ICON_INDEX", "DEFS.H:718"),
    M(27, "M027_PORTRAIT_X", "DEFS.H:821"),
    M(28, "M028_PORTRAIT_Y", "DEFS.H:822"),
    M(29, "M029_MAP", "DEFS.H:922"),
    M(30, "M030_TIME", "DEFS.H:923"),
    M(31, "M031_SET_MAP", "DEFS.H:924"),
    M(32, "M032_SET_TIME", "DEFS.H:925"),
    M(33, "M033_SET_MAP_AND_TIME", "DEFS.H:926"),
    M(34, "M034_SQUARE_TYPE", "DEFS.H:1001"),
    M(35, "M035_SQUARE", "DEFS.H:1002"),
    M(36, "M036_DOOR_STATE", "DEFS.H:1046"),
    M(37, "M037_SET_DOOR_STATE", "DEFS.H:1047"),
    M(38, "M038_DISTANCE", "DEFS.H:1121"),
    M(39, "M039_TYPE", "DEFS.H:1295"),
    M(40, "M040_DATA", "DEFS.H:1296"),
    M(41, "M041_SET_DATA", "DEFS.H:1297"),
    M(42, "M042_MASK_CURRENT", "DEFS.H:1298"),
    M(43, "M043_MASK_REFERENCE", "DEFS.H:1299"),
    M(44, "M044_SET_TYPE_DISABLED", "DEFS.H:1300"),
    M(45, "M045_HEALTH_MULTIPLIER", "DEFS.H:1301"),
    M(46, "M046_TICKS", "DEFS.H:1302"),
    M(47, "M047_KINETIC_ENERGY", "DEFS.H:1303"),
    M(48, "M048_STEP_ENERGY", "DEFS.H:1304"),
    M(49, "M049_LOCAL_EFFECT", "DEFS.H:1305"),
    M(50, "M050_CREATURE_VALUE", "DEFS.H:1369"),
};

const Dm1V1M001M050SourceAuditPc34 *
dm1_v1_m001_m050_source_audit_pc34(unsigned int number)
{
    if (number < 1U || number > 50U) return 0;
    return &kSymbols[number - 1U];
}

int dm1_v1_m001_m050_has_verified_owner_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_m001_m050_has_synthetic_route_pc34(void)
{
    return 0;
}

const char *dm1_v1_m001_m050_source_audit_evidence_pc34(void)
{
    return "ReDMCSB REDMCSB_LABEL_PARAMETER_FULL_AUDIT.tsv M001-M050; "
           "COMPILE.H:1039-1044 and DEFS.H:1-1369. These are macro/module "
           "expressions or local layout names, not standalone callable behavior. "
           "No generated macro, graphics, UI, input, or timing route.";
}
