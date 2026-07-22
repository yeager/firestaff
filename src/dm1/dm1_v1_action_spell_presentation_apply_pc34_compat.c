#include "dm1_v1_action_spell_presentation_apply_pc34_compat.h"

#include <string.h>

static unsigned int
dm1_v1_action_spell_apply_fingerprint_pc34(
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command =
            &commands->commands[i];
        const int values[] = {
            command->kind, command->graphicId, command->zoneId,
            command->zoneCount, command->sourceX, command->sourceY,
            command->sourceW, command->sourceH, command->sourceSurfaceIndex
        };
        int valueIndex;
        for (valueIndex = 0;
             valueIndex < (int)(sizeof(values) / sizeof(values[0]));
             ++valueIndex) {
            unsigned int value = (unsigned int)values[valueIndex];
            int byteIndex;
            for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
                hash ^= (value >> (byteIndex * 8)) & 0xffu;
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

static int
dm1_v1_action_spell_apply_materials_current_pc34(
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials)
{
    int i;
    if (!commands || !materials || !materials->surfaces ||
        materials->surfaceCount <= 0) {
        return 0;
    }
    for (i = 0; i < commands->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command =
            &commands->commands[i];
        const DM1_V1_ActionSpellHudSurfacePc34 *surface;
        if (command->sourceSurfaceIndex < 0 ||
            command->sourceSurfaceIndex >= materials->surfaceCount) {
            return 0;
        }
        surface = &materials->surfaces[command->sourceSurfaceIndex];
        if (!surface->sourceOwned || !surface->pixels ||
            surface->pixelCount <= 0 || surface->graphicId != command->graphicId) {
            return 0;
        }
        if (command->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
            (command->sourceX < 0 || command->sourceY < 0 ||
             command->sourceW <= 0 || command->sourceH <= 0 ||
             surface->width < command->sourceX + command->sourceW ||
             surface->height < command->sourceY + command->sourceH ||
             surface->pixelCount < surface->width * surface->height)) {
            return 0;
        }
    }
    return 1;
}

void
dm1_v1_action_spell_presentation_frame_begin_pc34(
    DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    unsigned int frameTick)
{
    if (!state) return;
    if (!state->frameOpen || state->frameTick != frameTick) {
        memset(state, 0, sizeof(*state));
        state->frameOpen = 1;
        state->frameTick = frameTick;
    }
}

int
dm1_v1_action_spell_presentation_apply_pc34(
    DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    const DM1_V1_ActionSpellExecutionReceiptPc34 *execution,
    const DM1_V1_ActionSpellRenderCommandReceiptPc34 *commands,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellPresentationApplyReceiptPc34 *outReceipt)
{
    unsigned int fingerprint;

    if (outReceipt) memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !execution || !commands || !execution->accepted ||
        !execution->readyForPresentation || !commands->accepted ||
        !commands->drawable || !state->frameOpen ||
        commands->presentationKind != execution->presentationKind ||
        commands->commandCount <= 0 ||
        commands->commandCount != execution->commandCount ||
        commands->sourceOwnedCommandCount != execution->sourceOwnedCommandCount ||
        execution->sourceTick > state->frameTick ||
        !dm1_v1_action_spell_apply_materials_current_pc34(commands, materials)) {
        return 0;
    }
    fingerprint = dm1_v1_action_spell_apply_fingerprint_pc34(commands);
    if (fingerprint != execution->commandFingerprint) return 0;

    if (state->hasPresentation) {
        if (state->serial != execution->serial ||
            state->commandFingerprint != fingerprint) {
            return 0;
        }
        if (outReceipt) {
            outReceipt->accepted = 1;
            outReceipt->alreadyApplied = 1;
            outReceipt->presentationKind = execution->presentationKind;
            outReceipt->commandCount = execution->commandCount;
            outReceipt->frameTick = state->frameTick;
            outReceipt->sourceTick = execution->sourceTick;
            outReceipt->serial = execution->serial;
            outReceipt->commandFingerprint = fingerprint;
        }
        return 1;
    }

    state->hasPresentation = 1;
    state->presentationKind = execution->presentationKind;
    state->championIndex = execution->championIndex;
    state->actionIndex = execution->actionIndex;
    state->spellKind = execution->spellKind;
    state->spellPowerOrdinal = execution->spellPowerOrdinal;
    state->sourceTick = execution->sourceTick;
    state->serial = execution->serial;
    state->commandFingerprint = fingerprint;
    state->commandCount = commands->commandCount;
    memcpy(state->commands, commands->commands,
           (size_t)commands->commandCount * sizeof(state->commands[0]));

    if (outReceipt) {
        outReceipt->accepted = 1;
        outReceipt->applied = 1;
        outReceipt->presentationKind = state->presentationKind;
        outReceipt->commandCount = state->commandCount;
        outReceipt->frameTick = state->frameTick;
        outReceipt->sourceTick = state->sourceTick;
        outReceipt->serial = state->serial;
        outReceipt->commandFingerprint = state->commandFingerprint;
    }
    return 1;
}
