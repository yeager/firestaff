#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_M11_BLIT_PLAN_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_M11_BLIT_PLAN_PC34_COMPAT_H

#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"

enum { DM1_V1_ACTION_SPELL_M11_BLIT_MAX_PC34 = 3 };

typedef struct {
    int graphicId;
    int zoneId;
    int zoneCount;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int destinationX;
    int destinationY;
} DM1_V1_ActionSpellM11BlitPc34;

typedef struct {
    int accepted;
    int presentationKind;
    int clearX;
    int clearY;
    int clearW;
    int clearH;
    int blitCount;
    DM1_V1_ActionSpellM11BlitPc34 blits[DM1_V1_ACTION_SPELL_M11_BLIT_MAX_PC34];
} DM1_V1_ActionSpellM11BlitPlanPc34;

/* Maps F0387/F0394's real GRAPHICS.DAT source regions to their PC34 zones.
 * The plan carries no pixels and never manufactures a fallback surface. */
int dm1_v1_action_spell_m11_blit_plan_build_pc34(
    int presentationKind,
    int actionMenuRowCount,
    DM1_V1_ActionSpellM11BlitPlanPc34 *outPlan);

#endif
