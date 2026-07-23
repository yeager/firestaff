#ifndef FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_COMMAND_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ACTION_SPELL_RENDER_COMMAND_ADMISSION_PC34_COMPAT_H

#include "dm1_v1_action_spell_presentation_sequence_pc34_compat.h"

/*
 * ReDMCSB ACTIDRAW.C F0387/F0386 and CASTER.C F0394/F0396/F0397/F0398.
 * This is the last DM1-owned boundary before a presentation backend receives
 * a command.  It admits source-owned decoded GRAPHICS.DAT payloads only.
 */
enum {
    DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34 =
        DM1_V1_ACTION_SPELL_SEQUENCE_MAX_STEPS_PC34
};

typedef struct {
    int kind;
    int graphicId;
    int zoneId;
    int zoneCount;
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int sourceSurfaceIndex;
} DM1_V1_ActionSpellRenderCommandPc34;

typedef struct {
    int accepted;
    int drawable;
    int presentationKind;
    int commandCount;
    int sourceOwnedCommandCount;
    DM1_V1_ActionSpellRenderCommandPc34
        commands[DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34];
} DM1_V1_ActionSpellRenderCommandReceiptPc34;

/*
 * Each admitted command refers directly to one nonempty source-owned
 * GRAPHICS.DAT surface supplied to the previous material boundary. Any absent,
 * detached, blank, undersized, mismatched surface, or non-M653 font rejects
 * the entire batch before publication.
 */
int dm1_v1_action_spell_render_command_admit_pc34(
    const DM1_V1_ActionSpellPresentationSequenceReceiptPc34 *sequence,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellRenderCommandReceiptPc34 *outReceipt);

#endif
