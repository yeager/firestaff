#include "dm1_v1_action_spell_render_command_admission_pc34_compat.h"

#include <string.h>

static int
dm1_v1_action_spell_render_has_pc34_dimensions(
    const DM1_V1_ActionSpellHudSurfacePc34 *surface)
{
    if (!surface) return 0;
    switch (surface->graphicId) {
        case 9:  return surface->width == 87 && surface->height == 25;
        case 10: return surface->width == 87 && surface->height == 45;
        case 11: return surface->width == 14 && surface->height == 39;
        case 14: return surface->width == 88 && surface->height == 45;
        default: return 1;
    }
}

static int
dm1_v1_action_spell_render_nonempty_source_pixels_pc34(
    const DM1_V1_ActionSpellHudSurfacePc34 *surface)
{
    int index;

    if (!surface || !surface->pixels || surface->pixelCount <= 0) return 0;
    for (index = 0; index < surface->pixelCount; ++index) {
        if (surface->pixels[index] != 0u) return 1;
    }
    return 0;
}

static int
dm1_v1_action_spell_render_nonempty_step_region_pc34(
    const DM1_V1_ActionSpellHudSurfacePc34 *surface,
    const DM1_V1_ActionSpellPresentationSequenceStepPc34 *step)
{
    int x;
    int y;

    if (!surface || !step || !surface->pixels || step->sourceX < 0 ||
        step->sourceY < 0 || step->sourceW <= 0 || step->sourceH <= 0 ||
        surface->width < step->sourceX + step->sourceW ||
        surface->height < step->sourceY + step->sourceH) {
        return 0;
    }
    for (y = step->sourceY; y < step->sourceY + step->sourceH; ++y) {
        for (x = step->sourceX; x < step->sourceX + step->sourceW; ++x) {
            if (surface->pixels[y * surface->width + x] != 0u) return 1;
        }
    }
    return 0;
}

static int
dm1_v1_action_spell_render_find_surface_pc34(
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    const DM1_V1_ActionSpellPresentationSequenceStepPc34 *step)
{
    int i;
    if (!materials || !materials->surfaces || materials->surfaceCount <= 0 ||
        !step || step->graphicId <= 0) {
        return -1;
    }
    for (i = 0; i < materials->surfaceCount; ++i) {
        const DM1_V1_ActionSpellHudSurfacePc34 *surface =
            &materials->surfaces[i];
        if (surface->graphicId != step->graphicId || !surface->sourceOwned ||
            !dm1_v1_action_spell_render_nonempty_source_pixels_pc34(surface) ||
            !dm1_v1_action_spell_render_has_pc34_dimensions(surface)) {
            continue;
        }
        if (step->kind == DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34) {
            /* M653 is the only HUD font source in the PC34 path. */
            if (surface->graphicId != 695 && surface->graphicId != 557) {
                continue;
            }
            return i;
        }
        if (step->sourceX < 0 || step->sourceY < 0 || step->sourceW <= 0 ||
            step->sourceH <= 0 || surface->width < step->sourceX + step->sourceW ||
            surface->height < step->sourceY + step->sourceH ||
            surface->pixelCount < surface->width * surface->height ||
            !dm1_v1_action_spell_render_nonempty_step_region_pc34(
                surface, step)) {
            continue;
        }
        return i;
    }
    return -1;
}

int
dm1_v1_action_spell_render_command_admit_pc34(
    const DM1_V1_ActionSpellPresentationSequenceReceiptPc34 *sequence,
    const DM1_V1_ActionSpellHudMaterialSetPc34 *materials,
    DM1_V1_ActionSpellRenderCommandReceiptPc34 *outReceipt)
{
    int i;

    if (!outReceipt) return 0;
    memset(outReceipt, 0, sizeof(*outReceipt));
    if (!sequence || !sequence->accepted || !sequence->drawable ||
        sequence->stepCount <= 0 ||
        sequence->stepCount > DM1_V1_ACTION_SPELL_RENDER_COMMAND_MAX_PC34) {
        return 0;
    }
    outReceipt->presentationKind = sequence->presentationKind;

    for (i = 0; i < sequence->stepCount; ++i) {
        const DM1_V1_ActionSpellPresentationSequenceStepPc34 *step =
            &sequence->steps[i];
        DM1_V1_ActionSpellRenderCommandPc34 *command;
        int surfaceIndex;

        if ((step->kind != DM1_V1_ACTION_SPELL_SEQUENCE_STEP_BLIT_PC34 &&
             step->kind != DM1_V1_ACTION_SPELL_SEQUENCE_STEP_FONT_ZONE_PC34) ||
            step->zoneId <= 0 || step->zoneCount <= 0) {
            return 0;
        }
        surfaceIndex = dm1_v1_action_spell_render_find_surface_pc34(
            materials, step);
        if (surfaceIndex < 0) return 0;

        command = &outReceipt->commands[outReceipt->commandCount++];
        command->kind = step->kind;
        command->graphicId = step->graphicId;
        command->zoneId = step->zoneId;
        command->zoneCount = step->zoneCount;
        command->sourceX = step->sourceX;
        command->sourceY = step->sourceY;
        command->sourceW = step->sourceW;
        command->sourceH = step->sourceH;
        command->sourceSurfaceIndex = surfaceIndex;
        ++outReceipt->sourceOwnedCommandCount;
    }

    outReceipt->accepted = 1;
    outReceipt->drawable = 1;
    return 1;
}
