#ifndef FIRESTAFF_REDMCSB_F0672_F0673_MOUSE_INPUT_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0672_F0673_MOUSE_INPUT_PC34_COMPAT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB COMMAND.C F0672/F0673, PC I34E/I34M route. */
enum {
    REDMCSB_F0672_F0673_COMMAND_NONE = 0,
    REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT = 9
};

typedef struct redmcsb_f0672_f0673_box_pc34_compat {
    int16_t x1;
    int16_t x2;
    int16_t y1;
    int16_t y2;
} redmcsb_f0672_f0673_box_pc34_compat;

typedef struct redmcsb_f0672_f0673_mouse_input_pc34_compat {
    int16_t command;
    redmcsb_f0672_f0673_box_pc34_compat box;
} redmcsb_f0672_f0673_mouse_input_pc34_compat;

typedef struct redmcsb_f0672_f0673_mouse_input_group_pc34_compat {
    redmcsb_f0672_f0673_mouse_input_pc34_compat *inputs;
    size_t input_count;
} redmcsb_f0672_f0673_mouse_input_group_pc34_compat;

typedef int (*redmcsb_f0672_f0673_get_zone_pc34_compat)(
    void *context, int16_t zone_index, int16_t xyz[4]);

typedef struct redmcsb_f0672_f0673_runtime_pc34_compat {
    redmcsb_f0672_f0673_get_zone_pc34_compat get_zone;
    void *context;
    int16_t viewport_screen_x;
    int16_t viewport_screen_y;
    int16_t screen_width;
    int16_t screen_height;
    int16_t viewport_width;
    int16_t viewport_height;
} redmcsb_f0672_f0673_runtime_pc34_compat;

/*
 * Resolves one source MOUSE_INPUT table. input_count bounds the original
 * command-none walk and must include the terminating COMMAND_NONE record.
 */
int redmcsb_f0673_set_mouse_input_boxes_from_zone_pc34_compat(
    redmcsb_f0672_f0673_mouse_input_pc34_compat *inputs,
    size_t input_count,
    const redmcsb_f0672_f0673_runtime_pc34_compat *runtime);

/* Resolves the nine F0672 source table groups in their COMMAND.C order. */
int redmcsb_f0672_initialize_all_mouse_input_pc34_compat(
    const redmcsb_f0672_f0673_mouse_input_group_pc34_compat groups[
        REDMCSB_F0672_F0673_MOUSE_INPUT_GROUP_COUNT],
    const redmcsb_f0672_f0673_runtime_pc34_compat *runtime);

const char *redmcsb_f0672_f0673_mouse_input_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
