#include "redmcsb_f0672_f0673_mouse_input_pc34_compat.h"

#include <stddef.h>

static void redmcsb_f0673_apply_zone_box_pc34_compat(
    redmcsb_f0672_f0673_mouse_input_pc34_compat *input,
    const redmcsb_f0672_f0673_runtime_pc34_compat *runtime,
    int16_t xyz[4])
{
    int16_t left = xyz[0];
    int16_t top = xyz[1];

    if (input->box.x1 == -2) {
        left = (int16_t)(left + runtime->viewport_screen_x);
        top = (int16_t)(top + runtime->viewport_screen_y);
    } else if (input->box.x1 == -3) {
        left = (int16_t)(left +
                         ((runtime->screen_width - runtime->viewport_width) >> 1));
        top = (int16_t)(top +
                        ((runtime->screen_height - runtime->viewport_height) >> 1));
    }
    input->box.x1 = left;
    input->box.y1 = top;
    input->box.x2 = (int16_t)(left + xyz[2] - 1);
    input->box.y2 = (int16_t)(top + xyz[3] - 1);
}

int redmcsb_f0673_set_mouse_input_boxes_from_zone_pc34_compat(
    redmcsb_f0672_f0673_mouse_input_pc34_compat *inputs,
    size_t input_count,
    const redmcsb_f0672_f0673_runtime_pc34_compat *runtime)
{
    size_t index;

    if (inputs == NULL || input_count == 0U || runtime == NULL ||
        runtime->get_zone == NULL) {
        return 0;
    }
    for (index = 0U; index < input_count; ++index) {
        int16_t xyz[4];

        if (inputs[index].command == REDMCSB_F0672_F0673_COMMAND_NONE) return 1;
        if (inputs[index].box.x1 < 0 &&
            runtime->get_zone(runtime->context, inputs[index].box.x2, xyz)) {
            redmcsb_f0673_apply_zone_box_pc34_compat(
                &inputs[index], runtime, xyz);
        }
    }
    return 0;
}

int redmcsb_f0672_initialize_all_mouse_input_pc34_compat(
    const redmcsb_f0672_f0673_mouse_input_group_pc34_compat groups[
        REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT],
    const redmcsb_f0672_f0673_runtime_pc34_compat *runtime)
{
    size_t group_index;

    if (groups == NULL || runtime == NULL) return 0;
    for (group_index = 0U;
         group_index < REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT;
         ++group_index) {
        if (!redmcsb_f0673_set_mouse_input_boxes_from_zone_pc34_compat(
                groups[group_index].inputs, groups[group_index].input_count,
                runtime)) {
            return 0;
        }
    }
    return 1;
}

const char *redmcsb_f0672_f0673_mouse_input_source_evidence_pc34(void)
{
    return "ReDMCSB COMMAND.C F0673_SetMouseInputBoxFromZone (1081-1096); "
           "F0672_InitializeAllMouseInput (1098-1111), I34E/I34M route";
}
