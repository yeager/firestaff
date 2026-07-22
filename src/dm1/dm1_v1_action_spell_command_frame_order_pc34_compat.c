#include "dm1_v1_action_spell_command_frame_order_pc34_compat.h"

#include "firestaff/dm1/v1/box_action_area_pc34_compat.h"
#include "firestaff/dm1/v1/box_spell_area_pc34_compat.h"

#include <string.h>

enum {
    kGraphicCreatureDamage = 14,
    kGraphicHudFontPrimary = 695,
    kGraphicHudFontAlternate = 557
};

static int
dm1_v1_action_spell_order_surface_current_pc34(
    const DM1_V1_ActionSpellRenderCommandPc34 *command,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials)
{
    const DM1_V1_ActionSpellHudSurfacePc34 *surface;
    if (!command || !materials || !materials->surfaces ||
        command->sourceSurfaceIndex < 0 ||
        command->sourceSurfaceIndex >= materials->surfaceCount) {
        return 0;
    }
    surface = &materials->surfaces[command->sourceSurfaceIndex];
    if (!surface->sourceOwned || !surface->pixels || surface->pixelCount <= 0 ||
        surface->graphicId != command->graphicId) {
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
    return 1;
}

static int
dm1_v1_action_spell_order_is_font_pc34(
    const DM1_V1_ActionSpellRenderCommandPc34 *command)
{
    return command &&
           command->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34 &&
           (command->graphicId == kGraphicHudFontPrimary ||
            command->graphicId == kGraphicHudFontAlternate);
}

static int
dm1_v1_action_spell_order_validate_damage_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state)
{
    const DM1_V1_ActionSpellRenderCommandPc34 *first = &state->commands[0];
    const DM1_V1_ActionSpellRenderCommandPc34 *second = &state->commands[1];
    return state->commandCount == 2 &&
           first->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
           first->graphicId == kGraphicCreatureDamage && first->zoneId == 75 &&
           first->sourceX == 0 && first->sourceY == 0 &&
           first->sourceW == 88 && first->sourceH == 45 &&
           dm1_v1_action_spell_order_is_font_pc34(second) && second->zoneId == 75;
}

static int
dm1_v1_action_spell_order_validate_action_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state)
{
    const DM1_V1_ActionSpellRenderCommandPc34 *first = &state->commands[0];
    int i;
    if (state->commandCount < 3 || state->commandCount > 5 ||
        first->kind != DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 ||
        first->graphicId != DM1_V1_ACTION_AREA_GRAPHIC_ID_PC34 ||
        first->zoneId != DM1_V1_ACTION_AREA_ZONE_ID_PC34 ||
        first->sourceX != 0 || first->sourceY != 0 ||
        first->sourceW != 87 || first->sourceH != 45 ||
        !dm1_v1_action_spell_order_is_font_pc34(&state->commands[1]) ||
        state->commands[1].zoneId != DM1_V1_ACTION_MENU_HEADER_ZONE_ID_PC34) {
        return 0;
    }
    for (i = 2; i < state->commandCount; ++i) {
        if (!dm1_v1_action_spell_order_is_font_pc34(&state->commands[i]) ||
            state->commands[i].zoneId !=
                dm1_v1_action_menu_row_zone_id_pc34(i - 2)) {
            return 0;
        }
    }
    return 1;
}

static int
dm1_v1_action_spell_order_validate_spell_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state)
{
    const DM1_V1_ActionSpellRenderCommandPc34 *background = &state->commands[0];
    const DM1_V1_ActionSpellRenderCommandPc34 *available = &state->commands[1];
    const DM1_V1_ActionSpellRenderCommandPc34 *selected = &state->commands[2];
    const DM1_V1_ActionSpellRenderCommandPc34 *font = &state->commands[3];
    return state->commandCount == 4 &&
           background->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
           background->graphicId == DM1_V1_SPELL_AREA_BACKGROUND_GRAPHIC_ID_PC34 &&
           background->zoneId == DM1_V1_SPELL_AREA_ZONE_ID_PC34 &&
           background->sourceW == 87 && background->sourceH == 25 &&
           available->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
           available->graphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
           available->zoneId == DM1_V1_SPELL_AVAILABLE_SYMBOL_PARENT_ZONE_ID_BASE_PC34 &&
           available->zoneCount == DM1_V1_SPELL_RUNE_SYMBOLS_PER_ROW_PC34 &&
           available->sourceY == DM1_V1_SPELL_AREA_LINES_AVAILABLE_Y_PC34 &&
           available->sourceW == 14 && available->sourceH == 12 &&
           selected->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
           selected->graphicId == DM1_V1_SPELL_AREA_LINES_GRAPHIC_ID_PC34 &&
           selected->zoneId == DM1_V1_SPELL_CHAMPION_SYMBOL_ZONE_ID_BASE_PC34 &&
           selected->zoneCount == DM1_V1_SPELL_RUNE_SEQUENCE_MAX_PC34 &&
           selected->sourceY == DM1_V1_SPELL_AREA_LINES_SELECTED_Y_PC34 &&
           selected->sourceW == 14 && selected->sourceH == 12 &&
           dm1_v1_action_spell_order_is_font_pc34(font) &&
           font->zoneId == DM1_V1_SPELL_CASTER_PANEL_ZONE_ID_PC34;
}

static unsigned int
dm1_v1_action_spell_order_fingerprint_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state)
{
    unsigned int hash = 2166136261u;
    int i;
    for (i = 0; i < state->commandCount; ++i) {
        const DM1_V1_ActionSpellRenderCommandPc34 *command = &state->commands[i];
        const int values[] = { command->graphicId, command->zoneId,
            command->zoneCount, command->sourceX, command->sourceY,
            command->sourceW, command->sourceH, command->sourceSurfaceIndex };
        int j;
        for (j = 0; j < (int)(sizeof(values) / sizeof(values[0])); ++j) {
            unsigned int value = (unsigned int)values[j];
            int byteIndex;
            for (byteIndex = 0; byteIndex < 4; ++byteIndex) {
                hash ^= (value >> (byteIndex * 8)) & 0xffu;
                hash *= 16777619u;
            }
        }
    }
    return hash;
}

int
dm1_v1_action_spell_command_frame_order_build_pc34(
    const DM1_V1_ActionSpellPresentationFrameStatePc34 *state,
    const DM1_V1_ActionSpellPresentationApplyReceiptPc34 *applyReceipt,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellCommandFrameOrderReceiptPc34 *outReceipt)
{
    int validOrder;
    int i;
    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!state || !applyReceipt || !materials || !state->frameOpen ||
        !state->hasPresentation || !applyReceipt->accepted ||
        (!applyReceipt->applied && !applyReceipt->alreadyApplied) ||
        applyReceipt->frameTick != state->frameTick ||
        applyReceipt->sourceTick != state->sourceTick ||
        applyReceipt->serial != state->serial ||
        applyReceipt->presentationKind != state->presentationKind ||
        applyReceipt->commandCount != state->commandCount ||
        applyReceipt->commandFingerprint != state->commandFingerprint ||
        state->commandCount <= 0 ||
        state->commandCount > DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34) {
        return 0;
    }
    for (i = 0; i < state->commandCount; ++i) {
        if (!dm1_v1_action_spell_order_surface_current_pc34(
                &state->commands[i], materials)) {
            return 0;
        }
    }
    switch (state->presentationKind) {
        case DM1_V1_ACTION_HUD_PRESENTATION_DAMAGE_PC34:
            validOrder = dm1_v1_action_spell_order_validate_damage_pc34(state);
            break;
        case DM1_V1_ACTION_HUD_PRESENTATION_ACTION_LOCK_PC34:
            validOrder = dm1_v1_action_spell_order_validate_action_pc34(state);
            break;
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_POTION_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_PROJECTILE_PC34:
        case DM1_V1_ACTION_HUD_PRESENTATION_SPELL_EFFECT_PC34:
            validOrder = dm1_v1_action_spell_order_validate_spell_pc34(state);
            break;
        default:
            validOrder = 0;
            break;
    }
    if (!validOrder) return 0;

    outReceipt->accepted = 1;
    outReceipt->readyForPresentation = 1;
    outReceipt->presentationKind = state->presentationKind;
    outReceipt->commandCount = state->commandCount;
    outReceipt->frameTick = state->frameTick;
    outReceipt->sourceTick = state->sourceTick;
    outReceipt->serial = state->serial;
    outReceipt->commandFingerprint = state->commandFingerprint;
    outReceipt->orderingFingerprint = dm1_v1_action_spell_order_fingerprint_pc34(state);
    for (i = 0; i < state->commandCount; ++i) {
        outReceipt->orderedSurfaceIndices[i] =
            state->commands[i].sourceSurfaceIndex;
    }
    return 1;
}
