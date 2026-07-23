#ifndef FIRESTAFF_DM1_V1_F0369_SPELL_ZONE_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0369_SPELL_ZONE_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* COMMAND.C C100 parent plus CLIKMENU.C F0369 C101..C108 children. */
enum {
    DM1_V1_F0369_C100_SPELL_PARENT_PC34 = 100,
    DM1_V1_F0369_C101_RUNE_FIRST_PC34 = 101,
    DM1_V1_F0369_C106_RUNE_LAST_PC34 = 106,
    DM1_V1_F0369_C107_RECANT_PC34 = 107,
    DM1_V1_F0369_C108_CAST_PC34 = 108
};

typedef struct DM1_V1_F0369SpellZoneRequestPc34 {
    int screenX;
    int screenY;
    int candidatePanelActive;
    int magicCasterLive;
    unsigned int sourceTick;
} DM1_V1_F0369SpellZoneRequestPc34;

typedef struct DM1_V1_F0369SpellZoneReceiptPc34 {
    int accepted;
    int commandId;
    int zoneIndex;
    int runeIndex;
    int recant;
    int cast;
    int parentGraphicId;
    int linesGraphicId;
    int fontGraphicId;
    unsigned int sourceTick;
    int suppressSyntheticFallback;
} DM1_V1_F0369SpellZoneReceiptPc34;

/*
 * Admits a spell-panel child click only through the source-owned C100/C101..
 * C108 matrix and an already-authenticated C009/C011/M653 receipt.  It emits
 * no pixels and does not execute F0412 cast effects.
 */
int dm1_v1_f0369_spell_zone_admit_pc34(
    const DM1_V1_F0369SpellZoneRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_V1_F0369SpellZoneReceiptPc34 *outReceipt);

const char *dm1_v1_f0369_spell_zone_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
