#ifndef FIRESTAFF_DM1_V1_F0399_F0400_SPELL_SYMBOL_CONSUME_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0399_F0400_SPELL_SYMBOL_CONSUME_PC34_COMPAT_H

#include "dm1_v1_f0369_spell_zone_admission_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0399F0400SpellSymbolRequestPc34 {
    int casterIndex;
    int magicCasterLive;
    unsigned int sourceTick;
} DM1_V1_F0399F0400SpellSymbolRequestPc34;

typedef struct DM1_V1_F0399F0400SpellSymbolReceiptPc34 {
    int accepted;
    int commandId;
    int symbolAdded;
    int symbolDeleted;
    int symbolIndex;
    int symbolStepBefore;
    int symbolStepAfter;
    int manaBefore;
    int manaAfter;
    unsigned int sourceTick;
    int suppressSyntheticFallback;
} DM1_V1_F0399F0400SpellSymbolReceiptPc34;

/*
 * Consumes only a same-tick F0369 C101..C107 receipt backed by the original
 * C009/C011/M653 material triplet. C108/F0412 is intentionally outside this
 * symbol-mutation boundary.
 */
int dm1_v1_f0399_f0400_spell_symbol_consume_pc34(
    const DM1_V1_F0399F0400SpellSymbolRequestPc34 *request,
    const DM1_V1_F0369SpellZoneReceiptPc34 *zoneReceipt,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_SpellCastingState *spellState,
    DM1_ChampionSpellStats *casterStats,
    DM1_V1_F0399F0400SpellSymbolReceiptPc34 *outReceipt);

const char *dm1_v1_f0399_f0400_spell_symbol_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
