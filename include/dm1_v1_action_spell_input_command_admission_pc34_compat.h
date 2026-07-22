#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_INPUT_COMMAND_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_INPUT_COMMAND_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

/* ReDMCSB COMMAND.C action/rune routes into ACTIDRAW.C F0387 and CASTER.C
 * F0394/F0399. This boundary admits source input only; it owns no host UI. */
enum {
    DM1_V1_ACTION_SPELL_INPUT_ACTION_SELECT_PC34 = 1,
    DM1_V1_ACTION_SPELL_INPUT_SPELL_RUNE_COMMIT_PC34 = 2
};

typedef struct {
    int kind;
    int active;
    int candidatePanelActive;
    int championIndex;
    int championCount;
    int actionIndex;
    int actionMenuRowCount;
    int selectedActionRow;
    DM1_V1_SpellPanelStatePc34 spellPanel;
    int runeSymbolIndex;
    unsigned int sourceTick;
} DM1_V1_ActionSpellInputCommandRequestPc34;

typedef struct {
    int accepted;
    int kind;
    int presentationKind;
    int championIndex;
    int actionIndex;
    int commandZoneId;
    int commandGraphicId;
    int secondaryGraphicId;
    int fontGraphicId;
    int runeValue;
    int runeRow;
    int runeCount;
    unsigned int sourceTick;
} DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34;

/*
 * Validates the original material route and source input geometry before any
 * later sequence/command builder can consume the request.
 */
int dm1_v1_action_spell_input_command_admit_pc34(
    const DM1_V1_ActionSpellInputCommandRequestPc34 *request,
    const DM1_V1_ActionSpellHudMaterialReceiptPc34 *materials,
    DM1_V1_ActionSpellInputCommandAdmissionReceiptPc34 *outReceipt);

#endif
