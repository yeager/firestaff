#include "dm1_v1_early_viewport_family_audit_pc34_compat.h"

#include <stddef.h>

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C F0100:3048-3061 through F0120:7051-7330; "
    "F0112:4341-4470; F0128:8478-8533 dispatch. Each entry is bound to "
    "an existing DM1 source-receipt owner; this catalog performs no draw.";

static const DM1_V1_EarlyViewportAuditPc34Compat s_audit[] = {
    {100u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0100:3048-3061", "dm1_v1_viewport_3d_pc34_compat"},
    {101u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0101:3064-3078", "dm1_v1_viewport_3d_pc34_compat"},
    {102u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0102:3082-3094", "dm1_v1_viewport_3d_pc34_compat"},
    {103u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0103:3096-3108", "dm1_v1_viewport_3d_pc34_compat"},
    {104u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0104:3113-3156", "dm1_v1_viewport_3d_pc34_compat"},
    {105u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0105:3185-3247", "dm1_v1_viewport_3d_pc34_compat"},
    {106u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0106; VBLANK.C", "dungeonview_test_reset_to_step1_cpsf"},
    {107u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0107:3502-3938", "dm1_v1_ornament_cache_owner_pc34_compat"},
    {108u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0108:3940-4011", "dm1_v1_viewport_3d_pc34_compat"},
    {109u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0109:4013-4117", "dm1_v1_door_ornament_render_pc34_compat"},
    {110u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0110:4119-4217", "dm1_v1_viewport_3d_pc34_compat"},
    {111u, DM1_V1_EARLY_VIEWPORT_OWNER_PRIMITIVE, 1, 1,
     "DUNVIEW.C F0111:4218-4334", "dm1_v1_viewport_3d_pc34_compat"},
    {112u, DM1_V1_EARLY_VIEWPORT_OWNER_CEILING_PIT, 1, 1,
     "DUNVIEW.C F0112:4341-4470", "dm1_v1_ceiling_pit_viewport_pc34_compat"},
    {113u, DM1_V1_EARLY_VIEWPORT_OWNER_FIELD_OR_THING, 1, 1,
     "DUNVIEW.C F0113:4382-4474", "dm1_v1_viewport_3d_pc34_compat"},
    {114u, DM1_V1_EARLY_VIEWPORT_OWNER_FIELD_OR_THING, 1, 1,
     "DUNVIEW.C F0114:4476-4530", "dm1_v1_viewport_3d_pc34_compat"},
    {115u, DM1_V1_EARLY_VIEWPORT_OWNER_FIELD_OR_THING, 1, 1,
     "DUNVIEW.C F0115:4532-6360", "dm1_v1_viewport_3d_pc34_compat"},
    {116u, DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER, 1, 1,
     "DUNVIEW.C F0116:6361-6499", "dm1_v1_viewport_3d_pc34_compat"},
    {117u, DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER, 1, 1,
     "DUNVIEW.C F0117:6500-6641", "dm1_v1_viewport_3d_pc34_compat"},
    {118u, DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER, 1, 1,
     "DUNVIEW.C F0118:6642-6899", "dm1_v1_viewport_3d_pc34_compat"},
    {119u, DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER, 1, 1,
     "DUNVIEW.C F0119:6900-7050", "dm1_v1_viewport_3d_pc34_compat"},
    {120u, DM1_V1_EARLY_VIEWPORT_OWNER_SQUARE_SCHEDULER, 1, 1,
     "DUNVIEW.C F0120:7051-7330", "dm1_v1_viewport_3d_pc34_compat"}
};

const DM1_V1_EarlyViewportAuditPc34Compat *
dm1_v1_early_viewport_family_audit_pc34(uint16_t function_number)
{
    size_t index;

    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index) {
        if (s_audit[index].functionNumber == function_number) return &s_audit[index];
    }
    return NULL;
}

const char *dm1_v1_early_viewport_family_source_evidence_pc34(void)
{
    return s_source_evidence;
}
