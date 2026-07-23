#include "dm1_v1_f1206_f1225_source_ownership_pc34_compat.h"

static const DM1_V1_F1206F1225OwnershipPc34 kOwnership[] = {
    {1206, DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1206_UpdateColorPalette", "ANIM.C:480", "No authenticated PC34 palette receipt; generated palette updates are blocked."},
    {1207, DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1207_", "ANIM.C:1150", "Unnamed sound route; no authenticated PC34 sound data or playback owner."},
    {1208, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1208", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1209, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1209", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1210, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "F1210_", "ANIM.C:1209", "Unnamed route with function-local delay storage; no standalone PC34 owner."},
    {1211, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1211", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1212, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1212", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1213, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1213", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1214, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1214", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1215, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1215", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1216, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1216", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1217, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1217", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1218, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1218", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1219, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1219", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1220, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1220", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1221, DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1221_AllocMemForSD", "ANIM.C:494", "No authenticated PC34 sound-device allocation route; no host allocation substitute."},
    {1222, DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1222_FreeMemForSD", "ANIM.C:495", "No authenticated PC34 sound-device free route; no host deallocation substitute."},
    {1223, DM1_V1_F1206_F1225_PLATFORM_BOUNDARY_PC34, "F1223_", "ANIM.C:496", "Unnamed sound-data route; no authenticated PC34 sound data or playback owner."},
    {1224, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "slot 1224", "ReDMCSB callable inventory", "No callable symbol is assigned to this numeric slot."},
    {1225, DM1_V1_F1206_F1225_UNNAMED_OR_LOCAL_PC34, "F1225_", "ANIM.C:1423", "Unnamed route with function-local state; no standalone PC34 owner."},
};

const DM1_V1_F1206F1225OwnershipPc34 *
dm1_v1_f1206_f1225_source_ownership_pc34(unsigned int number)
{
    if (number < 1206U || number > 1225U) return 0;
    return &kOwnership[number - 1206U];
}

int dm1_v1_f1206_f1225_admits_authentic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

int dm1_v1_f1206_f1225_has_synthetic_route_pc34(unsigned int number)
{
    (void)number;
    return 0;
}

const char *dm1_v1_f1206_f1225_source_ownership_evidence_pc34(void)
{
    return "ReDMCSB ANIM.C:480,494-496,1150,1209,1423 and callable/label "
           "inventories. No generated UI, graphics, timing, palette, sound, "
           "or memory route is admitted without authenticated PC34 data.";
}
