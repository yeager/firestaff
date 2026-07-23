#ifndef FIRESTAFF_DM1_V1_F0408_F0409_SPELL_CAST_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0408_F0409_SPELL_CAST_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_f0369_spell_zone_admission_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0408F0409SpellCastRequestPc34 {
    int casterIndex;
    int magicCasterLive;
    unsigned int sourceTick;
} DM1_V1_F0408F0409SpellCastRequestPc34;

typedef struct DM1_V1_F0408F0409SpellCastReceiptPc34 {
    int accepted;
    int commandId;
    int lookupMatched;
    int spellIndex;
    int powerOrdinal;
    int f0412DispatchRequired;
    unsigned int sourceTick;
    int suppressSyntheticFallback;
} DM1_V1_F0408F0409SpellCastReceiptPc34;

/*
 * Admits only a same-tick C108 cast click authenticated by F0369 and the
 * original C009/C011/M653 material triplet. F0409 lookup is performed over
 * the existing source-locked spell table. F0412 remains the sole owner of
 * cast results, symbol clearing, and all runtime mutations.
 */
int dm1_v1_f0408_f0409_spell_cast_admit_pc34(
    const DM1_V1_F0408F0409SpellCastRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    const DM1_SpellCastingState *spellState,
    DM1_V1_F0408F0409SpellCastReceiptPc34 *outReceipt);

const char *dm1_v1_f0408_f0409_spell_cast_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
