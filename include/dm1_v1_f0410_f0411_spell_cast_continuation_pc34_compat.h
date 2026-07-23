#ifndef FIRESTAFF_DM1_V1_F0410_F0411_SPELL_CAST_CONTINUATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0410_F0411_SPELL_CAST_CONTINUATION_PC34_COMPAT_H

#include "dm1_v1_f0408_f0409_spell_cast_admission_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0410F0411SpellCastRequestPc34 {
    int casterIndex;
    int magicCasterLive;
    unsigned int sourceTick;
} DM1_V1_F0410F0411SpellCastRequestPc34;

typedef struct DM1_V1_F0410F0411SpellCastReceiptPc34 {
    int accepted;
    int failureFeedbackAdmitted;
    int failureType;
    int messageColor;
    int printsChampionName;
    int appendsBaseSkillName;
    int flaskFound;
    int flaskSlotIndex;
    unsigned short flaskThing;
    unsigned int sourceTick;
    int suppressSyntheticFallback;
} DM1_V1_F0410F0411SpellCastReceiptPc34;

/*
 * Consumes a same-tick F0408/F0409 C108 admission and an already produced
 * F0412 receipt. F0410 only publishes its source-text metadata and F0411
 * only identifies the action-hand-first empty flask. Neither function may
 * execute, clear symbols, draw pixels, or mutate an object.
 */
int dm1_v1_f0410_f0411_spell_cast_continue_pc34(
    const DM1_V1_F0410F0411SpellCastRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_F0408F0409SpellCastReceiptPc34 *castReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    const DM1_SpellF0412RuntimeReceipt *runtimeReceipt,
    const DM1_SpellPotionInventory *inventory,
    DM1_V1_F0410F0411SpellCastReceiptPc34 *outReceipt);

const char *dm1_v1_f0410_f0411_spell_cast_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
