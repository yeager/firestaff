#include "redmcsb_f0672_f0673_mouse_input_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct zone_fixture {
    int call_count;
    int16_t seen_zone[16];
} zone_fixture;

static int get_zone(void *context, int16_t zone_index, int16_t xyz[4])
{
    zone_fixture *fixture = (zone_fixture *)context;

    fixture->seen_zone[fixture->call_count++] = zone_index;
    if (zone_index == 42) {
        xyz[0] = 10;
        xyz[1] = 20;
        xyz[2] = 30;
        xyz[3] = 40;
        return 1;
    }
    if (zone_index == 43) {
        xyz[0] = 5;
        xyz[1] = 7;
        xyz[2] = 8;
        xyz[3] = 9;
        return 1;
    }
    if (zone_index >= 100 && zone_index < 109) {
        xyz[0] = zone_index;
        xyz[1] = (int16_t)(zone_index + 1);
        xyz[2] = 2;
        xyz[3] = 3;
        return 1;
    }
    return 0;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    (void)fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    zone_fixture fixture;
    redmcsb_f0672_f0673_runtime_pc34_compat runtime;
    redmcsb_f0672_f0673_mouse_input_pc34_compat inputs[] = {
        {1, {-2, 42, 0, 0}},
        {2, {-3, 43, 0, 0}},
        {3, {7, 99, 8, 9}},
        {4, {-1, 77, 4, 5}},
        {REDMCSB_F0672_F0673_COMMAND_NONE, {0, 0, 0, 0}}
    };
    redmcsb_f0672_f0673_mouse_input_pc34_compat group_inputs[
        REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT][2];
    redmcsb_f0672_f0673_mouse_input_group_pc34_compat groups[
        REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT];
    size_t index;

    (void)memset(&fixture, 0, sizeof(fixture));
    runtime.get_zone = get_zone;
    runtime.context = &fixture;
    runtime.viewport_screen_x = 100;
    runtime.viewport_screen_y = 200;
    runtime.screen_width = 320;
    runtime.screen_height = 200;
    runtime.viewport_width = 224;
    runtime.viewport_height = 136;

    if (!expect(redmcsb_f0673_set_mouse_input_boxes_from_zone_pc34_compat(
                    inputs, sizeof(inputs) / sizeof(inputs[0]), &runtime),
                "F0673 resolves a command-none terminated table") ||
        !expect(inputs[0].box.x1 == 110 && inputs[0].box.y1 == 220 &&
                    inputs[0].box.x2 == 139 && inputs[0].box.y2 == 259,
                "F0673 applies the source -2 viewport offset and inclusive end") ||
        !expect(inputs[1].box.x1 == 53 && inputs[1].box.y1 == 39 &&
                    inputs[1].box.x2 == 60 && inputs[1].box.y2 == 47,
                "F0673 applies the source -3 centered viewport offset") ||
        !expect(inputs[2].box.x1 == 7 && inputs[2].box.x2 == 99 &&
                    inputs[2].box.y1 == 8 && inputs[2].box.y2 == 9,
                "F0673 preserves non-zone caller-owned boxes") ||
        !expect(inputs[3].box.x1 == -1 && inputs[3].box.x2 == 77,
                "F0673 leaves unresolved zones untouched") ||
        !expect(fixture.call_count == 3 && fixture.seen_zone[0] == 42 &&
                    fixture.seen_zone[1] == 43 && fixture.seen_zone[2] == 77,
                "F0673 resolves only negative zone boxes in table order")) {
        return 1;
    }

    (void)memset(&fixture, 0, sizeof(fixture));
    for (index = 0U; index < REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT;
         ++index) {
        group_inputs[index][0].command = 1;
        group_inputs[index][0].box.x1 = -1;
        group_inputs[index][0].box.x2 = (int16_t)(100 + index);
        group_inputs[index][0].box.y1 = 0;
        group_inputs[index][0].box.y2 = 0;
        group_inputs[index][1].command = REDMCSB_F0672_F0673_COMMAND_NONE;
        group_inputs[index][1].box.x1 = 0;
        group_inputs[index][1].box.x2 = 0;
        group_inputs[index][1].box.y1 = 0;
        group_inputs[index][1].box.y2 = 0;
        groups[index].inputs = group_inputs[index];
        groups[index].input_count = 2U;
    }
    if (!expect(redmcsb_f0672_initialize_all_mouse_input_pc34_compat(
                    groups, &runtime),
                "F0672 resolves all nine source mouse-input groups") ||
        !expect(fixture.call_count ==
                    REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT,
                "F0672 visits exactly nine source groups") ||
        !expect(fixture.seen_zone[0] == 100 && fixture.seen_zone[8] == 108 &&
                    group_inputs[0][0].box.x1 == 100 &&
                    group_inputs[8][0].box.y2 == 111,
                "F0672 preserves source group order and F0673 endpoint geometry")) {
        return 1;
    }

    group_inputs[4][1].command = 7;
    if (!expect(!redmcsb_f0672_initialize_all_mouse_input_pc34_compat(
                     groups, &runtime),
                "bounded adapter rejects a group without the source sentinel")) {
        return 1;
    }
    return 0;
}
