#include "csb_v1_f1206_f1225_source_ownership_pc34_compat.h"

static const CSB_V1_F1206F1225OwnershipPc34 k_ownership[] = {
    { 1206u, CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1206_UpdateColorPalette", "ANIM.C:480", "No authenticated CSB PC34 palette receipt; generated palette updates are blocked." },
    { 1207u, CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1207_", "ANIM.C:1150", "Unnamed sound route; no authenticated CSB PC34 sound data or playback owner." },
    { 1208u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1208", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1209u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1209", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1210u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "F1210_", "ANIM.C:1209", "Unnamed route with function-local delay storage; no standalone CSB PC34 owner." },
    { 1211u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1211", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1212u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1212", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1213u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1213", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1214u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1214", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1215u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1215", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1216u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1216", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1217u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1217", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1218u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1218", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1219u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1219", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1220u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1220", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1221u, CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1221_AllocMemForSD", "ANIM.C:494", "No authenticated CSB PC34 sound-device allocation route; no host allocation substitute." },
    { 1222u, CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1222_FreeMemForSD", "ANIM.C:495", "No authenticated CSB PC34 sound-device free route; no host deallocation substitute." },
    { 1223u, CSB_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1223_", "ANIM.C:496", "Unnamed sound-data route; no authenticated CSB PC34 sound data or playback owner." },
    { 1224u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1224", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot." },
    { 1225u, CSB_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "F1225_", "ANIM.C:1423", "Unnamed route with function-local state; no standalone CSB PC34 owner." }
};

const CSB_V1_F1206F1225OwnershipPc34 *
csb_v1_f1206_f1225_source_ownership_pc34(unsigned int number)
{
    if (number < 1206u || number > 1225u) return 0;
    return &k_ownership[number - 1206u];
}

int csb_v1_f1206_f1225_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int csb_v1_f1206_f1225_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *csb_v1_f1206_f1225_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C:480,494-496,1150,1209,1423 and callable/label "
           "inventories. No generated CSB UI, graphics, timing, palette, sound, "
           "or memory route is admitted without authenticated PC34 data.";
}
